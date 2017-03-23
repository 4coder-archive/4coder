/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 03.01.2017
 *
 * Working_Set data structure for 4coder
 *
 */

// TOP

//
// Working_Set of files
//

struct Non_File_Table_Entry{
    String name;
    Buffer_Slot_ID id;
};

struct File_Array{
    Editing_File *files;
    i32 size;
};

struct Working_Set{
    File_Array *file_arrays;
    i32 file_count, file_max;
    i16 array_count, array_max;
    
    File_Node free_sentinel;
    File_Node used_sentinel;
    
    Table canon_table;
    Table name_table;
    
    String clipboards[64];
    i32 clipboard_size, clipboard_max_size;
    i32 clipboard_current, clipboard_rolling;
    
    //u64 unique_file_counter;
    
    File_Node *sync_check_iter;
    
    i32 default_display_width;
    i32 default_minimum_base_display_width;
};

struct File_Name_Entry{
    String name;
    Buffer_Slot_ID id;
};


internal i32
tbl_name_compare(void *a, void *b, void *arg){
    String *fa = (String*)a;
    File_Name_Entry *fb = (File_Name_Entry*)b;
    
    i32 result = 1;
    if (match_ss(*fa, fb->name)){
        result = 0;
    }
    
    return(result);
}

internal void
working_set_extend_memory(Working_Set *working_set, Editing_File *new_space, i16 number_of_files){
    Assert(working_set->array_count < working_set->array_max);
    
    i16 high_part = working_set->array_count++;
    working_set->file_arrays[high_part].files = new_space;
    working_set->file_arrays[high_part].size = number_of_files;
    
    working_set->file_max += number_of_files;
    
    Buffer_Slot_ID id = {0};
    id.part[1] = high_part;
    
    Editing_File *file_ptr = new_space;
    File_Node *free_sentinel = &working_set->free_sentinel;
    for (i16 i = 0; i < number_of_files; ++i, ++file_ptr){
        id.part[0] = i;
        file_ptr->id = id;
        dll_insert(free_sentinel, &file_ptr->node);
    }
}

internal Editing_File*
working_set_alloc(Working_Set *working_set){
    Editing_File *result = 0;
    
    if (working_set->file_count < working_set->file_max){
        File_Node *node = working_set->free_sentinel.next;
        Assert(node != &working_set->free_sentinel);
        result = (Editing_File*)node;
        
        dll_remove(node);
        Buffer_Slot_ID id = result->id;
        *result = null_editing_file;
        result->id = id;
        //result->unique_buffer_id = ++working_set->unique_file_counter;
        dll_insert(&working_set->used_sentinel, node);
        result->settings.display_width = working_set->default_display_width;
        result->settings.minimum_base_display_width = working_set->default_minimum_base_display_width;
        result->settings.wrap_indicator = WrapIndicator_Show_At_Wrap_Edge;
        init_file_markers_state(&result->markers);
        ++working_set->file_count;
    }
    
    return(result);
}

internal Editing_File*
working_set_alloc_always(Working_Set *working_set, General_Memory *general){
    Editing_File *result = 0;
    if (working_set->file_count == working_set->file_max && working_set->array_count < working_set->array_max){
        i16 new_count = (i16)clamp_top(working_set->file_max, max_i16);
        Editing_File *new_chunk = gen_array(general, Editing_File, new_count);
        working_set_extend_memory(working_set, new_chunk, new_count);
    }
    result = working_set_alloc(working_set);
    return(result);
}

inline void
working_set_free_file(General_Memory *general, Working_Set  *working_set, Editing_File *file){
    if (working_set->sync_check_iter == &file->node){
        working_set->sync_check_iter = working_set->sync_check_iter->next;
    }
    
    file->is_dummy = 1;
    clear_file_markers_state(general, &file->markers);
    dll_remove(&file->node);
    dll_insert(&working_set->free_sentinel, &file->node);
    --working_set->file_count;
}

inline Editing_File*
working_set_index(Working_Set *working_set, Buffer_Slot_ID id){
    Editing_File *result = 0;
    File_Array *array = 0;
    
    if (id.part[1] >= 0 && id.part[1] < working_set->array_count){
        array = working_set->file_arrays + id.part[1];
        if (id.part[0] >= 0 && id.part[0] < array->size){
            result = array->files + id.part[0];
        }
    }
    
    return(result);
}

inline Editing_File*
working_set_index(Working_Set *working_set, i32 id){
    Editing_File *result = working_set_index(working_set, to_file_id(id));
    return(result);
}

inline Editing_File*
working_set_get_active_file(Working_Set *working_set, Buffer_Slot_ID id){
    Editing_File *result = 0;
    result = working_set_index(working_set, id);
    if (result && result->is_dummy){
        result = 0;
    }
    return(result);
}

inline Editing_File*
working_set_get_active_file(Working_Set *working_set, i32 id){
    Editing_File *result;
    result = working_set_get_active_file(working_set, to_file_id(id));
    return(result);
}

