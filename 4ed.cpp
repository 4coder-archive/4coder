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

    Live_Views live_set;

    App_State state;
    App_State_Resizing resizing;
    Complete_State complete_state;

    Command_Data command_data;
};

enum Coroutine_Type{
    Co_View,
    Co_Command
};
struct App_Coroutine_State{
    void *co;
    int type;
};
inline App_Coroutine_State
get_state(Application_Links *app){
    App_Coroutine_State state = {0};
    state.co = app->current_coroutine;
    state.type = app->type_coroutine;
    return(state);
}
inline void
restore_state(Application_Links *app, App_Coroutine_State state){
    app->current_coroutine = state.co;
    app->type_coroutine = state.type;
}

inline Coroutine*
app_launch_coroutine(System_Functions *system, Application_Links *app, Coroutine_Type type,
                     Coroutine *co, void *in, void *out){
    Coroutine* result = 0;
    
    App_Coroutine_State prev_state = get_state(app);
    
    app->current_coroutine = co;
    app->type_coroutine = type;
    {
        result = system->launch_coroutine(co, in, out);
    }
    
    restore_state(app, prev_state);
    
    return(result);
}

inline Coroutine*
app_resume_coroutine(System_Functions *system, Application_Links *app, Coroutine_Type type,
                     Coroutine *co, void *in, void *out){
    Coroutine* result = 0;
    
    App_Coroutine_State prev_state = get_state(app);
    
    app->current_coroutine = co;
    app->type_coroutine = type;
    {
        result = system->resume_coroutine(co, in, out);
    }
    
    restore_state(app, prev_state);
    
    return(result);
}

inline void
output_file_append(System_Functions *system, Models *models, Editing_File *file, String value, b32 cursor_at_end){
    i32 end = buffer_size(&file->state.buffer);
    i32 next_cursor = 0;
    if (cursor_at_end){
        next_cursor = end + value.size;
    }
    file_replace_range(system, models, file, end, end, value.str, value.size, next_cursor, 1);
}

inline void
do_feedback_message(System_Functions *system, Models *models, String value){
    Editing_File *file = models->message_buffer;

    if (file){
        output_file_append(system, models, file, value, 1);
        i32 pos = buffer_size(&file->state.buffer);
        for (View_Iter iter = file_view_iter_init(&models->layout, file, 0);
             file_view_iter_good(iter);
             iter = file_view_iter_next(iter)){
            view_cursor_move(iter.view, pos);
        }
    }
}

// Commands

#define USE_MODELS(n) Models *n = command->models
#define USE_VARS(n) App_Vars *n = command->vars
#define USE_PANEL(n) Panel *n = command->panel
#define USE_VIEW(n) View *n = command->view
#define USE_FILE(n,v) Editing_File *n = (v)->file_data.file

#define REQ_OPEN_VIEW(n) View *n = command->panel->view; if (view_lock_level(n) > LockLevel_Open) return
#define REQ_READABLE_VIEW(n) View *n = command->panel->view; if (view_lock_level(n) > LockLevel_NoWrite) return

#define REQ_FILE(n,v) Editing_File *n = (v)->file_data.file; if (!n) return
#define REQ_FILE_HISTORY(n,v) Editing_File *n = (v)->file_data.file; if (!n || !n->state.undo.undo.edits) return

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
panel_make_empty(System_Functions *system, App_Vars *vars, Panel *panel){
    Models *models = &vars->models;
    View_And_ID new_view;

    Assert(panel->view == 0);
    new_view = live_set_alloc_view(&vars->live_set, panel, models);
    view_set_file(new_view.view, 0, models);
    new_view.view->map = get_map(models, mapid_global);

    return(new_view.view);
}

COMMAND_DECL(null){
    AllowLocal(command);
}

COMMAND_DECL(write_character){

    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    char character;
    i32 pos, next_cursor_pos;

    character = command->key.character;
    if (character != 0){
        pos = view->recent->cursor.pos;
        next_cursor_pos = view->recent->cursor.pos + 1;
        view_replace_range(system, models, view, pos, pos, &character, 1, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
    }
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
        pos[0] = buffer_seek_whitespace_left(&file->state.buffer, view->recent->cursor.pos);
    }

    if (flags & (1 << 1)){
        if (file->state.tokens_complete){
            pos[1] = seek_token_left(&file->state.token_stack, view->recent->cursor.pos);
        }
        else{
            pos[1] = buffer_seek_whitespace_left(&file->state.buffer, view->recent->cursor.pos);
        }
    }

    if (flags & (1 << 2)){
        pos[2] = buffer_seek_alphanumeric_left(&file->state.buffer, view->recent->cursor.pos);
        if (flags & (1 << 3)){
            pos[3] = buffer_seek_range_camel_left(&file->state.buffer, view->recent->cursor.pos, pos[2]);
        }
    }
    else{
        if (flags & (1 << 3)){
            pos[3] = buffer_seek_alphanumeric_or_camel_left(&file->state.buffer, view->recent->cursor.pos);
        }
    }

    i32 new_pos = 0;
    for (i32 i = 0; i < ArrayCount(pos); ++i){
        if (pos[i] > new_pos) new_pos = pos[i];
    }

    view_cursor_move(view, new_pos);
}

COMMAND_DECL(seek_right){

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
        pos[0] = buffer_seek_whitespace_right(&file->state.buffer, view->recent->cursor.pos);
    }

    if (flags & (1 << 1)){
        if (file->state.tokens_complete){
            pos[1] = seek_token_right(&file->state.token_stack, view->recent->cursor.pos);
        }
        else{
            pos[1] = buffer_seek_whitespace_right(&file->state.buffer, view->recent->cursor.pos);
        }
    }

    if (flags & (1 << 2)){
        pos[2] = buffer_seek_alphanumeric_right(&file->state.buffer, view->recent->cursor.pos);
        if (flags & (1 << 3)){
            pos[3] = buffer_seek_range_camel_right(&file->state.buffer, view->recent->cursor.pos, pos[2]);
        }
    }
    else{
        if (flags & (1 << 3)){
            pos[3] = buffer_seek_alphanumeric_or_camel_right(&file->state.buffer, view->recent->cursor.pos);
        }
    }

    i32 new_pos = size;
    for (i32 i = 0; i < ArrayCount(pos); ++i){
        if (pos[i] < new_pos) new_pos = pos[i];
    }

    view_cursor_move(view, new_pos);
}

