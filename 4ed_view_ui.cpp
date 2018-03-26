/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.08.2015
 *
 * View GUI layouts and implementations
 *
 */

// TOP

internal Try_Kill_Result
interactive_try_kill_file(System_Functions *system, Models *models, Editing_File *file){
    Try_Kill_Result result = TryKill_CannotKill;
    if (!file->settings.never_kill){
        if (buffer_needs_save(file)){
            result = TryKill_NeedDialogue;
        }
        else{
            kill_file_and_update_views(system, models, file);
            result = TryKill_Success;
        }
    }
    return(result);
}

internal void
interactive_begin_sure_to_kill(System_Functions *system, View *view, Models *models, Editing_File *file){
    view_show_interactive(system, view, models, IAct_Sure_To_Kill, IInt_Sure_To_Kill, lit("Are you sure?"));
    copy(&view->transient.dest, file->unique_name.name);
}

internal void
interactive_view_complete(System_Functions *system, View *view, Models *models, String dest, i32 user_action){
    switch (view->transient.action){
        case IAct_Open:
        {
            Editing_File *file = open_file(system, models, dest);
            if (file != 0){
                view_set_file(system, models, view, file);
            }
            view_show_file(view);
        }break;
        
        case IAct_New:
        {
            if (dest.size > 0 && !char_is_slash(dest.str[dest.size-1])){
                Editing_File *file = 0;
                Editing_File_Name canon_name = {0};
                
                if (terminate_with_null(&dest) &&
                    get_canon_name(system, dest, &canon_name)){
                    Working_Set *working_set = &models->working_set;
                    file = working_set_contains_canon(working_set, canon_name.name);
                    if (file == 0){
                        Mem_Options *mem = &models->mem;
                        General_Memory *general = &mem->general;
                        Partition *part = &mem->part;
                        
                        file = working_set_alloc_always(working_set, general);
                        buffer_bind_file(system, general, working_set, file, canon_name.name);
                        buffer_bind_name(models, general, part, working_set, file, front_of_directory(dest));
                        
                        init_normal_file(system, models, 0, 0, file);
                    }
                    else{
                        edit_clear(system, models, file);
                    }
                }
                if (file != 0){
                    view_set_file(system, models, view, file);
                }
                view_show_file(view);
                if (file != 0 && models->hook_new_file != 0){
                    models->hook_new_file(&models->app_links, file->id.id);
                }
            }
        }break;
        
        case IAct_Switch:
        {
            Editing_File *file = working_set_contains_name(&models->working_set, dest);
            if (file){
                view_set_file(system, models, view, file);
            }
            view_show_file(view);
        }break;
        
        case IAct_Kill:
        {
            b32 kill_dialogue = false;
            Editing_File *file = working_set_contains_name(&models->working_set, dest);
            if (file != 0){
                kill_dialogue = (interactive_try_kill_file(system, models, file) == TryKill_NeedDialogue);
                if (kill_dialogue){
                    interactive_begin_sure_to_kill(system, view, models, file);
                }
            }
            if (!kill_dialogue){
                view_show_file(view);
            }
        }break;
        
        case IAct_Sure_To_Close:
        {
            switch (user_action){
                case UnsavedChangesUserResponse_ContinueAnyway:
                {
                    models->keep_playing = false;
                }break;
                
                case UnsavedChangesUserResponse_Cancel:
                {
                    view_show_file(view);
                }break;
                
                // TODO(allen): Save all and close.
                case UnsavedChangesUserResponse_SaveAndContinue: InvalidCodePath;
                default: InvalidCodePath;
            }
        }break;
        
        case IAct_Sure_To_Kill:
        {
            switch (user_action){
                case UnsavedChangesUserResponse_ContinueAnyway:
                {
                    Editing_File *file = working_set_contains_name(&models->working_set, dest);
                    if (file != 0){
                        kill_file_and_update_views(system, models, file);
                    }
                    view_show_file(view);
                }break;
                
                case UnsavedChangesUserResponse_Cancel:
                {
                    view_show_file(view);
                }break;
                
                case UnsavedChangesUserResponse_SaveAndContinue:
                {
                    Editing_File *file = working_set_contains_name(&models->working_set, dest);
                    if (file != 0){
                        save_file(system, models, file);
                        kill_file_and_update_views(system, models, file);
                    }
                    view_show_file(view);
                }break;
                
                default: InvalidCodePath;
            }
        }break;
        
        case IAct_OpenOrNew: InvalidCodePath;
    }
}

////////////////////////////////

internal Single_Line_Input_Step
app_single_line_input__inner(System_Functions *system, Working_Set *working_set, Key_Event_Data key, Single_Line_Mode mode){
    Single_Line_Input_Step result = {0};
    
    b8 ctrl = key.modifiers[MDFR_CONTROL_INDEX];
    b8 cmnd = key.modifiers[MDFR_COMMAND_INDEX];
    b8 alt  = key.modifiers[MDFR_ALT_INDEX];
    b8 is_modified = (ctrl || cmnd || alt);
    
    if (key.keycode == key_back){
        result.hit_backspace = true;
        if (mode.string->size > 0){
            result.made_a_change = true;
            --mode.string->size;
            b32 was_slash = char_is_slash(mode.string->str[mode.string->size]);
            terminate_with_null(mode.string);
            if (mode.type == SINGLE_LINE_FILE && was_slash && !is_modified){
                remove_last_folder(mode.string);
                terminate_with_null(mode.string);
                hot_directory_set(system, mode.hot_directory, *mode.string);
            }
        }
    }
    
    else if (key.character == '\n' || key.character == '\t'){
        // NOTE(allen): do nothing!
    }
    
    else if (key.keycode == key_esc){
        result.hit_esc = true;
        result.made_a_change = true;
    }
    
    else if (key.character != 0){
        result.hit_a_character = true;
        if (!is_modified){
            u8 c[4];
            u32 c_len = 0;
            u32_to_utf8_unchecked(key.character, c, &c_len);
            Assert(mode.string->memory_size >= 0);
            if (mode.string->size + c_len < (u32)mode.string->memory_size){
                result.made_a_change = true;
                append(mode.string, make_string(c, c_len));
                terminate_with_null(mode.string);
                if (mode.type == SINGLE_LINE_FILE && char_is_slash(c[0])){
                    hot_directory_set(system, mode.hot_directory, *mode.string);
                }
            }
        }
        else{
            result.did_command = true;
        }
    }
    
    return(result);
}

inline Single_Line_Input_Step
app_single_line_input_step(System_Functions *system, Key_Event_Data key, String *string){
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_STRING;
    mode.string = string;
    return(app_single_line_input__inner(system, 0, key, mode));
}

inline Single_Line_Input_Step
app_single_file_input_step(System_Functions *system,
                           Working_Set *working_set, Key_Event_Data key,
                           String *string, Hot_Directory *hot_directory){
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_FILE;
    mode.string = string;
    mode.hot_directory = hot_directory;
    return(app_single_line_input__inner(system, working_set, key, mode));
}


////////////////////////////////

