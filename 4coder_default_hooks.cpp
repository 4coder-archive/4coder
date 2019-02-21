/*
4coder_default_hooks.cpp - Sets up the hooks for the default framework.
*/

// TOP

#include "languages/4coder_language_cpp.h"
#include "languages/4coder_language_rust.h"
#include "languages/4coder_language_cs.h"
#include "languages/4coder_language_java.h"

CUSTOM_COMMAND_SIG(set_bindings_choose);
CUSTOM_COMMAND_SIG(set_bindings_default);
CUSTOM_COMMAND_SIG(set_bindings_mac_default);

static Named_Mapping named_maps_values[] = {
    {make_lit_string("mac-default")    , set_bindings_mac_default    },
    {make_lit_string("choose")         , set_bindings_choose         },
    {make_lit_string("default")        , set_bindings_default        },
};

START_HOOK_SIG(default_start){
    named_maps = named_maps_values;
    named_map_count = ArrayCount(named_maps_values);
    
    default_4coder_initialize(app);
    default_4coder_side_by_side_panels(app, files, file_count);
    
    if (global_config.automatically_load_project){
        load_project(app);
    }
    
    // no meaning for return
    return(0);
}

// NOTE(allen|a4.0.9): All command calls can now go through this hook
// If this hook is not implemented a default behavior of calling the
// command is used.  It is important to note that paste_next does not
// work without this hook.
// NOTE(allen|a4.0.10): As of this version the word_complete command
// also relies on this particular command caller hook.
COMMAND_CALLER_HOOK(default_command_caller){
    View_Summary view = get_active_view(app, AccessAll);
    Managed_Scope scope = view_get_managed_scope(app, view.view_id);
    managed_variable_set(app, scope, view_next_rewrite_loc, 0);
    if (fcoder_mode == FCoderMode_NotepadLike){
        for (View_Summary view_it = get_view_first(app, AccessAll);
             view_it.exists;
             get_view_next(app, &view_it, AccessAll)){
            Managed_Scope scope_it = view_get_managed_scope(app, view_it.view_id);
            managed_variable_set(app, scope_it, view_snap_mark_to_cursor, true);
        }
    }
    
    ////
    exec_command(app, cmd);
    ////
    
    uint64_t next_rewrite = 0;
    managed_variable_get(app, scope, view_next_rewrite_loc, &next_rewrite);
    managed_variable_set(app, scope, view_rewrite_loc, next_rewrite);
    if (fcoder_mode == FCoderMode_NotepadLike){
        for (View_Summary view_it = get_view_first(app, AccessAll);
             view_it.exists;
             get_view_next(app, &view_it, AccessAll)){
            Managed_Scope scope_it = view_get_managed_scope(app, view_it.view_id);
            uint64_t val = 0;
            if (managed_variable_get(app, scope_it, view_snap_mark_to_cursor, &val)){
                if (val != 0){
                    view_set_mark(app, &view_it, seek_pos(view_it.cursor.pos));
                }
            }
        }
    }
    
    return(0);
}

struct Highlight_Record{
    int32_t first;
    int32_t one_past_last;
    int_color color;
};

static void
sort_highlight_record(Highlight_Record *records, int32_t first, int32_t one_past_last){
    if (first + 1 < one_past_last){
        int32_t pivot_index = one_past_last - 1;
        int_color pivot_color = records[pivot_index].color;
        int32_t j = first;
        for (int32_t i = first; i < pivot_index; i += 1){
            int_color color = records[i].color;
            if (color < pivot_color){
                Swap(Highlight_Record, records[i], records[j]);
                j += 1;
            }
        }
        Swap(Highlight_Record, records[pivot_index], records[j]);
        pivot_index = j;
        
        sort_highlight_record(records, first, pivot_index);
        sort_highlight_record(records, pivot_index + 1, one_past_last);
    }
}

