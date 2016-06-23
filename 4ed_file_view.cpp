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
    i32 result;
    i32 user_map_count = models->user_map_count;
    i32 *map_id_table = models->map_id_table;
    for (result = 0; result < user_map_count; ++result){
        if (map_id_table[result] == mapid) break;
        if (map_id_table[result] == -1){
            map_id_table[result] = mapid;
            break;
        }
    }
    return result;
}

internal i32
get_map_index(Models *models, i32 mapid){
    i32 result;
    i32 user_map_count = models->user_map_count;
    i32 *map_id_table = models->map_id_table;
    for (result = 0; result < user_map_count; ++result){
        if (map_id_table[result] == mapid) break;
        if (map_id_table[result] == 0){
            result = user_map_count;
            break;
        }
    }
    return result;
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
    else if (mapid == mapid_global) map = &models->map_top;
    else if (mapid == mapid_file) map = &models->map_file;
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
map_set_count(Models *models, i32 mapid, i32 count){
    Command_Map *map = get_or_add_map(models, mapid);
    Assert(map->commands == 0);
    map->count = count;
    if (map->max < count){
        map->max = count;
    }
}

internal i32
map_get_count(Models *models, i32 mapid){
    Command_Map *map = get_or_add_map(models, mapid);
    i32 count = map->count;
    Assert(map->commands == 0);
    return(count);
}

internal i32
map_get_max_count(Models *models, i32 mapid){
    Command_Map *map = get_or_add_map(models, mapid);
    i32 count = map->max;
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

struct View_Mode{
    i32 rewrite;
};
inline View_Mode
view_mode_zero(){
    View_Mode mode={0};
    return(mode);
}

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
    CV_Mode_Adjusting
};

struct File_Viewing_Data{
    Editing_File *file;
    
    Full_Cursor temp_highlight;
    i32 temp_highlight_end_pos;
    b32 show_temp_highlight;
    
    b32 unwrapped_lines;
    b32 show_whitespace;
    b32 file_locked;
    
    i32 line_count, line_max;
    f32 *line_wrap_y;
};
inline File_Viewing_Data
file_viewing_data_zero(){
    File_Viewing_Data data={0};
    return(data);
}

struct Recent_File_Data{
    u64 unique_buffer_id;
    GUI_Scroll_Vars scroll;
    
    Full_Cursor cursor;
    i32 mark;
    f32 preferred_x;
    i32 scroll_i;
};
inline Recent_File_Data
recent_file_data_zero(){
    Recent_File_Data data = {0};
    return(data);
}

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
    
    View_Routine_Function *view_routine;
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
    i32 prev_cursor_pos;
    Scroll_Context prev_context;
    
    i32_Rect file_region_prev;
    i32_Rect file_region;
    
    i32_Rect scroll_region;
    Recent_File_Data recent[16];
    
    GUI_Scroll_Vars *current_scroll;
    
    View_UI showing_ui;
    GUI_Target gui_target;
    void *gui_mem;
    GUI_Scroll_Vars gui_scroll;
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
    i32 font_advance;
    i32 line_height;
    
    View_Mode mode, next_mode;
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

// TODO(allen): Switch over to using an i32 instead of
// an f32 for these.
inline f32
view_file_width(View *view){
    i32_Rect file_rect = view->file_region;
    f32 result = (f32)(file_rect.x1 - file_rect.x0);
    return (result);
}

inline f32
view_file_height(View *view){
    i32_Rect file_rect = view->file_region;
    f32 result = (f32)(file_rect.y1 - file_rect.y0);
    return (result);
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
file_init_strings(Editing_File *file){
    file->name.source_path = make_fixed_width_string(file->name.source_path_);
    file->name.live_name = make_fixed_width_string(file->name.live_name_);
    file->name.extension = make_fixed_width_string(file->name.extension_);
}

internal void
file_set_name(Working_Set *working_set, Editing_File *file, String filename){
    String ext;
    
    Assert(file->name.live_name.str != 0);
    
    copy_checked(&file->name.source_path, filename);
    
    file->name.live_name.size = 0;
    get_front_of_directory(&file->name.live_name, filename);
    
    if (file->name.source_path.size == file->name.live_name.size){
        file->name.extension.size = 0;
    }
    else{
        ext = file_extension(filename);
        copy(&file->name.extension, ext);
    }
    
    {
        File_Node *node, *used_nodes;
        Editing_File *file_ptr;
        i32 file_x, original_len;
        b32 hit_conflict;
        
        used_nodes = &working_set->used_sentinel;
        original_len = file->name.live_name.size;
        hit_conflict = 1;
        file_x = 0;
        while (hit_conflict){
            hit_conflict = 0;
            for (dll_items(node, used_nodes)){
                file_ptr = (Editing_File*)node;
                if (file_ptr != file && file_is_ready(file_ptr)){
                    if (match(file->name.live_name, file_ptr->name.live_name)){
                        ++file_x;
                        hit_conflict = 1;
                        break;
                    }
                }
            }
            
            if (hit_conflict){
                file->name.live_name.size = original_len;
                append(&file->name.live_name, " <");
                append_int_to_str(&file->name.live_name, file_x);
                append(&file->name.live_name, ">");
            }
        }
    }
}

inline void
file_set_name(Working_Set *working_set, Editing_File *file, char *filename){
    String f = make_string_slowly(filename);
    file_set_name(working_set, file, f);
}

inline void
file_synchronize_times(System_Functions *system, Editing_File *file, char *filename){
    u64 stamp = system->file_time_stamp(filename);
    if (stamp > 0){
        file->state.last_4ed_write_time = stamp;
        file->state.last_4ed_edit_time = stamp;
        file->state.last_sys_write_time = stamp;
    }
    file->state.sync = buffer_get_sync(file);
}

internal b32
file_save(System_Functions *system, Mem_Options *mem, Editing_File *file, char *filename){
    b32 result = 0;
    
    i32 max = 0, size = 0;
    b32 dos_write_mode = file->settings.dos_write_mode;
    char *data = 0;
    Buffer_Type *buffer = &file->state.buffer;
    
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
            data = (char*)general_memory_allocate(&mem->general, max, 0);
        }
    }
    Assert(data);
    
    if (dos_write_mode){
        size = buffer_convert_out(buffer, data, max);
    }
    else{
        size = max;
        buffer_stringify(buffer, 0, size, data);
    }
    
    result = system->file_save(filename, data, size);
    
    file_synchronize_times(system, file, filename);
    
    if (used_general){
        general_memory_free(&mem->general, data);
    }
    end_temp_memory(temp);
    
    return(result);
}

inline b32
file_save_and_set_names(System_Functions *system, Mem_Options *mem,
                        Working_Set *working_set, Editing_File *file,
                        char *filename){
    b32 result = 0;
    result = file_save(system, mem, file, filename);
    if (result){
        file_set_name(working_set, file, filename);
    }
    return result;
}

enum File_Bubble_Type{
    BUBBLE_BUFFER = 1,
    BUBBLE_STARTS,
    BUBBLE_WIDTHS,
    BUBBLE_WRAPS,
    BUBBLE_TOKENS,
    BUBBLE_UNDO_STRING,
    BUBBLE_UNDO,
    BUBBLE_UNDO_CHILDREN,
    //
    FILE_BUBBLE_TYPE_END,
};

#define GROW_FAILED 0
#define GROW_NOT_NEEDED 1
#define GROW_SUCCESS 2

internal i32
file_grow_starts_widths_as_needed(General_Memory *general, Buffer_Type *buffer, i32 additional_lines){
    b32 result = GROW_NOT_NEEDED;
    i32 max = buffer->line_max;
    i32 count = buffer->line_count;
    i32 target_lines = count + additional_lines;
    Assert(max == buffer->widths_max);
    
    if (target_lines > max || max == 0){
        max = LargeRoundUp(target_lines + max, Kbytes(1));
        
        f32 *new_widths = (f32*)general_memory_reallocate(
                                                          general, buffer->line_widths,
                                                          sizeof(f32)*count, sizeof(f32)*max, BUBBLE_WIDTHS);
        
        i32 *new_lines = (i32*)general_memory_reallocate(
                                                         general, buffer->line_starts,
                                                         sizeof(i32)*count, sizeof(i32)*max, BUBBLE_STARTS);
        
        if (new_lines){
            buffer->line_starts = new_lines;
            buffer->line_max = max;
        }
        if (new_widths){
            buffer->line_widths = new_widths;
            buffer->widths_max = max;
        }
        if (new_lines && new_widths){
            result = GROW_SUCCESS;
        }
        else{
            result = GROW_FAILED;
        }
    }
    
    return(result);
}

internal void
file_measure_starts_widths(System_Functions *system, General_Memory *general,
                           Buffer_Type *buffer, float *advance_data){
    if (!buffer->line_starts){
        i32 max = buffer->line_max = Kbytes(1);
        buffer->line_starts = (i32*)general_memory_allocate(general, max*sizeof(i32), BUBBLE_STARTS);
        TentativeAssert(buffer->line_starts);
        // TODO(allen): when unable to allocate?
    }
    if (!buffer->line_widths){
        i32 max = buffer->widths_max = Kbytes(1);
        buffer->line_widths = (f32*)general_memory_allocate(general, max*sizeof(f32), BUBBLE_WIDTHS);
        TentativeAssert(buffer->line_starts);
        // TODO(allen): when unable to allocate?
    }
    
    Buffer_Measure_Starts state = {};
    while (buffer_measure_starts_widths(&state, buffer, advance_data)){
        i32 count = state.count;
        i32 max = buffer->line_max;
        max = ((max + 1) << 1);
        
        {
            i32 *new_lines = (i32*)general_memory_reallocate(
                                                             general, buffer->line_starts, sizeof(i32)*count, sizeof(i32)*max, BUBBLE_STARTS);
            
            // TODO(allen): when unable to grow?
            TentativeAssert(new_lines);
            buffer->line_starts = new_lines;
            buffer->line_max = max;
        }
        
        {
            f32 *new_lines = (f32*)
                general_memory_reallocate(general, buffer->line_widths,
                                          sizeof(f32)*count, sizeof(f32)*max, BUBBLE_WIDTHS);
            
            // TODO(allen): when unable to grow?
            TentativeAssert(new_lines);
            buffer->line_widths = new_lines;
            buffer->widths_max = max;
        }
        
    }
    buffer->line_count = state.count;
    buffer->widths_count = state.count;
}

inline i32
view_wrapped_line_span(f32 line_width, f32 max_width){
    i32 line_count = CEIL32(line_width / max_width);
    if (line_count == 0) line_count = 1;
    return line_count;
}

internal i32
view_compute_lowest_line(View *view){
    i32 lowest_line = 0;
    i32 last_line = view->file_data.line_count - 1;
    if (last_line > 0){
        if (view->file_data.unwrapped_lines){
            lowest_line = last_line;
        }
        else{
            f32 wrap_y = view->file_data.line_wrap_y[last_line];
            lowest_line = FLOOR32(wrap_y / view->line_height);
            f32 max_width = view_file_width(view);
            
            Editing_File *file = view->file_data.file;
            Assert(!file->is_dummy);
            f32 width = file->state.buffer.line_widths[last_line];
            i32 line_span = view_wrapped_line_span(width, max_width);
            lowest_line += line_span - 1;
        }
    }
    return lowest_line;
}

inline i32
view_compute_max_target_y(i32 lowest_line, i32 line_height, f32 view_height){
    f32 max_target_y = ((lowest_line+.5f)*line_height) - view_height*.5f;
    max_target_y = clamp_bottom(0.f, max_target_y);
    return(CEIL32(max_target_y));
}

inline i32
view_compute_max_target_y(View *view){
    i32 lowest_line = view_compute_lowest_line(view);
    i32 line_height = view->line_height;
    f32 view_height = view_file_height(view);
    i32 max_target_y = view_compute_max_target_y(lowest_line, line_height, view_height);
    return(max_target_y);
}

internal void
view_measure_wraps(General_Memory *general, View *view){
    Buffer_Type *buffer;
    
    buffer = &view->file_data.file->state.buffer;
    i32 line_count = buffer->line_count;
    
    if (view->file_data.line_max < line_count){
        i32 max = view->file_data.line_max = LargeRoundUp(line_count, Kbytes(1));
        if (view->file_data.line_wrap_y){
            view->file_data.line_wrap_y = (f32*)
                general_memory_reallocate_nocopy(general, view->file_data.line_wrap_y, sizeof(f32)*max, BUBBLE_WRAPS);
        }
        else{
            view->file_data.line_wrap_y = (f32*)
                general_memory_allocate(general, sizeof(f32)*max, BUBBLE_WRAPS);
        }
    }
    
    f32 line_height = (f32)view->line_height;
    f32 max_width = view_file_width(view);
    buffer_measure_wrap_y(buffer, view->file_data.line_wrap_y, line_height, max_width);
    
    view->file_data.line_count = line_count;
}

internal void
file_create_from_string(System_Functions *system, Models *models,
                        Editing_File *file, char *name,
                        String val, b8 read_only = 0){
    
    Font_Set *font_set = models->font_set;
    Working_Set *working_set = &models->working_set;
    General_Memory *general = &models->mem.general;
    Partition *part = &models->mem.part;
    Buffer_Init_Type init;
    i32 page_size, scratch_size, init_success;
    
    file->state = editing_file_state_zero();
    
    init = buffer_begin_init(&file->state.buffer, val.str, val.size);
    for (; buffer_init_need_more(&init); ){
        page_size = buffer_init_page_size(&init);
        page_size = LargeRoundUp(page_size, Kbytes(4));
        if (page_size < Kbytes(4)) page_size = Kbytes(4);
        void *data = general_memory_allocate(general, page_size, BUBBLE_BUFFER);
        buffer_init_provide_page(&init, data, page_size);
    }
    
    scratch_size = partition_remaining(part);
    Assert(scratch_size > 0);
    init_success = buffer_end_init(&init, part->base + part->pos, scratch_size);
    AllowLocal(init_success);
    Assert(init_success);
    
    if (buffer_size(&file->state.buffer) < val.size){
        file->settings.dos_write_mode = 1;
    }
    
    file_init_strings(file);
    
    file_set_name(working_set, file, (char*)name);
    
    file->state.font_id = models->global_font.font_id;
    
    file_synchronize_times(system, file, name);
    
    Render_Font *font = get_font_info(font_set, file->state.font_id)->font;
    float *advance_data = 0;
    if (font) advance_data = font->advance_data;
    file_measure_starts_widths(system, general, &file->state.buffer, advance_data);
    
    file->settings.read_only = read_only;
    if (!read_only){
        // TODO(allen): Redo undo system (if you don't mind the pun)
        i32 request_size = Kbytes(64);
        file->state.undo.undo.max = request_size;
        file->state.undo.undo.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
        file->state.undo.undo.edit_max = request_size / sizeof(Edit_Step);
        file->state.undo.undo.edits = (Edit_Step*)general_memory_allocate(general, request_size, BUBBLE_UNDO);
        
        file->state.undo.redo.max = request_size;
        file->state.undo.redo.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
        file->state.undo.redo.edit_max = request_size / sizeof(Edit_Step);
        file->state.undo.redo.edits = (Edit_Step*)general_memory_allocate(general, request_size, BUBBLE_UNDO);
        
        file->state.undo.history.max = request_size;
        file->state.undo.history.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
        file->state.undo.history.edit_max = request_size / sizeof(Edit_Step);
        file->state.undo.history.edits = (Edit_Step*)general_memory_allocate(general, request_size, BUBBLE_UNDO);
        
        file->state.undo.children.max = request_size;
        file->state.undo.children.strings = (u8*)general_memory_allocate(general, request_size, BUBBLE_UNDO_STRING);
        file->state.undo.children.edit_max = request_size / sizeof(Buffer_Edit);
        file->state.undo.children.edits = (Buffer_Edit*)general_memory_allocate(general, request_size, BUBBLE_UNDO);
        
        file->state.undo.history_block_count = 1;
        file->state.undo.history_head_block = 0;
        file->state.undo.current_block_normal = 1;
    }
    
    Open_File_Hook_Function *open_hook = models->hook_open_file;
    if (open_hook){
        open_hook(&models->app_links, file->id.id);
    }
    file->settings.is_initialized = 1;
}

internal b32
file_create_empty(System_Functions *system,
                  Models *models, Editing_File *file, char *filename){
    file_create_from_string(system, models, file, filename, string_zero());
    return (1);
}

internal b32
file_create_read_only(System_Functions *system,
                      Models *models, Editing_File *file, char *filename){
    file_create_from_string(system, models, file, filename, string_zero(), 1);
    return (1);
}

internal void
file_close(System_Functions *system, General_Memory *general, Editing_File *file){
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_stack.tokens){
            general_memory_free(general, file->state.swap_stack.tokens);
            file->state.swap_stack.tokens = 0;
        }
    }
    if (file->state.token_stack.tokens){
        general_memory_free(general, file->state.token_stack.tokens);
    }
    
    Buffer_Type *buffer = &file->state.buffer;
    if (buffer->data){
        general_memory_free(general, buffer->data);
        general_memory_free(general, buffer->line_starts);
        general_memory_free(general, buffer->line_widths);
    }
    
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

