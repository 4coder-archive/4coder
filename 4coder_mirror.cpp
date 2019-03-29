/*
4coder_mirror.cpp - Commands and helpers for parsing jump locations from 
compiler errors, sticking markers on jump locations, and jumping to them.
*/

// TOP

static Managed_Variable_ID mirror_root_loc = 0;
#define DefaultMirrorRootName "DEFAULT.mirror_root"

static b32
mirror_edit_handler(Application_Links *app, Buffer_ID buffer_id, i32 start, i32 one_past_last, String text);

////////////////////////////////

static void
mirror__global_init(Application_Links *app){
    if (mirror_root_loc == 0){
        mirror_root_loc = managed_variable_create_or_get_id(app, DefaultMirrorRootName, 0);
    }
}

static Managed_Object
mirror__check_scope_for_mirror(Application_Links *app, Managed_Scope scope){
    Managed_Object result = 0;
    Managed_Object mirror = 0;
    if (managed_variable_get(app, scope, mirror_root_loc, &mirror)){
        if (mirror != 0){
            Managed_Object_Type object_type = managed_object_get_type(app, mirror);
            if (object_type == ManagedObjectType_Memory){
                result = mirror;
            }
            else{
                managed_variable_set(app, scope, mirror_root_loc, 0);
            }
        }
    }
    return(result);
}

static b32
mirror_init__inner(Application_Links *app, Buffer_ID buffer, Mirror_Flags flags, Managed_Object *mirror_object_out){
    b32 result = false;
    Managed_Scope scope = 0;
    if (buffer_get_managed_scope(app, buffer, &scope)){
        Managed_Object mirror = mirror__check_scope_for_mirror(app, scope);
        if (mirror == 0){
            mirror = alloc_managed_memory_in_scope(app, scope, sizeof(Mirror), 1);
            if (managed_variable_set(app, scope, mirror_root_loc, mirror)){
                Mirror mirror_data = {};
                mirror_data.mirror_buffer_id = buffer;
                mirror_data.mode = MirrorMode_Constructing;
                mirror_data.flags = flags;
                mirror_data.mirror_scope = create_user_managed_scope(app);
                *mirror_object_out = mirror;
                managed_object_store_data(app, mirror, 0, 1, &mirror_data);
                buffer_set_edit_handler(app, buffer, mirror_edit_handler);
                result = true;
            }
        }
    }
    return(result);
}

static b32
mirror_end__inner(Application_Links *app, Managed_Object mirror){
    b32 result = false;
    Managed_Scope scope = managed_object_get_containing_scope(app, mirror);
    Managed_Object mirror_check = mirror__check_scope_for_mirror(app, scope);
    if (mirror_check == mirror){
        Mirror mirror_data = {};
        if (managed_object_load_data(app, mirror, 0, 1, &mirror_data)){
            destroy_user_managed_scope(app, mirror_data.mirror_scope);
            buffer_set_edit_handler(app, mirror_data.mirror_buffer_id, 0);
            result = true;
        }
        managed_variable_set(app, scope, mirror_root_loc, 0);
        managed_object_free(app, mirror);
    }
    return(result);
}

struct Mirror__Binary_Search_Result{
    b32 abutting;
    i32 index;
};

static Mirror__Binary_Search_Result
mirror__binary_search_max_point_below(Marker *ranges, i32 target, i32 first, i32 one_past_last){
    Mirror__Binary_Search_Result result = {};
    result.index = -1;
    first = (first*2) - 1;
    one_past_last = one_past_last*2;
    for (;;){
        i32 mid = (first + one_past_last)/2;
        i32 pos = -1;
        if (mid != -1){
            Marker *marker = ranges + mid;
            pos = marker->pos;
        }
        if (pos > target){
            one_past_last = mid;
        }
        else if (pos < target){
            first = mid;
        }
        else{
            result.abutting = true;
            result.index = mid;
            break;
        }
        if (first + 1 >= one_past_last){
            result.index = first;
            break;
        }
    }
    return(result);
}

static Mirror__Binary_Search_Result
mirror__binary_search_min_point_above(Marker *ranges, i32 target, i32 first, i32 one_past_last,
                                      i32 fake_index, i32 fake_pos){
    Mirror__Binary_Search_Result result = {};
    result.index = -1;
    first = first*2;
    one_past_last = (one_past_last*2) + 1;
    for (;;){
        i32 mid = (first + one_past_last - 1)/2;
        i32 pos = fake_pos;
        if (mid != fake_index){
            Marker *marker = ranges + mid;
            pos = marker->pos;
        }
        if (pos > target){
            one_past_last = mid + 1;
        }
        else if (pos < target){
            first = mid + 1;
        }
        else{
            result.abutting = true;
            result.index = mid;
            break;
        }
        if (first + 1 >= one_past_last){
            result.index = first;
            break;
        }
    }
    return(result);
}

