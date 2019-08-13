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
    {string_u8_litexpr("mac-default")    , set_bindings_mac_default    },
    {string_u8_litexpr("choose")         , set_bindings_choose         },
    {string_u8_litexpr("default")        , set_bindings_default        },
};

START_HOOK_SIG(default_start){
    named_maps = named_maps_values;
    named_map_count = ArrayCount(named_maps_values);
    
    default_4coder_initialize(app, files, file_count);
    default_4coder_side_by_side_panels(app, files, file_count);
    
#if 0
    default_4coder_one_panel(app, files, file_count);
    
    Panel_ID left = 0;
    Panel_ID right = 0;
    Panel_ID bottom = 0;
    Panel_ID header = 0;
    
    View_ID left_view = get_active_view(app, AccessAll);
    
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
    
    
    Face_ID face_id = 0;
    get_face_id(app, 0, &face_id);
    Face_Metrics metrics = {};
    get_face_metrics(app, face_id, &metrics);
    f32 line_height = metrics.line_height;
    panel_set_split(app, h_split_main , PanelSplitKind_FixedPixels_BR, line_height*6.f + margin_vertical_pixels);
    panel_set_split(app, h_split_minor, PanelSplitKind_FixedPixels_TL, line_height     + header_vertical_pixels);
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
    View_ID view = get_active_view(app, AccessAll);
    Managed_Scope scope = view_get_managed_scope(app, view);
    managed_variable_set(app, scope, view_next_rewrite_loc, 0);
    if (fcoder_mode == FCoderMode_NotepadLike){
        for (View_ID view_it = get_view_next(app, 0, AccessAll);
             view_it != 0;
             view_it = get_view_next(app, view_it, AccessAll)){
            Managed_Scope scope_it = view_get_managed_scope(app, view_it);
            managed_variable_set(app, scope_it, view_snap_mark_to_cursor, true);
        }
    }
    
    cmd.command(app);
    
    u64 next_rewrite = 0;
    managed_variable_get(app, scope, view_next_rewrite_loc, &next_rewrite);
    managed_variable_set(app, scope, view_rewrite_loc, next_rewrite);
    if (fcoder_mode == FCoderMode_NotepadLike){
        for (View_ID view_it = get_view_next(app, 0, AccessAll);
             view_it != 0;
             view_it = get_view_next(app, view_it, AccessAll)){
            Managed_Scope scope_it = view_get_managed_scope(app, view_it);
            u64 val = 0;
            if (managed_variable_get(app, scope_it, view_snap_mark_to_cursor, &val)){
                if (val != 0){
                    i64 pos = view_get_cursor_pos(app, view_it);
                    view_set_mark(app, view_it, seek_pos(pos));
                }
            }
        }
    }
    
    return(0);
}

