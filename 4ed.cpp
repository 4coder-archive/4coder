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

#define SysAppCreateView 0x1
#define SysAppCreateNewBuffer 0x2

struct Sys_App_Binding{
    i32 sys_id;
    i32 app_id;

    u32 success;
    u32 fail;
    Panel *panel;
};

struct Complete_State{
    Search_Set set;
    Search_Iter iter;
    Table hits;
    String_Space str;
    i32 word_start, word_end;
    b32 initialized;
};

struct Command_Data{
    Models *models;
    struct App_Vars *vars;
    System_Functions *system;
    Exchange *exchange;
    Live_Views *live_set;

    Panel *panel;
    View *view;

    i32 screen_width, screen_height;
    Key_Event_Data key;

    Partition part;
};

struct App_Vars{
    Models models;

    CLI_List cli_processes;

    Sys_App_Binding *sys_app_bindings;
    i32 sys_app_count, sys_app_max;

    Live_Views live_set;

    App_State state;
    App_State_Resizing resizing;
    Complete_State complete_state;

    Command_Data command_data;
};

internal i32
app_get_or_add_map_index(Models *models, i32 mapid){
    i32 result;
    i32 user_map_count = models->user_map_count;
    i32 *map_id_table = models->map_id_table;
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
app_get_map_index(Models *models, i32 mapid){
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
app_get_map(Models *models, i32 mapid){
    Command_Map *map = 0;
    if (mapid < mapid_global) map = models->user_maps + mapid;
    else if (mapid == mapid_global) map = &models->map_top;
    else if (mapid == mapid_file) map = &models->map_file;
    return map;
}

// Commands

globalvar Application_Links app_links;

#define USE_MODELS(n) Models *n = command->models
#define USE_VARS(n) App_Vars *n = command->vars
#define USE_PANEL(n) Panel *n = command->panel
#define USE_VIEW(n) View *n = command->view
#define USE_FILE(n,v) Editing_File *n = (v)->file
#define USE_EXCHANGE(n) Exchange *n = command->exchange

#define REQ_OPEN_VIEW(n) View *n = command->panel->view; if (view_lock_level(n) > LockLevel_Open) return
#define REQ_READABLE_VIEW(n) View *n = command->panel->view; if (view_lock_level(n) > LockLevel_NoWrite) return

#define REQ_FILE(n,v) Editing_File *n = (v)->file; if (!n) return
#define REQ_FILE_HISTORY(n,v) Editing_File *n = (v)->file; if (!n || !n->state.undo.undo.edits) return

#define COMMAND_DECL(n) internal void command_##n(System_Functions *system, Command_Data *command, Command_Binding binding)

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

internal View*
panel_make_empty(System_Functions *system, Exchange *exchange, App_Vars *vars, Panel *panel){
    Models *models = &vars->models;
    View_And_ID new_view;

    Assert(panel->view == 0);
    new_view = live_set_alloc_view(&vars->live_set, panel, models);
    view_set_file(new_view.view, 0, models, 0, 0, 0);
    new_view.view->map = app_get_map(models, mapid_global);

    return(new_view.view);
}

COMMAND_DECL(null){
    AllowLocal(command);
}

COMMAND_DECL(write_character){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    char character;
    i32 pos, next_cursor_pos;

    character = command->key.character;
    if (character != 0){
        pos = view->cursor.pos;
        next_cursor_pos = view->cursor.pos + 1;
        view_replace_range(system, models, view, pos, pos, &character, 1, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
    }
}

COMMAND_DECL(seek_whitespace_right){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = buffer_seek_whitespace_right(&file->state.buffer, view->cursor.pos);
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

COMMAND_DECL(seek_left){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    u32 flags = BoundryWhitespace;

    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int p = dynamic_to_int(&param->param.param);
        switch (p){
            case par_flags:
            flags = dynamic_to_int(&param->param.value);
            break;
        }
    }

    i32 pos[4] = {0};

    if (flags & (1)){
        pos[0] = buffer_seek_whitespace_left(&file->state.buffer, view->cursor.pos);
    }

    if (flags & (1 << 1)){
        if (file->state.tokens_complete){
            pos[1] = seek_token_left(&file->state.token_stack, view->cursor.pos);
        }
        else{
            pos[1] = buffer_seek_whitespace_left(&file->state.buffer, view->cursor.pos);
        }
    }

    if (flags & (1 << 2)){
        pos[2] = buffer_seek_alphanumeric_left(&file->state.buffer, view->cursor.pos);
        if (flags & (1 << 3)){
            pos[3] = buffer_seek_range_camel_left(&file->state.buffer, view->cursor.pos, pos[2]);
        }
    }
    else{
        if (flags & (1 << 3)){
            pos[3] = buffer_seek_alphanumeric_or_camel_left(&file->state.buffer, view->cursor.pos);
        }
    }

    i32 new_pos = 0;
    for (i32 i = 0; i < ArrayCount(pos); ++i){
        if (pos[i] > new_pos) new_pos = pos[i];
    }

    view_cursor_move(view, new_pos);
}

COMMAND_DECL(seek_right){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    u32 flags = BoundryWhitespace;

    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int p = dynamic_to_int(&param->param.param);
        switch (p){
            case par_flags:
            flags = dynamic_to_int(&param->param.value);
            break;
        }
    }

    i32 size = buffer_size(&file->state.buffer);
    i32 pos[4];
    for (i32 i = 0; i < ArrayCount(pos); ++i) pos[i] = size;

    if (flags & (1)){
        pos[0] = buffer_seek_whitespace_right(&file->state.buffer, view->cursor.pos);
    }

    if (flags & (1 << 1)){
        if (file->state.tokens_complete){
            pos[1] = seek_token_right(&file->state.token_stack, view->cursor.pos);
        }
        else{
            pos[1] = buffer_seek_whitespace_right(&file->state.buffer, view->cursor.pos);
        }
    }

    if (flags & (1 << 2)){
        pos[2] = buffer_seek_alphanumeric_right(&file->state.buffer, view->cursor.pos);
        if (flags & (1 << 3)){
            pos[3] = buffer_seek_range_camel_right(&file->state.buffer, view->cursor.pos, pos[2]);
        }
    }
    else{
        if (flags & (1 << 3)){
            pos[3] = buffer_seek_alphanumeric_or_camel_right(&file->state.buffer, view->cursor.pos);
        }
    }

    i32 new_pos = size;
    for (i32 i = 0; i < ArrayCount(pos); ++i){
        if (pos[i] < new_pos) new_pos = pos[i];
    }

    view_cursor_move(view, new_pos);
}

COMMAND_DECL(seek_whitespace_left){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = buffer_seek_whitespace_left(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_whitespace_up){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = buffer_seek_whitespace_up(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_whitespace_down){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = buffer_seek_whitespace_down(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_token_left){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    if (file->state.tokens_complete){
        i32 pos = seek_token_left(&file->state.token_stack, view->cursor.pos);
        view_cursor_move(view, pos);
    }
}

COMMAND_DECL(seek_token_right){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    if (file->state.tokens_complete){
        i32 pos = seek_token_right(&file->state.token_stack, view->cursor.pos);
        view_cursor_move(view, pos);
    }
}

COMMAND_DECL(seek_white_or_token_right){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 token_pos, white_pos;
    if (file->state.tokens_complete){
        token_pos = seek_token_right(&file->state.token_stack, view->cursor.pos);
    }
    else{
        token_pos = buffer_size(&file->state.buffer);
    }
    white_pos = buffer_seek_whitespace_right(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, Min(token_pos, white_pos));
}

COMMAND_DECL(seek_white_or_token_left){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 token_pos, white_pos;
    if (file->state.tokens_complete){
        token_pos = seek_token_left(&file->state.token_stack, view->cursor.pos);
    }
    else{
        token_pos = 0;
    }
    white_pos = buffer_seek_whitespace_left(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, Max(token_pos, white_pos));
}

COMMAND_DECL(seek_alphanumeric_right){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    i32 pos = buffer_seek_alphanumeric_right(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_alphanumeric_left){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    i32 pos = buffer_seek_alphanumeric_left(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_alphanumeric_or_camel_right){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    i32 pos = buffer_seek_alphanumeric_or_camel_right(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_alphanumeric_or_camel_left){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    i32 pos = buffer_seek_alphanumeric_or_camel_left(&file->state.buffer, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(word_complete){
    ProfileMomentFunction();
    USE_MODELS(models);
    USE_VARS(vars);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    Partition *part = &models->mem.part;
    General_Memory *general = &models->mem.general;
    Working_Set *working_set = &models->working_set;
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

    i32 match_size;
    b32 do_init = 0;

    buffer = &file->state.buffer;
    size_of_buffer = buffer_size(buffer);

    if (view->mode.rewrite != 2){
        do_init = 1;
    }
    view->next_mode.rewrite = 2;

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

        {
            File_Node *node, *used_nodes;
            Editing_File *file_ptr;
            i32 buffer_count, j;

            buffer_count = working_set->file_count;
            search_set_init(general, &complete_state->set, buffer_count + 1);
            ranges = complete_state->set.ranges;
            ranges[0].buffer = buffer;
            ranges[0].start = 0;
            ranges[0].size = word_start;

            ranges[1].buffer = buffer;
            ranges[1].start = word_end;
            ranges[1].size = size_of_buffer - word_end;

            used_nodes = &working_set->used_sentinel;
            j = 2;
            for (dll_items(node, used_nodes)){
                file_ptr = (Editing_File*)node;
                if (file_ptr != file){
                    ranges[j].buffer = &file_ptr->state.buffer;
                    ranges[j].start = 0;
                    ranges[j].size = buffer_size(ranges[j].buffer); 
                    ++j;
                }
            }
            complete_state->set.count = j;
        }

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
                    view_replace_range(system, models, view, word_start, word_end, spare, match_size, word_end);

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
                view_replace_range(system, models, view, word_start, word_end,
                    complete_state->iter.word.str, match_size, word_end);

                complete_state->word_end = word_start + match_size;
                complete_state->set.ranges[1].start = word_start + match_size;
                break;
            }
        }
    }
}

COMMAND_DECL(set_mark){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    view->mark = (i32)view->cursor.pos;
}

COMMAND_DECL(copy){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    // TODO(allen): deduplicate
    int r_start = 0, r_end = 0;
    int start_set = 0, end_set = 0;

    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int p = dynamic_to_int(&param->param.param);
        switch (p){
            case par_range_start:
            start_set = 1;
            r_start = dynamic_to_int(&param->param.value);
            break;

            case par_range_end:
            end_set = 1;
            r_end = dynamic_to_int(&param->param.value);
            break;
        }
    }

    Range range = make_range(view->cursor.pos, view->mark);
    if (start_set) range.start = r_start;
    if (end_set) range.end = r_end;
    if (range.start < range.end){
        clipboard_copy(system, &models->mem.general, &models->working_set, range, file);
    }
}

COMMAND_DECL(cut){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    // TODO(allen): deduplicate
    int r_start = 0, r_end = 0;
    int start_set = 0, end_set = 0;

    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int p = dynamic_to_int(&param->param.param);
        switch (p){
            case par_range_start:
            start_set = 1;
            r_start = dynamic_to_int(&param->param.value);
            break;

            case par_range_end:
            end_set = 1;
            r_end = dynamic_to_int(&param->param.value);
            break;
        }
    }

    Range range = make_range(view->cursor.pos, view->mark);
    if (start_set) range.start = r_start;
    if (end_set) range.end = r_end;
    if (range.start < range.end){
        i32 next_cursor_pos = range.start;

        clipboard_copy(system, &models->mem.general, &models->working_set, range, file);
        view_replace_range(system, models, view, range.start, range.end, 0, 0, next_cursor_pos);

        view->mark = range.start;
        view_cursor_move(view, next_cursor_pos);
    }
}

COMMAND_DECL(paste){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    View_Iter iter;
    String *src;
    i32 pos_left, next_cursor_pos;

    if (models->working_set.clipboard_size > 0){
        view->next_mode.rewrite = 1;

        src = working_set_clipboard_head(&models->working_set);
        pos_left = view->cursor.pos;

        next_cursor_pos = pos_left+src->size;
        view_replace_range(system, models, view, pos_left, pos_left, src->str, src->size, next_cursor_pos);

        view_cursor_move(view, next_cursor_pos);
        view->mark = pos_left;

        for (iter = file_view_iter_init(&models->layout, file, 0);
            file_view_iter_good(iter);
            iter = file_view_iter_next(iter)){
            view_post_paste_effect(iter.view, 20, pos_left, src->size, models->style.main.paste_color);
        }
    }
}

COMMAND_DECL(paste_next){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    View_Iter iter;
    Range range;
    String *src;
    i32 next_cursor_pos;

    if (models->working_set.clipboard_size > 0 && view->mode.rewrite == 1){
        view->next_mode.rewrite = 1;

        range = make_range(view->mark, view->cursor.pos);
        src = working_set_clipboard_roll_down(&models->working_set);
        next_cursor_pos = range.start+src->size;
        view_replace_range(system,
            models, view, range.start, range.end,
            src->str, src->size, next_cursor_pos);

        view_cursor_move(view, next_cursor_pos);
        view->mark = range.start;

        for (iter = file_view_iter_init(&models->layout, file, 0);
            file_view_iter_good(iter);
            iter = file_view_iter_next(iter)){
            view_post_paste_effect(iter.view, 20, range.start, src->size, models->style.main.paste_color);
        }
    }
    else{
        command_paste(system, command, binding);
    }
}

COMMAND_DECL(delete_range){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    Range range;
    i32 next_cursor_pos;

    range = make_range(view->cursor.pos, view->mark);
    if (range.start < range.end){
        next_cursor_pos = range.start;
        view_replace_range(system, models, view, range.start, range.end, 0, 0, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
        view->mark = range.start;
    }
}

COMMAND_DECL(timeline_scrub){
    ProfileMomentFunction();
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);

    view_set_widget(view, FWIDG_TIMELINES);
    view->widget.timeline.undo_line = 1;
    view->widget.timeline.history_line = 1;
}

COMMAND_DECL(undo){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);

    view_undo(system, models, view);
}

COMMAND_DECL(redo){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);

    view_redo(system, models, view);
}

COMMAND_DECL(history_backward){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);

    view_history_step(system, models, view, hist_backward);
}

COMMAND_DECL(history_forward){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);

    view_history_step(system, models, view, hist_backward);
}

COMMAND_DECL(interactive_new){
    ProfileMomentFunction();
    USE_MODELS(models);
    USE_VIEW(view);

    view_show_interactive(system, view, &models->map_ui,
        IAct_New, IInt_Sys_File_List, make_lit_string("New: "));
}

internal Sys_App_Binding*
app_push_file_binding(App_Vars *vars, int sys_id, int app_id){
    Sys_App_Binding *binding;
    Assert(vars->sys_app_count < vars->sys_app_max);
    binding = vars->sys_app_bindings + vars->sys_app_count++;
    binding->sys_id = sys_id;
    binding->app_id = app_id;
    return(binding);
}

struct App_Open_File_Result{
    Editing_File *file;
    i32 sys_id;
    i32 file_index;
    b32 is_new;
};

COMMAND_DECL(interactive_open){
    ProfileMomentFunction();
    USE_MODELS(models);
    USE_PANEL(panel);
    USE_VIEW(view);

    Delay *delay = &models->delay1;

    char *filename = 0;
    int filename_len = 0;
    int do_in_background = 0;

    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        if (param->param.param.type == dynamic_type_int){
            if (param->param.param.int_value == par_name &&
                    param->param.value.type == dynamic_type_string){
                filename = param->param.value.str_value;
                filename_len = param->param.value.str_len;
            }
            else if (param->param.param.int_value == par_do_in_background){
                do_in_background = dynamic_to_int(&param->param.value);
            }
        }
    }

    if (filename){
        String string = make_string(filename, filename_len);
        if (do_in_background){
            delayed_open_background(delay, string);
        }
        else{
            // TODO(allen): Change the behavior of all delayed_open/background
            // calls so that they still allocate the buffer right away.  This way
            // it's still possible to get at the buffer if so wished in the API.
            // The switch for this view doesn't need to happen until the file is ready.
            delayed_open(delay, string, panel);
        }
    }
    else{
        view_show_interactive(system, view, &models->map_ui,
            IAct_Open, IInt_Sys_File_List, make_lit_string("Open: "));
    }
}