static Mirror_Hot
mirror__hot_from_data(Application_Links *app, Arena *arena, Mirror mirror_data){
    Mirror_Hot result = {};
    Mirror_Hot mirror_hot = {};
    mirror_hot.count = mirror_data.count;
    mirror_hot.source_buffer_ids = push_array(arena, Buffer_ID, mirror_hot.count);
    mirror_hot.mirror_ranges = push_array(arena, Marker, (mirror_hot.count)*2);
    mirror_hot.source_ranges = push_array(arena, Managed_Object, mirror_hot.count);
    if (managed_object_load_data(app, mirror_data.source_buffer_ids, 0, mirror_hot.count, mirror_hot.source_buffer_ids) &&
        managed_object_load_data(app, mirror_data.mirror_ranges, 0, mirror_hot.count*2, mirror_hot.mirror_ranges) &&
        managed_object_load_data(app, mirror_data.source_ranges, 0, mirror_hot.count, mirror_hot.source_ranges)){
        result = mirror_hot;
    }
    return(result);
}

static b32
mirror__min_max_point_indices_not_intersecting(i32 above_point, i32 below_point, i32 *range_index_out){
    b32 result = false;
    if (below_point < above_point && (above_point%2) == 0 && ((below_point + 2)%2) == 1){
        *range_index_out = above_point/2;
        result = true;
    }
    return(result);
}

static b32
mirror__min_max_point_indices_contained(i32 above_point, i32 below_point, i32 *range_index_out){
    b32 result = false;
    if (below_point < above_point && (above_point%2) == 1 && ((below_point + 2)%2) == 0){
        *range_index_out = above_point/2;
        result = true;
    }
    return(result);
}

struct Mirror__Check_Range_Result{
    b32 passed_checks;
    i32 insert_index;
};

static Mirror__Check_Range_Result
mirror__check_range_to_add(Application_Links *app, Arena *scratch, i32 mirror_first, i32 source_first, i32 length,
                           Buffer_Summary *source_buffer, Buffer_Summary *mirror_buffer, Mirror_Hot *mirror_hot,
                           i32 collidable_indices_first, b32 auto_trust_text){
    // check the new range for the following rules and determine the insert index
    Mirror__Check_Range_Result result = {};
    result.insert_index = -1;
    
    // 1. the source range must be entirely contained in the source buffer and
    //    the mirror range must be entirely contained in the mirror buffer
    i32 source_one_past_last = source_first + length;
    i32 mirror_one_past_last = mirror_first + length;
    if (0 <= source_first && source_one_past_last <= source_buffer->size &&
        0 <= mirror_first && mirror_one_past_last <= mirror_buffer->size){
        
        // 2. the mirror range must not overlap or abut any existing mirror ranges
        b32 independent_range = false;
        if (mirror_hot->count == 0){
            independent_range = true;
            result.insert_index = 0;
        }
        else{
            i32 fake_index = mirror_hot->count*2;
            i32 fake_pos = mirror_buffer->size + 1;
            Mirror__Binary_Search_Result above_point = mirror__binary_search_min_point_above(mirror_hot->mirror_ranges, mirror_first,
                                                                                             collidable_indices_first, mirror_hot->count,
                                                                                             fake_index, fake_pos);
            Mirror__Binary_Search_Result below_point = {};
            if (!above_point.abutting){
                below_point = mirror__binary_search_max_point_below(mirror_hot->mirror_ranges, mirror_first,
                                                                    collidable_indices_first, mirror_hot->count);
                if (!below_point.abutting){
                    if (mirror__min_max_point_indices_not_intersecting(above_point.index, below_point.index, &result.insert_index)){
                        independent_range = true;
                        Assert(0 <= result.insert_index && result.insert_index <= mirror_hot->count);
                    }
                }
            }
        }
        
        if (independent_range){
            // 3. the text in the source range must exactly match the text in the mirror range
            if (auto_trust_text){
                result.passed_checks = true;
            }
            else{
                char *buffer_1 = push_array(scratch, char, length);
                char *buffer_2 = push_array(scratch, char, length);
                if (buffer_read_range(app, source_buffer->buffer_id, source_first, source_first + length, buffer_1)){
                    if (buffer_read_range(app, mirror_buffer->buffer_id, mirror_first, mirror_first + length, buffer_2)){
                        if (memcmp(buffer_1, buffer_2, length) == 0){
                            result.passed_checks = true;
                        }
                    }
                }
            }
        }
    }
    return(result);
}

