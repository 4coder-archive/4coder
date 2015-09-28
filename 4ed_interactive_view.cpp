/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 19.09.2015
 *
 * File editing view for 4coder
 *
 */

// TOP

enum Action_Type{
    DACT_OPEN,
    DACT_SAVE_AS,
    DACT_NEW,
    DACT_SWITCH,
    DACT_KILL,
    DACT_CLOSE_MINOR,
    DACT_CLOSE_MAJOR,
    DACT_THEME_OPTIONS
};

struct Delayed_Action{
    Action_Type type;
    String string;
    Panel *panel;
};

struct Delay{
    Delayed_Action acts[8];
    i32 count, max;
};

inline void
delayed_action(Delay *delay, Action_Type type,
               String string, Panel *panel){
    Assert(delay->count < delay->max);
    Delayed_Action action;
    action.type = type;
    action.string = string;
    action.panel = panel;
    delay->acts[delay->count++] = action;
}

enum Interactive_View_Action{
    INTV_OPEN,
    INTV_SAVE_AS,
    INTV_NEW,
    INTV_SWITCH,
    INTV_KILL,
};

enum Interactive_View_Interaction{
    INTV_SYS_FILE_LIST,
    INTV_LIVE_FILE_LIST,
};

struct Interactive_View{
    View view_base;
    Hot_Directory *hot_directory;
    Style *style;
    Working_Set *working_set;
    Delay *delay;
    UI_State state;
    Interactive_View_Interaction interaction;
    Interactive_View_Action action;
    char query_[256];
    String query;
    char dest_[256];
    String dest;
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
        delayed_action(view->delay, DACT_OPEN,
                       view->hot_directory->string, panel);
        break;
        
    case INTV_SAVE_AS:
        delayed_action(view->delay, DACT_SAVE_AS, view->hot_directory->string, panel);
        delayed_action(view->delay, DACT_CLOSE_MINOR, {}, panel);
        break;
        
    case INTV_NEW:
        delayed_action(view->delay, DACT_NEW, view->hot_directory->string, panel);
        break;
        
    case INTV_SWITCH:
        delayed_action(view->delay, DACT_SWITCH, view->dest, panel);
        break;
        
    case INTV_KILL:
        delayed_action(view->delay, DACT_KILL, view->dest, panel);
        delayed_action(view->delay, DACT_CLOSE_MINOR, {}, panel);
        break;
    }
}

internal i32
step_draw_int_view(Interactive_View *view, Render_Target *target, i32_Rect rect,
                   Input_Summary *user_input, bool32 input_stage){
    i32 result = 0;
    
    UI_State state =
        ui_state_init(&view->state, target, user_input, view->style, view->working_set, input_stage);
    
    UI_Layout layout;
    begin_layout(&layout, rect);
    
    bool32 new_dir = 0;
    bool32 file_selected = 0;

    terminate_with_null(&view->query);
    do_label(&state, &layout, view->query.str, 1.f);
    
    switch (view->interaction){
    case INTV_SYS_FILE_LIST:
        if (do_file_list_box(&state, &layout, view->hot_directory, 0,
                             &new_dir, &file_selected, 0)){
            result = 1;
        }
        if (new_dir){
            hot_directory_reload(view->hot_directory, view->working_set);
        }
        break;
        
    case INTV_LIVE_FILE_LIST:
        if (do_live_file_list_box(&state, &layout, view->working_set, &view->dest, &file_selected)){
            result = 1;
        }
        break;
    }
    
    if (file_selected){
        interactive_view_complete(view);
    }
    
    if (ui_finish_frame(&view->state, &state, &layout, rect, 0, 0)){
        result = 1;
    }
    
    return result;
}

DO_VIEW_SIG(do_interactive_view){
    i32 result = 0;
    
    Interactive_View *int_view = (Interactive_View*)view;
    switch (message){
    case VMSG_STEP: case VMSG_DRAW:
        result = step_draw_int_view(int_view, target, rect, user_input, (message == VMSG_STEP));
        break;
    }
    
    return result;
}

internal Interactive_View*
interactive_view_init(View *view, Hot_Directory *hot_dir, Style *style,
                      Working_Set *working_set, Delay *delay){
    Interactive_View *result = (Interactive_View*)view;
    view->type = VIEW_TYPE_INTERACTIVE;
    view->do_view = do_interactive_view;
    result->hot_directory = hot_dir;
    hot_directory_clean_end(hot_dir);
    hot_directory_reload(hot_dir, working_set);
    result->query = make_fixed_width_string(result->query_);
    result->dest = make_fixed_width_string(result->dest_);
    result->style = style;
    result->working_set = working_set;
    result->delay = delay;
    return result;
}

// BOTTOM

