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

struct GUI_id{
    u64 id[2];
};
inline GUI_id
gui_id_zero(){
    GUI_id id = {0};
    return(id);
}

struct GUI_Scroll_Vars{
    f32 scroll_y;
    f32 target_y;
    f32 prev_target_y;
    f32 min_y, max_y;
    
    f32 scroll_x;
    f32 target_x;
    f32 prev_target_x;
};

struct GUI_Target{
    Partition push;
    
    GUI_id active;
    GUI_id mouse_hot;
    GUI_id auto_hot;
    GUI_id hover;
    
    // TODO(allen): Can we remove original yet?
    GUI_Scroll_Vars scroll_original;
    i32_Rect region_original;
    
    GUI_Scroll_Vars scroll_updated;
    i32_Rect region_updated;
    
    // TODO(allen): Would rather have a way of tracking this
    // for more than one list.  Perhaps just throw in a hash table?
    // Or maybe this only needs to be tracked for the active list.
    i32 list_max;
    b32 has_list_index_position;
    i32_Rect list_index_position;
    i32 list_view_min;
    i32 list_view_max;
    
    f32 delta;
    u32 scroll_id;
    b32 has_keys;
    b32 animating;
};

struct GUI_Item_Update{
    i32 partition_point;
    
    b32 has_adjustment;
    i32 adjustment_value;
    
    b32 has_index_position;
    i32_Rect index_position;
};

struct GUI_Header{
    i32 type;
    i32 size;
};

struct GUI_Interactive{
    GUI_Header h;
    GUI_id id;
};

struct GUI_Edit{
    GUI_Header h;
    GUI_id id;
    void *out;
};

enum GUI_Command_Type{
    guicom_null,
    guicom_begin_overlap,
    guicom_end_overlap,
    guicom_begin_serial,
    guicom_end_serial,
    guicom_top_bar,
    guicom_file,
    guicom_text_field,
    guicom_color_button,
    guicom_font_button,
    guicom_text_with_cursor,
    guicom_begin_list,
    guicom_end_list,
    guicom_file_option,
    guicom_fixed_option,
    guicom_button,
    guicom_fixed_option_checkbox,
    guicom_style_preview,
    guicom_scrollable,
    guicom_scrollable_bar,
    guicom_scrollable_top,
    guicom_scrollable_slider,
    guicom_scrollable_bottom,
    guicom_scrollable_invisible,
    guicom_begin_scrollable_section,
    guicom_end_scrollable_section,
};

inline b32
gui_id_eq(GUI_id id1, GUI_id id2){
    b32 result = (id1.id[0] == id2.id[0] && id1.id[1] == id2.id[1]);
    return(result);
}

inline b32
gui_id_is_null(GUI_id id){
    b32 result = (id.id[0] == 0 && id.id[1] == 0);
    return(result);
}

internal i32
gui_active_level(GUI_Target *target, GUI_id id){
    i32 level = 0;
    if (gui_id_eq(target->active, id)){
        level = 4;
	}
    else if (gui_id_eq(target->auto_hot, id)){
        level = 3;
    }
    else if (gui_id_eq(target->mouse_hot, id)){
        if (gui_id_eq(target->hover, id)){
            level = 3;
		}
        else{
            level = 2;
        }
    }
    else if (gui_id_eq(target->hover, id) && gui_id_is_null(target->mouse_hot)){
        level = 1;
    }
    return(level);
}

internal void
gui_rollback(GUI_Target *target, GUI_Item_Update *update){
    target->push.pos = update->partition_point;
}

internal void
gui_fill_update(GUI_Item_Update *update, GUI_Target *target, GUI_Header *h){
    if (update){
        update->partition_point = (i32)((char*)h - (char*)target->push.base);
        update->has_adjustment = 0;
        update->has_index_position = 0;
    }
}

internal void
gui_update_adjustment(GUI_Item_Update *update, i32 adjustment_value){
    if (update){
        update->has_adjustment = 1;
        update->adjustment_value = adjustment_value;
    }
}

internal void
gui_update_position(GUI_Item_Update *update, i32_Rect position){
    if (update){
        update->has_index_position = 1;
        update->index_position = position;
    }
}

internal void*
gui_push_item(GUI_Target *target, void *item, i32 size){
    void *dest = partition_allocate(&target->push, size);
    if (dest && item){
        memcpy(dest, item, size);
    }
    return(dest);
}

