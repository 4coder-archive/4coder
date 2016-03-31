/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.02.2016
 *
 * GUI system for 4coder
 *
 */

// TOP

struct Single_Line_Input_Step{
	b8 hit_newline;
	b8 hit_ctrl_newline;
	b8 hit_a_character;
    b8 hit_backspace;
	b8 hit_esc;
	b8 made_a_change;
    b8 did_command;
    b8 no_file_match;
};

enum Single_Line_Input_Type{
	SINGLE_LINE_STRING,
	SINGLE_LINE_FILE
};

struct Single_Line_Mode{
	Single_Line_Input_Type type;
	String *string;
	Hot_Directory *hot_directory;
    b32 fast_folder_select;
    b32 try_to_match;
    b32 case_sensitive;
};

internal Single_Line_Input_Step
app_single_line_input_core(System_Functions *system, Working_Set *working_set,
                           Key_Event_Data key, Single_Line_Mode mode){
    Single_Line_Input_Step result = {};
    
    if (key.keycode == key_back){
        result.hit_backspace = 1;
        if (mode.string->size > 0){
            result.made_a_change = 1;
            --mode.string->size;
            switch (mode.type){
            case SINGLE_LINE_STRING:
                mode.string->str[mode.string->size] = 0; break;
            
            case SINGLE_LINE_FILE:
            {
                char end_character = mode.string->str[mode.string->size];
                if (char_is_slash(end_character)){
                    mode.string->size = reverse_seek_slash(*mode.string) + 1;
                    mode.string->str[mode.string->size] = 0;
                    hot_directory_set(system, mode.hot_directory, *mode.string, working_set);
                }
                else{
                    mode.string->str[mode.string->size] = 0;
                }
            }break;
            }
        }
    }
    
    else if (key.character == '\n' || key.character == '\t'){
        result.made_a_change = 1;
        if (key.modifiers[MDFR_CONTROL_INDEX] ||
            key.modifiers[MDFR_ALT_INDEX]){
            result.hit_ctrl_newline = 1;
        }
        else{
            result.hit_newline = 1;
            if (mode.fast_folder_select){
                Hot_Directory_Match match;
                char front_name_space[256];
                String front_name = make_fixed_width_string(front_name_space);
                get_front_of_directory(&front_name, *mode.string);
                
                match =
                    hot_directory_first_match(mode.hot_directory, front_name, 1, 1, mode.case_sensitive);

                if (mode.try_to_match && !match.filename.str){
                    match = hot_directory_first_match(mode.hot_directory, front_name, 1, 0, mode.case_sensitive);
                }
                if (match.filename.str){
                    if (match.is_folder){
                        set_last_folder(mode.string, match.filename, mode.hot_directory->slash);
                        hot_directory_set(system, mode.hot_directory, *mode.string, working_set);
                        result.hit_newline = 0;
                    }
                    else{
                        if (mode.try_to_match){
                            mode.string->size = reverse_seek_slash(*mode.string) + 1;
                            append(mode.string, match.filename);
                        }
                    }
                }
                else{
                    if (mode.try_to_match){
                        result.no_file_match = 1;
                    }
                }
            }
        }
    }
    
    else if (key.keycode == key_esc){
        result.hit_esc = 1;
        result.made_a_change = 1;
    }
    
    else if (key.character){
        result.hit_a_character = 1;
        if (!key.modifiers[MDFR_CONTROL_INDEX] &&
            !key.modifiers[MDFR_ALT_INDEX]){
            if (mode.string->size+1 < mode.string->memory_size){
                u8 new_character = (u8)key.character;
                mode.string->str[mode.string->size] = new_character;
                mode.string->size++;
                mode.string->str[mode.string->size] = 0;
                if (mode.type == SINGLE_LINE_FILE && char_is_slash(new_character)){
                    hot_directory_set(system, mode.hot_directory, *mode.string, working_set);
                }
                result.made_a_change = 1;
            }
        }
        else{
            result.did_command = 1;
            result.made_a_change = 1;
        }
    }
    
    return result;
}

inline Single_Line_Input_Step
app_single_line_input_step(System_Functions *system, Key_Event_Data key, String *string){
	Single_Line_Mode mode = {};
	mode.type = SINGLE_LINE_STRING;
	mode.string = string;
	return app_single_line_input_core(system, 0, key, mode);
}

inline Single_Line_Input_Step
app_single_file_input_step(System_Functions *system,
                           Working_Set *working_set, Key_Event_Data key,
						   String *string, Hot_Directory *hot_directory,
                           b32 fast_folder_select, b32 try_to_match, b32 case_sensitive){
	Single_Line_Mode mode = {};
	mode.type = SINGLE_LINE_FILE;
	mode.string = string;
	mode.hot_directory = hot_directory;
    mode.fast_folder_select = fast_folder_select;
    mode.try_to_match = try_to_match;
    mode.case_sensitive = case_sensitive;
    return app_single_line_input_core(system, working_set, key, mode);
}

inline Single_Line_Input_Step
app_single_number_input_step(System_Functions *system, Key_Event_Data key, String *string){
    Single_Line_Input_Step result = {};
    Single_Line_Mode mode = {};
    mode.type = SINGLE_LINE_STRING;
    mode.string = string;

    char c = (char)key.character;
    if (c == 0 || c == '\n' || char_is_numeric(c))
        result = app_single_line_input_core(system, 0, key, mode);
    return result;
}

struct Widget_ID{
    i32 id;
    i32 sub_id0;
    i32 sub_id1;
    i32 sub_id2;
};

inline b32
widget_match(Widget_ID s1, Widget_ID s2){
    return (s1.id == s2.id && s1.sub_id0 == s2.sub_id0 &&
            s1.sub_id1 == s2.sub_id1 && s1.sub_id2 == s2.sub_id2);
}

struct UI_State{
    Render_Target *target;
    Style *style;
    Font_Set *font_set;
    Mouse_State *mouse;
    Key_Summary *keys;
    Working_Set *working_set;
    i16 font_id;

    Widget_ID selected, hover, hot;
    b32 activate_me;
    b32 redraw;
    b32 input_stage;
    i32 sub_id1_change;

    f32 height, view_y;
};

inline Widget_ID
make_id(UI_State *state, i32 id){
    Widget_ID r = state->selected;
    r.id = id;
    return r;
}

inline Widget_ID
make_sub0(UI_State *state, i32 id){
    Widget_ID r = state->selected;
    r.sub_id0 = id;
    return r;
}

inline Widget_ID
make_sub1(UI_State *state, i32 id){
    Widget_ID r = state->selected;
    r.sub_id1 = id;
    return r;
}

inline Widget_ID
make_sub2(UI_State *state, i32 id){
    Widget_ID r = state->selected;
    r.sub_id2 = id;
    return r;
}

inline b32
is_selected(UI_State *state, Widget_ID id){
    return widget_match(state->selected, id);
}

inline b32
is_hot(UI_State *state, Widget_ID id){
    return widget_match(state->hot, id);
}

inline b32
is_hover(UI_State *state, Widget_ID id){
    return widget_match(state->hover, id);
}

struct UI_Layout{
    i32 row_count;
    i32 row_item_width;
    i32 row_max_item_height;

    i32_Rect rect;
    i32 x, y;    
};

struct UI_Layout_Restore{
    UI_Layout layout;
    UI_Layout *dest;
};

inline void
begin_layout(UI_Layout *layout, i32_Rect rect){
    layout->rect = rect;
    layout->x = rect.x0;
    layout->y = rect.y0;
    layout->row_count = 0;
    layout->row_max_item_height = 0;
}

inline void
begin_row(UI_Layout *layout, i32 count){
    layout->row_count = count;
    layout->row_item_width = (layout->rect.x1 - layout->x) / count; 
}

inline i32_Rect
layout_rect(UI_Layout *layout, i32 height){
    i32_Rect rect;
    rect.x0 = layout->x;
    rect.y0 = layout->y;
    rect.x1 = rect.x0;
    rect.y1 = rect.y0 + height;
    if (layout->row_count > 0){
        --layout->row_count;
        rect.x1 = rect.x0 + layout->row_item_width;
        layout->x += layout->row_item_width;
        layout->row_max_item_height = Max(height, layout->row_max_item_height);
    }
    if (layout->row_count == 0){
        rect.x1 = layout->rect.x1;
        layout->row_max_item_height = Max(height, layout->row_max_item_height);
        layout->y += layout->row_max_item_height;
        layout->x = layout->rect.x0;
        layout->row_max_item_height = 0;
    }
    return rect;
}

inline UI_Layout_Restore
begin_sub_layout(UI_Layout *layout, i32_Rect area){
    UI_Layout_Restore restore;
    restore.layout = *layout;
    restore.dest = layout;
    begin_layout(layout, area);
    return restore;
}

inline void
end_sub_layout(UI_Layout_Restore restore){
    *restore.dest = restore.layout;
}

