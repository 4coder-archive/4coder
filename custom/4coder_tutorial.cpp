/*
4coder_tutorial.cpp - Guided graphical tutorial system.
*/

// TOP

global Tutorial_State tutorial = {};

CUSTOM_COMMAND_SIG(kill_tutorial)
CUSTOM_DOC("If there is an active tutorial, kill it.")
{
    if (!tutorial.in_tutorial){
        return;
    }
    
    tutorial.in_tutorial = false;
    view_close(app, tutorial.view);
}

function void
tutorial_activate(Application_Links *app){
    if (!tutorial.in_tutorial){
        return;
    }
    
    Panel_ID panel = view_get_panel(app, tutorial.view);
    Panel_ID parent = panel_get_parent(app, panel);
    panel_set_split(app, parent, PanelSplitKind_Ratio_Min, 0.5f);
    
    tutorial.is_active = true;
}

function void
tutorial_deactivate(Application_Links *app){
    if (!tutorial.in_tutorial){
        return;
    }
    
    Face_ID face = get_face_id(app, 0);
    Face_Metrics metrics = get_face_metrics(app, face);
    f32 line_height = metrics.line_height;
    
    Panel_ID panel = view_get_panel(app, tutorial.view);
    Panel_ID parent = panel_get_parent(app, panel);
    panel_set_split(app, parent, PanelSplitKind_FixedPixels_Min, line_height*4.f);
    
    tutorial.is_active = false;
}

function void
tutorial_action(Application_Links *app, Tutorial_Action action){
    switch (action){
        case TutorialAction_Prev:
        {
            tutorial.slide_index -= 1;
        }break;
        
        case TutorialAction_Next:
        {
            tutorial.slide_index += 1;
        }break;
        
        case TutorialAction_Exit:
        {
            kill_tutorial(app);
        }break;
        
        case TutorialAction_Restart:
        {
            tutorial.slide_index = 0;
        }break;
    }
}

function void
tutorial_init_title_face(Application_Links *app){
    if (tutorial.face == 0){
        Face_ID face = get_face_id(app, 0);
        Face_Description face_description = get_face_description(app, face);
        face_description.parameters.pt_size *= 2;
        tutorial.face = try_create_new_face(app, &face_description);
        if (tutorial.face == 0){
            tutorial.face = face;
        }
    }
}

function void
default_4coder_tutorial_render(Application_Links *app, Frame_Info frame_info, View_ID view_id){
    Face_ID face = get_face_id(app, 0);
    tutorial_init_title_face(app);
    Face_Metrics metrics = get_face_metrics(app, face);
    Face_Metrics tut_metrics = get_face_metrics(app, tutorial.face);
    
    ////////
    
    Scratch_Block scratch(app);
    tutorial.slide_index = clamp(0, tutorial.slide_index, tutorial.slide_count - 1);
    Tutorial_Slide_Function *slide_func = tutorial.slide_func_ptrs[tutorial.slide_index];
    Tutorial_Slide slide = slide_func(app, scratch);
    
    ////////
    
    f32 h0 = get_fancy_line_height(app, 0, &slide.short_details);
    f32 h1 = get_fancy_line_height(app, 0, slide.long_details.first);
    f32 title_height = max(h0, h1);
    
    ////////
    
    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    
    FColor margin_color = get_margin_color(is_active_view?UIHighlight_Active:UIHighlight_None);
    Rect_f32 region = draw_background_and_margin(app, view_id, margin_color, margin_color);
    Rect_f32 prev_clip = draw_set_clip(app, region);
    
    f32 panel_y0 = region.y0 - 3.f;
    
    region = rect_inner(region, 3.f);
    draw_rectangle(app, region, 20.f, fcolor_id(Stag_Back));
    region = rect_inner(region, 10.f);
    
    Vec2_f32 title_p = V2f32(region.x0, panel_y0 + (metrics.line_height*2.f) - title_height*0.5f);
    
    if (is_active_view){
        if (!tutorial.is_active){
            view_enqueue_command_function(app, view_id, tutorial_activate);
            }
    }
    else{
        if (tutorial.is_active){
            view_enqueue_command_function(app, view_id, tutorial_deactivate);
        }
    }
    
    tutorial.hover_action = TutorialAction_None;
    if (tutorial.is_active){
        draw_fancy_block(app, 0, fcolor_zero(), &slide.long_details, title_p);
        
        // NOTE(allen): buttons
        Rect_f32_Pair footer_pair = rect_split_top_bottom_neg(region, metrics.line_height*2.f);
        Rect_f32 footer = footer_pair.max;
        footer.x0 += 10.f;
        footer.y0 -= 10.f;
        footer.y1 -= 10.f;
        
        f32 b_width = metrics.normal_advance*10.f;
        Mouse_State mouse = get_mouse_state(app);
        Vec2_f32 m_p = V2f32(mouse.p);
        
        {
            Rect_f32_Pair pair = rect_split_left_right(footer, b_width);
            footer = pair.max;
            footer.x0 += 10.f;
        if (tutorial.slide_index > 0){
            if (draw_button(app, pair.min, m_p, face, string_u8_litexpr("prev"))){
                tutorial.hover_action = TutorialAction_Prev;
            }
        }
}
        
{
            Rect_f32_Pair pair = rect_split_left_right(footer, b_width);
            footer = pair.max;
            footer.x0 += 10.f;
        if (tutorial.slide_index < tutorial.slide_count - 1){
            if (draw_button(app, pair.min, m_p, face, string_u8_litexpr("next"))){
                tutorial.hover_action = TutorialAction_Next;
            }
        }
        }

{
            Rect_f32_Pair pair = rect_split_left_right(footer, b_width);
            footer = pair.max;
            footer.x0 += 10.f;
            Rect_f32 exit_box = pair.min;
            pair = rect_split_left_right(footer, b_width);
            Rect_f32 restart_box = pair.min;
            
        if (tutorial.slide_index == tutorial.slide_count - 1){
            if (draw_button(app, exit_box, m_p, face, string_u8_litexpr("exit"))){
                tutorial.hover_action = TutorialAction_Exit;
            }
            
                if (draw_button(app, restart_box, m_p, face, string_u8_litexpr("restart"))){
                tutorial.hover_action = TutorialAction_Restart;
            }
        }
}
    }
    else{
        draw_fancy_line(app, 0, fcolor_zero(), &slide.short_details, title_p);
    }
    
    draw_set_clip(app, prev_clip);
}

