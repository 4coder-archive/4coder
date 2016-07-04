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
    f32 min, max;
};

struct CLI_Process{
    CLI_Handles cli;
    Editing_File *out_file;
    b32 cursor_at_end;
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
};

enum Input_Types{
    Input_AnyKey,
    Input_Esc,
    Input_MouseMove,
    Input_MouseLeftButton,
    Input_MouseRightButton,
    Input_MouseWheel,
    Input_Count
};

struct Consumption_Record{
    b32 consumed;
    char consumer[32];
};

struct Available_Input{
    Key_Summary *keys;
    Mouse_State *mouse;
    Consumption_Record records[Input_Count];
};

Available_Input
init_available_input(Key_Summary *keys, Mouse_State *mouse){
    Available_Input result = {0};
    result.keys = keys;
    result.mouse = mouse;
    return(result);
}

Key_Summary
direct_get_key_data(Available_Input *available){
    Key_Summary result = *available->keys;
    return(result);
}

Mouse_State
direct_get_mouse_state(Available_Input *available){
    Mouse_State result = *available->mouse;
    return(result);
}

Key_Summary
get_key_data(Available_Input *available){
    Key_Summary result = {0};
    
    if (!available->records[Input_AnyKey].consumed){
        result = *available->keys;
    }
    else if (!available->records[Input_Esc].consumed){
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
    if (available->records[Input_MouseLeftButton].consumed){
        mouse.l = 0;
        mouse.press_l = 0;
        mouse.release_l = 0;
    }
    
    if (available->records[Input_MouseRightButton].consumed){
        mouse.r = 0;
        mouse.press_r = 0;
        mouse.release_r = 0;
    }
    
    if (available->records[Input_MouseWheel].consumed){
        mouse.wheel = 0;
    }
    
    return(mouse);
}

void
consume_input(Available_Input *available, i32 input_type, char *consumer){
    Consumption_Record *record = &available->records[input_type];
    record->consumed = 1;
    if (consumer){
        String str = make_fixed_width_string(record->consumer);
        copy(&str, consumer);
        terminate_with_null(&str);
    }
    else{
        record->consumer[0] = 0;
    }
}

struct App_Vars{
    Models models;
    // TODO(allen): This wants to live in
    // models with everyone else but the order
    // of declaration is a little bit off...
    Live_Views live_set;
    
    CLI_List cli_processes;
    
    App_State state;
    App_State_Resizing resizing;
    Complete_State complete_state;
    
    Command_Data command_data;
    
    Available_Input available_input;
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
    result = system->launch_coroutine(co, in, out);
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
    result = system->resume_coroutine(co, in, out);
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
        output_file_append(system, models, file, value, true);
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
#define REQ_READABLE_VIEW(n) View *n = command->panel->view; if (view_lock_level(n) > LockLevel_Protected) return

#define REQ_FILE(n,v) Editing_File *n = (v)->file_data.file; if (!n) return
#define REQ_FILE_HISTORY(n,v) Editing_File *n = (v)->file_data.file; if (!n || !n->state.undo.undo.edits) return

#define COMMAND_DECL(n) internal void command_##n(System_Functions *system, Command_Data *command, Command_Binding binding)

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

COMMAND_DECL(center_view){
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    
    Assert(view->edit_pos);
    
    f32 h = view_file_height(view);
    f32 y = view->edit_pos->cursor.wrapped_y;
    if (view->file_data.unwrapped_lines){
        y = view->edit_pos->cursor.unwrapped_y;
    }
    
    y = clamp_bottom(0.f, y - h*.5f);
    view->edit_pos->scroll.target_y = ROUND32(y);
}

COMMAND_DECL(left_adjust_view){
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    
    Assert(view->edit_pos);
    
    f32 x = view->edit_pos->cursor.wrapped_x;
    if (view->file_data.unwrapped_lines){
        x = view->edit_pos->cursor.unwrapped_x;
    }
    
    x = clamp_bottom(0.f, x - 30.f);
    view->edit_pos->scroll.target_x = ROUND32(x);
}

COMMAND_DECL(word_complete){
    USE_MODELS(models);
    USE_VARS(vars);
    REQ_OPEN_VIEW(view);
    REQ_FILE(file, view);
    
    Assert(view->edit_pos);
    
    Partition *part = &models->mem.part;
    General_Memory *general = &models->mem.general;
    Working_Set *working_set = &models->working_set;
    Complete_State *complete_state = &vars->complete_state;
    Search_Range *ranges = 0;
    
    Buffer_Type *buffer = &file->state.buffer;
    i32 size_of_buffer = buffer_size(buffer);
    
    i32 cursor_pos = 0;
    i32 word_start = 0;
    i32 word_end = 0;
    char c = 0;
    
    char *spare = 0;
    i32 size = 0;
    
    b32 do_init = false;
    if (view->mode.rewrite != 2){
        do_init = true;
    }
    view->next_mode.rewrite = 2;
    if (complete_state->initialized == 0){
        do_init = true;
    }
    
    if (do_init){
        word_end = view->edit_pos->cursor.pos;
        word_start = word_end;
        cursor_pos = word_end - 1;
        
        // TODO(allen): macros for these buffer loops and some method
        // of breaking out of them.
        for (Buffer_Backify_Type loop = buffer_backify_loop(buffer, cursor_pos, 0);
             buffer_backify_good(&loop);
             buffer_backify_next(&loop)){
            i32 end = loop.absolute_pos;
            char *data = loop.data - loop.absolute_pos;
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
            i32 match_size = 0;
            Search_Match match =
                search_next_match(part, &complete_state->set, &complete_state->iter);
            
            if (match.found_match){
                Temp_Memory temp = begin_temp_memory(part);
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

// TODO(allen): FIX THIS SHIT!
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
    USE_VIEW(view);
    
    view_show_interactive(system, view,
                          IAct_New, IInt_Sys_File_List,
                          make_lit_string("New: "));
}

COMMAND_DECL(interactive_open){
    USE_VIEW(view);
    
    view_show_interactive(system, view,
                          IAct_Open, IInt_Sys_File_List,
                          make_lit_string("Open: "));
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
    REQ_FILE(file, view);
    
    if (!file->is_dummy && file_is_ready(file)){
        String name = file->name.source_path;
        view_save_file(system, models, file, 0, name, 0);
    }
}

COMMAND_DECL(save_as){
    USE_VIEW(view);
    REQ_FILE(file, view);
    
    view_show_interactive(system, view,
                          IAct_Save_As, IInt_Sys_File_List,
                          make_lit_string("Save As: "));
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
    
    view_show_interactive(system, view,
                          IAct_Switch, IInt_Live_File_List,
                          make_lit_string("Switch Buffer: "));
}

COMMAND_DECL(interactive_kill_buffer){
    USE_VIEW(view);
    
    view_show_interactive(system, view,
                          IAct_Kill, IInt_Live_File_List,
                          make_lit_string("Kill Buffer: "));
}

COMMAND_DECL(kill_buffer){
    USE_MODELS(models);
    USE_VIEW(view);
    REQ_FILE(file, view);
    
    try_kill_file(system, models, file, view, string_zero());
}

COMMAND_DECL(toggle_line_wrap){
    REQ_READABLE_VIEW(view);
    REQ_FILE(file, view);
    
    Assert(view->edit_pos);
    
    // TODO(allen): WHAT TO DO HERE???
    Relative_Scrolling scrolling = view_get_relative_scrolling(view);
    if (view->file_data.unwrapped_lines){
        view->file_data.unwrapped_lines = 0;
        file->settings.unwrapped_lines = 0;
        view->edit_pos->scroll.target_x = 0;
        view_cursor_move(view, view->edit_pos->cursor.pos);
    }
    else{
        view->file_data.unwrapped_lines = 1;
        file->settings.unwrapped_lines = 1;
        view_cursor_move(view, view->edit_pos->cursor.pos);
    }
    view_set_relative_scrolling(view, scrolling);
}

#if 0
COMMAND_DECL(toggle_show_whitespace){
    REQ_READABLE_VIEW(view);
    
    view->file_data.show_whitespace = !view->file_data.show_whitespace;
}
#endif

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
    Range range = make_range(view->edit_pos->cursor.pos, view->edit_pos->mark);
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

#if 0
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
#endif

COMMAND_DECL(open_panel_vsplit){
    USE_VARS(vars);
    USE_MODELS(models);
    USE_PANEL(panel);
    
    if (models->layout.panel_count < models->layout.panel_max_count){
        Split_Result split = layout_split_panel(&models->layout, panel, 1);
        
        Panel *panel1 = panel;
        Panel *panel2 = split.panel;
        
        panel2->screen_region = panel1->screen_region;
        
        i32 x_pos = ROUND32(lerp((f32)panel1->full.x0,
                                 split.divider->pos,
                                 (f32)panel1->full.x1)
                            );
        
        panel2->full.x0 = x_pos;
        panel2->full.x1 = panel1->full.x1;
        panel1->full.x1 = x_pos;
        
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
        
        i32 y_pos = ROUND32(lerp((f32)panel1->full.y0,
                                 split.divider->pos,
                                 (f32)panel1->full.y1)
                            );
        
        panel2->full.y0 = y_pos;
        panel2->full.y1 = panel1->full.y1;
        panel1->full.y1 = y_pos;
        
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

COMMAND_DECL(page_down){
    REQ_READABLE_VIEW(view);
    
    Assert(view->edit_pos);
    
    i32 height = CEIL32(view_file_height(view));
    f32 y = view_get_cursor_y(view);
    f32 x = view->edit_pos->preferred_x;
    
    Full_Cursor cursor = view_compute_cursor_from_xy(view, x, y+height);
    view_set_cursor(view, cursor, false, view->file_data.unwrapped_lines);
}

COMMAND_DECL(page_up){
    REQ_READABLE_VIEW(view);
    
    Assert(view->edit_pos);
    
    i32 height = CEIL32(view_file_height(view));
    f32 y = view_get_cursor_y(view);
    f32 x = view->edit_pos->preferred_x;
    
    Full_Cursor cursor = view_compute_cursor_from_xy(view, x, y-height);
    view_set_cursor(view, cursor, false, view->file_data.unwrapped_lines);
}

COMMAND_DECL(open_color_tweaker){
    USE_VIEW(view);
    view_show_theme(view);
}

COMMAND_DECL(open_config){
    USE_VIEW(view);
    view_show_GUI(view, VUI_Config);
}

COMMAND_DECL(open_menu){
    USE_VIEW(view);
    view_show_GUI(view, VUI_Menu);
}

COMMAND_DECL(open_debug){
    USE_VIEW(view);
    view_show_GUI(view, VUI_Debug);
    view->debug_vars = debug_vars_zero();
}

COMMAND_DECL(user_callback){
    USE_MODELS(models);
    if (binding.custom) binding.custom(&models->app_links);
}

internal void
update_command_data(App_Vars *vars, Command_Data *cmd){
    cmd->panel = cmd->models->layout.panels + cmd->models->layout.active_panel;
    cmd->view = cmd->panel->view;
}

globalvar Command_Function command_table[cmdid_count];

#include "4ed_api_implementation.cpp"

struct Command_In{
    Command_Data *cmd;
    Command_Binding bind;
};

internal void
command_caller(Coroutine *coroutine){
    Command_In *cmd_in = (Command_In*)coroutine->in;
    Command_Data *cmd = cmd_in->cmd;
    Models *models = cmd->models;
    View *view = cmd->view;
    
    view->next_mode = view_mode_zero();
    if (models->command_caller){
        Generic_Command generic;
        if (cmd_in->bind.function == command_user_callback){
            generic.command = cmd_in->bind.custom;
            models->command_caller(&models->app_links, generic);
        }
        else{
            generic.cmdid = (Command_ID)cmd_in->bind.custom_id;
            models->command_caller(&models->app_links, generic);
        }
    }
    else{
        cmd_in->bind.function(cmd->system, cmd, cmd_in->bind);
    }
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
    
    FillAppLinksAPI(app_links);
    
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
    
    SET(center_view);
    SET(left_adjust_view);
    
    SET(word_complete);
    
    SET(undo);
    SET(redo);
    SET(history_backward);
    SET(history_forward);
    
    SET(interactive_new);
    SET(interactive_open);
    SET(reopen);
    SET(save);
    SET(save_as);
    SET(interactive_switch_buffer);
    SET(interactive_kill_buffer);
    SET(kill_buffer);
        
    SET(to_uppercase);
    SET(to_lowercase);
    
    SET(clean_all_lines);
    
    SET(open_panel_vsplit);
    SET(open_panel_hsplit);
    SET(close_panel);
    SET(change_active_panel);
    
    SET(open_color_tweaker);
    SET(open_config);
    SET(open_menu);
    SET(open_debug);
    
#undef SET
}

// App Functions

internal void
app_hardcode_styles(Models *models){
    Interactive_Style file_info_style = {0};
    Style *styles = models->styles.styles;
    Style *style = styles + 1;
    
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
    file_info_style.pop1_color = 0xFF3C57DC;
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
    
    /////////////////
    style_set_name(style, make_lit_string("Hjortshoej"));
    
    style->main.back_color = 0xFFF0F0F0;
    style->main.margin_color = 0xFF9E9E9E;
    style->main.margin_hover_color = 0xFF7E7E7E;
    style->main.margin_active_color = 0xFF5C5C5C;
    style->main.cursor_color = 0xFF000000;
    style->main.at_cursor_color = 0xFFD6D6D6;
    style->main.mark_color = 0xFF525252;
    style->main.highlight_color = 0xFFB87600;
    style->main.at_highlight_color = 0xFF000000;
    style->main.default_color = 0xFF000000;
    style->main.comment_color = 0xFF007E00;
    style->main.keyword_color = 0xFF8B4303;
    style->main.str_constant_color = 0xFF7C0000;
    style->main.char_constant_color = 0xFF7C0000;
    style->main.include_color = 0xFF7C0000;
    style->main.int_constant_color = 0xFF007C00;
    style->main.float_constant_color = 0xFF007C00;
    style->main.bool_constant_color = 0xFF007C00;
    style->main.preproc_color = 0xFF0000FF;
    style->main.special_character_color = 0xFF9A0000;
    
    style->main.paste_color = 0xFFB87600;
    style->main.undo_color = 0xFFB87600;
    
    style->main.highlight_junk_color = 0xFFFF7878;
    style->main.highlight_white_color = 0xFFB87600;
    
    file_info_style.bar_color = 0xFF606060;
    file_info_style.bar_active_color = 0xFF3E3E3E;
    file_info_style.base_color = 0xFFFFFFFF;
    file_info_style.pop1_color = 0xFF007E00;
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
    CLAct_FontStopHinting,
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
                        case 'u': action = CLAct_UserFile; strict = false; break;
                        case 'U': action = CLAct_UserFile; strict = true;  break;
                        
                        case 'd': action = CLAct_CustomDLL; strict = false;break;
                        case 'D': action = CLAct_CustomDLL; strict = true; break;
                        
                        case 'i': action = CLAct_InitialFilePosition;      break;
                        
                        case 'w': action = CLAct_WindowSize;               break;
                        case 'W': action = CLAct_WindowMaximize;           break;
                        case 'p': action = CLAct_WindowPosition;           break;
                        
                        case 'f': action = CLAct_FontSize;                 break;
                        case 'h': action = CLAct_FontStopHinting; --i;     break;
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
                    plat_settings->set_window_size  = true;
                    plat_settings->window_w = str_to_int(clparams.argv[i]);
                    plat_settings->window_h = str_to_int(clparams.argv[i+1]);
                    
                    ++i;
                }
                action = CLAct_Nothing;
            }break;
            
            case CLAct_WindowMaximize:
            {
                --i;
                plat_settings->maximize_window = true;
                action = CLAct_Nothing;
            }break;
            
            case CLAct_WindowPosition:
            {
                if (i + 1 < clparams.argc){
                    plat_settings->set_window_pos  = true;
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
            
            case CLAct_FontStopHinting:
            {
                plat_settings->use_hinting = true;
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

inline App_Settings
app_settings_zero(){
    App_Settings settings={0};
    return(settings);
}

App_Read_Command_Line_Sig(app_read_command_line){
    i32 out_size = 0;
    App_Vars *vars = app_setup_memory(memory);
    App_Settings *settings = &vars->models.settings;
    
    *settings = app_settings_zero();
    settings->font_size = 16;
    
    if (clparams.argc > 1){
        init_command_line_settings(&vars->models.settings, plat_settings, clparams);
    }
    
    *files = vars->models.settings.init_files;
    *file_count = &vars->models.settings.init_files_count;
    
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
        
        models->live_set = &vars->live_set;
        
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
        models->hook_open_file = 0;
        models->hook_new_file = 0;
        
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
                                    switch (hook_id){
                                        case _hook_open_file:
                                        models->hook_open_file = (Open_File_Hook_Function*)unit->hook.func;
                                        break;
                                        
                                        case _hook_new_file:
                                        models->hook_new_file = (Open_File_Hook_Function*)unit->hook.func;
                                        break;
                                        
                                        case _hook_command_caller:
                                        models->command_caller = (Command_Caller_Hook_Function*)unit->hook.func;
                                        break;
                                        
                                        case _hook_scroll_rule:
                                        models->scroll_rule = (Scroll_Rule_Function*)unit->hook.func;
                                        break;
                                        
                                        case _hook_input_filter:
                                        models->input_filter = (Input_Filter_Function*)unit->hook.func;
                                        break;
                                    }
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
                literal("Liberation Sans"),
                font_size},
            
            {literal("liberation-mono.ttf"),
                literal("Liberation Mono"),
                font_size},
            
            {literal("Hack-Regular.ttf"),
                literal("Hack"),
                font_size},
            
            {literal("CutiveMono-Regular.ttf"),
                literal("Cutive Mono"),
                font_size},
            
            {literal("Inconsolata-Regular.ttf"),
                literal("Inconsolata"),
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
}

internal i32
update_cli_handle_with_file(System_Functions *system, Models *models,
                            CLI_Handles *cli, Editing_File *file, char *dest, i32 max, b32 cursor_at_end){
    i32 result = 0;
    u32 amount = 0;
    
    system->cli_begin_update(cli);
    if (system->cli_update_step(cli, dest, max, &amount)){
        amount = eol_in_place_convert_in(dest, amount);
        output_file_append(system, models, file, make_string(dest, amount), cursor_at_end);
        result = 1;
    }
    
    if (system->cli_end_update(cli)){
        char str_space[256];
        String str = make_fixed_width_string(str_space);
        append(&str, "exited with code ");
        append_int_to_str(&str, cli->exit);
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

App_Step_Sig(app_step){
    Application_Step_Result app_result = *result;
    app_result.animating = 0;
    
    App_Vars *vars = (App_Vars*)memory->vars_memory;
    Models *models = &vars->models;
    target->partition = &models->mem.part;
    
    // NOTE(allen): OS clipboard event handling
    String clipboard = input->clipboard;
    
    if (clipboard.str){
        String *dest =
            working_set_next_clipboard_string(&models->mem.general,
                                              &models->working_set,
                                              clipboard.size);
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
            
            time_stamp =
                system->file_time_stamp(make_c_str(file->name.source_path));
            
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
    
    {
        for (i32 i = 0; i < input->keys.press_count; ++i){
            key_summary.keys[key_summary.count++] = input->keys.press[i];
        }
        for (i32 i = 0; i < input->keys.hold_count; ++i){
            key_summary.keys[key_summary.count++] = input->keys.hold[i];
        }
        
        if (models->input_filter){
            models->input_filter(&input->mouse);
        }
        
        Key_Event_Data mouse_event = {0};
        if (input->mouse.press_l ||
            input->mouse.press_r){
            memcpy(mouse_event.modifiers, input->keys.modifiers, sizeof(input->keys.modifiers));
        }
        
        if (input->mouse.press_l){
            mouse_event.keycode = key_mouse_left;
            key_summary.keys[key_summary.count++] = mouse_event;
        }
        
        if (input->mouse.press_r){
            mouse_event.keycode = key_mouse_right;
            key_summary.keys[key_summary.count++] = mouse_event;
        }
        
        input->mouse.wheel = -input->mouse.wheel;
    }
    
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
                Divider_And_ID div =layout_get_divider(&models->layout, mouse_divider_id);
                
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
        u32 max = Kbytes(128);
        char *dest = push_array(&models->mem.part, char, max);
        
        i32 count = vars->cli_processes.count;
        for (i32 i = 0; i < count; ++i){
            CLI_Process *proc = vars->cli_processes.procs + i;
            Editing_File *file = proc->out_file;
            
            if (file != 0){
                i32 r = update_cli_handle_with_file(
                    system, models, &proc->cli, file, dest, max, proc->cursor_at_end);
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
        
        {
            Editing_File *file = working_set_alloc_always(&models->working_set, general);
            file_create_read_only(system, models, file, "*messages*");
            working_set_add(system, &models->working_set, file, general);
            file->settings.never_kill = 1;
            file->settings.unimportant = 1;
            file->settings.unwrapped_lines = 1;
            
            models->message_buffer = file;
        }
        
        {
            Editing_File *file = working_set_alloc_always(&models->working_set, general);
            file_create_empty(system, models, file, "*scratch*");
            working_set_add(system, &models->working_set, file, general);
            file->settings.never_kill = 1;
            file->settings.unimportant = 1;
            file->settings.unwrapped_lines = 1;
            
            models->scratch_buffer = file;
        }
        
        if (models->hooks[hook_start]){
            models->hooks[hook_start](&models->app_links);
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
                        view_move_view_to_cursor(panel->view, &panel->view->recent.scroll);
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
            
            ++i;
            panel = panel->next;
        }
        
        for (;i < models->layout.panel_count; ++i, panel = panel->next){
            view_set_file(panel->view, models->scratch_buffer, models);
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
            
            for (i32 i = 0; i < 128 && command_coroutine; ++i){
                User_Input user_in = {0};
                user_in.abort = 1;
                
                command_coroutine =
                    app_resume_coroutine(system, &models->app_links, Co_Command,
                                         command_coroutine, &user_in,
                                         models->command_coroutine_flags);
            }
            if (command_coroutine != 0){
                // TODO(allen): post grave warning
                command_coroutine = 0;
            }
            if (view != 0){
                init_query_set(&view->query_set);
            }
            
            if (view == 0){
                Panel *panel = models->layout.used_sentinel.next;
                view = panel->view;
            }
            
            view_show_interactive(system, view,
                                  IAct_Sure_To_Close, IInt_Sure_To_Close,
                                  make_lit_string("Are you sure?"));
            
            models->command_coroutine = command_coroutine;
        }
        else{
            models->keep_playing = 0;
        }
    }
    
    // NOTE(allen): pass events to debug
    vars->available_input = init_available_input(&key_summary, &input->mouse);
    
#if FRED_INTERNAL
    {
        Debug_Data *debug = &models->debug;
        Key_Summary key_data = get_key_data(&vars->available_input);
        
        Debug_Input_Event *events = debug->input_events;
        
        i32 count = key_data.count;
        i32 preserved_inputs = ArrayCount(debug->input_events) - count;
        
        debug->this_frame_count = count;
        memmove(events + count, events,
                sizeof(Debug_Input_Event)*preserved_inputs);
        
        for (i32 i = 0; i < key_data.count; ++i){
            Key_Event_Data key = get_single_key(&key_data,  i);
            events[i].key = key.keycode;
            
            events[i].consumer[0] = 0;
            
            events[i].is_hold = key.modifiers[MDFR_HOLD_INDEX];
            events[i].is_ctrl = key.modifiers[MDFR_CONTROL_INDEX];
            events[i].is_alt = key.modifiers[MDFR_ALT_INDEX];
            events[i].is_shift = key.modifiers[MDFR_SHIFT_INDEX];
        }
    }
#endif
    
    // NOTE(allen): Keyboard input to command coroutine.
    
    if (models->command_coroutine != 0){
        Coroutine *command_coroutine = models->command_coroutine;
        u32 get_flags = models->command_coroutine_flags[0];
        u32 abort_flags = models->command_coroutine_flags[1];
        
        get_flags |= abort_flags;
        
        if ((get_flags & EventOnAnyKey) || (get_flags & EventOnEsc)){
            Key_Summary key_data = get_key_data(&vars->available_input);
            
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
                user_in.command.command = cmd_bind.custom;
                user_in.abort = 0;
                
                if ((EventOnEsc & abort_flags) && key.keycode == key_esc){
                    user_in.abort = 1;
                }
                else if (EventOnAnyKey & abort_flags){
                    user_in.abort = 1;
                }
                
                if (EventOnAnyKey & get_flags){
                    pass_in = 1;
                    consume_input(&vars->available_input, Input_AnyKey,
                                  "command coroutine");
                }
                if (key.keycode == key_esc){
                    if (EventOnEsc & get_flags){
                        pass_in = 1;
                    }
                    consume_input(&vars->available_input, Input_Esc,
                                  "command coroutine");
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
            user_in.command.cmdid = 0;
            user_in.abort = 0;
            
            if (abort_flags & EventOnMouseMove){
                user_in.abort = 1;
            }
            if (get_flags & EventOnMouseMove){
                pass_in = 1;
                consume_input(&vars->available_input, Input_MouseMove,
                              "command coroutine");
            }
            
            if (input->mouse.press_l || input->mouse.release_l || input->mouse.l){
                if (abort_flags & EventOnLeftButton){
                    user_in.abort = 1;
                }
                if (get_flags & EventOnLeftButton){
                    pass_in = 1;
                    consume_input(&vars->available_input, Input_MouseLeftButton,
                                  "command coroutine");
                }
            }
            
            if (input->mouse.press_r || input->mouse.release_r || input->mouse.r){
                if (abort_flags & EventOnRightButton){
                    user_in.abort = 1;
                }
                if (get_flags & EventOnRightButton){
                    pass_in = 1;
                    consume_input(&vars->available_input, Input_MouseRightButton,
                                  "command coroutine");
                }
            }
            
            if (input->mouse.wheel != 0){
                if (abort_flags & EventOnWheel){
                    user_in.abort = 1;
                }
                if (get_flags & EventOnWheel){
                    pass_in = 1;
                    consume_input(&vars->available_input, Input_MouseWheel,
                                  "command coroutine");
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
    dead_input.dt = input->dt;
    
    Input_Summary active_input = {};
    active_input.mouse.x = input->mouse.x;
    active_input.mouse.y = input->mouse.y;
    active_input.dt = input->dt;
    
    active_input.keys = get_key_data(&vars->available_input);
    
    Mouse_State mouse_state = get_mouse_state(&vars->available_input);
    
    {
        Panel *panel = 0, *used_panels = 0;
        View *view = 0, *active_view = 0;
        b32 active = 0;
        Input_Summary summary = {0};
        
        active_view = cmd->panel->view;
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            view = panel->view;
            active = (panel == cmd->panel);
            summary = (active)?(active_input):(dead_input);
            
            view->changed_context_in_step = 0;
            
            View_Step_Result result = step_file_view(system, view, active_view, summary);
            if (result.animating){
                app_result.animating = 1;
            }
            if (result.consume_keys){
                consume_input(&vars->available_input, Input_AnyKey,
                              "file view step");
            }
            if (result.consume_keys || result.consume_esc){
                consume_input(&vars->available_input, Input_Esc,
                              "file view step");
            }
            
            if (view->changed_context_in_step == 0){
                active = (panel == cmd->panel);
                summary = (active)?(active_input):(dead_input);
                if (panel == mouse_panel && !input->mouse.out_of_window){
                    summary.mouse = mouse_state;
                }
                
                b32 file_scroll = false;
                GUI_Scroll_Vars scroll_zero = {0};
                GUI_Scroll_Vars *scroll_vars = &view->gui_scroll;
                if (view->showing_ui == VUI_None){
                    if (view->file_data.file){
                        scroll_vars = &view->edit_pos->scroll;
                        file_scroll = true;
                    }
                    else{
                        scroll_vars = &scroll_zero;
                    }
                }
                
                Input_Process_Result ip_result =
                    do_step_file_view(system, view, panel->inner, active,
                                      &summary, *scroll_vars, view->scroll_region);
                if (ip_result.is_animating){
                    app_result.animating = 1;
                }
                if (ip_result.consumed_l){
                    consume_input(&vars->available_input, Input_MouseLeftButton,
                                  "file view step");
                }
                if (ip_result.consumed_r){
                    consume_input(&vars->available_input, Input_MouseRightButton,
                                  "file view step");
                }
                
                if (!gui_scroll_eq(scroll_vars, &ip_result.vars)){
                    if (file_scroll){
                        view_set_scroll(view, ip_result.vars);
                    }
                    else{
                        *scroll_vars = ip_result.vars;
                    }
                }
                
                view->scroll_region = ip_result.region;
            }
        }
    }
    
    update_command_data(vars, cmd);
    
    // NOTE(allen): post scroll vars back to the view's gui targets
    {
        Panel *panel = 0, *used_panels = 0;
        
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            Assert(panel->view);
            view_end_cursor_scroll_updates(panel->view);
        }
    }
    
    // NOTE(allen): command execution
    {
        Key_Summary key_data = get_key_data(&vars->available_input);
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
            consume_input(&vars->available_input, Input_AnyKey,
                          "command dispatcher");
        }
        if (hit_esc){
            consume_input(&vars->available_input, Input_Esc,
                          "command dispatcher");
        }
    }
    
    update_command_data(vars, cmd);
    
    // NOTE(allen): pass consumption data to debug
    {
        Debug_Data *debug = &models->debug;
        i32 count = debug->this_frame_count;
        
        Consumption_Record *record = 0;
        
        record = &vars->available_input.records[Input_MouseLeftButton];
        if (record->consumed && record->consumer[0] != 0){
            Debug_Input_Event *event = debug->input_events;
            for (i32 i = 0; i < count; ++i, ++event){
                if (event->key == key_mouse_left &&
                    event->consumer[0] == 0){
                    memcpy(event->consumer, record->consumer, sizeof(record->consumer));
                }
            }
        }
        
        record = &vars->available_input.records[Input_MouseRightButton];
        if (record->consumed && record->consumer[0] != 0){
            Debug_Input_Event *event = debug->input_events;
            for (i32 i = 0; i < count; ++i, ++event){
                if (event->key == key_mouse_right &&
                    event->consumer[0] == 0){
                    memcpy(event->consumer, record->consumer, sizeof(record->consumer));
                }
            }
        }
        
        record = &vars->available_input.records[Input_Esc];
        if (record->consumed && record->consumer[0] != 0){
            Debug_Input_Event *event = debug->input_events;
            for (i32 i = 0; i < count; ++i, ++event){
                if (event->key == key_esc &&
                    event->consumer[0] == 0){
                    memcpy(event->consumer, record->consumer, sizeof(record->consumer));
                }
            }
        }
        
        record = &vars->available_input.records[Input_AnyKey];
        if (record->consumed){
            Debug_Input_Event *event = debug->input_events;
            for (i32 i = 0; i < count; ++i, ++event){
                if (event->consumer[0] == 0){
                    memcpy(event->consumer, record->consumer, sizeof(record->consumer));
                }
            }
        }
    }
    
    // NOTE(allen): initialize message
    if (input->first_step){
        String welcome =
            make_lit_string("Welcome to " VERSION "\n"
                            "If you're new to 4coder there's no tutorial yet :(\n"
                            "you can use the key combo <ctrl o> to look for a file\n"
                            "and if you load README.txt you'll find all the key combos there are.\n"
                            "\n"
                            "Newest features:\n"
                            "-A scratch buffer is now opened with 4coder automatically\n"
                            "-A new mouse suppression mode toggled by <F2>\n"
                            "-Hinting is disabled by default, a -h flag on the command line enables it\n"
                            "-New 4coder_API.html documentation file included for the custom layer API\n"
                            "-Experimental new work-flow for building and jumping to errors\n"
                            "  (only available in power for this build)\n"
                            "\n"
                            "New in alpha 4.0.8:\n"
                            "-Eliminated the parameter stack\n"
                            "\n"
                            "New in alpha 4.0.7:\n"
                            "-Right click sets the mark\n"
                            "-Clicks now have key codes so they can have events bound in customizations\n"
                            "-<alt d> opens a debug view, see more in README.txt\n"
                            "\n"
                            "New in alpha 4.0.6:\n"
                            "-Tied the view scrolling and the list arrow navigation together\n"
                            "-Scroll bars are now toggleable with <alt s> for show and <alt w> for hide\n"
                            "\n"
                            "New in alpha 4.0.5:\n"
                            "-New indent rule\n"
                            "-app->buffer_compute_cursor in the customization API\n"
                            "-f keys are available in the customization system now\n"
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
                
                f32 min = 0;
                f32 max = 0;
                {
                    f32 mid = layout_get_position(&models->layout, mouse_divider_id);
                    if (mouse_divider_vertical){
                        max = (f32)models->layout.full_width;
                    }
                    else{
                        max = (f32)models->layout.full_height;
                    }
                    
                    i32 divider_id = div.id;
                    do{
                        Divider_And_ID other_div = layout_get_divider(&models->layout, divider_id);
                        b32 divider_match = (other_div.divider->v_divider == mouse_divider_vertical);
                        f32 pos = layout_get_position(&models->layout, divider_id);
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
                        --top;
                        Divider_And_ID other_div = layout_get_divider(&models->layout, divider_stack[top]);
                        b32 divider_match = (other_div.divider->v_divider == mouse_divider_vertical);
                        f32 pos = layout_get_position(&models->layout, divider_stack[top]);
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
                
                vars->resizing.min = 0.f;
                vars->resizing.max = 1.f;
            }
        }break;
        
        case APP_STATE_RESIZING:
        {
            if (input->mouse.l){
                Panel_Divider *divider = vars->resizing.divider;
                i32 pos = 0;
                if (divider->v_divider){
                    pos = clamp(0, mx, models->layout.full_width);
                }
                else{
                    pos = clamp(0, my, models->layout.full_height);
                }
                divider->pos = layout_compute_position(&models->layout, divider, pos);
                
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
            update_view_line_height(models, panel->view);
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
            
            b32 file_scroll = false;
            GUI_Scroll_Vars scroll_zero = {0};
            GUI_Scroll_Vars *scroll_vars = &view->gui_scroll;
            if (view->showing_ui == VUI_None){
                if (view->file_data.file){
                    scroll_vars = &view->edit_pos->scroll;
                    file_scroll = true;
                }
                else{
                    scroll_vars = &scroll_zero;
                }
            }
            
            do_render_file_view(system, view, scroll_vars, cmd->view, 
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