struct UI_Style{
    u32 dark, dim, bright;
    u32 base, pop1, pop2;
};

internal UI_Style
get_ui_style(Style *style){
    UI_Style ui_style;
    ui_style.dark = style->main.back_color;
    ui_style.dim = style->main.margin_color;
    ui_style.bright = style->main.margin_active_color;
    ui_style.base = style->main.default_color;
    ui_style.pop1 = style->main.file_info_style.pop1_color;
    ui_style.pop2 = style->main.file_info_style.pop2_color;
    return ui_style;
}

internal UI_Style
get_ui_style_upper(Style *style){
    UI_Style ui_style;
    ui_style.dark = style->main.margin_color;
    ui_style.dim = style->main.margin_hover_color;
    ui_style.bright = style->main.margin_active_color;
    ui_style.base = style->main.default_color;
    ui_style.pop1 = style->main.file_info_style.pop1_color;
    ui_style.pop2 = style->main.file_info_style.pop2_color;
    return ui_style;
}

inline void
get_colors(UI_State *state, u32 *back, u32 *fore, Widget_ID wid, UI_Style style){
    b32 hover = is_hover(state, wid);
    b32 hot = is_hot(state, wid);
    i32 level = hot + hover;
    switch (level){
        case 2:
        *back = style.bright;
        *fore = style.dark;
        break;
        case 1:
        *back = style.dim;
        *fore = style.bright;
        break;
        case 0:
        *back = style.dark;
        *fore = style.bright;
        break;
    }
}

inline void
get_pop_color(UI_State *state, u32 *pop, Widget_ID wid, UI_Style style){
    b32 hover = is_hover(state, wid);
    b32 hot = is_hot(state, wid);
    i32 level = hot + hover;
    switch (level){
        case 2:
        *pop = style.pop1;
        break;
        case 1:
        *pop = style.pop1;
        break;
        case 0:
        *pop = style.pop1;
        break;
    }
}

internal UI_State
ui_state_init(UI_State *state_in, Render_Target *target, Input_Summary *user_input,
    Style *style, i16 font_id, Font_Set *font_set, Working_Set *working_set, b32 input_stage){

    UI_State state = {};
    state.target = target;
    state.style = style;
    state.font_set = font_set;
    state.font_id = font_id;
    state.working_set = working_set;
    if (user_input){
        state.mouse = &user_input->mouse;
        state.keys = &user_input->keys;
    }
    state.selected = state_in->selected;
    state.hot = state_in->hot;
    if (input_stage) state.hover = {};
    else state.hover = state_in->hover;
    state.redraw = 0;
    state.activate_me = 0;
    state.input_stage = input_stage;
    state.height = state_in->height;
    state.view_y = state_in->view_y;
    return state;
}

inline b32
ui_state_match(UI_State a, UI_State b){
    return (widget_match(a.selected, b.selected) &&
            widget_match(a.hot, b.hot) &&
            widget_match(a.hover, b.hover));
}

internal b32
ui_finish_frame(UI_State *persist_state, UI_State *state, UI_Layout *layout, i32_Rect rect,
    b32 do_wheel, b32 *did_activation){
    b32 result = 0;
    f32 h = layout->y + persist_state->view_y - rect.y0;
    f32 max_y = h - (rect.y1 - rect.y0);

    persist_state->height = h;
    persist_state->view_y = state->view_y;

    if (state->input_stage){
        Mouse_State *mouse = state->mouse;
        Font_Set *font_set = state->font_set;

        if (mouse->wheel != 0 && do_wheel){
            i32 height = get_font_info(font_set, state->font_id)->height;
            persist_state->view_y += mouse->wheel*height;
            result = 1;
        }
        if (mouse->release_l && widget_match(state->hot, state->hover)){
            if (did_activation) *did_activation = 1;
            if (state->activate_me){
                state->selected = state->hot;
            }
        }
        if (!mouse->l && !mouse->r){
            state->hot = {};
        }

        if (!ui_state_match(*persist_state, *state) || state->redraw){
            result = 1;
        }

        *persist_state = *state;
    }

    if (persist_state->view_y >= max_y) persist_state->view_y = max_y;
    if (persist_state->view_y < 0) persist_state->view_y = 0;

    return result;
}

internal b32
ui_do_button_input(UI_State *state, i32_Rect rect, Widget_ID id, bool32 activate, bool32 *right = 0){
    b32 result = 0;
    Mouse_State *mouse = state->mouse;
    b32 hover = hit_check(mouse->x, mouse->y, rect);
    if (hover){
        state->hover = id;
        if (activate) state->activate_me = 1;
        if (mouse->press_l || (mouse->press_r && right)) state->hot = id;
        if (mouse->l && mouse->r) state->hot = {};
    }
    bool32 is_match = is_hot(state, id);
    if (mouse->release_l && is_match){
        if (hover) result = 1;
        state->redraw = 1;
    }
    if (right && mouse->release_r && is_match){
        if (hover) *right = 1;
        state->redraw = 1;
    }
    return result;
}

internal bool32
ui_do_subdivided_button_input(UI_State *state, i32_Rect rect, i32 parts, Widget_ID id, bool32 activate, i32 *indx_out, bool32 *right = 0){
    bool32 result = 0;
    real32 x0, x1;
    i32_Rect sub_rect;
    Widget_ID sub_widg = id;
    real32 sub_width = (rect.x1 - rect.x0) / (real32)parts;
    sub_rect.y0 = rect.y0;
    sub_rect.y1 = rect.y1;
    x1 = (real32)rect.x0;

    for (i32 i = 0; i < parts; ++i){
        x0 = x1;
        x1 = x1 + sub_width;
        sub_rect.x0 = TRUNC32(x0);
        sub_rect.x1 = TRUNC32(x1);
        sub_widg.sub_id2 = i;
        if (ui_do_button_input(state, sub_rect, sub_widg, activate, right)){
            *indx_out = i;
            break;
        }
    }

    return result;
}

internal real32
ui_do_vscroll_input(UI_State *state, i32_Rect top, i32_Rect bottom, i32_Rect slider,
    Widget_ID id, real32 val, real32 step_amount,
    real32 smin, real32 smax, real32 vmin, real32 vmax){
    Mouse_State *mouse = state->mouse;
    i32 mx = mouse->x;
    i32 my = mouse->y;
    if (hit_check(mx, my, top)){
        state->hover = id;
        state->hover.sub_id2 = 1;
    }
    if (hit_check(mx, my, bottom)){
        state->hover = id;
        state->hover.sub_id2 = 2;
    }
    if (hit_check(mx, my, slider)){
        state->hover = id;
        state->hover.sub_id2 = 3;
    }
    if (mouse->press_l) state->hot = state->hover;
    if (id.id == state->hot.id){
        if (mouse->release_l){
            Widget_ID wid1, wid2;
            wid1 = wid2 = id;
            wid1.sub_id2 = 1;
            wid2.sub_id2 = 2;
            if (state->hot.sub_id2 == 1 && is_hover(state, wid1)) val -= step_amount;
            if (state->hot.sub_id2 == 2 && is_hover(state, wid2)) val += step_amount;
            state->redraw = 1;
        }
        if (state->hot.sub_id2 == 3){
            f32 S, L;
            S = (f32)mouse->y - (slider.y1 - slider.y0) / 2;
            if (S < smin) S = smin;
            if (S > smax) S = smax;
            L = unlerp(smin, S, smax);
            val = lerp(vmin, L, vmax);
            state->redraw = 1;
        }
    }
    return val;
}

internal b32
ui_do_text_field_input(UI_State *state, String *str){
    b32 result = 0;
    Key_Summary *keys = state->keys;
    for (i32 key_i = 0; key_i < keys->count; ++key_i){
        Key_Event_Data key = get_single_key(keys, key_i);
        char c = (char)key.character;
        if (char_is_basic(c) && str->size < str->memory_size-1){
            str->str[str->size++] = c;
            str->str[str->size] = 0;
        }
        else if (c == '\n'){
            result = 1;
        }
        else if (key.keycode == key_back && str->size > 0){
            str->str[--str->size] = 0;
        }
    }
    return result;
}

internal b32
ui_do_file_field_input(System_Functions *system, UI_State *state,
    Hot_Directory *hot_dir, b32 try_to_match, b32 case_sensitive){
    Key_Event_Data key;
    Single_Line_Input_Step step;
    String *str = &hot_dir->string;
    Key_Summary *keys = state->keys;
    i32 key_i;
    b32 result = 0;

    terminate_with_null(str);

    for (key_i = 0; key_i < keys->count; ++key_i){
        key = get_single_key(keys, key_i);
        step =
            app_single_file_input_step(system, state->working_set, key, str,
            hot_dir, 1, try_to_match, case_sensitive);
        if ((step.hit_newline || step.hit_ctrl_newline) && !step.no_file_match) result = 1;
    }
    return result;
}

