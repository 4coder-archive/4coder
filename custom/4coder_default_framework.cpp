/*
4coder_default_framework.cpp - Sets up the basics of the framework that is used for default 4coder behaviour.
*/

// TOP

function void
unlock_jump_buffer(void){
    locked_buffer.size = 0;
}

function void
lock_jump_buffer(Application_Links *app, String_Const_u8 name){
    if (name.size < sizeof(locked_buffer_space)){
        block_copy(locked_buffer_space, name.str, name.size);
        locked_buffer = SCu8(locked_buffer_space, name.size);
        Scratch_Block scratch(app);
        String_Const_u8 escaped = string_escape(scratch, name);
        LogEventF(log_string(app, M), scratch, 0, 0, thread_get_id(app),
                  "lock jump buffer [name=\"%.*s\"]", string_expand(escaped));
    }
}

function void
lock_jump_buffer(Application_Links *app, char *name, i32 size){
    lock_jump_buffer(app, SCu8(name, size));
}

function void
lock_jump_buffer(Application_Links *app, Buffer_ID buffer_id){
    Scratch_Block scratch(app);
    String_Const_u8 buffer_name = push_buffer_unique_name(app, scratch, buffer_id);
    lock_jump_buffer(app, buffer_name);
}

function Buffer_ID
get_locked_jump_buffer(Application_Links *app){
    Buffer_ID result = 0;
    if (locked_buffer.size > 0){
        result = get_buffer_by_name(app, locked_buffer, Access_Always);
    }
    if (result == 0){
        unlock_jump_buffer();
    }
    return(result);
}

function View_ID
get_view_for_locked_jump_buffer(Application_Links *app){
    View_ID result = 0;
    Buffer_ID buffer = get_locked_jump_buffer(app);
    if (buffer != 0){
        result = get_first_view_with_buffer(app, buffer);
    }
    return(result);
}

////////////////////////////////

function void
new_view_settings(Application_Links *app, View_ID view){
    if (!global_config.use_scroll_bars){
        view_set_setting(app, view, ViewSetting_ShowScrollbar, false);
    }
    if (!global_config.use_file_bars){
        view_set_setting(app, view, ViewSetting_ShowFileBar, false);
    }
}

////////////////////////////////

function void
view_set_passive(Application_Links *app, View_ID view_id, b32 value){
    Managed_Scope scope = view_get_managed_scope(app, view_id);
    b32 *is_passive = scope_attachment(app, scope, view_is_passive_loc, b32);
    if (is_passive != 0){
        *is_passive = value;
    }
}

function b32
view_get_is_passive(Application_Links *app, View_ID view_id){
    Managed_Scope scope = view_get_managed_scope(app, view_id);
    b32 *is_passive = scope_attachment(app, scope, view_is_passive_loc, b32);
    b32 result = false;
    if (is_passive != 0){
        result = *is_passive;
    }
    return(result);
}

function View_ID
open_footer_panel(Application_Links *app, View_ID view){
    View_ID special_view = open_view(app, view, ViewSplit_Bottom);
    new_view_settings(app, special_view);
    Buffer_ID buffer = view_get_buffer(app, special_view, Access_Always);
    Face_ID face_id = get_face_id(app, buffer);
    Face_Metrics metrics = get_face_metrics(app, face_id);
    view_set_split_pixel_size(app, special_view, (i32)(metrics.line_height*14.f));
    view_set_passive(app, special_view, true);
    return(special_view);
}

function void
close_build_footer_panel(Application_Links *app){
    if (build_footer_panel_view_id != 0){
        view_close(app, build_footer_panel_view_id);
        build_footer_panel_view_id = 0;
    }
}

function View_ID
open_build_footer_panel(Application_Links *app){
    if (build_footer_panel_view_id == 0){
        View_ID view = get_active_view(app, Access_Always);
        build_footer_panel_view_id = open_footer_panel(app, view);
        view_set_active(app, view);
    }
    return(build_footer_panel_view_id);
}

function View_ID
get_next_view_looped_primary_panels(Application_Links *app, View_ID start_view_id, Access_Flag access){
    View_ID view_id = start_view_id;
    do{
        view_id = get_next_view_looped_all_panels(app, view_id, access);
        if (!view_get_is_passive(app, view_id)){
            break;
        }
    }while(view_id != start_view_id);
    return(view_id);
}