static Range_Array
get_enclosure_ranges(Application_Links *app, Partition *part,
                     Buffer_Summary *buffer, int32_t pos, uint32_t flags){
    Range_Array array = {};
    array.ranges = push_array(part, Range, 0);
    for (;;){
        Range range = {};
        if (find_scope_range(app, buffer, pos, &range, flags)){
            Range *r = push_array(part, Range, 1);
            *r = range;
            pos = range.first;
        }
        else{
            break;
        }
    }
    array.count = (int32_t)(push_array(part, Range, 0) - array.ranges);
    return(array);
}

static void
mark_enclosures(Application_Links *app, Partition *scratch, Managed_Scope render_scope,
                Buffer_Summary *buffer, int32_t pos, uint32_t flags,
                Marker_Visual_Type type,
                int_color *back_colors, int_color *fore_colors, int32_t color_count){
    Temp_Memory temp = begin_temp_memory(scratch);
    Range_Array ranges = get_enclosure_ranges(app, scratch,
                                              buffer, pos, flags);
    
    if (ranges.count > 0){
        int32_t marker_count = ranges.count*2;
        Marker *markers = push_array(scratch, Marker, marker_count);
        Marker *marker = markers;
        Range *range = ranges.ranges;
        for (int32_t i = 0;
             i < ranges.count;
             i += 1, range += 1, marker += 2){
            marker[0].pos = range->first;
            marker[1].pos = range->one_past_last - 1;
        }
        Managed_Object o = alloc_buffer_markers_on_buffer(app, buffer->buffer_id, marker_count, &render_scope);
        managed_object_store_data(app, o, 0, marker_count, markers);
        
        Marker_Visual_Take_Rule take_rule = {};
        take_rule.take_count_per_step = 2;
        take_rule.step_stride_in_marker_count = 8;
        
        int32_t first_color_index = (ranges.count - 1)%color_count;
        for (int32_t i = 0, color_index = first_color_index;
             i < color_count;
             i += 1){
            Marker_Visual visual = create_marker_visual(app, o);
            int_color back = SymbolicColor_Transparent;
            int_color fore = SymbolicColor_Default;
            if (back_colors != 0){
                back = back_colors[color_index];
            }
            if (fore_colors != 0){
                fore = fore_colors[color_index];
            }
            marker_visual_set_effect(app, visual, type, back, fore, 0);
            take_rule.first_index = i*2;
            marker_visual_set_take_rule(app, visual, take_rule);
            color_index = color_index - 1;
            if (color_index < 0){
                color_index += color_count;
            }
        }
    }
    
    end_temp_memory(temp);
}