internal
Job_Callback_Sig(job_full_lex){
    Editing_File *file = (Editing_File*)data[0];
    General_Memory *general = (General_Memory*)data[1];
    
    Cpp_File cpp_file;
    cpp_file.data = file->state.buffer.data;
    cpp_file.size = file->state.buffer.size;
    
    i32 buffer_size = file->state.buffer.size;
    buffer_size = (buffer_size + 3)&(~3);
    
#if 0
    while (memory->size < buffer_size*2){
        system->grow_thread_memory(memory);
    }
    
    char *tb = (char*)memory->data;
    
    Cpp_Token_Stack tokens;
    tokens.tokens = (Cpp_Token*)((char*)memory->data + buffer_size);
    tokens.max_count = (memory->size - buffer_size) / sizeof(Cpp_Token);
    tokens.count = 0;
    
    b32 still_lexing = 1;
    
    Lex_Data lex = lex_data_init(tb);
    
    do{
        i32 result = 
            cpp_lex_size_nonalloc(&lex,
                                  cpp_file.data, cpp_file.size, cpp_file.size,
                                  &tokens, 2048);
        
        switch (result){
            case LexNeedChunk: Assert(!"Invalid Path"); break;
            
            case LexNeedTokenMemory:
            if (system->check_cancel(thread)){
                return;
            }
            system->grow_thread_memory(memory);
            lex.tb = (char*)memory->data;
            tokens.tokens = (Cpp_Token*)((char*)memory->data + buffer_size);
            tokens.max_count = memory->size / sizeof(Cpp_Token);
            break;
            
            case LexHitTokenLimit:
            if (system->check_cancel(thread)){
                return;
            }
            break;
            
            case LexFinished: still_lexing = 0; break;
        }
    } while (still_lexing);
    
#else
    
    while (memory->size < buffer_size){
        system->grow_thread_memory(memory);
    }
    
    Cpp_Token_Stack tokens;
    tokens.tokens = (Cpp_Token*)(char*)memory->data;
    tokens.max_count = (memory->size) / sizeof(Cpp_Token);
    tokens.count = 0;
    
    Cpp_Lex_Data status = {};
    
    do{
        for (i32 r = 2048; r > 0 && status.pos < cpp_file.size; --r){
            Cpp_Lex_Data prev_lex = status;
            Cpp_Read_Result step_result = cpp_lex_step(cpp_file, &status);
            
            if (step_result.has_result){
                if (!cpp_push_token_nonalloc(&tokens, step_result.token)){
                    status = prev_lex;
                    system->grow_thread_memory(memory);
                    tokens.tokens = (Cpp_Token*)memory->data;
                    tokens.max_count = memory->size / sizeof(Cpp_Token);
                }
            }
        }
        
        if (status.pos >= cpp_file.size){
            status.complete = 1;
        }
        else{
            if (system->check_cancel(thread)){
                return;
            }
        }
    } while(!status.complete);
    
#endif
    
    
    
    i32 new_max = LargeRoundUp(tokens.count+1, Kbytes(1));
    
    system->acquire_lock(FRAME_LOCK);
    {
        Assert(file->state.swap_stack.tokens == 0);
        file->state.swap_stack.tokens = (Cpp_Token*)
            general_memory_allocate(general, new_max*sizeof(Cpp_Token), BUBBLE_TOKENS);
    }
    system->release_lock(FRAME_LOCK);
    
    u8 *dest = (u8*)file->state.swap_stack.tokens;
    u8 *src = (u8*)tokens.tokens;
    
    memcpy(dest, src, tokens.count*sizeof(Cpp_Token));
    
    system->acquire_lock(FRAME_LOCK);
    {
        Cpp_Token_Stack *file_stack = &file->state.token_stack;
        file_stack->count = tokens.count;
        file_stack->max_count = new_max;
        if (file_stack->tokens){
            general_memory_free(general, file_stack->tokens);
        }
        file_stack->tokens = file->state.swap_stack.tokens;
        file->state.swap_stack.tokens = 0;
    }
    system->release_lock(FRAME_LOCK);
    
    // NOTE(allen): These are outside the locked section because I don't
    // think getting these out of order will cause critical bugs, and I
    // want to minimize what's done in locked sections.
    file->state.tokens_complete = 1;
    file->state.still_lexing = 0;
}


internal void
file_kill_tokens(System_Functions *system,
                 General_Memory *general, Editing_File *file){
    file->settings.tokens_exist = 0;
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_stack.tokens){
            general_memory_free(general, file->state.swap_stack.tokens);
            file->state.swap_stack.tokens = 0;
        }
    }
    if (file->state.token_stack.tokens){
        general_memory_free(general, file->state.token_stack.tokens);
    }
    file->state.tokens_complete = 0;
    file->state.token_stack = cpp_token_stack_zero();
}

#if BUFFER_EXPERIMENT_SCALPEL <= 0
internal void
file_first_lex_parallel(System_Functions *system,
                        General_Memory *general, Editing_File *file){
    file->settings.tokens_exist = 1;
    
    if (file->is_loading == 0 && file->state.still_lexing == 0){
        Assert(file->state.token_stack.tokens == 0);
        
        file->state.tokens_complete = 0;
        file->state.still_lexing = 1;
        
        Job_Data job;
        job.callback = job_full_lex;
        job.data[0] = file;
        job.data[1] = general;
        file->state.lex_job = system->post_job(BACKGROUND_THREADS, job);
    }
}
#endif

internal void
file_relex_parallel(System_Functions *system,
                    Mem_Options *mem, Editing_File *file,
                    i32 start_i, i32 end_i, i32 amount){
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    if (file->state.token_stack.tokens == 0){
        file_first_lex_parallel(system, general, file);
        return;
    }
    
    b32 inline_lex = !file->state.still_lexing;
    if (inline_lex){
        Cpp_File cpp_file;
        cpp_file.data = file->state.buffer.data;
        cpp_file.size = file->state.buffer.size;
        
        Cpp_Token_Stack *stack = &file->state.token_stack;
        
        Cpp_Relex_State state = 
            cpp_relex_nonalloc_start(cpp_file, stack,
                                     start_i, end_i, amount, 100);
        
        Temp_Memory temp = begin_temp_memory(part);
        i32 relex_end;
        Cpp_Token_Stack relex_space;
        relex_space.count = 0;
        relex_space.max_count = state.space_request;
        relex_space.tokens = push_array(part, Cpp_Token, relex_space.max_count);
        
        //        char *spare = push_array(part, char, cpp_file.size);
        //        if (cpp_relex_nonalloc_main(&state, &relex_space, &relex_end, spare)){
        if (cpp_relex_nonalloc_main(&state, &relex_space, &relex_end)){
            inline_lex = 0;
        }
        else{
            i32 delete_amount = relex_end - state.start_token_i;
            i32 shift_amount = relex_space.count - delete_amount;
            
            if (shift_amount != 0){
                i32 new_count = stack->count + shift_amount;
                if (new_count > stack->max_count){
                    i32 new_max = LargeRoundUp(new_count, Kbytes(1));
                    stack->tokens = (Cpp_Token*)
                        general_memory_reallocate(general, stack->tokens,
                                                  stack->count*sizeof(Cpp_Token),
                                                  new_max*sizeof(Cpp_Token), BUBBLE_TOKENS);
                    stack->max_count = new_max;
                }
                
                i32 shift_size = stack->count - relex_end;
                if (shift_size > 0){
                    Cpp_Token *old_base = stack->tokens + relex_end;
                    memmove(old_base + shift_amount, old_base,
                            sizeof(Cpp_Token)*shift_size);
                }
                
                stack->count += shift_amount;
            }
            
            memcpy(state.stack->tokens + state.start_token_i, relex_space.tokens,
                   sizeof(Cpp_Token)*relex_space.count);
        }
        
        end_temp_memory(temp);
    }
    
    if (!inline_lex){
        Cpp_Token_Stack *stack = &file->state.token_stack;
        Cpp_Get_Token_Result get_token_result = cpp_get_token(stack, end_i);
        i32 end_token_i = get_token_result.token_index;
        
        if (end_token_i < 0) end_token_i = 0;
        else if (end_i > stack->tokens[end_token_i].start) ++end_token_i;
        
        cpp_shift_token_starts(stack, end_token_i, amount);
        --end_token_i;
        if (end_token_i >= 0){
            Cpp_Token *token = stack->tokens + end_token_i;
            if (token->start < end_i && token->start + token->size > end_i){
                token->size += amount;
            }
        }
        
        file->state.still_lexing = 1;
        
        Job_Data job;
        job.callback = job_full_lex;
        job.data[0] = file;
        job.data[1] = general;
        file->state.lex_job = system->post_job(BACKGROUND_THREADS, job);
    }
}

internal void
undo_stack_grow_string(General_Memory *general, Edit_Stack *stack, i32 extra_size){
    i32 old_max = stack->max;
    u8 *old_str = stack->strings;
    i32 new_max = old_max*2 + extra_size;
    u8 *new_str = (u8*)
        general_memory_reallocate(general, old_str, old_max, new_max);
    stack->strings = new_str;
    stack->max = new_max;
}

internal void
undo_stack_grow_edits(General_Memory *general, Edit_Stack *stack){
    i32 old_max = stack->edit_max;
    Edit_Step *old_eds = stack->edits;
    i32 new_max = old_max*2 + 2;
    Edit_Step *new_eds = (Edit_Step*)
        general_memory_reallocate(general, old_eds, old_max*sizeof(Edit_Step), new_max*sizeof(Edit_Step));
    stack->edits = new_eds;
    stack->edit_max = new_max;
}

internal void
child_stack_grow_string(General_Memory *general, Small_Edit_Stack *stack, i32 extra_size){
    i32 old_max = stack->max;
    u8 *old_str = stack->strings;
    i32 new_max = old_max*2 + extra_size;
    u8 *new_str = (u8*)
        general_memory_reallocate(general, old_str, old_max, new_max);
    stack->strings = new_str;
    stack->max = new_max;
}

internal void
child_stack_grow_edits(General_Memory *general, Small_Edit_Stack *stack, i32 amount){
    i32 old_max = stack->edit_max;
    Buffer_Edit *old_eds = stack->edits;
    i32 new_max = old_max*2 + amount;
    Buffer_Edit *new_eds = (Buffer_Edit*)
        general_memory_reallocate(general, old_eds, old_max*sizeof(Buffer_Edit), new_max*sizeof(Buffer_Edit));
    stack->edits = new_eds;
    stack->edit_max = new_max;
}

