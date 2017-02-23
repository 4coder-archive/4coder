/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * File editing view for 4coder
 *
 */

// TOP

internal i32
get_or_add_map_index(Models *models, i32 mapid){
    i32 result = 0;
    i32 user_map_count = models->user_map_count;
    i32 *map_id_table = models->map_id_table;
    for (result = 0; result < user_map_count; ++result){
        if (map_id_table[result] == mapid){
            break;
        }
        if (map_id_table[result] == -1){
            map_id_table[result] = mapid;
            break;
        }
    }
    return(result);
}

internal i32
get_map_index(Models *models, i32 mapid){
    i32 result = 0;
    i32 user_map_count = models->user_map_count;
    i32 *map_id_table = models->map_id_table;
    for (result = 0; result < user_map_count; ++result){
        if (map_id_table[result] == mapid){
            break;
        }
        if (map_id_table[result] == 0){
            result = user_map_count;
            break;
        }
    }
    return(result);
}

internal Command_Map*
get_map_base(Models *models, i32 mapid, b32 add){
    Command_Map *map = 0;
    if (mapid < mapid_global){
        if (add){
            mapid = get_or_add_map_index(models, mapid);
        }
        else{
            mapid = get_map_index(models, mapid);
        }
        if (mapid < models->user_map_count){
            map = models->user_maps + mapid;
        }
    }
    else if (mapid == mapid_global){
        map = &models->map_top;
    }
    else if (mapid == mapid_file){
        map = &models->map_file;
    }
    return(map);
}

internal Command_Map*
get_or_add_map(Models *models, i32 mapid){
    Command_Map *map = get_map_base(models, mapid, 1);
    return(map);
}

internal Command_Map*
get_map(Models *models, i32 mapid){
    Command_Map *map = get_map_base(models, mapid, 0);
    return(map);
}

internal void
map_set_count(Models *models, i32 mapid, u32 count){
    Command_Map *map = get_or_add_map(models, mapid);
    Assert(map->commands == 0);
    map->count = count;
    if (map->max < count){
        map->max = count;
    }
}

internal u32
map_get_count(Models *models, i32 mapid){
    Command_Map *map = get_or_add_map(models, mapid);
    u32 count = map->count;
    Assert(map->commands == 0);
    return(count);
}

internal u32
map_get_max_count(Models *models, i32 mapid){
    Command_Map *map = get_or_add_map(models, mapid);
    u32 count = map->max;
    return(count);
}

enum Interactive_Action{
    IAct_Open,
    IAct_Save_As,
    IAct_New,
    IAct_Switch,
    IAct_Kill,
    IAct_Sure_To_Kill,
    IAct_Sure_To_Close
};

enum Interactive_Interaction{
    IInt_Sys_File_List,
    IInt_Live_File_List,
    IInt_Sure_To_Kill,
    IInt_Sure_To_Close
};

enum View_UI{
    VUI_None,
    VUI_Theme,
    VUI_Interactive,
    VUI_Menu,
    VUI_Config,
    VUI_Debug
};

enum Debug_Mode{
    DBG_Input,
    DBG_Threads_And_Memory,
    DBG_View_Inspection
};

enum Color_View_Mode{
    CV_Mode_Library,
    CV_Mode_Font,
    CV_Mode_Global_Font,
    CV_Mode_Adjusting
};

struct File_Viewing_Data{
    Editing_File *file;
    
    Full_Cursor temp_highlight;
    i32 temp_highlight_end_pos;
    b32 show_temp_highlight;
    
    b32 show_whitespace;
    b32 file_locked;
};
static File_Viewing_Data null_file_viewing_data = {0};

struct Scroll_Context{
    Editing_File *file;
    GUI_id scroll;
    View_UI mode;
};
inline b32
context_eq(Scroll_Context a, Scroll_Context b){
    b32 result = 0;
    if (gui_id_eq(a.scroll, b.scroll)){
        if (a.file == b.file){
            if (a.mode == b.mode){
                result = 1;
            }
        }
    }
    return(result);
}

struct View_Persistent{
    i32 id;
    
    Coroutine *coroutine;
    Event_Message message_passing_slot;
    
    // TODO(allen): eliminate this models pointer: explicitly parameterize.
    Models *models;
};

struct Debug_Vars{
    i32 mode;
    i32 inspecting_view_id;
};
inline Debug_Vars
debug_vars_zero(){
    Debug_Vars vars = {0};
    return(vars);
}

struct View{
    View_Persistent persistent;
    
    View *next, *prev;
    Panel *panel;
    b32 in_use;
    Command_Map *map;
    
    File_Viewing_Data file_data;
    
    i32_Rect file_region_prev;
    i32_Rect file_region;
    
    i32_Rect scroll_region;
    File_Edit_Positions *edit_pos;
    
    View_UI showing_ui;
    GUI_Target gui_target;
    void *gui_mem;
    GUI_Scroll_Vars gui_scroll;
    i32 gui_max_y;
    i32 list_i;
    
    b32 hide_scrollbar;
    
    // interactive stuff
    Interactive_Interaction interaction;
    Interactive_Action action;
    
    char dest_[256];
    String dest;
    
    b32 changed_context_in_step;
    
    // theme stuff
    View *hot_file_view;
    u32 *palette;
    i32 palette_size;
    Color_View_Mode color_mode;
    Super_Color color;
    b32 p4c_only;
    Style_Library inspecting_styles;
    b8 import_export_check[64];
    i32 import_file_id;
    i32 current_color_editing;
    i32 color_cursor;
    
    // misc
    
    // TODO(allen): Can we burn line_height to the ground now?
    // It's what I've always wanted!!!! :D
    i32 line_height;
    
    // TODO(allen): Do I still use mode?
    Query_Set query_set;
    f32 widget_height;
    
    b32 reinit_scrolling;
    
    Debug_Vars debug_vars;
};

inline void*
get_view_body(View *view){
    char *result = (char*)view;
    result += sizeof(View_Persistent);
    return(result);
}
inline i32
get_view_size(){
    return(sizeof(View) - sizeof(View_Persistent));
}

// TODO(past-allen): Switch over to using an i32 for these.
inline f32
view_width(View *view){
    i32_Rect file_rect = view->file_region;
    f32 result = (f32)(file_rect.x1 - file_rect.x0);
    return (result);
}

inline f32
view_file_display_width(View *view){
    Editing_File *file = view->file_data.file;
    f32 result = (f32)file->settings.display_width;
    return(result);
}

inline f32
view_file_minimum_base__width(View *view){
    Editing_File *file = view->file_data.file;
    f32 result = (f32)file->settings.display_width;
    return(result);
}

inline f32
view_file_height(View *view){
    i32_Rect file_rect = view->file_region;
    f32 result = (f32)(file_rect.y1 - file_rect.y0);
    return (result);
}

inline i32
view_get_cursor_pos(View *view){
    i32 result = 0;
    if (view->file_data.show_temp_highlight){
        result = view->file_data.temp_highlight.pos;
    }
    else if (view->edit_pos){
        result = view->edit_pos->cursor.pos;
    }
    return(result);
}

inline f32
view_get_cursor_x(View *view){
    f32 result = 0;
    
    Full_Cursor *cursor = 0;
    if (view->file_data.show_temp_highlight){
        cursor = &view->file_data.temp_highlight;
    }
    else if (view->edit_pos){
        cursor = &view->edit_pos->cursor;
    }
    
    if (cursor){
        result = cursor->wrapped_x;
        if (view->file_data.file->settings.unwrapped_lines){
            result = cursor->unwrapped_x;
        }
    }
    
    return(result);
}

inline f32
view_get_cursor_y(View *view){
    f32 result = 0;
    
    Full_Cursor *cursor = 0;
    if (view->file_data.show_temp_highlight){
        cursor = &view->file_data.temp_highlight;
    }
    else if (view->edit_pos){
        cursor = &view->edit_pos->cursor;
    }
    
    if (cursor){
        result = cursor->wrapped_y;
        if (view->file_data.file->settings.unwrapped_lines){
            result = cursor->unwrapped_y;
        }
    }
    
    return(result);
}

struct Cursor_Limits{
    f32 min, max;
    f32 delta;
};

inline Cursor_Limits
view_cursor_limits(View *view){
    Cursor_Limits limits = {0};
    
    f32 line_height = (f32)view->line_height;
    f32 visible_height = view_file_height(view);
    
    limits.max = visible_height - line_height*3.f;
    limits.min = line_height * 2;
    
    if (limits.max - limits.min <= line_height){
        if (visible_height >= line_height){
            limits.max = visible_height - line_height;
            limits.min = -line_height;
        }
        else{
            limits.max = visible_height;
            limits.min = -line_height;
        }
    }
    
    limits.max = (limits.max > 0)?(limits.max):(0);
    limits.min = (limits.min > 0)?(limits.min):(0);
    
    limits.delta = clamp_top(line_height*3.f, (limits.max - limits.min)*.5f);
    
    return(limits);
}

internal Full_Cursor
view_compute_cursor(View *view, Buffer_Seek seek, b32 return_hint){
    Editing_File *file = view->file_data.file;
    Models *models = view->persistent.models;
    Render_Font *font = get_font_info(models->font_set, file->settings.font_id)->font;
    
    Full_Cursor result = {0};
    
    Buffer_Cursor_Seek_Params params;
    params.buffer           = &file->state.buffer;
    params.seek             = seek;
    params.font_height      = (f32)font->height;
    params.cp_adv           = font->codepoint_advance_data;
    params.byte_adv         = font->byte_advance;
    params.wrap_line_index  = file->state.wrap_line_index;
    params.character_starts = file->state.character_starts;
    params.virtual_white    = file->settings.virtual_white;
    params.return_hint      = return_hint;
    params.cursor_out       = &result;
    
    Buffer_Cursor_Seek_State state = {0};
    Buffer_Layout_Stop stop = {0};
    
    i32 size = buffer_size(params.buffer);
    
    f32 line_shift = 0.f;
    b32 do_wrap = 0;
    i32 wrap_unit_end = 0;
    
    b32 first_wrap_determination = 1;
    i32 wrap_array_index = 0;
    
    do{
        stop = buffer_cursor_seek(&state, params, line_shift, do_wrap, wrap_unit_end);
        switch (stop.status){
            case BLStatus_NeedWrapDetermination:
            {
                if (stop.pos >= size){
                    do_wrap = 0;
                    wrap_unit_end = max_i32;
                }
                else{
                    if (first_wrap_determination){
                        wrap_array_index = binary_search(file->state.wrap_positions, stop.pos, 0, file->state.wrap_position_count);
                        ++wrap_array_index;
                        if (file->state.wrap_positions[wrap_array_index] == stop.pos){
                            do_wrap = 1;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                        else{
                            do_wrap = 0;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                        first_wrap_determination = 0;
                    }
                    else{
                        assert(stop.pos == wrap_unit_end);
                        do_wrap = 1;
                        ++wrap_array_index;
                        wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                    }
                }
            }break;
            
            case BLStatus_NeedWrapLineShift:
            case BLStatus_NeedLineShift:
            {
                line_shift = file->state.line_indents[stop.wrap_line_index];
            }break;
        }
    }while(stop.status != BLStatus_Finished);
    
    return(result);
}

inline Full_Cursor
view_compute_cursor_from_xy(View *view, f32 seek_x, f32 seek_y){
    Buffer_Seek seek;
    if (view->file_data.file->settings.unwrapped_lines){
        seek = seek_unwrapped_xy(seek_x, seek_y, 0);
    }
    else{
        seek = seek_wrapped_xy(seek_x, seek_y, 0);
    }
    
    Full_Cursor result = view_compute_cursor(view, seek, 0);
    return(result);
}

inline i32
view_compute_max_target_y(i32 lowest_line, i32 line_height, f32 view_height){
    f32 max_target_y = ((lowest_line+.5f)*line_height) - view_height*.5f;
    max_target_y = clamp_bottom(0.f, max_target_y);
    return(ceil32(max_target_y));
}

internal i32
file_compute_lowest_line(Editing_File *file, f32 font_height){
    i32 lowest_line = 0;
    
    Gap_Buffer *buffer = &file->state.buffer;
    if (file->settings.unwrapped_lines){
        lowest_line = buffer->line_count;
    }
    else{
        lowest_line = file->state.wrap_line_index[buffer->line_count];
    }
    
    return(lowest_line);
}

inline i32
view_compute_max_target_y(View *view){
    i32 line_height = view->line_height;
    i32 lowest_line = file_compute_lowest_line(view->file_data.file, (f32)line_height);
    f32 view_height = clamp_bottom((f32)line_height, view_file_height(view));
    i32 max_target_y = view_compute_max_target_y(lowest_line, line_height, view_height);
    return(max_target_y);
}

internal b32
view_move_view_to_cursor(View *view, GUI_Scroll_Vars *scroll, b32 center_view){
    b32 result = 0;
    f32 max_x = view_width(view);
    i32 max_y = view_compute_max_target_y(view);
    
    f32 cursor_y = view_get_cursor_y(view);
    f32 cursor_x = view_get_cursor_x(view);
    
    GUI_Scroll_Vars scroll_vars = *scroll;
    i32 target_y = scroll_vars.target_y;
    i32 target_x = scroll_vars.target_x;
    
    Cursor_Limits limits = view_cursor_limits(view);
    
    if (cursor_y > target_y + limits.max){
        if (center_view){
            target_y = round32(cursor_y - limits.max*.5f);
        }
        else{
            target_y = ceil32(cursor_y - limits.max + limits.delta);
        }
    }
    if (cursor_y < target_y + limits.min){
        if (center_view){
            target_y = round32(cursor_y - limits.max*.5f);
        }
        else{
            target_y = floor32(cursor_y - limits.delta - limits.min);
        }
    }
    
    target_y = clamp(0, target_y, max_y);
    
    if (cursor_x >= target_x + max_x){
        target_x = ceil32(cursor_x - max_x/2);
    }
    else if (cursor_x < target_x){
        target_x = floor32(Max(0, cursor_x - max_x/2));
    }
    
    if (target_x != scroll_vars.target_x || target_y != scroll_vars.target_y){
        scroll->target_x = target_x;
        scroll->target_y = target_y;
        result = 1;
    }
    
    return(result);
}

internal b32
view_move_cursor_to_view(View *view, GUI_Scroll_Vars scroll,
                         Full_Cursor *cursor, f32 preferred_x){
    b32 result = 0;
    
    if (view->edit_pos){
        i32 line_height = view->line_height;
        f32 old_cursor_y = cursor->wrapped_y;
        if (view->file_data.file->settings.unwrapped_lines){
            old_cursor_y = cursor->unwrapped_y;
        }
        f32 cursor_y = old_cursor_y;
        f32 target_y = scroll.target_y + view->widget_height;
        
        Cursor_Limits limits = view_cursor_limits(view);
        
        if (cursor_y > target_y + limits.max){
            cursor_y = target_y + limits.max;
        }
        if (target_y != 0 && cursor_y < target_y + limits.min){
            cursor_y = target_y + limits.min;
        }
        
        if (cursor_y != old_cursor_y){
            if (cursor_y > old_cursor_y){
                cursor_y += line_height;
            }
            else{
                cursor_y -= line_height;
            }
            
            *cursor = view_compute_cursor_from_xy(
                view, preferred_x, cursor_y);
            
            result = 1;
        }
    }
    
    return(result);
}

internal void
view_set_cursor(View *view, Full_Cursor cursor, b32 set_preferred_x, b32 unwrapped_lines){
    if (edit_pos_move_to_front(view->file_data.file, view->edit_pos)){
        edit_pos_set_cursor(view->edit_pos, cursor, set_preferred_x, unwrapped_lines);
        GUI_Scroll_Vars scroll = view->edit_pos->scroll;
        if (view_move_view_to_cursor(view, &scroll, 0)){
            view->edit_pos->scroll = scroll;
        }
    }
}

internal void
view_set_scroll(View *view, GUI_Scroll_Vars scroll){
    if (edit_pos_move_to_front(view->file_data.file, view->edit_pos)){
        edit_pos_set_scroll(view->edit_pos, scroll);
        Full_Cursor cursor = view->edit_pos->cursor;
        if (view_move_cursor_to_view(view, view->edit_pos->scroll,
                                     &cursor, view->edit_pos->preferred_x)){
            view->edit_pos->cursor = cursor;
        }
    }
}

internal void
view_set_cursor_and_scroll(View *view, Full_Cursor cursor, b32 set_preferred_x, b32 unwrapped_lines, GUI_Scroll_Vars scroll){
    File_Edit_Positions *edit_pos = view->edit_pos;
    if (edit_pos_move_to_front(view->file_data.file, edit_pos)){
        edit_pos_set_cursor(edit_pos, cursor, set_preferred_x, unwrapped_lines);
        edit_pos_set_scroll(edit_pos, scroll);
        edit_pos->last_set_type = EditPos_None;
    }
}

inline void
view_set_temp_highlight(View *view, i32 pos, i32 end_pos){
    view->file_data.temp_highlight = view_compute_cursor(view, seek_pos(pos), 0);
    view->file_data.temp_highlight_end_pos = end_pos;
    view->file_data.show_temp_highlight = 1;
    
    view_set_cursor(view, view->file_data.temp_highlight,
                    0, view->file_data.file->settings.unwrapped_lines);
}

struct View_And_ID{
    View *view;
    i32 id;
};

struct Live_Views{
    View *views;
    View free_sentinel;
    i32 count, max;
};

enum Lock_Level{
    LockLevel_Open      = 0,
    LockLevel_Protected = 1,
    LockLevel_Hidden    = 2
};

inline u32
view_lock_flags(View *view){
    u32 result = AccessOpen;
    File_Viewing_Data *data = &view->file_data;
    if (view->showing_ui != VUI_None){
        result |= AccessHidden;
    }
    if (data->file_locked ||
        (data->file && data->file->settings.read_only)){
        result |= AccessProtected;
    }
    return(result);
}

inline i32
view_lock_level(View *view){
    i32 result = LockLevel_Open;
    u32 flags = view_lock_flags(view);
    if (flags & AccessHidden){
        result = LockLevel_Hidden;
    }
    else if (flags & AccessProtected){
        result = LockLevel_Protected;
    }
    return(result);
}

struct View_Iter{
    View *view;
    
    Editing_File *file;
    View *skip;
    Panel *used_panels;
    Panel *panel;
};

internal View_Iter
file_view_iter_next(View_Iter iter){
    View *view;
    
    for (iter.panel = iter.panel->next; iter.panel != iter.used_panels; iter.panel = iter.panel->next){
        view = iter.panel->view;
        if (view != iter.skip && (view->file_data.file == iter.file || iter.file == 0)){
            iter.view = view;
            break;
        }
    }
    
    return(iter);
}

internal View_Iter
file_view_iter_init(Editing_Layout *layout, Editing_File *file, View *skip){
    View_Iter result;
    result.used_panels = &layout->used_sentinel;
    result.panel = result.used_panels;
    result.file = file;
    result.skip = skip;
    
    result = file_view_iter_next(result);
    
    return(result);
}

internal b32
file_view_iter_good(View_Iter iter){
    b32 result = (iter.panel != iter.used_panels);
    return(result);
}

inline b32
starts_new_line(u8 character){
    return (character == '\n');
}

inline void
file_synchronize_times(System_Functions *system, Editing_File *file){
    file->state.dirty = DirtyState_UpToDate;
}

internal b32
save_file_to_name(System_Functions *system, Models *models, Editing_File *file, char *filename){
    b32 result = 0;
    b32 using_actual_filename = false;
    
    if (!filename){
        terminate_with_null(&file->canon.name);
        filename = file->canon.name.str;
        using_actual_filename = true;
    }
    
    if (filename){
        Mem_Options *mem = &models->mem;
        if (models->hook_save_file){
            models->hook_save_file(&models->app_links, file->id.id);
        }
        
        i32 max = 0, size = 0;
        b32 dos_write_mode = file->settings.dos_write_mode;
        char *data = 0;
        Gap_Buffer *buffer = &file->state.buffer;
        
        if (dos_write_mode){
            max = buffer_size(buffer) + buffer->line_count + 1;
        }
        else{
            max = buffer_size(buffer);
        }
        
        b32 used_general = 0;
        Temp_Memory temp = begin_temp_memory(&mem->part);
        char empty = 0;
        if (max == 0){
            data = &empty;
        }
        else{
            data = (char*)push_array(&mem->part, char, max);
            if (!data){
                used_general = 1;
                data = (char*)general_memory_allocate(&mem->general, max);
            }
        }
        Assert(data != 0);
        
        if (dos_write_mode){
            size = buffer_convert_out(buffer, data, max);
        }
        else{
            size = max;
            buffer_stringify(buffer, 0, size, data);
        }
        
        if (!using_actual_filename && file->canon.name.str != 0){
            char space[512];
            u32 length = str_size(filename);
            system->get_canonical(filename, length, space, sizeof(space));
            
            char *source_path = file->canon.name.str;
            if (match(space, source_path)){
                using_actual_filename = true;
            }
        }
        
        result = system->save_file(filename, data, size);
        
        if (result && using_actual_filename){
            file->state.ignore_behind_os = true;
        }
        
        file_mark_clean(file);
        
        if (used_general){
            general_memory_free(&mem->general, data);
        }
        end_temp_memory(temp);
        
        file_synchronize_times(system, file);
    }
    
    return(result);
}

inline b32
save_file(System_Functions *system, Models *models, Editing_File *file){
    b32 result = save_file_to_name(system, models, file, 0);
    return(result);
}

internal b32
buffer_link_to_new_file(System_Functions *system, General_Memory *general, Working_Set *working_set,
                        Editing_File *file, String filename){
    b32 result = 0;
    
    Editing_File_Canon_Name canon_name;
    if (get_canon_name(system, &canon_name, filename)){
        buffer_unbind_name(working_set, file);
        if (file->canon.name.size != 0){
            buffer_unbind_file(system, working_set, file);
        }
        buffer_bind_file(system, general, working_set, file, canon_name.name);
        buffer_bind_name(general, working_set, file, filename);
        result = 1;
    }
    
    return(result);
}

inline b32
file_save_and_set_names(System_Functions *system, Models *models, Editing_File *file, String filename){
    b32 result = buffer_link_to_new_file(system, &models->mem.general, &models->working_set, file, filename);
    if (result){
        result = save_file(system, models, file);
    }
    return(result);
}

enum{
    GROW_FAILED,
    GROW_NOT_NEEDED,
    GROW_SUCCESS,
};

internal i32
file_grow_starts_as_needed(General_Memory *general, Gap_Buffer *buffer, i32 additional_lines){
    b32 result = GROW_NOT_NEEDED;
    i32 max = buffer->line_max;
    i32 count = buffer->line_count;
    i32 target_lines = count + additional_lines;
    
    if (target_lines > max || max == 0){
        max = l_round_up_i32(target_lines + max, KB(1));
        
        i32 *new_lines = (i32*)general_memory_reallocate(general, buffer->line_starts, sizeof(i32)*count, sizeof(f32)*max);
        
        if (new_lines){
            result = GROW_SUCCESS;
            buffer->line_max = max;
            buffer->line_starts = new_lines;
        }
        else{
            result = GROW_FAILED;
        }
    }
    
    return(result);
}

internal void
file_update_cursor_positions(Models *models, Editing_File *file){
    Editing_Layout *layout = &models->layout;
    for (View_Iter iter = file_view_iter_init(layout, file, 0);
         file_view_iter_good(iter);
         iter = file_view_iter_next(iter)){
        i32 pos = view_get_cursor_pos(iter.view);
        
        if (!iter.view->file_data.show_temp_highlight){
            Full_Cursor cursor = view_compute_cursor(iter.view, seek_pos(pos), 0);
            view_set_cursor(iter.view, cursor, 1, iter.view->file_data.file->settings.unwrapped_lines);
        }
        else{
            view_set_temp_highlight(iter.view, pos, iter.view->file_data.temp_highlight_end_pos);
        }
    }
}

//
// File Metadata Measuring
//

internal void
file_measure_starts(General_Memory *general, Gap_Buffer *buffer){
    if (!buffer->line_starts){
        i32 max = buffer->line_max = KB(1);
        buffer->line_starts = (i32*)general_memory_allocate(general, max*sizeof(i32));
        TentativeAssert(buffer->line_starts);
        // TODO(allen): when unable to allocate?
    }
    
    Buffer_Measure_Starts state = {0};
    while (buffer_measure_starts(&state, buffer)){
        i32 count = state.count;
        i32 max = buffer->line_max;
        max = ((max + 1) << 1);
        
        {
            i32 *new_lines = (i32*)general_memory_reallocate(general, buffer->line_starts, sizeof(i32)*count, sizeof(i32)*max);
            
            // TODO(allen): when unable to grow?
            TentativeAssert(new_lines);
            buffer->line_starts = new_lines;
            buffer->line_max = max;
        }
    }
}

// NOTE(allen): These calls assumes that the buffer's line starts are already correct,
// and that the buffer's line_count is correct.
internal void
file_allocate_metadata_as_needed(General_Memory *general, Gap_Buffer *buffer, void **mem, i32 *mem_max_count, i32 count, i32 item_size){
    if (*mem == 0){
        i32 max = ((count+1)*2);
        max = (max+(0x3FF))&(~(0x3FF));
        *mem = general_memory_allocate(general, max*item_size);
        *mem_max_count = max;
    }
    else if (*mem_max_count < count){
        i32 old_max = *mem_max_count;
        i32 max = ((count+1)*2);
        max = (max+(0x3FF))&(~(0x3FF));
        
        void *new_mem = general_memory_reallocate(general, *mem, item_size*old_max, item_size*max);
        
        Assert(new_mem);
        *mem = new_mem;
        *mem_max_count = max;
    }
}

inline void
file_allocate_character_starts_as_needed(General_Memory *general, Editing_File *file){
    file_allocate_metadata_as_needed(general, &file->state.buffer, (void**)&file->state.character_starts, &file->state. character_start_max, file->state.buffer.line_count, sizeof(i32));
}

internal void
file_measure_character_starts(Models *models, Editing_File *file){
    file_allocate_character_starts_as_needed(&models->mem.general, file);
    buffer_measure_character_starts(&file->state.buffer, file->state.character_starts, 0, file->settings.virtual_white);
    file_update_cursor_positions(models, file);
}

internal void
file_allocate_indents_as_needed(General_Memory *general, Editing_File *file, i32 min_last_index){
    i32 min_amount = min_last_index + 1;
    file_allocate_metadata_as_needed(general, &file->state.buffer, (void**)&file->state.line_indents, &file->state.line_indent_max, min_amount, sizeof(f32));
}

inline void
file_allocate_wraps_as_needed(General_Memory *general, Editing_File *file){
    file_allocate_metadata_as_needed(general, &file->state.buffer, (void**)&file->state.wrap_line_index, &file->state.wrap_max, file->state.buffer.line_count, sizeof(f32));
}

inline void
file_allocate_wrap_positions_as_needed(General_Memory *general, Editing_File *file, i32 min_last_index){
    i32 min_amount = min_last_index + 1;
    file_allocate_metadata_as_needed(general, &file->state.buffer, (void**)&file->state.wrap_positions, &file->state.wrap_position_max, min_amount, sizeof(f32));
}

struct Code_Wrap_X{
    f32 base_x;
    f32 paren_nesting[32];
    i32 paren_safe_top;
    i32 paren_top;
};
global Code_Wrap_X null_wrap_x  = {0};

struct Code_Wrap_State{
    Cpp_Token_Array token_array;
    Cpp_Token *token_ptr;
    Cpp_Token *end_token;
    
    Code_Wrap_X wrap_x;
    
    b32 in_pp_body;
    Code_Wrap_X plane_wrap_x;
    
    i32 *line_starts;
    i32 line_index;
    i32 next_line_start;
    
    f32 x;
    b32 consume_newline;
    
    Gap_Buffer_Stream stream;
    i32 size;
    i32 i;
    
    f32 *cp_adv;
    f32 byte_adv;
    f32 tab_indent_amount;
    
    Buffer_Translating_State tran;
};

internal void
wrap_state_init(Code_Wrap_State *state, Editing_File *file, f32 *cp_adv, f32 byte_adv){
    state->token_array = file->state.token_array;
    state->token_ptr = state->token_array.tokens;
    state->end_token = state->token_ptr + state->token_array.count;
    
    state->line_starts = file->state.buffer.line_starts;
    state->next_line_start = state->line_starts[1];
    
    Gap_Buffer *buffer = &file->state.buffer;
    i32 size = buffer_size(buffer);
    buffer_stringify_loop(&state->stream, buffer, 0, size);
    state->size = size;
    state->i = 0;
    
    state->cp_adv = cp_adv;
    state->byte_adv = byte_adv;
    state->tab_indent_amount = cp_adv['\t'];
    
    state->tran = null_buffer_translating_state;
}

internal void
wrap_state_set_x(Code_Wrap_State *state, f32 line_shift){
    state->x = line_shift;
}

internal void
wrap_state_set_i(Code_Wrap_State *state, i32 i){
    state->i = i;
}

internal void
wrap_state_set_top(Code_Wrap_State *state, f32 line_shift){
    if (state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top] > line_shift){
        state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top] = line_shift;
    }
}

