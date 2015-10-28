/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application layer for project codename "4ed"
 *
 */

// TOP

// App Structs

enum App_State{
    APP_STATE_EDIT,
    APP_STATE_RESIZING,
    // never below this
    APP_STATE_COUNT
};

struct App_State_Resizing{
    Panel_Divider *divider;
    i32 min, max;
};

struct App_Vars{
    Mem_Options mem;
    Command_Map map_top;
    Command_Map map_file;
    Command_Map map_ui;
#if FRED_INTERNAL
    Command_Map map_debug;
#endif
    Command_Map *user_maps;
    i32 *map_id_table;
    i32 user_map_count;
    
    Custom_Command_Function *hooks[hook_type_count];
    
    Font_Set fonts;
    
    Style style;
    Style_Library styles;
    u32 *palette;
    i32 palette_size;
    
    Editing_Layout layout;
    Live_Views live_set;
    Working_Set working_set;

    char hot_dir_base_[256];
    String hot_dir_base;
    Hot_Directory hot_directory;
    
    char query_[256];
    char dest_[256];
    
    Delay delay;

    String mini_str;
    u8 mini_buffer[512];
    
    App_State state;
    App_State_Resizing resizing;
    Panel *prev_mouse_panel;
};

internal i32
app_get_or_add_map_index(App_Vars *vars, i32 mapid){
    i32 result;
    i32 user_map_count = vars->user_map_count;
    i32 *map_id_table = vars->map_id_table;
    for (result = 0; result < user_map_count; ++result){
        if (map_id_table[result] == mapid) break;
        if (map_id_table[result] == 0){
            map_id_table[result] = mapid;
            break;
        }
    }
    return result;
}