global_const Style_Color_Edit colors_to_edit[] = {
    {Stag_Back, Stag_Default, Stag_Back, lit("Background")},
    {Stag_Margin, Stag_Default, Stag_Margin, lit("Margin")},
    {Stag_Margin_Hover, Stag_Default, Stag_Margin_Hover, lit("Margin Hover")},
    {Stag_Margin_Active, Stag_Default, Stag_Margin_Active, lit("Margin Active")},
    
    {Stag_Cursor, Stag_At_Cursor, Stag_Cursor, lit("Cursor")},
    {Stag_At_Cursor, Stag_At_Cursor, Stag_Cursor, lit("Text At Cursor")},
    {Stag_Mark, Stag_Mark, Stag_Back, lit("Mark")},
    
    {Stag_Highlight, Stag_At_Highlight, Stag_Highlight, lit("Highlight")},
    {Stag_At_Highlight, Stag_At_Highlight, Stag_Highlight, lit("Text At Highlight")},
    
    {Stag_Default, Stag_Default, Stag_Back, lit("Text Default")},
    {Stag_Comment, Stag_Comment, Stag_Back, lit("Comment")},
    {Stag_Keyword, Stag_Keyword, Stag_Back, lit("Keyword")},
    {Stag_Str_Constant, Stag_Str_Constant, Stag_Back, lit("String Constant")},
    {Stag_Char_Constant, Stag_Char_Constant, Stag_Back, lit("Character Constant")},
    {Stag_Int_Constant, Stag_Int_Constant, Stag_Back, lit("Integer Constant")},
    {Stag_Float_Constant, Stag_Float_Constant, Stag_Back, lit("Float Constant")},
    {Stag_Bool_Constant, Stag_Bool_Constant, Stag_Back, lit("Boolean Constant")},
    {Stag_Preproc, Stag_Preproc, Stag_Back, lit("Preprocessor")},
    {Stag_Special_Character, Stag_Special_Character, Stag_Back, lit("Special Character")},
    
    {Stag_Highlight_Junk, Stag_Default, Stag_Highlight_Junk, lit("Junk Highlight")},
    {Stag_Highlight_White, Stag_Default, Stag_Highlight_White, lit("Whitespace Highlight")},
    
    {Stag_Paste, Stag_Paste, Stag_Back, lit("Paste Color")},
    
    {Stag_Bar, Stag_Base, Stag_Bar, lit("Bar")},
    {Stag_Base, Stag_Base, Stag_Bar, lit("Bar Text")},
    {Stag_Pop1, Stag_Pop1, Stag_Bar, lit("Bar Pop 1")},
    {Stag_Pop2, Stag_Pop2, Stag_Bar, lit("Bar Pop 2")},
};

