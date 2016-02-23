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

struct CLI_Process{
    CLI_Handles cli;
    Editing_File *out_file;
};

struct CLI_List{
    CLI_Process *procs;
    i32 count, max;
};

struct Sys_App_Binding{
    i32 sys_id;
    i32 app_id;
};

struct Complete_State{
    Search_Set set;
    Search_Iter iter;
    Table hits;
    String_Space str;
    i32 word_start, word_end;
    b32 initialized;
};

struct App_Vars{
    Mem_Options mem;

    App_Settings settings;
    
    Command_Map map_top;
    Command_Map map_file;
    Command_Map map_ui;
#if FRED_INTERNAL
    Command_Map map_debug;
#endif
    Command_Map *user_maps;
    i32 *map_id_table;
    i32 user_map_count;
    Command_Binding prev_command;
    
    Sys_App_Binding *sys_app_bindings;
    i32 sys_app_count, sys_app_max;
    
    Custom_Command_Function *hooks[hook_type_count];
    
    Font_Set *font_set;
    
    Style style;
    Style_Library styles;
    u32 *palette;
    i32 palette_size;
    
    Editing_Layout layout;
    Live_Views live_set;
    Working_Set working_set;

    char hot_dir_base_[256];
    Hot_Directory hot_directory;

    CLI_List cli_processes;
    
    char query_[256];
    char dest_[256];
    
    Delay delay;

    String mini_str;
    u8 mini_buffer[512];
    
    App_State state;
    App_State_Resizing resizing;
    Complete_State complete_state;
    Panel *prev_mouse_panel;

    Custom_API config_api;
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
    if (mapid < mapid_global) map = vars->user_maps + mapid;
    else if (mapid == mapid_global) map = &vars->map_top;
    else if (mapid == mapid_file) map = &vars->map_file;
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
#define USE_EXCHANGE(n) Exchange *n = command->exchange
#define USE_FONT_SET(n) Font_Set *n = command->vars->font_set;

#define REQ_VIEW(n) View *n = command->view; if (!n) return
#define REQ_FILE_VIEW(n) File_View *n = view_to_file_view(command->view); if (!n) return
#define REQ_OPEN_FILE_VIEW(n) File_View *n = view_to_file_view(command->view); if (!n || n->locked) return
#define REQ_FILE_HISTORY(n,v) Editing_File *n = (v)->file; if (!n || !buffer_good(&n->state.buffer) || n->state.is_dummy || !n->state.undo.undo.edits) return
#define REQ_FILE_LOADING(n,v) Editing_File *n = (v)->file; if (!n || n->state.is_dummy) return
#define REQ_FILE(n,v) Editing_File *n = (v)->file; if (!n || !buffer_good(&n->state.buffer) || n->state.is_dummy) return
#define REQ_COLOR_VIEW(n) Color_View *n = view_to_color_view(command->view); if (!n) return
#define REQ_DBG_VIEW(n) Debug_View *n = view_to_debug_view(command->view); if (!n) return

#define COMMAND_DECL(n) internal void command_##n(System_Functions *system, Command_Data *command, Command_Binding binding)
#define COMPOSE_DECL(n) internal void n(System_Functions *system, Command_Data *command, Command_Binding binding)

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
    Exchange *exchange;
    
    i32 screen_width, screen_height;
    Key_Event_Data key;
    
    Partition part;
    System_Functions *system;
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
    REQ_OPEN_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    u8 character = (u8)command->key.character;
    char str_space[2];
    String string = make_string(str_space, 2);
    str_space[0] = character;
    string.size = 1;
    
    i32 pos;
    pos = view->cursor.pos;
    i32 next_cursor_pos = view->cursor.pos + string.size;
    view_replace_range(system, mem, view, layout, pos, pos, string.str, string.size, next_cursor_pos);
    view_cursor_move(view, next_cursor_pos);
    file->state.cursor_pos = view->cursor.pos;
}