internal i32
app_get_map_index(App_Vars *vars, i32 mapid){
    i32 result;
    i32 user_map_count = vars->user_map_count;
    i32 *map_id_table = vars->map_id_table;
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
app_get_map(App_Vars *vars, i32 mapid){
    Command_Map *map = 0;
    if (mapid >= mapid_user_custom)
        map = vars->user_maps + mapid - mapid_user_custom;
    else if (mapid == mapid_global)
        map = &vars->map_top;
    else if (mapid == mapid_file)
        map = &vars->map_file;
    return map;
}

// Commands

globalvar Application_Links app_links;

#define USE_MEM(n) Mem_Options *n = command->mem
#define USE_PANEL(n) Panel *n = command->panel
#define USE_VIEW(n) View *n = command->view
#define USE_WORKING_SET(n) Working_Set *n = command->working_set
#define USE_LAYOUT(n) Editing_Layout *n = command->layout
#define USE_LIVE_SET(n) Live_Views *live_set = command->live_set
#define USE_STYLE(n) Style *n = command->style
#define USE_DELAY(n) Delay *n = command->delay
#define USE_VARS(n) App_Vars *n = command->vars

#define REQ_VIEW(n) View *n = command->view; if (!n) return
#define REQ_FILE_VIEW(n) File_View *n = view_to_file_view(command->view); if (!n) return
#define REQ_FILE(n,v) Editing_File *n = (v)->file; if (!n || !n->buffer.data || n->is_dummy) return
#define REQ_COLOR_VIEW(n) Color_View *n = view_to_color_view(command->view); if (!n) return
#define REQ_DBG_VIEW(n) Debug_View *n = view_to_debug_view(command->view); if (!n) return

#define COMMAND_DECL(n) internal void command_##n(Command_Data *command, Command_Binding binding)
#define COMPOSE_DECL(n) internal void n(Command_Data *command, Command_Binding binding)

struct Command_Data{
    Mem_Options *mem;
    Panel *panel;
    View *view;
    Working_Set *working_set;
    Editing_Layout *layout;
    Live_Views *live_set;
    Style *style;
    Delay *delay;
    App_Vars *vars;
    
    i32 screen_width, screen_height;
    Key_Single key;
    
    Partition part;
};

struct Command_Parameter{
    i32 type;
    union{
        struct{
            Dynamic param;
            Dynamic value;
        } param;
        struct{
            i32 len;
            char *str;
        } inline_string;
    };
};

inline Command_Parameter*
param_next(Command_Parameter *param, Command_Parameter *end){
    Command_Parameter *result = param;
    if (result->type == 0){
        ++result;
    }
    while (result->type != 0 && result < end){
        i32 len = result->inline_string.len;
        len += sizeof(*result) - 1;
        len -= (len % sizeof(*result));
        result = (Command_Parameter*)((char*)result + len + sizeof(*result));
    }
    return result;
}

inline Command_Parameter*
param_stack_first(Partition *part, Command_Parameter *end){
    Command_Parameter *result = (Command_Parameter*)part->base;
    if (result->type != 0) result = param_next(result, end);
    return result;
}

inline Command_Parameter*
param_stack_end(Partition *part){
    return (Command_Parameter*)((char*)part->base + part->pos);
}

COMMAND_DECL(null){
    AllowLocal(command);
}

COMMAND_DECL(write_character){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    u8 character = (u8)command->key.key.character;
    char str_space[2];
    String string = make_string(str_space, 2);
    str_space[0] = character;
    string.size = 1;
    
    i32 pos;
    pos = view->cursor.pos;
    i32 next_cursor_pos = view->cursor.pos + string.size;
    view_replace_range(mem, view, layout, pos, pos, (u8*)string.str, string.size, next_cursor_pos);
    view_cursor_move(view, next_cursor_pos);
    if (view->mark >= pos) view->mark += string.size;
    view->file->cursor_pos = view->cursor.pos;
}

COMMAND_DECL(seek_whitespace_right){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_whitespace_right(&file->buffer, view->cursor.pos);
    
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_whitespace_left){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_whitespace_left(&file->buffer, view->cursor.pos);
    
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_whitespace_up){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_whitespace_up(&file->buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_whitespace_down){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
        
    i32 pos = buffer_seek_whitespace_down(&file->buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

internal i32
seek_token_left(Cpp_Token_Stack *tokens, i32 pos){
    Cpp_Get_Token_Result get = cpp_get_token(tokens, pos);
    if (get.token_index == -1){
        get.token_index = 0;
    }
    
    Cpp_Token *token = tokens->tokens + get.token_index;
    if (token->start == pos && get.token_index > 0){
        --token;
    }
    
    return token->start;
}

internal i32
seek_token_right(Cpp_Token_Stack *tokens, i32 pos){
    Cpp_Get_Token_Result get = cpp_get_token(tokens, pos);
    if (get.in_whitespace){
        ++get.token_index;
    }
    if (get.token_index >= tokens->count){
        get.token_index = tokens->count-1;
    }
    
    Cpp_Token *token = tokens->tokens + get.token_index;
    return token->start + token->size;
}

COMMAND_DECL(seek_token_left){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    if (file->tokens_complete){
        i32 pos = seek_token_left(&file->token_stack, view->cursor.pos);
        view_cursor_move(view, pos);
    }
}

COMMAND_DECL(seek_token_right){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    if (file->tokens_complete){
        i32 pos = seek_token_right(&file->token_stack, view->cursor.pos);
        view_cursor_move(view, pos);
    }
}

COMMAND_DECL(seek_white_or_token_right){
#if BUFFER_EXPERIMENT_SCALPEL
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 token_pos, white_pos;
    token_pos = file->buffer.size;
    if (file->tokens_complete){
        token_pos = seek_token_right(&file->token_stack, view->cursor.pos);
    }
    white_pos = buffer_seek_whitespace_right(&file->buffer, view->cursor.pos);
    view_cursor_move(view, Min(token_pos, white_pos));
#endif
}

COMMAND_DECL(seek_white_or_token_left){
#if BUFFER_EXPERIMENT_SCALPEL
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 token_pos, white_pos;
    token_pos = file->buffer.size;
    if (file->tokens_complete){
        token_pos = seek_token_left(&file->token_stack, view->cursor.pos);
    }
    white_pos = buffer_seek_whitespace_left(&file->buffer, view->cursor.pos);
    view_cursor_move(view, Max(token_pos, white_pos));
#endif
}

COMMAND_DECL(seek_alphanumeric_right){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_alphanumeric_right(&file->buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_alphanumeric_left){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_alphanumeric_left(&file->buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_alphanumeric_or_camel_right){
#if 0
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    char *data = file->buffer.data;
    
    i32 an_pos, camel_pos;
    an_pos = seek_alphanumeric_right(
        (u8*)file->buffer.data, file->buffer.size, view->cursor.pos);
    
    u8 curr_char;
    u8 prev_char = data[view->cursor.pos + 1];
    for (camel_pos = view->cursor.pos + 2; camel_pos < an_pos; ++camel_pos){
        curr_char = data[camel_pos];
        if (char_is_upper(curr_char) && char_is_lower(prev_char)){
            break;
        }
        prev_char = curr_char;
    }
    
    view_cursor_move(view, camel_pos);
#endif
}

COMMAND_DECL(seek_alphanumeric_or_camel_left){
#if 0
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    u8 *data = (u8*)file->buffer.data;
    
    i32 an_pos, camel_pos;
    an_pos = buffer_seek_alphanumeric_left(
        data, view->cursor.pos);
    
    char curr_char;
    char prev_char = data[view->cursor.pos];
    for (camel_pos = view->cursor.pos - 1; camel_pos > an_pos; --camel_pos){
        curr_char = data[camel_pos];
        if (char_is_upper(curr_char) && char_is_lower(prev_char)){
            break;
        }
        prev_char = curr_char;
    }
    
    view_cursor_move(view, camel_pos);
#endif
}

COMMAND_DECL(search){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_VARS(vars);
    
    view_set_widget(view, FWIDG_SEARCH);
    view->isearch.str = vars->mini_str;
    view->isearch.reverse = 0;
    view->isearch.pos = view->cursor.pos;
}

COMMAND_DECL(rsearch){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_VARS(vars);
    
    view_set_widget(view, FWIDG_SEARCH);
    view->isearch.str = vars->mini_str;
    view->isearch.reverse = 1;
    view->isearch.pos = view->cursor.pos;
}

COMMAND_DECL(goto_line){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_VARS(vars);
    
    view_set_widget(view, FWIDG_GOTO_LINE);
    view->isearch.str = vars->mini_str;
    view->isearch.reverse = 1;
    view->isearch.pos = view->cursor.pos;
}

COMMAND_DECL(set_mark){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    view->mark = (i32)view->cursor.pos;
}

COMMAND_DECL(copy){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_WORKING_SET(working_set);
    USE_MEM(mem);
    
    Range range = get_range(view->cursor.pos, view->mark);
    if (range.start < range.end){
        u8 *data = (u8*)file->buffer.data;
        clipboard_copy(&mem->general, working_set, data, range);
    }
}

COMMAND_DECL(cut){
#if BUFFER_EXPERIMENT_SCALPEL
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_WORKING_SET(working_set);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    Range range = get_range(view->cursor.pos, view->mark);
    if (range.start < range.end){
        u8 *data = (u8*)file->buffer.data;
        
        i32 next_cursor_pos = range.start;
        clipboard_copy(&mem->general, working_set, data, range);
        view_replace_range(mem, view, layout, range.start, range.end, 0, 0, next_cursor_pos);
        
        view->mark = range.start;
        view_measure_wraps(&mem->general, view);
        view_cursor_move(view, next_cursor_pos);
    }
#endif
}

COMMAND_DECL(paste){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_WORKING_SET(working_set);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    if (working_set->clipboard_size > 0){
        view->next_mode.rewrite = 1;
        
        String *src = working_set_clipboard_head(working_set);
        i32 pos_left = view->cursor.pos;
        
        i32 next_cursor_pos = pos_left+src->size;
        view_replace_range(mem, view, layout, pos_left, pos_left, (u8*)src->str, src->size, next_cursor_pos);
        
        view_cursor_move(view, next_cursor_pos);
        view->mark = pos_left;
        
        Editing_Layout *layout = command->layout;
        Panel *panels = layout->panels;
        i32 panel_count = layout->panel_count;
        for (i32 i = 0; i < panel_count; ++i){
            Panel *current_panel = panels + i;
            File_View *current_view = view_to_file_view(current_panel->view);
            
            if (current_view && current_view->file == file){
                view_post_paste_effect(current_view, 20, pos_left, src->size,
                                       current_view->style->main.paste_color);
            }
        }
    }
}

COMMAND_DECL(paste_next){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_WORKING_SET(working_set);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    if (working_set->clipboard_size > 0 && view->mode.rewrite){
        view->next_mode.rewrite = 1;
        
        Range range = get_range(view->mark, view->cursor.pos);
        String *src = working_set_clipboard_roll_down(working_set);
        i32 next_cursor_pos = range.start+src->size;
        view_replace_range(mem, view, layout, range.start, range.end,
                           (u8*)src->str, src->size, next_cursor_pos);
        
        view_cursor_move(view, next_cursor_pos);
        view->mark = range.start;
        
        Editing_Layout *layout = command->layout;
        Panel *panels = layout->panels;
        i32 panel_count = layout->panel_count;
        for (i32 i = 0; i < panel_count; ++i){
            Panel *current_panel = panels + i;
            File_View *current_view = view_to_file_view(current_panel->view);
                
            if (current_view && current_view->file == file){
                view_post_paste_effect(current_view, 20, range.start, src->size,
                                       current_view->style->main.paste_color);
            }
        }
    }
    else{
        command_paste(command, binding);
    }
}

COMMAND_DECL(delete_chunk){
#if BUFFER_EXPERIMENT_SCALPEL
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    Range range = get_range(view->cursor.pos, view->mark);
    if (range.start < range.end){
        i32 next_cursor_pos = range.start;
        view_replace_range(mem, view, layout, range.start, range.end, 0, 0, next_cursor_pos);
        view_measure_wraps(&mem->general, view);
        view_cursor_move(view, next_cursor_pos);
        view->mark = range.start;
    }
#endif
}

COMMAND_DECL(timeline_scrub){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    view_set_widget(view, FWIDG_TIMELINES);
    view->widget.timeline.undo_line = 1;
    view->widget.timeline.history_line = 1;
}

COMMAND_DECL(undo){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    view_undo(mem, layout, view);
}

COMMAND_DECL(redo){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    view_redo(mem, layout, view);
}

COMMAND_DECL(increase_rewind_speed){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);

    i32 rewind_speed = ROUND32(view->rewind_speed * 4.f);
    if (rewind_speed > 1) rewind_speed >>= 1;
    else if (rewind_speed == 1) rewind_speed = 0;
    else if (rewind_speed == 0) rewind_speed = -1;
    else rewind_speed *= 2;
    
    view->rewind_speed = rewind_speed * 0.25f;
}

COMMAND_DECL(increase_fastforward_speed){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    
    i32 neg_rewind_speed = -ROUND32(view->rewind_speed * 4.f);
    if (neg_rewind_speed > 1) neg_rewind_speed >>= 1;
    else if (neg_rewind_speed == 1) neg_rewind_speed = 0;
    else if (neg_rewind_speed == 0) neg_rewind_speed = -1;
    else neg_rewind_speed *= 2;
    
    view->rewind_speed = -neg_rewind_speed * 0.25f;
}

COMMAND_DECL(stop_rewind_fastforward){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    
    view->rewind_speed = 0;
}

COMMAND_DECL(history_backward){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    view_history_step(mem, layout, view, hist_backward);
}

COMMAND_DECL(history_forward){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    view_history_step(mem, layout, view, hist_forward);
}

COMMAND_DECL(interactive_new){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_MEM(mem);
    USE_WORKING_SET(working_set);
    USE_STYLE(style);
    USE_DELAY(delay);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Interactive_View *int_view =
        interactive_view_init(new_view, &vars->hot_directory, style,
                              working_set, delay);
    int_view->interaction = INTV_SYS_FILE_LIST;
    int_view->action = INTV_NEW;
    copy(&int_view->query, "New: ");
}

internal File_View*
app_open_file(App_Vars *vars, General_Memory *general, Panel *panel,
              Working_Set *working_set, String *string, Style *style,
              Live_Views *live_set, Command_Data *command_data){
    File_View *result = 0;
    Editing_File *target_file = 0;
    bool32 created_file = 0;
    
    target_file = working_set_contains(working_set, *string);
    if (!target_file){
        Get_File_Result file = working_set_get_available_file(working_set);
        if (file.file){
            file_get_dummy(file.file);
            created_file = file_create(general, file.file, (u8*)string->str, style->font);
            table_add(&working_set->table, file.file->source_path, file.index);
            if (created_file){
                target_file = file.file;
            }
        }
    }
    
    if (target_file){
        View *new_view = live_set_alloc_view(live_set, &vars->mem);
        
        view_replace_major(new_view, panel, live_set);
        
        File_View *file_view = file_view_init(new_view, &vars->delay, &vars->layout);
        result = file_view;
        
        View *old_view = command_data->view;
        command_data->view = new_view;
        
        Partition old_part = command_data->part;
        Temp_Memory temp = begin_temp_memory(&vars->mem.part);
        command_data->part = partition_sub_part(&vars->mem.part, 16 << 10);
        
        view_set_file(file_view, target_file, style,
                      vars->hooks[hook_open_file], command_data, app_links);
        
        command_data->part = old_part;
        end_temp_memory(temp);
        
        command_data->view = old_view;
        
        new_view->map = app_get_map(vars, target_file->base_map_id);

#if BUFFER_EXPERIMENT_SCALPEL
        if (created_file && target_file->tokens_exist)
            file_first_lex_parallel(general, target_file);
#endif
    }
    
    return result;
}


COMMAND_DECL(interactive_open){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_MEM(mem);
    USE_WORKING_SET(working_set);
    USE_STYLE(style);
    USE_DELAY(delay);
    
    char *filename = 0;
    int filename_len = 0;
    
    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        if (param->param.param.type == dynamic_type_int &&
            param->param.param.int_value == par_name &&
            param->param.value.type == dynamic_type_string){
            filename = param->param.value.str_value;
            filename_len = param->param.value.str_len;
            break;
        }
    }
    
    bool32 made_file = 0;
    
    if (filename){
        String string = make_string(filename, filename_len);
        if (app_open_file(vars, &mem->general, panel, working_set,
                          &string, style, live_set, command)) made_file = 1;
    }
    
    if (!made_file){
        View *new_view = live_set_alloc_view(live_set, mem);
        view_replace_minor(new_view, panel, live_set);
        
        new_view->map = &vars->map_ui;
        Interactive_View *int_view =
            interactive_view_init(new_view, &vars->hot_directory, style,
                                  working_set, delay);
        int_view->interaction = INTV_SYS_FILE_LIST;
        int_view->action = INTV_OPEN;
        copy(&int_view->query, "Open: ");
    }
}

// TODO(allen): Improvements to reopen
// - Preserve existing token stack
// - Keep current version open and do some sort of diff to keep
//    the cursor position correct
COMMAND_DECL(reopen){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_STYLE(style);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    USE_VARS(vars);
    
    Editing_File temp_file;
    if (file_create(&mem->general, &temp_file, (u8*)make_c_str(file->source_path), style->font)){
        file_close(&mem->general, file);
        *file = temp_file;
        file->source_path.str = file->source_path_;
        file->live_name.str = file->live_name_;
#if BUFFER_EXPERIMENT_SCALPEL
        if (file->tokens_exist)
            file_first_lex_parallel(&mem->general, file);
#endif
        
        Partition old_part = command->part;
        Temp_Memory temp = begin_temp_memory(&vars->mem.part);
        command->part = partition_sub_part(&vars->mem.part, 16 << 10);
        
        view_set_file(view, file, style,
                      vars->hooks[hook_open_file], command, app_links);
        
        command->part = old_part;
        end_temp_memory(temp);
        
        i32 panel_count = layout->panel_count;
        Panel *panels = layout->panels;
        for (i32 i = 0; i < panel_count; ++i){
            Panel *current_panel = panels + i;
            View *current_view_ = current_panel->view;
            File_View *current_view = view_to_file_view(current_view_);
            if (current_view && current_view != view && current_view->file == file){
                view_set_file(current_view, current_view->file, style, 0, command, app_links);
            }
        }
    }
}

COMMAND_DECL(save){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_MEM(mem);
    
    String *file_path = &file->source_path;
    if (file_path->size > 0){
        file_save(&mem->part, file, (u8*)file_path->str);
    }
}

COMMAND_DECL(interactive_save_as){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_MEM(mem);
    USE_WORKING_SET(working_set);
    USE_STYLE(style);
    USE_DELAY(delay);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Interactive_View *int_view =
        interactive_view_init(new_view, &vars->hot_directory, style,
                              working_set, delay);
    int_view->interaction = INTV_SYS_FILE_LIST;
    int_view->action = INTV_SAVE_AS;
    copy(&int_view->query, "Save As: ");
}

COMMAND_DECL(change_active_panel){
    ProfileMomentFunction();
    USE_LAYOUT(layout);
    if (layout->panel_count > 1){
        ++layout->active_panel;
        if (layout->active_panel >= layout->panel_count){
            layout->active_panel = 0;
        }
    }
}

COMMAND_DECL(interactive_switch_buffer){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_MEM(mem);
    USE_WORKING_SET(working_set);
    USE_STYLE(style);
    USE_DELAY(delay);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Interactive_View *int_view = 
        interactive_view_init(new_view, &vars->hot_directory, style,
                              working_set, delay);
    int_view->interaction = INTV_LIVE_FILE_LIST;
    int_view->action = INTV_SWITCH;
    copy(&int_view->query, "Switch File: ");
}

COMMAND_DECL(interactive_kill_buffer){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_MEM(mem);
    USE_WORKING_SET(working_set);
    USE_STYLE(style);
    USE_DELAY(delay);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Interactive_View *int_view = 
        interactive_view_init(new_view, &vars->hot_directory, style,
                              working_set, delay);
    int_view->interaction = INTV_LIVE_FILE_LIST;
    int_view->action = INTV_KILL;
    copy(&int_view->query, "Kill File: ");
}

COMMAND_DECL(kill_buffer){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_DELAY(delay);
    
    delayed_action(delay, DACT_TRY_KILL, file->live_name, view->view_base.panel);
}

COMMAND_DECL(toggle_line_wrap){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    
    Relative_Scrolling scrolling = view_get_relative_scrolling(view);
    if (view->unwrapped_lines){
        view->unwrapped_lines = 0;
        view->target_x = 0;
        view->cursor =
            view_compute_cursor_from_pos(view, view->cursor.pos);
        view->preferred_x = view->cursor.wrapped_x;
    }
    else{
        view->unwrapped_lines = 1;
        view->cursor =
            view_compute_cursor_from_pos(view, view->cursor.pos);
        view->preferred_x = view->cursor.unwrapped_x;
    }
    view_set_relative_scrolling(view, scrolling);
}

COMMAND_DECL(toggle_show_whitespace){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    view->show_whitespace = !view->show_whitespace;
}

COMMAND_DECL(toggle_tokens){
#if BUFFER_EXPERIMENT_SCALPEL
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_MEM(mem);
    
    if (file->tokens_exist){
        file_kill_tokens(&mem->general, file);
    }
    else{
        file_first_lex_parallel(&mem->general, file);
    }
#endif
}

internal void
case_change_range(Mem_Options *mem, File_View *view, Editing_File *file,
                  u8 a, u8 z, u8 char_delta){
    Range range = get_range(view->cursor.pos, view->mark);
    if (range.start < range.end){
        Edit_Step step = {};
        step.type = ED_NORMAL;
        step.edit.start = range.start;
        step.edit.end = range.end;
        step.edit.len = range.end - range.start;
        
        if (file->still_lexing)
            system_cancel_job(BACKGROUND_THREADS, file->lex_job);
        
        view_update_history_before_edit(mem, file, step, 0, hist_normal);
        
        u8 *data = (u8*)file->buffer.data;
        for (i32 i = range.start; i < range.end; ++i){
            if (data[i] >= a && data[i] <= z){
                data[i] += char_delta;
            }
        }

#if BUFFER_EXPERIMENT_SCALPEL
        if (file->token_stack.tokens)
            file_relex_parallel(mem, file, range.start, range.end, 0);
#endif
    }
}

COMMAND_DECL(to_uppercase){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_MEM(mem);
    case_change_range(mem, view, file, 'a', 'z', (u8)('A' - 'a'));
}

COMMAND_DECL(to_lowercase){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_MEM(mem);
    case_change_range(mem, view, file, 'A', 'Z', (u8)('a' - 'A'));
}

COMMAND_DECL(clean_all_lines){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    view_clean_whitespace(mem, view, layout);
}

COMMAND_DECL(eol_dosify){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    file->dos_write_mode = 1;
    file->last_4ed_edit_time = system_get_now();
}

COMMAND_DECL(eol_nixify){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    file->dos_write_mode = 0;
    file->last_4ed_edit_time = system_get_now();
}

COMMAND_DECL(auto_tab){
#if 0
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);

    if (file->token_stack.tokens && file->tokens_complete){
        Range range = get_range(view->cursor.pos, view->mark);
        view_auto_tab_tokens(mem, view, layout, range.start, range.end);
    }
#endif
}

COMMAND_DECL(auto_tab_range){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);

    if (file->token_stack.tokens && file->tokens_complete){
        Range range = get_range(view->cursor.pos, view->mark);
        view_auto_tab_tokens(mem, view, layout, range.start, range.end, 1);
    }
}

COMMAND_DECL(auto_tab_line_at_cursor){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);

    if (file->token_stack.tokens && file->tokens_complete){
        i32 pos = view->cursor.pos;
        view_auto_tab_tokens(mem, view, layout, pos, pos, 0);
    }
}

COMMAND_DECL(auto_tab_whole_file){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);

    if (file->token_stack.tokens && file->tokens_complete){
        view_auto_tab_tokens(mem, view, layout, 0, buffer_size(&file->buffer), 1);
    }
}