struct Code_Wrap_Step{
    i32 position_start;
    i32 position_end;
    
    f32 start_x;
    f32 final_x;
    
    Cpp_Token *this_token;
};

internal Code_Wrap_Step
wrap_state_consume_token(Code_Wrap_State *state, i32 fixed_end_point){
    Code_Wrap_Step result = {0};
    i32 i = state->i;
    
    result.position_start = i;
    
    if (state->consume_newline){
        ++i;
        state->x = 0;
        state->consume_newline = 0;
    }
    
    if (state->in_pp_body){
        if (!(state->token_ptr->flags & CPP_TFLAG_PP_BODY)){
            state->in_pp_body = 0;
            state->wrap_x = state->plane_wrap_x;
        }
    }
    
    if (!state->in_pp_body){
        if (state->token_ptr->flags & CPP_TFLAG_PP_DIRECTIVE){
            state->in_pp_body = 1;
            state->plane_wrap_x = state->wrap_x;
            state->wrap_x = null_wrap_x;
        }
    }
    
    b32 skipping_whitespace = false;
    if (i >= state->next_line_start){
        state->x = state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top];
        state->x = state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top];
        skipping_whitespace = true;
    }
    
    // TODO(allen): exponential search this shit!
    while (i >= state->next_line_start){
        ++state->line_index;
        state->next_line_start = state->line_starts[state->line_index + 1];
    }
    
    i32 line_start = state->line_starts[state->line_index];
    b32 still_looping = 0;
    i32 end = state->token_ptr->start + state->token_ptr->size;
    
    if (fixed_end_point >= 0 && end > fixed_end_point){
        end = fixed_end_point;
    }
    
    if (i < line_start){
        i = line_start;
    }
    
    if (i == line_start){
        skipping_whitespace = true;
    }
    
    b32 recorded_start_x = false;
    do{
        for (; i < state->stream.end; ++i){
            if (!(i < end)){
                Assert(state->tran.fill_expected == 0);
                goto doublebreak;
            }
            
            u8 ch = (u8)state->stream.data[i];
            
            translating_consume_byte(&state->tran, ch, i, state->size);
            
            for (TRANSLATION_OUTPUT(&state->tran)){
                TRANSLATION_GET_STEP(&state->tran);
                
                if (state->tran.do_newline){
                    state->consume_newline = 1;
                    goto doublebreak;
                }
                else if(state->tran.do_number_advance || state->tran.do_codepoint_advance){
                    u32 n = state->tran.step_current.value;
                    f32 adv = 0;
                    if (state->tran.do_codepoint_advance){
                        adv = state->cp_adv[n];
                        if (n != ' ' && n != '\t'){
                            skipping_whitespace = false;
                        }
                    }
                    else{
                        adv = state->byte_adv;
                        skipping_whitespace = false;
                    }
                    
                    if (!skipping_whitespace){
                        if (!recorded_start_x){
                            result.start_x = state->x;
                            recorded_start_x = 1;
                        }
                        state->x += adv;
                    }
                }
            }
        }
        still_looping = buffer_stringify_next(&state->stream);
    }while(still_looping);
    doublebreak:;
    
    state->i = i;
    
    b32 consume_token = 0;
    if (i >= state->token_ptr->start + state->token_ptr->size){
        consume_token = 1;
    }
    
    result.this_token = state->token_ptr;
    if (consume_token){
        switch (state->token_ptr->type){
            case CPP_TOKEN_BRACE_OPEN:
            {
                state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top] += state->tab_indent_amount;
            }break;
            
            case CPP_TOKEN_BRACE_CLOSE:
            {
                state->wrap_x.paren_nesting[state->wrap_x.paren_safe_top] -= state->tab_indent_amount;
            }break;
            
            case CPP_TOKEN_PARENTHESE_OPEN:
            case CPP_TOKEN_BRACKET_OPEN:
            {
                ++state->wrap_x.paren_top;
                
                i32 top = state->wrap_x.paren_top;
                if (top >= ArrayCount(state->wrap_x.paren_nesting)){
                    top = ArrayCount(state->wrap_x.paren_nesting) - 1;
                }
                state->wrap_x.paren_safe_top = top;
                
                state->wrap_x.paren_nesting[top] = state->x;
            }break;
            
            case CPP_TOKEN_PARENTHESE_CLOSE:
            case CPP_TOKEN_BRACKET_CLOSE:
            {
                --state->wrap_x.paren_top;
                
                if (state->wrap_x.paren_top < 0){
                    state->wrap_x.paren_top = 0;
                }
                
                i32 top = state->wrap_x.paren_top;
                if (top >= ArrayCount(state->wrap_x.paren_nesting)){
                    top = ArrayCount(state->wrap_x.paren_nesting) - 1;
                }
                state->wrap_x.paren_safe_top = top;
            }break;
        }
        
        ++state->token_ptr;
    }
    
    result.position_end = state->i;
    result.final_x = state->x;
    
    if (!recorded_start_x){
        result.start_x = state->x;
        recorded_start_x = 1;
    }
    
    return(result);
}

struct Wrap_Indent_Pair{
    i32 wrap_position;
    f32 line_shift;
};

struct Potential_Wrap_Indent_Pair{
    i32 wrap_position;
    f32 line_shift;
    
    f32 wrap_x;
    i32 wrappable_score;
    
    b32 adjust_top_to_this;
};

internal i32
stickieness_guess(Cpp_Token_Type type, Cpp_Token_Type other_type, u16 flags, u16 other_flags, b32 on_left){
    i32 guess = 0;
    
    b32 is_words = 0, other_is_words = 0;
    if (type == CPP_TOKEN_IDENTIFIER || (type >= CPP_TOKEN_KEY_TYPE && type <= CPP_TOKEN_KEY_OTHER)){
        is_words = 1;
    }
    if (other_type == CPP_TOKEN_IDENTIFIER || (other_type >= CPP_TOKEN_KEY_TYPE && other_type <= CPP_TOKEN_KEY_OTHER)){
        other_is_words = 1;
    }
    
    b32 is_operator = 0, other_is_operator = 0;
    if (flags & CPP_TFLAG_IS_OPERATOR){
        is_operator = 1;
    }
    if (other_flags & CPP_TFLAG_IS_OPERATOR){
        other_is_operator = 1;
    }
    
    i32 operator_side_bias = 70*(!on_left);
    
    if (is_words && other_is_words){
        guess = 200;
    }
    else if (type == CPP_TOKEN_PARENTHESE_OPEN){
        if (on_left){
            guess = 100;
        }
        else{
            if (other_is_words){
                guess = 100;
            }
            else{
                guess = 0;
            }
        }
    }
    else if (type == CPP_TOKEN_SEMICOLON){
        if (on_left){
            guess = 0;
        }
        else{
            guess = 1000;
        }
    }
    else if (type == CPP_TOKEN_COMMA){
        guess = 20;
    }
    else if (type == CPP_TOKEN_COLON ||
             type == CPP_TOKEN_PARENTHESE_CLOSE ||
             type == CPP_TOKEN_BRACKET_OPEN ||
             type == CPP_TOKEN_BRACKET_CLOSE){
        if (on_left){
            guess = 20;
            if (other_is_words){
                guess = 100;
            }
        }
        else{
            guess = 100;
        }
    }
    else if (type == CPP_PP_DEFINED){
        if (on_left){
            guess = 100;
        }
        else{
            guess = 0;
        }
    }
    else if (type == CPP_TOKEN_SCOPE){
        guess = 90;
    }
    else if (type == CPP_TOKEN_MINUS){
        if (on_left){
            guess = 80;
        }
        else{
            guess = 60;
        }
    }
    else if (type == CPP_TOKEN_DOT ||
             type == CPP_TOKEN_ARROW){
        guess = 200;
    }
    else if (type == CPP_TOKEN_NOT ||
             type == CPP_TOKEN_TILDE){
        if (on_left){
            guess = 80;
        }
        else{
            guess = 20;
        }
    }
    else if (type == CPP_TOKEN_INCREMENT ||
             type == CPP_TOKEN_DECREMENT ||
             type == CPP_TOKEN_STAR ||
             type == CPP_TOKEN_AMPERSAND ||
             (type >= CPP_TOKEN_POSTINC &&
              type <= CPP_TOKEN_DELETE_ARRAY)){
        guess = 80;
        if (!on_left && other_is_operator){
            guess = 20;
        }
    }
    else if (type >= CPP_TOKEN_MUL && type <= CPP_TOKEN_MOD){
        guess = 70;
    }
    else if (type == CPP_TOKEN_PLUS){
        guess = 60 + operator_side_bias;
    }
    else if (type >= CPP_TOKEN_LSHIFT && type <= CPP_TOKEN_RSHIFT){
        guess = 50;
    }
    else if (type >= CPP_TOKEN_LESS && type <= CPP_TOKEN_NOTEQ){
        guess = 40 + operator_side_bias;
    }
    else if (type >= CPP_TOKEN_BIT_XOR && type <= CPP_TOKEN_BIT_OR){
        guess = 40;
    }
    else if (type >= CPP_TOKEN_AND && type <= CPP_TOKEN_OR){
        guess = 30 + operator_side_bias;
    }
    else if (type >= CPP_TOKEN_TERNARY_QMARK && type <= CPP_TOKEN_COLON){
        guess = 20 + operator_side_bias;
    }
    else if (type == CPP_TOKEN_THROW){
        if (on_left){
            guess = 100;
        }
        else{
            guess = 0;
        }
    }
    else if (type >= CPP_TOKEN_EQ && type <= CPP_TOKEN_XOREQ){
        guess = 15 + operator_side_bias;
    }
    
    return(guess);
}

internal f32
get_current_shift(Code_Wrap_State *wrap_state, i32 next_line_start, b32 *adjust_top_to_this){
    f32 current_shift = wrap_state->wrap_x.paren_nesting[wrap_state->wrap_x.paren_safe_top];
    
    Assert(adjust_top_to_this != 0);
    if (wrap_state->token_ptr > wrap_state->token_array.tokens){
        Cpp_Token prev_token = *(wrap_state->token_ptr-1);
        
        if (wrap_state->wrap_x.paren_safe_top != 0 && prev_token.type == CPP_TOKEN_PARENTHESE_OPEN){
            current_shift = wrap_state->wrap_x.paren_nesting[wrap_state->wrap_x.paren_safe_top-1] + wrap_state->tab_indent_amount;
            
            *adjust_top_to_this = 1;
        }
        
        f32 statement_continuation_indent = 0.f;
        if (current_shift != 0.f && wrap_state->wrap_x.paren_safe_top == 0){
            if (!(prev_token.flags & CPP_TFLAG_PP_BODY) && !(prev_token.flags & CPP_TFLAG_PP_DIRECTIVE)){
                
                switch (prev_token.type){
                    case CPP_TOKEN_BRACKET_OPEN:
                    case CPP_TOKEN_BRACE_OPEN:
                    case CPP_TOKEN_BRACE_CLOSE:
                    case CPP_TOKEN_SEMICOLON:
                    case CPP_TOKEN_COLON:
                    case CPP_TOKEN_COMMA:
                    case CPP_TOKEN_COMMENT: break;
                    default: statement_continuation_indent += wrap_state->tab_indent_amount; break;
                }
            }
        }
        
        switch (wrap_state->token_ptr->type){
            case CPP_TOKEN_BRACE_CLOSE: case CPP_TOKEN_BRACE_OPEN: break;
            default: current_shift += statement_continuation_indent; break;
        }
    }
    
    if (wrap_state->token_ptr->start < next_line_start){
        if (wrap_state->token_ptr->flags & CPP_TFLAG_PP_DIRECTIVE){
            current_shift = 0;
        }
        else{
            switch (wrap_state->token_ptr->type){
                case CPP_TOKEN_BRACE_CLOSE:
                {
                    if (wrap_state->wrap_x.paren_safe_top == 0){
                        current_shift -= wrap_state->tab_indent_amount;
                    }
                }break;
            }
        }
    }
    
    return(current_shift);
}

internal void
file_measure_wraps(Models *models, Editing_File *file, f32 font_height, f32 *cp_adv, f32 byte_adv){
    General_Memory *general = &models->mem.general;
    Partition *part = &models->mem.part;
    
    Temp_Memory temp = begin_temp_memory(part);
    
    file_allocate_wraps_as_needed(general, file);
    file_allocate_indents_as_needed(general, file, file->state.buffer.line_count);
    file_allocate_wrap_positions_as_needed(general, file, file->state.buffer.line_count);
    
    Buffer_Measure_Wrap_Params params;
    params.buffer          = &file->state.buffer;
    params.wrap_line_index = file->state.wrap_line_index;
    params.cp_adv          = cp_adv;
    params.byte_adv        = byte_adv;
    params.virtual_white   = file->settings.virtual_white;
    
    f32 width = (f32)file->settings.display_width;
    f32 minimum_base_width = (f32)file->settings.minimum_base_display_width;
    
    i32 size = buffer_size(params.buffer);
    
    Buffer_Measure_Wrap_State state = {0};
    Buffer_Layout_Stop stop = {0};
    
    f32 edge_tolerance = 50.f;
    if (edge_tolerance > width){
        edge_tolerance = width;
    }
    
    f32 line_shift = 0.f;
    b32 do_wrap = 0;
    i32 wrap_unit_end = 0;
    
    i32 wrap_position_index = 0;
    file->state.wrap_positions[wrap_position_index++] = 0;
    
    Code_Wrap_State wrap_state = {0};
    
    b32 use_tokens = 0;
    
    Wrap_Indent_Pair *wrap_indent_marks = 0;
    Potential_Wrap_Indent_Pair *potential_marks = 0;
    i32 max_wrap_indent_mark = 0;
    
    if (params.virtual_white && file->state.tokens_complete && !file->state.still_lexing){
        wrap_state_init(&wrap_state, file, cp_adv, byte_adv);
        use_tokens = 1;
        
        potential_marks = push_array(part, Potential_Wrap_Indent_Pair, floor32(width));
        
        max_wrap_indent_mark = partition_remaining(part)/sizeof(Wrap_Indent_Pair);
        wrap_indent_marks = push_array(part, Wrap_Indent_Pair, max_wrap_indent_mark);
    }
    
    i32 real_count = 0;
    i32 potential_count = 0;
    i32 stage = 0;
    
    do{
        stop = buffer_measure_wrap_y(&state, params, line_shift, do_wrap, wrap_unit_end);
        
        switch (stop.status){
            case BLStatus_NeedWrapDetermination:
            {
                if (use_tokens){
                    if (stage == 0){
                        do_wrap = 0;
                        wrap_unit_end = wrap_indent_marks[stage+1].wrap_position;
                        ++stage;
                    }
                    else{
                        do_wrap = 1;
                        wrap_unit_end = wrap_indent_marks[stage+1].wrap_position;
                        file_allocate_wrap_positions_as_needed(general, file, wrap_position_index);
                        file->state.wrap_positions[wrap_position_index++] = stop.pos;
                    }
                }
                else{
                    Gap_Buffer_Stream stream = {0};
                    
                    i32 word_stage = 0;
                    i32 i = stop.pos;
                    f32 x = stop.x;
                    f32 self_x = 0;
                    if (buffer_stringify_loop(&stream, params.buffer, i, size)){
                        b32 still_looping = 0;
                        do{
                            for (; i < stream.end; ++i){
                                u8 ch = stream.data[i];
                                
                                switch (word_stage){
                                    case 0:
                                    {
                                        if (char_is_whitespace(ch)){
                                            word_stage = 1;
                                        }
                                        else{
                                            f32 adv = params.cp_adv[ch];
                                            x += adv;
                                            self_x += adv;
                                            if (self_x > width){
                                                goto doublebreak;
                                            }
                                        }
                                    }break;
                                    
                                    case 1:
                                    {
                                        if (!char_is_whitespace(ch)){
                                            goto doublebreak;
                                        }
                                    }break;
                                }
                            }
                            still_looping = buffer_stringify_next(&stream);
                        }while(still_looping);
                    }
                    
                    doublebreak:;
                    wrap_unit_end = i;
                    if (x > width){
                        do_wrap = 1;
                        file_allocate_wrap_positions_as_needed(general, file, wrap_position_index);
                        file->state.wrap_positions[wrap_position_index++] = stop.pos;
                    }
                    else{
                        do_wrap = 0;
                    }
                }
            }break;
            
            case BLStatus_NeedWrapLineShift:
            case BLStatus_NeedLineShift:
            {
                f32 current_width = width;
                
                if (use_tokens){
                    Code_Wrap_State original_wrap_state = wrap_state;
                    i32 next_line_start = buffer_size(params.buffer);
                    if (stop.line_index+1 < params.buffer->line_count){
                        next_line_start = params.buffer->line_starts[stop.line_index+1];
                    }
                    
                    f32 base_adjusted_width = wrap_state.wrap_x.base_x + minimum_base_width;
                    
                    if (minimum_base_width != 0 && current_width < base_adjusted_width){
                        current_width = base_adjusted_width;
                    }
                    
                    if (stop.status == BLStatus_NeedLineShift){
                        real_count = 0;
                        potential_count = 0;
                        stage = 0;
                        
                        b32 adjust_top_to_this = 0;
                        f32 current_shift = get_current_shift(&wrap_state, next_line_start, &adjust_top_to_this);
                        
                        if (adjust_top_to_this){
                            wrap_state_set_top(&wrap_state, current_shift);
                        }
                        
                        wrap_indent_marks[real_count].wrap_position = 0;
                        wrap_indent_marks[real_count].line_shift = clamp_bottom(0.f, current_shift);
                        ++real_count;
                        
                        wrap_state.wrap_x.base_x = wrap_state.wrap_x.paren_nesting[0];
                        
                        for (; wrap_state.token_ptr < wrap_state.end_token; ){
                            Code_Wrap_Step step = {0};
                            b32 emit_comment_position = 0;
                            b32 first_word = 1;
                            
                            if (wrap_state.token_ptr->type == CPP_TOKEN_COMMENT || 
                                wrap_state.token_ptr->type == CPP_TOKEN_STRING_CONSTANT){
                                i32 i = wrap_state.token_ptr->start;
                                i32 end_i = i + wrap_state.token_ptr->size;
                                
                                if (i < wrap_state.i){
                                    i = wrap_state.i;
                                }
                                
                                if (end_i > wrap_state.next_line_start){
                                    end_i = wrap_state.next_line_start;
                                }
                                
                                f32 x = wrap_state.x;
                                
                                step.position_start = i;
                                step.start_x = x;
                                step.this_token = wrap_state.token_ptr;
                                
                                Gap_Buffer_Stream stream = {0};
                                
                                Potential_Wrap_Indent_Pair potential_wrap = {0};
                                potential_wrap.wrap_position = i;
                                potential_wrap.line_shift = x;
                                potential_wrap.wrappable_score = 5;
                                potential_wrap.wrap_x = x;
                                potential_wrap.adjust_top_to_this = 0;
                                
                                if (buffer_stringify_loop(&stream, params.buffer, i, end_i)){
                                    b32 still_looping = 1;
                                    
                                    while(still_looping){
                                        for (; i < stream.end; ++i){
                                            u8 ch = stream.data[i];
                                            
                                            if (!char_is_whitespace(ch)){
                                                goto doublebreak_stage_vspace;
                                            }
                                        }
                                        still_looping = buffer_stringify_next(&stream);
                                    }
                                    doublebreak_stage_vspace:;
                                    
                                    do{
                                        while (still_looping){
                                            for (; i < stream.end; ++i){
                                                u8 ch = stream.data[i];
                                                
                                                if (char_is_whitespace(ch)){
                                                    goto doublebreak_stage1;
                                                }
                                                
                                                f32 adv = params.cp_adv[ch];
                                                x += adv;
                                                if (!first_word && x > current_width){
                                                    emit_comment_position = 1;
                                                    goto doublebreak_stage1;
                                                }
                                            }
                                            still_looping = buffer_stringify_next(&stream);
                                        }
                                        doublebreak_stage1:;
                                        first_word = 0;
                                        
                                        if (emit_comment_position){
                                            step.position_end = i;
                                            step.final_x = x;
                                            goto finished_comment_split;
                                        }
                                        
                                        while(still_looping){
                                            for (; i < stream.end; ++i){
                                                u8 ch = stream.data[i];
                                                
                                                if (!char_is_whitespace(ch)){
                                                    goto doublebreak_stage2;
                                                }
                                                
                                                f32 adv = params.cp_adv[ch];
                                                x += adv;
                                            }
                                            still_looping = buffer_stringify_next(&stream);
                                        }
                                        doublebreak_stage2:;
                                        
                                        potential_wrap.wrap_position = i;
                                        potential_wrap.wrap_x = x;
                                    }while(still_looping);
                                }
                                
                                finished_comment_split:;
                                if (emit_comment_position){
                                    potential_marks[potential_count] = potential_wrap;
                                    ++potential_count;
                                }
                            }
                            
                            if (!emit_comment_position){
                                step = wrap_state_consume_token(&wrap_state, next_line_start-1);
                            }
                            
                            b32 need_to_choose_a_wrap = 0;
                            if (step.final_x > current_width){
                                need_to_choose_a_wrap = 1;
                            }
                            
                            adjust_top_to_this = 0;
                            current_shift = get_current_shift(&wrap_state, next_line_start, &adjust_top_to_this);
                            
                            b32 next_token_is_on_line = 0;
                            if (wrap_state.token_ptr->start < next_line_start){
                                next_token_is_on_line = 1;
                            }
                            
                            i32 wrap_position = step.position_end;
                            f32 wrap_x = step.final_x;
                            if (wrap_state.token_ptr->start > step.position_start && wrap_position < wrap_state.token_ptr->start && next_token_is_on_line){
                                wrap_position = wrap_state.token_ptr->start;
                            }
                            
                            if (!need_to_choose_a_wrap){
                                i32 wrappable_score = 1;
                                
                                Cpp_Token *this_token = step.this_token;
                                Cpp_Token *next_token = wrap_state.token_ptr;
                                
                                Cpp_Token_Type this_type = this_token->type;
                                Cpp_Token_Type next_type = CPP_TOKEN_JUNK;
                                
                                u16 this_flags = this_token->flags;
                                u16 next_flags = 0;
                                
                                if (this_token == next_token || !next_token_is_on_line){
                                    next_token = 0;
                                }
                                
                                if (next_token){
                                    next_type = next_token->type;
                                    next_flags = next_token->flags;
                                }
                                
                                i32 this_stickieness = stickieness_guess(this_type, next_type, this_flags, next_flags, 1);
                                
                                i32 next_stickieness = 0;
                                if (next_token){
                                    next_stickieness = stickieness_guess(next_type, this_type, next_flags, this_flags, 0);
                                }
                                
                                i32 general_stickieness = this_stickieness;
                                if (general_stickieness < next_stickieness){
                                    general_stickieness = next_stickieness;
                                }
                                
                                if (wrap_state.wrap_x.paren_top != 0 && this_type == CPP_TOKEN_COMMA){
                                    general_stickieness = 0;
                                }
                                
                                wrappable_score = 64*50;
                                wrappable_score += 101 - general_stickieness - wrap_state.wrap_x.paren_safe_top*80;
                                
                                potential_marks[potential_count].wrap_position = wrap_position;
                                potential_marks[potential_count].line_shift = current_shift;
                                potential_marks[potential_count].wrappable_score = wrappable_score;
                                potential_marks[potential_count].wrap_x = wrap_x;
                                potential_marks[potential_count].adjust_top_to_this = adjust_top_to_this;
                                ++potential_count;
                            }
                            
                            if (need_to_choose_a_wrap){
                                if (potential_count == 0){
                                    wrap_indent_marks[real_count].wrap_position = wrap_position;
                                    wrap_indent_marks[real_count].line_shift = current_shift;
                                    ++real_count;
                                }
                                else{
                                    i32 i = 0, best_i = 0;
                                    i32 best_score = -1;
                                    f32 best_x_shift = 0;
                                    
                                    f32 x_gain_threshold = 18.f;
                                    
                                    for (; i < potential_count; ++i){
                                        i32 this_score = potential_marks[i].wrappable_score;
                                        f32 x_shift = potential_marks[i].wrap_x - potential_marks[i].line_shift;
                                        
                                        f32 x_shift_adjusted = x_shift - x_gain_threshold;
                                        f32 x_left_over = step.final_x - x_shift;
                                        
                                        if (x_shift_adjusted < 0){
                                            this_score = 0;
                                        }
                                        else if (x_left_over <= x_gain_threshold){
                                            this_score = 1;
                                        }
                                        
                                        if (this_score > best_score){
                                            best_score = this_score;
                                            best_x_shift = x_shift;
                                            best_i = i;
                                        }
                                        else if (this_score == best_score && x_shift > best_x_shift){
                                            best_x_shift = x_shift;
                                            best_i = i;
                                        }
                                    }
                                    
                                    i32 wrap_position = potential_marks[best_i].wrap_position;
                                    f32 line_shift = potential_marks[best_i].line_shift;
                                    b32 adjust_top_to_this = potential_marks[best_i].adjust_top_to_this;
                                    wrap_indent_marks[real_count].wrap_position = wrap_position;
                                    wrap_indent_marks[real_count].line_shift    = line_shift;
                                    ++real_count;
                                    potential_count = 0;
                                    
                                    wrap_state = original_wrap_state;
                                    for (;;){
                                        step = wrap_state_consume_token(&wrap_state, wrap_position);
                                        if (step.position_end >= wrap_position){
                                            break;
                                        }
                                    }
                                    
                                    wrap_state_set_x(&wrap_state, line_shift);
                                    wrap_state_set_i(&wrap_state, wrap_position);
                                    if (adjust_top_to_this){
                                        wrap_state_set_top(&wrap_state, line_shift);
                                    }
                                    
                                    original_wrap_state = wrap_state;
                                }
                            }
                            
                            if (step.position_end >= next_line_start-1){
                                break;
                            }
                        }
                        
                        wrap_indent_marks[real_count].wrap_position = next_line_start;
                        wrap_indent_marks[real_count].line_shift    = 0;
                        ++real_count;
                        
                        for (i32 l = 0; wrap_state.i < next_line_start && l < 3; ++l){
                            wrap_state_consume_token(&wrap_state, next_line_start);
                        }
                    }
                    
                    line_shift = wrap_indent_marks[stage].line_shift;
                    
                    if (stage > 0){
                        ++stage;
                    }
                    
                    if (line_shift < 0){
                        line_shift = 0;
                    }
                    
                }
                else{
                    line_shift = 0.f;
                }
                
                if (line_shift > current_width - edge_tolerance){
                    line_shift = current_width - edge_tolerance;
                }
                
                if (stop.wrap_line_index >= file->state.line_indent_max){
                    file_allocate_indents_as_needed(general, file, stop.wrap_line_index);
                }
                
                file->state.line_indents[stop.wrap_line_index] = line_shift;
                file->state.wrap_line_count = stop.wrap_line_index;
            }break;
        }
    }while(stop.status != BLStatus_Finished);
    
    ++file->state.wrap_line_count;
    
    file_allocate_wrap_positions_as_needed(general, file, wrap_position_index);
    file->state.wrap_positions[wrap_position_index++] = size;
    file->state.wrap_position_count = wrap_position_index;
    
    end_temp_memory(temp);
}

