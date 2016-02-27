/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 13.09.2015
 *
 * Internal debug view for 4coder
 *
 */

// TOP
#if FRED_INTERNAL

enum Debug_Mode{
    DBG_MEMORY,
    DBG_OS_EVENTS
};

struct Dbg_Past_Key{
    Key_Event_Data key;
    i32 frame_index;
    bool8 modifiers[3];
};

struct Debug_View{
    View view_base;
    i16 font_id;
    Debug_Mode mode;
    Dbg_Past_Key past_keys[32];
    i32 past_key_count, past_key_pos;
    i16 prev_mouse_wheel;
};

inline Debug_View*
view_to_debug_view(View *view){
    Assert(!view || view->type == VIEW_TYPE_DEBUG);
    return (Debug_View*)view;
}

internal i32
draw_general_memory(Debug_View *view, i32_Rect rect, Render_Target *target, i32 y){
    i16 font_id = view->font_id;
    i32 y_advance = get_font_info(&target->font_set, font_id)->height;
    Bubble *sentinel = &view->view_base.mem->general.sentinel;
    
    for (Bubble *bubble = sentinel->next;
         bubble != sentinel;
         bubble = bubble->next){
        bool32 used = (bubble->flags & MEM_BUBBLE_USED) != 0;
        u32 color;
        if (used) color = 0xFFFFFFFF;
        else color = 0xFF00FFFF;
        
        char str[256];
        String s = make_fixed_width_string(str);
        if (used){
            switch (bubble->type){
            case BUBBLE_BUFFER: append(&s, "buffer "); break;
            case BUBBLE_STARTS: append(&s, "starts "); break;
            case BUBBLE_WIDTHS: append(&s, "widths "); break;
            case BUBBLE_WRAPS: append(&s, "wraps "); break;
            case BUBBLE_TOKENS: append(&s, "tokens "); break;
            case BUBBLE_UNDO_STRING: append(&s, "undo string "); break;
            case BUBBLE_UNDO: append(&s, "undo "); break;
            default: append(&s, "unknown "); break;
            }
        }
        else{
            append(&s, "unused ");
        }
        append_int_to_str(bubble->size, &s);
        terminate_with_null(&s);
        
        draw_string(target, font_id, str, rect.x0, y, color);
        y += y_advance;
        
        Bubble *next = bubble->next;
        if (next != sentinel){
            u8 *end_ptr = (u8*)(bubble + 1) + bubble->size;
            u8 *next_ptr = (u8*)(next);
            if (end_ptr != next_ptr){
                color = 0xFFFF0000;
                s = make_fixed_width_string(str);
                append(&s, "discontinuity");
                terminate_with_null(&s);
                
                draw_string(target, font_id, str, rect.x0, y, color);
                y += y_advance;
            }
        }
    }
    
    return y;
}

internal i32
draw_system_memory(System_Functions *system, Debug_View *view, i32_Rect rect,
                   Render_Target *target, i32 y){
    i16 font_id = view->font_id;
    i32 y_advance = get_font_info(&target->font_set, font_id)->height;
    Bubble *sentinel = system->internal_sentinel();
    
    for (Bubble *bubble = sentinel->next;
         bubble != sentinel;
         bubble = bubble->next){
        Sys_Bubble *sysb = (Sys_Bubble*)bubble;
        u32 color = 0xFFFFFFFF;
        
        char str[256];
        String s = make_fixed_width_string(str);
        
        append(&s, sysb->file_name);
        append(&s, " ");
        append_int_to_str(sysb->line_number, &s);
        append(&s, " ");
        append_int_to_str(bubble->size, &s);
        terminate_with_null(&s);
        
        draw_string(target, font_id, str, rect.x0, y, color);
        
        y += y_advance;
    }
    
    return y;
}

internal void
draw_background_threads(System_Functions *system,
                        Debug_View *view, i32_Rect rect, Render_Target *target){
    i32 pending;
    bool8 running[4];
    system->internal_get_thread_states(BACKGROUND_THREADS, running, &pending);
    
    i32 box_size = 30;
    
    i32_Rect trect;
    trect.x0 = rect.x1 - box_size;
    trect.y0 = rect.y0;
    trect.x1 = rect.x1;
    trect.y1 = rect.y0 + box_size;
    
    u32 light = 0xFF606060;
    for (i32 i = 0; i < 4; ++i){
        u32 color;
        if (running[i]) color = 0xFF9090FF;
        else color = 0xFF101010;
        draw_rectangle(target, trect, color);
        draw_rectangle_outline(target, trect, light);
        trect.x0 -= box_size;
        trect.x1 -= box_size;
    }
    
    char str[256];
    String s = make_fixed_width_string(str);
    append_int_to_str(pending, &s);
    terminate_with_null(&s);
    draw_string(target, view->font_id, str, trect.x1, trect.y1, light);
}

struct Dbg_Modifier{
    char *name;
    u8 modifier;
};
internal void
draw_modifiers(Debug_View *view, Render_Target *target,
               bool8 *modifiers, u32 on_color, u32 off_color, i32 *x, i32 y){
    persist Dbg_Modifier dm[] = {
        {"CTRL", MDFR_CONTROL_INDEX},
        {"ALT", MDFR_ALT_INDEX},
        {"SHIFT", MDFR_SHIFT_INDEX}
    };
    for (i32 i = 0; i < MDFR_INDEX_COUNT; ++i){
        Dbg_Modifier m = dm[i];
        u32 color;
        
        if (modifiers[m.modifier]) color = on_color;
        else color = off_color;
        
        *x = draw_string(target, view->font_id, m.name, *x, y, color);
        *x += 5;
    }
}