function void
default_4coder_tutorial_run(Application_Links *app)
{
    View_ID view = get_this_ctx_view(app, Access_Always);
    View_Context ctx = view_current_context(app, view);
    ctx.render_caller = default_4coder_tutorial_render;
    ctx.hides_buffer = true;
    View_Context_Block ctx_block(app, view, &ctx);
    
    tutorial.in_tutorial = true;
    tutorial.view = view;
    
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
                    tutorial.depressed_action = tutorial.hover_action;
                }
            }break;
            
            case InputEventKind_MouseButtonRelease:
            {
                if (in.event.mouse.code == MouseCode_Left){
                    if (tutorial.depressed_action == tutorial.hover_action){
                        tutorial_action(app, tutorial.depressed_action);
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
    
    tutorial.in_tutorial = false;
}

global String_Const_u8 hms_title = string_u8_litexpr("Handmade Seattle Demo");

function void
default_4coder_tutorial_short_details(Application_Links *app, Arena *arena, Fancy_Line *short_details){
    Face_ID face = get_face_id(app, 0);
    push_fancy_string(arena, short_details, tutorial.face, fcolor_id(Stag_Pop1), hms_title);
    push_fancy_string(arena, short_details, face, fcolor_id(Stag_Default), 8.f, 8.f, string_u8_litexpr("Welcome to Handmade Seattle and to this 4coder demo!"));
    push_fancy_string(arena, short_details, face, fcolor_id(Stag_Pop2), string_u8_litexpr("Click here to see more."));
}

function void
default_4coder_tutorial_long_start(Application_Links *app, Arena *arena, Fancy_Block *long_details){
    Fancy_Line *line = push_fancy_line(arena, long_details, tutorial.face, fcolor_id(Stag_Pop1), hms_title);
    
    Face_ID face = get_face_id(app, 0);
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
#define M "If there are any features you'd like to know about or try out, "
    push_fancy_string(arena, line, string_u8_litexpr(M));
#undef M
    push_fancy_string(arena, line, fcolor_id(Stag_Keyword), string_u8_litexpr("ask!"));
    
    //
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Pop1), string_u8_litexpr(""));
}

