/*
4coder_tutorial.cpp - Guided graphical tutorial system.
*/

// TOP

typedef i32 Tutorial_Action;
enum{
    TutorialAction_None,
    TutorialAction_Prev,
    TutorialAction_Next,
};

global b32 in_tutorial = false;
global View_ID tutorial_view = 0;
global Face_ID tutorial_face = 0;
global b32 tutorial_is_active = false;
global Tutorial_Action tutorial_hover_action = TutorialAction_None;
global Tutorial_Action tutorial_depressed_action = TutorialAction_None;

CUSTOM_COMMAND_SIG(kill_tutorial)
CUSTOM_DOC("If in the tutorial state - kill it.")
{
    if (!in_tutorial){
        return;
    }
    
    in_tutorial = false;
    view_close(app, tutorial_view);
}

function void
tutorial_activate(Application_Links *app){
    if (!in_tutorial){
        return;
    }
    
    Panel_ID panel = view_get_panel(app, tutorial_view);
    Panel_ID parent = panel_get_parent(app, panel);
    panel_set_split(app, parent, PanelSplitKind_Ratio_Min, 0.5f);
    
    tutorial_is_active = true;
}

function void
tutorial_deactivate(Application_Links *app){
    if (!in_tutorial){
        return;
    }
    
    Face_ID face = get_face_id(app, 0);
    Face_Metrics metrics = get_face_metrics(app, face);
    f32 line_height = metrics.line_height;
    
    Panel_ID panel = view_get_panel(app, tutorial_view);
    Panel_ID parent = panel_get_parent(app, panel);
    panel_set_split(app, parent, PanelSplitKind_FixedPixels_Min, line_height*4.f);
    
    tutorial_is_active = false;
}

function void
tutorial_action(Application_Links *app, Tutorial_Action action){
    
}

