/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application layer for project codename "4ed"
 *
 */

// TOP

// Messages
global_const char messages[] =
"----------------------------------------------------------------\n"
"Welcome to " VERSION "\n"
"If you're new to 4coder there are some tutorials at http://4coder.net/tutorials.html\n"
"\n"
"Newest features:\n"
"-Color schemes are now loaded in theme files from the \"themes\" folder\n"
"-After loading a project <alt h> sets the hot directory to the project directory\n"
"-The flag -L enables a logging system that will collect information in case more information is needed while debugging a problem\n"
"-All command line flags after the special flag --custom are now passed to the custom API start hook\n"
"-The start hook now gets the list of file names that were specified on the command line\n"
"  All of the files specified on the command line are loaded before the start hook runs\n"
"-It is now possible to set the hot directory from the custom API\n"
"-On windows the buildsuper scripts are improved to look for vcvarsall.bat in lots of common locations\n"
"\n"
"New in alpha 4.0.20:\n"
"-Option for LAlt + LCtrl = AltGr on Windows is now in config.4coder\n"
"-The 4cpp lexer now has a customizable keyword table, *experimental* expansion of language support to:\n"
"  Rust, C#, Java\n"
"  Arbitrary keyword customization available in custom code (super users)\n"
"\n"
"New in alpha 4.0.19:\n"
"-Lexer now handles string literal prefixes and is more optimized\n"
"-Fixes for lingering unicode bugs\n"
"-Power users have an experimental new jump to error that positions through edits (coming to all tiers soon)\n"
"\n"
"New in alpha 4.0.18:\n"
"-Support for rendering unicode characters\n"
"-<ctrl t> isearch alpha-numeric word under cursor\n"
"-<ctrl Q> query replace alpha-numeric word under cursor\n"
"-<alt b> toggle file bar\n"
"\n"
"New in alpha 4.0.17:\n"
"-New support for extended ascii input.\n"
"-Extended ascii encoded in buffers as utf8.\n"
"-The custom layer now has a 'markers' API for tracking buffer positions across changes.\n"
"\n"
"New in alpha 4.0.16:\n"
"-<alt 2> If the current file is a C++ code file, this opens the matching header.\n""  If the current file is a C++ header, this opens the matching code file.\n"
"-Option to automatically save changes on build in the config file.\n"
"  This works for builds triggered by <alt m>.\n"
"-Option in project files to have certain fkey commands save changes.\n"
"\n"
"New in alpha 4.0.15:\n"
"-<ctrl I> find all functions in the current buffer and list them in a jump buffer\n"
"-option to set user name in config.4coder\n"
"  The user name is used in <alt t> and <alt y> comment writing commands\n"
"\n"
"New in alpha 4.0.14:\n"
"-Option to have wrap widths automatically adjust based on average view width\n"
"-The 'config.4coder' file can now be placed with the 4ed executable file\n"
"-New options in 'config.4coder' to specify the font and color theme\n"
"-New built in project configuration system\n"
"-New on-save hooks allows custom behavior in the custom layer whenever a file is saved\n"
"-When using code wrapping, any saved file is automatically indented in the text format, this option can be turned off in config.4coder\n"
"\n"
"New in alpha 4.0.12 and 4.0.13:\n"
"-Text files wrap lines at whitespace when possible\n"
"-New code wrapping feature is on by default\n"
"-Introduced a 'config.4coder' for setting several wrapping options:\n"
"  enable_code_wrapping: set to false if you want the text like behavior\n"
"  default_wrap_width: the wrap width to set in new files\n"
"-<ctrl 2> decrease the current buffer's wrap width\n"
"-<ctrl 3> increase the current buffer's wrap width\n"
"-In the customization layer new settings for the buffer are exposed dealing with wrapping\n"
"-In the customization layer there is a call for setting what keys the GUI should use\n"
"\n"
"New in alpha 4.0.11:\n"
"-The commands for going to next error, previous error, etc now work\n"
"  on any buffer with jump locations including *search*\n"
"-4coder now supports proper, borderless, fullscreen with the flag -F\n"
"  and fullscreen can be toggled with <control pageup>.\n"
"  (This sometimes causes artifacts on the Windows task bar)\n"
"-<alt E> to exit\n"
"-hook on exit for the customization system\n"
"-tokens now exposed in customization system\n"
"-mouse release events in customization system\n"
"\n"
"New in alpha 4.0.10:\n"
"-<ctrl F> list all locations of a string across all open buffers\n"
"-Build now finds build.sh and Makefile on Linux\n"
"-<alt n> goes to the next error if the *compilation* buffer is open\n"
"-<alt N> goes to the previous error\n"
"-<alt M> goes to the first error\n"
"-<alt .> switch to the compilation buffer\n"
"-<alt ,> close the panel viewing the compilation buffer\n"
"-New documentation for the 4coder string library included in 4coder_API.html\n"
"-Low level allocation calls available in custom API\n"
"-Each panel can change font independently.\n"
"  Per-buffer fonts are exposed in the custom API.\n"
"\n"
"New in alpha 4.0.9:\n"
"-A scratch buffer is now opened with 4coder automatically\n"
"-A new mouse suppression mode toggled by <F2>\n"
"-Hinting is disabled by default, a -h flag on the command line enables it\n"
"-New 4coder_API.html documentation file provided for the custom layer API\n"
"-Experimental new work-flow for building and jumping to errors\n"
"  This system is only for MSVC in the 'power' version as of 4.0.9\n"
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
"-Set font size on command line with -f N, N = 16 by default\n\n";


// App Structs

#define DEFAULT_DISPLAY_WIDTH 672
#define DEFAULT_MINIMUM_BASE_DISPLAY_WIDTH 550

enum App_State{
    APP_STATE_EDIT,
    APP_STATE_RESIZING,
    // never below this
    APP_STATE_COUNT
};

struct App_State_Resizing{
    Panel_Divider *divider;
};

struct Command_Data{
    Models *models;
    struct App_Vars *vars;
    System_Functions *system;
    Live_Views *live_set;
    
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
    Key_Input_Data *keys;
    Mouse_State *mouse;
    Consumption_Record records[Input_Count];
};

internal Available_Input
init_available_input(Key_Input_Data *keys, Mouse_State *mouse){
    Available_Input result = {0};
    result.keys = keys;
    result.mouse = mouse;
    return(result);
}

internal Key_Input_Data
direct_get_key_data(Available_Input *available){
    Key_Input_Data result = *available->keys;
    return(result);
}

internal Mouse_State
direct_get_mouse_state(Available_Input *available){
    Mouse_State result = *available->mouse;
    return(result);
}

internal Key_Input_Data
get_key_data(Available_Input *available){
    Key_Input_Data result = {0};
    
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

internal Mouse_State
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

internal void
consume_input(Available_Input *available, i32 input_type, char *consumer){
    Consumption_Record *record = &available->records[input_type];
    record->consumed = 1;
    if (consumer){
        String str = make_fixed_width_string(record->consumer);
        copy_sc(&str, consumer);
        terminate_with_null(&str);
    }
    else{
        record->consumer[0] = 0;
    }
}

struct App_Vars{
    Models models;
    
    CLI_List cli_processes;
    
    App_State state;
    App_State_Resizing resizing;
    
    Command_Data command_data;
    
    Available_Input available_input;
};
global_const App_Vars null_app_vars = {0};

enum Coroutine_Type{
    Co_View,
    Co_Command
};
struct App_Coroutine_State{
    void *co;
    i32 type;
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

inline Coroutine_Head*
app_launch_coroutine(System_Functions *system, Application_Links *app, Coroutine_Type type, Coroutine_Head *co, void *in, void *out){
    Coroutine_Head *result = 0;
    
    App_Coroutine_State prev_state = get_state(app);
    
    app->current_coroutine = co;
    app->type_coroutine = type;
    result = system->launch_coroutine(co, in, out);
    restore_state(app, prev_state);
    
    return(result);
}

inline Coroutine_Head*
app_resume_coroutine(System_Functions *system, Application_Links *app, Coroutine_Type type, Coroutine_Head *co, void *in, void *out){
    Coroutine_Head *result = 0;
    
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
    file_replace_range(system, models, file, end, end, value.str, value.size);
}

inline void
do_feedback_message(System_Functions *system, Models *models, String value, b32 set_to_start = 0){
    Editing_File *file = models->message_buffer;
    
    if (file){
        output_file_append(system, models, file, value, true);
        i32 pos = 0;
        if (!set_to_start){
            pos = buffer_size(&file->state.buffer);
        }
        for (View_Iter iter = file_view_iter_init(&models->layout, file, 0);
             file_view_iter_good(iter);
             iter = file_view_iter_next(iter)){
            view_cursor_move(system, iter.view, pos);
        }
    }
}

// Commands

#define USE_MODELS(n) Models *n = command->models
#define USE_VARS(n) App_Vars *n = command->vars
#define USE_FILE(n,v) Editing_File *n = (v)->file_data.file


#define USE_PANEL(n) Panel *n = 0; do{                      \
    i32 panel_index = command->models->layout.active_panel; \
    n = command->models->layout.panels + panel_index;       \
}while(false)

#define USE_VIEW(n) View *n = 0; do{                                  \
    i32 panel_index = command->models->layout.active_panel;           \
    Panel *__panel__ = command->models->layout.panels + panel_index;  \
    n = __panel__->view;                                              \
}while(false)