COMMAND_DECL(open_panel_vsplit){
    ProfileMomentFunction();
    USE_LAYOUT(layout);
    USE_PANEL(panel);
    
    i32 panel_count = layout->panel_count;
    if (panel_count < layout->panel_max_count){
        Split_Result split = layout_split_panel(layout, panel, 1);
        
        Panel *panel1 = panel;
        Panel *panel2 = split.panel;
        
        panel2->screen_region = panel1->screen_region;
        
        panel2->full.x0 = split.divider->pos;
        panel2->full.x1 = panel1->full.x1;
        panel1->full.x1 = split.divider->pos;
        
        panel_fix_internal_area(panel1);
        panel_fix_internal_area(panel2);
        panel2->prev_inner = panel2->inner;
        
        layout->active_panel = (i32)(panel2 - layout->panels);
    }
}

COMMAND_DECL(open_panel_hsplit){
    ProfileMomentFunction();
    USE_LAYOUT(layout);
    USE_PANEL(panel);
    
    i32 panel_count = layout->panel_count;
    if (panel_count < layout->panel_max_count){
        Split_Result split = layout_split_panel(layout, panel, 0);
        
        Panel *panel1 = panel;
        Panel *panel2 = split.panel;
        
        panel2->screen_region = panel1->screen_region;
        
        panel2->full.y0 = split.divider->pos;
        panel2->full.y1 = panel1->full.y1;
        panel1->full.y1 = split.divider->pos;
        
        panel_fix_internal_area(panel1);
        panel_fix_internal_area(panel2);
        panel2->prev_inner = panel2->inner;
        
        layout->active_panel = (i32)(panel2 - layout->panels);
    }
}

COMMAND_DECL(close_panel){
    ProfileMomentFunction();
    USE_LAYOUT(layout);
    USE_PANEL(panel);
    USE_VIEW(view);
    
    if (layout->panel_count > 1){
        if (view){
            live_set_free_view(command->live_set, view);
            panel->view = 0;
        }
        
        Divider_And_ID div = layout_get_divider(layout, panel->parent);
        Assert(div.divider->child1 == -1 || div.divider->child2 == -1);
        
        i32 child;
        if (div.divider->child1 == -1){
            child = div.divider->child2;
        }
        else{
            child = div.divider->child1;
        }

        i32 parent = div.divider->parent;
        i32 which_child = div.divider->which_child;
        if (parent != -1){
            Divider_And_ID par = layout_get_divider(layout, parent);
            if (which_child == -1){
                par.divider->child1 = child;
            }
            else{
                par.divider->child2 = child;
            }
        }
        else{
            Assert(layout->root == div.id);
            layout->root = child;
        }
        
        if (child != -1){
            Divider_And_ID chi = layout_get_divider(layout, child);
            chi.divider->parent = parent;
            chi.divider->which_child = div.divider->which_child;
        }
        
        layout_free_divider(layout, div.divider);
        layout_free_panel(layout, panel);
        
        if (child == -1){
            panel = layout->panels;
            layout->active_panel = -1;
            for (i32 i = 0; i < layout->panel_count; ++i){
                if (panel->parent == div.id){
                    panel->parent = parent;
                    panel->which_child = which_child;
                    layout->active_panel = i;
                    break;
                }
                ++panel;
            }
            Assert(layout->active_panel != -1);
        }
        else{
            layout->active_panel = layout->active_panel % layout->panel_count;
        }
        
        layout_fix_all_panels(layout);
    }
}

COMMAND_DECL(move_left){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = view->cursor.pos;
    if (pos > 0) --pos;
    
    view_cursor_move(view, pos);
}

COMMAND_DECL(move_right){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 size = buffer_size(&file->buffer);
    i32 pos = view->cursor.pos;
    if (pos < size) ++pos;
    
    view_cursor_move(view, pos);
}