internal b32
ui_do_line_field_input(System_Functions *system,
    UI_State *state, String *string){
    b32 result = 0;
    Key_Summary *keys = state->keys;
    for (i32 key_i = 0; key_i < keys->count; ++key_i){
        Key_Event_Data key = get_single_key(keys, key_i);
        terminate_with_null(string);
        Single_Line_Input_Step step =
            app_single_line_input_step(system, key, string);
        if (step.hit_newline || step.hit_ctrl_newline) result = 1;
    }
    return result;
}

internal b32
ui_do_slider_input(UI_State *state, i32_Rect rect, Widget_ID wid,
    real32 min, real32 max, real32 *v){
    b32 result = 0;
    ui_do_button_input(state, rect, wid, 0);
    Mouse_State *mouse = state->mouse;
    if (is_hot(state, wid)){
        result = 1;
        *v = unlerp(min, (f32)mouse->x, max);
        state->redraw = 1;
    }
    return result;
}

internal bool32
do_text_field(Widget_ID wid, UI_State *state, UI_Layout *layout, String prompt, String dest){
    b32 result = 0;
    i32 character_h = get_font_info(state->font_set, state->font_id)->height;

    i32_Rect rect = layout_rect(layout, character_h);

    if (state->input_stage){
        ui_do_button_input(state, rect, wid, 1);
        if (is_selected(state, wid)){
            if (ui_do_text_field_input(state, &dest)){
                result = 1;
            }
        }
    }
    else{
        Render_Target *target = state->target;
        UI_Style ui_style = get_ui_style_upper(state->style);
        u32 back, fore, prompt_pop;
        get_colors(state, &back, &fore, wid, ui_style);
        get_pop_color(state, &prompt_pop, wid, ui_style);

        draw_rectangle(target, rect, back);

        i32 x = draw_string(target, state->font_id, prompt, rect.x0, rect.y0 + 1, prompt_pop);
        draw_string(target, state->font_id, dest, x, rect.y0 + 1, ui_style.base);
    }

    return result;
}

internal b32
do_button(i32 id, UI_State *state, UI_Layout *layout, char *text, i32 height_mult,
    b32 is_toggle = 0, b32 on = 0){
    b32 result = 0;
    i16 font_id = state->font_id;
    i32 character_h = get_font_info(state->font_set, font_id)->height;

    i32_Rect btn_rect = layout_rect(layout, character_h * height_mult);
    if (height_mult > 1) btn_rect = get_inner_rect(btn_rect, 2);
    else{
        btn_rect.x0 += 2;
        btn_rect.x1 -= 2;
    }

    Widget_ID wid = make_id(state, id);

    if (state->input_stage){
        if (ui_do_button_input(state, btn_rect, wid, 0)){
            result = 1;
        }
    }
    else{
        Render_Target *target = state->target;
        UI_Style ui_style = get_ui_style(state->style);
        u32 back, fore, outline;
        outline = ui_style.bright;
        get_colors(state, &back, &fore, wid, ui_style);

        draw_rectangle(target, btn_rect, back);
        draw_rectangle_outline(target, btn_rect, outline);
        real32 text_width = font_string_width(target, font_id, text);
        i32 box_width = btn_rect.x1 - btn_rect.x0;
        i32 box_height = btn_rect.y1 - btn_rect.y0;
        i32 x_pos = TRUNC32(btn_rect.x0 + (box_width - text_width)*.5f);
        draw_string(target, font_id, text, x_pos, btn_rect.y0 + (box_height - character_h) / 2, fore);

        if (is_toggle){
            i32_Rect on_box = get_inner_rect(btn_rect, character_h/2);
            on_box.x1 = on_box.x0 + (on_box.y1 - on_box.y0);

            if (on) draw_rectangle(target, on_box, fore);
            else draw_rectangle(target, on_box, back);
            draw_rectangle_outline(target, on_box, fore);
        }
    }

    return result;
}

internal b32
do_undo_slider(Widget_ID wid, UI_State *state, UI_Layout *layout, i32 max, i32 v, Undo_Data *undo, i32 *out){
    b32 result = 0;
    i16 font_id = state->font_id;
    i32 character_h = get_font_info(state->font_set, font_id)->height;

    i32_Rect containing_rect = layout_rect(layout, character_h);

    i32_Rect click_rect;
    click_rect.x0 = containing_rect.x0 + character_h - 1;
    click_rect.x1 = containing_rect.x1 - character_h + 1;
    click_rect.y0 = containing_rect.y0 + 2;
    click_rect.y1 = containing_rect.y1 - 2;

    if (state->input_stage){
        real32 l;
        if (ui_do_slider_input(state, click_rect, wid, (real32)click_rect.x0, (real32)click_rect.x1, &l)){
            real32 v_new = lerp(0.f, l, (real32)max);
            v = ROUND32(v_new);
            result = 1;
            if (out) *out = v;
        }
    }
    else{
        Render_Target *target = state->target;
        if (max > 0){
            UI_Style ui_style = get_ui_style_upper(state->style);

            real32 L = unlerp(0.f, (real32)v, (real32)max);
            i32 x = FLOOR32(lerp((real32)click_rect.x0, L, (real32)click_rect.x1));

            i32 bar_top = ((click_rect.y0 + click_rect.y1) >> 1) - 1;
            i32 bar_bottom = bar_top + 2;

            bool32 show_bar = 1;
            real32 tick_step = (click_rect.x1 - click_rect.x0) / (real32)max;
            bool32 show_ticks = 1;
            if (tick_step <= 5.f) show_ticks = 0;

            if (undo == 0){
                if (show_bar){
                    i32_Rect slider_rect;
                    slider_rect.x0 = click_rect.x0;
                    slider_rect.x1 = x;
                    slider_rect.y0 = bar_top;
                    slider_rect.y1 = bar_bottom;

                    draw_rectangle(target, slider_rect, ui_style.dim);

                    slider_rect.x0 = x;
                    slider_rect.x1 = click_rect.x1;
                    draw_rectangle(target, slider_rect, ui_style.pop1);
                }

                if (show_ticks){
                    f32_Rect tick;
                    tick.x0 = (real32)click_rect.x0 - 1;
                    tick.x1 = (real32)click_rect.x0 + 1;
                    tick.y0 = (real32)bar_top - 3;
                    tick.y1 = (real32)bar_bottom + 3;

                    for (i32 i = 0; i < v; ++i){
                        draw_rectangle(target, tick, ui_style.dim);
                        tick.x0 += tick_step;
                        tick.x1 += tick_step;
                    }

                    for (i32 i = v; i <= max; ++i){
                        draw_rectangle(target, tick, ui_style.pop1);
                        tick.x0 += tick_step;
                        tick.x1 += tick_step;
                    }
                }
            }
            else{
                if (show_bar){
                    i32_Rect slider_rect;
                    slider_rect.x0 = click_rect.x0;
                    slider_rect.y0 = bar_top;
                    slider_rect.y1 = bar_bottom;

                    Edit_Step *history = undo->history.edits;
                    i32 block_count = undo->history_block_count;
                    Edit_Step *step = history;
                    for (i32 i = 0; i < block_count; ++i){
                        u32 color;
                        if (step->type == ED_REDO ||
                                step->type == ED_UNDO) color = ui_style.pop1;
                        else color = ui_style.dim;

                        real32 L;
                        if (i + 1 == block_count){
                            L = 1.f;
                        }else{
                            step = history + step->next_block;
                            L = unlerp(0.f, (real32)(step - history), (real32)max);
                        }
                        if (L > 1.f) L = 1.f;
                        i32 x = FLOOR32(lerp((real32)click_rect.x0, L, (real32)click_rect.x1));

                        slider_rect.x1 = x;
                        draw_rectangle(target, slider_rect, color);
                        slider_rect.x0 = slider_rect.x1;

                        if (L == 1.f) break;
                    }
                }

                if (show_ticks){
                    f32_Rect tick;
                    tick.x0 = (real32)click_rect.x0 - 1;
                    tick.x1 = (real32)click_rect.x0 + 1;
                    tick.y0 = (real32)bar_top - 3;
                    tick.y1 = (real32)bar_bottom + 3;

                    Edit_Step *history = undo->history.edits;
                    u32 color = ui_style.dim;
                    for (i32 i = 0; i <= max; ++i){
                        if (i != max){
                            if (history[i].type == ED_REDO) color = ui_style.pop1;
                            else if (history[i].type == ED_UNDO ||
                                    history[i].type == ED_NORMAL) color = ui_style.pop2;
                            else color = ui_style.dim;
                        }
                        draw_rectangle(target, tick, color);
                        tick.x0 += tick_step;
                        tick.x1 += tick_step;
                    }
                }
            }

            i32_Rect slider_handle;
            slider_handle.x0 = x - 2;
            slider_handle.x1 = x + 2;
            slider_handle.y0 = click_rect.y0;
            slider_handle.y1 = click_rect.y1;

            draw_rectangle(target, slider_handle, ui_style.bright);
        }
    }

    return result;
}

