/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application layer for project codename "4ed"
 *
 */

// TOP

Mutex_Lock::Mutex_Lock(System_Functions *s, System_Mutex m){
    s->mutex_acquire(m);
    this->system = s;
    this->mutex = m;
}

Mutex_Lock::~Mutex_Lock(){
    this->system->mutex_release(this->mutex);
}

Mutex_Lock::operator System_Mutex(){
    return(this->mutex);
}

////////////////////////////////

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

internal Coroutine*
app_coroutine_handle_request(Models *models, Coroutine *co, u32 *vals){
    Coroutine *result = 0;
    System_Functions *system = models->system;
    switch (vals[2]){
        case AppCoroutineRequest_NewFontFace:
        {
            Face_Description *description = ((Face_Description**)vals)[0];
            Face *face = font_set_new_face(&models->font_set, description);
            result = coroutine_run(&models->coroutines, co, &face->id, vals);
        }break;
        
        case AppCoroutineRequest_ModifyFace:
        {
            Face_Description *description = ((Face_Description**)vals)[0];
            Face_ID face_id = ((Face_ID*)vals)[3];
            b32 success = font_set_modify_face(&models->font_set, face_id, description);
            result = coroutine_run(&models->coroutines, co, &success, vals);
        }break;
    }
    return(result);
}

internal Coroutine*
app_coroutine_run(Models *models, App_Coroutine_Purpose purpose, Coroutine *co, void *in, u32 *out){
    Application_Links *app = &models->app_links;
    App_Coroutine_State prev_state = get_state(app);
    app->current_coroutine = co;
    app->type_coroutine = purpose;
    u32 coroutine_out[4] = {};
    Coroutine *result = coroutine_run(&models->coroutines, co, in, coroutine_out);
    for (;result != 0 && coroutine_out[2] != 0;){
        result = app_coroutine_handle_request(models, result, coroutine_out);
    }
    block_copy(out, coroutine_out, sizeof(*out)*2);
    restore_state(app, prev_state);
    return(result);
}

internal void
output_file_append(Models *models, Editing_File *file, String_Const_u8 value){
    i64 end = buffer_size(&file->state.buffer);
    Edit_Behaviors behaviors = {};
    edit_single(models, file, Ii64(end), value, behaviors);
}