static b32
mirror_add_range__inner_check_optimization(Application_Links *app, Managed_Object mirror, Buffer_ID source,
                                           i32 mirror_first, i32 source_first, i32 length,
                                           i32 collidable_indices_first, b32 auto_trust_text, i32 *insert_index_out){
    b32 result = false;
    if (length > 0){
        Buffer_Summary source_buffer = {};
        if (get_buffer_summary(app, source, AccessAll, &source_buffer)){
            Managed_Scope scope = managed_object_get_containing_scope(app, mirror);
            Managed_Object mirror_check = mirror__check_scope_for_mirror(app, scope);
            Mirror mirror_data = {};
            if (mirror_check == mirror && managed_object_load_data(app, mirror, 0, 1, &mirror_data)){
                Buffer_Summary mirror_buffer = {};
                Buffer_ID mirror_id = mirror_data.mirror_buffer_id;
                if (get_buffer_summary(app, mirror_id, AccessAll, &mirror_buffer)){
                    Arena *arena = context_get_arena(app);
                    Temp_Memory_Arena temp = begin_temp_memory(arena);
                    
                    // read mirror data into hot structure
                    Mirror_Hot mirror_hot = {};
                    mirror_hot.count = mirror_data.count;
                    mirror_hot.source_buffer_ids = push_array(arena, Buffer_ID, mirror_hot.count);
                    mirror_hot.mirror_ranges = push_array(arena, Marker, (mirror_hot.count)*2);
                    mirror_hot.source_ranges = push_array(arena, Managed_Object, mirror_hot.count);
                    
                    b32 load_success = false;
                    if (mirror_hot.count == 0){
                        load_success = true;
                    }
                    else{
                        load_success = (managed_object_load_data(app, mirror_data.source_buffer_ids, 0, mirror_hot.count  , mirror_hot.source_buffer_ids) &&
                                        managed_object_load_data(app, mirror_data.mirror_ranges    , 0, mirror_hot.count*2, mirror_hot.mirror_ranges    ) &&
                                        managed_object_load_data(app, mirror_data.source_ranges    , 0, mirror_hot.count  , mirror_hot.source_ranges    ));
                    }
                    
                    if (load_success){
                        Mirror__Check_Range_Result check = mirror__check_range_to_add(app, arena, mirror_first, source_first, length,
                                                                                      &source_buffer, &mirror_buffer, &mirror_hot,
                                                                                      collidable_indices_first, auto_trust_text);
                        
                        if (check.passed_checks){
                            // insert the new range at the insert index
                            b32 r = true;
                            i32 insert_index = check.insert_index;
                            *insert_index_out = insert_index;
                            
                            Marker mirror_range[2] = {};
                            mirror_range[0].pos = mirror_first;
                            mirror_range[0].lean_right = false;
                            mirror_range[1].pos = mirror_first + length;
                            mirror_range[1].lean_right = true;
                            
                            Marker source_range[2] = {};
                            source_range[0].pos = source_first;
                            source_range[0].lean_right = false;
                            source_range[1].pos = source_first + length;
                            source_range[1].lean_right = true;
                            
                            Managed_Scope scopes[3] = {};
                            scopes[0] = scope;
                            scopes[1] = mirror_data.mirror_scope;
                            buffer_get_managed_scope(app, source, &scopes[2]);
                            
                            Managed_Scope source_sub_scope = get_managed_scope_with_multiple_dependencies(app, scopes, 3);
                            
                            Managed_Object new_source_range = alloc_buffer_markers_on_buffer(app, source, 2, &source_sub_scope);
                            r = r && managed_object_store_data(app, new_source_range, 0, 2, &source_range);
                            
                            if (mirror_data.count + 1 > mirror_data.max){
                                if (mirror_data.count != 0){
                                    r = r && managed_object_free(app, mirror_data.source_buffer_ids);
                                    r = r && managed_object_free(app, mirror_data.mirror_ranges);
                                    r = r && managed_object_free(app, mirror_data.source_ranges);
                                }
                                
                                Managed_Scope mirror_sub_scope = get_managed_scope_with_multiple_dependencies(app, scopes, 2);
                                i32 new_max = 256;
                                if (new_max <= mirror_data.max){
                                    new_max = mirror_data.max*2;
                                }
                                mirror_data.source_buffer_ids = alloc_managed_memory_in_scope(app, mirror_sub_scope, sizeof(Buffer_ID), new_max);
                                mirror_data.mirror_ranges = alloc_buffer_markers_on_buffer(app, mirror_id, new_max*2, &mirror_sub_scope);
                                mirror_data.source_ranges = alloc_managed_memory_in_scope(app, mirror_sub_scope, sizeof(Managed_Object), new_max);
                                
                                mirror_data.max = new_max;
                                
                                // head ranges
                                i32 head_count = insert_index;
                                if (head_count > 0){
                                    r = r && managed_object_store_data(app, mirror_data.source_buffer_ids, 0, head_count  , mirror_hot.source_buffer_ids);
                                    r = r && managed_object_store_data(app, mirror_data.mirror_ranges    , 0, head_count*2, mirror_hot.mirror_ranges    );
                                    r = r && managed_object_store_data(app, mirror_data.source_ranges    , 0, head_count  , mirror_hot.source_ranges    );
                                }
                            }
                            
                            // tail ranges
                            i32 tail_count = mirror_hot.count - insert_index;
                            if (tail_count > 0){
                                i32 to = insert_index + 1;
                                i32 from = insert_index;
                                r = r && managed_object_store_data(app, mirror_data.source_buffer_ids, to  , tail_count  , mirror_hot.source_buffer_ids + from);
                                r = r && managed_object_store_data(app, mirror_data.mirror_ranges    , to*2, tail_count*2, mirror_hot.mirror_ranges + from*2  );
                                r = r && managed_object_store_data(app, mirror_data.source_ranges    , to  , tail_count  , mirror_hot.source_ranges + from    );
                            }
                            
                            // new range
                            r = r && managed_object_store_data(app, mirror_data.source_buffer_ids, insert_index  , 1, &source          );
                            r = r && managed_object_store_data(app, mirror_data.mirror_ranges    , insert_index*2, 2, mirror_range     );
                            r = r && managed_object_store_data(app, mirror_data.source_ranges    , insert_index  , 1, &new_source_range);
                            
                            mirror_data.count += 1;
                            
                            managed_object_store_data(app, mirror, 0, 1, &mirror_data);
                            
                            result = r;
                        }
                    }
                    
                    end_temp_memory(temp);
                }
            }
        }
    }
    return(result);
}