struct Highlight_Record{
    Highlight_Record *next;
    Range_i64 range;
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

static Range_i64_Array
get_enclosure_ranges(Application_Links *app, Arena *arena, Buffer_ID buffer, i64 pos, u32 flags){
    Range_i64_Array array = {};
    i32 max = 100;
    array.ranges = push_array(arena, Range_i64, max);
    for (;;){
        Range_i64 range = {};
        if (find_scope_range(app, buffer, pos, &range, flags)){
            array.ranges[array.count] = range;
            array.count += 1;
            pos = range.first;
            if (array.count >= max){
                break;
            }
        }
        else{
            break;
        }
    }
    return(array);
}

typedef i32 Range_Highlight_Kind;
enum{
    RangeHighlightKind_LineHighlight,
    RangeHighlightKind_CharacterHighlight,
};

static void
draw_enclosures(Application_Links *app, Text_Layout_ID text_layout_id, Buffer_ID buffer,
                i64 pos, u32 flags, Range_Highlight_Kind kind,
                int_color *back_colors, int_color *fore_colors, i32 color_count){
    Scratch_Block scratch(app);
    Range_i64_Array ranges = get_enclosure_ranges(app, scratch, buffer, pos, flags);
    
    i32 color_index = 0;
    for (i32 i = ranges.count - 1; i >= 0; i -= 1){
        Range_i64 range = ranges.ranges[i];
        if (kind == RangeHighlightKind_LineHighlight){
            Range_i64 r[2] = {};
            if (i > 0){
                Range_i64 inner_range = ranges.ranges[i - 1];
                Range_i64 lines = get_line_range_from_pos_range(app, buffer, range);
                Range_i64 inner_lines = get_line_range_from_pos_range(app, buffer, inner_range);
                inner_lines.min = clamp_bot(lines.min, inner_lines.min);
                inner_lines.max = clamp_top(inner_lines.max, lines.max);
                inner_lines.min -= 1;
                inner_lines.max += 1;
                if (lines.min <= inner_lines.min){
                    r[0] = Ii64(lines.min, inner_lines.min);
                }
                if (inner_lines.max <= lines.max){
                    r[1] = Ii64(inner_lines.max, lines.max);
                }
            }
            else{
                r[0] = get_line_range_from_pos_range(app, buffer, range);
            }
            for (i32 j = 0; j < 2; j += 1){
                if (r[j].min == 0){
                    continue;
                }
                Range_i64 line_range = r[j];
                if (back_colors != 0){
                    draw_line_highlight(app, text_layout_id, line_range, back_colors[color_index]);
                }
                if (fore_colors != 0){
                    Range_i64 pos_range = get_pos_range_from_line_range(app, buffer, line_range);
                    paint_text_color(app, text_layout_id, pos_range, fore_colors[color_index]);
                }
            }
        }
        else{
            if (back_colors != 0){
                draw_character_block(app, text_layout_id, range.min, back_colors[color_index]);
                draw_character_block(app, text_layout_id, range.max - 1, back_colors[color_index]);
            }
            if (fore_colors != 0){
                paint_text_color(app, text_layout_id, range.min, fore_colors[color_index]);
                paint_text_color(app, text_layout_id, range.max - 1, fore_colors[color_index]);
            }
        }
        color_index += 1;
        color_index = (color_index%color_count);
    }
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
    Buffer_ID buffer = view_get_buffer(app, view_id, AccessAll);
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    i32 line_height = ceil32(metrics.line_height);
    
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
        i32 line_count = (i32)buffer_get_line_count(app, buffer);
        i32 line_count_digit_count = (i32)digit_count_from_integer(line_count, 10);
        i32 margin_width = ceil32((f32)line_count_digit_count*metrics.typical_character_width);
        sub_region.x0 += margin_width + 2;
    }
    
    return(sub_region);
}

static Buffer_Point
buffer_position_from_scroll_position(Application_Links *app, View_ID view_id, Vec2 scroll){
    Full_Cursor cursor = view_compute_cursor(app, view_id, seek_wrapped_xy(0.f, scroll.y, false));
    cursor = view_compute_cursor(app, view_id, seek_line_char(cursor.line, 1));
    Buffer_Point result = {};
    result.line_number = cursor.line;
    result.pixel_shift.x = scroll.x;
    result.pixel_shift.y = scroll.y - cursor.wrapped_y;
    return(result);
}

static i64
abs_position_from_buffer_point(Application_Links *app, View_ID view_id, Buffer_Point buffer_point){
    Full_Cursor cursor = view_compute_cursor(app, view_id, seek_line_char(buffer_point.line_number, 0));
    Buffer_Seek seek = seek_wrapped_xy(buffer_point.pixel_shift.x,
                                       buffer_point.pixel_shift.y + cursor.wrapped_y, false);
    cursor = view_compute_cursor(app, view_id, seek);
    return(cursor.pos);
}