internal void
do_label(UI_State *state, UI_Layout *layout, char *text, int text_size, f32 height = 2.f){
    Style *style = state->style;
    i16 font_id = state->font_id;
    i32 line_height = get_font_info(state->font_set, font_id)->height;
    i32_Rect label = layout_rect(layout, FLOOR32(line_height * height));

    if (!state->input_stage){
        Render_Target *target = state->target;
        u32 back = style->main.margin_color;
        u32 fore = style->main.default_color;
        draw_rectangle(target, label, back);
        i32 height = label.y1 - label.y0;

        String textstr = make_string(text, text_size);
        draw_string(target, font_id, textstr, label.x0,
            label.y0 + (height - line_height)/2, fore);
    }
}

inline void
do_label(UI_State *state, UI_Layout *layout, String text, f32 height = 2.f){
    do_label(state, layout, text.str, text.size, height);
}

internal void
do_scroll_bar(UI_State *state, i32_Rect rect){
    i32 id = 1;
    i32 w = (rect.x1 - rect.x0);
    i32 h = (rect.y1 - rect.y0);

    i32_Rect top_arrow, bottom_arrow;
    top_arrow.x0 = rect.x0;
    top_arrow.x1 = rect.x1;
    top_arrow.y0 = rect.y0;
    top_arrow.y1 = top_arrow.y0 + w;

    bottom_arrow.x0 = rect.x0;
    bottom_arrow.x1 = rect.x1;
    bottom_arrow.y1 = rect.y1;
    bottom_arrow.y0 = bottom_arrow.y1 - w;

    f32 space_h = (f32)(bottom_arrow.y0 - top_arrow.y1);
    if (space_h <= w) return;

    i32 slider_h = w;

    f32 view_hmin = 0;
    f32 view_hmax = state->height - h;
    f32 L = unlerp(view_hmin, state->view_y, view_hmax);

    f32 slider_hmin = (f32)top_arrow.y1;
    f32 slider_hmax = (f32)bottom_arrow.y0 - slider_h;
    f32 S = lerp(slider_hmin, L, slider_hmax);

    i32_Rect slider;
    slider.x0 = rect.x0;
    slider.x1 = rect.x1;
    slider.y0 = FLOOR32(S);
    slider.y1 = slider.y0 + slider_h;

    Widget_ID wid = make_id(state, id);

    if (state->input_stage){
        state->view_y = 
            ui_do_vscroll_input(state, top_arrow, bottom_arrow, slider, wid, state->view_y,
            (f32)(get_font_info(state->font_set, state->font_id)->height),
            slider_hmin, slider_hmax, view_hmin, view_hmax);
    }
    else{    
        Render_Target *target = state->target;

        f32 x0, y0, x1, y1, x2, y2;
        f32 w_1_2 = w*.5f;
        f32 w_1_3 = w*.333333f;
        f32 w_2_3 = w*.666667f;


        UI_Style ui_style = get_ui_style(state->style);
        u32 outline, back, fore;

        outline = ui_style.bright;

        wid.sub_id2 = 0;

        x0 = (w_1_2 + top_arrow.x0);
        x1 = (w_1_3 + top_arrow.x0);
        x2 = (w_2_3 + top_arrow.x0);

        ++wid.sub_id2;
        y0 = (w_1_3 + top_arrow.y0);
        y1 = (w_2_3 + top_arrow.y0);
        y2 = (w_2_3 + top_arrow.y0);
        get_colors(state, &back, &fore, wid, ui_style);
        draw_rectangle(target, top_arrow, back);
        draw_rectangle_outline(target, top_arrow, outline);

        ++wid.sub_id2;
        y0 = (w_2_3 + bottom_arrow.y0);
        y1 = (w_1_3 + bottom_arrow.y0);
        y2 = (w_1_3 + bottom_arrow.y0);
        get_colors(state, &back, &fore, wid, ui_style);
        draw_rectangle(target, bottom_arrow, back);
        draw_rectangle_outline(target, bottom_arrow, outline);

        ++wid.sub_id2;
        get_colors(state, &back, &fore, wid, ui_style);
        draw_rectangle(target, slider, back);
        draw_rectangle_outline(target, slider, outline);

        draw_rectangle_outline(target, rect, outline);
    }    
}

internal void
draw_gradient_slider(Render_Target *target, Vec4 base, i32 channel,
    i32 steps, f32 top, f32_Rect slider, b32 hsla){
    Vec4 low, high;
    f32 *lowv, *highv;
    f32 x;
    f32 next_x;
    f32 x_step;
    f32 v_step;
    f32 m;

    x = (real32)slider.x0;
    x_step = (real32)(slider.x1 - slider.x0) / steps;
    v_step = top / steps;
    m = 1.f / top;
    lowv = &low.v[channel];
    highv = &high.v[channel];

    if (hsla){
        for (i32 i = 0; i < steps; ++i){
            low = high = base;
            *lowv = (i * v_step);
            *highv = *lowv + v_step;
            *lowv *= m;
            *highv *= m;
            low = hsla_to_rgba(low);
            high = hsla_to_rgba(high);

            next_x = x + x_step;
            draw_gradient_2corner_clipped(
                target, x, slider.y0, next_x, slider.y1,
                low, high);
            x = next_x;
        }
    }
    else{
        for (i32 i = 0; i < steps; ++i){
            low = high = base;
            *lowv = (i * v_step);
            *highv = *lowv + v_step;
            *lowv *= m;
            *highv *= m;

            next_x = x + x_step;
            draw_gradient_2corner_clipped(
                target, x, slider.y0, next_x, slider.y1,
                low, high);
            x = next_x;
        }
    }
}

inline void
draw_hsl_slider(Render_Target *target, Vec4 base, i32 channel,
    i32 steps, f32 top, f32_Rect slider){
    draw_gradient_slider(target, base, channel, steps, top, slider, 1);
}

inline void
draw_rgb_slider(Render_Target *target, Vec4 base, i32 channel,
    i32 steps, f32 top, f32_Rect slider){
    draw_gradient_slider(target, base, channel, steps, top, slider, 0);
}

internal b32
do_main_file_box(System_Functions *system, UI_State *state, UI_Layout *layout,
    Hot_Directory *hot_directory, b32 try_to_match, b32 case_sensitive, char *end){
    b32 result = 0;
    Style *style = state->style;
    String *string = &hot_directory->string;

    i16 font_id = state->font_id;
    i32 line_height = get_font_info(state->font_set, font_id)->height;
    i32_Rect box = layout_rect(layout, line_height + 2);

    if (state->input_stage){
        if (ui_do_file_field_input(system, state, hot_directory, try_to_match, case_sensitive)){
            result = 1;
        }
    }
    else{
        Render_Target *target = state->target;
        u32 back = style->main.margin_color;
        u32 fore = style->main.default_color;
        u32 special = style->main.special_character_color;
        draw_rectangle(target, box, back);
        i32 x = box.x0;
        x = draw_string(target, font_id, string->str, x, box.y0, fore);
        if (end) draw_string(target, font_id, end, x, box.y0, special);
    }

    layout->y = box.y1;
    return result;
}

internal b32
do_main_string_box(System_Functions *system, UI_State *state, UI_Layout *layout, String *string){
    b32 result = 0;
    Style *style = state->style;

    i16 font_id = state->font_id;
    i32 line_height = get_font_info(state->font_set, font_id)->height;
    i32_Rect box = layout_rect(layout, line_height + 2);

    if (state->input_stage){
        if (ui_do_line_field_input(system, state, string)){
            result = 1;
        }
    }
    else{
        Render_Target *target = state->target;
        u32 back = style->main.margin_color;
        u32 fore = style->main.default_color;
        draw_rectangle(target, box, back);
        i32 x = box.x0;
        x = draw_string(target, font_id, string->str, x, box.y0, fore);
    }

    layout->y = box.y1;
    return result;
}

internal b32
do_list_option(i32 id, UI_State *state, UI_Layout *layout, String text){
    b32 result = 0;
    Style *style = state->style;

    i16 font_id = state->font_id;
    i32 character_h = get_font_info(state->font_set, font_id)->height;

    i32_Rect box = layout_rect(layout, character_h*2);
    Widget_ID wid = make_id(state, id);

    if (state->input_stage){
        if (ui_do_button_input(state, box, wid, 0)){
            result = 1;
        }
    }
    else{
        Render_Target *target = state->target;
        i32_Rect inner = get_inner_rect(box, 3);
        u32 back, outline, fore, pop;
        back = style->main.back_color;
        fore = style->main.default_color;
        pop = style->main.file_info_style.pop2_color;
        if (is_hover(state, wid)) outline = style->main.margin_active_color;
        else outline = style->main.margin_color;

        draw_rectangle(target, inner, back);
        i32 x = inner.x0, y = box.y0 + character_h/2;
        x = draw_string(target, font_id, text, x, y, fore);
        draw_margin(target, box, inner, outline);
    }

    layout->y = box.y1;
    return result;
}

