/*
4coder_default_hooks.cpp - Sets up the hooks for the default framework.
*/

// TOP

CUSTOM_COMMAND_SIG(default_startup)
CUSTOM_DOC("Default command for responding to a startup event")
{
    ProfileScope(app, "default startup");
    User_Input input = get_current_input(app);
    if (match_core_code(&input, CoreCode_Startup)){
        String_Const_u8_Array file_names = input.event.core.file_names;
        default_4coder_initialize(app, file_names);
        default_4coder_side_by_side_panels(app, file_names);
        if (global_config.automatically_load_project){
            load_project(app);
        }
    }
}

CUSTOM_COMMAND_SIG(default_try_exit)
CUSTOM_DOC("Default command for responding to a try-exit event")
{
    User_Input input = get_current_input(app);
    if (match_core_code(&input, CoreCode_TryExit)){
        b32 do_exit = true;
        if (!allow_immediate_close_without_checking_for_changes){
            b32 has_unsaved_changes = false;
            for (Buffer_ID buffer = get_buffer_next(app, 0, Access_Always);
                 buffer != 0;
                 buffer = get_buffer_next(app, buffer, Access_Always)){
                Dirty_State dirty = buffer_get_dirty_state(app, buffer);
                if (HasFlag(dirty, DirtyState_UnsavedChanges)){
                    has_unsaved_changes = true;
                    break;
                }
            }
            if (has_unsaved_changes){
                View_ID view = get_active_view(app, Access_Always);
                do_exit = do_gui_sure_to_close_4coder(app, view);
            }
        }
        if (do_exit){
            // NOTE(allen): By leaving try exit unhandled we indicate
            // that the core should take responsibility for handling this,
            // and it will handle it by exiting 4coder.  If we leave this
            // event marked as handled on the other hand (for instance by 
            // running a confirmation GUI that cancels the exit) then 4coder
            // will not exit.
            leave_current_input_unhandled(app);
        }
    }
}


CUSTOM_COMMAND_SIG(default_view_input_handler)
CUSTOM_DOC("Input consumption loop for default view behavior")
{
    Thread_Context *tctx = get_thread_context(app);
    Scratch_Block scratch(tctx);
    
    {
        
        View_ID view = get_active_view(app, Access_Always);
        String_Const_u8 name = push_u8_stringf(scratch, "view %d", view);
        ProfileThreadName(tctx, name);
        
        View_Context ctx = view_current_context(app, view);
        ctx.mapping = &framework_mapping;
        ctx.map_id = mapid_global;
        view_alter_context(app, view, &ctx);
    }
    
    for (;;){
        // NOTE(allen): Get the binding from the buffer's current map
        User_Input input = get_next_input(app, EventPropertyGroup_Any, 0);
        ProfileScopeNamed(app, "before view input", view_input_profile);
        if (input.abort){
            break;
        }
        
        Event_Property event_properties = get_event_properties(&input.event);
        
        if (suppressing_mouse && (event_properties & EventPropertyGroup_AnyMouseEvent) != 0){
            continue;
        }
        
        View_ID view = get_active_view(app, Access_Always);
        
        Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
        Managed_Scope buffer_scope = buffer_get_managed_scope(app, buffer);
        Command_Map_ID *map_id_ptr =
            scope_attachment(app, buffer_scope, buffer_map_id, Command_Map_ID);
        if (*map_id_ptr == 0){
            *map_id_ptr = mapid_file;
        }
        Command_Map_ID map_id = *map_id_ptr;
        
        Command_Binding binding =
            map_get_binding_recursive(&framework_mapping, map_id, &input.event);
        
        Managed_Scope scope = view_get_managed_scope(app, view);
        Custom_Command_Function** next_call = 0;
        
	    call_again:
        next_call = scope_attachment(app, scope, view_call_next,
                                     Custom_Command_Function*);
        *next_call = 0;
        
        if (binding.custom == 0){
            // NOTE(allen): we don't have anything to do with this input,
            // leave it marked unhandled so that if there's a follow up
            // event it is not blocked.
            leave_current_input_unhandled(app);
        }
        else{
            // NOTE(allen): before the command is called do some book keeping
            Rewrite_Type *next_rewrite =
                scope_attachment(app, scope, view_next_rewrite_loc, Rewrite_Type);
            *next_rewrite = Rewrite_None;
            if (fcoder_mode == FCoderMode_NotepadLike){
                for (View_ID view_it = get_view_next(app, 0, Access_Always);
                     view_it != 0;
                     view_it = get_view_next(app, view_it, Access_Always)){
                    Managed_Scope scope_it = view_get_managed_scope(app, view_it);
                    b32 *snap_mark_to_cursor =
                        scope_attachment(app, scope_it, view_snap_mark_to_cursor,
                                         b32);
                    *snap_mark_to_cursor = true;
                }
            }
            
            view_input_profile.close_now();
            
            // NOTE(allen): call the command
            binding.custom(app);
            
            // NOTE(allen): after the command is called do some book keeping
            ProfileScope(app, "after view input");
            
            next_rewrite = scope_attachment(app, scope, view_next_rewrite_loc,
                                            Rewrite_Type);
            if (next_rewrite != 0){
                Rewrite_Type *rewrite =
                    scope_attachment(app, scope, view_rewrite_loc, Rewrite_Type);
                *rewrite = *next_rewrite;
                if (fcoder_mode == FCoderMode_NotepadLike){
                    for (View_ID view_it = get_view_next(app, 0, Access_Always);
                         view_it != 0;
                         view_it = get_view_next(app, view_it, Access_Always)){
                        Managed_Scope scope_it = view_get_managed_scope(app, view_it);
                        b32 *snap_mark_to_cursor =
                            scope_attachment(app, scope_it, view_snap_mark_to_cursor,
                                             b32);
                        if (*snap_mark_to_cursor){
                            i64 pos = view_get_cursor_pos(app, view_it);
                            view_set_mark(app, view_it, seek_pos(pos));
                        }
                    }
                }
            }
            
            next_call = scope_attachment(app, scope, view_call_next,
                                         Custom_Command_Function*);
            if (next_call != 0 && *next_call != 0){
                binding.custom = *next_call;
                goto call_again;
            }
        }
    }
}