RENDER_CALLER_SIG(default_render_caller){
    View_Summary view = get_view(app, view_id, AccessAll);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
    View_Summary active_view = get_active_view(app, AccessAll);
    bool32 is_active_view = (active_view.view_id == view_id);
    
    static Managed_Scope render_scope = 0;
    if (render_scope == 0){
        render_scope = create_user_managed_scope(app);
    }
    
    Partition *scratch = &global_part;
    
    // NOTE(allen): Scan for TODOs and NOTEs
    {
        Theme_Color colors[2];
        colors[0].tag = Stag_Text_Cycle_2;
        colors[1].tag = Stag_Text_Cycle_1;
        get_theme_colors(app, colors, 2);
        
        Temp_Memory temp = begin_temp_memory(scratch);
        int32_t text_size = on_screen_range.one_past_last - on_screen_range.first;
        char *text = push_array(scratch, char, text_size);
        buffer_read_range(app, &buffer, on_screen_range.first, on_screen_range.one_past_last, text);
        
        Highlight_Record *records = push_array(scratch, Highlight_Record, 0);
        String tail = make_string(text, text_size);
        for (int32_t i = 0; i < text_size; tail.str += 1, tail.size -= 1, i += 1){
            if (match_part(tail, make_lit_string("NOTE"))){
                Highlight_Record *record = push_array(scratch, Highlight_Record, 1);
                record->first = i + on_screen_range.first;
                record->one_past_last = record->first + 4;
                record->color = colors[0].color;
                tail.str += 3;
                tail.size -= 3;
                i += 3;
            }
            else if (match_part(tail, make_lit_string("TODO"))){
                Highlight_Record *record = push_array(scratch, Highlight_Record, 1);
                record->first = i + on_screen_range.first;
                record->one_past_last = record->first + 4;
                record->color = colors[1].color;
                tail.str += 3;
                tail.size -= 3;
                i += 3;
            }
        }
        int32_t record_count = (int32_t)(push_array(scratch, Highlight_Record, 0) - records);
        push_array(scratch, Highlight_Record, 1);
        
        if (record_count > 0){
            sort_highlight_record(records, 0, record_count);
            Temp_Memory marker_temp = begin_temp_memory(scratch);
            Marker *markers = push_array(scratch, Marker, 0);
            int_color current_color = records[0].color;
            {
                Marker *marker = push_array(scratch, Marker, 2);
                marker[0].pos = records[0].first;
                marker[1].pos = records[0].one_past_last;
            }
            for (int32_t i = 1; i <= record_count; i += 1){
                bool32 do_emit = i == record_count || (records[i].color != current_color);
                if (do_emit){
                    int32_t marker_count = (int32_t)(push_array(scratch, Marker, 0) - markers);
                    Managed_Object o = alloc_buffer_markers_on_buffer(app, buffer.buffer_id, marker_count, &render_scope);
                    managed_object_store_data(app, o, 0, marker_count, markers);
                    Marker_Visual v = create_marker_visual(app, o);
                    marker_visual_set_effect(app, v,
                                             VisualType_CharacterHighlightRanges,
                                             SymbolicColor_Transparent, current_color, 0);
                    marker_visual_set_priority(app, v, VisualPriority_Lowest);
                    end_temp_memory(marker_temp);
                    current_color = records[i].color;
                }
                
                Marker *marker = push_array(scratch, Marker, 2);
                marker[0].pos = records[i].first;
                marker[1].pos = records[i].one_past_last;
            }
        }
        
        end_temp_memory(temp);
    }
    
    // NOTE(allen): Cursor and mark
    Managed_Object cursor_and_mark = alloc_buffer_markers_on_buffer(app, buffer.buffer_id, 2, &render_scope);
    Marker cm_markers[2] = {};
    cm_markers[0].pos = view.cursor.pos;
    cm_markers[1].pos = view.mark.pos;
    managed_object_store_data(app, cursor_and_mark, 0, 2, cm_markers);
    
    bool32 cursor_is_hidden_in_this_view = (cursor_is_hidden && is_active_view);
    if (!cursor_is_hidden_in_this_view){
        switch (fcoder_mode){
            case FCoderMode_Original:
            {
                Theme_Color colors[2] = {};
                colors[0].tag = Stag_Cursor;
                colors[1].tag = Stag_Mark;
                get_theme_colors(app, colors, 2);
                int_color cursor_color = SymbolicColorFromPalette(Stag_Cursor);
                int_color mark_color   = SymbolicColorFromPalette(Stag_Mark);
                int_color text_color    = is_active_view?
                    SymbolicColorFromPalette(Stag_At_Cursor):SymbolicColorFromPalette(Stag_Default);
                
                Marker_Visual_Take_Rule take_rule = {};
                take_rule.first_index = 0;
                take_rule.take_count_per_step = 1;
                take_rule.step_stride_in_marker_count = 1;
                take_rule.maximum_number_of_markers = 1;
                
                Marker_Visual visual = create_marker_visual(app, cursor_and_mark);
                Marker_Visual_Type type = is_active_view?VisualType_CharacterBlocks:VisualType_CharacterWireFrames;
                marker_visual_set_effect(app, visual,
                                         type, cursor_color, text_color, 0);
                marker_visual_set_take_rule(app, visual, take_rule);
                marker_visual_set_priority(app, visual, VisualPriority_Highest);
                
                visual = create_marker_visual(app, cursor_and_mark);
                marker_visual_set_effect(app, visual,
                                         VisualType_CharacterWireFrames, mark_color, 0, 0);
                take_rule.first_index = 1;
                marker_visual_set_take_rule(app, visual, take_rule);
                marker_visual_set_priority(app, visual, VisualPriority_Highest);
            }break;
            
            case FCoderMode_NotepadLike:
            {
                Theme_Color colors[2] = {};
                colors[0].tag = Stag_Cursor;
                colors[1].tag = Stag_Highlight;
                get_theme_colors(app, colors, 2);
                int_color cursor_color    = SymbolicColorFromPalette(Stag_Cursor);
                int_color highlight_color = SymbolicColorFromPalette(Stag_Highlight);
                
                Marker_Visual_Take_Rule take_rule = {};
                take_rule.first_index = 0;
                take_rule.take_count_per_step = 1;
                take_rule.step_stride_in_marker_count = 1;
                take_rule.maximum_number_of_markers = 1;
                
                Marker_Visual visual = create_marker_visual(app, cursor_and_mark);
                marker_visual_set_effect(app, visual, VisualType_CharacterIBars, cursor_color, 0, 0);
                marker_visual_set_take_rule(app, visual, take_rule);
                marker_visual_set_priority(app, visual, VisualPriority_Highest);
                
                if (view.cursor.pos != view.mark.pos){
                    visual = create_marker_visual(app, cursor_and_mark);
                    marker_visual_set_effect(app, visual, VisualType_CharacterHighlightRanges, highlight_color, SymbolicColorFromPalette(Stag_At_Highlight), 0);
                    take_rule.maximum_number_of_markers = 2;
                    marker_visual_set_take_rule(app, visual, take_rule);
                    marker_visual_set_priority(app, visual, VisualPriority_Highest);
                }
            }break;
        }
    }
    
    // NOTE(allen): Line highlight setup
    if (highlight_line_at_cursor && is_active_view){
        Theme_Color color = {};
        color.tag = Stag_Highlight_Cursor_Line;
        get_theme_colors(app, &color, 1);
        uint32_t line_color = color.color;
        Marker_Visual visual = create_marker_visual(app, cursor_and_mark);
        marker_visual_set_effect(app, visual, VisualType_LineHighlights,
                                 line_color, 0, 0);
        Marker_Visual_Take_Rule take_rule = {};
        take_rule.first_index = 0;
        take_rule.take_count_per_step = 1;
        take_rule.step_stride_in_marker_count = 1;
        take_rule.maximum_number_of_markers = 1;
        marker_visual_set_take_rule(app, visual, take_rule);
        marker_visual_set_priority(app, visual, VisualPriority_Highest);
    }
    
    // NOTE(allen): Token highlight setup
    bool32 do_token_highlight = false;
    if (do_token_highlight){
        Theme_Color color = {};
        color.tag = Stag_Cursor;
        get_theme_colors(app, &color, 1);
        uint32_t token_color = (0x50 << 24) | (color.color&0xFFFFFF);
        
        uint32_t token_flags = BoundaryToken|BoundaryWhitespace;
        int32_t pos0 = view.cursor.pos;
        int32_t pos1 = buffer_boundary_seek(app, &buffer, pos0, DirLeft , token_flags);
        int32_t pos2 = buffer_boundary_seek(app, &buffer, pos1, DirRight, token_flags);
        
        Managed_Object token_highlight = alloc_buffer_markers_on_buffer(app, buffer.buffer_id, 2, &render_scope);
        Marker range_markers[2] = {};
        range_markers[0].pos = pos1;
        range_markers[1].pos = pos2;
        managed_object_store_data(app, token_highlight, 0, 2, range_markers);
        Marker_Visual visual = create_marker_visual(app, token_highlight);
        marker_visual_set_effect(app, visual, VisualType_CharacterHighlightRanges, token_color, SymbolicColorFromPalette(Stag_At_Highlight), 0);
    }
    
    // NOTE(allen): Matching enclosure highlight setup
    static const int32_t color_count = 4;
    if (do_matching_enclosure_highlight){
        Theme_Color theme_colors[color_count];
        int_color colors[color_count];
        for (int32_t i = 0; i < 4; i += 1){
            theme_colors[i].tag = Stag_Back_Cycle_1 + i;
        }
        get_theme_colors(app, theme_colors, color_count);
        for (int32_t i = 0; i < 4; i += 1){
            colors[i] = theme_colors[i].color;
        }
        mark_enclosures(app, scratch, render_scope,
                        &buffer, view.cursor.pos, FindScope_Brace,
                        VisualType_LineHighlightRanges,
                        colors, 0, color_count);
    }
    if (do_matching_paren_highlight){
        Theme_Color theme_colors[color_count];
        int_color colors[color_count];
        for (int32_t i = 0; i < 4; i += 1){
            theme_colors[i].tag = Stag_Text_Cycle_1 + i;
        }
        get_theme_colors(app, theme_colors, color_count);
        for (int32_t i = 0; i < 4; i += 1){
            colors[i] = theme_colors[i].color;
        }
        int32_t pos = view.cursor.pos;
        if (buffer_get_char(app, &buffer, pos) == '('){
            pos += 1;
        }
        else if (pos > 0){
            if (buffer_get_char(app, &buffer, pos - 1) == ')'){
                pos -= 1;
            }
        }
        mark_enclosures(app, scratch, render_scope,
                        &buffer, pos, FindScope_Paren,
                        VisualType_CharacterBlocks,
                        0, colors, color_count);
    }
    
    do_core_render(app);
    
    managed_scope_clear_self_all_dependent_scopes(app, render_scope);
}