internal void
file_measure_wraps_and_fix_cursor(Models *models, Editing_File *file, f32 font_height, f32 *cp_adv, f32 byte_adv){
    file_measure_wraps(models, file, font_height, cp_adv, byte_adv);
    file_update_cursor_positions(models, file);
}

internal void
file_set_width(Models *models, Editing_File *file, i32 display_width, f32 font_height, f32 *cp_adv, f32 byte_adv){
    file->settings.display_width = display_width;
    file_measure_wraps_and_fix_cursor(models, file, font_height, cp_adv, byte_adv);
}

internal void
file_set_min_base_width(Models *models, Editing_File *file, i32 minimum_base_display_width, f32 font_height, f32 *cp_adv, f32 byte_adv){
    file->settings.minimum_base_display_width = minimum_base_display_width;
    file_measure_wraps_and_fix_cursor(models, file, font_height, cp_adv, byte_adv);
}

//
//
//

internal void
file_create_from_string(System_Functions *system, Models *models, Editing_File *file, String val, b8 read_only = 0){
    
    Font_Set *font_set = models->font_set;
    General_Memory *general = &models->mem.general;
    Partition *part = &models->mem.part;
    Open_File_Hook_Function *hook_open_file = models->hook_open_file;
    Application_Links *app_links = &models->app_links;
    
    file->state = null_editing_file_state;
    Gap_Buffer_Init init = buffer_begin_init(&file->state.buffer, val.str, val.size);
    for (; buffer_init_need_more(&init); ){
        i32 page_size = buffer_init_page_size(&init);
        page_size = l_round_up_i32(page_size, KB(4));
        if (page_size < KB(4)){
            page_size = KB(4);
        }
        void *data = general_memory_allocate(general, page_size);
        buffer_init_provide_page(&init, data, page_size);
    }
    
    i32 scratch_size = partition_remaining(part);
    Assert(scratch_size > 0);
    b32 init_success = buffer_end_init(&init, part->base + part->pos, scratch_size);
    AllowLocal(init_success); Assert(init_success);
    
    if (buffer_size(&file->state.buffer) < val.size){
        file->settings.dos_write_mode = 1;
    }
    file_synchronize_times(system, file);
    
    // TODO(allen): batch some of these together so we can avoid
    // making so many passes over the buffer?
    file_measure_starts(general, &file->state.buffer);
    
    file_allocate_character_starts_as_needed(general, file);
    buffer_measure_character_starts(&file->state.buffer, file->state.character_starts, 0, file->settings.virtual_white);
    
    i16 font_id = models->global_font.font_id;
    file->settings.font_id = font_id;
    Render_Font *font = get_font_info(font_set, font_id)->font;
    
    file_measure_wraps(models, file, (f32)font->height, font->codepoint_advance_data, font->byte_advance);
    
    file->settings.read_only = read_only;
    if (!read_only){
        // TODO(allen): Redo undo system (if you don't mind the pun)
        i32 request_size = KB(64);
        file->state.undo.undo.max = request_size;
        file->state.undo.undo.strings = (u8*)general_memory_allocate(general, request_size);
        file->state.undo.undo.edit_max = request_size / sizeof(Edit_Step);
        file->state.undo.undo.edits = (Edit_Step*)general_memory_allocate(general, request_size);
        
        file->state.undo.redo.max = request_size;
        file->state.undo.redo.strings = (u8*)general_memory_allocate(general, request_size);
        file->state.undo.redo.edit_max = request_size / sizeof(Edit_Step);
        file->state.undo.redo.edits = (Edit_Step*)general_memory_allocate(general, request_size);
        
        file->state.undo.history.max = request_size;
        file->state.undo.history.strings = (u8*)general_memory_allocate(general, request_size);
        file->state.undo.history.edit_max = request_size / sizeof(Edit_Step);
        file->state.undo.history.edits = (Edit_Step*)general_memory_allocate(general, request_size);
        
        file->state.undo.children.max = request_size;
        file->state.undo.children.strings = (u8*)general_memory_allocate(general, request_size);
        file->state.undo.children.edit_max = request_size / sizeof(Buffer_Edit);
        file->state.undo.children.edits = (Buffer_Edit*)general_memory_allocate(general, request_size);
        
        file->state.undo.history_block_count = 1;
        file->state.undo.history_head_block = 0;
        file->state.undo.current_block_normal = 1;
    }
    
    if (hook_open_file){
        hook_open_file(app_links, file->id.id);
    }
    file->settings.is_initialized = 1;
}

internal void
file_close(System_Functions *system, General_Memory *general, Editing_File *file){
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_array.tokens){
            general_memory_free(general, file->state.swap_array.tokens);
            file->state.swap_array.tokens = 0;
        }
    }
    if (file->state.token_array.tokens){
        general_memory_free(general, file->state.token_array.tokens);
    }
    
    Gap_Buffer *buffer = &file->state.buffer;
    if (buffer->data){
        general_memory_free(general, buffer->data);
        general_memory_free(general, buffer->line_starts);
    }
    
    general_memory_free(general, file->state.wrap_line_index);
    general_memory_free(general, file->state.character_starts);
    general_memory_free(general, file->state.line_indents);
    
    if (file->state.undo.undo.edits){
        general_memory_free(general, file->state.undo.undo.strings);
        general_memory_free(general, file->state.undo.undo.edits);
        
        general_memory_free(general, file->state.undo.redo.strings);
        general_memory_free(general, file->state.undo.redo.edits);
        
        general_memory_free(general, file->state.undo.history.strings);
        general_memory_free(general, file->state.undo.history.edits);
        
        general_memory_free(general, file->state.undo.children.strings);
        general_memory_free(general, file->state.undo.children.edits);
    }
}

struct Shift_Information{
    i32 start, end, amount;
};

// TODO(allen): I want this code audited soon
internal
Job_Callback_Sig(job_full_lex){
    Editing_File *file = (Editing_File*)data[0];
    General_Memory *general = (General_Memory*)data[1];
    
    Gap_Buffer *buffer = &file->state.buffer;
    i32 text_size = buffer_size(buffer);
    
    i32 aligned_buffer_size = (text_size + 3)&(~3);
    
    while (memory->size < aligned_buffer_size){
        system->grow_thread_memory(memory);
    }
    
    Cpp_Token_Array tokens;
    tokens.tokens = (Cpp_Token*)(memory->data);
    tokens.max_count = memory->size / sizeof(Cpp_Token);
    tokens.count = 0;
    
    b32 still_lexing = 1;
    
    Cpp_Lex_Data lex = cpp_lex_data_init();
    
    // TODO(allen): deduplicate this against relex
    char *chunks[3];
    i32 chunk_sizes[3];
    
    chunks[0] = buffer->data;
    chunk_sizes[0] = buffer->size1;
    
    chunks[1] = buffer->data + buffer->size1 + buffer->gap_size;
    chunk_sizes[1] = buffer->size2;
    
    chunks[2] = 0;
    chunk_sizes[2] = 0;
    
    i32 chunk_index = 0;
    
    do{
        char *chunk = chunks[chunk_index];
        i32 chunk_size = chunk_sizes[chunk_index];
        
        i32 result =
            cpp_lex_step(&lex, chunk, chunk_size, text_size, &tokens, 2048);
        
        switch (result){
            case LexResult_NeedChunk: ++chunk_index; break;
            
            case LexResult_NeedTokenMemory:
            if (system->check_cancel(thread)){
                return;
            }
            system->grow_thread_memory(memory);
            tokens.tokens = (Cpp_Token*)(memory->data);
            tokens.max_count = memory->size / sizeof(Cpp_Token);
            break;
            
            case LexResult_HitTokenLimit:
            if (system->check_cancel(thread)){
                return;
            }
            break;
            
            case LexResult_Finished: still_lexing = 0; break;
        }
    } while (still_lexing);
    
    i32 new_max = l_round_up_i32(tokens.count+1, KB(1));
    
    system->acquire_lock(FRAME_LOCK);
    {
        Assert(file->state.swap_array.tokens == 0);
        file->state.swap_array.tokens = (Cpp_Token*)general_memory_allocate(general, new_max*sizeof(Cpp_Token));
    }
    system->release_lock(FRAME_LOCK);
    
    u8 *dest = (u8*)file->state.swap_array.tokens;
    u8 *src = (u8*)tokens.tokens;
    
    memcpy(dest, src, tokens.count*sizeof(Cpp_Token));
    
    system->acquire_lock(FRAME_LOCK);
    {
        Cpp_Token_Array *file_token_array = &file->state.token_array;
        file_token_array->count = tokens.count;
        file_token_array->max_count = new_max;
        if (file_token_array->tokens){
            general_memory_free(general, file_token_array->tokens);
        }
        file_token_array->tokens = file->state.swap_array.tokens;
        file->state.swap_array.tokens = 0;
    }
    system->release_lock(FRAME_LOCK);
    
    // NOTE(allen): These are outside the locked section because I don't
    // think getting these out of order will cause critical bugs, and I
    // want to minimize what's done in locked sections.
    file->state.tokens_complete = 1;
    file->state.still_lexing = 0;
}

internal void
file_kill_tokens(System_Functions *system, General_Memory *general, Editing_File *file){
    file->settings.tokens_exist = 0;
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_array.tokens){
            general_memory_free(general, file->state.swap_array.tokens);
            file->state.swap_array.tokens = 0;
        }
    }
    if (file->state.token_array.tokens){
        general_memory_free(general, file->state.token_array.tokens);
    }
    file->state.tokens_complete = 0;
    file->state.token_array = null_cpp_token_array;
}

internal void
file_first_lex_parallel(System_Functions *system, Mem_Options *mem, Editing_File *file){
    General_Memory *general = &mem->general;
    file->settings.tokens_exist = 1;
    
    if (file->is_loading == 0 && file->state.still_lexing == 0){
        Assert(file->state.token_array.tokens == 0);
        
        file->state.tokens_complete = 0;
        file->state.still_lexing = 1;
        
        Job_Data job;
        job.callback = job_full_lex;
        job.data[0] = file;
        job.data[1] = general;
        file->state.lex_job = system->post_job(BACKGROUND_THREADS, job);
    }
}

internal void
file_first_lex_serial(Mem_Options *mem, Editing_File *file){
    Partition *part = &mem->part;
    General_Memory *general = &mem->general;
    file->settings.tokens_exist = 1;
    
    Assert(file->state.still_lexing == 0);
    
    if (file->is_loading == 0){
        Assert(file->state.token_array.tokens == 0);
        
        {
            Temp_Memory temp = begin_temp_memory(part);
            
            Gap_Buffer *buffer = &file->state.buffer;
            i32 text_size = buffer_size(buffer);
            
            i32 mem_size = partition_remaining(part);
            
            Cpp_Token_Array tokens;
            tokens.max_count = mem_size / sizeof(Cpp_Token);
            tokens.count = 0;
            tokens.tokens = push_array(part, Cpp_Token, tokens.max_count);
            
            b32 still_lexing = 1;
            
            Cpp_Lex_Data lex = cpp_lex_data_init();
            
            // TODO(allen): deduplicate this against relex
            char *chunks[3];
            i32 chunk_sizes[3];
            
            chunks[0] = buffer->data;
            chunk_sizes[0] = buffer->size1;
            
            chunks[1] = buffer->data + buffer->size1 + buffer->gap_size;
            chunk_sizes[1] = buffer->size2;
            
            chunks[2] = 0;
            chunk_sizes[2] = 0;
            
            i32 chunk_index = 0;
            
            Cpp_Token_Array *swap_array = &file->state.swap_array;
            
            do{
                char *chunk = chunks[chunk_index];
                i32 chunk_size = chunk_sizes[chunk_index];
                
                i32 result = cpp_lex_step(&lex, chunk, chunk_size, text_size, &tokens, NO_OUT_LIMIT);
                
                switch (result){
                    case LexResult_NeedChunk: ++chunk_index; break;
                    
                    
                    case LexResult_Finished:
                    case LexResult_NeedTokenMemory:
                    {
                        u32 new_max = l_round_up_u32(tokens.count+1, KB(1));
                        u32 new_mem_max = new_max*sizeof(Cpp_Token);
                        u32 old_mem_max = swap_array->count*sizeof(Cpp_Token);
                        if (swap_array->tokens == 0){
                            swap_array->tokens = (Cpp_Token*)general_memory_allocate(general, new_mem_max);
                        }
                        else{
                            swap_array->tokens = (Cpp_Token*)
                                general_memory_reallocate(general, swap_array->tokens, old_mem_max, new_mem_max);
                        }
                        swap_array->max_count = new_max;
                        
                        memcpy(swap_array->tokens + swap_array->count, tokens.tokens, tokens.count*sizeof(Cpp_Token));
                        swap_array->count += tokens.count;
                        tokens.count = 0;
                        
                        if (result == LexResult_Finished){
                            still_lexing = 0;
                        }
                    }break;
                    
                    case LexResult_HitTokenLimit: InvalidCodePath;
                }
            } while (still_lexing);
            
            Cpp_Token_Array *token_array = &file->state.token_array;
            token_array->count = swap_array->count;
            token_array->max_count = swap_array->max_count;
            if (token_array->tokens != 0){
                general_memory_free(general, token_array->tokens);
            }
            token_array->tokens = swap_array->tokens;
            
            swap_array->tokens = 0;
            swap_array->count = 0;
            swap_array->max_count = 0;
            
            file->state.tokens_complete = true;
            file->state.still_lexing = false;
            
            end_temp_memory(temp);
        }
        
        file->state.tokens_complete = 1;
    }
}

internal b32
file_relex_parallel(System_Functions *system, Mem_Options *mem, Editing_File *file, i32 start_i, i32 end_i, i32 shift_amount){
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    
    if (file->state.token_array.tokens == 0){
        file_first_lex_parallel(system, mem, file);
        return(0);
    }
    
    b32 result = 1;
    b32 inline_lex = !file->state.still_lexing;
    if (inline_lex){
        Gap_Buffer *buffer = &file->state.buffer;
        i32 extra_tolerance = 100;
        
        Cpp_Token_Array *array = &file->state.token_array;
        Cpp_Relex_Range relex_range = cpp_get_relex_range(array, start_i, end_i);
        
        i32 relex_space_size =
            relex_range.end_token_index - relex_range.start_token_index + extra_tolerance;
        
        Temp_Memory temp = begin_temp_memory(part);
        Cpp_Token_Array relex_array;
        relex_array.count = 0;
        relex_array.max_count = relex_space_size;
        relex_array.tokens = push_array(part, Cpp_Token, relex_array.max_count);
        
        i32 size = buffer_size(buffer);
        
        Cpp_Relex_Data state = cpp_relex_init(array, start_i, end_i, shift_amount);
        
        char *chunks[3];
        i32 chunk_sizes[3];
        
        chunks[0] = buffer->data;
        chunk_sizes[0] = buffer->size1;
        
        chunks[1] = buffer->data + buffer->size1 + buffer->gap_size;
        chunk_sizes[1] = buffer->size2;
        
        chunks[2] = 0;
        chunk_sizes[2] = 0;
        
        i32 chunk_index = 0;
        char *chunk = chunks[chunk_index];
        i32 chunk_size = chunk_sizes[chunk_index];
        
        while (!cpp_relex_is_start_chunk(&state, chunk, chunk_size)){
            ++chunk_index;
            chunk = chunks[chunk_index];
            chunk_size = chunk_sizes[chunk_index];
        }
        
        for(;;){
            Cpp_Lex_Result lex_result =
                cpp_relex_step(&state, chunk, chunk_size, size, array, &relex_array);
            
            switch (lex_result){
                case LexResult_NeedChunk:
                ++chunk_index;
                chunk = chunks[chunk_index];
                chunk_size = chunk_sizes[chunk_index];
                break;
                
                case LexResult_NeedTokenMemory:
                inline_lex = 0;
                goto doublebreak;
                
                case LexResult_Finished:
                goto doublebreak;
            }
        }
        doublebreak:;
        
        if (inline_lex){
            i32 new_count = cpp_relex_get_new_count(&state, array->count, &relex_array);
            if (new_count > array->max_count){
                i32 new_max = l_round_up_i32(new_count, KB(1));
                array->tokens = (Cpp_Token*)
                    general_memory_reallocate(general, array->tokens, array->count*sizeof(Cpp_Token), new_max*sizeof(Cpp_Token));
                array->max_count = new_max;
            }
            
            cpp_relex_complete(&state, array, &relex_array);
        }
        else{
            cpp_relex_abort(&state, array);
        }
        
        end_temp_memory(temp);
    }
    
    if (!inline_lex){
        Cpp_Token_Array *array = &file->state.token_array;
        Cpp_Get_Token_Result get_token_result = cpp_get_token(*array, end_i);
        i32 end_token_i = get_token_result.token_index;
        
        if (end_token_i < 0){
            end_token_i = 0;
        }
        else if (end_i > array->tokens[end_token_i].start){
            ++end_token_i;
        }
        
        cpp_shift_token_starts(array, end_token_i, shift_amount);
        --end_token_i;
        if (end_token_i >= 0){
            Cpp_Token *token = array->tokens + end_token_i;
            if (token->start < end_i && token->start + token->size > end_i){
                token->size += shift_amount;
            }
        }
        
        file->state.still_lexing = 1;
        
        Job_Data job;
        job.callback = job_full_lex;
        job.data[0] = file;
        job.data[1] = general;
        file->state.lex_job = system->post_job(BACKGROUND_THREADS, job);
        result = 0;
    }
    
    return(result);
}

internal b32
file_relex_serial(Mem_Options *mem, Editing_File *file, i32 start_i, i32 end_i, i32 shift_amount){
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    
    if (file->state.token_array.tokens == 0){
        file_first_lex_serial(mem, file);
        return(1);
    }
    
    Assert(!file->state.still_lexing);
    
    Gap_Buffer *buffer = &file->state.buffer;
    Cpp_Token_Array *array = &file->state.token_array;
    
    Temp_Memory temp = begin_temp_memory(part);
    Cpp_Token_Array relex_array;
    relex_array.count = 0;
    relex_array.max_count = partition_remaining(part) / sizeof(Cpp_Token);
    relex_array.tokens = push_array(part, Cpp_Token, relex_array.max_count);
    
    i32 size = buffer_size(buffer);
    
    Cpp_Relex_Data state = cpp_relex_init(array, start_i, end_i, shift_amount);
    
    char *chunks[3];
    i32 chunk_sizes[3];
    
    chunks[0] = buffer->data;
    chunk_sizes[0] = buffer->size1;
    
    chunks[1] = buffer->data + buffer->size1 + buffer->gap_size;
    chunk_sizes[1] = buffer->size2;
    
    chunks[2] = 0;
    chunk_sizes[2] = 0;
    
    i32 chunk_index = 0;
    char *chunk = chunks[chunk_index];
    i32 chunk_size = chunk_sizes[chunk_index];
    
    while (!cpp_relex_is_start_chunk(&state, chunk, chunk_size)){
        ++chunk_index;
        chunk = chunks[chunk_index];
        chunk_size = chunk_sizes[chunk_index];
    }
    
    for(;;){
        Cpp_Lex_Result lex_result = cpp_relex_step(&state, chunk, chunk_size, size, array, &relex_array);
        
        switch (lex_result){
            case LexResult_NeedChunk:
            ++chunk_index;
            chunk = chunks[chunk_index];
            chunk_size = chunk_sizes[chunk_index];
            break;
            
            case LexResult_NeedTokenMemory: InvalidCodePath;
            
            case LexResult_Finished: goto doublebreak;
        }
    }
    doublebreak:;
    
    i32 new_count = cpp_relex_get_new_count(&state, array->count, &relex_array);
    if (new_count > array->max_count){
        i32 new_max = l_round_up_i32(new_count, KB(1));
        array->tokens = (Cpp_Token*)general_memory_reallocate(general, array->tokens, array->count*sizeof(Cpp_Token), new_max*sizeof(Cpp_Token));
        array->max_count = new_max;
    }
    
    cpp_relex_complete(&state, array, &relex_array);
    
    end_temp_memory(temp);
    
    return(1);
}

internal void
undo_stack_grow_string(General_Memory *general, Edit_Stack *stack, i32 extra_size){
    i32 old_max = stack->max;
    u8 *old_str = stack->strings;
    i32 new_max = old_max*2 + extra_size;
    u8 *new_str = (u8*)general_memory_reallocate(general, old_str, old_max, new_max);
    stack->strings = new_str;
    stack->max = new_max;
}

internal void
undo_stack_grow_edits(General_Memory *general, Edit_Stack *stack){
    i32 old_max = stack->edit_max;
    Edit_Step *old_eds = stack->edits;
    i32 new_max = old_max*2 + 2;
    Edit_Step *new_eds = (Edit_Step*)general_memory_reallocate(general, old_eds, old_max*sizeof(Edit_Step), new_max*sizeof(Edit_Step));
    stack->edits = new_eds;
    stack->edit_max = new_max;
}

internal void
child_stack_grow_string(General_Memory *general, Small_Edit_Stack *stack, i32 extra_size){
    i32 old_max = stack->max;
    u8 *old_str = stack->strings;
    i32 new_max = old_max*2 + extra_size;
    u8 *new_str = (u8*)general_memory_reallocate(general, old_str, old_max, new_max);
    stack->strings = new_str;
    stack->max = new_max;
}

internal void
child_stack_grow_edits(General_Memory *general, Small_Edit_Stack *stack, i32 amount){
    i32 old_max = stack->edit_max;
    Buffer_Edit *old_eds = stack->edits;
    i32 new_max = old_max*2 + amount;
    Buffer_Edit *new_eds = (Buffer_Edit*)general_memory_reallocate(general, old_eds, old_max*sizeof(Buffer_Edit), new_max*sizeof(Buffer_Edit));
    stack->edits = new_eds;
    stack->edit_max = new_max;
}

internal i32
undo_children_push(General_Memory *general, Small_Edit_Stack *children, Buffer_Edit *edits, i32 edit_count, u8 *strings, i32 string_size){
    i32 result = children->edit_count;
    if (children->edit_count + edit_count > children->edit_max){
        child_stack_grow_edits(general, children, edit_count);
    }
    
    if (children->size + string_size > children->max){
        child_stack_grow_string(general, children, string_size);
    }
    
    memcpy(children->edits + children->edit_count, edits, edit_count*sizeof(Buffer_Edit));
    memcpy(children->strings + children->size, strings, string_size);
    
    Buffer_Edit *edit = children->edits + children->edit_count;
    i32 start_pos = children->size;
    for (i32 i = 0; i < edit_count; ++i, ++edit){
        edit->str_start += start_pos;
    }
    
    children->edit_count += edit_count;
    children->size += string_size;
    
    return result;
}

struct Edit_Spec{
    u8 *str;
    Edit_Step step;
};

internal Edit_Step*
file_post_undo(General_Memory *general, Editing_File *file, Edit_Step step, b32 do_merge, b32 can_merge){
    if (step.type == ED_NORMAL){
        file->state.undo.redo.size = 0;
        file->state.undo.redo.edit_count = 0;
    }
    
    Edit_Stack *undo = &file->state.undo.undo;
    Edit_Step *result = 0;
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + undo->size > undo->max){
            undo_stack_grow_string(general, undo, step.edit.end - step.edit.start);
        }
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv, (char*)undo->strings, &undo->size, undo->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.can_merge = (b8)can_merge;
        inv_step.type = ED_UNDO;
        
        b32 did_merge = 0;
        if (do_merge && undo->edit_count > 0){
            Edit_Step prev = undo->edits[undo->edit_count-1];
            if (prev.can_merge && inv_step.edit.len == 0 && prev.edit.len == 0){
                if (prev.edit.end == inv_step.edit.start){
                    did_merge = 1;
                    inv_step.edit.start = prev.edit.start;
                }
            }
        }
        
        if (did_merge){
            result = undo->edits + (undo->edit_count-1);
            *result = inv_step;
        }
        else{
            if (undo->edit_count == undo->edit_max){
                undo_stack_grow_edits(general, undo);
            }
            
            result = undo->edits + (undo->edit_count++);
            *result = inv_step;
        }
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = ED_UNDO;
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.child_count = step.inverse_child_count;
        inv_step.inverse_child_count = step.child_count;
        
        if (undo->edit_count == undo->edit_max){
            undo_stack_grow_edits(general, undo);
        }
        result = undo->edits + (undo->edit_count++);
        *result = inv_step;
    }
    return result;
}

