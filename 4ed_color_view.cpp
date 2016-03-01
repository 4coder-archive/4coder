/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.08.2015
 *
 * Color customizing view for 4coder
 *
 */

// TOP

enum Color_View_Mode{
    CV_MODE_LIBRARY,
    CV_MODE_IMPORT_FILE,
    CV_MODE_EXPORT_FILE,
    CV_MODE_IMPORT,
    CV_MODE_EXPORT,
    CV_MODE_IMPORT_WAIT,
    CV_MODE_ADJUSTING
};

struct Super_Color{
    Vec4 hsla;
    Vec4 rgba;
    u32 *out;
};

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
    UI_State state;
    UI_Layout layout;

    Font_Set *fonts;
    
    Style_Library *styles;
    Hot_Directory *hot_directory;
};

struct Color_UI{
    UI_State state;
    UI_Layout layout;
    
    Font_Set *fonts;
    
    real32 hex_advance;
    u32 *palette;
    i32 palette_size;
    
    Color_Highlight highlight;
    Super_Color color;
    
    bool32 has_hover_color;
    Super_Color hover_color;
};

struct Color_View{
    View view_base;
    Hot_Directory *hot_directory;
    Style *main_style;
    Style_Library *styles;
    File_View *hot_file_view;
    Font_Set *font_set;
    u32 *palette;
    Working_Set *working_set;
    i32 palette_size;
    Color_View_Mode mode;
    UI_State state;
    Super_Color color;
    Color_Highlight highlight;
    b32 p4c_only;
    Style_Library inspecting_styles;
    b8 import_export_check[64];
    i32 import_file_id;
};

inline Color_View*
view_to_color_view(View *view){
    Assert(!view || view->type == VIEW_TYPE_COLOR);
    return (Color_View*)view;
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
                i32 steps, real32 top, f32_Rect slider){
    draw_gradient_slider(target, base, channel, steps, top, slider, 1);
}

inline void
draw_rgb_slider(Render_Target *target, Vec4 base, i32 channel,
                i32 steps, f32 top, f32_Rect slider){
    draw_gradient_slider(target, base, channel, steps, top, slider, 0);
}

