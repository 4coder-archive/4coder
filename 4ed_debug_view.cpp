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
    DBG_OS_EVENTS,
    DBG_PROFILE
};

struct Dbg_Past_Key{
    Key_Event_Data key;
    i32 frame_index;
    bool8 modifiers[3];
};

struct Debug_View{
    View view_base;
    Font *font;
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
    Font *font = view->font;
    i32 y_advance = font->height;
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
        
        draw_string(target, font, str, rect.x0, y, color);
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
                
                draw_string(target, font, str, rect.x0, y, color);
                y += y_advance;
            }
        }
    }
    
    return y;
}

internal i32
draw_system_memory(Debug_View *view, i32_Rect rect, Render_Target *target, i32 y){
    Font *font = view->font;
    i32 y_advance = font->height;
    Bubble *sentinel = INTERNAL_system_sentinel();
    
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
        
        draw_string(target, font, str, rect.x0, y, color);
        
        y += y_advance;
    }
    
    return y;
}

internal void
draw_background_threads(Debug_View *view, i32_Rect rect, Render_Target *target){
    i32 pending;
    bool8 running[4];
    INTERNAL_get_thread_states(BACKGROUND_THREADS, running, &pending);
    
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
    draw_string(target, view->font, str, trect.x1, trect.y1, light);
}

struct Dbg_Modifier{
    char *name;
    u8 modifier;
};
internal void
draw_modifiers(Debug_View *view, Render_Target *target,
               bool8 *modifiers, u32 on_color, u32 off_color, i32 *x, i32 y){
    persist Dbg_Modifier dm[] = {
        {"CTRL", CONTROL_KEY_CONTROL},
        {"ALT", CONTROL_KEY_ALT},
        {"SHIFT", CONTROL_KEY_SHIFT}
    };
    for (i32 i = 0; i < CONTROL_KEY_COUNT; ++i){
        Dbg_Modifier m = dm[i];
        u32 color;
        
        if (modifiers[m.modifier]) color = on_color;
        else color = off_color;
        
        *x = draw_string(target, view->font, m.name, *x, y, color);
        *x += 5;
    }
}

internal i32
draw_key_event(Debug_View *view, Render_Target *target,
               Dbg_Past_Key *key, i32 x, i32 y, u32 on_color, u32 off_color){
    Font *font = view->font;
    draw_modifiers(view, target, key->modifiers,
                   on_color, off_color, &x, y);
    
    if (font->glyphs[key->key.character].exists){
        char c[2];
        c[0] = (char)key->key.character;
        c[1] = 0;
        x = draw_string(target, font, c, x, y, on_color);
    }
    else{
        char c[10] = {};
        String str = make_fixed_width_string(c);
        append(&str, "\\");
        append_int_to_str(key->key.keycode, &str);
        terminate_with_null(&str);
        x = draw_string(target, font, c, x, y, on_color);
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
    
    Font *font = view->font;
    
    draw_modifiers(view, target, active_input->keys.modifiers,
                   0xFFFFFFFF, 0xFF444444, &x, y);
    max_x = x;
    x = rect.x0;
    y += font->height;
    
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
        y += font->height;
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
            draw_string(target, font, "OUT", x, y, color);
        }
        else{
            color = 0xFF008800;
            draw_string(target, font, "IN", x, y, color);
        }
        y += font->height;
        
        char c[16];
        String s = make_fixed_width_string(c);
        append_int_to_str(active_input->mouse.mx, &s);
        append(&s, ", ");
        append_int_to_str(active_input->mouse.my, &s);
        terminate_with_null(&s);
        draw_string(target, font, c, x, y, color);
        y += font->height;
        
        u32 btn_color;
        if (active_input->mouse.l) btn_color = color;
        else btn_color = 0xFF444444;
        x = draw_string(target, font, "L ", x, y, btn_color);
        
        if (active_input->mouse.r) btn_color = color;
        else btn_color = 0xFF444444;
        x = draw_string(target, font, "R", x, y, btn_color);
        
        x = mrect.x0;
        y += font->height;
        
        s = make_fixed_width_string(c);
        append_int_to_str(view->prev_mouse_wheel, &s);
        terminate_with_null(&s);
        
        if (active_input->mouse.wheel_used) btn_color = color;
        else btn_color = 0xFF444444;
        draw_string(target, font, c, x, y, btn_color);
        
        y += font->height;
    }
}

