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

internal GUI_Scroll_Vars
do_step_file_view(System_Functions *system, Models *models, View *view, i32_Rect rect, b32 is_active, f32 dt, GUI_Scroll_Vars scroll){
#if 0
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
#endif
    
    Editing_File *file = view->file;
    
    // TODO(allen): do(eliminate the built in paste_effect)
    if (!file->is_loading && file->state.paste_effect.seconds_down > 0.f){
        file->state.paste_effect.seconds_down -= dt;
        models->animate_next_frame = true;
    }
    
    // NOTE(allen): call scroll rule hook
    b32 is_new_target = (scroll.target_x != view->prev_target.x || scroll.target_y != view->prev_target.y);
    
    f32 target_x = (f32)scroll.target_x;
    f32 target_y = (f32)scroll.target_y;
    
    View_ID view_id = view_get_id(&models->live_set, view);
    if (models->scroll_rule(target_x, target_y, &scroll.scroll_x, &scroll.scroll_y, view_id, is_new_target, dt)){
        models->animate_next_frame = true;
    }
    
    view->prev_target.x = scroll.target_x;
    view->prev_target.y = scroll.target_y;
    
    return(scroll);
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
            Full_Cursor cursor = file_compute_cursor(system, view->file, seek_pos(edit_pos.cursor_pos));
            
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
                case DirtyState_UnsavedChanges:
                {
                    intbar_draw_string(system, target, &bar, lit(" *"), pop2_color);
                }break;
                
                case DirtyState_UnloadedChanges:
                {
                    intbar_draw_string(system, target, &bar, lit(" !"), pop2_color);
                }break;
                
                case DirtyState_UnsavedChangesAndUnloadedChanges:
                {
                    intbar_draw_string(system, target, &bar, lit(" *!"), pop2_color);
                }break;
            }
        }
    }
}

internal void
do_render_file_view(System_Functions *system, View *view, Models *models, GUI_Scroll_Vars *scroll, View *active, i32_Rect rect, b32 is_active, Render_Target *target){
    Editing_File *file = view->file;
    Assert(file != 0);
    
    draw_push_clip(target, rect);
    
    if (!view->ui_mode){
        if (file_is_ready(file)){
            Rect_i32 file_region = view_get_file_region(models, view);
            render_loaded_file_in_view(system, view, models, file_region, is_active, target);
        }
    }
    else{
        Full_Cursor render_cursor = {};
        Range on_screen_range = {};
        view_call_render_caller(models, target, view, rect, render_cursor, on_screen_range, 0, 0, dont_do_core_render);
    }
    
    draw_pop_clip(target);
}

// BOTTOM