internal void
view_file_in_panel(Command_Data *cmd, Panel *panel, Editing_File *file){
    System_Functions *system = cmd->system;
    Models *models = cmd->models;

    Partition old_part;
    Temp_Memory temp;
    View *old_view;
    Partition *part;

    old_view = cmd->view;
    old_part = cmd->part;

    cmd->view = panel->view;
    part = &models->mem.part;
    temp = begin_temp_memory(part);
    cmd->part = partition_sub_part(part, Kbytes(16));

    view_set_file(panel->view, file, models, system,
        models->hooks[hook_open_file], &app_links);

    cmd->part = old_part;
    end_temp_memory(temp);
    cmd->view = old_view;

    panel->view->map = app_get_map(models, file->settings.base_map_id);
}

// TODO(allen): Improvements to reopen
// - Preserve existing token stack
// - Keep current version open and do some sort of diff to keep
//    the cursor position correct
COMMAND_DECL(reopen){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_MODELS(models);
    USE_VIEW(view);
    REQ_FILE(file, view);
    USE_EXCHANGE(exchange);
    
    if (match(file->name.source_path, file->name.live_name)) return;
    
    i32 file_id = exchange_request_file(exchange, expand_str(file->name.source_path));
    i32 index = 0;
    if (file_id){
        file_set_to_loading(file);
        index = file->id.id;
        app_push_file_binding(vars, file_id, index);

        view_set_file(view, file, models, system,
            models->hooks[hook_open_file], &app_links);
    }
    else{
        // TODO(allen): feedback message
    }
}

COMMAND_DECL(save){
    ProfileMomentFunction();
    USE_MODELS(models);
    USE_VIEW(view);
    USE_FILE(file, view);

    Delay *delay = &models->delay1;

    char *filename = 0;
    int filename_len = 0;
    int buffer_id = -1;

    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int v = dynamic_to_int(&param->param.param);
        if (v == par_name && param->param.value.type == dynamic_type_string){
            filename = param->param.value.str_value;
            filename_len = param->param.value.str_len;
        }
        else if (v == par_buffer_id && param->param.value.type == dynamic_type_int){
            buffer_id = dynamic_to_int(&param->param.value);
        }
    }

    String name = {};
    if (filename){
        name = make_string(filename, filename_len);
    }
    else if (file){
        name = file->name.source_path;
    }

    if (name.size != 0){
        if (buffer_id == -1){
            if (file){
                delayed_save(delay, name, file);
            }
        }
        else{
            file = working_set_get_active_file(&models->working_set, buffer_id);
            
            if (!file->state.is_dummy && file_is_ready(file)){
                delayed_save(delay, name, file);
            }
            else{
                delayed_save(delay, name);
            }
        }
    }
}

COMMAND_DECL(interactive_save_as){
    ProfileMomentFunction();
    USE_VIEW(view);
    USE_MODELS(models);

    view_show_interactive(system, view, &models->map_ui,
        IAct_Save_As, IInt_Sys_File_List, make_lit_string("Save As: "));
}

COMMAND_DECL(change_active_panel){
    ProfileMomentFunction();
    USE_MODELS(models);
    USE_PANEL(panel);

    panel = panel->next;
    if (panel == &models->layout.used_sentinel){
        panel = panel->next;
    }
    models->layout.active_panel = (i32)(panel - models->layout.panels);
}

COMMAND_DECL(interactive_switch_buffer){
    ProfileMomentFunction();
    USE_VIEW(view);
    USE_MODELS(models);

    view_show_interactive(system, view, &models->map_ui,
        IAct_Switch, IInt_Live_File_List, make_lit_string("Switch Buffer: "));
}

COMMAND_DECL(interactive_kill_buffer){
    ProfileMomentFunction();
    USE_VIEW(view);
    USE_MODELS(models);

    view_show_interactive(system, view, &models->map_ui,
        IAct_Kill, IInt_Live_File_List, make_lit_string("Kill Buffer: "));
}

COMMAND_DECL(kill_buffer){
    ProfileMomentFunction();
    USE_MODELS(models);
    USE_VIEW(view);
    USE_FILE(file, view);

    Delay *delay = &models->delay1;
    int buffer_id = 0;

    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int v = dynamic_to_int(&param->param.param);
        if (v == par_buffer_id && param->param.value.type == dynamic_type_int){
            buffer_id = dynamic_to_int(&param->param.value);
        }
    }

    if (buffer_id != 0){
        file = working_set_get_active_file(&models->working_set, buffer_id);
        if (file){
            delayed_kill(delay, file);
        }
    }
    else if (file){
        delayed_try_kill(delay, file, view->panel);
    }
}

COMMAND_DECL(toggle_line_wrap){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
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
    REQ_READABLE_VIEW(view);
    view->show_whitespace = !view->show_whitespace;
}

COMMAND_DECL(toggle_tokens){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    if (file->settings.tokens_exist){
        file_kill_tokens(system, &models->mem.general, file);
    }
    else{
        file_first_lex_parallel(system, &models->mem.general, file);
    }
#endif
}

internal void
case_change_range(System_Functions *system,
    Mem_Options *mem, View *view, Editing_File *file,
    u8 a, u8 z, u8 char_delta){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
    Range range = make_range(view->cursor.pos, view->mark);
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
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);
    case_change_range(system, &models->mem, view, file, 'a', 'z', (u8)('A' - 'a'));
}

COMMAND_DECL(to_lowercase){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);
    case_change_range(system, &models->mem, view, file, 'A', 'Z', (u8)('a' - 'A'));
}

COMMAND_DECL(clean_all_lines){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    view_clean_whitespace(system, models, view);
}

COMMAND_DECL(eol_dosify){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    file->settings.dos_write_mode = 1;
    file->state.last_4ed_edit_time = system->time();
}

COMMAND_DECL(eol_nixify){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    file->settings.dos_write_mode = 0;
    file->state.last_4ed_edit_time = system->time();
}

COMMAND_DECL(auto_tab_range){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    int r_start = 0, r_end = 0;
    int start_set = 0, end_set = 0;
    int clear_blank_lines = 1;

    // TODO(allen): deduplicate
    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int p = dynamic_to_int(&param->param.param);
        switch (p){
            case par_range_start:
            start_set = 1;
            r_start = dynamic_to_int(&param->param.value);
            break;

            case par_range_end:
            end_set = 1;
            r_end = dynamic_to_int(&param->param.value);
            break;

            case par_clear_blank_lines:
            clear_blank_lines = dynamic_to_bool(&param->param.value);
            break;
        }
    }

    if (file->state.token_stack.tokens && file->state.tokens_complete && !file->state.still_lexing){
        Range range = make_range(view->cursor.pos, view->mark);
        if (start_set) range.start = r_start;
        if (end_set) range.end = r_end;
        view_auto_tab_tokens(system, models, view, range.start, range.end, clear_blank_lines);
    }
}

COMMAND_DECL(open_panel_vsplit){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_MODELS(models);
    USE_PANEL(panel);
    USE_EXCHANGE(exchange);

    if (models->layout.panel_count < models->layout.panel_max_count){
        Split_Result split = layout_split_panel(&models->layout, panel, 1);

        Panel *panel1 = panel;
        Panel *panel2 = split.panel;

        panel2->screen_region = panel1->screen_region;

        panel2->full.x0 = split.divider->pos;
        panel2->full.x1 = panel1->full.x1;
        panel1->full.x1 = split.divider->pos;

        panel_fix_internal_area(panel1);
        panel_fix_internal_area(panel2);
        panel2->prev_inner = panel2->inner;

        models->layout.active_panel = (i32)(panel2 - models->layout.panels);
        panel_make_empty(system, exchange, vars, panel2);
    }
}