#define REQ_OPEN_VIEW(n) USE_VIEW(n); if (view_lock_level(n) > LockLevel_Open) return

#define REQ_READABLE_VIEW(n) USE_VIEW(n); if (view_lock_level(n) > LockLevel_Protected) return

#define REQ_FILE(n,v) Editing_File *n = (v)->file_data.file; if (!n) return
#define REQ_FILE_HISTORY(n,v) Editing_File *n = (v)->file_data.file; if (!n || !n->state.undo.undo.edits) return

#define COMMAND_DECL(n) internal void command_##n(System_Functions *system, Command_Data *command, Command_Binding binding)

internal View*
panel_make_empty(System_Functions *system, Models *models, Panel *panel){
    Assert(panel->view == 0);
    View_And_ID new_view = live_set_alloc_view(&models->live_set, panel, models);
    view_set_file(system, new_view.view, models->scratch_buffer, models);
    new_view.view->map = models->scratch_buffer->settings.base_map_id;
    return(new_view.view);
}

COMMAND_DECL(null){
    AllowLocal(command);
}

COMMAND_DECL(undo){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);
    
    view_undo_redo(system, models, view, &file->state.undo.undo, ED_UNDO);
    
    Assert(file->state.undo.undo.size >= 0);
}

COMMAND_DECL(redo){
    USE_MODELS(models);
    REQ_OPEN_VIEW(view);
    REQ_FILE_HISTORY(file, view);
    
    view_undo_redo(system, models, view, &file->state.undo.redo, ED_REDO);
    
    Assert(file->state.undo.undo.size >= 0);
}

COMMAND_DECL(interactive_new){
    USE_MODELS(models);
    USE_VIEW(view);
    view_show_interactive(system, view, models, IAct_New, IInt_Sys_File_List, make_lit_string("New: "));
}

COMMAND_DECL(interactive_open){
    USE_MODELS(models);
    USE_VIEW(view);
    view_show_interactive(system, view, models, IAct_Open, IInt_Sys_File_List,make_lit_string("Open: "));
}

COMMAND_DECL(interactive_open_or_new){
    USE_MODELS(models);
    USE_VIEW(view);
    view_show_interactive(system, view, models, IAct_OpenOrNew, IInt_Sys_File_List,make_lit_string("Open: "));
}

// TODO(allen): Improvements to reopen
// - Perform a diff
// - If the diff is not tremendously big, apply the edits.
COMMAND_DECL(reopen){
    USE_MODELS(models);
    USE_VIEW(view);
    REQ_FILE(file, view);
    
    if (file->canon.name.str == 0) return;
    
    if (file->canon.name.size != 0){
        Plat_Handle handle;
        if (system->load_handle(file->canon.name.str, &handle)){
            
            i32 size = system->load_size(handle);
            
            Partition *part = &models->mem.part;
            Temp_Memory temp = begin_temp_memory(part);
            char *buffer = push_array(part, char, size);
            
            if (buffer){
                if (system->load_file(handle, buffer, size)){
                    system->load_close(handle);
                    
                    General_Memory *general = &models->mem.general;
                    
                    File_Edit_Positions edit_poss[16];
                    int32_t line_number[16];
                    int32_t column_number[16];
                    View *vptrs[16];
                    i32 vptr_count = 0;
                    for (View_Iter iter = file_view_iter_init(&models->layout, file, 0);
                         file_view_iter_good(iter);
                         iter = file_view_iter_next(iter)){
                        vptrs[vptr_count] = iter.view;
                        edit_poss[vptr_count] = iter.view->edit_pos[0];
                        line_number[vptr_count] = iter.view->edit_pos[0].cursor.line;
                        column_number[vptr_count] = iter.view->edit_pos[0].cursor.character;
                        iter.view->edit_pos = 0;
                        ++vptr_count;
                    }
                    
                    file_close(system, general, file);
                    init_normal_file(system, models, file, buffer, size);
                    
                    for (i32 i = 0; i < vptr_count; ++i){
                        view_set_file(system, vptrs[i], file, models);
                        
                        int32_t line = line_number[i];
                        int32_t character = column_number[i];
                        
                        *vptrs[i]->edit_pos = edit_poss[i];
                        Full_Cursor cursor = view_compute_cursor(system, vptrs[i], seek_line_char(line, character), 0);
                        
                        view_set_cursor(vptrs[i], cursor, true, file->settings.unwrapped_lines);
                    }
                }
                else{
                    system->load_close(handle);
                }
            }
            else{
                system->load_close(handle);
            }
            
            end_temp_memory(temp);
        }
    }
}

COMMAND_DECL(save){
    USE_MODELS(models);
    USE_VIEW(view);
    REQ_FILE(file, view);
    
    if (!file->is_dummy && file_is_ready(file) && buffer_can_save(file)){
        save_file(system, models, file);
    }
}

COMMAND_DECL(save_as){
    USE_MODELS(models);
    USE_VIEW(view);
    REQ_FILE(file, view);
    
    view_show_interactive(system, view, models, IAct_Save_As, IInt_Sys_File_List, make_lit_string("Save As: "));
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
    USE_MODELS(models);
    USE_VIEW(view);
    
    view_show_interactive(system, view, models, IAct_Switch, IInt_Live_File_List, make_lit_string("Switch Buffer: "));
}

COMMAND_DECL(interactive_kill_buffer){
    USE_MODELS(models);
    USE_VIEW(view);
    
    view_show_interactive(system, view, models, IAct_Kill, IInt_Live_File_List, make_lit_string("Kill Buffer: "));
}

COMMAND_DECL(kill_buffer){
    USE_MODELS(models);
    USE_VIEW(view);
    REQ_FILE(file, view);
    
    interactive_try_kill_file(system, models, view, file);
}

internal void
case_change_range(System_Functions *system, Models *models, View *view, Editing_File *file, u8 a, u8 z, u8 char_delta){
    Range range  = {0};
    range.min = Min(view->edit_pos->cursor.pos, view->edit_pos->mark);
    range.max = Max(view->edit_pos->cursor.pos, view->edit_pos->mark);
    if (range.start < range.end){
        Edit_Step step = {};
        step.type = ED_NORMAL;
        step.edit.start = range.start;
        step.edit.end = range.end;
        step.edit.len = range.end - range.start;
        
        if (file->state.still_lexing){
            system->cancel_job(BACKGROUND_THREADS, file->state.lex_job);
        }
        
        file_update_history_before_edit(&models->mem, file, step, 0, hist_normal);
        
        u8 *data = (u8*)file->state.buffer.data;
        for (i32 i = range.start; i < range.end; ++i){
            if (data[i] >= a && data[i] <= z){
                data[i] += char_delta;
            }
        }
        
        if (file->state.token_array.tokens){
            file_relex_parallel(system, models, file, range.start, range.end, 0);
        }
    }
}