internal void
file_cursor_to_end(System_Functions *system, Models *models, Editing_File *file){
    Assert(file != 0);
    i64 pos = buffer_size(&file->state.buffer);
    Layout *layout = &models->layout;
    for (Panel *panel = layout_get_first_open_panel(layout);
         panel != 0;
         panel = layout_get_next_open_panel(layout, panel)){
        View *view = panel->view;
        if (view->file != file){
            continue;
        }
        view_set_cursor(models, view, pos);
        view->mark = pos;
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


DELTA_RULE_SIG(fallback_scroll_rule){
    return(pending_delta);
}

#define DEFAULT_MAP_SIZE 10
#define DEFAULT_UI_MAP_SIZE 32

internal void
setup_ui_commands(Command_Map *commands, Cursor *cursor, i32 parent){
    map_init(commands, cursor, DEFAULT_UI_MAP_SIZE, parent);
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
setup_file_commands(Command_Map *commands, Cursor *cursor, i32 parent){
    map_init(commands, cursor, DEFAULT_MAP_SIZE, parent);
}

internal void
setup_top_commands(Command_Map *commands, Cursor *cursor, i32 parent){
    map_init(commands, cursor, DEFAULT_MAP_SIZE, parent);
}

// TODO(allen): REWRITE REWRITE REWRITE!
internal b32
interpret_binding_buffer(Models *models, void *buffer, i32 size){
    b32 result = true;
    
    Heap *gen = &models->mem.heap;
    Arena *scratch = &models->mem.arena;
    Temp_Memory temp = begin_temp(scratch);
    
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
        new_mapping.map_id_table = push_array(scratch, i32, user_map_count);
        memset(new_mapping.map_id_table, -1, user_map_count*sizeof(i32));
        
        new_mapping.user_maps = push_array(scratch, Command_Map, user_map_count);
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
            count_global = clamp_bot(6, new_mapping.map_top.count*3/2);
        }
        
        i32 count_file = DEFAULT_MAP_SIZE;
        if (did_file){
            count_file = clamp_bot(6, new_mapping.map_file.count*3/2);
        }
        
        i32 count_ui = DEFAULT_UI_MAP_SIZE;
        
        i32 count_user = 0;
        for (i32 i = 0; i < user_map_count; ++i){
            Command_Map *map = &new_mapping.user_maps[i];
            count_user += clamp_bot(6, map->max*3/2);
        }
        
        i32 binding_memsize = (count_global + count_file + count_ui + count_user)*sizeof(Command_Binding);
        
        // Allocate Needed Memory
        i32 map_id_table_memsize = user_map_count*sizeof(i32);
        i32 user_maps_memsize = user_map_count*sizeof(Command_Map);
        
        i32 map_id_table_rounded_memsize = round_up_i32(map_id_table_memsize, 8);
        i32 user_maps_rounded_memsize = round_up_i32(user_maps_memsize, 8);
        
        i32 binding_rounded_memsize = round_up_i32(binding_memsize, 8);
        
        i32 needed_memsize = map_id_table_rounded_memsize + user_maps_rounded_memsize + binding_rounded_memsize;
        new_mapping.memory = heap_allocate(gen, needed_memsize);
        Cursor local_cursor = make_cursor(new_mapping.memory, needed_memsize);
        
        // Move ID Table Memory and Pointer
        i32 *old_table = new_mapping.map_id_table;
        new_mapping.map_id_table = push_array(&local_cursor, i32, user_map_count);
        memmove(new_mapping.map_id_table, old_table, map_id_table_memsize);
        
        // Move User Maps Memory and Pointer
        Command_Map *old_maps = new_mapping.user_maps;
        new_mapping.user_maps = push_array(&local_cursor, Command_Map, user_map_count);
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
                            i32 table_max = clamp_bot(6, count*3/2);
                            map_init(map_ptr, &local_cursor, table_max, mapid_global);
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
                                    models->hook_open_file = (Buffer_Hook_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_new_file:
                                {
                                    models->hook_new_file = (Buffer_Hook_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_save_file:
                                {
                                    models->hook_save_file = (Buffer_Hook_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_end_file:
                                {
                                    models->hook_end_file = (Buffer_Hook_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_file_edit_range:
                                {
                                    models->hook_file_edit_range = (File_Edit_Range_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_file_edit_finished:
                                {
                                    models->hook_file_edit_finished = (File_Edit_Finished_Function*)unit->hook.func;
                                }break;
                                
                                case special_hook_file_externally_modified:
                                {
                                    models->hook_file_externally_modified = (File_Externally_Modified_Function*)unit->hook.func;
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
                                    models->scroll_rule = (Delta_Rule_Function*)unit->hook.func;
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
            setup_top_commands(&new_mapping.map_top, &local_cursor, mapid_global);
        }
        if (!did_file){
            setup_file_commands(&new_mapping.map_file, &local_cursor, mapid_global);
        }
        setup_ui_commands(&new_mapping.map_ui, &local_cursor, mapid_global);
    }
    else{
        // TODO(allen): do(Error report: bad binding units map.)
        // TODO(allen): do(no bindings set recovery plan.)
        InvalidPath;
    }
    
    Mapping old_mapping = models->mapping;
    if (old_mapping.memory != 0){
        heap_free(gen, old_mapping.memory);
    }
    
    models->mapping = new_mapping;
    end_temp(temp);
    
    return(result);
}

#include "4ed_api_implementation.cpp"

internal void
command_caller(Coroutine *coroutine){
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
    Arena *arena = &models->mem.arena;
    Color_Table color_table = {};
    color_table.count = Stag_COUNT;
    color_table.vals = push_array(arena, u32, color_table.count);
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
            if (string_match(SCu8(long_arg_name),
                             string_u8_litexpr("custom"))){
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
                            settings->initial_line = (i32)string_to_integer(SCu8(argv[i]), 10);
                        }
                        action = CLAct_Nothing;
                    }break;
                    
                    case CLAct_WindowSize:
                    {
                        if (i + 1 < argc){
                            plat_settings->set_window_size = true;
                            
                            i32 w = (i32)string_to_integer(SCu8(argv[i]), 10);
                            i32 h = (i32)string_to_integer(SCu8(argv[i + 1]), 10);
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
                            
                            i32 x = (i32)string_to_integer(SCu8(argv[i]), 10);
                            i32 y = (i32)string_to_integer(SCu8(argv[i + 1]), 10);
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
                            plat_settings->font_size = (i32)string_to_integer(SCu8(argv[i]), 10);
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

////////////////////////////////

internal Models*
app_setup_memory(System_Functions *system, Application_Memory *memory){
    Cursor cursor = make_cursor(memory->vars_memory, memory->vars_memory_size);
    Models *models = push_array_zero(&cursor, Models, 1);
    models->mem.arena = make_arena_system(system);
    models->base_allocator = models->mem.arena.base_allocator;
    heap_init(&models->mem.heap, models->base_allocator);
    return(models);
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
        models->command_coroutine = app_coroutine_run(models, Co_Command, models->command_coroutine, &user_in, models->command_coroutine_flags);
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
        Coroutine *command_coroutine = coroutine_create(&models->coroutines, command_caller);
        models->command_coroutine = command_coroutine;
        
        Command_In cmd_in = {};
        cmd_in.models = models;
        cmd_in.bind = cmd_bind;
        
        models->command_coroutine = app_coroutine_run(models, Co_Command, models->command_coroutine, &cmd_in, models->command_coroutine_flags);
        
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

internal Log_Function*
app_get_logger(System_Functions *system){
    log_init(system);
    return(log_string);
}

App_Read_Command_Line_Sig(app_read_command_line){
    i32 out_size = 0;
    Models *models = app_setup_memory(system, memory);
    App_Settings *settings = &models->settings;
    memset(settings, 0, sizeof(*settings));
    plat_settings->font_size = 16;
    if (argc > 1){
        init_command_line_settings(&models->settings, plat_settings, argc, argv);
    }
    *files = models->settings.init_files;
    *file_count = &models->settings.init_files_count;
    return(out_size);
}

App_Init_Sig(app_init){
    Models *models = (Models*)memory->vars_memory;
    models->system = system;
    models->keep_playing = true;
    
    app_links_init(system, &models->app_links, memory->user_memory, memory->user_memory_size);
    models->custom_layer_arena = make_arena_models(models);
    
    models->config_api = api;
    models->app_links.cmd_context = models;
    
    Arena *arena = &models->mem.arena;
    
    // NOTE(allen): coroutines
    coroutine_system_init(system, &models->coroutines);
    
    // NOTE(allen): font set
    font_set_init(system, &models->font_set);
    
    // NOTE(allen): live set
    {
        models->live_set.count = 0;
        models->live_set.max = MAX_VIEWS;
        models->live_set.views = push_array(arena, View, models->live_set.max);
        
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
        Assert(models->config_api.get_bindings != 0);
        i32 wanted_size = models->config_api.get_bindings(models->app_links.memory, models->app_links.memory_size);
        Assert(wanted_size <= models->app_links.memory_size);
        interpret_binding_buffer(models, models->app_links.memory, wanted_size);
        memset(models->app_links.memory, 0, wanted_size);
    }
    
    managed_ids_init(models->base_allocator, &models->managed_id_set);
    lifetime_allocator_init(models->base_allocator, &models->lifetime_allocator);
    dynamic_workspace_init(&models->lifetime_allocator, DynamicWorkspace_Global, 0, &models->dynamic_workspace);
    
    // NOTE(allen): file setup
    working_set_init(models, &models->working_set);
    
    Mutex_Lock file_order_lock(system, models->working_set.mutex);
    
    // NOTE(allen): 
    global_history_init(&models->global_history);
    text_layout_init(models, &models->text_layouts);
    
    // NOTE(allen): clipboard setup
    models->working_set.clipboard_max_size = ArrayCount(models->working_set.clipboards);
    models->working_set.clipboard_size = 0;
    models->working_set.clipboard_current = 0;
    models->working_set.clipboard_rolling = 0;
    
    // TODO(allen): do(better clipboard allocation)
    if (clipboard.str != 0){
        String_Const_u8 *dest = working_set_next_clipboard_string(&models->mem.heap, &models->working_set, clipboard.size);
        block_copy(dest->str, clipboard.str, clipboard.size);
    }
    
    // NOTE(allen): style setup
    {
        Face_Description description = {};
        description.font.file_name = string_u8_litexpr("liberation-mono.ttf");
        description.font.in_4coder_font_folder = true;
        description.parameters.pt_size = 12;
        Face *new_face = font_set_new_face(&models->font_set, &description);
        models->global_face_id = new_face->id;
    }
    app_hardcode_default_style(models);
    
    // NOTE(allen): title space
    models->has_new_title = true;
    models->title_capacity = KB(4);
    models->title_space = push_array(arena, char, models->title_capacity);
    block_copy(models->title_space, WINDOW_NAME, sizeof(WINDOW_NAME));
    
    // NOTE(allen): init baked in buffers
    File_Init init_files[] = {
        { string_u8_litinit("*messages*"), &models->message_buffer, true , },
        { string_u8_litinit("*scratch*") , &models->scratch_buffer, false, },
        { string_u8_litinit("*log*")     , &models->log_buffer    , true , },
    };
    
    Heap *heap = &models->mem.heap;
    for (i32 i = 0; i < ArrayCount(init_files); ++i){
        Editing_File *file = working_set_allocate_file(&models->working_set, &models->lifetime_allocator);
        buffer_bind_name(models, arena, &models->working_set, file, init_files[i].name);
        
        if (init_files[i].ptr != 0){
            *init_files[i].ptr = file;
        }
        
        File_Attributes attributes = {};
        file_create_from_string(models, file, SCu8(), attributes);
        if (init_files[i].read_only){
            file->settings.read_only = true;
            history_free(&file->state.history);
        }
        
        file->settings.never_kill = true;
        file_set_unimportant(file, true);
    }
    
    // NOTE(allen): setup first panel
    {
        Panel *panel = layout_initialize(arena, &models->layout);
        View *new_view = live_set_alloc_view(&models->lifetime_allocator, &models->live_set, panel);
        view_set_file(system, models, new_view, models->scratch_buffer);
    }
    
    // NOTE(allen): miscellaneous init
    hot_directory_init(system, arena, &models->hot_directory, current_directory);
    child_process_container_init(models->base_allocator, &models->child_processes);
    models->user_up_key = key_up;
    models->user_down_key = key_down;
    models->period_wakeup_timer = system->wake_up_timer_create();
}

App_Step_Sig(app_step){
    Models *models = (Models*)memory->vars_memory;
    
    Mutex_Lock file_order_lock(system, models->working_set.mutex);
    
    models->next_animate_delay = max_u32;
    models->animate_next_frame = false;
    
    // NOTE(allen): per-frame update of models state
    begin_frame(target, &models->font_set);
    models->target = target;
    models->input = input;
    
    // NOTE(allen): OS clipboard event handling
    String_Const_u8 clipboard = input->clipboard;
    if (clipboard.str != 0){
        String_Const_u8 *dest = working_set_next_clipboard_string(&models->mem.heap, &models->working_set, clipboard.size);
        dest->size = eol_convert_in((char*)dest->str, (char*)clipboard.str, (i32)clipboard.size);
        if (input->clipboard_changed && models->clipboard_change != 0){
            models->clipboard_change(&models->app_links, *dest, ClipboardFlag_FromOS);
        }
    }
    
    // NOTE(allen): reorganizing panels on screen
    Vec2_i32 prev_dim = layout_get_root_size(&models->layout);
    Vec2_i32 current_dim = V2i32(target->width, target->height);
    layout_set_root_size(&models->layout, current_dim);
    
    // NOTE(allen): update child processes
    f32 dt = input->dt;
    if (dt > 0){
        Arena *scratch = &models->mem.arena;
        Child_Process_Container *child_processes = &models->child_processes;
        
        Temp_Memory temp = begin_temp(scratch);
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
                    output_file_append(models, file, SCu8(dest, amount));
                    edited_file = true;
                }
            }
            
            if (system->cli_end_update(cli)){
                if (file != 0){
                    String_Const_u8 str = push_u8_stringf(scratch, "exited with code %d", cli->exit);
                    output_file_append(models, file, str);
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
        
        end_temp(temp);
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
        b32 was_in_window = rect_contains_point(i32R(0, 0, prev_dim.x, prev_dim.y), models->prev_p);
        b32 is_in_window  = rect_contains_point(i32R(0, 0, current_dim.x, current_dim.y), input->mouse.p);
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
            if (rect_contains_point(panel->rect_full, mouse)){
                mouse_panel = panel;
                if (!rect_contains_point(panel->rect_inner, mouse)){
                    mouse_in_margin = true;
                    for (divider_panel = mouse_panel->parent;
                         divider_panel != 0;
                         divider_panel = divider_panel->parent){
                        if (rect_contains_point(divider_panel->rect_inner, mouse)){
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
    }
    
    // NOTE(allen): consume event stream
    Key_Event_Data *key_ptr = input->keys.keys;
    Key_Event_Data *key_end = key_ptr + input->keys.count;
    for (;key_ptr < key_end; key_ptr += 1){
        Panel *active_panel = layout_get_active_panel(layout);
        View *view = active_panel->view;
        Assert(view != 0);
        
        switch (models->state){
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
                        models->state = APP_STATE_RESIZING;
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
                            
                            Coroutine *command_coroutine = models->command_coroutine;
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
                                models->command_coroutine =  app_coroutine_run(models, Co_Command, command_coroutine, &user_in, models->command_coroutine_flags);
                                
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
                    models->state = APP_STATE_EDIT;
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
                        models->state = APP_STATE_EDIT;
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
    
    // NOTE(allen): dt
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
    
    // NOTE(allen): on the first frame there should be no scrolling
    if (input->first_step){
        for (Panel *panel = layout_get_first_open_panel(layout);
             panel != 0;
             panel = layout_get_next_open_panel(layout, panel)){
            View *view = panel->view;
            if (!view->ui_mode){
                File_Edit_Positions edit_pos = view_get_edit_pos(view);
                edit_pos.scroll.position = view_normalize_buffer_point(models, view, edit_pos.scroll.target);
                block_zero_struct(&edit_pos.scroll.target);
                view_set_edit_pos(view, edit_pos);
            }
            else{
                view->ui_scroll.position = view->ui_scroll.target;
                block_zero_struct(&view->ui_scroll.target);
            }
        }
    }
    
    // NOTE(allen): apply pending smooth deltas
    {
        Delta_Rule_Function *scroll_rule = models->scroll_rule;
        for (Panel *panel = layout_get_first_open_panel(layout);
             panel != 0;
             panel = layout_get_next_open_panel(layout, panel)){
            View *view = panel->view;
            
            View_ID view_id = view_get_id(&models->live_set, view);
            b32 new_target = (b32)view->new_scroll_target;
            if (view->ui_mode){
                Vec2_f32 pending = view->ui_scroll.target - view->ui_scroll.position;
                if (!near_zero(pending, 0.5f)){
                    Vec2_f32 partial = scroll_rule(pending, view_id, new_target, animation_dt);
                    view->ui_scroll.position += partial;
                    models->animate_next_frame = true;
                }
                else{
                    view->ui_scroll.position = view->ui_scroll.target;
                }
            }
            else{
                File_Edit_Positions edit_pos = view_get_edit_pos(view);
                Vec2_f32 pending = view_buffer_point_difference(models, view,
                                                                edit_pos.scroll.target, edit_pos.scroll.position);
                if (!near_zero(pending, 0.5f)){
                    Vec2_f32 partial = scroll_rule(pending, view_id, new_target, animation_dt);
                    edit_pos.scroll.position = view_move_buffer_point(models, view,
                                                                      edit_pos.scroll.position, partial);
                    view_set_edit_pos(view, edit_pos);
                    models->animate_next_frame = true;
                }
                else{
                    edit_pos.scroll.position = edit_pos.scroll.target;
                    view_set_edit_pos(view, edit_pos);
                }
            }
        }
    }
    
    // NOTE(allen): hook for files reloaded
    {
        File_Externally_Modified_Function *hook_file_externally_modified = models->hook_file_externally_modified;
        if (hook_file_externally_modified != 0){
            Working_Set *working_set = &models->working_set;
            Assert(working_set->has_external_mod_sentinel.next != 0);
            if (working_set->has_external_mod_sentinel.next != &working_set->has_external_mod_sentinel){
                for (Node *node = working_set->has_external_mod_sentinel.next, *next = 0;
                     node != &working_set->has_external_mod_sentinel;
                     node = next){
                    next = node->next;
                    Editing_File *file = CastFromMember(Editing_File, external_mod_node, node);
                    dll_remove(node);
                    block_zero_struct(node);
                    hook_file_externally_modified(&models->app_links, file->id);
                }
            }
        }
    }
    
    // NOTE(allen): hook for files marked "edit finished"
    {
        File_Edit_Finished_Function *hook_file_edit_finished = models->hook_file_edit_finished;
        if (hook_file_edit_finished != 0){
            Working_Set *working_set = &models->working_set;
            if (working_set->edit_finished_count > 0){
                Assert(working_set->edit_finished_sentinel.next != 0);
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
                    Arena *scratch = &models->mem.arena;
                    Temp_Memory temp = begin_temp(scratch);
                    Node *first = working_set->edit_finished_sentinel.next;
                    Node *one_past_last = &working_set->edit_finished_sentinel;
                    
                    i32 max_id_count = working_set->edit_finished_count;
                    Editing_File **file_ptrs = push_array(scratch, Editing_File*, max_id_count);
                    Buffer_ID *ids = push_array(scratch, Buffer_ID, max_id_count);
                    i32 id_count = 0;
                    for (Node *node = first;
                         node != one_past_last;
                         node = node->next){
                        Editing_File *file_ptr = CastFromMember(Editing_File, edit_finished_mark_node, node);
                        file_ptrs[id_count] = file_ptr;
                        ids[id_count] = file_ptr->id;
                        id_count += 1;
                    }
                    
                    working_set->do_not_mark_edits = true;
                    hook_file_edit_finished(&models->app_links, ids, id_count);
                    working_set->do_not_mark_edits = false;
                    
                    for (i32 i = 0; i < id_count; i += 1){
                        file_ptrs[i]->edit_finished_marked = false;
                    }
                    
                    dll_init_sentinel(&working_set->edit_finished_sentinel);
                    working_set->edit_finished_count = 0;
                    working_set->time_of_next_edit_finished_signal = 0;
                    
                    end_temp(temp);
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
        
        models->in_render_mode = false;
        end_render_section(target, system);
    }
    
    // NOTE(allen): flush the log
    log_flush(models);
    
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
    
    result.get_logger = app_get_logger;
    result.read_command_line = app_read_command_line;
    result.init = app_init;
    result.step = app_step;
    
    return(result);
}

// BOTTOM