COMMAND_DECL(open_panel_hsplit){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_MODELS(models);
    USE_PANEL(panel);
    USE_EXCHANGE(exchange);

    if (models->layout.panel_count < models->layout.panel_max_count){
        Split_Result split = layout_split_panel(&models->layout, panel, 0);

        Panel *panel1 = panel;
        Panel *panel2 = split.panel;

        panel2->screen_region = panel1->screen_region;

        panel2->full.y0 = split.divider->pos;
        panel2->full.y1 = panel1->full.y1;
        panel1->full.y1 = split.divider->pos;

        panel_fix_internal_area(panel1);
        panel_fix_internal_area(panel2);
        panel2->prev_inner = panel2->inner;

        models->layout.active_panel = (i32)(panel2 - models->layout.panels);
        panel_make_empty(system, exchange, vars, panel2);
    }
}

COMMAND_DECL(close_panel){
    ProfileMomentFunction();
    USE_MODELS(models);
    USE_PANEL(panel);
    USE_VIEW(view);
    USE_EXCHANGE(exchange);

    Panel *panel_ptr, *used_panels;
    Divider_And_ID div, parent_div, child_div;
    i32 child;
    i32 parent;
    i32 which_child;
    i32 active;

    if (models->layout.panel_count > 1){
        live_set_free_view(system, exchange, command->live_set, view);
        panel->view = 0;

        div = layout_get_divider(&models->layout, panel->parent);

        // This divider cannot have two child dividers.
        Assert(div.divider->child1 == -1 || div.divider->child2 == -1);

        // Get the child who needs to fill in this node's spot
        child = div.divider->child1;
        if (child == -1) child = div.divider->child2;

        parent = div.divider->parent;
        which_child = div.divider->which_child;

        // Fill the child in the slot this node use to hold
        if (parent == -1){
            Assert(models->layout.root == div.id);
            models->layout.root = child;
        }
        else{
            parent_div = layout_get_divider(&models->layout, parent);
            if (which_child == -1){
                parent_div.divider->child1 = child;
            }
            else{
                parent_div.divider->child2 = child;
            }
        }

        // If there was a child divider, give it information about it's new parent.
        if (child != -1){
            child_div = layout_get_divider(&models->layout, child);
            child_div.divider->parent = parent;
            child_div.divider->which_child = div.divider->which_child;
        }

        // What is the new active panel?
        active = -1;
        if (child == -1){
            used_panels = &models->layout.used_sentinel;
            for (dll_items(panel_ptr, used_panels)){
                if (panel_ptr != panel && panel_ptr->parent == div.id){
                    panel_ptr->parent = parent;
                    panel_ptr->which_child = which_child;
                    active = (i32)(panel_ptr - models->layout.panels);
                    break;
                }
            }
        }
        else{
            panel_ptr = panel->next;
            if (panel_ptr == &models->layout.used_sentinel) panel_ptr = panel_ptr->next;
            Assert(panel_ptr != panel);
            active = (i32)(panel_ptr - models->layout.panels);
        }

        Assert(active != -1 && panel != models->layout.panels + active);
        models->layout.active_panel = active;

        layout_free_divider(&models->layout, div.divider);
        layout_free_panel(&models->layout, panel);
        layout_fix_all_panels(&models->layout);
    }
}

COMMAND_DECL(move_left){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = view->cursor.pos;
    if (pos > 0) --pos;
    view_cursor_move(view, pos);
}

COMMAND_DECL(move_right){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 size = buffer_size(&file->state.buffer);
    i32 pos = view->cursor.pos;
    if (pos < size) ++pos;
    view_cursor_move(view, pos);
}

COMMAND_DECL(delete){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    i32 size = buffer_size(&file->state.buffer);
    i32 cursor_pos = view->cursor.pos;
    if (0 < size && cursor_pos < size){
        i32 start, end;
        start = cursor_pos;
        end = cursor_pos+1;

        Assert(end - start > 0);

        i32 next_cursor_pos = start;
        view_replace_range(system, models, view, start, end, 0, 0, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
    }
}

COMMAND_DECL(backspace){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    i32 size = buffer_size(&file->state.buffer);
    i32 cursor_pos = view->cursor.pos;
    if (cursor_pos > 0 && cursor_pos <= size){
        i32 start, end;
        end = cursor_pos;
        start = cursor_pos-1;

        i32 shift = (end - start);
        Assert(shift > 0);

        i32 next_cursor_pos = view->cursor.pos - shift;
        view_replace_range(system, models, view, start, end, 0, 0, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
    }
}

COMMAND_DECL(move_up){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    f32 font_height = (f32)get_font_info(models->font_set, models->global_font.font_id)->height;
    f32 cy = view_get_cursor_y(view)-font_height;
    f32 px = view->preferred_x;
    if (cy >= 0){
        view->cursor = view_compute_cursor_from_xy(view, px, cy);
        file->state.cursor_pos = view->cursor.pos;
    }
}

COMMAND_DECL(move_down){
    ProfileMomentFunction();
    USE_MODELS(models);
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    f32 font_height = (f32)get_font_info(models->font_set, models->global_font.font_id)->height;
    f32 cy = view_get_cursor_y(view)+font_height;
    f32 px = view->preferred_x;
    view->cursor = view_compute_cursor_from_xy(view, px, cy);
    file->state.cursor_pos = view->cursor.pos;
}

COMMAND_DECL(seek_end_of_line){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = view_find_end_of_line(view, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_beginning_of_line){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = view_find_beginning_of_line(view, view->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(page_down){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);

    f32 height = view_compute_height(view);
    f32 max_target_y = view_compute_max_target_y(view);

    view->target_y += height;
    if (view->target_y > max_target_y) view->target_y = max_target_y;

    view->cursor = view_compute_cursor_from_xy(
        view, 0, view->target_y + (height - view->font_height)*.5f);
}

COMMAND_DECL(page_up){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);

    f32 height = view_compute_height(view);

    view->target_y -= height;
    if (view->target_y < 0) view->target_y = 0;

    view->cursor = view_compute_cursor_from_xy(
        view, 0, view->target_y + (height - view->font_height)*.5f);
}

COMMAND_DECL(open_color_tweaker){
    ProfileMomentFunction();
    USE_VIEW(view);
    USE_MODELS(models);

    view_show_theme(view, &models->map_ui);
}

COMMAND_DECL(open_config){
    ProfileMomentFunction();
    USE_VIEW(view);
    USE_MODELS(models);

    view_show_config(view, &models->map_ui);
}

COMMAND_DECL(open_menu){
    ProfileMomentFunction();
    USE_VIEW(view);
    USE_MODELS(models);

    view_show_menu(view, &models->map_ui);
}

COMMAND_DECL(close_minor_view){
    ProfileMomentFunction();
    USE_VIEW(view);
    USE_MODELS(models);

    Command_Map *map = &models->map_top;
    if (view->file){
        map = app_get_map(models, view->file->settings.base_map_id);
    }
    view_show_file(view, map);
}

COMMAND_DECL(cursor_mark_swap){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);

    i32 pos = view->cursor.pos;
    view_cursor_move(view, view->mark);
    view->mark = pos;
}

COMMAND_DECL(user_callback){
    ProfileMomentFunction();
    if (binding.custom) binding.custom(&app_links);
}

COMMAND_DECL(set_settings){
    ProfileMomentFunction();
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    USE_MODELS(models);

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
                    if (!v) file_kill_tokens(system, &models->mem.general, file);
                }
                else{
                    if (v) file_first_lex_parallel(system, &models->mem.general, file);
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
                    int index = app_get_map_index(models, v);
                    if (index < models->user_map_count) file->settings.base_map_id = v;
                    else file->settings.base_map_id = mapid_file;
                }
            }break;
        }
    }
}

COMMAND_DECL(command_line){
    ProfileMomentFunction();
    USE_VARS(vars);
    USE_MODELS(models);
    USE_PANEL(panel);
    USE_VIEW(view);

    Partition *part = &models->mem.part;

    char *buffer_name = 0;
    char *path = 0;
    char *script = 0;

    i32 buffer_id = 0;
    i32 buffer_name_len = 0;
    i32 path_len = 0;
    i32 script_len = 0;
    u32 flags = CLI_OverlapWithConflict;
    b32 do_in_background = 0;

    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int p = dynamic_to_int(&param->param.param);
        switch (p){
            case par_name:
            {
                char *new_buffer_name = dynamic_to_string(&param->param.value, &buffer_name_len);
                if (new_buffer_name){
                    buffer_name = new_buffer_name;
                }
            }break;

            case par_buffer_id:
            {
                buffer_id = dynamic_to_int(&param->param.value);
            }break;

            case par_do_in_background:
            {
                do_in_background = 1;
            }break;

            case par_cli_path:
            {
                char *new_cli_path = dynamic_to_string(&param->param.value, &path_len);
                if (new_cli_path){
                    path = new_cli_path;
                }
            }break;

            case par_cli_command:
            {
                char *new_command = dynamic_to_string(&param->param.value, &script_len);
                if (new_command){
                    script = new_command;
                }
            }break;

            case par_flags:
            {
                flags = (u32)dynamic_to_int(&param->param.value);
            }break;
        }
    }

    {
        Working_Set *working_set = &models->working_set;
        CLI_Process *procs = vars->cli_processes.procs, *proc = 0;
        Editing_File *file = 0;
        b32 bind_to_new_view = !do_in_background;

        if (vars->cli_processes.count < vars->cli_processes.max){
            if (buffer_id){
                file = working_set_get_active_file(working_set, buffer_id);
                if (file && file->settings.read_only == 0){
                    // TODO(allen): feedback message - file not read only
                    return;
                }
            }
            else if (buffer_name){
                file = working_set_contains(system, working_set, make_string(buffer_name, buffer_name_len));
                if (file){
                    if (file->settings.read_only == 0){
                        // TODO(allen): feedback message - file not read only
                        return;
                    }
                }
                else{
                    file = working_set_alloc_always(working_set, &models->mem.general);
                    
                    file_create_read_only(system, models, file, buffer_name);
                    working_set_add(system, working_set, file);
                    
                    if (file == 0){
                        // TODO(allen): feedback message - no available file
                        return;
                    }
                }
            }
            
            if (file){
                i32 proc_count = vars->cli_processes.count;
                View_Iter iter;
                i32 i;
                
                for (i = 0; i < proc_count; ++i){
                    if (procs[i].out_file == file){
                        if (flags & CLI_OverlapWithConflict)
                            procs[i].out_file = 0;
                        else
                            file = 0;
                        break;
                    }
                }
                
                if (file){
                    file_clear(system, models, file, 1);
                    file->settings.unimportant = 1;

                    if (!(flags & CLI_AlwaysBindToView)){
                        iter = file_view_iter_init(&models->layout, file, 0);
                        if (file_view_iter_good(iter)){
                            bind_to_new_view = 0;
                        }
                    }
                }
                else{
                    // TODO(allen): feedback message - file conflict
                    return;
                }
            }
            
            if (!path){
                path = models->hot_directory.string.str;
                terminate_with_null(&models->hot_directory.string);
            }

            {
                Temp_Memory temp;
                Range range;
                Editing_File *view_file;
                i32 size;

                temp = begin_temp_memory(part);
                if (!script){
                    view_file = view->file;
                    if (view_file){
                        range = make_range(view->cursor.pos, view->mark);
                        size = range.end - range.start;
                        script = push_array(part, char, size + 1);
                        buffer_stringify(&view_file->state.buffer, range.start, range.end, script);
                        script[size] = 0;
                    }
                    else{
                        script = " echo no script specified";
                    }
                }

                if (bind_to_new_view){
                    view_file_in_panel(command, panel, file);
                }

                proc = procs + vars->cli_processes.count++;
                proc->out_file = file;

                if (!system->cli_call(path, script, &proc->cli)){
                    --vars->cli_processes.count;
                }
                end_temp_memory(temp);
            }
        }
        else{
            // TODO(allen): feedback message - no available process slot
            return;
        }
    }
}

