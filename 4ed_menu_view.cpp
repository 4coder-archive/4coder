/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 26.09.2015
 *
 * File editing view for 4coder
 *
 */

// TOP

struct Menu_View{
    View view_base;
    Style *style;
    Working_Set *working_set;
    Delay *delay;
    Font_Set *font_set;
    UI_State state;
};

inline Menu_View*
view_to_menu_view(View *view){
    Menu_View *result = 0;
    if (view->type == VIEW_TYPE_MENU){
        result = (Menu_View*)view;
    }
    return result;
}

internal i32
step_draw_menu_view(Menu_View *view, Render_Target *target, i32_Rect rect,
                    Input_Summary *user_input, bool32 input_stage){
    i32 result = 0;
    
    UI_State state =
        ui_state_init(&view->state, target, user_input,
                      view->style, view->font_set, view->working_set, input_stage);
    
    UI_Layout layout;
    begin_layout(&layout, rect);

    i32 id = 0;
    
    do_label(&state, &layout, literal("Menu"), 2.f);
    
    if (do_list_option_lit(++id, &state, &layout, "Theme Options")){
        delayed_action(view->delay, DACT_THEME_OPTIONS, {}, view->view_base.panel);
    }
    
    if (ui_finish_frame(&view->state, &state, &layout, rect, 0, 0)){
        result = 1;
    }
    
    return result;
}

Do_View_Sig(do_menu_view){
    i32 result = 0;
    
    Menu_View *menu_view = (Menu_View*)view;
    switch (message){
    case VMSG_STEP: case VMSG_DRAW:
        result = step_draw_menu_view(menu_view, target, rect, user_input, (message == VMSG_STEP));
        break;
    }
    
    return result;
}

internal Menu_View*
menu_view_init(View *view, Style *style, Working_Set *working_set,
               Delay *delay, Font_Set *font_set){
    view->type = VIEW_TYPE_INTERACTIVE;
    view->do_view = do_menu_view;
    
    Menu_View *result;
    result = (Menu_View*)view;
    result->style = style;
    result->working_set = working_set;
    result->delay = delay;
    result->font_set = font_set;
    return result;
}

// BOTTOM