internal void
do_label(UI_State *state, UI_Layout *layout, char *text, int text_size, f32 height = 2.f){
    Style *style = state->style;
    i16 font_id = style->font_id;
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
do_single_slider(i32 sub_id, Color_UI *ui, i32 channel, b32 is_rgba,
                 i32 grad_steps, f32 top, f32_Rect slider, f32 v_handle,
                 i32_Rect rect){
    f32_Rect click_box = slider;
    click_box.y0 -= v_handle;
    
    if (ui->state.input_stage){
        real32 v;
        if (ui_do_slider_input(&ui->state, i32R(click_box), make_sub1(&ui->state, sub_id),
                               slider.x0, slider.x1, &v)){
            Vec4 new_color;
            if (is_rgba) new_color = ui->color.rgba;
            else new_color = ui->color.hsla;
            new_color.v[channel] = clamp(0.f, v, 1.f);
            if (is_rgba) super_color_post_rgba(&ui->color, new_color);
            else super_color_post_hsla(&ui->color, new_color);
        }
    }
    else{
        Render_Target *target = ui->state.target;
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

enum Channel_Field_Type{
    CF_DEC,
    CF_HEX
};

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

    i16 font_id = ui->state.font_id;
    i32 line_height = get_font_info(ui->state.font_set, font_id)->height;
    i32_Rect hit_region;
    hit_region.x0 = x0;
    hit_region.x1 = x1;
    hit_region.y0 = y;
    hit_region.y1 = y + line_height;
    
    i32 digit_count;
    if (ftype == CF_DEC) digit_count = 3;
    else digit_count = 2;

    Render_Target *target = ui->state.target;
    
    if (ui->state.input_stage){
        i32 indx;
        ui_do_subdivided_button_input(&ui->state, hit_region, digit_count,
                                      make_sub1(&ui->state, sub_id), 1, &indx);
    }
    else{
        if (ui->state.hover.sub_id1 == sub_id && ui->state.selected.sub_id1 != sub_id){
            draw_rectangle(target, hit_region, back);
        }
    }
    
    char string_buffer[4];
    string_buffer[digit_count] = 0;
    fill_buffer_color_channel(string_buffer, *channel, ftype);
    
    if (ui->state.selected.sub_id1 == sub_id){
        i32 indx = ui->state.selected.sub_id2;
        if (ui->state.input_stage){
            Key_Summary *keys = ui->state.keys;
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
                        ui->state.sub_id1_change = sub_id + 3; break;
                        
                    case 7: case 8:
                        ui->state.sub_id1_change = sub_id - 6; break;
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
                ui->state.selected.sub_id2 = indx;
            }
        }
        else{
            f32_Rect r = f32R(hit_region);
            r.x0 += indx*ui->hex_advance+1;
            r.x1 = r.x0+ui->hex_advance+1;
            draw_rectangle(target, r, back);
        }
    }
    
    if (!ui->state.input_stage)
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
    
    if (ui->state.input_stage){
        bool32 right = 0;
        if (ui_do_button_input(&ui->state, i32R(blob), make_sub1(&ui->state, sub_id), 0, &right)){
            super_color_post_packed(&ui->color, color);
        }
        else if (right) *set_me = 1;
    }
    else{
        Render_Target *target = ui->state.target;
        draw_rectangle(target, blob, color);
        persist u32 silver = 0xFFa0a0a0;
        draw_rectangle_outline(target, blob, silver);
    }
}

inline void
do_blob(Color_UI *ui, Blob_Layout *layout, u32 *color, bool32 *set_me){
    i32 sub_id = (i32)((char*)color - (char*)ui->state.style);
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
    Style *style = ui->state.style;
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
    
    if (!ui->state.input_stage){
        Render_Target *target = ui->state.target;
        draw_string(target, style->font_id, "Global Palette: right click to save color",
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
            ui->state.redraw = 1;
        }
    }
}

internal void
do_sub_button(i32 id, Color_UI *ui, char *text){
    i16 font_id = ui->state.font_id;
    i32 line_height = get_font_info(ui->state.font_set, font_id)->height;
    i32_Rect rect = layout_rect(&ui->layout, line_height + 2);
    
    if (ui->state.input_stage){
        ui_do_button_input(&ui->state, rect, make_sub0(&ui->state, id), 1);
    }
    else{
        Render_Target *target = ui->state.target;
        
        u32 back_color, text_color;
        text_color = 0xFFDDDDDD;
        if (ui->state.selected.sub_id0 == id){
            back_color = 0xFF444444;
        }
        else if (ui->state.hover.sub_id0 == id){
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
    i32 id = raw_ptr_dif(color, ui->state.style);
    i16 font_id = ui->state.font_id;
    i32 character_h = get_font_info(ui->state.font_set, font_id)->height;
    u32 text = 0, back = 0;
    
    i32_Rect bar = layout_rect(&ui->layout, character_h);
    
    if (ui->state.input_stage){
        if (ui_do_button_input(&ui->state, bar, make_id(&ui->state, id), 1)){
            ui->has_hover_color = 1;
            ui->hover_color = super_color_create(*color);
        }
    }
    
    else{
        Render_Target *target = ui->state.target;
        u32 text_hover = 0xFF101010;
        u32 back_hover = 0xFF999999;
        if (ui->state.selected.id != id && ui->state.hover.id == id){
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
    
    if (ui->state.selected.id == id){
        Render_Target *target = ui->state.target;
        i32_Rect expanded = layout_rect(&ui->layout, 115 + (character_h + 2));
        UI_Layout_Restore restore = begin_sub_layout(&ui->layout, expanded);
        
        ui->color.out = color;
        
        if (ui->state.input_stage){
            if (ui->state.selected.sub_id0 == 0){
                ui->state.selected.sub_id0 = 1;
            }
        }
        else{
            draw_rectangle(target, expanded, 0xff000000);
        }
        
        begin_row(&ui->layout, 3);
        do_sub_button(1, ui, "HSL");
        do_sub_button(2, ui, "RGB");
        do_sub_button(3, ui, "Palette");
        
        i32_Rect sub_rect;
        sub_rect = expanded;
        sub_rect.y0 += 10 + character_h;
        
        switch (ui->state.selected.sub_id0){
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
    
    i16 font_id = ui->state.font_id;
    i32 line_height = get_font_info(ui->state.font_set, font_id)->height;
    
    i32_Rect srect = layout_rect(&ui->layout, line_height);
    
    Widget_ID wid = make_id(&ui->state, id);
    b32 selected = is_selected(&ui->state, wid);

    if (ui->state.input_stage){
        if (!selected){
            ui_do_button_input(&ui->state, srect, wid, 1);
        }
        else{
            Style *style = ui->state.style;
            if (ui_do_text_field_input(&ui->state, &style->name)){
                ui->state.selected = {};
            }
        }
    }
    else{
        Render_Target *target = ui->state.target;
        Style *style = ui->state.style;
        u32 back, fore_text, fore_label;
        if (selected){
            back = 0xFF000000;
            fore_label = 0xFF808080;
            fore_text = 0xFFFFFFFF;
        }
        else if (is_hover(&ui->state, wid)){
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
    Font_Info *info = get_font_info(ui->state.font_set, font_id);
    
    i32 sub_id = (i32)(i64)(info);
    i32_Rect orect = layout_rect(&ui->layout, info->height);
    
    Widget_ID wid = make_sub0(&ui->state, sub_id);
    if (ui->state.input_stage){
        if (ui_do_button_input(&ui->state, orect, wid, 0)){
            result = 1;
        }
    }
    else{
        Render_Target *target = ui->state.target;
        u32 back, fore;
        if (is_hover(&ui->state, wid)){
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
    Render_Target *target = ui->state.target;
    Font_Set *font_set = ui->state.font_set;
    
    i16 font_id = ui->state.font_id;
    Font_Info *info = get_font_info(font_set, font_id);
    i32 character_h = info->height;
    
    i32_Rect srect = layout_rect(&ui->layout, character_h);
    Widget_ID wid = make_id(&ui->state, id);
    
    if (ui->state.input_stage){
        ui_do_button_input(&ui->state, srect, wid, 1);
    }
    else{
        Style *style = ui->state.style;
        u32 back, fore;
        if (is_hover(&ui->state, wid) && !is_selected(&ui->state, wid)){
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
    
    if (is_selected(&ui->state, wid)){
        srect = layout_rect(&ui->layout, character_h/2);
        if (!ui->state.input_stage)
            draw_rectangle(target, srect, 0xFF000000);
        
        i32 count = font_set->count + 1;
        
        for (i16 i = 1; i < count; ++i){
            if (i == font_id) continue;
            if (do_font_option(ui, i)){
                ui->state.style->font_id = i;
                ui->state.style->font_changed = 1;
            }
        }
        
        srect = layout_rect(&ui->layout, character_h/2);
        if (!ui->state.input_stage)
            draw_rectangle(target, srect, 0xFF000000);
    }
}

internal i32
step_draw_adjusting(Color_View *color_view, i32_Rect rect, View_Message message,
                    Render_Target *target, Input_Summary *user_input){
    Style *style = color_view->main_style;
    i32 result = 0;
    
    if (message != VMSG_DRAW && message != VMSG_STEP) return result;
    
    Color_UI ui;
    ui.state = ui_state_init(&color_view->state, target, user_input,
                             style, color_view->font_set, color_view->working_set,
                             (message == VMSG_STEP));
    
    begin_layout(&ui.layout, rect);
    
    ui.fonts = color_view->font_set;
    ui.highlight = color_view->highlight;
    ui.color = color_view->color;
    ui.has_hover_color = 0;
    ui.state.sub_id1_change = 0;
    ui.hex_advance = font_get_max_width(ui.fonts, ui.state.font_id, "0123456789abcdefx");
    ui.palette = color_view->palette;
    ui.palette_size = color_view->palette_size;
    
    i32_Rect bar_rect = ui.layout.rect;
    bar_rect.x0 = bar_rect.x1 - 20;
    do_scroll_bar(&ui.state, bar_rect);
    
    ui.layout.y -= FLOOR32(color_view->state.view_y);
    ui.layout.rect.x1 -= 20;
    
    if (!ui.state.input_stage) draw_push_clip(target, ui.layout.rect);
    if (do_button(-1, &ui.state, &ui.layout, "Back to Library", 2)){
        color_view->mode = CV_MODE_LIBRARY;
        ui.state.view_y = 0;
    }
    
    do_style_name(&ui);
    do_font_switch(&ui);
    
    do_color_adjuster(&ui, &style->main.back_color,
                      style->main.default_color, style->main.back_color,
                      "Background");
    do_color_adjuster(&ui, &style->main.margin_color,
                      style->main.default_color, style->main.margin_color,
                      "Margin");
    do_color_adjuster(&ui, &style->main.margin_hover_color,
                      style->main.default_color, style->main.margin_hover_color,
                      "Margin Hover");
    do_color_adjuster(&ui, &style->main.margin_active_color,
                      style->main.default_color, style->main.margin_active_color,
                      "Margin Active");
        
    do_color_adjuster(&ui, &style->main.cursor_color,
                      style->main.at_cursor_color, style->main.cursor_color,
                      "Cursor");
    do_color_adjuster(&ui, &style->main.at_cursor_color,
                      style->main.at_cursor_color, style->main.cursor_color,
                      "Text At Cursor");
    do_color_adjuster(&ui, &style->main.mark_color,
                      style->main.mark_color, style->main.back_color,
                      "Mark");
    
    do_color_adjuster(&ui, &style->main.highlight_color,
                      style->main.at_highlight_color, style->main.highlight_color,
                      "Highlight");
    do_color_adjuster(&ui, &style->main.at_highlight_color,
                      style->main.at_highlight_color, style->main.highlight_color,
                      "Text At Highlight");
    
    do_color_adjuster(&ui, &style->main.default_color,
                      style->main.default_color, style->main.back_color,
                      "Text Default");
    do_color_adjuster(&ui, &style->main.comment_color,
                      style->main.comment_color, style->main.back_color,
                      "Comment");
    do_color_adjuster(&ui, &style->main.keyword_color,
                      style->main.keyword_color, style->main.back_color,
                      "Keyword");
    do_color_adjuster(&ui, &style->main.str_constant_color,
                      style->main.str_constant_color, style->main.back_color,
                      "String Constant");
    do_color_adjuster(&ui, &style->main.char_constant_color,
                      style->main.char_constant_color, style->main.back_color,
                      "Character Constant");
    do_color_adjuster(&ui, &style->main.int_constant_color,
                      style->main.int_constant_color, style->main.back_color,
                      "Integer Constant");
    do_color_adjuster(&ui, &style->main.float_constant_color,
                      style->main.float_constant_color, style->main.back_color,
                      "Float Constant");
    do_color_adjuster(&ui, &style->main.bool_constant_color,
                      style->main.bool_constant_color, style->main.back_color,
                      "Boolean Constant");
    do_color_adjuster(&ui, &style->main.preproc_color,
                      style->main.preproc_color, style->main.back_color,
                      "Preprocessor");
    do_color_adjuster(&ui, &style->main.include_color,
                      style->main.include_color, style->main.back_color,
                      "Include Constant");
    do_color_adjuster(&ui, &style->main.special_character_color,
                      style->main.special_character_color, style->main.back_color,
                      "Special Character");
    
    do_color_adjuster(&ui, &style->main.highlight_junk_color,
                      style->main.default_color, style->main.highlight_junk_color,
                      "Junk Highlight");
    do_color_adjuster(&ui, &style->main.highlight_white_color,
                      style->main.default_color, style->main.highlight_white_color,
                      "Whitespace Highlight");
    
    do_color_adjuster(&ui, &style->main.paste_color,
                      style->main.paste_color, style->main.back_color,
                      "Paste Color");
    
    Interactive_Style *bar_style = &style->main.file_info_style;
    do_color_adjuster(&ui, &bar_style->bar_color,
                      bar_style->base_color, bar_style->bar_color,
                      "Bar");
    do_color_adjuster(&ui, &bar_style->base_color,
                      bar_style->base_color, bar_style->bar_color,
                      "Bar Text");
    do_color_adjuster(&ui, &bar_style->pop1_color,
                      bar_style->pop1_color, bar_style->bar_color,
                      "Bar Pop 1");
    do_color_adjuster(&ui, &bar_style->pop2_color,
                      bar_style->pop2_color, bar_style->bar_color,
                      "Bar Pop 2");
    
    i32 did_activation = 0;
    if (ui_finish_frame(&color_view->state, &ui.state, &ui.layout, rect, 1, &did_activation)){
        result = 1;
    }
    if (did_activation){
        if (ui.has_hover_color){
            ui.color = ui.hover_color;
        }
    }
    if (!ui.state.input_stage) draw_pop_clip(target);
    color_view->color = ui.color;
    
    return result;
}

internal void
update_highlighting(Color_View *color_view){
    File_View *file_view = color_view->hot_file_view;
    if (!file_view){
        color_view->highlight = {};
        return;
    }

#if BUFFER_EXPERIMENT_SCALPEL <= 0
    Style *style = color_view->main_style;
    Editing_File *file = file_view->file;
    i32 pos = view_get_cursor_pos(file_view);
    char c = file->state.buffer.data[pos];
    
    if (c == '\r'){
        color_view->highlight.ids[0] =
            raw_ptr_dif(&style->main.special_character_color, style);
    }
    
    else if (file->state.tokens_complete){
        Cpp_Token_Stack *tokens = &file->state.token_stack;
        Cpp_Get_Token_Result result = cpp_get_token(tokens, pos);
        Cpp_Token token = tokens->tokens[result.token_index];
        if (!result.in_whitespace){
            u32 *color = style_get_color(style, token);
            color_view->highlight.ids[0] = raw_ptr_dif(color, style);
            if (token.type == CPP_TOKEN_JUNK){
                color_view->highlight.ids[1] =
                    raw_ptr_dif(&style->main.highlight_junk_color, style);
            }
            else if (char_is_whitespace(c)){
                color_view->highlight.ids[1] =
                    raw_ptr_dif(&style->main.highlight_white_color, style);
            }
            else{
                color_view->highlight.ids[1] = 0;
            }
        }
        else{
            color_view->highlight.ids[0] = 0;
            color_view->highlight.ids[1] =
                raw_ptr_dif(&style->main.highlight_white_color, style);
        }
    }
    
    else{
        if (char_is_whitespace(c)){
            color_view->highlight.ids[0] = 0;
            color_view->highlight.ids[1] =
                raw_ptr_dif(&style->main.highlight_white_color, style);
        }
        else{
            color_view->highlight.ids[0] =
                raw_ptr_dif(&style->main.default_color, style);
            color_view->highlight.ids[1] = 0;
        }
    }
    
    if (file_view->show_temp_highlight){
        color_view->highlight.ids[2] =
            raw_ptr_dif(&style->main.highlight_color, style);
        color_view->highlight.ids[3] =
            raw_ptr_dif(&style->main.at_highlight_color, style);
    }
    else if (file->state.paste_effect.tick_down > 0){
        color_view->highlight.ids[2] =
            raw_ptr_dif(&style->main.paste_color, style);
        color_view->highlight.ids[3] = 0;
    }
    else{
        color_view->highlight.ids[2] = 0;
        color_view->highlight.ids[3] = 0;
    }
#endif
}

internal b32
do_style_preview(Library_UI *ui, Style *style, i32 toggle = -1){
    b32 result = 0;
    i32 id;
    if (style == ui->state.style) id = 2;
    else id = raw_ptr_dif(style, ui->styles->styles) + 100;

    i16 font_id = style->font_id;
    Font_Info *info = get_font_info(ui->state.font_set, font_id);
    
    i32_Rect prect = layout_rect(&ui->layout, info->height*3 + 6);
    
    Widget_ID wid = make_id(&ui->state, id);
    
    if (ui->state.input_stage){
        if (ui_do_button_input(&ui->state, prect, wid, 0)){
            result = 1;
        }
    }
    else{
        Render_Target *target = ui->state.target;
        u32 margin_color = style->main.margin_color;
        if (is_hover(&ui->state, wid)){
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
    
    ui->layout.y = prect.y1;
    return result;
}

internal b32
do_main_file_box(System_Functions *system, UI_State *state, UI_Layout *layout,
                 Hot_Directory *hot_directory, b32 try_to_match, b32 case_sensitive, char *end){
    b32 result = 0;
    Style *style = state->style;
    String *string = &hot_directory->string;

    i16 font_id = style->font_id;
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
    
    i16 font_id = style->font_id;
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
        
    i16 font_id = style->font_id;
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
    
    i16 font_id = style->font_id;
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
    i16 font_id = style->font_id;
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
            
            Editing_File *file = working_set_contains(state->working_set, full_path);
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
        
        i32 count = working_set->file_index_count;
        Editing_File *files = working_set->files;
        for (i32 i = 0; i < count; ++i){
            Editing_File *file = files + i;
            
            if (!file->state.is_dummy){
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
            }
        }
    }
    
    return result;
}

internal i32
step_draw_library(System_Functions *system, Exchange *exchange, Mem_Options *mem,
                  Color_View *color_view, i32_Rect rect, View_Message message,
                  Render_Target *target, Input_Summary *user_input){
    i32 result = 0;
    
    Library_UI ui;
    ui.state = ui_state_init(&color_view->state, target, user_input,
                             color_view->main_style, color_view->font_set,
                             color_view->working_set, (message == VMSG_STEP));
    
    ui.fonts = color_view->font_set;
    ui.hot_directory = color_view->hot_directory;
    ui.styles = color_view->styles;
    
    begin_layout(&ui.layout, rect);
    
    Color_View_Mode mode = color_view->mode;
    
    i32_Rect bar_rect = ui.layout.rect;
    bar_rect.x0 = bar_rect.x1 - 20;
    do_scroll_bar(&ui.state, bar_rect);
    
    ui.layout.y -= FLOOR32(color_view->state.view_y);
    ui.layout.rect.x1 -= 20;
    
    b32 case_sensitive = 0;
    
    if (!ui.state.input_stage) draw_push_clip(ui.state.target, ui.layout.rect);
    switch (mode){
    case CV_MODE_LIBRARY:
    {
        do_label(&ui.state, &ui.layout, literal("Current Theme - Click to Edit"));
        if (do_style_preview(&ui, color_view->main_style)){
            color_view->mode = CV_MODE_ADJUSTING;
            color_view->state.selected = {};
            ui.state.view_y = 0;
            result = 1;
        }
        
        begin_row(&ui.layout, 3);
        if (ui.state.style->name.size >= 1){
            if (do_button(-2, &ui.state, &ui.layout, "Save", 2)){
                style_library_add(ui.styles, ui.state.style);
            }
        }
        else{
            do_button(-2, &ui.state, &ui.layout, "~Need's Name~", 2);
        }
        if (do_button(-3, &ui.state, &ui.layout, "Import", 2)){
            color_view->mode = CV_MODE_IMPORT_FILE;
            hot_directory_clean_end(color_view->hot_directory);
            hot_directory_reload(system, color_view->hot_directory, color_view->working_set);
        }
        if (do_button(-4, &ui.state, &ui.layout, "Export", 2)){
            color_view->mode = CV_MODE_EXPORT;
            hot_directory_clean_end(color_view->hot_directory);
            hot_directory_reload(system, color_view->hot_directory, color_view->working_set);
            memset(color_view->import_export_check, 0, sizeof(color_view->import_export_check));
        }
        
        do_label(&ui.state, &ui.layout, literal("Theme Library - Click to Select"));
        
        i32 style_count = color_view->styles->count;
        Style *style = color_view->styles->styles;
        for (i32 i = 0; i < style_count; ++i, ++style){
            if (do_style_preview(&ui, style)){
                style_copy(color_view->main_style, style);
                result = 1;
            }
        }
    }break;
    
    case CV_MODE_IMPORT_FILE:
    {
        do_label(&ui.state, &ui.layout, literal("Current Theme"));
        do_style_preview(&ui, color_view->main_style);
        
        b32 file_selected = 0;
        
        do_label(&ui.state, &ui.layout, literal("Import Which File?"));
        begin_row(&ui.layout, 2);
        if (do_button(-2, &ui.state, &ui.layout, "*.p4c only", 2, 1, color_view->p4c_only)){
            color_view->p4c_only = !color_view->p4c_only;
        }
        if (do_button(-3, &ui.state, &ui.layout, "Cancel", 2)){
            color_view->mode = CV_MODE_LIBRARY;
        }
        
        b32 new_dir = 0;
        if (do_file_list_box(system, &ui.state, &ui.layout,
                             ui.hot_directory, color_view->p4c_only, 1, case_sensitive,
                             &new_dir, &file_selected, 0)){
            result = 1;
        }
        
        if (new_dir){
            hot_directory_reload(system, ui.hot_directory, ui.state.working_set);
        }
        if (file_selected){
            memset(&color_view->inspecting_styles, 0, sizeof(Style_Library));
            memset(color_view->import_export_check, 1,
                   sizeof(color_view->import_export_check));
            
            color_view->import_file_id =
                exchange_request_file(exchange,
                                      color_view->hot_directory->string.str,
                                      color_view->hot_directory->string.size);
            color_view->mode = CV_MODE_IMPORT_WAIT;

        }
    }break;

    case CV_MODE_IMPORT_WAIT:
    {
        Style *styles = color_view->inspecting_styles.styles;
        Data file = {};
        i32 file_max = 0;
        
        i32 count = 0;
        i32 max = ArrayCount(color_view->inspecting_styles.styles);
        
        AllowLocal(styles);
        AllowLocal(max);
        
        if (exchange_file_ready(exchange, color_view->import_file_id,
                                &file.data, &file.size, &file_max)){
            if (file.data){
                if (0 /* && style_library_import(file, ui.fonts, styles, max, &count) */){
                    color_view->mode = CV_MODE_IMPORT;
                }
                else{
                    color_view->mode = CV_MODE_LIBRARY;
                }
                color_view->inspecting_styles.count = count;
            }
            else{
                Assert(!"this shouldn't happen!");
            }
            
            exchange_free_file(exchange, color_view->import_file_id);
        }
    }break;
    
    case CV_MODE_EXPORT_FILE:
    {
        do_label(&ui.state, &ui.layout, literal("Current Theme"));
        do_style_preview(&ui, color_view->main_style);
        
        b32 file_selected = 0;
        
        do_label(&ui.state, &ui.layout, literal("Export File Name?"));
        begin_row(&ui.layout, 2);
        if (do_button(-2, &ui.state, &ui.layout, "Finish Export", 2)){
            file_selected = 1;
        }
        if (do_button(-3, &ui.state, &ui.layout, "Cancel", 2)){
            color_view->mode = CV_MODE_LIBRARY;
        }
        
        b32 new_dir = 0;
        if (do_file_list_box(system, &ui.state, &ui.layout,
                             ui.hot_directory, 1, 1, case_sensitive,
                             &new_dir, &file_selected, ".p4c")){
            result = 1;
        }
        
        if (new_dir){
            hot_directory_reload(system,
                                 ui.hot_directory, ui.state.working_set);
        }
        if (file_selected){
            i32 count = ui.styles->count;
            Temp_Memory temp = begin_temp_memory(&mem->part);
            Style **styles = push_array(&mem->part, Style*, sizeof(Style*)*count);
            
            Style *style = ui.styles->styles;
            bool8 *export_check = color_view->import_export_check;
            i32 export_count = 0;
            for (i32 i = 0; i < count; ++i, ++style){
                if (export_check[i]){
                    styles[export_count++] = style;
                }
            }
            char *data = push_array(&mem->part, char, ui.hot_directory->string.size + 5);
            String str = make_string(data, 0, ui.hot_directory->string.size + 5);
            copy(&str, ui.hot_directory->string);
            append(&str, make_lit_string(".p4c"));
            /*style_library_export(system, exchange, mem, &target->font_set, str.str, styles, export_count);*/
            
            end_temp_memory(temp);
            color_view->mode = CV_MODE_LIBRARY;
        }
    }break;
    
    case CV_MODE_IMPORT:
    {
        do_label(&ui.state, &ui.layout, literal("Current Theme"));
        do_style_preview(&ui, color_view->main_style);
        
        i32 style_count = color_view->inspecting_styles.count;
        Style *styles = color_view->inspecting_styles.styles;
        bool8 *import_check = color_view->import_export_check;
        
        do_label(&ui.state, &ui.layout, literal("Pack"));
        begin_row(&ui.layout, 2);
        if (do_button(-2, &ui.state, &ui.layout, "Finish Import", 2)){
            Style *style = styles;
            for (i32 i = 0; i < style_count; ++i, ++style){
                if (import_check[i]) style_library_add(ui.styles, style);
            }
            color_view->mode = CV_MODE_LIBRARY;
        }
        if (do_button(-3, &ui.state, &ui.layout, "Cancel", 2)){
            color_view->mode = CV_MODE_LIBRARY;
        }
        
        Style *style = styles;
        for (i32 i = 0; i < style_count; ++i, ++style){
            if (do_style_preview(&ui, style, import_check[i])){
                import_check[i] = !import_check[i];
                result = 1;
            }
        }
    }break;
    
    case CV_MODE_EXPORT:
    {
        do_label(&ui.state, &ui.layout, literal("Current Theme"));
        do_style_preview(&ui, color_view->main_style);
        
        do_label(&ui.state, &ui.layout, literal("Export Which Themes?"));
        begin_row(&ui.layout, 2);
        if (do_button(-2, &ui.state, &ui.layout, "Export", 2)){
            color_view->mode = CV_MODE_EXPORT_FILE;
        }
        if (do_button(-3, &ui.state, &ui.layout, "Cancel", 2)){
            color_view->mode = CV_MODE_LIBRARY;
        }
        
        i32 style_count = color_view->styles->count;
        Style *style = color_view->styles->styles;
        bool8 *export_check = color_view->import_export_check;
        for (i32 i = 0; i < style_count; ++i, ++style){
            if (do_style_preview(&ui, style, export_check[i])){
                export_check[i] = !export_check[i];
                result = 1;
            }
        }
    }break;
    }
    if (!ui.state.input_stage) draw_pop_clip(ui.state.target);
    
    if (ui_finish_frame(&color_view->state, &ui.state, &ui.layout, rect, 1, 0)){
        result = 1;
    }
    
    return result;
}

internal
Do_View_Sig(do_color_view){
    view->mouse_cursor_type = APP_MOUSE_CURSOR_ARROW;
    Color_View *color_view = (Color_View*)view;
    i32 result = 0;
    
    switch (color_view->mode){
    case CV_MODE_LIBRARY:
    case CV_MODE_IMPORT_FILE:
    case CV_MODE_EXPORT_FILE:
    case CV_MODE_IMPORT:
    case CV_MODE_EXPORT:
    case CV_MODE_IMPORT_WAIT:
        switch (message){
        case VMSG_STEP:
        {
            result = step_draw_library(system, exchange, view->mem,
                                       color_view, rect, message, target, user_input);
        }break;
        case VMSG_DRAW:
        {
            step_draw_library(system, exchange, view->mem,
                              color_view, rect, message, target, user_input);
        }break;
        }break;
        
    case CV_MODE_ADJUSTING:
        switch (message){
        case VMSG_STEP:
        {
            result = step_draw_adjusting(color_view, rect, message, target, user_input);
        }break;
        case VMSG_DRAW:
        {
            if (view != active){
                File_View *file_view = view_to_file_view(active);
                color_view->hot_file_view = file_view;
            }
            if (color_view->hot_file_view && !color_view->hot_file_view->view_base.is_active){
                color_view->hot_file_view = 0;
            }
            update_highlighting(color_view);
            step_draw_adjusting(color_view, rect, message, target, user_input);
        }break;
        }break;
    }
    
    return result;
}

internal Color_View*
color_view_init(View *view, Working_Set *working_set){
    Color_View* result = (Color_View*)view;
    view->type = VIEW_TYPE_COLOR;
    view->do_view = do_color_view;
    result->working_set = working_set;
    return result;
}

// BOTTOM