#if 0
static argb_color default_colors[Stag_COUNT] = {};
MODIFY_COLOR_TABLE_SIG(default_modify_color_table){
    if (default_colors[Stag_NOOP] == 0){
        default_colors[Stag_NOOP]                  = 0xFFFF00FF;
        
        default_colors[Stag_Back]                  = 0xFF0C0C0C;
        default_colors[Stag_Margin]                = 0xFF181818;
        default_colors[Stag_Margin_Hover]          = 0xFF252525;
        default_colors[Stag_Margin_Active]         = 0xFF323232;
        default_colors[Stag_List_Item]             = default_colors[Stag_Margin];
        default_colors[Stag_List_Item_Hover]       = default_colors[Stag_Margin_Hover];
        default_colors[Stag_List_Item_Active]      = default_colors[Stag_Margin_Active];
        default_colors[Stag_Cursor]                = 0xFF00EE00;
        default_colors[Stag_Highlight]             = 0xFFDDEE00;
        default_colors[Stag_Mark]                  = 0xFF494949;
        default_colors[Stag_Default]               = 0xFF90B080;
        default_colors[Stag_At_Cursor]             = default_colors[Stag_Back];
        default_colors[Stag_Highlight_Cursor_Line] = 0xFF1E1E1E;
        default_colors[Stag_At_Highlight]          = 0xFFFF44DD;
        default_colors[Stag_Comment]               = 0xFF2090F0;
        default_colors[Stag_Keyword]               = 0xFFD08F20;
        default_colors[Stag_Str_Constant]          = 0xFF50FF30;
        default_colors[Stag_Char_Constant]         = default_colors[Stag_Str_Constant];
        default_colors[Stag_Int_Constant]          = default_colors[Stag_Str_Constant];
        default_colors[Stag_Float_Constant]        = default_colors[Stag_Str_Constant];
        default_colors[Stag_Bool_Constant]         = default_colors[Stag_Str_Constant];
        default_colors[Stag_Include]               = default_colors[Stag_Str_Constant];
        default_colors[Stag_Preproc]               = default_colors[Stag_Default];
        default_colors[Stag_Special_Character]     = 0xFFFF0000;
        default_colors[Stag_Ghost_Character]       = 0xFF4E5E46;
        
        default_colors[Stag_Paste] = 0xFFDDEE00;
        default_colors[Stag_Undo]  = 0xFF00DDEE;
        
        default_colors[Stag_Highlight_Junk]  = 0xFF3A0000;
        default_colors[Stag_Highlight_White] = 0xFF003A3A;
        
        default_colors[Stag_Bar]        = 0xFF888888;
        default_colors[Stag_Base]       = 0xFF000000;
        default_colors[Stag_Pop1]       = 0xFF3C57DC;
        default_colors[Stag_Pop2]       = 0xFFFF0000;
        
        default_colors[Stag_Back_Cycle_1] = 0x10A00000;
        default_colors[Stag_Back_Cycle_2] = 0x0C00A000;
        default_colors[Stag_Back_Cycle_3] = 0x0C0000A0;
        default_colors[Stag_Back_Cycle_4] = 0x0CA0A000;
        default_colors[Stag_Text_Cycle_1] = 0xFFA00000;
        default_colors[Stag_Text_Cycle_2] = 0xFF00A000;
        default_colors[Stag_Text_Cycle_3] = 0xFF0030B0;
        default_colors[Stag_Text_Cycle_4] = 0xFFA0A000;
        
        default_colors[Stag_Line_Numbers_Back] = 0xFF101010;
        default_colors[Stag_Line_Numbers_Text] = 0xFF404040;
    }
    
    Color_Table color_table = {};
    color_table.vals = default_colors;
    color_table.count = ArrayCount(default_colors);
    return(color_table);
}
#endif