COMMAND_DECL(seek_whitespace_up){

    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = buffer_seek_whitespace_up(&file->state.buffer, view->recent->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_whitespace_down){

    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = buffer_seek_whitespace_down(&file->state.buffer, view->recent->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(center_view){

    USE_VIEW(view);
    REQ_FILE(file, view);

    f32 y, h;
    if (view->file_data.unwrapped_lines){
        y = view->recent->cursor.unwrapped_y;
    }
    else{
        y = view->recent->cursor.wrapped_y;
    }
    
    h = view_file_height(view);
    y = clamp_bottom(0.f, y - h*.5f);

    view->recent->scroll.target_y = y;
}

COMMAND_DECL(word_complete){
    
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
        word_end = view->recent->cursor.pos;
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
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    view->recent->mark = (i32)view->recent->cursor.pos;
    view->recent->preferred_x = view_get_cursor_x(view);
}

COMMAND_DECL(copy){
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

    Range range = make_range(view->recent->cursor.pos, view->recent->mark);
    if (start_set) range.start = r_start;
    if (end_set) range.end = r_end;
    if (range.start < range.end){
        clipboard_copy(system, &models->mem.general, &models->working_set, range, file);
    }
}

COMMAND_DECL(cut){
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

    Range range = make_range(view->recent->cursor.pos, view->recent->mark);
    if (start_set) range.start = r_start;
    if (end_set) range.end = r_end;
    if (range.start < range.end){
        i32 next_cursor_pos = range.start;

        clipboard_copy(system, &models->mem.general, &models->working_set, range, file);
        view_replace_range(system, models, view, range.start, range.end, 0, 0, next_cursor_pos);

        view->recent->mark = range.start;
        view_cursor_move(view, next_cursor_pos);
    }
}

COMMAND_DECL(paste){
    
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    View_Iter iter;
    String *src;
    i32 pos_left, next_cursor_pos;

    if (models->working_set.clipboard_size > 0){
        view->next_mode.rewrite = 1;

        src = working_set_clipboard_head(&models->working_set);
        pos_left = view->recent->cursor.pos;

        next_cursor_pos = pos_left+src->size;
        view_replace_range(system, models, view, pos_left, pos_left, src->str, src->size, next_cursor_pos);

        view_cursor_move(view, next_cursor_pos);
        view->recent->mark = pos_left;

        Style *style = main_style(models);
        u32 paste_color = style->main.paste_color;
        for (iter = file_view_iter_init(&models->layout, file, 0);
            file_view_iter_good(iter);
            iter = file_view_iter_next(iter)){
            view_post_paste_effect(iter.view, 20, pos_left, src->size, paste_color);
        }
    }
}

COMMAND_DECL(paste_next){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    View_Iter iter;
    Range range;
    String *src;
    i32 next_cursor_pos;

    if (models->working_set.clipboard_size > 0 && view->mode.rewrite == 1){
        view->next_mode.rewrite = 1;

        range = make_range(view->recent->mark, view->recent->cursor.pos);
        src = working_set_clipboard_roll_down(&models->working_set);
        next_cursor_pos = range.start+src->size;
        view_replace_range(system,
            models, view, range.start, range.end,
            src->str, src->size, next_cursor_pos);

        view_cursor_move(view, next_cursor_pos);
        view->recent->mark = range.start;

        Style *style = main_style(models);
        u32 paste_color = style->main.paste_color;
        for (iter = file_view_iter_init(&models->layout, file, 0);
            file_view_iter_good(iter);
            iter = file_view_iter_next(iter)){
            view_post_paste_effect(iter.view, 20, range.start, src->size, paste_color);
        }
    }
    else{
        command_paste(system, command, binding);
    }
}

COMMAND_DECL(delete_range){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    Range range;
    i32 next_cursor_pos;

    range = make_range(view->recent->cursor.pos, view->recent->mark);
    if (range.start < range.end){
        next_cursor_pos = range.start;
        Assert(range.end <= buffer_size(&file->state.buffer));
        view_replace_range(system, models, view, range.start, range.end, 0, 0, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
        view->recent->mark = range.start;
    }
}

COMMAND_DECL(undo){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);

    view_undo(system, models, view);
    
    Assert(file->state.undo.undo.size >= 0);

}

COMMAND_DECL(redo){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);

    view_redo(system, models, view);
    
    Assert(file->state.undo.undo.size >= 0);
}

COMMAND_DECL(history_backward){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);

    view_history_step(system, models, view, hist_backward);
}

COMMAND_DECL(history_forward){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);

    view_history_step(system, models, view, hist_backward);
}

COMMAND_DECL(interactive_new){
    USE_MODELS(models);
    USE_VIEW(view);

    view_show_interactive(system, view, &models->map_ui,
        IAct_New, IInt_Sys_File_List, make_lit_string("New: "));
}

COMMAND_DECL(interactive_open){
    USE_MODELS(models);
    USE_VIEW(view);

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
            view_open_file(system, models, 0, string);
        }
        else{
            view_open_file(system, models, view, string);
        }
    }
    else{
        view_show_interactive(system, view, &models->map_ui,
            IAct_Open, IInt_Sys_File_List, make_lit_string("Open: "));
    }
}

// TODO(allen): Improvements to reopen
// - Preserve existing token stack
// - Keep current version open and do some sort of diff to keep
//    the cursor position correct
COMMAND_DECL(reopen){
    USE_MODELS(models);
    USE_VIEW(view);
    REQ_FILE(file, view);
    
    if (match(file->name.source_path, file->name.live_name)) return;
    
    File_Loading loading = system->file_load_begin(file->name.source_path.str);
    
    if (loading.exists){
        Partition *part = &models->mem.part;
        Temp_Memory temp = begin_temp_memory(part);
        char *buffer = push_array(part, char, loading.size);
        
        if (system->file_load_end(loading, buffer)){
            General_Memory *general = &models->mem.general;
            
            file_close(system, general, file);
            
            init_normal_file(system, models, file, buffer, loading.size);
        }
        
        end_temp_memory(temp);
    }
}

COMMAND_DECL(save){
    USE_MODELS(models);
    USE_VIEW(view);
    USE_FILE(file, view);

    char *filename = 0;
    int filename_len = 0;
    int buffer_id = -1;
    int update_names = 0;

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
        else if (v == par_save_update_name){
			update_names = dynamic_to_bool(&param->param.value);
		}
    }

    if (buffer_id != -1){
        file = working_set_get_active_file(&models->working_set, buffer_id);
	}
    
    if (update_names){
        String name = {};
        if (filename){
            name = make_string(filename, filename_len);
        }
        
        if (file){
            if (name.str){
                if (!file->is_dummy && file_is_ready(file)){
                    view_save_file(system, models, file, 0, name, 1);
                }
            }
            else{
                view_show_interactive(system, view, &models->map_ui,
                    IAct_Save_As, IInt_Sys_File_List, make_lit_string("Save As: "));
            }
        }
    }
    else{
        String name = {0};
        if (filename){
            name = make_string(filename, filename_len);
        }
        else if (file){
            name = file->name.source_path;
        }
        
        if (name.size != 0){
            if (file){
                if (!file->is_dummy && file_is_ready(file)){
                    view_save_file(system, models, file, 0, name, 0);
                }
            }
            else{
                view_save_file(system, models, 0, 0, name, 0);
            }
        }
    }
}

COMMAND_DECL(change_active_panel){
    
    USE_MODELS(models);
    USE_PANEL(panel);

    panel = panel->next;
    if (panel == &models->layout.used_sentinel){
        panel = panel->next;
    }
    models->layout.active_panel = (i32)(panel - models->layout.panels);
}

COMMAND_DECL(interactive_switch_buffer){
    
    USE_VIEW(view);
    USE_MODELS(models);

    view_show_interactive(system, view, &models->map_ui,
        IAct_Switch, IInt_Live_File_List, make_lit_string("Switch Buffer: "));
}

COMMAND_DECL(interactive_kill_buffer){
    
    USE_VIEW(view);
    USE_MODELS(models);

    view_show_interactive(system, view, &models->map_ui,
        IAct_Kill, IInt_Live_File_List, make_lit_string("Kill Buffer: "));
}

COMMAND_DECL(kill_buffer){
    USE_MODELS(models);
    USE_VIEW(view);
    USE_FILE(file, view);

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
            kill_file(system, models, file, string_zero());
        }
    }
    else if (file){
        try_kill_file(system, models,
                      file, view, string_zero());
    }
}

COMMAND_DECL(toggle_line_wrap){
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    Relative_Scrolling scrolling = view_get_relative_scrolling(view);
    if (view->file_data.unwrapped_lines){
        view->file_data.unwrapped_lines = 0;
        file->settings.unwrapped_lines = 0;
        view->recent->scroll.target_x = 0;
        view->recent->cursor = view_compute_cursor_from_pos(
            view, view->recent->cursor.pos);
        view->recent->preferred_x = view->recent->cursor.wrapped_x;
    }
    else{
        view->file_data.unwrapped_lines = 1;
        file->settings.unwrapped_lines = 1;
        view->recent->cursor =
            view_compute_cursor_from_pos(view, view->recent->cursor.pos);
        view->recent->preferred_x = view->recent->cursor.unwrapped_x;
    }
    view_set_relative_scrolling(view, scrolling);
}

COMMAND_DECL(toggle_show_whitespace){
    REQ_READABLE_VIEW(view);
    view->file_data.show_whitespace = !view->file_data.show_whitespace;
}

COMMAND_DECL(toggle_tokens){
#if BUFFER_EXPERIMENT_SCALPEL <= 0
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
    Range range = make_range(view->recent->cursor.pos, view->recent->mark);
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
    
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);
    case_change_range(system, &models->mem, view, file, 'a', 'z', (u8)('A' - 'a'));
}

COMMAND_DECL(to_lowercase){
    
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);
    case_change_range(system, &models->mem, view, file, 'A', 'Z', (u8)('a' - 'A'));
}

COMMAND_DECL(clean_all_lines){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    view_clean_whitespace(system, models, view);
}

COMMAND_DECL(eol_dosify){
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    file->settings.dos_write_mode = 1;
    file->state.last_4ed_edit_time = system->now_time_stamp();
}