internal i32
draw_key_event(Debug_View *view, Render_Target *target,
               Dbg_Past_Key *key, i32 x, i32 y, u32 on_color, u32 off_color){
    draw_modifiers(view, target, key->modifiers,
                   on_color, off_color, &x, y);

    i16 font_id = view->font_id;
    Render_Font *font = get_font_info(&target->font_set, font_id)->font;
    
    if (font && font->glyphs[key->key.character].exists){
        char c[2];
        c[0] = (char)key->key.character;
        c[1] = 0;
        x = draw_string(target, font_id, c, x, y, on_color);
    }
    else{
        char c[10] = {};
        String str = make_fixed_width_string(c);
        append(&str, "\\");
        append_int_to_str(key->key.keycode, &str);
        terminate_with_null(&str);
        x = draw_string(target, font_id, c, x, y, on_color);
    }
    
    return x;
}

internal void
draw_os_events(Debug_View *view, i32_Rect rect, Render_Target *target,
               Input_Summary *active_input){
    persist i32 max_past = ArrayCount(view->past_keys);
    
    i32 x, y, max_x, max_y;
    x = rect.x0;
    y = rect.y0;

    i16 font_id = view->font_id;
    i32 line_height = get_font_info(&target->font_set, font_id)->height;

#if 0
    draw_modifiers(view, target, active_input->keys.modifiers,
                   0xFFFFFFFF, 0xFF444444, &x, y);
#endif
    
    max_x = x;
    x = rect.x0;
    y += line_height;
    
    for (i32 j = 0; j < view->past_key_count; ++j){
        Dbg_Past_Key *key = view->past_keys + j;
        u32 on_color, off_color;
        
        switch ((view->past_key_pos - j - 1 + max_past*2) % max_past){
        case 0: on_color = 0xFFAAAAFF; off_color = 0xFF505088; break;
        case 1: on_color = 0xFF9999CC; off_color = 0xFF404077; break;
        case 2: on_color = 0xFF8888AA; off_color = 0xFF303066; break;
        default: on_color = 0xFF888888; off_color = 0xFF303030; break;
        }
        
        x = draw_key_event(view, target, key, x, y, on_color, off_color);
        
        if (max_x < x) max_x = x;
        x = rect.x0;
        y += line_height;
    }
    
    i32_Rect mrect = rect;
    mrect.x0 = max_x + 1;
    
    max_y = y;
    x = mrect.x0;
    y = mrect.y0;
    
    {
        u32 color;
        if (active_input->mouse.out_of_window){
            color = 0xFFFF0000;
            draw_string(target, font_id, "OUT", x, y, color);
        }
        else{
            color = 0xFF008800;
            draw_string(target, font_id, "IN", x, y, color);
        }
        y += line_height;
        
        char c[16];
        String s = make_fixed_width_string(c);
        append_int_to_str(active_input->mouse.x, &s);
        append(&s, ", ");
        append_int_to_str(active_input->mouse.y, &s);
        terminate_with_null(&s);
        draw_string(target, font_id, c, x, y, color);
        y += line_height;
        
        u32 btn_color;
        if (active_input->mouse.l) btn_color = color;
        else btn_color = 0xFF444444;
        x = draw_string(target, font_id, "L ", x, y, btn_color);
        
        if (active_input->mouse.r) btn_color = color;
        else btn_color = 0xFF444444;
        x = draw_string(target, font_id, "R", x, y, btn_color);
        
        x = mrect.x0;
        y += line_height;
        
        s = make_fixed_width_string(c);
        append_int_to_str(view->prev_mouse_wheel, &s);
        terminate_with_null(&s);
        
        if (active_input->mouse.wheel != 0) btn_color = color;
        else btn_color = 0xFF444444;
        draw_string(target, font_id, c, x, y, btn_color);
        
        y += line_height;
    }
}

internal i32
draw_debug_view(System_Functions *system,
                Debug_View *view, i32_Rect rect, Render_Target *target,
                Input_Summary *active_input){
    i32 result = 0;
    
    draw_rectangle(target, rect, 0xFF000000);
    
    switch (view->mode){
    case DBG_MEMORY:
    {
        i32 y = rect.y0;
        y = draw_general_memory(view, rect, target, y);
        draw_rectangle(target, i32R(rect.x0, y, rect.x1, y+2), 0xFF222222);
        y += 2;
        y = draw_system_memory(system, view, rect, target, y);
    }break;
    case DBG_OS_EVENTS:
    {
        draw_os_events(view, rect, target, active_input);
    }break;
    }
    return result;
}

internal void
step_debug_view(Debug_View *view, i32_Rect rect, Render_Target *target,
               Input_Summary *active_input){
    persist i32 max_past = ArrayCount(view->past_keys);
    AllowLocal(max_past);
}

internal
Do_View_Sig(do_debug_view){
    view->mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
    Debug_View *debug_view = (Debug_View*)view;
    i32 result = 0;
    
    switch (message){
    case VMSG_RESIZE: break;
    case VMSG_STYLE_CHANGE: break;
    case VMSG_STEP: step_debug_view(debug_view, rect, target, active_input); result = 1; break;
    case VMSG_DRAW: draw_debug_view(system, debug_view, rect, target, active_input); break;
    case VMSG_FREE: break;
    }
    
    return result;
}

internal Debug_View*
debug_view_init(View *view){
    Debug_View *result = (Debug_View*)view;
    view->type = VIEW_TYPE_DEBUG;
    view->do_view = do_debug_view;
    return result;
}

#endif
// BOTTOM