inline void
undo_stack_pop(Edit_Stack *stack){
    if (stack->edit_count > 0){
        Edit_Step *edit = stack->edits + (--stack->edit_count);
        if (edit->child_count == 0){
            stack->size -= edit->edit.len;
        }
    }
}

internal void
file_post_redo(General_Memory *general, Editing_File *file, Edit_Step step){
    Edit_Stack *redo = &file->state.undo.redo;
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + redo->size > redo->max){
            undo_stack_grow_string(general, redo, step.edit.end - step.edit.start);
        }
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv, (char*)redo->strings, &redo->size, redo->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.type = ED_REDO;
        
        if (redo->edit_count == redo->edit_max){
            undo_stack_grow_edits(general, redo);
        }
        redo->edits[redo->edit_count++] = inv_step;
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = ED_REDO;
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.child_count = step.inverse_child_count;
        inv_step.inverse_child_count = step.child_count;
        
        if (redo->edit_count == redo->edit_max){
            undo_stack_grow_edits(general, redo);
        }
        redo->edits[redo->edit_count++] = inv_step;
    }
}

inline void
file_post_history_block(Editing_File *file, i32 pos){
    Assert(file->state.undo.history_head_block < pos);
    Assert(pos < file->state.undo.history.edit_count);
    
    Edit_Step *history = file->state.undo.history.edits;
    Edit_Step *step = history + file->state.undo.history_head_block;
    step->next_block = pos;
    step = history + pos;
    step->prev_block = file->state.undo.history_head_block;
    file->state.undo.history_head_block = pos;
    ++file->state.undo.history_block_count;
}

inline void
file_unpost_history_block(Editing_File *file){
    Assert(file->state.undo.history_block_count > 1);
    --file->state.undo.history_block_count;
    Edit_Step *old_head = file->state.undo.history.edits + file->state.undo.history_head_block;
    file->state.undo.history_head_block = old_head->prev_block;
}

internal Edit_Step*
file_post_history(General_Memory *general, Editing_File *file, Edit_Step step, b32 do_merge, b32 can_merge){
    Edit_Stack *history = &file->state.undo.history;
    Edit_Step *result = 0;
    
    local_persist Edit_Type reverse_types[4];
    if (reverse_types[ED_UNDO] == 0){
        reverse_types[ED_NORMAL] = ED_REVERSE_NORMAL;
        reverse_types[ED_REVERSE_NORMAL] = ED_NORMAL;
        reverse_types[ED_UNDO] = ED_REDO;
        reverse_types[ED_REDO] = ED_UNDO;
    }
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + history->size > history->max){
            undo_stack_grow_string(general, history, step.edit.end - step.edit.start);
        }
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv,
                           (char*)history->strings, &history->size, history->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.can_merge = (b8)can_merge;
        inv_step.type = reverse_types[step.type];
        
        b32 did_merge = 0;
        if (do_merge && history->edit_count > 0){
            Edit_Step prev = history->edits[history->edit_count-1];
            if (prev.can_merge && inv_step.edit.len == 0 && prev.edit.len == 0){
                if (prev.edit.end == inv_step.edit.start){
                    did_merge = 1;
                    inv_step.edit.start = prev.edit.start;
                }
            }
        }
        
        if (did_merge){
            result = history->edits + (history->edit_count-1);
        }
        else{
            if (history->edit_count == history->edit_max){
                undo_stack_grow_edits(general, history);
            }
            result = history->edits + (history->edit_count++);
        }
        
        *result = inv_step;
    }
    else{
        Edit_Step inv_step = {};
        inv_step.type = reverse_types[step.type];
        inv_step.first_child = step.inverse_first_child;
        inv_step.inverse_first_child = step.first_child;
        inv_step.special_type = step.special_type;
        inv_step.inverse_child_count = step.child_count;
        inv_step.child_count = step.inverse_child_count;
        
        if (history->edit_count == history->edit_max){
            undo_stack_grow_edits(general, history);
        }
        result = history->edits + (history->edit_count++);
        *result = inv_step;
    }
    
    return(result);
}

// TODO(allen): burn this shit to the ground yo!
inline void
file_view_nullify_file(View *view){
    view->file_data = null_file_viewing_data;
}

internal void
update_view_line_height(Models *models, View *view, i16 font_id){
    Render_Font *font = get_font_info(models->font_set, font_id)->font;
    view->line_height = font->height;
}

inline void
view_cursor_move(View *view, Full_Cursor cursor){
    view_set_cursor(view, cursor, 1, view->file_data.file->settings.unwrapped_lines);
    view->file_data.show_temp_highlight = 0;
}

inline void
view_cursor_move(View *view, i32 pos){
    Full_Cursor cursor = view_compute_cursor(view, seek_pos(pos), 0);
    view_cursor_move(view, cursor);
}

inline void
view_cursor_move(View *view, f32 x, f32 y, b32 round_down = 0){
    Buffer_Seek seek;
    if (view->file_data.file->settings.unwrapped_lines){
        seek = seek_unwrapped_xy(x, y, round_down);
    }
    else{
        seek = seek_wrapped_xy(x, y, round_down);
    }
    
    Full_Cursor cursor = view_compute_cursor(view, seek, 0);
    view_cursor_move(view, cursor);
}

inline void
view_cursor_move(View *view, i32 line, i32 character){
    Full_Cursor cursor = view_compute_cursor(view, seek_line_char(line, character), 0);
    view_cursor_move(view, cursor);
}

inline void
view_show_file(View *view){
    Editing_File *file = view->file_data.file;
    if (file){
        view->map = get_map(view->persistent.models, file->settings.base_map_id);
    }
    else{
        view->map = get_map(view->persistent.models, mapid_global);
    }
    
    if (view->showing_ui != VUI_None){
        view->showing_ui = VUI_None;
        view->changed_context_in_step = 1;
    }
}

internal void
view_set_file(View *view, Editing_File *file, Models *models){
    Assert(file);
    
    if (view->file_data.file != 0){
        touch_file(&models->working_set, view->file_data.file);
    }
    
    File_Edit_Positions *edit_pos = view->edit_pos;
    
    if (edit_pos){
        edit_pos_unset(view->file_data.file, edit_pos);
        edit_pos = 0;
    }
    
    file_view_nullify_file(view);
    view->file_data.file = file;
    
    edit_pos = edit_pos_get_new(file, view->persistent.id);
    view->edit_pos = edit_pos;
    
    update_view_line_height(models, view, file->settings.font_id);
    
    if (edit_pos->cursor.line == 0){
        view_cursor_move(view, 0);
    }
    
    if (view->showing_ui == VUI_None){
        view_show_file(view);
    }
}

struct Relative_Scrolling{
    f32 scroll_x, scroll_y;
    f32 target_x, target_y;
};

internal Relative_Scrolling
view_get_relative_scrolling(View *view){
    Relative_Scrolling result = {0};
    if (view->edit_pos){
        f32 cursor_y = view_get_cursor_y(view);
        result.scroll_y = cursor_y - view->edit_pos->scroll.scroll_y;
        result.target_y = cursor_y - view->edit_pos->scroll.target_y;
    }
    return(result);
}

internal void
view_set_relative_scrolling(View *view, Relative_Scrolling scrolling){
    f32 cursor_y = view_get_cursor_y(view);
    
    if (view->edit_pos){
        view->edit_pos->scroll.scroll_y = cursor_y - scrolling.scroll_y;
        view->edit_pos->scroll.target_y = round32(clamp_bottom(0.f, cursor_y - scrolling.target_y));
    }
}

inline i32_Rect
view_widget_rect(View *view, i32 line_height){
    Panel *panel = view->panel;
    i32_Rect result = panel->inner;
    
    if (view->file_data.file){
        result.y0 = result.y0 + line_height + 2;
    }
    
    return(result);
}

enum History_Mode{
    hist_normal,
    hist_backward,
    hist_forward
};

internal void
file_update_history_before_edit(Mem_Options *mem, Editing_File *file, Edit_Step step, u8 *str, History_Mode history_mode){
    if (!file->state.undo.undo.edits) return;
    General_Memory *general = &mem->general;
    
    b32 can_merge = 0, do_merge = 0;
    switch (step.type){
        case ED_NORMAL:
        {
            if (step.edit.len == 1 && str && char_is_alpha_numeric(*str)){
                can_merge = 1;
            }
            if (step.edit.len == 1 && str && (can_merge || char_is_whitespace(*str))){
                do_merge = 1;
            }
            
            if (history_mode != hist_forward){
                file_post_history(general, file, step, do_merge, can_merge);
            }
            
            file_post_undo(general, file, step, do_merge, can_merge);
        }break;
        
        case ED_REVERSE_NORMAL:
        {
            if (history_mode != hist_forward){
                file_post_history(general, file, step, do_merge, can_merge);
            }
            
            undo_stack_pop(&file->state.undo.undo);
            
            b32 restore_redos = 0;
            Edit_Step *redo_end = 0;
            
            if (history_mode == hist_backward && file->state.undo.edit_history_cursor > 0){
                restore_redos = 1;
                redo_end = file->state.undo.history.edits + (file->state.undo.edit_history_cursor - 1);
            }
            else if (history_mode == hist_forward && file->state.undo.history.edit_count > 0){
                restore_redos = 1;
                redo_end = file->state.undo.history.edits + (file->state.undo.history.edit_count - 1);
            }
            
            if (restore_redos){
                Edit_Step *redo_start = redo_end;
                i32 steps_of_redo = 0;
                i32 strings_of_redo = 0;
                i32 undo_count = 0;
                while (redo_start->type == ED_REDO || redo_start->type == ED_UNDO){
                    if (redo_start->type == ED_REDO){
                        if (undo_count > 0) --undo_count;
                        else{
                            ++steps_of_redo;
                            strings_of_redo += redo_start->edit.len;
                        }
                    }
                    else{
                        ++undo_count;
                    }
                    --redo_start;
                }
                
                if (redo_start < redo_end){
                    ++redo_start;
                    ++redo_end;
                    
                    if (file->state.undo.redo.edit_count + steps_of_redo > file->state.undo.redo.edit_max)
                        undo_stack_grow_edits(general, &file->state.undo.redo);
                    
                    if (file->state.undo.redo.size + strings_of_redo > file->state.undo.redo.max)
                        undo_stack_grow_string(general, &file->state.undo.redo, strings_of_redo);
                    
                    u8 *str_src = file->state.undo.history.strings + redo_end->edit.str_start;
                    u8 *str_dest_base = file->state.undo.redo.strings;
                    i32 str_redo_pos = file->state.undo.redo.size + strings_of_redo;
                    
                    Edit_Step *edit_src = redo_end;
                    Edit_Step *edit_dest = file->state.undo.redo.edits + file->state.undo.redo.edit_count + steps_of_redo;
                    
                    i32 undo_count = 0;
                    for (i32 i = 0; i < steps_of_redo;){
                        --edit_src;
                        str_src -= edit_src->edit.len;
                        if (edit_src->type == ED_REDO){
                            if (undo_count > 0){
                                --undo_count;
                            }
                            else{
                                ++i;
                                
                                --edit_dest;
                                *edit_dest = *edit_src;
                                
                                str_redo_pos -= edit_dest->edit.len;
                                edit_dest->edit.str_start = str_redo_pos;
                                
                                memcpy(str_dest_base + str_redo_pos, str_src, edit_dest->edit.len);
                            }
                        }
                        else{
                            ++undo_count;
                        }
                    }
                    Assert(undo_count == 0);
                    
                    file->state.undo.redo.size += strings_of_redo;
                    file->state.undo.redo.edit_count += steps_of_redo;
                }
            }
        }break;
        
        case ED_UNDO:
        {
            if (history_mode != hist_forward){
                file_post_history(general, file, step, do_merge, can_merge);
            }
            file_post_redo(general, file, step);
            undo_stack_pop(&file->state.undo.undo);
        }break;
        
        case ED_REDO:
        {
            if (step.edit.len == 1 && str && char_is_alpha_numeric(*str)) can_merge = 1;
            if (step.edit.len == 1 && str && (can_merge || char_is_whitespace(*str))) do_merge = 1;
            
            if (history_mode != hist_forward){
                file_post_history(general, file, step, do_merge, can_merge);
            }
            
            file_post_undo(general, file, step, do_merge, can_merge);
            undo_stack_pop(&file->state.undo.redo);
        }break;
    }
    
    if (history_mode != hist_forward){
        if (step.type == ED_UNDO || step.type == ED_REDO){
            if (file->state.undo.current_block_normal){
                file_post_history_block(file, file->state.undo.history.edit_count - 1);
                file->state.undo.current_block_normal = 0;
            }
        }
        else{
            if (!file->state.undo.current_block_normal){
                file_post_history_block(file, file->state.undo.history.edit_count - 1);
                file->state.undo.current_block_normal = 1;
            }
        }
    }
    else{
        if (file->state.undo.history_head_block == file->state.undo.history.edit_count){
            file_unpost_history_block(file);
            file->state.undo.current_block_normal = !file->state.undo.current_block_normal;
        }
    }
    
    if (history_mode == hist_normal){
        file->state.undo.edit_history_cursor = file->state.undo.history.edit_count;
    }
}

inline void
file_pre_edit_maintenance(System_Functions *system, General_Memory *general, Editing_File *file){
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_array.tokens){
            general_memory_free(general, file->state.swap_array.tokens);
            file->state.swap_array.tokens = 0;
        }
        file->state.still_lexing = 0;
    }
    file_mark_dirty(file);
}

struct Cursor_Fix_Descriptor{
    b32 is_batch;
    union{
        struct{
            Buffer_Edit *batch;
            i32 batch_size;
        };
        struct{
            i32 start, end;
            i32 shift_amount;
        };
    };
};

internal void
file_edit_cursor_fix(System_Functions *system, Models *models,
                     Editing_File *file, Editing_Layout *layout,
                     Cursor_Fix_Descriptor desc){
    
    Partition *part = &models->mem.part;
    
    Temp_Memory cursor_temp = begin_temp_memory(part);
    i32 cursor_max = layout->panel_max_count * 2;
    Cursor_With_Index *cursors = push_array(part, Cursor_With_Index, cursor_max);
    
    i32 cursor_count = 0;
    
    View *view = 0;
    Panel *panel = 0, *used_panels = &layout->used_sentinel;
    for (dll_items(panel, used_panels)){
        view = panel->view;
        if (view->file_data.file == file){
            Assert(view->edit_pos);
            write_cursor_with_index(cursors, &cursor_count, view->edit_pos->cursor.pos);
            write_cursor_with_index(cursors, &cursor_count, view->edit_pos->mark);
            write_cursor_with_index(cursors, &cursor_count, view->edit_pos->scroll_i);
        }
    }
    
    if (cursor_count > 0){
        buffer_sort_cursors(cursors, cursor_count);
        if (desc.is_batch){
            buffer_batch_edit_update_cursors(cursors, cursor_count,
                                             desc.batch, desc.batch_size);
        }
        else{
            buffer_update_cursors(cursors, cursor_count,
                                  desc.start, desc.end,
                                  desc.shift_amount + (desc.end - desc.start));
        }
        buffer_unsort_cursors(cursors, cursor_count);
        
        cursor_count = 0;
        for (dll_items(panel, used_panels)){
            view = panel->view;
            if (view->file_data.file == file){
                Assert(view->edit_pos);
                
                i32 cursor_pos = cursors[cursor_count++].pos;
                Full_Cursor new_cursor = view_compute_cursor(view, seek_pos(cursor_pos), 0);
                
                GUI_Scroll_Vars scroll = view->edit_pos->scroll;
                
                view->edit_pos->mark = cursors[cursor_count++].pos;
                i32 new_scroll_i = cursors[cursor_count++].pos;
                if (view->edit_pos->scroll_i != new_scroll_i){
                    view->edit_pos->scroll_i = new_scroll_i;
                    
                    Full_Cursor temp_cursor = view_compute_cursor(view, seek_pos(view->edit_pos->scroll_i), 0);
                    
                    f32 y_offset = MOD(view->edit_pos->scroll.scroll_y, view->line_height);
                    f32 y_position = temp_cursor.wrapped_y;
                    if (view->file_data.file->settings.unwrapped_lines){
                        y_position = temp_cursor.unwrapped_y;
                    }
                    y_position += y_offset;
                    
                    scroll.target_y += round32(y_position - scroll.scroll_y);
                    scroll.scroll_y = y_position;
                }
                
                view_set_cursor_and_scroll(view, new_cursor,
                                           1, view->file_data.file->settings.unwrapped_lines,
                                           scroll);
            }
        }
    }
    
    end_temp_memory(cursor_temp);
}

internal void
file_do_single_edit(System_Functions *system, Models *models, Editing_File *file, Edit_Spec spec, History_Mode history_mode){
    
    Mem_Options *mem = &models->mem;
    Editing_Layout *layout = &models->layout;
    
    // NOTE(allen): fixing stuff beforewards????
    file_update_history_before_edit(mem, file, spec.step, spec.str, history_mode);
    file_pre_edit_maintenance(system, &mem->general, file);
    
    // NOTE(allen): actual text replacement
    i32 shift_amount = 0;
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    
    char *str = (char*)spec.str;
    i32 start = spec.step.edit.start;
    i32 end = spec.step.edit.end;
    i32 str_len = spec.step.edit.len;
    
    i32 scratch_size = partition_remaining(part);
    
    Assert(scratch_size > 0);
    i32 request_amount = 0;
    Assert(end <= buffer_size(&file->state.buffer));
    while (buffer_replace_range(&file->state.buffer, start, end, str, str_len, &shift_amount,
                                part->base + part->pos, scratch_size, &request_amount)){
        void *new_data = 0;
        if (request_amount > 0){
            new_data = general_memory_allocate(general, request_amount);
        }
        void *old_data = buffer_edit_provide_memory(&file->state.buffer, new_data, request_amount);
        if (old_data){
            general_memory_free(general, old_data);
        }
    }
    
    // NOTE(allen): token fixing
    if (file->settings.tokens_exist){
        if (!file->settings.virtual_white){
            file_relex_parallel(system, mem, file, start, end, shift_amount);
        }
        else{
            file_relex_serial(mem, file, start, end, shift_amount);
        }
    }
    
    // NOTE(allen): meta data
    Gap_Buffer *buffer = &file->state.buffer;
    i32 line_start = buffer_get_line_index(&file->state.buffer, start);
    i32 line_end = buffer_get_line_index(&file->state.buffer, end);
    i32 replaced_line_count = line_end - line_start;
    i32 new_line_count = buffer_count_newlines(&file->state.buffer, start, start+str_len);
    i32 line_shift =  new_line_count - replaced_line_count;
    
    Render_Font *font = get_font_info(models->font_set, file->settings.font_id)->font;
    file_grow_starts_as_needed(general, buffer, line_shift);
    buffer_remeasure_starts(buffer, line_start, line_end, line_shift, shift_amount);
    
    file_allocate_character_starts_as_needed(general, file);
    buffer_remeasure_character_starts(buffer, line_start, line_end, line_shift, file->state.character_starts, 0, file->settings.virtual_white);
    
    // TODO(allen): Redo this as some sort of dialogical API
#if 0
    file_allocate_wraps_as_needed(general, file);
    buffer_remeasure_wrap_y(buffer, line_start, line_end, line_shift,
                            file->state.wraps, (f32)font->height, font->advance_data,
                            (f32)file->settings.display_width);
#endif
    
    file_measure_wraps(models, file, (f32)font->height, font->codepoint_advance_data, font->byte_advance);
    
    // NOTE(allen): cursor fixing
    Cursor_Fix_Descriptor desc = {};
    desc.start = start;
    desc.end = end;
    desc.shift_amount = shift_amount;
    
    file_edit_cursor_fix(system, models, file, layout, desc);
}

internal void
file_do_batch_edit(System_Functions *system, Models *models, Editing_File *file,
                   Edit_Spec spec, History_Mode history_mode, i32 batch_type){
    
    Mem_Options *mem = &models->mem;
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    Editing_Layout *layout = &models->layout;
    
    // NOTE(allen): fixing stuff "beforewards"???
    Assert(spec.str == 0);
    file_update_history_before_edit(mem, file, spec.step, 0, history_mode);
    file_pre_edit_maintenance(system, &mem->general, file);
    
    // NOTE(allen): actual text replacement
    u8 *str_base = file->state.undo.children.strings;
    i32 batch_size = spec.step.child_count;
    Buffer_Edit *batch = file->state.undo.children.edits + spec.step.first_child;
    
    Assert(spec.step.first_child < file->state.undo.children.edit_count);
    Assert(batch_size >= 0);
    
    i32 scratch_size = partition_remaining(part);
    Buffer_Batch_State state = {};
    i32 request_amount = 0;
    while (buffer_batch_edit_step(&state, &file->state.buffer, batch,
                                  (char*)str_base, batch_size, part->base + part->pos,
                                  scratch_size, &request_amount)){
        void *new_data = 0;
        if (request_amount > 0){
            new_data = general_memory_allocate(general, request_amount);
        }
        void *old_data = buffer_edit_provide_memory(&file->state.buffer, new_data, request_amount);
        if (old_data){
            general_memory_free(general, old_data);
        }
    }
    
    i32 shift_total = state.shift_total;
    
    // NOTE(allen): token fixing
    switch (batch_type){
        case BatchEdit_Normal:
        {
            if (file->settings.tokens_exist){
                // TODO(allen): Write a smart fast one here someday.
                Buffer_Edit *first_edit = batch;
                Buffer_Edit *last_edit = batch + batch_size - 1;
                
                if (!file->settings.virtual_white){
                    file_relex_parallel(system, mem, file, first_edit->start, last_edit->end, shift_total);
                }
                else{
                    file_relex_serial(mem, file, first_edit->start, last_edit->end, shift_total);
                }
            }
        }break;
        
        case BatchEdit_PreserveTokens:
        {
            if (file->state.tokens_complete){
                Cpp_Token_Array tokens = file->state.token_array;
                Cpp_Token *token = tokens.tokens;
                Cpp_Token *end_token = tokens.tokens + tokens.count;
                Cpp_Token original = {(Cpp_Token_Type)0};
                
                Buffer_Edit *edit = batch;
                Buffer_Edit *end_edit = batch + batch_size;
                
                i32 shift_amount = 0;
                i32 local_shift = 0;
                
                for (; token < end_token; ++token){
                    original = *token;
                    for (; edit < end_edit && edit->start <= original.start; ++edit){
                        local_shift = (edit->len - (edit->end - edit->start));
                        shift_amount += local_shift;
                    }
                    token->start += shift_amount;
                    local_shift = 0;
                    for (; edit < end_edit && edit->start < original.start + original.size; ++edit){
                        local_shift += (edit->len - (edit->end - edit->start));
                    }
                    token->size += local_shift;
                    shift_amount += local_shift;
                }
            }
        }break;
    }
    
    // TODO(allen): Let's try to switch to remeasuring here moron!
    // We'll need to get the total shift from the actual batch edit state
    // instead of from the cursor fixing.  The only reason we're getting
    // it from cursor fixing is because you're a lazy asshole.
    
    // NOTE(allen): meta data
    Buffer_Measure_Starts measure_state = {};
    buffer_measure_starts(&measure_state, &file->state.buffer);
    
    // TODO(allen): write the remeasurement version
    file_allocate_character_starts_as_needed(&models->mem.general, file);
    buffer_measure_character_starts(&file->state.buffer, file->state.character_starts, 0, file->settings.virtual_white);
    
    Render_Font *font = get_font_info(models->font_set, file->settings.font_id)->font;
    file_measure_wraps(models, file, (f32)font->height, font->codepoint_advance_data, font->byte_advance);
    
    // NOTE(allen): cursor fixing
    Cursor_Fix_Descriptor desc = {0};
    desc.is_batch = 1;
    desc.batch = batch;
    desc.batch_size = batch_size;
    file_edit_cursor_fix(system, models, file, layout, desc);
}

inline void
file_replace_range(System_Functions *system, Models *models, Editing_File *file,
                   i32 start, i32 end, char *str, i32 len){
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.edit.start =  start;
    spec.step.edit.end = end;
    spec.step.edit.len = len;
    spec.str = (u8*)str;
    file_do_single_edit(system, models, file, spec, hist_normal);
}

inline void
file_clear(System_Functions *system, Models *models, Editing_File *file){
    file_replace_range(system, models, file, 0, buffer_size(&file->state.buffer), 0, 0);
}

// TODO(allen): get rid of this
inline void
view_replace_range(System_Functions *system, Models *models, View *view,
                   i32 start, i32 end, char *str, i32 len){
    file_replace_range(system, models, view->file_data.file, start, end, str, len);
}

inline void
view_post_paste_effect(View *view, f32 seconds, i32 start, i32 size, u32 color){
    Editing_File *file = view->file_data.file;
    
    file->state.paste_effect.start = start;
    file->state.paste_effect.end = start + size;
    file->state.paste_effect.color = color;
    file->state.paste_effect.seconds_down = seconds;
    file->state.paste_effect.seconds_max = seconds;
}

internal Style*
get_style(Models *models, i32 i){
    return (&models->styles.styles[i]);
}

internal Style*
main_style(Models *models){
    return (get_style(models, 0));
}

internal void
apply_history_edit(System_Functions *system, Models *models,
                   Editing_File *file, View *view,
                   Edit_Stack *stack, Edit_Step step, History_Mode history_mode){
    Edit_Spec spec = {};
    spec.step = step;
    
    if (step.child_count == 0){
        spec.step.edit.str_start = 0;
        spec.str = stack->strings + step.edit.str_start;
        
        file_do_single_edit(system, models, file, spec, history_mode);
        
        if (view){
            view_cursor_move(view, step.edit.start + step.edit.len);
            view->edit_pos->mark = view->edit_pos->cursor.pos;
            
            Style *style = main_style(models);
            view_post_paste_effect(view, 0.333f,
                                   step.edit.start, step.edit.len,
                                   style->main.undo_color);
        }
    }
    else{
        file_do_batch_edit(system, models, view->file_data.file, spec, hist_normal, spec.step.special_type);
    }
}

internal void
view_undo_redo(System_Functions *system,
               Models *models, View *view,
               Edit_Stack *stack, Edit_Type expected_type){
    Editing_File *file = view->file_data.file;
    
    Assert(file);
    Assert(view->edit_pos);
    
    if (stack->edit_count > 0){
        Edit_Step step = stack->edits[stack->edit_count-1];
        
        Assert(step.type == expected_type);
        
        apply_history_edit(system, models,
                           file, view,
                           stack, step, hist_normal);
    }
}

inline void
view_undo(System_Functions *system, Models *models, View *view){
    view_undo_redo(system, models, view, &view->file_data.file->state.undo.undo, ED_UNDO);
}

inline void
view_redo(System_Functions *system, Models *models, View *view){
    view_undo_redo(system, models, view, &view->file_data.file->state.undo.redo, ED_REDO);
}

inline u8*
write_data(u8 *ptr, void *x, i32 size){
    memcpy(ptr, x, size);
    return (ptr + size);
}

#define UseFileHistoryDump 0