static void
default_buffer_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id, Rect_f32 view_inner_rect){
    Buffer_ID buffer = view_get_buffer(app, view_id, AccessAll);
    
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics face_metrics = get_face_metrics(app, face_id);
    
    f32 line_height = face_metrics.line_height;
    
    Rect_f32 sub_region = Rf32(V2(0, 0), rect_dim(view_inner_rect));
    sub_region = default_view_buffer_region(app, view_id, sub_region);
    Rect_f32 buffer_rect = Rf32(V2(view_inner_rect.p0 + sub_region.p0),
                                V2(view_inner_rect.p0 + sub_region.p1));
    buffer_rect = rect_intersect(buffer_rect, view_inner_rect);
    
    GUI_Scroll_Vars scroll = view_get_scroll_vars(app, view_id);
    
    Buffer_Point buffer_point = buffer_position_from_scroll_position(app, view_id, scroll.scroll_p);
    Text_Layout_ID text_layout_id = compute_render_layout(app, view_id, buffer, buffer_rect.p0, rect_dim(buffer_rect), buffer_point, max_i32);
    Range_i64 on_screen_range = text_layout_get_on_screen_range(app, text_layout_id);
    
    View_ID active_view = get_active_view(app, AccessAll);
    b32 is_active_view = (active_view == view_id);
    
    Scratch_Block scratch(app);
    
    {
        Rect_f32 r_cursor = view_get_screen_rect(app, view_id);
        r_cursor.p1 -= r_cursor.p0;
        r_cursor.p0 = V2(0.f,0.f);
        
        // NOTE(allen): Filebar
        {
            b32 showing_file_bar = false;
            if (view_get_setting(app, view_id, ViewSetting_ShowFileBar, &showing_file_bar)){
                if (showing_file_bar){
                    Rect_f32 bar = r_cursor;
                    bar.y1 = bar.y0 + line_height + 2.f;
                    r_cursor.y0 = bar.y1;
                    
                    draw_rectangle(app, bar, Stag_Bar);
                    
                    Fancy_Color base_color = fancy_id(Stag_Base);
                    Fancy_Color pop2_color = fancy_id(Stag_Pop2);
                    
                    Temp_Memory temp = begin_temp(scratch);
                    
                    i64 cursor_position = view_get_cursor_pos(app, view_id);
                    Full_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(cursor_position));
                    
                    Fancy_String_List list = {};
                    String_Const_u8 unique_name = push_buffer_unique_name(app, scratch, buffer);
                    push_fancy_string(scratch, &list, base_color, unique_name);
                    push_fancy_stringf(scratch, &list, base_color, " - Row: %3.lld Col: %3.lld -", cursor.line, cursor.character);
                    
                    b32 is_dos_mode = false;
                    if (buffer_get_setting(app, buffer, BufferSetting_Eol, &is_dos_mode)){
                        if (is_dos_mode){
                            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" dos"));
                        }
                        else{
                            push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" nix"));
                        }
                    }
                    else{
                        push_fancy_string(scratch, &list, base_color, string_u8_litexpr(" ???"));
                    }
                    
                    {
                        Dirty_State dirty = buffer_get_dirty_state(app, buffer);
                        u8 space[3];
                        String_u8 str = Su8(space, 0, 3);
                        if (dirty != 0){
                            string_append(&str, string_u8_litexpr(" "));
                        }
                        if (HasFlag(dirty, DirtyState_UnsavedChanges)){
                            string_append(&str, string_u8_litexpr("*"));
                        }
                        if (HasFlag(dirty, DirtyState_UnloadedChanges)){
                            string_append(&str, string_u8_litexpr("!"));
                        }
                        push_fancy_string(scratch, &list, pop2_color, str.string);
                    }
                    
                    Vec2 p = bar.p0 + V2(0.f, 2.f);
                    draw_fancy_string(app, face_id, list.first, p, Stag_Default, 0);
                    
                    end_temp(temp);
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
                    
                    Temp_Memory temp = begin_temp(scratch);
                    Fancy_String_List list = {};
                    
                    Fancy_Color default_color = fancy_id(Stag_Default);
                    Fancy_Color pop1_color = fancy_id(Stag_Pop1);
                    
                    push_fancy_string(scratch, &list, pop1_color   , query_bar->prompt);
                    push_fancy_string(scratch, &list, default_color, query_bar->string);
                    
                    Vec2 p = bar.p0 + V2(0.f, 2.f);
                    draw_fancy_string(app, face_id, list.first, p, Stag_Default, 0);
                    
                    end_temp(temp);
                }
            }
        }
        
        // NOTE(allen): Line Numbers
        if (global_config.show_line_number_margins){
            i32 line_count = (i32)buffer_get_line_count(app, buffer);
            i32 line_count_digit_count = (i32)digit_count_from_integer(line_count, 10);
            // TODO(allen): I need a "digit width"
            f32 zero = get_string_advance(app, face_id, string_u8_litexpr("0"));
            f32 margin_width = (f32)line_count_digit_count*zero;
            
            Rect_f32 left_margin = r_cursor;
            left_margin.x1 = left_margin.x0 + margin_width + 2;
            r_cursor.x0 = left_margin.x1;
            
            draw_rectangle(app, left_margin, Stag_Line_Numbers_Back);
            
            Rect_f32 clip_region = left_margin;
            clip_region.p0 += view_inner_rect.p0;
            clip_region.p1 += view_inner_rect.p0;
            draw_clip_push(app, clip_region);
            
            Fancy_Color line_color = fancy_id(Stag_Line_Numbers_Text);
            
            Full_Cursor cursor = view_compute_cursor(app, view_id, seek_pos(on_screen_range.first));
            GUI_Scroll_Vars scroll_vars = view_get_scroll_vars(app, view_id);
            for (;cursor.pos <= on_screen_range.one_past_last;){
                Vec2 p = panel_space_from_view_space(cursor.wrapped_p, scroll_vars.scroll_p);
                p.x = left_margin.x0;
                Temp_Memory temp = begin_temp(scratch);
                Fancy_String *line_string = push_fancy_stringf(scratch, line_color, "%*lld", line_count_digit_count, cursor.line);
                draw_fancy_string(app, face_id, line_string, p, Stag_Margin_Active, 0);
                end_temp(temp);
                i64 next_line = cursor.line + 1;
                cursor = view_compute_cursor(app, view_id, seek_line_char(next_line, 1));
                if (cursor.line < next_line){
                    break;
                }
            }
            
            draw_clip_pop(app);
        }
    }
    
    // NOTE(allen): Scan for TODOs and NOTEs
    {
        Temp_Memory temp = begin_temp(scratch);
        String_Const_u8 tail = push_buffer_range(app, scratch, buffer, on_screen_range);
        
        Highlight_Record *record_first = 0;
        Highlight_Record *record_last = 0;
        i32 record_count = 0;
        i32 index = 0;
        
        for (;tail.size > 0; tail = string_skip(tail, 1), index += 1){
            if (string_match(string_prefix(tail, 4), string_u8_litexpr("NOTE"))){
                Highlight_Record *record = push_array(scratch, Highlight_Record, 1);
                sll_queue_push(record_first, record_last, record);
                record_count += 1;
                record->range.first = on_screen_range.first + index;
                record->range.one_past_last = record->range.first + 4;
                record->color = Stag_Text_Cycle_2;
                tail = string_skip(tail, 3);
                index += 3;
            }
            else if (string_match(string_prefix(tail, 4), string_u8_litexpr("TODO"))){
                Highlight_Record *record = push_array(scratch, Highlight_Record, 1);
                sll_queue_push(record_first, record_last, record);
                record_count += 1;
                record->range.first = on_screen_range.first + index;
                record->range.one_past_last = record->range.first + 4;
                record->color = Stag_Text_Cycle_1;
                tail = string_skip(tail, 3);
                index += 3;
            }
        }
        
        for (Highlight_Record *node = record_first;
             node != 0;
             node = node->next){
            paint_text_color(app, text_layout_id, node->range, node->color);
        }
        
        end_temp(temp);
    }
    
    i64 cursor_pos = view_get_cursor_pos(app, view_id);
    i64 mark_pos = view_get_mark_pos(app, view_id);
    
    // NOTE(allen): Scope highlight
    if (do_matching_enclosure_highlight){
        static const i32 color_count = 4;
        int_color colors[color_count];
        for (u16 i = 0; i < color_count; i += 1){
            colors[i] = Stag_Back_Cycle_1 + i;
        }
        draw_enclosures(app, text_layout_id, buffer,
                        cursor_pos, FindScope_Brace, RangeHighlightKind_LineHighlight,
                        colors, 0, color_count);
    }
    
    // NOTE(allen): Error highlight
    {
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, AccessAll);
        if (compilation_buffer != 0){
            Managed_Scope scopes[2];
            scopes[0] = buffer_get_managed_scope(app, compilation_buffer);
            scopes[1] = buffer_get_managed_scope(app, buffer);
            Managed_Scope scope = get_managed_scope_with_multiple_dependencies(app, scopes, ArrayCount(scopes));
            Managed_Object markers_object = 0;
            if (managed_variable_get(app, scope, sticky_jump_marker_handle, &markers_object)){
                Temp_Memory temp = begin_temp(scratch);
                i32 count = managed_object_get_item_count(app, markers_object);
                Marker *markers = push_array(scratch, Marker, count);
                managed_object_load_data(app, markers_object, 0, count, markers);
                for (i32 i = 0; i < count; i += 1){
                    i64 line_number = get_line_number_from_pos(app, buffer, markers[i].pos);
                    draw_line_highlight(app, text_layout_id, line_number,
                                        Stag_Highlight_Junk);
                }
                end_temp(temp);
            }
        }
    }
    
    // NOTE(allen): Color parens
    if (do_matching_paren_highlight){
        i64 pos = cursor_pos;
        if (buffer_get_char(app, buffer, pos) == '('){
            pos += 1;
        }
        else if (pos > 0){
            if (buffer_get_char(app, buffer, pos - 1) == ')'){
                pos -= 1;
            }
        }
        static const i32 color_count = 4;
        int_color colors[color_count];
        for (u16 i = 0; i < color_count; i += 1){
            colors[i] = Stag_Text_Cycle_1 + i;
        }
        draw_enclosures(app, text_layout_id, buffer,
                        cursor_pos, FindScope_Paren, RangeHighlightKind_CharacterHighlight,
                        0, colors, color_count);
    }
    
    // NOTE(allen): Line highlight
    if (highlight_line_at_cursor && is_active_view){
        i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
        draw_line_highlight(app, text_layout_id, line_number, Stag_Highlight_Cursor_Line);
    }
    
    // NOTE(allen): Highlight range
    b32 has_highlight_range = false;
    {
        Managed_Scope scope = view_get_managed_scope(app, view_id);
        u64 highlight_buffer = 0;
        managed_variable_get(app, scope, view_highlight_buffer, &highlight_buffer);
        if (highlight_buffer != 0){
            if ((Buffer_ID)highlight_buffer != buffer){
                view_disable_highlight_range(app, view_id);
            }
            else{
                has_highlight_range = true;
                Managed_Object highlight = 0;
                managed_variable_get(app, scope, view_highlight_range, &highlight);
                Marker marker_range[2];
                if (managed_object_load_data(app, highlight, 0, 2, marker_range)){
                    Range_i64 range = Ii64(marker_range[0].pos,
                                           marker_range[1].pos);
                    draw_character_block(app, text_layout_id, range, Stag_Highlight);
                    paint_text_color(app, text_layout_id, range, Stag_At_Highlight);
                }
            }
        }
    }
    
    // NOTE(allen): Cursor and mark
    b32 cursor_is_hidden_in_this_view = (cursor_is_hidden && is_active_view);
    if (!cursor_is_hidden_in_this_view){
        switch (fcoder_mode){
            case FCoderMode_Original:
            {
                if (is_active_view){
                    draw_character_block(app, text_layout_id, cursor_pos, Stag_Cursor);
                    paint_text_color(app, text_layout_id, cursor_pos, Stag_At_Cursor);
                    draw_character_wire_frame(app, text_layout_id, mark_pos, Stag_Mark);
                }
                else{
                    draw_character_wire_frame(app, text_layout_id, mark_pos, Stag_Mark);
                    draw_character_wire_frame(app, text_layout_id, cursor_pos, Stag_Cursor);
                }
            }break;
            
            case FCoderMode_NotepadLike:
            {
                if (cursor_pos != mark_pos){
                    Range_i64 range = Ii64(cursor_pos, mark_pos);
                    draw_character_block(app, text_layout_id, range, Stag_Highlight);
                    paint_text_color(app, text_layout_id, range, Stag_At_Highlight);
                }
                draw_character_i_bar(app, text_layout_id, cursor_pos, Stag_Cursor);
            }break;
        }
    }
    
    draw_clip_push(app, buffer_rect);
    draw_render_layout(app, view_id);
    text_layout_free(app, text_layout_id);
    draw_clip_pop(app);
    
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
        
        Rect_f32 hud_rect = view_get_screen_rect(app, view_id);
        hud_rect.p1 -= hud_rect.p0;
        hud_rect.p0 = V2(0.f, 0.f);
        hud_rect.y0 = hud_rect.y1 - line_height*(f32)(history_depth);
        draw_rectangle(app, hud_rect, 0xFF000000);
        draw_rectangle_outline(app, hud_rect, 0xFFFFFFFF);
        
        Vec2 p = hud_rect.p0;
        
        Range ranges[2];
        ranges[0].first = wrapped_index;
        ranges[0].one_past_last = -1;
        ranges[1].first = history_depth - 1;
        ranges[1].one_past_last = wrapped_index;
        for (i32 i = 0; i < 2; i += 1){
            Range r = ranges[i];
            for (i32 j = r.first; j > r.one_past_last; j -= 1, p.y += line_height){
                f32 dts[2];
                dts[0] = history_literal_dt[j];
                dts[1] = history_animation_dt[j];
                i32 frame_index = history_frame_index[j];
                
                Fancy_Color white = fancy_rgba(1.f, 1.f, 1.f, 1.f);
                Fancy_Color pink  = fancy_rgba(1.f, 0.f, 1.f, 1.f);
                Fancy_Color green = fancy_rgba(0.f, 1.f, 0.f, 1.f);
                Fancy_String_List list = {};
                push_fancy_stringf(scratch, &list, pink , "FPS: ");
                push_fancy_stringf(scratch, &list, green, "[");
                push_fancy_stringf(scratch, &list, white, "%5d", frame_index);
                push_fancy_stringf(scratch, &list, green, "]: ");
                
                for (i32 k = 0; k < 2; k += 1){
                    f32 dt = dts[k];
                    if (dt == 0.f){
                        push_fancy_stringf(scratch, &list, white, "----------");
                    }
                    else{
                        push_fancy_stringf(scratch, &list, white, "%10.6f", dt);
                    }
                    push_fancy_stringf(scratch, &list, green, " | ");
                }
                
                draw_fancy_string(app, face_id, list.first, p, Stag_Default, 0, 0, V2(1.f, 0.f));
            }
        }
        
        animate_in_n_milliseconds(app, 1000);
    }
    
    //managed_scope_clear_self_all_dependent_scopes(app, render_scope);
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
        GUI_Scroll_Vars ui_scroll = view_get_scroll_vars(app, view_id);
        
        for (UI_Item *item = ui_data->list.first;
             item != 0;
             item = item->next){
            Rect_f32 item_rect = f32R(item->rect_outer);
            
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
                Rect_f32 inner = rect_inner(item_rect, (f32)item->inner_margin);
                
                Face_Metrics metrics = get_face_metrics(app, face_id);
                f32 line_height = metrics.line_height;
                f32 info_height = (f32)item->line_count*line_height;
                
                draw_rectangle(app, inner, Stag_Back);
                Vec2 p = V2(inner.x0 + 3.f, (f32)(round32((inner.y0 + inner.y1 - info_height)*0.5f)));
                for (i32 i = 0; i < item->line_count; i += 1){
                    draw_fancy_string(app, face_id, item->lines[i].first, p, Stag_Default, 0, 0, V2(1.f, 0));
                    p.y += line_height;
                }
                if (item->inner_margin > 0){
                    draw_margin(app, item_rect, inner, get_margin_color(item->activation_level));
                }
            }
        }
    }
}
static void
default_ui_render_caller(Application_Links *app, View_ID view_id, Rect_f32 rect_f32){
    Buffer_ID buffer = view_get_buffer(app, view_id, AccessAll);
    Face_ID face_id = get_face_id(app, buffer);
    default_ui_render_caller(app, view_id, rect_f32, face_id);
}
static void
default_ui_render_caller(Application_Links *app, View_ID view_id, Face_ID face_id){
    Rect_f32 rect = view_get_screen_rect(app, view_id);
    rect.p1 -= rect.p0;
    rect.p0 = V2(0.f,0.f);
    default_ui_render_caller(app, view_id, rect, face_id);
}
static void
default_ui_render_caller(Application_Links *app, View_ID view){
    Rect_f32 rect = view_get_screen_rect(app, view);
    rect.p1 -= rect.p0;
    rect.p0 = V2(0.f,0.f);
    Buffer_ID buffer = view_get_buffer(app, view, AccessAll);
    Face_ID face_id = get_face_id(app, buffer);
    default_ui_render_caller(app, view, rect, face_id);
}