internal b32
do_checkbox_list_option(i32 id, UI_State *state, UI_Layout *layout, String text, b32 is_on){
    b32 result = 0;
    Style *style = state->style;

    i16 font_id = state->font_id;
    i32 character_h = get_font_info(state->font_set, font_id)->height;

    i32_Rect box = layout_rect(layout, character_h*2);
    Widget_ID wid = make_id(state, id);

    if (state->input_stage){
        if (ui_do_button_input(state, box, wid, 0)){
            result = 1;
        }
    }
    else{
        Render_Target *target = state->target;
        i32_Rect inner = get_inner_rect(box, 3);
        u32 back, outline, fore, pop, box_color;
        back = style->main.back_color;
        fore = style->main.default_color;
        pop = style->main.file_info_style.pop2_color;
        if (is_hover(state, wid)) outline = style->main.margin_active_color;
        else outline = style->main.margin_color;
        box_color = style->main.margin_active_color;

        draw_rectangle(target, inner, back);

        i32_Rect square;
        square = get_inner_rect(inner, character_h/3);
        square.x1 = square.x0 + (square.y1 - square.y0);
        if (is_on) draw_rectangle(target, square, box_color);
        else draw_margin(target, square, 1, box_color);

        i32 x = square.x1 + 3;
        i32 y = box.y0 + character_h/2;
        x = draw_string(target, font_id, text, x, y, fore);
        draw_margin(target, box, inner, outline);
    }

    layout->y = box.y1;
    return result;
}


internal b32
do_file_option(i32 id, UI_State *state, UI_Layout *layout, String filename, b32 is_folder, String extra, char slash){
    b32 result = 0;
    Style *style = state->style;
    i16 font_id = state->font_id;
    i32 character_h = get_font_info(state->font_set, font_id)->height;
    char slash_buf[2] = { slash, 0 };

    i32_Rect box = layout_rect(layout, character_h*2);
    Widget_ID wid = make_id(state, id);

    if (state->input_stage){
        if (ui_do_button_input(state, box, wid, 0)){
            result = 1;
        }
    }
    else{
        Render_Target *target = state->target;
        i32_Rect inner = get_inner_rect(box, 3);
        u32 back, outline, fore, pop;
        back = style->main.back_color;
        fore = style->main.default_color;
        pop = style->main.file_info_style.pop2_color;
        if (is_hover(state, wid)) outline = style->main.margin_active_color;
        else outline = style->main.margin_color;

        draw_rectangle(target, inner, back);
        i32 x = inner.x0, y = box.y0 + character_h/2;
        x = draw_string(target, font_id, filename, x, y, fore);
        if (is_folder) x = draw_string(target, font_id, slash_buf, x, y, fore);
        draw_string(target, font_id, extra, x, y, pop);
        draw_margin(target, box, inner, outline);
    }

    layout->y = box.y1;
    return result;
}

internal b32
do_file_list_box(System_Functions *system, UI_State *state, UI_Layout *layout,
    Hot_Directory *hot_dir, b32 has_filter, b32 try_to_match, b32 case_sensitive,
    b32 *new_dir, b32 *selected, char *end){
    b32 result = 0;
    File_List *files = &hot_dir->file_list;

    if (do_main_file_box(system, state, layout, hot_dir, try_to_match, case_sensitive, end)){
        *selected = 1;
        terminate_with_null(&hot_dir->string);
    }
    else{
        persist String p4c_extension = make_lit_string("p4c");
        persist String message_loaded = make_lit_string(" LOADED");
        persist String message_unsaved = make_lit_string(" LOADED *");
        persist String message_unsynced = make_lit_string(" LOADED !");
        persist String message_nothing = {};

        char front_name_space[256];
        String front_name = make_fixed_width_string(front_name_space);
        get_front_of_directory(&front_name, hot_dir->string);

        Absolutes absolutes;
        get_absolutes(front_name, &absolutes, 1, 1);

        char full_path_[256];
        String full_path = make_fixed_width_string(full_path_);
        get_path_of_directory(&full_path, hot_dir->string);
        i32 restore_size = full_path.size;

        i32 i;
        File_Info *info, *end;
        end = files->infos + files->count;
        for (info = files->infos, i = 0; info != end; ++info, ++i){
            String filename = info->filename;

            append(&full_path, filename);
            terminate_with_null(&full_path);

            Editing_File *file = working_set_contains(system, state->working_set, full_path);
            full_path.size = restore_size;

            b8 is_folder = (info->folder != 0);
            b8 ext_match = (match(file_extension(filename), p4c_extension) != 0);
            b8 name_match = (filename_match(front_name, &absolutes, filename, case_sensitive) != 0);
            b8 is_loaded = (file != 0 && file_is_ready(file));

            String message = message_nothing;
            if (is_loaded){
                switch (buffer_get_sync(file)){
                    case SYNC_GOOD: message = message_loaded; break;
                    case SYNC_BEHIND_OS: message = message_unsynced; break;
                    case SYNC_UNSAVED: message = message_unsaved; break;
                }
            }

            if ((is_folder || !has_filter || ext_match) && name_match){
                if (do_file_option(100+i, state, layout, filename, is_folder, message, system->slash)){
                    result = 1;
                    hot_directory_clean_end(hot_dir);
                    append(&hot_dir->string, filename);
                    if (is_folder){
                        *new_dir = 1;
                        append(&hot_dir->string, system->slash);
                    }
                    else{
                        *selected = 1;
                    }
                    terminate_with_null(&hot_dir->string);
                }
            }
        }
    }

    return result;
}

internal b32
do_live_file_list_box(System_Functions *system, UI_State *state, UI_Layout *layout,
    Working_Set *working_set, String *string, b32 *selected){
    b32 result = 0;
    
    if (do_main_string_box(system, state, layout, string)){
        *selected = 1;
        terminate_with_null(string);
    }
    else{
        persist String message_unsaved = make_lit_string(" *");
        persist String message_unsynced = make_lit_string(" !");
        persist String message_nothing = {};
        
        Absolutes absolutes;
        get_absolutes(*string, &absolutes, 1, 1);
        
        Editing_File *file;
        File_Node *node, *used_nodes;
        i32 i = 0;
        used_nodes = &working_set->used_sentinel;
        
        for (dll_items(node, used_nodes)){
            file = (Editing_File*)node;
            Assert(!file->state.is_dummy);
            
            String message = message_nothing;
            switch (buffer_get_sync(file)){
                case SYNC_BEHIND_OS: message = message_unsynced; break;
                case SYNC_UNSAVED: message = message_unsaved; break;
            }
            
            if (filename_match(*string, &absolutes, file->name.live_name, 1)){
                if (do_file_option(100+i, state, layout, file->name.live_name, 0, message, system->slash)){
                    result = 1;
                    *selected = 1;
                    copy(string, file->name.source_path);
                    terminate_with_null(string);
                }
            }
            
            ++i;
        }
    }
    
    return result;
}

internal Super_Color
super_color_create(u32 packed){
    Super_Color result = {};
    result.rgba = unpack_color4(packed);
    result.hsla = rgba_to_hsla(result.rgba);
    return result;
}

internal void
super_color_post_hsla(Super_Color *color, Vec4 hsla){
    color->hsla = hsla;
    if (hsla.h == 1.f)
        hsla.h = 0.f;
    color->rgba = hsla_to_rgba(hsla);
    *color->out = pack_color4(color->rgba);
}

internal void
super_color_post_rgba(Super_Color *color, Vec4 rgba){
    color->rgba = rgba;
    color->hsla = rgba_to_hsla(rgba);
    *color->out = pack_color4(rgba);
}

internal void
super_color_post_packed(Super_Color *color, u32 packed){
    color->rgba = unpack_color4(packed);
    color->hsla = rgba_to_hsla(color->rgba);
    *color->out = packed;
}

u32 super_color_clear_masks[] = {0xFF00FFFF, 0xFFFF00FF, 0xFFFFFF00};
u32 super_color_shifts[] = {16, 8, 0};

internal u32
super_color_post_byte(Super_Color *color, i32 channel, u8 byte){
    u32 packed = *color->out;
    packed &= super_color_clear_masks[channel];
    packed |= (byte << super_color_shifts[channel]);
    super_color_post_packed(color, packed);
    return packed;
}

struct Color_Highlight{
    i32 ids[4];
};

struct Library_UI{
    UI_State *state;
    UI_Layout *layout;

    Font_Set *fonts;
    
    Style_Library *styles;
    Hot_Directory *hot_directory;
};