internal void
working_set_init(Working_Set *working_set, Partition *partition, General_Memory *general){
    i16 init_count = 16;
    i16 array_init_count = 256;
    
    dll_init_sentinel(&working_set->free_sentinel);
    dll_init_sentinel(&working_set->used_sentinel);
    
    working_set->array_max = array_init_count;
    working_set->file_arrays = push_array(partition, File_Array, array_init_count);
    
    Editing_File *files = push_array(partition, Editing_File, init_count);
    working_set_extend_memory(working_set, files, init_count);
    
    // TODO(NAME): Unclear that this is still needed.  But double check that the buffer id 0 does not start getting used by the next real buffer when this is removed before actually removing it.  Buffer id cannot be allowed to be zero on real buffers.
#if 1
    // NOTE(allen): init null file
    {
        Editing_File *null_file = working_set_index(working_set, 0);
        dll_remove(&null_file->node);
        null_file->is_dummy = 1;
        ++working_set->file_count;
    }
#endif
    
    // NOTE(allen): init canon table
    {
        i32 item_size = sizeof(File_Name_Entry);
        i32 table_size = working_set->file_max;
        umem mem_size = table_required_mem_size(table_size, item_size);
        void *mem = general_memory_allocate(general, mem_size);
        memset(mem, 0, mem_size);
        table_init_memory(&working_set->canon_table, mem, table_size, item_size);
    }
    
    // NOTE(allen): init name table
    {
        u32 item_size = sizeof(File_Name_Entry);
        u32 table_size = working_set->file_max;
        umem mem_size = table_required_mem_size(table_size, item_size);
        void *mem = general_memory_allocate(general, mem_size);
        memset(mem, 0, mem_size);
        table_init_memory(&working_set->name_table, mem, table_size, item_size);
    }
}

inline void
working_set__grow_if_needed(Table *table, General_Memory *general, void *arg, Hash_Function *hash_func, Compare_Function *comp_func){
    if (table_at_capacity(table)){
        Table btable = {0};
        u32 new_max = table->max * 2;
        umem mem_size = table_required_mem_size(new_max, table->item_size);
        void *mem = general_memory_allocate(general, mem_size);
        table_init_memory(&btable, mem, new_max, table->item_size);
        table_clear(&btable);
        table_rehash(table, &btable, 0, hash_func, comp_func);
        general_memory_free(general, table->hash_array);
        *table = btable;
    }
}

internal Editing_File*
working_set_contains_basic(Working_Set *working_set, Table *table, String name){
    Editing_File *result = 0;
    
    File_Name_Entry *entry = (File_Name_Entry*)
        table_find_item(table, &name, 0, tbl_string_hash, tbl_string_compare);
    if (entry){
        result = working_set_index(working_set, entry->id);
    }
    
    return(result);
}

internal b32
working_set_add_basic(General_Memory *general, Working_Set *working_set, Table *table, Editing_File *file, String name){
    working_set__grow_if_needed(table, general, 0, tbl_string_hash, tbl_string_compare);
    
    File_Name_Entry entry;
    entry.name = name;
    entry.id = file->id;
    b32 result = (table_add(table, &entry, 0, tbl_string_hash, tbl_string_compare) == 0);
    return(result);
}

internal void
working_set_remove_basic(Working_Set *working_set, Table *table, String name){
    table_remove_match(table, &name, 0, tbl_string_hash, tbl_string_compare);
}

internal Editing_File*
working_set_canon_contains(Working_Set *working_set, String name){
    Editing_File *result =
        working_set_contains_basic(working_set, &working_set->canon_table, name);
    return(result);
}

internal b32
working_set_canon_add(General_Memory *general, Working_Set *working_set, Editing_File *file, String name){
    b32 result = working_set_add_basic(general,working_set, &working_set->canon_table, file, name);
    return(result);
}

internal void
working_set_canon_remove(Working_Set *working_set, String name){
    working_set_remove_basic(working_set, &working_set->canon_table, name);
}

internal Editing_File*
working_set_name_contains(Working_Set *working_set, String name){
    Editing_File *result =
        working_set_contains_basic(working_set, &working_set->name_table, name);
    return(result);
}

internal b32
working_set_name_add(General_Memory *general, Working_Set *working_set, Editing_File *file, String name){
    b32 result = working_set_add_basic(general, working_set, &working_set->name_table, file, name);
    return(result);
}

internal void
working_set_name_remove(Working_Set *working_set, String name){
    working_set_remove_basic(working_set, &working_set->name_table, name);
}


// TODO(allen): Pick better first options.
internal Editing_File*
working_set_lookup_file(Working_Set *working_set, String string){
    Editing_File *file = 0;
    
    {
        // TODO(allen): use the name table for this
        File_Node *node, *used_nodes;
        used_nodes = &working_set->used_sentinel;
        for (dll_items(node, used_nodes)){
            file = (Editing_File*)node;
            if (string.size == 0 || match_ss(string, file->name.live_name)){
                break;
            }
        }
        if (node == used_nodes) file = 0;
    }
    
    if (!file){
        File_Node *node, *used_nodes;
        used_nodes = &working_set->used_sentinel;
        for (dll_items(node, used_nodes)){
            file = (Editing_File*)node;
            if (string.size == 0 || has_substr_s(file->name.live_name, string)){
                break;
            }
        }
        if (node == used_nodes) file = 0;
    }
    
    return (file);
}