static void
default_render_view(Application_Links *app, Frame_Info frame_info, View_ID view, b32 is_active){
    Rect_f32 view_rect = view_get_screen_rect(app, view);
    Rect_f32 inner = rect_inner(view_rect, 3);
    draw_rectangle(app, view_rect, get_margin_color(is_active?UIActivation_Active:UIActivation_None));
    draw_rectangle(app, inner, Stag_Back);
    draw_clip_push(app, inner);
    draw_coordinate_center_push(app, inner.p0);
    if (view_is_in_ui_mode(app, view)){
        default_ui_render_caller(app, view);
    }
    else{
        default_buffer_render_caller(app, frame_info, view, inner);
    }
    draw_clip_pop(app);
    draw_coordinate_center_pop(app);
}

RENDER_CALLER_SIG(default_render_caller){
    View_ID active_view_id = get_active_view(app, AccessAll);
    for (View_ID view_id = get_view_next(app, 0, AccessAll);
         view_id != 0;
         view_id = get_view_next(app, view_id, AccessAll)){
        default_render_view(app, frame_info, view_id, (active_view_id == view_id));
    }
}

HOOK_SIG(default_exit){
    // If this returns zero it cancels the exit.
    
    i32 result = 1;
    
    if (!allow_immediate_close_without_checking_for_changes){
        b32 has_unsaved_changes = false;
        for (Buffer_ID buffer = get_buffer_next(app, 0, AccessAll);
             buffer != 0;
             buffer = get_buffer_next(app, buffer, AccessAll)){
            Dirty_State dirty = buffer_get_dirty_state(app, buffer);
            if (HasFlag(dirty, DirtyState_UnsavedChanges)){
                has_unsaved_changes = true;
                break;
            }
        }
        if (has_unsaved_changes){
            View_ID view = get_active_view(app, AccessAll);
            do_gui_sure_to_close_4coder(app, view);
            result = 0;
        }
    }
    
    return(result);
}