function Rect_f32
default_buffer_region(Application_Links *app, View_ID view_id, Rect_f32 region){
    Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    f32 line_height = metrics.line_height;
    f32 digit_advance = metrics.decimal_digit_advance;
    
    // NOTE(allen): margins
    region = rect_inner(region, 3.f);
    
    // NOTE(allen): file bar
    b64 showing_file_bar = false;
    if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) &&
        showing_file_bar){
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        region = pair.max;
    }
    
    // NOTE(allen): query bars
    {
        Query_Bar *space[32];
        Query_Bar_Ptr_Array query_bars = {};
        query_bars.ptrs = space;
        if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars)){
            Rect_f32_Pair pair = layout_query_bar_on_top(region, line_height, query_bars.count);
            region = pair.max;
        }
    }
    
    // NOTE(allen): FPS hud
    if (show_fps_hud){
        Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
        region = pair.min;
    }
    
    // NOTE(allen): line numbers
    if (global_config.show_line_number_margins){
        Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
        region = pair.max;
    }
    
    return(region);
}

function void
default_render_buffer(Application_Links *app, View_ID view_id, b32 is_active_view,
                      Buffer_ID buffer, Text_Layout_ID text_layout_id,
                      Rect_f32 rect){
    ProfileScope(app, "render buffer");
    Rect_f32 prev_clip = draw_set_clip(app, rect);
    
    // NOTE(allen): Token colorizing
    Token_Array token_array = get_token_array_from_buffer(app, buffer);
    if (token_array.tokens != 0){
        draw_buffer_add_cpp_token_colors(app, text_layout_id, &token_array);
        
        // NOTE(allen): Scan for TODOs and NOTEs
        if (global_config.use_comment_keyword){
            Comment_Highlight_Pair pairs[] = {
                {string_u8_litexpr("NOTE"), Stag_Text_Cycle_2},
                {string_u8_litexpr("TODO"), Stag_Text_Cycle_1},
            };
            draw_comment_highlights(app, buffer, text_layout_id,
                                    &token_array, pairs, ArrayCount(pairs));
        }
    }
    
    i64 cursor_pos = view_get_cursor_pos(app, view_id);
    
    // NOTE(allen): Scope highlight
    if (global_config.use_scope_highlight){
        FColor colors[] = {
            fcolor_id(Stag_Back_Cycle_1), fcolor_id(Stag_Back_Cycle_2),
            fcolor_id(Stag_Back_Cycle_3), fcolor_id(Stag_Back_Cycle_4),
        };
        draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors, ArrayCount(colors));
    }
    
    if (global_config.use_error_highlight || global_config.use_jump_highlight){
        // NOTE(allen): Error highlight
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        if (global_config.use_error_highlight){
            draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                                 fcolor_id(Stag_Highlight_Junk));
        }
        
        // NOTE(allen): Search highlight
        if (global_config.use_jump_highlight){
            Buffer_ID jump_buffer = get_locked_jump_buffer(app);
            if (jump_buffer != compilation_buffer){
                draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                                     fcolor_id(Stag_Highlight_White));
            }
        }
    }
    
    // NOTE(allen): Color parens
    if (global_config.use_paren_helper){
        FColor colors[] = {
            fcolor_id(Stag_Text_Cycle_1), fcolor_id(Stag_Text_Cycle_2),
            fcolor_id(Stag_Text_Cycle_3), fcolor_id(Stag_Text_Cycle_4),
        };
        draw_paren_highlight(app, buffer, text_layout_id, cursor_pos,
                             colors, ArrayCount(colors));
    }
    
    // NOTE(allen): Line highlight
    if (global_config.highlight_line_at_cursor && is_active_view){
        i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
        draw_line_highlight(app, text_layout_id, line_number,
                            fcolor_id(Stag_Highlight_Cursor_Line));
    }
    
    // NOTE(allen): Cursor shape
    f32 cursor_roundness = 4.f;
    f32 mark_thickness = 2.f;
    
    // NOTE(allen): Cursor
    switch (fcoder_mode){
        case FCoderMode_Original:
        {
            draw_original_4coder_style_cursor_mark_highlight(app, view_id, is_active_view,
                                                             buffer, text_layout_id,
                                                             cursor_roundness, mark_thickness);
        }break;
        case FCoderMode_NotepadLike:
        {
            draw_notepad_style_cursor_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);
        }break;
    }
    
    // NOTE(allen): put the actual text on the actual screen
    draw_text_layout(app, text_layout_id);
    
    draw_set_clip(app, prev_clip);
}

