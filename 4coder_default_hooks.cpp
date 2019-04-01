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
    
    default_4coder_initialize(app, files, file_count);
    default_4coder_side_by_side_panels(app, files, file_count);
    
#if 0
    
    default_4coder_one_panel(app, files, file_count);
    
    View_ID left_view = 0;
    
    Panel_ID left = 0;
    Panel_ID right = 0;
    Panel_ID bottom = 0;
    Panel_ID header = 0;
    
    get_active_view(app, AccessAll, &left_view);
    
    view_get_panel(app, left_view, &left);
    
    Panel_ID h_split_main = 0;
    Panel_ID h_split_minor = 0;
    
    panel_create_split(app, left  , ViewSplit_Bottom, &bottom, &h_split_main);
    panel_create_split(app, bottom, ViewSplit_Top   , &header, &h_split_minor);
    panel_create_split(app, left  , ViewSplit_Right , &right,  0);
    
    i32_Rect header_margin = {};
    i32_Rect bottom_margin = {};
    panel_get_margin(app, header, &header_margin);
    panel_get_margin(app, header, &bottom_margin);
    
    i32 header_vertical_pixels = header_margin.y0 + header_margin.y1;
    i32 margin_vertical_pixels = header_vertical_pixels + bottom_margin.y0 + bottom_margin.y1;
    
    View_Summary view = {};
    get_view_summary(app, left_view, AccessAll, &view);
    float line = view.line_height;
    panel_set_split(app, h_split_main , PanelSplitKind_FixedPixels_BR, line*6.f + margin_vertical_pixels);
    panel_set_split(app, h_split_minor, PanelSplitKind_FixedPixels_TL, line + header_vertical_pixels);