// TODO(allen): how to deal with multiple sizes on a single view
// TODO(allen): expected character advance.
HOOK_SIG(default_view_adjust){
    for (View_ID view = get_view_next(app, 0, AccessAll);
         view != 0;
         view = get_view_next(app, view, AccessAll)){
        Buffer_ID buffer = view_get_buffer(app, view, AccessAll);
        
        Rect_f32 screen_rect = view_get_screen_rect(app, view);
        f32 view_width = rect_width(screen_rect);
        
        Face_ID face_id = get_face_id(app, buffer);
        f32 em = get_string_advance(app, face_id, string_u8_litexpr("m"));
        
        f32 wrap_width = view_width - 2.0f*em;
        f32 min_width = 40.0f*em;
        if (wrap_width < min_width){
            wrap_width = min_width;
        }
        
        f32 min_base_width = 20.0f*em;
        buffer_set_setting(app, buffer, BufferSetting_WrapPosition, (i32)(wrap_width));
        buffer_set_setting(app, buffer, BufferSetting_MinimumBaseWrapPosition, (i32)(min_base_width));
    }
    return(0);
}

BUFFER_NAME_RESOLVER_SIG(default_buffer_name_resolution){
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
                memcpy(conflict->unique_name_in_out, conflict->base_name.str, size);
                
                if (conflict->file_name.str != 0){
                    Scratch_Block per_file_closer(scratch);
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

OPEN_FILE_HOOK_SIG(default_file_settings){
    b32 treat_as_code = false;
    b32 treat_as_todo = false;
    b32 lex_without_strings = false;
    
    String_Const_u8_Array extensions = global_config.code_exts;
    
    Parse_Context_ID parse_context_id = 0;
    
    Arena *scratch = context_get_arena(app);
    Temp_Memory temp = begin_temp(scratch);
    
    String_Const_u8 file_name = push_buffer_file_name(app, scratch, buffer_id);
    i32 buffer_size = (i32)buffer_get_size(app, buffer_id);
    
    if (file_name.size > 0 && buffer_size < MB(32)){
        String_Const_u8 ext = string_file_extension(file_name);
        for (i32 i = 0; i < extensions.count; ++i){
            if (string_match(ext, extensions.strings[i])){
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
                    lex_without_strings = true;
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
                
                break;
            }
        }
        
        if (!treat_as_code){
            treat_as_todo = string_match_insensitive(string_front_of_path(file_name),
                                                     string_u8_litexpr("todo.txt"));
        }
    }
    
    i32 map_id = (treat_as_code)?((i32)default_code_map):((i32)mapid_file);
    i32 map_id_query = 0;
    
    buffer_set_setting(app, buffer_id, BufferSetting_MapID, default_lister_ui_map);
    buffer_get_setting(app, buffer_id, BufferSetting_MapID, &map_id_query);
    Assert(map_id_query == default_lister_ui_map);
    
    buffer_set_setting(app, buffer_id, BufferSetting_WrapPosition, global_config.default_wrap_width);
    buffer_set_setting(app, buffer_id, BufferSetting_MinimumBaseWrapPosition, global_config.default_min_base_width);
    buffer_set_setting(app, buffer_id, BufferSetting_MapID, map_id);
    buffer_get_setting(app, buffer_id, BufferSetting_MapID, &map_id_query);
    Assert(map_id_query == map_id);
    buffer_set_setting(app, buffer_id, BufferSetting_ParserContext, parse_context_id);
    
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
    
    String_Const_u8 buffer_name = push_buffer_base_name(app, scratch, buffer_id);
    if (string_match(buffer_name, string_u8_litexpr("*compilation*"))){
        wrap_lines = false;
    }
    if (buffer_size >= (1 << 20)){
        wrap_lines = false;
        use_virtual_whitespace = false;
    }
    
    // NOTE(allen|a4.0.12): There is a little bit of grossness going on here.
    // If we set BufferSetting_Lex to true, it will launch a lexing job.
    // If a lexing job is active when we set BufferSetting_VirtualWhitespace, the call can fail.
    // Unfortunantely without tokens virtual whitespace doesn't really make sense.
    // So for now I have it automatically turning on lexing when virtual whitespace is turned on.
    // Cleaning some of that up is a goal for future versions.
    buffer_set_setting(app, buffer_id, BufferSetting_LexWithoutStrings, lex_without_strings);
    buffer_set_setting(app, buffer_id, BufferSetting_WrapLine, wrap_lines);
    buffer_set_setting(app, buffer_id, BufferSetting_VirtualWhitespace, use_virtual_whitespace);
    buffer_set_setting(app, buffer_id, BufferSetting_Lex, use_lexer);
    
    end_temp(temp);
    
    // no meaning for return
    return(0);
}

OPEN_FILE_HOOK_SIG(default_new_file){
    // no meaning for return
    // buffer_id
    return(0);
}

OPEN_FILE_HOOK_SIG(default_file_save){
    b32 is_virtual = false;
    if (global_config.automatically_indent_text_on_save &&
        buffer_get_setting(app, buffer_id, BufferSetting_VirtualWhitespace, &is_virtual)){ 
        if (is_virtual){
            i32 buffer_size = (i32)buffer_get_size(app, buffer_id);
            buffer_auto_indent(app, buffer_id, 0, buffer_size, DEF_TAB_WIDTH, DEFAULT_INDENT_FLAGS | AutoIndent_FullTokens);
        }
    }
    // no meaning for return
    return(0);
}

FILE_EDIT_RANGE_SIG(default_file_edit_range){
    // no meaning for return
    // buffer_id, range, text
    return(0);
}

FILE_EDIT_FINISHED_SIG(default_file_edit_finished){
#if 0
    for (i32 i = 0; i < buffer_id_count; i += 1){
        // buffer_ids[i]
    }
#endif
    
    // no meaning for return
    return(0);
}

FILE_EXTERNALLY_MODIFIED_SIG(default_file_externally_modified){
    Scratch_Block scratch(app);
    String_Const_u8 name = push_buffer_unique_name(app, scratch, buffer_id);
    String_Const_u8 str = push_u8_stringf(scratch, "Modified externally: %s\n", name.str);
    print_message(app, str);
    // no meaning for return
    return(0);
}

OPEN_FILE_HOOK_SIG(default_end_file){
    Scratch_Block scratch(app);
    String_Const_u8 buffer_name = push_buffer_unique_name(app, scratch, buffer_id);
    String_Const_u8 str = push_u8_stringf(scratch, "Ending file: %s\n", buffer_name.str);
    print_message(app, str);
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

// TODO(allen): FIX FIX FIX FIX
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
// This id DOES correspond to the views that View_ _Summary contains.
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
    set_file_externally_modified_hook(context, default_file_externally_modified);
    
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