internal View_Step_Result
step_view(System_Functions *system, View *view, Models *models, View *active_view, Input_Summary input){
    View_Step_Result result = {0};
    GUI_Target *target = &view->transient.gui_target;
    Key_Input_Data keys = input.keys;
    
    b32 show_scrollbar = !view->transient.hide_scrollbar;
    
    if (view->transient.showing_ui != VUI_None){
        b32 did_esc = false;
        for (i32 i = 0; i < keys.count; ++i){
            Key_Event_Data key = keys.keys[i];
            if (key.keycode == key_esc){
                did_esc = 1;
                break;
            }
        }
        
        if (did_esc){
            view_show_file(view);
            result.consume_esc = true;
        }
    }
    
    gui_begin_top_level(target, input);
    if (!view->transient.hide_file_bar){
        gui_do_top_bar(target);
    }
    
    // NOTE(allen): A temporary measure... although in
    // general we maybe want the user to be able to ask
    // how large a particular section of the GUI turns
    // out to be after layout?
    i32 bar_count = 0;
    for (Query_Slot *slot = view->transient.query_set.used_slot;
         slot != 0;
         slot = slot->next, ++bar_count){
        Query_Bar *bar = slot->query_bar;
        gui_do_text_field(target, bar->prompt, bar->string);
    }
    view->transient.widget_height = (f32)bar_count*(view->transient.line_height + 2);
    
    Editing_File *file = view->transient.file_data.file;
    Assert(file != 0);
    Assert(view->transient.edit_pos != 0);
    
    switch (view->transient.showing_ui){
        case VUI_None:
        {
            gui_begin_serial_section(target);
            
            i32 delta = 9*view->transient.line_height;
            GUI_id scroll_context = gui_id((u64)(file), view->transient.showing_ui);
            
            GUI_Scroll_Vars *scroll = &view->transient.edit_pos->scroll;
            gui_begin_scrollable(target, scroll_context, *scroll,
                                 delta, show_scrollbar);
            gui_do_file(target);
            gui_end_scrollable(target);
            
            gui_end_serial_section(target);
        }break;
        
        case VUI_Theme:
        {
            u64 high_id = VUI_Theme + ((u64)view->transient.color_mode << 32);
            GUI_id scroll_context = gui_id(0, high_id);
            
            switch (view->transient.color_mode){
                case CV_Mode_Library:
                {
                    gui_do_text_field(target, lit("Current Theme - Click to Edit"), make_string(0, 0));
                    
                    if (gui_do_style_preview(target, gui_id((u64)(&models->styles.styles[0]), high_id), 0)){
                        view->transient.color_mode = CV_Mode_Adjusting;
                    }
                    
                    if (gui_do_button(target, gui_id((u64)(&file->settings.font_id), high_id), lit("Set Font"))){
                        view->transient.color_mode = CV_Mode_Font;
                    }
                    
                    if (gui_do_button(target, gui_id((u64)(&models->global_font_id), high_id), lit("Set Global Font"))){
                        view->transient.color_mode = CV_Mode_Global_Font;
                    }
                    
                    gui_do_text_field(target, lit("Theme Library - Click to Select"), make_string(0, 0));
                    
                    gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll,
                                         9*view->transient.line_height, show_scrollbar);
                    
                    Style *style = models->styles.styles + 1;
                    i32 count = models->styles.count;
                    for (i32 i = 1; i < count; ++i, ++style){
                        if (gui_do_style_preview(target, gui_id((u64)(style), high_id), i)){
                            style_copy(&models->styles.styles[0], style);
                        }
                    }
                    
                    gui_end_scrollable(target);
                }break;
                
                case CV_Mode_Font:
                case CV_Mode_Global_Font:
                {
                    if (gui_do_button(target, gui_id(1, high_id), lit("Back"))){
                        view->transient.color_mode = CV_Mode_Library;
                    }
                    
                    gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll, 9*view->transient.line_height, show_scrollbar);
                    
                    Face_ID font_id = file->settings.font_id;
                    Face_ID new_font_id = 0;
                    
                    Face_ID largest_id = system->font.get_largest_id();
                    for (Face_ID i = 1; i <= largest_id; ++i){
                        Font_Pointers font = system->font.get_pointers_by_id(i);
                        if (font.valid){
                            Font_Settings *settings = font.settings;
                            Font_Metrics *metrics = font.metrics;
                            
                            char space[512];
                            String m = make_fixed_width_string(space);
                            if (i == font_id){
                                append(&m, "*");
                            }
                            
                            append(&m, " \"");
                            append(&m, make_string(metrics->name, metrics->name_len));
                            append(&m, "\" ");
                            append_int_to_str(&m, settings->parameters.pt_size);
                            append(&m, " ");
                            append(&m, (char*)(settings->parameters.italics?"italics ":""));
                            append(&m, (char*)(settings->parameters.bold?"bold ":""));
                            append(&m, (char*)(settings->parameters.underline?"underline ":""));
                            append(&m, (char*)(settings->parameters.use_hinting?"hinting ":""));
                            
                            if (i == font_id){
                                append(&m, "*");
                            }
                            
                            if (gui_do_button(target, gui_id(i*2, high_id), m)){
                                if (new_font_id == 0){
                                    new_font_id = i;
                                }
                            }
                            
                            if (gui_do_button(target, gui_id(i*2 + 1, high_id), lit("edit"))){
                                view->transient.font_edit_id = i;
                                if (view->transient.color_mode == CV_Mode_Font){
                                    view->transient.color_mode = CV_Mode_Font_Editing;
                                }
                                else{
                                    view->transient.color_mode = CV_Mode_Global_Font_Editing;
                                }
                            }
                        }
                    }
                    
                    if (gui_do_button(target, gui_id(largest_id*2 + 2, high_id), lit("new face"))){
                        if (new_font_id == 0){
                            Font_Pointers font = system->font.get_pointers_by_id(font_id);
                            view->transient.font_edit_id = system->font.face_allocate_and_init(font.settings);
                            if (view->transient.color_mode == CV_Mode_Font){
                                view->transient.color_mode = CV_Mode_Font_Editing;
                            }
                            else{
                                view->transient.color_mode = CV_Mode_Global_Font_Editing;
                            }
                        }
                    }
                    
                    if (new_font_id != 0){
                        if (view->transient.color_mode == CV_Mode_Font && new_font_id != font_id){
                            file_set_font(system, models, file, new_font_id);
                        }
                        else if (new_font_id != font_id || new_font_id != models->global_font_id){
                            global_set_font_and_update_files(system, models, new_font_id);
                        }
                    }
                    
                    gui_end_scrollable(target);
                }break;
                
                case CV_Mode_Font_Editing:
                case CV_Mode_Global_Font_Editing:
                {
                    i32 low_id = 1;
                    if (gui_do_button(target, gui_id(low_id++, high_id), lit("Back"))){
                        if (view->transient.color_mode == CV_Mode_Font_Editing){
                            view->transient.color_mode = CV_Mode_Font;
                        }
                        else{
                            view->transient.color_mode = CV_Mode_Global_Font;
                        }
                    }
                    
                    gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll, 9*view->transient.line_height, show_scrollbar);
                    
                    Face_ID font_edit_id = view->transient.font_edit_id;
                    Font_Pointers font = system->font.get_pointers_by_id(font_edit_id);
                    Font_Settings *settings = font.settings;
                    Font_Metrics *metrics = font.metrics;
                    Font_Settings new_settings = *settings;
                    b32 has_new_settings = false;
                    
                    char space[128];
                    String m = make_fixed_width_string(space);
                    copy(&m, "Size Up (");
                    append_int_to_str(&m, settings->parameters.pt_size);
                    append(&m, ")");
                    if (gui_do_button(target, gui_id(low_id++, high_id), m)){
                        if (!has_new_settings){
                            has_new_settings = true;
                            ++new_settings.parameters.pt_size;
                        }
                    }
                    
                    copy(&m, "Size Down (");
                    append_int_to_str(&m, settings->parameters.pt_size);
                    append(&m, ")");
                    if (gui_do_button(target, gui_id(low_id++, high_id), m)){
                        if (!has_new_settings){
                            has_new_settings = true;
                            --new_settings.parameters.pt_size;
                        }
                    }
                    
                    copy(&m, "Italics [");
                    append(&m, (char*)(settings->parameters.italics?"+":" "));
                    append(&m, "]");
                    if (gui_do_button(target, gui_id(low_id++, high_id), m)){
                        if (!has_new_settings){
                            has_new_settings = true;
                            new_settings.parameters.italics = !new_settings.parameters.italics;
                        }
                    }
                    
                    copy(&m, "Bold [");
                    append(&m, (char*)(settings->parameters.bold?"+":" "));
                    append(&m, "]");
                    if (gui_do_button(target, gui_id(low_id++, high_id), m)){
                        if (!has_new_settings){
                            has_new_settings = true;
                            new_settings.parameters.bold = !new_settings.parameters.bold;
                        }
                    }
                    
                    copy(&m, "Underline [");
                    append(&m, (char*)(settings->parameters.underline?"+":" "));
                    append(&m, "]");
                    if (gui_do_button(target, gui_id(low_id++, high_id), m)){
                        if (!has_new_settings){
                            has_new_settings = true;
                            new_settings.parameters.underline = !new_settings.parameters.underline;
                        }
                    }
                    
                    copy(&m, "Hinting [");
                    append(&m, (char*)(settings->parameters.use_hinting?"+":" "));
                    append(&m, "]");
                    if (gui_do_button(target, gui_id(low_id++, high_id), m)){
                        if (!has_new_settings){
                            has_new_settings = true;
                            new_settings.parameters.use_hinting = !new_settings.parameters.use_hinting;
                        }
                    }
                    
                    copy(&m, "Current Family: ");
                    append(&m, make_string(metrics->name, metrics->name_len));
                    gui_do_button(target, gui_id(low_id++, high_id), m);
                    
                    i32 total_count = system->font.get_loadable_count();
                    for (i32 i = 0; i < total_count; ++i){
                        Font_Loadable_Description loadable = {0};
                        system->font.get_loadable(i, &loadable);
                        
                        if (loadable.valid){
                            String name = make_string(loadable.display_name, loadable.display_len);
                            if (gui_do_button(target, gui_id(low_id++, high_id), name)){
                                if (!has_new_settings){
                                    has_new_settings = true;
                                    memcpy(&new_settings.stub, &loadable.stub, sizeof(loadable.stub));
                                }
                            }
                        }
                    }
                    
                    gui_end_scrollable(target);
                    
                    if (has_new_settings){
                        alter_font_and_update_files(system, models, font_edit_id, &new_settings);
                    }
                }break;
                
                case CV_Mode_Adjusting:
                {
                    if (gui_do_button(target, gui_id(1, high_id), lit("Back"))){
                        view->transient.color_mode = CV_Mode_Library;
                    }
                    
                    gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll, 9*view->transient.line_height, show_scrollbar);
                    
                    i32 next_color_editing = view->transient.current_color_editing;
                    
                    Style *style = &models->styles.styles[0];
                    for (i32 i = 0; i < ArrayCount(colors_to_edit); ++i){
                        u32 *edit_color = style_index_by_tag(&style->main, colors_to_edit[i].target);
                        
                        u32 *fore = style_index_by_tag(&style->main, colors_to_edit[i].fore);
                        u32 *back = style_index_by_tag(&style->main, colors_to_edit[i].back);
                        
                        if (gui_do_color_button(target, gui_id((u64)(edit_color), high_id),
                                                *fore, *back, colors_to_edit[i].text)){
                            next_color_editing = i;
                            view->transient.color_cursor = 0;
                        }
                        
                        if (view->transient.current_color_editing == i){
                            GUI_Item_Update update = {0};
                            char text_space[7];
                            String text = make_fixed_width_string(text_space);
                            
                            color_to_hexstr(&text, *edit_color);
                            if (gui_do_text_with_cursor(target, view->transient.color_cursor, text, &update)){
                                b32 rollback = false;
                                
                                for (i32 j = 0; j < keys.count; ++j){
                                    Key_Code key = keys.keys[j].keycode;
                                    b32 set_consume_keys = true;
                                    switch (key){
                                        case key_left:
                                        {
                                            --view->transient.color_cursor;
                                            rollback = true;
                                        }break;
                                        
                                        case key_right:
                                        {
                                            ++view->transient.color_cursor;
                                            rollback = true;
                                        }break;
                                        
                                        case key_up:
                                        {
                                            --next_color_editing;
                                        }break;
                                        
                                        case key_down:
                                        {
                                            ++next_color_editing;
                                        }break;
                                        
                                        default:
                                        {
                                            if (char_is_hex((char)key)){
                                                text.str[view->transient.color_cursor] = (char)key;
                                                rollback = true;
                                            }
                                            else{
                                                set_consume_keys = false;
                                            }
                                        }break;
                                    }
                                    result.consume_keys = result.consume_keys || set_consume_keys;
                                    view->transient.color_cursor = clamp(0, view->transient.color_cursor, 5);
                                    next_color_editing = clamp(0, next_color_editing, ArrayCount(colors_to_edit) - 1);
                                }
                                
                                if (rollback){
                                    hexstr_to_color(text, edit_color);
                                    gui_rollback(target, &update);
                                    gui_do_text_with_cursor(target, view->transient.color_cursor, text, 0);
                                }
                            }
                        }
                    }
                    
                    if (view->transient.current_color_editing != next_color_editing){
                        view->transient.current_color_editing = next_color_editing;
                        view->transient.color_cursor = 0;
                    }
                    
                    gui_end_scrollable(target);
                }break;
            }
        }break;
        
        case VUI_Interactive:
        {
            b32 complete = 0;
            char comp_dest_space[1024];
            String comp_dest = make_fixed_width_string(comp_dest_space);
            i32 comp_action = 0;
            
            u64 high_id = VUI_Interactive + ((u64)view->transient.interaction << 32);
            GUI_id scroll_context = gui_id(0, high_id);
            
            Key_Code user_up_key = models->user_up_key;
            Key_Code user_down_key = models->user_down_key;
            Key_Modifier user_up_key_modifier = models->user_up_key_modifier;
            Key_Modifier user_down_key_modifier = models->user_down_key_modifier;
            
            switch (view->transient.interaction){
                case IInt_Sys_File_List:
                {
                    b32 autocomplete_with_enter = true;
                    b32 activate_directly = false;
                    
                    if (view->transient.action == IAct_New){
                        autocomplete_with_enter = false;
                    }
                    
                    String message = null_string;
                    switch (view->transient.action){
                        case IAct_OpenOrNew:
                        case IAct_Open:
                        {
                            message = lit("Open: ");
                        }break;
                        
                        case IAct_New:
                        {
                            message = lit("New: ");
                        }break;
                    }
                    
                    GUI_Item_Update update = {0};
                    Hot_Directory *hdir = &models->hot_directory;
                    
                    b32 do_open_or_new = false;
                    for (i32 i = 0; i < keys.count; ++i){
                        Key_Event_Data key = keys.keys[i];
                        Single_Line_Input_Step step = app_single_file_input_step(system, &models->working_set, key,
                                                                                 &hdir->string, hdir);
                        
                        if (step.made_a_change){
                            view->transient.list_i = 0;
                            result.consume_keys = true;
                        }
                        
                        if (key.keycode == '\n'){
                            if (!autocomplete_with_enter){
                                activate_directly = true;
                                result.consume_keys = true;
                            }
                            else if (view->transient.action == IAct_OpenOrNew){
                                do_open_or_new = true;
                                result.consume_keys = true;
                            }
                        }
                    }
                    
                    gui_do_text_field(target, message, hdir->string);
                    
                    b32 snap_into_view = false;
                    scroll_context.id[0] = (u64)(hdir);
                    if (gui_scroll_was_activated(target, scroll_context)){
                        snap_into_view = true;
                    }
                    gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll, 9*view->transient.line_height, show_scrollbar);
                    
                    GUI_id list_id = gui_id((u64)(hdir) + 1, high_id);
                    if (gui_begin_list(target, list_id,
                                       view->transient.list_i, 0, snap_into_view, &update)){
                        // TODO(allen): Allow me to handle key consumption correctly here!
                        gui_standard_list(target, list_id, &view->transient.gui_scroll, view->transient.scroll_region, &keys, &view->transient.list_i, &update, user_up_key, user_up_key_modifier, user_down_key, user_down_key_modifier);
                    }
                    
                    b32 do_new_directory = false;
                    
                    Working_Set *working_set = &models->working_set;
                    
                    char full_path_space[256];
                    String full_path = make_fixed_width_string(full_path_space);
                    copy(&full_path, hdir->canon_dir);
                    
                    char front_name_space[256];
                    String front_name = make_fixed_width_string(front_name_space);
                    copy(&front_name, front_of_directory(hdir->string));
                    
                    Absolutes absolutes;
                    get_absolutes(front_name, &absolutes, true, true);
                    
                    i32 full_path_restore = full_path.size;
                    
                    File_Info *info_ptr = hdir->file_list.infos;
                    i32 file_count = hdir->file_list.count;
                    for (i32 i = 0; i < file_count; ++i, ++info_ptr){
                        
                        full_path.size = full_path_restore;
                        append(&full_path, info_ptr->filename);
                        terminate_with_null(&full_path);
                        
                        String filename = make_string_cap(info_ptr->filename, info_ptr->filename_len, info_ptr->filename_len + 1);
                        
                        if (wildcard_match_s(&absolutes, filename, false)){
                            Editing_File *file_ptr = working_set_contains_canon(working_set, full_path);
                            
                            String message = {0};
                            if (file_ptr != 0 && file_is_ready(file_ptr)){
                                switch (file_ptr->state.dirty){
                                    case DirtyState_UpToDate:
                                    {
                                        message = lit(" LOADED");
                                    }break;
                                    
                                    case DirtyState_UnsavedChanges:
                                    {
                                        message = lit(" LOADED *");
                                    }break;
                                    
                                    case DirtyState_UnloadedChanges:
                                    {
                                        message = lit(" LOADED !");
                                    }break;
                                    
                                    default: InvalidCodePath;
                                }
                            }
                            
                            char *str = info_ptr->filename;
                            i32 len = info_ptr->filename_len;
                            String filename = make_string_cap(str, len, len + 1);
                            b32 is_folder = info_ptr->folder;
                            
                            if (gui_do_file_option(target, gui_id((u64)(info_ptr), high_id), filename, is_folder, message)){
                                if (is_folder){
                                    set_last_folder(&hdir->string, info_ptr->filename, '/');
                                    do_new_directory = true;
                                }
                                
                                else if (autocomplete_with_enter){
                                    complete = true;
                                    copy(&comp_dest, full_path);
                                    if (view->transient.action == IAct_OpenOrNew){
                                        view->transient.action = IAct_Open;
                                    }
                                }
                            }
                            
                            if (do_open_or_new){
                                do_open_or_new = false;
                            }
                        }
                    }
                    
                    gui_end_list(target);
                    
                    if (activate_directly || do_open_or_new){
                        complete = true;
                        copy(&comp_dest, hdir->string);
                        if (do_open_or_new){
                            view->transient.action = IAct_New;
                        }
                    }
                    
                    if (do_new_directory){
                        hot_directory_reload(system, hdir);
                    }
                    
                    gui_end_scrollable(target);
                }break;
                
                case IInt_Live_File_List:
                {
                    String message = null_string;
                    switch (view->transient.action){
                        case IAct_Switch:
                        {
                            message = lit("Switch: ");
                        }break;
                        
                        case IAct_Kill:
                        {
                            message = lit("Kill: ");
                        }break;
                    }
                    
                    Working_Set *working_set = &models->working_set;
                    Editing_Layout *layout = &models->layout;
                    GUI_Item_Update update = {0};
                    
                    for (i32 i = 0; i < keys.count; ++i){
                        Key_Event_Data key = keys.keys[i];
                        Single_Line_Input_Step step = app_single_line_input_step(system, key, &view->transient.dest);
                        if (step.made_a_change){
                            view->transient.list_i = 0;
                            result.consume_keys = 1;
                        }
                    }
                    
                    Absolutes absolutes = {0};
                    get_absolutes(view->transient.dest, &absolutes, 1, 1);
                    
                    gui_do_text_field(target, message, view->transient.dest);
                    
                    b32 snap_into_view = 0;
                    scroll_context.id[0] = (u64)(working_set);
                    if (gui_scroll_was_activated(target, scroll_context)){
                        snap_into_view = 1;
                    }
                    gui_begin_scrollable(target, scroll_context, view->transient.gui_scroll, 9*view->transient.line_height, show_scrollbar);
                    
                    GUI_id list_id = gui_id((u64)(working_set) + 1, high_id);
                    if (gui_begin_list(target, list_id, view->transient.list_i, 0, snap_into_view, &update)){
                        gui_standard_list(target, list_id, &view->transient.gui_scroll, view->transient.scroll_region, &keys, &view->transient.list_i, &update, user_up_key, user_up_key_modifier, user_down_key, user_down_key_modifier);
                    }
                    
                    File_Node *node = working_set->used_sentinel.next;
                    Assert(node != &working_set->used_sentinel);
                    
                    Partition *part = &models->mem.part;
                    Temp_Memory temp = begin_temp_memory(part);
                    partition_align(part, 8);
                    Editing_File **reserved_ptr = push_array(part, Editing_File*, 0);
                    Editing_File **reserved_end = 0;
                    
                    for (;reserved_ptr != reserved_end;){
                        Editing_File *file_ptr = 0;
                        b32 emit_this_file = false;
                        
                        if (reserved_end == 0){
                            file_ptr = (Editing_File*)node;
                            Assert(!file_ptr->is_dummy);
                            if (wildcard_match_s(&absolutes, file_ptr->unique_name.name, 0) != 0){
                                if (file_is_viewed(layout, file_ptr) || file_ptr->unique_name.name.str[0] == '*'){
                                    Editing_File **reserved = push_array(part, Editing_File*, 1);
                                    *reserved = file_ptr;
                                }
                                else{
                                    emit_this_file = true;
                                }
                            }
                            node = node->next;
                            if (node == &working_set->used_sentinel){
                                reserved_end = push_array(part, Editing_File*, 0);
                            }
                        }
                        else{
                            file_ptr = *reserved_ptr;
                            ++reserved_ptr;
                            emit_this_file = true;
                        }
                        
                        if (emit_this_file){
                            String message = {0};
                            switch (file_ptr->state.dirty){
                                case DirtyState_UnloadedChanges:
                                {
                                    message = lit(" *");
                                }break;
                                
                                case DirtyState_UnsavedChanges:
                                {
                                    message = lit(" !");
                                }break;
                            }
                            if (gui_do_file_option(target, gui_id((u64)(file_ptr), high_id), file_ptr->unique_name.name, 0, message)){
                                complete = true;
                                copy(&comp_dest, file_ptr->unique_name.name);
                            }
                        }
                    }
                    end_temp_memory(temp);
                    gui_end_list(target);
                    gui_end_scrollable(target);
                }break;
                
                case IInt_Sure_To_Close:
                {
                    Unsaved_Changes_User_Response response = UnsavedChangesUserResponse_Error;
                    
                    gui_do_text_field(target, lit("There are one or more files unsaved changes, close anyway?"), lit(""));
                    
                    if (gui_do_fixed_option(target, gui_id('y', high_id), lit("(Y)es"), 'y')){
                        response = UnsavedChangesUserResponse_ContinueAnyway;
                    }
                    
                    if (gui_do_fixed_option(target, gui_id('n', high_id), lit("(N)o"), 'n')){
                        response = UnsavedChangesUserResponse_Cancel;
                    }
                    
                    if (response != UnsavedChangesUserResponse_Error){
                        complete = true;
                        copy(&comp_dest, view->transient.dest);
                        comp_action = response;
                    }
                }break;
                
                case IInt_Sure_To_Kill:
                {
                    Unsaved_Changes_User_Response response = UnsavedChangesUserResponse_Error;
                    
                    gui_do_text_field(target, lit("There are unsaved changes, close anyway?"), lit(""));
                    
                    if (gui_do_fixed_option(target, gui_id('y', high_id), lit("(Y)es"), 'y')){
                        response = UnsavedChangesUserResponse_ContinueAnyway;
                    }
                    
                    if (gui_do_fixed_option(target, gui_id('n', high_id), lit("(N)o"), 'n')){
                        response = UnsavedChangesUserResponse_Cancel;
                    }
                    
                    if (gui_do_fixed_option(target, gui_id('s', high_id), lit("(S)ave and kill"), 's')){
                        response = UnsavedChangesUserResponse_SaveAndContinue;
                    }
                    
                    if (response != UnsavedChangesUserResponse_Error){
                        complete = true;
                        copy(&comp_dest, view->transient.dest);
                        comp_action = response;
                    }
                }break;
            }
            
            if (complete){
                terminate_with_null(&comp_dest);
                interactive_view_complete(system, view, models, comp_dest, comp_action);
            }
        }break;
    }
    gui_end_top_level(target);
    
    result.animating = target->animating;
    return(result);
}