HOOK_SIG(default_exit){
    // If this returns zero it cancels the exit.
    if (allow_immediate_close_without_checking_for_changes){
        return(1);
    }
    
    bool32 has_unsaved_changes = false;
    
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        if (buffer.dirty == DirtyState_UnsavedChanges){
            has_unsaved_changes = true;
            break;
        }
    }
    
    if (has_unsaved_changes){
        View_Summary view = get_active_view(app, AccessAll);
        do_gui_sure_to_close_4coder(app, &view);
        return(0);
    }
    
    return(1);
}

HOOK_SIG(default_view_adjust){
    int32_t count = 0;
    int32_t new_wrap_width = 0;
    for (View_Summary view = get_view_first(app, AccessAll);
         view.exists;
         get_view_next(app, &view, AccessAll)){
        new_wrap_width += view.view_region.x1 - view.view_region.x0;
        ++count;
    }
    
    new_wrap_width /= count;
    new_wrap_width = (int32_t)(new_wrap_width * .9f);
    
    int32_t new_min_base_width = (int32_t)(new_wrap_width * .77f);
    if (global_config.automatically_adjust_wrapping){
        adjust_all_buffer_wrap_widths(app, new_wrap_width, new_min_base_width);
        global_config.default_wrap_width = new_wrap_width;
        global_config.default_min_base_width = new_min_base_width;
    }
    
    // no meaning for return
    return(0);
}