internal void
update_command_data(App_Vars *vars, Command_Data *cmd){
    cmd->panel = cmd->models->layout.panels + cmd->models->layout.active_panel;
    cmd->view = cmd->panel->view;
}

globalvar Command_Function command_table[cmdid_count];

internal void
fill_buffer_summary(Buffer_Summary *buffer, Editing_File *file, Working_Set *working_set){
    *buffer = {};
    if (!file->state.is_dummy){
        buffer->exists = 1;
        buffer->ready = file_is_ready(file);

        buffer->is_lexed = file->settings.tokens_exist;
        buffer->buffer_id = file->id.id;
        buffer->size = file->state.buffer.size;
        buffer->buffer_cursor_pos = file->state.cursor_pos;

        buffer->file_name_len = file->name.source_path.size;
        buffer->buffer_name_len = file->name.live_name.size;
        buffer->file_name = file->name.source_path.str;
        buffer->buffer_name = file->name.live_name.str;

        buffer->map_id = file->settings.base_map_id;
    }
}

internal void
fill_view_summary(View_Summary *view, View *vptr, Live_Views *live_set, Working_Set *working_set){
    i32 lock_level;
    int buffer_id;
    *view = {};

    if (vptr->in_use){
        view->exists = 1;
        view->view_id = (int)(vptr - live_set->views) + 1;
        view->line_height = vptr->font_height;
        view->unwrapped_lines = vptr->unwrapped_lines;

        if (vptr->file){
            lock_level = view_lock_level(vptr);
            buffer_id = vptr->file->id.id;

            if (lock_level <= 0){
                view->buffer_id = buffer_id;
            }
            else{
                view->buffer_id = 0;
            }

            if (lock_level <= 1){
                view->locked_buffer_id = buffer_id;
            }
            else{
                view->locked_buffer_id = 0;
            }

            if (lock_level <= 2){
                view->hidden_buffer_id = buffer_id;
            }
            else{
                view->hidden_buffer_id = 0;
            }

            view->mark = view_compute_cursor_from_pos(vptr, vptr->mark);
            view->cursor = vptr->cursor;
            view->preferred_x = vptr->preferred_x;
        }
    }
}

extern "C"{
    EXECUTE_COMMAND_SIG(external_exec_command_keep_stack){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Command_Function function = command_table[command_id];
        Command_Binding binding = {};
        binding.function = function;
        if (function) function(cmd->system, cmd, binding);

        update_command_data(cmd->vars, cmd);
    }

    PUSH_PARAMETER_SIG(external_push_parameter){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Partition *part = &cmd->part;
        Command_Parameter *cmd_param = push_struct(part, Command_Parameter);
        cmd_param->type = 0;
        cmd_param->param.param = param;
        cmd_param->param.value = value;
    }

    PUSH_MEMORY_SIG(external_push_memory){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
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
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        cmd->part.pos = 0;
    }

    DIRECTORY_GET_HOT_SIG(external_directory_get_hot){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Hot_Directory *hot = &cmd->models->hot_directory;
        i32 copy_max = capacity - 1;
        hot_directory_clean_end(hot);
        if (copy_max > hot->string.size)
            copy_max = hot->string.size;
        memcpy(out, hot->string.str, copy_max);
        out[copy_max] = 0;
        return(hot->string.size);
    }

    GET_FILE_LIST_SIG(external_get_file_list){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        System_Functions *system = cmd->system;
        File_List result = {};
        system->set_file_list(&result, make_string(dir, len));
        return(result);
    }

    FREE_FILE_LIST_SIG(external_free_file_list){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        System_Functions *system = cmd->system;
        system->set_file_list(&list, make_string(0, 0));
    }

    GET_BUFFER_FIRST_SIG(external_get_buffer_first){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Working_Set *working_set = &cmd->models->working_set;
        Buffer_Summary result = {};
        if (working_set->file_count > 0){
            fill_buffer_summary(&result, (Editing_File*)working_set->used_sentinel.next, working_set);
        }
        return(result);
    }

    GET_BUFFER_NEXT_SIG(external_get_buffer_next){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Working_Set *working_set = &cmd->models->working_set;
        Editing_File *file;

        file = working_set_get_active_file(working_set, buffer->buffer_id);
        if (file){
            file = (Editing_File*)file->node.next;
            fill_buffer_summary(buffer, file, working_set);
        }
        else{
            *buffer = {};
        }
    }

    GET_BUFFER_SIG(external_get_buffer){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Working_Set *working_set = &cmd->models->working_set;
        Buffer_Summary buffer = {};
        Editing_File *file;

        file = working_set_get_active_file(working_set, index);
        if (file){
            fill_buffer_summary(&buffer, file, working_set);
        }

        return(buffer);
    }

    GET_ACTIVE_BUFFER_SIG(external_get_active_buffer){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Buffer_Summary buffer = {};
        View *view = cmd->view;
        Editing_File *file;

        if (view_lock_level(view) <= LockLevel_Open){
            file = view->file;
            if (file){
                fill_buffer_summary(&buffer, file, &cmd->models->working_set);
            }
        }

        return(buffer);
    }

    GET_PARAMETER_BUFFER_SIG(external_get_parameter_buffer){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Models *models = cmd->models;
        Buffer_Summary buffer = {};

        if (param_index >= 0 && param_index < models->buffer_param_count){
            buffer = external_get_buffer(app, models->buffer_param_indices[param_index]);
        }

        return(buffer);
    }

    GET_BUFFER_BY_NAME(external_get_buffer_by_name){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Buffer_Summary buffer = {};
        Editing_File *file;
        Working_Set *working_set = &cmd->models->working_set;

        file = working_set_contains(cmd->system, working_set, make_string(filename, len));
        if (file && !file->state.is_dummy){
            fill_buffer_summary(&buffer, file, working_set);
        }
        
        return(buffer);
    }

    BUFFER_SEEK_DELIMITER_SIG(external_buffer_seek_delimiter){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Editing_File *file;
        Working_Set *working_set;
        int result = 0;
        int size;

        if (buffer->exists){
            working_set = &cmd->models->working_set;
            file = working_set_get_active_file(working_set, buffer->buffer_id);
            if (file && file_is_ready(file)){
                size = buffer_size(&file->state.buffer);
                result = 1;

                if (start < 0 && !seek_forward) *out = start;
                else if (start >= size && seek_forward) *out = start;
                else{
                    if (seek_forward){
                        *out = buffer_seek_delimiter(&file->state.buffer, start, delim);
                    }
                    else{
                        *out = buffer_reverse_seek_delimiter(&file->state.buffer, start, delim);
                    }
                }

                fill_buffer_summary(buffer, file, working_set);
            }
        }

        return(result);
    }

    BUFFER_SEEK_STRING_SIG(external_buffer_seek_string){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Models *models;
        Editing_File *file;
        Working_Set *working_set;
        Partition *part;
        Temp_Memory temp;
        char *spare;
        int result = 0;
        int size;

        if (buffer->exists){
            models = cmd->models;
            working_set = &models->working_set;
            file = working_set_get_active_file(working_set, buffer->buffer_id);
            if (file && file_is_ready(file)){
                size = buffer_size(&file->state.buffer);

                if (start < 0 && !seek_forward) *out = start;
                else if (start >= size && seek_forward) *out = start;
                else{
                    part = &models->mem.part;
                    temp = begin_temp_memory(part);
                    spare = push_array(part, char, len);
                    result = 1;
                    if (seek_forward){
                        *out = buffer_find_string(&file->state.buffer, start, size, str, len, spare);
                    }
                    else{
                        *out = buffer_rfind_string(&file->state.buffer, start, str, len, spare);
                    }
                    end_temp_memory(temp);
                }
                fill_buffer_summary(buffer, file, working_set);
            }
        }

        return(result);
    }

    REFRESH_BUFFER_SIG(external_refresh_buffer){
        int result;
        *buffer = external_get_buffer(app, buffer->buffer_id);
        result = buffer->exists;
        return(result);
    }

    BUFFER_READ_RANGE_SIG(external_buffer_read_range){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Editing_File *file;
        Working_Set *working_set;
        int result = 0;
        int size;

        if (buffer->exists){
            working_set = &cmd->models->working_set;
            file = working_set_get_active_file(working_set, buffer->buffer_id);
            if (file && file_is_ready(file)){
                size = buffer_size(&file->state.buffer);
                if (0 <= start && start <= end && end <= size){
                    result = 1;
                    buffer_stringify(&file->state.buffer, start, end, out);
                }
                fill_buffer_summary(buffer, file, working_set);
            }
        }

        return(result);
    }

    BUFFER_REPLACE_RANGE_SIG(external_buffer_replace_range){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Editing_File *file;
        Working_Set *working_set;

        Models *models;

        int result = 0;
        int size;
        int next_cursor, pos;

        if (buffer->exists){
            models = cmd->models;
            working_set = &models->working_set;
            file = working_set_get_active_file(working_set, buffer->buffer_id);
            if (file && file_is_ready(file)){
                size = buffer_size(&file->state.buffer);
                if (0 <= start && start <= end && end <= size){
                    result = 1;

                    pos = file->state.cursor_pos;
                    if (pos < start) next_cursor = pos;
                    else if (pos < end) next_cursor = start;
                    else next_cursor = pos + end - start - len;

                    file_replace_range(cmd->system, models, file, start, end, str, len, next_cursor);
                }
                fill_buffer_summary(buffer, file, working_set);
            }
        }

        return(result);
    }

    BUFFER_SET_POS_SIG(external_buffer_set_pos){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Editing_File *file;
        Working_Set *working_set;

        int result = 0;
        int size;

        if (buffer->exists){
            working_set = &cmd->models->working_set;
            file = working_set_get_active_file(working_set, buffer->buffer_id);
            if (file && file_is_ready(file)){
                result = 1;
                size = buffer_size(&file->state.buffer);
                if (pos < 0) pos = 0;
                if (pos > size) pos = size;
                file->state.cursor_pos = pos;
                fill_buffer_summary(buffer, file, working_set);
            }
        }

        return(result);
    }

    GET_VIEW_FIRST_SIG(external_get_view_first){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Editing_Layout *layout = &cmd->models->layout;
        View_Summary view = {};

        Panel *panel = layout->used_sentinel.next;

        Assert(panel != &layout->used_sentinel);
        fill_view_summary(&view, panel->view, &cmd->vars->live_set, &cmd->models->working_set);

        return(view);
    }

    GET_VIEW_NEXT_SIG(external_get_view_next){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Editing_Layout *layout = &cmd->models->layout;
        Live_Views *live_set = &cmd->vars->live_set;
        View *vptr;
        Panel *panel;
        int index = view->view_id - 1;

        if (index >= 0 && index < live_set->max){
            vptr = live_set->views + index;
            panel = vptr->panel;
            if (panel) panel = panel->next;
            if (panel && panel != &layout->used_sentinel){
                fill_view_summary(view, panel->view, &cmd->vars->live_set, &cmd->models->working_set);
            }
            else{
                *view = {};
            }
        }
        else{
            *view = {};
        }
    }

    GET_VIEW_SIG(external_get_view){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        View_Summary view = {};
        Live_Views *live_set = cmd->live_set;
        int max = live_set->max;
        View *vptr;

        index -= 1;
        if (index >= 0 && index < max){
            vptr = live_set->views + index;
            fill_view_summary(&view, vptr, live_set, &cmd->models->working_set);
        }

        return(view);
    }

    GET_ACTIVE_VIEW_SIG(external_get_active_view){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        View_Summary view = {};

        fill_view_summary(&view, cmd->view, &cmd->vars->live_set, &cmd->models->working_set);

        return(view);
    }

    REFRESH_VIEW_SIG(external_refresh_view){
        int result;
        *view = external_get_view(app, view->view_id);
        result = view->exists;
        return(result);
    }

    VIEW_SET_CURSOR_SIG(external_view_set_cursor){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Live_Views *live_set;
        View *vptr;
        int result = 0;
        int view_id;

        if (view->exists){
            live_set = cmd->live_set;
            view_id = view->view_id - 1;
            if (view_id >= 0 && view_id < live_set->max){
                vptr = live_set->views + view_id;
                result = 1;
                if (seek.type == buffer_seek_line_char && seek.character <= 0){
                    seek.character = 1;
                }
                vptr->cursor = view_compute_cursor(vptr, seek);
                if (set_preferred_x){
                    vptr->preferred_x = view_get_cursor_x(vptr);
                }
                fill_view_summary(view, vptr, live_set, &cmd->models->working_set);
            }
        }

        return(result);
    }

    VIEW_SET_MARK_SIG(external_view_set_mark){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Live_Views *live_set;
        View *vptr;
        Full_Cursor cursor;
        int result = 0;
        int view_id;

        if (view->exists){
            live_set = cmd->live_set;
            view_id = view->view_id - 1;
            if (view_id >= 0 && view_id < live_set->max){
                vptr = live_set->views + view_id;
                result = 1;
                if (seek.type != buffer_seek_pos){
                    cursor = view_compute_cursor(vptr, seek);
                    vptr->mark = cursor.pos;
                }
                else{
                    vptr->mark = seek.pos;
                }
                fill_view_summary(view, vptr, live_set, &cmd->models->working_set);
            }
        }

        return(result);
    }

    VIEW_SET_HIGHLIGHT_SIG(external_view_set_highlight){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Live_Views *live_set;
        View *vptr;
        int result = 0;
        int view_id;

        if (view->exists){
            live_set = cmd->live_set;
            view_id = view->view_id - 1;
            if (view_id >= 0 && view_id < live_set->max){
                vptr = live_set->views + view_id;
                result = 1;
                if (turn_on){
                    view_set_temp_highlight(vptr, start, end);
                }
                else{
                    vptr->show_temp_highlight = 0;
                }
                fill_view_summary(view, vptr, live_set, &cmd->models->working_set);
            }
        }

        return(result);
    }

    VIEW_SET_BUFFER_SIG(external_view_set_buffer){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Live_Views *live_set;
        View *vptr;
        Editing_File *file;
        Working_Set *working_set;
        Models *models;
        int result = 0;
        int view_id;

        if (view->exists){
            models = cmd->models;
            live_set = cmd->live_set;
            view_id = view->view_id - 1;
            if (view_id >= 0 && view_id < live_set->max){
                vptr = live_set->views + view_id;
                working_set = &models->working_set;
                file = working_set_get_active_file(working_set, buffer_id);

                if (file){
                    result = 1;
                    if (file != vptr->file){
                        view_set_file(vptr, file, models, cmd->system,
                            models->hooks[hook_open_file], &app_links);
                    }
                }

                fill_view_summary(view, vptr, live_set, working_set);
            }
        }

        return(result);
    }

    GET_USER_INPUT_SIG(external_get_user_input){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        System_Functions *system = cmd->system;
        Coroutine *coroutine = cmd->models->command_coroutine;
        User_Input result;

        Assert(coroutine);
        *((u32*)coroutine->out+0) = get_type;
        *((u32*)coroutine->out+1) = abort_type;
        system->yield_coroutine(coroutine);
        result = *(User_Input*)coroutine->in;

        return(result);
    }

    START_QUERY_BAR_SIG(external_start_query_bar){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Query_Slot *slot = 0;
        View *vptr;

        vptr = cmd->view;

        slot = alloc_query_slot(&vptr->query_set);
        slot->query_bar = bar;

        return(slot != 0);
    }

    END_QUERY_BAR_SIG(external_end_query_bar){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        View *vptr;

        vptr = cmd->view;
        free_query_slot(&vptr->query_set, bar);
    }
    
    CHANGE_THEME_SIG(external_change_theme){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Style_Library *styles = &cmd->models->styles;
        String theme_name = make_string(name, len);
        Style *s;
        i32 i, count;
        
        count = styles->count;
        s = styles->styles;
        for (i = 0; i < count; ++i, ++s){
            if (match(s->name, theme_name)){
                style_copy(&cmd->models->style, s);
                break;
            }
        }
    }
    
    CHANGE_FONT_SIG(external_change_font){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Font_Set *set = cmd->models->font_set;
        Style_Font *global_font = &cmd->models->global_font;
        String font_name = make_string(name, len);
        i16 font_id;
        
        if (font_set_extract(set, font_name, &font_id)){
            global_font->font_id = font_id;
            global_font->font_changed = 1;
        }
    }
    
    SET_THEME_COLORS_SIG(external_set_theme_colors){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Style *style = &cmd->models->style;
        Theme_Color *theme_color;
        u32 *color;
        i32 i;

        theme_color = colors;
        for (i = 0; i < count; ++i, ++theme_color){
            color = style_index_by_tag(&style->main, theme_color->tag);
            if (color) *color = theme_color->color | 0xFF000000;
        }
    }
}