internal void*
gui_align(GUI_Target *target){
    void *ptr;
    partition_align(&target->push, 8);
    ptr = partition_current(&target->push);
    return(ptr);
}

internal void*
gui_align(GUI_Target *target, GUI_Header *h){
    void *ptr;
    partition_align(&target->push, 8);
    ptr = partition_current(&target->push);
    h->size = (i32)((char*)ptr - (char*)h);
    return(ptr);
}

internal void*
advance_to_alignment(void *ptr){
    u64 p = (u64)ptr;
    p = (p + 7) & (~7);
    return (void*)p;
}

internal void*
gui_push_aligned_item(GUI_Target *target, GUI_Header *h, void *item, i32 size){
    char *ptr = (char*)partition_allocate(&target->push, size);
    if (ptr && item){
        memcpy(ptr, item, size);
	}
    gui_align(target, h);
    return(ptr);
}

internal void*
gui_push_item(GUI_Target *target, GUI_Header *h, void *item, i32 size){
    void *ptr;
    ptr = (char*)partition_allocate(&target->push, size);
    if (ptr && item){
        memcpy(ptr, item, size);
	}
    h->size += size;
    return(ptr);
}

internal GUI_Header*
gui_push_simple_command(GUI_Target *target, i32 type){
    GUI_Header *result = 0;
    GUI_Header item;
    item.type = type;
    item.size = sizeof(item);
    result = (GUI_Header*)gui_push_item(target, &item, sizeof(item));
    return(result);
}

internal GUI_Edit*
gui_push_string_edit_command(GUI_Target *target, i32 type, GUI_id id, void *out){
    GUI_Edit *result = 0;
    GUI_Edit item;
    item.h.type = type;
    item.h.size = sizeof(item);
    item.id = id;
    item.out = out;
    result = (GUI_Edit*)gui_push_item(target, &item, sizeof(item));
    return(result);
}

internal GUI_Interactive*
gui_push_button_command(GUI_Target *target, i32 type, GUI_id id){
    GUI_Interactive *result = 0;
    GUI_Interactive item;
    item.h.type = type;
    item.h.size = sizeof(item);
    item.id = id;
    result = (GUI_Interactive*)gui_push_item(target, &item, sizeof(item));
    return(result);
}

internal void
gui_push_string(GUI_Target *target, GUI_Header *h, String s, i32 extra){
    u8 *start, *end, *str_start;
    i32 size;
    i32 *cap;
    
    start = (u8*)gui_push_item(target, &s.size, sizeof(s.size));
    cap = (i32*)gui_push_item(target, 0, sizeof(i32));
    str_start = (u8*)gui_push_item(target, s.str, s.size);
    if (extra) gui_push_item(target, 0, extra);
    end = (u8*)gui_align(target);
    size = (i32)(end - start);
    *cap = (i32)(end - str_start);
    h->size += size;
}

internal void
gui_push_string(GUI_Target *target, GUI_Header *h, String s){
    gui_push_string(target, h, s, 0);
}

internal void
gui_begin_overlap(GUI_Target *target){
    gui_push_simple_command(target, guicom_begin_overlap);
}

internal void
gui_end_overlap(GUI_Target *target){
    gui_push_simple_command(target, guicom_end_overlap);
}

internal void
gui_begin_serial_section(GUI_Target *target){
    gui_push_simple_command(target, guicom_begin_serial);
}

internal void
gui_end_serial_section(GUI_Target *target){
    gui_push_simple_command(target, guicom_end_serial);
}

internal void
gui_begin_top_level(GUI_Target *target, Input_Summary input){
    target->push.pos = 0;
    target->has_keys = (input.keys.count > 0);
    target->animating = 0;
}

internal void
gui_end_top_level(GUI_Target *target){
    gui_push_simple_command(target, guicom_null);
    target->has_list_index_position = 0;
}

internal void
gui_do_top_bar(GUI_Target *target){
    gui_push_simple_command(target, guicom_top_bar);
}

internal void
gui_do_file(GUI_Target *target){
    gui_push_simple_command(target, guicom_file);
}

internal void
gui_do_text_field(GUI_Target *target, String prompt, String text){
    GUI_Header *h = gui_push_simple_command(target, guicom_text_field);
    gui_push_string(target, h, prompt);
    gui_push_string(target, h, text);
}

