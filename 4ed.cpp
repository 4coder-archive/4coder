/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application layer for project codename "4ed"
 *
 */

// TOP

#define DEFAULT_DISPLAY_WIDTH 672
#define DEFAULT_MINIMUM_BASE_DISPLAY_WIDTH 550

internal App_Coroutine_State
get_state(Application_Links *app){
    App_Coroutine_State state = {};
    state.co = app->current_coroutine;
    state.type = app->type_coroutine;
    return(state);
}
internal void
restore_state(Application_Links *app, App_Coroutine_State state){
    app->current_coroutine = state.co;
    app->type_coroutine = state.type;
}

internal Coroutine_Head*
app_launch_coroutine(System_Functions *system, Application_Links *app, Coroutine_Type type, Coroutine_Head *co, void *in, void *out){
    Coroutine_Head *result = 0;
    
    App_Coroutine_State prev_state = get_state(app);
    
    app->current_coroutine = co;
    app->type_coroutine = type;
    result = system->launch_coroutine(co, in, out);
    restore_state(app, prev_state);
    
    return(result);
}

internal Coroutine_Head*
app_resume_coroutine(System_Functions *system, Application_Links *app, Coroutine_Type type, Coroutine_Head *co, void *in, void *out){
    Coroutine_Head *result = 0;
    
    App_Coroutine_State prev_state = get_state(app);
    
    app->current_coroutine = co;
    app->type_coroutine = type;
    result = system->resume_coroutine(co, in, out);
    restore_state(app, prev_state);
    
    return(result);
}

internal void
output_file_append(System_Functions *system, Models *models, Editing_File *file, String value){
    if (!file->is_dummy){
        i32 end = buffer_size(&file->state.buffer);
        edit_single(system, models, file, end, end, value.str, value.size);
    }
}

internal void
file_cursor_to_end(System_Functions *system, Models *models, Editing_File *file){
    Assert(file != 0);
    i32 pos = buffer_size(&file->state.buffer);
    for (Panel *panel = models->layout.used_sentinel.next;
         panel != &models->layout.used_sentinel;
         panel = panel->next){
        View *view = panel->view;
        if (view->transient.file_data.file != file){
            continue;
        }
        view_cursor_move(system, view, pos);
        view->transient.edit_pos->mark = view->transient.edit_pos->cursor.pos;
    }
}

internal void
do_feedback_message(System_Functions *system, Models *models, String value){
    Editing_File *file = models->message_buffer;
    if (file != 0){
        output_file_append(system, models, file, value);
        file_cursor_to_end(system, models, file);
    }
}

// Commands

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

#define REQ_OPEN_VIEW(n) USE_VIEW(n); if (view_lock_flags(n) != 0) return

#define REQ_FILE(n,v) Editing_File *n = (v)->transient.file_data.file; if (n == 0) return
#define REQ_FILE_HISTORY(n,v) Editing_File *n = (v)->transient.file_data.file; if (n == 0 || n->state.undo.undo.edits == 0) return

#define COMMAND_DECL(n) internal void command_##n(System_Functions *system, Models *models, Command_Binding binding)

internal View*
panel_make_empty(System_Functions *system, Models *models, Panel *panel){
    Assert(panel->view == 0);
    View_And_ID new_view = live_set_alloc_view(&models->mem.heap, &models->lifetime_allocator, &models->live_set, panel);
    view_set_file(system, models, new_view.view, models->scratch_buffer);
    return(new_view.view);
}

COMMAND_DECL(null){}

internal void
view_undo_redo(System_Functions *system, Models *models, View *view, Edit_Stack *stack, Edit_Type expected_type){
    Editing_File *file = view->transient.file_data.file;
    Assert(file != 0);
    Assert(view->transient.edit_pos != 0);
    if (stack->edit_count > 0){
        Edit_Step step = stack->edits[stack->edit_count - 1];
        Assert(step.type == expected_type);
        edit_historical(system, models, file, view, stack,
                        step, hist_normal);
    }
}

COMMAND_DECL(undo){
    Panel *active_panel = &models->layout.panels[models->layout.active_panel];
    View *view = active_panel->view;
    if (view_lock_flags(view) != 0){
        return;
    }
    Editing_File *file = view->transient.file_data.file;
    if (file->state.undo.undo.edits == 0){
        return;
    }
    view_undo_redo(system, models, view, &file->state.undo.undo, ED_UNDO);
    Assert(file->state.undo.undo.size >= 0);
}