struct Command_In{
    Command_Data *cmd;
    Command_Binding bind;
};

internal void
command_caller(Coroutine *coroutine){
    Command_In *cmd_in = (Command_In*)coroutine->in;
    Command_Data *cmd = cmd_in->cmd;
    View *view = cmd->view;

    // TODO(allen): this isn't really super awesome, could have issues if
    // the file view get's change out under us.
    view->next_mode = {};
    cmd_in->bind.function(cmd->system, cmd, cmd_in->bind);
    view->mode = view->next_mode;
}

internal void
app_links_init(System_Functions *system, void *data, int size){
    app_links.memory = data;
    app_links.memory_size = size;

    app_links.exec_command_keep_stack = external_exec_command_keep_stack;
    app_links.push_parameter = external_push_parameter;
    app_links.push_memory = external_push_memory;
    app_links.clear_parameters = external_clear_parameters;

    app_links.directory_get_hot = external_directory_get_hot;
    app_links.file_exists = system->file_exists;
    app_links.directory_cd = system->directory_cd;
    app_links.get_file_list = external_get_file_list;
    app_links.free_file_list = external_free_file_list;

    app_links.get_buffer_first = external_get_buffer_first;
    app_links.get_buffer_next = external_get_buffer_next;

    app_links.get_buffer = external_get_buffer;
    app_links.get_active_buffer = external_get_active_buffer;
    app_links.get_parameter_buffer = external_get_parameter_buffer;
    app_links.get_buffer_by_name = external_get_buffer_by_name;

    app_links.refresh_buffer = external_refresh_buffer;
    app_links.buffer_seek_delimiter = external_buffer_seek_delimiter;
    app_links.buffer_seek_string = external_buffer_seek_string;
    app_links.buffer_read_range = external_buffer_read_range;
    app_links.buffer_replace_range = external_buffer_replace_range;

    app_links.get_view_first = external_get_view_first;
    app_links.get_view_next = external_get_view_next;

    app_links.get_view = external_get_view;
    app_links.get_active_view = external_get_active_view;

    app_links.refresh_view = external_refresh_view;
    app_links.view_set_cursor = external_view_set_cursor;
    app_links.view_set_mark = external_view_set_mark;
    app_links.view_set_highlight = external_view_set_highlight;
    app_links.view_set_buffer = external_view_set_buffer;

    app_links.get_user_input = external_get_user_input;

    app_links.start_query_bar = external_start_query_bar;
    app_links.end_query_bar = external_end_query_bar;

    app_links.change_theme = external_change_theme;
    app_links.change_font = external_change_font;
    app_links.set_theme_colors = external_set_theme_colors;
}

internal void
setup_ui_commands(Command_Map *commands, Partition *part, Command_Map *parent){
    map_init(commands, part, 32, parent);

    commands->vanilla_keyboard_default.function = command_null;

    // TODO(allen): This is hacky, when the new UI stuff happens, let's fix it, and by that
    // I mean actually fix it, don't just say you fixed it with something stupid again.
    u8 mdfr;
    u8 mdfr_array[] = {MDFR_NONE, MDFR_SHIFT, MDFR_CTRL, MDFR_SHIFT | MDFR_CTRL};
    for (i32 i = 0; i < 4; ++i){
        mdfr = mdfr_array[i];
        map_add(commands, key_left, mdfr, command_null);
        map_add(commands, key_right, mdfr, command_null);
        map_add(commands, key_up, mdfr, command_null);
        map_add(commands, key_down, mdfr, command_null);
        map_add(commands, key_back, mdfr, command_null);
        map_add(commands, key_esc, mdfr, command_close_minor_view);
    }
}

internal void
setup_file_commands(Command_Map *commands, Partition *part, Command_Map *parent){
    map_init(commands, part, 10, parent);
}

internal void
setup_top_commands(Command_Map *commands, Partition *part, Command_Map *parent){
    map_init(commands, part, 10, parent);
}

internal void
setup_command_table(){
#define SET(n) command_table[cmdid_##n] = command_##n

    SET(null);
    SET(write_character);
    SET(seek_left);
    SET(seek_right);
    SET(seek_whitespace_up);
    SET(seek_whitespace_down);
    SET(word_complete);
    SET(set_mark);
    SET(copy);
    SET(cut);
    SET(paste);
    SET(paste_next);
    SET(delete_range);
    SET(timeline_scrub);
    SET(undo);
    SET(redo);
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
    SET(cursor_mark_swap);
    SET(open_menu);
    SET(set_settings);
    SET(command_line);

#undef SET
}

// App Functions

internal void
app_hardcode_styles(Models *models){
    Interactive_Style file_info_style;
    Style *styles, *style;
    styles = models->styles.styles;
    style = styles;

    i16 fonts = 1;
    models->global_font.font_id = fonts + 0;
    models->global_font.font_changed = 0;

    /////////////////
    style_set_name(style, make_lit_string("4coder"));

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
    ++style;

    /////////////////
    style_set_name(style, make_lit_string("Handmade Hero"));

    style->main.back_color = 0xFF161616;
    style->main.margin_color = 0xFF262626;
    style->main.margin_hover_color = 0xFF333333;
    style->main.margin_active_color = 0xFF404040;
    style->main.cursor_color = 0xFF40FF40;
    style->main.at_cursor_color = style->main.back_color;
    style->main.mark_color = 0xFF808080;
    style->main.highlight_color = 0xFF703419;
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
    file_info_style.pop1_color = 0xFF03CF0C;
    file_info_style.pop2_color = 0xFFFF0000;
    style->main.file_info_style = file_info_style;
    ++style;

    /////////////////
    style_set_name(style, make_lit_string("Twilight"));

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
    ++style;

    /////////////////
    style_set_name(style, make_lit_string("Wolverine"));

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
    ++style;

    /////////////////
    style_set_name(style, make_lit_string("stb"));

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
    style->main.comment_color = 0xFF005800;
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
    ++style;

    models->styles.count = (i32)(style - styles);
    models->styles.max = ArrayCount(models->styles.styles);
    style_copy(&models->style, models->styles.styles);
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
    vars->models.mem.part = _partition;

    general_memory_open(&vars->models.mem.general, memory->target_memory, memory->target_memory_size);

    return(vars);
}

internal i32
execute_special_tool(void *memory, i32 size, Command_Line_Parameters clparams){
    i32 result;
    char message[] = "tool was not specified or is invalid";
    result = sizeof(message) - 1;
    memcpy(memory, message, result);
    if (clparams.argc > 2){
        if (match(clparams.argv[2], "version")){
            result = sizeof(VERSION) - 1;
            memcpy(memory, VERSION, result);
        }
    }
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
            init_command_line_settings(&vars->models.settings, plat_settings, clparams);
        }
        else{
            vars->models.settings = {};
        }
        *files = vars->models.settings.init_files;
        *file_count = &vars->models.settings.init_files_count;
    }

    return(out_size);
}

extern "C" SCROLL_RULE_SIG(fallback_scroll_rule){
    int result = 0;

    if (target_x != *scroll_x){
        *scroll_x = target_x;
        result = 1;
    }
    if (target_y != *scroll_y){
        *scroll_y = target_y;
        result = 1;
    }

    return(result);
}