function View_ID
get_prev_view_looped_primary_panels(Application_Links *app, View_ID start_view_id, Access_Flag access){
    View_ID view_id = start_view_id;
    do{
        view_id = get_prev_view_looped_all_panels(app, view_id, access);
        if (!view_get_is_passive(app, view_id)){
            break;
        }
    }while(view_id != start_view_id);
    return(view_id);
}

function View_ID
get_next_view_after_active(Application_Links *app, Access_Flag access){
    View_ID view = get_active_view(app, access);
    if (view != 0){
        view = get_next_view_looped_primary_panels(app, view, access);
    }
    return(view);
}

////////////////////////////////

function void
call_after_ctx_shutdown(Application_Links *app, View_ID view, Custom_Command_Function *func){
    view_enqueue_command_function(app, view, func);
}

function Fallback_Dispatch_Result
fallback_command_dispatch(Application_Links *app, Mapping *mapping, Command_Map *map,
                          User_Input *in){
    Fallback_Dispatch_Result result = {};
    if (mapping != 0 && map != 0){
        Command_Binding binding = map_get_binding_recursive(mapping, map, &in->event);
        if (binding.custom != 0){
            Command_Metadata *metadata = get_command_metadata(binding.custom);
            if (metadata != 0){
                if (metadata->is_ui){
                    result.code = FallbackDispatch_DelayedUICall;
                    result.func = binding.custom;
                }
                else{
                    binding.custom(app);
                    result.code = FallbackDispatch_DidCall;
                }
            }
            else{
                binding.custom(app);
                result.code = FallbackDispatch_DidCall;
            }
        }
    }
    return(result);
}

function b32
ui_fallback_command_dispatch(Application_Links *app, View_ID view,
                             Mapping *mapping, Command_Map *map, User_Input *in){
    b32 result = false;
    Fallback_Dispatch_Result disp_result =
        fallback_command_dispatch(app, mapping, map, in);
    if (disp_result.code == FallbackDispatch_DelayedUICall){
        call_after_ctx_shutdown(app, view, disp_result.func);
        result = true;
    }
    if (disp_result.code == FallbackDispatch_Unhandled){
        leave_current_input_unhandled(app);
    }
    return(result);
}

function b32
ui_fallback_command_dispatch(Application_Links *app, View_ID view, User_Input *in){
    b32 result = false;
    View_Context ctx = view_current_context(app, view);
    if (ctx.mapping != 0){
        Command_Map *map = mapping_get_map(ctx.mapping, ctx.map_id);
        result = ui_fallback_command_dispatch(app, view, ctx.mapping, map, in);
    }
    else{
        leave_current_input_unhandled(app);
    }
    return(result);
}

////////////////////////////////

function void
view_buffer_set(Application_Links *app, Buffer_ID *buffers, i32 *positions, i32 count){
    if (count > 0){
        Scratch_Block scratch(app, Scratch_Share);
        
        struct View_Node{
            View_Node *next;
            View_ID view_id;
        };
        
        View_ID active_view_id = get_active_view(app, Access_Always);
        View_ID first_view_id = active_view_id;
        if (view_get_is_passive(app, active_view_id)){
            first_view_id = get_next_view_looped_primary_panels(app, active_view_id, Access_Always);
        }
        
        View_ID view_id = first_view_id;
        
        View_Node *primary_view_first = 0;
        View_Node *primary_view_last = 0;
        i32 available_view_count = 0;
        
        primary_view_first = primary_view_last = push_array(scratch, View_Node, 1);
        primary_view_last->next = 0;
        primary_view_last->view_id = view_id;
        available_view_count += 1;
        for (;;){
            view_id = get_next_view_looped_primary_panels(app, view_id, Access_Always);
            if (view_id == first_view_id){
                break;
            }
            View_Node *node = push_array(scratch, View_Node, 1);
            primary_view_last->next = node;
            node->next = 0;
            node->view_id = view_id;
            primary_view_last = node;
            available_view_count += 1;
        }
        
        i32 buffer_set_count = clamp_top(count, available_view_count);
        View_Node *node = primary_view_first;
        for (i32 i = 0; i < buffer_set_count; i += 1, node = node->next){
            if (view_set_buffer(app, node->view_id, buffers[i], 0)){
                view_set_cursor_and_preferred_x(app, node->view_id, seek_pos(positions[i]));
            }
        }
    }
}