internal b32
gui_do_text_with_cursor(GUI_Target *target, i32 pos, String text, GUI_Item_Update *update){
    b32 result = 0;
    GUI_Header *h = gui_push_simple_command(target, guicom_text_with_cursor);
    gui_push_string(target, h, text);
    gui_push_item(target, h, &pos, sizeof(i32));
    
    result = target->has_keys;
    if (result){
        gui_fill_update(update, target, h);
        target->animating = 1;
    }
    
    return(result);
}

internal b32
gui_do_color_button(GUI_Target *target, GUI_id id, u32 fore, u32 back, String text){
    b32 result = 0;
    GUI_Interactive *b = gui_push_button_command(target, guicom_color_button, id);
    GUI_Header *h = (GUI_Header*)b;
    gui_push_item(target, h, &fore, sizeof(fore));
    gui_push_item(target, h, &back, sizeof(back));
    gui_push_string(target, h, text);
    
    if (gui_id_eq(id, target->active)){
        result = 1;
        target->animating = 1;
	}
    
    return(result);
}

internal b32
gui_do_font_button(GUI_Target *target, GUI_id id, i16 font_id, String text){
    b32 result = 0;
    i32 font_id32 = font_id;
    GUI_Interactive *b = gui_push_button_command(target, guicom_font_button, id);
    GUI_Header *h = (GUI_Header*)b;
    gui_push_item(target, h, &font_id32, sizeof(font_id32));
    gui_push_string(target, h, text);
    
    if (gui_id_eq(id, target->active)){
        result = 1;
        target->animating = 1;
	}
    
    return(result);
}

internal b32
gui_begin_list(GUI_Target *target, GUI_id id, i32 list_i,
               b32 activate_item, b32 snap_into_view, GUI_Item_Update *update){
    b32 result = 0;
    b32 active = 0;
    
    i32 list_min = 0;
    i32 list_max = target->list_max;
    
    GUI_Interactive *b = gui_push_button_command(target, guicom_begin_list, id);
    GUI_Header *h = (GUI_Header*)b;
    gui_push_item(target, h, &list_i, sizeof(list_i));
    gui_push_item(target, h, &activate_item, sizeof(activate_item));
    
    result = target->has_keys || target->has_list_index_position;
    if (gui_id_eq(id, target->active)){
        active = 1;
        result = 1;
	}
    
    if (snap_into_view){
        if (target->list_view_min > list_min){
            list_min = target->list_view_min;
        }
        if (target->list_view_max < list_max){
            list_max = target->list_view_max;
        }
    }
    
    if (list_i < list_min || list_i >= list_max){
        result = 1;
    }
    
    if (result){
        gui_fill_update(update, target, h);
        if (list_i < list_min){
            gui_update_adjustment(update, list_min);
        }
        else if (list_i >= list_max){
            gui_update_adjustment(update, list_max - 1);
        }
        if (target->has_list_index_position){
            gui_update_position(update, target->list_index_position);
        }
        
        target->animating = 1;
    }
    
    return(result);
}

internal void
gui_end_list(GUI_Target *target){
    gui_push_simple_command(target, guicom_end_list);
}

internal b32
gui_do_file_option(GUI_Target *target, GUI_id id, String filename,
                   b32 is_folder, String message){
    b32 result = 0;
    GUI_Interactive *b = gui_push_button_command(target, guicom_file_option, id);
    GUI_Header *h = (GUI_Header*)b;
    gui_push_item(target, h, &is_folder, sizeof(is_folder));
    gui_push_string(target, h, filename, 1);
    gui_push_string(target, h, message);
    
    if (gui_id_eq(id, target->active)){
        result = 1;
        target->animating = 1;
	}
    
    return(result);
}

internal b32
gui_do_button(GUI_Target *target, GUI_id id, String message){
    b32 result = 0;
    GUI_Interactive *b = gui_push_button_command(target, guicom_button, id);
    GUI_Header *h = (GUI_Header*)b;
    gui_push_string(target, h, message);
    gui_align(target, h);
    
    if (gui_id_eq(id, target->active)){
        result = 1;
        target->animating = 1;
	}
    
    return(result);
}

internal b32
gui_do_fixed_option(GUI_Target *target, GUI_id id, String message, char key){
    b32 result = 0;
    GUI_Interactive *b = gui_push_button_command(target, guicom_fixed_option, id);
    GUI_Header *h = (GUI_Header*)b;
    gui_push_string(target, h, message);
    gui_push_item(target, h, &key, 1);
    gui_align(target, h);
    
    if (gui_id_eq(id, target->active)){
        result = 1;
        target->animating = 1;
	}
    
    return(result);
}