App_Init_Sig(app_init){
    App_Vars *vars;
    Models *models;
    Partition *partition;
    Panel *panels, *panel;
    Panel_Divider *dividers, *div;
    i32 panel_max_count;
    i32 divider_max_count;

    app_links_init(system, memory->user_memory, memory->user_memory_size);

    vars = (App_Vars*)memory->vars_memory;
    models = &vars->models;

    models->config_api = api;
    app_links.cmd_context = &vars->command_data;

    partition = &models->mem.part;
    target->partition = partition;

    {
        i32 i;

        panel_max_count = models->layout.panel_max_count = 16;
        divider_max_count = panel_max_count - 1;
        models->layout.panel_count = 0;

        panels = push_array(partition, Panel, panel_max_count);
        models->layout.panels = panels;

        dll_init_sentinel(&models->layout.free_sentinel);
        dll_init_sentinel(&models->layout.used_sentinel);

        panel = panels;
        for (i = 0; i < panel_max_count; ++i, ++panel){
            dll_insert(&models->layout.free_sentinel, panel);
        }

        dividers = push_array(partition, Panel_Divider, divider_max_count);
        models->layout.dividers = dividers;

        div = dividers;
        for (i = 0; i < divider_max_count-1; ++i, ++div){
            div->next = (div + 1);
        }
        div->next = 0;
        models->layout.free_divider = dividers;
    }

    {
        View *vptr = 0;
        i32 i = 0;
        i32 max = 0;

        vars->live_set.count = 0;
        vars->live_set.max = panel_max_count;

        vars->live_set.views = push_array(partition, View, vars->live_set.max);

        dll_init_sentinel(&vars->live_set.free_sentinel);

        max = vars->live_set.max;
        vptr = vars->live_set.views;
        for (i = 0; i < max; ++i, ++vptr){
            dll_insert(&vars->live_set.free_sentinel, vptr);
        }
    }

    {
        Command_Map *global;
        i32 wanted_size = 0;
        b32 did_top = 0;
        b32 did_file = 0;

        models->scroll_rule = fallback_scroll_rule;

        setup_command_table();

        global = &models->map_top;
        Assert(models->config_api.get_bindings != 0);

        wanted_size = models->config_api.get_bindings(app_links.memory, app_links.memory_size);

        if (wanted_size <= app_links.memory_size){
            Command_Map *map_ptr = 0;
            Binding_Unit *unit, *end;
            i32 user_map_count;

            unit = (Binding_Unit*)app_links.memory;
            if (unit->type == unit_header && unit->header.error == 0){
                end = unit + unit->header.total_size;

                user_map_count = unit->header.user_map_count;

                models->map_id_table = push_array(
                    &models->mem.part, i32, user_map_count);

                models->user_maps = push_array(
                    &models->mem.part, Command_Map, user_map_count);

                models->user_map_count = user_map_count;

                for (++unit; unit < end; ++unit){
                    switch (unit->type){
                        case unit_map_begin:
                        {
                            int table_max = unit->map_begin.bind_count * 3 / 2;
                            int mapid = unit->map_begin.mapid;
                            if (mapid == mapid_global){
                                map_ptr = &models->map_top;
                                map_init(map_ptr, &models->mem.part, table_max, global);
                                did_top = 1;
                            }
                            else if (mapid == mapid_file){
                                map_ptr = &models->map_file;
                                map_init(map_ptr, &models->mem.part, table_max, global);
                                did_file = 1;
                            }
                            else if (mapid < mapid_global){
                                i32 index = app_get_or_add_map_index(models, mapid);
                                Assert(index < user_map_count);
                                map_ptr = models->user_maps + index;
                                map_init(map_ptr, &models->mem.part, table_max, global);
                            }
                            else map_ptr = 0;
                        }break;

                        case unit_inherit:
                        if (map_ptr){
                            Command_Map *parent = 0;
                            int mapid = unit->map_inherit.mapid;
                            if (mapid == mapid_global) parent = &models->map_top;
                            else if (mapid == mapid_file) parent = &models->map_file;
                            else if (mapid < mapid_global){
                                i32 index = app_get_or_add_map_index(models, mapid);
                                if (index < user_map_count) parent = models->user_maps + index;
                                else parent = 0;
                            }
                            map_ptr->parent = parent;
                        }break;

                        case unit_binding:
                        if (map_ptr){
                            Command_Function func = 0;
                            if (unit->binding.command_id >= 0 && unit->binding.command_id < cmdid_count)
                                func = command_table[unit->binding.command_id];
                            if (func){
                                if (unit->binding.code == 0 && unit->binding.modifiers == 0){
                                    map_ptr->vanilla_keyboard_default.function = func;
                                    map_ptr->vanilla_keyboard_default.custom_id = unit->binding.command_id;
                                }
                                else{
                                    map_add(map_ptr, unit->binding.code, unit->binding.modifiers, func, unit->binding.command_id);
                                }
                            }
                        }
                        break;

                        case unit_callback:
                        if (map_ptr){
                            Command_Function func = command_user_callback;
                            Custom_Command_Function *custom = unit->callback.func;
                            if (func){
                                if (unit->callback.code == 0 && unit->callback.modifiers == 0){
                                    map_ptr->vanilla_keyboard_default.function = func;
                                    map_ptr->vanilla_keyboard_default.custom = custom;
                                }
                                else{
                                    map_add(map_ptr, unit->callback.code, unit->callback.modifiers, func, custom);
                                }
                            }
                        }
                        break;

                        case unit_hook:
                        {
                            int hook_id = unit->hook.hook_id;
                            if (hook_id >= 0){
                                if (hook_id < hook_type_count){
                                    models->hooks[hook_id] = (Hook_Function*)unit->hook.func;
                                }
                                else{
                                    models->scroll_rule = (Scroll_Rule_Function*)unit->hook.func;
                                }
                            }
                        }break;
                    }
                }
            }
        }

        memset(app_links.memory, 0, wanted_size);
        if (!did_top) setup_top_commands(&models->map_top, &models->mem.part, global);
        if (!did_file) setup_file_commands(&models->map_file, &models->mem.part, global);

#if !defined(FRED_SUPER)
        models->hooks[hook_start] = 0;
#endif

        setup_ui_commands(&models->map_ui, &models->mem.part, global);

        models->font_set = &target->font_set;

        font_set_init(models->font_set, partition, 16, 5);
    }

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

            font_set_add(partition, models->font_set, file_name, name, pt_size);
        }
    }
    
    // NOTE(allen): file setup
    working_set_init(&models->working_set, partition);
    
    // NOTE(allen): clipboard setup
    models->working_set.clipboard_max_size = ArrayCount(models->working_set.clipboards);
    models->working_set.clipboard_size = 0;
    models->working_set.clipboard_current = 0;
    models->working_set.clipboard_rolling = 0;

    // TODO(allen): more robust allocation solution for the clipboard
    if (clipboard.str){
        String *dest = working_set_next_clipboard_string(&models->mem.general, &models->working_set, clipboard.size);
        copy(dest, make_string((char*)clipboard.str, clipboard.size));
    }

    // NOTE(allen): delay setup
    models->delay1.general = &models->mem.general;
    models->delay1.max = 16;
    models->delay1.acts = (Delayed_Action*)general_memory_allocate(
        &models->mem.general, models->delay1.max*sizeof(Delayed_Action), 0);

    models->delay2.general = &models->mem.general;
    models->delay2.max = 16;
    models->delay2.acts = (Delayed_Action*)general_memory_allocate(
        &models->mem.general, models->delay2.max*sizeof(Delayed_Action), 0);

    // NOTE(allen): style setup
    app_hardcode_styles(models);

    models->palette_size = 40;
    models->palette = push_array(partition, u32, models->palette_size);

    // NOTE(allen): init first panel
    Panel_And_ID p = layout_alloc_panel(&models->layout);
    panel_make_empty(system, exchange, vars, p.panel);
    models->layout.active_panel = p.id;

    String hdbase = make_fixed_width_string(models->hot_dir_base_);
    hot_directory_init(&models->hot_directory, hdbase, current_directory, system->slash);

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

    // NOTE(allen): parameter setup
    models->buffer_param_max = 32;
    models->buffer_param_count = 0;
    models->buffer_param_indices = push_array(partition, i32, models->buffer_param_max);
}