////////////////////////////////

function void
change_active_panel_send_command(Application_Links *app, Custom_Command_Function *custom_func){
    View_ID view = get_active_view(app, Access_Always);
    view = get_next_view_looped_primary_panels(app, view, Access_Always);
    if (view != 0){
        view_set_active(app, view);
    }
    if (custom_func != 0){
        view_enqueue_command_function(app, view, custom_func);
    }
}

CUSTOM_COMMAND_SIG(change_active_panel)
CUSTOM_DOC("Change the currently active panel, moving to the panel with the next highest view_id.")
{
    change_active_panel_send_command(app, 0);
}

CUSTOM_COMMAND_SIG(change_active_panel_backwards)
CUSTOM_DOC("Change the currently active panel, moving to the panel with the next lowest view_id.")
{
    View_ID view = get_active_view(app, Access_Always);
    view = get_prev_view_looped_primary_panels(app, view, Access_Always);
    if (view != 0){
        view_set_active(app, view);
    }
}

CUSTOM_COMMAND_SIG(open_panel_vsplit)
CUSTOM_DOC("Create a new panel by vertically splitting the active panel.")
{
    View_ID view = get_active_view(app, Access_Always);
    View_ID new_view = open_view(app, view, ViewSplit_Right);
    new_view_settings(app, new_view);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    view_set_buffer(app, new_view, buffer, 0);
}

CUSTOM_COMMAND_SIG(open_panel_hsplit)
CUSTOM_DOC("Create a new panel by horizontally splitting the active panel.")
{
    View_ID view = get_active_view(app, Access_Always);
    View_ID new_view = open_view(app, view, ViewSplit_Bottom);
    new_view_settings(app, new_view);
    Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
    view_set_buffer(app, new_view, buffer, 0);
}

////////////////////////////////

// NOTE(allen): Credits to nj/FlyingSolomon for authoring the original version of this helper.

function Buffer_ID
create_or_switch_to_buffer_and_clear_by_name(Application_Links *app, String_Const_u8 name_string, View_ID default_target_view){
    Buffer_ID search_buffer = get_buffer_by_name(app, name_string, Access_Always);
    if (search_buffer != 0){
        buffer_set_setting(app, search_buffer, BufferSetting_ReadOnly, true);
        
        View_ID target_view = default_target_view;
        
        View_ID view_with_buffer_already_open = get_first_view_with_buffer(app, search_buffer);
        if (view_with_buffer_already_open != 0){
            target_view = view_with_buffer_already_open;
            // TODO(allen): there needs to be something like
            // view_exit_to_base_context(app, target_view);
            //view_end_ui_mode(app, target_view);
        }
        else{
            view_set_buffer(app, target_view, search_buffer, 0);
        }
        view_set_active(app, target_view);
        
        clear_buffer(app, search_buffer);
        buffer_send_end_signal(app, search_buffer);
    }
    else{
        search_buffer = create_buffer(app, name_string, BufferCreate_AlwaysNew);
        buffer_set_setting(app, search_buffer, BufferSetting_Unimportant, true);
        buffer_set_setting(app, search_buffer, BufferSetting_ReadOnly, true);
#if 0
        buffer_set_setting(app, search_buffer, BufferSetting_WrapLine, false);
#endif
        view_set_buffer(app, default_target_view, search_buffer, 0);
        view_set_active(app, default_target_view);
    }
    
    return(search_buffer);
}

////////////////////////////////

function void
set_mouse_suppression(b32 suppress){
    if (suppress){
        suppressing_mouse = true;
        system_show_mouse_cursor(MouseCursorShow_Never);
    }
    else{
        suppressing_mouse = false;
        system_show_mouse_cursor(MouseCursorShow_Always);
    }
}

CUSTOM_COMMAND_SIG(suppress_mouse)
CUSTOM_DOC("Hides the mouse and causes all mosue input (clicks, position, wheel) to be ignored.")
{
    set_mouse_suppression(true);
}

CUSTOM_COMMAND_SIG(allow_mouse)
CUSTOM_DOC("Shows the mouse and causes all mouse input to be processed normally.")
{
    set_mouse_suppression(false);
}

CUSTOM_COMMAND_SIG(toggle_mouse)
CUSTOM_DOC("Toggles the mouse suppression mode, see suppress_mouse and allow_mouse.")
{
    set_mouse_suppression(!suppressing_mouse);
}