internal b32
click_button_input(GUI_Target *target, GUI_Session *session, b32 in_scroll, i32_Rect scroll_rect, Input_Summary *user_input, GUI_Interactive *b, b32 *is_animating){
    b32 result = false;
    i32 mx = user_input->mouse.x;
    i32 my = user_input->mouse.y;
    
    b32 in_sub_region = true;
    if (in_scroll && !hit_check(mx, my, scroll_rect)){
        in_sub_region = false;
    }
    
    if (in_sub_region && hit_check(mx, my, session->rect)){
        target->hover = b->id;
        if (user_input->mouse.press_l){
            target->mouse_hot = b->id;
            *is_animating = true;
            result = true;
        }
        if (user_input->mouse.release_l && gui_id_eq(target->mouse_hot, b->id)){
            target->active = b->id;
            memset(&target->mouse_hot, 0, sizeof(target->mouse_hot));
            *is_animating = true;
        }
    }
    else if (gui_id_eq(target->hover, b->id)){
        memset(&target->hover, 0, sizeof(target->hover));
    }
    
    return(result);
}

internal b32
scroll_button_input(GUI_Target *target, GUI_Session *session, Input_Summary *user_input, GUI_id id, b32 *is_animating){
    b32 result = false;
    i32 mx = user_input->mouse.x;
    i32 my = user_input->mouse.y;
    
    if (hit_check(mx, my, session->rect)){
        target->hover = id;
        if (user_input->mouse.l){
            target->mouse_hot = id;
            gui_activate_scrolling(target);
            *is_animating = true;
            result = true;
        }
    }
    else if (gui_id_eq(target->hover, id)){
        memset(&target->hover, 0, sizeof(target->hover));
    }
    
    return(result);
}