function void
default_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    ProfileScope(app, "default render caller");
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    Rect_f32 region = draw_background_and_margin(app, view_id, is_active_view);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    f32 line_height = face_metrics.line_height;
    f32 digit_advance = face_metrics.decimal_digit_advance;
    
    // NOTE(allen): file bar
    b64 showing_file_bar = false;
    if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar) && showing_file_bar){
        Rect_f32_Pair pair = layout_file_bar_on_top(region, line_height);
        draw_file_bar(app, view_id, buffer, face_id, pair.min);
        region = pair.max;
    }
    
    Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
    
    Buffer_Point_Delta_Result delta = delta_apply(app, view_id,
                                                  frame_info.animation_dt, scroll);
    if (!block_match_struct(&scroll.position, &delta.point)){
        block_copy_struct(&scroll.position, &delta.point);
        view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
    }
    if (delta.still_animating){
        animate_in_n_milliseconds(app, 0);
    }
    
    // NOTE(allen): query bars
    {
        Query_Bar *space[32];
        Query_Bar_Ptr_Array query_bars = {};
        query_bars.ptrs = space;
        if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars)){
            for (i32 i = 0; i < query_bars.count; i += 1){
                Rect_f32_Pair pair = layout_query_bar_on_top(region, line_height, 1);
                draw_query_bar(app, query_bars.ptrs[i], face_id, pair.min);
                region = pair.max;
            }
        }
    }
    
    // NOTE(allen): FPS hud
    if (show_fps_hud){
        Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
        draw_fps_hud(app, frame_info, face_id, pair.max);
        region = pair.min;
        animate_in_n_milliseconds(app, 1000);
    }
    
    // NOTE(allen): layout line numbers
    Rect_f32 line_number_rect = {};
    if (global_config.show_line_number_margins){
        Rect_f32_Pair pair = layout_line_number_margin(app, buffer, region, digit_advance);
        line_number_rect = pair.min;
        region = pair.max;
    }
    
    // NOTE(allen): begin buffer render
    Buffer_Point buffer_point = scroll.position;
    Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);
    
    // NOTE(allen): draw line numbers
    if (global_config.show_line_number_margins){
        draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
    }
    
    // NOTE(allen): draw the buffer
    default_render_buffer(app, view_id, is_active_view,
                          buffer, text_layout_id,
                          region);
    
    text_layout_free(app, text_layout_id);
    draw_set_clip(app, prev_clip);
}

