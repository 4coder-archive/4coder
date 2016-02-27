/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.09.2015
 *
 * File editing view for 4coder
 *
 */

// TOP

enum Interactive_View_Action{
    INTV_OPEN,
    INTV_SAVE_AS,
    INTV_NEW,
    INTV_SWITCH,
    INTV_KILL,
    INTV_SURE_TO_KILL
};

enum Interactive_View_Interaction{
    INTV_SYS_FILE_LIST,
    INTV_LIVE_FILE_LIST,
    INTV_SURE_TO_KILL_INTER
};

struct Interactive_View{
    View view_base;
    Hot_Directory *hot_directory;
    Style *style;
    Working_Set *working_set;
    Delay *delay;
    Font_Set *font_set;
    UI_State state;
    Interactive_View_Interaction interaction;
    Interactive_View_Action action;
    
    char query_[256];
    String query;
    char dest_[256];
    String dest;
    i32 user_action;
};

inline Interactive_View*
view_to_interactive_view(View *view){
    Interactive_View *result = 0;
    if (view->type == VIEW_TYPE_INTERACTIVE)
        result = (Interactive_View*)view;
    return result;
}

internal void
interactive_view_complete(Interactive_View *view){
    Panel *panel = view->view_base.panel;
    switch (view->action){
    case INTV_OPEN:
    delayed_open(view->delay, view->hot_directory->string, panel);
    break;
        
    case INTV_SAVE_AS:
    delayed_save_as(view->delay, view->hot_directory->string, panel);
    delayed_close_minor(view->delay, panel);
    break;
        
    case INTV_NEW:
        delayed_action(view->delay, DACT_NEW, view->hot_directory->string, panel);
        break;
        
    case INTV_SWITCH:
        delayed_action(view->delay, DACT_SWITCH, view->dest, panel);
        break;
        
    case INTV_KILL:
        delayed_action(view->delay, DACT_TRY_KILL, view->dest, panel);
        break;
        
    case INTV_SURE_TO_KILL:
        switch (view->user_action){
        case 0:
            delayed_action(view->delay, DACT_KILL, view->dest, panel);
            delayed_action(view->delay, DACT_CLOSE_MINOR, {}, panel);
            break;
            
        case 1:
            delayed_action(view->delay, DACT_CLOSE_MINOR, {}, panel);
            break;

        case 2:
            delayed_action(view->delay, DACT_SAVE, view->dest, panel);
            delayed_action(view->delay, DACT_KILL, view->dest, panel);
            delayed_action(view->delay, DACT_CLOSE_MINOR, {}, panel);
            break;
        }
        break;
    }
}

internal i32
step_draw_int_view(System_Functions *system, Interactive_View *view,
                   Render_Target *target, i32_Rect rect,
                   Input_Summary *user_input, b32 input_stage){
    i32 result = 0;
    
    UI_State state =
        ui_state_init(&view->state, target, user_input,
                      view->style, view->font_set, view->working_set, input_stage);
    
    UI_Layout layout;
    begin_layout(&layout, rect);
    
    b32 new_dir = 0;
    b32 complete = 0;
    
    do_label(&state, &layout, view->query, 1.f);
    
    b32 case_sensitive = 0;
    
    switch (view->interaction){
    case INTV_SYS_FILE_LIST:
    {
        b32 is_new = (view->action == INTV_NEW);
        
        if (do_file_list_box(system, &state,
                             &layout, view->hot_directory, 0, !is_new, case_sensitive,
                             &new_dir, &complete, 0)){
            result = 1;
        }
        if (new_dir){
            hot_directory_reload(system,
                                 view->hot_directory, view->working_set);
        }
    }break;
    
    case INTV_LIVE_FILE_LIST:
        if (do_live_file_list_box(system, &state, &layout, view->working_set, &view->dest, &complete)){
            result = 1;
        }
        break;
        
    case INTV_SURE_TO_KILL_INTER:
    {
        i32 action = -1;
        char s_[256];
        String s = make_fixed_width_string(s_);
        append(&s, view->dest);
        append(&s, " has unsaved changes, kill it?");
        do_label(&state, &layout, s, 1.f);
            
        i32 id = 0;
        if (do_list_option(++id, &state, &layout, make_lit_string("(Y)es"))){
            action = 0;
        }
            
        if (do_list_option(++id, &state, &layout, make_lit_string("(N)o"))){
            action = 1;
        }
            
        if (do_list_option(++id, &state, &layout, make_lit_string("(S)ave and kill"))){
            action = 2;
        }
        
        if (action == -1 && input_stage){
            i32 key_count = user_input->keys.count;
            for (i32 i = 0; i < key_count; ++i){
                Key_Event_Data key = user_input->keys.keys[i];
                switch (key.character){
                case 'y': case 'Y': action = 0; break;
                case 'n': case 'N': action = 1; break;
                case 's': case 'S': action = 2; break;
                }
                if (action == -1 && key.keycode == key_esc) action = 1;
                if (action != -1) break;
            }
        }
        
        if (action != -1){
            complete = 1;
            view->user_action = action;
        }
    }break;
    }
    
    if (complete){
        interactive_view_complete(view);
    }
    
    if (ui_finish_frame(&view->state, &state, &layout, rect, 0, 0)){
        result = 1;
    }
    
    return result;
}

Do_View_Sig(do_interactive_view){
    i32 result = 0;
    
    view->mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
    Interactive_View *int_view = (Interactive_View*)view;
    switch (message){
    case VMSG_STEP: case VMSG_DRAW:
        result = step_draw_int_view(system, int_view, target, rect, user_input, (message == VMSG_STEP));
        break;
    }
    
    return result;
}

internal Interactive_View*
interactive_view_init(System_Functions *system, View *view,
                      Hot_Directory *hot_dir, Style *style,
                      Working_Set *working_set, Font_Set *font_set, Delay *delay){
    Interactive_View *result = (Interactive_View*)view;
    view->type = VIEW_TYPE_INTERACTIVE;
    view->do_view = do_interactive_view;
    result->hot_directory = hot_dir;
    hot_directory_clean_end(hot_dir);
    hot_directory_reload(system, hot_dir, working_set);
    result->query = make_fixed_width_string(result->query_);
    result->dest = make_fixed_width_string(result->dest_);
    result->style = style;
    result->working_set = working_set;
    result->font_set = font_set;
    result->delay = delay;
    return result;
}

// BOTTOM