internal b32
gui_do_fixed_option_checkbox(GUI_Target *target, GUI_id id, String message, char key, b8 state){
    b32 result = 0;
    GUI_Interactive *b = gui_push_button_command(target, guicom_fixed_option_checkbox, id);
    GUI_Header *h = (GUI_Header*)b;
    gui_push_string(target, h, message);
    gui_push_item(target, h, &key, 1);
    gui_push_item(target, h, &state, 1);
    gui_align(target, h);
    
    if (gui_id_eq(id, target->active)){
        result = 1;
        target->animating = 1;
	}
    
    return(result);
}

internal b32
gui_do_style_preview(GUI_Target *target, GUI_id id, i32 style_index){
    b32 result = 0;
    GUI_Interactive *b = gui_push_button_command(target, guicom_style_preview, id);
    GUI_Header *h = (GUI_Header*)b;
    gui_push_item(target, h, &style_index, sizeof(style_index));
    gui_align(target, h);
    
    if (gui_id_eq(id, target->active)){
        result = 1;
        target->animating = 1;
	}
    
    return(result);
}

internal GUI_id
gui_id_scrollbar(){
    GUI_id id;
    id.id[0] = 0;
    id.id[1] = max_u64;
    return(id);
}

internal GUI_id
gui_id_scrollbar_top(){
    GUI_id id;
    id.id[0] = 1;
    id.id[1] = max_u64;
    return(id);
}

internal GUI_id
gui_id_scrollbar_slider(){
    GUI_id id;
    id.id[0] = 2;
    id.id[1] = max_u64;
    return(id);
}

internal GUI_id
gui_id_scrollbar_bottom(){
    GUI_id id;
    id.id[0] = 3;
    id.id[1] = max_u64;
    return(id);
}

internal b32
gui_scroll_eq(GUI_Scroll_Vars *a, GUI_Scroll_Vars *b){
    b32 result = (memcmp(a, b, sizeof(*a)) == 0);
    return(result);
}

// TODO(allen): Rethink this a little, seems like there are two separate things we want to do here:
// Getting the updated scroll vars, and telling the user when scrolling actions occur.
internal b32
gui_get_scroll_vars(GUI_Target *target, u32 scroll_id, GUI_Scroll_Vars *vars_out, i32_Rect *region_out){
    b32 result = 0;
    if (target->scroll_id == scroll_id){
        *vars_out = target->scroll_updated;
        *region_out = target->region_updated;
        
        if (vars_out->target_y < vars_out->min_y) vars_out->target_y = vars_out->min_y;
        if (vars_out->target_y > vars_out->max_y) vars_out->target_y = vars_out->max_y;
        
        if (vars_out->scroll_y < vars_out->min_y) vars_out->scroll_y = vars_out->min_y;
        if (vars_out->scroll_y > vars_out->max_y) vars_out->scroll_y = vars_out->max_y;
        
        if (gui_id_eq(target->active, gui_id_scrollbar())){
            result = 1;
            target->animating = 1;
        }
	}
    return(result);
}

internal GUI_Scroll_Vars
gui_current_scroll_vars(GUI_Target *target){
    GUI_Scroll_Vars vars = target->scroll_updated;
    return(vars);
}

internal void
gui_post_scroll_vars(GUI_Target *target, GUI_Scroll_Vars *vars_in){
    if (!gui_scroll_eq(vars_in, &target->scroll_updated)){
        target->scroll_updated = *vars_in;
        target->animating = 1;
        target->active = gui_id_scrollbar();
    }
}

internal void
gui_begin_scrollable(GUI_Target *target, u32 scroll_id,
                     GUI_Scroll_Vars scroll_vars, f32 delta, b32 show_bar){
    GUI_Header *h;
    
    gui_begin_serial_section(target);
    
    target->delta = delta;
    h = gui_push_simple_command(target, guicom_scrollable);
    
    target->scroll_original = scroll_vars;
    target->scroll_updated = scroll_vars;
    target->scroll_id = scroll_id;
    
    if (show_bar){
        gui_push_simple_command(target, guicom_scrollable_bar);
        gui_push_simple_command(target, guicom_scrollable_top);
        gui_push_simple_command(target, guicom_scrollable_slider);
        gui_push_simple_command(target, guicom_scrollable_bottom);
    }
    else{
        gui_push_simple_command(target, guicom_scrollable_invisible);
    }
    gui_push_simple_command(target, guicom_begin_scrollable_section);
}