internal i32
draw_profile_frame(Render_Target *target, Profile_Frame *frame,
                   i32 x, i32 top, i32 bottom, i32 goal, Input_Summary *active_input){
    i32 result = -1;
    
    persist u32 colors[] = {
        0x80C06000,
        0x8000C060,
        0x806000C0,
        0x8060C000,
        0x800060C0,
        0x80C00060,
    };
    
    persist i32 color_max = ArrayCount(colors);
    Mouse_Summary *mouse = &active_input->mouse;
    
    i32 count = frame->events.count;
    Debug_Event *events = frame->events.e;
    
    i32 i;
    for (i = 0; i < count && events[i].type == DBGEV_START; ++i){
        i64 start = events[i++].time;
        i64 end = events[i].time;
        
        real32 rtop = unlerp(0, (real32)end, FRAME_TIME);
        real32 rbot = unlerp(0, (real32)start, FRAME_TIME);
        
        rtop = lerp((real32)bottom, rtop, (real32)goal);
        rbot = lerp((real32)bottom, rbot, (real32)goal);
        
        i32_Rect r = i32R(x, (i32)rtop, x+5, (i32)rbot);
        draw_rectangle(target, r, colors[events[i].event_index % color_max]);
        if (hit_check(mouse->mx, mouse->my, r)) result = (i - 1);
    }
    
    {
        real32 rtop = unlerp(0, (real32)frame->dbg_procing_end, FRAME_TIME);
        real32 rbot = unlerp(0, (real32)frame->dbg_procing_start, FRAME_TIME);
        
        rtop = lerp((real32)bottom, rtop, (real32)goal);
        rbot = lerp((real32)bottom, rbot, (real32)goal);
        
        i32_Rect r = i32R(x, (i32)rtop, x+5, (i32)rbot);
        draw_rectangle(target, r, 0xFF808080);
    }
    
    for (; i < count; ++i){
        
        Assert(events[i].type == DBGEV_MOMENT);
        
        real32 ry = unlerp(0, (real32)events[i].time, FRAME_TIME);
        ry = lerp((real32)bottom, ry, (real32)goal);
        
        i32_Rect r = i32R(x-1, (i32)ry, x+6, (i32)ry+1);
        draw_rectangle(target, r, 0xFFFFFFFF);
        if (hit_check(mouse->mx, mouse->my, r)) result = i;
    }
    
    return result;
}

