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
        Edit edit = {};
        edit.str = value.str;
        edit.length = value.size;
        edit.range.first = end;
        edit.range.one_past_last = end;
        
        Edit_Behaviors behaviors = {};
        edit_single(system, models, file, edit, behaviors);
    }
}

internal void
file_cursor_to_end(System_Functions *system, Models *models, Editing_File *file){
    Assert(file != 0);
    i32 pos = buffer_size(&file->state.buffer);
    Layout *layout = &models->layout;
    for (Panel *panel = layout_get_first_open_panel(layout);
         panel != 0;
         panel = layout_get_next_open_panel(layout, panel)){
        View *view = panel->view;
        if (view->file != file){
            continue;
        }
        Full_Cursor cursor = file_compute_cursor(system, file, seek_pos(pos));
        view_set_cursor(system, models, view, cursor, true);
        view->mark = cursor.pos;
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

#define REQ_FILE(n,v) Editing_File *n = (v)->file_data.file; if (n == 0) return
#define REQ_FILE_HISTORY(n,v) Editing_File *n = (v)->file_data.file; if (n == 0 || n->state.undo.undo.edits == 0) return

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
    // TODO(allen): do(fix the weird built-in-ness of the ui map)
    u8 mdfr_array[] = {MDFR_NONE, MDFR_SHIFT, MDFR_CTRL, MDFR_SHIFT | MDFR_CTRL};
    for (i32 i = 0; i < 4; ++i){
        u8 mdfr = mdfr_array[i];
        map_add(commands, key_left , mdfr, (Custom_Command_Function*)0);
        map_add(commands, key_right, mdfr, (Custom_Command_Function*)0);
        map_add(commands, key_up   , mdfr, (Custom_Command_Function*)0);
        map_add(commands, key_down , mdfr, (Custom_Command_Function*)0);
        map_add(commands, key_back , mdfr, (Custom_Command_Function*)0);
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
    models->hook_file_edit_range = 0;
    models->hook_file_edit_finished = 0;
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
                
                case unit_callback:
                {
                    if (map_ptr != 0){
                        Custom_Command_Function *custom = unit->callback.func;
                        if (unit->callback.code == 0){
                            u32 index = 0;
                            if (map_get_modifiers_hash(unit->callback.modifiers, &index)){
                                map_ptr->vanilla_keyboard_default[index].custom = custom;
                            }
                        }
                        else{
                            map_add(map_ptr, unit->callback.code, unit->callback.modifiers, custom);
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
                                
                                case special_hook_file_edit_range:
                                {
                                    models->hook_file_edit_range = (File_Edit_Range_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_file_edit_finished:
                                {
                                    models->hook_file_edit_finished = (File_Edit_Finished_Function*)unit->hook.func;
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
                                
                                case special_hook_modify_color_table:
                                {
                                    models->modify_color_table = (Modify_Color_Table_Function*)unit->hook.func;
                                }break;
                                
								case special_hook_clipboard_change:
                                {
                                    models->clipboard_change = (Clipboard_Change_Hook_Function*)unit->hook.func;
                                }break;
								
                                case special_hook_get_view_buffer_region:
                                {
                                    models->get_view_buffer_region = (Get_View_Buffer_Region_Function*)unit->hook.func;
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
        // TODO(allen): do(Error report: bad binding units map.)
        // TODO(allen): do(no bindings set recovery plan.)
        InvalidCodePath;
    }
    
    Mapping old_mapping = models->mapping;
    if (old_mapping.memory != 0){
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
    if (models->command_caller != 0){
        Generic_Command generic = {};
        generic.command = cmd_in->bind.custom;
        models->command_caller(&models->app_links, generic);
    }
    else{
        cmd_in->bind.custom(&models->app_links);
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

// App Functions

internal void
fill_hardcode_default_style(Color_Table color_table){
    color_table.vals[Stag_Back]                  = 0xFF0C0C0C;
    color_table.vals[Stag_Margin]                = 0xFF181818;
    color_table.vals[Stag_Margin_Hover]          = 0xFF252525;
    color_table.vals[Stag_Margin_Active]         = 0xFF323232;
    color_table.vals[Stag_List_Item]             = color_table.vals[Stag_Margin];
    color_table.vals[Stag_List_Item_Hover]       = color_table.vals[Stag_Margin_Hover];
    color_table.vals[Stag_List_Item_Active]      = color_table.vals[Stag_Margin_Active];
    color_table.vals[Stag_Cursor]                = 0xFF00EE00;
    color_table.vals[Stag_Highlight]             = 0xFFDDEE00;
    color_table.vals[Stag_Mark]                  = 0xFF494949;
    color_table.vals[Stag_Default]               = 0xFF90B080;
    color_table.vals[Stag_At_Cursor]             = color_table.vals[Stag_Back];
    color_table.vals[Stag_Highlight_Cursor_Line] = 0xFF1E1E1E;
    color_table.vals[Stag_At_Highlight]          = 0xFFFF44DD;
    color_table.vals[Stag_Comment]               = 0xFF2090F0;
    color_table.vals[Stag_Keyword]               = 0xFFD08F20;
    color_table.vals[Stag_Str_Constant]          = 0xFF50FF30;
    color_table.vals[Stag_Char_Constant]         = color_table.vals[Stag_Str_Constant];
    color_table.vals[Stag_Int_Constant]          = color_table.vals[Stag_Str_Constant];
    color_table.vals[Stag_Float_Constant]        = color_table.vals[Stag_Str_Constant];
    color_table.vals[Stag_Bool_Constant]         = color_table.vals[Stag_Str_Constant];
    color_table.vals[Stag_Include]               = color_table.vals[Stag_Str_Constant];
    color_table.vals[Stag_Preproc]               = color_table.vals[Stag_Default];
    color_table.vals[Stag_Special_Character]     = 0xFFFF0000;
    color_table.vals[Stag_Ghost_Character]       = color_blend(color_table.vals[Stag_Default],
                                                               0.5f,
                                                               color_table.vals[Stag_Back]);
    
    color_table.vals[Stag_Paste] = 0xFFDDEE00;
    color_table.vals[Stag_Undo]  = 0xFF00DDEE;
    
    color_table.vals[Stag_Highlight_Junk]  = 0xff3a0000;
    color_table.vals[Stag_Highlight_White] = 0xff003a3a;
    
    color_table.vals[Stag_Bar]        = 0xFF888888;
    color_table.vals[Stag_Base]       = 0xFF000000;
    color_table.vals[Stag_Pop1]       = 0xFF3C57DC;
    color_table.vals[Stag_Pop2]       = 0xFFFF0000;
    
    color_table.vals[Stag_Back_Cycle_1] = 0x10A00000;
    color_table.vals[Stag_Back_Cycle_2] = 0x0C00A000;
    color_table.vals[Stag_Back_Cycle_3] = 0x0C0000A0;
    color_table.vals[Stag_Back_Cycle_4] = 0x0CA0A000;
    color_table.vals[Stag_Text_Cycle_1] = 0xFFA00000;
    color_table.vals[Stag_Text_Cycle_2] = 0xFF00A000;
    color_table.vals[Stag_Text_Cycle_3] = 0xFF0030B0;
    color_table.vals[Stag_Text_Cycle_4] = 0xFFA0A000;
}

internal void
app_hardcode_default_style(Models *models){
    Partition *part = &models->mem.part;
    Color_Table color_table = {};
    color_table.vals = push_array(part, u32, Stag_COUNT);
    color_table.count = Stag_COUNT;
    fill_hardcode_default_style(color_table);
    models->fallback_color_table = color_table;
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
    init_query_set(&view->query_set);
}

internal void
launch_command_via_event(System_Functions *system, Models *models, View *view, Key_Event_Data event){
    models->key = event;
    
    i32 map = view_get_map(view);
    Command_Binding cmd_bind = map_extract_recursive(&models->mapping, map, event);
    
    if (cmd_bind.custom != 0){
        Assert(models->command_coroutine == 0);
        Coroutine_Head *command_coroutine = system->create_coroutine(command_caller);
        models->command_coroutine = command_coroutine;
        
        Command_In cmd_in = {};
        cmd_in.models = models;
        cmd_in.bind = cmd_bind;
        
        models->command_coroutine = app_launch_coroutine(system, &models->app_links, Co_Command, models->command_coroutine, &cmd_in, models->command_coroutine_flags);
        
        models->prev_command = cmd_bind;
        if (event.keycode != key_animate){
            models->animate_next_frame = true;
        }
    }
}

internal void
launch_command_via_keycode(System_Functions *system, Models *models, View *view, Key_Code keycode){
    Key_Event_Data event = {};
    event.keycode = keycode;
    launch_command_via_event(system, models, view, event);
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
    
    Partition *part = &models->mem.part;
    
    // NOTE(allen): live set
    {
        models->live_set.count = 0;
        models->live_set.max = MAX_VIEWS;
        models->live_set.views = push_array(part, View, models->live_set.max);
        
        //dll_init_sentinel
        models->live_set.free_sentinel.next = &models->live_set.free_sentinel;
        models->live_set.free_sentinel.prev = &models->live_set.free_sentinel;
        
        i32 max = models->live_set.max;
        View *view = models->live_set.views;
        for (i32 i = 0; i < max; ++i, ++view){
            //dll_insert(&models->live_set.free_sentinel, view);
            view->next = models->live_set.free_sentinel.next;
            view->prev = &models->live_set.free_sentinel;
            models->live_set.free_sentinel.next = view;
            view->next->prev = view;
        }
    }
    
    {
        umem memsize = KB(8);
        void *mem = push_array(part, u8, (i32)memsize);
        parse_context_init_memory(&models->parse_context_memory, mem, memsize);
        parse_context_add_default(&models->parse_context_memory, &models->mem.heap);
    }
    
    {
        Assert(models->config_api.get_bindings != 0);
        i32 wanted_size = models->config_api.get_bindings(models->app_links.memory, models->app_links.memory_size);
        Assert(wanted_size <= models->app_links.memory_size);
        interpret_binding_buffer(models, models->app_links.memory, wanted_size);
        memset(models->app_links.memory, 0, wanted_size);
    }
    
    dynamic_variables_init(&models->variable_layout);
    dynamic_workspace_init(&models->mem.heap, &models->lifetime_allocator, DynamicWorkspace_Global, 0, &models->dynamic_workspace);
    
    // NOTE(allen): file setup
    working_set_init(system, &models->working_set, part, &vars->models.mem.heap);
    models->working_set.default_display_width = DEFAULT_DISPLAY_WIDTH;
    models->working_set.default_minimum_base_display_width = DEFAULT_MINIMUM_BASE_DISPLAY_WIDTH;
    
    // NOTE(allen): 
    global_history_init(&models->global_history);
    text_layout_init(&models->app_links, &models->text_layouts);
    
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
    models->title_space = push_array(part, char, models->title_capacity);
    {
        String builder = make_string_cap(models->title_space, 0, models->title_capacity);
        append(&builder, WINDOW_NAME);
        terminate_with_null(&builder);
    }
    
    // NOTE(allen): init system context
    models->system = system;
    models->vars = vars;
    
    // NOTE(allen): init baked in buffers
    File_Init init_files[] = {
        { make_lit_string("*messages*"), &models->message_buffer, true , },
        { make_lit_string("*scratch*"),  &models->scratch_buffer, false, },
    };
    
    Heap *heap = &models->mem.heap;
    for (i32 i = 0; i < ArrayCount(init_files); ++i){
        Editing_File *file = working_set_alloc_always(&models->working_set, heap, &models->lifetime_allocator);
        buffer_bind_name(models, heap, part, &models->working_set, file, init_files[i].name);
        
        if (init_files[i].ptr != 0){
            *init_files[i].ptr = file;
        }
        
        if (init_files[i].read_only){
            init_read_only_file(system, models, file);
        }
        else{
            File_Attributes attributes = {};
            init_normal_file(system, models, 0, 0, attributes, file);
        }
        
        file->settings.never_kill = true;
        file_set_unimportant(file, true);
        file->settings.unwrapped_lines = true;
    }
    
    // NOTE(allen): setup first panel
    {
        Panel *panel = layout_initialize(part, &models->layout);
        View *new_view = live_set_alloc_view(&models->mem.heap, &models->lifetime_allocator, &models->live_set, panel);
        view_set_file(system, models, new_view, models->scratch_buffer);
    }
    
    // NOTE(allen): miscellaneous init
    hot_directory_init(&models->hot_directory, current_directory);
    child_process_container_init(&models->child_processes, models);
    models->user_up_key = key_up;
    models->user_down_key = key_down;
    models->period_wakeup_timer = system->wake_up_timer_create();
    models->custom_layer_arena = make_arena(&models->app_links);
}

App_Step_Sig(app_step){
    App_Vars *vars = (App_Vars*)memory->vars_memory;
    Models *models = &vars->models;
    
    models->next_animate_delay = max_u32;
    models->animate_next_frame = false;
    
    // NOTE(allen): per-frame update of models state
    begin_frame(target);
    models->target = target;
    models->input = input;
    
    // NOTE(allen): OS clipboard event handling
    String clipboard = input->clipboard;
    if (clipboard.str != 0){
        String *dest = working_set_next_clipboard_string(&models->mem.heap, &models->working_set, clipboard.size);
        dest->size = eol_convert_in(dest->str, clipboard.str, clipboard.size);
        if (input->clipboard_changed && models->clipboard_change != 0){
            models->clipboard_change(&models->app_links, *dest, ClipboardFlag_FromOS);
        }
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
                        file_add_dirty_flag(file, DirtyState_UnloadedChanges);
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
    Vec2_i32 prev_dim = layout_get_root_size(&models->layout);
    Vec2_i32 current_dim = V2i32(target->width, target->height);
    layout_set_root_size(&models->layout, current_dim);
    
    // NOTE(allen): update child processes
    f32 dt = input->dt;
    if (dt > 0){
        Partition *scratch = &models->mem.part;
        Child_Process_Container *child_processes = &models->child_processes;
        
        Temp_Memory temp = begin_temp_memory(scratch);
        Child_Process **processes_to_free = push_array(scratch, Child_Process*, child_processes->active_child_process_count);
        i32 processes_to_free_count = 0;
        
        u32 max = KB(128);
        char *dest = push_array(scratch, char, max);
        
        for (Node *node = child_processes->child_process_active_list.next;
             node != &child_processes->child_process_active_list;
             node = node->next){
            Child_Process *child_process = CastFromMember(Child_Process, node, node);
            
            Editing_File *file = child_process->out_file;
            CLI_Handles *cli = &child_process->cli;
            
            // TODO(allen): do(call a 'child process updated hook' let that hook populate the buffer if it so chooses)
            
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
                processes_to_free[processes_to_free_count++] = child_process;
                child_process_set_return_code(models, child_processes, child_process->id, cli->exit);
            }
            
            if (child_process->cursor_at_end && file != 0){
                file_cursor_to_end(system, models, file);
            }
        }
        
        for (i32 i = 0; i < processes_to_free_count; ++i){
            child_process_free(child_processes, processes_to_free[i]->id);
        }
        
        end_temp_memory(temp);
    }
    
    // NOTE(allen): input filter and simulated events
    if (models->input_filter != 0){
        models->input_filter(&input->mouse);
    }
    
    Key_Event_Data mouse_event = {};
    block_copy(mouse_event.modifiers, input->keys.modifiers, sizeof(mouse_event.modifiers));
    
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
    
    if (input->mouse.p != models->prev_p){
        b32 was_in_window = hit_check(i32R(0, 0, prev_dim.x, prev_dim.y), models->prev_p);
        b32 is_in_window  = hit_check(i32R(0, 0, current_dim.x, current_dim.y), input->mouse.p);
        if (is_in_window || was_in_window){
            mouse_event.keycode = key_mouse_move;
            input->keys.keys[input->keys.count++] = mouse_event;
        }
    }
    
    if (models->animated_last_frame){
        mouse_event.keycode = key_animate;
        input->keys.keys[input->keys.count++] = mouse_event;
    }
    
    // NOTE(allen): expose layout
    Layout *layout = &models->layout;
    
    // NOTE(allen): mouse hover status
    Panel *mouse_panel = 0;
    Panel *divider_panel = 0;
    b32 mouse_in_margin = false;
    Vec2_i32 mouse = input->mouse.p;
    {
        for (Panel *panel = layout_get_first_open_panel(layout);
             panel != 0;
             panel = layout_get_next_open_panel(layout, panel)){
            if (hit_check(mouse.x, mouse.y, panel->rect_full)){
                mouse_panel = panel;
                if (!hit_check(mouse.x, mouse.y, panel->rect_inner)){
                    mouse_in_margin = true;
                    for (divider_panel = mouse_panel->parent;
                         divider_panel != 0;
                         divider_panel = divider_panel->parent){
                        if (hit_check(mouse.x, mouse.y, divider_panel->rect_inner)){
                            break;
                        }
                    }
                }
            }
            if (mouse_panel != 0){
                break;
            }
        }
    }
    
    // NOTE(allen): First frame initialization
    if (input->first_step){
        if (models->hook_start != 0){
            char **files = models->settings.init_files;
            i32 files_count = models->settings.init_files_count;
            
            char **flags = models->settings.custom_flags;
            i32 flags_count = models->settings.custom_flags_count;
            
            models->hook_start(&models->app_links, files, files_count, flags, flags_count);
        }
        
#if 0
        // Open command line files.
        char space[512];
        String cl_file_name = make_fixed_width_string(space);
        copy_ss(&cl_file_name, models->hot_directory.string);
        i32 cl_file_name_len = cl_file_name.size;
        for (i32 i = 0; i < models->settings.init_files_count; ++i){
            cl_file_name.size = cl_file_name_len;
            
            String file_name = {};
            Editing_File_Name canon_name = {};
            if (get_canon_name(system, make_string_slowly(models->settings.init_files[i]), &canon_name)){
                file_name = canon_name.name;
            }
            else{
                append_sc(&cl_file_name, models->settings.init_files[i]);
                file_name = cl_file_name;
            }
            
            Buffer_ID id = 0;
            create_buffer(&models->app_links, file_name, 0, &id);
        }
#endif
    }
    
    // NOTE(allen): consume event stream
    Key_Event_Data *key_ptr = input->keys.keys;
    Key_Event_Data *key_end = key_ptr + input->keys.count;
    for (;key_ptr < key_end; key_ptr += 1){
        Panel *active_panel = layout_get_active_panel(layout);
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
                if (keycode == key_mouse_left && input->mouse.press_l && (divider_panel != 0)){
                    event_consume_mode = EventConsume_BeginResize;
                }
                else if (keycode == key_mouse_left && input->mouse.press_l && mouse_panel != 0 && mouse_panel != active_panel){
                    event_consume_mode = EventConsume_ClickChangeView;
                }
                
                switch (event_consume_mode){
                    case EventConsume_BeginResize:
                    {
                        vars->state = APP_STATE_RESIZING;
                        models->resizing_intermediate_panel = divider_panel;
                    }break;
                    
                    case EventConsume_ClickChangeView:
                    {
                        // NOTE(allen): kill coroutine if we have one
                        if (models->command_coroutine != 0){
                            force_abort_coroutine(system, models, view);
                        }
                        
                        // NOTE(allen): run deactivate command
                        launch_command_via_keycode(system, models, view, key_click_deactivate_view);
                        
                        // NOTE(allen): kill coroutine if we have one (again because we just launched a command)
                        if (models->command_coroutine != 0){
                            force_abort_coroutine(system, models, view);
                        }
                        
                        layout->active_panel = mouse_panel;
                        models->animate_next_frame = true;
                        active_panel = mouse_panel;
                        view = active_panel->view;
                        
                        // NOTE(allen): run activate command
                        launch_command_via_keycode(system, models, view, key_click_activate_view);
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
                                
                                if (user_in.key.keycode != key_animate){
                                    models->animate_next_frame = true;
                                }
                                
                                if (models->command_coroutine == 0){
                                    init_query_set(&view->query_set);
                                }
                            }
                        }
                        
                        // NOTE(allen): launch command
                        else{
                            launch_command_via_event(system, models, view, *key_ptr);
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
                        Panel *split = models->resizing_intermediate_panel;
                        Range limits = layout_get_limiting_range_on_split(layout, split);
                        i32 mouse_position = (split->vertical_split)?(mouse.x):(mouse.y);
                        mouse_position = clamp(limits.min, mouse_position, limits.max);
                        layout_set_split_absolute_position(layout, split, mouse_position);
                    }
                    else{
                        vars->state = APP_STATE_EDIT;
                    }
                }
            }break;
        }
    }
    
    // NOTE(allen): send panel size update
    if (models->layout.panel_state_dirty && models->hooks[hook_buffer_viewer_update] != 0){
        models->layout.panel_state_dirty = false;
        models->hooks[hook_buffer_viewer_update](&models->app_links);
    }
    
    // NOTE(allen): step panels
    {
        Panel *active_panel = layout_get_active_panel(layout);
        
        for (Panel *panel = layout_get_first_open_panel(layout);
             panel != 0;
             panel = layout_get_next_open_panel(layout, panel)){
            View *view = panel->view;
            
            GUI_Scroll_Vars *scroll_vars = 0;
            b32 file_scroll = false;
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            if (!view->ui_mode){
                scroll_vars = &edit_pos.scroll;
                file_scroll = true;
            }
            else{
                scroll_vars = &view->ui_scroll;
                file_scroll = false;
            }
            
            b32 active = (panel == active_panel);
            GUI_Scroll_Vars new_scroll = do_step_file_view(system, models, view, panel->rect_inner, active, dt, *scroll_vars);
            
            if (memcmp(scroll_vars, &new_scroll, sizeof(*scroll_vars)) != 0){
                if (file_scroll){
                    view_set_scroll(system, models, view, new_scroll);
                }
                else{
                    *scroll_vars = new_scroll;
                }
            }
        }
    }
    
    // NOTE(allen): on the first frame there should be no scrolling
    if (input->first_step){
        for (Panel *panel = layout_get_first_open_panel(layout);
             panel != 0;
             panel = layout_get_next_open_panel(layout, panel)){
            View *view = panel->view;
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            edit_pos.scroll.scroll_x = (f32)edit_pos.scroll.target_x;
            edit_pos.scroll.scroll_y = (f32)edit_pos.scroll.target_y;
            view_set_edit_pos(view, edit_pos);
        }
    }
    
    // NOTE(allen): hook for files marked "edit finished"
    {
        File_Edit_Finished_Function *hook_file_edit_finished = models->hook_file_edit_finished;
        if (hook_file_edit_finished != 0){
            Working_Set *working_set = &models->working_set;
            if (working_set->edit_finished_list.next != &working_set->edit_finished_list){
                b32 trigger_hook = false;
                
                u32 elapse_time = models->edit_finished_hook_repeat_speed;
                if (elapse_time != 0){
                    trigger_hook = true;
                }
                else if (working_set->time_of_next_edit_finished_signal == 0){
                    u32 trigger_window_length = 0;
                    if (elapse_time > 5){
                        trigger_window_length = elapse_time - 5;
                    }
                    u64 trigger_time = system->now_time() + (u64)trigger_window_length*1000LLU;
                    working_set->time_of_next_edit_finished_signal = trigger_time;
                    system->wake_up_timer_set(working_set->edit_finished_timer, elapse_time);
                }
                else if (system->now_time() >= working_set->time_of_next_edit_finished_signal){
                    trigger_hook = true;
                }
                if (trigger_hook){
                    Partition *scratch = &models->mem.part;
                    
                    Temp_Memory temp = begin_temp_memory(scratch);
                    Node *first = working_set->edit_finished_list.next;
                    Node *stop = &working_set->edit_finished_list;
                    
                    Editing_File **file_ptrs = push_array(scratch, Editing_File*, 0);
                    for (Node *node = first;
                         node != stop;
                         node = node->next){
                        Editing_File **file_ptr = push_array(scratch, Editing_File*, 1);
                        *file_ptr = CastFromMember(Editing_File, edit_finished_mark_node, node);
                    }
                    i32 id_count = (i32)(push_array(scratch, Editing_File*, 0) - file_ptrs);
                    
                    Buffer_ID *ids = push_array(scratch, Buffer_ID, id_count);
                    for (i32 i = 0; i < id_count; i += 1){
                        ids[i] = file_ptrs[i]->id.id;
                    }
                    
                    working_set->do_not_mark_edits = true;
                    hook_file_edit_finished(&models->app_links, ids, id_count);
                    working_set->do_not_mark_edits = false;
                    
                    for (i32 i = 0; i < id_count; i += 1){
                        block_zero_struct(&file_ptrs[i]->edit_finished_mark_node);
                    }
                    
                    dll_init_sentinel(&working_set->edit_finished_list);
                    working_set->time_of_next_edit_finished_signal = 0;
                    
                    end_temp_memory(temp);
                }
            }
        }
    }
    
    // NOTE(allen): if the exit signal has been sent, run the exit hook.
    if (input->trying_to_kill){
        models->keep_playing = false;
    }
    if (!models->keep_playing){
        Hook_Function *exit_func = models->hooks[hook_exit];
        if (exit_func != 0){
            if (exit_func(&models->app_links) == 0){
                models->animate_next_frame = true;
                models->keep_playing = true;
            }
        }
    }
    
    // NOTE(allen): rendering
    {
        f32 literal_dt = 0.f;
        u64 now_usecond_stamp = system->now_time();
        if (!input->first_step){
            u64 elapsed_useconds = now_usecond_stamp - models->last_render_usecond_stamp;
            literal_dt = (f32)((f64)(elapsed_useconds)/1000000.f);
        }
        models->last_render_usecond_stamp = now_usecond_stamp;
        
        f32 animation_dt = 0.f;
        if (models->animated_last_frame){
            animation_dt = literal_dt;
        }
        
        Frame_Info frame = {};
        frame.index = models->frame_counter;
        frame.literal_dt = literal_dt;
        frame.animation_dt = animation_dt;
        
        {
            Color_Table color_table = models->fallback_color_table;
            if (models->modify_color_table != 0){
                color_table = models->modify_color_table(&models->app_links, frame);
                if (color_table.count < models->fallback_color_table.count){
                    block_copy(models->fallback_color_table.vals, color_table.vals, color_table.count*sizeof(*color_table.vals));
                    color_table = models->fallback_color_table;
                }
            }
            models->color_table = color_table;
        }
        
        begin_render_section(target, system, models->frame_counter, literal_dt, animation_dt);
        models->in_render_mode = true;
        
        if (models->render_caller != 0){
            models->render_caller(&models->app_links, frame);
        }
        
#if 0        
        Panel *active_panel = layout_get_active_panel(layout);
        View *active_view = active_panel->view;
        
        // NOTE(allen): render the panels
        for (Panel *panel = layout_get_first_open_panel(layout);
             panel != 0;
             panel = layout_get_next_open_panel(layout, panel)){
            i32_Rect full = panel->rect_full;
            i32_Rect inner = panel->rect_inner;
            
            View *view = panel->view;
            //Style *style = &models->styles.styles[0];
            Color_Table color_table = models->color_table;
            
            draw_rectangle(target, full, color_table.vals[Stag_Back]);
            
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            GUI_Scroll_Vars *scroll_vars = &edit_pos.scroll;
            
            b32 active = (panel == active_panel);
            do_render_file_view(system, view, models, scroll_vars, active_view, inner, active, target);
            
            view_set_edit_pos(view, edit_pos);
            
            u32 margin_color = 0;
            if (active){
                margin_color = color_table.vals[Stag_Margin_Active];
            }
            else if (panel == mouse_panel){
                margin_color = color_table.vals[Stag_Margin_Hover];
            }
            else{
                margin_color = color_table.vals[Stag_Margin];
            }
            draw_rectangle(target, i32R( full.x0,  full.y0,  full.x1, inner.y0), margin_color);
            draw_rectangle(target, i32R( full.x0, inner.y1,  full.x1,  full.y1), margin_color);
            draw_rectangle(target, i32R( full.x0, inner.y0, inner.x0, inner.y1), margin_color);
            draw_rectangle(target, i32R(inner.x1, inner.y0,  full.x1, inner.y1), margin_color);
        }
#endif
        
        models->in_render_mode = false;
        end_render_section(target, system);
    }
    
    // NOTE(allen): set the app_result
    Application_Step_Result app_result = {};
    app_result.mouse_cursor_type = APP_MOUSE_CURSOR_DEFAULT;
    app_result.lctrl_lalt_is_altgr = models->settings.lctrl_lalt_is_altgr;
    
    // NOTE(allen): get new window title
    if (models->has_new_title){
        models->has_new_title = false;
        app_result.has_new_title = true;
        app_result.title_string = models->title_space;
    }
    
    // NOTE(allen): get cursor type
    if (mouse_panel != 0 && !mouse_in_margin){
        app_result.mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
    }
    else if (divider_panel != 0){
        if (divider_panel->vertical_split){
            app_result.mouse_cursor_type = APP_MOUSE_CURSOR_LEFTRIGHT;
        }
        else{
            app_result.mouse_cursor_type = APP_MOUSE_CURSOR_UPDOWN;
        }
    }
    else{
        app_result.mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
    }
    
    models->prev_mouse_panel = mouse_panel;
    app_result.lctrl_lalt_is_altgr = models->settings.lctrl_lalt_is_altgr;
    app_result.perform_kill = !models->keep_playing;
    app_result.animating = models->animate_next_frame;
    if (models->animate_next_frame){
        // NOTE(allen): Silence the timer, because we're going to do another frame right away anyways.
        system->wake_up_timer_set(models->period_wakeup_timer, max_u32);
    }
    else{
        // NOTE(allen): Set the timer's wakeup period, possibly to max_u32 thus effectively silencing it.
        system->wake_up_timer_set(models->period_wakeup_timer, models->next_animate_delay);
    }
    
    // NOTE(allen): Update Frame to Frame States
    models->prev_p = input->mouse.p;
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

