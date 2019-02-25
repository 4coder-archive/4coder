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
do_step_file_view(System_Functions *system, Models *models, View *view, i32_Rect rect, b32 is_active, f32 dt, GUI_Scroll_Vars scroll, i32 max_y){
    Input_Process_Result result = {};
    scroll.target_y = clamp(0, scroll.target_y, max_y);
    result.scroll = scroll;
    
    i32 line_height = view->line_height;
    
    if (!view->hide_file_bar){
        i32_Rect top_bar_rect = {};
        top_bar_rect.x0 = rect.x0;
        top_bar_rect.y0 = rect.y0;
        top_bar_rect.x1 = rect.x1;
        top_bar_rect.y1 = rect.y0 + line_height + 2;
        rect.y0 = top_bar_rect.y1;
    }
    view->file_region = rect;
    
    i32 bar_count = 0;
    for (Query_Slot *slot = view->query_set.used_slot;
         slot != 0;
         slot = slot->next, ++bar_count);
    view->widget_height = (f32)bar_count*(view->line_height + 2);
    
    Editing_File *file = view->file_data.file;
    
    // TODO(allen): do(eliminate the built in paste_effect)
    if (!file->is_loading && file->state.paste_effect.seconds_down > 0.f){
        file->state.paste_effect.seconds_down -= dt;
        result.is_animating = true;
    }
    
    // NOTE(allen): call scroll rule hook
    b32 is_new_target = (result.scroll.target_x != view->prev_target.x ||
                         result.scroll.target_y != view->prev_target.y);
    
    f32 target_x = (f32)result.scroll.target_x;
    f32 target_y = (f32)result.scroll.target_y;
    
    View_ID view_id = view_get_id(&models->live_set, view);
    if (models->scroll_rule(target_x, target_y, &result.scroll.scroll_x, &result.scroll.scroll_y, view_id, is_new_target, dt)){
        result.is_animating = true;
    }
    
    view->prev_target.x = result.scroll.target_x;
    view->prev_target.y = result.scroll.target_y;
    
    return(result);
}

////////////////////////////////

internal void
intbar_draw_string(System_Functions *system, Render_Target *target, File_Bar *bar, String str, u32 char_color){
    Vec2 p = bar->pos + bar->text_shift;
    bar->pos.x += draw_string(system, target, bar->font_id, str, p, char_color);
}