CUSTOM_COMMAND_SIG(set_mode_to_original)
CUSTOM_DOC("Sets the edit mode to 4coder original.")
{
    fcoder_mode = FCoderMode_Original;
}

CUSTOM_COMMAND_SIG(set_mode_to_notepad_like)
CUSTOM_DOC("Sets the edit mode to Notepad like.")
{
    begin_notepad_mode(app);
}

CUSTOM_COMMAND_SIG(toggle_highlight_line_at_cursor)
CUSTOM_DOC("Toggles the line highlight at the cursor.")
{
    global_config.highlight_line_at_cursor = !global_config.highlight_line_at_cursor;
}

CUSTOM_COMMAND_SIG(toggle_highlight_enclosing_scopes)
CUSTOM_DOC("In code files scopes surrounding the cursor are highlighted with distinguishing colors.")
{
    global_config.use_scope_highlight = !global_config.use_scope_highlight;
}

CUSTOM_COMMAND_SIG(toggle_paren_matching_helper)
CUSTOM_DOC("In code files matching parentheses pairs are colored with distinguishing colors.")
{
    global_config.use_paren_helper = !global_config.use_paren_helper;
}

CUSTOM_COMMAND_SIG(toggle_fullscreen)
CUSTOM_DOC("Toggle fullscreen mode on or off.  The change(s) do not take effect until the next frame.")
{
    system_set_fullscreen(!system_is_fullscreen());
}

////////////////////////////////

function void
default_4coder_initialize(Application_Links *app, String_Const_u8_Array file_names,
                          i32 override_font_size, b32 override_hinting){
    Thread_Context *tctx = get_thread_context(app);
    heap_init(&global_heap, tctx->allocator);
    
#define M \
    "Welcome to " VERSION "\n" \
        "If you're new to 4coder there are some tutorials at http://4coder.net/tutorials.html\n" \
        "Direct bug reports and feature requests to https://github.com/4coder-editor/4coder/issues\n" \
        "Other questions and discussion can be directed to editor@4coder.net or 4coder.handmade.network\n" \
        "The change log can be found in CHANGES.txt\n" \
        "\n"
        print_message(app, string_u8_litexpr(M));
#undef M
    
#if 0
    load_folder_of_themes_into_live_set(app, &global_part, "themes");
#endif
    global_config_arena = reserve_arena(app);
    load_config_and_apply(app, global_config_arena, &global_config, override_font_size, override_hinting);
    
    // open command line files
    Scratch_Block scratch(app);
    String_Const_u8 hot_directory = push_hot_directory(app, scratch);
    for (i32 i = 0; i < file_names.count; i += 1){
        Temp_Memory_Block temp(scratch);
        String_Const_u8 input_name = file_names.vals[i];
        String_Const_u8 full_name = push_u8_stringf(scratch, "%.*s/%.*s",
                                                    string_expand(hot_directory),
                                                    string_expand(input_name));
        Buffer_ID new_buffer = create_buffer(app, full_name, BufferCreate_NeverNew|BufferCreate_MustAttachToFile);
        if (new_buffer == 0){
            create_buffer(app, input_name, 0);
        }
    }
}

function void
default_4coder_initialize(Application_Links *app,
                          i32 override_font_size, b32 override_hinting){
    String_Const_u8_Array file_names = {};
    default_4coder_initialize(app, file_names, override_font_size, override_hinting);
}

function void
default_4coder_initialize(Application_Links *app, String_Const_u8_Array file_names){
    Face_Description description = get_face_description(app, 0);
    default_4coder_initialize(app, file_names,
                              description.parameters.pt_size,
                              description.parameters.hinting);
}

function void
default_4coder_initialize(Application_Links *app){
    Face_Description command_line_description = get_face_description(app, 0);
    String_Const_u8_Array file_names = {};
    default_4coder_initialize(app, file_names, command_line_description.parameters.pt_size, command_line_description.parameters.hinting);
}