COMMAND_DECL(open_color_tweaker){
    USE_VIEW(view);
    USE_MODELS(models);
    view_show_theme(view, models);
}

COMMAND_DECL(open_debug){
    USE_VIEW(view);
    USE_MODELS(models);
    view_show_GUI(view, models, VUI_Debug);
    view->debug_vars = null_debug_vars;
}

COMMAND_DECL(user_callback){
    USE_MODELS(models);
    if (binding.custom) binding.custom(&models->app_links);
}

global Command_Function *command_table[cmdid_count];

SCROLL_RULE_SIG(fallback_scroll_rule){
    b32 result = false;
    if (target_x != *scroll_x){
        *scroll_x = target_x;
        result = true;
    }
    if (target_y != *scroll_y){
        *scroll_y = target_y;
        result = true;
    }
    return(result);
}

#define DEFAULT_MAP_SIZE 10
#define DEFAULT_UI_MAP_SIZE 32

internal void
setup_ui_commands(Command_Map *commands, Partition *part, i32 parent){
    map_init(commands, part, DEFAULT_UI_MAP_SIZE, parent);
    
    // TODO(allen): This is hacky, when the new UI stuff happens, let's fix it,
    // and by that I mean actually fix it, don't just say you fixed it with
    // something stupid again.
    u8 mdfr_array[] = {MDFR_NONE, MDFR_SHIFT, MDFR_CTRL, MDFR_SHIFT | MDFR_CTRL};
    for (i32 i = 0; i < 4; ++i){
        u8 mdfr = mdfr_array[i];
        map_add(commands, key_left, mdfr, command_null);
        map_add(commands, key_right, mdfr, command_null);
        map_add(commands, key_up, mdfr, command_null);
        map_add(commands, key_down, mdfr, command_null);
        map_add(commands, key_back, mdfr, command_null);
    }
}

internal void
setup_file_commands(Command_Map *commands, Partition *part, i32 parent){
    map_init(commands, part, DEFAULT_MAP_SIZE, parent);
}

internal void
setup_top_commands(Command_Map *commands, Partition *part, i32 parent){
    map_init(commands, part, DEFAULT_MAP_SIZE, parent);
}