internal void
draw_profile(Debug_View *view, i32_Rect rect, Render_Target *target, Input_Summary *active_input){
    i32 j = (INTERNAL_frame_index % 30);

    i32 event_index = -1;
    i32 frame_index = -1;
    
    i32 target_time = (rect.y0 + rect.y1)/2;
    
    i32 x = rect.x0;
    for (i32 i = 0; i < PAST_PROFILE_COUNT; ++i){
        Profile_Frame *frame = past_frames + j;
        i32 s = draw_profile_frame(target, frame, x, rect.y0, rect.y1, target_time, active_input);
        if (s != -1){
            event_index = s;
            frame_index = j;
        }
        x += 10;
        j = ((j+1) % PAST_PROFILE_COUNT);
    }
    
    draw_rectangle(target, i32R(rect.x0, target_time, rect.x1, target_time + 1), 0xFFFFFFFF);
    
    char c[200];
    
    if (frame_index != -1){
        Profile_Frame *frame = past_frames + frame_index;
        Debug_Event *events = frame->events.e;
        Debug_Event *event = events + event_index;
        
        Font *font = view->font;
        
        u32 color = 0xFFFFFFFF;
        
        i32 x, y;
        x = rect.x0;
        y = rect.y0;
        
        String s = make_fixed_width_string(c);
        append(&s, event->name);
        append(&s, ": ");
        
        Assert(event->type == DBGEV_START || event->type == DBGEV_MOMENT);
        if (event->type == DBGEV_START){
            Debug_Event *next_event = event + 1;
            Assert(next_event->type == DBGEV_END);
            append_int_to_str((i32)(next_event->time - event->time), &s);
        }
        else{
            append_int_to_str((i32)event->time, &s);
        }
        terminate_with_null(&s);
        draw_string(target, font, c, x, y, color);
        y += font->height;
        
        if (frame->first_key != -1){
            Dbg_Past_Key *key = view->past_keys + frame->first_key;
            Dbg_Past_Key *end_key = view->past_keys + ArrayCount(view->past_keys);
            while (key->frame_index == frame->index){
                draw_key_event(view, target, key,
                               x, y, 0xFFFFFFFF, 0xFF808080);
                y += font->height;
                ++key;
                if (key == end_key) key = view->past_keys;
            }
        }

        i32 count = frame->events.count;
        for (i32 i = 0; i < count; ++i){
            if (events[i].type == DBGEV_START) ++i;
            else{
                s = make_fixed_width_string(c);
                append(&s, events[i].name);
                append(&s, ": ");
                append_int_to_str((i32)events[i].time, &s);
                terminate_with_null(&s);
                draw_string(target, font, c, x, y, color);
                y += font->height;
            }
        }
    }
}

internal i32
draw_debug_view(Debug_View *view, i32_Rect rect, Render_Target *target,
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
        y = draw_system_memory(view, rect, target, y);
    }break;
    case DBG_OS_EVENTS:
    {
        draw_os_events(view, rect, target, active_input);
    }break;
    case DBG_PROFILE:
    {
        draw_background_threads(view, rect, target);
        draw_profile(view, rect, target, active_input);
    }break;
    }
    return result;
}

internal void
step_debug_view(Debug_View *view, i32_Rect rect, Render_Target *target,
               Input_Summary *active_input){
    persist i32 max_past = ArrayCount(view->past_keys);

    bool8 *modifiers = active_input->keys.modifiers;
    for (i32 i = 0; i < active_input->keys.count; ++i){
        i32 this_index = view->past_key_pos;
        Dbg_Past_Key *past_key = view->past_keys + view->past_key_pos;
        ++view->past_key_pos;
        view->past_key_pos = view->past_key_pos % max_past;
        
        past_key->key = active_input->keys.keys[i];
        past_key->modifiers[0] = modifiers[0];
        past_key->modifiers[1] = modifiers[1];
        past_key->modifiers[2] = modifiers[2];

        if (INTERNAL_updating_profile){
            past_key->frame_index = INTERNAL_frame_index;
            if (profile_frame.first_key == -1){
                profile_frame.first_key = this_index;
            }
        }
        else{
            past_key->frame_index = -1;
        }
        
        if (view->past_key_count < max_past) ++view->past_key_count;
    }
    
    if (active_input->mouse.wheel_used)
        view->prev_mouse_wheel = active_input->mouse.wheel_amount;
}

internal
DO_VIEW_SIG(do_debug_view){
    view->mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
    Debug_View *debug_view = (Debug_View*)view;
    i32 result = 0;
    
    switch (message){
    case VMSG_RESIZE: break;
    case VMSG_STYLE_CHANGE: break;
    case VMSG_STEP: step_debug_view(debug_view, rect, target, active_input); result = 1; break;
    case VMSG_DRAW: draw_debug_view(debug_view, rect, target, active_input); break;
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