BUFFER_NAME_RESOLVER_SIG(default_buffer_name_resolution){
    if (conflict_count > 1){
        // List of unresolved conflicts
        Partition *part = &global_part;
        Temp_Memory temp = begin_temp_memory(part);
        
        int32_t *unresolved = push_array(part, int32_t, conflict_count);
        if (unresolved == 0) return;
        
        int32_t unresolved_count = conflict_count;
        for (int32_t i = 0; i < conflict_count; ++i){
            unresolved[i] = i;
        }
        
        // Resolution Loop
        int32_t x = 0;
        for (;;){
            // Resolution Pass
            ++x;
            for (int32_t i = 0; i < unresolved_count; ++i){
                int32_t conflict_index = unresolved[i];
                Buffer_Name_Conflict_Entry *conflict = &conflicts[conflict_index];
                
                int32_t len = conflict->base_name_len;
                if (len < 0){
                    len = 0;
                }
                if (len > conflict->unique_name_capacity){
                    len = conflict->unique_name_capacity;
                }
                conflict->unique_name_len_in_out = len;
                memcpy(conflict->unique_name_in_out, conflict->base_name, len);
                
                if (conflict->file_name != 0){
                    char uniqueifier_space[256];
                    String uniqueifier = make_fixed_width_string(uniqueifier_space);
                    
                    String s_file_name = make_string(conflict->file_name, conflict->file_name_len);
                    s_file_name = path_of_directory(s_file_name);
                    if (s_file_name.size > 0){
                        s_file_name.size -= 1;
                        char *end = s_file_name.str + s_file_name.size;
                        bool32 past_the_end = false;
                        for (int32_t j = 0; j < x; ++j){
                            s_file_name = path_of_directory(s_file_name);
                            if (j + 1 < x){
                                s_file_name.size -= 1;
                            }
                            if (s_file_name.size <= 0){
                                if (j + 1 < x){
                                    past_the_end = true;
                                }
                                s_file_name.size = 0;
                                break;
                            }
                        }
                        char *start = s_file_name.str + s_file_name.size;
                        
                        append(&uniqueifier, make_string(start, (int32_t)(end - start)));
                        if (past_the_end){
                            append(&uniqueifier, "~");
                            append_int_to_str(&uniqueifier, i);
                        }
                    }
                    else{
                        append_int_to_str(&uniqueifier, i);
                    }
                    
                    String builder = make_string_cap(conflict->unique_name_in_out,
                                                     conflict->unique_name_len_in_out,
                                                     conflict->unique_name_capacity);
                    append(&builder, " <");
                    append(&builder, uniqueifier);
                    append(&builder, ">");
                    conflict->unique_name_len_in_out = builder.size;
                }
            }
            
            // Conflict Check Pass
            bool32 has_conflicts = false;
            for (int32_t i = 0; i < unresolved_count; ++i){
                int32_t conflict_index = unresolved[i];
                Buffer_Name_Conflict_Entry *conflict = &conflicts[conflict_index];
                String conflict_name = make_string(conflict->unique_name_in_out,
                                                   conflict->unique_name_len_in_out);
                
                bool32 hit_conflict = false;
                if (conflict->file_name != 0){
                    for (int32_t j = 0; j < unresolved_count; ++j){
                        if (i == j) continue;
                        
                        int32_t conflict_j_index = unresolved[j];
                        Buffer_Name_Conflict_Entry *conflict_j = &conflicts[conflict_j_index];
                        
                        if (match(conflict_name, make_string(conflict_j->unique_name_in_out,
                                                             conflict_j->unique_name_len_in_out))){
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
        
        end_temp_memory(temp);
    }
}

OPEN_FILE_HOOK_SIG(default_file_settings){
    // NOTE(allen|a4.0.8): The get_parameter_buffer was eliminated
    // and instead the buffer is passed as an explicit parameter through
    // the function call.  That is where buffer_id comes from here.
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    Assert(buffer.exists);
    
    bool32 treat_as_code = false;
    bool32 treat_as_todo = false;
    bool32 lex_without_strings = false;
    
    CString_Array extensions = get_code_extensions(&global_config.code_exts);
    
    Parse_Context_ID parse_context_id = 0;
    
    if (buffer.file_name != 0 && buffer.size < (16 << 20)){
        String name = make_string(buffer.file_name, buffer.file_name_len);
        String ext = file_extension(name);
        for (int32_t i = 0; i < extensions.count; ++i){
            if (match(ext, extensions.strings[i])){
                treat_as_code = true;
                
                if (match(ext, "cs")){
                    if (parse_context_language_cs == 0){
                        init_language_cs(app);
                    }
                    parse_context_id = parse_context_language_cs;
                }
                
                if (match(ext, "java")){
                    if (parse_context_language_java == 0){
                        init_language_java(app);
                    }
                    parse_context_id = parse_context_language_java;
                }
                
                if (match(ext, "rs")){
                    if (parse_context_language_rust == 0){
                        init_language_rust(app);
                    }
                    parse_context_id = parse_context_language_rust;
                    lex_without_strings = true;
                }
                
                if (match(ext, "cpp") || match(ext, "h") || match(ext, "c") || match(ext, "hpp") || match(ext, "cc")){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real GLSL highlighting
                if (match(ext, "glsl")){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                // TODO(NAME): Real Objective-C highlighting
                if (match(ext, "m")){
                    if (parse_context_language_cpp == 0){
                        init_language_cpp(app);
                    }
                    parse_context_id = parse_context_language_cpp;
                }
                
                break;
            }
        }
        
        if (!treat_as_code){
            treat_as_todo = match_insensitive(front_of_directory(name), "todo.txt");
        }
    }
    
    int32_t map_id = (treat_as_code)?((int32_t)default_code_map):((int32_t)mapid_file);
    int32_t map_id_query = 0;
    
    buffer_set_setting(app, &buffer, BufferSetting_MapID, default_lister_ui_map);
    buffer_get_setting(app, &buffer, BufferSetting_MapID, &map_id_query);
    Assert(map_id_query == default_lister_ui_map);
    
    buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, global_config.default_wrap_width);
    buffer_set_setting(app, &buffer, BufferSetting_MinimumBaseWrapPosition, global_config.default_min_base_width);
    buffer_set_setting(app, &buffer, BufferSetting_MapID, map_id);
    buffer_get_setting(app, &buffer, BufferSetting_MapID, &map_id_query);
    Assert(map_id_query == map_id);
    buffer_set_setting(app, &buffer, BufferSetting_ParserContext, parse_context_id);
    
    // NOTE(allen): Decide buffer settings
    bool32 wrap_lines = true;
    bool32 use_virtual_whitespace = false;
    bool32 use_lexer = false;
    if (treat_as_todo){
        lex_without_strings = true;
        wrap_lines = true;
        use_virtual_whitespace = true;
        use_lexer = true;
    }
    else if (treat_as_code){
        wrap_lines = global_config.enable_code_wrapping;
        use_virtual_whitespace = global_config.enable_virtual_whitespace;
        use_lexer = true;
    }
    if (match(make_string(buffer.buffer_name, buffer.buffer_name_len), "*compilation*")){
        wrap_lines = false;
    }
    //if (buffer.size >= (192 << 10)){
    if (buffer.size >= (128 << 10)){
        wrap_lines = false;
        use_virtual_whitespace = false;
    }
    
    // NOTE(allen|a4.0.12): There is a little bit of grossness going on here.
    // If we set BufferSetting_Lex to true, it will launch a lexing job.
    // If a lexing job is active when we set BufferSetting_VirtualWhitespace, the call can fail.
    // Unfortunantely without tokens virtual whitespace doesn't really make sense.
    // So for now I have it automatically turning on lexing when virtual whitespace is turned on.
    // Cleaning some of that up is a goal for future versions.
    buffer_set_setting(app, &buffer, BufferSetting_LexWithoutStrings, lex_without_strings);
    buffer_set_setting(app, &buffer, BufferSetting_WrapLine, wrap_lines);
    buffer_set_setting(app, &buffer, BufferSetting_VirtualWhitespace, use_virtual_whitespace);
    buffer_set_setting(app, &buffer, BufferSetting_Lex, use_lexer);
    
    // no meaning for return
    return(0);
}

OPEN_FILE_HOOK_SIG(default_new_file){
#if 0
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessOpen);
    char str[] = "/*\nNew File\n*/\n\n\n";
    buffer_replace_range(app, &buffer, 0, 0, str, sizeof(str)-1);
#endif
    
    // no meaning for return
    return(0);
}

OPEN_FILE_HOOK_SIG(default_file_save){
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    Assert(buffer.exists);
    
    int32_t is_virtual = 0;
    if (global_config.automatically_indent_text_on_save &&
        buffer_get_setting(app, &buffer, BufferSetting_VirtualWhitespace, &is_virtual)){ 
        if (is_virtual){
            buffer_auto_indent(app, &global_part, &buffer, 0, buffer.size, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
        }
    }
    
    // no meaning for return
    return(0);
}

FILE_EDIT_FINISHED_SIG(default_file_edit){
    for (int32_t i = 0; i < buffer_id_count; i += 1){
#if 0
        // NOTE(allen|4.0.31): This code is example usage, it's not a particularly nice feature to actually have.
        
        Buffer_Summary buffer = get_buffer(app, buffer_ids[i], AccessAll);
        Assert(buffer.exists);
        
        String buffer_name = make_string(buffer.buffer_name, buffer.buffer_name_len);
        char space[256];
        String str = make_fixed_width_string(space);
        append(&str, "edit finished: ");
        append(&str, buffer_name);
        append(&str, "\n");
        print_message(app, str.str, str.size);
#endif
    }
    
    // no meaning for return
    return(0);
}

OPEN_FILE_HOOK_SIG(default_end_file){
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    Assert(buffer.exists);
    
    char space[1024];
    String str = make_fixed_width_string(space);
    append(&str, "Ending file: ");
    append(&str, make_string(buffer.buffer_name, buffer.buffer_name_len));
    append(&str, "\n");
    
    print_message(app, str.str, str.size);
    
    // no meaning for return
    return(0);
}

// NOTE(allen|a4.0.9): The input filter allows you to modify the input
// to a frame before 4coder starts processing it at all.
//
// Right now it only has access to the mouse state, but it will be
// extended to have access to the key presses soon.
INPUT_FILTER_SIG(default_suppress_mouse_filter){
    if (suppressing_mouse){
        *mouse = null_mouse_state;
        mouse->x = -100;
        mouse->y = -100;
    }
}

// NOTE(allen|a4): scroll rule information
//
// The parameters:
// target_x, target_y
//  This is where the view would like to be for the purpose of
// following the cursor, doing mouse wheel work, etc.
//
// scroll_x, scroll_y
//  These are pointers to where the scrolling actually is. If you bind
// the scroll rule it is you have to update these in some way to move
// the actual location of the scrolling.
//
// view_id
//  This corresponds to which view is computing it's new scrolling position.
// This id DOES correspond to the views that View_Summary contains.
// This will always be between 1 and 16 (0 is a null id).
// See below for an example of having state that carries across scroll udpates.
//
// is_new_target
//  If the target of the view is different from the last target in either x or y
// this is true, otherwise it is false.
//
// The return:
//  Should be true if and only if scroll_x or scroll_y are changed.
//
// Don't try to use the app pointer in a scroll rule, you're asking for trouble.
//
// If you don't bind scroll_rule, nothing bad will happen, yo will get default
// 4coder scrolling behavior.
//

struct Scroll_Velocity{
    float x, y;
};

Scroll_Velocity scroll_velocity_[16] = {};
Scroll_Velocity *scroll_velocity = scroll_velocity_ - 1;

static int32_t
smooth_camera_step(float target, float *current, float *vel, float S, float T){
    int32_t result = 0;
    float curr = *current;
    float v = *vel;
    if (curr != target){
        if (curr > target - .1f && curr < target + .1f){
            curr = target;
            v = 1.f;
        }
        else{
            float L = curr + T*(target - curr);
            
            int32_t sign = (target > curr) - (target < curr);
            float V = curr + sign*v;
            
            if (sign > 0) curr = (L<V)?(L):(V);
            else curr = (L>V)?(L):(V);
            
            if (curr == V){
                v *= S;
            }
        }
        
        *current = curr;
        *vel = v;
        result = 1;
    }
    return(result);
}

SCROLL_RULE_SIG(smooth_scroll_rule){
    Scroll_Velocity *velocity = scroll_velocity + view_id;
    int32_t result = 0;
    if (velocity->x == 0.f){
        velocity->x = 1.f;
        velocity->y = 1.f;
    }
    if (smooth_camera_step(target_y, scroll_y, &velocity->y, 80.f, 1.f/2.f)){
        result = 1;
    }
    if (smooth_camera_step(target_x, scroll_x, &velocity->x, 80.f, 1.f/2.f)){
        result = 1;
    }
    return(result);
}

static void
set_all_default_hooks(Bind_Helper *context){
    set_hook(context, hook_exit, default_exit);
    set_hook(context, hook_view_size_change, default_view_adjust);
    
    set_start_hook(context, default_start);
    set_open_file_hook(context, default_file_settings);
    set_new_file_hook(context, default_new_file);
    set_save_file_hook(context, default_file_save);
    set_file_edit_finished_hook(context, default_file_edit);
    
    set_end_file_hook(context, end_file_close_jump_list);
    
    set_command_caller(context, default_command_caller);
    set_render_caller(context, default_render_caller);
    set_input_filter(context, default_suppress_mouse_filter);
    set_scroll_rule(context, smooth_scroll_rule);
    set_buffer_name_resolver(context, default_buffer_name_resolution);
}

// BOTTOM