function void
default_4coder_side_by_side_panels(Application_Links *app,
                                   Buffer_Identifier left, Buffer_Identifier right){
    Buffer_ID left_id = buffer_identifier_to_id(app, left);
    Buffer_ID right_id = buffer_identifier_to_id(app, right);
    
    // Left Panel
    View_ID view = get_active_view(app, Access_Always);
    new_view_settings(app, view);
    view_set_buffer(app, view, left_id, 0);
    
    // Right Panel
    open_panel_vsplit(app);
    View_ID right_view = get_active_view(app, Access_Always);
    view_set_buffer(app, right_view, right_id, 0);
    
    // Restore Active to Left
    view_set_active(app, view);
}

function void
default_4coder_side_by_side_panels(Application_Links *app,
                                   Buffer_Identifier left, Buffer_Identifier right,
                                   String_Const_u8_Array file_names){
    if (file_names.count > 0){
        left = buffer_identifier(file_names.vals[0]);
        if (file_names.count > 1){
            right = buffer_identifier(file_names.vals[1]);
        }
    }
    default_4coder_side_by_side_panels(app, left, right);
}

function void
default_4coder_side_by_side_panels(Application_Links *app, String_Const_u8_Array file_names){
    Buffer_Identifier left = buffer_identifier(string_u8_litexpr("*scratch*"));
    Buffer_Identifier right = buffer_identifier(string_u8_litexpr("*messages*"));
    default_4coder_side_by_side_panels(app, left, right, file_names);
}

function void
default_4coder_side_by_side_panels(Application_Links *app){
    String_Const_u8_Array file_names = {};
    default_4coder_side_by_side_panels(app, file_names);
}

function void
default_4coder_one_panel(Application_Links *app, Buffer_Identifier buffer){
    Buffer_ID id = buffer_identifier_to_id(app, buffer);
    View_ID view = get_active_view(app, Access_Always);
    new_view_settings(app, view);
    view_set_buffer(app, view, id, 0);
}

function void
default_4coder_one_panel(Application_Links *app, String_Const_u8_Array file_names){
    Buffer_Identifier buffer = buffer_identifier(string_u8_litexpr("*messages*"));
    if (file_names.count > 0){
        buffer = buffer_identifier(file_names.vals[0]);
    }
    default_4coder_one_panel(app, buffer);
}

function void
default_4coder_one_panel(Application_Links *app){
    String_Const_u8_Array file_names = {};
    default_4coder_one_panel(app, file_names);
}

////////////////////////////////

function void
buffer_modified_set_init(void){
    Buffer_Modified_Set *set = &global_buffer_modified_set;
    
    block_zero_struct(set);
    Base_Allocator *allocator = get_base_allocator_system();
    set->arena = make_arena(allocator);
    set->id_to_node = make_table_u64_u64(allocator, 100);
}

function Buffer_Modified_Node*
buffer_modified_set__alloc_node(Buffer_Modified_Set *set){
    Buffer_Modified_Node *result = set->free;
    if (result == 0){
        result = push_array(&set->arena, Buffer_Modified_Node, 1);
    }
    else{
        sll_stack_pop(set->free);
    }
    return(result);
}

function void
buffer_mark_as_modified(Buffer_ID buffer){
    Buffer_Modified_Set *set = &global_buffer_modified_set;
    
    Table_Lookup lookup = table_lookup(&set->id_to_node, (u64)buffer);
    if (!lookup.found_match){
        Buffer_Modified_Node *node = buffer_modified_set__alloc_node(set);
        zdll_push_back(set->first, set->last, node);
        node->buffer = buffer;
        table_insert(&set->id_to_node, (u64)buffer, (u64)PtrAsInt(node));
    }
}

function void
buffer_unmark_as_modified(Buffer_ID buffer){
    Buffer_Modified_Set *set = &global_buffer_modified_set;
    
    Table_Lookup lookup = table_lookup(&set->id_to_node, (u64)buffer);
    if (lookup.found_match){
        u64 val = 0;
        table_read(&set->id_to_node, (u64)buffer, &val);
        Buffer_Modified_Node *node = (Buffer_Modified_Node*)IntAsPtr(val);
        zdll_remove(set->first, set->last, node);
        table_erase(&set->id_to_node, lookup);
        sll_stack_push(set->free, node);
    }
}

function void
buffer_modified_set_clear(void){
    Buffer_Modified_Set *set = &global_buffer_modified_set;
    
    table_clear(&set->id_to_node);
    if (set->last != 0){
        set->last->next = set->free;
        set->free = set->first;
        set->first = 0;
        set->last = 0;
    }
}

// BOTTOM