COMMAND_DECL(redo){
    Panel *active_panel = &models->layout.panels[models->layout.active_panel];
    View *view = active_panel->view;
    if (view_lock_flags(view) != 0){
        return;
    }
    Editing_File *file = view->transient.file_data.file;
    if (file->state.undo.undo.edits == 0){
        return;
    }
    view_undo_redo(system, models, view, &file->state.undo.redo, ED_REDO);
    Assert(file->state.undo.undo.size >= 0);
}

COMMAND_DECL(user_callback){
    if (binding.custom != 0){
        binding.custom(&models->app_links);
    }
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
        map_add(commands, key_left , mdfr, command_null, (Custom_Command_Function*)0);
        map_add(commands, key_right, mdfr, command_null, (Custom_Command_Function*)0);
        map_add(commands, key_up   , mdfr, command_null, (Custom_Command_Function*)0);
        map_add(commands, key_down , mdfr, command_null, (Custom_Command_Function*)0);
        map_add(commands, key_back , mdfr, command_null, (Custom_Command_Function*)0);
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
    
    Heap *gen = &models->mem.heap;
    Partition *part = &models->mem.part;
    Temp_Memory temp = begin_temp_memory(part);
    
    Partition local_part = {};
    
    Mapping new_mapping = {};
    
    models->scroll_rule = fallback_scroll_rule;
    models->hook_open_file = 0;
    models->hook_new_file = 0;
    models->hook_save_file = 0;
    models->hook_end_file = 0;
    models->command_caller = 0;
    models->render_caller = 0;
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
        new_mapping.memory = heap_allocate(gen, needed_memsize);
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
                                
                                case special_hook_render_caller:
                                {
                                    models->render_caller = (Render_Caller_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_scroll_rule:
                                {
                                    models->scroll_rule = (Scroll_Rule_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_buffer_name_resolver:
                                {
                                    models->buffer_name_resolver = (Buffer_Name_Resolver_Function*)unit->hook.func;
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
        heap_free(gen, old_mapping.memory);
    }
    
    models->mapping = new_mapping;
    end_temp_memory(temp);
    
    return(result);
}

#include "4ed_api_implementation.cpp"

internal void
command_caller(Coroutine_Head *coroutine){
    Command_In *cmd_in = (Command_In*)coroutine->in;
    Models *models = cmd_in->models;
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
        cmd_in->bind.function(models->system, models, cmd_in->bind);
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
setup_command_table(void){
#define SET(n) command_table[cmdid_##n] = command_##n
    SET(null);
    SET(undo);
    SET(redo);
#undef SET
}

// App Functions

internal void
app_hardcode_default_style(Models *models){
    Interactive_Style file_info_style = {};
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
    style->main.highlight_cursor_line_color = 0xFF1E1E1E;
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
    style_copy(&models->styles.styles[0], models->styles.styles + 1);
}

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
                            settings->font_size = plat_settings->font_size;
                        }
                        action = CLAct_Nothing;
                    }break;
                    
                    case CLAct_FontUseHinting:
                    {
                        plat_settings->use_hinting = true;
                        settings->use_hinting = plat_settings->use_hinting;
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
    App_Vars *vars = push_array(&_partition, App_Vars, 1);
    Assert(vars != 0);
    memset(vars, 0, sizeof(*vars));
    vars->models.mem.part = _partition;
    heap_init(&vars->models.mem.heap);
    heap_extend(&vars->models.mem.heap, memory->target_memory, memory->target_memory_size);
    return(vars);
}

internal u32
get_event_flags(Key_Code keycode){
    u32 event_flags = 0;
    if (keycode == key_esc){
        event_flags |= EventOnEsc;
        event_flags |= EventOnAnyKey;
    }
    else if (keycode == key_mouse_left || keycode == key_mouse_left_release){
        event_flags |= EventOnMouseLeftButton;
    }
    else if (keycode == key_mouse_right || keycode == key_mouse_right_release){
        event_flags |= EventOnMouseRightButton;
    }
    else if (keycode == key_mouse_wheel){
        event_flags |= EventOnMouseWheel;
    }
    else if (keycode == key_mouse_move){
        event_flags |= EventOnMouseMove;
    }
    else if (keycode == key_animate){
        event_flags |= EventOnAnimate;
    }
    else if (keycode == key_click_activate_view || keycode == key_click_deactivate_view){
        event_flags |= EventOnViewActivation;
    }
    else{
        event_flags |= EventOnAnyKey;
    }
    return(event_flags);
}

internal void
force_abort_coroutine(System_Functions *system, Models *models, View *view){
    User_Input user_in = {};
    user_in.abort = true;
    for (u32 j = 0; j < 10 && models->command_coroutine != 0; ++j){
        models->command_coroutine = app_resume_coroutine(system, &models->app_links, Co_Command, models->command_coroutine, &user_in, models->command_coroutine_flags);
    }
    if (models->command_coroutine != 0){
        // TODO(allen): post grave warning
        models->command_coroutine = 0;
    }
    init_query_set(&view->transient.query_set);
}

internal void
launch_command_via_event(System_Functions *system, Application_Step_Result *app_result, Models *models, View *view, Key_Event_Data event){
    models->key = event;
    
    i32 map = view_get_map(view);
    Command_Binding cmd_bind = map_extract_recursive(&models->mapping, map, event);
    
    if (cmd_bind.function != 0){
        Assert(models->command_coroutine == 0);
        Coroutine_Head *command_coroutine = system->create_coroutine(command_caller);
        models->command_coroutine = command_coroutine;
        
        Command_In cmd_in;
        cmd_in.models = models;
        cmd_in.bind = cmd_bind;
        
        models->command_coroutine = app_launch_coroutine(system, &models->app_links, Co_Command, models->command_coroutine, &cmd_in, models->command_coroutine_flags);
        
        models->prev_command = cmd_bind;
        app_result->animating = true;
    }
}

internal void
launch_command_via_keycode(System_Functions *system, Application_Step_Result *app_result, Models *models, View *view, Key_Code keycode){
    Key_Event_Data event = {};
    event.keycode = keycode;
    launch_command_via_event(system, app_result, models, view, event);
}

App_Read_Command_Line_Sig(app_read_command_line){
    i32 out_size = 0;
    App_Vars *vars = app_setup_memory(system, memory);
    App_Settings *settings = &vars->models.settings;
    memset(settings, 0, sizeof(*settings));
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
    models->app_links.cmd_context = models;
    
    Partition *partition = &models->mem.part;
    
    i32 panel_max_count = MAX_VIEWS;
    models->layout.panel_max_count = panel_max_count;
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
        
        //dll_init_sentinel
        models->live_set.free_sentinel.transient.next = &models->live_set.free_sentinel;
        models->live_set.free_sentinel.transient.prev = &models->live_set.free_sentinel;
        
        i32 max = models->live_set.max;
        View *view = models->live_set.views;
        for (i32 i = 0; i < max; ++i, ++view){
            //dll_insert(&models->live_set.free_sentinel, view);
            view->transient.next = models->live_set.free_sentinel.transient.next;
            view->transient.prev = &models->live_set.free_sentinel;
            models->live_set.free_sentinel.transient.next = view;
            view->transient.next->transient.prev = view;
            
            View_Persistent *persistent = &view->persistent;
            persistent->id = i;
        }
    }
    
    {
        umem memsize = KB(8);
        void *mem = push_array(partition, u8, (i32)memsize);
        parse_context_init_memory(&models->parse_context_memory, mem, memsize);
        parse_context_add_default(&models->parse_context_memory, &models->mem.heap);
    }
    
    {
        setup_command_table();
        Assert(models->config_api.get_bindings != 0);
        i32 wanted_size = models->config_api.get_bindings(models->app_links.memory, models->app_links.memory_size);
        Assert(wanted_size <= models->app_links.memory_size);
        interpret_binding_buffer(models, models->app_links.memory, wanted_size);
        memset(models->app_links.memory, 0, wanted_size);
    }
    
    dynamic_variables_init(&models->variable_layout);
    dynamic_workspace_init(&models->mem.heap, &models->lifetime_allocator,
                           DynamicWorkspace_Global, 0,
                           &models->dynamic_workspace);
    
    // NOTE(allen): file setup
    working_set_init(&models->working_set, partition, &vars->models.mem.heap);
    models->working_set.default_display_width = DEFAULT_DISPLAY_WIDTH;
    models->working_set.default_minimum_base_display_width = DEFAULT_MINIMUM_BASE_DISPLAY_WIDTH;
    
    // NOTE(allen): clipboard setup
    models->working_set.clipboard_max_size = ArrayCount(models->working_set.clipboards);
    models->working_set.clipboard_size = 0;
    models->working_set.clipboard_current = 0;
    models->working_set.clipboard_rolling = 0;
    
    // TODO(allen): do(better clipboard allocation)
    if (clipboard.str != 0){
        String *dest = working_set_next_clipboard_string(&models->mem.heap, &models->working_set, clipboard.size);
        copy(dest, make_string((char*)clipboard.str, clipboard.size));
    }
    
    // NOTE(allen): style setup
    models->global_font_id = 1;
    app_hardcode_default_style(models);
    
    // NOTE(allen): title space
    models->has_new_title = true;
    models->title_capacity = KB(4);
    models->title_space = push_array(partition, char, models->title_capacity);
    {
        String builder = make_string_cap(models->title_space, 0, models->title_capacity);
        append(&builder, WINDOW_NAME);
        terminate_with_null(&builder);
    }
    
    // NOTE(allen): init system context
    models->system = system;
    models->vars = vars;
    
    // NOTE(allen): init first panel
    File_Init init_files[] = {
        { make_lit_string("*messages*"), &models->message_buffer, true , },
        { make_lit_string("*scratch*"),  &models->scratch_buffer, false, },
    };
    
    Heap *heap = &models->mem.heap;
    for (i32 i = 0; i < ArrayCount(init_files); ++i){
        Editing_File *file = working_set_alloc_always(&models->working_set, heap, &models->lifetime_allocator);
        buffer_bind_name(models, heap, partition, &models->working_set, file, init_files[i].name);
        
        if (init_files[i].read_only){
            init_read_only_file(system, models, file);
        }
        else{
            init_normal_file(system, models, 0, 0, file);
        }
        
        file->settings.never_kill = true;
        file_set_unimportant(file, true);
        file->settings.unwrapped_lines = true;
        
        if (init_files[i].ptr != 0){
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
    App_Vars *vars = (App_Vars*)memory->vars_memory;
    Models *models = &vars->models;
    
    Application_Step_Result app_result = {};
    app_result.mouse_cursor_type = APP_MOUSE_CURSOR_DEFAULT;
    app_result.lctrl_lalt_is_altgr = models->settings.lctrl_lalt_is_altgr;
    
    // NOTE(allen): OS clipboard event handling
    String clipboard = input->clipboard;
    if (clipboard.str != 0){
        String *dest = working_set_next_clipboard_string(&models->mem.heap, &models->working_set, clipboard.size);
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
            Editing_File_Name canon = {};
            if (get_canon_name(system, make_string(buffer, size), &canon)){
                Editing_File *file = working_set_contains_canon(working_set, canon.name);
                if (file != 0){
                    if (file->state.ignore_behind_os == 0){
                        file_set_dirty_flag(file, DirtyState_UnloadedChanges);
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
    i32 prev_width = models->layout.full_width;
    i32 prev_height = models->layout.full_height;
    i32 current_width = target->width;
    i32 current_height = target->height;
    {
        models->layout.full_width = current_width;
        models->layout.full_height = current_height;
        
        if (prev_width != current_width || prev_height != current_height){
            layout_refit(&models->layout, prev_width, prev_height);
        }
    }
    
    // NOTE(allen): First frame initialization
    if (input->first_step){
        // Open command line files.
        char space[512];
        String cl_filename = make_fixed_width_string(space);
        copy_ss(&cl_filename, models->hot_directory.string);
        i32 cl_filename_len = cl_filename.size;
        for (i32 i = 0; i < models->settings.init_files_count; ++i){
            cl_filename.size = cl_filename_len;
            
            String filename = {};
            Editing_File_Name canon_name = {};
            if (get_canon_name(system, make_string_slowly(models->settings.init_files[i]),
                               &canon_name)){
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
    }
    
    // NOTE(allen): update child processes
    f32 dt = input->dt;
    if (dt > 0){
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
            
            b32 edited_file = false;
            u32 amount = 0;
            system->cli_begin_update(cli);
            if (system->cli_update_step(cli, dest, max, &amount)){
                if (file != 0 && amount > 0){
                    amount = eol_in_place_convert_in(dest, amount);
                    output_file_append(system, models, file, make_string(dest, amount));
                    edited_file = true;
                }
            }
            
            if (system->cli_end_update(cli)){
                if (file != 0){
                    char str_space[256];
                    String str = make_fixed_width_string(str_space);
                    append(&str, make_lit_string("exited with code "));
                    append_int_to_str(&str, cli->exit);
                    output_file_append(system, models, file, str);
                    edited_file = true;
                }
                procs_to_free[proc_free_count++] = proc_ptr;
            }
            
            if (proc_ptr->cursor_at_end && file != 0){
                file_cursor_to_end(system, models, file);
            }
        }
        
        for (i32 i = proc_free_count - 1; i >= 0; --i){
            cli_list_free_proc(list, procs_to_free[i]);
        }
        
        end_temp_memory(temp);
    }
    
    // NOTE(allen): init event context
    models->input = input;
    
    // NOTE(allen): input filter and simulated events
    if (models->input_filter != 0){
        models->input_filter(&input->mouse);
    }
    
    Key_Event_Data mouse_event = {};
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
    
    if (input->mouse.wheel != 0){
        mouse_event.keycode = key_mouse_wheel;
        input->keys.keys[input->keys.count++] = mouse_event;
    }
    
    if (input->mouse.x != models->prev_x || input->mouse.y != models->prev_y){
        b32 was_in_window = hit_check(models->prev_x, models->prev_y, i32R(0, 0, prev_width, prev_height));
        b32 is_in_window  = hit_check(input->mouse.x, input->mouse.y, i32R(0, 0, current_width, current_height));
        if (is_in_window || was_in_window){
            mouse_event.keycode = key_mouse_move;
            input->keys.keys[input->keys.count++] = mouse_event;
        }
    }
    
    if (models->animated_last_frame){
        mouse_event.keycode = key_animate;
        input->keys.keys[input->keys.count++] = mouse_event;
    }
    
    // NOTE(allen): mouse hover status
    Panel *mouse_panel = 0;
    b32 mouse_in_edit_area = false;
    b32 mouse_in_margin_area = false;
    b32 mouse_on_divider = false;
    b32 mouse_divider_vertical = false;
    i32 mouse_divider_id = 0;
    i32 mouse_divider_side = 0;
    
    i32 mx = input->mouse.x;
    i32 my = input->mouse.y;
    for (Panel *panel = models->layout.used_sentinel.next;
         panel != &models->layout.used_sentinel;
         panel = panel->next){
        if (hit_check(mx, my, panel->inner)){
            mouse_panel = panel;
            mouse_in_edit_area = true;
        }
        else if (hit_check(mx, my, panel->full)){
            mouse_panel = panel;
            mouse_in_margin_area = true;
            
            if (mx >= panel->inner.x0 && mx < panel->inner.x1){
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
                    Divider_And_ID div = layout_get_divider(&models->layout, mouse_divider_id);
                    if (which_child == mouse_divider_side && div.divider->v_divider == mouse_divider_vertical){
                        mouse_on_divider = true;
                        break;
                    }
                    if (mouse_divider_id == models->layout.root){
                        break;
                    }
                    mouse_divider_id = div.divider->parent;
                    which_child = div.divider->which_child;
                }
                
            }
            else{
                mouse_on_divider = false;
                mouse_divider_id = 0;
            }
        }
        
        if (mouse_panel != 0){
            break;
        }
    }
    
    // NOTE(allen): consume event stream
    Key_Event_Data *key_ptr = input->keys.keys;
    Key_Event_Data *key_end = key_ptr + input->keys.count;
    for (;key_ptr < key_end; key_ptr += 1){
        Panel *active_panel = &models->layout.panels[models->layout.active_panel];
        View *view = active_panel->view;
        Assert(view != 0);
        
        switch (vars->state){
            case APP_STATE_EDIT:
            {
                Key_Code keycode = key_ptr->keycode;
                
                enum{
                    EventConsume_None,
                    EventConsume_BeginResize,
                    EventConsume_ClickChangeView,
                    EventConsume_Command,
                };
                i32 event_consume_mode = EventConsume_Command;
                if (keycode == key_mouse_left && input->mouse.press_l && mouse_on_divider){
                    event_consume_mode = EventConsume_BeginResize;
                }
                else if (keycode == key_mouse_left && input->mouse.press_l && mouse_panel != 0 && mouse_panel != active_panel){
                    event_consume_mode = EventConsume_ClickChangeView;
                }
                
                switch (event_consume_mode){
                    case EventConsume_BeginResize:
                    {
                        vars->state = APP_STATE_RESIZING;
                        Divider_And_ID div = layout_get_divider(&models->layout, mouse_divider_id);
                        vars->resizing.divider = div.divider;
                        
                        f32 min = 0;
                        f32 max = 0;
                        if (mouse_divider_vertical){
                            max = (f32)models->layout.full_width;
                        }
                        else{
                            max = (f32)models->layout.full_height;
                        }
                        f32 mid = layout_get_position(&models->layout, mouse_divider_id);
                        
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
                        for (;top > 0;){
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
                    }break;
                    
                    case EventConsume_ClickChangeView:
                    {
                        // NOTE(allen): kill coroutine if we have one
                        if (models->command_coroutine != 0){
                            force_abort_coroutine(system, models, view);
                        }
                        
                        // NOTE(allen): run deactivate command
                        launch_command_via_keycode(system, &app_result, models, view, key_click_deactivate_view);
                        
                        // NOTE(allen): kill coroutine if we have one (again because we just launched a command)
                        if (models->command_coroutine != 0){
                            force_abort_coroutine(system, models, view);
                        }
                        
                        models->layout.active_panel = (i32)(mouse_panel - models->layout.panels);
                        app_result.animating = true;
                        active_panel = mouse_panel;
                        view = active_panel->view;
                        
                        // NOTE(allen): run activate command
                        launch_command_via_keycode(system, &app_result, models, view, key_click_activate_view);
                    }break;
                    
                    case EventConsume_Command:
                    {
                        
                        // NOTE(allen): update command coroutine
                        if (models->command_coroutine != 0){
                            models->key = *key_ptr;
                            
                            Coroutine_Head *command_coroutine = models->command_coroutine;
                            u32 abort_flags = models->command_coroutine_flags[1];
                            u32 get_flags = models->command_coroutine_flags[0] | abort_flags;
                            
                            u32 event_flags = get_event_flags(key_ptr->keycode);
                            if ((get_flags & event_flags) != 0){
                                i32 map = view_get_map(view);
                                Command_Binding cmd_bind = map_extract_recursive(&models->mapping, map, *key_ptr);
                                
                                User_Input user_in = {};
                                user_in.key = *key_ptr;
                                user_in.command.command = cmd_bind.custom;
                                user_in.abort = ((abort_flags & event_flags) != 0);
                                models->command_coroutine =  app_resume_coroutine(system, &models->app_links, Co_Command, command_coroutine, &user_in, models->command_coroutine_flags);
                                
                                app_result.animating = true;
                                if (models->command_coroutine == 0){
                                    init_query_set(&view->transient.query_set);
                                }
                            }
                        }
                        
                        // NOTE(allen): launch command
                        else{
                            launch_command_via_event(system, &app_result, models, view, *key_ptr);
                        }
                    }break;
                }
                
            }break;
            
            case APP_STATE_RESIZING:
            {
                Key_Code keycode = key_ptr->keycode;
                u32 event_flags = get_event_flags(keycode);
                if (event_flags & EventOnAnyKey || keycode == key_mouse_left_release){
                    vars->state = APP_STATE_EDIT;
                }
                else if (keycode == key_mouse_move){
                    if (input->mouse.l){
                        Panel_Divider *divider = vars->resizing.divider;
                        i32 mouse_position = 0;
                        
                        i32 absolute_positions[MAX_VIEWS];
                        i32 min = 0;
                        i32 max = 0;
                        i32 div_id = (i32)(divider - models->layout.dividers);
                        
                        layout_compute_absolute_positions(&models->layout, absolute_positions);
                        mouse_position = (divider->v_divider)?(mx):(my);
                        layout_get_min_max(&models->layout, divider, absolute_positions, &min, &max);
                        absolute_positions[div_id] = clamp(min, mouse_position, max);
                        layout_update_all_positions(&models->layout, absolute_positions);
                        
                        layout_fix_all_panels(&models->layout);
                    }
                    else{
                        vars->state = APP_STATE_EDIT;
                    }
                }
            }break;
        }
    }
    
    // NOTE(allen): send panel size update
    if (models->layout.panel_state_dirty && models->hooks[hook_view_size_change] != 0){
        models->layout.panel_state_dirty = false;
        models->hooks[hook_view_size_change](&models->app_links);
    }
    
    // NOTE(allen): step panels
    {
        Panel *active_panel = &models->layout.panels[models->layout.active_panel];
        
        for (Panel *panel = models->layout.used_sentinel.next;
             panel != &models->layout.used_sentinel;
             panel = panel->next){
            View *view = panel->view;
            
            GUI_Scroll_Vars *scroll_vars = 0;
            i32 max_y = 0;
            b32 file_scroll = false;
            if (!view->transient.ui_mode){
                scroll_vars = &view->transient.edit_pos->scroll;
                max_y = view_compute_max_target_y(view);
                file_scroll = true;
            }
            else{
                scroll_vars = &view->transient.ui_scroll;
                i32 bottom = view->transient.ui_control.bounding_box[UICoordinates_Scrolled].y1;
                max_y = view_compute_max_target_y_from_bottom_y(view, (f32)bottom);
                file_scroll = false;
            }
            
            b32 active = (panel == active_panel);
            Input_Process_Result ip_result = do_step_file_view(system, view, models, panel->inner, active, dt, *scroll_vars, max_y);
            
            if (ip_result.is_animating){
                app_result.animating = true;
            }
            
            if (memcmp(scroll_vars, &ip_result.scroll, sizeof(*scroll_vars)) != 0){
                if (file_scroll){
                    view_set_scroll(system, view, ip_result.scroll);
                }
                else{
                    *scroll_vars = ip_result.scroll;
                }
            }
        }
    }
    
    // NOTE(allen): on the first frame there should be no scrolling
    if (input->first_step){
        for (Panel *panel = models->layout.used_sentinel.next;
             panel != &models->layout.used_sentinel;
             panel = panel->next){
            View *view = panel->view;
            if (view->transient.edit_pos != 0){
                GUI_Scroll_Vars *scroll_vars = &view->transient.edit_pos->scroll;
                scroll_vars->scroll_x = (f32)scroll_vars->target_x;
                scroll_vars->scroll_y = (f32)scroll_vars->target_y;
            }
        }
    }
    
    // NOTE(allen): If the exit signal has been sent, run the exit hook.
    if (input->trying_to_kill){
        models->keep_playing = false;
    }
    if (!models->keep_playing){
        Hook_Function *exit_func = models->hooks[hook_exit];
        if (exit_func != 0){
            if (exit_func(&models->app_links) == 0){
                app_result.animating = true;
                models->keep_playing = true;
            }
        }
    }
    
    // NOTE(allen): rendering
    {
        begin_render_section(target, system);
        
        Panel *active_panel = &models->layout.panels[models->layout.active_panel];
        View *active_view = active_panel->view;
        
        // NOTE(allen): render the panels
        for (Panel *panel = models->layout.used_sentinel.next;
             panel != &models->layout.used_sentinel;
             panel = panel->next){
            i32_Rect full = panel->full;
            i32_Rect inner = panel->inner;
            
            View *view = panel->view;
            Style *style = &models->styles.styles[0];
            
            b32 active = (panel == active_panel);
            u32 back_color = style->main.back_color;
            draw_rectangle(target, full, back_color);
            
            GUI_Scroll_Vars *scroll_vars = &view->transient.edit_pos->scroll;
            
            do_render_file_view(system, view, models, scroll_vars, active_view, panel->inner, active, target);
            
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
    
    // NOTE(allen): get new window title
    if (models->has_new_title){
        models->has_new_title = false;
        app_result.has_new_title = true;
        app_result.title_string = models->title_space;
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
    
    // NOTE(allen): Update Frame to Frame States
    models->prev_x = input->mouse.x;
    models->prev_y = input->mouse.y;
    models->animated_last_frame = app_result.animating;
    models->frame_counter += 1;
    
    // end-of-app_step
    return(app_result);
}

extern "C" App_Get_Functions_Sig(app_get_functions){
    App_Functions result = {};
    
    result.read_command_line = app_read_command_line;
    result.init = app_init;
    result.step = app_step;
    
    return(result);
}

// BOTTOM