internal b32
interpret_binding_buffer(Models *models, void *buffer, i32 size){
    b32 result = true;
    
    General_Memory *gen = &models->mem.general;
    Partition *part = &models->mem.part;
    Temp_Memory temp = begin_temp_memory(part);
    
    Partition local_part = {0};
    
    Mapping new_mapping = {0};
    
    models->scroll_rule = fallback_scroll_rule;
    models->hook_open_file = 0;
    models->hook_new_file = 0;
    models->hook_save_file = 0;
    models->hook_end_file = 0;
    models->command_caller = 0;
    models->input_filter = 0;
    
    b32 did_top = false;
    b32 did_file = false;
    
    Command_Map *map_ptr = 0;
    
    Binding_Unit *unit = (Binding_Unit*)buffer;
    if (unit->type == unit_header && unit->header.error == 0){
        Binding_Unit *end = unit + unit->header.total_size;
        
        i32 user_map_count = unit->header.user_map_count;
        new_mapping.user_map_count = user_map_count;
        
        // Initialize Table and User Maps in Temp Buffer
        new_mapping.map_id_table = push_array(part, i32, user_map_count);
        memset(new_mapping.map_id_table, -1, user_map_count*sizeof(i32));
        
        new_mapping.user_maps = push_array(part, Command_Map, user_map_count);
        memset(new_mapping.user_maps, 0, user_map_count*sizeof(Command_Map));
        
        // Find the Size of Each Map
        for (++unit; unit < end; ++unit){
            switch (unit->type){
                case unit_map_begin:
                {
                    i32 mapid = unit->map_begin.mapid;
                    
                    if (mapid == mapid_ui || mapid == mapid_nomap){
                        break;
                    }
                    else if (mapid == mapid_global){
                        did_top = true;
                    }
                    else if (mapid == mapid_file){
                        did_file = true;
                    }
                    
                    Command_Map *map = get_or_add_map(&new_mapping, mapid);
                    i32 count = map->count;
                    i32 new_count = 0;
                    if (unit->map_begin.replace){
                        map->real_beginning = unit;
                        new_count = unit->map_begin.bind_count;
                    }
                    else{
                        if (map->real_beginning == 0){
                            map->real_beginning = unit;
                        }
                        new_count = unit->map_begin.bind_count + count;
                    }
                    
                    map->count = new_count;
                    if (map->max < new_count){
                        map->max = new_count;
                    }
                }break;
            }
        }
        
        // Add up the Map Counts
        i32 count_global = DEFAULT_MAP_SIZE;
        if (did_top){
            count_global = clamp_bottom(6, new_mapping.map_top.count*3/2);
        }
        
        i32 count_file = DEFAULT_MAP_SIZE;
        if (did_file){
            count_file = clamp_bottom(6, new_mapping.map_file.count*3/2);
        }
        
        i32 count_ui = DEFAULT_UI_MAP_SIZE;
        
        i32 count_user = 0;
        for (i32 i = 0; i < user_map_count; ++i){
            Command_Map *map = &new_mapping.user_maps[i];
            count_user += clamp_bottom(6, map->max*3/2);
        }
        
        i32 binding_memsize = (count_global + count_file + count_ui + count_user)*sizeof(Command_Binding);
        
        // Allocate Needed Memory
        i32 map_id_table_memsize = user_map_count*sizeof(i32);
        i32 user_maps_memsize = user_map_count*sizeof(Command_Map);
        
        i32 map_id_table_rounded_memsize = l_round_up_i32(map_id_table_memsize, 8);
        i32 user_maps_rounded_memsize = l_round_up_i32(user_maps_memsize, 8);
        
        i32 binding_rounded_memsize = l_round_up_i32(binding_memsize, 8);
        
        i32 needed_memsize = map_id_table_rounded_memsize + user_maps_rounded_memsize + binding_rounded_memsize;
        new_mapping.memory = general_memory_allocate(gen, needed_memsize);
        local_part = make_part(new_mapping.memory, needed_memsize);
        
        // Move ID Table Memory and Pointer
        i32 *old_table = new_mapping.map_id_table;
        new_mapping.map_id_table = push_array(&local_part, i32, user_map_count);
        memmove(new_mapping.map_id_table, old_table, map_id_table_memsize);
        
        // Move User Maps Memory and Pointer
        Command_Map *old_maps = new_mapping.user_maps;
        new_mapping.user_maps = push_array(&local_part, Command_Map, user_map_count);
        memmove(new_mapping.user_maps, old_maps, user_maps_memsize);
        
        // Fill in Command Maps
        unit = (Binding_Unit*)buffer;
        for (++unit; unit < end; ++unit){
            switch (unit->type){
                case unit_map_begin:
                {
                    i32 mapid = unit->map_begin.mapid;
                    
                    if (mapid == mapid_ui || mapid == mapid_nomap){
                        map_ptr = 0;
                        break;
                    }
                    
                    Command_Map *map = get_or_add_map(&new_mapping, mapid);
                    if (unit >= map->real_beginning){
                        if (mapid == mapid_global){
                            map_ptr = &new_mapping.map_top;
                        }
                        else if (mapid == mapid_file){
                            map_ptr = &new_mapping.map_file;
                        }
                        else if (mapid < mapid_global){
                            i32 index = get_or_add_map_index(&new_mapping, mapid);
                            Assert(index < user_map_count);
                            map_ptr = &new_mapping.user_maps[index];
                        }
                        else{
                            map_ptr = 0;
                            break;
                        }
                        
                        Assert(map_ptr != 0);
                        
                        // NOTE(allen): Map can begin multiple times, only alloc and clear when we first see it.
                        if (map_ptr->commands == 0){
                            i32 count = map->max;
                            i32 table_max = clamp_bottom(6, count*3/2);
                            map_init(map_ptr, &local_part, table_max, mapid_global);
                        }
                    }
                }break;
                
                case unit_inherit:
                {
                    if (map_ptr != 0){
                        map_ptr->parent = unit->map_inherit.mapid;
                    }
                }break;
                
                case unit_binding:
                {
                    if (map_ptr != 0){
                        Command_Function *func = 0;
                        if (unit->binding.command_id < cmdid_count){
                            func = command_table[unit->binding.command_id];
                        }
                        if (func != 0){
                            if (unit->binding.code == 0){
                                u32 index = 0;
                                if (map_get_modifiers_hash(unit->binding.modifiers, &index)){
                                    map_ptr->vanilla_keyboard_default[index].function = func;
                                    map_ptr->vanilla_keyboard_default[index].custom_id = unit->binding.command_id;
                                }
                            }
                            else{
                                map_add(map_ptr, unit->binding.code, unit->binding.modifiers, func, unit->binding.command_id);
                            }
                        }
                    }
                }break;
                
                case unit_callback:
                {
                    if (map_ptr != 0){
                        Command_Function *func = command_user_callback;
                        Custom_Command_Function *custom = unit->callback.func;
                        if (func != 0){
                            if (unit->callback.code == 0){
                                u32 index = 0;
                                if (map_get_modifiers_hash(unit->binding.modifiers, &index)){
                                    map_ptr->vanilla_keyboard_default[index].function = func;
                                    map_ptr->vanilla_keyboard_default[index].custom = custom;
                                }
                            }
                            else{
                                map_add(map_ptr, unit->callback.code, unit->callback.modifiers, func, custom);
                            }
                        }
                    }
                }break;
                
                case unit_hook:
                {
                    i32 hook_id = unit->hook.hook_id;
                    if (hook_id >= 0){
                        if (hook_id < hook_type_count){
                            models->hooks[hook_id] = (Hook_Function*)unit->hook.func;
                        }
                        else{
                            switch (hook_id){
                                case special_hook_open_file:
                                {
                                    models->hook_open_file = (Open_File_Hook_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_new_file:
                                {
                                    models->hook_new_file = (Open_File_Hook_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_save_file:
                                {
                                    models->hook_save_file = (Open_File_Hook_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_end_file:
                                {
                                    models->hook_end_file = (Open_File_Hook_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_command_caller:
                                {
                                    models->command_caller = (Command_Caller_Hook_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_scroll_rule:
                                {
                                    models->scroll_rule = (Scroll_Rule_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_input_filter:
                                {
                                    models->input_filter = (Input_Filter_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_start:
                                {
                                    models->hook_start = (Start_Hook_Function*)unit->hook.func;
                                }break;
                            }
                        }
                    }
                }break;
            }
        }
        
        if (!did_top){
            setup_top_commands(&new_mapping.map_top, &local_part, mapid_global);
        }
        if (!did_file){
            setup_file_commands(&new_mapping.map_file, &local_part, mapid_global);
        }
        setup_ui_commands(&new_mapping.map_ui, &local_part, mapid_global);
    }
    else{
        // TODO(allen): Probably should have some recovery plan here.
        InvalidCodePath;
    }
    
    Mapping old_mapping = models->mapping;
    if (old_mapping.memory != 0){
        // TODO(allen): Do I need to explicity work on recovering the old ids of buffers?
        general_memory_free(gen, old_mapping.memory);
    }
    
    models->mapping = new_mapping;
    end_temp_memory(temp);
    
    return(result);
}

#include "4ed_api_implementation.cpp"

struct Command_In{
    Command_Data *cmd;
    Command_Binding bind;
};

internal void
command_caller(Coroutine_Head *coroutine){
    Command_In *cmd_in = (Command_In*)coroutine->in;
    Command_Data *command = cmd_in->cmd;
    Models *models = command->models;
    USE_VIEW(view);
    
    if (models->command_caller){
        Generic_Command generic;
        if (cmd_in->bind.function == command_user_callback){
            generic.command = cmd_in->bind.custom;
        }
        else{
            generic.cmdid = (Command_ID)cmd_in->bind.custom_id;
        }
        models->command_caller(&models->app_links, generic);
    }
    else{
        cmd_in->bind.function(command->system, command, cmd_in->bind);
    }
}

internal void
app_links_init(System_Functions *system, Application_Links *app_links, void *data, i32 size){
    app_links->memory = data;
    app_links->memory_size = size;
    
    FillAppLinksAPI(app_links);
    
    app_links->current_coroutine = 0;
    app_links->system_links = system;
}

internal void
setup_command_table(){
#define SET(n) command_table[cmdid_##n] = command_##n
    SET(null);
    
    SET(undo);
    SET(redo);
    
    SET(interactive_new);
    SET(interactive_open);
    SET(interactive_open_or_new);
    SET(interactive_switch_buffer);
    SET(interactive_kill_buffer);
    SET(save_as);
    
    SET(reopen);
    SET(save);
    SET(kill_buffer);
    
    SET(open_color_tweaker);
    SET(open_debug);
#undef SET
}

// App Functions

internal void
app_hardcode_default_style(Models *models){
    Interactive_Style file_info_style = {0};
    Style *styles = models->styles.styles;
    Style *style = styles + 1;
    
    /////////////////
    style_set_name(style, make_lit_string("4coder"));
    
    style->main.back_color = 0xFF0C0C0C;
    style->main.margin_color = 0xFF181818;
    style->main.margin_hover_color = 0xFF252525;
    style->main.margin_active_color = 0xFF323232;
    style->main.list_item_color = style->main.margin_color;
    style->main.list_item_hover_color = style->main.margin_hover_color;
    style->main.list_item_active_color = style->main.margin_active_color;
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
    style->main.ghost_character_color = color_blend(style->main.default_color, 0.5f, style->main.back_color);
    
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
    models->styles.count = (i32)(style - styles);
    models->styles.max = ArrayCount(models->styles.styles);
    style_copy(main_style(models), models->styles.styles + 1);
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
    CLAct_WindowFullscreen,
    CLAct_FontSize,
    CLAct_FontUseHinting,
    CLAct_LogStdout,
    CLAct_LogFile,
    CLAct_Count
};

enum Command_Line_Mode{
    CLMode_App,
    CLMode_Custom
};

internal void
init_command_line_settings(App_Settings *settings, Plat_Settings *plat_settings, i32 argc, char **argv){
    char *arg = 0;
    Command_Line_Mode mode = CLMode_App;
    Command_Line_Action action = CLAct_Nothing;
    b32 strict = false;
    
    settings->init_files_max = ArrayCount(settings->init_files);
    for (i32 i = 1; i <= argc; ++i){
        if (i == argc){
            arg = "";
        }
        else{
            arg = argv[i];
        }
        
        if (arg[0] == '-' && arg[1] == '-'){
            char *long_arg_name = arg+2;
            if (match_cc(long_arg_name, "custom")){
                mode = CLMode_Custom;
                continue;
            }
        }
        
        switch (mode){
            case CLMode_App:
            {
                switch (action){
                    case CLAct_Nothing:
                    {
                        if (arg[0] == '-'){
                            action = CLAct_Ignore;
                            switch (arg[1]){
                                case 'd': action = CLAct_CustomDLL; strict = false;     break;
                                case 'D': action = CLAct_CustomDLL; strict = true;      break;
                                
                                case 'i': action = CLAct_InitialFilePosition;           break;
                                
                                case 'w': action = CLAct_WindowSize;                    break;
                                case 'W': action = CLAct_WindowMaximize;                break;
                                case 'p': action = CLAct_WindowPosition;                break;
                                case 'F': action = CLAct_WindowFullscreen;              break;
                                
                                case 'f': action = CLAct_FontSize;                      break;
                                case 'h': action = CLAct_FontUseHinting; --i;           break;
                                
                                case 'l': action = CLAct_LogStdout; --i;                break;
                                case 'L': action = CLAct_LogFile; --i;                  break;
                            }
                        }
                        else if (arg[0] != 0){
                            if (settings->init_files_count < settings->init_files_max){
                                i32 index = settings->init_files_count++;
                                settings->init_files[index] = arg;
                            }
                        }
                    }break;
                    
                    case CLAct_CustomDLL:
                    {
                        plat_settings->custom_dll_is_strict = (b8)strict;
                        if (i < argc){
                            plat_settings->custom_dll = argv[i];
                        }
                        action = CLAct_Nothing;
                    }break;
                    
                    case CLAct_InitialFilePosition:
                    {
                        if (i < argc){
                            settings->initial_line = str_to_int_c(argv[i]);
                        }
                        action = CLAct_Nothing;
                    }break;
                    
                    case CLAct_WindowSize:
                    {
                        if (i + 1 < argc){
                            plat_settings->set_window_size = true;
                            
                            i32 w = str_to_int_c(argv[i]);
                            i32 h = str_to_int_c(argv[i+1]);
                            if (w > 0){
                                plat_settings->window_w = w;
                            }
                            if (h > 0){
                                plat_settings->window_h = h;
                            }
                            
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
                        if (i + 1 < argc){
                            plat_settings->set_window_pos = true;
                            
                            i32 x = str_to_int_c(argv[i]);
                            i32 y = str_to_int_c(argv[i+1]);
                            if (x > 0){
                                plat_settings->window_x = x;
                            }
                            if (y > 0){
                                plat_settings->window_y = y;
                            }
                            
                            ++i;
                        }
                        action = CLAct_Nothing;
                    }break;
                    
                    case CLAct_WindowFullscreen:
                    {
                        --i;
                        plat_settings->fullscreen_window = true;
                        action = CLAct_Nothing;
                    }break;
                    
                    case CLAct_FontSize:
                    {
                        if (i < argc){
                            plat_settings->font_size = str_to_int_c(argv[i]);
                            plat_settings->font_size = clamp_bottom(8, plat_settings->font_size);
                        }
                        action = CLAct_Nothing;
                    }break;
                    
                    case CLAct_FontUseHinting:
                    {
                        plat_settings->use_hinting = true;
                        action = CLAct_Nothing;
                    }break;
                    
                    case CLAct_LogStdout:
                    {
                        plat_settings->use_log = LogTo_Stdout;
                        action = CLAct_Nothing;
                    }break;
                    
                    case CLAct_LogFile:
                    {
                        plat_settings->use_log = LogTo_LogFile;
                        action = CLAct_Nothing;
                    }break;
                }
            }break;
            
            case CLMode_Custom:
            {
                settings->custom_flags = argv + i;
                settings->custom_flags_count = argc - i;
                i = argc;
                mode = CLMode_App;
            }break;
        }
    }
}

internal App_Vars*
app_setup_memory(System_Functions *system, Application_Memory *memory){
    Partition _partition = make_part(memory->vars_memory, memory->vars_memory_size);
    App_Vars *vars = push_struct(&_partition, App_Vars);
    Assert(vars != 0);
    *vars = null_app_vars;
    vars->models.mem.part = _partition;
    
#if defined(USE_DEBUG_MEMORY)
    general_memory_open(system, &vars->models.mem.general, memory->target_memory, memory->target_memory_size);
#else
    general_memory_open(&vars->models.mem.general, memory->target_memory, memory->target_memory_size);
#endif
    
    return(vars);
}

App_Read_Command_Line_Sig(app_read_command_line){
    i32 out_size = 0;
    App_Vars *vars = app_setup_memory(system, memory);
    App_Settings *settings = &vars->models.settings;
    
    *settings = null_app_settings;
    plat_settings->font_size = 16;
    
    if (argc > 1){
        init_command_line_settings(&vars->models.settings, plat_settings, argc, argv);
    }
    
    *files = vars->models.settings.init_files;
    *file_count = &vars->models.settings.init_files_count;
    
    return(out_size);
}

App_Init_Sig(app_init){
    App_Vars *vars = (App_Vars*)memory->vars_memory;
    Models *models = &vars->models;
    models->keep_playing = true;
    
    app_links_init(system, &models->app_links, memory->user_memory, memory->user_memory_size);
    
    models->config_api = api;
    models->app_links.cmd_context = &vars->command_data;
    
    Partition *partition = &models->mem.part;
    
    i32 panel_max_count = models->layout.panel_max_count = MAX_VIEWS;
    i32 divider_max_count = panel_max_count - 1;
    models->layout.panel_count = 0;
    
    Panel *panels = push_array(partition, Panel, panel_max_count);
    models->layout.panels = panels;
    
    dll_init_sentinel(&models->layout.free_sentinel);
    dll_init_sentinel(&models->layout.used_sentinel);
    
    Panel *panel = panels;
    for (i32 i = 0; i < panel_max_count; ++i, ++panel){
        dll_insert(&models->layout.free_sentinel, panel);
    }
    
    Panel_Divider *dividers = push_array(partition, Panel_Divider, divider_max_count);
    models->layout.dividers = dividers;
    
    Panel_Divider *div = dividers;
    for (i32 i = 0; i < divider_max_count-1; ++i, ++div){
        div->next = (div + 1);
    }
    div->next = 0;
    models->layout.free_divider = dividers;
    
    {
        models->live_set.count = 0;
        models->live_set.max = panel_max_count;
        
        models->live_set.views = push_array(partition, View, models->live_set.max);
        
        dll_init_sentinel(&models->live_set.free_sentinel);
        
        i32 max = models->live_set.max;
        View *view = models->live_set.views;
        for (i32 i = 0; i < max; ++i, ++view){
            dll_insert(&models->live_set.free_sentinel, view);
            
            View_Persistent *persistent = &view->persistent;
            persistent->id = i;
        }
    }
    
    {
        umem memsize = KB(8);
        void *mem = push_block(partition, (i32)memsize);
        parse_context_init_memory(&models->parse_context_memory, mem, memsize);
        parse_context_add_default(&models->parse_context_memory, &models->mem.general);
    }
    
    {
        setup_command_table();
        
        Assert(models->config_api.get_bindings != 0);
        i32 wanted_size = models->config_api.get_bindings(models->app_links.memory, models->app_links.memory_size);
        Assert(wanted_size <= models->app_links.memory_size);
        interpret_binding_buffer(models, models->app_links.memory, wanted_size);
        
        memset(models->app_links.memory, 0, wanted_size);
    }
    
    // NOTE(allen): file setup
    working_set_init(&models->working_set, partition, &vars->models.mem.general);
    models->working_set.default_display_width = DEFAULT_DISPLAY_WIDTH;
    models->working_set.default_minimum_base_display_width = DEFAULT_MINIMUM_BASE_DISPLAY_WIDTH;
    
    // NOTE(allen): clipboard setup
    models->working_set.clipboard_max_size = ArrayCount(models->working_set.clipboards);
    models->working_set.clipboard_size = 0;
    models->working_set.clipboard_current = 0;
    models->working_set.clipboard_rolling = 0;
    
    // TODO(allen): more robust allocation solution for the clipboard
    if (clipboard.str){
        String *dest = working_set_next_clipboard_string(&models->mem.general, &models->working_set, clipboard.size);
        copy_ss(dest, make_string((char*)clipboard.str, clipboard.size));
    }
    
    // NOTE(allen): style setup
    models->global_font_id = 1;
    app_hardcode_default_style(models);
    
    // NOTE(allen): init first panel
    Command_Data *cmd = &vars->command_data;
    
    cmd->models = models;
    cmd->vars = vars;
    cmd->system = system;
    cmd->live_set = &models->live_set;
    
    cmd->screen_width = target->width;
    cmd->screen_height = target->height;
    
    cmd->key = null_key_event_data;
    
    General_Memory *general = &models->mem.general;
    
    struct File_Init{
        String name;
        Editing_File **ptr;
        b32 read_only;
    };
    
    File_Init init_files[] = {
        { make_lit_string("*messages*"), &models->message_buffer, true , },
        { make_lit_string("*scratch*"),  &models->scratch_buffer, false, }
    };
    
    for (i32 i = 0; i < ArrayCount(init_files); ++i){
        Editing_File *file = working_set_alloc_always(&models->working_set, general);
        buffer_bind_name(general, &models->working_set, file, init_files[i].name);
        
        if (init_files[i].read_only){
            init_read_only_file(system, models, file);
        }
        else{
            init_normal_file(system, models, file, 0, 0);
        }
        
        file->settings.never_kill = true;
        file->settings.unimportant = true;
        file->settings.unwrapped_lines = true;
        
        if (init_files[i].ptr){
            *init_files[i].ptr = file;
        }
    }
    
    Panel_And_ID p = layout_alloc_panel(&models->layout);
    panel_make_empty(system, models, p.panel);
    models->layout.active_panel = p.id;
    
    hot_directory_init(&models->hot_directory, current_directory);
    
    // NOTE(allen): child proc list setup
    vars->cli_processes = make_cli_list(partition, 16);
    
    // NOTE(allen): init GUI keys
    models->user_up_key = key_up;
    models->user_down_key = key_down;
}

App_Step_Sig(app_step){
    Application_Step_Result app_result = *app_result_;
    app_result.animating = 0;
    
    App_Vars *vars = (App_Vars*)memory->vars_memory;
    Models *models = &vars->models;
    
    // NOTE(allen): OS clipboard event handling
    String clipboard = input->clipboard;
    if (clipboard.str){
        String *dest =working_set_next_clipboard_string(&models->mem.general, &models->working_set, clipboard.size);
        dest->size = eol_convert_in(dest->str, clipboard.str, clipboard.size);
    }
    
    // NOTE(allen): check files are up to date
    {
        b32 mem_too_small = 0;
        i32 size = 0;
        i32 buffer_size = KB(32);
        
        Partition *part = &models->mem.part;
        Temp_Memory temp = begin_temp_memory(part);
        char *buffer = push_array(part, char, buffer_size);
        u32 unmark_top = 0;
        u32 unmark_max = (8 << 10);
        Editing_File **unmark = (Editing_File**)push_array(part, Editing_File*, unmark_max);
        
        Working_Set *working_set = &models->working_set;
        
        for (;system->get_file_change(buffer, buffer_size, &mem_too_small, &size);){
            Assert(!mem_too_small);
            Editing_File_Canon_Name canon;
            if (get_canon_name(system, &canon, make_string(buffer, size))){
                Editing_File *file = working_set_canon_contains(working_set, canon.name);
                if (file){
                    if (file->state.ignore_behind_os == 0){
                        file_mark_behind_os(file);
                    }
                    else if (file->state.ignore_behind_os == 1){
                        file->state.ignore_behind_os = 2;
                        unmark[unmark_top++] = file;
                        if (unmark_top == unmark_max){
                            break;
                        }
                    }
                }
            }
        }
        
        for (u32 i = 0; i < unmark_top; ++i){
            unmark[i]->state.ignore_behind_os = 0;
        }
        
        end_temp_memory(temp);
    }
    
    // NOTE(allen): reorganizing panels on screen
    {
        i32 prev_width = models->layout.full_width;
        i32 prev_height = models->layout.full_height;
        i32 current_width = target->width;
        i32 current_height = target->height;
        
        models->layout.full_width = current_width;
        models->layout.full_height = current_height;
        
        if (prev_width != current_width || prev_height != current_height){
            layout_refit(&models->layout, prev_width, prev_height);
        }
    }
    
    // NOTE(allen): prepare input information
    {
        if (models->input_filter){
            models->input_filter(&input->mouse);
        }
        
        Key_Event_Data mouse_event = {0};
        if (input->mouse.press_l){
            mouse_event.keycode = key_mouse_left;
            input->keys.keys[input->keys.count++] = mouse_event;
        }
        else if (input->mouse.release_l){
            mouse_event.keycode = key_mouse_left_release;
            input->keys.keys[input->keys.count++] = mouse_event;
        }
        
        if (input->mouse.press_r){
            mouse_event.keycode = key_mouse_right;
            input->keys.keys[input->keys.count++] = mouse_event;
        }
        else if (input->mouse.release_r){
            mouse_event.keycode = key_mouse_right_release;
            input->keys.keys[input->keys.count++] = mouse_event;
        }
    }
    
    // NOTE(allen): detect mouse hover status
    i32 mx = input->mouse.x;
    i32 my = input->mouse.y;
    b32 mouse_in_edit_area = false;
    b32 mouse_in_margin_area = false;
    
    Panel *mouse_panel = 0;
    {
        Panel *used_panels = &models->layout.used_sentinel, *panel = 0;
        for (dll_items(panel, used_panels)){
            if (hit_check(mx, my, panel->inner)){
                mouse_panel = panel;
                mouse_in_edit_area = true;
                break;
            }
            else if (hit_check(mx, my, panel->full)){
                mouse_panel = panel;
                mouse_in_margin_area = true;
                break;
            }
        }
    }
    
    b32 mouse_on_divider = false;
    b32 mouse_divider_vertical = false;
    i32 mouse_divider_id = 0;
    i32 mouse_divider_side = 0;
    
    if (mouse_in_margin_area){
        Panel *panel = mouse_panel;
        if (mx >= panel->inner.x0 && mx < panel->inner.x1){
            mouse_divider_vertical = false;
            if (my > panel->inner.y0){
                mouse_divider_side = -1;
            }
            else{
                mouse_divider_side = 1;
            }
        }
        else{
            mouse_divider_vertical = true;
            if (mx > panel->inner.x0){
                mouse_divider_side = -1;
            }
            else{
                mouse_divider_side = 1;
            }
        }
        
        if (models->layout.panel_count > 1){
            mouse_divider_id = panel->parent;
            i32 which_child = panel->which_child;
            for (;;){
                Divider_And_ID div =layout_get_divider(&models->layout, mouse_divider_id);
                
                if (which_child == mouse_divider_side &&
                    div.divider->v_divider == mouse_divider_vertical){
                    mouse_on_divider = true;
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
        Partition *scratch = &models->mem.part;
        
        CLI_List *list = &vars->cli_processes;
        
        Temp_Memory temp = begin_temp_memory(&models->mem.part);
        CLI_Process **procs_to_free = push_array(scratch, CLI_Process*, list->count);
        u32 proc_free_count = 0;
        
        u32 max = KB(128);
        char *dest = push_array(scratch, char, max);
        
        CLI_Process *proc_ptr = list->procs;
        for (u32 i = 0; i < list->count; ++i, ++proc_ptr){
            Editing_File *file = proc_ptr->out_file;
            
            CLI_Handles *cli = &proc_ptr->cli;
            
            u32 amount = 0;
            system->cli_begin_update(cli);
            if (system->cli_update_step(cli, dest, max, &amount)){
                if (file != 0){
                    amount = eol_in_place_convert_in(dest, amount);
                    output_file_append(system, models, file, make_string(dest, amount), proc_ptr->cursor_at_end);
                }
            }
            
            if (system->cli_end_update(cli)){
                if (file != 0){
                    char str_space[256];
                    String str = make_fixed_width_string(str_space);
                    append(&str, make_lit_string("exited with code "));
                    append_int_to_str(&str, cli->exit);
                    output_file_append(system, models, file, str, proc_ptr->cursor_at_end);
                }
                procs_to_free[proc_free_count++] = proc_ptr;
            }
        }
        
        for (i32 i = proc_free_count - 1; i >= 0; --i){
            cli_list_free_proc(list, procs_to_free[i]);
        }
        
        end_temp_memory(temp);
    }
    
    // NOTE(allen): prepare to start executing commands
    Command_Data *cmd = &vars->command_data;
    
    cmd->models = models;
    cmd->vars = vars;
    cmd->system = system;
    cmd->live_set = &models->live_set;
    
    cmd->screen_width = target->width;
    cmd->screen_height = target->height;
    
    cmd->key = null_key_event_data;
    
    if (input->first_step){
        // Open command line files.
        char space[512];
        String cl_filename = make_fixed_width_string(space);
        copy_ss(&cl_filename, models->hot_directory.string);
        i32 cl_filename_len = cl_filename.size;
        for (i32 i = 0; i < models->settings.init_files_count; ++i){
            cl_filename.size = cl_filename_len;
            
            String filename = {0};
            Editing_File_Canon_Name canon_name;
            if (get_canon_name(system, &canon_name, make_string_slowly(models->settings.init_files[i]))){
                filename = canon_name.name;
            }
            else{
                append_sc(&cl_filename, models->settings.init_files[i]);
                filename = cl_filename;
            }
            
            open_file(system, models, filename);
        }
        
        if (models->hook_start != 0){
            char **files = models->settings.init_files;
            i32 files_count = models->settings.init_files_count;
            
            char **flags = models->settings.custom_flags;
            i32 flags_count = models->settings.custom_flags_count;
            
            models->hook_start(&models->app_links, files, files_count, flags, flags_count);
        }
        
        Panel *panel = models->layout.used_sentinel.next;
        for (i32 i = 0; i < models->settings.init_files_count; ++i, panel = panel->next){
            Assert(panel->view->file_data.file != 0);
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
            Coroutine_Head *command_coroutine = models->command_coroutine;
            Command_Data *command = cmd;
            USE_VIEW(view);
            
            for (i32 i = 0; i < 128 && command_coroutine != 0; ++i){
                User_Input user_in = {0};
                user_in.abort = true;
                
                command_coroutine = app_resume_coroutine(system, &models->app_links, Co_Command, command_coroutine, &user_in, models->command_coroutine_flags);
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
            
            view_show_interactive(system, view, models, IAct_Sure_To_Close, IInt_Sure_To_Close, make_lit_string("Are you sure?"));
            
            models->command_coroutine = command_coroutine;
        }
        else{
            models->keep_playing = 0;
        }
    }
    
    // NOTE(allen): pass events to debug
    vars->available_input = init_available_input(&input->keys, &input->mouse);
    
    {
        Debug_Data *debug = &models->debug;
        Key_Input_Data key_data = get_key_data(&vars->available_input);
        
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
    
    // NOTE(allen): Keyboard and Mouse input to command coroutine.
    if (models->command_coroutine != 0){
        Coroutine_Head *command_coroutine = models->command_coroutine;
        u32 get_flags = models->command_coroutine_flags[0];
        u32 abort_flags = models->command_coroutine_flags[1];
        
        get_flags |= abort_flags;
        
        Command_Data *command = cmd;
        USE_VIEW(view);
        
        Partition *part = &models->mem.part;
        Temp_Memory temp = begin_temp_memory(part);
        
        // HACK(allen): This can be simplified a lot more.
        enum{
            Event_Keyboard,
            Event_Mouse,
        };
        struct Coroutine_Event{
            u32 type;
            u32 key_i;
        };
        
        Coroutine_Event *events = push_array(part, Coroutine_Event, 32);
        u32 event_count = 0;
        
        Key_Input_Data key_data = get_key_data(&vars->available_input);
        
        if ((get_flags & EventOnAnyKey) || (get_flags & EventOnEsc)){
            for (i32 key_i = 0; key_i < key_data.count; ++key_i){
                Coroutine_Event *new_event = &events[event_count++];
                new_event->type = Event_Keyboard;
                new_event->key_i = key_i;
            }
        }
        
        if (models->command_coroutine != 0 && (get_flags & EventOnMouse)){
            Coroutine_Event *new_event = &events[event_count++];
            new_event->type = Event_Mouse;
        }
        
        Coroutine_Event *event = events;
        for (u32 event_i = 0; event_i < event_count; ++event_i, ++event){
            b32 pass_in = false;
            User_Input user_in = {0};
            
            switch (event->type){
                case Event_Keyboard:
                {
                    Key_Event_Data key = get_single_key(&key_data, event->key_i);
                    cmd->key = key;
                    
                    i32 map = mapid_global;
                    if (view != 0){
                        map = view->map;
                    }
                    Command_Binding cmd_bind = map_extract_recursive(&models->mapping, map, key);
                    
                    user_in.type = UserInputKey;
                    user_in.key = key;
                    user_in.command.command = cmd_bind.custom;
                    
                    if ((abort_flags & EventOnEsc) && key.keycode == key_esc){
                        user_in.abort = true;
                    }
                    else if (abort_flags & EventOnAnyKey){
                        user_in.abort = true;
                    }
                    
                    if (get_flags & EventOnAnyKey){
                        pass_in = true;
                        consume_input(&vars->available_input, Input_AnyKey, "command coroutine");
                    }
                    if (key.keycode == key_esc){
                        if (get_flags & EventOnEsc){
                            pass_in = true;
                        }
                        consume_input(&vars->available_input, Input_Esc, "command coroutine");
                    }
                }break;
                
                case Event_Mouse:
                {
                    user_in.type = UserInputMouse;
                    user_in.mouse = input->mouse;
                    
                    if (abort_flags & EventOnMouseMove){
                        user_in.abort = true;
                    }
                    if (get_flags & EventOnMouseMove){
                        pass_in = true;
                        consume_input(&vars->available_input, Input_MouseMove, "command coroutine");
                    }
                    
                    if (input->mouse.press_l || input->mouse.release_l || input->mouse.l){
                        if (abort_flags & EventOnLeftButton){
                            user_in.abort = true;
                        }
                        if (get_flags & EventOnLeftButton){
                            pass_in = true;
                            consume_input(&vars->available_input, Input_MouseLeftButton, "command coroutine");
                        }
                    }
                    
                    if (input->mouse.press_r || input->mouse.release_r || input->mouse.r){
                        if (abort_flags & EventOnRightButton){
                            user_in.abort = true;
                        }
                        if (get_flags & EventOnRightButton){
                            pass_in = true;
                            consume_input(&vars->available_input, Input_MouseRightButton, "command coroutine");
                        }
                    }
                    
                    if (input->mouse.wheel != 0){
                        if (abort_flags & EventOnWheel){
                            user_in.abort = true;
                        }
                        if (get_flags & EventOnWheel){
                            pass_in = true;
                            consume_input(&vars->available_input, Input_MouseWheel, "command coroutine");
                        }
                    }
                }break;
            }
            
            if (pass_in){
                models->command_coroutine =  app_resume_coroutine(system, &models->app_links, Co_Command, command_coroutine, &user_in, models->command_coroutine_flags);
                
                app_result.animating = true;
                
                // TODO(allen): Should I somehow allow a view to clean up however it wants after a
                // command finishes, or after transfering to another view mid command?
                if (view != 0 && models->command_coroutine == 0){
                    init_query_set(&view->query_set);
                }
                if (models->command_coroutine == 0) break;
            }
        }
        
        end_temp_memory(temp);
    }
    
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
        View *view = 0;
        b32 active = 0;
        Input_Summary summary = {0};
        
        Command_Data *command = cmd;
        USE_VIEW(active_view);
        USE_PANEL(active_panel);
        
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            view = panel->view;
            active = (panel == active_panel);
            summary = (active)?(active_input):(dead_input);
            
            view->changed_context_in_step = 0;
            
            View_Step_Result result = step_file_view(system, view, models, active_view, summary);
            
            if (result.animating){
                app_result.animating = 1;
            }
            if (result.consume_keys){
                consume_input(&vars->available_input, Input_AnyKey, "file view step");
            }
            if (result.consume_keys || result.consume_esc){
                consume_input(&vars->available_input, Input_Esc, "file view step");
            }
            
            if (view->changed_context_in_step == 0){
                active = (panel == active_panel);
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
                
                i32 max_y = 0;
                if (view->showing_ui == VUI_None){
                    max_y = view_compute_max_target_y(view);
                }
                else{
                    max_y = view->gui_max_y;
                }
                
                Input_Process_Result ip_result = do_step_file_view(system, view, models, panel->inner, active, &summary, *scroll_vars, view->scroll_region, max_y);
                
                if (ip_result.is_animating){
                    app_result.animating = 1;
                }
                if (ip_result.consumed_l){
                    consume_input(&vars->available_input, Input_MouseLeftButton, "file view step");
                }
                if (ip_result.consumed_r){
                    consume_input(&vars->available_input, Input_MouseRightButton, "file view step");
                }
                
                if (ip_result.has_max_y_suggestion){
                    view->gui_max_y = ip_result.max_y;
                }
                
                if (!gui_scroll_eq(scroll_vars, &ip_result.vars)){
                    if (file_scroll){
                        view_set_scroll(system, view, ip_result.vars);
                    }
                    else{
                        *scroll_vars = ip_result.vars;
                    }
                }
                
                view->scroll_region = ip_result.region;
            }
        }
    }
    
    // NOTE(allen): command execution
    {
        Key_Input_Data key_data = get_key_data(&vars->available_input);
        b32 hit_something = 0;
        b32 hit_esc = 0;
        
        for (i32 key_i = 0; key_i < key_data.count; ++key_i){
            if (models->command_coroutine != 0){
                break;
            }
            
            switch (vars->state){
                case APP_STATE_EDIT:
                {
                    Key_Event_Data key = get_single_key(&key_data, key_i);
                    cmd->key = key;
                    
                    Command_Data *command = cmd;
                    USE_VIEW(view);
                    Assert(view != 0);
                    
                    Command_Binding cmd_bind = map_extract_recursive(&models->mapping, view->map, key);
                    
                    if (cmd_bind.function){
                        if (key.keycode == key_esc){
                            hit_esc = true;
                        }
                        else{
                            hit_something = true;
                        }
                        
                        Assert(models->command_coroutine == 0);
                        Coroutine_Head *command_coroutine = system->create_coroutine(command_caller);
                        models->command_coroutine = command_coroutine;
                        
                        Command_In cmd_in;
                        cmd_in.cmd = cmd;
                        cmd_in.bind = cmd_bind;
                        
                        models->command_coroutine = app_launch_coroutine(system, &models->app_links, Co_Command, models->command_coroutine, &cmd_in, models->command_coroutine_flags);
                        
                        models->prev_command = cmd_bind;
                        
                        app_result.animating = true;
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
            consume_input(&vars->available_input, Input_AnyKey, "command dispatcher");
        }
        if (hit_esc){
            consume_input(&vars->available_input, Input_Esc, "command dispatcher");
        }
    }
    
    // NOTE(allen): pass consumption data to debug
    {
        Debug_Data *debug = &models->debug;
        i32 count = debug->this_frame_count;
        
        Consumption_Record *record = 0;
        
        record = &vars->available_input.records[Input_MouseLeftButton];
        if (record->consumed && record->consumer[0] != 0){
            Debug_Input_Event *event = debug->input_events;
            for (i32 i = 0; i < count; ++i, ++event){
                if (event->key == key_mouse_left && event->consumer[0] == 0){
                    memcpy(event->consumer, record->consumer, sizeof(record->consumer));
                }
            }
        }
        
        record = &vars->available_input.records[Input_MouseRightButton];
        if (record->consumed && record->consumer[0] != 0){
            Debug_Input_Event *event = debug->input_events;
            for (i32 i = 0; i < count; ++i, ++event){
                if (event->key == key_mouse_right && event->consumer[0] == 0){
                    memcpy(event->consumer, record->consumer, sizeof(record->consumer));
                }
            }
        }
        
        record = &vars->available_input.records[Input_Esc];
        if (record->consumed && record->consumer[0] != 0){
            Debug_Input_Event *event = debug->input_events;
            for (i32 i = 0; i < count; ++i, ++event){
                if (event->key == key_esc && event->consumer[0] == 0){
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
        String welcome = make_lit_string(messages);
        do_feedback_message(system, models, welcome, true);
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
            }
        }break;
        
        case APP_STATE_RESIZING:
        {
            if (input->mouse.l){
                Panel_Divider *divider = vars->resizing.divider;
                i32 mouse_position = 0;
                
                b32 do_absolute_positions = 1;
                if (do_absolute_positions){
                    i32 absolute_positions[MAX_VIEWS];
                    i32 min = 0, max = 0;
                    i32 div_id = (i32)(divider - models->layout.dividers);
                    
                    layout_compute_absolute_positions(&models->layout, absolute_positions);
                    mouse_position = (divider->v_divider)?(mx):(my);
                    layout_get_min_max(&models->layout, divider, absolute_positions, &min, &max);
                    absolute_positions[div_id] = clamp(min, mouse_position, max);
                    layout_update_all_positions(&models->layout, absolute_positions);
                }
                
                else{
                    if (divider->v_divider){
                        mouse_position = clamp(0, mx, models->layout.full_width);
                    }
                    else{
                        mouse_position = clamp(0, my, models->layout.full_height);
                    }
                    divider->pos = layout_compute_position(&models->layout, divider, mouse_position);
                }
                
                layout_fix_all_panels(&models->layout);
            }
            else{
                vars->state = APP_STATE_EDIT;
            }
        }break;
    }
    
    if (models->layout.panel_state_dirty && models->hooks[hook_view_size_change] != 0){
        models->layout.panel_state_dirty = 0;
        models->hooks[hook_view_size_change](&models->app_links);
    }
    
    if (mouse_in_edit_area && mouse_panel != 0 && input->mouse.press_l){
        i32 new_panel_id = (i32)(mouse_panel - models->layout.panels);
        if (models->layout.active_panel != new_panel_id){
            if (models->command_coroutine != 0){
                User_Input user_in = {0};
                user_in.abort = true;
                
                for (u32 j = 0; j < 10 && models->command_coroutine != 0; ++j){
                    models->command_coroutine = app_resume_coroutine(system, &models->app_links, Co_Command, models->command_coroutine, &user_in, models->command_coroutine_flags);
                }
                
                if (models->command_coroutine != 0){
                    // TODO(allen): post grave warning
                    models->command_coroutine = 0;
                }
                
                Panel *active_panel = &models->layout.panels[models->layout.active_panel];
                View *view = active_panel->view;
                init_query_set(&view->query_set);
            }
            
            models->layout.active_panel = new_panel_id;
            app_result.animating = true;
        }
    }
    
    // NOTE(allen): on the first frame there should be no scrolling
    if (input->first_step){
        Panel *panel = 0, *used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            View *view = panel->view;
            GUI_Scroll_Vars *scroll_vars = &view->gui_scroll;
            if (view->edit_pos){
                scroll_vars = &view->edit_pos->scroll;
            }
            scroll_vars->scroll_x = (f32)scroll_vars->target_x;
            scroll_vars->scroll_y = (f32)scroll_vars->target_y;
        }
    }
    
    // NOTE(allen): if this is the last frame, run the exit hook
    if (!models->keep_playing && models->hooks[hook_exit]){
        if (!models->hooks[hook_exit](&models->app_links)){
            models->keep_playing = 1;
        }
    }
    
    // NOTE(allen): rendering
    {
        begin_render_section(target, system);
        
        Command_Data *command = cmd;
        USE_PANEL(active_panel);
        USE_VIEW(active_view);
        
        // NOTE(allen): render the panels
        Panel *panel, *used_panels;
        used_panels = &models->layout.used_sentinel;
        for (dll_items(panel, used_panels)){
            i32_Rect full = panel->full;
            i32_Rect inner = panel->inner;
            
            View *view = panel->view;
            Style *style = main_style(models);
            
            b32 active = (panel == active_panel);
            u32 back_color = style->main.back_color;
            draw_rectangle(target, full, back_color);
            
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
            
            do_render_file_view(system, view, models, scroll_vars, active_view, panel->inner, active, target, &dead_input);
            
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
    
    *app_result_ = app_result;
    
    // end-of-app_step
}

extern "C" App_Get_Functions_Sig(app_get_functions){
    App_Functions result = {};
    
    result.read_command_line = app_read_command_line;
    result.init = app_init;
    result.step = app_step;
    
    return(result);
}

// BOTTOM