struct Color_UI{
    UI_State *state;
    UI_Layout *layout;
    
    Font_Set *fonts;
    Style_Font *global_font;
    
    f32 hex_advance;
    u32 *palette;
    i32 palette_size;
    
    Color_Highlight highlight;
    Super_Color color;
    
    b32 has_hover_color;
    Super_Color hover_color;
};

enum Channel_Field_Type{
    CF_DEC,
    CF_HEX
};

internal void
do_single_slider(i32 sub_id, Color_UI *ui, i32 channel, b32 is_rgba,
                 i32 grad_steps, f32 top, f32_Rect slider, f32 v_handle,
                 i32_Rect rect){
    f32_Rect click_box = slider;
    click_box.y0 -= v_handle;
    
    if (ui->state->input_stage){
        real32 v;
        if (ui_do_slider_input(ui->state, i32R(click_box), make_sub1(ui->state, sub_id), slider.x0, slider.x1, &v)){
            Vec4 new_color;
            if (is_rgba) new_color = ui->color.rgba;
            else new_color = ui->color.hsla;
            new_color.v[channel] = clamp(0.f, v, 1.f);
            if (is_rgba) super_color_post_rgba(&ui->color, new_color);
            else super_color_post_hsla(&ui->color, new_color);
        }
    }
    else{
        Render_Target *target = ui->state->target;
        Vec4 color;
        real32 x;
        if (is_rgba){
            color = ui->color.rgba;
            draw_rgb_slider(target, V4(0,0,0,1.f), channel, 10, 100.f, slider);
        }
        else{
            i32 steps;
            real32 top;
            if (channel == 0){
                steps = 45;
                top = 360.f;
            }
            else{
                steps = 10;
                top = 100.f;
            }
            color = ui->color.hsla;
            draw_hsl_slider(target, color, channel, steps, top, slider);
        }
        
        x = lerp(slider.x0, color.v[channel], slider.x1);
        draw_rectangle(
            target, f32R(x, slider.y0, x + 1, slider.y1), 0xff000000);
        
        draw_rectangle(
            target, f32R(x-2, click_box.y0, x+3, slider.y0-4), 0xff777777);
    }
}

internal void
do_hsl_sliders(Color_UI *ui, i32_Rect rect){
    real32 bar_width = (real32)(rect.x1 - rect.x0 - 20);
    if (bar_width > 45){
        f32_Rect slider;
        real32 y;
        i32 sub_id;
        
        real32 v_full_space = 30.f;
        real32 v_half_space = 15.f;
        real32 v_quarter_space = 12.f;
        real32 v_handle = 9.f;
        
        slider.x0 = rect.x0 + 10.f;
        slider.x1 = slider.x0 + bar_width;
        
        sub_id = 0;
        
        i32 step_count[] = {45, 10, 10};
        real32 tops[] = {360.f, 100.f, 100.f};
        
        y = rect.y0 + v_quarter_space;
        for (i32 i = 0; i < 3; ++i){
            ++sub_id;
            slider.y0 = y;
            slider.y1 = slider.y0 + v_half_space;
            do_single_slider(sub_id, ui, i, 0, step_count[i], tops[i], slider, v_handle, rect);
            y += v_full_space;
        }
    }
}

internal void
fill_buffer_color_channel(char *buffer, u8 x, Channel_Field_Type ftype){
    if (ftype == CF_DEC){
        u8 x0;
        x0 = x / 10;
        buffer[2] = (x - (10*x0)) + '0';
        x = x0;
        x0 = x / 10;
        buffer[1] = (x - (10*x0)) + '0';
        x = x0;
        x0 = x / 10;
        buffer[0] = (x - (10*x0)) + '0';
    }
    else{
        u8 n;
        n = x & 0xF;
        buffer[1] = int_to_hexchar(n);
        x >>= 4;
        n = x & 0xF;
        buffer[0] = int_to_hexchar(n);
    }
}

internal b32
do_channel_field(i32 sub_id, Color_UI *ui, u8 *channel, Channel_Field_Type ftype,
                 i32 y, u32 color, u32 back, i32 x0, i32 x1){
    b32 result = 0;

    i16 font_id = ui->state->font_id;
    i32 line_height = get_font_info(ui->state->font_set, font_id)->height;
    i32_Rect hit_region;
    hit_region.x0 = x0;
    hit_region.x1 = x1;
    hit_region.y0 = y;
    hit_region.y1 = y + line_height;
    
    i32 digit_count;
    if (ftype == CF_DEC) digit_count = 3;
    else digit_count = 2;

    Render_Target *target = ui->state->target;
    
    if (ui->state->input_stage){
        i32 indx;
        ui_do_subdivided_button_input(ui->state, hit_region, digit_count,
            make_sub1(ui->state, sub_id), 1, &indx);
    }
    else{
        if (ui->state->hover.sub_id1 == sub_id && ui->state->selected.sub_id1 != sub_id){
            draw_rectangle(target, hit_region, back);
        }
    }
    
    char string_buffer[4];
    string_buffer[digit_count] = 0;
    fill_buffer_color_channel(string_buffer, *channel, ftype);
    
    if (ui->state->selected.sub_id1 == sub_id){
        i32 indx = ui->state->selected.sub_id2;
        if (ui->state->input_stage){
            Key_Summary *keys = ui->state->keys;
            for (i32 key_i = 0; key_i < keys->count; ++key_i){
                Key_Event_Data key = get_single_key(keys, key_i);
                
                if (key.keycode == key_right){
                    ++indx;
                    if (indx > digit_count-1) indx = 0;
                }
                if (key.keycode == key_left){
                    --indx;
                    if (indx < 0) indx = digit_count-1;
                }
                
                i32 new_value = *channel;
                if (key.keycode == key_up || key.keycode == key_down){
                    i32 place = digit_count-1-indx;
                    i32 base = (ftype == CF_DEC)?10:0x10;
                    i32 step_amount = 1;
                    while (place > 0){
                        step_amount *= base;
                        --place;
                    }
                    if (key.keycode == key_down){
                        step_amount = 0 - step_amount;
                    }
                    new_value += step_amount;
                }
                
                u8 c = (u8)key.character;
                bool32 is_good = (ftype == CF_DEC)?char_is_numeric(c):char_is_hex(c);
                if (is_good){
                    string_buffer[indx] = c;
                    if (ftype == CF_DEC)
                        new_value = str_to_int(make_string(string_buffer, 3));
                    else
                        new_value = hexstr_to_int(make_string(string_buffer, 2));
                    ++indx;
                    if (indx > digit_count-1) indx = 0;
                }
                
                if (c == '\n'){
                    switch (sub_id){
                    case 1: case 2:
                    case 4: case 5:
                        ui->state->sub_id1_change = sub_id + 3; break;
                        
                    case 7: case 8:
                        ui->state->sub_id1_change = sub_id - 6; break;
                    }
                }
                
                if (new_value != *channel){
                    if (new_value > 255){
                        *channel = 255;
                    }
                    else if (new_value < 0){
                        *channel = 0;
                    }
                    else{
                        *channel = (u8)new_value;
                    }
                    fill_buffer_color_channel(string_buffer, *channel, ftype);
                    result = 1;
                }
                ui->state->selected.sub_id2 = indx;
            }
        }
        else{
            f32_Rect r = f32R(hit_region);
            r.x0 += indx*ui->hex_advance+1;
            r.x1 = r.x0+ui->hex_advance+1;
            draw_rectangle(target, r, back);
        }
    }
    
    if (!ui->state->input_stage)
        draw_string_mono(target, font_id, string_buffer,
                         (real32)x0 + 1, (real32)y, ui->hex_advance,
                         color);
    
    return result;
}

internal void
do_rgb_sliders(Color_UI *ui, i32_Rect rect){
    i32 dec_x0, dec_x1;
    dec_x0 = rect.x0 + 10;
    dec_x1 = TRUNC32(dec_x0 + ui->hex_advance*3 + 2);
    
    i32 hex_x0, hex_x1;
    hex_x0 = dec_x1 + 10;
    hex_x1 = TRUNC32(hex_x0 + ui->hex_advance*2 + 2);
    
    rect.x0 = hex_x1;
    real32 bar_width = (real32)(rect.x1 - rect.x0 - 20);
    
    f32_Rect slider;
    f32 y;
    i32 sub_id;
    u8 channel;
    
    real32 v_full_space = 30.f;
    real32 v_half_space = 15.f;
    real32 v_quarter_space = 12.f;
    real32 v_handle = 9.f;
    
    u32 packed_color = *ui->color.out;
    
    y = rect.y0 + v_quarter_space;
    slider.x0 = rect.x0 + 10.f;
    slider.x1 = slider.x0 + bar_width;
    
    sub_id = 0;
    
    persist i32 shifts[3] = { 16, 8, 0 };
    persist u32 fore_colors[3] = { 0xFFFF0000, 0xFF00FF00, 0xFF1919FF };
    persist u32 back_colors[3] = { 0xFF222222, 0xFF222222, 0xFF131313 };
    
    for (i32 i = 0; i < 3; ++i){
        i32 shift = shifts[i];
        u32 fore = fore_colors[i];
        u32 back = back_colors[i];
        
        ++sub_id;
        channel = (packed_color >> shift) & 0xFF;
        if (do_channel_field(sub_id, ui, &channel, CF_DEC,
                             (i32)y, fore, back, dec_x0, dec_x1))
            super_color_post_byte(&ui->color, i, channel);
        
        ++sub_id;
        channel = (packed_color >> shift) & 0xFF;
        if (do_channel_field(sub_id, ui, &channel, CF_HEX,
                             (i32)y, fore, back, hex_x0, hex_x1))
            super_color_post_byte(&ui->color, i, channel);
        
        ++sub_id;
        slider.y0 = y;
        slider.y1 = slider.y0 + v_half_space;
        if (bar_width > 45.f)
            do_single_slider(sub_id, ui, i, 1, 10, 100.f, slider, v_handle, rect);
        y += v_full_space;
    }
}

