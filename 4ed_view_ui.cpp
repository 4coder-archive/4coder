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

#if 0
internal void
do_render_file_view(System_Functions *system, View *view, Models *models, GUI_Scroll_Vars *scroll, View *active, i32_Rect rect, b32 is_active, Render_Target *target){
    Editing_File *file = view->file;
    Assert(file != 0);
    draw_push_clip(target, rect);
    if (!view->ui_mode){
        if (file_is_ready(file)){
            Rect_i32 buffer_rect = view_get_buffer_rect(models, view);
            //render_loaded_file_in_view(system, view, models, buffer_rect, is_active, target);
            
            Editing_File *file = view->file;
            i32 line_height = view->line_height;
            
            f32 max_x = (f32)file->settings.display_width;
            i32 max_y = buffer_rect.y1 - buffer_rect.y0 + line_height;
            
            Assert(file != 0);
            Assert(!file->is_dummy);
            Assert(buffer_good(&file->state.buffer));
            
            Partition *part = &models->mem.part;
            Temp_Memory temp = begin_temp_memory(part);
            
            push_align(part, 4);
            
            f32 left_side_space = 0;
            
            i32 max = part_remaining(part)/sizeof(Buffer_Render_Item);
            Buffer_Render_Item *items = push_array(part, Buffer_Render_Item, 0);
            
            b32 wrapped = !file->settings.unwrapped_lines;
            Face_ID font_id = file->settings.font_id;
            Font_Pointers font = system->font.get_pointers_by_id(font_id);
            
            File_Edit_Positions edit_pos = view_get_edit_pos(view);
            f32 scroll_x = edit_pos.scroll.scroll_x;
            f32 scroll_y = edit_pos.scroll.scroll_y;
            
            Full_Cursor render_cursor = view_get_render_cursor(system, view);
            
            i32 item_count = 0;
            i32 end_pos = 0;
            {
                Buffer_Render_Params params;
                params.buffer        = &file->state.buffer;
                params.items         = items;
                params.max           = max;
                params.count         = &item_count;
                params.port_x        = (f32)buffer_rect.x0 + left_side_space;
                params.port_y        = (f32)buffer_rect.y0;
                params.clip_w        = view_width(models, view) - left_side_space;
                params.scroll_x      = scroll_x;
                params.scroll_y      = scroll_y;
                params.width         = max_x;
                params.height        = (f32)max_y;
                params.start_cursor  = render_cursor;
                params.wrapped       = wrapped;
                params.system        = system;
                params.font          = font;
                params.virtual_white = file->settings.virtual_white;
                params.wrap_slashes  = file->settings.wrap_indicator;
                
                Buffer_Render_State state = {};
                Buffer_Layout_Stop stop = {};
                
                f32 line_shift = 0.f;
                b32 do_wrap = false;
                i32 wrap_unit_end = 0;
                
                b32 first_wrap_determination = true;
                i32 wrap_array_index = 0;
                
                do{
                    stop = buffer_render_data(&state, params, line_shift, do_wrap, wrap_unit_end);
                    switch (stop.status){
                        case BLStatus_NeedWrapDetermination:
                        {
                            if (first_wrap_determination){
                                wrap_array_index = binary_search(file->state.wrap_positions, stop.pos, 0, file->state.wrap_position_count);
                                ++wrap_array_index;
                                if (file->state.wrap_positions[wrap_array_index] == stop.pos){
                                    do_wrap = true;
                                    wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                                }
                                else{
                                    do_wrap = false;
                                    wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                                }
                                first_wrap_determination = false;
                            }
                            else{
                                Assert(stop.pos == wrap_unit_end);
                                do_wrap = true;
                                ++wrap_array_index;
                                wrap_unit_end = file->state.wrap_positions[wrap_array_index];
                            }
                        }break;
                        
                        case BLStatus_NeedWrapLineShift:
                        case BLStatus_NeedLineShift:
                        {
                            line_shift = file->state.line_indents[stop.wrap_line_index];
                        }break;
                    }
                }while(stop.status != BLStatus_Finished);
                
                end_pos = state.i;
            }
            push_array(part, Buffer_Render_Item, item_count);
            
            Range on_screen_range = {};
            on_screen_range.first = render_cursor.pos;
            on_screen_range.one_past_last = end_pos;
            
            ////////////////////////////////
            
            if (models->render_caller != 0){
                view_call_render_caller(models, target, view, buffer_rect, render_cursor, on_screen_range, items, item_count, do_core_render);
            }
            else{
                render_loaded_file_in_view__inner(models, target, view, buffer_rect, render_cursor, on_screen_range, items, item_count);
            }
            
            end_temp_memory(temp);
        }
    }
    else{
        Full_Cursor render_cursor = {};
        Range on_screen_range = {};
        if (models->render_caller != 0){
            view_call_render_caller(models, target, view, buffer_rect, render_cursor, on_screen_range, 0, 0, dont_do_core_render);
        }
    }
    draw_pop_clip(target);
}
#endif

// BOTTOM