internal void
gui_end_scrollable(GUI_Target *target){
    gui_push_simple_command(target, guicom_end_scrollable_section);
    gui_end_serial_section(target);
}

internal void
gui_activate_scrolling(GUI_Target *target){
    target->active = gui_id_scrollbar();
}

struct GUI_Section{
    b32 overlapped;
    i32 max_v, v, top_v;
};

struct GUI_List_Vars{
    b32 in_list;
    i32 index;
    i32 auto_hot;
    i32 auto_activate;
};

struct GUI_Session{
    i32_Rect full_rect;
    i32_Rect rect;
    
    f32 suggested_min_y;
    f32 suggested_max_y;
    i32 clip_y;
    
    i32 line_height;
    b32 is_scrollable;
    i32 scrollable_items_bottom;
    
    i32_Rect scroll_region;
    i32_Rect scroll_rect;
    f32 scroll_top, scroll_bottom;
    
    GUI_List_Vars list;
    
    GUI_Section sections[64];
    i32 t;
};

#define GUIScrollbarWidth 16

internal i32
gui_session_get_eclipsed_y(GUI_Session *session){
    GUI_Section *section = session->sections;
    i32 count = session->t + 1, i;
    i32 max_v = 0;
    for (i = 0; i < count; ++i, ++section){
        if (section->overlapped){
            max_v = Max(max_v, section->max_v);
        }
    }
    max_v = Max(max_v, session->sections[count-1].top_v);
    return(max_v);
}

internal i32
gui_session_get_current_top(GUI_Session *session){
    i32 result = session->sections[session->t].top_v;
    return(result);
}

inline GUI_Session
gui_session_zero(){
    GUI_Session session={0};
    return(session);
}

internal void
gui_session_init(GUI_Session *session, GUI_Target *target,
                 i32_Rect full_rect, i32 line_height){
    GUI_Section *section;
    
    *session = gui_session_zero();
    session->full_rect = full_rect;
    session->line_height = line_height;
    
    section = &session->sections[0];
    section->v = full_rect.y0;
    section->max_v = full_rect.y0;
    
    target->list_view_min = max_i32;
    target->list_view_max = min_i32;
}

internal void
gui_section_end_item(GUI_Section *section, i32 v){
    if (!section->overlapped){
        section->v = v;
	}
    if (section->max_v < v){
        section->max_v = v;
    }
}

inline i32_Rect
gui_layout_top_bottom(GUI_Session *session, i32 y0, i32 y1){
    i32_Rect rect = {0};
    
    rect.y0 = y0;
    rect.y1 = y1;
    rect.x0 = session->full_rect.x0;
    rect.x1 = session->full_rect.x1;
    
    if (session->is_scrollable){
        rect.x0 = session->scroll_region.x0;
        rect.x1 = session->scroll_region.x1;
	}
    
    return(rect);
}

inline i32_Rect
gui_layout_fixed_h(GUI_Session *session, i32 y, i32 h){
    i32_Rect rect;
    rect = gui_layout_top_bottom(session, y, y + h);
    return(rect);
}

internal void
gui_scrollbar_top(i32_Rect bar, i32_Rect *top){
    i32 w = (bar.x1 - bar.x0);
    top->x0 = bar.x0;
    top->x1 = bar.x1;
    top->y0 = bar.y0;
    top->y1 = top->y0 + w;
}

internal void
gui_scrollbar_slider(i32_Rect bar, i32_Rect *slider, f32 s, f32 *min_out, f32 *max_out, f32 target_min, f32 target_max){
    i32 h, w = (bar.x1 - bar.x0);
    i32 min, max, pos;
    
    f32 screen_size;
    f32 full_size;
    f32 ratio;
    
    screen_size = (f32)(bar.y1 - bar.y0);
    full_size = (f32)(target_max - target_min + screen_size);
    ratio = 1.f;
    if (full_size > screen_size){
        ratio = screen_size/full_size;
    }
    
    h = (i32)(ratio * bar.y1 - bar.y0 - w*2);
    
    if (h < w){
        h = w;
    }
    
    slider->x0 = bar.x0;
    slider->x1 = bar.x1;
    
    min = bar.y0 + w + h/2;
    max = bar.y1 - w - h/2;
    
    pos = lerp(min, s, max);
    
    slider->y0 = pos - h/2;
    slider->y1 = slider->y0 + h;
    
    *min_out = (f32)min;
    *max_out = (f32)max;
}