static b32
mirror_add_range__inner(Application_Links *app, Managed_Object mirror, Buffer_ID source,
                        i32 mirror_first, i32 source_first, i32 length){
    i32 ignore = 0;
    return(mirror_add_range__inner_check_optimization(app, mirror, source, mirror_first, source_first, length, 0, false, &ignore));
}

static b32
mirror_set_mode__inner(Application_Links *app, Managed_Object mirror, Mirror_Mode mode){
    b32 result = false;
    Managed_Scope scope = managed_object_get_containing_scope(app, mirror);
    Managed_Object mirror_check = mirror__check_scope_for_mirror(app, scope);
    if (mirror_check == mirror){
        Mirror mirror_data = {};
        if (managed_object_load_data(app, mirror, 0, 1, &mirror_data)){
            mirror_data.mode = mode;
            if (managed_object_store_data(app, mirror, 0, 1, &mirror_data)){
                result = true;
            }
        }
    }
    return(result);
}

static b32
mirror_get_mode__inner(Application_Links *app, Managed_Object mirror, Mirror_Mode *mode_out){
    b32 result = false;
    Managed_Scope scope = managed_object_get_containing_scope(app, mirror);
    Managed_Object mirror_check = mirror__check_scope_for_mirror(app, scope);
    if (mirror_check == mirror){
        Mirror mirror_data = {};
        if (managed_object_load_data(app, mirror, 0, 1, &mirror_data)){
            *mode_out = mirror_data.mode;
            result = true;
        }
    }
    return(result);
}

static b32
mirror_set_flags__inner(Application_Links *app, Managed_Object mirror, Mirror_Flags flags){
    b32 result = false;
    Managed_Scope scope = managed_object_get_containing_scope(app, mirror);
    Managed_Object mirror_check = mirror__check_scope_for_mirror(app, scope);
    if (mirror_check == mirror){
        Mirror mirror_data = {};
        if (managed_object_load_data(app, mirror, 0, 1, &mirror_data)){
            mirror_data.flags = flags;
            if (managed_object_store_data(app, mirror, 0, 1, &mirror_data)){
                result = true;
            }
        }
    }
    return(result);
}