HOOK_SIG(default_view_adjust){
    // NOTE(allen): Called whenever the view layout/sizes have been modified,
    // including by full window resize.
    return(0);
}

BUFFER_NAME_RESOLVER_SIG(default_buffer_name_resolution){
    ProfileScope(app, "default buffer name resolution");
    if (conflict_count > 1){
        // List of unresolved conflicts
        Scratch_Block scratch(app);
        
        i32 *unresolved = push_array(scratch, i32, conflict_count);
        i32 unresolved_count = conflict_count;
        for (i32 i = 0; i < conflict_count; ++i){
            unresolved[i] = i;
        }
        
        // Resolution Loop
        i32 x = 0;
        for (;;){
            // Resolution Pass
            ++x;
            for (i32 i = 0; i < unresolved_count; ++i){
                i32 conflict_index = unresolved[i];
                Buffer_Name_Conflict_Entry *conflict = &conflicts[conflict_index];
                
                umem size = conflict->base_name.size;
                size = clamp_top(size, conflict->unique_name_capacity);
                conflict->unique_name_len_in_out = size;
                block_copy(conflict->unique_name_in_out, conflict->base_name.str, size);
                
                if (conflict->file_name.str != 0){
                    Temp_Memory_Block temp(scratch);
                    String_Const_u8 uniqueifier = {};
                    
                    String_Const_u8 file_name = string_remove_last_folder(conflict->file_name);
                    if (file_name.size > 0){
                        file_name = string_chop(file_name, 1);
                        u8 *end = file_name.str + file_name.size;
                        b32 past_the_end = false;
                        for (i32 j = 0; j < x; ++j){
                            file_name = string_remove_last_folder(file_name);
                            if (j + 1 < x){
                                file_name = string_chop(file_name, 1);
                            }
                            if (file_name.size == 0){
                                if (j + 1 < x){
                                    past_the_end = true;
                                }
                                break;
                            }
                        }
                        u8 *start = file_name.str + file_name.size;
                        
                        uniqueifier = SCu8(start, end);
                        if (past_the_end){
                            uniqueifier = push_u8_stringf(scratch, "%.*s~%d",
                                                          string_expand(uniqueifier), i);
                        }
                    }
                    else{
                        uniqueifier = push_u8_stringf(scratch, "%d", i);
                    }
                    
                    String_u8 builder = Su8(conflict->unique_name_in_out,
                                            conflict->unique_name_len_in_out,
                                            conflict->unique_name_capacity);
                    string_append(&builder, string_u8_litexpr(" <"));
                    string_append(&builder, uniqueifier);
                    string_append(&builder, string_u8_litexpr(">"));
                    conflict->unique_name_len_in_out = builder.size;
                }
            }
            
            // Conflict Check Pass
            b32 has_conflicts = false;
            for (i32 i = 0; i < unresolved_count; ++i){
                i32 conflict_index = unresolved[i];
                Buffer_Name_Conflict_Entry *conflict = &conflicts[conflict_index];
                String_Const_u8 conflict_name = SCu8(conflict->unique_name_in_out,
                                                     conflict->unique_name_len_in_out);
                
                b32 hit_conflict = false;
                if (conflict->file_name.str != 0){
                    for (i32 j = 0; j < unresolved_count; ++j){
                        if (i == j) continue;
                        
                        i32 conflict_j_index = unresolved[j];
                        Buffer_Name_Conflict_Entry *conflict_j = &conflicts[conflict_j_index];
                        
                        String_Const_u8 conflict_name_j = SCu8(conflict_j->unique_name_in_out,
                                                               conflict_j->unique_name_len_in_out);
                        
                        if (string_match(conflict_name, conflict_name_j)){
                            hit_conflict = true;
                            break;
                        }
                    }
                }
                
                if (hit_conflict){
                    has_conflicts = true;
                }
                else{
                    --unresolved_count;
                    unresolved[i] = unresolved[unresolved_count];
                    --i;
                }
            }
            
            if (!has_conflicts){
                break;
            }
        }
    }
}