internal void
gui_scrollbar_bottom(i32_Rect bar, i32_Rect *bottom){
    i32 w = (bar.x1 - bar.x0);
    bottom->x0 = bar.x0;
    bottom->x1 = bar.x1;
    bottom->y1 = bar.y1;
    bottom->y0 = bottom->y1 - w;
}

#define NextHeader(h) ((GUI_Header*)((char*)(h) + (h)->size))

internal i8
gui_read_byte(void **ptr){
    i8 result;
    result = *(i8*)*ptr;
    *ptr = ((char*)*ptr) + 1;
    return(result);
}

internal i32
gui_read_integer(void **ptr){
    i32 result;
    result = *(i32*)*ptr;
    *ptr = ((char*)*ptr) + 4;
    return(result);
}

internal f32
gui_read_float(void **ptr){
    f32 result;
    result = *(f32*)*ptr;
    *ptr = ((char*)*ptr) + 4;
    return(result);
}

internal String
gui_read_string(void **ptr){
    String result;
    
    result.size = *(i32*)*ptr;
    *ptr = ((i32*)*ptr) + 1;
    result.memory_size = *(i32*)*ptr;
    *ptr = ((i32*)*ptr) + 1;
    
    result.str = (char*)*ptr;
    *ptr = result.str + result.memory_size;
    
    return(result);
}

internal void*
gui_read_out(void **ptr){
    void *result;
    result = *(void**)*ptr;
    *ptr = ((void**)ptr) + 1;
    return(result);
}

struct GUI_Interpret_Result{
    b32 has_info;
    b32 auto_hot;
    b32 auto_activate;
    i32 screen_orientation;
};