static b32
mirror_get_flags__inner(Application_Links *app, Managed_Object mirror, Mirror_Flags *flags_out){
    b32 result = false;
    Managed_Scope scope = managed_object_get_containing_scope(app, mirror);
    Managed_Object mirror_check = mirror__check_scope_for_mirror(app, scope);
    if (mirror_check == mirror){
        Mirror mirror_data = {};
        if (managed_object_load_data(app, mirror, 0, 1, &mirror_data)){
            *flags_out = mirror_data.flags;
            result = true;
        }
    }
    return(result);
}

////////////////////////////////

static b32
mirror_init(Application_Links *app, Buffer_ID buffer, Mirror_Flags flags, Managed_Object *mirror_object_out){
    mirror__global_init(app);
    return(mirror_init__inner(app, buffer, flags, mirror_object_out));
}

static b32
mirror_end(Application_Links *app, Managed_Object mirror){
    mirror__global_init(app);
    return(mirror_end__inner(app, mirror));
}

static b32
mirror_add_range(Application_Links *app, Managed_Object mirror, Buffer_ID source,
                 i32 mirror_first, i32 source_first, i32 length){
    mirror__global_init(app);
    return(mirror_add_range__inner(app, mirror, source, mirror_first, source_first, length));
}

static b32
mirror_set_mode(Application_Links *app, Managed_Object mirror, Mirror_Mode mode){
    mirror__global_init(app);
    return(mirror_set_mode__inner(app, mirror, mode));
}

static b32
mirror_get_mode(Application_Links *app, Managed_Object mirror, Mirror_Mode *mode_out){
    mirror__global_init(app);
    return(mirror_get_mode__inner(app, mirror, mode_out));
}

static b32
mirror_set_flags(Application_Links *app, Managed_Object mirror, Mirror_Flags flags){
    mirror__global_init(app);
    return(mirror_set_flags__inner(app, mirror, flags));
}

static b32
mirror_get_flags(Application_Links *app, Managed_Object mirror, Mirror_Flags *flags_out){
    mirror__global_init(app);
    return(mirror_get_flags__inner(app, mirror, flags_out));
}

////////////////////////////////

static b32
mirror_buffer_create(Application_Links *app, String buffer_name, Mirror_Flags flags, Buffer_ID *mirror_buffer_id_out){
    mirror__global_init(app);
    b32 result = false;
    Buffer_ID new_buffer = 0;
    if (!get_buffer_by_name(app, buffer_name, AccessAll, &new_buffer)){
        if (new_buffer == 0){
            if (create_buffer(app, buffer_name, BufferCreate_NeverAttachToFile|BufferCreate_AlwaysNew, &new_buffer)){
                Managed_Object ignore_object = 0;
                if (mirror_init__inner(app, new_buffer, flags, &ignore_object)){
                    *mirror_buffer_id_out = new_buffer;
                    result = true;
                }
            }
        }
    }
    return(false);
}

static Managed_Scope
mirror__buffer_to_object(Application_Links *app, Buffer_ID buffer){
    Managed_Object result = 0;
    Managed_Scope scope = 0;
    if (buffer_get_managed_scope(app, buffer, &scope)){
        result = mirror__check_scope_for_mirror(app, scope);
    }
    return(result);
}

static b32
mirror_buffer_end(Application_Links *app, Buffer_ID mirror){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        result = mirror_end__inner(app, mirror_object);
    }
    return(result);
}

static b32
mirror_buffer_add_range_exact(Application_Links *app, Buffer_ID mirror, Buffer_ID source,
                              i32 mirror_first, i32 source_first, i32 length){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        result = mirror_add_range__inner(app, mirror_object, source, mirror_first, source_first, length);
    }
    return(result);
}

static i32
mirror__range_loose_get_length(Application_Links *app, Buffer_ID mirror, Buffer_ID source,
                               i32 mirror_first, i32 source_first, i32 max_length){
    i32 result = 0;
    Arena *arena = context_get_arena(app);
    Temp_Memory_Arena temp = begin_temp_memory(arena);
    char *buffer_1 = push_array(arena, char, max_length);
    char *buffer_2 = push_array(arena, char, max_length);
    if (buffer_read_range(app, source, source_first, source_first + max_length, buffer_1)){
        if (buffer_read_range(app, mirror, mirror_first, mirror_first + max_length, buffer_2)){
            for (; result < max_length;
                 result += 1){
                if (buffer_1[result] != buffer_2[result]){
                    break;
                }
            }
        }
    }
    end_temp_memory(temp);
    return(result);
}