function void
do_full_lex_async__inner(Async_Context *actx, Buffer_ID buffer_id){
    Application_Links *app = actx->app;
    ProfileScope(app, "async lex");
    Thread_Context *tctx = get_thread_context(app);
    Scratch_Block scratch(tctx);
    
    String_Const_u8 contents = {};
    {
        ProfileBlock(app, "async lex contents (before mutex)");
        system_acquire_global_frame_mutex(tctx);
        ProfileBlock(app, "async lex contents (after mutex)");
        contents = push_whole_buffer(app, scratch, buffer_id);
        system_release_global_frame_mutex(tctx);
    }
    
    Lex_State_Cpp state = {};
    lex_full_input_cpp_init(&state, contents);
    
    Token_List list = {};
    b32 canceled = false;
    for (;;){
        ProfileBlock(app, "async lex block");
        if (lex_full_input_cpp_breaks(scratch, &list, &state, 10000)){
            break;
        }
        if (async_check_canceled(actx)){
            canceled = true;
            break;
        }
    }
    
    if (!canceled){
        ProfileBlock(app, "async lex save results (before mutex)");
        system_acquire_global_frame_mutex(tctx);
        ProfileBlock(app, "async lex save results (after mutex)");
        Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
        if (scope != 0){
            Base_Allocator *allocator = managed_scope_allocator(app, scope);
            Token_Array *tokens_ptr = scope_attachment(app, scope, attachment_tokens,
                                                       Token_Array);
            base_free(allocator, tokens_ptr->tokens);
            
            Token_Array tokens = {};
            tokens.tokens = base_array(allocator, Token, list.total_count);
            tokens.count = list.total_count;
            tokens.max = list.total_count;
            token_fill_memory_from_list(tokens.tokens, &list);
            block_copy_struct(tokens_ptr, &tokens);
        }
        system_release_global_frame_mutex(tctx);
    }
}

function void
do_full_lex_async(Async_Context *actx, Data data){
    if (data.size == sizeof(Buffer_ID)){
        Buffer_ID buffer = *(Buffer_ID*)data.data;
        do_full_lex_async__inner(actx, buffer);
    }
}

