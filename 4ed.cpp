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
            if (string_match(SCu8(long_arg_name), string_u8_litexpr("custom"))){
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
                                case 'd': action = CLAct_CustomDLL; strict = false; break;
                                case 'D': action = CLAct_CustomDLL; strict = true; break;
                                
                                case 'w': action = CLAct_WindowSize; break;
                                case 'W': action = CLAct_WindowMaximize; break;
                                case 'p': action = CLAct_WindowPosition; break;
                                case 'F': action = CLAct_WindowFullscreen; break;
                                
                                case 'f': action = CLAct_FontSize; break;
                                case 'h': action = CLAct_FontUseHinting; --i; break;
                                case 'U': action = CLAct_UserDirectory; break;
                                
                                case 'L': action = CLAct_Nothing; break;
                                //case 'L': enables log, parsed before this is called (because I'm a dumbass)
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
                    
                    case CLAct_UserDirectory:
                    {
                        if (i < argc){
                            plat_settings->user_directory = argv[i];
                        }
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
app_load_vtables(API_VTable_system *vtable_system, API_VTable_font *vtable_font, API_VTable_graphics *vtable_graphics){
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
    models->hard_exit = false;
    
    models->config_api = api;
    models->virtual_event_arena = make_arena_system();
    
    profile_init(&models->profile_list);
    
    managed_ids_init(tctx->allocator, &models->managed_id_set);
    
    API_VTable_custom custom_vtable = {};
    custom_api_fill_vtable(&custom_vtable);
    API_VTable_system system_vtable = {};
    system_api_fill_vtable(&system_vtable);
    Custom_Layer_Init_Type *custom_init = api.init_apis(&custom_vtable, &system_vtable);
    Assert(custom_init != 0);
    
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
    
    lifetime_allocator_init(tctx->allocator, &models->lifetime_allocator);
    dynamic_workspace_init(&models->lifetime_allocator, DynamicWorkspace_Global, 0, &models->dynamic_workspace);
    
    // NOTE(allen): file setup
    working_set_init(models, &models->working_set);
    Mutex_Lock file_order_lock(models->working_set.mutex);
    
    // NOTE(allen):
    global_history_init(&models->global_history);
    text_layout_init(tctx, &models->text_layouts);
    
    // NOTE(allen): style setup
    {
        Scratch_Block scratch(tctx, arena);
        
        String8 binary_path = system_get_path(scratch, SystemPath_Binary);
        String8 full_path = push_u8_stringf(arena, "%.*sfonts/liberation-mono.ttf", string_expand(binary_path));
        
        Face_Description description = {};
        description.font.file_name = full_path;
        description.parameters.pt_size = 12;
        Face *new_face = font_set_new_face(&models->font_set, &description);
        if (new_face == 0){
            system_error_box("Could not load the required fallback font");
        }
        models->global_face_id = new_face->id;
    }
    
    // NOTE(allen): title space
    models->has_new_title = true;
    models->title_capacity = KB(4);
    models->title_space = push_array(arena, char, models->title_capacity);
    block_copy(models->title_space, WINDOW_NAME, sizeof(WINDOW_NAME));
    
    // NOTE(allen): miscellaneous init
    hot_directory_init(arena, &models->hot_directory, current_directory);
    child_process_container_init(tctx->allocator, &models->child_processes);
    models->period_wakeup_timer = system_wake_up_timer_create();
    
    // NOTE(allen): custom layer init
    Application_Links app = {};
    app.tctx = tctx;
    app.cmd_context = models;
    custom_init(&app);
    
    // NOTE(allen): init baked in buffers
    File_Init init_files[] = {
        { str8_lit("*messages*"), &models->message_buffer , true , },
        { str8_lit("*scratch*") , &models->scratch_buffer , false, },
        { str8_lit("*log*")     , &models->log_buffer     , true , },
        { str8_lit("*keyboard*"), &models->keyboard_buffer, true , },
    };
    
    Buffer_Hook_Function *begin_buffer_func = models->begin_buffer;
    models->begin_buffer = 0;
    
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
    
    models->begin_buffer = begin_buffer_func;
    
    // NOTE(allen): setup first panel
    {
        Panel *panel = layout_initialize(arena, &models->layout);
        View *new_view = live_set_alloc_view(&models->lifetime_allocator, &models->view_set, panel);
        view_init(tctx, models, new_view, models->scratch_buffer, models->view_event_handler);
    }
}

App_Step_Sig(app_step){
    Models *models = (Models*)base_ptr;
    
    Mutex_Lock file_order_lock(models->working_set.mutex);
    Scratch_Block scratch(tctx);
    
    models->next_animate_delay = max_u32;
    models->animate_next_frame = false;
    
    // NOTE(allen): per-frame update of models state
    begin_frame(target, &models->font_set);
    models->target = target;
    models->input = input;
    
    // NOTE(allen): OS clipboard event handling
    if (input->clipboard.str != 0){
        co_send_core_event(tctx, models, CoreCode_NewClipboardContents, input->clipboard);
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
        event.mouse_wheel.modifiers = copy_modifier_set(scratch, &modifiers);
        push_input_event(scratch, &input_list, &event);
    }
    if (input->mouse.p != models->prev_p){
        b32 was_in_window = rect_contains_point(Ri32(0, 0, prev_dim.x, prev_dim.y), models->prev_p);
        b32 is_in_window  = rect_contains_point(Ri32(0, 0, current_dim.x, current_dim.y), input->mouse.p);
        if (is_in_window || was_in_window){
            Input_Event event = {};
            event.kind = InputEventKind_MouseMove;
            event.mouse_move.p = input->mouse.p;
            event.mouse_move.modifiers = copy_modifier_set(scratch, &modifiers);
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
        
        // NOTE(allen): Actually do the buffer settings for the built ins now.
        Buffer_Hook_Function *begin_buffer_func = models->begin_buffer;
        if (begin_buffer_func != 0){
            Application_Links app = {};
            app.tctx = tctx;
            app.cmd_context = models;
            begin_buffer_func(&app, models->message_buffer->id);
            begin_buffer_func(&app, models->scratch_buffer->id);
            begin_buffer_func(&app, models->log_buffer->id);
            begin_buffer_func(&app, models->keyboard_buffer->id);
        }
    }
    
    // NOTE(allen): consume event stream
    Input_Event_Node *input_node = input_list.first;
    Input_Event_Node *input_node_next = 0;
    for (;; input_node = input_node_next){
        // NOTE(allen): first handle any events coming from the view command
        // function queue
        Model_View_Command_Function cmd_func = models_pop_view_command_function(models);
        if (cmd_func.custom_func != 0){
            View *view = imp_get_view(models, cmd_func.view_id);
            if (view != 0){
                input_node_next = input_node;
                Input_Event cmd_func_event = {};
                cmd_func_event.kind = InputEventKind_CustomFunction;
                cmd_func_event.custom_func = cmd_func.custom_func;
                co_send_event(tctx, models, view, &cmd_func_event);
                continue;
            }
        }
        
        Temp_Memory_Block temp(scratch);
        Input_Event *simulated_input = 0;
        Input_Event virtual_event = models_pop_virtual_event(scratch, models);
        if (virtual_event.kind != InputEventKind_None){
            virtual_event.virtual_event = true;
            simulated_input = &virtual_event;
        }
        else{
            if (input_node == 0){
                break;
            }
            input_node_next = input_node->next;
            simulated_input = &input_node->event;
            
            if (simulated_input->kind == InputEventKind_TextInsert && simulated_input->text.blocked){
                continue;
            }
            
            // NOTE(allen): record to keyboard history
            if (simulated_input->kind == InputEventKind_KeyStroke ||
                simulated_input->kind == InputEventKind_KeyRelease ||
                simulated_input->kind == InputEventKind_TextInsert){
                Temp_Memory_Block temp_key_line(scratch);
                String_Const_u8 key_line = stringize_keyboard_event(scratch, simulated_input);
                output_file_append(tctx, models, models->keyboard_buffer, key_line);
            }
        }
        
        b32 event_was_handled = false;
        Input_Event *event = simulated_input;
        
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
    
    linalloc_clear(&models->virtual_event_arena);
    models->free_virtual_event = 0;
    models->first_virtual_event = 0;
    models->last_virtual_event = 0;
    
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
    if (!models->keep_playing || input->trying_to_kill){
        co_send_core_event(tctx, models, CoreCode_TryExit);
        models->keep_playing = true;
    }
    
    // NOTE(allen): rendering
    {
        Frame_Info frame = {};
        frame.index = models->frame_counter;
        frame.literal_dt = literal_dt;
        frame.animation_dt = animation_dt;
        
        Application_Links app = {};
        app.tctx = tctx;
        app.cmd_context = models;
        
        if (models->tick != 0){
            models->tick(&app, frame);
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
                    render_caller(&app, frame, view_get_id(live_views, view));
                }
            }
        }
        
        if (models->whole_screen_render_caller != 0){
            models->whole_screen_render_caller(&app, frame);
        }
        
        models->in_render_mode = false;
        end_render_section(target);
    }
    
    // TODO(allen): This is dumb. Let's rethink view cleanup strategy.
    // NOTE(allen): wind down coroutines
    for (;;){
        Model_Wind_Down_Co *node = models->wind_down_stack;
        if (node == 0){
            break;
        }
        sll_stack_pop(models->wind_down_stack);
        Coroutine *co = node->co;
        
        for (i32 j = 0; co != 0; j += 1){
            Co_In in = {};
            in.user_input.abort = true;
            Co_Out ignore = {};
            co = co_run(tctx, models, co, &in, &ignore);
            if (j == 100){
                Application_Links app = {};
                app.tctx = tctx;
                app.cmd_context = models;
#define M "SERIOUS ERROR: coroutine wind down did not complete\n"
                print_message(&app, string_u8_litexpr(M));
#undef M
                break;
            }
        }
        
        sll_stack_push(models->free_wind_downs, node);
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
    app_result.perform_kill = models->hard_exit;
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