static b32
mirror_buffer_add_range_loose(Application_Links *app, Buffer_ID mirror, Buffer_ID source,
                              i32 mirror_first, i32 source_first, i32 max_length){
    mirror__global_init(app);
    b32 result = false;
    i32 length = mirror__range_loose_get_length(app, mirror, source, mirror_first, source_first, max_length);
    if (length > 0){
        Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
        if (mirror_object != 0){
            result = mirror_add_range__inner(app, mirror_object, source, mirror_first, source_first, length);
        }
    }
    return(result);
}

static b32
mirror_buffer_insert_range(Application_Links *app, Buffer_ID mirror, Buffer_ID source,
                           i32 mirror_insert_pos, i32 source_first, i32 length){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        Mirror_Mode mode = 0;
        if (mirror_get_mode__inner(app, mirror_object, &mode)){
            if (mode == MirrorMode_Constructing){
                b32 did_insert = false;
                {
                    Arena *arena = context_get_arena(app);
                    Temp_Memory_Arena temp = begin_temp_memory(arena);
                    char *buffer = push_array(arena, char, length);
                    if (buffer_read_range(app, source, source_first, source_first + length, buffer)){
                        String string = make_string(buffer, length);
                        if (buffer_replace_range(app, mirror, mirror_insert_pos, mirror_insert_pos, string)){
                            did_insert = true;
                        }
                    }
                    end_temp_memory(temp);
                }
                if (did_insert){
                    result = mirror_add_range__inner(app, mirror_object, source, mirror_insert_pos, source_first, length);
                }
            }
        }
    }
    return(result);
}

static b32
mirror_buffer_set_mode(Application_Links *app, Buffer_ID mirror, Mirror_Mode mode){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        result = mirror_set_mode__inner(app, mirror_object, mode);
    }
    return(result);
}

static b32
mirror_buffer_get_mode(Application_Links *app, Buffer_ID mirror, Mirror_Mode *mode_out){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        result = mirror_get_mode__inner(app, mirror_object, mode_out);
    }
    return(result);
}

static b32
mirror_buffer_set_flags(Application_Links *app, Buffer_ID mirror, Mirror_Flags flags){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        result = mirror_set_flags__inner(app, mirror_object, flags);
    }
    return(result);
}

static b32
mirror_buffer_get_flags(Application_Links *app, Buffer_ID mirror, Mirror_Flags *flags_out){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        result = mirror_get_flags__inner(app, mirror_object, flags_out);
    }
    return(result);
}

static b32
mirror_buffer_refresh(Application_Links *app, Buffer_ID mirror){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        // TODO(allen):
    }
    return(result);
}

static void
mirror_quick_sort_mirror_ranges(Mirror_Range *ranges, i32 first, i32 one_past_last){
    i32 last = one_past_last - 1;
    if (first < last){
        i32 pivot_mirror_first = ranges[last].mirror_first;
        i32 j = first;
        for (i32 i = first; i < last; i += 1){
            i32 mirror_first = ranges[i].mirror_first;
            if (mirror_first < pivot_mirror_first){
                if (j < i){
                    Mirror_Range t = ranges[i];
                    ranges[i] = ranges[j];
                    ranges[j] = t;
                }
                j += 1;
            }
        }
        {
            Mirror_Range t = ranges[last];
            ranges[last] = ranges[j];
            ranges[j] = t;
        }
        i32 pivot = j;
        mirror_quick_sort_mirror_ranges(ranges, first, pivot);
        mirror_quick_sort_mirror_ranges(ranges, pivot + 1, one_past_last);
    }
}

static b32
mirror__check_range_array_sorting(Mirror_Range *ranges, i32 count){
    b32 result = true;
    i32 prev_pos = -1;
    for (i32 i = 0; i < count; i += 1){
        i32 first = ranges[i].mirror_first;
        i32 one_past_last = first + ranges[i].length;
        if (prev_pos < first && first <= one_past_last){
            prev_pos = one_past_last;
        }
        else{
            result = false;
            break;
        }
    }
    return(result);
}