#if UseFileHistoryDump
internal void
file_dump_history(System_Functions *system, Mem_Options *mem, Editing_File *file, char *filename){
    if (!file->state.undo.undo.edits) return;
    
    i32 size = 0;
    
    size += sizeof(i32);
    size += file->state.undo.undo.edit_count*sizeof(Edit_Step);
    size += sizeof(i32);
    size += file->state.undo.redo.edit_count*sizeof(Edit_Step);
    size += sizeof(i32);
    size += file->state.undo.history.edit_count*sizeof(Edit_Step);
    size += sizeof(i32);
    size += file->state.undo.children.edit_count*sizeof(Buffer_Edit);
    
    size += sizeof(i32);
    size += file->state.undo.undo.size;
    size += sizeof(i32);
    size += file->state.undo.redo.size;
    size += sizeof(i32);
    size += file->state.undo.history.size;
    size += sizeof(i32);
    size += file->state.undo.children.size;
    
    Partition *part = &mem->part;
    i32 remaining = partition_remaining(part);
    if (size < remaining){
        u8 *data, *curs;
        data = (u8*)part->base + part->pos;
        curs = data;
        curs = write_data(curs, &file->state.undo.undo.edit_count, 4);
        curs = write_data(curs, &file->state.undo.redo.edit_count, 4);
        curs = write_data(curs, &file->state.undo.history.edit_count, 4);
        curs = write_data(curs, &file->state.undo.children.edit_count, 4);
        curs = write_data(curs, &file->state.undo.undo.size, 4);
        curs = write_data(curs, &file->state.undo.redo.size, 4);
        curs = write_data(curs, &file->state.undo.history.size, 4);
        curs = write_data(curs, &file->state.undo.children.size, 4);
        
        curs = write_data(curs, file->state.undo.undo.edits, sizeof(Edit_Step)*file->state.undo.undo.edit_count);
        curs = write_data(curs, file->state.undo.redo.edits, sizeof(Edit_Step)*file->state.undo.redo.edit_count);
        curs = write_data(curs, file->state.undo.history.edits, sizeof(Edit_Step)*file->state.undo.history.edit_count);
        curs = write_data(curs, file->state.undo.children.edits, sizeof(Buffer_Edit)*file->state.undo.children.edit_count);
        
        curs = write_data(curs, file->state.undo.undo.strings, file->state.undo.undo.size);
        curs = write_data(curs, file->state.undo.redo.strings, file->state.undo.redo.size);
        curs = write_data(curs, file->state.undo.history.strings, file->state.undo.history.size);
        curs = write_data(curs, file->state.undo.children.strings, file->state.undo.children.size);
        
        Assert((i32)(curs - data) == size);
        system->save_file(filename, data, size);
    }
}
#endif

internal void
view_history_step(System_Functions *system, Models *models, View *view, History_Mode history_mode){
    Assert(history_mode != hist_normal);
    
    Editing_File *file = view->file_data.file;
    
    Assert(file);
    Assert(view->edit_pos);
    
    b32 do_history_step = 0;
    Edit_Step step = {};
    if (history_mode == hist_backward){
        if (file->state.undo.edit_history_cursor > 0){
            do_history_step = 1;
            step = file->state.undo.history.edits[--file->state.undo.edit_history_cursor];
        }
    }
    else{
        if (file->state.undo.edit_history_cursor < file->state.undo.history.edit_count){
            Assert(((file->state.undo.history.edit_count - file->state.undo.edit_history_cursor) & 1) == 0);
            step = file->state.undo.history.edits[--file->state.undo.history.edit_count];
            file->state.undo.history.size -= step.edit.len;
            ++file->state.undo.edit_history_cursor;
            do_history_step = 1;
        }
    }
    
    if (do_history_step){
        apply_history_edit(system, models,
                           file, view,
                           &file->state.undo.history, step, history_mode);
    }
}

internal String*
working_set_next_clipboard_string(General_Memory *general, Working_Set *working, i32 str_size){
    String *result = 0;
    i32 clipboard_current = working->clipboard_current;
    if (working->clipboard_size == 0){
        clipboard_current = 0;
        working->clipboard_size = 1;
    }
    else{
        ++clipboard_current;
        if (clipboard_current >= working->clipboard_max_size){
            clipboard_current = 0;
        }
        else if (working->clipboard_size <= clipboard_current){
            working->clipboard_size = clipboard_current+1;
        }
    }
    result = &working->clipboards[clipboard_current];
    working->clipboard_current = clipboard_current;
    working->clipboard_rolling = clipboard_current;
    char *new_str;
    if (result->str){
        new_str = (char*)general_memory_reallocate(general, result->str, result->size, str_size);
    }
    else{
        new_str = (char*)general_memory_allocate(general, str_size+1);
    }
    // TODO(allen): What if new_str == 0?
    *result = make_string_cap(new_str, 0, str_size);
    return result;
}

internal String*
working_set_clipboard_index(Working_Set *working, i32 index){
    String *result = 0;
    i32 size = working->clipboard_size;
    i32 current = working->clipboard_current;
    if (index >= 0 && size > 0){
        index = index % size;
        index = current + size - index;
        index = index % size;
        result = &working->clipboards[index];
    }
    return(result);
}

internal String*
working_set_clipboard_head(Working_Set *working){
    String *result = 0;
    if (working->clipboard_size > 0){
        working->clipboard_rolling = 0;
        result = working_set_clipboard_index(working, working->clipboard_rolling);
    }
    return(result);
}

internal String*
working_set_clipboard_roll_down(Working_Set *working){
    String *result = 0;
    if (working->clipboard_size > 0){
        i32 clipboard_index = working->clipboard_rolling;
        ++clipboard_index;
        working->clipboard_rolling = clipboard_index;
        result = working_set_clipboard_index(working, working->clipboard_rolling);
    }
    return(result);
}

internal void
clipboard_copy(System_Functions *system, General_Memory *general, Working_Set *working, Range range, Editing_File *file){
    i32 size = range.end - range.start;
    String *dest = working_set_next_clipboard_string(general, working, size);
    buffer_stringify(&file->state.buffer, range.start, range.end, dest->str);
    dest->size = size;
    system->post_clipboard(*dest);
}

internal Edit_Spec
file_compute_edit(Mem_Options *mem, Editing_File *file, Buffer_Edit *edits, char *str_base, i32 str_size, Buffer_Edit *inverse_array, char *inv_str, i32 inv_max, i32 edit_count, i32 batch_type){
    General_Memory *general = &mem->general;
    
    i32 inv_str_pos = 0;
    Buffer_Invert_Batch state = {};
    if (buffer_invert_batch(&state, &file->state.buffer, edits, edit_count,
                            inverse_array, inv_str, &inv_str_pos, inv_max)){
        Assert(0);
    }
    
    i32 first_child = undo_children_push(general, &file->state.undo.children, edits, edit_count, (u8*)(str_base), str_size);
    i32 inverse_first_child = undo_children_push(general, &file->state.undo.children, inverse_array, edit_count, (u8*)(inv_str), inv_str_pos);
    
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.first_child = first_child;
    spec.step.inverse_first_child = inverse_first_child;
    spec.step.special_type = batch_type;
    spec.step.child_count = edit_count;
    spec.step.inverse_child_count = edit_count;
    
    return(spec);
}

internal u32*
style_get_color(Style *style, Cpp_Token token){
    u32 *result;
    if (token.flags & CPP_TFLAG_IS_KEYWORD){
        if (token.type == CPP_TOKEN_BOOLEAN_CONSTANT){
            result = &style->main.bool_constant_color;
        }
        else{
            result = &style->main.keyword_color;
        }
    }
    else if(token.flags & CPP_TFLAG_PP_DIRECTIVE){
        result = &style->main.preproc_color;
    }
    else{
        switch (token.type){
            case CPP_TOKEN_COMMENT:
            result = &style->main.comment_color;
            break;
            
            case CPP_TOKEN_STRING_CONSTANT:
            result = &style->main.str_constant_color;
            break;
            
            case CPP_TOKEN_CHARACTER_CONSTANT:
            result = &style->main.char_constant_color;
            break;
            
            case CPP_TOKEN_INTEGER_CONSTANT:
            result = &style->main.int_constant_color;
            break;
            
            case CPP_TOKEN_FLOATING_CONSTANT:
            result = &style->main.float_constant_color;
            break;
            
            case CPP_PP_INCLUDE_FILE:
            result = &style->main.include_color;
            break;
            
            default:
            result = &style->main.default_color;
            break;
        }
    }
    return result;
}

internal void
file_set_font(Models *models, Editing_File *file, i16 font_id){
    file->settings.font_id = font_id;
    Render_Font *font = get_font_info(models->font_set, file->settings.font_id)->font;
    file_measure_wraps_and_fix_cursor(models, file, (f32)font->height, font->codepoint_advance_data, font->byte_advance);
    
    Editing_Layout *layout = &models->layout;
    for (View_Iter iter = file_view_iter_init(layout, file, 0);
         file_view_iter_good(iter);
         iter = file_view_iter_next(iter)){
        update_view_line_height(models, iter.view, font_id);
    }
}

internal void
global_set_font(Models *models, i16 font_id){
    File_Node *node = 0;
    File_Node *sentinel = &models->working_set.used_sentinel;
    for (dll_items(node, sentinel)){
        Editing_File *file = (Editing_File*)node;
        file_set_font(models, file, font_id);
    }
    
    models->global_font.font_id = font_id;
}

inline void
view_show_GUI(View *view, View_UI ui){
    view->map = &view->persistent.models->map_ui;
    view->showing_ui = ui;
    view->changed_context_in_step = 1;
}

inline void
view_show_interactive(System_Functions *system, View *view,
                      Interactive_Action action,
                      Interactive_Interaction interaction,
                      String query){
    
    Models *models = view->persistent.models;
    
    view->showing_ui = VUI_Interactive;
    view->action = action;
    view->interaction = interaction;
    view->dest = make_fixed_width_string(view->dest_);
    view->list_i = 0;
    
    view->map = &models->map_ui;
    
    hot_directory_clean_end(&models->hot_directory);
    hot_directory_reload(system, &models->hot_directory);
    view->changed_context_in_step = 1;
}

inline void
view_show_theme(View *view){
    view->map = &view->persistent.models->map_ui;
    view->showing_ui = VUI_Theme;
    view->color_mode = CV_Mode_Library;
    view->color = super_color_create(0xFF000000);
    view->current_color_editing = 0;
    view->changed_context_in_step = 1;
}

internal String
make_string_terminated(Partition *part, char *str, i32 len){
    char *space = (char*)push_array(part, char, len + 1);
    String string = make_string_cap(str, len, len+1);
    copy_fast_unsafe_cs(space, string);
    string.str = space;
    terminate_with_null(&string);
    return(string);
}

internal void
init_normal_file(System_Functions *system, Models *models, Editing_File *file, char *buffer, i32 size){
    String val = make_string(buffer, size);
    file_create_from_string(system, models, file, val);
    
    Mem_Options *mem = &models->mem;
    if (file->settings.tokens_exist && file->state.token_array.tokens == 0){
        if (!file->settings.virtual_white){
            file_first_lex_parallel(system, mem, file);
        }
        else{
            file_first_lex_serial(mem, file);
        }
    }
}

internal void
init_read_only_file(System_Functions *system, Models *models, Editing_File *file){
    String val = null_string;
    file_create_from_string(system, models, file, val, 1);
    
    Mem_Options *mem = &models->mem;
    if (file->settings.tokens_exist && file->state.token_array.tokens == 0){
        if (!file->settings.virtual_white){
            file_first_lex_parallel(system, mem, file);
        }
        else{
            file_first_lex_serial(mem, file);
        }
    }
}


internal void
view_open_file(System_Functions *system, Models *models, View *view, String filename){
    Working_Set *working_set = &models->working_set;
    Editing_File *file = 0;
    
    if (terminate_with_null(&filename)){
        Editing_File_Canon_Name canon_name;
        if (get_canon_name(system, &canon_name, filename)){
            file = working_set_canon_contains(working_set, canon_name.name);
            
            if (!file){
                
                Plat_Handle handle;
                if (system->load_handle(canon_name.name.str, &handle)){
                    Mem_Options *mem = &models->mem;
                    General_Memory *general = &mem->general;
                    
                    file = working_set_alloc_always(working_set, general);
                    
                    buffer_bind_file(system, general, working_set, file, canon_name.name);
                    buffer_bind_name(general, working_set, file, filename);
                    
                    i32 size = system->load_size(handle);
                    Partition *part = &mem->part;
                    char *buffer = 0;
                    b32 gen_buffer = 0;
                    
                    Temp_Memory temp = begin_temp_memory(part);
                    
                    buffer = push_array(part, char, size);
                    if (buffer == 0){
                        buffer = (char*)general_memory_allocate(general, size);
                        Assert(buffer);
                        gen_buffer = 1;
                    }
                    
                    if (system->load_file(handle, buffer, size)){
                        system->load_close(handle);
                        init_normal_file(system, models, file, buffer, size);
                    }
                    else{
                        system->load_close(handle);
                    }
                    
                    if (gen_buffer){
                        general_memory_free(general, buffer);
                    }
                    
                    end_temp_memory(temp);
                }
            }
        }
    }
    
    if (file){
        view_set_file(view, file, models);
    }
}

internal void
view_interactive_save_as(System_Functions *system, Models *models, Editing_File *file, String filename){
    if (terminate_with_null(&filename)){
        file_save_and_set_names(system, models, file, filename);
    }
}

internal void
view_interactive_new_file(System_Functions *system, Models *models, View *view, String filename){
    Working_Set *working_set = &models->working_set;
    Editing_File *file = 0;
    
    if (terminate_with_null(&filename)){
        Editing_File_Canon_Name canon_name;
        
        if (get_canon_name(system, &canon_name, filename)){
            
            file = working_set_canon_contains(working_set, canon_name.name);
            if (file){
                file_clear(system, models, file);
            }
            else{
                
                Mem_Options *mem = &models->mem;
                General_Memory *general = &mem->general;
                
                file = working_set_alloc_always(working_set, general);
                
                buffer_bind_file(system, general, working_set, file, canon_name.name);
                buffer_bind_name(general, working_set, file, filename);
                
                init_normal_file(system, models, file, 0, 0);
            }
        }
    }
    
    if (file){
        view_set_file(view, file, models);
    }
}

internal void
kill_file(System_Functions *system, Models *models, Editing_File *file){
    Working_Set *working_set = &models->working_set;
    
    if (file && !file->settings.never_kill){
        buffer_unbind_name(working_set, file);
        if (file->canon.name.size != 0){
            buffer_unbind_file(system, working_set, file);
        }
        file_close(system, &models->mem.general, file);
        working_set_free_file(working_set, file);
        
        File_Node *used = &models->working_set.used_sentinel;
        File_Node *node = used->next;
        for (View_Iter iter = file_view_iter_init(&models->layout, file, 0);
             file_view_iter_good(iter);
             iter = file_view_iter_next(iter)){
            if (node != used){
                iter.view->file_data.file = 0;
                view_set_file(iter.view, (Editing_File*)node, models);
                node = node->next;
            }
            else{
                iter.view->file_data.file = 0;
                view_set_file(iter.view, 0, models);
            }
        }
    }
}

internal void
kill_file_by_name(System_Functions *system, Models *models, String name){
    Editing_File *file = working_set_name_contains(&models->working_set, name);
    if (file){
        kill_file(system, models, file);
    }
}

internal void
save_file_by_name(System_Functions *system, Models *models, String name){
    Editing_File *file = working_set_name_contains(&models->working_set, name);
    if (file){
        save_file(system, models, file);
    }
}

internal void
interactive_begin_sure_to_kill(System_Functions *system, View *view, Editing_File *file){
    view_show_interactive(system, view,
                          IAct_Sure_To_Kill, IInt_Sure_To_Kill,
                          make_lit_string("Are you sure?"));
    copy_ss(&view->dest, file->name.live_name);
}

enum Try_Kill_Result{
    TryKill_CannotKill,
    TryKill_NeedDialogue,
    TryKill_Success
};

internal Try_Kill_Result
interactive_try_kill_file(System_Functions *system, Models *models, Editing_File *file){
    Try_Kill_Result result = TryKill_CannotKill;
    
    if (!file->settings.never_kill){
        if (buffer_needs_save(file)){
            result = TryKill_NeedDialogue;
        }
        else{
            kill_file(system, models, file);
            result = TryKill_Success;
        }
    }
    
    return(result);
}

internal b32
interactive_try_kill_file(System_Functions *system, Models *models, View *view, Editing_File *file){
    Try_Kill_Result kill_result = interactive_try_kill_file(system, models, file);
    b32 result = (kill_result == TryKill_NeedDialogue);
    if (result){
        interactive_begin_sure_to_kill(system, view, file);
    }
    return(result);
}

internal b32
interactive_try_kill_file_by_name(System_Functions *system, Models *models, View *view, String name){
    b32 kill_dialogue = 0;
    
    Editing_File *file = working_set_name_contains(&models->working_set, name);
    if (file){
        kill_dialogue = interactive_try_kill_file(system, models, view, file);
    }
    
    return(kill_dialogue);
}

internal void
interactive_view_complete(System_Functions *system, View *view, String dest, i32 user_action){
    Models *models = view->persistent.models;
    
    switch (view->action){
        case IAct_Open:
        view_open_file(system, models, view, dest);
        view_show_file(view);
        break;
        
        case IAct_Save_As:
        view_interactive_save_as(system, models, view->file_data.file, dest);
        view_show_file(view);
        break;
        
        case IAct_New:
        if (dest.size > 0 && !char_is_slash(dest.str[dest.size-1])){
            view_interactive_new_file(system, models, view, dest);
            view_show_file(view);
        }
        break;
        
        case IAct_Switch:
        {
            Editing_File *file = working_set_name_contains(&models->working_set, dest);
            if (file){
                view_set_file(view, file, models);
            }
            view_show_file(view);
        }
        break;
        
        case IAct_Kill:
        if (!interactive_try_kill_file_by_name(system, models, view, dest)){
            view_show_file(view);
        }
        break;
        
        case IAct_Sure_To_Close:
        switch (user_action){
            case 0:
            models->keep_playing = 0;
            break;
            
            case 1:
            view_show_file(view);
            break;
            
            case 2:
            // TODO(allen): Save all and close.
            break;
        }
        break;
        
        case IAct_Sure_To_Kill:
        switch (user_action){
            case 0:
            kill_file_by_name(system, models, dest);
            view_show_file(view);
            break;
            
            case 1:
            view_show_file(view);
            break;
            
            case 2:
            save_file_by_name(system, models, dest);
            kill_file_by_name(system, models, dest);
            view_show_file(view);
            break;
        }
        break;
    }
}

#if 0
internal void
update_highlighting(View *view){
    View *file_view = view->hot_file_view;
    if (!file_view){
        view->highlight = {};
        return;
    }
    
    Editing_File *file = file_view->file;
    if (!file || !file_is_ready(file)){
        view->highlight = {};
        return;
    }
    
    Models *models = view->persistent.models;
    
    Style *style = &models->style;
    i32 pos = view_get_cursor_pos(file_view);
    char c = buffer_get_char(&file->state.buffer, pos);
    
    if (c == '\r'){
        view->highlight.ids[0] =
            raw_ptr_dif(&style->main.special_character_color, style);
    }
    
    else if (file->state.tokens_complete){
        Cpp_Token_Stack *tokens = &file->state.token_array;
        Cpp_Get_Token_Result result = cpp_get_token(tokens, pos);
        Cpp_Token token = tokens->tokens[result.token_index];
        if (!result.in_whitespace){
            u32 *color = style_get_color(style, token);
            view->highlight.ids[0] = raw_ptr_dif(color, style);
            if (token.type == CPP_TOKEN_JUNK){
                view->highlight.ids[1] =
                    raw_ptr_dif(&style->main.highlight_junk_color, style);
            }
            else if (char_is_whitespace(c)){
                view->highlight.ids[1] =
                    raw_ptr_dif(&style->main.highlight_white_color, style);
            }
            else{
                view->highlight.ids[1] = 0;
            }
        }
        else{
            view->highlight.ids[0] = 0;
            view->highlight.ids[1] =
                raw_ptr_dif(&style->main.highlight_white_color, style);
        }
    }
    
    else{
        if (char_is_whitespace(c)){
            view->highlight.ids[0] = 0;
            view->highlight.ids[1] =
                raw_ptr_dif(&style->main.highlight_white_color, style);
        }
        else{
            view->highlight.ids[0] =
                raw_ptr_dif(&style->main.default_color, style);
            view->highlight.ids[1] = 0;
        }
    }
    
    if (file_view->show_temp_highlight){
        view->highlight.ids[2] =
            raw_ptr_dif(&style->main.highlight_color, style);
        view->highlight.ids[3] =
            raw_ptr_dif(&style->main.at_highlight_color, style);
    }
    else if (file->state.paste_effect.tick_down > 0){
        view->highlight.ids[2] =
            raw_ptr_dif(&style->main.paste_color, style);
        view->highlight.ids[3] = 0;
    }
    else{
        view->highlight.ids[2] = 0;
        view->highlight.ids[3] = 0;
    }
}
#endif

struct File_Bar{
    f32 pos_x, pos_y;
    f32 text_shift_x, text_shift_y;
    i32_Rect rect;
    i16 font_id;
};

internal void
intbar_draw_string(Render_Target *target, File_Bar *bar, String str, u32 char_color){
    i16 font_id = bar->font_id;
    draw_string(target, font_id, str,
                (i32)(bar->pos_x + bar->text_shift_x),
                (i32)(bar->pos_y + bar->text_shift_y),
                char_color);
    bar->pos_x += font_string_width(target, font_id, str);
}

internal GUI_Scroll_Vars
view_reinit_scrolling(View *view){
    Editing_File *file = view->file_data.file;
    f32 w = 0, h = 0;
    f32 cursor_x = 0, cursor_y = 0;
    i32 target_x = 0, target_y = 0;
    
    view->reinit_scrolling = 0;
    
    if (file && file_is_ready(file)){
        cursor_x = view_get_cursor_x(view);
        cursor_y = view_get_cursor_y(view);
        
        w = view_width(view);
        h = view_file_height(view);
        
        if (cursor_x >= target_x + w){
            target_x = round32(cursor_x - w*.35f);
        }
        
        target_y = clamp_bottom(0, floor32(cursor_y - h*.5f));
    }
    
    GUI_Scroll_Vars scroll = {0};
    
    scroll.target_y = target_y;
    scroll.scroll_y = (f32)target_y;
    scroll.prev_target_y = -1000;
    
    scroll.target_x = target_x;
    scroll.scroll_x = (f32)target_x;
    scroll.prev_target_x = -1000;
    
    return(scroll);
}

internal b32
file_step(View *view, i32_Rect region, Input_Summary *user_input, b32 is_active, b32 *consumed_l){
    i32 is_animating = 0;
    Editing_File *file = view->file_data.file;
    if (file && !file->is_loading){
        if (file->state.paste_effect.seconds_down > 0.f){
            file->state.paste_effect.seconds_down -= user_input->dt;
            is_animating = 1;
        }
    }
    
    return(is_animating);
}

internal void
do_widget(View *view, GUI_Target *target){
    Query_Slot *slot;
    Query_Bar *bar;
    
    // NOTE(allen): A temporary measure... although in
    // general we maybe want the user to be able to ask
    // how large a particular section of the GUI turns
    // out to be after layout?
    f32 height = 0.f;
    
    for (slot = view->query_set.used_slot; slot != 0; slot = slot->next){
        bar = slot->query_bar;
        gui_do_text_field(target, bar->prompt, bar->string);
        
        height += view->line_height + 2;
    }
    
    view->widget_height = height;
}

struct Exhaustive_File_Loop{
    char front_name_[256];
    char full_path_[256];
    String front_name, full_path;
    
    Absolutes absolutes;
    
    File_Info *infos;
    i32 count, r;
};

struct Exhaustive_File_Info{
    File_Info *info;
    String message;
    b8 is_folder;
    b8 name_match;
    b8 is_loaded;
};

internal void
begin_exhaustive_loop(Exhaustive_File_Loop *loop, Hot_Directory *hdir){
    loop->front_name = make_fixed_width_string(loop->front_name_);
    loop->full_path = make_fixed_width_string(loop->full_path_);
    
    loop->infos = hdir->file_list.infos;
    loop->count = hdir->file_list.count;
    
    copy_ss(&loop->front_name, front_of_directory(hdir->string));
    get_absolutes(loop->front_name, &loop->absolutes, 1, 1);
    copy_ss(&loop->full_path, hdir->canon_dir);
    loop->r = loop->full_path.size;
}

internal Exhaustive_File_Info
get_exhaustive_info(System_Functions *system, Working_Set *working_set, Exhaustive_File_Loop *loop, i32 i){
    local_persist String message_loaded = make_lit_string(" LOADED");
    local_persist String message_unsaved = make_lit_string(" LOADED *");
    local_persist String message_unsynced = make_lit_string(" LOADED !");
    
    Exhaustive_File_Info result = {0};
    
    result.info = loop->infos + i;
    loop->full_path.size = loop->r;
    append_sc(&loop->full_path, result.info->filename);
    terminate_with_null(&loop->full_path);
    
    Editing_File *file = working_set_canon_contains(working_set, loop->full_path);
    
    String filename = make_string_cap(result.info->filename, result.info->filename_len, result.info->filename_len+1);
    
    result.is_folder = (result.info->folder != 0);
    result.name_match = (wildcard_match_s(&loop->absolutes, filename, 0) != 0);
    result.is_loaded = (file != 0 && file_is_ready(file));
    
    result.message = null_string;
    if (result.is_loaded){
        switch (file_get_sync(file)){
            case DirtyState_UpToDate: result.message = message_loaded; break;
            case DirtyState_UnsavedChanges: result.message = message_unsaved; break;
            case DirtyState_UnloadedChanges: result.message = message_unsynced; break;
        }
    }
    
    return(result);
}

struct Style_Color_Edit{
    Style_Tag target;
    Style_Tag fore;
    Style_Tag back;
    String text;
};

