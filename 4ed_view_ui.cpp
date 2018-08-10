/*
 * Mr. 4th Dimention - Allen Webster
 *
 * ??.??.2018
 *
 * View GUI layouts and implementations
 *
 */

// TOP

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
    
    i32 bar_count = 0;
    for (Query_Slot *slot = view->transient.query_set.used_slot;
         slot != 0;
         slot = slot->next, ++bar_count);
    view->transient.widget_height = (f32)bar_count*(view->transient.line_height + 2);
    
    if (view->transient.ui_mode_counter == 0){
        if (user_input->mouse.wheel != 0){
            result.scroll.target_y += user_input->mouse.wheel;
            result.scroll.target_y = clamp(0, result.scroll.target_y, max_y);
            result.is_animating = true;
        }
        
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
    char font_name_space[256];
    String font_name = make_fixed_width_string(font_name_space);
    font_name.size = system->font.get_name_by_id(font_id, font_name.str, font_name.memory_size);
    Font_Pointers font = system->font.get_pointers_by_id(font_id);
    
    if (!view->transient.hide_file_bar){
        i32_Rect top_bar_rect = {0};
        top_bar_rect.x0 = rect.x0;
        top_bar_rect.y0 = rect.y0;
        top_bar_rect.x1 = rect.x1;
        top_bar_rect.y1 = rect.y0 + line_height + 2;
        rect.y0 = top_bar_rect.y1;
        draw_file_bar(system, target, view, models, file, top_bar_rect);
    }
    
    i32 bar_count = 0;
    for (Query_Slot *slot = view->transient.query_set.used_slot;
         slot != 0;
         slot = slot->next, ++bar_count){
        i32_Rect query_bar_rect = {0};
        query_bar_rect.x0 = rect.x0;
        query_bar_rect.y0 = rect.y0;
        query_bar_rect.x1 = rect.x1;
        query_bar_rect.y1 = rect.y0 + line_height + 2;
        rect.y0 = query_bar_rect.y1;
        u32 back_color = style->main.margin_color;
        u32 text1_color = style->main.default_color;
        u32 text2_color = style->main.file_info_style.pop1_color;
        i32 x = query_bar_rect.x0;
        i32 y = query_bar_rect.y0 + 2;
        draw_rectangle(target, query_bar_rect, back_color);
        x = ceil32(draw_string(system, target, font_id, slot->query_bar->prompt, x, y, text2_color));
        draw_string(system, target, font_id, slot->query_bar->string, x, y, text1_color);
    }
    view->transient.widget_height = (f32)bar_count*(view->transient.line_height + 2);
    
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
            switch (item->coordinates){
                case UICoordinates_Scrolled:
                {
                    item_rect.x0 += rect_f32.x0 - scroll.scroll_x;
                    item_rect.y0 += rect_f32.y0 - scroll.scroll_y;
                    item_rect.x1 += rect_f32.x0 - scroll.scroll_x;
                    item_rect.y1 += rect_f32.y0 - scroll.scroll_y;
                }break;
                
                case UICoordinates_ViewRelative:
                {
                    item_rect.x0 += rect_f32.x0;
                    item_rect.y0 += rect_f32.y0;
                    item_rect.x1 += rect_f32.x0;
                    item_rect.y1 += rect_f32.y0;
                }break;
            }
            
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
                        x = ceil32(draw_string(system, target, font_id, item->option.string, x, y, text_color));
                        draw_string(system, target, font_id, item->option.status, x, y, pop_color);
                    }break;
                    
                    case UIType_TextField:
                    {
                        u32 back_color = style->main.margin_color;
                        u32 text1_color = style->main.default_color;
                        u32 text2_color = style->main.file_info_style.pop1_color;
                        draw_rectangle(target, item_rect, back_color);
                        i32 x = (i32)item_rect.x0;
                        i32 y = (i32)item_rect.y0 + 2;
                        x = ceil32(draw_string(system, target, font_id, item->text_field.query, x, y, text2_color));
                        draw_string(system, target, font_id, item->text_field.string, x, y, text1_color);
                    }break;
                    
                    case UIType_ColorTheme:
                    {
                        Style *style_preview = &models->styles.styles[item->color_theme.index];
                        u32 margin_color = style_get_margin_color(item->activation_level, style_preview);
                        u32 back = style_preview->main.back_color;
                        u32 text_color = style_preview->main.default_color;
                        u32 keyword_color = style_preview->main.keyword_color;
                        u32 int_constant_color = style_preview->main.int_constant_color;
                        u32 comment_color = style_preview->main.comment_color;
                        
                        f32_Rect inner = get_inner_rect(item_rect, 3);
                        
                        draw_margin(target, item_rect, inner, margin_color);
                        draw_rectangle(target, inner, back);
                        
                        i32 y = (i32)inner.y0;
                        i32 x = (i32)inner.x0;
                        String str = item->color_theme.string;
                        if (str.str == 0){
                            str = style_preview->name;
                        }
                        x = ceil32(draw_string(system, target, font_id, str, x, y, text_color));
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
                    }break;
                }
            }
        }
    }
    draw_pop_clip(target);
    
    return(result);
}

// BOTTOM

