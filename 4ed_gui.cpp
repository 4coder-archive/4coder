/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.02.2016
 *
 * GUI system for 4coder
 *
 */

// TOP

struct Query_Slot{
    Query_Slot *next;
    Query_Bar *query_bar;
};

struct Query_Set{
    Query_Slot slots[8];
    Query_Slot *free_slot;
    Query_Slot *used_slot;
};

internal void
init_query_set(Query_Set *set){
    Query_Slot *slot = set->slots;
    int i;
    
    set->free_slot = slot;
    set->used_slot = 0;
    for (i = 0; i+1 < ArrayCount(set->slots); ++i, ++slot){
        slot->next = slot + 1;
    }
}

internal Query_Slot*
alloc_query_slot(Query_Set *set){
    Query_Slot *slot = set->free_slot;
    if (slot != 0){
        set->free_slot = slot->next;
        slot->next = set->used_slot;
        set->used_slot = slot;
    }
    return(slot);
}

internal void
free_query_slot(Query_Set *set, Query_Bar *match_bar){
    Query_Slot *slot = 0, *prev = 0;
    
    for (slot = set->used_slot; slot != 0; slot = slot->next){
        if (slot->query_bar == match_bar) break;
        prev = slot;
    }

    if (slot){
        if (prev){
            prev->next = slot->next;
        }
        else{
            set->used_slot = slot->next;
        }
        slot->next = set->free_slot;
        set->free_slot = slot;
    }
}

#if 0
enum GUI_Piece_Type{
    gui_type_text_input,
    gui_type_number_input,
    gui_type_label,
    gui_type_slider
};

struct GUI_Piece_Header{
    i32 type;
    i32 padding;
};

// TODO(allen): Inline string for prompt?
struct GUI_Piece_Text_Input{
    String *dest;
    f32_Rect rect;
    String prompt;
};

struct GUI_Piece_Number_Input{
    i32 *dest;
    f32_Rect rect;
    String prompt;
};

struct GUI_Piece_Label{
    f32_Rect rect;
    String text;
};

struct GUI_Piece_Slider{
    i32 *dest;
    f32_Rect rect;
    i32 max;
};

struct GUI_Layout_Engine{
    i32_Rect region;
    i32 x, y;
};

struct GUI_Target{
    Partition push_buffer;
    GUI_Layout_Engine layout;
};

internal void
refresh_gui(GUI_Target *target, i32_Rect region){
    target->push_buffer.pos = 0;
    target->layout.region = region;
    target->layout.x = 0;
    target->layout.y = 0;
}

internal void
push_gui_item(GUI_Target *target, GUI_Piece_Header header, void *item, i32 size){
    GUI_Piece_Header *ptr;
    i32 total_size;
    
    Assert(sizeof(header) == 8);
    
    total_size = sizeof(header) + size;
    total_size = ((total_size + 7) & ~7);
    
    ptr = (GUI_Piece_Header*)push_block(&target->push_buffer, size);
    if (ptr){
        *ptr = header;
        memcpy(ptr + 1, item, size);
    }
    else{
        Assert(!"bad situation");
    }
}

internal void
push_gui_text_in(GUI_Target *target, String prompt, String *dest){
    GUI_Piece_Header header = {};
    GUI_Piece_Text_Input item = {};
    
    header.type = gui_type_text_input;
    item.dest = dest;
    item.rect = gui_layout(target); // ?? what do we need here?
    item.prompt = prompt;
    
    push_gui_item(target, header, &item, sizeof(item));
}

internal void
push_gui_number_in(GUI_Target *target, String prompt, i32 *dest){
    GUI_Piece_Header header = {};
    GUI_Piece_Number_Input item = {};
    
    header.type = gui_type_number_input;
    item.dest = dest;
    item.rect = gui_layout(target); // ?? what do we need here?
    item.prompt = prompt;
    
    push_gui_item(target, header, &item, sizeof(item));
}

internal void
push_gui_label(GUI_Target *target, String text){
    GUI_Piece_Header header = {};
    GUI_Piece_Label item = {};
    
    header.type = gui_type_label;
    item.rect = gui_layout(target); // ?? what do we need here?
    item.text = text;
    
    push_gui_item(target, header, &item, sizeof(item));    
}
#endif

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
    
    real32 height, view_y;
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
    bool32 hover = is_hover(state, wid);
    bool32 hot = is_hot(state, wid);
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
    bool32 hover = is_hover(state, wid);
    bool32 hot = is_hot(state, wid);
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
              Style *style, Font_Set *font_set, Working_Set *working_set, b32 input_stage){
    UI_State state = {};
    state.target = target;
    state.style = style;
    state.font_set = font_set;
    state.font_id = style->font_id;
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
    i16 font_id = state->style->font_id;
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
    i16 font_id = state->style->font_id;
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



// BOTTOM