COMMAND_DECL(eol_nixify){
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    file->settings.dos_write_mode = 0;
    file->state.last_4ed_edit_time = system->now_time_stamp();
}

COMMAND_DECL(auto_tab_range){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    int r_start = 0, r_end = 0;
    int start_set = 0, end_set = 0;
    Indent_Options opts;
    opts.empty_blank_lines = 0;
    opts.use_tabs = 0;
    opts.tab_width = 4;

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
            opts.empty_blank_lines = dynamic_to_bool(&param->param.value);
            break;
            
            case par_use_tabs:
            opts.use_tabs = dynamic_to_bool(&param->param.value);
            break;
        }
    }

    if (file->state.token_stack.tokens && file->state.tokens_complete && !file->state.still_lexing){
        Range range = make_range(view->recent->cursor.pos, view->recent->mark);
        if (start_set) range.start = r_start;
        if (end_set) range.end = r_end;
        view_auto_tab_tokens(system, models, view, range.start, range.end, opts);
    }
}

COMMAND_DECL(open_panel_vsplit){
    USE_VARS(vars);
    USE_MODELS(models);
    USE_PANEL(panel);

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
        panel_make_empty(system, vars, panel2);
    }
}

COMMAND_DECL(open_panel_hsplit){
    USE_VARS(vars);
    USE_MODELS(models);
    USE_PANEL(panel);

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
        panel_make_empty(system, vars, panel2);
    }
}

COMMAND_DECL(close_panel){
    USE_MODELS(models);
    USE_PANEL(panel);
    USE_VIEW(view);

    Panel *panel_ptr, *used_panels;
    Divider_And_ID div, parent_div, child_div;
    i32 child;
    i32 parent;
    i32 which_child;
    i32 active;

    if (models->layout.panel_count > 1){
        live_set_free_view(system, command->live_set, view);
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
    
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 pos = view->recent->cursor.pos;
    if (pos > 0) --pos;
    view_cursor_move(view, pos);
}

COMMAND_DECL(move_right){
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    i32 size = buffer_size(&file->state.buffer);
    i32 pos = view->recent->cursor.pos;
    if (pos < size) ++pos;
    view_cursor_move(view, pos);
}

COMMAND_DECL(delete){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    i32 size = buffer_size(&file->state.buffer);
    i32 cursor_pos = view->recent->cursor.pos;
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
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);

    i32 size = buffer_size(&file->state.buffer);
    i32 cursor_pos = view->recent->cursor.pos;
    if (cursor_pos > 0 && cursor_pos <= size){
        i32 start, end;
        end = cursor_pos;
        start = cursor_pos-1;

        i32 shift = (end - start);
        Assert(shift > 0);

        i32 next_cursor_pos = view->recent->cursor.pos - shift;
        view_replace_range(system, models, view, start, end, 0, 0, next_cursor_pos);
        view_cursor_move(view, next_cursor_pos);
    }
}

COMMAND_DECL(move_up){
    USE_MODELS(models);
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);

    f32 line_height = (f32)get_font_info(models->font_set, models->global_font.font_id)->height;
    f32 cy = view_get_cursor_y(view)-line_height;
    f32 px = view->recent->preferred_x;
    if (cy >= 0){
        view->recent->cursor = view_compute_cursor_from_xy(view, px, cy);
        file->state.cursor_pos = view->recent->cursor.pos;
    }
}

COMMAND_DECL(move_down){
    
    USE_MODELS(models);
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    
    f32 line_height = (f32)get_font_info(models->font_set, models->global_font.font_id)->height;
    f32 cy = view_get_cursor_y(view)+line_height;
    f32 px = view->recent->preferred_x;
    view->recent->cursor = view_compute_cursor_from_xy(view, px, cy);
    file->state.cursor_pos = view->recent->cursor.pos;
}