internal Input_Process_Result
do_step_file_view(System_Functions *system, View *view, Models *models, i32_Rect rect, b32 is_active, Input_Summary *user_input, GUI_Scroll_Vars vars, i32_Rect region, i32 max_y){
    Input_Process_Result result = {0};
    b32 is_file_scroll = false;
    
    GUI_Session gui_session = {0};
    GUI_Header *h = 0;
    GUI_Target *target = &view->transient.gui_target;
    GUI_Interpret_Result interpret_result = {0};
    
    vars.target_y = clamp(0, vars.target_y, max_y);
    
    result.vars = vars;
    result.region = region;
    
    memset(&target->active, 0, sizeof(target->active));
    
    // HACK(allen): UI sucks!  Now just forcing it to 
    // not have the bug where it clicks buttons behind the 
    // header buttons before the scrollable section.
    b32 in_scroll = false;
    i32_Rect scroll_rect = {0};
    i32 prev_bottom = 0;
    
    if (target->push.pos > 0){
        gui_session_init(&gui_session, target, rect, view->transient.line_height);
        
        for (h = (GUI_Header*)target->push.base;
             h->type;
             h = NextHeader(h)){
            interpret_result = gui_interpret(target, &gui_session, h,
                                             result.vars, result.region, max_y);
            
            if (interpret_result.has_region){
                result.region = interpret_result.region;
            }
            
            switch (h->type){
                case guicom_file_option:
                case guicom_fixed_option:
                case guicom_fixed_option_checkbox:
                {
                    GUI_Interactive *b = (GUI_Interactive*)h;
                    
                    if (interpret_result.auto_activate){
                        memset(&target->auto_hot, 0, sizeof(target->auto_hot));
                        target->active = b->id;
                        result.is_animating = 1;
                    }
                    else if (interpret_result.auto_hot){
                        if (!gui_id_eq(target->auto_hot, b->id)){
                            target->auto_hot = b->id;
                            result.is_animating = 1;
                        }
                    }
                }break;
            }
            
            if (interpret_result.has_info){
                switch (h->type){
                    case guicom_top_bar: break;
                    
                    case guicom_file:
                    {
                        // NOTE(allen): Set the file region first because the
                        // computation of view_compute_max_target_y depends on it.
                        view->transient.file_region = gui_session.rect;
                        
                        Editing_File *file = view->transient.file_data.file;
                        Assert(file != 0);
                        
                        if (view->transient.reinit_scrolling){
                            view->transient.reinit_scrolling = false;
                            result.is_animating = true;
                            
                            i32 target_x = 0;
                            i32 target_y = 0;
                            if (file_is_ready(file)){
                                Vec2 cursor = view_get_cursor_xy(view);
                                
                                f32 w = view_width(view);
                                f32 h = view_height(view);
                                
                                if (cursor.x >= target_x + w){
                                    target_x = round32(cursor.x - w*.35f);
                                }
                                
                                target_y = clamp_bottom(0, floor32(cursor.y - h*.5f));
                            }
                            
                            result.vars.target_y = target_y;
                            result.vars.scroll_y = (f32)target_y;
                            result.vars.prev_target_y = -1000;
                            
                            result.vars.target_x = target_x;
                            result.vars.scroll_x = (f32)target_x;
                            result.vars.prev_target_x = -1000;
                        }
                        
                        if (!file->is_loading && file->state.paste_effect.seconds_down > 0.f){
                            file->state.paste_effect.seconds_down -= user_input->dt;
                            result.is_animating = true;
                        }
                        
                        is_file_scroll = true;
                    }break;
                    
                    case guicom_color_button:
                    case guicom_button:
                    case guicom_file_option:
                    case guicom_style_preview:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        
                        if (click_button_input(target, &gui_session, in_scroll, scroll_rect, user_input, b, &result.is_animating)){
                            result.consumed_l = true;
                        }
                        
                        prev_bottom = gui_session.rect.y1;
                    }break;
                    
                    case guicom_fixed_option:
                    case guicom_fixed_option_checkbox:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        
                        if (click_button_input(target, &gui_session, in_scroll, scroll_rect, user_input, b, &result.is_animating)){
                            result.consumed_l = true;
                        }
                        
                        Key_Input_Data *keys = &user_input->keys;
                        
                        void *ptr = (b + 1);
                        String string = gui_read_string(&ptr);
                        AllowLocal(string);
                        
                        char activation_key = *(char*)ptr;
                        activation_key = char_to_upper(activation_key);
                        
                        if (activation_key != 0){
                            i32 count = keys->count;
                            for (i32 i = 0; i < count; ++i){
                                Key_Event_Data key = keys->keys[i];
                                
                                u8 character[4];
                                u32 length = 0;
                                if (key.character != 0){
                                    u32_to_utf8_unchecked(key.character, character, &length);
                                }
                                if (length == 1){
                                    if (char_to_upper(character[0]) == activation_key){
                                        target->active = b->id;
                                        result.is_animating = 1;
                                        break;
                                    }
                                }
                            }
                        }
                    }break;
                    
                    case guicom_scrollable_slider:
                    {
                        GUI_id id = gui_id_scrollbar_slider();
                        i32 mx = user_input->mouse.x;
                        i32 my = user_input->mouse.y;
                        f32 v = 0;
                        
                        if (hit_check(mx, my, gui_session.rect)){
                            target->hover = id;
                            if (user_input->mouse.press_l){
                                target->mouse_hot = id;
                                result.is_animating = 1;
                                result.consumed_l = 1;
                            }
                        }
                        else if (gui_id_eq(target->hover, id)){
                            memset(&target->hover, 0, sizeof(target->hover));
                        }
                        
                        if (gui_id_eq(target->mouse_hot, id)){
                            v = unlerp(gui_session.scroll_top, (f32)my,
                                       gui_session.scroll_bottom);
                            v = clamp(0.f, v, 1.f);
                            result.vars.target_y = round32(lerp(0.f, v, (f32)max_y));
                            
                            gui_activate_scrolling(target);
                            result.is_animating = 1;
                        }
                    }
                    // NOTE(allen): NO BREAK HERE!!
                    
                    case guicom_scrollable_invisible:
                    {
                        if (user_input->mouse.wheel != 0){
                            result.vars.target_y += user_input->mouse.wheel;
                            
                            result.vars.target_y =
                                clamp(0, result.vars.target_y, max_y);
                            gui_activate_scrolling(target);
                            result.is_animating = 1;
                        }
                    }break;
                    
                    case guicom_scrollable_top:
                    {
                        GUI_id id = gui_id_scrollbar_top();
                        
                        if (scroll_button_input(target, &gui_session, user_input, id, &result.is_animating)){
                            result.vars.target_y -= clamp_bottom(1, target->delta >> 2);
                            result.vars.target_y = clamp_bottom(0, result.vars.target_y);
                            result.consumed_l = 1;
                        }
                    }break;
                    
                    case guicom_scrollable_bottom:
                    {
                        GUI_id id = gui_id_scrollbar_bottom();
                        
                        if (scroll_button_input(target, &gui_session, user_input, id, &result.is_animating)){
                            result.vars.target_y += clamp_bottom(1, target->delta >> 2);
                            result.vars.target_y = clamp_top(result.vars.target_y, max_y);
                            result.consumed_l = 1;
                        }
                    }break;
                    
                    case guicom_begin_scrollable_section:
                    {
                        in_scroll = true;
                        scroll_rect.x0 = region.x0;
                        scroll_rect.y0 = prev_bottom;
                        scroll_rect.x1 = region.x1;
                        scroll_rect.y1 = region.y1;
                    }break;
                    
                    case guicom_end_scrollable_section:
                    {
                        in_scroll = false;
                        if (!is_file_scroll){
                            result.has_max_y_suggestion = 1;
                            result.max_y = gui_session.suggested_max_y;
                        }
                    }break;
                }
            }
        }
        
        if (!user_input->mouse.l){
            if (!gui_id_is_null(target->mouse_hot)){
                memset(&target->mouse_hot, 0, sizeof(target->mouse_hot));
                result.is_animating = true;
            }
        }
        
        GUI_Scroll_Vars scroll_vars = result.vars;
        b32 is_new_target = (scroll_vars.target_x != scroll_vars.prev_target_x ||
                             scroll_vars.target_y != scroll_vars.prev_target_y);
        
        f32 target_x = (f32)scroll_vars.target_x;
        f32 target_y = (f32)scroll_vars.target_y;
        
        if (models->scroll_rule(target_x, target_y, &scroll_vars.scroll_x, &scroll_vars.scroll_y, (view->persistent.id) + 1, is_new_target, user_input->dt)){
            result.is_animating = true;
        }
        
        scroll_vars.prev_target_x = scroll_vars.target_x;
        scroll_vars.prev_target_y = scroll_vars.target_y;
        
        result.vars = scroll_vars;
    }
    
    return(result);
}