static Style_Color_Edit colors_to_edit[] = {
    {Stag_Back, Stag_Default, Stag_Back, make_lit_string("Background")},
    {Stag_Margin, Stag_Default, Stag_Margin, make_lit_string("Margin")},
    {Stag_Margin_Hover, Stag_Default, Stag_Margin_Hover, make_lit_string("Margin Hover")},
    {Stag_Margin_Active, Stag_Default, Stag_Margin_Active, make_lit_string("Margin Active")},
    
    {Stag_Cursor, Stag_At_Cursor, Stag_Cursor, make_lit_string("Cursor")},
    {Stag_At_Cursor, Stag_At_Cursor, Stag_Cursor, make_lit_string("Text At Cursor")},
    {Stag_Mark, Stag_Mark, Stag_Back, make_lit_string("Mark")},
    
    {Stag_Highlight, Stag_At_Highlight, Stag_Highlight, make_lit_string("Highlight")},
    {Stag_At_Highlight, Stag_At_Highlight, Stag_Highlight, make_lit_string("Text At Highlight")},
    
    {Stag_Default, Stag_Default, Stag_Back, make_lit_string("Text Default")},
    {Stag_Comment, Stag_Comment, Stag_Back, make_lit_string("Comment")},
    {Stag_Keyword, Stag_Keyword, Stag_Back, make_lit_string("Keyword")},
    {Stag_Str_Constant, Stag_Str_Constant, Stag_Back, make_lit_string("String Constant")},
    {Stag_Char_Constant, Stag_Char_Constant, Stag_Back, make_lit_string("Character Constant")},
    {Stag_Int_Constant, Stag_Int_Constant, Stag_Back, make_lit_string("Integer Constant")},
    {Stag_Float_Constant, Stag_Float_Constant, Stag_Back, make_lit_string("Float Constant")},
    {Stag_Bool_Constant, Stag_Bool_Constant, Stag_Back, make_lit_string("Boolean Constant")},
    {Stag_Preproc, Stag_Preproc, Stag_Back, make_lit_string("Preprocessor")},
    {Stag_Special_Character, Stag_Special_Character, Stag_Back, make_lit_string("Special Character")},
    
    {Stag_Highlight_Junk, Stag_Default, Stag_Highlight_Junk, make_lit_string("Junk Highlight")},
    {Stag_Highlight_White, Stag_Default, Stag_Highlight_White, make_lit_string("Whitespace Highlight")},
    
    {Stag_Paste, Stag_Paste, Stag_Back, make_lit_string("Paste Color")},
    
    {Stag_Bar, Stag_Base, Stag_Bar, make_lit_string("Bar")},
    {Stag_Base, Stag_Base, Stag_Bar, make_lit_string("Bar Text")},
    {Stag_Pop1, Stag_Pop1, Stag_Bar, make_lit_string("Bar Pop 1")},
    {Stag_Pop2, Stag_Pop2, Stag_Bar, make_lit_string("Bar Pop 2")},
};

struct Single_Line_Input_Step{
    b8 hit_newline;
    b8 hit_ctrl_newline;
    b8 hit_a_character;
    b8 hit_backspace;
    b8 hit_esc;
    b8 made_a_change;
    b8 did_command;
    b8 no_file_match;
};

enum Single_Line_Input_Type{
    SINGLE_LINE_STRING,
    SINGLE_LINE_FILE
};

struct Single_Line_Mode{
    Single_Line_Input_Type type;
    String *string;
    Hot_Directory *hot_directory;
    b32 fast_folder_select;
    b32 try_to_match;
    b32 case_sensitive;
};

internal Single_Line_Input_Step
app_single_line_input_core(System_Functions *system, Working_Set *working_set, Key_Event_Data key, Single_Line_Mode mode){
    Single_Line_Input_Step result = {0};
    
    if (key.keycode == key_back){
        result.hit_backspace = 1;
        if (mode.string->size > 0){
            result.made_a_change = 1;
            --mode.string->size;
            switch (mode.type){
                case SINGLE_LINE_STRING:
                {
                    mode.string->str[mode.string->size] = 0;
                }break;
                
                case SINGLE_LINE_FILE:
                {
                    if (!key.modifiers[MDFR_CONTROL_INDEX]){
                        char end_character = mode.string->str[mode.string->size];
                        if (char_is_slash(end_character)){
                            mode.string->size = reverse_seek_slash(*mode.string) + 1;
                            mode.string->str[mode.string->size] = 0;
                            hot_directory_set(system, mode.hot_directory, *mode.string);
                        }
                        else{
                            mode.string->str[mode.string->size] = 0;
                        }
                    }
                    else{
                        mode.string->str[mode.string->size] = 0;
                    }
                }break;
            }
        }
    }
    
    else if (key.character == '\n' || key.character == '\t'){
        // NOTE(allen): do nothing!
    }
    
    else if (key.keycode == key_esc){
        result.hit_esc = 1;
        result.made_a_change = 1;
    }
    
    else if (key.character){
        result.hit_a_character = 1;
        if (!key.modifiers[MDFR_CONTROL_INDEX] && !key.modifiers[MDFR_ALT_INDEX]){
            if (mode.string->size+1 < mode.string->memory_size){
                u8 new_character = (u8)key.character;
                mode.string->str[mode.string->size] = new_character;
                mode.string->size++;
                mode.string->str[mode.string->size] = 0;
                if (mode.type == SINGLE_LINE_FILE && char_is_slash(new_character)){
                    hot_directory_set(system, mode.hot_directory, *mode.string);
                }
                result.made_a_change = 1;
            }
        }
        else{
            result.did_command = 1;
        }
    }
    
    return result;
}

inline Single_Line_Input_Step
app_single_line_input_step(System_Functions *system, Key_Event_Data key, String *string){
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_STRING;
    mode.string = string;
    return app_single_line_input_core(system, 0, key, mode);
}

inline Single_Line_Input_Step
app_single_file_input_step(System_Functions *system,
                           Working_Set *working_set, Key_Event_Data key,
                           String *string, Hot_Directory *hot_directory,
                           b32 try_to_match, b32 case_sensitive){
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_FILE;
    mode.string = string;
    mode.hot_directory = hot_directory;
    mode.try_to_match = try_to_match;
    mode.case_sensitive = case_sensitive;
    return app_single_line_input_core(system, working_set, key, mode);
}

inline Single_Line_Input_Step
app_single_number_input_step(System_Functions *system, Key_Event_Data key, String *string){
    Single_Line_Input_Step result = {};
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_STRING;
    mode.string = string;
    
    char c = (char)key.character;
    if (c == 0 || c == '\n' || char_is_numeric(c))
        result = app_single_line_input_core(system, 0, key, mode);
    return result;
}

internal void
append_label(String *string, i32 indent_level, char *message){
    i32 r = 0;
    for (r = 0; r < indent_level; ++r){
        append_s_char(string, '>');
    }
    append_sc(string, message);
}

internal void
show_gui_line(GUI_Target *target, String *string, i32 indent_level, i32 h_align, char *message, char *follow_up){
    string->size = 0;
    append_label(string, indent_level, message);
    if (follow_up){
        append_padding(string, '-', h_align);
        append_s_char(string, ' ');
        append_sc(string, follow_up);
    }
    gui_do_text_field(target, *string, null_string);
}

internal void
show_gui_int(GUI_Target *target, String *string,
             i32 indent_level, i32 h_align, char *message, i32 x){
    string->size = 0;
    append_label(string, indent_level, message);
    append_padding(string, '-', h_align);
    append_s_char(string, ' ');
    append_int_to_str(string, x);
    gui_do_text_field(target, *string, null_string);
}

internal void
show_gui_u64(GUI_Target *target, String *string,
             i32 indent_level, i32 h_align, char *message, u64 x){
    string->size = 0;
    append_label(string, indent_level, message);
    append_padding(string, '-', h_align);
    append_s_char(string, ' ');
    append_u64_to_str(string, x);
    gui_do_text_field(target, *string, null_string);
}

internal void
show_gui_int_int(GUI_Target *target, String *string,
                 i32 indent_level, i32 h_align, char *message, i32 x, i32 m){
    string->size = 0;
    append_label(string, indent_level, message);
    append_padding(string, '-', h_align);
    append_s_char(string, ' ');
    append_int_to_str(string, x);
    append_s_char(string, '/');
    append_int_to_str(string, m);
    gui_do_text_field(target, *string, null_string);
}

internal void
show_gui_id(GUI_Target *target, String *string,
            i32 indent_level, i32 h_align, char *message, GUI_id id){
    string->size = 0;
    append_label(string, indent_level, message);
    append_padding(string, '-', h_align);
    append_ss(string, make_lit_string(" [0]: "));
    append_u64_to_str(string, id.id[0]);
    append_padding(string, ' ', h_align + 26);
    append_ss(string, make_lit_string(" [1]: "));
    append_u64_to_str(string, id.id[1]);
    gui_do_text_field(target, *string, null_string);
}

internal void
show_gui_float(GUI_Target *target, String *string,
               i32 indent_level, i32 h_align, char *message, float x){
    string->size = 0;
    append_label(string, indent_level, message);
    append_padding(string, '-', h_align);
    append_s_char(string, ' ');
    append_float_to_str(string, x);
    gui_do_text_field(target, *string, null_string);
}

internal void
show_gui_scroll(GUI_Target *target, String *string,
                i32 indent_level, i32 h_align, char *message,
                GUI_Scroll_Vars scroll){
    show_gui_line (target, string, indent_level, 0, message, 0);
    show_gui_float(target, string, indent_level+1, h_align, " scroll_y ", scroll.scroll_y);
    show_gui_int  (target, string, indent_level+1, h_align, " target_y ", scroll.target_y);
    show_gui_int  (target, string, indent_level+1, h_align, " prev_target_y ", scroll.prev_target_y);
    show_gui_float(target, string, indent_level+1, h_align, " scroll_x ", scroll.scroll_x);
    show_gui_int  (target, string, indent_level+1, h_align, " target_x ", scroll.target_x);
    show_gui_int  (target, string, indent_level+1, h_align, " prev_target_x ", scroll.prev_target_x);
}

internal void
show_gui_cursor(GUI_Target *target, String *string,
                i32 indent_level, i32 h_align, char *message,
                Full_Cursor cursor){
    show_gui_line (target, string, indent_level, 0, message, 0);
    show_gui_int  (target, string, indent_level+1, h_align, " pos ", cursor.pos);
    show_gui_int  (target, string, indent_level+1, h_align, " line ", cursor.line);
    show_gui_int  (target, string, indent_level+1, h_align, " column ", cursor.character);
    show_gui_float(target, string, indent_level+1, h_align, " unwrapped_x ", cursor.unwrapped_x);
    show_gui_float(target, string, indent_level+1, h_align, " unwrapped_y ", cursor.unwrapped_y);
    show_gui_float(target, string, indent_level+1, h_align, " wrapped_x ", cursor.wrapped_x);
    show_gui_float(target, string, indent_level+1, h_align, " wrapped_y ", cursor.wrapped_y);
}

internal void
show_gui_region(GUI_Target *target, String *string,
                i32 indent_level, i32 h_align, char *message,
                i32_Rect region){
    show_gui_line(target, string, indent_level, 0, message, 0);
    show_gui_int (target, string, indent_level+1, h_align, " x0 ", region.x0);
    show_gui_int (target, string, indent_level+1, h_align, " y0 ", region.y0);
    show_gui_int (target, string, indent_level+1, h_align, " x1 ", region.x1);
    show_gui_int (target, string, indent_level+1, h_align, " y1 ", region.y1);
}

struct View_Step_Result{
    b32 animating;
    b32 consume_keys;
    b32 consume_esc;
};

inline void
gui_show_mouse(GUI_Target *target, String *string, i32 mx, i32 my){
    string->size = 0;
    append_ss(string, make_lit_string("mouse: ("));
    append_int_to_str(string, mx);
    append_s_char(string, ',');
    append_int_to_str(string, my);
    append_s_char(string, ')');
    
    gui_do_text_field(target, *string, null_string);
}