COMMAND_DECL(seek_end_of_line){
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = view_find_end_of_line(view, view->recent->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(seek_beginning_of_line){
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    
    i32 pos = view_find_beginning_of_line(view, view->recent->cursor.pos);
    view_cursor_move(view, pos);
}

COMMAND_DECL(page_down){
    REQ_READABLE_VIEW(view);
    
    f32 height = view_file_height(view);
    f32 max_target_y = view->recent->scroll.max_y;
    
    view->recent->scroll.target_y =
        clamp_top(view->recent->scroll.target_y + height, max_target_y);
    
    view->recent->cursor =
        view_compute_cursor_from_xy(view, 0, view->recent->scroll.target_y + (height - view->line_height)*.5f);
}

COMMAND_DECL(page_up){
    REQ_READABLE_VIEW(view);
    
    f32 height = view_file_height(view);
    
    view->recent->scroll.target_y =
        clamp_bottom(0.f, view->recent->scroll.target_y - height);
    
    view->recent->cursor =
        view_compute_cursor_from_xy(view, 0, view->recent->scroll.target_y + (height - view->line_height)*.5f);
}

COMMAND_DECL(open_color_tweaker){
    USE_VIEW(view);
    USE_MODELS(models);
    
    view_show_theme(view, &models->map_ui);
}

COMMAND_DECL(open_config){
    USE_VIEW(view);
    USE_MODELS(models);
    
    view_show_config(view, &models->map_ui);
}

COMMAND_DECL(open_menu){
    USE_VIEW(view);
    USE_MODELS(models);

    view_show_menu(view, &models->map_ui);
}

COMMAND_DECL(close_minor_view){
    USE_VIEW(view);
    view_show_file(view);
}

COMMAND_DECL(cursor_mark_swap){
    REQ_READABLE_VIEW(view);

    i32 pos = view->recent->cursor.pos;
    view_cursor_move(view, view->recent->mark);
    view->recent->mark = pos;
}

COMMAND_DECL(user_callback){
    USE_MODELS(models);
    if (binding.custom) binding.custom(&models->app_links);
}

COMMAND_DECL(hide_scrollbar){
    USE_VIEW(view);
    view->hide_scrollbar = 1;
}

COMMAND_DECL(show_scrollbar){
    USE_VIEW(view);
    view->hide_scrollbar = 0;
}

COMMAND_DECL(set_settings){
    USE_MODELS(models);
    
    Editing_File *file = 0;
    b32 set_mapid = 0;
    i32 new_mapid = 0;
    
    Command_Parameter *end = param_stack_end(&command->part);
    Command_Parameter *param = param_stack_first(&command->part, end);
    for (; param < end; param = param_next(param, end)){
        int p = dynamic_to_int(&param->param.param);
        switch (p){
            case par_buffer_id:
            {
                int v = dynamic_to_int(&param->param.value);
                file = working_set_get_active_file(&models->working_set, v);
            }break;
            
            case par_lex_as_cpp_file:
            {
#if BUFFER_EXPERIMENT_SCALPEL <= 0
                if (file){
                    int v = dynamic_to_bool(&param->param.value);
                    if (file->settings.tokens_exist){
                        if (!v) file_kill_tokens(system, &models->mem.general, file);
                    }
                    else{
                        if (v) file_first_lex_parallel(system, &models->mem.general, file);
                    }
                }
#endif
            }break;
            
            case par_wrap_lines:
            {
                int v = dynamic_to_bool(&param->param.value);
                if (file){
                    file->settings.unwrapped_lines = !v;
                }
            }break;

            case par_key_mapid:
            {
                if (file){
                    set_mapid = 1;
                    int v = dynamic_to_int(&param->param.value);
                    if (v == mapid_global) file->settings.base_map_id = mapid_global;
                    else if (v == mapid_file) file->settings.base_map_id = mapid_file;
                    else if (v < mapid_global){
                        new_mapid = get_map_index(models, v);
                        if (new_mapid  < models->user_map_count) file->settings.base_map_id = v;
                        else file->settings.base_map_id = mapid_file;
                    }
                }
            }break;
        }
    }
    
    if (set_mapid){
        for (View_Iter iter = file_view_iter_init(&models->layout, file, 0);
            file_view_iter_good(iter);
            iter = file_view_iter_next(iter)){
            iter.view->map = get_map(models, file->settings.base_map_id);
        }
    }
}

COMMAND_DECL(command_line){
    USE_VARS(vars);
    USE_MODELS(models);
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
    
    char feedback_space[256];
    String feedback_str = make_fixed_width_string(feedback_space);

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
        General_Memory *general = &models->mem.general;

        if (vars->cli_processes.count < vars->cli_processes.max){
            if (buffer_id){
                file = working_set_get_active_file(working_set, buffer_id);
                if (file && file->settings.read_only == 0){
                    append(&feedback_str, "ERROR: ");
                    append(&feedback_str, file->name.live_name);
                    append(&feedback_str, " is not a read-only buffer\n");
                    do_feedback_message(system, models, feedback_str);
                    return;
                }
                if (file->settings.never_kill){
                    append(&feedback_str, "The buffer ");
                    append(&feedback_str, file->name.live_name);
                    append(&feedback_str, " is not killable");
                    do_feedback_message(system, models, feedback_str);
                    return;
                }
            }
            else if (buffer_name){
                file = working_set_contains(system, working_set, make_string(buffer_name, buffer_name_len));
                if (file){
                    if (file->settings.read_only == 0){
                        append(&feedback_str, "ERROR: ");
                        append(&feedback_str, file->name.live_name);
                        append(&feedback_str, " is not a read-only buffer\n");
                        do_feedback_message(system, models, feedback_str);
                        return;
                    }
                    if (file->settings.never_kill){
                        append(&feedback_str, "The buffer ");
                        append(&feedback_str, file->name.live_name);
                        append(&feedback_str, " is not killable");
                        do_feedback_message(system, models, feedback_str);
                        return;
                    }
                }
                else{
                    file = working_set_alloc_always(working_set, general);
                    if (file == 0){
                        append(&feedback_str, "ERROR: unable to  allocate a new buffer\n");
                        do_feedback_message(system, models, feedback_str);
                        return;
                    }
                    file_create_read_only(system, models, file, buffer_name);
                    working_set_add(system, working_set, file, general);
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
                    append(&feedback_str, "did not begin command-line command because the target buffer is already in use\n");
                    do_feedback_message(system, models, feedback_str);
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
                Editing_File *file2;
                i32 size;

                temp = begin_temp_memory(part);
                if (!script){
                    file2 = view->file_data.file;
                    if (file2){
                        range = make_range(view->recent->cursor.pos, view->recent->mark);
                        size = range.end - range.start;
                        script = push_array(part, char, size + 1);
                        buffer_stringify(&file2->state.buffer, range.start, range.end, script);
                        script[size] = 0;
                    }
                    else{
                        script = " echo no script specified";
                    }
                }

                if (bind_to_new_view){
                    view_set_file(view, file, models);
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
            append(&feedback_str, "ERROR: no available process slot\n");
            do_feedback_message(system, models, feedback_str);
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
    *buffer = buffer_summary_zero();
    if (!file->is_dummy){
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
    *view = view_summary_zero();

    if (vptr->in_use){
        view->exists = 1;
        view->view_id = (int)(vptr - live_set->views) + 1;
        view->line_height = vptr->line_height;
        view->unwrapped_lines = vptr->file_data.unwrapped_lines;

        if (vptr->file_data.file){
            lock_level = view_lock_level(vptr);
            buffer_id = vptr->file_data.file->id.id;

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

            view->mark = view_compute_cursor_from_pos(vptr, vptr->recent->mark);
            view->cursor = vptr->recent->cursor;
            view->preferred_x = vptr->recent->preferred_x;
        }
    }
}

extern "C"{
    EXEC_COMMAND_KEEP_STACK_SIG(external_exec_command_keep_stack){
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
            *buffer = buffer_summary_zero();
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

    GET_PARAMETER_BUFFER_SIG(external_get_parameter_buffer){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Models *models = cmd->models;
        Buffer_Summary buffer = {};

        if (param_index >= 0 && param_index < models->buffer_param_count){
            buffer = external_get_buffer(app, models->buffer_param_indices[param_index]);
        }

        return(buffer);
    }

    GET_BUFFER_BY_NAME_SIG(external_get_buffer_by_name){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Buffer_Summary buffer = {};
        Editing_File *file;
        Working_Set *working_set = &cmd->models->working_set;

        file = working_set_contains(cmd->system, working_set, make_string(filename, len));
        if (file && !file->is_dummy){
            fill_buffer_summary(&buffer, file, working_set);
        }
        
        return(buffer);
    }
    
#if 0
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
    
    // TODO(allen): Reduce duplication between this and external_buffer_seek_string
    BUFFER_SEEK_STRING_SIG(external_buffer_seek_string_insensitive){
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
                        *out = buffer_find_string_insensitive(&file->state.buffer, start, size, str, len, spare);
                    }
                    else{
                        *out = buffer_rfind_string_insensitive(&file->state.buffer, start, str, len, spare);
                    }
                    end_temp_memory(temp);
                }
                fill_buffer_summary(buffer, file, working_set);
            }
        }

        return(result);
    }
#endif

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
                *view = view_summary_zero();
            }
        }
        else{
            *view = view_summary_zero();
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
    
    VIEW_COMPUTE_CURSOR_SIG(external_view_compute_cursor){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Live_Views *live_set;
        View *vptr;
        Editing_File *file;
        Full_Cursor result = {0};
        int view_id;
        
        if (view->exists){
            live_set = cmd->live_set;
            view_id = view->view_id - 1;
            if (view_id >= 0 && view_id < live_set->max){
                vptr = live_set->views + view_id;
                file = vptr->file_data.file;
                if (file && !file->is_loading){
                    if (seek.type == buffer_seek_line_char && seek.character <= 0){
                        seek.character = 1;
                    }
                    result = view_compute_cursor(vptr, seek);
                    fill_view_summary(view, vptr, live_set, &cmd->models->working_set);
                }
            }
        }
        
        return(result);
    }
    
    VIEW_SET_CURSOR_SIG(external_view_set_cursor){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Live_Views *live_set;
        View *vptr;
        Editing_File *file;
        int result = 0;
        int view_id;

        if (view->exists){
            live_set = cmd->live_set;
            view_id = view->view_id - 1;
            if (view_id >= 0 && view_id < live_set->max){
                vptr = live_set->views + view_id;
                file = vptr->file_data.file;
                if (file && !file->is_loading){
                    result = 1;
                    if (seek.type == buffer_seek_line_char && seek.character <= 0){
                        seek.character = 1;
                    }
                    vptr->recent->cursor = view_compute_cursor(vptr, seek);
                    if (set_preferred_x){
                        vptr->recent->preferred_x = view_get_cursor_x(vptr);
                    }
                    fill_view_summary(view, vptr, live_set, &cmd->models->working_set);
                    file->state.cursor_pos = vptr->recent->cursor.pos;
                }
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
                    vptr->recent->mark = cursor.pos;
                }
                else{
                    vptr->recent->mark = seek.pos;
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
                    vptr->file_data.show_temp_highlight = 0;
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
                    if (file != vptr->file_data.file){
                        view_set_file(vptr, file, models);
                        view_show_file(vptr);
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
        Coroutine *coroutine = (Coroutine*)app->current_coroutine;
        User_Input result = {0};
        
        if (app->type_coroutine == Co_Command){
            Assert(coroutine);
            *((u32*)coroutine->out+0) = get_type;
            *((u32*)coroutine->out+1) = abort_type;
            system->yield_coroutine(coroutine);
            result = *(User_Input*)coroutine->in;
        }
        
        return(result);
    }
    
    GET_COMMAND_INPUT_SIG(external_get_command_input){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        User_Input result;
        
        result.type = UserInputKey;
        result.abort = 0;
        result.key = cmd->key;
        result.command = 0;
        
        return(result);
    }
    
    GET_EVENT_MESSAGE_SIG(external_get_event_message){
        Event_Message message = {0};
        System_Functions *system = (System_Functions*)app->system_links;
        Coroutine *coroutine = (Coroutine*)app->current_coroutine;
        
        if (app->type_coroutine == Co_View){
            Assert(coroutine);
            system->yield_coroutine(coroutine);
            message = *(Event_Message*)coroutine->in;
        }
        
        return(message);
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
    
    PRINT_MESSAGE_SIG(external_print_message){
        Command_Data *cmd = (Command_Data*)app->cmd_context;
        Models *models = cmd->models;
        do_feedback_message(cmd->system, models, make_string(string, len));
    }
    
    GET_GUI_FUNCTIONS_SIG(external_get_gui_functions){
        GUI_Functions *guifn = 0;
        NotImplemented;
        return(guifn);
    }
    
    GET_GUI_SIG(external_get_gui){
        GUI *gui = 0;
        NotImplemented;
        return(gui);
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
                style_copy(main_style(cmd->models), s);
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
        Style *style = main_style(cmd->models);
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
    view->next_mode = view_mode_zero();
    cmd_in->bind.function(cmd->system, cmd, cmd_in->bind);
    view->mode = view->next_mode;
}

internal void
view_caller(Coroutine *coroutine){
    View *view = (View*)coroutine->in;
    View_Persistent *persistent = &view->persistent;
    persistent->view_routine(&persistent->models->app_links, persistent->id);
}

internal void
app_links_init(System_Functions *system, Application_Links *app_links, void *data, int size){
    app_links->memory = data;
    app_links->memory_size = size;
    
    app_links->exec_command_keep_stack = external_exec_command_keep_stack;
    app_links->push_parameter = external_push_parameter;
    app_links->push_memory = external_push_memory;
    app_links->clear_parameters = external_clear_parameters;
    
    app_links->directory_get_hot = external_directory_get_hot;
    app_links->get_4ed_path = system->get_4ed_path;
    app_links->file_exists = system->file_exists;
    app_links->directory_cd = system->directory_cd;
    app_links->get_file_list = external_get_file_list;
    app_links->free_file_list = external_free_file_list;
    
    app_links->get_buffer_first = external_get_buffer_first;
    app_links->get_buffer_next = external_get_buffer_next;
    
    app_links->get_buffer = external_get_buffer;
    app_links->get_parameter_buffer = external_get_parameter_buffer;
    app_links->get_buffer_by_name = external_get_buffer_by_name;
    
    app_links->refresh_buffer = external_refresh_buffer;
    
#if 0
    app_links->buffer_seek_delimiter = external_buffer_seek_delimiter;
    app_links->buffer_seek_string = external_buffer_seek_string;
    app_links->buffer_seek_string_insensitive = external_buffer_seek_string_insensitive;
#endif

    app_links->buffer_read_range = external_buffer_read_range;
    app_links->buffer_replace_range = external_buffer_replace_range;
    
    app_links->get_view_first = external_get_view_first;
    app_links->get_view_next = external_get_view_next;
    
    app_links->get_view = external_get_view;
    app_links->get_active_view = external_get_active_view;
    
    app_links->refresh_view = external_refresh_view;
    app_links->view_compute_cursor = external_view_compute_cursor;
    app_links->view_set_cursor = external_view_set_cursor;
    app_links->view_set_mark = external_view_set_mark;
    app_links->view_set_highlight = external_view_set_highlight;
    app_links->view_set_buffer = external_view_set_buffer;
    
    app_links->get_user_input = external_get_user_input;
    app_links->get_command_input = external_get_command_input;
    app_links->get_event_message = external_get_event_message;
    
    app_links->start_query_bar = external_start_query_bar;
    app_links->end_query_bar = external_end_query_bar;
    app_links->print_message = external_print_message;
    app_links->get_gui_functions = external_get_gui_functions;
    app_links->get_gui = external_get_gui;
    
    app_links->change_theme = external_change_theme;
    app_links->change_font = external_change_font;
    app_links->set_theme_colors = external_set_theme_colors;
    
    app_links->current_coroutine = 0;
    app_links->system_links = system;
}

internal void
setup_ui_commands(Command_Map *commands, Partition *part, Command_Map *parent){
    map_init(commands, part, 32, parent);

    commands->vanilla_keyboard_default.function = command_null;

    // TODO(allen): This is hacky, when the new UI stuff happens, let's fix it,
    // and by that I mean actually fix it, don't just say you fixed it with
    // something stupid again.
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
    SET(center_view);
    SET(word_complete);
    SET(set_mark);
    SET(copy);
    SET(cut);
    SET(paste);
    SET(paste_next);
    SET(delete_range);
    SET(undo);
    SET(redo);
    SET(history_backward);
    SET(history_forward);
    SET(interactive_new);
    SET(interactive_open);
    SET(reopen);
    SET(save);
    //SET(interactive_save_as);
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
    SET(hide_scrollbar);
    SET(show_scrollbar);
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
    style = styles + 1;

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
    style->main.default_color = 0xFFA08563;
    style->main.comment_color = 0xFF7D7D7D;
    style->main.keyword_color = 0xFFCD950C;
    style->main.str_constant_color = 0xFF6B8E23;
    style->main.char_constant_color = style->main.str_constant_color;
    style->main.int_constant_color = style->main.str_constant_color;
    style->main.float_constant_color = style->main.str_constant_color;
    style->main.bool_constant_color = style->main.str_constant_color;
    style->main.include_color = 0xFF6B8E23;
    style->main.preproc_color = 0xFFDAB98F;
    style->main.special_character_color = 0xFFFF0000;

    style->main.paste_color = 0xFFFFBB00;
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
    style_copy(main_style(models), models->styles.styles + 1);
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
    CLAct_FontSize,
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
                        
                        case 'f': action = CLAct_FontSize; break;
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
            
            case CLAct_FontSize:
            {
                if (i < clparams.argc){
                    settings->font_size = str_to_int(clparams.argv[i]);
                }
                action = CLAct_Nothing;
            }break;
        }
    }
}

inline App_Vars
app_vars_zero(){
    App_Vars vars={0};
    return(vars);
}

internal App_Vars*
app_setup_memory(Application_Memory *memory){
    Partition _partition = make_part(memory->vars_memory, memory->vars_memory_size);
    App_Vars *vars = push_struct(&_partition, App_Vars);
    Assert(vars);
    *vars = app_vars_zero();
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
            ((char*)memory)[result++] = '\n';
        }
    }
    return(result);
}

inline App_Settings
app_settings_zero(){
    App_Settings settings={0};
    return(settings);
}

App_Read_Command_Line_Sig(app_read_command_line){
    App_Vars *vars;
    App_Settings *settings;
    i32 out_size = 0;
    
    if (clparams.argc > 1 && match(clparams.argv[1], "-T")){
        out_size = execute_special_tool(memory->target_memory, memory->target_memory_size, clparams);
    }
    else{
        vars = app_setup_memory(memory);
        
        settings = &vars->models.settings;
        *settings = app_settings_zero();
        settings->font_size = 16;
        
        if (clparams.argc > 1){
            init_command_line_settings(&vars->models.settings, plat_settings, clparams);
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
    
    vars = (App_Vars*)memory->vars_memory;
    models = &vars->models;
    models->keep_playing = 1;
    
    app_links_init(system, &models->app_links, memory->user_memory, memory->user_memory_size);
    
    models->config_api = api;
    models->app_links.cmd_context = &vars->command_data;
    
    partition = &models->mem.part;
    target->partition = partition;
    
    {
        i32 i;
        
        panel_max_count = models->layout.panel_max_count = MAX_VIEWS;
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
        View *view = 0;
        View_Persistent *persistent = 0;
        i32 i = 0;
        i32 max = 0;
        
        vars->live_set.count = 0;
        vars->live_set.max = panel_max_count;
        
        vars->live_set.views = push_array(partition, View, vars->live_set.max);
        
        dll_init_sentinel(&vars->live_set.free_sentinel);
        
        max = vars->live_set.max;
        view = vars->live_set.views;
        for (i = 0; i < max; ++i, ++view){
            dll_insert(&vars->live_set.free_sentinel, view);
            
            persistent = &view->persistent;
            persistent->id = i;
            persistent->models = models;
            persistent->view_routine = models->config_api.view_routine;
        }
    }
    
    {
        Command_Map *global = 0;
        i32 wanted_size = 0;
        b32 did_top = 0;
        b32 did_file = 0;
        
        models->scroll_rule = fallback_scroll_rule;
        
        setup_command_table();
        
        global = &models->map_top;
        Assert(models->config_api.get_bindings != 0);
        
        wanted_size = models->config_api.get_bindings(models->app_links.memory, models->app_links.memory_size);
        
        if (wanted_size <= models->app_links.memory_size){
            Command_Map *map_ptr = 0;
            Binding_Unit *unit, *end;
            i32 user_map_count;
            
            unit = (Binding_Unit*)models->app_links.memory;
            if (unit->type == unit_header && unit->header.error == 0){
                end = unit + unit->header.total_size;
                
                user_map_count = unit->header.user_map_count;
                
                models->map_id_table = push_array(
                                                  &models->mem.part, i32, user_map_count);
                memset(models->map_id_table, -1, user_map_count*sizeof(i32));
                
                models->user_maps = push_array(
                                               &models->mem.part, Command_Map, user_map_count);
                
                models->user_map_count = user_map_count;
                
                for (++unit; unit < end; ++unit){
                    switch (unit->type){
                        case unit_map_begin:
                        {
                            int mapid = unit->map_begin.mapid;
                            int count = map_get_count(models, mapid);
                            if (unit->map_begin.replace){
                                map_set_count(models, mapid, unit->map_begin.bind_count);
                            }
                            else{
                                map_set_count(models, mapid, unit->map_begin.bind_count + count);
                            }
                        };
                    }
                }
                
                unit = (Binding_Unit*)models->app_links.memory;
                for (++unit; unit < end; ++unit){
                    switch (unit->type){
                        case unit_map_begin:
                        {
                            int mapid = unit->map_begin.mapid;
                            int count = map_get_max_count(models, mapid);
                            int table_max = count * 3 / 2;
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
                                i32 index = get_or_add_map_index(models, mapid);
                                Assert(index < user_map_count);
                                map_ptr = models->user_maps + index;
                                map_init(map_ptr, &models->mem.part, table_max, global);
                            }
                            else map_ptr = 0;
                            
                            if (map_ptr && unit->map_begin.replace){
                                map_clear(map_ptr);
                            }
                        }break;
                        
                        case unit_inherit:
                        if (map_ptr){
                            Command_Map *parent = 0;
                            int mapid = unit->map_inherit.mapid;
                            if (mapid == mapid_global) parent = &models->map_top;
                            else if (mapid == mapid_file) parent = &models->map_file;
                            else if (mapid < mapid_global){
                                i32 index = get_or_add_map_index(models, mapid);
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
        
        memset(models->app_links.memory, 0, wanted_size);
        if (!did_top) setup_top_commands(&models->map_top, &models->mem.part, global);
        if (!did_file) setup_file_commands(&models->map_file, &models->mem.part, global);
        
#ifndef FRED_SUPER
        models->hooks[hook_start] = 0;
#endif
        
        setup_ui_commands(&models->map_ui, &models->mem.part, global);
    }
    
    // NOTE(allen): font setup
    {
        models->font_set = &target->font_set;
        
        font_set_init(models->font_set, partition, 16, 5);
        
        struct Font_Setup{
            char *c_file_name;
            i32 file_name_len;
            char *c_name;
            i32 name_len;
            i32 pt_size;
        };
        
        int font_size = models->settings.font_size;
        
        if (font_size < 8) font_size = 8;
        
        Font_Setup font_setup[] = {
            {literal("LiberationSans-Regular.ttf"),
                literal("liberation sans"),
                font_size},
            
            {literal("liberation-mono.ttf"),
                literal("liberation mono"),
                font_size},
            
            {literal("Hack-Regular.ttf"),
                literal("hack"),
                font_size},
            
            {literal("CutiveMono-Regular.ttf"),
                literal("cutive mono"),
                font_size},
            
            {literal("Inconsolata-Regular.ttf"),
                literal("inconsolata"),
                font_size},
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
    working_set_init(&models->working_set, partition, &vars->models.mem.general);
    
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
    
    // NOTE(allen): style setup
    app_hardcode_styles(models);
    
    models->palette_size = 40;
    models->palette = push_array(partition, u32, models->palette_size);
    
    // NOTE(allen): init first panel
    Panel_And_ID p = layout_alloc_panel(&models->layout);
    panel_make_empty(system, vars, p.panel);
    models->layout.active_panel = p.id;
    
    String hdbase = make_fixed_width_string(models->hot_dir_base_);
    hot_directory_init(&models->hot_directory, hdbase, current_directory, system->slash);
    
    // NOTE(allen): child proc list setup
    i32 max_children = 16;
    partition_align(partition, 8);
    vars->cli_processes.procs = push_array(partition, CLI_Process, max_children);
    vars->cli_processes.max = max_children;
    vars->cli_processes.count = 0;
    
    // NOTE(allen): parameter setup
    models->buffer_param_max = 1;
    models->buffer_param_count = 0;
    models->buffer_param_indices = push_array(partition, i32, models->buffer_param_max);
}

internal i32
update_cli_handle_with_file(System_Functions *system, Models *models,
                            CLI_Handles *cli, Editing_File *file, char *dest, i32 max, b32 cursor_at_end){
    i32 result = 0;
    u32 amount;
    
    for (system->cli_begin_update(cli);
         system->cli_update_step(cli, dest, max, &amount);){
        amount = eol_in_place_convert_in(dest, amount);
        output_file_append(system, models, file, make_string(dest, amount), cursor_at_end);
        result = 1;
    }
    
    if (system->cli_end_update(cli)){
        char str_space[256];
        String str = make_fixed_width_string(str_space);
        append(&str, "exited with code ");
        append_int_to_str(cli->exit, &str);
        output_file_append(system, models, file, str, cursor_at_end);
        result = -1;
    }
    
    i32 new_cursor = 0;
    
    if (cursor_at_end){
        new_cursor = buffer_size(&file->state.buffer);
    }
    
    for (View_Iter iter = file_view_iter_init(&models->layout, file, 0);
         file_view_iter_good(iter);
         iter = file_view_iter_next(iter)){
        view_cursor_move(iter.view, new_cursor);
    }
    
    return(result);
}

enum Input_Types{
    Input_AnyKey,
    Input_Esc,
    Input_MouseMove,
    Input_MouseLeftButton,
    Input_MouseRightButton,
    Input_MouseWheel,
    Input_Count
};

struct Available_Input{
    Key_Summary *keys;
    Mouse_State *mouse;
    b32 consumed[Input_Count];
};

Available_Input
init_available_input(Key_Summary *keys, Mouse_State *mouse){
    Available_Input result = {0};
    result.keys = keys;
    result.mouse = mouse;
    return(result);
}

Key_Summary
get_key_data(Available_Input *available){
    Key_Summary result = {0};
    
    if (!available->consumed[Input_AnyKey]){
        result = *available->keys;
    }
    else if (!available->consumed[Input_Esc]){
        i32 i = 0;
        i32 count = available->keys->count;
        Key_Event_Data key = {0};
        
        for (i = 0; i < count; ++i){
            key = get_single_key(available->keys, i);
            if (key.keycode == key_esc){
                result.count = 1;
                result.keys[0] = key;
                break;
            }
        }
    }
    
    return(result);
}

Mouse_State
get_mouse_state(Available_Input *available){
    Mouse_State mouse = *available->mouse;
    if (available->consumed[Input_MouseLeftButton]){
        mouse.l = 0;
        mouse.press_l = 0;
        mouse.release_l = 0;
    }
    
    if (available->consumed[Input_MouseRightButton]){
        mouse.r = 0;
        mouse.press_r = 0;
        mouse.release_r = 0;
    }
    
    if (available->consumed[Input_MouseWheel]){
        mouse.wheel = 0;
    }
    
    return(mouse);
}

void
consume_input(Available_Input *available, i32 input_type){
    available->consumed[input_type] = 1;
}

App_Step_Sig(app_step){
    Application_Step_Result app_result = *result;
    app_result.animating = 0;
    
    App_Vars *vars = (App_Vars*)memory->vars_memory;
    Models *models = &vars->models;
    target->partition = &models->mem.part;
    
    // NOTE(allen): OS clipboard event handling
    String clipboard = input->clipboard;
    
    if (clipboard.str){
        String *dest = working_set_next_clipboard_string(&models->mem.general, &models->working_set, clipboard.size);
        dest->size = eol_convert_in(dest->str, clipboard.str, clipboard.size);
    }
    
    // NOTE(allen): check files are up to date
    {
        File_Node *node, *used_nodes;
        Editing_File *file;
        u64 time_stamp;
        
        used_nodes = &models->working_set.used_sentinel;
        for (dll_items(node, used_nodes)){
            file = (Editing_File*)node;
            
            time_stamp = system->file_time_stamp(make_c_str(file->name.source_path));
            
            if (time_stamp > 0){
                file->state.last_sys_write_time = time_stamp;
            }
        }
    }
    
    // NOTE(allen): begin allowing the cursors and scroll locations
    // to move around.
    {
        Panel *panel = 0, *used_panels = 0;
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            Assert(panel->view);
            view_begin_cursor_scroll_updates(panel->view);
        }
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
                view_change_size(&models->mem.general, view);
            }
        }
    }
    
    // NOTE(allen): prepare input information
    Key_Summary key_summary = {0};
    for (i32 i = 0; i < input->keys.press_count; ++i){
        key_summary.keys[key_summary.count++] = input->keys.press[i];
    }
    for (i32 i = 0; i < input->keys.hold_count; ++i){
        key_summary.keys[key_summary.count++] = input->keys.hold[i];
    }
    
    input->mouse.wheel = -input->mouse.wheel;
    
    // NOTE(allen): detect mouse hover status
    i32 mx = input->mouse.x;
    i32 my = input->mouse.y;
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
    
    // NOTE(allen): update child processes
    if (input->dt > 0){
        Temp_Memory temp = begin_temp_memory(&models->mem.part);
        u32 max = Kbytes(32);
        char *dest = push_array(&models->mem.part, char, max);
        
        i32 count = vars->cli_processes.count;
        for (i32 i = 0; i < count; ++i){
            CLI_Process *proc = vars->cli_processes.procs + i;
            Editing_File *file = proc->out_file;
            
            if (file != 0){
                i32 r = update_cli_handle_with_file(system, models, &proc->cli, file, dest, max, 0);
                if (r < 0){
                    *proc = vars->cli_processes.procs[--count];
                    --i;
                }
            }
        }
        
        vars->cli_processes.count = count;
        end_temp_memory(temp);
    }
    
    // NOTE(allen): prepare to start executing commands
    Command_Data *cmd = &vars->command_data;
    
    cmd->models = models;
    cmd->vars = vars;
    cmd->system = system;
    cmd->live_set = &vars->live_set;
    
    cmd->panel = models->layout.panels + models->layout.active_panel;
    cmd->view = cmd->panel->view;
    
    cmd->screen_width = target->width;
    cmd->screen_height = target->height;
    
    cmd->key = key_event_data_zero();
    
    Temp_Memory param_stack_temp = begin_temp_memory(&models->mem.part);
    cmd->part = partition_sub_part(&models->mem.part, 16 << 10);
    
    if (input->first_step){
        
#if 0
        {
            View *view = 0;
            View_Persistent *persistent = 0;
            i32 i = 0;
            i32 max = 0;
            
            max = vars->live_set.max;
            view = vars->live_set.views;
            for (i = 1; i <= max; ++i, ++view){
                persistent = &view->persistent;
                
                persistent->coroutine =
                    system->create_coroutine(view_caller);
                
                persistent->coroutine =
                    app_launch_coroutine(system, &models->app_links, Co_View,
                                         persistent->coroutine, view, 0);
                
                if (!persistent->coroutine){
                    // TODO(allen): Error message and recover
                    NotImplemented;
                }
            }
        }
#endif
        
        General_Memory *general = &models->mem.general;
        Editing_File *file = working_set_alloc_always(&models->working_set, general);
        file_create_read_only(system, models, file, "*messages*");
        working_set_add(system, &models->working_set, file, general);
        file->settings.never_kill = 1;
        file->settings.unimportant = 1;
        file->settings.unwrapped_lines = 1;
        
        models->message_buffer = file;
        
        if (models->hooks[hook_start]){
            models->hooks[hook_start](&models->app_links);
            cmd->part.pos = 0;
        }
        
        i32 i;
        String filename;
        Panel *panel = models->layout.used_sentinel.next;
        for (i = 0;
             i < models->settings.init_files_count;
             ++i, panel = panel->next){
            filename = make_string_slowly(models->settings.init_files[i]);
            
            if (i < models->layout.panel_count){
                view_open_file(system, models, panel->view, filename);
                view_show_file(panel->view);
#if 0
                if (i == 0){
                    if (panel->view->file_data.file){
                        // TODO(allen): How to set the cursor of a file on the first frame?
                        view_compute_cursor_from_line_pos(panel->view, models->settings.initial_line, 1);
                        view_move_view_to_cursor(panel->view, &panel->view->recent->scroll);
                    }
                }
#endif
            }
            else{
                view_open_file(system, models, 0, filename);
            }
        }
        
        if (i < models->layout.panel_count){
            view_set_file(panel->view, models->message_buffer, models);
            view_show_file(panel->view);
        }
    }
    
    // NOTE(allen): respond if the user is trying to kill the application
    if (app_result.trying_to_kill){
        b32 there_is_unsaved = 0;
        app_result.animating = 1;
        
        File_Node *node, *sent;
        sent = &models->working_set.used_sentinel;
        for (dll_items(node, sent)){
            Editing_File *file = (Editing_File*)node;
            if (buffer_needs_save(file)){
                there_is_unsaved = 1;
                break;
            }
        }
        
        if (there_is_unsaved){
            Coroutine *command_coroutine = models->command_coroutine;
            View *view = cmd->view;
            i32 i = 0;
            
            while (command_coroutine){
                User_Input user_in = {0};
                user_in.abort = 1;
                
                command_coroutine =
                    app_resume_coroutine(system, &models->app_links, Co_Command,
                                         command_coroutine, &user_in,
                                         models->command_coroutine_flags);
                
                ++i;
                if (i >= 128){
                    // TODO(allen): post grave warning, resource cleanup system.
                    command_coroutine = 0;
                }
            }
            if (view != 0){
                init_query_set(&view->query_set);
            }
            
            if (view == 0){
                Panel *panel = models->layout.used_sentinel.next;
                view = panel->view;
            }
            
            view_show_interactive(system, view, &models->map_ui,
                                  IAct_Sure_To_Close, IInt_Sure_To_Close,
                                  make_lit_string("Are you sure?"));
            
            models->command_coroutine = command_coroutine;
        }
        else{
            models->keep_playing = 0;
        }
    }
    
    // NOTE(allen): Keyboard input to command coroutine.
    Available_Input available_input = init_available_input(&key_summary, &input->mouse);
    
    if (models->command_coroutine != 0){
        Coroutine *command_coroutine = models->command_coroutine;
        u32 get_flags = models->command_coroutine_flags[0];
        u32 abort_flags = models->command_coroutine_flags[1];
        
        get_flags |= abort_flags;
        
        if ((get_flags & EventOnAnyKey) || (get_flags & EventOnEsc)){
            Key_Summary key_data = get_key_data(&available_input);
            
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
                    consume_input(&available_input, Input_AnyKey);
                }
                if (key.keycode == key_esc){
                    if (EventOnEsc & get_flags){
                        pass_in = 1;
                    }
                    consume_input(&available_input, Input_Esc);
                }
                
                if (pass_in){
                    models->command_coroutine =
                        app_resume_coroutine(system, &models->app_links, Co_Command,
                                             command_coroutine,
                                             &user_in,
                                             models->command_coroutine_flags);
                    
                    app_result.animating = 1;
                    
                    // TOOD(allen): Deduplicate
                    // TODO(allen): Should I somehow allow a view to clean up however it wants after a
                    // command finishes, or after transfering to another view mid command?
                    if (view != 0 && models->command_coroutine == 0){
                        init_query_set(&view->query_set);
                    }
                    if (models->command_coroutine == 0) break;
                }
            }
        }
        
        // NOTE(allen): Mouse input to command coroutine
        if (models->command_coroutine != 0 && (get_flags & EventOnMouse)){
            View *view = cmd->view;
            b32 pass_in = 0;
            
            User_Input user_in;
            user_in.type = UserInputMouse;
            user_in.mouse = input->mouse;
            user_in.command = 0;
            user_in.abort = 0;
            
            if (abort_flags & EventOnMouseMove){
                user_in.abort = 1;
            }
            if (get_flags & EventOnMouseMove){
                pass_in = 1;
                consume_input(&available_input, Input_MouseMove);
            }
            
            if (input->mouse.press_l || input->mouse.release_l || input->mouse.l){
                if (abort_flags & EventOnLeftButton){
                    user_in.abort = 1;
                }
                if (get_flags & EventOnLeftButton){
                    pass_in = 1;
                    consume_input(&available_input, Input_MouseLeftButton);
                }
            }
            
            if (input->mouse.press_r || input->mouse.release_r || input->mouse.r){
                if (abort_flags & EventOnRightButton){
                    user_in.abort = 1;
                }
                if (get_flags & EventOnRightButton){
                    pass_in = 1;
                    consume_input(&available_input, Input_MouseRightButton);
                }
            }
            
            if (input->mouse.wheel != 0){
                if (abort_flags & EventOnWheel){
                    user_in.abort = 1;
                }
                if (get_flags & EventOnWheel){
                    pass_in = 1;
                    consume_input(&available_input, Input_MouseWheel);
                }
            }
            
            if (pass_in){
                models->command_coroutine = 
                    app_resume_coroutine(system, &models->app_links, Co_Command,
                                         command_coroutine,
                                         &user_in,
                                         models->command_coroutine_flags);
                
                app_result.animating = 1;
                
                // TOOD(allen): Deduplicate
                // TODO(allen): Should I somehow allow a view to clean up however it wants after a
                // command finishes, or after transfering to another view mid command?
                if (view != 0 && models->command_coroutine == 0){
                    init_query_set(&view->query_set);
                }
            }
        }
    }
    
    update_command_data(vars, cmd);
    
    // NOTE(allen): pass raw input to the panels
    Input_Summary dead_input = {};
    dead_input.mouse.x = input->mouse.x;
    dead_input.mouse.y = input->mouse.y;
    
    Input_Summary active_input = {};
    active_input.mouse.x = input->mouse.x;
    active_input.mouse.y = input->mouse.y;
    
    active_input.keys = get_key_data(&available_input);
    
    Mouse_State mouse_state = get_mouse_state(&available_input);
    
    {
        Panel *panel = 0, *used_panels = 0;
        View *view = 0, *active_view = 0;
        b32 active = 0;
        Input_Summary summary = {0};
        Input_Process_Result result = {0};
        
        active_view = cmd->panel->view;
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            view = panel->view;
            active = (panel == cmd->panel);
            summary = (active)?(active_input):(dead_input);
            
            View_Step_Result result = step_file_view(system, view, active_view, summary);
            if (result.animating){
                app_result.animating = 1;
            }
            if (result.consume_keys){
                consume_input(&available_input, Input_AnyKey);
            }
            if (result.consume_keys || result.consume_esc){
                consume_input(&available_input, Input_Esc);
            }
        }
        
        for (dll_items(panel, used_panels)){
            view = panel->view;
            Assert(view->current_scroll);
            active = (panel == cmd->panel);
            summary = (active)?(active_input):(dead_input);
            if (panel == mouse_panel && !input->mouse.out_of_window){
                summary.mouse = mouse_state;
            }
            
            
            GUI_Scroll_Vars *vars = view->current_scroll;
            // TODO(allen): I feel like the scroll context should actually not
            // be allowed to change in here at all.
            result = do_input_file_view(system, view, panel->inner, active,
                                        &summary, *vars, view->scroll_region);
            if (result.is_animating){
                app_result.animating = 1;
            }
            *vars = result.vars;
            view->scroll_region = result.region;
        }
    }
    
    update_command_data(vars, cmd);
    
    // NOTE(allen): command execution
    {
        Key_Summary key_data = get_key_data(&available_input);
        b32 hit_something = 0;
        b32 hit_esc = 0;
        
        for (i32 key_i = 0; key_i < key_data.count; ++key_i){
            if (models->command_coroutine != 0) break;
            
            switch (vars->state){
                case APP_STATE_EDIT:
                {
                    Key_Event_Data key = get_single_key(&key_data, key_i);
                    cmd->key = key;
                    
                    View *view = cmd->view;
                    
                    Command_Map *map = 0;
                    if (view) map = view->map;
                    if (map == 0) map = &models->map_top;
                    Command_Binding cmd_bind = map_extract_recursive(map, key);
                    
                    if (cmd_bind.function){
                        if (key.keycode == key_esc){
                            hit_esc = 1;
                        }
                        else{
                            hit_something = 1;
                        }
                        
                        Assert(models->command_coroutine == 0);
                        Coroutine *command_coroutine = system->create_coroutine(command_caller);
                        models->command_coroutine = command_coroutine;
                        
                        Command_In cmd_in;
                        cmd_in.cmd = cmd;
                        cmd_in.bind = cmd_bind;
                        
                        models->command_coroutine =
                            app_launch_coroutine(system, &models->app_links, Co_Command,
                                                 models->command_coroutine,
                                                 &cmd_in,
                                                 models->command_coroutine_flags);
                        
                        models->prev_command = cmd_bind;
                        
                        app_result.animating = 1;
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
        
        if (hit_something){
            consume_input(&available_input, Input_AnyKey);
        }
        if (hit_esc){
            consume_input(&available_input, Input_Esc);
        }
    }
    
    update_command_data(vars, cmd);
    
    // NOTE(allen): initialize message
    if (input->first_step){
        String welcome =
            make_lit_string("Welcome to " VERSION "\n"
                            "If you're new to 4coder there's no tutorial yet :(\n"
                            "you can use the key combo control + o to look for a file\n"
                            "and if you load README.txt you'll find all the key combos there are.\n"
                            "\n"
                            "Newest features:\n"
                            "-Tied the view scrolling and the list arrow navigation together\n"
                            "-Scroll bars are now toggleable with ALT-s for show and ALT-w for hide\n"
                            "\n"
                            "New in alpha 4.0.5:\n"
                            "-New indent rule\n"
                            "-app->buffer_compute_cursor in the customization API\n"
                            "-f keys are available\n"
                            "\n"
                            "New in alpha 4.0.3 and 4.0.4:\n"
                            "-Scroll bar on files and file lists\n"
                            "-Arrow navigation in lists\n"
                            "-A new minimal theme editor\n"
                            "\n"
                            "New in alpha 4.0.2:\n"
                            "-The file count limit is over 8 million now\n"
                            "-File equality is handled better so renamings (such as 'subst') are safe now\n"
                            "-This buffer will report events including errors that happen in 4coder\n"
                            "-Super users can post their own messages here with app->print_message\n"
                            "-<ctrl e> centers view on cursor; cmdid_center_view in customization API\n"
                            "-Set font size on command line with -f N, N = 16 by default\n\n"
                            );
        
        do_feedback_message(system, models, welcome);
    }
    
    // NOTE(allen): panel resizing
    switch (vars->state){
        case APP_STATE_EDIT:
        {
            if (input->mouse.press_l && mouse_on_divider){
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
            if (input->mouse.l){
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
    
    if (mouse_in_edit_area && mouse_panel != 0 && input->mouse.press_l){
        models->layout.active_panel = (i32)(mouse_panel - models->layout.panels);
    }
    
    update_command_data(vars, cmd);
    
    end_temp_memory(param_stack_temp);
    
    // NOTE(allen): send resize messages to panels that have changed size
    {
        Panel *panel = 0, *used_panels = 0;
        
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            View *view = panel->view;
            i32_Rect prev = view->file_region_prev;
            i32_Rect region = view->file_region;
            if (!rect_equal(prev, region)){
                remeasure_file_view(system, panel->view);
            }
            view->file_region_prev = region;
        }
    }
    
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
            remeasure_file_view(system, panel->view);
        }
    }
    
    // NOTE(allen): post scroll vars back to the view's gui targets
    {
        Panel *panel = 0, *used_panels = 0;
        
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            Assert(panel->view);
            view_end_cursor_scroll_updates(panel->view);
        }
    }
    
    // NOTE(allen): rendering
    {
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
            Style *style = main_style(models);

            b32 active = (panel == cmd->panel);
            u32 back_color = style->main.back_color;
            draw_rectangle(target, full, back_color);

            draw_push_clip(target, panel->inner);
            do_render_file_view(system, view, cmd->view,
                                panel->inner, active, target, &dead_input);
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
    
    app_result.lctrl_lalt_is_altgr = models->settings.lctrl_lalt_is_altgr;
    app_result.perform_kill = !models->keep_playing;
    
    *result = app_result;
    
    Assert(general_memory_check(&models->mem.general));
    
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