////////////////////////////////

internal void
draw_text_field(System_Functions *system, Render_Target *target, View *view, Models *models, Face_ID font_id, i32_Rect rect, String p, String t){
    Style *style = &models->styles.styles[0];
    
    u32 back_color = style->main.margin_color;
    u32 text1_color = style->main.default_color;
    u32 text2_color = style->main.file_info_style.pop1_color;
    
    i32 x = rect.x0;
    i32 y = rect.y0 + 2;
    
    if (target){
        draw_rectangle(target, rect, back_color);
        x = ceil32(draw_string(system, target, font_id, p, x, y, text2_color));
        draw_string(system, target, font_id, t, x, y, text1_color);
    }
}

internal void
draw_text_with_cursor(System_Functions *system, Render_Target *target, View *view, Models *models, Face_ID font_id, i32_Rect rect, String s, i32 pos){
    Style *style = &models->styles.styles[0];
    
    u32 back_color = style->main.margin_color;
    u32 text_color = style->main.default_color;
    u32 cursor_color = style->main.cursor_color;
    u32 at_cursor_color = style->main.at_cursor_color;
    
    f32 x = (f32)rect.x0;
    i32 y = rect.y0 + 2;
    
    if (target){
        draw_rectangle(target, rect, back_color);
        
        if (pos >= 0 && pos <  s.size){
            Font_Pointers font = system->font.get_pointers_by_id(font_id);
            
            String part1 = substr(s, 0, pos);
            String part2 = substr(s, pos, 1);
            String part3 = substr(s, pos+1, s.size-pos-1);
            
            x = draw_string(system, target, font_id, part1, floor32(x), y, text_color);
            
            f32 adv = font_get_glyph_advance(system, font.settings, font.metrics, font.pages, s.str[pos]);
            i32_Rect cursor_rect;
            cursor_rect.x0 = floor32(x);
            cursor_rect.x1 = floor32(x) + ceil32(adv);
            cursor_rect.y0 = y;
            cursor_rect.y1 = y + view->transient.line_height;
            draw_rectangle(target, cursor_rect, cursor_color);
            x = draw_string(system, target, font_id, part2, floor32(x), y, at_cursor_color);
            
            draw_string(system, target, font_id, part3, floor32(x), y, text_color);
        }
        else{
            draw_string(system, target, font_id, s, floor32(x), y, text_color);
        }
    }
}

