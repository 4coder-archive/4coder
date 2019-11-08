/*
4coder_tutorial.cpp - Guided graphical tutorial system.
*/

// TOP

global b32 in_tutorial = false;

function void
kill_tutorial(Application_Links *app){
    if (in_tutorial){
        
    }
}

function void
tutorial_default_4coder_render(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    Rect_f32 region = draw_background_and_margin(app, view_id, is_active_view);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    draw_rectangle(app, rect_inner(region, 10.f), 20.f, fcolor_id(Stag_Margin));
    
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
    
    for (;;){
        User_Input input = get_next_input(app, EventPropertyGroup_Any, 0);
        if (input.abort){
            break;
        }
    }
}

CUSTOM_COMMAND_SIG(tutorial_default_4coder)
CUSTOM_DOC("Tutorial for built in 4coder bindings and features.")
{
    kill_tutorial(app);
    
    Panel_ID root_panel = panel_get_root(app);
    if (panel_split(app, root_panel, Dimension_Y)){
        panel_swap_children(app, root_panel);
        panel_set_split(app, root_panel, PanelSplitKind_Ratio_Min, 0.5f);
        Panel_ID tutorial_panel = panel_get_child(app, root_panel, Side_Min);
        View_ID tutorial_view = panel_get_view(app, tutorial_panel, Access_Always);
        view_set_passive(app, tutorial_view, true);
        view_enqueue_command_function(app, tutorial_view, tutorial_default_4coder_run);
    }
}

// BOTTOM