COMMAND_DECL(seek_whitespace_right){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_whitespace_right(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
#endif
}

COMMAND_DECL(seek_whitespace_left){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_whitespace_left(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
#endif
}

COMMAND_DECL(seek_whitespace_up){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_whitespace_up(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
#endif
}

COMMAND_DECL(seek_whitespace_down){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
        
    i32 pos = buffer_seek_whitespace_down(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
#endif
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
    
    if (file->state.tokens_complete){
        i32 pos = seek_token_left(&file->state.token_stack, view->cursor.pos);
        view_cursor_move(view, pos);
    }
}

COMMAND_DECL(seek_token_right){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    if (file->state.tokens_complete){
        i32 pos = seek_token_right(&file->state.token_stack, view->cursor.pos);
        view_cursor_move(view, pos);
    }
}

COMMAND_DECL(seek_white_or_token_right){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 token_pos, white_pos;
    token_pos = file->state.buffer.size;
    if (file->state.tokens_complete){
        token_pos = seek_token_right(&file->state.token_stack, view->cursor.pos);
    }
    white_pos = buffer_seek_whitespace_right(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, Min(token_pos, white_pos));
#endif
}

COMMAND_DECL(seek_white_or_token_left){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 token_pos, white_pos;
    token_pos = file->state.buffer.size;
    if (file->state.tokens_complete){
        token_pos = seek_token_left(&file->state.token_stack, view->cursor.pos);
    }
    white_pos = buffer_seek_whitespace_left(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, Max(token_pos, white_pos));
#endif
}

COMMAND_DECL(seek_alphanumeric_right){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_alphanumeric_right(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
#endif
}

COMMAND_DECL(seek_alphanumeric_left){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_alphanumeric_left(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
#endif
}

COMMAND_DECL(seek_alphanumeric_or_camel_right){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = buffer_seek_alphanumeric_or_camel_right(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
#endif
}

COMMAND_DECL(seek_alphanumeric_or_camel_left){
#if BUFFER_EXPERIMENT_SCALPEL <= 3
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = buffer_seek_alphanumeric_or_camel_left(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
#endif
}

COMMAND_DECL(search){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(fixed, view);
    USE_VARS(vars);
    
    view_set_widget(view, FWIDG_SEARCH);
    view->isearch.str = vars->mini_str;
    view->isearch.reverse = 0;
    view->isearch.pos = view->cursor.pos - 1;
}

COMMAND_DECL(rsearch){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(fixed, view);
    USE_VARS(vars);
    
    view_set_widget(view, FWIDG_SEARCH);
    view->isearch.str = vars->mini_str;
    view->isearch.reverse = 1;
    view->isearch.pos = view->cursor.pos + 1;
}

COMMAND_DECL(word_complete){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    USE_VARS(vars);
    USE_WORKING_SET(working_set);
    
    Partition *part = &mem->part;
    General_Memory *general = &mem->general;
    Complete_State *complete_state = &vars->complete_state;
    Search_Range *ranges;
    Search_Match match;
    
    Temp_Memory temp;
    
    Buffer_Type *buffer;
    Buffer_Backify_Type loop;
    char *data;
    i32 end;
    i32 size_of_buffer;
    
    i32 cursor_pos, word_start, word_end;
    char c;
    
    char *spare;
    i32 size;
    
    i32 buffer_count, i, j;
    Editing_File *file_ptr;
    
    i32 match_size;
    b32 do_init = 0;
    
    buffer = &file->state.buffer;
    size_of_buffer = buffer_size(buffer);
    
    if (vars->prev_command.function != command_word_complete){
        do_init = 1;
    }
    
    if (complete_state->initialized == 0){
        do_init = 1;
    }

    if (do_init){
        word_end = view->cursor.pos;
        word_start = word_end;
        cursor_pos = word_end - 1;

        // TODO(allen): macros for these buffer loops and some method of breaking out of them.
        for (loop = buffer_backify_loop(buffer, cursor_pos, 0);
            buffer_backify_good(&loop);
            buffer_backify_next(&loop)){
            end = loop.absolute_pos;
            data = loop.data - loop.absolute_pos;
            for (; cursor_pos >= end; --cursor_pos){
                c = data[cursor_pos];
                if (char_is_alpha(c)){
                    word_start = cursor_pos;
                }
                else if (!char_is_numeric(c)){
                    goto double_break;
                }
            }
        }
        // TODO(allen): figure out how labels are scoped.
        double_break:;
        
        size = word_end - word_start;
        
        if (size == 0){
            complete_state->initialized = 0;
            return;
        }
        
        complete_state->initialized = 1;
        search_iter_init(general, &complete_state->iter, size);
        buffer_stringify(buffer, word_start, word_end, complete_state->iter.word.str);
        complete_state->iter.word.size = size;
        
        buffer_count = working_set->file_index_count;
        search_set_init(general, &complete_state->set, buffer_count + 1);
        ranges = complete_state->set.ranges;
        ranges[0].buffer = buffer;
        ranges[0].start = 0;
        ranges[0].size = word_start;
        
        ranges[1].buffer = buffer;
        ranges[1].start = word_end;
        ranges[1].size = size_of_buffer - word_end;
        
        file_ptr = working_set->files;
        for (i = 0, j = 2; i < buffer_count; ++i, ++file_ptr){
            if (file_ptr != file && !file_ptr->state.is_dummy && file_is_ready(file_ptr)){
                ranges[j].buffer = &file_ptr->state.buffer;
                ranges[j].start = 0;
                ranges[j].size = buffer_size(ranges[j].buffer); 
                ++j;
            }
        }
        complete_state->set.count = j;
        
        search_hits_init(general, &complete_state->hits, &complete_state->str, 100, Kbytes(4));
        search_hit_add(general, &complete_state->hits, &complete_state->str,
            complete_state->iter.word.str, complete_state->iter.word.size);
        
        complete_state->word_start = word_start;
        complete_state->word_end = word_end;
    }
    else{
        word_start = complete_state->word_start;
        word_end = complete_state->word_end;
        size = complete_state->iter.word.size;
    }

    if (size > 0){
        for (;;){
            match = search_next_match(part, &complete_state->set, &complete_state->iter);

            if (match.found_match){
                temp = begin_temp_memory(part);
                match_size = match.end - match.start;
                spare = (char*)push_array(part, char, match_size);
                buffer_stringify(match.buffer, match.start, match.end, spare);

                if (search_hit_add(general, &complete_state->hits, &complete_state->str, spare, match_size)){
                    view_replace_range(system, mem, view, layout, word_start, word_end, spare, match_size, word_end);

                    complete_state->word_end = word_start + match_size;
                    complete_state->set.ranges[1].start = word_start + match_size;
                    break;
                }
                end_temp_memory(temp);
            }
            else{
                complete_state->iter.pos = 0;
                complete_state->iter.i = 0;

                search_hits_init(general, &complete_state->hits, &complete_state->str, 100, Kbytes(4));
                search_hit_add(general, &complete_state->hits, &complete_state->str,
                    complete_state->iter.word.str, complete_state->iter.word.size);

                match_size = complete_state->iter.word.size;
                view_replace_range(system, mem, view, layout, word_start, word_end,
                    complete_state->iter.word.str, match_size, word_end);

                complete_state->word_end = word_start + match_size;
                complete_state->set.ranges[1].start = word_start + match_size;
                break;
            }
        }
    }
}

COMMAND_DECL(goto_line){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(fixed, view);
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
        clipboard_copy(system, &mem->general, working_set, range, file);
    }
}

COMMAND_DECL(cut){
    ProfileMomentFunction();
    REQ_OPEN_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_WORKING_SET(working_set);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    Range range = get_range(view->cursor.pos, view->mark);
    if (range.start < range.end){
        i32 next_cursor_pos = range.start;

        clipboard_copy(system, &mem->general, working_set, range, file);
        view_replace_range(system, mem, view, layout, range.start, range.end, 0, 0, next_cursor_pos);
        
        view->mark = range.start;
        view_cursor_move(view, next_cursor_pos);
    }
}

COMMAND_DECL(paste){
    ProfileMomentFunction();
    REQ_OPEN_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_WORKING_SET(working_set);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    Panel *current_panel;
    String *src;
    File_View *current_view;
    i32 pos_left, next_cursor_pos;
    i32 panel_count;
    i32 i;
    
    if (working_set->clipboard_size > 0){
        view->next_mode.rewrite = 1;
        
        src = working_set_clipboard_head(working_set);
        pos_left = view->cursor.pos;

        next_cursor_pos = pos_left+src->size;
        view_replace_range(system, mem, view, layout, pos_left, pos_left, src->str, src->size, next_cursor_pos);
        
        view_cursor_move(view, next_cursor_pos);
        view->mark = pos_left;
        
        current_panel = layout->panels;
        panel_count = layout->panel_count;
        for (i = 0; i < panel_count; ++i, ++current_panel){
            current_view = view_to_file_view(current_panel->view);
            
            if (current_view && current_view->file == file){
                view_post_paste_effect(current_view, 20, pos_left, src->size,
                                       current_view->style->main.paste_color);
            }
        }
    }
}

COMMAND_DECL(paste_next){
    ProfileMomentFunction();
    REQ_OPEN_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_WORKING_SET(working_set);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    if (working_set->clipboard_size > 0 && view->mode.rewrite){
        view->next_mode.rewrite = 1;
        
        Range range = get_range(view->mark, view->cursor.pos);
        String *src = working_set_clipboard_roll_down(working_set);
        i32 next_cursor_pos = range.start+src->size;
        view_replace_range(system,
                           mem, view, layout, range.start, range.end,
                           src->str, src->size, next_cursor_pos);
        
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
        command_paste(system, command, binding);
    }
}

COMMAND_DECL(delete_range){
    ProfileMomentFunction();
    REQ_OPEN_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    Range range = get_range(view->cursor.pos, view->mark);
    if (range.start < range.end){
        i32 next_cursor_pos = range.start;
        view_replace_range(system, mem, view, layout, range.start, range.end,
                           0, 0, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
        view->mark = range.start;
    }
}

COMMAND_DECL(timeline_scrub){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE_HISTORY(file, view);
    
    view_set_widget(view, FWIDG_TIMELINES);
    view->widget.timeline.undo_line = 1;
    view->widget.timeline.history_line = 1;
}

COMMAND_DECL(undo){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE_HISTORY(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    view_undo(system, mem, layout, view);
}

COMMAND_DECL(redo){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE_HISTORY(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    view_redo(system, mem, layout, view);
}

COMMAND_DECL(increase_rewind_speed){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE_HISTORY(file, view);

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
    REQ_FILE_HISTORY(file, view);
    
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
    REQ_FILE_HISTORY(file, view);
    
    view->rewind_speed = 0;
}

COMMAND_DECL(history_backward){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE_HISTORY(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    view_history_step(system, mem, layout, view, hist_backward);
}

COMMAND_DECL(history_forward){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE_HISTORY(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    view_history_step(system, mem, layout, view, hist_forward);
}

#if UseFileHistoryDump
COMMAND_DECL(save_history){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_MEM(mem);
    
    file_dump_history(system, mem, file, "history_data.hst");
}
#endif

COMMAND_DECL(interactive_new){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_MEM(mem);
    USE_WORKING_SET(working_set);
    USE_STYLE(style);
    USE_DELAY(delay);
    USE_EXCHANGE(exchange);
    USE_FONT_SET(font_set);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(system, exchange, new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Interactive_View *int_view =
        interactive_view_init(system, new_view, &vars->hot_directory,
                              style, working_set,
                              font_set, delay);
    int_view->interaction = INTV_SYS_FILE_LIST;
    int_view->action = INTV_NEW;
    copy(&int_view->query, "New: ");
}

internal void
app_push_file_binding(App_Vars *vars, int sys_id, int app_id){
    Sys_App_Binding binding;
    Assert(vars->sys_app_count < vars->sys_app_max);
    binding.sys_id = sys_id;
    binding.app_id = app_id;
    vars->sys_app_bindings[vars->sys_app_count++] = binding;
}

struct App_Open_File_Result{
    Editing_File *file;
    b32 is_new;
};

internal App_Open_File_Result
app_open_file_background(App_Vars *vars, Exchange *exchange, Working_Set *working_set, String filename){
    Get_File_Result file;
    i32 file_id;
    App_Open_File_Result result = {};
    
    result.file = working_set_contains(working_set, filename);
    if (result.file == 0){
        file = working_set_get_available_file(working_set);
        if (file.file){
            file_id = exchange_request_file(exchange, filename.str, filename.size);
            if (file_id){
                result.is_new = 1;
                result.file = file.file;
                file_init_strings(result.file);
                file_set_name(working_set, result.file, filename.str);
                file_set_to_loading(result.file);
                table_add(&working_set->table, result.file->name.source_path, file.index);
                
                app_push_file_binding(vars, file_id, file.index);
            }
            else{
                file_get_dummy(file.file);
            }
        }
    }
    
    return(result);
}

internal File_View*
app_open_file(System_Functions *system, App_Vars *vars, Exchange *exchange,
              Live_Views *live_set, Working_Set *working_set, Panel *panel,
              Command_Data *command_data, String filename){
    App_Open_File_Result file;
    Mem_Options *mem = &vars->mem;
    File_View *result = 0;
    
    file = app_open_file_background(vars, exchange, working_set, filename);
    
    if (file.file){
        Style *style = command_data->style;
        
        View *new_view = live_set_alloc_view(live_set, mem);
        view_replace_major(system, exchange, new_view, panel, live_set);
        
        File_View *file_view = file_view_init(new_view, &vars->layout);
        result = file_view;
        
        View *old_view = command_data->view;
        command_data->view = new_view;
        
        Partition old_part = command_data->part;
        Temp_Memory temp = begin_temp_memory(&mem->part);
        command_data->part = partition_sub_part(&mem->part, Kbytes(16));
        
        view_set_file(system, file_view, file.file, vars->font_set,
                      style, vars->hooks[hook_open_file], command_data,
                      &app_links);
        
        command_data->part = old_part;
        end_temp_memory(temp);
        command_data->view = old_view;
        
        new_view->map = app_get_map(vars, file.file->settings.base_map_id);
    }
    
    return(result);
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
    USE_EXCHANGE(exchange);
    USE_FONT_SET(font_set);
    
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
    
    if (filename){
        String string = make_string(filename, filename_len);
        app_open_file(system, vars, exchange,
                      live_set, working_set, panel,
                      command, string);
    }
    else{
        View *new_view = live_set_alloc_view(live_set, mem);
        view_replace_minor(system, exchange, new_view, panel, live_set);
        
        new_view->map = &vars->map_ui;
        Interactive_View *int_view =
            interactive_view_init(system, new_view, &vars->hot_directory,
                                  style, working_set, font_set, delay);
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
    USE_EXCHANGE(exchange);
    USE_WORKING_SET(working_set);
    USE_VARS(vars);
    USE_STYLE(style);
    
    i32 file_id = exchange_request_file(exchange, expand_str(file->name.source_path));
    i32 index = 0;
    if (file_id){
        file_set_to_loading(file);
        index = working_set_get_index(working_set, file);
        app_push_file_binding(vars, file_id, index);

        view_set_file(system, view, file, vars->font_set, style,
                      vars->hooks[hook_open_file], command, &app_links);
    }
    else{
        // TODO(allen): feedback message
    }
}

COMMAND_DECL(save){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_DELAY(delay);
    USE_PANEL(panel);
    
    delayed_action(delay, DACT_SAVE, file->name.source_path, panel);
#if 0
    String *file_path = &file->name.source_path;
    if (file_path->size > 0){
        i32 sys_id = file_save(system, exchange, mem, file, file_path->str);
        app_push_file_binding(vars, sys_id, get_file_id(working_set, file));
    }
#endif
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
    USE_EXCHANGE(exchange);
    USE_FONT_SET(font_set);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(system, exchange, new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Interactive_View *int_view =
        interactive_view_init(system, new_view, &vars->hot_directory, style,
                              working_set, font_set, delay);
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
    USE_EXCHANGE(exchange);
    USE_FONT_SET(font_set);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(system, exchange, new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Interactive_View *int_view = 
        interactive_view_init(system, new_view, &vars->hot_directory, style,
                              working_set, font_set, delay);
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
    USE_EXCHANGE(exchange);
    USE_FONT_SET(font_set);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(system, exchange, new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Interactive_View *int_view = 
        interactive_view_init(system, new_view, &vars->hot_directory, style,
                              working_set, font_set, delay);
    int_view->interaction = INTV_LIVE_FILE_LIST;
    int_view->action = INTV_KILL;
    copy(&int_view->query, "Kill File: ");
}

COMMAND_DECL(kill_buffer){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_DELAY(delay);
    
    delayed_action(delay, DACT_TRY_KILL, file->name.live_name, view->view_base.panel);
}

COMMAND_DECL(toggle_line_wrap){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    Relative_Scrolling scrolling = view_get_relative_scrolling(view);
    if (view->unwrapped_lines){
        view->unwrapped_lines = 0;
        file->settings.unwrapped_lines = 0;
        view->target_x = 0;
        view->cursor =
            view_compute_cursor_from_pos(view, view->cursor.pos);
        view->preferred_x = view->cursor.wrapped_x;
    }
    else{
        view->unwrapped_lines = 1;
        file->settings.unwrapped_lines = 1;
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
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_MEM(mem);
    
    if (file->settings.tokens_exist){
        file_kill_tokens(system, &mem->general, file);
    }
    else{
        file_first_lex_parallel(system, &mem->general, file);
    }
#endif
}

internal void
case_change_range(System_Functions *system,
                  Mem_Options *mem, File_View *view, Editing_File *file,
                  u8 a, u8 z, u8 char_delta){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    Range range = get_range(view->cursor.pos, view->mark);
    if (range.start < range.end){
        Edit_Step step = {};
        step.type = ED_NORMAL;
        step.edit.start = range.start;
        step.edit.end = range.end;
        step.edit.len = range.end - range.start;
        
        if (file->state.still_lexing)
            system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        
        file_update_history_before_edit(mem, file, step, 0, hist_normal);
        
        u8 *data = (u8*)file->state.buffer.data;
        for (i32 i = range.start; i < range.end; ++i){
            if (data[i] >= a && data[i] <= z){
                data[i] += char_delta;
            }
        }

        if (file->state.token_stack.tokens)
            file_relex_parallel(system, mem, file, range.start, range.end, 0);
    }
#endif
}

COMMAND_DECL(to_uppercase){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_MEM(mem);
    case_change_range(system, mem, view, file, 'a', 'z', (u8)('A' - 'a'));
}

COMMAND_DECL(to_lowercase){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_MEM(mem);
    case_change_range(system, mem, view, file, 'A', 'Z', (u8)('a' - 'A'));
}

COMMAND_DECL(clean_all_lines){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    view_clean_whitespace(system, mem, view, layout);
}

COMMAND_DECL(eol_dosify){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    file->settings.dos_write_mode = 1;
    file->state.last_4ed_edit_time = system->time();
}

COMMAND_DECL(eol_nixify){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    
    file->settings.dos_write_mode = 0;
    file->state.last_4ed_edit_time = system->time();
}

COMMAND_DECL(auto_tab_range){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);

    if (file->state.token_stack.tokens && file->state.tokens_complete){
        Range range = get_range(view->cursor.pos, view->mark);
        view_auto_tab_tokens(system, mem, view, layout, range.start, range.end, 1);
    }
}

COMMAND_DECL(auto_tab_line_at_cursor){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);

    if (file->state.token_stack.tokens && file->state.tokens_complete){
        i32 pos = view->cursor.pos;
        view_auto_tab_tokens(system, mem, view, layout, pos, pos, 0);
    }
}

COMMAND_DECL(auto_tab_whole_file){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);

    if (file->state.token_stack.tokens && file->state.tokens_complete){
        view_auto_tab_tokens(system, mem, view, layout, 0, buffer_size(&file->state.buffer), 1);
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
    USE_EXCHANGE(exchange);
    
    if (layout->panel_count > 1){
        if (view){
            live_set_free_view(system, exchange, command->live_set, view);
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
    
    i32 size = buffer_size(&file->state.buffer);
    i32 pos = view->cursor.pos;
    if (pos < size) ++pos;
    view_cursor_move(view, pos);
}

COMMAND_DECL(delete){
    ProfileMomentFunction();
    REQ_OPEN_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);

    i32 size = buffer_size(&file->state.buffer);
    i32 cursor_pos = view->cursor.pos;
    if (0 < size && cursor_pos < size){
        i32 start, end;
        start = cursor_pos;
        end = cursor_pos+1;
        
        i32 shift = (end - start);
        Assert(shift > 0);
        
        i32 next_cursor_pos = start;
        view_replace_range(system, mem, view, layout, start, end, 0, 0, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
    }
}

COMMAND_DECL(backspace){
    ProfileMomentFunction();
    REQ_OPEN_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_LAYOUT(layout);
    USE_MEM(mem);
    
    i32 size = buffer_size(&file->state.buffer);
    i32 cursor_pos = view->cursor.pos;
    if (cursor_pos > 0 && cursor_pos <= size){
        i32 start, end;
        end = cursor_pos;
        start = cursor_pos-1;
        
        i32 shift = (end - start);
        Assert(shift > 0);
        
        i32 next_cursor_pos = view->cursor.pos - shift;
        view_replace_range(system, mem, view, layout, start, end, 0, 0, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
    }
}

COMMAND_DECL(move_up){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_FONT_SET(font_set);
    
    f32 font_height = (f32)get_font_info(font_set, view->style->font_id)->height;
    f32 cy = view_get_cursor_y(view)-font_height;
    f32 px = view->preferred_x;
    if (cy >= 0){
        view->cursor = view_compute_cursor_from_xy(view, px, cy);
        file->state.cursor_pos = view->cursor.pos;
    }
}

COMMAND_DECL(move_down){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE(file, view);
    USE_FONT_SET(font_set);

    f32 font_height = (f32)get_font_info(font_set, view->style->font_id)->height;
    f32 cy = view_get_cursor_y(view)+font_height;
    f32 px = view->preferred_x;
    view->cursor = view_compute_cursor_from_xy(view, px, cy);
    file->state.cursor_pos = view->cursor.pos;
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
    
    f32 height = view_compute_height(view);
    f32 max_target_y = view_compute_max_target_y(view);
    
    view->target_y += height;
    if (view->target_y > max_target_y) view->target_y = max_target_y;

    view->cursor = view_compute_cursor_from_xy(
        view, 0, view->target_y + (height - view->font_height)*.5f);
}

COMMAND_DECL(page_up){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    
    f32 height = view_compute_height(view);
    
    view->target_y -= height;
    if (view->target_y < 0) view->target_y = 0;
    
    view->cursor = view_compute_cursor_from_xy(
        view, 0, view->target_y + (height - view->font_height)*.5f);
}

inline void
open_theme_options(System_Functions *system, Exchange *exchange,
                   App_Vars *vars, Live_Views *live_set, Mem_Options *mem, Panel *panel){
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(system, exchange, new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Color_View *color_view = color_view_init(new_view, &vars->working_set);
    color_view->hot_directory = &vars->hot_directory;
    color_view->main_style = &vars->style;
    color_view->styles = &vars->styles;
    color_view->palette = vars->palette;
    color_view->palette_size = vars->palette_size;
    color_view->font_set = vars->font_set;
}

COMMAND_DECL(open_color_tweaker){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_MEM(mem);
    USE_PANEL(panel);
    USE_EXCHANGE(exchange);
    
    open_theme_options(system, exchange, vars, live_set, mem, panel);
}

inline void
open_config_options(System_Functions *system, Exchange *exchange,
                   App_Vars *vars, Live_Views *live_set, Mem_Options *mem, Panel *panel){
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(system, exchange, new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    config_view_init(new_view, &vars->style,
                     &vars->working_set, vars->font_set,
                     &vars->settings);
}

COMMAND_DECL(open_config){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_MEM(mem);
    USE_PANEL(panel);
    USE_EXCHANGE(exchange);
    
    open_config_options(system, exchange, vars, live_set, mem, panel);
}

COMMAND_DECL(open_menu){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_MEM(mem);
    USE_WORKING_SET(working_set);
    USE_STYLE(style);
    USE_EXCHANGE(exchange);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_minor(system, exchange, new_view, panel, live_set);
    
    new_view->map = &vars->map_ui;
    Menu_View *menu_view = menu_view_init(new_view, style, working_set,
                                          &vars->delay, vars->font_set);
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
    USE_EXCHANGE(exchange);
    
    View *new_view = live_set_alloc_view(live_set, mem);
    view_replace_major(system, exchange, new_view, panel, live_set);
    
    new_view->map = &vars->map_debug;
    Debug_View *debug_view = debug_view_init(new_view);
    debug_view->font_id = style->font_id;
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

#endif

COMMAND_DECL(close_minor_view){
    ProfileMomentFunction();
    REQ_VIEW(view);
    USE_PANEL(panel);
    USE_LIVE_SET(live_set);
    USE_EXCHANGE(exchange);
    
    view_remove_minor(system, exchange, panel, live_set);
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
    if (binding.custom) binding.custom(command, &app_links);
}

COMMAND_DECL(set_settings){
    ProfileMomentFunction();
    REQ_FILE_VIEW(view);
    REQ_FILE_LOADING(file, view);
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
#if BUFFER_EXPERIMENT_SCALPEL <= 0
            int v = dynamic_to_bool(&param->param.value);
            if (file->settings.tokens_exist){
                if (!v) file_kill_tokens(system, &mem->general, file);
            }
            else{
                if (v) file_first_lex_parallel(system, &mem->general, file);
            }
#endif
        }break;
        
        case par_wrap_lines:
        {
            int v = dynamic_to_bool(&param->param.value);
            if (view->unwrapped_lines){
                if (v){
                    view->unwrapped_lines = 0;
                    file->settings.unwrapped_lines = 0;
                    
                    if (!file->state.is_loading){
                        Relative_Scrolling scrolling = view_get_relative_scrolling(view);
                        view->target_x = 0;
                        view->cursor =
                            view_compute_cursor_from_pos(view, view->cursor.pos);
                        view_set_relative_scrolling(view, scrolling);
                    }
                }
            }
            else{
                if (!v){
                    view->unwrapped_lines = 1;
                    file->settings.unwrapped_lines = 1;
                    
                    if (!file->state.is_loading){
                        Relative_Scrolling scrolling = view_get_relative_scrolling(view);
                        view->cursor =
                            view_compute_cursor_from_pos(view, view->cursor.pos);
                        view_set_relative_scrolling(view, scrolling);
                    }
                }
            }
        }break;
        
        case par_key_mapid:
        {
            int v = dynamic_to_int(&param->param.value);
            if (v == mapid_global) file->settings.base_map_id = mapid_global;
            else if (v == mapid_file) file->settings.base_map_id = mapid_file;
            else if (v < mapid_global){
                int index = app_get_map_index(vars, v);
                if (index < vars->user_map_count) file->settings.base_map_id = v;
                else file->settings.base_map_id = mapid_file;
            }
        }break;
        }
    }
}

#define CLI_OverlapWithConflict (1<<0)
#define CLI_AlwaysBindToView (2<<0)

internal void
build(System_Functions *system, Mem_Options *mem,
      App_Vars *vars, Working_Set *working_set,
      Font_Set *font_set, Style *style,
      Live_Views *live_set, Exchange *exchange,
      Panel *panel, Command_Data *command,
      String hot_directory,
      char *buffer_name, i32 buffer_name_len,
      char *path, i32 path_len,
      char *script, i32 script_len,
      u32 flags){
    if (buffer_name == 0 || path == 0 || script == 0){
        return;
    }
    
    if (vars->cli_processes.count < vars->cli_processes.max){
        Editing_Layout *layout = &vars->layout;
        Editing_File *file = working_set_contains(working_set, make_string_slowly(buffer_name));
        i32 index;
        b32 bind_to_new_view = 1;
        
        if (!file){
            Get_File_Result get_file = working_set_get_available_file(working_set);
            file = get_file.file;
            index = get_file.index;
        }
        else{
            i32 proc_count = vars->cli_processes.count;
            for (i32 i = 0; i < proc_count; ++i){
                if (vars->cli_processes.procs[i].out_file == file){
                    if (flags & CLI_OverlapWithConflict)
                        vars->cli_processes.procs[i].out_file = 0;
                    else file = 0;
                    break;
                }
            }
            index = (i32)(file - vars->working_set.files);
            if (file){
                if (!(flags & CLI_AlwaysBindToView)){
                    Panel *panel = layout->panels;
                    for (i32 i = 0; i < layout->panel_count; ++i, ++panel){
                        File_View *fview = view_to_file_view(panel->view);
                        if (fview && fview->file == file){
                            bind_to_new_view = 0;
                            break;
                        }
                    }
                }
            }
        }
        
        if (file){
            file_create_super_locked(system, mem, working_set, file, buffer_name, font_set, style->font_id);
            file->settings.unimportant = 1;
            table_add(&working_set->table, file->name.source_path, index);
            
            if (bind_to_new_view){
                View *new_view = live_set_alloc_view(live_set, mem);
                view_replace_major(system, exchange, new_view, panel, live_set);
                
                File_View *file_view = file_view_init(new_view, layout);
                view_set_file(system, file_view, file, font_set, style,
                              vars->hooks[hook_open_file], command, &app_links);
                new_view->map = app_get_map(vars, file->settings.base_map_id);
            }
            
            i32 i = vars->cli_processes.count++;
            CLI_Process *proc = vars->cli_processes.procs + i;
            if (!system->cli_call(path, script, &proc->cli)){
                --vars->cli_processes.count;
            }
            proc->out_file = file;
        }
        else{
            // TODO(allen): feedback message - no available file
        }
    }
    else{
        // TODO(allen): feedback message - no available process slot
    }
}

COMMAND_DECL(build){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_MEM(mem);
    USE_WORKING_SET(working_set);
    USE_STYLE(style);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_EXCHANGE(exchange);
    USE_FONT_SET(font_set);

    char *buffer_name = 0;
    char *path = 0;
    char *script = 0;
    
    int buffer_name_len = 0;
    int path_len = 0;
    int script_len = 0;
    u32 flags = CLI_OverlapWithConflict;
    
    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int p = dynamic_to_int(&param->param.param);
        switch (p){
        case par_target_buffer_name:
        {
            if (buffer_name == 0){
                char *new_buffer_name = dynamic_to_string(&param->param.value, &buffer_name_len);
                if (new_buffer_name){
                    buffer_name = new_buffer_name;
                }
            }
        }break;

        case par_cli_path:
        {
            if (path == 0){
                char *new_cli_path = dynamic_to_string(&param->param.value, &path_len);
                if (new_cli_path){
                    path = new_cli_path;
                }
            }
        }break;

        case par_cli_command:
        {
            if (script == 0){
                char *new_command = dynamic_to_string(&param->param.value, &script_len);
                if (new_command){
                    script = new_command;
                }
            }
        }break;
        
        case par_cli_overlap_with_conflict:
        {
            if (dynamic_to_int(&param->param.value))
                flags |= CLI_OverlapWithConflict;
            else
                flags &= (~CLI_OverlapWithConflict);
        }break;
        
        case par_cli_always_bind_to_view:
        {
            if (dynamic_to_int(&param->param.value))
                flags |= CLI_OverlapWithConflict;
            else
                flags &= (~CLI_OverlapWithConflict);
        }break;
        }
    }
    
    build(system, mem, vars, working_set,
          font_set, style, live_set, exchange,
          panel, command,
          vars->hot_directory.string,
          buffer_name, buffer_name_len,
          path, path_len,
          script, script_len,
          flags);
}

COMMAND_DECL(build_here){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_MEM(mem);
    USE_WORKING_SET(working_set);
    USE_STYLE(style);
    USE_LIVE_SET(live_set);
    USE_PANEL(panel);
    USE_EXCHANGE(exchange);
    USE_FONT_SET(font_set);
    
    u32 flags = 0;

    char *buffer_name = "*compilation*";
    int buffer_name_len = sizeof("*compilation*")-1;

    char path_space[512];
    String path = make_fixed_width_string(path_space);
    path.size = app_links.directory_get_hot(command, path.str, path.memory_size);

    char dir_space[512];
    String dir = make_fixed_width_string(dir_space);
    dir.size = app_links.directory_get_hot(command, dir.str, dir.memory_size);

    for (;;){
        if (app_links.directory_has_file(dir, "build.bat")){
            if (append(&dir, "build")){
                break;
            }
        }

        if (app_links.directory_cd(&dir, "..") == 0){
            copy(&dir, "build");
            break;
        }
    }
    terminate_with_null(&dir);

    build(system, mem, vars, working_set,
          font_set, style, live_set, exchange,
          panel, command,
          vars->hot_directory.string,
          buffer_name, buffer_name_len,
          path.str, path.size,
          dir.str, dir.size,
          flags);
}

internal void
update_command_data(App_Vars *vars, Command_Data *cmd){
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
    command_data.system = cmd->system;
    command_data.exchange = cmd->exchange;
    
    *cmd = command_data;    
}

COMPOSE_DECL(compose_write_auto_tab_line){
    command_write_character(system, command, binding);
    update_command_data(command->vars, command);
    command_auto_tab_line_at_cursor(system, command, binding);
    update_command_data(command->vars, command);
}

globalvar Command_Function command_table[cmdid_count];

extern "C"{
    EXECUTE_COMMAND_SIG(external_exec_command_keep_stack){
        Command_Data *cmd = (Command_Data*)cmd_context;
        Command_Function function = command_table[command_id];
        Command_Binding binding;
        binding.function = function;
        if (function) function(cmd->system, cmd, binding);

        update_command_data(cmd->vars, cmd);
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
        return(result);
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
            if (file && !file->state.is_dummy){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
                Working_Set *working_set = cmd->working_set;
                buffer.file_id = (int)(file - working_set->files);
                buffer.size = file->state.buffer.size;
                buffer.file_cursor_pos = file->state.cursor_pos;
                
                buffer.file_name_len = file->name.source_path.size;
                buffer.buffer_name_len = file->name.live_name.size;
                buffer.file_name = file->name.source_path.str;
                buffer.buffer_name = file->name.live_name.str;
                
                buffer.is_lexed = file->settings.tokens_exist;
                buffer.map_id = file->settings.base_map_id;
#endif
            }
        }
        
        return(buffer);
    }

    DIRECTORY_GET_HOT_SIG(external_directory_get_hot){
        Command_Data *cmd = (Command_Data*)cmd_context;
        Hot_Directory *hot = &cmd->vars->hot_directory;
        i32 copy_max = max - 1;
        hot_directory_clean_end(hot);
        if (copy_max > hot->string.size)
            copy_max = hot->string.size;
        memcpy(buffer, hot->string.str, copy_max);
        buffer[copy_max] = 0;
        return(copy_max);
    }
}

inline void
app_links_init(System_Functions *system){
    app_links.exec_command_keep_stack = external_exec_command_keep_stack;
    app_links.push_parameter = external_push_parameter;
    app_links.push_memory = external_push_memory;
    app_links.clear_parameters = external_clear_parameters;
    app_links.get_active_buffer = external_get_active_buffer;
    app_links.directory_get_hot = external_directory_get_hot;
    app_links.directory_has_file = system->directory_has_file;
    app_links.directory_cd = system->directory_cd;
}

#if FRED_INTERNAL
internal void
setup_debug_commands(Command_Map *commands, Partition *part, Key_Codes *codes, Command_Map *parent){
    map_init(commands, part, 6, parent);
    
    map_add(commands, 'm', MDFR_NONE, command_debug_memory);
    map_add(commands, 'o', MDFR_NONE, command_debug_os_events);
}
#endif

internal void
setup_ui_commands(Command_Map *commands, Partition *part, Key_Codes *codes, Command_Map *parent){
    map_init(commands, part, 32, parent);
    
    commands->vanilla_keyboard_default.function = command_null;
    
    // TODO(allen): This is hacky, when the new UI stuff happens, let's fix it, and by that
    // I mean actually fix it, don't just say you fixed it with something stupid again.
    u8 mdfr;
    u8 mdfr_array[] = {MDFR_NONE, MDFR_SHIFT, MDFR_CTRL, MDFR_SHIFT | MDFR_CTRL};
    for (i32 i = 0; i < 4; ++i){
        mdfr = mdfr_array[i];
        map_add(commands, codes->left, mdfr, command_null);
        map_add(commands, codes->right, mdfr, command_null);
        map_add(commands, codes->up, mdfr, command_null);
        map_add(commands, codes->down, mdfr, command_null);
        map_add(commands, codes->back, mdfr, command_null);
        map_add(commands, codes->esc, mdfr, command_close_minor_view);
    }
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
    
    map_add(commands, codes->right, MDFR_CTRL, command_seek_alphanumeric_or_camel_right);
    map_add(commands, codes->left, MDFR_CTRL, command_seek_alphanumeric_or_camel_left);
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
    map_add(commands, 'd', MDFR_CTRL, command_delete_range);
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
    map_add(commands, '#', MDFR_NONE, compose_write_auto_tab_line);
    
    map_add(commands, '\t', MDFR_NONE, command_word_complete);
    map_add(commands, '\t', MDFR_CTRL, command_auto_tab_range);
    map_add(commands, '\t', MDFR_SHIFT, command_auto_tab_line_at_cursor);
    
    map_add(commands, 'K', MDFR_CTRL, command_kill_buffer);
    map_add(commands, 'O', MDFR_CTRL, command_reopen);
    map_add(commands, 's', MDFR_CTRL, command_save);
    map_add(commands, 'w', MDFR_CTRL, command_interactive_save_as);

#if UseFileHistoryDump
    map_add(commands, 'h', MDFR_ALT, command_save_history);
#endif
}

internal void
setup_top_commands(Command_Map *commands, Partition *part, Key_Codes *codes, Command_Map *parent){
    map_init(commands, part, 51, parent);
    
#if FRED_INTERNAL
    map_add(commands, 'd', MDFR_ALT, command_open_debug_view);
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
    map_add(commands, 'm', MDFR_ALT, command_build_here);
}

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
    SET(word_complete);
    SET(goto_line);
    SET(set_mark);
    SET(copy);
    SET(cut);
    SET(paste);
    SET(paste_next);
    SET(delete_range);
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
    SET(build);
    
#undef SET
}

// App Functions

internal void
app_hardcode_styles(App_Vars *vars){
    Interactive_Style file_info_style;
    Style *styles, *style;
    styles = vars->styles.styles;
    style = styles;

    i16 fonts = 1;
    
    /////////////////
    style_set_name(style, make_lit_string("4coder"));
    style->font_id = fonts + 0;
    
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
    style->font_id = fonts + 1;
    ++style;
    
    /////////////////
    style_set_name(style, make_lit_string("Handmade Hero"));
    style->font_id = fonts + 1;
    
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
    style->font_id = fonts + 2;
    
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
    style->font_id = fonts + 3;
    
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
    style->font_id = fonts + 4;
    
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

HOOK_SIG(default_open_file_hook){
    Buffer_Summary buffer = app->get_active_buffer(cmd_context);
    
    int treat_as_code = 0;
    
    if (buffer.file_name && buffer.size < (16 << 20)){
        int extension_len;
        char *extension = _4coder_get_extension(buffer.file_name, buffer.file_name_len, &extension_len);
        if (_4coder_str_match(extension, extension_len, literal("cpp"))) treat_as_code = 1;
        else if (_4coder_str_match(extension, extension_len, literal("h"))) treat_as_code = 1;
        else if (_4coder_str_match(extension, extension_len, literal("c"))) treat_as_code = 1;
        else if (_4coder_str_match(extension, extension_len, literal("hpp"))) treat_as_code = 1;
    }
    
    app->push_parameter(cmd_context, dynamic_int(par_lex_as_cpp_file), dynamic_int(treat_as_code));
    app->push_parameter(cmd_context, dynamic_int(par_wrap_lines), dynamic_int(!treat_as_code));
    app->push_parameter(cmd_context, dynamic_int(par_key_mapid), dynamic_int(mapid_file));
    
    app->exec_command_keep_stack(cmd_context, cmdid_set_settings);
    app->clear_parameters(cmd_context);
}

enum Command_Line_Action{
    CLAct_Nothing,
    CLAct_Ignore,
    CLAct_UserFile,
    CLAct_CustomDLL,
    CLAct_InitialFilePosition,
    CLAct_WindowSize,
    CLAct_WindowMaximize,
    CLAct_WindowPosition,
    CLAct_Count
};

void
init_command_line_settings(App_Settings *settings, Plat_Settings *plat_settings,
                           Command_Line_Parameters clparams){
    char *arg;
    Command_Line_Action action = CLAct_Nothing;
    i32 i,index;
    b32 strict = 0;

    settings->init_files_max = ArrayCount(settings->init_files);
    for (i = 1; i <= clparams.argc; ++i){
        if (i == clparams.argc) arg = "";
        else arg = clparams.argv[i];
        switch (action){
            case CLAct_Nothing:
            {
                if (arg[0] == '-'){
                    action = CLAct_Ignore;
                    switch (arg[1]){
                        case 'u': action = CLAct_UserFile; strict = 0;     break;
                        case 'U': action = CLAct_UserFile; strict = 1;     break;

                        case 'd': action = CLAct_CustomDLL; strict = 0;    break;
                        case 'D': action = CLAct_CustomDLL; strict = 1;    break;

                        case 'i': action = CLAct_InitialFilePosition;      break;

                        case 'w': action = CLAct_WindowSize;               break;
                        case 'W': action = CLAct_WindowMaximize;         break;
                        case 'p': action = CLAct_WindowPosition;           break;
                    }
                }
                else if (arg[0] != 0){
                    if (settings->init_files_count < settings->init_files_max){
                        index = settings->init_files_count++;
                        settings->init_files[index] = arg;
                    }
                }
            }break;

            case CLAct_UserFile:
            {
                settings->user_file_is_strict = strict;
                if (i < clparams.argc){
                    settings->user_file = clparams.argv[i];
                }
                action = CLAct_Nothing;
            }break;

            case CLAct_CustomDLL:
            {
                plat_settings->custom_dll_is_strict = strict;
                if (i < clparams.argc){
                    plat_settings->custom_dll = clparams.argv[i];
                }
                action = CLAct_Nothing;
            }break;

            case CLAct_InitialFilePosition:
            {
                if (i < clparams.argc){
                    settings->initial_line = str_to_int(clparams.argv[i]);
                }
                action = CLAct_Nothing;
            }break;

            case CLAct_WindowSize:
            {
                if (i + 1 < clparams.argc){
                    plat_settings->set_window_size  = 1;
                    plat_settings->window_w = str_to_int(clparams.argv[i]);
                    plat_settings->window_h = str_to_int(clparams.argv[i+1]);

                    ++i;
                }
                action = CLAct_Nothing;
            }break;

            case CLAct_WindowMaximize:
            {
                --i;
                plat_settings->maximize_window = 1;
                action = CLAct_Nothing;
            }break;
            
            case CLAct_WindowPosition:
            {
                if (i + 1 < clparams.argc){
                    plat_settings->set_window_pos  = 1;
                    plat_settings->window_x = str_to_int(clparams.argv[i]);
                    plat_settings->window_y = str_to_int(clparams.argv[i+1]);
                    
                    ++i;
                }
                action = CLAct_Nothing;
            }break;
        }
    }
}

internal App_Vars*
app_setup_memory(Application_Memory *memory){
    Partition _partition = partition_open(memory->vars_memory, memory->vars_memory_size);
    App_Vars *vars = push_struct(&_partition, App_Vars);
    Assert(vars);
    *vars = {};
    vars->mem.part = _partition;
    
    general_memory_open(&vars->mem.general, memory->target_memory, memory->target_memory_size);
    
    return(vars);
}

internal i32
execute_special_tool(void *memory, i32 size, Command_Line_Parameters clparams){
    char message[] = "Hell World!";
    i32 result = sizeof(message) - 1;
    memcpy(memory, message, result);
    return(result);
}

App_Read_Command_Line_Sig(app_read_command_line){
    App_Vars *vars;
    i32 out_size = 0;

    if (clparams.argc > 1 && match(clparams.argv[1], "-T")){
        out_size = execute_special_tool(memory->target_memory, memory->target_memory_size, clparams);
    }
    else{
        vars = app_setup_memory(memory);
        if (clparams.argc > 1){
            init_command_line_settings(&vars->settings, plat_settings, clparams);
        }
        else{
            vars->settings = {};
        }
        *files = vars->settings.init_files;
        *file_count = &vars->settings.init_files_count;
    }

    return(out_size);
}

App_Init_Sig(app_init){
    app_links_init(system);
    
    App_Vars *vars = (App_Vars*)memory->vars_memory;
    vars->config_api = api;
    Partition *partition = &vars->mem.part;
    target->partition = partition;
    
    i32 panel_max_count = vars->layout.panel_max_count = 16;
    i32 divider_max_count = panel_max_count - 1;
    vars->layout.panel_count = 1;
    
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
        sizeof(Menu_View),
#if FRED_INTERNAL
        sizeof(Debug_View),
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
    if (vars->config_api.get_bindings){
        i32 size = partition_remaining(partition);
        void *data = partition_current(partition);
        
        // TODO(allen): Use a giant bubble of general memory for this.
        // So that it doesn't interfere with the command maps as they allocate
        // their own memory.
        i32 wanted_size = vars->config_api.get_bindings(data, size, codes);
        
        b32 did_top = 0;
        b32 did_file = 0;
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
                        else if (mapid < mapid_global){
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
                            else if (mapid < mapid_global){
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
        
        if (!did_top) setup_top_commands(&vars->map_top, &vars->mem.part, codes, global);
        if (!did_file) setup_file_commands(&vars->map_file, &vars->mem.part, codes, global);
    }
    else{
        setup_top_commands(&vars->map_top, &vars->mem.part, codes, global);
        setup_file_commands(&vars->map_file, &vars->mem.part, codes, global);
    }
    
    setup_ui_commands(&vars->map_ui, &vars->mem.part, codes, global);
#if FRED_INTERNAL
    setup_debug_commands(&vars->map_debug, &vars->mem.part, codes, global);
#endif
    
    if (vars->hooks[hook_open_file] == 0){
        vars->hooks[hook_open_file] = default_open_file_hook;
    }

    vars->font_set = &target->font_set;
    
    font_set_init(vars->font_set, partition, 16, 4);
    
    {
        struct Font_Setup{
            char *c_file_name;
            i32 file_name_len;
            char *c_name;
            i32 name_len;
            i32 pt_size;
        };

#define LitStr(n) n, sizeof(n)-1
        
        Font_Setup font_setup[] = {
            {LitStr("LiberationSans-Regular.ttf"),
             LitStr("liberation sans"),
             16},

            {LitStr("liberation-mono.ttf"),
             LitStr("liberation mono"),
             16},
                        
            {LitStr("Hack-Regular.ttf"),
             LitStr("hack"),
             16},
            
            {LitStr("CutiveMono-Regular.ttf"),
             LitStr("cutive mono"),
             16},
            
            {LitStr("Inconsolata-Regular.ttf"),
             LitStr("inconsolata"),
             16},
            
        };
        i32 font_count = ArrayCount(font_setup);
        
        for (i32 i = 0; i < font_count; ++i){
            String file_name = make_string(font_setup[i].c_file_name,
                                           font_setup[i].file_name_len);
            String name = make_string(font_setup[i].c_name,
                                      font_setup[i].name_len);
            i32 pt_size = font_setup[i].pt_size;
            
            font_set_add(partition, vars->font_set, file_name, name, pt_size);
        }
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
    
    panel_init(&panels[0]);

    String hdbase = make_fixed_width_string(vars->hot_dir_base_);
    hot_directory_init(&vars->hot_directory, hdbase, current_directory, system->slash);

    vars->mini_str = make_string((char*)vars->mini_buffer, 0, 512);
    
    // NOTE(allen): child proc list setup
    i32 max_children = 16;
    partition_align(partition, 8);
    vars->cli_processes.procs = push_array(partition, CLI_Process, max_children);
    vars->cli_processes.max = max_children;
    vars->cli_processes.count = 0;

    // NOTE(allen): sys app binding setup
    vars->sys_app_max = exchange->file.max;
    vars->sys_app_count = 0;
    vars->sys_app_bindings = (Sys_App_Binding*)push_array(partition, Sys_App_Binding, vars->sys_app_max);
}

App_Step_Sig(app_step){
    ProfileStart(OS_syncing);
    Application_Step_Result app_result = *result;
    app_result.redraw = force_redraw;
    
    App_Vars *vars = (App_Vars*)memory->vars_memory;
    target->partition = &vars->mem.part;
    
    if (first_step || !time_step){
        app_result.redraw = 1;
    }
    
    Panel *panels = vars->layout.panels;
    Panel *active_panel = &panels[vars->layout.active_panel];
    
    // NOTE(allen): OS clipboard event handling
    if (clipboard.str){
        String *dest = working_set_next_clipboard_string(&vars->mem.general, &vars->working_set, clipboard.size);
        dest->size = eol_convert_in(dest->str, clipboard.str, clipboard.size);
    }
    
    // TODO(allen): profile this make sure it's not costing me too much power.
    // NOTE(allen): check files are up to date
    for (i32 i = 0; i < vars->working_set.file_index_count; ++i){
        Editing_File *file = vars->working_set.files + i;
        
        if (!file->state.is_dummy){
            u64 time_stamp = system->file_time_stamp(make_c_str(file->name.source_path));
            
            if (time_stamp > 0){
                file->state.last_sys_write_time = time_stamp;
                if (file->state.last_sys_write_time != file->state.last_4ed_write_time){
                    app_result.redraw = 1;
                }
            }
        }
    }
    
    // NOTE(allen): update child processes
    if (time_step){
        Temp_Memory temp = begin_temp_memory(&vars->mem.part);
        u32 max = Kbytes(32);
        char *dest = push_array(&vars->mem.part, char, max);
        u32 amount;
        
        i32 count = vars->cli_processes.count;
        for (i32 i = 0; i < count; ++i){
            CLI_Process *proc = vars->cli_processes.procs + i;
            Editing_File *out_file = proc->out_file;
            
            if (out_file != 0){
                i32 new_cursor = out_file->state.cursor_pos;

                for (system->cli_begin_update(&proc->cli);
                     system->cli_update_step(&proc->cli, dest, max, &amount);){
                    amount = eol_in_place_convert_in(dest, amount);
                    Edit_Spec spec = {};
                    spec.step.type = ED_NORMAL;
                    spec.step.edit.start = buffer_size(&out_file->state.buffer);
                    spec.step.edit.end = spec.step.edit.start;
                    spec.step.edit.len = amount;
                    spec.step.pre_pos = new_cursor;
                    spec.step.post_pos = spec.step.edit.start + amount;
                    spec.str = (u8*)dest;
                    file_do_single_edit(system, &vars->mem, out_file,
                                        &vars->layout, spec, hist_normal);
                    app_result.redraw = 1;
                    new_cursor = spec.step.post_pos;
                }
            
                if (system->cli_end_update(&proc->cli)){
                    *proc = vars->cli_processes.procs[--count];
                    --i;

                    char str_space[256];
                    String str = make_fixed_width_string(str_space);
                    append(&str, "exited with code ");
                    append_int_to_str(proc->cli.exit, &str);
                    
                    Edit_Spec spec = {};
                    spec.step.type = ED_NORMAL;
                    spec.step.edit.start = buffer_size(&out_file->state.buffer);
                    spec.step.edit.end = spec.step.edit.start;
                    spec.step.edit.len = str.size;
                    spec.step.pre_pos = new_cursor;
                    spec.step.post_pos = spec.step.edit.start + str.size;
                    spec.str = (u8*)str.str;
                    file_do_single_edit(system, &vars->mem, out_file,
                                        &vars->layout, spec, hist_normal);
                    app_result.redraw = 1;
                    new_cursor = spec.step.post_pos;
                }
            
                Panel *panel = vars->layout.panels;
                i32 panel_count = vars->layout.panel_count;
                for (i32 i = 0; i < panel_count; ++i, ++panel){
                    View *view = panel->view;
                    if (view && view->is_minor) view = view->major;
                    File_View *fview = view_to_file_view(view);
                    if (fview && fview->file == out_file){
                        view_cursor_move(fview, new_cursor);
                    }
                }
            }
        }
        
        vars->cli_processes.count = count;
        end_temp_memory(temp);
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
            view_measure_wraps(system, &vars->mem.general, view);
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
    
    Mouse_Summary mouse_data;
    mouse_data.mx = mouse->x;
    mouse_data.my = mouse->y;
    
    mouse_data.l = mouse->left_button;
    mouse_data.r = mouse->right_button;
    mouse_data.press_l = mouse->left_button_pressed;
    mouse_data.press_r = mouse->right_button_pressed;
    mouse_data.release_l = mouse->left_button_released;
    mouse_data.release_r = mouse->right_button_released;
    
    mouse_data.out_of_window = mouse->out_of_window;
    mouse_data.wheel_used = (mouse->wheel != 0);
    mouse_data.wheel_amount = -mouse->wheel;
    ProfileEnd(OS_syncing);
    
    ProfileStart(hover_status);
    // NOTE(allen): detect mouse hover status
    i32 mx = mouse_data.mx;
    i32 my = mouse_data.my;
    b32 mouse_in_edit_area = 0;
    b32 mouse_in_margin_area = 0;
    Panel *mouse_panel = 0;
    i32 mouse_panel_i = 0;
    
    {
        Panel *panel = 0;
        b32 in_edit_area = 0;
        b32 in_margin_area = 0;
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
    
    b32 mouse_on_divider = 0;
    b32 mouse_divider_vertical = 0;
    i32 mouse_divider_id = 0;
    i32 mouse_divider_side = 0;
    
    if (mouse_in_margin_area){
        b32 resize_area = 0;
        i32 divider_id = 0;
        b32 seeking_v_divider = 0;
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
                    b32 divider_match = (other_div.divider->v_divider == mouse_divider_vertical);
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
                    b32 divider_match = (other_div.divider->v_divider == mouse_divider_vertical);
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
    command_data.exchange = exchange;
    command_data.screen_width = target->width;
    command_data.screen_height = target->height;
    command_data.system = system;
    
    Temp_Memory param_stack_temp = begin_temp_memory(&vars->mem.part);
    command_data.part = partition_sub_part(&vars->mem.part, 16 << 10);
    
    if (first_step){
        if (vars->hooks[hook_start]){
            vars->hooks[hook_start](&command_data, &app_links);
            command_data.part.pos = 0;
        }
        
        i32 i;
        String file_name;
        File_View *fview;
        Editing_File *file;
        Panel *panel = vars->layout.panels;
        for (i = 0; i < vars->settings.init_files_count; ++i, ++panel){
            file_name = make_string_slowly(vars->settings.init_files[i]);
            
            if (i < vars->layout.panel_count){
                fview = app_open_file(system, vars, exchange, &vars->live_set, &vars->working_set, panel,
                    &command_data, file_name);
                
                if (i == 0){
                    if (fview){
                        file = fview->file;
                        if (file){
                            file->preload.start_line = vars->settings.initial_line;
                        }
                    }
                }
            }
            else{
                app_open_file_background(vars, exchange, &vars->working_set, file_name);
            }
        }
    }
    
    // NOTE(allen): command input to active view
    for (i32 key_i = 0; key_i < key_data.count; ++key_i){
        Command_Binding cmd = {};
        Command_Map *map = 0;
        View *view = active_panel->view;
        
        Key_Event_Data key = get_single_key(&key_data, key_i);
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
                handle_command(system, view, &command_data, cmd, key, codes);
                app_result.redraw = 1;
            }
            else{
                if (cmd.function){
                    cmd.function(system, &command_data, cmd);
                    app_result.redraw = 1;
                }
            }
            vars->prev_command = cmd;
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
                b32 active = (panel == active_panel);
                Input_Summary input = (active)?(active_input):(dead_input);
                if (panel == mouse_panel && !mouse_data.out_of_window){
                    input.mouse = mouse_data;
                }
                if (view_->do_view(system, exchange, view_, panel->inner, active_view,
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
                    app_open_file(system, vars, exchange,
                                  live_set, working_set, panel,
                                  &command_data, *string);
            }break;
            
            case DACT_SAVE_AS:
            {
                View *view = panel->view;
                File_View *fview = view_to_file_view(view);
                
                if (!fview && view->is_minor) fview = view_to_file_view(view->major);
                if (fview){
                    Editing_File *file = fview->file;
                    if (file && !file->state.is_dummy){
                        i32 sys_id = file_save_and_set_names(system, exchange, mem, working_set, file, string->str);
                        app_push_file_binding(vars, sys_id, get_file_id(working_set, file));
                    }
                }
            }break;
            
            case DACT_SAVE:
            {
                Editing_File *file = working_set_lookup_file(working_set, *string);
                if (!file->state.is_dummy){
                    i32 sys_id = file_save(system, exchange, mem, file, file->name.source_path.str);
                    app_push_file_binding(vars, sys_id, get_file_id(working_set, file));
                }
            }break;
            
            case DACT_NEW:
            {
                Get_File_Result file = working_set_get_available_file(working_set);
                file_create_empty(system, mem, working_set, file.file, string->str,
                                  vars->font_set, style->font_id);
                table_add(&working_set->table, file.file->name.source_path, file.index);
                
                View *new_view = live_set_alloc_view(live_set, mem);
                view_replace_major(system, exchange, new_view, panel, live_set);
                
                File_View *file_view = file_view_init(new_view, &vars->layout);
                command_data.view = (View*)file_view;
                view_set_file(system, file_view, file.file, vars->font_set, style,
                              vars->hooks[hook_open_file], &command_data, &app_links);
                new_view->map = app_get_map(vars, file.file->settings.base_map_id);
#if BUFFER_EXPERIMENT_SCALPEL <= 0
                if (file.file->settings.tokens_exist)
                    file_first_lex_parallel(system, general, file.file);
#endif
            }break;
            
            case DACT_SWITCH:
            {
                Editing_File *file = working_set_lookup_file(working_set, *string);
                if (file){
                    View *new_view = live_set_alloc_view(live_set, mem);
                    view_replace_major(system, exchange, new_view, panel, live_set);
                    
                    File_View *file_view = file_view_init(new_view, &vars->layout);
                    command_data.view = (View*)file_view;
                    
                    view_set_file(system, file_view, file, vars->font_set, style,
                                  vars->hooks[hook_open_file], &command_data, &app_links);
                    
                    new_view->map = app_get_map(vars, file->settings.base_map_id);
                }
            }break;
            
            case DACT_KILL:
            {
                Editing_File *file = working_set_lookup_file(working_set, *string);
                if (file){
                    table_remove(&working_set->table, file->name.source_path);
                    kill_file(system, exchange, general, file, live_set, &vars->layout);
                }
            }break;
            
            case DACT_TRY_KILL:
            {
                Editing_File *file = working_set_lookup_file(working_set, *string);
                if (file){
                    if (buffer_needs_save(file)){
                        View *new_view = live_set_alloc_view(live_set, mem);
                        view_replace_minor(system, exchange, new_view, panel, live_set);
                            
                        new_view->map = &vars->map_ui;
                        Interactive_View *int_view = 
                            interactive_view_init(system, new_view, &vars->hot_directory, style,
                                                  working_set, vars->font_set, &vars->delay);
                        int_view->interaction = INTV_SURE_TO_KILL_INTER;
                        int_view->action = INTV_SURE_TO_KILL;
                        copy(&int_view->query, "Are you sure?");
                        copy(&int_view->dest, file->name.live_name);
                    }
                    else{
                        table_remove(&working_set->table, file->name.source_path);
                        kill_file(system, exchange, general, file, live_set, &vars->layout);
                        view_remove_minor(system, exchange, panel, live_set);
                    }
                }
            }break;
            
            case DACT_CLOSE_MINOR:
            {
                view_remove_minor(system, exchange, panel, live_set);
            }break;
            
            case DACT_CLOSE_MAJOR:
            {
                view_remove_major(system, exchange, panel, live_set);
            }break;
            
            case DACT_THEME_OPTIONS:
            {
                open_theme_options(system, exchange, vars, live_set, mem, panel);
            }break;

            case DACT_KEYBOARD_OPTIONS:
            {
                open_config_options(system, exchange, vars, live_set, mem, panel);
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
                    view->do_view(system, exchange,
                                  view, inner, active_view,
                                  VMSG_RESIZE, 0, &dead_input, &active_input);
                    view = (view->is_minor)?view->major:0;
                    if (view){
                        view->do_view(system, exchange,
                                      view, inner, active_view,
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

        Editing_File *file = vars->working_set.files;
        for (i32 i = vars->working_set.file_index_count; i > 0; --i, ++file){
            if (buffer_good(&file->state.buffer) && !file->state.is_dummy){
                Render_Font *font = get_font_info(vars->font_set, vars->style.font_id)->font;
                float *advance_data = 0;
                if (font) advance_data = font->advance_data;
                
                file_measure_starts_widths(system, &vars->mem.general,
                                           &file->state.buffer, advance_data);
            }
        }
        
        Panel *panel = panels;
        for (i32 panel_i = vars->layout.panel_count; panel_i > 0; --panel_i, ++panel){
            View *view = panel->view;
            if (view){
                view->do_view(system, exchange,
                              view, panel->inner, active_view,
                              VMSG_STYLE_CHANGE, 0, &dead_input, &active_input);
                view = (view->is_minor)?view->major:0;
                if (view){
                    view->do_view(system, exchange,
                                  view, panel->inner, active_view,
                                  VMSG_STYLE_CHANGE, 0, &dead_input, &active_input);
                }
            }
        }
    }
    ProfileEnd(style_change);

    // NOTE(allen): processing bindings between system and application
    ProfileStart(sys_app_bind_processing);
    for (i32 i = 0; i < vars->sys_app_count; ++i){
        Sys_App_Binding *binding;
        b32 remove = 0;
        b32 failed = 0;
        binding = vars->sys_app_bindings + i;
        
        byte *data;
        i32 size, max;
        Editing_File *ed_file;
        Editing_File_Preload preload_settings;
        char *filename;
        
        Working_Set *working_set = &vars->working_set;
        
        if (exchange_file_ready(exchange, binding->sys_id, &data, &size, &max)){
            ed_file = working_set->files + binding->app_id;
            filename = exchange_file_filename(exchange, binding->sys_id);
            preload_settings = ed_file->preload;
            if (data){
                String val = make_string((char*)data, size);
                file_create_from_string(system, &vars->mem, working_set, ed_file, filename,
                                        vars->font_set, vars->style.font_id, val);
                
                if (ed_file->settings.tokens_exist)
                    file_first_lex_parallel(system, &vars->mem.general, ed_file);
            }
            else{
                file_create_empty(system, &vars->mem, working_set, ed_file, filename,
                                  vars->font_set, vars->style.font_id);
            }

            for (File_View_Iter iter = file_view_iter_init(&vars->layout, ed_file, 0);
                file_view_iter_good(iter);
                iter = file_view_iter_next(iter)){
                view_file_loaded_init(system, iter.view, 0);
                view_cursor_move(iter.view, preload_settings.start_line, 0);
            }
            
            exchange_free_file(exchange, binding->sys_id);
            remove = 1;
        }
        
        if (exchange_file_save_complete(exchange, binding->sys_id, &data, &size, &max, &failed)){
            Assert(remove == 0);
            
            if (data){
                general_memory_free(&vars->mem.general, data);
                exchange_clear_file(exchange, binding->sys_id);
            }
            
            Editing_File *file = get_file(working_set, binding->app_id);
            if (file){
                file_synchronize_times(system, file, file->name.source_path.str);
            }
            
            exchange_free_file(exchange, binding->sys_id);
            remove = 1;
            
            // if (failed) { TODO(allen): saving error, now what? }
        }
        
        if (remove){
            *binding = vars->sys_app_bindings[--vars->sys_app_count];
            --i;
        }
    }
    ProfileEnd(sys_app_bind_processing);
    
    ProfileStart(redraw);
    if (mouse_panel != vars->prev_mouse_panel) app_result.redraw = 1;
    if (app_result.redraw){
        begin_render_section(target, system);
        
        target->clip_top = -1;
        draw_push_clip(target, rect_from_target(target));
        
        // NOTE(allen): render the panels
        Panel *panel = panels;
        for (i32 panel_i = vars->layout.panel_count; panel_i > 0; --panel_i, ++panel){
            i32_Rect full = panel->full;
            i32_Rect inner = panel->inner;
            
            View *view = panel->view;
            Style *style = &vars->style;
            
            b32 active = (panel == active_panel);
            u32 back_color = style->main.back_color;
            draw_rectangle(target, full, back_color);
            
            if (view){
                Assert(view->do_view);
                draw_push_clip(target, panel->inner);
                view->do_view(system, exchange,
                              view, panel->inner, active_view,
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

        end_render_section(target, system);
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
    
    *result = app_result;
    result->lctrl_lalt_is_altgr = vars->settings.lctrl_lalt_is_altgr;
    
    // end-of-app_step
}

internal
App_Alloc_Sig(app_alloc){
    Mem_Options *mem = (Mem_Options*)(handle);
    void *result = general_memory_allocate(&mem->general, size, 0);
    return(result);
}

internal
App_Free_Sig(app_free){
    Mem_Options *mem = (Mem_Options*)(handle);
    general_memory_free(&mem->general, block);
}

external App_Get_Functions_Sig(app_get_functions){
    App_Functions result = {};
    
    result.read_command_line = app_read_command_line;
    result.init = app_init;
    result.step = app_step;
    
    result.alloc = app_alloc;
    result.free = app_free;
    
    return(result);
}

// BOTTOM

