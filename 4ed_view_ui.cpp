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

internal Input_Process_Result
do_step_file_view(System_Functions *system, View *view, Models *models, i32_Rect rect, b32 is_active, Input_Summary *user_input, GUI_Scroll_Vars scroll, i32 max_y){
    scroll.target_y = clamp(0, scroll.target_y, max_y);
    
    Input_Process_Result result = {0};
    result.scroll = scroll;
    
    i32 line_height = view->transient.line_height;
    
    if (!view->transient.hide_file_bar){
        i32_Rect top_bar_rect = {0};
        top_bar_rect.x0 = rect.x0;
        top_bar_rect.y0 = rect.y0;
        top_bar_rect.x1 = rect.x1;
        top_bar_rect.y1 = rect.y0 + line_height + 2;
        rect.y0 = top_bar_rect.y1;
    }
    
    if (user_input->mouse.wheel != 0){
        result.scroll.target_y += user_input->mouse.wheel;
        result.scroll.target_y = clamp(0, result.scroll.target_y, max_y);
        result.is_animating = true;
    }
    
    if (view->transient.ui_mode_counter == 0){
        view->transient.file_region = rect;
        
        Editing_File *file = view->transient.file_data.file;
        if (view->transient.reinit_scrolling){
            view->transient.reinit_scrolling = false;
            result.is_animating = true;
            
            i32 target_x = 0;
            i32 target_y = 0;
            if (file_is_ready(file)){
                Vec2 cursor = view_get_cursor_xy(view);
                
                f32 width = view_width(view);
                f32 height = view_height(view);
                
                if (cursor.x >= target_x + width){
                    target_x = round32(cursor.x - width*.35f);
                }
                
                target_y = clamp_bottom(0, floor32(cursor.y - height*.5f));
            }
            
            result.scroll.target_y = target_y;
            result.scroll.scroll_y = (f32)target_y;
            result.scroll.prev_target_y = -1000;
            
            result.scroll.target_x = target_x;
            result.scroll.scroll_x = (f32)target_x;
            result.scroll.prev_target_x = -1000;
        }
        
        if (!file->is_loading && file->state.paste_effect.seconds_down > 0.f){
            file->state.paste_effect.seconds_down -= user_input->dt;
            result.is_animating = true;
        }
    }
    
    {    
        GUI_Scroll_Vars scroll_vars = result.scroll;
        b32 is_new_target = (scroll_vars.target_x != scroll_vars.prev_target_x ||
                             scroll_vars.target_y != scroll_vars.prev_target_y);
        
        f32 target_x = (f32)scroll_vars.target_x;
        f32 target_y = (f32)scroll_vars.target_y;
        
        if (models->scroll_rule(target_x, target_y, &scroll_vars.scroll_x, &scroll_vars.scroll_y, (view->persistent.id) + 1, is_new_target, user_input->dt)){
            result.is_animating = true;
        }
        
        scroll_vars.prev_target_x = scroll_vars.target_x;
        scroll_vars.prev_target_y = scroll_vars.target_y;
        
        result.scroll = scroll_vars;
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

internal i32
do_render_file_view(System_Functions *system, View *view, Models *models, GUI_Scroll_Vars *scroll, View *active, i32_Rect rect, b32 is_active, Render_Target *target, Input_Summary *user_input){
    
    Editing_File *file = view->transient.file_data.file;
    Assert(file != 0);
    
    i32 result = 0;
    
    i32 line_height = view->transient.line_height;
    Style *style = &models->styles.styles[0];
    Face_ID font_id = file->settings.font_id;
    
    if (!view->transient.hide_file_bar){
        i32_Rect top_bar_rect = {0};
        top_bar_rect.x0 = rect.x0;
        top_bar_rect.y0 = rect.y0;
        top_bar_rect.x1 = rect.x1;
        top_bar_rect.y1 = rect.y0 + line_height + 2;
        rect.y0 = top_bar_rect.y1;
        draw_file_bar(system, target, view, models, file, top_bar_rect);
    }
    
    draw_push_clip(target, rect);
    if (view->transient.ui_mode_counter == 0){
        if (file_is_ready(file)){
            result = render_loaded_file_in_view(system, view, models, rect, is_active, target);
        }
    }
    else{
        f32_Rect rect_f32 = f32R(rect);
        
        i32 item_count = view->transient.ui_control.count;
        UI_Item *item = view->transient.ui_control.items;
        GUI_Scroll_Vars scroll = view->transient.ui_scroll;
        for (i32 i = 0; i < item_count; ++i, item += 1){
            f32_Rect item_rect = f32R(item->rectangle);
            item_rect.x0 += rect_f32.x0 - scroll.scroll_x;
            item_rect.y0 += rect_f32.y0 - scroll.scroll_y;
            item_rect.x1 += rect_f32.x0 - scroll.scroll_x;
            item_rect.y1 += rect_f32.y0 - scroll.scroll_y;
            
            if (rect_opverlap(item_rect, rect_f32)){
                switch (item->type){
                    case UIType_Option:
                    {
                        u32 back = style->main.back_color;
                        u32 margin_color = style_get_margin_color(item->activation_level, style);
                        u32 text_color = style->main.default_color;
                        u32 pop_color = style->main.file_info_style.pop2_color;
                        f32_Rect inner = get_inner_rect(item_rect, 3);
                        draw_rectangle(target, inner, back);
                        draw_margin(target, item_rect, inner, margin_color);
                        i32 x = (i32)inner.x0 + 3;
                        i32 y = (i32)inner.y0 + line_height/2 - 1;
                        x = ceil32(draw_string(system, target, font_id, item->string, x, y, text_color));
                        draw_string(system, target, font_id, item->status, x, y, pop_color);
                    }break;
                    
                    case UIType_TextField:
                    {
                        u32 back_color = style->main.margin_color;
                        u32 text1_color = style->main.default_color;
                        u32 text2_color = style->main.file_info_style.pop1_color;
                        draw_rectangle(target, item_rect, back_color);
                        i32 x = (i32)item_rect.x0;
                        i32 y = (i32)item_rect.y0 + 2;
                        x = ceil32(draw_string(system, target, font_id, item->query, x, y, text2_color));
                        draw_string(system, target, font_id, item->string, x, y, text1_color);
                    }break;
                }
            }
        }
    }
    draw_pop_clip(target);
    
    return(result);
}

// BOTTOM