function Tutorial_Slide
default_4coder_tutorial_slide_1(Application_Links *app, Arena *arena){
    Tutorial_Slide result = {};
    
    Face_ID face = get_face_id(app, 0);
    tutorial_init_title_face(app);
    
    default_4coder_tutorial_short_details(app, arena, &result.short_details);
    
    Fancy_Block *long_details = &result.long_details;
    default_4coder_tutorial_long_start(app, arena, long_details);
    
    Fancy_Line *line = 0;
    
    //
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Keyword), string_u8_litexpr("Let's start with a few navigation commands:"));
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control Comma>");
    push_fancy_stringf(arena, line, "change active panel");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<*AnyArrow*>");
    push_fancy_stringf(arena, line, "move cursor one character or line");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control *AnyArrow*>");
    push_fancy_stringf(arena, line, "move cursor by 'chunks'");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Home>, <End>");
    push_fancy_stringf(arena, line, "move cursor to the first/last character of the line");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<PageUp>, <PageDown>");
    push_fancy_stringf(arena, line, "move cursor by full pages up/down");
    
    //
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Keyword), string_u8_litexpr("Available in code files:"));
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Alt OpenBracket>");
    push_fancy_stringf(arena, line, "move cursor and mark to surrounding scope");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Alt CloseBracket>");
    push_fancy_stringf(arena, line, "move cursor and mark to previous scope");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Alt SingleQuote>");
    push_fancy_stringf(arena, line, "move cursor and mark to next scope");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Alt Shift CloseBracket>");
    push_fancy_stringf(arena, line, "move cursor and mark to previous top-level scope");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Alt Shift SingleQuote>");
    push_fancy_stringf(arena, line, "move cursor and mark to next top-level scope");
    
    return(result);
}

function Tutorial_Slide
default_4coder_tutorial_slide_2(Application_Links *app, Arena *arena){
    Tutorial_Slide result = {};
    
    Face_ID face = get_face_id(app, 0);
    tutorial_init_title_face(app);
    
    default_4coder_tutorial_short_details(app, arena, &result.short_details);
    
    Fancy_Block *long_details = &result.long_details;
    default_4coder_tutorial_long_start(app, arena, long_details);
    
    Fancy_Line *line = 0;
    
    //
    push_fancy_line(arena, long_details, face, fcolor_id(Stag_Keyword), string_u8_litexpr("4coder's default editing paradigm is emacs-like but with some differences:"));
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<TextInsert>");
    push_fancy_stringf(arena, line, "non-modal text insertion works in any user-writable buffers at the cursor");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Backspace>/<Delete>");
    push_fancy_stringf(arena, line, "delete the previous/next character from the cursor");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control Space>");
    push_fancy_stringf(arena, line, "moves the mark to the cursor cursor");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control D>");
    push_fancy_stringf(arena, line, "delete the cursor to mark range");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control C>");
    push_fancy_stringf(arena, line, "copy the cursor to mark range to the clipboard");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control X>");
    push_fancy_stringf(arena, line, "cut the cursor to mark range to the clipboard");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control V>");
    push_fancy_stringf(arena, line, "paste the clipboard to the buffer");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control Shift V>");
    push_fancy_stringf(arena, line, "paste the clipboard to the buffer cycling through the clipboard's 'clips'");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control Z>");
    push_fancy_stringf(arena, line, "undo the last edit");
    
    //
    line = push_fancy_line(arena, long_details, face, fcolor_id(Stag_Default));
    push_fancy_stringf(arena, line, fcolor_id(Stag_Pop2),
                       "%40s ", "<Control Y>");
    push_fancy_stringf(arena, line, "redo the last undone edit");
    
    
    return(result);
}

CUSTOM_COMMAND_SIG(default_4coder_tutorial)
CUSTOM_DOC("Tutorial for built in 4coder bindings and features.")
{
    kill_tutorial(app);
    
    Panel_ID root_panel = panel_get_root(app);
    if (panel_split(app, root_panel, Dimension_Y)){
        panel_swap_children(app, root_panel);
        Panel_ID tutorial_panel = panel_get_child(app, root_panel, Side_Min);
        tutorial.view = panel_get_view(app, tutorial_panel, Access_Always);
        view_set_passive(app, tutorial.view, true);
        tutorial_activate(app);
        
        local_persist Tutorial_Slide_Function *slides[] = {
            default_4coder_tutorial_slide_1,
            default_4coder_tutorial_slide_2,
        };
        tutorial.slide_index = 0;
        tutorial.slide_func_ptrs = slides;
        tutorial.slide_count = ArrayCount(slides);
        
        view_enqueue_command_function(app, tutorial.view, default_4coder_tutorial_run);
    }
}

// BOTTOM