internal View_Step_Result
step_file_view(System_Functions *system, View *view, View *active_view, Input_Summary input){
    View_Step_Result result = {0};
    GUI_Target *target = &view->gui_target;
    Models *models = view->persistent.models;
    Key_Input_Data keys = input.keys;
    
    b32 show_scrollbar = !view->hide_scrollbar;
    
    if (view->showing_ui != VUI_None){
        b32 did_esc = 0;
        Key_Event_Data key;
        i32 i;
        
        for (i = 0; i < keys.count; ++i){
            key = get_single_key(&keys, i);
            if (key.keycode == key_esc){
                did_esc = 1;
                break;
            }
        }
        
        if (did_esc){
            view_show_file(view);
            result.consume_esc = 1;
        }
    }
    
    gui_begin_top_level(target, input);
    {
        if (view->showing_ui != VUI_Debug){
            gui_do_top_bar(target);
        }
        do_widget(view, target);
        
        if (view->showing_ui == VUI_None){
            
            gui_begin_serial_section(target);
            {
                i32 delta = 9 * view->line_height;
                GUI_id scroll_context = {0};
                scroll_context.id[1] = view->showing_ui;
                scroll_context.id[0] = (u64)(view->file_data.file);
                
                GUI_Scroll_Vars scroll_zero = {0};
                GUI_Scroll_Vars *scroll = &scroll_zero;
                
                if (view->file_data.file){
                    Assert(view->edit_pos);
                    scroll = &view->edit_pos->scroll;
                }
                
                gui_begin_scrollable(target, scroll_context, *scroll,
                                     delta, show_scrollbar);
                
                gui_do_file(target);
                gui_end_scrollable(target);
            }
            gui_end_serial_section(target);
        }
        else{
            switch (view->showing_ui){
                case VUI_Menu:
                {
                    String message = make_lit_string("Menu");
                    String empty_string = {0};
                    GUI_id id = {0};
                    id.id[1] = VUI_Menu;
                    
                    gui_do_text_field(target, message, empty_string);
                    
                    id.id[0] = 0;
                    message = make_lit_string("Theme");
                    if (gui_do_fixed_option(target, id, message, 0)){
                        view_show_theme(view);
                    }
                    
                    id.id[0] = 1;
                    message = make_lit_string("Config");
                    if (gui_do_fixed_option(target, id, message, 0)){
                        view_show_GUI(view, VUI_Config);
                    }
                }break;
                
                case VUI_Config:
                {
                    String message = make_lit_string("Config");
                    String empty_string = {0};
                    GUI_id id = {0};
                    id.id[1] = VUI_Config;
                    
                    gui_do_text_field(target, message, empty_string);
                    
                    id.id[0] = 0;
                    message = make_lit_string("Left Ctrl + Left Alt = AltGr");
                    if (gui_do_fixed_option_checkbox(target, id, message, 0, (b8)models->settings.lctrl_lalt_is_altgr)){
                        models->settings.lctrl_lalt_is_altgr = !models->settings.lctrl_lalt_is_altgr;
                    }
                }break;
                
                case VUI_Theme:
                {
                    if (view != active_view){
                        view->hot_file_view = active_view;
                    }
                    
                    String message = {0};
                    String empty_string = {0};
                    
                    GUI_id id = {0};
                    id.id[1] = VUI_Theme + ((u64)view->color_mode << 32);
                    
                    GUI_id scroll_context = {0};
                    scroll_context.id[0] = 0;
                    scroll_context.id[1] = VUI_Theme + ((u64)view->color_mode << 32);
                    
                    switch (view->color_mode){
                        case CV_Mode_Library:
                        message = make_lit_string("Current Theme - Click to Edit");
                        gui_do_text_field(target, message, empty_string);
                        
                        id.id[0] = (u64)(main_style(models));
                        if (gui_do_style_preview(target, id, 0)){
                            view->color_mode = CV_Mode_Adjusting;
                        }
                        
                        if (view->file_data.file){
                            message = make_lit_string("Set Font");
                            id.id[0] = (u64)(&view->file_data.file->settings.font_id);
                            
                            if (gui_do_button(target, id, message)){
                                view->color_mode = CV_Mode_Font;
                            }
                        }
                        
                        message = make_lit_string("Set Global Font");
                        id.id[0] = (u64)(&models->global_font);
                        
                        if (gui_do_button(target, id, message)){
                            view->color_mode = CV_Mode_Global_Font;
                        }
                        
                        
                        
                        message = make_lit_string("Theme Library - Click to Select");
                        gui_do_text_field(target, message, empty_string);
                        
                        gui_begin_scrollable(target, scroll_context, view->gui_scroll,
                                             9 * view->line_height, show_scrollbar);
                        
                        {
                            i32 count = models->styles.count;
                            Style *style;
                            i32 i;
                            
                            for (i = 1; i < count; ++i, ++style){
                                style = get_style(models, i);
                                id.id[0] = (u64)(style);
                                if (gui_do_style_preview(target, id, i)){
                                    style_copy(main_style(models), style);
                                }
                            }
                        }
                        
                        gui_end_scrollable(target);
                        break;
                        
                        case CV_Mode_Global_Font:
                        case CV_Mode_Font:
                        {
                            Assert(view->file_data.file);
                            
                            Font_Set *font_set = models->font_set;
                            Font_Info *info = 0;
                            
                            i16 i = 1, count = (i16)models->font_set->count + 1;
                            
                            String message = make_lit_string("Back");
                            
                            id.id[0] = 0;
                            if (gui_do_button(target, id, message)){
                                view->color_mode = CV_Mode_Library;
                            }
                            
                            i16 font_id = models->global_font.font_id;
                            if (view->color_mode == CV_Mode_Font){
                                font_id = view->file_data.file->settings.font_id;
                            }
                            
                            i16 new_font_id = font_id;
                            
                            for (i = 1; i < count; ++i){
                                info = get_font_info(font_set, i);
                                id.id[0] = (u64)i;
                                if (i != font_id){
                                    if (gui_do_font_button(target, id, i, info->name)){
                                        new_font_id = i;
                                    }
                                }
                                else{
                                    char message_space[256];
                                    message = make_fixed_width_string(message_space);
                                    copy_ss(&message, make_lit_string("currently selected: "));
                                    append_ss(&message, info->name);
                                    gui_do_font_button(target, id, i, message);
                                }
                            }
                            
                            if (font_id != new_font_id){
                                if (view->color_mode == CV_Mode_Font){
                                    file_set_font(models, view->file_data.file, new_font_id);
                                }
                                else{
                                    global_set_font(models, new_font_id);
                                }
                            }
                        }break;
                        
                        case CV_Mode_Adjusting:
                        {
                            Style *style = main_style(models);
                            u32 *edit_color = 0;
                            u32 *fore = 0, *back = 0;
                            i32 i = 0;
                            
                            String message = make_lit_string("Back");
                            
                            id.id[0] = 0;
                            if (gui_do_button(target, id, message)){
                                view->color_mode = CV_Mode_Library;
                            }
                            
                            gui_begin_scrollable(target, scroll_context, view->gui_scroll,
                                                 9 * view->line_height, show_scrollbar);
                            
                            i32 next_color_editing = view->current_color_editing;
                            
                            for (i = 0; i < ArrayCount(colors_to_edit); ++i){
                                edit_color = style_index_by_tag(&style->main, colors_to_edit[i].target);
                                id.id[0] = (u64)(edit_color);
                                
                                fore = style_index_by_tag(&style->main, colors_to_edit[i].fore);
                                back = style_index_by_tag(&style->main, colors_to_edit[i].back);
                                
                                if (gui_do_color_button(target, id, *fore, *back, colors_to_edit[i].text)){
                                    next_color_editing = i;
                                    view->color_cursor = 0;
                                }
                                
                                if (view->current_color_editing == i){
                                    GUI_Item_Update update = {0};
                                    char text_space[7];
                                    String text = make_fixed_width_string(text_space);
                                    
                                    color_to_hexstr(&text, *edit_color);
                                    if (gui_do_text_with_cursor(target, view->color_cursor, text, &update)){
                                        b32 r = 0;
                                        i32 j = 0;
                                        
                                        for (j = 0; j < keys.count; ++j){
                                            Key_Code key = keys.keys[j].keycode;
                                            switch (key){
                                                case key_left: --view->color_cursor; r = 1; result.consume_keys = 1; break;
                                                case key_right: ++view->color_cursor; r = 1; result.consume_keys = 1; break;
                                                
                                                case key_up:
                                                if (next_color_editing > 0){
                                                    --next_color_editing;
                                                }
                                                result.consume_keys = 1;
                                                break;
                                                
                                                case key_down:
                                                if (next_color_editing <= ArrayCount(colors_to_edit)-1){
                                                    ++next_color_editing;
                                                }
                                                result.consume_keys = 1;
                                                break;
                                                
                                                default:
                                                if ((key >= '0' && key <= '9') || (key >= 'a' && key <= 'f') || (key >= 'A' && key <= 'F')){
                                                    text.str[view->color_cursor] = (char)key;
                                                    r = 1;
                                                    result.consume_keys = 1;
                                                }
                                                break;
                                            }
                                            
                                            if (view->color_cursor < 0) view->color_cursor = 0;
                                            if (view->color_cursor >= 6) view->color_cursor = 5;
                                        }
                                        
                                        if (r){
                                            hexstr_to_color(text, edit_color);
                                            gui_rollback(target, &update);
                                            gui_do_text_with_cursor(target, view->color_cursor, text, 0);
                                        }
                                    }
                                }
                            }
                            
                            if (view->current_color_editing != next_color_editing){
                                view->current_color_editing = next_color_editing;
                                view->color_cursor = 0;
                            }
                            
                            gui_end_scrollable(target);
                        }break;
                    }
                }break;
                
                case VUI_Interactive:
                {
                    b32 complete = 0;
                    char comp_dest_space[1024];
                    String comp_dest = make_fixed_width_string(comp_dest_space);
                    i32 comp_action = 0;
                    
                    GUI_id id = {0};
                    id.id[1] = VUI_Interactive + ((u64)view->interaction << 32);
                    
                    GUI_id scroll_context = {0};
                    scroll_context.id[1] = VUI_Interactive + ((u64)view->interaction << 32);
                    
                    Key_Code user_up_key = models->user_up_key;
                    Key_Code user_down_key = models->user_down_key;
                    
                    switch (view->interaction){
                        case IInt_Sys_File_List:
                        {
                            b32 autocomplete_with_enter = 1;
                            b32 activate_directly = 0;
                            
                            if (view->action == IAct_Save_As || view->action == IAct_New){
                                autocomplete_with_enter = 0;
                            }
                            
                            String message = {0};
                            switch (view->action){
                                case IAct_Open: message = make_lit_string("Open: "); break;
                                case IAct_Save_As: message = make_lit_string("Save As: "); break;
                                case IAct_New: message = make_lit_string("New: "); break;
                            }
                            
                            Exhaustive_File_Loop loop;
                            Exhaustive_File_Info file_info;
                            
                            GUI_Item_Update update = {0};
                            Hot_Directory *hdir = &models->hot_directory;
                            b32 do_new_directory = 0;
                            i32 i = 0;
                            
                            {
                                Single_Line_Input_Step step = {0};
                                Key_Event_Data key = {0};
                                i32 i;
                                
                                for (i = 0; i < keys.count; ++i){
                                    key = get_single_key(&keys, i);
                                    step = app_single_file_input_step(system, &models->working_set, key,
                                                                      &hdir->string, hdir, 1, 0);
                                    if (step.made_a_change){
                                        view->list_i = 0;
                                        result.consume_keys = 1;
                                    }
                                    
                                    if (!autocomplete_with_enter && key.keycode == '\n'){
                                        activate_directly = 1;
                                        result.consume_keys = 1;
                                    }
                                }
                            }
                            
                            gui_do_text_field(target, message, hdir->string);
                            
                            b32 snap_into_view = 0;
                            scroll_context.id[0] = (u64)(hdir);
                            if (gui_scroll_was_activated(target, scroll_context)){
                                snap_into_view = 1;
                            }
                            gui_begin_scrollable(target, scroll_context, view->gui_scroll,
                                                 9 * view->line_height, show_scrollbar);
                            
                            id.id[0] = (u64)(hdir) + 1;
                            
                            if (gui_begin_list(target, id, view->list_i, 0,
                                               snap_into_view, &update)){
                                // TODO(allen): Allow me to handle key consumption correctly here!
                                gui_standard_list(target, id, &view->gui_scroll, view->scroll_region, &keys, &view->list_i, &update, user_up_key, user_down_key);
                            }
                            
                            begin_exhaustive_loop(&loop, hdir);
                            for (i = 0; i < loop.count; ++i){
                                file_info = get_exhaustive_info(system, &models->working_set, &loop, i);
                                
                                if (file_info.name_match){
                                    id.id[0] = (u64)(file_info.info);
                                    
                                    char *str = file_info.info->filename;
                                    i32 len = file_info.info->filename_len;
                                    String filename = make_string_cap(str, len, len + 1);
                                    
                                    if (gui_do_file_option(target, id, filename, file_info.is_folder, file_info.message)){
                                        if (file_info.is_folder){
                                            set_last_folder_sc(&hdir->string, file_info.info->filename, '/');
                                            do_new_directory = 1;
                                        }
                                        
                                        else if (autocomplete_with_enter){
                                            complete = 1;
                                            copy_ss(&comp_dest, loop.full_path);
                                        }
                                    }
                                }
                            }
                            
                            gui_end_list(target);
                            
                            if (activate_directly){
                                complete = 1;
                                copy_ss(&comp_dest, hdir->string);
                            }
                            
                            if (do_new_directory){
                                hot_directory_reload(system, hdir);
                            }
                            
                            gui_end_scrollable(target);
                        }break;
                        
                        case IInt_Live_File_List:
                        {
                            local_persist String message_unsaved = make_lit_string(" *");
                            local_persist String message_unsynced = make_lit_string(" !");
                            
                            String message = {0};
                            switch (view->action){
                                case IAct_Switch: message = make_lit_string("Switch: "); break;
                                case IAct_Kill: message = make_lit_string("Kill: "); break;
                            }
                            
                            Working_Set *working_set = &models->working_set;
                            Editing_Layout *layout = &models->layout;
                            GUI_Item_Update update = {0};
                            
                            {
                                Single_Line_Input_Step step;
                                Key_Event_Data key;
                                i32 i;
                                for (i = 0; i < keys.count; ++i){
                                    key = get_single_key(&keys, i);
                                    step = app_single_line_input_step(system, key, &view->dest);
                                    if (step.made_a_change){
                                        view->list_i = 0;
                                        result.consume_keys = 1;
                                    }
                                }
                            }
                            
                            Absolutes absolutes = {0};
                            get_absolutes(view->dest, &absolutes, 1, 1);
                            
                            gui_do_text_field(target, message, view->dest);
                            
                            b32 snap_into_view = 0;
                            scroll_context.id[0] = (u64)(working_set);
                            if (gui_scroll_was_activated(target, scroll_context)){
                                snap_into_view = 1;
                            }
                            gui_begin_scrollable(target, scroll_context, view->gui_scroll, 9 * view->line_height, show_scrollbar);
                            
                            id.id[0] = (u64)(working_set) + 1;
                            if (gui_begin_list(target, id, view->list_i, 0, snap_into_view, &update)){
                                gui_standard_list(target, id, &view->gui_scroll, view->scroll_region, &keys, &view->list_i, &update, user_up_key, user_down_key);
                            }
                            
                            {
                                Partition *part = &models->mem.part;
                                Temp_Memory temp = begin_temp_memory(part);
                                File_Node *node = 0, *used_nodes = 0;
                                Editing_File **reserved_files = 0;
                                i32 reserved_top = 0, i = 0;
                                
                                partition_align(part, sizeof(i32));
                                reserved_files = (Editing_File**)partition_current(part);
                                
                                used_nodes = &working_set->used_sentinel;
                                for (dll_items(node, used_nodes)){
                                    Editing_File *file = (Editing_File*)node;
                                    Assert(!file->is_dummy);
                                    
                                    if (wildcard_match_s(&absolutes, file->name.live_name, 0) != 0){
                                        View_Iter iter = file_view_iter_init(layout, file, 0);
                                        if (file_view_iter_good(iter)){
                                            reserved_files[reserved_top++] = file;
                                        }
                                        else{
                                            if (file->name.live_name.str[0] == '*'){
                                                reserved_files[reserved_top++] = file;
                                            }
                                            else{
                                                message = null_string;
                                                if (!file->settings.unimportant){
                                                    switch (file_get_sync(file)){
                                                        case DirtyState_UnloadedChanges: message = message_unsynced; break;
                                                        case DirtyState_UnsavedChanges: message = message_unsaved; break;
                                                    }
                                                }
                                                
                                                id.id[0] = (u64)(file);
                                                if (gui_do_file_option(target, id, file->name.live_name, 0, message)){
                                                    complete = 1;
                                                    copy_ss(&comp_dest, file->name.live_name);
                                                }
                                            }
                                        }
                                    }
                                }
                                
                                for (i = 0; i < reserved_top; ++i){
                                    Editing_File *file = reserved_files[i];
                                    
                                    message = null_string;
                                    if (!file->settings.unimportant){
                                        switch (file_get_sync(file)){
                                            case DirtyState_UnloadedChanges: message = message_unsynced; break;
                                            case DirtyState_UnsavedChanges: message = message_unsaved; break;
                                        }
                                    }
                                    
                                    id.id[0] = (u64)(file);
                                    if (gui_do_file_option(target, id, file->name.live_name, 0, message)){
                                        complete = 1;
                                        copy_ss(&comp_dest, file->name.live_name);
                                    }
                                }
                                
                                end_temp_memory(temp);
                            }
                            
                            gui_end_list(target);
                            
                            gui_end_scrollable(target);
                        }break;
                        
                        case IInt_Sure_To_Close:
                        {
                            i32 action = -1;
                            
                            String empty_str = {0};
                            String message = make_lit_string("There is one or more files unsaved changes, close anyway?");
                            
                            gui_do_text_field(target, message, empty_str);
                            
                            id.id[0] = (u64)('y');
                            message = make_lit_string("(Y)es");
                            if (gui_do_fixed_option(target, id, message, 'y')){
                                action = 0;
                            }
                            
                            id.id[0] = (u64)('n');
                            message = make_lit_string("(N)o");
                            if (gui_do_fixed_option(target, id, message, 'n')){
                                action = 1;
                            }
                            
                            if (action != -1){
                                complete = 1;
                                copy_ss(&comp_dest, view->dest);
                                comp_action = action;
                            }
                        }break;
                        
                        case IInt_Sure_To_Kill:
                        {
                            i32 action = -1;
                            
                            String empty_str = {0};
                            String message = make_lit_string("There are unsaved changes, close anyway?");
                            
                            gui_do_text_field(target, message, empty_str);
                            
                            id.id[0] = (u64)('y');
                            message = make_lit_string("(Y)es");
                            if (gui_do_fixed_option(target, id, message, 'y')){
                                action = 0;
                            }
                            
                            id.id[0] = (u64)('n');
                            message = make_lit_string("(N)o");
                            if (gui_do_fixed_option(target, id, message, 'n')){
                                action = 1;
                            }
                            
                            id.id[0] = (u64)('s');
                            message = make_lit_string("(S)ave and kill");
                            if (gui_do_fixed_option(target, id, message, 's')){
                                action = 2;
                            }
                            
                            if (action != -1){
                                complete = 1;
                                copy_ss(&comp_dest, view->dest);
                                comp_action = action;
                            }
                        }break;
                    }
                    
                    if (complete){
                        terminate_with_null(&comp_dest);
                        interactive_view_complete(system, view, comp_dest, comp_action);
                    }
                }break;
                
                case VUI_Debug:
                {
                    GUI_id scroll_context = {0};
                    scroll_context.id[1] = VUI_Debug + ((u64)view->debug_vars.mode << 32);
                    
                    GUI_id id = {0};
                    id.id[1] = VUI_Debug + ((u64)view->debug_vars.mode << 32);
                    
                    // TODO(allen):
                    // + Incoming input
                    // + Memory info
                    // + Thread info
                    // + View inspection
                    //   - auto generate?
                    //   - expand/collapse sections
                    // - Buffer inspection
                    // - Command maps inspection
                    // - Clipboard inspection
                    
                    String empty_str = null_string;
                    
                    char space1[512];
                    String string = make_fixed_width_string(space1);
                    
                    // Time Watcher
                    {
                        string.size = 0;
                        u64 time = system->now_time();
                        
                        append_ss(&string, make_lit_string("last redraw: "));
                        append_u64_to_str(&string, time);
                        
                        gui_do_text_field(target, string, empty_str);
                    }
                    
                    {
                        i32 prev_mode = view->debug_vars.mode;
                        for (i32 i = 0; i < keys.count; ++i){
                            Key_Event_Data key = get_single_key(&keys, i);
                            
                            if (key.modifiers[MDFR_CONTROL_INDEX] == 0 &&
                                key.modifiers[MDFR_ALT_INDEX] == 0){
                                if (key.keycode == 'i'){
                                    view->debug_vars.mode = DBG_Input;
                                }
                                if (key.keycode == 'm'){
                                    view->debug_vars.mode = DBG_Threads_And_Memory;
                                }
                                if (key.keycode == 'v'){
                                    view->debug_vars.mode = DBG_View_Inspection;
                                }
                            }
                        }
                        if (prev_mode != view->debug_vars.mode){
                            result.consume_keys = 1;
                        }
                    }
                    
                    gui_begin_scrollable(target, scroll_context, view->gui_scroll,
                                         9 * view->line_height, show_scrollbar);
                    
                    switch (view->debug_vars.mode)
                    {
                        case DBG_Input:
                        {
                            Debug_Data *debug = &view->persistent.models->debug;
                            
                            gui_show_mouse(target, &string, input.mouse.x, input.mouse.y);
                            
                            Debug_Input_Event *input_event = debug->input_events;
                            for (i32 i = 0;
                                 i < ArrayCount(debug->input_events);
                                 ++i, ++input_event){
                                string.size = 0;
                                
                                if (input_event->is_hold){
                                    append_ss(&string, make_lit_string("hold:  "));
                                }
                                else{
                                    append_ss(&string, make_lit_string("press: "));
                                }
                                
                                if (input_event->is_ctrl){
                                    append_ss(&string, make_lit_string("ctrl-"));
                                }
                                else{
                                    append_ss(&string, make_lit_string("    -"));
                                }
                                
                                if (input_event->is_alt){
                                    append_ss(&string, make_lit_string("alt-"));
                                }
                                else{
                                    append_ss(&string, make_lit_string("   -"));
                                }
                                
                                if (input_event->is_shift){
                                    append_ss(&string, make_lit_string("shift "));
                                }
                                else{
                                    append_ss(&string, make_lit_string("      "));
                                }
                                
                                if (input_event->key > ' ' && input_event->key <= '~'){
                                    append_ss(&string, make_string(&input_event->key, 1));
                                }
                                else if (input_event->key == ' '){
                                    append_ss(&string, make_lit_string("space"));
                                }
                                else if (input_event->key == '\n'){
                                    append_ss(&string, make_lit_string("\\n"));
                                }
                                else if (input_event->key == '\t'){
                                    append_ss(&string, make_lit_string("\\t"));
                                }
                                else{
                                    String str;
                                    str.str = global_key_name(input_event->key, &str.size);
                                    if (str.str){
                                        str.memory_size = str.size + 1;
                                        append_ss(&string, str);
                                    }
                                    else{
                                        append_ss(&string, make_lit_string("unrecognized!"));
                                    }
                                }
                                
                                if (input_event->consumer[0] != 0){
                                    append_padding(&string, ' ', 40);
                                    append_sc(&string, input_event->consumer);
                                }
                                
                                gui_do_text_field(target, string, empty_str);
                            }
                        }break;
                        
                        case DBG_Threads_And_Memory:
                        {
                            b8 threads[4];
                            i32 pending = 0;
                            
                            if (system->internal_get_thread_states){
                                system->internal_get_thread_states(BACKGROUND_THREADS,
                                                                   threads, &pending);
                                
                                string.size = 0;
                                append_ss(&string, make_lit_string("pending jobs: "));
                                append_int_to_str(&string, pending);
                                gui_do_text_field(target, string, empty_str);
                                
                                for (i32 i = 0; i < 4; ++i){
                                    string.size = 0;
                                    append_ss(&string, make_lit_string("thread "));
                                    append_int_to_str(&string, i);
                                    append_ss(&string, make_lit_string(": "));
                                    
                                    if (threads[i]){
                                        append_ss(&string, make_lit_string("running"));
                                    }
                                    else{
                                        append_ss(&string, make_lit_string("waiting"));
                                    }
                                    
                                    gui_do_text_field(target, string, empty_str);
                                }
                            }
                            
                            Partition *part = &models->mem.part;
                            
                            string.size = 0;
                            append_ss(&string, make_lit_string("part memory: "));
                            append_int_to_str(&string, part->pos);
                            append_s_char(&string, '/');
                            append_int_to_str(&string, part->max);
                            gui_do_text_field(target, string, empty_str);
                            
#if !defined(FED_DEBUG_MEM_H)
                            General_Memory *general = &models->mem.general;
                            Bubble *bubble = 0, *sentinel = &general->sentinel;
                            for (dll_items(bubble, sentinel)){
                                string.size = 0;
                                if (bubble->flags & MEM_BUBBLE_USED){
                                    append_ss(&string, make_lit_string(" used: "));
                                }
                                else{
                                    append_ss(&string, make_lit_string(" free: "));
                                }
                                
                                append_int_to_str(&string, bubble->size);
                                append_padding(&string, ' ', 40);
                                gui_do_text_field(target, string, empty_str);
                            }
#endif
                        }break;
                        
                        case DBG_View_Inspection:
                        {
                            i32 inspecting_id = view->debug_vars.inspecting_view_id;
                            View *views_to_inspect[16];
                            i32 view_count = 0;
                            b32 low_detail = 1;
                            
                            if (inspecting_id == 0){
                                Editing_Layout *layout = &models->layout;
                                
                                Panel *panel, *sentinel;
                                sentinel = &layout->used_sentinel;
                                for (dll_items(panel, sentinel)){
                                    View *view_ptr = panel->view;
                                    views_to_inspect[view_count++] = view_ptr;
                                }
                            }
                            else if (inspecting_id >= 1 && inspecting_id <= 16){
                                Live_Views *live_set = models->live_set;
                                View *view_ptr = live_set->views + inspecting_id - 1;
                                views_to_inspect[view_count++] = view_ptr;
                                low_detail = 0;
                            }
                            
                            for (i32 i = 0; i < view_count; ++i){
                                View *view_ptr = views_to_inspect[i];
                                
                                string.size = 0;
                                append_ss(&string, make_lit_string("view: "));
                                append_int_to_str(&string, view_ptr->persistent.id + 1);
                                gui_do_text_field(target, string, empty_str);
                                
                                string.size = 0;
                                Editing_File *file = view_ptr->file_data.file;
                                append_ss(&string, make_lit_string(" > buffer: "));
                                if (file){
                                    append_ss(&string, file->name.live_name);
                                    gui_do_text_field(target, string, empty_str);
                                    string.size = 0;
                                    append_ss(&string, make_lit_string(" >> buffer-slot-id: "));
                                    append_int_to_str(&string, file->id.id);
                                }
                                else{
                                    append_ss(&string, make_lit_string("*NULL*"));
                                    gui_do_text_field(target, string, empty_str);
                                }
                                
                                if (low_detail){
                                    string.size = 0;
                                    append_ss(&string, make_lit_string("inspect this"));
                                    
                                    id.id[0] = (u64)(view_ptr->persistent.id);
                                    if (gui_do_button(target, id, string)){
                                        view->debug_vars.inspecting_view_id = view_ptr->persistent.id + 1;
                                    }
                                }
                                else{
                                    
                                    gui_show_mouse(target, &string, input.mouse.x, input.mouse.y);
                                    
#define SHOW_GUI_BLANK(n)            show_gui_line(target, &string, n, 0, "", 0)
#define SHOW_GUI_LINE(n, str)        show_gui_line(target, &string, n, 0, " " str, 0)
#define SHOW_GUI_STRING(n, h, str, mes) show_gui_line(target, &string, n, h, " " str " ", mes)
#define SHOW_GUI_INT(n, h, str, v)   show_gui_int(target, &string, n, h, " " str " ", v)
#define SHOW_GUI_INT_INT(n, h, str, v, m) show_gui_int_int(target, &string, n, h, " " str " ", v, m)
#define SHOW_GUI_U64(n, h, str, v)   show_gui_u64(target, &string, n, h, " " str " ", v)
#define SHOW_GUI_ID(n, h, str, v)    show_gui_id(target, &string, n, h, " " str, v)
#define SHOW_GUI_FLOAT(n, h, str, v) show_gui_float(target, &string, n, h, " " str " ", v)
#define SHOW_GUI_BOOL(n, h, str, v)  do { if (v) { show_gui_line(target, &string, n, h, " " str " ", "true"); }\
                                        else { show_gui_line(target, &string, n, h, " " str " ", "false"); } } while(0)
                                    
#define SHOW_GUI_SCROLL(n, h, str, v) show_gui_scroll(target, &string, n, h, " " str, v)
#define SHOW_GUI_CURSOR(n, h, str, v) show_gui_cursor(target, &string, n, h, " " str, v)
#define SHOW_GUI_REGION(n, h, str, v) show_gui_region(target, &string, n, h, " " str, v)
                                    
                                    i32 h_align = 31;
                                    
                                    SHOW_GUI_BLANK(0);
                                    {
                                        Command_Map *map = view_ptr->map;
                                        
#define MAP_LABEL "command map"
                                        
                                        if (map == &models->map_top){
                                            SHOW_GUI_STRING(1, h_align, MAP_LABEL, "global");
                                        }
                                        else if (map == &models->map_file){
                                            SHOW_GUI_STRING(1, h_align, MAP_LABEL, "file");
                                        }
                                        else if (map == &models->map_ui){
                                            SHOW_GUI_STRING(1, h_align, MAP_LABEL, "gui");
                                        }
                                        else{
                                            i32 map_index = (i32)(view_ptr->map - models->user_maps);
                                            i32 map_id = models->map_id_table[map_index];
                                            
                                            SHOW_GUI_STRING(1, h_align, MAP_LABEL, "user");
                                            SHOW_GUI_INT(2, h_align, "custom map id", map_id);
                                        }
                                    }
                                    
                                    SHOW_GUI_BLANK(0);
                                    SHOW_GUI_LINE(1, "file data:");
                                    SHOW_GUI_BOOL(2, h_align, "has file", view_ptr->file_data.file);
                                    SHOW_GUI_BOOL(2, h_align, "show temp highlight", view_ptr->file_data.show_temp_highlight);
                                    SHOW_GUI_INT (2, h_align, "start temp highlight", view_ptr->file_data.temp_highlight.pos);
                                    SHOW_GUI_INT (2, h_align, "end temp highlight", view_ptr->file_data.temp_highlight_end_pos);
                                    
                                    SHOW_GUI_BOOL(2, h_align, "show whitespace", view_ptr->file_data.show_whitespace);
                                    SHOW_GUI_BOOL(2, h_align, "locked", view_ptr->file_data.file_locked);
                                    
                                    SHOW_GUI_BLANK(0);
                                    SHOW_GUI_REGION(1, h_align, "scroll region", view_ptr->scroll_region);
                                    
                                    SHOW_GUI_BLANK(0);
                                    SHOW_GUI_LINE(1, "file editing positions");
                                    {
                                        File_Edit_Positions *edit_pos = view_ptr->edit_pos;
                                        
                                        if (edit_pos){
                                            SHOW_GUI_SCROLL(2, h_align, "scroll:", edit_pos->scroll);
                                            SHOW_GUI_BLANK (2);
                                            SHOW_GUI_CURSOR(2, h_align, "cursor:", edit_pos->cursor);
                                            SHOW_GUI_BLANK (2);
                                            SHOW_GUI_INT   (2, h_align, "mark", edit_pos->mark);
                                            SHOW_GUI_FLOAT (2, h_align, "preferred_x", edit_pos->preferred_x);
                                            SHOW_GUI_INT   (2, h_align, "scroll_i", edit_pos->scroll_i);
                                        }
                                        else{
                                            SHOW_GUI_LINE(2, "NULL");
                                        }
                                    }
                                    
                                    SHOW_GUI_BLANK (0);
                                    SHOW_GUI_SCROLL(1, h_align, "gui scroll:", view_ptr->gui_scroll);
                                    
                                    SHOW_GUI_BLANK  (0);
                                    SHOW_GUI_LINE   (1, "gui target");
                                    SHOW_GUI_INT_INT(2, h_align, "gui partition",
                                                     view_ptr->gui_target.push.pos,
                                                     view_ptr->gui_target.push.max);
                                    
                                    SHOW_GUI_BLANK  (2);
                                    SHOW_GUI_ID     (2, h_align, "active", view_ptr->gui_target.active);
                                    SHOW_GUI_ID     (2, h_align, "mouse_hot", view_ptr->gui_target.mouse_hot);
                                    SHOW_GUI_ID     (2, h_align, "auto_hot", view_ptr->gui_target.auto_hot);
                                    SHOW_GUI_ID     (2, h_align, "hover", view_ptr->gui_target.hover);
                                    SHOW_GUI_ID     (2, h_align, "scroll_id", view_ptr->gui_target.scroll_id);
                                    
                                    SHOW_GUI_BLANK  (2);
                                    SHOW_GUI_SCROLL (2, h_align, "scroll_original", view_ptr->gui_target.scroll_original);
                                    SHOW_GUI_REGION (2, h_align, "region_original", view_ptr->gui_target.region_original);
                                    
                                    SHOW_GUI_BLANK  (2);
                                    SHOW_GUI_REGION (2, h_align, "region_updated", view_ptr->gui_target.region_updated);
                                    
                                    
                                    SHOW_GUI_BLANK  (1);
                                    SHOW_GUI_SCROLL (1, h_align, "gui scroll", view_ptr->gui_scroll);
                                }
                            }
                        }break;
                    }
                    
                    gui_end_scrollable(target);
                }break;
            }
        }
    }
    gui_end_top_level(target);
    
    result.animating = target->animating;
    return(result);
}

internal f32
view_get_scroll_y(View *view){
    f32 v = 0;
    if (view->showing_ui == VUI_None){
        File_Edit_Positions *edit_pos = view->edit_pos;
        if (edit_pos){
            v = edit_pos->scroll.scroll_y;
        }
    }
    else{
        v = view->gui_scroll.scroll_y;
    }
    return(v);
}

internal b32
click_button_input(GUI_Target *target, GUI_Session *session, Input_Summary *user_input,
                   GUI_Interactive *b, b32 *is_animating){
    b32 result = 0;
    i32 mx = user_input->mouse.x;
    i32 my = user_input->mouse.y;
    
    if (hit_check(mx, my, session->rect)){
        target->hover = b->id;
        if (user_input->mouse.press_l){
            target->mouse_hot = b->id;
            *is_animating = 1;
            result = 1;
        }
        if (user_input->mouse.release_l && gui_id_eq(target->mouse_hot, b->id)){
            target->active = b->id;
            target->mouse_hot = gui_id_zero();
            *is_animating = 1;
        }
    }
    else if (gui_id_eq(target->hover, b->id)){
        target->hover = gui_id_zero();
    }
    return(result);
}

internal b32
scroll_button_input(GUI_Target *target, GUI_Session *session, Input_Summary *user_input,
                    GUI_id id, b32 *is_animating){
    b32 result = 0;
    i32 mx = user_input->mouse.x;
    i32 my = user_input->mouse.y;
    
    if (hit_check(mx, my, session->rect)){
        target->hover = id;
        if (user_input->mouse.l){
            target->mouse_hot = id;
            gui_activate_scrolling(target);
            *is_animating = 1;
            result = 1;
        }
    }
    else if (gui_id_eq(target->hover, id)){
        target->hover = gui_id_zero();
    }
    return(result);
}

struct Input_Process_Result{
    GUI_Scroll_Vars vars;
    i32_Rect region;
    b32 is_animating;
    b32 consumed_l;
    b32 consumed_r;
    
    b32 has_max_y_suggestion;
    i32 max_y;
};

static char
to_writable_char(Key_Code long_character){
    char character = 0;
    if (long_character < ' '){
        if (long_character == '\n'){
            character = '\n';
        }
        else if (long_character == '\t'){
            character = '\t';
        }
    }
    else if (long_character >= ' ' && long_character <= 255 && long_character != 127){
        character = (char)long_character;
    }
    return(character);
}

internal Input_Process_Result
do_step_file_view(System_Functions *system, View *view, i32_Rect rect, b32 is_active, Input_Summary *user_input, GUI_Scroll_Vars vars, i32_Rect region, i32 max_y){
    Input_Process_Result result = {0};
    b32 is_file_scroll = 0;
    
    GUI_Session gui_session = {0};
    GUI_Header *h = 0;
    GUI_Target *target = &view->gui_target;
    GUI_Interpret_Result interpret_result = {0};
    
    vars.target_y = clamp(0, vars.target_y, max_y);
    
    result.vars = vars;
    result.region = region;
    
    target->active = gui_id_zero();
    
    if (target->push.pos > 0){
        gui_session_init(&gui_session, target, rect, view->line_height);
        
        for (h = (GUI_Header*)target->push.base;
             h->type;
             h = NextHeader(h)){
            interpret_result = gui_interpret(target, &gui_session, h,
                                             result.vars, result.region, max_y);
            
            if (interpret_result.has_region){
                result.region = interpret_result.region;
            }
            
            switch (h->type){
                case guicom_file_option:
                case guicom_fixed_option:
                case guicom_fixed_option_checkbox:
                {
                    GUI_Interactive *b = (GUI_Interactive*)h;
                    
                    if (interpret_result.auto_activate){
                        target->auto_hot = gui_id_zero();
                        target->active = b->id;
                        result.is_animating = 1;
                    }
                    else if (interpret_result.auto_hot){
                        if (!gui_id_eq(target->auto_hot, b->id)){
                            target->auto_hot = b->id;
                            result.is_animating = 1;
                        }
                    }
                }break;
            }
            
            if (interpret_result.has_info){
                switch (h->type){
                    case guicom_top_bar: break;
                    
                    case guicom_file:
                    {
                        // NOTE(allen): Set the file region first because the
                        // computation of view_compute_max_target_y depends on it.
                        view->file_region = gui_session.rect;
                        
                        if (view->reinit_scrolling){
                            result.vars = view_reinit_scrolling(view);
                            result.is_animating = 1;
                        }
                        
                        if (file_step(view, gui_session.rect, user_input, is_active, &result.consumed_l)){
                            result.is_animating = 1;
                        }
                        is_file_scroll = 1;
                    }break;
                    
                    case guicom_color_button:
                    case guicom_font_button:
                    case guicom_button:
                    case guicom_file_option:
                    case guicom_style_preview:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        
                        if (click_button_input(target, &gui_session, user_input,
                                               b, &result.is_animating)){
                            result.consumed_l = 1;
                        }
                    }break;
                    
                    case guicom_fixed_option:
                    case guicom_fixed_option_checkbox:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        
                        if (click_button_input(target, &gui_session, user_input,
                                               b, &result.is_animating)){
                            result.consumed_l = 1;
                        }
                        
                        {
                            Key_Input_Data *keys = &user_input->keys;
                            
                            void *ptr = (b + 1);
                            String string = gui_read_string(&ptr);
                            AllowLocal(string);
                            
                            char activation_key = *(char*)ptr;
                            activation_key = char_to_upper(activation_key);
                            
                            if (activation_key != 0){
                                i32 count = keys->count;
                                for (i32 i = 0; i < count; ++i){
                                    Key_Event_Data key = get_single_key(keys, i);
                                    char character = to_writable_char(key.character);
                                    if (char_to_upper(character) == activation_key){
                                        target->active = b->id;
                                        result.is_animating = 1;
                                        break;
                                    }
                                }
                            }
                        }
                    }break;
                    
                    case guicom_scrollable_slider:
                    {
                        GUI_id id = gui_id_scrollbar_slider();
                        i32 mx = user_input->mouse.x;
                        i32 my = user_input->mouse.y;
                        f32 v = 0;
                        
                        if (hit_check(mx, my, gui_session.rect)){
                            target->hover = id;
                            if (user_input->mouse.press_l){
                                target->mouse_hot = id;
                                result.is_animating = 1;
                                result.consumed_l = 1;
                            }
                        }
                        else if (gui_id_eq(target->hover, id)){
                            target->hover = gui_id_zero();
                        }
                        
                        if (gui_id_eq(target->mouse_hot, id)){
                            v = unlerp(gui_session.scroll_top, (f32)my,
                                       gui_session.scroll_bottom);
                            v = clamp(0.f, v, 1.f);
                            result.vars.target_y = round32(lerp(0.f, v, (f32)max_y));
                            
                            gui_activate_scrolling(target);
                            result.is_animating = 1;
                        }
                    }
                    // NOTE(allen): NO BREAK HERE!!
                    
                    case guicom_scrollable_invisible:
                    {
                        if (user_input->mouse.wheel != 0){
                            result.vars.target_y += user_input->mouse.wheel*target->delta;
                            
                            result.vars.target_y =
                                clamp(0, result.vars.target_y, max_y);
                            gui_activate_scrolling(target);
                            result.is_animating = 1;
                        }
                    }break;
                    
                    case guicom_scrollable_top:
                    {
                        GUI_id id = gui_id_scrollbar_top();
                        
                        if (scroll_button_input(target, &gui_session, user_input, id, &result.is_animating)){
                            result.vars.target_y -= clamp_bottom(1, target->delta >> 2);
                            result.vars.target_y = clamp_bottom(0, result.vars.target_y);
                            result.consumed_l = 1;
                        }
                    }break;
                    
                    case guicom_scrollable_bottom:
                    {
                        GUI_id id = gui_id_scrollbar_bottom();
                        
                        if (scroll_button_input(target, &gui_session, user_input, id, &result.is_animating)){
                            result.vars.target_y += clamp_bottom(1, target->delta >> 2);
                            result.vars.target_y = clamp_top(result.vars.target_y, max_y);
                            result.consumed_l = 1;
                        }
                    }break;
                    
                    case guicom_end_scrollable_section:
                    {
                        if (!is_file_scroll){
                            result.has_max_y_suggestion = 1;
                            result.max_y = gui_session.suggested_max_y;
                        }
                    }break;
                }
            }
        }
        
        if (!user_input->mouse.l){
            if (!gui_id_is_null(target->mouse_hot)){
                target->mouse_hot = gui_id_zero();
                result.is_animating = 1;
            }
        }
        
        {
            GUI_Scroll_Vars scroll_vars = result.vars;
            b32 is_new_target = 0;
            if (scroll_vars.target_x != scroll_vars.prev_target_x ||
                scroll_vars.target_y != scroll_vars.prev_target_y){
                is_new_target = 1;
            }
            
            f32 target_x = (f32)scroll_vars.target_x;
            f32 target_y = (f32)scroll_vars.target_y;
            
            if (view->persistent.models->scroll_rule(target_x, target_y,
                                                     &scroll_vars.scroll_x, &scroll_vars.scroll_y,
                                                     (view->persistent.id) + 1, is_new_target, user_input->dt)){
                result.is_animating = 1;
            }
            
            scroll_vars.prev_target_x = scroll_vars.target_x;
            scroll_vars.prev_target_y = scroll_vars.target_y;
            
            result.vars = scroll_vars;
        }
    }
    
    return(result);
}