internal GUI_Interpret_Result
gui_interpret(GUI_Target *target, GUI_Session *session, GUI_Header *h){
    GUI_Interpret_Result result = {0};
    GUI_Section *section = 0;
    GUI_Section *new_section = 0;
    GUI_Section *prev_section = 0;
    GUI_Section *end_section = 0;
    b32 give_to_user = 0;
    b32 always_give_to_user = 0;
    b32 do_layout = 1;
    b32 is_list_item = 0;
    i32_Rect rect = {0};
    i32 y = 0;
    i32 end_v = -1;
    f32 lerp_space_scroll_v = 0;
    i32 scroll_v = (i32)target->scroll_original.scroll_y;
    i32 target_v = (i32)target->scroll_updated.target_y;
    
    Assert(session->t < ArrayCount(session->sections));
    section = session->sections + session->t;
    y = section->v;
    
    if (!session->is_scrollable) scroll_v = 0;

    switch (h->type){
        case guicom_null: Assert(0); break;

        case guicom_begin_overlap:
        ++session->t;
        Assert(session->t < ArrayCount(session->sections));
        new_section = &session->sections[session->t];
        new_section->overlapped = 1;
        new_section->v = y;
        new_section->max_v = y;
        new_section->top_v = y;
        break;

        case guicom_end_overlap:
        Assert(session->t > 0);
        Assert(section->overlapped);
        prev_section = &session->sections[--session->t];
        end_v = section->max_v;
        end_section = prev_section;
        break;

        case guicom_begin_serial:
        ++session->t;
        Assert(session->t < ArrayCount(session->sections));
        new_section = &session->sections[session->t];
        new_section->overlapped = 0;
        new_section->v = y;
        new_section->max_v = y;
        new_section->top_v = y;
        break;

        case guicom_end_serial:
        Assert(session->t > 0);
        Assert(!section->overlapped);
        prev_section = &session->sections[--session->t];
        end_v = section->max_v;
        end_section = prev_section;
        break;

        case guicom_top_bar:
        give_to_user = 1;
        rect = gui_layout_fixed_h(session, y, session->line_height + 2);
        end_v = rect.y1;
        end_section = section;
        break;

        case guicom_file:
        give_to_user = 1;
        rect = gui_layout_top_bottom(session, y, session->full_rect.y1);
        end_v = rect.y1;
        end_section = section;
        scroll_v = 0;
        break;

        case guicom_text_with_cursor:
        case guicom_text_field:
        give_to_user = 1;
        rect = gui_layout_fixed_h(session, y, session->line_height + 2);
        end_v = rect.y1;
        end_section = section;
        break;

        case guicom_color_button:
        case guicom_font_button:
        give_to_user = 1;
        rect = gui_layout_fixed_h(session, y, session->line_height + 2);
        end_v = rect.y1;
        end_section = section;
        break;

        case guicom_begin_list:
        {
            GUI_Interactive *b = (GUI_Interactive*)h;
            void *ptr = (b + 1);
            i32 index = gui_read_integer(&ptr);
            b32 activate = (b32)gui_read_integer(&ptr);
            
            Assert(session->list.in_list == 0);
            session->list.in_list = 1;
            session->list.index = 0;
            session->list.auto_hot = index;
            session->list.auto_activate = -1;
            if (activate){
                session->list.auto_activate = index;
            }
        }break;
        
        case guicom_end_list:
        Assert(session->list.in_list == 1);
        session->list.in_list = 0;
        target->list_max = session->list.index;
        break;
        
        case guicom_file_option:
        case guicom_fixed_option:
        case guicom_fixed_option_checkbox:
        {
            give_to_user = 1;
            rect = gui_layout_fixed_h(session, y, session->line_height * 2);
            end_v = rect.y1;
            end_section = section;
            is_list_item = 1;
            
            if (session->list.in_list){
                if (session->list.auto_hot == session->list.index){
                    result.auto_hot = 1;
                    if (!rect_equal(target->list_index_position, rect)){
                        target->has_list_index_position = 1;
                        target->list_index_position = rect;
                    }
                }
                if (session->list.auto_activate == session->list.index){
                    result.auto_activate = 1;
                }
                
                ++session->list.index;
            }
        }break;
        
        case guicom_button:
        give_to_user = 1;
        rect = gui_layout_fixed_h(session, y, session->line_height * 2);
        end_v = rect.y1;
        end_section = section;
        break;
        
        case guicom_style_preview:
        give_to_user = 1;
        rect = gui_layout_fixed_h(session, y, session->line_height * 3 + 6);
        end_v = rect.y1;
        end_section = section;
        break;
        
        case guicom_scrollable:
        Assert(session->is_scrollable == 0);
        Assert(!section->overlapped);
        session->is_scrollable = 1;
        always_give_to_user = 1;
        
        {
            i32_Rect scrollable_rect = {0};
            scrollable_rect.x0 = session->full_rect.x0;
            scrollable_rect.x1 = session->full_rect.x1;
            scrollable_rect.y0 = y;
            scrollable_rect.y1 = session->full_rect.y1;
            target->region_updated = scrollable_rect;
            session->scroll_region = scrollable_rect;
        }
        break;
        
        case guicom_scrollable_bar:
        Assert(session->is_scrollable);
        Assert(!section->overlapped);
        give_to_user = 1;
        rect.x1 = session->full_rect.x1;
        rect.x0 = rect.x1 - GUIScrollbarWidth;
        rect.y0 = y;
        rect.y1 = session->full_rect.y1;
        session->scroll_rect = rect;
        
        {
            i32_Rect scrollable_rect = {0};
            scrollable_rect.x0 = session->full_rect.x0;
            scrollable_rect.x1 = rect.x0;
            scrollable_rect.y0 = y;
            scrollable_rect.y1 = session->full_rect.y1;
            target->region_updated = scrollable_rect;
            session->scroll_region = scrollable_rect;
        }
        scroll_v = 0;
        break;
        
        case guicom_scrollable_top:
        Assert(session->is_scrollable);
        Assert(!section->overlapped);
        give_to_user = 1;
        gui_scrollbar_top(session->scroll_rect, &rect);
        scroll_v = 0;
        break;
        
        case guicom_scrollable_slider:
        Assert(session->is_scrollable);
        Assert(!section->overlapped);
        give_to_user = 1;
        
        lerp_space_scroll_v =
            unlerp((f32)target->scroll_original.min_y,
                   (f32)target->scroll_original.target_y,
                   (f32)target->scroll_original.max_y);
        
        gui_scrollbar_slider(session->scroll_rect, &rect, lerp_space_scroll_v,
                             &session->scroll_top, &session->scroll_bottom,
                             target->scroll_original.min_y, target->scroll_original.max_y);
        scroll_v = 0;
        break;
        
        case guicom_scrollable_invisible:
        Assert(session->is_scrollable);
        Assert(!section->overlapped);
        always_give_to_user = 1;
        break;
        
        case guicom_scrollable_bottom:
        Assert(session->is_scrollable);
        Assert(!section->overlapped);
        give_to_user = 1;
        gui_scrollbar_bottom(session->scroll_rect, &rect);
        scroll_v = 0;
        break;

        case guicom_begin_scrollable_section:
        always_give_to_user = 1;
        session->scrollable_items_bottom = 0;
        rect = gui_layout_top_bottom(session, y, session->full_rect.y1);
        end_v = rect.y1;
        break;

        case guicom_end_scrollable_section:
        always_give_to_user = 1;
        session->suggested_min_y =
            -(f32)(gui_session_get_eclipsed_y(session) -
                   gui_session_get_current_top(session));
        session->suggested_max_y =
            (f32)(session->scrollable_items_bottom -
                  session->full_rect.y1 * .5f);
        if (session->suggested_max_y < 0){
            session->suggested_max_y = 0;
        }
        break;
    }
    
    if (do_layout){
        if (session->list.in_list && is_list_item){
            i32 list_i = session->list.index - 1;
            i32_Rect region = target->region_updated;
            
            if (rect.y0 - target_v >= region.y0 &&
                rect.y1 - target_v <= region.y1){
                if (list_i < target->list_view_min){
                    target->list_view_min = list_i;
                }
                if (list_i+1 > target->list_view_max){
                    target->list_view_max = list_i+1;
                }
            }
        }
        
        if (give_to_user){
            if (session->is_scrollable){
                session->scrollable_items_bottom =
                    Max(session->scrollable_items_bottom, rect.y1);
            }
            
            rect.y0 -= scroll_v;
            rect.y1 -= scroll_v;
            
            if (rect.y1 > session->full_rect.y0){
                session->rect = rect;
            }
            else{
                give_to_user = 0;
                result.screen_orientation = -1;
            }
        }
        
        if (end_section){
            gui_section_end_item(end_section, end_v);
        }
        
        // TODO(allen): Why is this here, is there a particular reason?
        if (y - scroll_v >= session->full_rect.y1){
            give_to_user = 0;
            result.screen_orientation = 1;
        }
    }
    
    session->clip_y = gui_session_get_eclipsed_y(session);
    
    result.has_info = (give_to_user || always_give_to_user);
    
    return(result);
}