COMMAND_DECL(delete){
#if BUFFER_EXPERIMENT_SCALPEL
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    i32 cursor_pos = view->cursor.pos;
    if (file->buffer.size > 0 && cursor_pos < file->buffer.size){
        i32 start, end;
        start = cursor_pos;
        end = cursor_pos+1;
        
        i32 shift = (end - start);
        Assert(shift > 0);
        
        i32 next_cursor_pos = start;
        view_replace_range(mem, view, layout, start, end, 0, 0, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
        if (view->mark >= end) view->mark -= shift;
    }
#endif
}

COMMAND_DECL(backspace){
#if BUFFER_EXPERIMENT_SCALPEL
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    i32 cursor_pos = view->cursor.pos;
    if (cursor_pos > 0 && cursor_pos <= (i32)file->buffer.size){
        i32 start, end;
        end = cursor_pos;
        
        start = cursor_pos-1;
        
        i32 shift = (end - start);
        Assert(shift > 0);
        
        i32 next_cursor_pos = view->cursor.pos - shift;
        view_replace_range(mem, view, layout, start, end, 0, 0, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
        if (view->mark >= end) view->mark -= shift;
    }
#endif
}

COMMAND_DECL(move_up){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    real32 cy = view_get_cursor_y(view)-view->style->font->height;
    real32 px = view->preferred_x;
    if (cy >= 0){
        view->cursor = view_compute_cursor_from_xy(view, px, cy);
        view->file->cursor_pos = view->cursor.pos;
    }
}

COMMAND_DECL(move_down){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    real32 cy = view_get_cursor_y(view)+view->style->font->height;
    real32 px = view->preferred_x;
    view->cursor = view_compute_cursor_from_xy(view, px, cy);
    view->file->cursor_pos = view->cursor.pos;
}

COMMAND_DECL(seek_end_of_line){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = view_find_end_of_line(view, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_beginning_of_line){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = view_find_beginning_of_line(view, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(page_down){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    
    real32 height = view_compute_height(view);
    real32 max_target_y = view_compute_max_target_y(view);
    real32 cursor_y = view_get_cursor_y(view);
    
    view->target_y += height;
    if (view->target_y > max_target_y) view->target_y = max_target_y;
    
    if (view->target_y >= cursor_y){
        view->cursor =
            view_compute_cursor_from_xy(view, 0, view->target_y);
    }
}

COMMAND_DECL(page_up){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    
    real32 height = view_compute_height(view);
    real32 cursor_y = view_get_cursor_y(view);
    
    view->target_y -= height;
    if (view->target_y < 0) view->target_y = 0;
    
    if (view->target_y + height <= cursor_y){
        view->cursor =
            view_compute_cursor_from_xy(view, 0, view->target_y + height - view->font_height);
    }
}

inline void
open_theme_options(App_Vars *vars, Live_Views *live_set, Mem_Options *mem, Panel *panel){
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Color_View *color_view = color_view_init(new_view, &vars->working_set);
    color_view->hot_directory = &vars->hot_directory;
    color_view->main_style = &vars->style;
    color_view->styles = &vars->styles;
    color_view->palette = vars->palette;
    color_view->palette_size = vars->palette_size;
    color_view->fonts = &vars->fonts;
}

COMMAND_DECL(open_color_tweaker){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_MEM(mem);
    USE_PANEL(panel);
    
    open_theme_options(vars, live_set, mem, panel);
}

COMMAND_DECL(open_menu){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_MEM(mem);
    USE_WORKING_SET(working_set);
    USE_STYLE(style);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Menu_View *menu_view = menu_view_init(new_view, style, working_set, &vars->delay);
    AllowLocal(menu_view);
}

#if FRED_INTERNAL
COMMAND_DECL(open_debug_view){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_STYLE(style);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_MEM(mem);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_major(new_view, panel, live_set);
    
    new_view->map = &vars->map_debug;
    Debug_View *debug_view = debug_view_init(new_view);
    debug_view->font = style->font;
    debug_view->mode = DBG_MEMORY;
}

COMMAND_DECL(debug_memory){
    ProfileMomentFunction();
    REQ_DBG_VIEW(view);
    view->mode = DBG_MEMORY;
}

COMMAND_DECL(debug_os_events){
    ProfileMomentFunction();
    REQ_DBG_VIEW(view);
    view->mode = DBG_OS_EVENTS;
}

COMMAND_DECL(debug_profile){
    ProfileMomentFunction();
    REQ_DBG_VIEW(view);
    view->mode = DBG_PROFILE;
}

COMMAND_DECL(pause_unpause_profile){
    ProfileMomentFunction();
    INTERNAL_updating_profile = !INTERNAL_updating_profile;
}
#endif

COMMAND_DECL(close_minor_view){
    ProfileMomentFunction();
    REQ_VIEW(view);
    USE_PANEL(panel);
    USE_LIVE_SET(live_set);
    
    view_remove_minor(panel, live_set);
}

COMMAND_DECL(cursor_mark_swap){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    
    i32 pos = view->cursor.pos;
    view->cursor = view_compute_cursor_from_pos(view, view->mark);
    view->mark = pos;
}

COMMAND_DECL(user_callback){
    ProfileMomentFunction();
    if (binding.custom) binding.custom(command, app_links);
}

COMMAND_DECL(set_settings){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_VARS(vars);
    USE_MEM(mem);
    AllowLocal(mem);
    
    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int p = dynamic_to_int(&param->param.param);
        switch (p){
        case par_lex_as_cpp_file:
        {
#if BUFFER_EXPERIMENT_SCALPEL
            int v = dynamic_to_bool(&param->param.value);
            if (file->tokens_exist){
                if (!v) file_kill_tokens(&mem->general, file);
            }
            else{
                if (v) file_first_lex_parallel(&mem->general, file);
            }
#endif
        }break;
        
        case par_wrap_lines:
        {
            int v = dynamic_to_bool(&param->param.value);
            if (view->unwrapped_lines){
                if (v){
                    Relative_Scrolling scrolling = view_get_relative_scrolling(view);
                    view->unwrapped_lines = 0;
                    view->target_x = 0;
                    view->cursor =
                        view_compute_cursor_from_pos(view, view->cursor.pos);
                    view_set_relative_scrolling(view, scrolling);
                }
            }
            else{
                if (!v){
                    Relative_Scrolling scrolling = view_get_relative_scrolling(view);
                    view->unwrapped_lines = 1;
                    view->cursor =
                        view_compute_cursor_from_pos(view, view->cursor.pos);
                    view_set_relative_scrolling(view, scrolling);
                }
            }
        }break;
        
        case par_key_mapid:
        {
            int v = dynamic_to_int(&param->param.value);
            Command_Map *map = 0;
            if (v == mapid_global) map = &vars->map_top;
            else if (v == mapid_file) map = &vars->map_file;
            else if (v >= mapid_user_custom){
                int index = app_get_map_index(vars, v);
                if (index < vars->user_map_count) map = vars->user_maps + index;
                else map = 0;
            }
        }break;
        }
    }
}

COMPOSE_DECL(compose_write_auto_tab_line){
    command_write_character(command, binding);
    command_auto_tab_line_at_cursor(command, binding);
}

globalvar Command_Function command_table[cmdid_count];

extern "C"{
    EXECUTE_COMMAND_SIG(external_exec_command_keep_stack){
        Command_Data *cmd = (Command_Data*)cmd_context;
        Command_Function function = command_table[command_id];
        Command_Binding binding;
        binding.function = function;
        if (function) function(cmd, binding);
        
        App_Vars *vars = cmd->vars;
        Command_Data command_data;
        command_data.vars = vars;
        command_data.mem = &vars->mem;
        command_data.working_set = &vars->working_set;
        command_data.layout = &vars->layout;
        command_data.panel = command_data.layout->panels + command_data.layout->active_panel;
        command_data.view = command_data.panel->view;
        command_data.live_set = &vars->live_set;
        command_data.style = &vars->style;
        command_data.delay = &vars->delay;
        command_data.screen_width = cmd->screen_width;
        command_data.screen_height = cmd->screen_height;
        command_data.key = cmd->key;
        command_data.part = cmd->part;
        
        *cmd = command_data;
    }
    
    PUSH_PARAMETER_SIG(external_push_parameter){
        Command_Data *cmd = (Command_Data*)cmd_context;
        Partition *part = &cmd->part;
        Command_Parameter *cmd_param = push_struct(part, Command_Parameter);
        cmd_param->type = 0;
        cmd_param->param.param = param;
        cmd_param->param.value = value;
    }
    
    PUSH_MEMORY_SIG(external_push_memory){
        Command_Data *cmd = (Command_Data*)cmd_context;
        Partition *part = &cmd->part;
        Command_Parameter *base = push_struct(part, Command_Parameter);
        char *result = push_array(part, char, len);
        int full_len = len + sizeof(Command_Parameter) - 1;
        full_len -= (full_len % sizeof(Command_Parameter));
        part->pos += full_len - len;
        base->type = 1;
        base->inline_string.str = result;
        base->inline_string.len = len;
        return result;
    }
    
    CLEAR_PARAMETERS_SIG(external_clear_parameters){
        Command_Data *cmd = (Command_Data*)cmd_context;
        cmd->part.pos = 0;
    }
    
    GET_ACTIVE_BUFFER_SIG(external_get_active_buffer){
        Command_Data *cmd = (Command_Data*)cmd_context;
        Buffer_Summary buffer = {};
        
        File_View *view = view_to_file_view(cmd->view);
        if (view){
            Editing_File *file = view->file;
            if (file && !file->is_dummy){
#if BUFFER_EXPERIMENT_SCALPEL
                Working_Set *working_set = cmd->working_set;
                buffer.file_id = (int)(file - working_set->files);
                buffer.size = file->buffer.size;
                buffer.data = (const char*)file->buffer.data;
                buffer.file_name_len = file->source_path.size;
                buffer.buffer_name_len = file->live_name.size;
                buffer.file_name = file->source_path.str;
                buffer.buffer_name = file->live_name.str;
                buffer.file_cursor_pos = file->cursor_pos;
                buffer.is_lexed = file->tokens_exist;
                buffer.map_id = file->base_map_id;
#endif
            }
        }
        
        return buffer;
    }
}

inline void
app_links_init(){
    app_links.exec_command_keep_stack = external_exec_command_keep_stack;
    app_links.push_parameter = external_push_parameter;
    app_links.push_memory = external_push_memory;
    app_links.clear_parameters = external_clear_parameters;
    app_links.get_active_buffer = external_get_active_buffer;
}

#if FRED_INTERNAL
internal void
setup_debug_commands(Command_Map *commands, Partition *part, Key_Codes *codes, Command_Map *parent){
    map_init(commands, part, 6, parent);
    
    map_add(commands, 'm', MDFR_NONE, command_debug_memory);
    map_add(commands, 'o', MDFR_NONE, command_debug_os_events);
    map_add(commands, 'p', MDFR_NONE, command_debug_profile);
}
#endif

internal void
setup_ui_commands(Command_Map *commands, Partition *part, Key_Codes *codes, Command_Map *parent){
    map_init(commands, part, 12, parent);
    
    commands->vanilla_keyboard_default.function = command_null;
    
    map_add(commands, codes->left, MDFR_NONE, command_null);
    map_add(commands, codes->right, MDFR_NONE, command_null);
    map_add(commands, codes->up, MDFR_NONE, command_null);
    map_add(commands, codes->down, MDFR_NONE, command_null);
    map_add(commands, codes->back, MDFR_NONE, command_null);
    map_add(commands, codes->esc, MDFR_NONE, command_close_minor_view);
}

internal void
setup_file_commands(Command_Map *commands, Partition *part, Key_Codes *codes, Command_Map *parent){
    map_init(commands, part, 101, parent);
    
    commands->vanilla_keyboard_default.function = command_write_character;
    
    map_add(commands, codes->left, MDFR_NONE, command_move_left);
    map_add(commands, codes->right, MDFR_NONE, command_move_right);
    map_add(commands, codes->del, MDFR_NONE, command_delete);
    map_add(commands, codes->back, MDFR_NONE, command_backspace);
    map_add(commands, codes->up, MDFR_NONE, command_move_up);
    map_add(commands, codes->down, MDFR_NONE, command_move_down);
    map_add(commands, codes->end, MDFR_NONE, command_seek_end_of_line);
    map_add(commands, codes->home, MDFR_NONE, command_seek_beginning_of_line);
    map_add(commands, codes->page_up, MDFR_NONE, command_page_up);
    map_add(commands, codes->page_down, MDFR_NONE, command_page_down);
    
    map_add(commands, codes->right, MDFR_CTRL, command_seek_whitespace_right);
    map_add(commands, codes->left, MDFR_CTRL, command_seek_whitespace_left);
    map_add(commands, codes->up, MDFR_CTRL, command_seek_whitespace_up);
    map_add(commands, codes->down, MDFR_CTRL, command_seek_whitespace_down);
    
    map_add(commands, ' ', MDFR_CTRL, command_set_mark);
    map_add(commands, 'm', MDFR_CTRL, command_cursor_mark_swap);
    map_add(commands, 'c', MDFR_CTRL, command_copy);
    map_add(commands, 'x', MDFR_CTRL, command_cut);
    map_add(commands, 'v', MDFR_CTRL, command_paste);
    map_add(commands, 'V', MDFR_CTRL, command_paste_next);
    map_add(commands, 'z', MDFR_CTRL, command_undo);
    map_add(commands, 'y', MDFR_CTRL, command_redo);
    map_add(commands, 'Z', MDFR_CTRL, command_timeline_scrub);
    map_add(commands, codes->left, MDFR_ALT, command_increase_rewind_speed);
    map_add(commands, codes->right, MDFR_ALT, command_increase_fastforward_speed);
    map_add(commands, codes->down, MDFR_ALT, command_stop_rewind_fastforward);
    map_add(commands, 'h', MDFR_CTRL, command_history_backward);
    map_add(commands, 'H', MDFR_CTRL, command_history_forward);
    map_add(commands, 'd', MDFR_CTRL, command_delete_chunk);
    map_add(commands, 'l', MDFR_CTRL, command_toggle_line_wrap);
    map_add(commands, '?', MDFR_CTRL, command_toggle_show_whitespace);
    map_add(commands, '|', MDFR_CTRL, command_toggle_tokens);
    map_add(commands, 'u', MDFR_CTRL, command_to_uppercase);
    map_add(commands, 'j', MDFR_CTRL, command_to_lowercase);
    map_add(commands, '~', MDFR_CTRL, command_clean_all_lines);
    map_add(commands, 'f', MDFR_CTRL, command_search);
    map_add(commands, 'r', MDFR_CTRL, command_rsearch);
    map_add(commands, 'g', MDFR_CTRL, command_goto_line);
    
    map_add(commands, '\n', MDFR_NONE, compose_write_auto_tab_line);
    map_add(commands, '}', MDFR_NONE, compose_write_auto_tab_line);
    map_add(commands, ')', MDFR_NONE, compose_write_auto_tab_line);
    map_add(commands, ']', MDFR_NONE, compose_write_auto_tab_line);
    map_add(commands, ';', MDFR_NONE, compose_write_auto_tab_line);
    
    map_add(commands, '\t', MDFR_NONE, command_auto_tab_line_at_cursor);
    map_add(commands, '\t', MDFR_CTRL, command_auto_tab_range);
    map_add(commands, '\t', MDFR_CTRL | MDFR_SHIFT, command_write_character);
    
    map_add(commands, 'K', MDFR_CTRL, command_kill_buffer);
    map_add(commands, 'O', MDFR_CTRL, command_reopen);
    map_add(commands, 's', MDFR_CTRL, command_save);
    map_add(commands, 'w', MDFR_CTRL, command_interactive_save_as);
}

internal void
setup_top_commands(Command_Map *commands, Partition *part, Key_Codes *codes, Command_Map *parent){
    map_init(commands, part, 51, parent);
    
#if FRED_INTERNAL
    map_add(commands, 'd', MDFR_ALT, command_open_debug_view);
    map_add(commands, 'p', MDFR_CTRL | MDFR_ALT, command_pause_unpause_profile);
#endif
    
    map_add(commands, 'p', MDFR_CTRL, command_open_panel_vsplit);
    map_add(commands, '-', MDFR_CTRL, command_open_panel_hsplit);
    map_add(commands, 'P', MDFR_CTRL, command_close_panel);
    map_add(commands, 'n', MDFR_CTRL, command_interactive_new);
    map_add(commands, 'o', MDFR_CTRL, command_interactive_open);
    map_add(commands, ',', MDFR_CTRL, command_change_active_panel);
    map_add(commands, 'k', MDFR_CTRL, command_interactive_kill_buffer);
    map_add(commands, 'i', MDFR_CTRL, command_interactive_switch_buffer);
    map_add(commands, 'c', MDFR_ALT, command_open_color_tweaker);
    map_add(commands, 'x', MDFR_ALT, command_open_menu);
}

#if 0 // TODO(allen): Here's an idea
internal void
setup_command_table(){
    BEGIN_META_CODE{
        int count;
        char **command_names = get_all_commands(&count);
        for (int i = 0; i < count; ++i){
            char *name_ = command_names[i];
            String name = make_string_slowly(name_);
            
            outcode("command_table[cmdid_", out_str(name), "] = command_", out_str(name), "\n");
        }
    }END_META_CODE
}
#endif

internal void
setup_command_table(){
#define SET(n) command_table[cmdid_##n] = command_##n
    
    SET(null);
    SET(write_character);
    SET(seek_whitespace_right);
    SET(seek_whitespace_left);
    SET(seek_whitespace_up);
    SET(seek_whitespace_down);
    SET(seek_token_left);
    SET(seek_token_right);
    SET(seek_white_or_token_right);
    SET(seek_white_or_token_left);
    SET(seek_alphanumeric_right);
    SET(seek_alphanumeric_left);
    SET(seek_alphanumeric_or_camel_right);
    SET(seek_alphanumeric_or_camel_left);
    SET(search);
    SET(rsearch);
    SET(goto_line);
    SET(set_mark);
    SET(copy);
    SET(cut);
    SET(paste);
    SET(paste_next);
    SET(delete_chunk);
    SET(timeline_scrub);
    SET(undo);
    SET(redo);
    SET(increase_rewind_speed);
    SET(increase_fastforward_speed);
    SET(stop_rewind_fastforward);
    SET(history_backward);
    SET(history_forward);
    SET(interactive_new);
    SET(interactive_open);
    SET(reopen);
    SET(save);
    SET(interactive_save_as);
    SET(change_active_panel);
    SET(interactive_switch_buffer);
    SET(interactive_kill_buffer);
    SET(kill_buffer);
    SET(toggle_line_wrap);
    SET(to_uppercase);
    SET(to_lowercase);
    SET(toggle_show_whitespace);
    SET(clean_all_lines);
    SET(eol_dosify);
    SET(eol_nixify);
    SET(auto_tab);
    SET(auto_tab_range);
    SET(auto_tab_line_at_cursor);
    SET(auto_tab_whole_file);
    SET(open_panel_vsplit);
    SET(open_panel_hsplit);
    SET(close_panel);
    SET(move_left);
    SET(move_right);
    SET(delete);
    SET(backspace);
    SET(move_up);
    SET(move_down);
    SET(seek_end_of_line);
    SET(seek_beginning_of_line);
    SET(page_up);
    SET(page_down);
    SET(open_color_tweaker);
    SET(close_minor_view);
    SET(cursor_mark_swap);
    SET(open_menu);
    SET(set_settings);
    
#undef SET
}

// Interactive Bar

internal void
hot_directory_draw_helper(Render_Target *target,
                          Hot_Directory *hot_directory,
                          Interactive_Bar *bar, String *string,
                          bool32 include_files){
    persist u8 str_open_bracket[] = " {";
    persist u8 str_close_bracket[] = "}";
    persist u8 str_comma[] = ", ";
    
    intbar_draw_string(target, bar, *string, bar->style.pop1_color);
    intbar_draw_string(target, bar, str_open_bracket, bar->style.base_color);
    
    char front_name_[256];
    String front_name = make_fixed_width_string(front_name_);
    get_front_of_directory(&front_name, *string);
    
    bool32 is_first_string = 1;
    File_List *files = &hot_directory->file_list;
    
    Absolutes absolutes;
    get_absolutes(front_name, &absolutes, 1, 1);

    File_Info *info, *end;
    end = files->infos + files->count;
    for (info = files->infos; info != end; ++info){
        String filename = info->filename;
        
        if (filename_match(front_name, &absolutes, filename)){
            if (is_first_string){
                is_first_string = 0;
            }
            else{
                intbar_draw_string(target, bar, str_comma, bar->style.base_color);
            }
            if (info->folder){
                intbar_draw_string(target, bar, filename, bar->style.pop1_color);
                intbar_draw_string(target, bar, (u8*)"/", bar->style.pop1_color);
            }
            else{
                intbar_draw_string(target, bar, filename, bar->style.base_color);
            }
        }
    }
    
    intbar_draw_string(target, bar, str_close_bracket, bar->style.base_color);
}

internal void
live_file_draw_helper(Render_Target *target, Working_Set *working_set,
                      Interactive_Bar *bar, String *string){
    persist u8 str_open_bracket[] = " {";
    persist u8 str_close_bracket[] = "}";
    persist u8 str_comma[] = ", ";
    
    intbar_draw_string(target, bar, *string, bar->style.base_color);
    
    intbar_draw_string(target, bar, str_open_bracket, bar->style.base_color);
    
    bool32 is_first_string = 1;
    for (i32 file_i = 0;
         file_i < working_set->file_index_count;
         ++file_i){
        Editing_File *file = &working_set->files[file_i];
        if (file->live_name.str &&
            (string->size == 0 || has_substr_unsensitive(file->live_name, *string))){
            if (is_first_string){
                is_first_string = 0;
            }
            else{
                intbar_draw_string(target, bar, str_comma, bar->style.base_color);
            }
            intbar_draw_string(target, bar, file->live_name, bar->style.base_color);
        }
    }
    intbar_draw_string(target, bar, str_close_bracket, bar->style.base_color);
}

// App Functions

internal void
app_hardcode_styles(App_Vars *vars){
    Interactive_Style file_info_style;
    Style *styles, *style;
    styles = vars->styles.styles;
    style = styles;
    
    Font *fonts = vars->fonts.fonts;
    
    /////////////////
    style_set_name(style, make_lit_string("4coder"));
    style->font = fonts + 1;
    
    style->main.back_color = 0xFF0C0C0C;
    style->main.margin_color = 0xFF181818;
    style->main.margin_hover_color = 0xFF252525;
    style->main.margin_active_color = 0xFF323232;
    style->main.cursor_color = 0xFF00EE00;
    style->main.highlight_color = 0xFFDDEE00;
    style->main.mark_color = 0xFF494949;
    style->main.default_color = 0xFF90B080;
    style->main.at_cursor_color = style->main.back_color;
    style->main.at_highlight_color = 0xFFFF44DD;
    style->main.comment_color = 0xFF2090F0;
    style->main.keyword_color = 0xFFD08F20;
    style->main.str_constant_color = 0xFF50FF30;
    style->main.char_constant_color = style->main.str_constant_color;
    style->main.int_constant_color = style->main.str_constant_color;
    style->main.float_constant_color = style->main.str_constant_color;
    style->main.bool_constant_color = style->main.str_constant_color;
    style->main.include_color = style->main.str_constant_color;
    style->main.preproc_color = style->main.default_color;
    style->main.special_character_color = 0xFFFF0000;
    
    style->main.paste_color = 0xFFDDEE00;
    style->main.undo_color = 0xFF00DDEE;
    style->main.next_undo_color = 0xFF006E77;
    
    style->main.highlight_junk_color = 0xff3a0000;
    style->main.highlight_white_color = 0xff003a3a;
    
    file_info_style.bar_color = 0xFF888888;
    file_info_style.bar_active_color = 0xFF666666;
    file_info_style.base_color = 0xFF000000;
    file_info_style.pop1_color = 0xFF4444AA;
    file_info_style.pop2_color = 0xFFFF0000;
    style->main.file_info_style = file_info_style;
    style->font_changed = 1;
    ++style;

    /////////////////
    *style = *(style-1);
    style_set_name(style, make_lit_string("4coder-mono"));
    style->font = fonts;
    ++style;
    
    /////////////////
    style_set_name(style, make_lit_string("Handmade Hero"));
    style->font = fonts;
    
    style->main.back_color = 0xFF161616;
    style->main.margin_color = 0xFF262626;
    style->main.margin_hover_color = 0xFF333333;
    style->main.margin_active_color = 0xFF404040;
    style->main.cursor_color = 0xFF40FF40;
    style->main.at_cursor_color = style->main.back_color;
    style->main.mark_color = 0xFF808080;
    style->main.highlight_color = 0xFF191970;
    style->main.at_highlight_color = 0xFFCDAA7D;
    style->main.default_color = 0xFFCDAA7D;
    style->main.comment_color = 0xFF7F7F7F;
    style->main.keyword_color = 0xFFCD950C;
    style->main.str_constant_color = 0xFF6B8E23;
    style->main.char_constant_color = style->main.str_constant_color;
    style->main.int_constant_color = style->main.str_constant_color;
    style->main.float_constant_color = style->main.str_constant_color;
    style->main.bool_constant_color = style->main.str_constant_color;
    style->main.include_color = style->main.str_constant_color;
    style->main.preproc_color = style->main.default_color;
    style->main.special_character_color = 0xFFFF0000;
    
    style->main.paste_color = 0xFFFFBB00;
    style->main.undo_color = 0xFFFF00BB;
    style->main.undo_color = 0xFF80005D;
    
    style->main.highlight_junk_color = 0xFF3A0000;
    style->main.highlight_white_color = 0xFF003A3A;
    
    file_info_style.bar_color = 0xFFCACACA;
    file_info_style.bar_active_color = 0xFFA8A8A8;
    file_info_style.base_color = 0xFF000000;
    file_info_style.pop1_color = 0xFF1504CF;
    file_info_style.pop2_color = 0xFFFF0000;
    style->main.file_info_style = file_info_style;
    style->font_changed = 1;
    ++style;
    
    /////////////////
    style_set_name(style, make_lit_string("Twilight"));
    style->font = fonts + 2;
    
    style->main.back_color = 0xFF090D12;
    style->main.margin_color = 0xFF1A2634;
    style->main.margin_hover_color = 0xFF2D415B;
    style->main.margin_active_color = 0xFF405D82;
    style->main.cursor_color = 0xFFEEE800;
    style->main.at_cursor_color = style->main.back_color;
    style->main.mark_color = 0xFF8BA8CC;
    style->main.highlight_color = 0xFF037A7B;
    style->main.at_highlight_color = 0xFFFEB56C;
    style->main.default_color = 0xFFB7C19E;
    style->main.comment_color = 0xFF20ECF0;
    style->main.keyword_color = 0xFFD86909;
    style->main.str_constant_color = 0xFFC4EA5D;
    style->main.char_constant_color = style->main.str_constant_color;
    style->main.int_constant_color = style->main.str_constant_color;
    style->main.float_constant_color = style->main.str_constant_color;
    style->main.bool_constant_color = style->main.str_constant_color;
    style->main.include_color = style->main.str_constant_color;
    style->main.preproc_color = style->main.default_color;
    style->main.special_character_color = 0xFFFF0000;
    
    style->main.paste_color = 0xFFDDEE00;
    style->main.undo_color = 0xFF00DDEE;
    style->main.next_undo_color = 0xFF006E77;
    
    style->main.highlight_junk_color = 0xff3a0000;
    style->main.highlight_white_color = 0xFF151F2A;
    
    file_info_style.bar_color = 0xFF315E68;
    file_info_style.bar_active_color = 0xFF0F3C46;
    file_info_style.base_color = 0xFF000000;
    file_info_style.pop1_color = 0xFF1BFF0C;
    file_info_style.pop2_color = 0xFFFF200D;
    style->main.file_info_style = file_info_style;
    style->font_changed = 1;
    ++style;
    
    /////////////////
    style_set_name(style, make_lit_string("Wolverine"));
    style->font = fonts + 1;
    
    style->main.back_color = 0xFF070711;
    style->main.margin_color = 0xFF111168;
    style->main.margin_hover_color = 0xFF191996;
    style->main.margin_active_color = 0xFF2121C3;
    style->main.cursor_color = 0xFF7082F9;
    style->main.at_cursor_color = 0xFF000014;
    style->main.mark_color = 0xFF4b5028;
    style->main.highlight_color = 0xFFDDEE00;
    style->main.at_highlight_color = 0xFF000019;
    style->main.default_color = 0xFF8C9740;
    style->main.comment_color = 0xFF3A8B29;
    style->main.keyword_color = 0xFFD6B109;
    style->main.str_constant_color = 0xFFAF5FA7;
    style->main.char_constant_color = style->main.str_constant_color;
    style->main.int_constant_color = style->main.str_constant_color;
    style->main.float_constant_color = style->main.str_constant_color;
    style->main.bool_constant_color = style->main.str_constant_color;
    style->main.include_color = style->main.str_constant_color;
    style->main.preproc_color = style->main.default_color;
    style->main.special_character_color = 0xFFFF0000;
    
    style->main.paste_color = 0xFF900090;
    style->main.undo_color = 0xFF606090;
    style->main.next_undo_color = 0xFF404070;
    
    style->main.highlight_junk_color = 0xff3a0000;
    style->main.highlight_white_color = 0xff003a3a;
    
    file_info_style.bar_color = 0xFF7082F9;
    file_info_style.bar_active_color = 0xFF4E60D7;
    file_info_style.base_color = 0xFF000000;
    file_info_style.pop1_color = 0xFFFAFA15;
    file_info_style.pop2_color = 0xFFD20000;
    style->main.file_info_style = file_info_style;
    style->font_changed = 1;
    ++style;
    
    /////////////////
    style_set_name(style, make_lit_string("stb"));
    style->font = fonts + 3;
    
    style->main.back_color = 0xFFD6D6D6;
    style->main.margin_color = 0xFF9E9E9E;
    style->main.margin_hover_color = 0xFF7E7E7E;
    style->main.margin_active_color = 0xFF5C5C5C;
    style->main.cursor_color = 0xFF000000;
    style->main.at_cursor_color = 0xFFD6D6D6;
    style->main.mark_color = 0xFF525252;
    style->main.highlight_color = 0xFF0044FF;
    style->main.at_highlight_color = 0xFFD6D6D6;
    style->main.default_color = 0xFF000000;
    style->main.comment_color = 0xFF000000;
    style->main.keyword_color = 0xFF000000;
    style->main.str_constant_color = 0xFF000000;
    style->main.char_constant_color = style->main.str_constant_color;
    style->main.int_constant_color = style->main.str_constant_color;
    style->main.float_constant_color = style->main.str_constant_color;
    style->main.bool_constant_color = style->main.str_constant_color;
    style->main.include_color = style->main.str_constant_color;
    style->main.preproc_color = style->main.default_color;
    style->main.special_character_color = 0xFF9A0000;
    
    style->main.paste_color = 0xFF00B8B8;
    style->main.undo_color = 0xFFB800B8;
    style->main.next_undo_color = 0xFF5C005C;
    
    style->main.highlight_junk_color = 0xFFFF7878;
    style->main.highlight_white_color = 0xFFBCBCBC;
    
    file_info_style.bar_color = 0xFF606060;
    file_info_style.bar_active_color = 0xFF3E3E3E;
    file_info_style.base_color = 0xFF000000;
    file_info_style.pop1_color = 0xFF1111DC;
    file_info_style.pop2_color = 0xFFE80505;
    style->main.file_info_style = file_info_style;
    style->font_changed = 1;
    ++style;
    
    vars->styles.count = (i32)(style - styles);
    vars->styles.max = ArrayCount(vars->styles.styles);
    style_copy(&vars->style, vars->styles.styles);
    vars->style.font_changed = 0;
}

internal bool32
app_load_font(Font *font, char *filename, i32 size, void *memory,
              i32 *used, i32 tab_width, String name){
    if (font_load(font, filename, size, memory, font_predict_size(size), used, tab_width)){
        font->loaded = 1;
        font->name_[ArrayCount(font->name_)-1] = 0;
        font->name = make_string(font->name_, 0, ArrayCount(font->name_)-1);
        copy(&font->name, name);
    }
    return font->loaded;
}

char *_4coder_get_extension(const char *filename, int len, int *extension_len){
    char *c = (char*)(filename + len - 1);
    char *end = c;
    while (*c != '.' && c > filename) --c;
    *extension_len = (int)(end - c);
    return c+1;
}

bool _4coder_str_match(const char *a, int len_a, const char *b, int len_b){
    bool result = 0;
    if (len_a == len_b){
        char *end = (char*)(a + len_a);
        while (a < end && *a == *b){
            ++a; ++b;
        }
        if (a == end) result = 1;
    }
    return result;
}

#define literal(s) s, (sizeof(s) - 1)

HOOK_SIG(default_open_file_hook){
    Buffer_Summary buffer = app.get_active_buffer(cmd_context);
    
    int treat_as_code = 0;
    
    if (buffer.file_name && buffer.size < (16 << 20)){
        int extension_len;
        char *extension = _4coder_get_extension(buffer.file_name, buffer.file_name_len, &extension_len);
        if (_4coder_str_match(extension, extension_len, literal("cpp"))) treat_as_code = 1;
        else if (_4coder_str_match(extension, extension_len, literal("h"))) treat_as_code = 1;
        else if (_4coder_str_match(extension, extension_len, literal("c"))) treat_as_code = 1;
        else if (_4coder_str_match(extension, extension_len, literal("hpp"))) treat_as_code = 1;
    }
    
    app.push_parameter(cmd_context, dynamic_int(par_lex_as_cpp_file), dynamic_int(treat_as_code));
    app.push_parameter(cmd_context, dynamic_int(par_wrap_lines), dynamic_int(!treat_as_code));
    app.push_parameter(cmd_context, dynamic_int(par_key_mapid), dynamic_int(mapid_file));
    
    app.exec_command_keep_stack(cmd_context, cmdid_set_settings);
    app.clear_parameters(cmd_context);
}

internal bool32
app_init(Thread_Context *thread, Application_Memory *memory,
         Key_Codes *loose_codes, Clipboard_Contents clipboard){
    app_links_init();
    
    Partition _partition = partition_open(memory->vars_memory, memory->vars_memory_size);
    App_Vars *vars = push_struct(&_partition, App_Vars);
    Assert(vars);
    *vars = {};
    vars->mem.part = _partition;
    Partition *partition = &vars->mem.part;
    general_memory_open(&vars->mem.general, memory->target_memory, memory->target_memory_size);
    
    i32 panel_max_count = vars->layout.panel_max_count = 16;
    i32 panel_count = vars->layout.panel_count = 1;
    i32 divider_max_count = panel_max_count - 1;
    
    Panel *panels = vars->layout.panels =
        push_array(partition, Panel, panel_max_count);
    
    Panel_Divider *dividers = vars->layout.dividers =
        push_array(partition, Panel_Divider, divider_max_count);
    
    Panel_Divider *divider = dividers;
    for (i32 i = 0; i < divider_max_count-1; ++i, ++divider){
        divider->next_free = (divider + 1);
    }
    divider->next_free = 0;
    vars->layout.free_divider = dividers;
    
    vars->live_set.count = 0;
    vars->live_set.max = 1 + 2*panel_max_count;
    i32 view_sizes[] = {
        sizeof(File_View),
        sizeof(Color_View),
        sizeof(Interactive_View),
#if FRED_INTERNAL
        sizeof(Debug_View)
#endif
    };
    i32 view_chunk_size = 0;
    for (i32 i = 0; i < ArrayCount(view_sizes); ++i){
        view_chunk_size = Max(view_chunk_size, view_sizes[i]);
    }
    vars->live_set.stride = view_chunk_size;
    vars->live_set.views = (File_View*)
        push_block(partition, view_chunk_size*vars->live_set.max);
    
    char *views_ = (char*)vars->live_set.views;
    for (i32 i = panel_max_count-2; i >= 0; --i){
        View *view = (View*)(views_ + i*view_chunk_size);
        View *view_next = (View*)((char*)view + view_chunk_size);
        view->next_free = view_next;
    }
    {
        View *view = (View*)(views_ + (panel_max_count-1)*view_chunk_size);
        view->next_free = 0;
    }
    vars->live_set.free_view = (View*)views_;

    setup_command_table();
    
    Command_Map *global = &vars->map_top;
    if (TEMP.get_bindings){
        i32 size = partition_remaining(partition);
        void *data = partition_current(partition);

        // TODO(allen): Use a giant bubble of general memory for this.
        // So that it doesn't interfere with the command maps as they allocate
        // their own memory.
        i32 wanted_size = TEMP.get_bindings(data, size, loose_codes);
        
        bool32 did_top = 0;
        bool32 did_file = 0;
        if (wanted_size <= size){
            partition_allocate(partition, wanted_size);
            
            Binding_Unit *unit = (Binding_Unit*)data;
            if (unit->type == unit_header && unit->header.error == 0){
                Binding_Unit *end = unit + unit->header.total_size;
                
                i32 user_map_count = unit->header.user_map_count;
                
                vars->map_id_table =
                    push_array(&vars->mem.part, i32, user_map_count);
                
                vars->user_maps =
                    push_array(&vars->mem.part, Command_Map, user_map_count);

                vars->user_map_count = user_map_count;
                
                Command_Map *mapptr = 0;
                for (++unit; unit < end; ++unit){
                    switch (unit->type){
                    case unit_map_begin:
                    {
                        int table_max = unit->map_begin.bind_count * 3 / 2;
                        int mapid = unit->map_begin.mapid;
                        if (mapid == mapid_global){
                            mapptr = &vars->map_top;
                            map_init(mapptr, &vars->mem.part, table_max, global);
                            did_top = 1;
                        }
                        else if (mapid == mapid_file){
                            mapptr = &vars->map_file;
                            map_init(mapptr, &vars->mem.part, table_max, global);
                            did_file = 1;
                        }
                        else if (mapid >= mapid_user_custom){
                            i32 index = app_get_or_add_map_index(vars, mapid);
                            Assert(index < user_map_count);
                            mapptr = vars->user_maps + index;
                            map_init(mapptr, &vars->mem.part, table_max, global);
                        }
                        else mapptr = 0;
                    }break;
                    
                    case unit_inherit:
                        if (mapptr){
                            Command_Map *parent = 0;
                            int mapid = unit->map_inherit.mapid;
                            if (mapid == mapid_global) parent = &vars->map_top;
                            else if (mapid == mapid_file) parent = &vars->map_file;
                            else if (mapid >= mapid_user_custom){
                                i32 index = app_get_or_add_map_index(vars, mapid);
                                if (index < user_map_count) parent = vars->user_maps + index;
                                else parent = 0;
                            }
                            mapptr->parent = parent;
                        }break;
                    
                    case unit_binding:
                        if (mapptr){
                            Command_Function func = 0;
                            if (unit->binding.command_id >= 0 && unit->binding.command_id < cmdid_count)
                                func = command_table [unit->binding.command_id];
                            if (func){
                                if (unit->binding.code == 0 && unit->binding.modifiers == 0){
                                    mapptr->vanilla_keyboard_default.function = func;
                                }
                                else{
                                    map_add(mapptr, unit->binding.code, unit->binding.modifiers, func);
                                }
                            }
                        }
                        break;
                        
                    case unit_callback:
                        if (mapptr){
                            Command_Function func = command_user_callback;
                            Custom_Command_Function *custom = unit->callback.func;
                            if (func){
                                if (unit->callback.code == 0 && unit->callback.modifiers == 0){
                                    mapptr->vanilla_keyboard_default.function = func;
                                    mapptr->vanilla_keyboard_default.custom = custom;
                                }
                                else{
                                    map_add(mapptr, unit->callback.code, unit->callback.modifiers, func, custom);
                                }
                            }
                        }
                        break;
                    
                    case unit_hook:
                    {
                        int hook_id = unit->hook.hook_id;
                        if (hook_id >= 0 && hook_id < hook_type_count){
                            vars->hooks[hook_id] = unit->hook.func;
                        }
                    }break;
                    }
                }
            }
        }
        
        if (!did_top) setup_top_commands(&vars->map_top, &vars->mem.part, loose_codes, global);
        if (!did_file) setup_file_commands(&vars->map_file, &vars->mem.part, loose_codes, global);
    }
    else{
        setup_top_commands(&vars->map_top, &vars->mem.part, loose_codes, global);
        setup_file_commands(&vars->map_file, &vars->mem.part, loose_codes, global);
    }
    
    setup_ui_commands(&vars->map_ui, &vars->mem.part, loose_codes, global);
#if FRED_INTERNAL
    setup_debug_commands(&vars->map_debug, &vars->mem.part, loose_codes, global);
#endif
    
    if (vars->hooks[hook_open_file] == 0){
        vars->hooks[hook_open_file] = default_open_file_hook;
    }
    
    if (!font_init()) return 0;
    
    vars->fonts.max = 6;
    vars->fonts.fonts = push_array(partition, Font, vars->fonts.max);
    
    {
        i32 font_count = 0;
        i32 memory_used;
        
        memory_used = 0;
        app_load_font(vars->fonts.fonts + font_count++, "liberation-mono.ttf", 17,
                      partition_current(partition),
                      &memory_used, 4, make_lit_string("liberation mono"));
        push_block(partition, memory_used);
        
        memory_used = 0;
        app_load_font(vars->fonts.fonts + font_count++, "LiberationSans-Regular.ttf", 17,
                      partition_current(partition),
                      &memory_used, 4, make_lit_string("liberation sans"));
        push_block(partition, memory_used);
        
        memory_used = 0;
        app_load_font(vars->fonts.fonts + font_count++, "Hack-Regular.ttf", 17,
                      partition_current(partition),
                      &memory_used, 4, make_lit_string("hack"));
        push_block(partition, memory_used);
        
        memory_used = 0;
        app_load_font(vars->fonts.fonts + font_count++, "CutiveMono-Regular.ttf", 17,
                      partition_current(partition),
                      &memory_used, 4, make_lit_string("cutive mono"));
        push_block(partition, memory_used);
        
        memory_used = 0;
        app_load_font(vars->fonts.fonts + font_count++, "Inconsolata-Regular.ttf", 17,
                      partition_current(partition),
                      &memory_used, 4, make_lit_string("inconsolata"));
        push_block(partition, memory_used);
        
        if (TEMP.set_extra_font){
            Extra_Font extra;
            extra.size = 17;
            TEMP.set_extra_font(&extra);
            memory_used = 0;
            if (app_load_font(vars->fonts.fonts + font_count, extra.file_name, extra.size,
                              partition_current(partition),
                              &memory_used, 4, make_string_slowly(extra.font_name))){
                ++font_count;
            }
            else{
                vars->fonts.fonts[font_count] = {};
            }
            push_block(partition, memory_used);
        }
        
        vars->fonts.count = font_count;
    }
    
    // NOTE(allen): file setup
    vars->working_set.file_index_count = 1;
    vars->working_set.file_max_count = 29;
    vars->working_set.files =
        push_array(partition, Editing_File, vars->working_set.file_max_count);
    
    file_get_dummy(&vars->working_set.files[0]);
    
    vars->working_set.table.max = vars->working_set.file_max_count * 3 / 2;
    vars->working_set.table.count = 0;
    vars->working_set.table.table =
        push_array(partition, File_Table_Entry, vars->working_set.table.max);
    memset(vars->working_set.table.table, 0, sizeof(File_Table_Entry) * vars->working_set.table.max);
    
    // NOTE(allen): clipboard setup
    vars->working_set.clipboard_max_size = ArrayCount(vars->working_set.clipboards);
    vars->working_set.clipboard_size = 0;
    vars->working_set.clipboard_current = 0;
    vars->working_set.clipboard_rolling = 0;
    
    // TODO(allen): more robust allocation solution for the clipboard
    if (clipboard.str){
        String *dest = working_set_next_clipboard_string(&vars->mem.general, &vars->working_set, clipboard.size);
        copy(dest, make_string((char*)clipboard.str, clipboard.size));
    }
    
    // NOTE(allen): delay setup
    vars->delay.max = ArrayCount(vars->delay.acts);
    
    // NOTE(allen): style setup
    app_hardcode_styles(vars);
    
    vars->palette_size = 40;
    vars->palette = push_array(partition, u32, vars->palette_size);
    
    AllowLocal(panel_count);
    panel_init(&panels[0]);

    vars->hot_dir_base = make_fixed_width_string(vars->hot_dir_base_);
    hot_directory_init(&vars->hot_directory, vars->hot_dir_base);

    vars->mini_str = make_string((char*)vars->mini_buffer, 0, 512);
    
    return 1;
}

internal Application_Step_Result
app_step(Thread_Context *thread, Key_Codes *codes,
         Key_Input_Data *input, Mouse_State *mouse,
         bool32 time_step, Render_Target *target,
         Application_Memory *memory,
         Clipboard_Contents clipboard,
         bool32 first_step, bool32 force_redraw){
    
    ProfileStart(OS_syncing);
    Application_Step_Result app_result = {};
    app_result.redraw = force_redraw;
    
    App_Vars *vars = (App_Vars*)memory->vars_memory;
    
    if (first_step || !time_step){
        app_result.redraw = 1;
    }
    
    Panel *panels = vars->layout.panels;
    Panel *active_panel = &panels[vars->layout.active_panel];
    
    // NOTE(allen): OS clipboard event handling
    if (clipboard.str){
        String *dest = working_set_next_clipboard_string(&vars->mem.general, &vars->working_set, clipboard.size);
        copy(dest, make_string((char*)clipboard.str, clipboard.size));
    }
    
    // NOTE(allen): check files are up to date
    for (i32 i = 0; i < vars->working_set.file_index_count; ++i){
        Editing_File *file = vars->working_set.files + i;
        
        if (!file->is_dummy){
            Time_Stamp time_stamp;
            time_stamp = system_file_time_stamp((u8*)make_c_str(file->source_path));
            
            if (time_stamp.success){
                file->last_sys_write_time = time_stamp.time;
                if (file->last_sys_write_time != file->last_4ed_write_time){
                    app_result.redraw = 1;
                }
            }
        }
    }
    
    // NOTE(allen): reorganizing panels on screen
    i32 prev_width = vars->layout.full_width;
    i32 prev_height = vars->layout.full_height;
    i32 prev_y_off = 0;
    i32 prev_x_off = 0;
    
    i32 y_off = 0;
    i32 x_off = 0;
    i32 full_width = vars->layout.full_width = target->width;
    i32 full_height = vars->layout.full_height = target->height;
    
    if (prev_width != full_width || prev_height != full_height ||
        prev_x_off != x_off || prev_y_off != y_off){
        layout_refit(&vars->layout, prev_x_off, prev_y_off, prev_width, prev_height);
        int view_count = vars->layout.panel_max_count;
        for (i32 view_i = 0; view_i < view_count; ++view_i){
            View *view_ = live_set_get_view(&vars->live_set, view_i);
            if (!view_->is_active) continue;
            File_View *view = view_to_file_view(view_);
            if (!view) continue;
#if BUFFER_EXPERIMENT_SCALPEL
            view_measure_wraps(&vars->mem.general, view);
#endif
            view->cursor = view_compute_cursor_from_pos(view, view->cursor.pos);
        }
        app_result.redraw = 1;
    }
    
    // NOTE(allen): collect input information
    Key_Summary key_data = {};
    for (i32 i = 0; i < input->press_count; ++i){
        key_data.keys[key_data.count++] = input->press[i];
    }
    
    for (i32 i = 0; i < input->hold_count; ++i){
        key_data.keys[key_data.count++] = input->hold[i];
    }
    
    for (i32 i = 0; i < CONTROL_KEY_COUNT; ++i){
        key_data.modifiers[i] = input->control_keys[i];
    }
    
    Mouse_Summary mouse_data;
    mouse_data.mx = mouse->x;
    mouse_data.my = mouse->y;
    
    mouse_data.l = mouse->left_button;
    mouse_data.r = mouse->right_button;
    mouse_data.press_l = mouse_data.l && !mouse->left_button_prev;
    mouse_data.press_r = mouse_data.r && !mouse->right_button_prev;
    mouse_data.release_l = !mouse_data.l && mouse->left_button_prev;
    mouse_data.release_r = !mouse_data.r && mouse->right_button_prev;
    
    mouse_data.out_of_window = mouse->out_of_window;
    mouse_data.wheel_used = (mouse->wheel != 0);
    mouse_data.wheel_amount = -mouse->wheel;
    ProfileEnd(OS_syncing);
    
    ProfileStart(hover_status);
    // NOTE(allen): detect mouse hover status
    i32 mx = mouse_data.mx;
    i32 my = mouse_data.my;
    bool32 mouse_in_edit_area = 0;
    bool32 mouse_in_margin_area = 0;
    Panel *mouse_panel = 0;
    i32 mouse_panel_i = 0;
    
    {
        Panel *panel = 0;
        bool32 in_edit_area = 0;
        bool32 in_margin_area = 0;
        i32 panel_count = vars->layout.panel_count;
        i32 panel_i;
        
        for (panel_i = 0; panel_i < panel_count; ++panel_i){
            panel = panels + panel_i;
            if (hit_check(mx, my, panel->inner)){
                in_edit_area = 1;
                break;
            }
            else if (hit_check(mx, my, panel->full)){
                in_margin_area = 1;
                break;
            }
        }
        
        mouse_in_edit_area = in_edit_area;
        mouse_in_margin_area = in_margin_area;
        if (in_edit_area || in_margin_area){
            mouse_panel = panel;
            mouse_panel_i = panel_i;
        }
    }
    
    bool32 mouse_on_divider = 0;
    bool32 mouse_divider_vertical = 0;
    i32 mouse_divider_id = 0;
    i32 mouse_divider_side = 0;
    
    if (mouse_in_margin_area){
        bool32 resize_area = 0;
        i32 divider_id = 0;
        bool32 seeking_v_divider = 0;
        i32 seeking_child_on_side = 0;
        Panel *panel = mouse_panel;
        if (mx >= panel->inner.x0 && mx < panel->inner.x1){
            seeking_v_divider = 0;
            if (my > panel->inner.y0){
                seeking_child_on_side = -1;
            }
            else{
                seeking_child_on_side = 1;
            }
        }
        else{
            seeking_v_divider = 1;
            if (mx > panel->inner.x0){
                seeking_child_on_side = -1;
            }
            else{
                seeking_child_on_side = 1;
            }
        }
        mouse_divider_vertical = seeking_v_divider;
        mouse_divider_side = seeking_child_on_side;
        
        if (vars->layout.panel_count > 1){
            i32 which_child;
            divider_id = panel->parent;
            which_child = panel->which_child;
            for (;;){
                Divider_And_ID div = layout_get_divider(&vars->layout, divider_id);
                
                if (which_child == seeking_child_on_side &&
                    div.divider->v_divider == seeking_v_divider){
                    resize_area = 1;
                    break;
                }
                
                if (divider_id == vars->layout.root){
                    break;
                }
                else{
                    divider_id = div.divider->parent;
                    which_child = div.divider->which_child;
                }
            }
            
            mouse_on_divider = resize_area;
            mouse_divider_id = divider_id;
        }
    }
    ProfileEnd(hover_status);
    
    ProfileStart(resizing);
    // NOTE(allen): panel resizing
    switch (vars->state){
    case APP_STATE_EDIT:
    {
        if (mouse_data.press_l && mouse_on_divider){
            vars->state = APP_STATE_RESIZING;
            Divider_And_ID div = layout_get_divider(&vars->layout, mouse_divider_id);
            vars->resizing.divider = div.divider;
            
            i32 min, max;
            {
                i32 mid, MIN, MAX;
                mid = div.divider->pos;
                if (mouse_divider_vertical){
                    MIN = 0;
                    MAX = MIN + vars->layout.full_width;
                }
                else{
                    MIN = 0;
                    MAX = MIN + vars->layout.full_height;
                }
                min = MIN;
                max = MAX;
                
                i32 divider_id = div.id;
                do{
                    Divider_And_ID other_div = layout_get_divider(&vars->layout, divider_id);
                    bool32 divider_match = (other_div.divider->v_divider == mouse_divider_vertical);
                    i32 pos = other_div.divider->pos;
                    if (divider_match && pos > mid && pos < max){
                        max = pos;
                    }
                    else if (divider_match && pos < mid && pos > min){
                        min = pos;
                    }
                    divider_id = other_div.divider->parent;
                }while(divider_id != -1);
                
                Temp_Memory temp = begin_temp_memory(&vars->mem.part);
                i32 *divider_stack = push_array(&vars->mem.part, i32, vars->layout.panel_count);
                i32 top = 0;
                divider_stack[top++] = div.id;
                
                while (top > 0){
                    Divider_And_ID other_div = layout_get_divider(&vars->layout, divider_stack[--top]);
                    bool32 divider_match = (other_div.divider->v_divider == mouse_divider_vertical);
                    i32 pos = other_div.divider->pos;
                    if (divider_match && pos > mid && pos < max){
                        max = pos;
                    }
                    else if (divider_match && pos < mid && pos > min){
                        min = pos;
                    }
                    if (other_div.divider->child1 != -1){
                        divider_stack[top++] = other_div.divider->child1;
                    }
                    if (other_div.divider->child2 != -1){
                        divider_stack[top++] = other_div.divider->child2;
                    }
                }
                
                end_temp_memory(temp);
            }
            
            vars->resizing.min = min;
            vars->resizing.max = max;
        }
    }break;
    
    case APP_STATE_RESIZING:
    {
        app_result.redraw = 1;
        if (mouse_data.l){
            Panel_Divider *divider = vars->resizing.divider;
            if (divider->v_divider){
                divider->pos = mx;
            }
            else{
                divider->pos = my;
            }
            
            if (divider->pos < vars->resizing.min){
                divider->pos = vars->resizing.min;
            }
            else if (divider->pos > vars->resizing.max){
                divider->pos = vars->resizing.max - 1;
            }
            
            layout_fix_all_panels(&vars->layout);
        }
        else{
            vars->state = APP_STATE_EDIT;
        }
    }break;
    }
    ProfileEnd(resizing);
    
    ProfileStart(command);
    // NOTE(allen): update active panel
    if (mouse_in_edit_area && mouse_panel != 0 && mouse_data.press_l){
        active_panel = mouse_panel;
        vars->layout.active_panel = mouse_panel_i;
        app_result.redraw = 1;
    }
    
    Command_Data command_data;
    command_data.mem = &vars->mem;
    command_data.panel = active_panel;
    command_data.view = active_panel->view;
    command_data.working_set = &vars->working_set;
    command_data.layout = &vars->layout;
    command_data.live_set = &vars->live_set;
    command_data.style = &vars->style;
    command_data.delay = &vars->delay;
    command_data.vars = vars;
    command_data.screen_width = target->width;
    command_data.screen_height = target->height;
    
    Temp_Memory param_stack_temp = begin_temp_memory(&vars->mem.part);
    command_data.part = partition_sub_part(&vars->mem.part, 16 << 10);
    
    if (first_step && vars->hooks[hook_start]){
        vars->hooks[hook_start](&command_data, app_links);
        command_data.part.pos = 0;
    }
    
    // NOTE(allen): command input to active view
    for (i32 key_i = 0; key_i < key_data.count; ++key_i){
        Command_Binding cmd = {};
        Command_Map *map = 0;
        View *view = active_panel->view;
        
        Key_Single key = get_single_key(&key_data, key_i);
        command_data.key = key;
        
        Command_Map *visited_maps[16] = {};
        i32 visited_top = 0;
        
        if (view) map = view->map;
        if (map == 0) map = &vars->map_top;
        while (map){
            cmd = map_extract(map, key);
            if (cmd.function == 0){
                if (visited_top < ArrayCount(visited_maps)){
                    visited_maps[visited_top++] = map;
                    map = map->parent;
                    for (i32 i = 0; i < visited_top; ++i){
                        if (map == visited_maps[i]){
                            map = 0;
                            break;
                        }
                    }
                }
                else map = 0;
            }
            else map = 0;
        }
        
        switch (vars->state){
        case APP_STATE_EDIT:
        {
            Handle_Command_Function *handle_command = 0;
            if (view) handle_command = view->handle_command;
            if (handle_command){
                handle_command(view, &command_data, cmd, key, codes);
                app_result.redraw = 1;
            }
            else{
                if (cmd.function){
                    cmd.function(&command_data, cmd);
                    app_result.redraw = 1;
                }
            }
        }break;
        
        case APP_STATE_RESIZING:
        {
            if (key_data.count > 0){
                vars->state = APP_STATE_EDIT;
            }
        }break;
        }
    }
    
    active_panel = panels + vars->layout.active_panel;
    ProfileEnd(command);
    
    ProfileStart(step);
    View *active_view = active_panel->view;
    
    Input_Summary dead_input = {};
    dead_input.mouse.mx = mouse_data.mx;
    dead_input.mouse.my = mouse_data.my;
    dead_input.codes = codes;
    dead_input.keys.modifiers[0] = key_data.modifiers[0];
    dead_input.keys.modifiers[1] = key_data.modifiers[1];
    dead_input.keys.modifiers[2] = key_data.modifiers[2];
    
    Input_Summary active_input = {};
    dead_input.mouse.mx = mouse_data.mx;
    dead_input.mouse.my = mouse_data.my;
    active_input.keys = key_data;
    active_input.codes = codes;
    
    // NOTE(allen): pass raw input to the panels
    {
        Panel *panel = panels;
        for (i32 panel_i = vars->layout.panel_count; panel_i > 0; --panel_i, ++panel){
            View *view_ = panel->view;
            if (view_){
                Assert(view_->do_view);
                bool32 active = (panel == active_panel);
                Input_Summary input = (active)?(active_input):(dead_input);
                if (panel == mouse_panel){
                    input.mouse = mouse_data;
                }
                if (view_->do_view(thread, view_, panel->inner, active_view,
                                   VMSG_STEP, 0, &input, &active_input)){
                    app_result.redraw = 1;
                }
            }
        }
    }
    ProfileEnd(step);
    
    ProfileStart(delayed_actions);
    if (vars->delay.count > 0){
        Style *style = &vars->style;
        Working_Set *working_set = &vars->working_set;
        Live_Views *live_set = &vars->live_set;
        Mem_Options *mem = &vars->mem;
        General_Memory *general = &vars->mem.general;
        
        i32 count = vars->delay.count;
        vars->delay.count = 0;
        
        for (i32 i = 0; i < count; ++i){
            Delayed_Action *act = vars->delay.acts + i;
            String *string = &act->string;
            Panel *panel = act->panel;
            
            switch (act->type){
            case DACT_OPEN:
            {
                command_data.view = (View*)
                    app_open_file(vars, general,panel, working_set, string, style, live_set, &command_data);
            }break;
            
            case DACT_SAVE_AS:
            {
                View *view = panel->view;
                File_View *fview = view_to_file_view(view);
                
                if (!fview && view->is_minor) fview = view_to_file_view(view->major);
                if (fview){
                    Editing_File *file = fview->file;
                    if (file && !file->is_dummy){
                        file_save_and_set_names(&vars->mem.part, file, (u8*)string->str);
                    }
                }
            }break;
            
            case DACT_SAVE:
            {
                Editing_File *file = working_set_lookup_file(working_set, *string);
                if (file && !file->is_dummy){
                    file_save(&vars->mem.part, file, (u8*)file->source_path.str);
                }
            }break;
            
            case DACT_NEW:
            {
                Get_File_Result file = working_set_get_available_file(working_set);
                file_create_empty(general, file.file, (u8*)string->str, style->font);
                table_add(&working_set->table, file.file->source_path, file.index);
                
                View *new_view = live_set_alloc_view(live_set, mem);
                view_replace_major(new_view, panel, live_set);
                
                File_View *file_view = file_view_init(new_view, &vars->delay, &vars->layout);
                command_data.view = (View*)file_view;
                view_set_file(file_view, file.file, style,
                              vars->hooks[hook_open_file], &command_data, app_links);
                new_view->map = app_get_map(vars, file.file->base_map_id);
#if BUFFER_EXPERIMENT_SCALPEL
                if (file.file->tokens_exist) file_first_lex_parallel(general, file.file);
#endif
            }break;
            
            case DACT_SWITCH:
            {
                Editing_File *file = working_set_lookup_file(working_set, *string);
                if (file){
                    View *new_view = live_set_alloc_view(live_set, mem);
                    view_replace_major(new_view, panel, live_set);
                    
                    File_View *file_view = file_view_init(new_view, &vars->delay, &vars->layout);
                    command_data.view = (View*)file_view;
                    view_set_file(file_view, file, style,
                                  vars->hooks[hook_open_file], &command_data, app_links);
                    new_view->map = app_get_map(vars, file->base_map_id);
                }
            }break;
            
            case DACT_KILL:
            {
                Editing_File *file = working_set_lookup_file(working_set, *string);
                if (file){
                    table_remove(&working_set->table, file->source_path);
                    kill_buffer(general, file, live_set, &vars->layout);
                }
            }break;
            
            case DACT_TRY_KILL:
            {
                Editing_File *file = working_set_lookup_file(working_set, *string);
                if (file){
                    switch (buffer_get_sync(file)){
                    case SYNC_BEHIND_OS:
                    case SYNC_GOOD:
                    {
                        table_remove(&working_set->table, file->source_path);
                        kill_buffer(general, file, live_set, &vars->layout);
                        view_remove_minor(panel, live_set);
                    }break;
                    
                    case SYNC_UNSAVED:
                    {
                        View *new_view = live_set_alloc_view(live_set, mem);
                        view_replace_minor(new_view, panel, live_set);
                        
                        new_view->map = &vars->map_ui;
                        Interactive_View *int_view = 
                            interactive_view_init(new_view, &vars->hot_directory, style,
                                                  working_set, &vars->delay);
                        int_view->interaction = INTV_SURE_TO_KILL_INTER;
                        int_view->action = INTV_SURE_TO_KILL;
                        copy(&int_view->query, "Are you sure?");
                        copy(&int_view->dest, file->live_name);
                    }break;
                    
                    default: Assert(!"invalid path");
                    }
                }
            }break;
            
            case DACT_CLOSE_MINOR:
            {
                view_remove_minor(panel, live_set);
            }break;
            
            case DACT_CLOSE_MAJOR:
            {
                view_remove_major(panel, live_set);
            }break;
            
            case DACT_THEME_OPTIONS:
            {
                open_theme_options(vars, live_set, mem, panel);
            }break;
            }
        }
    }
    ProfileEnd(delayed_actions);
    
    end_temp_memory(param_stack_temp);
    
    ProfileStart(resize);
    // NOTE(allen): send resize messages to panels that have changed size
    {
        Panel *panel = panels;
        for (i32 panel_i = vars->layout.panel_count; panel_i > 0; --panel_i, ++panel){
            i32_Rect prev = panel->prev_inner;
            i32_Rect inner = panel->inner;
            if (prev.x0 != inner.x0 || prev.y0 != inner.y0 ||
                prev.x1 != inner.x1 || prev.y1 != inner.y1){
                View *view = panel->view;
                if (view){
                    view->do_view(thread, view, inner, active_view,
                                  VMSG_RESIZE, 0, &dead_input, &active_input);
                    view = (view->is_minor)?view->major:0;
                    if (view){
                        view->do_view(thread, view, inner, active_view,
                                      VMSG_RESIZE, 0, &dead_input, &active_input);
                    }
                }
            }
            panel->prev_inner = inner;
        }
    }
    ProfileEnd(resize);
    
    ProfileStart(style_change);
    // NOTE(allen): send style change messages if the style has changed
    if (vars->style.font_changed){
        vars->style.font_changed = 0;
        
#if BUFFER_EXPERIMENT_SCALPEL
        Editing_File *file = vars->working_set.files;
        for (i32 i = vars->working_set.file_index_count; i > 0; --i, ++file){
            if (file->buffer.data && !file->is_dummy){
                file_measure_widths(&vars->mem.general, file, vars->style.font);
            }
        }
#endif
        
        Panel *panel = panels;
        for (i32 panel_i = vars->layout.panel_count; panel_i > 0; --panel_i, ++panel){
            View *view = panel->view;
            if (view){
                view->do_view(thread, view, panel->inner, active_view,
                              VMSG_STYLE_CHANGE, 0, &dead_input, &active_input);
                view = (view->is_minor)?view->major:0;
                if (view){
                    view->do_view(thread, view, panel->inner, active_view,
                                  VMSG_STYLE_CHANGE, 0, &dead_input, &active_input);
                }
            }
        }
    }
    ProfileEnd(style_change);
    
    ProfileStart(redraw);
    if (mouse_panel != vars->prev_mouse_panel) app_result.redraw = 1;
    if (app_result.redraw){
        target->clip_top = -1;
        draw_push_clip(target, rect_from_target(target));
        
        // NOTE(allen): render the panels
        Panel *panel = panels;
        for (i32 panel_i = vars->layout.panel_count; panel_i > 0; --panel_i, ++panel){
            i32_Rect full = panel->full;
            i32_Rect inner = panel->inner;
            
            View *view_ = panel->view;
            Style *style = &vars->style;
            
            bool32 active = (panel == active_panel);
            u32 back_color = style->main.back_color;
            draw_rectangle(target, full, back_color);
            
            if (view_){
                Assert(view_->do_view);
                draw_push_clip(target, panel->inner);
                view_->do_view(thread, view_, panel->inner, active_view,
                               VMSG_DRAW, target, &dead_input, &active_input);
                draw_pop_clip(target);
            }
            
            u32 margin_color;
            if (active){
                margin_color = style->main.margin_active_color;
            }
            else if (panel == mouse_panel){
                margin_color = style->main.margin_hover_color;
            }
            else{
                margin_color = style->main.margin_color;
            }
            draw_rectangle(target, i32R(full.x0, full.y0, full.x1, inner.y0), margin_color);
            draw_rectangle(target, i32R(full.x0, inner.y1, full.x1, full.y1), margin_color);
            draw_rectangle(target, i32R(full.x0, inner.y0, inner.x0, inner.y1), margin_color);
            draw_rectangle(target, i32R(inner.x1, inner.y0, full.x1, inner.y1), margin_color);
        }
    }
    ProfileEnd(redraw);
    
    ProfileStart(get_cursor);
    // NOTE(allen): get cursor type
    if (mouse_in_edit_area){
        View *view = mouse_panel->view;
        if (view){
            app_result.mouse_cursor_type = view->mouse_cursor_type;
        }
        else{
            app_result.mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
        }
    }
    else if (mouse_in_margin_area){
        if (mouse_on_divider){
            if (mouse_divider_vertical){
                app_result.mouse_cursor_type = APP_MOUSE_CURSOR_LEFTRIGHT;
            }
            else{
                app_result.mouse_cursor_type = APP_MOUSE_CURSOR_UPDOWN;
            }
        }
        else{
            app_result.mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
        }
    }
    vars->prev_mouse_panel = mouse_panel;
    ProfileEnd(get_cursor);
    
    return app_result;
}

// BOTTOM

