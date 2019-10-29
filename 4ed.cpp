/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application layer for project codename "4ed"
 *
 */

// TOP

internal void
output_file_append(Thread_Context *tctx, Models *models, Editing_File *file, String_Const_u8 value){
    i64 end = buffer_size(&file->state.buffer);
    Edit_Behaviors behaviors = {};
    edit_single(tctx, models, file, Ii64(end), value, behaviors);
}

internal void
file_cursor_to_end(Thread_Context *tctx, Models *models, Editing_File *file){
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
        view_set_cursor(tctx, models, view, pos);
        view->mark = pos;
    }
}

#include "4ed_api_implementation.cpp"

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
    
    color_table.vals[Stag_Line_Numbers_Back] = 0xFF101010;
    color_table.vals[Stag_Line_Numbers_Text] = 0xFF404040;
}

internal void
app_hardcode_default_style(Models *models){
    Color_Table color_table = {};
    color_table.count = Stag_COUNT;
    color_table.vals = push_array(models->arena, u32, color_table.count);
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
                            settings->font_size = (i32)string_to_integer(SCu8(argv[i]), 10);
                        }
                        action = CLAct_Nothing;
                    }break;
                    
                    case CLAct_FontUseHinting:
                    {
                        plat_settings->use_hinting = true;
                        settings->use_hinting = plat_settings->use_hinting;
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
models_init(void){
    Arena arena = make_arena_system();
    Models *models = push_array_zero(&arena, Models, 1);
    models->arena_ = arena;
    models->arena = &models->arena_;
    heap_init(&models->heap, get_base_allocator_system());
    return(models);
}

internal void
app_load_vtables(API_VTable_system *vtable_system,
                 API_VTable_font *vtable_font,
                 API_VTable_graphics *vtable_graphics){
    system_api_read_vtable(vtable_system);
    font_api_read_vtable(vtable_font);
    graphics_api_read_vtable(vtable_graphics);
}

internal Log_Function*
app_get_logger(void){
    log_init();
    return(log_string);
}

App_Read_Command_Line_Sig(app_read_command_line){
    Models *models = models_init();
    App_Settings *settings = &models->settings;
    block_zero_struct(settings);
    if (argc > 1){
        init_command_line_settings(&models->settings, plat_settings, argc, argv);
    }
    *files = models->settings.init_files;
    *file_count = &models->settings.init_files_count;
    return(models);
}

App_Init_Sig(app_init){
    Models *models = (Models*)base_ptr;
    models->keep_playing = true;
    
    models->config_api = api;
    
    profile_init(&models->profile_list);
    
    API_VTable_custom custom_vtable = {};
    custom_api_fill_vtable(&custom_vtable);
    API_VTable_system system_vtable = {};
    system_api_fill_vtable(&system_vtable);
    Custom_Layer_Init_Type *custom_init = api.init_apis(&custom_vtable, &system_vtable);
    Assert(custom_init != 0);
    
    Application_Links app = {};
    app.tctx = tctx;
    app.cmd_context = models;
    custom_init(&app);
    
    // NOTE(allen): coroutines
    coroutine_system_init(&models->coroutines);
    
    // NOTE(allen): font set
    font_set_init(&models->font_set);
    
    // NOTE(allen): live set
    Arena *arena = models->arena;
    {
        models->view_set.count = 0;
        models->view_set.max = MAX_VIEWS;
        models->view_set.views = push_array(arena, View, models->view_set.max);
        
        //dll_init_sentinel
        models->view_set.free_sentinel.next = &models->view_set.free_sentinel;
        models->view_set.free_sentinel.prev = &models->view_set.free_sentinel;
        
        i32 max = models->view_set.max;
        View *view = models->view_set.views;
        for (i32 i = 0; i < max; ++i, ++view){
            //dll_insert(&models->view_set.free_sentinel, view);
            view->next = models->view_set.free_sentinel.next;
            view->prev = &models->view_set.free_sentinel;
            models->view_set.free_sentinel.next = view;
            view->next->prev = view;
        }
    }
    
    managed_ids_init(tctx->allocator, &models->managed_id_set);
    lifetime_allocator_init(tctx->allocator, &models->lifetime_allocator);
    dynamic_workspace_init(&models->lifetime_allocator, DynamicWorkspace_Global, 0, &models->dynamic_workspace);
    
    // NOTE(allen): file setup
    working_set_init(models, &models->working_set);
    
    Mutex_Lock file_order_lock(models->working_set.mutex);
    
    // NOTE(allen): 
    global_history_init(&models->global_history);
    text_layout_init(tctx, &models->text_layouts);
    
    // NOTE(allen): clipboard setup
    models->working_set.clipboard_max_size = ArrayCount(models->working_set.clipboards);
    models->working_set.clipboard_size = 0;
    models->working_set.clipboard_current = 0;
    models->working_set.clipboard_rolling = 0;
    
    // TODO(allen): do(better clipboard allocation)
    if (clipboard.str != 0){
        String_Const_u8 *dest = working_set_next_clipboard_string(&models->heap, &models->working_set, clipboard.size);
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
    
    Heap *heap = &models->heap;
    for (i32 i = 0; i < ArrayCount(init_files); ++i){
        Editing_File *file = working_set_allocate_file(&models->working_set, &models->lifetime_allocator);
        buffer_bind_name(tctx, models, arena, &models->working_set, file, init_files[i].name);
        
        if (init_files[i].ptr != 0){
            *init_files[i].ptr = file;
        }
        
        File_Attributes attributes = {};
        file_create_from_string(tctx, models, file, SCu8(), attributes);
        if (init_files[i].read_only){
            file->settings.read_only = true;
            history_free(tctx, &file->state.history);
        }
        
        file->settings.never_kill = true;
        file_set_unimportant(file, true);
    }
    
    // NOTE(allen): setup first panel
    {
        Panel *panel = layout_initialize(arena, &models->layout);
        View *new_view = live_set_alloc_view(&models->lifetime_allocator, &models->view_set, panel);
        view_init(tctx, models, new_view, models->scratch_buffer, models->view_event_handler);
    }
    
    // NOTE(allen): miscellaneous init
    hot_directory_init(arena, &models->hot_directory, current_directory);
    child_process_container_init(tctx->allocator, &models->child_processes);
    models->period_wakeup_timer = system_wake_up_timer_create();
}

App_Step_Sig(app_step){
    Models *models = (Models*)base_ptr;
    
    Mutex_Lock file_order_lock(models->working_set.mutex);
    Scratch_Block scratch(tctx, Scratch_Share);
    
    models->next_animate_delay = max_u32;
    models->animate_next_frame = false;
    
    // NOTE(allen): per-frame update of models state
    begin_frame(target, &models->font_set);
    models->target = target;
    models->input = input;
    
    // NOTE(allen): OS clipboard event handling
    String_Const_u8 clipboard = input->clipboard;
    if (clipboard.str != 0){
        String_Const_u8 *dest = working_set_next_clipboard_string(&models->heap, &models->working_set, clipboard.size);
        dest->size = eol_convert_in((char*)dest->str, (char*)clipboard.str, (i32)clipboard.size);
        if (input->clipboard_changed){
            co_send_core_event(tctx, models, CoreCode_NewClipboardContents, *dest);
        }
    }
    
    // NOTE(allen): reorganizing panels on screen
    Vec2_i32 prev_dim = layout_get_root_size(&models->layout);
    Vec2_i32 current_dim = V2i32(target->width, target->height);
    layout_set_root_size(&models->layout, current_dim);
    
    // NOTE(allen): update child processes
    f32 dt = input->dt;
    if (dt > 0){
        Temp_Memory_Block temp(scratch);
        
        Child_Process_Container *child_processes = &models->child_processes;
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
            system_cli_begin_update(cli);
            if (system_cli_update_step(cli, dest, max, &amount)){
                if (file != 0 && amount > 0){
                    amount = eol_in_place_convert_in(dest, amount);
                    output_file_append(tctx, models, file, SCu8(dest, amount));
                    edited_file = true;
                }
            }
            
            if (system_cli_end_update(cli)){
                if (file != 0){
                    String_Const_u8 str = push_u8_stringf(scratch, "exited with code %d", cli->exit);
                    output_file_append(tctx, models, file, str);
                    edited_file = true;
                }
                processes_to_free[processes_to_free_count++] = child_process;
                child_process_set_return_code(models, child_processes, child_process->id, cli->exit);
            }
            
            if (child_process->cursor_at_end && file != 0){
                file_cursor_to_end(tctx, models, file);
            }
        }
        
        for (i32 i = 0; i < processes_to_free_count; ++i){
            child_process_free(child_processes, processes_to_free[i]->id);
        }
    }
    
    // NOTE(allen): simulated events
    Input_List input_list = input->events;
    Input_Modifier_Set modifiers = system_get_keyboard_modifiers(scratch);
    if (input->mouse.press_l){
        Input_Event event = {};
        event.kind = InputEventKind_MouseButton;
        event.mouse.code = MouseCode_Left;
        event.mouse.p = input->mouse.p;
        event.mouse.modifiers = copy_modifier_set(scratch, &modifiers);
        push_input_event(scratch, &input_list, &event);
    }
    else if (input->mouse.release_l){
        Input_Event event = {};
        event.kind = InputEventKind_MouseButtonRelease;
        event.mouse.code = MouseCode_Left;
        event.mouse.p = input->mouse.p;
        event.mouse.modifiers = copy_modifier_set(scratch, &modifiers);
        push_input_event(scratch, &input_list, &event);
    }
    if (input->mouse.press_r){
        Input_Event event = {};
        event.kind = InputEventKind_MouseButton;
        event.mouse.code = MouseCode_Right;
        event.mouse.p = input->mouse.p;
        event.mouse.modifiers = copy_modifier_set(scratch, &modifiers);
        push_input_event(scratch, &input_list, &event);
    }
    else if (input->mouse.release_r){
        Input_Event event = {};
        event.kind = InputEventKind_MouseButtonRelease;
        event.mouse.code = MouseCode_Right;
        event.mouse.p = input->mouse.p;
        event.mouse.modifiers = copy_modifier_set(scratch, &modifiers);
        push_input_event(scratch, &input_list, &event);
    }
    if (input->mouse.wheel != 0){
        Input_Event event = {};
        event.kind = InputEventKind_MouseWheel;
        event.mouse_wheel.value = (f32)(input->mouse.wheel);
        event.mouse_wheel.p = input->mouse.p;
        event.mouse.modifiers = copy_modifier_set(scratch, &modifiers);
        push_input_event(scratch, &input_list, &event);
    }
    if (input->mouse.p != models->prev_p){
        b32 was_in_window = rect_contains_point(Ri32(0, 0, prev_dim.x, prev_dim.y), models->prev_p);
        b32 is_in_window  = rect_contains_point(Ri32(0, 0, current_dim.x, current_dim.y), input->mouse.p);
        if (is_in_window || was_in_window){
            Input_Event event = {};
            event.kind = InputEventKind_MouseMove;
            event.mouse_move.p = input->mouse.p;
            event.mouse.modifiers = copy_modifier_set(scratch, &modifiers);
            push_input_event(scratch, &input_list, &event);
        }
    }
    if (models->animated_last_frame){
        Input_Event event = {};
        event.kind = InputEventKind_Core;
        event.core.code = CoreCode_Animate;
        push_input_event(scratch, &input_list, &event);
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
        Temp_Memory_Block temp(scratch);
        
        String_Const_u8_Array file_names = {};
        file_names.count = models->settings.init_files_count;
        file_names.vals = push_array(scratch, String_Const_u8, file_names.count);
        for (i32 i = 0; i < file_names.count; i += 1){
            file_names.vals[i] = SCu8(models->settings.init_files[i]);
        }
        
        String_Const_u8_Array flags = {};
        flags.count = models->settings.custom_flags_count;
        flags.vals = push_array(scratch, String_Const_u8, flags.count);
        for (i32 i = 0; i < flags.count; i += 1){
            flags.vals[i] = SCu8(models->settings.custom_flags[i]);
        }
        
        Input_Event event = {};
        event.kind = InputEventKind_Core;
        event.core.code = CoreCode_Startup;
        event.core.flag_strings = flags;
        event.core.file_names = file_names;
        co_send_event(tctx, models, &event);
    }
    
    // NOTE(allen): consume event stream
    for (Input_Event_Node *node = input_list.first;
         node != 0;
         node = node->next){
        b32 event_was_handled = false;
        Input_Event *event = &node->event;
        
        if (event->kind == InputEventKind_TextInsert &&
            event->text.blocked){
            continue;
        }
        
        Panel *active_panel = layout_get_active_panel(layout);
        View *view = active_panel->view;
        Assert(view != 0);
        
        switch (models->state){
            case APP_STATE_EDIT:
            {
                typedef i32 Event_Consume_Rule;
                enum{
                    EventConsume_None,
                    EventConsume_BeginResize,
                    EventConsume_ClickChangeView,
                    EventConsume_CustomCommand,
                };
                
                Event_Consume_Rule consume_rule = EventConsume_CustomCommand;
                if (match_mouse_code(event, MouseCode_Left) && (divider_panel != 0)){
                    consume_rule = EventConsume_BeginResize;
                }
                else if (match_mouse_code(event, MouseCode_Left) &&
                         mouse_panel != 0 && mouse_panel != active_panel){
                    consume_rule = EventConsume_ClickChangeView;
                }
                
                switch (consume_rule){
                    case EventConsume_BeginResize:
                    {
                        models->state = APP_STATE_RESIZING;
                        models->resizing_intermediate_panel = divider_panel;
                        event_was_handled = true;
                    }break;
                    
                    case EventConsume_ClickChangeView:
                    {
                        // NOTE(allen): run deactivate command
                        co_send_core_event(tctx, models, view, CoreCode_ClickDeactivateView);
                        
                        layout->active_panel = mouse_panel;
                        models->animate_next_frame = true;
                        active_panel = mouse_panel;
                        view = active_panel->view;
                        
                        // NOTE(allen): run activate command
                        co_send_core_event(tctx, models, view, CoreCode_ClickActivateView);
                        
                        event_was_handled = true;
                    }break;
                    
                    case EventConsume_CustomCommand:
                    {
                        event_was_handled = co_send_event(tctx, models, view, event);
                    }break;
                }
            }break;
            
            case APP_STATE_RESIZING:
            {
                Event_Property event_flags = get_event_properties(event);
                if (HasFlag(event_flags, EventProperty_AnyKey) ||
                    match_mouse_code_release(event, MouseCode_Left)){
                    models->state = APP_STATE_EDIT;
                }
                else if (event->kind == InputEventKind_MouseMove){
                    if (input->mouse.l){
                        Panel *split = models->resizing_intermediate_panel;
                        Range_i32 limits = layout_get_limiting_range_on_split(layout, split);
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
        
        if (event_was_handled && event->kind == InputEventKind_KeyStroke){
            for (Input_Event *dependent_text = event->key.first_dependent_text;
                 dependent_text != 0;
                 dependent_text = dependent_text->text.next_text){
                Assert(dependent_text->kind == InputEventKind_TextInsert);
                dependent_text->text.blocked = true;
            }
        }
    }
    
    // NOTE(allen): send panel size update
    if (models->layout.panel_state_dirty){
        models->layout.panel_state_dirty = false;
        if (models->buffer_viewer_update != 0){
            Application_Links app = {};
            app.tctx = tctx;
            app.cmd_context = models;
            models->buffer_viewer_update(&app);
        }
    }
    
    // NOTE(allen): dt
    f32 literal_dt = 0.f;
    u64 now_usecond_stamp = system_now_time();
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
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            edit_pos.scroll.position = view_normalize_buffer_point(tctx, models, view, edit_pos.scroll.target);
            block_zero_struct(&edit_pos.scroll.target);
            view_set_edit_pos(view, edit_pos);
        }
    }
    
    // NOTE(allen): hook for files reloaded
    {
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
                co_send_core_event(tctx, models, CoreCode_FileExternallyModified, file->id);
            }
        }
    }
    
    // NOTE(allen): if the exit signal has been sent, run the exit hook.
    if (input->trying_to_kill){
        models->keep_playing = false;
    }
    if (!models->keep_playing){
        if (co_send_core_event(tctx, models, CoreCode_TryExit)){
            models->keep_playing = true;
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
#if 0
            if (models->modify_color_table != 0){
                color_table = models->modify_color_table(&models->app_links, frame);
                if (color_table.count < models->fallback_color_table.count){
                    block_copy(models->fallback_color_table.vals, color_table.vals, color_table.count*sizeof(*color_table.vals));
                    color_table = models->fallback_color_table;
                }
            }
#endif
            models->color_table = color_table;
        }
        
        begin_render_section(target, models->frame_counter, literal_dt, animation_dt);
        models->in_render_mode = true;
        
        Live_Views *live_views = &models->view_set;
        for (Node *node = layout->open_panels.next;
             node != &layout->open_panels;
             node = node->next){
            Panel *panel = CastFromMember(Panel, node, node);
            View *view = panel->view;
            View_Context_Node *ctx = view->ctx;
            if (ctx != 0){
                Render_Caller_Function *render_caller = ctx->ctx.render_caller;
                if (render_caller != 0){
                    Application_Links app = {};
                    app.tctx = tctx;
                    app.cmd_context = models;
                    render_caller(&app, frame, view_get_id(live_views, view));
                }
            }
        }
        
        models->in_render_mode = false;
        end_render_section(target);
    }
    
    // NOTE(allen): flush the log
    log_flush(tctx, models);
    
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
        system_wake_up_timer_set(models->period_wakeup_timer, max_u32);
    }
    else{
        // NOTE(allen): Set the timer's wakeup period, possibly to max_u32 thus effectively silencing it.
        system_wake_up_timer_set(models->period_wakeup_timer, models->next_animate_delay);
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
    
    result.load_vtables = app_load_vtables;
    result.get_logger = app_get_logger;
    result.read_command_line = app_read_command_line;
    result.init = app_init;
    result.step = app_step;
    
    return(result);
}

// BOTTOM