struct GUI_View_Jump{
    f32 view_min;
    f32 view_max;
};

internal GUI_View_Jump
gui_compute_view_jump(i32_Rect scroll_region, i32_Rect position){
    GUI_View_Jump jump = {0};
    i32 region_h = scroll_region.y1 - scroll_region.y0;
    jump.view_min = (f32)position.y1 - region_h - scroll_region.y0;
    jump.view_max = (f32)position.y0 - scroll_region.y0;
    return(jump);
}

internal void
gui_do_jump(GUI_Target *target, GUI_View_Jump jump){
    GUI_Scroll_Vars vars = gui_current_scroll_vars(target);
    if (vars.target_y < jump.view_min){
        vars.target_y = jump.view_min;
        gui_post_scroll_vars(target, &vars);
    }
    else if (vars.target_y > jump.view_max){
        vars.target_y = jump.view_max;
        gui_post_scroll_vars(target, &vars);
    }
}

internal void
gui_standard_list(GUI_Target *target, GUI_id id, i32_Rect scroll_region,
    Key_Summary *keys, i32 *list_i, GUI_Item_Update *update){
    
    if (update->has_adjustment){
        *list_i = update->adjustment_value;
    }
    
    if (update->has_index_position){
        GUI_View_Jump jump =
            gui_compute_view_jump(scroll_region, update->index_position);
        jump.view_min += 45.f;
        jump.view_max -= 45.f;
        gui_do_jump(target, jump);
    }
    
    b32 indirectly_activate = 0;
    for (i32 j = 0; j < keys->count; ++j){
        i16 key = keys->keys[j].keycode;
        switch (key){
            case key_up:
            --*list_i;
            break;
            
            case key_down:
            ++*list_i;
            break;
            
            case '\n': case '\t':
            indirectly_activate = 1;
            break;
        }
    }
    
    gui_rollback(target, update);
    gui_begin_list(target, id, *list_i, indirectly_activate, 0, 0);
}

// BOTTOM

