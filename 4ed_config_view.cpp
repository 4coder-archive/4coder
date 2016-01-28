/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 27.01.2016
 *
 * Configuration customizing view for 4coder
 *
 */

// TOP

struct Config_View{
    View view_base;
    UI_State state;
    Style *style;
    Font_Set *font_set;
    Working_Set *working_set;
    
    App_Settings *settings;
};

inline Config_View*
view_to_config_view(View *view){
    Config_View *result = 0;
    if (view->type == VIEW_TYPE_CONFIG){
        result = (Config_View*)view;
    }
    return result;
}

internal i32
step_draw_config_view(Config_View *view, Render_Target *target, i32_Rect rect,
                      Input_Summary *user_input, b32 input_stage){
    i32 result = 0;
    
    UI_State state =
        ui_state_init(&view->state, target, user_input,
                      view->style, view->font_set, view->working_set, input_stage);
    
    UI_Layout layout;
    begin_layout(&layout, rect);

    i32 id = 0;
    
    do_label(&state, &layout, literal("Config"), 2.f);
    
    if (do_checkbox_list_option(++id, &state, &layout, make_lit_string("Left Ctrl + Left Alt = AltGr"),
                                view->settings->lctrl_lalt_is_altgr)){
        view->settings->lctrl_lalt_is_altgr = !view->settings->lctrl_lalt_is_altgr;
    }
    
    if (ui_finish_frame(&view->state, &state, &layout, rect, 0, 0)){
        result = 1;
    }
    
    return result;
}

Do_View_Sig(do_config_view){
    i32 result = 0;
        
    Config_View *config_view = (Config_View*)view;
    switch (message){
    case VMSG_STEP: case VMSG_DRAW:
        result = step_draw_config_view(config_view, target, rect, user_input,
                                       (message == VMSG_STEP));
        break;
    }

    return result;
}

internal Config_View*
config_view_init(View *view, Style *style,  Working_Set *working_set,
                 Font_Set *font_set, App_Settings *settings){
    view->type = VIEW_TYPE_CONFIG;
    view->do_view = do_config_view;
    
    Config_View *result;
    result = (Config_View*)view;
    result->style = style;
    result->working_set = working_set;
    result->font_set = font_set;
    result->settings = settings;
    return result;
}

// BOTTOM

