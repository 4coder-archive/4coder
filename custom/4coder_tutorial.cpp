/*
4coder_tutorial.cpp - Guided graphical tutorial system.
*/

// TOP

global b32 in_tutorial = false;
global View_ID tutorial_view = 0;
global Face_ID tutorial_face = 0;
global b32 tutorial_is_active = false;

function void
kill_tutorial(Application_Links *app){
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
tutorial_default_4coder_render(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    FColor margin_color = get_margin_color(is_active_view?UIHighlight_Active:UIHighlight_None);
    Rect_f32 region = draw_background_and_margin(app, view_id, margin_color, margin_color);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    region = rect_inner(region, 3.f);
    draw_rectangle(app, region, 20.f, fcolor_id(Stag_Back));
    
    if (tutorial_face == 0){
        Face_ID face = get_face_id(app, 0);
        Face_Description face_description = get_face_description(app, face);
        face_description.parameters.pt_size *= 2;
        tutorial_face = try_create_new_face(app, &face_description);
        if (tutorial_face == 0){
            tutorial_face = face;
        }
    }
    
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
    
    Fancy_Line line = {};
    push_fancy_string(scratch, &line, tutorial_face, fcolor_id(Stag_Default), string_u8_litexpr("Tutorial"));
    
    Vec2_f32 line_dim = get_fancy_line_dim(app, 0, &line);
    Vec2_f32 p = (region.p0 + region.p1 - line_dim)*0.5f;
    draw_fancy_line(app, 0, fcolor_zero(), &line, p);
    
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

CUSTOM_COMMAND_SIG(tutorial_default_4coder)
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