struct Blob_Layout{
    i32_Rect rect;
    i32 x, y;
    i32 size, space;
};

internal void
begin_layout(Blob_Layout *layout, i32_Rect rect){
    layout->rect = rect;
    layout->x = rect.x0 + 10;
    layout->y = rect.y0;
    layout->size = 20;
    layout->space = 5;
}

internal void
do_blob(Color_UI *ui, Blob_Layout *layout, u32 color, bool32 *set_me, i32 sub_id){
    i32_Rect rect = layout->rect;
    f32_Rect blob;
    blob.x0 = (real32)layout->x;
    blob.y0 = (real32)layout->y;
    blob.x1 = blob.x0 + layout->size;
    blob.y1 = blob.y0 + layout->size;
    
    layout->y += layout->size + layout->space;
    if (layout->y + layout->size + layout->space*2 > rect.y1){
        layout->y = rect.y0;
        layout->x += layout->size + layout->space;
    }
    
    if (ui->state->input_stage){
        bool32 right = 0;
        if (ui_do_button_input(ui->state, i32R(blob), make_sub1(ui->state, sub_id), 0, &right)){
            super_color_post_packed(&ui->color, color);
        }
        else if (right) *set_me = 1;
    }
    else{
        Render_Target *target = ui->state->target;
        draw_rectangle(target, blob, color);
        persist u32 silver = 0xFFa0a0a0;
        draw_rectangle_outline(target, blob, silver);
    }
}

inline void
do_blob(Color_UI *ui, Blob_Layout *layout, u32 *color, bool32 *set_me){
    i32 sub_id = (i32)((char*)color - (char*)ui->state->style);
    do_blob(ui, layout, *color, set_me, sub_id);
}

internal void
do_v_divide(Color_UI *ui, Blob_Layout *layout, i32 width){
    i32_Rect rect = layout->rect;
    if (layout->y > rect.y0){
        layout->x += layout->size + layout->space;
    }
    layout->x += width;
    layout->y = rect.y0;
}

internal void
do_palette(Color_UI *ui, i32_Rect rect){
    Style *style = ui->state->style;
    Blob_Layout layout;
    begin_layout(&layout, rect);
    bool32 set_me;
    
    do_blob(ui, &layout, &style->main.back_color, &set_me);
    do_blob(ui, &layout, &style->main.margin_color, &set_me);
    do_blob(ui, &layout, &style->main.margin_active_color, &set_me);
    
    do_blob(ui, &layout, &style->main.cursor_color, &set_me);
    do_blob(ui, &layout, &style->main.at_cursor_color, &set_me);
    do_blob(ui, &layout, &style->main.mark_color, &set_me);
    
    do_blob(ui, &layout, &style->main.highlight_color, &set_me);
    do_blob(ui, &layout, &style->main.at_highlight_color, &set_me);
    
    do_blob(ui, &layout, &style->main.default_color, &set_me);
    do_blob(ui, &layout, &style->main.comment_color, &set_me);
    do_blob(ui, &layout, &style->main.keyword_color, &set_me);
    do_blob(ui, &layout, &style->main.str_constant_color, &set_me);
    do_blob(ui, &layout, &style->main.char_constant_color, &set_me);
    do_blob(ui, &layout, &style->main.int_constant_color, &set_me);
    do_blob(ui, &layout, &style->main.float_constant_color, &set_me);
    do_blob(ui, &layout, &style->main.bool_constant_color, &set_me);
    do_blob(ui, &layout, &style->main.include_color, &set_me);
    do_blob(ui, &layout, &style->main.preproc_color, &set_me);
    do_blob(ui, &layout, &style->main.special_character_color, &set_me);
    
    do_blob(ui, &layout, &style->main.highlight_junk_color, &set_me);
    do_blob(ui, &layout, &style->main.highlight_white_color, &set_me);
    
    do_blob(ui, &layout, &style->main.paste_color, &set_me);
    
    do_blob(ui, &layout, &style->main.file_info_style.bar_color, &set_me);
    do_blob(ui, &layout, &style->main.file_info_style.base_color, &set_me);
    do_blob(ui, &layout, &style->main.file_info_style.pop1_color, &set_me);
    do_blob(ui, &layout, &style->main.file_info_style.pop2_color, &set_me);
    
    do_v_divide(ui, &layout, 20);
    
    if (!ui->state->input_stage){
        Render_Target *target = ui->state->target;
        draw_string(target, ui->state->font_id, "Global Palette: right click to save color",
                    layout.x, layout.rect.y0, style->main.default_color);
    }
    
    layout.rect.y0 += layout.size + layout.space;
    layout.y = layout.rect.y0;
    i32 palette_size = ui->palette_size + 1000;
    u32 *color = ui->palette;
    for (i32 i = 1000; i < palette_size; ++i, ++color){
        set_me = 0;
        do_blob(ui, &layout, *color, &set_me, i);
        if (set_me){
            *color = *ui->color.out;
            ui->state->redraw = 1;
        }
    }
}

internal void
do_sub_button(i32 id, Color_UI *ui, char *text){
    i16 font_id = ui->state->font_id;
    i32 line_height = get_font_info(ui->state->font_set, font_id)->height;
    i32_Rect rect = layout_rect(ui->layout, line_height + 2);
    
    if (ui->state->input_stage){
        ui_do_button_input(ui->state, rect, make_sub0(ui->state, id), 1);
    }
    else{
        Render_Target *target = ui->state->target;
        
        u32 back_color, text_color;
        text_color = 0xFFDDDDDD;
        if (ui->state->selected.sub_id0 == id){
            back_color = 0xFF444444;
        }
        else if (ui->state->hover.sub_id0 == id){
            back_color = 0xFF222222;
        }
        else{
            back_color = 0xFF111111;
        }
        
        draw_rectangle(target, rect, back_color);
        draw_string(target, font_id, text, rect.x0, rect.y0 + 1,
                    text_color);
    }
}

internal void
do_color_adjuster(Color_UI *ui, u32 *color,
                  u32 text_color, u32 back_color, char *name){
    i32 id = raw_ptr_dif(color, ui->state->style);
    i16 font_id = ui->state->font_id;
    i32 character_h = get_font_info(ui->state->font_set, font_id)->height;
    u32 text = 0, back = 0;
    
    i32_Rect bar = layout_rect(ui->layout, character_h);
    
    if (ui->state->input_stage){
        if (ui_do_button_input(ui->state, bar, make_id(ui->state, id), 1)){
            ui->has_hover_color = 1;
            ui->hover_color = super_color_create(*color);
        }
    }
    
    else{
        Render_Target *target = ui->state->target;
        u32 text_hover = 0xFF101010;
        u32 back_hover = 0xFF999999;
        if (ui->state->selected.id != id && ui->state->hover.id == id){
            text = text_hover;
            back = back_hover;
        }
        else{
            text = text_color;
            back = back_color;
        }
        
        draw_rectangle(target, bar, back);
        i32 end_pos = draw_string(target, font_id, name, bar.x0, bar.y0, text);
        
        real32 x_spacing = ui->hex_advance;
        i32_Rect temp_rect = bar;
        temp_rect.x0 = temp_rect.x1 - CEIL32(x_spacing * 9.f);
        if (temp_rect.x0 >= end_pos + x_spacing){
            u32 n = *color;
            char full_hex_string[] = "0x000000";
            for (i32 i = 7; i >= 2; --i){
                i32 m = (n & 0xF);
                n >>= 4;
                full_hex_string[i] = int_to_hexchar(m);
            }
            draw_string_mono(target, font_id, full_hex_string,
                             (f32)temp_rect.x0, (f32)bar.y0,
                             x_spacing, text);
        }
        
        for (i32 i = 0; i < ArrayCount(ui->highlight.ids); ++i){
            if (ui->highlight.ids[i] == id){
                draw_rectangle_outline(target, f32R(bar), text_color);
                break;
            }
        }
    }
    
    if (ui->state->selected.id == id){
        Render_Target *target = ui->state->target;
        i32_Rect expanded = layout_rect(ui->layout, 115 + (character_h + 2));
        UI_Layout_Restore restore = begin_sub_layout(ui->layout, expanded);
        
        ui->color.out = color;
        
        if (ui->state->input_stage){
            if (ui->state->selected.sub_id0 == 0){
                ui->state->selected.sub_id0 = 1;
            }
        }
        else{
            draw_rectangle(target, expanded, 0xff000000);
        }
        
        begin_row(ui->layout, 3);
        do_sub_button(1, ui, "HSL");
        do_sub_button(2, ui, "RGB");
        do_sub_button(3, ui, "Palette");
        
        i32_Rect sub_rect;
        sub_rect = expanded;
        sub_rect.y0 += 10 + character_h;
        
        switch (ui->state->selected.sub_id0){
        case 1: do_hsl_sliders(ui, sub_rect); break;
        case 2: do_rgb_sliders(ui, sub_rect); break;
        case 3: do_palette(ui, sub_rect); break;
        }
        
        end_sub_layout(restore);
    }
}