#endif
    
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
    
    cmd.command(app);
    
    u64 next_rewrite = 0;
    managed_variable_get(app, scope, view_next_rewrite_loc, &next_rewrite);
    managed_variable_set(app, scope, view_rewrite_loc, next_rewrite);
    if (fcoder_mode == FCoderMode_NotepadLike){
        for (View_Summary view_it = get_view_first(app, AccessAll);
             view_it.exists;
             get_view_next(app, &view_it, AccessAll)){
            Managed_Scope scope_it = view_get_managed_scope(app, view_it.view_id);
            u64 val = 0;
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
    i32 first;
    i32 one_past_last;
    int_color color;
};

static void
sort_highlight_record(Highlight_Record *records, i32 first, i32 one_past_last){
    if (first + 1 < one_past_last){
        i32 pivot_index = one_past_last - 1;
        int_color pivot_color = records[pivot_index].color;
        i32 j = first;
        for (i32 i = first; i < pivot_index; i += 1){
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
                     Buffer_Summary *buffer, i32 pos, u32 flags){
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
    array.count = (i32)(push_array(part, Range, 0) - array.ranges);
    return(array);
}

static void
mark_enclosures(Application_Links *app, Partition *scratch, Managed_Scope render_scope,
                Buffer_Summary *buffer, i32 pos, u32 flags,
                Marker_Visual_Type type,
                int_color *back_colors, int_color *fore_colors, i32 color_count){
    Temp_Memory temp = begin_temp_memory(scratch);
    Range_Array ranges = get_enclosure_ranges(app, scratch, buffer, pos, flags);
    
    if (ranges.count > 0){
        i32 marker_count = ranges.count*2;
        Marker *markers = push_array(scratch, Marker, marker_count);
        Marker *marker = markers;
        Range *range = ranges.ranges;
        for (i32 i = 0;
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
        
        i32 first_color_index = (ranges.count - 1)%color_count;
        for (i32 i = 0, color_index = first_color_index;
             i < color_count;
             i += 1){
            Marker_Visual visual = create_marker_visual(app, o);
            int_color back = 0;
            int_color fore = 0;
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

GET_VIEW_BUFFER_REGION_SIG(default_view_buffer_region){
    View_Summary view = {};
    get_view_summary(app, view_id, AccessAll, &view);
    i32 line_height = ceil32(view.line_height);
    
    // file bar
    {
        b32 showing_file_bar = false;
        if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar)){
            if (showing_file_bar){
                sub_region.y0 += line_height + 2;
            }
        }
    }
    
    // query bar
    {
        Query_Bar *space[32];
        Query_Bar_Ptr_Array query_bars = {};
        query_bars.ptrs = space;
        if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars)){
            i32 widget_height = (line_height + 2)*query_bars.count;
            sub_region.y0 += widget_height;
        }
    }
    
    // line number margins
    if (global_config.show_line_number_margins){
        Buffer_Summary buffer = {};
        get_buffer_summary(app, view.buffer_id, AccessAll, &buffer);
        i32 line_count_digit_count = int_to_str_size(buffer.line_count);
        Face_ID font_id = 0;
        get_face_id(app, view.buffer_id, &font_id);
        // TODO(allen): I need a "digit width"
        f32 zero = get_string_advance(app, font_id, make_lit_string("0"));
        i32 margin_width = ceil32((f32)line_count_digit_count*zero);
        sub_region.x0 += margin_width + 2;
    }
    
    return(sub_region);
}

struct View_Render_Parameters{
    View_ID view_id;
    Range on_screen_range;
    Rect_i32 buffer_rect;
};

static Buffer_Point
buffer_position_from_scroll_position(Application_Links *app, View_ID view_id, Vec2 scroll){
    Full_Cursor cursor = {};
    view_compute_cursor(app, view_id, seek_wrapped_xy(0.f, scroll.y, false), &cursor);
    view_compute_cursor(app, view_id, seek_line_char(cursor.line, 1), &cursor);
    Buffer_Point result = {};
    result.line_number = cursor.line;
    result.pixel_shift.x = scroll.x;
    result.pixel_shift.y = scroll.y - cursor.wrapped_y;
    return(result);
}

static void
default_buffer_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id, Rect_i32 view_inner_rect){
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view_id, AccessAll, &buffer_id);
    
    Rect_i32 sub_region = i32R(0, 0, rect_width(view_inner_rect), rect_height(view_inner_rect));
    sub_region = default_view_buffer_region(app, view_id, sub_region);
    Rect_i32 buffer_rect = {};
    buffer_rect.p0 = view_inner_rect.p0 + sub_region.p0;
    buffer_rect.p1 = view_inner_rect.p0 + sub_region.p1;
    buffer_rect.x1 = clamp_top(buffer_rect.x1, view_inner_rect.x1);
    buffer_rect.y1 = clamp_top(buffer_rect.y1, view_inner_rect.y1);
    buffer_rect.x0 = clamp_top(buffer_rect.x0, buffer_rect.x1);
    buffer_rect.y0 = clamp_top(buffer_rect.y0, buffer_rect.y1);
    
    GUI_Scroll_Vars scroll = {};
    view_get_scroll_vars(app, view_id, &scroll);
    
    Buffer_Point buffer_point = buffer_position_from_scroll_position(app, view_id, scroll.scroll_p);
    Range on_screen_range = {};
    Text_Layout_ID text_layout_id = 0;
    compute_render_layout(app, view_id, buffer_id, buffer_rect, buffer_point,
                          &on_screen_range, &text_layout_id);
    
    View_Summary view = get_view(app, view_id, AccessAll);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
    View_Summary active_view = get_active_view(app, AccessAll);
    b32 is_active_view = (active_view.view_id == view_id);
    
    f32 line_height = view.line_height;
    
    Arena *arena = context_get_arena(app);
    Temp_Memory_Arena major_temp = begin_temp_memory(arena);
    
    static Managed_Scope render_scope = 0;
    if (render_scope == 0){
        render_scope = create_user_managed_scope(app);
    }
    
    {
        Rect_f32 r_cursor = f32R(view.render_region);
        
        // NOTE(allen): Filebar
        {
            b32 showing_file_bar = false;
            if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar)){
                if (showing_file_bar){
                    Face_ID face_id = 0;
                    get_face_id(app, view.buffer_id, &face_id);
                    
                    Rect_f32 bar = r_cursor;
                    bar.y1 = bar.y0 + line_height + 2.f;
                    r_cursor.y0 = bar.y1;
                    
                    draw_rectangle(app, bar, Stag_Bar);
                    
                    Fancy_Color base_color = fancy_id(Stag_Base);
                    Fancy_Color pop2_color = fancy_id(Stag_Pop2);
                    
                    Temp_Memory_Arena temp = begin_temp_memory(arena);
                    
                    Fancy_String_List list = {};
                    push_fancy_string(arena, &list, base_color, make_string(buffer.buffer_name, buffer.buffer_name_len));
                    push_fancy_stringf(arena, &list, base_color, " - L#%d C#%d -", view.cursor.line, view.cursor.character);
                    
                    Face_Metrics face_metrics = {};
                    get_face_metrics(app, face_id, &face_metrics);
                    push_fancy_stringf(arena, &list, base_color, " LH: %f; TCW: %f-",
                                       face_metrics.line_height, face_metrics.typical_character_width);
                    
                    b32 is_dos_mode = false;
                    if (buffer_get_setting(app, buffer.buffer_id, BufferSetting_Eol, &is_dos_mode)){
                        if (is_dos_mode){
                            push_fancy_string(arena, &list, base_color, make_lit_string(" dos"));
                        }
                        else{
                            push_fancy_string(arena, &list, base_color, make_lit_string(" nix"));
                        }
                    }
                    else{
                        push_fancy_string(arena, &list, base_color, make_lit_string(" ???"));
                    }
                    
                    {
                        Dirty_State dirty = buffer.dirty;
                        char space[3];
                        String str = make_fixed_width_string(space);
                        if (dirty != 0){
                            append(&str, " ");
                        }
                        if (HasFlag(dirty, DirtyState_UnsavedChanges)){
                            append(&str, "*");
                        }
                        if (HasFlag(dirty, DirtyState_UnloadedChanges)){
                            append(&str, "!");
                        }
                        push_fancy_string(arena, &list, pop2_color, str);
                    }
                    
                    Vec2 p = bar.p0 + V2(0.f, 2.f);
                    draw_fancy_string(app, face_id, list.first, p, Stag_Default, 0);
                    
                    end_temp_memory(temp);
                }
            }
        }
        
        // NOTE(allen): Query Bars
        {
            Query_Bar *space[32];
            Query_Bar_Ptr_Array query_bars = {};
            query_bars.ptrs = space;
            if (get_active_query_bars(app, view_id, ArrayCount(space), &query_bars)){
                for (i32 i = 0; i < query_bars.count; i += 1){
                    Query_Bar *query_bar = query_bars.ptrs[i];
                    
                    Rect_f32 bar = r_cursor;
                    bar.y1 = bar.y0 + line_height + 2.f;
                    r_cursor.y0 = bar.y1;
                    
                    Temp_Memory_Arena temp = begin_temp_memory(arena);
                    Fancy_String_List list = {};
                    
                    Fancy_Color default_color = fancy_id(Stag_Default);
                    Fancy_Color pop1_color = fancy_id(Stag_Pop1);
                    
                    push_fancy_string(arena, &list, pop1_color   , query_bar->prompt);
                    push_fancy_string(arena, &list, default_color, query_bar->string);
                    
                    Face_ID font_id = 0;
                    get_face_id(app, view.buffer_id, &font_id);
                    Vec2 p = bar.p0 + V2(0.f, 2.f);
                    draw_fancy_string(app, font_id, list.first, p, Stag_Default, 0);
                    
                    end_temp_memory(temp);
                }
            }
        }
        
        // NOTE(allen): Line Numbers
        if (global_config.show_line_number_margins){
            i32 line_count_digit_count = int_to_str_size(buffer.line_count);
            Face_ID font_id = 0;
            get_face_id(app, view.buffer_id, &font_id);
            // TODO(allen): I need a "digit width"
            f32 zero = get_string_advance(app, font_id, make_lit_string("0"));
            f32 margin_width = (f32)line_count_digit_count*zero;
            
            Rect_f32 left_margin = r_cursor;
            left_margin.x1 = left_margin.x0 + margin_width + 2;
            r_cursor.x0 = left_margin.x1;
            
            draw_rectangle(app, left_margin, Stag_Line_Numbers_Back);
            draw_clip_push(app, left_margin);
            
            Fancy_Color line_color = fancy_id(Stag_Line_Numbers_Text);
            
            Full_Cursor cursor = {};
            view_compute_cursor(app, view_id, seek_pos(on_screen_range.first), &cursor);
            for (;cursor.pos <= on_screen_range.one_past_last;){
                Vec2 p = panel_space_from_view_space(cursor.wrapped_p, view.scroll_vars.scroll_p);
                p += V2(buffer_rect.p0);
                p.x = left_margin.x0;
                Temp_Memory_Arena temp = begin_temp_memory(arena);
                Fancy_String *line_string = push_fancy_stringf(arena, line_color, "%*d", line_count_digit_count, cursor.line);
                draw_fancy_string(app, font_id, line_string, p, Stag_Margin_Active, 0);
                end_temp_memory(temp);
                i32 next_line = cursor.line + 1;
                view_compute_cursor(app, view_id, seek_line_char(next_line, 1), &cursor);
                if (cursor.line < next_line){
                    break;
                }
            }
            
            draw_clip_pop(app);
        }
    }
    
    // TODO(allen): eliminate scratch partition usage
    Partition *scratch = &global_part;
    // NOTE(allen): Scan for TODOs and NOTEs
    {
        Temp_Memory temp = begin_temp_memory(scratch);
        i32 text_size = on_screen_range.one_past_last - on_screen_range.first;
        char *text = push_array(scratch, char, text_size);
        buffer_read_range(app, &buffer, on_screen_range.first, on_screen_range.one_past_last, text);
        
        Highlight_Record *records = push_array(scratch, Highlight_Record, 0);
        String tail = make_string(text, text_size);
        for (i32 i = 0; i < text_size; tail.str += 1, tail.size -= 1, i += 1){
            if (match_part(tail, make_lit_string("NOTE"))){
                Highlight_Record *record = push_array(scratch, Highlight_Record, 1);
                record->first = i + on_screen_range.first;
                record->one_past_last = record->first + 4;
                record->color = Stag_Text_Cycle_2;
                tail.str += 3;
                tail.size -= 3;
                i += 3;
            }
            else if (match_part(tail, make_lit_string("TODO"))){
                Highlight_Record *record = push_array(scratch, Highlight_Record, 1);
                record->first = i + on_screen_range.first;
                record->one_past_last = record->first + 4;
                record->color = Stag_Text_Cycle_1;
                tail.str += 3;
                tail.size -= 3;
                i += 3;
            }
        }
        i32 record_count = (i32)(push_array(scratch, Highlight_Record, 0) - records);
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
            for (i32 i = 1; i <= record_count; i += 1){
                b32 do_emit = i == record_count || (records[i].color != current_color);
                if (do_emit){
                    i32 marker_count = (i32)(push_array(scratch, Marker, 0) - markers);
                    Managed_Object o = alloc_buffer_markers_on_buffer(app, buffer.buffer_id, marker_count, &render_scope);
                    managed_object_store_data(app, o, 0, marker_count, markers);
                    Marker_Visual v = create_marker_visual(app, o);
                    marker_visual_set_effect(app, v, VisualType_CharacterHighlightRanges, SymbolicColor_Default, current_color, 0);
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
    
    b32 cursor_is_hidden_in_this_view = (cursor_is_hidden && is_active_view);
    if (!cursor_is_hidden_in_this_view){
        switch (fcoder_mode){
            case FCoderMode_Original:
            {
                Marker_Visual_Take_Rule take_rule = {};
                take_rule.first_index = 0;
                take_rule.take_count_per_step = 1;
                take_rule.step_stride_in_marker_count = 1;
                take_rule.maximum_number_of_markers = 1;
                
                Marker_Visual visual = create_marker_visual(app, cursor_and_mark);
                Marker_Visual_Type type = is_active_view?VisualType_CharacterBlocks:VisualType_CharacterWireFrames;
                int_color cursor_color = Stag_Cursor;
                int_color text_color = is_active_view?Stag_At_Cursor:Stag_Default;
                marker_visual_set_effect(app, visual, type, cursor_color, text_color, 0);
                marker_visual_set_take_rule(app, visual, take_rule);
                marker_visual_set_priority(app, visual, VisualPriority_Highest);
                
                visual = create_marker_visual(app, cursor_and_mark);
                marker_visual_set_effect(app, visual, VisualType_CharacterWireFrames, Stag_Mark, 0, 0);
                take_rule.first_index = 1;
                marker_visual_set_take_rule(app, visual, take_rule);
                marker_visual_set_priority(app, visual, VisualPriority_Highest);
            }break;
            
            case FCoderMode_NotepadLike:
            {
                int_color cursor_color    = Stag_Cursor;
                int_color highlight_color = Stag_Highlight;
                
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
                    marker_visual_set_effect(app, visual, VisualType_CharacterHighlightRanges, highlight_color, Stag_At_Highlight, 0);
                    take_rule.maximum_number_of_markers = 2;
                    marker_visual_set_take_rule(app, visual, take_rule);
                    marker_visual_set_priority(app, visual, VisualPriority_Highest);
                }
            }break;
        }
    }
    
    // NOTE(allen): Line highlight setup
    if (highlight_line_at_cursor && is_active_view){
        u32 line_color = Stag_Highlight_Cursor_Line;
        Marker_Visual visual = create_marker_visual(app, cursor_and_mark);
        marker_visual_set_effect(app, visual, VisualType_LineHighlights, line_color, 0, 0);
        Marker_Visual_Take_Rule take_rule = {};
        take_rule.first_index = 0;
        take_rule.take_count_per_step = 1;
        take_rule.step_stride_in_marker_count = 1;
        take_rule.maximum_number_of_markers = 1;
        marker_visual_set_take_rule(app, visual, take_rule);
        marker_visual_set_priority(app, visual, VisualPriority_Highest);
    }
    
    // NOTE(allen): Token highlight setup
    b32 do_token_highlight = false;
    if (do_token_highlight){
        int_color token_color = 0x5000EE00;
        
        u32 token_flags = BoundaryToken|BoundaryWhitespace;
        i32 pos0 = view.cursor.pos;
        i32 pos1 = buffer_boundary_seek(app, buffer.buffer_id, pos0, DirLeft , token_flags);
        if (pos1 >= 0){
            i32 pos2 = buffer_boundary_seek(app, buffer.buffer_id, pos1, DirRight, token_flags);
            if (pos2 <= buffer.size){
                Managed_Object token_highlight = alloc_buffer_markers_on_buffer(app, buffer.buffer_id, 2, &render_scope);
                Marker range_markers[2] = {};
                range_markers[0].pos = pos1;
                range_markers[1].pos = pos2;
                managed_object_store_data(app, token_highlight, 0, 2, range_markers);
                Marker_Visual visual = create_marker_visual(app, token_highlight);
                marker_visual_set_effect(app, visual, VisualType_CharacterHighlightRanges, token_color, Stag_At_Highlight, 0);
            }
        }
    }
    
    // NOTE(allen): Matching enclosure highlight setup
    static const i32 color_count = 4;
    if (do_matching_enclosure_highlight){
        int_color colors[color_count];
        for (u16 i = 0; i < color_count; i += 1){
            colors[i] = Stag_Back_Cycle_1 + i;
        }
        mark_enclosures(app, scratch, render_scope, &buffer, view.cursor.pos, FindScope_Brace, VisualType_LineHighlightRanges, colors, 0, color_count);
    }
    if (do_matching_paren_highlight){
        i32 pos = view.cursor.pos;
        if (buffer_get_char(app, buffer.buffer_id, pos) == '('){
            pos += 1;
        }
        else if (pos > 0){
            if (buffer_get_char(app, buffer.buffer_id, pos - 1) == ')'){
                pos -= 1;
            }
        }
        int_color colors[color_count];
        for (u16 i = 0; i < color_count; i += 1){
            colors[i] = Stag_Text_Cycle_1 + i;
        }
        mark_enclosures(app, scratch, render_scope, &buffer, pos, FindScope_Paren, VisualType_CharacterBlocks, 0, colors, color_count);
    }
    
    draw_render_layout(app, view_id);
    text_layout_free(app, text_layout_id);
    
    // NOTE(allen): FPS HUD
    if (show_fps_hud){
        static const i32 history_depth = 10;
        static f32 history_literal_dt[history_depth] = {};
        static f32 history_animation_dt[history_depth] = {};
        static i32 history_frame_index[history_depth] = {};
        
        i32 wrapped_index = frame_info.index%history_depth;
        history_literal_dt[wrapped_index]   = frame_info.literal_dt;
        history_animation_dt[wrapped_index] = frame_info.animation_dt;
        history_frame_index[wrapped_index]  = frame_info.index;
        
        Rect_f32 hud_rect = f32R(view.render_region);
        hud_rect.y0 = hud_rect.y1 - view.line_height*(f32)(history_depth);
        draw_rectangle(app, hud_rect, 0xFF000000);
        draw_rectangle_outline(app, hud_rect, 0xFFFFFFFF);
        
        Face_ID font_id = 0;
        get_face_id(app, view.buffer_id, &font_id);
        
        Vec2 p = hud_rect.p0;
        
        Range ranges[2];
        ranges[0].first = wrapped_index;
        ranges[0].one_past_last = -1;
        ranges[1].first = history_depth - 1;
        ranges[1].one_past_last = wrapped_index;
        for (i32 i = 0; i < 2; i += 1){
            Range r = ranges[i];
            for (i32 j = r.first; j > r.one_past_last; j -= 1, p.y += view.line_height){
                f32 dts[2];
                dts[0] = history_literal_dt[j];
                dts[1] = history_animation_dt[j];
                i32 frame_index = history_frame_index[j];
                
                char space[256];
                String str = make_fixed_width_string(space);
                
                Fancy_Color white = fancy_rgba(1.f, 1.f, 1.f, 1.f);
                Fancy_Color pink  = fancy_rgba(1.f, 0.f, 1.f, 1.f);
                Fancy_Color green = fancy_rgba(0.f, 1.f, 0.f, 1.f);
                Fancy_String_List list = {};
                push_fancy_stringf(arena, &list, pink , "FPS: ");
                push_fancy_stringf(arena, &list, green, "[");
                push_fancy_stringf(arena, &list, white, "%5d", frame_index);
                push_fancy_stringf(arena, &list, green, "]: ");
                
                for (i32 k = 0; k < 2; k += 1){
                    f32 dt = dts[k];
                    str.size = 0;
                    if (dt == 0.f){
                        push_fancy_stringf(arena, &list, white, "----------");
                    }
                    else{
                        push_fancy_stringf(arena, &list, white, "%10.6f", dt);
                    }
                    push_fancy_stringf(arena, &list, green, " | ");
                }
                
                draw_fancy_string(app, font_id, list.first, p, Stag_Default, 0, 0, V2(1.f, 0.f));
            }
        }
        
        animate_in_n_milliseconds(app, 1000);
    }
    
    end_temp_memory(major_temp);
    managed_scope_clear_self_all_dependent_scopes(app, render_scope);
}

static int_color
get_margin_color(i32 level){
    int_color margin = 0;
    switch (level){
        default:
        case UIActivation_None:
        {
            margin = Stag_List_Item;
        }break;
        case UIActivation_Hover:
        {
            margin = Stag_List_Item_Hover;
        }break;
        case UIActivation_Active:
        {
            margin = Stag_List_Item_Active;
        }break;
    }
    return(margin);
}

static void
default_ui_render_caller(Application_Links *app, View_ID view_id, Rect_f32 rect_f32, Face_ID face_id){
    UI_Data *ui_data = 0;
    Arena *ui_arena = 0;
    if (view_get_ui_data(app, view_id, ViewGetUIFlag_KeepDataAsIs, &ui_data, &ui_arena)){
        GUI_Scroll_Vars ui_scroll = {};
        view_get_scroll_vars(app, view_id, &ui_scroll);
        
        for (UI_Item *item = ui_data->list.first;
             item != 0;
             item = item->next){
            Rect_i32 item_rect_i32 = item->rect_outer;
            Rect_f32 item_rect = f32R(item_rect_i32);
            
            switch (item->coordinates){
                case UICoordinates_ViewSpace:
                {
                    item_rect.p0 -= ui_scroll.scroll_p;
                    item_rect.p1 -= ui_scroll.scroll_p;
                }break;
                case UICoordinates_PanelSpace:
                {}break;
            }
            
            if (rect_overlap(item_rect, rect_f32)){
                Rect_f32 inner_rect = get_inner_rect(item_rect, (f32)item->inner_margin);
                
                Face_Metrics metrics = {};
                get_face_metrics(app, face_id, &metrics);
                f32 line_height = metrics.line_height;
                f32 info_height = (f32)item->line_count*line_height;
                
                draw_rectangle(app, inner_rect, Stag_Back);
                Vec2 p = V2(inner_rect.x0 + 3.f, (f32)(round32((inner_rect.y0 + inner_rect.y1 - info_height)*0.5f)));
                for (i32 i = 0; i < item->line_count; i += 1){
                    draw_fancy_string(app, face_id, item->lines[i].first, p, Stag_Default, 0, 0, V2(1.f, 0));
                    p.y += line_height;
                }
                if (item->inner_margin > 0){
                    draw_margin(app, item_rect, inner_rect, get_margin_color(item->activation_level));
                }
            }
        }
    }
}
static void
default_ui_render_caller(Application_Links *app, View_ID view_id, Rect_f32 rect_f32){
    Buffer_ID buffer_id = 0;
    view_get_buffer(app, view_id, AccessAll, &buffer_id);
    Face_ID face_id = 0;
    get_face_id(app, buffer_id, &face_id);
    default_ui_render_caller(app, view_id, rect_f32, face_id);
}
static void
default_ui_render_caller(Application_Links *app, View_ID view_id, Face_ID face_id){
    View_Summary view = {};
    if (get_view_summary(app, view_id, AccessAll, &view)){
        Rect_f32 rect_f32 = f32R(view.render_region);
        default_ui_render_caller(app, view_id, rect_f32, face_id);
    }
}
static void
default_ui_render_caller(Application_Links *app, View_ID view_id){
    View_Summary view = {};
    if (get_view_summary(app, view_id, AccessAll, &view)){
        Rect_f32 rect_f32 = f32R(view.render_region);
        Buffer_ID buffer_id = 0;
        view_get_buffer(app, view_id, AccessAll, &buffer_id);
        Face_ID face_id = 0;
        get_face_id(app, buffer_id, &face_id);
        default_ui_render_caller(app, view_id, rect_f32, face_id);
    }
}

static void
default_render_view(Application_Links *app, Frame_Info frame_info, View_ID view_id, b32 is_active){
    Rect_i32 view_rect = {};
    view_get_region(app, view_id, &view_rect);
    Rect_i32 inner = get_inner_rect(view_rect, 3);
    draw_rectangle(app, f32R(view_rect), get_margin_color(is_active?UIActivation_Active:UIActivation_None));
    draw_rectangle(app, f32R(inner), Stag_Back);
    draw_clip_push(app, f32R(inner));
    draw_coordinate_center_push(app, V2(inner.p0));
    if (view_is_in_ui_mode(app, view_id)){
        default_ui_render_caller(app, view_id);
    }
    else{
        default_buffer_render_caller(app, frame_info, view_id, inner);
    }
    draw_clip_pop(app);
    draw_coordinate_center_pop(app);
}

RENDER_CALLER_SIG(default_render_caller){
    View_ID active_view_id = 0;
    get_active_view(app, AccessAll, &active_view_id);
    View_ID view_id = 0;
    for (get_view_next(app, 0, AccessAll, &view_id);
         view_id != 0;
         get_view_next(app, view_id, AccessAll, &view_id)){
        default_render_view(app, frame_info, view_id, (active_view_id == view_id));
    }
}

HOOK_SIG(default_exit){
    // If this returns zero it cancels the exit.
    if (allow_immediate_close_without_checking_for_changes){
        return(1);
    }
    
    b32 has_unsaved_changes = false;
    
    for (Buffer_Summary buffer = get_buffer_first(app, AccessAll);
         buffer.exists;
         get_buffer_next(app, &buffer, AccessAll)){
        if (HasFlag(buffer.dirty, DirtyState_UnsavedChanges)){
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

// TODO(allen): how to deal with multiple sizes on a single view
// TODO(allen): expected character advance.
HOOK_SIG(default_view_adjust){
    for (View_Summary view = get_view_first(app, AccessAll);
         view.exists;
         get_view_next(app, &view, AccessAll)){
        Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessAll);
        i32 view_width = view.render_region.x1 - view.render_region.x0;
        Face_ID face_id = get_default_font_for_view(app, view.view_id);
        f32 em = get_string_advance(app, face_id, make_lit_string("m"));
        
        f32 wrap_width = view_width - 2.0f*em;
        f32 min_width = 40.0f*em;
        if (wrap_width < min_width){
            wrap_width = min_width;
        }
        
        f32 min_base_width = 20.0f*em;
        buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, (i32)(wrap_width));
        buffer_set_setting(app, &buffer, BufferSetting_MinimumBaseWrapPosition, (i32)(min_base_width));
    }
    return(0);
}

BUFFER_NAME_RESOLVER_SIG(default_buffer_name_resolution){
    if (conflict_count > 1){
        // List of unresolved conflicts
        Partition *part = &global_part;
        Temp_Memory temp = begin_temp_memory(part);
        
        i32 *unresolved = push_array(part, i32, conflict_count);
        if (unresolved == 0) return;
        
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
                
                i32 len = conflict->base_name_len;
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
                        b32 past_the_end = false;
                        for (i32 j = 0; j < x; ++j){
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
                        
                        append(&uniqueifier, make_string(start, (i32)(end - start)));
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
            b32 has_conflicts = false;
            for (i32 i = 0; i < unresolved_count; ++i){
                i32 conflict_index = unresolved[i];
                Buffer_Name_Conflict_Entry *conflict = &conflicts[conflict_index];
                String conflict_name = make_string(conflict->unique_name_in_out,
                                                   conflict->unique_name_len_in_out);
                
                b32 hit_conflict = false;
                if (conflict->file_name != 0){
                    for (i32 j = 0; j < unresolved_count; ++j){
                        if (i == j) continue;
                        
                        i32 conflict_j_index = unresolved[j];
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
    
    b32 treat_as_code = false;
    b32 treat_as_todo = false;
    b32 lex_without_strings = false;
    
    CString_Array extensions = get_code_extensions(&global_config.code_exts);
    
    Parse_Context_ID parse_context_id = 0;
    
    if (buffer.file_name != 0 && buffer.size < (16 << 20)){
        String name = make_string(buffer.file_name, buffer.file_name_len);
        String ext = file_extension(name);
        for (i32 i = 0; i < extensions.count; ++i){
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
    
    i32 map_id = (treat_as_code)?((i32)default_code_map):((i32)mapid_file);
    i32 map_id_query = 0;
    
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
    b32 wrap_lines = true;
    b32 use_virtual_whitespace = false;
    b32 use_lexer = false;
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
    
    i32 is_virtual = 0;
    if (global_config.automatically_indent_text_on_save &&
        buffer_get_setting(app, &buffer, BufferSetting_VirtualWhitespace, &is_virtual)){ 
        if (is_virtual){
            buffer_auto_indent(app, &global_part, &buffer, 0, buffer.size, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
        }
    }
    
    // no meaning for return
    return(0);
}

FILE_EDIT_RANGE_SIG(default_file_edit_range){
#if 0
    Buffer_Summary buffer_summary = {};
    if (get_buffer_summary(app, buffer_id, AccessAll, &buffer_summary)){
        if (!match(make_string(buffer_summary.buffer_name, buffer_summary.buffer_name_len), make_lit_string("*messages*"))){
            char space[1024];
            String str = make_fixed_width_string(space);
            append(&str, "'");
            append(&str, make_string(buffer_summary.buffer_name, buffer_summary.buffer_name_len));
            append(&str, "' [");
            append_int_to_str(&str, range.first);
            append(&str, ", ");
            append_int_to_str(&str, range.one_past_last);
            append(&str, ") '");
            append(&str, substr(text, 0, 32));
            append(&str, "'\n");
            print_message(app, str.str, str.size);
        }
    }
#endif
    
    // no meaning for return
    return(0);
}

FILE_EDIT_FINISHED_SIG(default_file_edit_finished){
    for (i32 i = 0; i < buffer_id_count; i += 1){
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
        memset(mouse, 0, sizeof(*mouse));
        mouse->p.x = -100;
        mouse->p.y = -100;
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

Vec2 scroll_velocity_[16] = {};
Vec2 *scroll_velocity = scroll_velocity_ - 1;

static i32
smooth_camera_step(f32 target, f32 *current, f32 *vel, f32 S, f32 T){
    i32 result = 0;
    f32 curr = *current;
    f32 v = *vel;
    if (curr != target){
        if (curr > target - .1f && curr < target + .1f){
            curr = target;
            v = 1.f;
        }
        else{
            f32 L = curr + T*(target - curr);
            
            i32 sign = (target > curr) - (target < curr);
            f32 V = curr + sign*v;
            
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
    Vec2 *velocity = scroll_velocity + view_id;
    i32 result = 0;
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
    set_hook(context, hook_buffer_viewer_update, default_view_adjust);
    
    set_start_hook(context, default_start);
    set_open_file_hook(context, default_file_settings);
    set_new_file_hook(context, default_new_file);
    set_save_file_hook(context, default_file_save);
    set_file_edit_range_hook(context, default_file_edit_range);
    set_file_edit_finished_hook(context, default_file_edit_finished);
    set_file_edit_range_hook(context, default_file_edit_range);
    
    set_end_file_hook(context, end_file_close_jump_list);
    
    set_command_caller(context, default_command_caller);
    set_render_caller(context, default_render_caller);
    set_input_filter(context, default_suppress_mouse_filter);
    set_scroll_rule(context, smooth_scroll_rule);
    set_buffer_name_resolver(context, default_buffer_name_resolution);
    set_modify_color_table_hook(context, default_modify_color_table);
    set_get_view_buffer_region_hook(context, default_view_buffer_region);
}

// BOTTOM