App_Step_Sig(app_step){
    ProfileStart(OS_syncing);
    Application_Step_Result app_result = *result;
    app_result.redraw = force_redraw;

    App_Vars *vars = (App_Vars*)memory->vars_memory;
    Models *models = &vars->models;
    target->partition = &models->mem.part;

    if (first_step || !time_step){
        app_result.redraw = 1;
    }

    // NOTE(allen): OS clipboard event handling
    if (clipboard.str){
        String *dest = working_set_next_clipboard_string(&models->mem.general, &models->working_set, clipboard.size);
        dest->size = eol_convert_in(dest->str, clipboard.str, clipboard.size);
    }

    // TODO(allen): profile this make sure it's not costing me too much power.
    // NOTE(allen): check files are up to date
    {
        File_Node *node, *used_nodes;
        Editing_File *file;
        u64 time_stamp;

        used_nodes = &models->working_set.used_sentinel;
        for (dll_items(node, used_nodes)){
            file = (Editing_File*)node;

            time_stamp = system->file_time_stamp(make_c_str(file->name.source_path));

            // TODO(allen): This is a bit wasteful! Let's dial it in a bit.
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
        Temp_Memory temp = begin_temp_memory(&models->mem.part);
        u32 max = Kbytes(32);
        char *dest = push_array(&models->mem.part, char, max);
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

                    i32 end = buffer_size(&out_file->state.buffer);
                    file_replace_range(system, models, out_file,
                        end, end, dest, amount, end + amount, 1);
                    app_result.redraw = 1;
                    new_cursor = end + amount;
                }

                if (system->cli_end_update(&proc->cli)){
                    *proc = vars->cli_processes.procs[--count];
                    --i;

                    char str_space[256];
                    String str = make_fixed_width_string(str_space);
                    append(&str, "exited with code ");
                    append_int_to_str(proc->cli.exit, &str);

                    i32 end = buffer_size(&out_file->state.buffer);
                    file_replace_range(system, models, out_file,
                        end, end, str.str, str.size, end + str.size, 1);
                    app_result.redraw = 1;
                    new_cursor = end + str.size;
                }

                new_cursor = 0;

                for (View_Iter iter = file_view_iter_init(&models->layout, out_file, 0);
                    file_view_iter_good(iter);
                    iter = file_view_iter_next(iter)){
                    view_cursor_move(iter.view, new_cursor);
                }
            }
        }

        vars->cli_processes.count = count;
        end_temp_memory(temp);
    }

    // NOTE(allen): reorganizing panels on screen
    {
        i32 prev_width = models->layout.full_width;
        i32 prev_height = models->layout.full_height;
        i32 current_width = target->width;
        i32 current_height = target->height;

        Panel *panel, *used_panels;
        View *view;

        models->layout.full_width = current_width;
        models->layout.full_height = current_height;

        if (prev_width != current_width || prev_height != current_height){
            layout_refit(&models->layout, prev_width, prev_height);

            used_panels = &models->layout.used_sentinel;
            for (dll_items(panel, used_panels)){
                view = panel->view;
                Assert(view);
                // TODO(allen): All responses to a panel changing size should
                // be handled in the same place.
                view_change_size(system, &models->mem.general, view);
            }

            app_result.redraw = 1;
        }
    }

    // NOTE(allen): prepare input information
    Key_Summary key_data = {};
    for (i32 i = 0; i < input->press_count; ++i){
        key_data.keys[key_data.count++] = input->press[i];
    }
    for (i32 i = 0; i < input->hold_count; ++i){
        key_data.keys[key_data.count++] = input->hold[i];
    }

    mouse->wheel = -mouse->wheel;

    ProfileEnd(OS_syncing);

    ProfileStart(hover_status);
    // NOTE(allen): detect mouse hover status
    i32 mx = mouse->x;
    i32 my = mouse->y;
    b32 mouse_in_edit_area = 0;
    b32 mouse_in_margin_area = 0;
    Panel *mouse_panel, *used_panels;

    used_panels = &models->layout.used_sentinel;
    for (dll_items(mouse_panel, used_panels)){
        if (hit_check(mx, my, mouse_panel->inner)){
            mouse_in_edit_area = 1;
            break;
        }
        else if (hit_check(mx, my, mouse_panel->full)){
            mouse_in_margin_area = 1;
            break;
        }
    }

    if (!(mouse_in_edit_area || mouse_in_margin_area)){
        mouse_panel = 0;
    }

    b32 mouse_on_divider = 0;
    b32 mouse_divider_vertical = 0;
    i32 mouse_divider_id = 0;
    i32 mouse_divider_side = 0;

    if (mouse_in_margin_area){
        Panel *panel = mouse_panel;
        if (mx >= panel->inner.x0 && mx < panel->inner.x1){
            mouse_divider_vertical = 0;
            if (my > panel->inner.y0){
                mouse_divider_side = -1;
            }
            else{
                mouse_divider_side = 1;
            }
        }
        else{
            mouse_divider_vertical = 1;
            if (mx > panel->inner.x0){
                mouse_divider_side = -1;
            }
            else{
                mouse_divider_side = 1;
            }
        }

        if (models->layout.panel_count > 1){
            i32 which_child;
            mouse_divider_id = panel->parent;
            which_child = panel->which_child;
            for (;;){
                Divider_And_ID div = layout_get_divider(&models->layout, mouse_divider_id);

                if (which_child == mouse_divider_side &&
                        div.divider->v_divider == mouse_divider_vertical){
                    mouse_on_divider = 1;
                    break;
                }

                if (mouse_divider_id == models->layout.root){
                    break;
                }
                else{
                    mouse_divider_id = div.divider->parent;
                    which_child = div.divider->which_child;
                }
            }
        }
        else{
            mouse_on_divider = 0;
            mouse_divider_id = 0;
        }
    }
    ProfileEnd(hover_status);

    // NOTE(allen): prepare to start executing commands
    ProfileStart(prepare_commands);

    Command_Data *cmd = &vars->command_data;

    cmd->models = models;
    cmd->vars = vars;
    cmd->system = system;
    cmd->exchange = exchange;
    cmd->live_set = &vars->live_set;

    cmd->panel = models->layout.panels + models->layout.active_panel;
    cmd->view = cmd->panel->view;

    cmd->screen_width = target->width;
    cmd->screen_height = target->height;

    cmd->key = {};

    Temp_Memory param_stack_temp = begin_temp_memory(&models->mem.part);
    cmd->part = partition_sub_part(&models->mem.part, 16 << 10);

    if (first_step){
        if (models->hooks[hook_start]){
            models->hooks[hook_start](&app_links);
            cmd->part.pos = 0;
        }

        i32 i;
        String file_name;
        Panel *panel = models->layout.used_sentinel.next;
        for (i = 0; i < models->settings.init_files_count; ++i, panel = panel->next){
            file_name = make_string_slowly(models->settings.init_files[i]);

            if (i < models->layout.panel_count){
                delayed_open(&models->delay1, file_name, panel);
                if (i == 0){
                    delayed_set_line(&models->delay1, panel, models->settings.initial_line);
                }
            }
            else{
                delayed_open_background(&models->delay1, file_name);
            }
        }
    }
    ProfileEnd(prepare_commands);
    
    // NOTE(allen): process the command_coroutine if it is unfinished
    ProfileStart(command_coroutine);
    b8 consumed_input[6] = {0};

    if (models->command_coroutine != 0){
        Coroutine *command_coroutine = models->command_coroutine;
        u32 get_flags = models->command_coroutine_flags[0];
        u32 abort_flags = models->command_coroutine_flags[1];

        get_flags |= abort_flags;

        if ((get_flags & EventOnAnyKey) || (get_flags & EventOnEsc)){
            for (i32 key_i = 0; key_i < key_data.count; ++key_i){
                Key_Event_Data key = get_single_key(&key_data, key_i);
                View *view = cmd->view;
                b32 pass_in = 0;
                cmd->key = key;

                Command_Map *map = 0;
                if (view) map = view->map;
                if (map == 0) map = &models->map_top;
                Command_Binding cmd_bind = map_extract_recursive(map, key);

                User_Input user_in;
                user_in.type = UserInputKey;
                user_in.key = key;
                user_in.command = (unsigned long long)cmd_bind.custom;
                user_in.abort = 0;

                if ((EventOnEsc & abort_flags) && key.keycode == key_esc){
                    user_in.abort = 1;
                }
                else if (EventOnAnyKey & abort_flags){
                    user_in.abort = 1;
                }

                if (EventOnAnyKey & get_flags){
                    pass_in = 1;
                    consumed_input[0] = 1;
                }
                if (key.keycode == key_esc){
                    if (EventOnEsc & get_flags){
                        pass_in = 1;
                    }
                    consumed_input[1] = 1;
                }

                if (pass_in){
                    models->command_coroutine = system->resume_coroutine(command_coroutine, &user_in, models->command_coroutine_flags);
                    app_result.redraw = 1;

                    // TOOD(allen): Deduplicate
                    // TODO(allen): Allow a view to clean up however it wants after a command 
                    // finishes, or after transfering to another view mid command.
                    if (view != 0 && models->command_coroutine == 0){
                        init_query_set(&view->query_set);
                    }
                    if (models->command_coroutine == 0) break;
                }
            }
        }

        if (models->command_coroutine != 0 && (get_flags & EventOnMouse)){
            View *view = cmd->view;
            b32 pass_in = 0;

            User_Input user_in;
            user_in.type = UserInputMouse;
            user_in.mouse = *mouse;
            user_in.command = 0;
            user_in.abort = 0;

            if (abort_flags & EventOnMouseMove){
                user_in.abort = 1;
            }
            if (get_flags & EventOnMouseMove){
                pass_in = 1;
                consumed_input[2] = 1;
            }

            if (mouse->press_l || mouse->release_l || mouse->l){
                if (abort_flags & EventOnLeftButton){
                    user_in.abort = 1;
                }
                if (get_flags & EventOnLeftButton){
                    pass_in = 1;
                    consumed_input[3] = 1;
                }
            }

            if (mouse->press_r || mouse->release_r || mouse->r){
                if (abort_flags & EventOnRightButton){
                    user_in.abort = 1;
                }
                if (get_flags & EventOnRightButton){
                    pass_in = 1;
                    consumed_input[4] = 1;
                }
            }

            if (mouse->wheel != 0){
                if (abort_flags & EventOnWheel){
                    user_in.abort = 1;
                }
                if (get_flags & EventOnWheel){
                    pass_in = 1;
                    consumed_input[5] = 1;
                }
            }

            if (pass_in){
                models->command_coroutine = system->resume_coroutine(command_coroutine, &user_in,
                    models->command_coroutine_flags);
                app_result.redraw = 1;

                // TOOD(allen): Deduplicate
                // TODO(allen): Allow a view to clean up however it wants after a command finishes,
                // or after transfering to another view mid command.
                if (view != 0 && models->command_coroutine == 0){
                    init_query_set(&view->query_set);
                }
            }
        }
    }

    update_command_data(vars, cmd);

    ProfileEnd(command_coroutine);
    
    ProfileStart(frame_hook);
    if (models->hooks[hook_frame]){
        if ((models->hooks[hook_frame])(&app_links)){
            app_result.redraw = 1;
        }
    }
    ProfileStart(frame_hook);
    
    // NOTE(allen): pass raw input to the panels
    ProfileStart(step);

    Input_Summary dead_input = {};
    dead_input.mouse.x = mouse->x;
    dead_input.mouse.y = mouse->y;

    Input_Summary active_input = {};
    active_input.mouse.x = mouse->x;
    active_input.mouse.y = mouse->y;
    if (!consumed_input[0]){
        active_input.keys = key_data;
    }
    else if (!consumed_input[1]){
        for (i32 i = 0; i < key_data.count; ++i){
            Key_Event_Data key = get_single_key(&key_data, i);
            if (key.keycode == key_esc){
                active_input.keys.count = 1;
                active_input.keys.keys[0] = key;
                break;
            }
        }
    }

    Mouse_State mouse_state = *mouse;

    if (consumed_input[3]){
        mouse_state.l = 0;
        mouse_state.press_l = 0;
        mouse_state.release_l = 0;
    }

    if (consumed_input[4]){
        mouse_state.r = 0;
        mouse_state.press_r = 0;
        mouse_state.release_r = 0;
    }

    if (consumed_input[5]){
        mouse_state.wheel = 0;
    }

    {
        Panel *panel, *used_panels;
        View *view;
        b32 active;

        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            view = panel->view;
            active = (panel == cmd->panel);
            Input_Summary input = (active)?(active_input):(dead_input);
            if (panel == mouse_panel && !mouse->out_of_window){
                input.mouse = mouse_state;
            }
            if (step_file_view(system, exchange, view, panel->inner, active, &input)){
                app_result.redraw = 1;
            }
        }
    }

    update_command_data(vars, cmd);
    ProfileEnd(step);

    // NOTE(allen): command execution
    ProfileStart(command);
    if (!consumed_input[0] || !consumed_input[1]){
        b32 consumed_input2[2] = {0};

        for (i32 key_i = 0; key_i < key_data.count; ++key_i){
            if (models->command_coroutine != 0) break;

            switch (vars->state){
                case APP_STATE_EDIT:
                {
                    Key_Event_Data key = get_single_key(&key_data, key_i);
                    b32 hit_esc = (key.keycode == key_esc);
                    cmd->key = key;

                    if (hit_esc || !consumed_input[0]){
                        View *view = cmd->view;

                        Command_Map *map = 0;
                        if (view) map = view->map;
                        if (map == 0) map = &models->map_top;
                        Command_Binding cmd_bind = map_extract_recursive(map, key);

                        if (cmd_bind.function){
                            if (hit_esc){
                                consumed_input2[1] = 1;
                            }
                            else{
                                consumed_input2[0] = 1;
                            }

                            Assert(models->command_coroutine == 0);
                            Coroutine *command_coroutine = system->create_coroutine(command_caller);
                            models->command_coroutine = command_coroutine;

                            Command_In cmd_in;
                            cmd_in.cmd = cmd;
                            cmd_in.bind = cmd_bind;

                            models->command_coroutine = system->launch_coroutine(models->command_coroutine,
                                &cmd_in, models->command_coroutine_flags);
                            models->prev_command = cmd_bind;
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

        consumed_input[0] |= consumed_input2[0];
        consumed_input[1] |= consumed_input2[1];
    }

    update_command_data(vars, cmd);
    ProfileEnd(command);

    ProfileStart(resizing);
    // NOTE(allen): panel resizing
    switch (vars->state){
        case APP_STATE_EDIT:
        {
            if (mouse->press_l && mouse_on_divider){
                vars->state = APP_STATE_RESIZING;
                Divider_And_ID div = layout_get_divider(&models->layout, mouse_divider_id);
                vars->resizing.divider = div.divider;

                i32 min, max;
                {
                    i32 mid, MIN, MAX;
                    mid = div.divider->pos;
                    if (mouse_divider_vertical){
                        MIN = 0;
                        MAX = MIN + models->layout.full_width;
                    }
                    else{
                        MIN = 0;
                        MAX = MIN + models->layout.full_height;
                    }
                    min = MIN;
                    max = MAX;

                    i32 divider_id = div.id;
                    do{
                        Divider_And_ID other_div = layout_get_divider(&models->layout, divider_id);
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

                    Temp_Memory temp = begin_temp_memory(&models->mem.part);
                    i32 *divider_stack = push_array(&models->mem.part, i32, models->layout.panel_count);
                    i32 top = 0;
                    divider_stack[top++] = div.id;

                    while (top > 0){
                        Divider_And_ID other_div = layout_get_divider(&models->layout, divider_stack[--top]);
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
            if (mouse->l){
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

                layout_fix_all_panels(&models->layout);
            }
            else{
                vars->state = APP_STATE_EDIT;
            }
        }break;
    }

    if (mouse_in_edit_area && mouse_panel != 0 && mouse->press_l){
        models->layout.active_panel = (i32)(mouse_panel - models->layout.panels);
        app_result.redraw = 1;
    }

    update_command_data(vars, cmd);
    ProfileEnd(resizing);

    // NOTE(allen): processing sys app bindings
    ProfileStart(sys_app_bind_processing);
    {
        Mem_Options *mem = &models->mem;
        General_Memory *general = &mem->general;

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

            Working_Set *working_set = &models->working_set;

            if (exchange_file_ready(exchange, binding->sys_id, &data, &size, &max)){
                ed_file = working_set_get_active_file(working_set, binding->app_id);
                Assert(ed_file);

                filename = exchange_file_filename(exchange, binding->sys_id);
                preload_settings = ed_file->preload;
                if (data){
                    String val = make_string((char*)data, size);
                    // TODO(allen): reduce to just passing models
                    file_create_from_string(system, models, ed_file, filename, val);

                    if (ed_file->settings.tokens_exist){
                        file_first_lex_parallel(system, general, ed_file);
                    }

                    if ((binding->success & SysAppCreateView) && binding->panel != 0){
                        view_file_in_panel(cmd, binding->panel, ed_file);
                    }
                    
                    for (View_Iter iter = file_view_iter_init(&models->layout, ed_file, 0);
                        file_view_iter_good(iter);
                        iter = file_view_iter_next(iter)){
                        view_measure_wraps(system, general, iter.view);
                        view_cursor_move(iter.view, preload_settings.start_line, 0);
                    }

                    app_result.redraw = 1;
                }
                else{
                    if (binding->fail & SysAppCreateNewBuffer){
                        file_create_empty(system, models, ed_file, filename);
                        if (binding->fail & SysAppCreateView){
                            view_file_in_panel(cmd, binding->panel, ed_file);
                        }
                    }
                    else{
                        working_set_remove(system, &models->working_set, ed_file->name.source_path);
                        working_set_free_file(&models->working_set, ed_file);
                    }

                    app_result.redraw = 1;
                }

                exchange_free_file(exchange, binding->sys_id);
                remove = 1;
            }

            if (exchange_file_save_complete(exchange, binding->sys_id, &data, &size, &max, &failed)){
                Assert(remove == 0);

                if (data){
                    general_memory_free(general, data);
                    exchange_clear_file(exchange, binding->sys_id);
                }

                Editing_File *file = working_set_get_active_file(working_set, binding->app_id);
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
    }
    ProfileEnd(sys_app_bind_processing);

    // NOTE(allen): process as many delayed actions as possible
    ProfileStart(delayed_actions);
    if (models->delay1.count > 0){
        Working_Set *working_set = &models->working_set;
        Mem_Options *mem = &models->mem;
        General_Memory *general = &mem->general;

        i32 count = models->delay1.count;
        models->delay1.count = 0;
        models->delay2.count = 0;

        Delayed_Action *act = models->delay1.acts;
        for (i32 i = 0; i < count; ++i, ++act){
            String string = act->string;
            Panel *panel = act->panel;
            Editing_File *file = act->file;
            i32 integer = act->integer;

            // TODO(allen): Paramter checking in each DACT case.
            switch (act->type){
                case DACT_TOUCH_FILE:
                {
                    if (file){
                        Assert(!file->state.is_dummy);
                        dll_remove(&file->node);
                        dll_insert(&models->working_set.used_sentinel, &file->node);
                    }
                }break;

                case DACT_OPEN:
                case DACT_OPEN_BACKGROUND:
                {
                    App_Open_File_Result result = {};
                    {
                        String filename = string;
                        i32 file_id;
                        
                        filename.str[0] = char_to_lower(filename.str[0]);
    
                        result.file = working_set_contains(system, working_set, filename);
                        if (result.file == 0){
                            result.is_new = 1;
                            result.file = working_set_alloc_always(working_set, general);
                            if (result.file){
                                file_id = exchange_request_file(exchange, filename.str, filename.size);
                                if (file_id){
                                    file_init_strings(result.file);
                                    file_set_name(working_set, result.file, filename.str);
                                    file_set_to_loading(result.file);
                                    working_set_add(system, working_set, result.file);

                                    result.sys_id = file_id;
                                    result.file_index = result.file->id.id;
                                }
                                else{
                                    working_set_free_file(working_set, result.file);
                                    delayed_action_repush(&models->delay2, act);
                                    break;
                                }
                            }
                        }
                    }

                    if (result.is_new){
                        if (result.file){
                            Assert(result.sys_id);
                            Sys_App_Binding *binding = app_push_file_binding(vars, result.sys_id, result.file_index);
                            binding->success = (act->type == DACT_OPEN) ? SysAppCreateView : 0;
                            binding->fail = 0;
                            binding->panel = panel;
                        }
                    }
                    else{
                        if (act->type == DACT_OPEN){
                            Assert(result.file);
                            if (!result.file->state.is_loading){
                                view_file_in_panel(cmd, panel, result.file);
                            }
                        }
                    }
                }break;

                case DACT_SET_LINE:
                {
                    // TODO(allen): deduplicate
                    Editing_File *file = 0;
                    if (panel){
                        file = panel->view->file;
                    }
                    else if (string.str && string.size > 0){
                        file = working_set_lookup_file(working_set, string);
                    }
                    if (file){
                        if (file->state.is_loading){
                            file->preload.start_line = integer;
                        }
                        else{
                            // TODO(allen): write this case
                        }
                    }
                }break;

                case DACT_SAVE_AS:
                {
                    // TODO(allen): deduplicate
                    Editing_File *file = 0;
                    if (panel){
                        file = panel->view->file;
                    }
                    else if (string.str && string.size > 0){
                        file = working_set_lookup_file(working_set, string);
                    }
                    if (file){
                        i32 sys_id = file_save_and_set_names(system, exchange, mem, working_set, file, string.str);
                        if (sys_id){
                            app_push_file_binding(vars, sys_id, file->id.id);
                        }
                        else{
                            delayed_action_repush(&models->delay2, act);
                        }
                    }
                }break;

                case DACT_SAVE:
                {
                    if (!file){
                        if (panel){
                            View *view = panel->view;
                            Assert(view);
                            file = view->file;
                        }
                        else{
                            file = working_set_lookup_file(working_set, string);
                        }
                    }
                    // TODO(allen): We could handle the case where someone tries to save the same thing
                    // twice... that would be nice to have under control.
                    if (file){
                        i32 sys_id = file_save(system, exchange, mem, file, file->name.source_path.str);
                        if (sys_id){
                            // TODO(allen): This is fishy! Shouldn't we bind it to a file name instead? This file
                            // might be killed before we get notified that the saving is done!
                            app_push_file_binding(vars, sys_id, file->id.id);
                        }
                        else{
                            delayed_action_repush(&models->delay2, act);
                        }
                    }
                }break;

                case DACT_NEW:
                {
                    Editing_File *file = working_set_alloc_always(working_set, general);
                    file_create_empty(system, models, file, string.str);
                    working_set_add(system, working_set, file);

                    View *view = panel->view;

                    view_set_file(view, file, models, system, models->hooks[hook_open_file], &app_links);
                    view->map = app_get_map(models, file->settings.base_map_id);
#if BUFFER_EXPERIMENT_SCALPEL <= 0
                    if (file->settings.tokens_exist)
                        file_first_lex_parallel(system, general, file);
#endif
                }break;

                case DACT_SWITCH:
                {
                    if (!file && string.str){
                        file = working_set_lookup_file(working_set, string);
                        
                        if (!file){
                            file = working_set_contains(system, working_set, string);
                        }
                    }
                    
                    if (file){
                        View *view = panel->view;

                        view_set_file(view, file, models, system,
                            models->hooks[hook_open_file], &app_links);
                        view->map = app_get_map(models, file->settings.base_map_id);
                    }
                }break;

                case DACT_KILL:
                {
                    if (!file && string.str){
                        file = working_set_lookup_file(working_set, string);
                        
                        if (!file){
                            file = working_set_contains(system, working_set, string);
                        }
                    }
                    
                    if (file){
                        working_set_remove(system, working_set, file->name.source_path);
                        kill_file(system, exchange, models, file,
                            models->hooks[hook_open_file], &app_links);
                    }
                }break;

                case DACT_TRY_KILL:
                {
                    View *view = 0;
                    if (panel){
                        view = panel->view;
                    }
                    else{
                        view = (models->layout.panels + models->layout.active_panel)->view;
                    }
                    
                    if (!file && string.str){
                        file = working_set_lookup_file(working_set, string);
                        
                        if (!file){
                            file = working_set_contains(system, working_set, string);
                        }
                    }

                    if (file){
                        if (buffer_needs_save(file)){
                            view_show_interactive(system, view, &models->map_ui,
                                IAct_Sure_To_Kill, IInt_Sure_To_Kill, make_lit_string("Are you sure?"));
                            copy(&view->dest, file->name.live_name);
                        }
                        else{
                            working_set_remove(system, working_set, file->name.source_path);
                            kill_file(system, exchange, models, file,
                                models->hooks[hook_open_file], &app_links);
                        }
                    }
                }break;
            }

            if (string.str){
                general_memory_free(general, string.str);
            }
        }
        Swap(models->delay1, models->delay2);
    }

    end_temp_memory(param_stack_temp);
    ProfileEnd(delayed_actions);

    ProfileStart(resize);
    // NOTE(allen): send resize messages to panels that have changed size
    {
        Panel *panel, *used_panels;
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            i32_Rect prev = panel->prev_inner;
            i32_Rect inner = panel->inner;
            if (prev.x0 != inner.x0 || prev.y0 != inner.y0 ||
                    prev.x1 != inner.x1 || prev.y1 != inner.y1){
                remeasure_file_view(system, panel->view, panel->inner);
            }
            panel->prev_inner = inner;
        }
    }
    ProfileEnd(resize);

    ProfileStart(style_change);
    // NOTE(allen): send style change messages if the style has changed
    if (models->global_font.font_changed){
        models->global_font.font_changed = 0;

        File_Node *node, *used_nodes;
        Editing_File *file;
        Render_Font *font = get_font_info(models->font_set, models->global_font.font_id)->font;
        float *advance_data = 0;
        if (font) advance_data = font->advance_data;

        used_nodes = &models->working_set.used_sentinel;
        for (dll_items(node, used_nodes)){
            file = (Editing_File*)node;
            file_measure_starts_widths(system, &models->mem.general, &file->state.buffer, advance_data);
        }

        Panel *panel, *used_panels;
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            remeasure_file_view(system, panel->view, panel->inner);
        }
    }
    ProfileEnd(style_change);

    ProfileStart(redraw);
    if (mouse_panel != models->prev_mouse_panel) app_result.redraw = 1;
    if (app_result.redraw){
        begin_render_section(target, system);

        target->clip_top = -1;
        draw_push_clip(target, rect_from_target(target));

        // NOTE(allen): render the panels
        Panel *panel, *used_panels;
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            i32_Rect full = panel->full;
            i32_Rect inner = panel->inner;

            View *view = panel->view;
            Style *style = &models->style;

            b32 active = (panel == cmd->panel);
            u32 back_color = style->main.back_color;
            draw_rectangle(target, full, back_color);

            draw_push_clip(target, panel->inner);
            draw_file_view(system, exchange, view, cmd->view, panel->inner, active, target, &dead_input);
            draw_pop_clip(target);

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
            app_result.mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
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
    models->prev_mouse_panel = mouse_panel;
    ProfileEnd(get_cursor);

    app_result.lctrl_lalt_is_altgr = models->settings.lctrl_lalt_is_altgr;
    *result = app_result;

    // end-of-app_step
}

external App_Get_Functions_Sig(app_get_functions){
    App_Functions result = {};

    result.read_command_line = app_read_command_line;
    result.init = app_init;
    result.step = app_step;

    return(result);
}

// BOTTOM

