/*
* Fancy string - immediate mode renderer for colored strings
*/

// TOP

static Fancy_Color
fancy_blend(id_color a, f32 t, id_color b){
    Fancy_Color result = {};
    result.index_a = (u16)a;
    result.index_b = (u16)b;
    result.table_a = 1;
    result.table_b = 1;
    result.c_b = (u8)(clamp(0, 255.0f*t, 255.0f));
    result.c_a = 255 - result.c_b;
    return(result);
}

static Fancy_Color
fancy_id(id_color a){
    Fancy_Color result = {};
    result.index_a = (u16)a;
    result.index_b = 0;
    result.table_a = 1;
    result.table_b = 0;
    result.c_a = 255;
    result.c_b = 0;
    return(result);
}

static Fancy_Color
fancy_rgba(argb_color color){
    Fancy_Color result = {};
    result.rgba = color;
    result.code = 0;
    return(result);
}

static Fancy_Color
fancy_rgba(f32 r, f32 g, f32 b, f32 a){
    Fancy_Color result = fancy_rgba(pack_color4(V4(r, g, b, a)));
    return(result);
}

static Fancy_Color
fancy_resolve_to_rgba(Application_Links *app, Fancy_Color source){
    if (source.code != 0){
        Vec4 a = unpack_color4(finalize_color(app, source.index_a));
        Vec4 b = unpack_color4(finalize_color(app, source.index_b));
        
        f32 ca = (f32)source.c_a/255.0f;
        f32 cb = (f32)source.c_b/255.0f;
        
        Vec4 value = ca*a + cb*b;
        
        source.rgba = pack_color4(value);
        source.code = 0;
    }
    return(source);
}

static Fancy_Color
fancy_pass_through(void){
    Fancy_Color result = {};
    return(result);
}

static int_color
int_color_from(Application_Links *app, Fancy_Color source){
    int_color result = {};
    if ((source.c_a == 255) && (source.c_b == 0)){
        result = source.index_a;
    }
    else{
        source = fancy_resolve_to_rgba(app, source);
        result = source.rgba;
    }
    return(result);
}

static b32
is_valid(Fancy_Color source){
    b32 result = !((source.code == 0) && (source.rgba == 0));
    return(result);
}

static void
fancy_string_list_push(Fancy_String_List *list, Fancy_String *string){
    list->last = (list->last ? list->last->next : list->first) = string;
}

static Fancy_String *
push_fancy_string(Arena *arena, Fancy_String_List *list, Fancy_Color fore, Fancy_Color back, String_Const_u8 value){
    Fancy_String *result = push_array_zero(arena, Fancy_String, 1);
    result->value = push_string_copy(arena, value);
    result->fore = fore;
    result->back = back;
    if (list != 0){
        fancy_string_list_push(list, result);
    }
    return(result);
}

static Fancy_String *
push_fancy_string(Arena *arena, Fancy_Color fore, Fancy_Color back, String_Const_u8 value){
    return(push_fancy_string(arena, 0, fore, back, value));
}

static Fancy_String *
push_fancy_string(Arena *arena, Fancy_String_List *list, Fancy_Color fore, String_Const_u8 value){
    return(push_fancy_string(arena, list, fore, fancy_pass_through(), value));
}

static Fancy_String *
push_fancy_string(Arena *arena, Fancy_Color fore, String_Const_u8 value){
    return(push_fancy_string(arena, 0, fore, fancy_pass_through(), value));
}

static Fancy_String *
push_fancy_string(Arena *arena, Fancy_String_List *list, String_Const_u8 value){
    return(push_fancy_string(arena, list, fancy_pass_through(), fancy_pass_through(), value));
}

static Fancy_String *
push_fancy_string(Arena *arena, String_Const_u8 value){
    return(push_fancy_string(arena, 0, fancy_pass_through(), fancy_pass_through(), value));
}

static Fancy_String*
push_fancy_stringfv(Arena *arena, Fancy_String_List *list, Fancy_Color fore, Fancy_Color back, char *format, va_list args){
    String_Const_u8 str = push_u8_stringfv(arena, format, args);
    Fancy_String *result = 0;
    if (str.size > 0){
        result = push_fancy_string(arena, list, fore, back, str);
    }
    return(result);
}

static Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_String_List *list, Fancy_Color fore, Fancy_Color back, char *format, ...){
    va_list args;
    va_start(args, format);
    Fancy_String *result = push_fancy_stringfv(arena, list, fore, back, format, args);
    va_end(args);
    return(result);
}

static Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_String_List *list, Fancy_Color fore, char *format, ...){
    va_list args;
    va_start(args, format);
    Fancy_String *result = push_fancy_stringfv(arena, list, fore, fancy_pass_through(), format, args);
    va_end(args);
    return(result);
}

static Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_String_List *list, char *format, ...){
    va_list args;
    va_start(args, format);
    Fancy_String *result = push_fancy_stringfv(arena, list, fancy_pass_through(), fancy_pass_through(), format, args);
    va_end(args);
    return(result);
}

static Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_Color fore, Fancy_Color back, char *format, ...){
    va_list args;
    va_start(args, format);
    Fancy_String *result = push_fancy_stringfv(arena, 0, fore, back, format, args);
    va_end(args);
    return(result);
}

static Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_Color fore, char *format, ...){
    va_list args;
    va_start(args, format);
    Fancy_String *result = push_fancy_stringfv(arena, 0, fore, fancy_pass_through(), format, args);
    va_end(args);
    return(result);
}

static Fancy_String*
push_fancy_stringf(Arena *arena, char *format, ...){
    va_list args;
    va_start(args, format);
    Fancy_String *result = push_fancy_stringfv(arena, 0, fancy_pass_through(), fancy_pass_through(), format, args);
    va_end(args);
    return(result);
}

static Fancy_String_List
fancy_string_list_single(Fancy_String *fancy_string){
    Fancy_String_List list = {};
    list.first = fancy_string;
    list.last = fancy_string;
    return(list);
}

static Vec2_f32
draw_fancy_string(Application_Links *app, Face_ID font_id, Fancy_String *string,
                  Vec2_f32 P, int_color fore, int_color back, u32 flags, Vec2_f32 dP){
    for (;string != 0;
         string = string->next){
        Face_ID use_font_id = (string->font_id) ? string->font_id : font_id;
        int_color use_fore = is_valid(string->fore) ? int_color_from(app, string->fore) : fore;
        
        f32 adv = get_string_advance(app, use_font_id, string->value);
        
        // TODO(casey): need to fill the background here, but I don't know the line 
        // height, and I can't actually render filled shapes, so, like, I can't
        // properly do dP :(
        
        Face_Metrics metrics = get_face_metrics(app, font_id);
        
        P += (string->pre_margin*metrics.normal_advance)*dP;
        draw_string_oriented(app, use_font_id, string->value, P, use_fore, flags, dP);
        P += (adv + string->post_margin*metrics.normal_advance)*dP;
    }
    return(P);
}

static Vec2_f32
draw_fancy_string(Application_Links *app, Face_ID font_id, Fancy_String *string,
                  Vec2_f32 P, int_color fore, int_color back){
    return(draw_fancy_string(app, font_id, string, P, fore, back, 0, V2(1.f, 0.f)));
}

static f32
get_fancy_string_advance(Application_Links *app, Face_ID font_id, Fancy_String *string){
    f32 advance = 0.f;
    for (;string != 0;
         string = string->next){
        Face_ID use_font_id = (string->font_id) ? string->font_id : font_id;
        f32 adv = get_string_advance(app, use_font_id, string->value);
        Face_Metrics metrics = get_face_metrics(app, font_id);
        advance += (string->pre_margin + string->post_margin)*metrics.normal_advance + adv;
    }
    return(advance);
}

static void
draw_rectangle_fancy(Application_Links *app, Rect_f32 rect, Fancy_Color fancy_color){
    int_color color = int_color_from(app, fancy_color);
    draw_rectangle(app, rect, 0.f, color);
}

////////////////////////////////

// TODO(allen): beta: color palette
global Fancy_Color white      = fancy_rgba(1.0f, 1.0f, 1.0f, 1.0f);
global Fancy_Color light_gray = fancy_rgba(0.7f, 0.7f, 0.7f, 1.0f);
global Fancy_Color gray       = fancy_rgba(0.5f, 0.5f, 0.5f, 1.0f);
global Fancy_Color dark_gray  = fancy_rgba(0.3f, 0.3f, 0.3f, 1.0f);
global Fancy_Color black      = fancy_rgba(0.0f, 0.0f, 0.0f, 1.0f);
global Fancy_Color pink       = fancy_rgba(1.0f, 0.0f, 1.0f, 1.0f);
global Fancy_Color green      = fancy_rgba(0.0f, 1.0f, 0.0f, 1.0f);

// BOTTOM