internal void
draw_file_bar(System_Functions *system, Render_Target *target, View *view, Models *models, Editing_File *file, i32_Rect rect){
    File_Bar bar = {};
    Color_Table color_table = models->color_table;
    
    u32 back_color = color_table.vals[Stag_Bar];
    u32 base_color = color_table.vals[Stag_Base];
    u32 pop1_color = color_table.vals[Stag_Pop1];
    u32 pop2_color = color_table.vals[Stag_Pop2];
    
    bar.rect = rect;
    
    if (target != 0){
        bar.font_id = file->settings.font_id;
        bar.pos = V2(bar.rect.p0);
        bar.text_shift = V2(0.f, 2.f);
        
        draw_rectangle(target, bar.rect, back_color);
        
        Assert(file != 0);
        
        intbar_draw_string(system, target, &bar, file->unique_name.name, base_color);
        intbar_draw_string(system, target, &bar, lit(" -"), base_color);
        
        if (file->is_loading){
            intbar_draw_string(system, target, &bar, lit(" loading"), base_color);
        }
        else{
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            Full_Cursor cursor = file_compute_cursor(system, view->file_data.file, seek_pos(edit_pos.cursor_pos));
            
            char bar_space[526];
            String bar_text = make_fixed_width_string(bar_space);
            append_ss        (&bar_text, lit(" L#"));
            append_int_to_str(&bar_text, cursor.line);
            append_ss        (&bar_text, lit(" C#"));
            append_int_to_str(&bar_text, cursor.character);
            
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

internal u32
get_margin_color(Color_Table color_table, i32 level){
    u32 margin = 0;
    switch (level){
        default:
        case UIActivation_None:
        {
            margin = color_table.vals[Stag_List_Item];
        }break;
        case UIActivation_Hover:
        {
            margin = color_table.vals[Stag_List_Item_Hover];
        }break;
        case UIActivation_Active:
        {
            margin = color_table.vals[Stag_List_Item_Active];
        }break;
    }
    return(margin);
}

internal void
do_render_file_view(System_Functions *system, View *view, Models *models, GUI_Scroll_Vars *scroll, View *active, i32_Rect rect, b32 is_active, Render_Target *target){
    
    Editing_File *file = view->file_data.file;
    Assert(file != 0);
    
    i32 line_height = view->line_height;
    Color_Table color_table = models->color_table;
    Face_ID font_id = file->settings.font_id;
    char font_name_space[256];
    String font_name = make_fixed_width_string(font_name_space);
    font_name.size = system->font.get_name_by_id(font_id, font_name.str, font_name.memory_size);
    
    if (!view->hide_file_bar){
        i32_Rect top_bar_rect = {};
        top_bar_rect.x0 = rect.x0;
        top_bar_rect.y0 = rect.y0;
        top_bar_rect.x1 = rect.x1;
        top_bar_rect.y1 = rect.y0 + line_height + 2;
        rect.y0 = top_bar_rect.y1;
        draw_file_bar(system, target, view, models, file, top_bar_rect);
    }
    
    i32 bar_count = 0;
    for (Query_Slot *slot = view->query_set.used_slot;
         slot != 0;
         slot = slot->next, ++bar_count){
        i32_Rect query_bar_rect = {};
        query_bar_rect.x0 = rect.x0;
        query_bar_rect.y0 = rect.y0;
        query_bar_rect.x1 = rect.x1;
        query_bar_rect.y1 = rect.y0 + line_height + 2;
        rect.y0 = query_bar_rect.y1;
        u32 back_color  = color_table.vals[Stag_Back];
        u32 text1_color = color_table.vals[Stag_Default];
        u32 text2_color = color_table.vals[Stag_Pop1];
        Vec2 p = V2(query_bar_rect.p0);
        p.y += 2.f;
        draw_rectangle(target, query_bar_rect, back_color);
        p.x += draw_string(system, target, font_id, slot->query_bar->prompt, p, text2_color);
        draw_string(system, target, font_id, slot->query_bar->string, p, text1_color);
    }
    view->widget_height = (f32)bar_count*(view->line_height + 2);
    
    draw_push_clip(target, rect);
    if (!view->ui_mode){
        if (file_is_ready(file)){
            render_loaded_file_in_view(system, view, models, rect, is_active, target);
        }
    }
    else{
        f32_Rect rect_f32 = f32R(rect);
        
        i32 item_count = view->ui_control.count;
        UI_Item *item = view->ui_control.items;
        GUI_Scroll_Vars ui_scroll = view->ui_scroll;
        for (i32 i = 0; i < item_count; ++i, item += 1){
            
            f32_Rect item_rect = f32R(item->rectangle);
            switch (item->coordinates){
                case UICoordinates_ViewSpace:
                {
                    item_rect.x0 += rect_f32.x0 - ui_scroll.scroll_x;
                    item_rect.y0 += rect_f32.y0 - ui_scroll.scroll_y;
                    item_rect.x1 += rect_f32.x0 - ui_scroll.scroll_x;
                    item_rect.y1 += rect_f32.y0 - ui_scroll.scroll_y;
                }break;
                case UICoordinates_PanelSpace:
                {
                    item_rect.x0 += rect_f32.x0;
                    item_rect.y0 += rect_f32.y0;
                    item_rect.x1 += rect_f32.x0;
                    item_rect.y1 += rect_f32.y0;
                }break;
            }
            
            if (rect_overlap(item_rect, rect_f32)){
                switch (item->type){
                    case UIType_Option:
                    {
                        u32 back       = color_table.vals[Stag_Back];
                        u32 text_color = color_table.vals[Stag_Default];
                        u32 pop_color  = color_table.vals[Stag_Pop2];
                        u32 margin_color = get_margin_color(color_table, item->activation_level);
                        f32_Rect inner = get_inner_rect(item_rect, 3);
                        draw_rectangle(target, inner, back);
                        Vec2 p = V2(inner.p0) + V2(3.f, line_height*0.5f - 1.f);
                        p.x += draw_string(system, target, font_id, item->option.string, p, text_color);
                        p.x += font_string_width(system, target, font_id, make_lit_string(" "));
                        draw_string(system, target, font_id, item->option.status, p, pop_color);
                        draw_margin(target, item_rect, inner, margin_color);
                    }break;
                    
                    case UIType_TextField:
                    {
                        u32 back  = color_table.vals[Stag_Back];
                        u32 text1 = color_table.vals[Stag_Default];
                        u32 text2 = color_table.vals[Stag_Pop1];
                        draw_rectangle(target, item_rect, back);
                        Vec2 p = V2(item_rect.p0) + V2(0.f, 2.f);
                        p.x += draw_string(system, target, font_id, item->text_field.query, p, text2);
                        p.x += font_string_width(system, target, font_id, make_lit_string(" "));
                        p.x += draw_string(system, target, font_id, item->text_field.string, p, text1);
                    }break;
                    
                    // TODO(allen): figure out how this should work again later
                    case UIType_ColorTheme:
                    {}break;
                    
#if 0
                    case UIType_ColorTheme:
                    {
                        Style *style_preview = &models->styles.styles[item->color_theme.index];
                        u32 margin_color = get_margin_color(style_preview, item->activation_level);
                        u32 back               = style_preview->theme.colors[Stag_Back];
                        u32 text_color         = style_preview->theme.colors[Stag_Default];
                        u32 keyword_color      = style_preview->theme.colors[Stag_Keyword];
                        u32 int_constant_color = style_preview->theme.colors[Stag_Int_Constant];
                        u32 comment_color      = style_preview->theme.colors[Stag_Comment];
                        
                        f32_Rect inner = get_inner_rect(item_rect, 3);
                        
                        draw_margin(target, item_rect, inner, margin_color);
                        draw_rectangle(target, inner, back);
                        
                        Vec2 p = V2(inner.p0);
                        String str = item->color_theme.string;
                        if (str.str == 0){
                            str = style_preview->name;
                        }
                        p.x += draw_string(system, target, font_id, str, p, text_color);
                        f32 font_x = inner.x1 - font_string_width(system, target, font_id, font_name);
                        if (font_x > p.x + 10.f){
                            draw_string(system, target, font_id, font_name, V2(font_x, p.y), text_color);
                        }
                        
                        Font_Pointers font = system->font.get_pointers_by_id(font_id);
                        i32 height = font.metrics->height;
                        p = V2(inner.x0, p.y + (f32)height);
                        p.x += draw_string(system, target, font_id, "if", p, keyword_color);
                        p.x += draw_string(system, target, font_id, "(x < ", p, text_color);
                        p.x += draw_string(system, target, font_id, "0", p, int_constant_color);
                        p.x += draw_string(system, target, font_id, ") { x = ", p, text_color);
                        p.x += draw_string(system, target, font_id, "0", p, int_constant_color);
                        p.x += draw_string(system, target, font_id, "; } ", p, text_color);
                        p.x += draw_string(system, target, font_id, "// comment", p, comment_color);
                        
                        p = V2(inner.x0, p.y + (f32)height);
                        draw_string(system, target, font_id, "[] () {}; * -> +-/ <>= ! && || % ^", p, text_color);
                    }break;
#endif
                }
            }
        }
    }
    draw_pop_clip(target);
}

// BOTTOM