function void
tutorial_default_4coder_render(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    FColor margin_color = get_margin_color(is_active_view?UIHighlight_Active:UIHighlight_None);
    Rect_f32 region = draw_background_and_margin(app, view_id, margin_color, margin_color);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    f32 panel_y0 = region.y0 - 3.f;
    
    region = rect_inner(region, 3.f);
    draw_rectangle(app, region, 20.f, fcolor_id(Stag_Back));
    region = rect_inner(region, 10.f);
    
    Face_ID face = get_face_id(app, 0);
    
    if (tutorial_face == 0){
        Face_Description face_description = get_face_description(app, face);
        face_description.parameters.pt_size *= 2;
        tutorial_face = try_create_new_face(app, &face_description);
        if (tutorial_face == 0){
            tutorial_face = face;
        }
    }
    
    Face_Metrics metrics = get_face_metrics(app, face);
    Face_Metrics tut_metrics = get_face_metrics(app, tutorial_face);
    
    Vec2_f32 title_p = V2f32(region.x0, panel_y0 + (metrics.line_height*2.f) - tut_metrics.line_height*0.5f);
    
    if (is_active_view){
        if (!tutorial_is_active){
            view_enqueue_command_function(app, view_id, tutorial_activate);
            }
    }
    else{
        if (tutorial_is_active){
            view_enqueue_command_function(app, view_id, tutorial_deactivate);
        }
    }
    
    Scratch_Block scratch(app);
    
    String_Const_u8 title = string_u8_litexpr("Handmade Seattle Demo");
    
    Fancy_Block long_details = {};
    push_fancy_line(scratch, &long_details, tutorial_face, fcolor_id(Stag_Pop1), title);
    
    //
        Fancy_Line *line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
#define M "If there are any features you'd like to know about or try out, "
        push_fancy_string(scratch, line, string_u8_litexpr(M));
        #undef M
        push_fancy_string(scratch, line, fcolor_id(Stag_Keyword), string_u8_litexpr("ask!"));
    
    
    //
    push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr(""));
    
    //
    push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Keyword), string_u8_litexpr("Let's start with a few navigation commands:"));
    
    //
        line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(scratch, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control Comma>");
    push_fancy_stringf(scratch, line, "change active panel");
    
    //
    line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(scratch, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<*AnyArrow*>");
    push_fancy_stringf(scratch, line, "move cursor one character or line");
    
    //
    line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(scratch, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control *AnyArrow*>");
    push_fancy_stringf(scratch, line, "move cursor by 'chunks'");
    
    //
    line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(scratch, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Home>, <End>");
    push_fancy_stringf(scratch, line, "move cursor to the first/last character of the line");
    
    //
    line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(scratch, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<PageUp>, <PageDown>");
    push_fancy_stringf(scratch, line, "move cursor by full pages up/down");
    
    //
    push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Keyword), string_u8_litexpr("Available in code files:"));
    
    //
    line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(scratch, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Alt OpenBracket>");
    push_fancy_stringf(scratch, line, "move cursor and mark to surrounding scope");
    
    //
    line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(scratch, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Alt CloseBracket>");
    push_fancy_stringf(scratch, line, "move cursor and mark to previous scope");
    
    //
    line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(scratch, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Alt SingleQuote>");
    push_fancy_stringf(scratch, line, "move cursor and mark to next scope");
    
    //
    line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(scratch, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Alt Shift CloseBracket>");
    push_fancy_stringf(scratch, line, "move cursor and mark to previous top-level scope");
    
    //
    line = push_fancy_line(scratch, &long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(scratch, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Alt Shift SingleQuote>");
    push_fancy_stringf(scratch, line, "move cursor and mark to next top-level scope");
    
    
    ////
    Fancy_Line short_details = {};
        push_fancy_string(scratch, &short_details, tutorial_face, fcolor_id(Stag_Pop1), title);
    push_fancy_string(scratch, &short_details, face, fcolor_id(Stag_Default), 8.f, 8.f, string_u8_litexpr("Welcome to Handmade Seattle and to this 4coder demo!"));
        push_fancy_string(scratch, &short_details, face, fcolor_id(Stag_Pop2), string_u8_litexpr("Click here to see more."));
    
    tutorial_hover_action = TutorialAction_None;
    
    if (tutorial_is_active){
        draw_fancy_block(app, 0, fcolor_zero(), &long_details, title_p);
        
        // NOTE(allen): buttons
        Rect_f32_Pair pair = rect_split_top_bottom_neg(region, metrics.line_height*2.f);
        Rect_f32 footer = pair.max;
        footer.x0 += 10.f;
        footer.y0 -= 10.f;
        footer.y1 -= 10.f;
        
        f32 b_width = metrics.normal_advance*10.f;
        Rect_f32_Pair b1_pair = rect_split_left_right(footer, b_width);
        footer = b1_pair.max;
        footer.x0 += 10.f;
        Rect_f32_Pair b2_pair = rect_split_left_right(footer, b_width);
        
        Mouse_State mouse = get_mouse_state(app);
        Vec2_f32 m_p = V2f32(mouse.p);
        
        if (draw_button(app, b1_pair.min, m_p, face, string_u8_litexpr("prev"))){
            tutorial_hover_action = TutorialAction_Prev;
        }
        if (draw_button(app, b2_pair.min, m_p, face, string_u8_litexpr("next"))){
            tutorial_hover_action = TutorialAction_Next;
        }
    }
    else{
        draw_fancy_line(app, 0, fcolor_zero(), &short_details, title_p);
    }
    
    draw_set_clip(app, prev_clip);
}

function void
tutorial_default_4coder_run(Application_Links *app)
{
    View_ID view = get_this_ctx_view(app, Access_Always);
    View_Context ctx = view_current_context(app, view);
    ctx.render_caller = tutorial_default_4coder_render;
    ctx.hides_buffer = true;
    View_Context_Block ctx_block(app, view, &ctx);
    
    in_tutorial = true;
    tutorial_view = view;
    
    for (;;){
        User_Input in = get_next_input(app, EventPropertyGroup_Any, 0);
        if (in.abort){
            break;
        }
        
    b32 handled = true;
        switch (in.event.kind){
            case InputEventKind_MouseButton:
            {
                if (in.event.mouse.code == MouseCode_Left){
                    tutorial_depressed_action = tutorial_hover_action;
                }
            }break;
            
            case InputEventKind_MouseButtonRelease:
            {
                if (in.event.mouse.code == MouseCode_Left){
                    if (tutorial_depressed_action == tutorial_hover_action){
                        tutorial_action(app, tutorial_depressed_action);
                    }
                }
            }break;
            
            case InputEventKind_Core:
            {
                switch (in.event.core.code){
                    case CoreCode_ClickActivateView:
                    {
                        tutorial_activate(app);
                    }break;
                    
                    case CoreCode_ClickDeactivateView:
                    {
                        tutorial_deactivate(app);
                    }break;
                    
                    default:
                    {
                        handled = false;
                    }break;
                }
            }break;
            
            default:
            {
                handled = false;
            }break;
        }

        if (!handled){
            Mapping *mapping = ctx.mapping;
            Command_Map *map = mapping_get_map(mapping, ctx.map_id);
            
            Fallback_Dispatch_Result disp_result =
                fallback_command_dispatch(app, mapping, map, &in);
            if (disp_result.code == FallbackDispatch_DelayedUICall){
                call_after_ctx_shutdown(app, view, disp_result.func);
                break;
            }
            if (disp_result.code == FallbackDispatch_Unhandled){
                leave_current_input_unhandled(app);
            }
        }
    }
    
    in_tutorial = false;
}

CUSTOM_COMMAND_SIG(default_4coder_tutorial)
CUSTOM_DOC("Tutorial for built in 4coder bindings and features.")
{
    kill_tutorial(app);
    
    Panel_ID root_panel = panel_get_root(app);
    if (panel_split(app, root_panel, Dimension_Y)){
        panel_swap_children(app, root_panel);
        Panel_ID tutorial_panel = panel_get_child(app, root_panel, Side_Min);
        tutorial_view = panel_get_view(app, tutorial_panel, Access_Always);
        view_set_passive(app, tutorial_view, true);
        tutorial_activate(app);
        view_enqueue_command_function(app, tutorial_view, tutorial_default_4coder_run);
    }
}

// BOTTOM