internal i32
draw_file_loaded(View *view, i32_Rect rect, b32 is_active, Render_Target *target){
    Models *models = view->persistent.models;
    Editing_File *file = view->file_data.file;
    Style *style = main_style(models);
    i32 line_height = view->line_height;
    
    f32 max_x = view_file_display_width(view);
    i32 max_y = rect.y1 - rect.y0 + line_height;
    
    Assert(file && !file->is_dummy && buffer_good(&file->state.buffer));
    Assert(view->edit_pos);
    
    b32 tokens_use = 0;
    Cpp_Token_Array token_array = {};
    if (file){
        tokens_use = file->state.tokens_complete && (file->state.token_array.count > 0);
        token_array = file->state.token_array;
    }
    
    Partition *part = &models->mem.part;
    Temp_Memory temp = begin_temp_memory(part);
    
    partition_align(part, 4);
    
    f32 left_side_space = 0;
    
    i32 max = partition_remaining(part) / sizeof(Buffer_Render_Item);
    Buffer_Render_Item *items = push_array(part, Buffer_Render_Item, max);
    
    i16 font_id = file->settings.font_id;
    Render_Font *font = get_font_info(models->font_set, font_id)->font;
    
    f32 scroll_x = view->edit_pos->scroll.scroll_x;
    f32 scroll_y = view->edit_pos->scroll.scroll_y;
    
    // NOTE(allen): For now we will temporarily adjust scroll_y to try
    // to prevent the view moving around until floating sections are added
    // to the gui system.
    scroll_y += view->widget_height;
    
    Full_Cursor render_cursor;
    if (!file->settings.unwrapped_lines){
        render_cursor = view_compute_cursor(view, seek_wrapped_xy(0, scroll_y, 0), 1);
    }
    else{
        render_cursor = view_compute_cursor(view, seek_unwrapped_xy(0, scroll_y, 0), 1);
    }
    
    view->edit_pos->scroll_i = render_cursor.pos;
    
    i32 count = 0;
    {
        b32 wrapped = !file->settings.unwrapped_lines;
        
        Buffer_Render_Params params;
        params.buffer        = &file->state.buffer;
        params.items         = items;
        params.max           = max;
        params.count         = &count;
        params.port_x        = (f32)rect.x0 + left_side_space;
        params.port_y        = (f32)rect.y0;
        params.clip_w        = view_width(view) - left_side_space;
        params.scroll_x      = scroll_x;
        params.scroll_y      = scroll_y;
        params.width         = max_x;
        params.height        = (f32)max_y;
        params.start_cursor  = render_cursor;
        params.wrapped       = wrapped;
        params.font_height   = (f32)line_height;
        params.cp_adv        = font->codepoint_advance_data;
        params.byte_adv      = font->byte_advance;
        params.virtual_white = file->settings.virtual_white;
        params.wrap_slashes  = file->settings.wrap_indicator;
        
        Buffer_Render_State state = {0};
        Buffer_Layout_Stop stop = {0};
        
        f32 line_shift = 0.f;
        b32 do_wrap = 0;
        i32 wrap_unit_end = 0;
        
        b32 first_wrap_determination = 1;
        i32 wrap_array_index = 0;
        
        do{
            stop = buffer_render_data(&state, params, line_shift, do_wrap, wrap_unit_end);
            switch (stop.status){
                case BLStatus_NeedWrapDetermination:
                {
                    if (first_wrap_determination){
                        wrap_array_index = binary_search(file->state.wrap_positions, stop.pos, 0, file->state.wrap_position_count);
                        ++wrap_array_index;
                        if (file->state.wrap_positions[wrap_array_index] == stop.pos){
                            do_wrap = 1;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                        else{
                            do_wrap = 0;
                            wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                        }
                        first_wrap_determination = 0;
                    }
                    else{
                        assert(stop.pos == wrap_unit_end);
                        do_wrap = 1;
                        ++wrap_array_index;
                        wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                    }
                }break;
                
                case BLStatus_NeedWrapLineShift:
                case BLStatus_NeedLineShift:
                {
                    line_shift = file->state.line_indents[stop.wrap_line_index];
                }break;
            }
        }while(stop.status != BLStatus_Finished);
    }
    
    i32 cursor_begin = 0, cursor_end = 0;
    u32 cursor_color = 0, at_cursor_color = 0;
    if (view->file_data.show_temp_highlight){
        cursor_begin = view->file_data.temp_highlight.pos;
        cursor_end = view->file_data.temp_highlight_end_pos;
        cursor_color = style->main.highlight_color;
        at_cursor_color = style->main.at_highlight_color;
    }
    else{
        cursor_begin = view->edit_pos->cursor.pos;
        cursor_end = cursor_begin + 1;
        cursor_color = style->main.cursor_color;
        at_cursor_color = style->main.at_cursor_color;
    }
    
    i32 token_i = 0;
    u32 main_color = style->main.default_color;
    u32 special_color = style->main.special_character_color;
    u32 ghost_color = style->main.ghost_character_color;
    if (tokens_use){
        Cpp_Get_Token_Result result = cpp_get_token(token_array, items->index);
        main_color = *style_get_color(style, token_array.tokens[result.token_index]);
        token_i = result.token_index + 1;
    }
    
    u32 mark_color = style->main.mark_color;
    Buffer_Render_Item *item = items;
    Buffer_Render_Item *item_end = item + count;
    i32 prev_ind = -1;
    u32 highlight_color = 0;
    u32 highlight_this_color = 0;
    
    for (; item < item_end; ++item){
        i32 ind = item->index;
        highlight_this_color = 0;
        if (tokens_use && ind != prev_ind){
            Cpp_Token current_token = token_array.tokens[token_i-1];
            
            if (token_i < token_array.count){
                if (ind >= token_array.tokens[token_i].start){
                    for (;ind >= token_array.tokens[token_i].start && token_i < token_array.count; ++token_i){
                        main_color = *style_get_color(style, token_array.tokens[token_i]);
                        current_token = token_array.tokens[token_i];
                    }
                }
                else if (ind >= current_token.start + current_token.size){
                    main_color = style->main.default_color;
                }
            }
            
            if (current_token.type == CPP_TOKEN_JUNK && ind >= current_token.start && ind < current_token.start + current_token.size){
                highlight_color = style->main.highlight_junk_color;
            }
            else{
                highlight_color = 0;
            }
        }
        
        u32 char_color = main_color;
        if (item->flags & BRFlag_Special_Character){
            char_color = special_color;
        }
        else if (item->flags & BRFlag_Ghost_Character){
            char_color = ghost_color;
        }
        
        f32_Rect char_rect = f32R(item->x0, item->y0, item->x1,  item->y1);
        
        if (view->file_data.show_whitespace && highlight_color == 0 &&
            char_is_whitespace((char)item->glyphid)){
            highlight_this_color = style->main.highlight_white_color;
        }
        else{
            highlight_this_color = highlight_color;
        }
        
        if (cursor_begin <= ind && ind < cursor_end && (ind != prev_ind || cursor_begin < ind)){
            if (is_active){
                draw_rectangle(target, char_rect, cursor_color);
                char_color = at_cursor_color;
            }
            else{
                if (!view->file_data.show_temp_highlight){
                    draw_rectangle_outline(target, char_rect, cursor_color);
                }
            }
        }
        else if (highlight_this_color){
            draw_rectangle(target, char_rect, highlight_this_color);
        }
        
        u32 fade_color = 0xFFFF00FF;
        f32 fade_amount = 0.f;
        
        if (file->state.paste_effect.seconds_down > 0.f &&
            file->state.paste_effect.start <= ind &&
            ind < file->state.paste_effect.end){
            fade_color = file->state.paste_effect.color;
            fade_amount = file->state.paste_effect.seconds_down;
            fade_amount /= file->state.paste_effect.seconds_max;
        }
        
        char_color = color_blend(char_color, fade_amount, fade_color);
        
        if (ind == view->edit_pos->mark && prev_ind != ind){
            draw_rectangle_outline(target, char_rect, mark_color);
        }
        if (item->glyphid != 0){
            font_draw_glyph(target, font_id, (u8)item->glyphid, item->x0, item->y0, char_color);
        }
        prev_ind = ind;
    }
    
    end_temp_memory(temp);
    
    return(0);
}

internal void
draw_text_field(Render_Target *target, View *view, i16 font_id,
                i32_Rect rect, String p, String t){
    Models *models = view->persistent.models;
    Style *style = main_style(models);
    
    u32 back_color = style->main.margin_color;
    u32 text1_color = style->main.default_color;
    u32 text2_color = style->main.file_info_style.pop1_color;
    
    i32 x = rect.x0;
    i32 y = rect.y0 + 2;
    
    if (target){
        draw_rectangle(target, rect, back_color);
        x = ceil32(draw_string(target, font_id, p, x, y, text2_color));
        draw_string(target, font_id, t, x, y, text1_color);
    }
}

internal void
draw_text_with_cursor(Render_Target *target, View *view, i16 font_id, i32_Rect rect, String s, i32 pos){
    Models *models = view->persistent.models;
    Style *style = main_style(models);
    
    u32 back_color = style->main.margin_color;
    u32 text_color = style->main.default_color;
    u32 cursor_color = style->main.cursor_color;
    u32 at_cursor_color = style->main.at_cursor_color;
    
    f32 x = (f32)rect.x0;
    i32 y = rect.y0 + 2;
    
    if (target){
        draw_rectangle(target, rect, back_color);
        
        if (pos >= 0 && pos <  s.size){
            String part1, part2, part3;
            i32_Rect cursor_rect;
            Render_Font *font = get_font_info(models->font_set, font_id)->font;
            
            part1 = substr(s, 0, pos);
            part2 = substr(s, pos, 1);
            part3 = substr(s, pos+1, s.size-pos-1);
            
            
            x = draw_string(target, font_id, part1, floor32(x), y, text_color);
            
            // TODO(allen): WHAT TO DO NOW?
            cursor_rect.x0 = floor32(x);
            cursor_rect.x1 = floor32(x) + ceil32(font->codepoint_advance_data[s.str[pos]]);
            cursor_rect.y0 = y;
            cursor_rect.y1 = y + view->line_height;
            draw_rectangle(target, cursor_rect, cursor_color);
            x = draw_string(target, font_id, part2, floor32(x), y, at_cursor_color);
            
            draw_string(target, font_id, part3, floor32(x), y, text_color);
        }
        else{
            draw_string(target, font_id, s, floor32(x), y, text_color);
        }
    }
}

internal void
draw_file_bar(Render_Target *target, View *view, Editing_File *file, i32_Rect rect){
    File_Bar bar;
    Models *models = view->persistent.models;
    Style *style = main_style(models);
    Interactive_Style bar_style = style->main.file_info_style;
    
    u32 back_color = bar_style.bar_color;
    u32 base_color = bar_style.base_color;
    u32 pop1_color = bar_style.pop1_color;
    u32 pop2_color = bar_style.pop2_color;
    
    bar.rect = rect;
    
    if (target){
        bar.font_id = file->settings.font_id;
        bar.pos_x = (f32)bar.rect.x0;
        bar.pos_y = (f32)bar.rect.y0;
        bar.text_shift_y = 2;
        bar.text_shift_x = 0;
        
        draw_rectangle(target, bar.rect, back_color);
        
        Assert(file);
        
        intbar_draw_string(target, &bar, file->name.live_name, base_color);
        intbar_draw_string(target, &bar, make_lit_string(" -"), base_color);
        
        if (file->is_loading){
            intbar_draw_string(target, &bar, make_lit_string(" loading"), base_color);
        }
        else{
            char bar_space[526];
            String bar_text = make_fixed_width_string(bar_space);
            append_ss        (&bar_text, make_lit_string(" L#"));
            append_int_to_str(&bar_text, view->edit_pos->cursor.line);
            append_ss        (&bar_text, make_lit_string(" C#"));
            append_int_to_str(&bar_text, view->edit_pos->cursor.character);
            
            append_ss(&bar_text, make_lit_string(" -"));
            
            if (file->settings.dos_write_mode){
                append_ss(&bar_text, make_lit_string(" dos"));
            }
            else{
                append_ss(&bar_text, make_lit_string(" nix"));
            }
            
            intbar_draw_string(target, &bar, bar_text, base_color);
            
            
            if (file->state.still_lexing){
                intbar_draw_string(target, &bar, make_lit_string(" parsing"), pop1_color);
            }
            
            if (!file->settings.unimportant){
                switch (file_get_sync(file)){
                    case DirtyState_UnloadedChanges:
                    {
                        local_persist String out_of_sync = make_lit_string(" !");
                        intbar_draw_string(target, &bar, out_of_sync, pop2_color);
                    }break;
                    
                    case DirtyState_UnsavedChanges:
                    {
                        local_persist String out_of_sync = make_lit_string(" *");
                        intbar_draw_string(target, &bar, out_of_sync, pop2_color);
                    }break;
                }
            }
        }
    }
}

u32
get_margin_color(i32 active_level, Style *style){
    u32 margin = 0xFFFFFFFF;
    
    switch (active_level){
        default:
        margin = style->main.margin_color;
        break;
        
        case 1: case 2:
        margin = style->main.margin_hover_color;
        break;
        
        case 3: case 4:
        margin = style->main.margin_active_color;
        break;
    }
    
    return(margin);
}

internal void
draw_color_button(GUI_Target *gui_target, Render_Target *target, View *view,
                  i16 font_id, i32_Rect rect, GUI_id id, u32 fore, u32 back, String text){
    i32 active_level = gui_active_level(gui_target, id);
    
    if (active_level > 0){
        Swap(u32, back, fore);
    }
    
    draw_rectangle(target, rect, back);
    draw_string(target, font_id, text, rect.x0, rect.y0 + 1, fore);
}

internal void
draw_font_button(GUI_Target *gui_target, Render_Target *target, View *view,
                 i32_Rect rect, GUI_id id, i16 font_id, String text){
    Models *models = view->persistent.models;
    Style *style = main_style(models);
    
    i32 active_level = gui_active_level(gui_target, id);
    
    u32 margin_color = get_margin_color(active_level, style);
    u32 back_color = style->main.back_color;
    u32 text_color = style->main.default_color;
    
    draw_rectangle(target, rect, back_color);
    draw_rectangle_outline(target, rect, margin_color);
    draw_string(target, font_id, text, rect.x0, rect.y0 + 1, text_color);
}

internal void
draw_fat_option_block(GUI_Target *gui_target, Render_Target *target, View *view,
                      i16 font_id, i32_Rect rect, GUI_id id,
                      String text, String pop, i8 checkbox = -1){
    Models *models = view->persistent.models;
    Style *style = main_style(models);
    
    i32 active_level = gui_active_level(gui_target, id);
    
    i32_Rect inner = get_inner_rect(rect, 3);
    
    u32 margin = get_margin_color(active_level, style);
    u32 back = style->main.back_color;
    u32 text_color = style->main.default_color;
    u32 pop_color = style->main.file_info_style.pop2_color;
    
    i32 h = view->line_height;
    i32 x = inner.x0 + 3;
    i32 y = inner.y0 + h/2 - 1;
    
    draw_rectangle(target, inner, back);
    draw_margin(target, rect, inner, margin);
    
    if (checkbox != -1){
        u32 checkbox_color = style->main.margin_active_color;
        i32_Rect checkbox_rect = get_inner_rect(inner, (inner.y1 - inner.y0 - h)/2);
        checkbox_rect.x1 = checkbox_rect.x0 + (checkbox_rect.y1 - checkbox_rect.y0);
        
        if (checkbox == 0){
            draw_rectangle_outline(target, checkbox_rect, checkbox_color);
        }
        else{
            draw_rectangle(target, checkbox_rect, checkbox_color);
        }
        
        x = checkbox_rect.x1 + 3;
    }
    
    x = ceil32(draw_string(target, font_id, text, x, y, text_color));
    draw_string(target, font_id, pop, x, y, pop_color);
}

internal void
draw_button(GUI_Target *gui_target, Render_Target *target, View *view, i16 font_id, i32_Rect rect, GUI_id id, String text){
    Models *models = view->persistent.models;
    Style *style = main_style(models);
    
    i32 active_level = gui_active_level(gui_target, id);
    
    i32_Rect inner = get_inner_rect(rect, 3);
    
    u32 margin = style->main.default_color;
    u32 back = get_margin_color(active_level, style);
    u32 text_color = style->main.default_color;
    
    i32 h = view->line_height;
    i32 y = inner.y0 + h/2 - 1;
    
    i32 w = (i32)font_string_width(target, font_id, text);
    i32 x = (inner.x1 + inner.x0 - w)/2;
    
    draw_rectangle(target, inner, back);
    draw_rectangle_outline(target, inner, margin);
    
    draw_string(target, font_id, text, x, y, text_color);
}

internal void
draw_style_preview(GUI_Target *gui_target, Render_Target *target, View *view,
                   i16 font_id, i32_Rect rect, GUI_id id, Style *style){
    Models *models = view->persistent.models;
    
    i32 active_level = gui_active_level(gui_target, id);
    Font_Info *info = get_font_info(models->font_set, font_id);
    
    i32_Rect inner = get_inner_rect(rect, 3);
    
    u32 margin_color = get_margin_color(active_level, style);
    u32 back = style->main.back_color;
    u32 text_color = style->main.default_color;
    u32 keyword_color = style->main.keyword_color;
    u32 int_constant_color = style->main.int_constant_color;
    u32 comment_color = style->main.comment_color;
    
    draw_margin(target, rect, inner, margin_color);
    draw_rectangle(target, inner, back);
    
    i32 y = inner.y0;
    i32 x = inner.x0;
    x = ceil32(draw_string(target, font_id, style->name.str, x, y, text_color));
    i32 font_x = (i32)(inner.x1 - font_string_width(target, font_id, info->name.str));
    if (font_x > x + 10){
        draw_string(target, font_id, info->name.str, font_x, y, text_color);
    }
    
    x = inner.x0;
    y += info->height;
    x = ceil32(draw_string(target, font_id, "if", x, y, keyword_color));
    x = ceil32(draw_string(target, font_id, "(x < ", x, y, text_color));
    x = ceil32(draw_string(target, font_id, "0", x, y, int_constant_color));
    x = ceil32(draw_string(target, font_id, ") { x = ", x, y, text_color));
    x = ceil32(draw_string(target, font_id, "0", x, y, int_constant_color));
    x = ceil32(draw_string(target, font_id, "; } ", x, y, text_color));
    x = ceil32(draw_string(target, font_id, "// comment", x, y, comment_color));
    
    x = inner.x0;
    y += info->height;
    draw_string(target, font_id, "[] () {}; * -> +-/ <>= ! && || % ^", x, y, text_color);
}

internal i32
do_render_file_view(System_Functions *system, View *view, GUI_Scroll_Vars *scroll,
                    View *active, i32_Rect rect, b32 is_active,
                    Render_Target *target, Input_Summary *user_input){
    
    Editing_File *file = view->file_data.file;
    i32 result = 0;
    
    GUI_Session gui_session = {0};
    GUI_Header *h = 0;
    GUI_Target *gui_target = &view->gui_target;
    GUI_Interpret_Result interpret_result = {0};
    
    f32 v = {0};
    i32 max_y = view_compute_max_target_y(view);
    i16 font_id = 0;
    
    Assert(file != 0);
    
    font_id = file->settings.font_id;
    if (gui_target->push.pos > 0){
        gui_session_init(&gui_session, gui_target, rect, view->line_height);
        
        v = view_get_scroll_y(view);
        
        i32_Rect clip_rect = rect;
        draw_push_clip(target, clip_rect);
        
        for (h = (GUI_Header*)gui_target->push.base;
             h->type;
             h = NextHeader(h)){
            interpret_result = gui_interpret(gui_target, &gui_session, h,
                                             *scroll, view->scroll_region, max_y);
            
            if (interpret_result.has_info){
                if (gui_session.clip_y > clip_rect.y0){
                    clip_rect.y0 = gui_session.clip_y;
                    draw_change_clip(target, clip_rect);
                }
                
                switch (h->type){
                    case guicom_top_bar:
                    {
                        draw_file_bar(target, view, file, gui_session.rect);
                    }break;
                    
                    case guicom_file:
                    {
                        if (file_is_ready(file)){
                            result = draw_file_loaded(view, gui_session.rect, is_active, target);
                        }
                    }break;
                    
                    case guicom_text_field:
                    {
                        void *ptr = (h+1);
                        String p = gui_read_string(&ptr);
                        String t = gui_read_string(&ptr);
                        draw_text_field(target, view, font_id, gui_session.rect, p, t);
                    }break;
                    
                    case guicom_text_with_cursor:
                    {
                        void *ptr = (h+1);
                        String s = gui_read_string(&ptr);
                        i32 pos = gui_read_integer(&ptr);
                        
                        draw_text_with_cursor(target, view, font_id, gui_session.rect, s, pos);
                    }break;
                    
                    case guicom_color_button:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        u32 fore = (u32)gui_read_integer(&ptr);
                        u32 back = (u32)gui_read_integer(&ptr);
                        String t = gui_read_string(&ptr);
                        
                        draw_color_button(gui_target, target, view, font_id, gui_session.rect, b->id, fore, back, t);
                    }break;
                    
                    case guicom_font_button:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        i16 font_id = (i16)gui_read_integer(&ptr);
                        String t = gui_read_string(&ptr);
                        
                        draw_font_button(gui_target, target, view, gui_session.rect, b->id, font_id, t);
                    }break;
                    
                    case guicom_file_option:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        b32 folder = gui_read_integer(&ptr);
                        String f = gui_read_string(&ptr);
                        String m = gui_read_string(&ptr);
                        
                        if (folder){
                            append_s_char(&f, '/');
                        }
                        
                        draw_fat_option_block(gui_target, target, view, font_id, gui_session.rect, b->id, f, m);
                    }break;
                    
                    case guicom_style_preview:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        i32 style_index = *(i32*)(b + 1);
                        Style *style = get_style(view->persistent.models, style_index);
                        
                        draw_style_preview(gui_target, target, view, font_id, gui_session.rect, b->id, style);
                    }break;
                    
                    case guicom_fixed_option:
                    case guicom_fixed_option_checkbox:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        String f = gui_read_string(&ptr);
                        String m = {0};
                        i8 status = -1;
                        if (h->type == guicom_fixed_option_checkbox){
                            gui_read_byte(&ptr);
                            status = (i8)gui_read_byte(&ptr);
                        }
                        
                        draw_fat_option_block(gui_target, target, view, font_id, gui_session.rect, b->id, f, m, status);
                    }break;
                    
                    case guicom_button:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        String t = gui_read_string(&ptr);
                        
                        draw_button(gui_target, target, view, font_id, gui_session.rect, b->id, t);
                    }break;
                    
                    case guicom_scrollable_bar:
                    {
                        Models *models = view->persistent.models;
                        Style *style = main_style(models);
                        
                        u32 back;
                        u32 outline;
                        
                        i32_Rect bar = gui_session.rect;
                        
                        back = style->main.back_color;
                        if (is_active){
                            outline = style->main.margin_active_color;
                        }
                        else{
                            outline = style->main.margin_color;
                        }
                        
                        draw_rectangle(target, bar, back);
                        draw_rectangle_outline(target, bar, outline);
                    }break;
                    
                    case guicom_scrollable_top:
                    case guicom_scrollable_slider:
                    case guicom_scrollable_bottom:
                    {
                        GUI_id id;
                        Models *models = view->persistent.models;
                        Style *style = main_style(models);
                        i32_Rect box = gui_session.rect;
                        
                        i32 active_level;
                        
                        u32 back;
                        u32 outline;
                        
                        switch (h->type){
                            case guicom_scrollable_top: id = gui_id_scrollbar_top(); break;
                            case guicom_scrollable_bottom: id = gui_id_scrollbar_bottom(); break;
                            default: id = gui_id_scrollbar_slider(); break;
                        }
                        
                        active_level = gui_active_level(gui_target, id);
                        
                        switch (active_level){
                            case 0: back = style->main.back_color; break;
                            case 1: back = style->main.margin_hover_color; break;
                            default: back = style->main.margin_active_color; break;
                        }
                        
                        if (is_active){
                            outline = style->main.margin_active_color;
                        }
                        else{
                            outline = style->main.margin_color;
                        }
                        
                        draw_rectangle(target, box, back);
                        draw_margin(target, box, get_inner_rect(box, 2), outline);
                    }break;
                    
                    case guicom_begin_scrollable_section:
                    clip_rect.x1 = Min(gui_session.scroll_region.x1, clip_rect.x1);
                    draw_push_clip(target, clip_rect);
                    break;
                    
                    case guicom_end_scrollable_section:
                    clip_rect = draw_pop_clip(target);
                    break;
                }
            }
        }
        
        draw_pop_clip(target);
    }
    
    return(result);
}

inline void
file_view_free_buffers(View *view){
    General_Memory *general = &view->persistent.models->mem.general;
    general_memory_free(general, view->gui_mem);
    view->gui_mem = 0;
}

internal View_And_ID
live_set_alloc_view(Live_Views *live_set, Panel *panel, Models *models){
    View_And_ID result = {};
    
    Assert(live_set->count < live_set->max);
    ++live_set->count;
    
    result.view = live_set->free_sentinel.next;
    result.id = (i32)(result.view - live_set->views);
    Assert(result.id == result.view->persistent.id);
    
    dll_remove(result.view);
    memset(get_view_body(result.view), 0, get_view_size());
    
    result.view->in_use = 1;
    panel->view = result.view;
    result.view->panel = panel;
    
    result.view->persistent.models = models;
    
    init_query_set(&result.view->query_set);
    
    {
        i32 gui_mem_size = KB(512);
        void *gui_mem = general_memory_allocate(&models->mem.general, gui_mem_size + 8);
        result.view->gui_mem = gui_mem;
        gui_mem = advance_to_alignment(gui_mem);
        result.view->gui_target.push = make_part(gui_mem, gui_mem_size);
    }
    
    return(result);
}

inline void
live_set_free_view(Live_Views *live_set, View *view){
    Assert(live_set->count > 0);
    --live_set->count;
    file_view_free_buffers(view);
    dll_insert(&live_set->free_sentinel, view);
    view->in_use = 0;
}

// BOTTOM