internal void
intbar_draw_string(System_Functions *system, Render_Target *target, File_Bar *bar, String str, u32 char_color){
    draw_string(system, target, bar->font_id, str, (i32)(bar->pos_x + bar->text_shift_x), (i32)(bar->pos_y + bar->text_shift_y), char_color);
    bar->pos_x += font_string_width(system, target, bar->font_id, str);
}

internal void
draw_file_bar(System_Functions *system, Render_Target *target, View *view, Models *models, Editing_File *file, i32_Rect rect){
    File_Bar bar;
    Style *style = &models->styles.styles[0];
    Interactive_Style bar_style = style->main.file_info_style;
    
    u32 back_color = bar_style.bar_color;
    u32 base_color = bar_style.base_color;
    u32 pop1_color = bar_style.pop1_color;
    u32 pop2_color = bar_style.pop2_color;
    
    bar.rect = rect;
    
    if (target != 0){
        bar.font_id = file->settings.font_id;
        bar.pos_x = (f32)bar.rect.x0;
        bar.pos_y = (f32)bar.rect.y0;
        bar.text_shift_y = 2;
        bar.text_shift_x = 0;
        
        draw_rectangle(target, bar.rect, back_color);
        
        Assert(file != 0);
        
        intbar_draw_string(system, target, &bar, file->unique_name.name, base_color);
        intbar_draw_string(system, target, &bar, lit(" -"), base_color);
        
        if (file->is_loading){
            intbar_draw_string(system, target, &bar, lit(" loading"), base_color);
        }
        else{
            char bar_space[526];
            String bar_text = make_fixed_width_string(bar_space);
            append_ss        (&bar_text, lit(" L#"));
            append_int_to_str(&bar_text, view->transient.edit_pos->cursor.line);
            append_ss        (&bar_text, lit(" C#"));
            append_int_to_str(&bar_text, view->transient.edit_pos->cursor.character);
            
            append_ss(&bar_text, lit(" -"));
            
            if (file->settings.dos_write_mode){
                append_ss(&bar_text, lit(" dos"));
            }
            else{
                append_ss(&bar_text, lit(" nix"));
            }
            
            intbar_draw_string(system, target, &bar, bar_text, base_color);
            
            
            if (file->state.still_lexing){
                intbar_draw_string(system, target, &bar, lit(" parsing"), pop1_color);
            }
            
            switch (file->state.dirty){
                case DirtyState_UnloadedChanges:
                {
                    intbar_draw_string(system, target, &bar, lit(" !"), pop2_color);
                }break;
                
                case DirtyState_UnsavedChanges:
                {
                    intbar_draw_string(system, target, &bar, lit(" *"), pop2_color);
                }break;
            }
        }
    }
}

internal void
draw_color_button(System_Functions *system, GUI_Target *gui_target, Render_Target *target, View *view, Face_ID font_id, i32_Rect rect, GUI_id id, u32 fore, u32 back, String text){
    i32 active_level = gui_active_level(gui_target, id);
    
    if (active_level > 0){
        Swap(u32, back, fore);
    }
    
    draw_rectangle(target, rect, back);
    draw_string(system, target, font_id, text, rect.x0, rect.y0 + 1, fore);
}

internal void
draw_fat_option_block(System_Functions *system, GUI_Target *gui_target, Render_Target *target, View *view, Models *models, Face_ID font_id, i32_Rect rect, GUI_id id, String text, String pop, i8 checkbox = -1){
    Style *style = &models->styles.styles[0];
    
    i32 active_level = gui_active_level(gui_target, id);
    
    i32_Rect inner = get_inner_rect(rect, 3);
    
    u32 margin = style_get_margin_color(active_level, style);
    u32 back = style->main.back_color;
    u32 text_color = style->main.default_color;
    u32 pop_color = style->main.file_info_style.pop2_color;
    
    i32 h = view->transient.line_height;
    i32 x = inner.x0 + 3;
    i32 y = inner.y0 + h/2 - 1;
    
    draw_rectangle(target, inner, back);
    draw_margin(target, rect, inner, margin);
    
    if (checkbox != -1){
        u32 checkbox_color = style->main.margin_active_color;
        i32_Rect checkbox_rect = get_inner_rect(inner, (inner.y1 - inner.y0 - h)/2);
        checkbox_rect.x1 = checkbox_rect.x0 + (checkbox_rect.y1 - checkbox_rect.y0);
        
        if (checkbox == 0){
            draw_rectangle_outline(target, checkbox_rect, checkbox_color);
        }
        else{
            draw_rectangle(target, checkbox_rect, checkbox_color);
        }
        
        x = checkbox_rect.x1 + 3;
    }
    
    x = ceil32(draw_string(system, target, font_id, text, x, y, text_color));
    draw_string(system, target, font_id, pop, x, y, pop_color);
}

internal void
draw_button(System_Functions *system, GUI_Target *gui_target, Render_Target *target, View *view, Models *models, Face_ID font_id, i32_Rect rect, GUI_id id, String text){
    Style *style = &models->styles.styles[0];
    
    i32 active_level = gui_active_level(gui_target, id);
    
    i32_Rect inner = get_inner_rect(rect, 3);
    
    u32 margin = style->main.default_color;
    u32 back = style_get_margin_color(active_level, style);
    u32 text_color = style->main.default_color;
    
    i32 h = view->transient.line_height;
    i32 y = inner.y0 + h/2 - 1;
    
    i32 w = (i32)font_string_width(system, target, font_id, text);
    i32 x = (inner.x1 + inner.x0 - w)/2;
    
    draw_rectangle(target, inner, back);
    draw_rectangle_outline(target, inner, margin);
    
    draw_string(system, target, font_id, text, x, y, text_color);
}