static b32
mirror_buffer_add_range_exact_array(Application_Links *app, Buffer_ID mirror, Mirror_Range *ranges, i32 count){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        if (mirror__check_range_array_sorting(ranges, count)){
            b32 r = true;
            Mirror_Range *range = ranges;
            i32 safe_to_ignore_index = 0;
            for (i32 i = 0; i < count; i += 1, range += 1){
                i32 new_range_index = 0;
                if (range->length > 0){
                    if (!mirror_add_range__inner_check_optimization(app, mirror_object, range->source_buffer_id,
                                                                    range->mirror_first, range->source_first, range->length,
                                                                    safe_to_ignore_index, false, &new_range_index)){
                        r = false;
                    }
                    else{
                        safe_to_ignore_index = new_range_index + 1;
                    }
                }
            }
            result = r;
        }
    }
    return(result);
}

static b32
mirror_buffer_add_range_loose_array(Application_Links *app, Buffer_ID mirror, Mirror_Range *ranges, i32 count){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        {
            Mirror_Range *range = ranges;
            for (i32 i = 0; i < count; i += 1, range += 1){
                range->length = mirror__range_loose_get_length(app, mirror, range->source_buffer_id, range->mirror_first, range->source_first, range->length);
            }
        }
        if (mirror__check_range_array_sorting(ranges, count)){
            b32 r = true;
            Mirror_Range *range = ranges;
            i32 safe_to_ignore_index = 0;
            for (i32 i = 0; i < count; i += 1, range += 1){
                i32 new_range_index = 0;
                if (range->length > 0){
                    if (!mirror_add_range__inner_check_optimization(app, mirror_object, range->source_buffer_id,
                                                                    range->mirror_first, range->source_first, range->length,
                                                                    safe_to_ignore_index, false, &new_range_index)){
                        r = false;
                    }
                    else{
                        safe_to_ignore_index = new_range_index + 1;
                    }
                }
            }
            result = r;
        }
    }
    return(result);
}

static b32
mirror_buffer_insert_range_array(Application_Links *app, Buffer_ID mirror, Mirror_Range *ranges, i32 count){
    mirror__global_init(app);
    b32 result = false;
    Managed_Object mirror_object = mirror__buffer_to_object(app, mirror);
    if (mirror_object != 0){
        Mirror_Mode mode = 0;
        if (mirror_get_mode__inner(app, mirror_object, &mode)){
            if (mode == MirrorMode_Constructing){
                if (mirror__check_range_array_sorting(ranges, count)){
                    b32 r = true;
                    Mirror_Range *range = ranges;
                    i32 safe_to_ignore_index = 0;
                    i32 total_shift = 0;
                    Arena *arena = context_get_arena(app);
                    Temp_Memory_Arena temp = begin_temp_memory(arena);
                    for (i32 i = 0; i < count; i += 1, range += 1){
                        i32 mirror_first = range->mirror_first + total_shift;
                        b32 did_insert = false;
                        {
                            Temp_Memory_Arena buffer_temp = begin_temp_memory(arena);
                            char *buffer = push_array(arena, char, range->length);
                            if (buffer_read_range(app, range->source_buffer_id, range->source_first, range->source_first + range->length, buffer)){
                                String string = make_string(buffer, range->length);
                                if (buffer_replace_range(app, mirror, mirror_first, mirror_first, string)){
                                    did_insert = true;
                                }
                            }
                            end_temp_memory(buffer_temp);
                        }
                        i32 new_range_index = 0;
                        if (range->length > 0){
                            if (!mirror_add_range__inner_check_optimization(app, mirror_object, range->source_buffer_id,
                                                                            mirror_first, range->source_first, range->length,
                                                                            safe_to_ignore_index, false, &new_range_index)){
                                r = false;
                            }
                            else{
                                safe_to_ignore_index = new_range_index + 1;
                            }
                        }
                        total_shift += range->length;
                    }
                    end_temp_memory(temp);
                    result = r;
                }
            }
        }
    }
    return(result);
}

////////////////////////////////