internal void
do_style_name(Color_UI *ui){
    i32 id = -3;
    
    i16 font_id = ui->state->font_id;
    i32 line_height = get_font_info(ui->state->font_set, font_id)->height;
    
    i32_Rect srect = layout_rect(ui->layout, line_height);
    
    Widget_ID wid = make_id(ui->state, id);
    b32 selected = is_selected(ui->state, wid);

    if (ui->state->input_stage){
        if (!selected){
            ui_do_button_input(ui->state, srect, wid, 1);
        }
        else{
            Style *style = ui->state->style;
            if (ui_do_text_field_input(ui->state, &style->name)){
                ui->state->selected = {};
            }
        }
    }
    else{
        Render_Target *target = ui->state->target;
        Style *style = ui->state->style;
        u32 back, fore_text, fore_label;
        if (selected){
            back = 0xFF000000;
            fore_label = 0xFF808080;
            fore_text = 0xFFFFFFFF;
        }
        else if (is_hover(ui->state, wid)){
            back = 0xFF999999;
            fore_text = fore_label = 0xFF101010;
        }
        else{
            back = style->main.back_color;
            fore_text = fore_label = style->main.default_color;
        }
        
        draw_rectangle(target, srect, back);
        i32 x = srect.x0;
        x = draw_string(target, font_id, "NAME: ",
                        x, srect.y0, fore_label);
        x = draw_string(target, font_id, style->name.str,
                        x, srect.y0, fore_text);
    }
}

internal b32
do_font_option(Color_UI *ui, i16 font_id){
    b32 result = 0;
    Font_Info *info = get_font_info(ui->state->font_set, font_id);
    
    i32 sub_id = (i32)(i64)(info);
    i32_Rect orect = layout_rect(ui->layout, info->height);
    
    Widget_ID wid = make_sub0(ui->state, sub_id);
    if (ui->state->input_stage){
        if (ui_do_button_input(ui->state, orect, wid, 0)){
            result = 1;
        }
    }
    else{
        Render_Target *target = ui->state->target;
        u32 back, fore;
        if (is_hover(ui->state, wid)){
            back = 0xFF999999;
            fore = 0xFF101010;
        }
        else{
            back = 0xFF000000;
            fore = 0xFFFFFFFF;
        }
        draw_rectangle(target, orect, back);
        i32 x = orect.x0;
        x = draw_string(target, font_id, "->", x, orect.y0, fore);
        draw_string(target, font_id, info->name.str, x, orect.y0, fore);
    }
    
    return result;
}

internal void
do_font_switch(Color_UI *ui){
    i32 id = -2;
    Render_Target *target = ui->state->target;
    Font_Set *font_set = ui->state->font_set;
    
    i16 font_id = ui->state->font_id;
    Font_Info *info = get_font_info(font_set, font_id);
    i32 character_h = info->height;
    
    i32_Rect srect = layout_rect(ui->layout, character_h);
    Widget_ID wid = make_id(ui->state, id);
    
    if (ui->state->input_stage){
        ui_do_button_input(ui->state, srect, wid, 1);
    }
    else{
        Style *style = ui->state->style;
        u32 back, fore;
        if (is_hover(ui->state, wid) && !is_selected(ui->state, wid)){
            back = 0xFF999999;
            fore = 0xFF101010;
        }
        else{
            back = style->main.back_color;
            fore = style->main.default_color;
        }
        draw_rectangle(target, srect, back);
        i32 x = srect.x0;
        x = draw_string(target, font_id, "FONT: ",
                        x, srect.y0, fore);
        x = draw_string(target, font_id, info->name.str,
                        x, srect.y0, fore);
    }
    
    if (is_selected(ui->state, wid)){
        srect = layout_rect(ui->layout, character_h/2);
        if (!ui->state->input_stage)
            draw_rectangle(target, srect, 0xFF000000);
        
        i32 count = font_set->count + 1;
        
        for (i16 i = 1; i < count; ++i){
            if (i == font_id) continue;
            if (do_font_option(ui, i)){
                ui->global_font->font_id = i;
                ui->global_font->font_changed = 1;
            }
        }
        
        srect = layout_rect(ui->layout, character_h/2);
        if (!ui->state->input_stage)
            draw_rectangle(target, srect, 0xFF000000);
    }
}

internal b32
do_style_preview(Library_UI *ui, Style *style, i32 toggle = -1){
    b32 result = 0;
    i32 id;
    if (style == ui->state->style) id = 2;
    else id = raw_ptr_dif(style, ui->styles->styles) + 100;

    i16 font_id = ui->state->font_id;
    Font_Info *info = get_font_info(ui->state->font_set, font_id);
    
    i32_Rect prect = layout_rect(ui->layout, info->height*3 + 6);
    
    Widget_ID wid = make_id(ui->state, id);
    
    if (ui->state->input_stage){
        if (ui_do_button_input(ui->state, prect, wid, 0)){
            result = 1;
        }
    }
    else{
        Render_Target *target = ui->state->target;
        u32 margin_color = style->main.margin_color;
        if (is_hover(ui->state, wid)){
            margin_color = style->main.margin_active_color;
        }
        
        i32_Rect inner;
        if (toggle != -1){
            i32_Rect toggle_box = prect;
            toggle_box.x1 = toggle_box.x0 + info->height*2 + 6;
            prect.x0 = toggle_box.x1;
            
            inner = get_inner_rect(toggle_box, 3);
            draw_margin(target, toggle_box, inner, margin_color); 
            draw_rectangle(target, inner, style->main.back_color);
            
            i32 d;
            d = info->height/2;
            i32_Rect b;
            b.x0 = (inner.x1 + inner.x0)/2 - d;
            b.y0 = (inner.y1 + inner.y0)/2 - d;
            b.x1 = b.x0 + info->height;
            b.y1 = b.y0 + info->height;
            if (toggle) draw_rectangle(target, b, margin_color);
            else draw_rectangle_outline(target, b, margin_color);
        }
        
        inner = get_inner_rect(prect, 3);
        draw_margin(target, prect, inner, margin_color);
        draw_rectangle(target, inner, style->main.back_color);
        
        i32 text_y = inner.y0;
        i32 text_x = inner.x0;
        text_x = draw_string(target, font_id, style->name.str,
                             text_x, text_y, style->main.default_color);
        i32 font_x = (i32)(inner.x1 - font_string_width(target, font_id, info->name.str));
        if (font_x > text_x + 10)
            draw_string(target, font_id, info->name.str,
                        font_x, text_y, style->main.default_color);
        
        text_x = inner.x0;
        text_y += info->height;
        text_x = draw_string(target, font_id, "if ", text_x, text_y,
                             style->main.keyword_color);
        text_x = draw_string(target, font_id, "(x < ", text_x, text_y,
                             style->main.default_color);
        text_x = draw_string(target, font_id, "0", text_x, text_y,
                             style->main.int_constant_color);
        text_x = draw_string(target, font_id, ") { x = ", text_x, text_y,
                             style->main.default_color);
        text_x = draw_string(target, font_id, "0", text_x, text_y,
                             style->main.int_constant_color);
        text_x = draw_string(target, font_id, "; } ", text_x, text_y,
                             style->main.default_color);
        text_x = draw_string(target, font_id, "// comment", text_x, text_y,
                             style->main.comment_color);
        
        text_x = inner.x0;
        text_y += info->height;
        text_x = draw_string(target, font_id, "[] () {}; * -> +-/ <>= ! && || % ^",
                             text_x, text_y, style->main.default_color);
    }
    
    ui->layout->y = prect.y1;
    return result;
}


// BOTTOM