internal i32
undo_children_push(General_Memory *general, Small_Edit_Stack *children,
                   Buffer_Edit *edits, i32 edit_count, u8 *strings, i32 string_size){
    i32 result = children->edit_count;
    if (children->edit_count + edit_count > children->edit_max)
        child_stack_grow_edits(general, children, edit_count);
    
    if (children->size + string_size > children->max)
        child_stack_grow_string(general, children, string_size);
    
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
file_post_undo(General_Memory *general, Editing_File *file,
               Edit_Step step, b32 do_merge, b32 can_merge){
    if (step.type == ED_NORMAL){
        file->state.undo.redo.size = 0;
        file->state.undo.redo.edit_count = 0;
    }
    
    Edit_Stack *undo = &file->state.undo.undo;
    Edit_Step *result = 0;
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + undo->size > undo->max)
            undo_stack_grow_string(general, undo, step.edit.end - step.edit.start);
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv,
                           (char*)undo->strings, &undo->size, undo->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.pre_pos = step.pre_pos;
        inv_step.post_pos = step.post_pos;
        inv_step.can_merge = (b8)can_merge;
        inv_step.type = ED_UNDO;
        
        b32 did_merge = 0;
        if (do_merge && undo->edit_count > 0){
            Edit_Step prev = undo->edits[undo->edit_count-1];
            if (prev.can_merge && inv_step.edit.len == 0 && prev.edit.len == 0){
                if (prev.edit.end == inv_step.edit.start){
                    did_merge = 1;
                    inv_step.edit.start = prev.edit.start;
                    inv_step.pre_pos = prev.pre_pos;
                }
            }
        }
        
        if (did_merge){
            result = undo->edits + (undo->edit_count-1);
            *result = inv_step;
        }
        else{
            if (undo->edit_count == undo->edit_max)
                undo_stack_grow_edits(general, undo);
            
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
        
        if (undo->edit_count == undo->edit_max)
            undo_stack_grow_edits(general, undo);
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
        if (step.edit.end - step.edit.start + redo->size > redo->max)
            undo_stack_grow_string(general, redo, step.edit.end - step.edit.start);
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv,
                           (char*)redo->strings, &redo->size, redo->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.pre_pos = step.pre_pos;
        inv_step.post_pos = step.post_pos;
        inv_step.type = ED_REDO;
        
        if (redo->edit_count == redo->edit_max)
            undo_stack_grow_edits(general, redo);
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
file_post_history(General_Memory *general, Editing_File *file,
                  Edit_Step step, b32 do_merge, b32 can_merge){
    Edit_Stack *history = &file->state.undo.history;
    Edit_Step *result = 0;
    
    persist Edit_Type reverse_types[4];
    if (reverse_types[ED_UNDO] == 0){
        reverse_types[ED_NORMAL] = ED_REVERSE_NORMAL;
        reverse_types[ED_REVERSE_NORMAL] = ED_NORMAL;
        reverse_types[ED_UNDO] = ED_REDO;
        reverse_types[ED_REDO] = ED_UNDO;
    }
    
    if (step.child_count == 0){
        if (step.edit.end - step.edit.start + history->size > history->max)
            undo_stack_grow_string(general, history, step.edit.end - step.edit.start);
        
        Buffer_Edit inv;
        buffer_invert_edit(&file->state.buffer, step.edit, &inv,
                           (char*)history->strings, &history->size, history->max);
        
        Edit_Step inv_step = {};
        inv_step.edit = inv;
        inv_step.pre_pos = step.pre_pos;
        inv_step.post_pos = step.post_pos;
        inv_step.can_merge = (b8)can_merge;
        inv_step.type = reverse_types[step.type];
        
        bool32 did_merge = 0;
        if (do_merge && history->edit_count > 0){
            Edit_Step prev = history->edits[history->edit_count-1];
            if (prev.can_merge && inv_step.edit.len == 0 && prev.edit.len == 0){
                if (prev.edit.end == inv_step.edit.start){
                    did_merge = 1;
                    inv_step.edit.start = prev.edit.start;
                    inv_step.pre_pos = prev.pre_pos;
                }
            }
        }
        
        if (did_merge){
            result = history->edits + (history->edit_count-1);
        }
        else{
            if (history->edit_count == history->edit_max)
                undo_stack_grow_edits(general, history);
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
        
        if (history->edit_count == history->edit_max)
            undo_stack_grow_edits(general, history);
        result = history->edits + (history->edit_count++);
        *result = inv_step;
    }
    
    return result;
}

inline Full_Cursor
view_compute_cursor_from_pos(View *view, i32 pos){
    Editing_File *file = view->file_data.file;
    Models *models = view->persistent.models;
    Render_Font *font = get_font_info(models->font_set, models->global_font.font_id)->font;
    
    Full_Cursor result = {};
    if (font){
        f32 max_width = view_file_width(view);
        result = buffer_cursor_from_pos(&file->state.buffer, pos, view->file_data.line_wrap_y,
                                        max_width, (f32)view->line_height, font->advance_data);
    }
    return result;
}

inline Full_Cursor
view_compute_cursor_from_unwrapped_xy(View *view, f32 seek_x, f32 seek_y, b32 round_down = 0){
    Editing_File *file = view->file_data.file;
    Models *models = view->persistent.models;
    Render_Font *font = get_font_info(models->font_set, models->global_font.font_id)->font;
    
    Full_Cursor result = {};
    if (font){
        f32 max_width = view_file_width(view);
        result = buffer_cursor_from_unwrapped_xy(&file->state.buffer, seek_x, seek_y,
                                                 round_down, view->file_data.line_wrap_y, max_width, (f32)view->line_height, font->advance_data);
    }
    
    return result;
}

internal Full_Cursor
view_compute_cursor_from_wrapped_xy(View *view, f32 seek_x, f32 seek_y, b32 round_down = 0){
    Editing_File *file = view->file_data.file;
    Models *models = view->persistent.models;
    Render_Font *font = get_font_info(models->font_set, models->global_font.font_id)->font;
    
    Full_Cursor result = {};
    if (font){
        f32 max_width = view_file_width(view);
        result = buffer_cursor_from_wrapped_xy(&file->state.buffer, seek_x, seek_y,
                                               round_down, view->file_data.line_wrap_y,
                                               max_width, (f32)view->line_height, font->advance_data);
    }
    
    return (result);
}

internal Full_Cursor
view_compute_cursor_from_line_pos(View *view, i32 line, i32 pos){
    Editing_File *file = view->file_data.file;
    Models *models = view->persistent.models;
    Render_Font *font = get_font_info(models->font_set, models->global_font.font_id)->font;
    
    Full_Cursor result = {};
    if (font){
        f32 max_width = view_file_width(view);
        result = buffer_cursor_from_line_character(&file->state.buffer, line, pos,
                                                   view->file_data.line_wrap_y, max_width, (f32)view->line_height, font->advance_data);
    }
    
    return (result);
}

inline Full_Cursor
view_compute_cursor(View *view, Buffer_Seek seek){
    Full_Cursor result = {};
    
    switch(seek.type){
        case buffer_seek_pos:
        result = view_compute_cursor_from_pos(view, seek.pos);
        break;
        
        case buffer_seek_wrapped_xy:
        result = view_compute_cursor_from_wrapped_xy(view, seek.x, seek.y, seek.round_down);
        break;
        
        case buffer_seek_unwrapped_xy:
        result = view_compute_cursor_from_unwrapped_xy(view, seek.x, seek.y, seek.round_down);
        break;
        
        case buffer_seek_line_char:
        result = view_compute_cursor_from_line_pos(view, seek.line, seek.character);
        break;
    }
    
    return (result);
}

inline Full_Cursor
view_compute_cursor_from_xy(View *view, f32 seek_x, f32 seek_y){
    Full_Cursor result;
    if (view->file_data.unwrapped_lines) result = view_compute_cursor_from_unwrapped_xy(view, seek_x, seek_y);
    else result = view_compute_cursor_from_wrapped_xy(view, seek_x, seek_y);
    return result;
}

inline void
view_set_temp_highlight(View *view, i32 pos, i32 end_pos){
    view->file_data.temp_highlight = view_compute_cursor_from_pos(view, pos);
    view->file_data.temp_highlight_end_pos = end_pos;
    view->file_data.show_temp_highlight = 1;
}

inline i32
view_get_cursor_pos(View *view){
    i32 result;
    if (view->file_data.show_temp_highlight){
        result = view->file_data.temp_highlight.pos;
    }
    else{
        result = view->recent->cursor.pos;
    }
    return result;
}

inline f32
view_get_cursor_x(View *view){
    f32 result;
    Full_Cursor *cursor;
    if (view->file_data.show_temp_highlight){
        cursor = &view->file_data.temp_highlight;
    }
    else{
        cursor = &view->recent->cursor;
    }
    if (view->file_data.unwrapped_lines){
        result = cursor->unwrapped_x;
    }
    else{
        result = cursor->wrapped_x;
    }
    return result;
}

inline f32
view_get_cursor_y(View *view){
    Full_Cursor *cursor;
    f32 result;
    
    if (view->file_data.show_temp_highlight) cursor = &view->file_data.temp_highlight;
    else cursor = &view->recent->cursor;
    
    if (view->file_data.unwrapped_lines) result = cursor->unwrapped_y;
    else result = cursor->wrapped_y;
    
    return result;
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

internal void
view_move_cursor_to_view(View *view, GUI_Scroll_Vars scroll){
    i32 line_height = view->line_height;
    f32 old_cursor_y = view_get_cursor_y(view);
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
        view->recent->cursor =
            view_compute_cursor_from_xy(view, view->recent->preferred_x, cursor_y);
    }
}

internal void
view_move_view_to_cursor(View *view, GUI_Scroll_Vars *scroll){
    f32 max_x = view_file_width(view);
    
    f32 cursor_y = view_get_cursor_y(view);
    f32 cursor_x = view_get_cursor_x(view);
    
    GUI_Scroll_Vars scroll_vars = *scroll;
    i32 target_y = scroll_vars.target_y;
    i32 target_x = scroll_vars.target_x;
   
    Cursor_Limits limits = view_cursor_limits(view);
    
    if (cursor_y > target_y + limits.max){
        target_y = CEIL32(cursor_y - limits.max + limits.delta);
    }
    if (cursor_y < target_y + limits.min){
        target_y = FLOOR32(cursor_y - limits.delta - limits.min);
    }
    
    target_y = clamp(0, target_y, scroll_vars.max_y);
    
    if (cursor_x >= target_x + max_x){
        target_x = CEIL32(cursor_x - max_x/2);
    }
    else if (cursor_x < target_x){
        target_x = FLOOR32(Max(0, cursor_x - max_x/2));
    }
    
    if (target_x != scroll_vars.target_x || target_y != scroll_vars.target_y){
        scroll->target_x = target_x;
        scroll->target_y = target_y;
    }
}

inline void
file_view_nullify_file(View *view){
    General_Memory *general = &view->persistent.models->mem.general;
    if (view->file_data.line_wrap_y){
        general_memory_free(general, view->file_data.line_wrap_y);
    }
    view->file_data = file_viewing_data_zero();
}

internal void
view_set_file(View *view, Editing_File *file, Models *models){
    Font_Info *fnt_info;
    
    // TODO(allen): This belongs somewhere else.
    fnt_info = get_font_info(models->font_set, models->global_font.font_id);
    view->font_advance = fnt_info->advance;
    view->line_height = fnt_info->height;
    
    if (view->file_data.file != 0){
        touch_file(&models->working_set, view->file_data.file);
    }
    
    file_view_nullify_file(view);
    view->file_data.file = file;
    
    if (file){
        u64 unique_buffer_id = file->unique_buffer_id;
        Recent_File_Data *recent = view->recent;
        Recent_File_Data temp_recent = {0};
        i32 i = 0;
        i32 max = ArrayCount(view->recent)-1;
        b32 found_recent_entry = 0;
        
        view->file_data.unwrapped_lines = file->settings.unwrapped_lines;
        
        for (; i < max; ++i, ++recent){
            if (recent->unique_buffer_id == unique_buffer_id){
                temp_recent = *recent;
                memmove(view->recent+1, view->recent, sizeof(*recent)*i);
                view->recent[0] = temp_recent;
                found_recent_entry = 1;
                break;
            }
        }
        
        if (found_recent_entry){
            if (file_is_ready(file)){
                view_measure_wraps(&models->mem.general, view);
                view->recent->cursor = view_compute_cursor_from_pos(view, view->recent->cursor.pos);
                view->recent->scroll.max_y = view_compute_max_target_y(view);
                
                view_move_view_to_cursor(view, &view->recent->scroll);
            }
        }
        else{
            i = 15;
            recent = view->recent + i;
            memmove(view->recent+1, view->recent, sizeof(*recent)*i);
            view->recent[0] = recent_file_data_zero();
            
            recent = view->recent;
            recent->unique_buffer_id = unique_buffer_id;
            
            if (file_is_ready(file)){
                view_measure_wraps(&models->mem.general, view);
                view->recent->cursor = view_compute_cursor_from_pos(view, file->state.cursor_pos);
                view->recent->scroll.max_y = view_compute_max_target_y(view);
                
                view_move_view_to_cursor(view, &view->recent->scroll);
                view->reinit_scrolling = 1;
            }
        }
    }
}

struct Relative_Scrolling{
    f32 scroll_x, scroll_y;
    f32 target_x, target_y;
};

internal Relative_Scrolling
view_get_relative_scrolling(View *view){
    Relative_Scrolling result;
    f32 cursor_y = view_get_cursor_y(view);
    result.scroll_y = cursor_y - view->recent->scroll.scroll_y;
    result.target_y = cursor_y - view->recent->scroll.target_y;
    return(result);
}

internal void
view_set_relative_scrolling(View *view, Relative_Scrolling scrolling){
    f32 cursor_y = view_get_cursor_y(view);
    view->recent->scroll.scroll_y = cursor_y - scrolling.scroll_y;
    view->recent->scroll.target_y =
        ROUND32(clamp_bottom(0.f, cursor_y - scrolling.target_y));
}

inline void
view_cursor_move(View *view, Full_Cursor cursor){
    view->recent->cursor = cursor;
    view->recent->preferred_x = view_get_cursor_x(view);
    view->file_data.file->state.cursor_pos = view->recent->cursor.pos;
    view->file_data.show_temp_highlight = 0;
}

inline void
view_cursor_move(View *view, i32 pos){
    Full_Cursor cursor = view_compute_cursor_from_pos(view, pos);
    view_cursor_move(view, cursor);
}

inline void
view_cursor_move(View *view, f32 x, f32 y, b32 round_down = 0){
    Full_Cursor cursor;
    if (view->file_data.unwrapped_lines){
        cursor = view_compute_cursor_from_unwrapped_xy(view, x, y, round_down);
    }
    else{
        cursor = view_compute_cursor_from_wrapped_xy(view, x, y, round_down);
    }
    view_cursor_move(view, cursor);
}

inline void
view_cursor_move(View *view, i32 line, i32 pos){
    Full_Cursor cursor = view_compute_cursor_from_line_pos(view, line, pos);
    view_cursor_move(view, cursor);
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
file_update_history_before_edit(Mem_Options *mem, Editing_File *file, Edit_Step step, u8 *str,
                                History_Mode history_mode){
    if (!file->state.undo.undo.edits) return;
    General_Memory *general = &mem->general;
    
    b32 can_merge = 0, do_merge = 0;
    switch (step.type){
        case ED_NORMAL:
        {
            if (step.edit.len == 1 && str && char_is_alpha_numeric(*str)) can_merge = 1;
            if (step.edit.len == 1 && str && (can_merge || char_is_whitespace(*str))) do_merge = 1;
            
            if (history_mode != hist_forward)
                file_post_history(general, file, step, do_merge, can_merge);
            
            file_post_undo(general, file, step, do_merge, can_merge);
        }break;
        
        case ED_REVERSE_NORMAL:
        {
            if (history_mode != hist_forward)
                file_post_history(general, file, step, do_merge, can_merge);
            
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
                    Edit_Step *edit_dest =
                        file->state.undo.redo.edits + file->state.undo.redo.edit_count + steps_of_redo;
                    
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
            if (history_mode != hist_forward)
                file_post_history(general, file, step, do_merge, can_merge);
            file_post_redo(general, file, step);
            undo_stack_pop(&file->state.undo.undo);
        }break;
        
        case ED_REDO:
        {
            if (step.edit.len == 1 && str && char_is_alpha_numeric(*str)) can_merge = 1;
            if (step.edit.len == 1 && str && (can_merge || char_is_whitespace(*str))) do_merge = 1;
            
            if (history_mode != hist_forward)
                file_post_history(general, file, step, do_merge, can_merge);
            
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
file_pre_edit_maintenance(System_Functions *system,
                          General_Memory *general,
                          Editing_File *file){
    if (file->state.still_lexing){
        system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        if (file->state.swap_stack.tokens){
            general_memory_free(general, file->state.swap_stack.tokens);
            file->state.swap_stack.tokens = 0;
        }
        file->state.still_lexing = 0;
    }
    file->state.last_4ed_edit_time = system->now_time_stamp();
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
file_edit_cursor_fix(System_Functions *system,
                     Partition *part, General_Memory *general,
                     Editing_File *file, Editing_Layout *layout,
                     Cursor_Fix_Descriptor desc){
    
    Full_Cursor temp_cursor;
    Temp_Memory cursor_temp = begin_temp_memory(part);
    i32 cursor_max = layout->panel_max_count * 2;
    Cursor_With_Index *cursors = push_array(part, Cursor_With_Index, cursor_max);
    
    f32 y_offset = 0, y_position = 0;
    i32 cursor_count = 0;
    
    View *view;
    Panel *panel, *used_panels;
    used_panels = &layout->used_sentinel;
    
    for (dll_items(panel, used_panels)){
        view = panel->view;
        if (view->file_data.file == file){
            view_measure_wraps(general, view);
            write_cursor_with_index(cursors, &cursor_count, view->recent->cursor.pos);
            write_cursor_with_index(cursors, &cursor_count, view->recent->mark - 1);
            write_cursor_with_index(cursors, &cursor_count, view->recent->scroll_i - 1);
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
            if (view && view->file_data.file == file){
                view_cursor_move(view, cursors[cursor_count++].pos);
                view->recent->preferred_x = view_get_cursor_x(view);
                
                view->recent->mark = cursors[cursor_count++].pos + 1;
                i32 new_scroll_i = cursors[cursor_count++].pos + 1;
                if (view->recent->scroll_i != new_scroll_i){
                    view->recent->scroll_i = new_scroll_i;
                    temp_cursor = view_compute_cursor_from_pos(view, view->recent->scroll_i);
                    y_offset = MOD(view->recent->scroll.scroll_y, view->line_height);
                    
                    if (view->file_data.unwrapped_lines){
                        y_position = temp_cursor.unwrapped_y + y_offset;
                        view->recent->scroll.target_y += ROUND32(y_position - view->recent->scroll.scroll_y);
                        view->recent->scroll.scroll_y = y_position;
                    }
                    else{
                        y_position = temp_cursor.wrapped_y + y_offset;
                        view->recent->scroll.target_y += ROUND32(y_position - view->recent->scroll.scroll_y);
                        view->recent->scroll.scroll_y = y_position;
                    }
                }
            }
        }
    }
    
    end_temp_memory(cursor_temp);
}

internal void
file_do_single_edit(System_Functions *system,
                    Models *models, Editing_File *file,
                    Edit_Spec spec, History_Mode history_mode, b32 use_high_permission = 0){
    if (!use_high_permission && file->settings.read_only) return;
    
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
            new_data = general_memory_allocate(general, request_amount, BUBBLE_BUFFER);
        }
        void *old_data = buffer_edit_provide_memory(&file->state.buffer, new_data, request_amount);
        if (old_data) general_memory_free(general, old_data);
    }
    
    Buffer_Type *buffer = &file->state.buffer;
    i32 line_start = buffer_get_line_index(&file->state.buffer, start);
    i32 line_end = buffer_get_line_index(&file->state.buffer, end);
    i32 replaced_line_count = line_end - line_start;
    i32 new_line_count = buffer_count_newlines(&file->state.buffer, start, start+str_len);
    i32 line_shift =  new_line_count - replaced_line_count;
    
    Render_Font *font = get_font_info(models->font_set, file->state.font_id)->font;
    
    file_grow_starts_widths_as_needed(general, buffer, line_shift);
    buffer_remeasure_starts(buffer, line_start, line_end, line_shift, shift_amount);
    buffer_remeasure_widths(buffer, font->advance_data, line_start, line_end, line_shift);
    
    // NOTE(allen): update the views looking at this file
    Panel *panel, *used_panels;
    used_panels = &layout->used_sentinel;
    
    for (dll_items(panel, used_panels)){
        View *view = panel->view;
        if (view->file_data.file == file){
            view_measure_wraps(general, view);
        }
    }
    
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    // NOTE(allen): fixing stuff afterwards
    if (file->settings.tokens_exist)
        file_relex_parallel(system, mem, file, start, end, shift_amount);
#endif
    
    Cursor_Fix_Descriptor desc = {};
    desc.start = start;
    desc.end = end;
    desc.shift_amount = shift_amount;
    
    file_edit_cursor_fix(system, part, general, file, layout, desc);
}

internal void
file_do_white_batch_edit(System_Functions *system, Models *models, Editing_File *file,
                         Edit_Spec spec, History_Mode history_mode, b32 use_high_permission = 0){
    if (!use_high_permission && file->settings.read_only) return;
    
    Mem_Options *mem = &models->mem;
    Editing_Layout *layout = &models->layout;
    
    // NOTE(allen): fixing stuff "beforewards"???    
    Assert(spec.str == 0);
    file_update_history_before_edit(mem, file, spec.step, 0, history_mode);
    file_pre_edit_maintenance(system, &mem->general, file);
    
    // NOTE(allen): actual text replacement
    General_Memory *general = &mem->general;
    Partition *part = &mem->part;
    
    u8 *str_base = file->state.undo.children.strings;
    i32 batch_size = spec.step.child_count;
    Buffer_Edit *batch = file->state.undo.children.edits + spec.step.first_child;
    
    Assert(spec.step.first_child < file->state.undo.children.edit_count);
    Assert(batch_size >= 0);
    
    i32 scratch_size = partition_remaining(part);
    Buffer_Batch_State state = {};
    i32 request_amount;
    while (buffer_batch_edit_step(&state, &file->state.buffer, batch,
                                  (char*)str_base, batch_size, part->base + part->pos,
                                  scratch_size, &request_amount)){
        void *new_data = 0;
        if (request_amount > 0){
            new_data = general_memory_allocate(general, request_amount, BUBBLE_BUFFER);
        }
        void *old_data = buffer_edit_provide_memory(&file->state.buffer, new_data, request_amount);
        if (old_data) general_memory_free(general, old_data);
    }
    
    // NOTE(allen): meta data
    {
        Buffer_Measure_Starts state = {};
        Render_Font *font = get_font_info(models->font_set, file->state.font_id)->font;
        float *advance_data = 0;
        if (font) advance_data = font->advance_data;
        buffer_measure_starts_widths(&state, &file->state.buffer, advance_data);
    }
    
    // NOTE(allen): cursor fixing
    {
        Cursor_Fix_Descriptor desc = {};
        desc.is_batch = 1;
        desc.batch = batch;
        desc.batch_size = batch_size;
        
        file_edit_cursor_fix(system, part, general, file, layout, desc);
    }
    
    // NOTE(allen): token fixing
    if (file->state.tokens_complete){
        Cpp_Token_Stack tokens = file->state.token_stack;
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
}

inline void
file_replace_range(System_Functions *system, Models *models, Editing_File *file,
                   i32 start, i32 end, char *str, i32 len, i32 next_cursor, b32 use_high_permission = 0){
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.edit.start =  start;
    spec.step.edit.end = end;
    
    spec.step.edit.len = len;
    spec.step.pre_pos = file->state.cursor_pos;
    spec.step.post_pos = next_cursor;
    spec.str = (u8*)str;
    file_do_single_edit(system, models, file, spec, hist_normal, use_high_permission);
}

inline void
file_clear(System_Functions *system, Models *models, Editing_File *file, b32 use_high_permission = 0){
    file_replace_range(system, models, file, 0, buffer_size(&file->state.buffer), 0, 0, 0, use_high_permission);
}

inline void
view_replace_range(System_Functions *system, Models *models, View *view,
                   i32 start, i32 end, char *str, i32 len, i32 next_cursor){
    file_replace_range(system, models, view->file_data.file, start, end, str, len, next_cursor);
}

inline void
view_post_paste_effect(View *view, i32 ticks, i32 start, i32 size, u32 color){
    Editing_File *file = view->file_data.file;
    
    file->state.paste_effect.start = start;
    file->state.paste_effect.end = start + size;
    file->state.paste_effect.color = color;
    file->state.paste_effect.tick_down = ticks;
    file->state.paste_effect.tick_max = ticks;
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
view_undo_redo(System_Functions *system,
               Models *models, View *view,
               Edit_Stack *stack, Edit_Type expected_type){
    Editing_File *file = view->file_data.file;
    
    if (stack->edit_count > 0){
        Edit_Step step = stack->edits[stack->edit_count-1];
        
        Assert(step.type == expected_type);
        
        Edit_Spec spec = {};
        spec.step = step;
        
        if (step.child_count == 0){
            spec.step.edit.str_start = 0;
            spec.str = stack->strings + step.edit.str_start;
            
            file_do_single_edit(system, models, file, spec, hist_normal);
            
            if (expected_type == ED_UNDO) view_cursor_move(view, step.pre_pos);
            else view_cursor_move(view, step.post_pos);
            view->recent->mark = view->recent->cursor.pos;
            
            Style *style = main_style(models);
            view_post_paste_effect(view, 10, step.edit.start, step.edit.len,
                                   style->main.undo_color);
        }
        else{
            TentativeAssert(spec.step.special_type == 1);
            file_do_white_batch_edit(system, models, view->file_data.file, spec, hist_normal);
        }
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
        Edit_Spec spec;
        spec.step = step;
        
        if (spec.step.child_count == 0){
            spec.step.edit.str_start = 0;
            spec.str = file->state.undo.history.strings + step.edit.str_start;
            
            file_do_single_edit(system, models, file, spec, history_mode);
            
            switch (spec.step.type){
                case ED_NORMAL:
                case ED_REDO:
                view_cursor_move(view, step.post_pos);
                break;
                
                case ED_REVERSE_NORMAL:
                case ED_UNDO:
                view_cursor_move(view, step.pre_pos);
                break;
            }
            view->recent->mark = view->recent->cursor.pos;
        }
        else{
            TentativeAssert(spec.step.special_type == 1);
            file_do_white_batch_edit(system, models, view->file_data.file, spec, history_mode);
        }
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
    *result = make_string(new_str, 0, str_size);
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
file_compute_whitespace_edit(Mem_Options *mem, Editing_File *file, i32 cursor_pos,
                             Buffer_Edit *edits, char *str_base, i32 str_size,
                             Buffer_Edit *inverse_array, char *inv_str, i32 inv_max,
                             i32 edit_count){
    General_Memory *general = &mem->general;
    
    i32 inv_str_pos = 0;
    Buffer_Invert_Batch state = {};
    if (buffer_invert_batch(&state, &file->state.buffer, edits, edit_count,
                            inverse_array, inv_str, &inv_str_pos, inv_max)){
        Assert(0);
    }
    
    i32 first_child =
        undo_children_push(general, &file->state.undo.children,
                           edits, edit_count, (u8*)(str_base), str_size);
    i32 inverse_first_child =
        undo_children_push(general, &file->state.undo.children,
                           inverse_array, edit_count, (u8*)(inv_str), inv_str_pos);
    
    Edit_Spec spec = {};
    spec.step.type = ED_NORMAL;
    spec.step.first_child = first_child;
    spec.step.inverse_first_child = inverse_first_child;
    spec.step.special_type = 1;
    spec.step.child_count = edit_count;
    spec.step.inverse_child_count = edit_count;
    spec.step.pre_pos = cursor_pos;
    spec.step.post_pos = cursor_pos;
    
    return spec;
}

internal void
view_clean_whitespace(System_Functions *system, Models *models, View *view){
    Mem_Options *mem = &models->mem;
    Editing_File *file = view->file_data.file;
    
    Partition *part = &mem->part;
    i32 line_count = file->state.buffer.line_count;
    i32 edit_max = line_count * 2;
    i32 edit_count = 0;
    
    Assert(file && !file->is_dummy);
    
    Temp_Memory temp = begin_temp_memory(part);
    Buffer_Edit *edits = push_array(part, Buffer_Edit, edit_max);
    
    char *str_base = (char*)part->base + part->pos;
    i32 str_size = 0;
    for (i32 line_i = 0; line_i < line_count; ++line_i){
        i32 start = file->state.buffer.line_starts[line_i];
        Hard_Start_Result hard_start = 
            buffer_find_hard_start(&file->state.buffer, start, 4);
        
        if (hard_start.all_whitespace) hard_start.indent_pos = 0;
        
        if ((hard_start.all_whitespace && hard_start.char_pos > start) || !hard_start.all_space){
            Buffer_Edit new_edit;
            new_edit.str_start = str_size;
            str_size += hard_start.indent_pos;
            char *str = push_array(part, char, hard_start.indent_pos);
            for (i32 j = 0; j < hard_start.indent_pos; ++j) str[j] = ' ';
            new_edit.len = hard_start.indent_pos;
            new_edit.start = start;
            new_edit.end = hard_start.char_pos;
            edits[edit_count++] = new_edit;
        }
        Assert(edit_count <= edit_max);
    }
    
    if (edit_count > 0){
        Assert(buffer_batch_debug_sort_check(edits, edit_count));
        
        // NOTE(allen): computing edit spec, doing batch edit
        Buffer_Edit *inverse_array = push_array(part, Buffer_Edit, edit_count);
        Assert(inverse_array);
        
        char *inv_str = (char*)part->base + part->pos;
        Edit_Spec spec =
            file_compute_whitespace_edit(mem, file, view->recent->cursor.pos, edits, str_base, str_size,
                                         inverse_array, inv_str, part->max - part->pos, edit_count);
        
        file_do_white_batch_edit(system, models, view->file_data.file, spec, hist_normal);
    }
    
    end_temp_memory(temp);
}

struct Indent_Options{
    b32 empty_blank_lines;
    b32 use_tabs;
    i32 tab_width;
};

struct Make_Batch_Result{
    char *str_base;
    i32 str_size;
    
    Buffer_Edit *edits;
    i32 edit_max;
    i32 edit_count;
};

internal Cpp_Token*
get_first_token_at_line(Buffer *buffer, Cpp_Token_Stack tokens, i32 line){
    Cpp_Token *result = 0;
    i32 start_pos = 0;
    Cpp_Get_Token_Result get_token = {0};
    
    start_pos = buffer->line_starts[line];
    get_token = cpp_get_token(&tokens, start_pos);
    if (get_token.in_whitespace) get_token.token_index += 1;
    result = tokens.tokens + get_token.token_index;
    
    return(result);
}

internal Cpp_Token*
seek_matching_token_backwards(Cpp_Token_Stack tokens, Cpp_Token *token,
                              Cpp_Token_Type open_type, Cpp_Token_Type close_type){
    int nesting_level = 0;
    if (token <= tokens.tokens){
        token = tokens.tokens;
    }
    else{
        for (; token > tokens.tokens; --token){
            if (!(token->flags & CPP_TFLAG_PP_BODY)){
                if (token->type == close_type){
                    ++nesting_level;
                }
                else if (token->type == open_type){
                    if (nesting_level == 0){
                        break;
                    }
                    else{
                        --nesting_level;
                    }
                }
            }
        }
    }
    return(token);
}

struct Indent_Parse_State{
    i32 current_indent;
    i32 previous_line_indent;
    i32 paren_nesting;
    i32 paren_anchor_indent[16];
};

internal i32
compute_this_indent(Buffer *buffer, Indent_Parse_State indent,
                    Cpp_Token T, Cpp_Token prev_token, i32 line_i, i32 tab_width){
    
    i32 previous_indent = indent.previous_line_indent;
    i32 this_indent = 0;
    
    i32 this_line_start = buffer->line_starts[line_i];
    i32 next_line_start = 0;
    
    if (line_i+1 < buffer->line_count){
        next_line_start = buffer->line_starts[line_i+1];
    }
    else{
        next_line_start = buffer_size(buffer);
    }
    
    if ((prev_token.type == CPP_TOKEN_COMMENT ||
         prev_token.type == CPP_TOKEN_STRING_CONSTANT) &&
        prev_token.start <= this_line_start &&
        prev_token.start + prev_token.size > this_line_start){
        this_indent = previous_indent;
    }
    else{
        this_indent = indent.current_indent;
        if (T.start < next_line_start){
            if (T.flags & CPP_TFLAG_PP_DIRECTIVE){
                this_indent = 0;
            }
            else{
                switch (T.type){
                    case CPP_TOKEN_BRACKET_CLOSE: this_indent -= tab_width; break;
                    case CPP_TOKEN_BRACE_CLOSE: this_indent -= tab_width; break;
                    case CPP_TOKEN_BRACE_OPEN: break;
                    
                    default:
                    if (indent.current_indent > 0){
                        if (!(prev_token.flags & CPP_TFLAG_PP_BODY ||
                              prev_token.flags & CPP_TFLAG_PP_DIRECTIVE)){
                            switch (prev_token.type){
                                case CPP_TOKEN_BRACKET_OPEN:
                                case CPP_TOKEN_BRACE_OPEN: case CPP_TOKEN_BRACE_CLOSE:
                                case CPP_TOKEN_SEMICOLON: case CPP_TOKEN_COLON:
                                case CPP_TOKEN_COMMA: case CPP_TOKEN_COMMENT: break;
                                default: this_indent += tab_width;
                            }
                        }
                    }
                }
            }
        }
        if (this_indent < 0) this_indent = 0;
    }
    
    if (indent.paren_nesting > 0){
        if (prev_token.type != CPP_TOKEN_PARENTHESE_OPEN){
            i32 level = indent.paren_nesting-1;
            if (level >= ArrayCount(indent.paren_anchor_indent)){
                level = ArrayCount(indent.paren_anchor_indent)-1;
            }
            this_indent = indent.paren_anchor_indent[level];
        }
    }
    return(this_indent);
}

internal i32*
get_line_indentation_marks(Partition *part, Buffer *buffer, Cpp_Token_Stack tokens,
                           i32 line_start, i32 line_end, i32 tab_width){
    
    i32 indent_mark_count = line_end - line_start;
    i32 *indent_marks = push_array(part, i32, indent_mark_count);
    
    Indent_Parse_State indent = {0};
    Cpp_Token *token = get_first_token_at_line(buffer, tokens, line_start);
    
    if (token != tokens.tokens){
        --token;
        for (; token > tokens.tokens; --token){
            if (!(token->flags & CPP_TFLAG_PP_BODY)){
                switch(token->type){
                    case CPP_TOKEN_BRACE_OPEN:
                    case CPP_TOKEN_BRACE_CLOSE:
                    goto out_of_loop;
                }
            }
        }
        out_of_loop:;
    }
    
    // TODO(allen): This can maybe be it's own function now, so that we
    // can do the decls in the order we want and avoid the extra binary search.
    i32 found_safe_start_position = 0;
    do{
        i32 line = buffer_get_line_index(buffer, token->start);
        i32 start = buffer->line_starts[line];
        Hard_Start_Result hard_start = buffer_find_hard_start(buffer, start, tab_width);
        
        indent.current_indent = hard_start.indent_pos;
        
        Cpp_Token *start_token = get_first_token_at_line(buffer, tokens, line);
        Cpp_Token *brace_token = token;
        
        if (start_token->type == CPP_TOKEN_PARENTHESE_OPEN){
            if (start_token == tokens.tokens){
                found_safe_start_position = 1;
            }
            else{
                token = start_token-1;
            }
        }
        else{
            int close = 0;
            
            for (token = brace_token; token > start_token; --token){
                switch(token->type){
                    case CPP_TOKEN_PARENTHESE_CLOSE:
                    case CPP_TOKEN_BRACKET_CLOSE:
                    case CPP_TOKEN_BRACE_CLOSE:
                    close = token->type;
                    goto out_of_loop2;
                }
            }
            out_of_loop2:;
            
            switch (close){
                case 0: token = start_token; found_safe_start_position = 1; break;
                
                case CPP_TOKEN_PARENTHESE_CLOSE:
                token = seek_matching_token_backwards(tokens, token-1,
                                                      CPP_TOKEN_PARENTHESE_OPEN,
                                                      CPP_TOKEN_PARENTHESE_CLOSE);
                break;
                
                case CPP_TOKEN_BRACKET_CLOSE:
                token = seek_matching_token_backwards(tokens, token-1,
                                                      CPP_TOKEN_BRACKET_OPEN,
                                                      CPP_TOKEN_BRACKET_CLOSE);
                break;
                
                case CPP_TOKEN_BRACE_CLOSE:
                token = seek_matching_token_backwards(tokens, token-1,
                                                      CPP_TOKEN_BRACE_OPEN,
                                                      CPP_TOKEN_BRACE_CLOSE);
                break;
            }
        }
    } while(found_safe_start_position == 0);
    
    // NOTE(allen): Shift the array so that line_i can just operate in
    // it's natural value range.
    indent_marks -= line_start;
    
    i32 line_i = buffer_get_line_index(buffer, token->start);
    
    if (line_i > line_start){
        line_i = line_start;
    }
    
    i32 next_line_start = buffer_size(buffer);
    if (line_i+1 < buffer->line_count){
        next_line_start = buffer->line_starts[line_i+1];
    }
    
    switch (token->type){
        case CPP_TOKEN_BRACKET_OPEN: indent.current_indent += tab_width; break;
        case CPP_TOKEN_BRACE_OPEN: indent.current_indent += tab_width; break;
        case CPP_TOKEN_PARENTHESE_OPEN: indent.current_indent += tab_width; break;
    }
    
    indent.previous_line_indent = indent.current_indent;
    Cpp_Token T;
    Cpp_Token prev_token = *token;
    ++token;
    
    for (; line_i < line_end; ++token){
        if (token < tokens.tokens + tokens.count){
            T = *token;
        }
        else{
            T.type = CPP_TOKEN_EOF;
            T.start = buffer_size(buffer);
            T.flags = 0;
        }
        
        for (; T.start >= next_line_start && line_i < line_end;){
            if (line_i+1 < buffer->line_count){
                next_line_start = buffer->line_starts[line_i+1];
            }
            else{
                next_line_start = buffer_size(buffer);
            }
            
            i32 this_indent =
                compute_this_indent(buffer, indent, T, prev_token, line_i, tab_width);
            
            // NOTE(allen): Rebase the paren anchor if the first token
            // after an open paren is on the next line.
            if (indent.paren_nesting > 0){
                if (prev_token.type == CPP_TOKEN_PARENTHESE_OPEN){
                    i32 level = indent.paren_nesting-1;
                    if (level >= ArrayCount(indent.paren_anchor_indent)){
                        level = ArrayCount(indent.paren_anchor_indent)-1;
                    }
                    indent.paren_anchor_indent[level] = this_indent;
                }
            }
            
            if (line_i >= line_start){
                indent_marks[line_i] = this_indent;
            }
            ++line_i;
            
            indent.previous_line_indent = this_indent;
        }
        
        switch (T.type){
            case CPP_TOKEN_BRACKET_OPEN: indent.current_indent += 4; break;
            case CPP_TOKEN_BRACKET_CLOSE: indent.current_indent -= 4; break;
            case CPP_TOKEN_BRACE_OPEN: indent.current_indent += 4; break;
            case CPP_TOKEN_BRACE_CLOSE: indent.current_indent -= 4; break;
            
            case CPP_TOKEN_PARENTHESE_OPEN:
            if (!(T.flags & CPP_TFLAG_PP_BODY)){
                if (indent.paren_nesting < ArrayCount(indent.paren_anchor_indent)){
                    i32 line = buffer_get_line_index(buffer, T.start);
                    i32 start = buffer->line_starts[line];
                    i32 char_pos = T.start - start;
                    
                    Hard_Start_Result hard_start =
                        buffer_find_hard_start(buffer, start, tab_width);
                    
                    i32 line_pos = hard_start.char_pos - start;
                    
                    indent.paren_anchor_indent[indent.paren_nesting] =
                        char_pos - line_pos + indent.previous_line_indent + 1;
                }
                ++indent.paren_nesting;
            }
            break;
            
            case CPP_TOKEN_PARENTHESE_CLOSE:
            if (!(T.flags & CPP_TFLAG_PP_BODY)){
                --indent.paren_nesting;
            }
            break;
        }
        prev_token = T;
    }
    
    // NOTE(allen): Unshift the indent_marks array so that the return value
    // is the exact starting point of the array that was actually allocated.
    indent_marks += line_start;
    
    return(indent_marks);
}

internal Make_Batch_Result
make_batch_from_indent_marks(Partition *part, Buffer *buffer, i32 line_start, i32 line_end,
                             i32 *indent_marks, Indent_Options opts){
    
    Make_Batch_Result result = {0};
    
    i32 edit_max = line_end - line_start;
    i32 edit_count = 0;
    
    Buffer_Edit *edits = push_array(part, Buffer_Edit, edit_max);
    
    char *str_base = (char*)part->base + part->pos;
    i32 str_size = 0;
    
    // NOTE(allen): Shift the array so that line_i can just operate in
    // it's natural value range.
    indent_marks -= line_start;
    
    for (i32 line_i = line_start; line_i < line_end; ++line_i){
        i32 start = buffer->line_starts[line_i];
        Hard_Start_Result hard_start = 
            buffer_find_hard_start(buffer, start, opts.tab_width);
        
        i32 correct_indentation = indent_marks[line_i];
        if (hard_start.all_whitespace && opts.empty_blank_lines) correct_indentation = 0;
        if (correct_indentation == -1) correct_indentation = hard_start.indent_pos;
        
        if ((hard_start.all_whitespace && hard_start.char_pos > start) ||
            !hard_start.all_space || correct_indentation != hard_start.indent_pos){
            Buffer_Edit new_edit;
            new_edit.str_start = str_size;
            str_size += correct_indentation;
            char *str = push_array(part, char, correct_indentation);
            i32 j = 0;
            if (opts.use_tabs){
                i32 i = 0;
                for (; i + opts.tab_width <= correct_indentation; i += opts.tab_width) str[j++] = '\t';
                for (; i < correct_indentation; ++i) str[j++] = ' ';
            }
            else{
                for (; j < correct_indentation; ++j) str[j] = ' ';
            }
            new_edit.len = j;
            new_edit.start = start;
            new_edit.end = hard_start.char_pos;
            edits[edit_count++] = new_edit;
        }
        
        Assert(edit_count <= edit_max);
    }
    
    result.str_base = str_base;
    result.str_size = str_size;
    
    result.edits = edits;
    result.edit_max = edit_max;
    result.edit_count = edit_count;
    
    return(result);
}

internal void
view_auto_tab_tokens(System_Functions *system, Models *models,
                     View *view, i32 start, i32 end, Indent_Options opts){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    Editing_File *file = view->file_data.file;
    Mem_Options *mem = &models->mem;
    Partition *part = &mem->part;
    Buffer *buffer = &file->state.buffer;
    
    Assert(file && !file->is_dummy);
    Cpp_Token_Stack tokens = file->state.token_stack;
    Assert(tokens.tokens);
    
    i32 line_start = buffer_get_line_index(buffer, start);
    i32 line_end = buffer_get_line_index(buffer, end) + 1;
    
    Temp_Memory temp = begin_temp_memory(part);
    
    i32 *indent_marks =
        get_line_indentation_marks(part, buffer, tokens, line_start, line_end, opts.tab_width);
    
    Make_Batch_Result batch = 
        make_batch_from_indent_marks(part, buffer, line_start, line_end, indent_marks, opts);
    
    if (batch.edit_count > 0){
        Assert(buffer_batch_debug_sort_check(batch.edits, batch.edit_count));
        
        // NOTE(allen): computing edit spec, doing batch edit
        Buffer_Edit *inverse_array = push_array(part, Buffer_Edit, batch.edit_count);
        Assert(inverse_array);
        
        char *inv_str = (char*)part->base + part->pos;
        Edit_Spec spec =
            file_compute_whitespace_edit(mem, file, view->recent->cursor.pos,
                                         batch.edits, batch.str_base, batch.str_size,
                                         inverse_array, inv_str, part->max - part->pos, batch.edit_count);
        
        file_do_white_batch_edit(system, models, view->file_data.file, spec, hist_normal);
    }
    end_temp_memory(temp);
    
    {
        i32 start = view->recent->cursor.pos;
        Hard_Start_Result hard_start = buffer_find_hard_start(buffer, start, 4);
        
        view_cursor_move(view, hard_start.char_pos);
    }
#endif
}

struct Get_Link_Result{
    b32 in_link;
    i32 index;
};

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
            
            case CPP_TOKEN_INCLUDE_FILE:
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
remeasure_file_view(System_Functions *system, View *view){
    if (file_is_ready(view->file_data.file)){
        Relative_Scrolling relative = view_get_relative_scrolling(view);
        view_measure_wraps(&view->persistent.models->mem.general, view);
        if (view->file_data.show_temp_highlight == 0){
            view_cursor_move(view, view->recent->cursor.pos);
            view->recent->preferred_x = view_get_cursor_x(view);
        }
        view_set_relative_scrolling(view, relative);
    }
}

inline void
view_show_GUI(View *view, View_UI ui){
    view->map = &view->persistent.models->map_ui;
    view->showing_ui = ui;
    view->current_scroll = &view->gui_scroll;
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
    view->current_scroll = &view->gui_scroll;
    
    view->map = &models->map_ui;
    
    hot_directory_clean_end(&models->hot_directory);
    hot_directory_reload(system, &models->hot_directory, &models->working_set);
    view->changed_context_in_step = 1;
}

inline void
view_show_theme(View *view){
    view->map = &view->persistent.models->map_ui;
    view->showing_ui = VUI_Theme;
    view->color_mode = CV_Mode_Library;
    view->color = super_color_create(0xFF000000);
    view->current_color_editing = 0;
    view->current_scroll = &view->gui_scroll;
    view->changed_context_in_step = 1;
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
    view->showing_ui = VUI_None;
    view->current_scroll = &view->recent->scroll;
    view->recent->scroll.max_y = view_compute_max_target_y(view);
    view->changed_context_in_step = 1;
}

internal void
view_save_file(System_Functions *system, Models *models,
               Editing_File *file, View *view, String filename, b32 save_as){
    Mem_Options *mem = &models->mem;
    Working_Set *working_set = &models->working_set;
    
    Temp_Memory temp = begin_temp_memory(&mem->part);
    
    String filename_string =
        make_string_terminated(&mem->part, filename.str, filename.size);
    
    if (!file){
        if (view){
            file = view->file_data.file;
        }
        else{
            file = working_set_lookup_file(working_set, filename_string);
        }
    }
    
    if (file && buffer_get_sync(file) != SYNC_GOOD){
        if (file_save(system, mem, file, filename_string.str)){
            if (save_as){
                file_set_name(working_set, file, filename_string.str);
            }
        }
    }
    
    end_temp_memory(temp);
}

internal void
view_new_file(System_Functions *system, Models *models,
              View *view, String string){
    Working_Set *working_set = &models->working_set;
    General_Memory *general = &models->mem.general;
    
    Editing_File *file = working_set_alloc_always(working_set, general);
    file_create_empty(system, models, file, string.str);
    working_set_add(system, working_set, file, general);
    
    view_set_file(view, file, models);
    view_show_file(view);
    view->map = get_map(models, file->settings.base_map_id);
    
    Open_File_Hook_Function *new_file_fnc = models->hook_new_file;
    if (new_file_fnc){
        new_file_fnc(&models->app_links, file->id.id);
    }
    file->settings.is_initialized = 1;
    
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    if (file->settings.tokens_exist){
        file_first_lex_parallel(system, general, file);
    }
#endif
}

internal void
init_normal_file(System_Functions *system, Models *models, Editing_File *file,
                 char *buffer, i32 size){
    
    General_Memory *general = &models->mem.general;
    
    String val = make_string(buffer, size);
    file_create_from_string(system, models, file, file->name.source_path.str, val);
    
    if (file->settings.tokens_exist){
        file_first_lex_parallel(system, general, file);
    }
    
    for (View_Iter iter = file_view_iter_init(&models->layout, file, 0);
         file_view_iter_good(iter);
         iter = file_view_iter_next(iter)){
        view_measure_wraps(general, iter.view);
    }
}

internal void
view_open_file(System_Functions *system, Models *models,
               View *view, String filename){
    Working_Set *working_set = &models->working_set;
    General_Memory *general = &models->mem.general;
    Partition *part = &models->mem.part;
    
    Editing_File *file = working_set_contains(system, working_set, filename);
    
    if (file == 0){
        File_Loading loading = system->file_load_begin(filename.str);
        
        if (loading.exists){
            b32 in_general_mem = 0;
            Temp_Memory temp = begin_temp_memory(part);
            char *buffer = push_array(part, char, loading.size);
            
            // TODO(allen): How will we get temporary space for large
            // buffers?  The main partition isn't always big enough
            // but getting a general block this large and copying it
            // then freeing it is *super* dumb!
            if (buffer == 0){
                buffer = (char*)general_memory_allocate(general, loading.size);
                if (buffer != 0){
                    in_general_mem = 1;
                }
            }
            
            if (system->file_load_end(loading, buffer)){
                file = working_set_alloc_always(working_set, general);
                if (file){
                    file_init_strings(file);
                    file_set_name(working_set, file, filename.str);
                    working_set_add(system, working_set, file, general);
                    
                    init_normal_file(system, models, file,
                                     buffer, loading.size);
                }
            }
            
            if (in_general_mem){
                general_memory_free(general, buffer);
            }
            
            end_temp_memory(temp);
        }
    }
    
    if (file){
        if (view){
            view_set_file(view, file, models);
            view_show_file(view);
        }
    }
}

internal void
kill_file(System_Functions *system, Models *models,
          Editing_File *file, String string){
    Working_Set *working_set = &models->working_set;
    
    if (!file && string.str){
        file = working_set_lookup_file(working_set, string);
        if (!file){
            file = working_set_contains(system, working_set, string);
        }
    }
    
    if (file && !file->settings.never_kill){
        working_set_remove(system, working_set, file->name.source_path);
        file_close(system, &models->mem.general, file);
        working_set_free_file(&models->working_set, file);
        
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
try_kill_file(System_Functions *system, Models *models,
              Editing_File *file, View *view, String string){
    Working_Set *working_set = &models->working_set;
    
    if (!file && string.str){
        file = working_set_lookup_file(working_set, string);
        if (!file){
            file = working_set_contains(system, working_set, string);
        }
    }
    
    if (file && !file->settings.never_kill){
        if (buffer_needs_save(file)){
            if (view == 0){
                view = models->layout.panels[models->layout.active_panel].view;
            }
            view_show_interactive(system, view,
                                  IAct_Sure_To_Kill, IInt_Sure_To_Kill,
                                  make_lit_string("Are you sure?"));
            copy(&view->dest, file->name.live_name);
        }
        else{
            kill_file(system, models, file, string_zero());
        }
    }
}

internal void
interactive_view_complete(System_Functions *system, View *view, String dest, i32 user_action){
    Models *models = view->persistent.models;
    //Editing_File *old_file = view->file_data.file;
    
    switch (view->action){
        case IAct_Open:
        view_open_file(system, models, view, dest);
        //touch_file(&models->working_set, old_file);
        view_show_file(view);
        break;
        
        case IAct_Save_As:
        view_save_file(system, models, 0, view, dest, 1);
        view_show_file(view);
        break;
        
        case IAct_New:
        if (dest.size > 0 && !char_is_slash(dest.str[dest.size-1])){
            view_new_file(system, models, view, dest);
            view_show_file(view);
        }break;
        
        case IAct_Switch:
        {
            //touch_file(&models->working_set, old_file);
            
            Editing_File *file = 0;
            String string = dest;
            
            file = working_set_lookup_file(&models->working_set, string);
            if (!file){
                file = working_set_contains(system, &models->working_set, string);
            }
            if (file){
                view_set_file(view, file, models);
            }
            view_show_file(view);
        }
        break;
        
        case IAct_Kill:
        try_kill_file(system, models, 0, 0, dest);
        view_show_file(view);
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
            kill_file(system, models, 0, dest);
            view_show_file(view);
            break;
            
            case 1:
            view_show_file(view);
            break;
            
            case 2:
            view_save_file(system, models, 0, 0, dest, 0);
            kill_file(system, models, 0, dest);
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
        Cpp_Token_Stack *tokens = &file->state.token_stack;
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

internal void
view_reinit_scrolling(View *view){
    Editing_File *file = view->file_data.file;
    f32 w = 0, h = 0;
    f32 cursor_x = 0, cursor_y = 0;
    i32 target_x = 0, target_y = 0;
    
    view->reinit_scrolling = 0;
    
    if (file && file_is_ready(file)){
        cursor_x = view_get_cursor_x(view);
        cursor_y = view_get_cursor_y(view);
        
        w = view_file_width(view);
        h = view_file_height(view);
        
        if (cursor_x >= target_x + w){
            target_x = ROUND32(cursor_x - w*.5f);
        }
        
        target_y = clamp_bottom(0, FLOOR32(cursor_y - h*.5f));
    }
    
    view->recent->scroll.target_y = target_y;
    view->recent->scroll.scroll_y = (f32)target_y;
    view->recent->scroll.prev_target_y = -1000;
    
    view->recent->scroll.target_x = target_x;
    view->recent->scroll.scroll_x = (f32)target_x;
    view->recent->scroll.prev_target_x = -1000;
}

enum CursorScroll_State{
    CursorScroll_NoChange = 0x0,
    CursorScroll_Cursor = 0x1,
    CursorScroll_Scroll = 0x2,
    CursorScroll_ContextChange = 0x4
};

internal u32
view_get_cursor_scroll_change_state(View *view){
    u32 result = 0;
    i32 pos = 0;
    Scroll_Context context = {0};
    
    if (view->gui_target.did_file){
        pos = view_get_cursor_pos(view);
        if ((view->prev_cursor_pos != pos)){
            result |= CursorScroll_Cursor;
        }
    }
    
    if (view->current_scroll){
        if (!gui_scroll_eq(view->current_scroll, &view->gui_target.scroll_original)){
            result |= CursorScroll_Scroll;
        }
    }
    
    if (context.mode == VUI_None){
        context.file = view->file_data.file;
    }
    else{
        context.file = view->prev_context.file;
    }
    context.scroll = view->gui_target.scroll_id;
    context.mode = view->showing_ui;
    
    if (!context_eq(view->prev_context, context)){
        result |= CursorScroll_ContextChange;
    }
    
    return(result);
}

internal void
view_begin_cursor_scroll_updates(View *view){
    if (view->file_data.file && view->file_data.file == view->prev_context.file){
        Assert(view->prev_cursor_pos == view_get_cursor_pos(view));
    }
}

internal void
view_end_cursor_scroll_updates(View *view){
    i32 cursor_scroll_state =
        view_get_cursor_scroll_change_state(view);
    
    switch (cursor_scroll_state){
        case CursorScroll_NoChange:break;
        
        case CursorScroll_Cursor:
        case CursorScroll_Cursor|CursorScroll_Scroll:
        if (view->gui_target.did_file){
            view->recent->scroll.max_y = view_compute_max_target_y(view);
        }
        view_move_view_to_cursor(view, view->current_scroll);
        gui_post_scroll_vars(&view->gui_target, view->current_scroll, view->scroll_region);
        break;
        
        case CursorScroll_Scroll:
        view_move_cursor_to_view(view, view->recent->scroll);
        gui_post_scroll_vars(&view->gui_target, view->current_scroll, view->scroll_region);
        break;
    }
    
    if (cursor_scroll_state & CursorScroll_ContextChange){
        view->current_scroll->scroll_y = (f32)view->current_scroll->target_y;
        view->current_scroll->scroll_x = (f32)view->current_scroll->target_x;
        gui_post_scroll_vars(&view->gui_target, view->current_scroll, view->scroll_region);
    }
    
    view->prev_cursor_pos = view_get_cursor_pos(view);
    
    view->prev_context.file = view->file_data.file;
    view->prev_context.scroll = view->gui_target.scroll_id;
    view->prev_context.mode = view->showing_ui;
}

internal b32
file_step(View *view, i32_Rect region, Input_Summary *user_input, b32 is_active, b32 *consumed_l){
    i32 is_animating = 0;
    Editing_File *file = view->file_data.file;
    if (file && !file->is_loading){
        if (file->state.paste_effect.tick_down > 0){
            --file->state.paste_effect.tick_down;
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
    
    get_front_of_directory(&loop->front_name, hdir->string);
    get_absolutes(loop->front_name, &loop->absolutes, 1, 1);
    get_path_of_directory(&loop->full_path, hdir->string);
    loop->r = loop->full_path.size;
}

internal Exhaustive_File_Info
get_exhaustive_info(System_Functions *system, Working_Set *working_set, Exhaustive_File_Loop *loop, i32 i){
    persist String message_loaded = make_lit_string(" LOADED");
    persist String message_unsaved = make_lit_string(" LOADED *");
    persist String message_unsynced = make_lit_string(" LOADED !");
    
    Exhaustive_File_Info result = {0};
    Editing_File *file = 0;
    
    result.info = loop->infos + i;
    loop->full_path.size = loop->r;
    append(&loop->full_path, result.info->filename);
    terminate_with_null(&loop->full_path);
    file = working_set_contains(system, working_set, loop->full_path);
    
    result.is_folder = (result.info->folder != 0);
    result.name_match = (filename_match(loop->front_name, &loop->absolutes, result.info->filename, 0) != 0);
    result.is_loaded = (file != 0 && file_is_ready(file));
    
    result.message = string_zero();
    if (result.is_loaded){
        switch (buffer_get_sync(file)){
            case SYNC_GOOD: result.message = message_loaded; break;
            case SYNC_BEHIND_OS: result.message = message_unsynced; break;
            case SYNC_UNSAVED: result.message = message_unsaved; break;
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
app_single_line_input_core(System_Functions *system, Working_Set *working_set,
    Key_Event_Data key, Single_Line_Mode mode){
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
                    char end_character = mode.string->str[mode.string->size];
                    if (char_is_slash(end_character)){
                        mode.string->size = reverse_seek_slash(*mode.string) + 1;
                        mode.string->str[mode.string->size] = 0;
                        hot_directory_set(system, mode.hot_directory, *mode.string, working_set);
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
        if (!key.modifiers[MDFR_CONTROL_INDEX] &&
                !key.modifiers[MDFR_ALT_INDEX]){
            if (mode.string->size+1 < mode.string->memory_size){
                u8 new_character = (u8)key.character;
                mode.string->str[mode.string->size] = new_character;
                mode.string->size++;
                mode.string->str[mode.string->size] = 0;
                if (mode.type == SINGLE_LINE_FILE && char_is_slash(new_character)){
                    hot_directory_set(system, mode.hot_directory, *mode.string, working_set);
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
                           b32 fast_folder_select, b32 try_to_match, b32 case_sensitive){
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_FILE;
    mode.string = string;
    mode.hot_directory = hot_directory;
    mode.fast_folder_select = fast_folder_select;
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
        append(string, '>');
    }
    append(string, message);
}

internal void
show_gui_line(GUI_Target *target, String *string,
              i32 indent_level, i32 h_align, char *message, char *follow_up){
    string->size = 0;
    append_label(string, indent_level, message);
    if (follow_up){
        append_padding(string, '-', h_align);
        append(string, ' ');
        append(string, follow_up);
    }
    gui_do_text_field(target, *string, string_zero());
}

internal void
show_gui_int(GUI_Target *target, String *string,
             i32 indent_level, i32 h_align, char *message, i32 x){
    string->size = 0;
    append_label(string, indent_level, message);
    append_padding(string, '-', h_align);
    append(string, ' ');
    append_int_to_str(string, x);
    gui_do_text_field(target, *string, string_zero());
}

internal void
show_gui_u64(GUI_Target *target, String *string,
             i32 indent_level, i32 h_align, char *message, u64 x){
    string->size = 0;
    append_label(string, indent_level, message);
    append_padding(string, '-', h_align);
    append(string, ' ');
    append_u64_to_str(string, x);
    gui_do_text_field(target, *string, string_zero());
}

internal void
show_gui_int_int(GUI_Target *target, String *string,
                 i32 indent_level, i32 h_align, char *message, i32 x, i32 m){
    string->size = 0;
    append_label(string, indent_level, message);
    append_padding(string, '-', h_align);
    append(string, ' ');
    append_int_to_str(string, x);
    append(string, '/');
    append_int_to_str(string, m);
    gui_do_text_field(target, *string, string_zero());
}

internal void
show_gui_id(GUI_Target *target, String *string,
            i32 indent_level, i32 h_align, char *message, GUI_id id){
    string->size = 0;
    append_label(string, indent_level, message);
    append_padding(string, '-', h_align);
    append(string, " [0]: ");
    append_u64_to_str(string, id.id[0]);
    append_padding(string, ' ', h_align + 26);
    append(string, " [1]: ");
    append_u64_to_str(string, id.id[1]);
    gui_do_text_field(target, *string, string_zero());
}

internal void
show_gui_float(GUI_Target *target, String *string,
               i32 indent_level, i32 h_align, char *message, float x){
    string->size = 0;
    append_label(string, indent_level, message);
    append_padding(string, '-', h_align);
    append(string, ' ');
    append_float_to_str(string, x);
    gui_do_text_field(target, *string, string_zero());
}

internal void
show_gui_scroll(GUI_Target *target, String *string,
                i32 indent_level, i32 h_align, char *message,
                GUI_Scroll_Vars scroll){
    show_gui_line (target, string, indent_level, 0, message, 0);
    show_gui_float(target, string, indent_level+1, h_align, " scroll_y ", scroll.scroll_y);
    show_gui_int  (target, string, indent_level+1, h_align, " target_y ", scroll.target_y);
    show_gui_int  (target, string, indent_level+1, h_align, " prev_target_y ", scroll.prev_target_y);
    show_gui_int  (target, string, indent_level+1, h_align, " max_y ", scroll.max_y);
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
    append(string, "mouse: (");
    append_int_to_str(string, mx);
    append(string, ",");
    append_int_to_str(string, my);
    append(string, ")");
    
    gui_do_text_field(target, *string, string_zero());
}

internal View_Step_Result
step_file_view(System_Functions *system, View *view, View *active_view, Input_Summary input){
    View_Step_Result result = {0};
    GUI_Target *target = &view->gui_target;
    Models *models = view->persistent.models;
    Key_Summary keys = input.keys;
    
    b32 show_scrollbar = !view->hide_scrollbar;
    
    view->current_scroll = 0;
    
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
                
                view->current_scroll = &view->recent->scroll;
                gui_get_scroll_vars(target, scroll_context,
                                    &view->recent->scroll, &view->scroll_region);
                
                gui_begin_scrollable(target, scroll_context, view->recent->scroll,
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
                    view->current_scroll = &view->gui_scroll;
                    
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
                    view->current_scroll = &view->gui_scroll;
                    
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
                    view->current_scroll = &view->gui_scroll;
                    
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
                        
                        message = make_lit_string("Set Font");
                        id.id[0] = (u64)(&models->global_font);
                        if (gui_do_button(target, id, message)){
                            view->color_mode = CV_Mode_Font;
                        }
                        
                        message = make_lit_string("Theme Library - Click to Select");
                        gui_do_text_field(target, message, empty_string);
                        
                        view->current_scroll = &view->gui_scroll;
                        gui_get_scroll_vars(target, scroll_context, &view->gui_scroll, &view->scroll_region);
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
                        
                        case CV_Mode_Font:
                        {
                            Font_Set *font_set = models->font_set;
                            Font_Info *info = 0;
                            
                            i16 i = 1, count = (i16)models->font_set->count + 1;
                            i16 font_id = 0, new_font_id = 0;
                            
                            String message = make_lit_string("Back");
                            
                            id.id[0] = 0;
                            if (gui_do_button(target, id, message)){
                                view->color_mode = CV_Mode_Library;
                            }
                            
                            font_id = models->global_font.font_id;
                            new_font_id = font_id;
                            
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
                                    copy(&message, make_lit_string("currently selected: "));
                                    append(&message, info->name);
                                    gui_do_font_button(target, id, i, message);
                                }
                            }
                            
                            models->global_font.font_id = (i16)(new_font_id);
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
                            
                            view->current_scroll = &view->gui_scroll;
                            gui_get_scroll_vars(target, scroll_context, &view->gui_scroll, &view->scroll_region);
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
                                    
                                    color_to_hexstr(*edit_color, &text);
                                    if (gui_do_text_with_cursor(target, view->color_cursor, text, &update)){
                                        b32 r = 0;
                                        i32 j = 0;
                                        
                                        for (j = 0; j < keys.count; ++j){
                                            i16 key = keys.keys[j].keycode;
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
                    
                    view->current_scroll = &view->gui_scroll;
                    
                    GUI_id id = {0};
                    id.id[1] = VUI_Interactive + ((u64)view->interaction << 32);
                    
                    GUI_id scroll_context = {0};
                    scroll_context.id[1] = VUI_Interactive + ((u64)view->interaction << 32);
                    
                    switch (view->interaction){
                        case IInt_Sys_File_List:
                        {
                            b32 use_item_in_list = 1;
                            b32 activate_directly = 0;
                            
                            if (view->action == IAct_Save_As || view->action == IAct_New){
                                use_item_in_list = 0;
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
                            b32 snap_into_view = 0;
                            i32 i = 0;
                            
                            {
                                Single_Line_Input_Step step = {0};
                                Key_Event_Data key = {0};
                                i32 i;
                                
                                for (i = 0; i < keys.count; ++i){
                                    key = get_single_key(&keys, i);
                                    step = app_single_file_input_step(system, &models->working_set, key,
                                                                      &hdir->string, hdir, 1, 1, 0);
                                    if (step.made_a_change){
                                        view->list_i = 0;
                                        result.consume_keys = 1;
                                    }
                                    if (!use_item_in_list && (key.keycode == '\n' || key.keycode == '\t')){
                                        activate_directly = 1;
                                        result.consume_keys = 1;
                                    }
                                }
                            }
                            
                            gui_do_text_field(target, message, hdir->string);
                            
                            scroll_context.id[0] = (u64)(hdir);
                            if (gui_get_scroll_vars(target, scroll_context,
                                                    &view->gui_scroll, &view->scroll_region)){
                                snap_into_view = 1;
                            }
                            gui_begin_scrollable(target, scroll_context, view->gui_scroll,
                                                 9 * view->line_height, show_scrollbar);
                            
                            id.id[0] = (u64)(hdir) + 1;
                            
                            if (gui_begin_list(target, id, view->list_i, 0,
                                               snap_into_view, &update)){
                                // TODO(allen): Allow me to handle key consumption correctly here!
                                gui_standard_list(target, id, view->current_scroll, view->scroll_region,
                                                  &keys, &view->list_i, &update);
                            }
                            
                            {
                                begin_exhaustive_loop(&loop, hdir);
                                for (i = 0; i < loop.count; ++i){
                                    file_info = get_exhaustive_info(system, &models->working_set, &loop, i);
                                    
                                    if (file_info.name_match){
                                        id.id[0] = (u64)(file_info.info);
                                        if (gui_do_file_option(target, id, file_info.info->filename,
                                                               file_info.is_folder, file_info.message)){
                                            if (file_info.is_folder){
                                                set_last_folder(&hdir->string, file_info.info->filename, '/');
                                                do_new_directory = 1;
                                            }
                                            else if (use_item_in_list){
                                                complete = 1;
                                                copy(&comp_dest, loop.full_path);
                                            }
                                        }
                                    }
                                }
                            }
                            
                            gui_end_list(target);
                            
                            if (activate_directly){
                                complete = 1;
                                copy(&comp_dest, hdir->string);
                            }
                            
                            if (do_new_directory){
                                hot_directory_reload(system, hdir, &models->working_set);
                            }
                            
                            gui_end_scrollable(target);
                        }break;
                        
                        case IInt_Live_File_List:
                        {
                            b32 snap_into_view = 0;
                            persist String message_unsaved = make_lit_string(" *");
                            persist String message_unsynced = make_lit_string(" !");
                            
                            String message = {0};
                            switch (view->action){
                                case IAct_Switch: message = make_lit_string("Switch: "); break;
                                case IAct_Kill: message = make_lit_string("Kill: "); break;
                            }
                            
                            Absolutes absolutes;
                            Editing_File *file;
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
                            
                            get_absolutes(view->dest, &absolutes, 1, 1);
                            
                            gui_do_text_field(target, message, view->dest);
                            
                            scroll_context.id[0] = (u64)(working_set);
                            if (gui_get_scroll_vars(target, scroll_context,
                                                    &view->gui_scroll, &view->scroll_region)){
                                snap_into_view = 1;
                            }
                            gui_begin_scrollable(target, scroll_context, view->gui_scroll,
                                                 9 * view->line_height, show_scrollbar);
                            
                            id.id[0] = (u64)(working_set) + 1;
                            if (gui_begin_list(target, id, view->list_i,
                                               0, snap_into_view, &update)){
                                gui_standard_list(target, id, view->current_scroll, view->scroll_region,
                                                  &keys, &view->list_i, &update);
                            }
                            
                            {
                                Partition *part = &models->mem.part;
                                Temp_Memory temp = begin_temp_memory(part);
                                File_Node *node = 0, *used_nodes = 0;
                                Editing_File **reserved_files = 0;
                                i32 reserved_top = 0, i = 0;
                                View_Iter iter = {0};
                                
                                partition_align(part, sizeof(i32));
                                reserved_files = (Editing_File**)partition_current(part);
                                
                                used_nodes = &working_set->used_sentinel;
                                for (dll_items(node, used_nodes)){
                                    file = (Editing_File*)node;
                                    Assert(!file->is_dummy);
                                    
                                    if (filename_match(view->dest, &absolutes, file->name.live_name, 1)){
                                        iter = file_view_iter_init(layout, file, 0);
                                        if (file_view_iter_good(iter)){
                                            reserved_files[reserved_top++] = file;
                                        }
                                        else{
                                            if (file->name.live_name.str[0] == '*'){
                                                reserved_files[reserved_top++] = file;
                                            }
                                            else{
                                                message = string_zero();
                                                
                                                if (!file->settings.unimportant){
                                                    switch (buffer_get_sync(file)){
                                                        case SYNC_BEHIND_OS: message = message_unsynced; break;
                                                        case SYNC_UNSAVED: message = message_unsaved; break;
                                                    }
                                                }
                                                
                                                id.id[0] = (u64)(file);
                                                if (gui_do_file_option(target, id, file->name.live_name, 0, message)){
                                                    complete = 1;
                                                    copy(&comp_dest, file->name.live_name);
                                                }
                                            }
                                        }
                                    }
                                }
                                
                                for (i = 0; i < reserved_top; ++i){
                                    file = reserved_files[i];
                                    
                                    message = string_zero();
                                    switch (buffer_get_sync(file)){
                                        case SYNC_BEHIND_OS: message = message_unsynced; break;
                                        case SYNC_UNSAVED: message = message_unsaved; break;
                                    }
                                    
                                    id.id[0] = (u64)(file);
                                    if (gui_do_file_option(target, id, file->name.live_name, 0, message)){
                                        complete = 1;
                                        copy(&comp_dest, file->name.live_name);
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
                                copy(&comp_dest, view->dest);
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
                                copy(&comp_dest, view->dest);
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
                    
                    view->current_scroll = &view->gui_scroll;
                    
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
                    
                    String empty_str = string_zero();
                    
                    char space1[512];
                    String string = make_fixed_width_string(space1);
                    
                    // Time Watcher
                    {
                        string.size = 0;
                        u64 time = system->now_time_stamp();
                        
                        append(&string, "last redraw: ");
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
                                    append(&string, "hold:  ");
                                }
                                else{
                                    append(&string, "press: ");
                                }
                                
                                if (input_event->is_ctrl){
                                    append(&string, "ctrl-");
                                }
                                else{
                                    append(&string, "    -");
                                }
                                
                                if (input_event->is_alt){
                                    append(&string, "alt-");
                                }
                                else{
                                    append(&string, "   -");
                                }
                                
                                if (input_event->is_shift){
                                    append(&string, "shift ");
                                }
                                else{
                                    append(&string, "      ");
                                }
                                
                                if (input_event->key > ' ' && input_event->key <= '~'){
                                    append(&string, make_string(&input_event->key, 1));
                                }
                                else if (input_event->key == ' '){
                                    append(&string, "space");
                                }
                                else if (input_event->key == '\n'){
                                    append(&string, "\\n");
                                }
                                else if (input_event->key == '\t'){
                                    append(&string, "\\t");
                                }
                                else{
                                    String str;
                                    str.str = global_key_name(input_event->key, &str.size);
                                    if (str.str){
                                        str.memory_size = str.size + 1;
                                        append(&string, str);
                                    }
                                    else{
                                        append(&string, "unrecognized!");
                                    }
                                }
                                
                                if (input_event->consumer[0] != 0){
                                    append_padding(&string, ' ', 40);
                                    append(&string, input_event->consumer);
                                }
                                
                                gui_do_text_field(target, string, empty_str);
                            }
                        }break;
                        
                        case DBG_Threads_And_Memory:
                        {
                            b8 threads[4];
                            i32 pending = 0;
                            system->internal_get_thread_states(BACKGROUND_THREADS,
                                                               threads, &pending);
                            
                            string.size = 0;
                            append(&string, "pending jobs: ");
                            append_int_to_str(&string, pending);
                            gui_do_text_field(target, string, empty_str);
                            
                            for (i32 i = 0; i < 4; ++i){
                                string.size = 0;
                                append(&string, "thread ");
                                append_int_to_str(&string, i);
                                append(&string, ": ");
                                
                                if (threads[i]){
                                    append(&string, "running");
                                }
                                else{
                                    append(&string, "waiting");
                                }
                                
                                gui_do_text_field(target, string, empty_str);
                            }
                            
                            Partition *part = &models->mem.part;
                            General_Memory *general = &models->mem.general;
                            
                            string.size = 0;
                            append(&string, "part memory: ");
                            append_int_to_str(&string, part->pos);
                            append(&string, "/");
                            append_int_to_str(&string, part->max);
                            gui_do_text_field(target, string, empty_str);
                            
                            Bubble *bubble, *sentinel;
                            sentinel = &general->sentinel;
                            for (dll_items(bubble, sentinel)){
                                string.size = 0;
                                if (bubble->flags & MEM_BUBBLE_USED){
                                    append(&string, " used: ");
                                }
                                else{
                                    append(&string, " free: ");
                                }
                                
                                append_int_to_str(&string, bubble->size);
                                append_padding(&string, ' ', 40);
                                append(&string, " type: ");
                                append_int_to_str(&string, bubble->type);
                                gui_do_text_field(target, string, empty_str);
                            }
                        }break;
                        
                        case DBG_View_Inspection:
                        {
                            i32 inspecting_id = view->debug_vars.inspecting_view_id;
                            View *views_to_inspect[16];
                            i32 view_count = 0;
                            b32 low_detail = true;
                            
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
                                low_detail = false;
                            }
                            
                            for (i32 i = 0; i < view_count; ++i){
                                View *view_ptr = views_to_inspect[i];
                                
                                string.size = 0;
                                append(&string, "view: ");
                                append_int_to_str(&string, view_ptr->persistent.id + 1);
                                gui_do_text_field(target, string, empty_str);
                                
                                string.size = 0;
                                Editing_File *file = view_ptr->file_data.file;
                                append(&string, " > buffer: ");
                                if (file){
                                    append(&string, file->name.live_name);
                                    gui_do_text_field(target, string, empty_str);
                                    string.size = 0;
                                    append(&string, " >> buffer-slot-id: ");
                                    append_int_to_str(&string, file->id.id);
                                }
                                else{
                                    append(&string, "*NULL*");
                                    gui_do_text_field(target, string, empty_str);
                                }
                                
                                if (low_detail){
                                    string.size = 0;
                                    append(&string, "inspect this");
                                    
                                    id.id[0] = (u64)(view_ptr->persistent.id);
                                    if (gui_do_button(target, id, string)){
                                        view->debug_vars.inspecting_view_id = view_ptr->persistent.id + 1;
                                    }
                                }
                                else{
                                    
                                    gui_show_mouse(target, &string, input.mouse.x, input.mouse.y);
                                    
#define SHOW_GUI_BLANK(n) show_gui_line(target, &string, n, 0, "", 0)
#define SHOW_GUI_LINE(n, str) show_gui_line(target, &string, n, 0, " " str, 0)
#define SHOW_GUI_STRING(n, h, str, mes) show_gui_line(target, &string, n, h, " " str " ", mes)
#define SHOW_GUI_INT(n, h, str, v) show_gui_int(target, &string, n, h, " " str " ", v)
#define SHOW_GUI_INT_INT(n, h, str, v, m) show_gui_int_int(target, &string, n, h, " " str " ", v, m)
#define SHOW_GUI_U64(n, h, str, v) show_gui_u64(target, &string, n, h, " " str " ", v)
#define SHOW_GUI_ID(n, h, str, v) show_gui_id(target, &string, n, h, " " str, v)
#define SHOW_GUI_FLOAT(n, h, str, v) show_gui_float(target, &string, n, h, " " str " ", v)
#define SHOW_GUI_BOOL(n, h, str, v) do { if (v) { show_gui_line(target, &string, n, h, " " str " ", "true"); }\
                                        else { show_gui_line(target, &string, n, h, " " str " ", "false"); } } while(false)
                                    
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
                                    SHOW_GUI_BOOL(2, h_align, "unwrapped lines", view_ptr->file_data.unwrapped_lines);
                                    SHOW_GUI_BOOL(2, h_align, "show whitespace", view_ptr->file_data.show_whitespace);
                                    SHOW_GUI_BOOL(2, h_align, "locked", view_ptr->file_data.file_locked);
                                    SHOW_GUI_INT_INT(2, h_align, "line count",
                                                     view_ptr->file_data.line_count,
                                                     view_ptr->file_data.line_max);
                                    
                                    GUI_Scroll_Vars scroll = *view_ptr->current_scroll;
                                    
                                    SHOW_GUI_BLANK(0);
                                    SHOW_GUI_REGION(1, h_align, "scroll region", view_ptr->scroll_region);
                                    
                                    SHOW_GUI_BLANK(0);
                                    SHOW_GUI_LINE(1, "recent file data");
                                    {
                                        i32 recent_index = 0;
                                        Recent_File_Data *recent = view_ptr->recent + recent_index;
                                        
                                        {
                                            SHOW_GUI_INT   (2, h_align, "recent", recent_index);
                                            SHOW_GUI_U64   (3, h_align, "absolute buffer id", recent->unique_buffer_id);
                                            SHOW_GUI_BLANK (3);
                                            SHOW_GUI_SCROLL(3, h_align, "scroll:", recent->scroll);
                                            SHOW_GUI_BLANK (3);
                                            SHOW_GUI_CURSOR(3, h_align, "cursor:", recent->cursor);
                                            SHOW_GUI_BLANK (3);
                                            SHOW_GUI_INT   (3, h_align, "mark", recent->mark);
                                            SHOW_GUI_FLOAT (3, h_align, "preferred_x", recent->preferred_x);
                                            SHOW_GUI_INT   (3, h_align, "scroll_i", recent->scroll_i);
                                        }
                                    }
                                    
                                    SHOW_GUI_BLANK (0);
                                    SHOW_GUI_SCROLL(1, h_align, "curent scroll:", scroll);
                                    
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
                                    SHOW_GUI_SCROLL (2, h_align, "scroll_updated", view_ptr->gui_target.scroll_updated);
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
    f32 v;
    if (view->showing_ui == VUI_None){
        v = view->recent->scroll.scroll_y;
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
};

internal Input_Process_Result
do_step_file_view(System_Functions *system,
                  View *view, i32_Rect rect, b32 is_active,
                  Input_Summary *user_input,
                  GUI_Scroll_Vars vars, i32_Rect region){
    Input_Process_Result result = {0};
    b32 is_file_scroll = 0;
    
    GUI_Session gui_session = {0};
    GUI_Header *h = 0;
    GUI_Target *target = &view->gui_target;
    GUI_Interpret_Result interpret_result = {0};
    
    vars.target_y = clamp(0, vars.target_y, vars.max_y);
    
    result.vars = vars;
    result.region = region;
    
    target->active = gui_id_zero();
    
    if (target->push.pos > 0){
        gui_session_init(&gui_session, target, rect, view->line_height);
        
        for (h = (GUI_Header*)target->push.base;
             h->type;
             h = NextHeader(h)){
            interpret_result = gui_interpret(target, &gui_session, h,
                                             result.vars, result.region);
            
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
                        result.is_animating = true;
                    }
                    else if (interpret_result.auto_hot){
                        if (!gui_id_eq(target->auto_hot, b->id)){
                            target->auto_hot = b->id;
                            result.is_animating = true;
                        }
                    }
                }break;
            }
            
            if (interpret_result.has_info){
                switch (h->type){
                    case guicom_top_bar: break;
                    
                    case guicom_file:
                    {
                        i32 new_max_y = view_compute_max_target_y(view);
                        
                        view->file_region = gui_session.rect;
                        result.vars.max_y = new_max_y;
                        
                        if (view->reinit_scrolling){
                            view_reinit_scrolling(view);
                            result.is_animating = true;
                        }
                        if (file_step(view, gui_session.rect, user_input, is_active, &result.consumed_l)){
                            result.is_animating = true;
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
                            result.consumed_l = true;
                        }
                    }break;
                    
                    case guicom_fixed_option:
                    case guicom_fixed_option_checkbox:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        
                        if (click_button_input(target, &gui_session, user_input,
                                               b, &result.is_animating)){
                            result.consumed_l = true;
                        }
                        
                        {
                            Key_Event_Data key;
                            Key_Summary *keys = &user_input->keys;
                            
                            void *ptr = (b + 1);
                            String string;
                            char activation_key;
                            
                            i32 i, count;
                            
                            string = gui_read_string(&ptr);
                            activation_key = *(char*)ptr;
                            activation_key = char_to_upper(activation_key);
                            
                            if (activation_key != 0){
                            count = keys->count;
                            for (i = 0; i < count; ++i){
                                key = get_single_key(keys, i);
                                if (char_to_upper(key.character) == activation_key){
                                    target->active = b->id;
                                    result.is_animating = true;
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
                                result.is_animating = true;
                                result.consumed_l = true;
                            }
                        }
                        else if (gui_id_eq(target->hover, id)){
                            target->hover = gui_id_zero();
                        }
                        
                        if (gui_id_eq(target->mouse_hot, id)){
                            v = unlerp(gui_session.scroll_top, (f32)my,
                                       gui_session.scroll_bottom);
                            v = clamp(0.f, v, 1.f);
                            result.vars.target_y = ROUND32(lerp(0.f, v, (f32)result.vars.max_y));
                            
                            gui_activate_scrolling(target);
                            result.is_animating = true;
                        }
                    }
                    // NOTE(allen): NO BREAK HERE!!
                    
                    case guicom_scrollable_invisible:
                    {
                        if (user_input->mouse.wheel != 0){
                            result.vars.target_y += user_input->mouse.wheel*target->delta;
                            
                            result.vars.target_y =
                                clamp(0, result.vars.target_y, result.vars.max_y);
                            gui_activate_scrolling(target);
                            result.is_animating = true;
                        }
                    }break;
                    
                    case guicom_scrollable_top:
                    {
                        GUI_id id = gui_id_scrollbar_top();
                        
                        if (scroll_button_input(target, &gui_session, user_input, id, &result.is_animating)){
                            result.vars.target_y -= clamp_bottom(1, target->delta >> 2);
                            result.vars.target_y = clamp_bottom(0, result.vars.target_y);
                            result.consumed_l = true;
                        }
                    }break;
                    
                    case guicom_scrollable_bottom:
                    {
                        GUI_id id = gui_id_scrollbar_bottom();
                        
                        if (scroll_button_input(target, &gui_session, user_input, id, &result.is_animating)){
                            result.vars.target_y += clamp_bottom(1, target->delta >> 2);
                            result.vars.target_y = clamp_top(result.vars.target_y, result.vars.max_y);
                            result.consumed_l = true;
                        }
                    }break;
                    
                    case guicom_end_scrollable_section:
                    {
                        if (!is_file_scroll){
                            result.vars.max_y = gui_session.suggested_max_y;
                        }
                    }break;
                }
            }
        }
        
        if (!user_input->mouse.l){
            if (!gui_id_is_null(target->mouse_hot)){
                target->mouse_hot = gui_id_zero();
                result.is_animating = true;
            }
        }
        
        {
            GUI_Scroll_Vars scroll_vars = result.vars;
            b32 is_new_target = false;
            if (scroll_vars.target_x != scroll_vars.prev_target_x) is_new_target = true;
            if (scroll_vars.target_y != scroll_vars.prev_target_y) is_new_target = true;
            
            f32 target_x = (f32)scroll_vars.target_x;
            f32 target_y = (f32)scroll_vars.target_y;
            
            if (view->persistent.models->scroll_rule(target_x, target_y,
                                                     &scroll_vars.scroll_x, &scroll_vars.scroll_y,
                                                     (view->persistent.id) + 1, is_new_target, user_input->dt)){
                result.is_animating = true;
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
    
    i32 max_x = rect.x1 - rect.x0;
    i32 max_y = rect.y1 - rect.y0 + line_height;
    
    Assert(file && !file->is_dummy && buffer_good(&file->state.buffer));
    
    b32 tokens_use = 0;
    Cpp_Token_Stack token_stack = {};
    if (file){
        tokens_use = file->state.tokens_complete && (file->state.token_stack.count > 0);
        token_stack = file->state.token_stack;
    }
    
    Partition *part = &models->mem.part;
    
    Temp_Memory temp = begin_temp_memory(part);
    
    partition_align(part, 4);
    i32 max = partition_remaining(part) / sizeof(Buffer_Render_Item);
    Buffer_Render_Item *items = push_array(part, Buffer_Render_Item, max);
    
    i16 font_id = models->global_font.font_id;
    Render_Font *font = get_font_info(models->font_set, font_id)->font;
    float *advance_data = 0;
    if (font) advance_data = font->advance_data;
    
    i32 count = 0;
    Full_Cursor render_cursor = {0};
    Buffer_Render_Options opts = {};
    
    f32 *wraps = view->file_data.line_wrap_y;
    f32 scroll_x = view->recent->scroll.scroll_x;
    f32 scroll_y = view->recent->scroll.scroll_y;
    
    // NOTE(allen): For now we will temporarily adjust scroll_y to try
    // to prevent the view moving around until floating sections are added
    // to the gui system.
    scroll_y += view->widget_height;
    
    {
        render_cursor = buffer_get_start_cursor(&file->state.buffer, wraps, scroll_y,
                                                !view->file_data.unwrapped_lines,
                                                (f32)max_x,
                                                advance_data, (f32)line_height);
        
        view->recent->scroll_i = render_cursor.pos;
        
        buffer_get_render_data(&file->state.buffer, items, max, &count,
                               (f32)rect.x0, (f32)rect.y0,
                               scroll_x, scroll_y, render_cursor,
                               !view->file_data.unwrapped_lines,
                               (f32)max_x, (f32)max_y,
                               advance_data, (f32)line_height,
                               opts);
    }
    
    Assert(count > 0);
    
    i32 cursor_begin = 0, cursor_end = 0;
    u32 cursor_color = 0, at_cursor_color = 0;
    if (view->file_data.show_temp_highlight){
        cursor_begin = view->file_data.temp_highlight.pos;
        cursor_end = view->file_data.temp_highlight_end_pos;
        cursor_color = style->main.highlight_color;
        at_cursor_color = style->main.at_highlight_color;
    }
    else{
        cursor_begin = view->recent->cursor.pos;
        cursor_end = cursor_begin + 1;
        cursor_color = style->main.cursor_color;
        at_cursor_color = style->main.at_cursor_color;
    }
    
    i32 token_i = 0;
    u32 main_color = style->main.default_color;
    u32 special_color = style->main.special_character_color;
    if (tokens_use){
        Cpp_Get_Token_Result result = cpp_get_token(&token_stack, items->index);
        main_color = *style_get_color(style, token_stack.tokens[result.token_index]);
        token_i = result.token_index + 1;
    }
    
    u32 mark_color = style->main.mark_color;
    Buffer_Render_Item *item = items;
    i32 prev_ind = -1;
    u32 highlight_color = 0;
    u32 highlight_this_color = 0;
    
    for (i32 i = 0; i < count; ++i, ++item){
        i32 ind = item->index;
        highlight_this_color = 0;
        if (tokens_use && ind != prev_ind){
            Cpp_Token current_token = token_stack.tokens[token_i-1];
            
            if (token_i < token_stack.count){
                if (ind >= token_stack.tokens[token_i].start){
                    main_color =
                        *style_get_color(style, token_stack.tokens[token_i]);
                    current_token = token_stack.tokens[token_i];
                    ++token_i;
                }
                else if (ind >= current_token.start + current_token.size){
                    main_color = 0xFFFFFFFF;
                }
            }
            
            if (current_token.type == CPP_TOKEN_JUNK &&
                i >= current_token.start && i < current_token.start + current_token.size){
                highlight_color = style->main.highlight_junk_color;
            }
            else{
                highlight_color = 0;
            }
        }
        
        u32 char_color = main_color;
        if (item->flags & BRFlag_Special_Character) char_color = special_color;
        
        f32_Rect char_rect = f32R(item->x0, item->y0,
                                  item->x1,  item->y1);
        
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
        
        if (file->state.paste_effect.tick_down > 0 &&
            file->state.paste_effect.start <= ind &&
            ind < file->state.paste_effect.end){
            fade_color = file->state.paste_effect.color;
            fade_amount = (f32)(file->state.paste_effect.tick_down) / file->state.paste_effect.tick_max;
        }
        
        char_color = color_blend(char_color, fade_amount, fade_color);
        
        if (ind == view->recent->mark && prev_ind != ind){
            draw_rectangle_outline(target, char_rect, mark_color);
        }
        if (item->glyphid != 0){
            font_draw_glyph(target, font_id, (u8)item->glyphid,
                            item->x0, item->y0, char_color);
        }
        prev_ind = ind;
    }
    
    end_temp_memory(temp);
    
    return(0);
}

internal void
draw_text_field(Render_Target *target, View *view, i32_Rect rect, String p, String t){
    Models *models = view->persistent.models;
    Style *style = main_style(models);
    
    u32 back_color = style->main.margin_color;
    u32 text1_color = style->main.default_color;
    u32 text2_color = style->main.file_info_style.pop1_color;
    
    i32 x = rect.x0;
    i32 y = rect.y0 + 2;
    
    i16 font_id = models->global_font.font_id;
    
    if (target){
        draw_rectangle(target, rect, back_color);
        x = CEIL32(draw_string(target, font_id, p, x, y, text2_color));
        draw_string(target, font_id, t, x, y, text1_color);
	}
}

internal void
draw_text_with_cursor(Render_Target *target, View *view, i32_Rect rect, String s, i32 pos){
    Models *models = view->persistent.models;
    Style *style = main_style(models);
    
    u32 back_color = style->main.margin_color;
    u32 text_color = style->main.default_color;
    u32 cursor_color = style->main.cursor_color;
    u32 at_cursor_color = style->main.at_cursor_color;
    
    f32 x = (f32)rect.x0;
    i32 y = rect.y0 + 2;
    
    i16 font_id = models->global_font.font_id;
    
    if (target){
        draw_rectangle(target, rect, back_color);
        
        if (pos >= 0 && pos <  s.size){
            String part1, part2, part3;
            i32_Rect cursor_rect;
            Render_Font *font = get_font_info(models->font_set, font_id)->font;
            
            part1 = substr(s, 0, pos);
            part2 = substr(s, pos, 1);
            part3 = substr(s, pos+1, s.size-pos-1);
            
            
            x = draw_string(target, font_id, part1, FLOOR32(x), y, text_color);
            
            cursor_rect.x0 = FLOOR32(x);
            cursor_rect.x1 = FLOOR32(x) + CEIL32(font->advance_data[s.str[pos]]);
            cursor_rect.y0 = y;
            cursor_rect.y1 = y + view->line_height;
            draw_rectangle(target, cursor_rect, cursor_color);
            x = draw_string(target, font_id, part2, FLOOR32(x), y, at_cursor_color);
            
            draw_string(target, font_id, part3, FLOOR32(x), y, text_color);
        }
        else{
            draw_string(target, font_id, s, FLOOR32(x), y, text_color);
        }
	}
}

internal void
draw_file_bar(Render_Target *target, View *view, Editing_File *file, i32_Rect rect){
    File_Bar bar;
    Models *models = view->persistent.models;
    Style_Font *font = &models->global_font;
    Style *style = main_style(models);
    Interactive_Style bar_style = style->main.file_info_style;

    u32 back_color = bar_style.bar_color;
    u32 base_color = bar_style.base_color;
    u32 pop1_color = bar_style.pop1_color;
    u32 pop2_color = bar_style.pop2_color;

    bar.rect = rect;

    if (target){
        bar.font_id = font->font_id;
        bar.pos_x = (f32)bar.rect.x0;
        bar.pos_y = (f32)bar.rect.y0;
        bar.text_shift_y = 2;
        bar.text_shift_x = 0;

        draw_rectangle(target, bar.rect, back_color);    
        if (!file){
            intbar_draw_string(target, &bar, make_lit_string("*NULL*"), base_color);
        }
        else{
            intbar_draw_string(target, &bar, file->name.live_name, base_color);
            intbar_draw_string(target, &bar, make_lit_string(" -"), base_color);
            
            if (file->is_loading){
                intbar_draw_string(target, &bar, make_lit_string(" loading"), base_color);
            }
            else{
                char line_number_space[30];
                String line_number = make_fixed_width_string(line_number_space);
                append(&line_number, " L#");
                append_int_to_str(&line_number, view->recent->cursor.line);
                append(&line_number, " C#");
                append_int_to_str(&line_number, view->recent->cursor.character);

                intbar_draw_string(target, &bar, line_number, base_color);

                intbar_draw_string(target, &bar, make_lit_string(" -"), base_color);

                if (file->settings.dos_write_mode){
                    intbar_draw_string(target, &bar, make_lit_string(" dos"), base_color);
                }
                else{
                    intbar_draw_string(target, &bar, make_lit_string(" nix"), base_color);
                }

                if (file->state.still_lexing){
                    intbar_draw_string(target, &bar, make_lit_string(" parsing"), pop1_color);
                }

                if (!file->settings.unimportant){
                    switch (buffer_get_sync(file)){
                        case SYNC_BEHIND_OS:
                        {
                            persist String out_of_sync = make_lit_string(" !");
                            intbar_draw_string(target, &bar, out_of_sync, pop2_color);
                        }break;

                        case SYNC_UNSAVED:
                        {
                            persist String out_of_sync = make_lit_string(" *");
                            intbar_draw_string(target, &bar, out_of_sync, pop2_color);
                        }break;
                    }
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
    i32_Rect rect, GUI_id id, u32 fore, u32 back, String text){
    Models *models = view->persistent.models;
    
    i32 active_level = gui_active_level(gui_target, id);
    i16 font_id = models->global_font.font_id;
        
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
    
    u32 margin = get_margin_color(active_level, style);
    u32 back = style->main.back_color;
    u32 text_color = style->main.default_color;

    draw_rectangle(target, rect, back);
    draw_rectangle_outline(target, rect, margin);
    draw_string(target, font_id, text, rect.x0, rect.y0 + 1, text_color);
}

internal void
draw_fat_option_block(GUI_Target *gui_target, Render_Target *target, View *view, i32_Rect rect, GUI_id id,
    String text, String pop, i8 checkbox = -1){
    Models *models = view->persistent.models;
    Style *style = main_style(models);
    
    i32 active_level = gui_active_level(gui_target, id);
    i16 font_id = models->global_font.font_id;
    
    i32_Rect inner = get_inner_rect(rect, 3);
    
    u32 margin = get_margin_color(active_level, style);
    u32 back = style->main.back_color;
    u32 text_color = style->main.default_color;
    u32 pop_color = style->main.special_character_color;
    
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
    
    x = CEIL32(draw_string(target, font_id, text, x, y, text_color));
    draw_string(target, font_id, pop, x, y, pop_color);
}

internal void
draw_button(GUI_Target *gui_target, Render_Target *target, View *view, i32_Rect rect, GUI_id id, String text){
    Models *models = view->persistent.models;
    Style *style = main_style(models);
    
    i32 active_level = gui_active_level(gui_target, id);
    i16 font_id = models->global_font.font_id;
    
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
draw_style_preview(GUI_Target *gui_target, Render_Target *target, View *view, i32_Rect rect, GUI_id id, Style *style){
    Models *models = view->persistent.models;
    
    i32 active_level = gui_active_level(gui_target, id);
    i16 font_id = models->global_font.font_id;
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
    x = CEIL32(draw_string(target, font_id, style->name.str, x, y, text_color));
    i32 font_x = (i32)(inner.x1 - font_string_width(target, font_id, info->name.str));
    if (font_x > x + 10){
        draw_string(target, font_id, info->name.str, font_x, y, text_color);
    }

    x = inner.x0;
    y += info->height;
    x = CEIL32(draw_string(target, font_id, "if", x, y, keyword_color));
    x = CEIL32(draw_string(target, font_id, "(x < ", x, y, text_color));
    x = CEIL32(draw_string(target, font_id, "0", x, y, int_constant_color));
    x = CEIL32(draw_string(target, font_id, ") { x = ", x, y, text_color));
    x = CEIL32(draw_string(target, font_id, "0", x, y, int_constant_color));
    x = CEIL32(draw_string(target, font_id, "; } ", x, y, text_color));
    x = CEIL32(draw_string(target, font_id, "// comment", x, y, comment_color));
    
    x = inner.x0;
    y += info->height;
    draw_string(target, font_id, "[] () {}; * -> +-/ <>= ! && || % ^", x, y, text_color);
}

internal i32
do_render_file_view(System_Functions *system, View *view,
                    View *active, i32_Rect rect, b32 is_active,
                    Render_Target *target, Input_Summary *user_input){
    
    Editing_File *file = view->file_data.file;
    i32 result = 0;
    
    GUI_Session gui_session = {0};
    GUI_Header *h;
    GUI_Target *gui_target = &view->gui_target;
    GUI_Interpret_Result interpret_result = {0};
    
    f32 v;
    
    if (gui_target->push.pos > 0){
        gui_session_init(&gui_session, gui_target, rect, view->line_height);
        
        v = view_get_scroll_y(view);
        
        i32_Rect clip_rect = rect;
        draw_push_clip(target, clip_rect);
        
        for (h = (GUI_Header*)gui_target->push.base;
             h->type;
             h = NextHeader(h)){
            interpret_result = gui_interpret(gui_target, &gui_session, h,
                                             *view->current_scroll,
                                             view->scroll_region);
            
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
                        if (view->reinit_scrolling){
                            view_reinit_scrolling(view);
                        }
                        if (file && file_is_ready(file)){
                            result = draw_file_loaded(view, gui_session.rect, is_active, target);
                        }
                    }break;
                    
                    case guicom_text_field:
                    {
                        void *ptr = (h+1);
                        String p = gui_read_string(&ptr);
                        String t = gui_read_string(&ptr);
                        draw_text_field(target, view, gui_session.rect, p, t);
                    }break;
                    
                    case guicom_text_with_cursor:
                    {
                        void *ptr = (h+1);
                        String s = gui_read_string(&ptr);
                        i32 pos = gui_read_integer(&ptr);
                        
                        draw_text_with_cursor(target, view, gui_session.rect, s, pos);
                    }break;
                    
                    case guicom_color_button:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        u32 fore = (u32)gui_read_integer(&ptr);
                        u32 back = (u32)gui_read_integer(&ptr);
                        String t = gui_read_string(&ptr);
                        
                        draw_color_button(gui_target, target, view, gui_session.rect, b->id, fore, back, t);
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
                            append(&f, system->slash);
                        }
                        
                        draw_fat_option_block(gui_target, target, view, gui_session.rect, b->id, f, m);
                    }break;
                    
                    case guicom_style_preview:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        i32 style_index = *(i32*)(b + 1);
                        Style *style = get_style(view->persistent.models, style_index);
                        
                        draw_style_preview(gui_target, target, view, gui_session.rect, b->id, style);
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
                        
                        draw_fat_option_block(gui_target, target, view, gui_session.rect, b->id, f, m, status);
                    }break;
                    
                    case guicom_button:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        String t = gui_read_string(&ptr);
                        
                        draw_button(gui_target, target, view, gui_session.rect, b->id, t);
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
    if (view->file_data.line_wrap_y){
        general_memory_free(general, view->file_data.line_wrap_y);
        view->file_data.line_wrap_y = 0;
    }
    general_memory_free(general, view->gui_mem);
    view->gui_mem = 0;
}

struct Search_Range{
    Buffer_Type *buffer;
    i32 start, size;
};

struct Search_Set{
    Search_Range *ranges;
    i32 count, max;
};

struct Search_Iter{
    String word;
    i32 pos;
    i32 i;
};

struct Search_Match{
    Buffer_Type *buffer;
    i32 start, end;
    b32 found_match;
};

internal void
search_iter_init(General_Memory *general, Search_Iter *iter, i32 size){
    i32 str_max;

    if (iter->word.str == 0){
        str_max = size*2;
        iter->word.str = (char*)general_memory_allocate(general, str_max, 0);
        iter->word.memory_size = str_max;
    }
    else if (iter->word.memory_size < size){
        str_max = size*2;
        iter->word.str = (char*)general_memory_reallocate_nocopy(general, iter->word.str, str_max, 0);
        iter->word.memory_size = str_max;
    }

    iter->i = 0;
    iter->pos = 0;
}

internal void
search_set_init(General_Memory *general, Search_Set *set, i32 set_count){
    i32 max;

    if (set->ranges == 0){
        max = set_count*2;
        set->ranges = (Search_Range*)general_memory_allocate(general, sizeof(Search_Range)*max, 0);
        set->max = max;
    }
    else if (set->max < set_count){
        max = set_count*2;
        set->ranges = (Search_Range*)general_memory_reallocate_nocopy(
            general, set->ranges, sizeof(Search_Range)*max, 0);
        set->max = max;
    }

    set->count = set_count;
}

internal void
search_hits_table_alloc(General_Memory *general, Table *hits, i32 table_size){
    void *mem;
    i32 mem_size;
    
    mem_size = table_required_mem_size(table_size, sizeof(Offset_String));
    if (hits->hash_array == 0){
        mem = general_memory_allocate(general, mem_size, 0);
    }
    else{
        mem = general_memory_reallocate_nocopy(general, hits->hash_array, mem_size, 0);
    }
    table_init_memory(hits, mem, table_size, sizeof(Offset_String));
}

internal void
search_hits_init(General_Memory *general, Table *hits, String_Space *str, i32 table_size, i32 str_size){
    void *mem;
    i32 mem_size;

    if (hits->hash_array == 0){
        search_hits_table_alloc(general, hits, table_size);
    }
    else if (hits->max < table_size){
        mem_size = table_required_mem_size(table_size, sizeof(Offset_String));
        mem = general_memory_reallocate_nocopy(general, hits->hash_array, mem_size, 0);
        table_init_memory(hits, mem, table_size, sizeof(Offset_String));
    }

    if (str->space == 0){
        str->space = (char*)general_memory_allocate(general, str_size, 0);
        str->max = str_size;
    }
    else if (str->max < str_size){
        str->space = (char*)general_memory_reallocate_nocopy(general, str->space, str_size, 0);
        str->max = str_size;
    }

    str->pos = str->new_pos = 0;
    table_clear(hits);
}

internal b32
search_hit_add(General_Memory *general, Table *hits, String_Space *space, char *str, i32 len){
    b32 result;
    i32 new_size;
    Offset_String ostring;
    Table new_hits;

    Assert(len != 0);

    ostring = strspace_append(space, str, len);
    if (ostring.size == 0){
        new_size = Max(space->max*2, space->max + len);
        space->space = (char*)general_memory_reallocate(general, space->space, space->new_pos, new_size, 0);
        ostring = strspace_append(space, str, len);
    }

    Assert(ostring.size != 0);

    if (table_at_capacity(hits)){
        search_hits_table_alloc(general, &new_hits, hits->max*2);
        table_clear(&new_hits);
        table_rehash(hits, &new_hits, space->space, tbl_offset_string_hash, tbl_offset_string_compare);
        general_memory_free(general, hits->hash_array);
        *hits = new_hits;
    }

    if (!table_add(hits, &ostring, space->space, tbl_offset_string_hash, tbl_offset_string_compare)){
        result = 1;
        strspace_keep_prev(space);
    }
    else{
        result = 0;
        strspace_discard_prev(space);
    }

    return(result);
}

internal Search_Match
search_next_match(Partition *part, Search_Set *set, Search_Iter *iter_){
    Search_Match result = {};
    Search_Iter iter = *iter_;
    Search_Range *range;
    Temp_Memory temp;
    char *spare;
    i32 start_pos, end_pos, count;

    temp = begin_temp_memory(part);
    spare = push_array(part, char, iter.word.size);

    count = set->count;
    for (; iter.i < count;){
        range = set->ranges + iter.i;

        end_pos = range->start + range->size;

        if (iter.pos + iter.word.size < end_pos){
            start_pos = Max(iter.pos, range->start);
            result.start = buffer_find_string(range->buffer, start_pos, end_pos, iter.word.str, iter.word.size, spare);

            if (result.start < end_pos){
                iter.pos = result.start + 1;
                if (result.start == 0 || !char_is_alpha_numeric(buffer_get_char(range->buffer, result.start - 1))){
                    result.end = buffer_seek_word_right_assume_on_word(range->buffer, result.start);
                    if (result.end < end_pos){
                        result.found_match = 1;
                        result.buffer = range->buffer;
                        iter.pos = result.end;
                        break;
                    }
                }
            }
            else{
                ++iter.i, iter.pos = 0;
            }
        }
        else{
            ++iter.i, iter.pos = 0;
        }
    }
    end_temp_memory(temp);

    *iter_ = iter;

    return(result);
}

inline void
view_change_size(General_Memory *general, View *view){
    if (view->file_data.file){
        view_measure_wraps(general, view);
        view->recent->cursor = view_compute_cursor_from_pos(view, view->recent->cursor.pos);
    }
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
    result.view->current_scroll = &result.view->recent->scroll;

    init_query_set(&result.view->query_set);

    {
        i32 gui_mem_size = Kbytes(32);
        void *gui_mem = general_memory_allocate(&models->mem.general, gui_mem_size + 8, 0);
        result.view->gui_mem = gui_mem;
        gui_mem = advance_to_alignment(gui_mem);
        result.view->gui_target.push = make_part(gui_mem, gui_mem_size);
    }
    
    return(result);
}

inline void
live_set_free_view(System_Functions *system, Live_Views *live_set, View *view){
    Assert(live_set->count > 0);
    --live_set->count;
    file_view_free_buffers(view);
    dll_insert(&live_set->free_sentinel, view);
    view->in_use = 0;
}

// BOTTOM