internal void
touch_file(Working_Set *working_set, Editing_File *file){
    if (file){
        Assert(!file->is_dummy);
        dll_remove(&file->node);
        dll_insert(&working_set->used_sentinel, &file->node);
    }
}


//
// Name Binding
//

internal void
editing_file_name_init(Editing_File_Name *name){
    name->live_name = make_fixed_width_string(name->live_name_);
    //name->source_path = make_fixed_width_string(name->source_path_);
    name->extension = make_fixed_width_string(name->extension_);
}

internal b32
get_canon_name(System_Functions *system, Editing_File_Canon_Name *canon_name, String filename){
    canon_name->name = make_fixed_width_string(canon_name->name_);
    
    canon_name->name.size = system->get_canonical((u8*)filename.str, filename.size, (u8*)canon_name->name.str, canon_name->name.memory_size);
    terminate_with_null(&canon_name->name);
    
    b32 result = (canon_name->name.size != 0);
    return(result);
}

internal void
buffer_get_new_name(Working_Set *working_set, Editing_File_Name *name, String filename){
    Assert(name->live_name.str != 0);
    
    //copy_checked_ss(&name->source_path, filename);
    copy_ss(&name->live_name, front_of_directory(filename));
    
    String ext = file_extension(filename);
    copy_ss(&name->extension, ext);
#if 0
    if (name->source_path.size == name->live_name.size){
        name->extension.size = 0;
    }
    else{
        String ext = file_extension(filename);
        copy_ss(&name->extension, ext);
    }
#endif
    
    {
        i32 original_len = name->live_name.size;
        i32 file_x = 0;
        b32 hit_conflict = 1;
        while (hit_conflict){
            hit_conflict = 0;
            
            File_Node *used_nodes = &working_set->used_sentinel, *node;
            for (dll_items(node, used_nodes)){
                Editing_File *file_ptr = (Editing_File*)node;
                if (file_is_ready(file_ptr)){
                    if (match_ss(name->live_name, file_ptr->name.live_name)){
                        ++file_x;
                        hit_conflict = 1;
                        break;
                    }
                }
            }
            
            if (hit_conflict){
                name->live_name.size = original_len;
                append_ss(&name->live_name, make_lit_string(" <"));
                append_int_to_str(&name->live_name, file_x);
                append_s_char(&name->live_name, '>');
            }
        }
    }
}

inline void
buffer_get_new_name(Working_Set *working_set, Editing_File_Name *name, char *filename){
    String f = make_string_slowly(filename);
    buffer_get_new_name(working_set, name, f);
}

internal void
buffer_bind_file(System_Functions *system, General_Memory *general, Working_Set *working_set, Editing_File *file, String canon_filename){
    Assert(file->name.live_name.size == 0 && file->name.extension.size == 0);
    //&& file->name.source_path.size == 0);
    Assert(file->canon.name.size == 0);
    
    file->canon.name = make_fixed_width_string(file->canon.name_);
    copy_ss(&file->canon.name, canon_filename);
    terminate_with_null(&file->canon.name);
    system->add_listener(file->canon.name.str);
    b32 result = working_set_canon_add(general, working_set, file, file->canon.name);
    Assert(result); AllowLocal(result);
}

internal void
buffer_unbind_file(System_Functions *system, Working_Set *working_set, Editing_File *file){
    Assert(file->name.live_name.size == 0 && file->name.extension.size == 0);
    // && file->name.source_path.size == 0
    Assert(file->canon.name.size != 0);
    
    system->remove_listener(file->canon.name_);
    working_set_canon_remove(working_set, file->canon.name);
    file->canon.name.size = 0;
}

internal void
buffer_bind_name(General_Memory *general, Working_Set *working_set, Editing_File *file, String filename){
    Assert(file->name.live_name.size == 0 &&
           file->name.extension.size == 0);
    // && file->name.source_path.size == 0
    
    Editing_File_Name new_name;
    editing_file_name_init(&new_name);
    buffer_get_new_name(working_set, &new_name, filename);
    
    editing_file_name_init(&file->name);
    copy_ss(&file->name.live_name, new_name.live_name);
    //copy_ss(&file->name.source_path, new_name.source_path);
    copy_ss(&file->name.extension, new_name.extension);
    
    b32 result = working_set_name_add(general, working_set, file, file->name.live_name);
    Assert(result); AllowLocal(result);
}

internal void
buffer_unbind_name(Working_Set *working_set, Editing_File *file){
    Assert(file->name.live_name.size != 0);
    working_set_name_remove(working_set, file->name.live_name);
    file->name.live_name.size = 0;
    //file->name.source_path.size = 0;
    file->name.extension.size = 0;
}


// BOTTOM

