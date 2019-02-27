/*
 * Mr. 4th Dimention - Allen Webster
 *
 * ??.??.2018
 *
 * View GUI layouts and implementations
 *
 */

// TOP

internal GUI_Scroll_Vars
do_step_file_view(System_Functions *system, Models *models, View *view, i32_Rect rect, b32 is_active, f32 dt, GUI_Scroll_Vars scroll){
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