static b32
mirror_edit_handler(Application_Links *app, Buffer_ID buffer_id, i32 first, i32 one_past_last, String text){
    mirror__global_init(app);
    b32 result = false;
    Buffer_Summary mirror_buffer = {};
    if (get_buffer_summary(app, buffer_id, AccessAll, &mirror_buffer)){
        Managed_Object mirror = mirror__buffer_to_object(app, buffer_id);
        if (mirror != 0){
            Mirror_Mode mode = 0;
            if (mirror_get_mode__inner(app, mirror, &mode)){
                Mirror mirror_data = {};
                if (managed_object_load_data(app, mirror, 0, 1, &mirror_data)){
                    Arena *arena = context_get_arena(app);
                    Temp_Memory_Arena temp = begin_temp_memory(arena);
                    
                    Mirror_Hot mirror_hot = mirror__hot_from_data(app, arena, mirror_data);
                    switch (mode){
                        case MirrorMode_Constructing:
                        {
                            b32 unreflected_range = false;
                            if (mirror_hot.count == 0){
                                unreflected_range = true;
                            }
                            else{
                                i32 fake_index = mirror_hot.count*2;
                                i32 fake_pos = mirror_buffer.size + 1;
                                Mirror__Binary_Search_Result above_point = mirror__binary_search_min_point_above(mirror_hot.mirror_ranges, first,
                                                                                                                 0, mirror_hot.count,
                                                                                                                 fake_index, fake_pos);
                                Mirror__Binary_Search_Result below_point = mirror__binary_search_max_point_below(mirror_hot.mirror_ranges, one_past_last,
                                                                                                                 0, mirror_hot.count);
                                i32 ignore = 0;
                                if (mirror__min_max_point_indices_not_intersecting(above_point.index, below_point.index, &ignore)){
                                    unreflected_range = true;
                                }
                            }
                            
                            if (unreflected_range){
                                result = buffer_replace_range(app, buffer_id, first, one_past_last, text);
                            }
                        }break;
                        
                        case MirrorMode_Reflecting:
                        {
                            b32 newlines_are_jumps = ((mirror_data.flags & MirrorFlag_NewlinesAreJumps) != 0);
                            
                            b32 blocked = false;
                            if (newlines_are_jumps){
                                if (has_substr(text, make_lit_string("\n"))){
                                    blocked = true;
                                }
                                if (match(text, "\n")){
                                    User_Input in = get_command_input(app);
                                    if (in.key.modifiers[MDFR_SHIFT_INDEX]){
                                        goto_jump_at_cursor_same_panel_sticky(app);
                                    }
                                    else{
                                        goto_jump_at_cursor_sticky(app);
                                    }
                                    lock_jump_buffer(app, buffer_id);
                                }
                            }
                            
                            if (!blocked){
                                b32 reflected_range = false;
                                i32 range_index = 0;
                                if (mirror_hot.count > 0){
                                    i32 fake_index = mirror_hot.count*2;
                                    i32 fake_pos = mirror_buffer.size + 1;
                                    Mirror__Binary_Search_Result above_point = mirror__binary_search_min_point_above(mirror_hot.mirror_ranges, first,
                                                                                                                     0, mirror_hot.count,
                                                                                                                     fake_index, fake_pos);
                                    Mirror__Binary_Search_Result below_point = mirror__binary_search_max_point_below(mirror_hot.mirror_ranges, one_past_last,
                                                                                                                     0, mirror_hot.count);
                                    if (mirror__min_max_point_indices_contained(above_point.index, below_point.index, &range_index)){
                                        reflected_range = true;
                                    }
                                }
                                
                                if (reflected_range){
                                    // check that the range_index is valid and get source if it is
                                    Managed_Object source_range_object = mirror_hot.source_ranges[range_index];
                                    if (managed_object_get_type(app, source_range_object) == ManagedObjectType_Markers){
                                        Buffer_ID source = mirror_hot.source_buffer_ids[range_index];
                                        Marker *mirror_range = mirror_hot.mirror_ranges + range_index*2;
                                        Marker source_range[2];
                                        if (managed_object_load_data(app, source_range_object, 0, 2, source_range)){
                                            i32 base_source_first = source_range[0].pos;
                                            //i32 base_source_one_past_last = source_range[1].pos;
                                            
                                            i32 base_mirror_first = mirror_range[0].pos;
                                            //i32 base_mirror_one_past_last = mirror_range[1].pos;
                                            
                                            i32 source_first = base_source_first + (first - base_mirror_first);
                                            i32 source_one_past_last = source_first + (one_past_last - first);
                                            
                                            global_history_edit_group_begin(app);
                                            if (buffer_replace_range(app, source, source_first, source_one_past_last, text)){
                                                result = buffer_replace_range(app, buffer_id, first, one_past_last, text);
                                            }
                                            global_history_edit_group_end(app);
                                        }
                                    }
                                }
                            }
                        }break;
                    }
                    
                    end_temp_memory(temp);
                }
            }
        }
    }
    return(result);
}

// BOTTOM