internal void
draw_style_preview(System_Functions *system, GUI_Target *gui_target, Render_Target *target, View *view, Models *models, Face_ID font_id, i32_Rect rect, GUI_id id, Style *style){
    i32 active_level = gui_active_level(gui_target, id);
    char font_name_space[256];
    String font_name = make_fixed_width_string(font_name_space);
    font_name.size = system->font.get_name_by_id(font_id, font_name.str, font_name.memory_size);
    Font_Pointers font = system->font.get_pointers_by_id(font_id);
    
    i32_Rect inner = get_inner_rect(rect, 3);
    
    u32 margin_color = style_get_margin_color(active_level, style);
    u32 back = style->main.back_color;
    u32 text_color = style->main.default_color;
    u32 keyword_color = style->main.keyword_color;
    u32 int_constant_color = style->main.int_constant_color;
    u32 comment_color = style->main.comment_color;
    
    draw_margin(target, rect, inner, margin_color);
    draw_rectangle(target, inner, back);
    
    i32 y = inner.y0;
    i32 x = inner.x0;
    x = ceil32(draw_string(system, target, font_id, style->name.str, x, y, text_color));
    i32 font_x = (i32)(inner.x1 - font_string_width(system, target, font_id, font_name));
    if (font_x > x + 10){
        draw_string(system, target, font_id, font_name, font_x, y, text_color);
    }
    
    i32 height = font.metrics->height;
    x = inner.x0;
    y += height;
    x = ceil32(draw_string(system, target, font_id, "if", x, y, keyword_color));
    x = ceil32(draw_string(system, target, font_id, "(x < ", x, y, text_color));
    x = ceil32(draw_string(system, target, font_id, "0", x, y, int_constant_color));
    x = ceil32(draw_string(system, target, font_id, ") { x = ", x, y, text_color));
    x = ceil32(draw_string(system, target, font_id, "0", x, y, int_constant_color));
    x = ceil32(draw_string(system, target, font_id, "; } ", x, y, text_color));
    x = ceil32(draw_string(system, target, font_id, "// comment", x, y, comment_color));
    
    x = inner.x0;
    y += height;
    draw_string(system, target, font_id, "[] () {}; * -> +-/ <>= ! && || % ^", x, y, text_color);
}

internal i32
do_render_file_view(System_Functions *system, View *view, Models *models, GUI_Scroll_Vars *scroll, View *active, i32_Rect rect, b32 is_active, Render_Target *target, Input_Summary *user_input){
    
    Editing_File *file = view->transient.file_data.file;
    Assert(file != 0);
    
    i32 result = 0;
    
    GUI_Session gui_session = {0};
    GUI_Header *h = 0;
    GUI_Target *gui_target = &view->transient.gui_target;
    GUI_Interpret_Result interpret_result = {0};
    
    f32 v = {0};
    i32 max_y = view_compute_max_target_y(view);
    
    Face_ID font_id = file->settings.font_id;
    if (gui_target->push.pos > 0){
        gui_session_init(&gui_session, gui_target, rect, view->transient.line_height);
        
        v = view_get_scroll_y(view);
        
        i32_Rect clip_rect = rect;
        draw_push_clip(target, clip_rect);
        
        for (h = (GUI_Header*)gui_target->push.base;
             h->type;
             h = NextHeader(h)){
            interpret_result = gui_interpret(gui_target, &gui_session, h,
                                             *scroll, view->transient.scroll_region, max_y);
            
            if (interpret_result.has_info){
                if (gui_session.clip_y > clip_rect.y0){
                    clip_rect.y0 = gui_session.clip_y;
                    draw_change_clip(target, clip_rect);
                }
                
                switch (h->type){
                    case guicom_top_bar:
                    {
                        draw_file_bar(system, target, view, models, file, gui_session.rect);
                    }break;
                    
                    case guicom_file:
                    {
                        if (file_is_ready(file)){
                            result = render_loaded_file_in_view(system, view, models, gui_session.rect, is_active, target);
                        }
                    }break;
                    
                    case guicom_text_field:
                    {
                        void *ptr = (h+1);
                        String p = gui_read_string(&ptr);
                        String t = gui_read_string(&ptr);
                        draw_text_field(system, target, view, models, font_id, gui_session.rect, p, t);
                    }break;
                    
                    case guicom_text_with_cursor:
                    {
                        void *ptr = (h+1);
                        String s = gui_read_string(&ptr);
                        i32 pos = gui_read_integer(&ptr);
                        
                        draw_text_with_cursor(system, target, view, models, font_id, gui_session.rect, s, pos);
                    }break;
                    
                    case guicom_color_button:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        u32 fore = (u32)gui_read_integer(&ptr);
                        u32 back = (u32)gui_read_integer(&ptr);
                        String t = gui_read_string(&ptr);
                        
                        draw_color_button(system, gui_target, target, view, font_id, gui_session.rect, b->id, fore, back, t);
                    }break;
                    
                    case guicom_file_option:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        b32 folder = gui_read_integer(&ptr);
                        String f = gui_read_string(&ptr);
                        String m = gui_read_string(&ptr);
                        
                        if (folder){
                            append_s_char(&f, '/');
                        }
                        
                        draw_fat_option_block(system, gui_target, target, view, models, font_id, gui_session.rect, b->id, f, m);
                    }break;
                    
                    case guicom_style_preview:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        i32 style_index = *(i32*)(b + 1);
                        Style *style = &models->styles.styles[style_index];
                        
                        draw_style_preview(system, gui_target, target, view, models, font_id, gui_session.rect, b->id, style);
                    }break;
                    
                    case guicom_fixed_option:
                    case guicom_fixed_option_checkbox:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        String f = gui_read_string(&ptr);
                        String m = {0};
                        i8 status = -1;
                        if (h->type == guicom_fixed_option_checkbox){
                            gui_read_byte(&ptr);
                            status = (i8)gui_read_byte(&ptr);
                        }
                        
                        draw_fat_option_block(system, gui_target, target, view, models, font_id, gui_session.rect, b->id, f, m, status);
                    }break;
                    
                    case guicom_button:
                    {
                        GUI_Interactive *b = (GUI_Interactive*)h;
                        void *ptr = (b + 1);
                        String t = gui_read_string(&ptr);
                        
                        draw_button(system, gui_target, target, view, models, font_id, gui_session.rect, b->id, t);
                    }break;
                    
                    case guicom_scrollable_bar:
                    {
                        Style *style = &models->styles.styles[0];
                        
                        u32 back;
                        u32 outline;
                        
                        i32_Rect bar = gui_session.rect;
                        
                        back = style->main.back_color;
                        if (is_active){
                            outline = style->main.margin_active_color;
                        }
                        else{
                            outline = style->main.margin_color;
                        }
                        
                        draw_rectangle(target, bar, back);
                        draw_rectangle_outline(target, bar, outline);
                    }break;
                    
                    case guicom_scrollable_top:
                    case guicom_scrollable_slider:
                    case guicom_scrollable_bottom:
                    {
                        GUI_id id;
                        Style *style = &models->styles.styles[0];
                        i32_Rect box = gui_session.rect;
                        
                        i32 active_level;
                        
                        u32 back;
                        u32 outline;
                        
                        switch (h->type){
                            case guicom_scrollable_top: id = gui_id_scrollbar_top(); break;
                            case guicom_scrollable_bottom: id = gui_id_scrollbar_bottom(); break;
                            default: id = gui_id_scrollbar_slider(); break;
                        }
                        
                        active_level = gui_active_level(gui_target, id);
                        
                        switch (active_level){
                            case 0: back = style->main.back_color; break;
                            case 1: back = style->main.margin_hover_color; break;
                            default: back = style->main.margin_active_color; break;
                        }
                        
                        if (is_active){
                            outline = style->main.margin_active_color;
                        }
                        else{
                            outline = style->main.margin_color;
                        }
                        
                        draw_rectangle(target, box, back);
                        draw_margin(target, box, get_inner_rect(box, 2), outline);
                    }break;
                    
                    case guicom_begin_scrollable_section:
                    clip_rect.x1 = Min(gui_session.scroll_region.x1, clip_rect.x1);
                    draw_push_clip(target, clip_rect);
                    break;
                    
                    case guicom_end_scrollable_section:
                    clip_rect = draw_pop_clip(target);
                    break;
                }
            }
        }
        
        draw_pop_clip(target);
    }
    
    return(result);
}

// BOTTOM

