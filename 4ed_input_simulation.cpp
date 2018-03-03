/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 02.03.2018
 *
 * Input simulation implementation
 *
 */

// TOP

internal void
simulate_key(Application_Step_Input *input,
             Key_Code keycode, Key_Code character, Key_Code character_no_caps_lock, i8 *modifiers){
    Key_Input_Data *keys = &input->keys;
    Assert(keys->count < ArrayCount(keys->keys));
    Key_Event_Data *key = &keys->keys[keys->count++];
    key->keycode = keycode;
    key->character = character;
    key->character_no_caps_lock = character_no_caps_lock;
    memcpy(key->modifiers, modifiers, sizeof(key->modifiers));
}

internal void
simulate_key(Application_Step_Input *input,
             Key_Code code, i8 *modifiers){
    i32 size = 0;
    char *keycode_name = global_key_name(code, &size);
    if (keycode_name != 0){
        simulate_key(input, code, 0, 0, modifiers);
    }
    else{
        Key_Code no_caps = code;
        if (modifiers[MDFR_CAPS_INDEX]){
            if (no_caps >= 'a' && no_caps <= 'z'){
                no_caps -= (u8)('a' - 'A');
            }
            else if (no_caps >= 'A' && no_caps <= 'Z'){
                no_caps += (u8)('a' - 'A');
            }
        }
        simulate_key(input, code, code, no_caps, modifiers);
    }
}

internal void
simulate_key(Application_Step_Input *input,
             Key_Code code, u8 modifiers){
    i8 mod_array[MDFR_INDEX_COUNT];
    memset(mod_array, 0, sizeof(mod_array));
    if (modifiers & MDFR_CTRL){
        mod_array[MDFR_CONTROL_INDEX] = 1;
    }
    if (modifiers & MDFR_ALT){
        mod_array[MDFR_ALT_INDEX] = 1;
    }
    if (modifiers & MDFR_CMND){
        mod_array[MDFR_COMMAND_INDEX] = 1;
    }
    if (modifiers & MDFR_SHIFT){
        mod_array[MDFR_SHIFT_INDEX] = 1;
    }
    simulate_key(input, code, mod_array);
}

internal void
simulate_mouse_state(Application_Step_Input *input, Mouse_State state){
    input->mouse = state;
}

internal void
simulate_mouse_update(Application_Step_Input *input, Mouse_State prev_mouse){
    input->mouse = prev_mouse;
    input->mouse.press_l = false;
    input->mouse.press_r = false;
    input->mouse.release_l = false;
    input->mouse.release_r = false;
    input->mouse.wheel = 0;
}

internal void
simulate_mouse_xy(Application_Step_Input *input, i32 x, i32 y, i32 width, i32 height){
    input->mouse.x = x;
    input->mouse.y = y;
    input->mouse.out_of_window = (x < 0 || y < 0 || x > width || y > height);
}

internal void
simulate_mouse_left_press(Application_Step_Input *input){
    input->mouse.l = true;
    input->mouse.press_l = true;
}

internal void
simulate_mouse_left_release(Application_Step_Input *input){
    input->mouse.l = false;
    input->mouse.release_l = true;
}

internal void
simulate_mouse_right_press(Application_Step_Input *input){
    input->mouse.r = true;
    input->mouse.press_r = true;
}

internal void
simulate_mouse_right_release(Application_Step_Input *input){
    input->mouse.r = false;
    input->mouse.release_r = true;
}

internal void
simulate_mouse_wheel(Application_Step_Input *input, i32 wheel){
    input->mouse.wheel = wheel;
}

internal void
simulate_exit(Application_Step_Input *input){
    input->trying_to_kill = true;
}

////////////////

internal void
simulation_init(Input_Simulation_Controls *sim_controls){
    memset(sim_controls, 0, sizeof(*sim_controls));
    sim_controls->enforce_regular_mouse = true;
}

internal void
simulation_step_begin(Input_Simulation_Controls *sim_controls,
                      Application_Step_Input *input,
                      b32 first_step, f32 dt){
    if (sim_controls->enforce_regular_mouse){
        simulate_mouse_update(input, sim_controls->prev_mouse);
    }
    input->first_step = first_step;
    input->dt = dt;
}

internal void
simulation_step_end(Input_Simulation_Controls *sim_controls,
                    Application_Step_Input *input){
    sim_controls->counter += 1;
    sim_controls->prev_mouse = input->mouse;
}

////////////////

internal void
simulation_stream_init(Simulation_Event_Stream_State *stream){
    stream->index = 0;
}

internal void
simulation_drive_from_events(Input_Simulation_Controls *sim_controls,
                             Simulation_Event_Stream_State *stream,
                             Application_Step_Input *input,
                             Simulation_Event *events, i32 event_count,
                             i32 width, i32 height){
    Simulation_Event *event = events + stream->index;
    for (; stream->index < event_count; ++stream->index, ++event){
        if (event->counter_index > sim_controls->counter){
            break;
        }
        
        switch (event->type){
            case SimulationEvent_Noop:InvalidCodePath;
            
            case SimulationEvent_DebugNumber:
            {
                input->debug_number = event->debug_number;
            }break;
            
            case SimulationEvent_Key:
            {
                simulate_key(input, event->key.code, event->key.modifiers);
            }break;
            
            case SimulationEvent_MouseLeftPress:
            {
                simulate_mouse_left_press(input);
            }break;
            
            case SimulationEvent_MouseLeftRelease:
            {
                simulate_mouse_left_release(input);
            }break;
            
            case SimulationEvent_MouseRightPress:
            {
                simulate_mouse_right_press(input);
            }break;
            
            case SimulationEvent_MouseRightRelease:
            {
                simulate_mouse_right_release(input);
            }break;
            
            case SimulationEvent_MouseWheel:
            {
                simulate_mouse_wheel(input, event->wheel);
            }break;
            
            case SimulationEvent_MouseXY:
            {
                simulate_mouse_xy(input, event->mouse_xy.x, event->mouse_xy.y,
                                  width, height);
            }break;
            
            case SimulationEvent_Exit:
            {
                simulate_exit(input);
            }break;
            
            default:InvalidCodePath;
        }
    }
}

// BOTTOM