BUFFER_HOOK_SIG(default_begin_buffer){
    ProfileScope(app, "begin buffer");
    
    b32 treat_as_code = false;
    
    String_Const_u8_Array extensions = global_config.code_exts;
    
    Scratch_Block scratch(app);
    
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
    
    if (file_name.size > 0){
        String_Const_u8 ext = string_file_extension(file_name);
        for (i32 i = 0; i < extensions.count; ++i){
            if (string_match(ext, extensions.strings[i])){
                
                if (string_match(ext, string_u8_litexpr("cpp")) || 
                    string_match(ext, string_u8_litexpr("h")) ||
                    string_match(ext, string_u8_litexpr("c")) ||
                    string_match(ext, string_u8_litexpr("hpp")) ||
                    string_match(ext, string_u8_litexpr("cc"))){
                    treat_as_code = true;
                }
                
#if 0                
                treat_as_code = true;
                
                if (string_match(ext, string_u8_litexpr("cs"))){
                    if (parse_context_language_cs == 0){
                        init_language_cs(app);
                    }
                    parse_context_id = parse_context_language_cs;
                }
                
                if (string_match(ext, string_u8_litexpr("java"))){
                    if (parse_context_language_java == 0){
                        init_language_java(app);
                    }
                    parse_context_id = parse_context_language_java;
                }
                
                if (string_match(ext, string_u8_litexpr("rs"))){
                    if (parse_context_language_rust == 0){
                        init_language_rust(app);
                    }
                    parse_context_id = parse_context_language_rust;
                }
                
                if (string_match(ext, string_u8_litexpr("cpp")) || 
                    string_match(ext, string_u8_litexpr("h")) ||
                    string_match(ext, string_u8_litexpr("c")) ||
                    string_match(ext, string_u8_litexpr("hpp")) ||
                    string_match(ext, string_u8_litexpr("cc"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real GLSL highlighting
                if (string_match(ext, string_u8_litexpr("glsl"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real Objective-C highlighting
                if (string_match(ext, string_u8_litexpr("m"))){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
#endif
                
                break;
            }
        }
    }
    
    local_persist b32 first_call = true;
    if (first_call){
        first_call = false;
        buffer_map_id = managed_id_declare(app, SCu8("DEFAULT.buffer_map_id"));
        buffer_eol_setting = managed_id_declare(app, SCu8("DEFAULT.buffer_eol_setting"));
        buffer_lex_task = managed_id_declare(app, SCu8("DEFAULT.buffer_lex_task"));
    }
    
    Command_Map_ID map_id = (treat_as_code)?(default_code_map):(mapid_file);
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Command_Map_ID *map_id_ptr = scope_attachment(app, scope, buffer_map_id,
                                                  Command_Map_ID);
    *map_id_ptr = map_id;
    
    Line_Ending_Kind setting = guess_line_ending_kind_from_buffer_contents(app, buffer_id);
    Line_Ending_Kind *eol_setting = scope_attachment(app, scope, buffer_eol_setting,
                                                     Line_Ending_Kind);
    *eol_setting = setting;
    
    // NOTE(allen): Decide buffer settings
    b32 wrap_lines = true;
    b32 use_virtual_whitespace = false;
    b32 use_lexer = false;
    if (treat_as_code){
        wrap_lines = global_config.enable_code_wrapping;
        use_virtual_whitespace = global_config.enable_virtual_whitespace;
        use_lexer = true;
    }
    
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
    if (string_match(buffer_name, string_u8_litexpr("*compilation*"))){
        wrap_lines = false;
    }
    
    if (use_lexer){
        ProfileBlock(app, "begin buffer kick off lexer");
        Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
        *lex_task_ptr = async_task_no_dep(&global_async_system, do_full_lex_async, make_data_struct(&buffer_id));
    }
    
    // no meaning for return
    return(0);
}

BUFFER_HOOK_SIG(default_new_file){
    // buffer_id
    // no meaning for return
    return(0);
}

BUFFER_HOOK_SIG(default_file_save){
    // buffer_id
    ProfileScope(app, "default file save");
    b32 is_virtual = false;
    if (global_config.automatically_indent_text_on_save && is_virtual){ 
        auto_indent_buffer(app, buffer_id, buffer_range(app, buffer_id));
    }
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Line_Ending_Kind *eol = scope_attachment(app, scope, buffer_eol_setting,
                                             Line_Ending_Kind);
    switch (*eol){
        case LineEndingKind_LF:
        {
            rewrite_lines_to_lf(app, buffer_id);
        }break;
        case LineEndingKind_CRLF:
        {
            rewrite_lines_to_crlf(app, buffer_id);
        }break;
    }
    
    // no meaning for return
    return(0);
}

BUFFER_EDIT_RANGE_SIG(default_buffer_edit_range){
    // buffer_id, new_range, text
    ProfileScope(app, "default edit range");
    
    Interval_i64 old_range = Ii64(new_range.first, new_range.first + text.size);
    i64 insert_size = range_size(new_range);
    i64 text_shift = replace_range_shift(old_range, insert_size);
    
    Scratch_Block scratch(app);
    
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
    if (async_task_is_running_or_pending(&global_async_system, *lex_task_ptr)){
        async_task_cancel(&global_async_system, *lex_task_ptr);
        *lex_task_ptr = async_task_no_dep(&global_async_system, do_full_lex_async, make_data_struct(&buffer_id));
    }
    else{
        Token_Array *ptr = scope_attachment(app, scope, attachment_tokens, Token_Array);
        if (ptr != 0 && ptr->tokens != 0){
            i64 token_index_first = token_relex_first(ptr, old_range.first, 1);
            i64 token_index_resync_guess =
                token_relex_resync(ptr, old_range.one_past_last, 16);
            
            Token *token_first = ptr->tokens + token_index_first;
            Token *token_resync = ptr->tokens + token_index_resync_guess;
            
            Range_i64 relex_range =
                Ii64(token_first->pos,
                     token_resync->pos + token_resync->size + text_shift);
            String_Const_u8 partial_text = push_buffer_range(app, scratch, buffer_id,
                                                             relex_range);
            
            Token_List relex_list = lex_full_input_cpp(scratch, partial_text);
            if (relex_range.one_past_last < buffer_get_size(app, buffer_id)){
                token_drop_eof(&relex_list);
            }
            
            Base_Allocator *allocator = managed_scope_allocator(app, scope);
            
            Token_Relex relex = token_relex(relex_list, relex_range.first - text_shift,
                                            ptr->tokens, token_index_first, token_index_resync_guess);
            
            if (relex.successful_resync){
                i64 token_index_resync = relex.first_resync_index;
                
                Interval_i64 head = Ii64(0, token_index_first);
                Interval_i64 replaced = Ii64(token_index_first, token_index_resync);
                Interval_i64 tail = Ii64(token_index_resync, ptr->count);
                i64 resynced_count = (token_index_resync_guess + 1) - token_index_resync;
                i64 relexed_count = relex_list.total_count - resynced_count;
                i64 tail_shift = relexed_count - (token_index_resync - token_index_first);
                
                i64 new_tokens_count = ptr->count + tail_shift;
                Token *new_tokens = base_array(allocator, Token, new_tokens_count);
                
                Token *old_tokens = ptr->tokens;
                block_copy_array_shift(new_tokens, old_tokens, head, 0);
                token_fill_memory_from_list(new_tokens + replaced.first, &relex_list, relexed_count);
                for (i64 i = 0, index = replaced.first; i < relexed_count; i += 1, index += 1){
                    new_tokens[index].pos += relex_range.first;
                }
                for (i64 i = tail.first; i < tail.one_past_last; i += 1){
                    old_tokens[i].pos += text_shift;
                }
                block_copy_array_shift(new_tokens, ptr->tokens, tail, tail_shift);
                
                base_free(allocator, ptr->tokens);
                
                ptr->tokens = new_tokens;
                ptr->count = new_tokens_count;
                ptr->max = new_tokens_count;
            }
            else{
                base_free(allocator, ptr->tokens);
                block_zero_struct(ptr);
                *lex_task_ptr = async_task_no_dep(&global_async_system, do_full_lex_async,
                                                  make_data_struct(&buffer_id));
            }
        }
    }
    
    // no meaning for return
    return(0);
}

BUFFER_HOOK_SIG(default_end_buffer){
    Managed_Scope scope = buffer_get_managed_scope(app, buffer_id);
    Async_Task *lex_task_ptr = scope_attachment(app, scope, buffer_lex_task, Async_Task);
    if (lex_task_ptr != 0){
        async_task_cancel(&global_async_system, *lex_task_ptr);
    }
    // no meaning for return
    return(0);
}

internal void
set_all_default_hooks(Application_Links *app){
    set_custom_hook(app, HookID_BufferViewerUpdate, default_view_adjust);
    
    set_custom_hook(app, HookID_ViewEventHandler, default_view_input_handler);
    set_custom_hook(app, HookID_RenderCaller, default_render_caller);
#if 0
    set_custom_hook(app, HookID_DeltaRule, original_delta);
    set_custom_hook_memory_size(app, HookID_DeltaRule,
                                delta_ctx_size(original_delta_memory_size));
#else
    set_custom_hook(app, HookID_DeltaRule, fixed_time_cubic_delta);
    set_custom_hook_memory_size(app, HookID_DeltaRule,
                                delta_ctx_size(fixed_time_cubic_delta_memory_size));
#endif
    set_custom_hook(app, HookID_BufferNameResolver, default_buffer_name_resolution);
    
    set_custom_hook(app, HookID_BeginBuffer, default_begin_buffer);
    set_custom_hook(app, HookID_EndBuffer, end_buffer_close_jump_list);
    set_custom_hook(app, HookID_NewFile, default_new_file);
    set_custom_hook(app, HookID_SaveFile, default_file_save);
    set_custom_hook(app, HookID_BufferEditRange, default_buffer_edit_range);
    set_custom_hook(app, HookID_BufferRegion, default_buffer_region);
}

// BOTTOM

