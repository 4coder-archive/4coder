/*
* Fancy string - immediate mode renderer for colored strings
*/

// TOP

function FColor
fcolor_argb(ARGB_Color color){
    FColor result = {};
    result.argb = color;
    if (result.a_byte == 0){
        result.argb = 0;
    }
    return(result);
}
function FColor
fcolor_argb(Vec4_f32 color){
    return(fcolor_argb(pack_color(color)));
}
function FColor
fcolor_argb(f32 r, f32 g, f32 b, f32 a){
    return(fcolor_argb(pack_color(V4f32(r, g, b, a))));
}

function FColor
fcolor_id(Managed_ID id){
    FColor result = {};
    result.id = (ID_Color)id;
    return(result);
}

function FColor
fcolor_id(Managed_ID id, u32 sub_index){
    FColor result = {};
    result.id = (ID_Color)id;
    result.sub_index = (u8)sub_index;
    return(result);
}

function ARGB_Color
argb_color_blend(ARGB_Color a, f32 at, ARGB_Color b, f32 bt){
    Vec4_f32 av = unpack_color(a);
    Vec4_f32 bv = unpack_color(b);
    Vec4_f32 value = at*av + bt*bv;
    return(pack_color(value));
}
function ARGB_Color
argb_color_blend(ARGB_Color a, f32 t, ARGB_Color b){
    return(argb_color_blend(a, 1.f - t, b, t));
}

function ARGB_Color
fcolor_resolve(FColor color){
    ARGB_Color result = 0;
    if (color.a_byte == 0){
        if (color.id != 0){
            result = finalize_color(color.id, color.sub_index);
        }
    }
    else{
        result = color.argb;
    }
    return(result);
}

function FColor
fcolor_change_alpha(FColor color, f32 alpha){
    Vec4_f32 v = unpack_color(fcolor_resolve(color));
    v.a = alpha;
    return(fcolor_argb(pack_color(v)));
}
function FColor
fcolor_blend(FColor a, f32 at, FColor b, f32 bt){
    ARGB_Color a_argb = fcolor_resolve(a);
    ARGB_Color b_argb = fcolor_resolve(b);
    return(fcolor_argb(argb_color_blend(a_argb, at, b_argb, bt)));
}
function FColor
fcolor_blend(FColor a, f32 t, FColor b){
    return(fcolor_blend(a, 1.f - t, b, t));
}

function FColor
fcolor_zero(void){
    FColor result = {};
    return(result);
}

function b32
fcolor_is_valid(FColor color){
    return(color.argb != 0);
}

////////////////////////////////

function void
push_fancy_string(Fancy_Line *line, Fancy_String *string){
    sll_queue_push(line->first, line->last, string);
}

function void
push_fancy_line(Fancy_Block *block, Fancy_Line *line){
    sll_queue_push(block->first, block->last, line);
    block->line_count += 1;
}

////////////////////////////////

function Fancy_String*
fill_fancy_string(Fancy_String *ptr, Face_ID face, FColor fore, f32 pre_margin, f32 post_margin,
                  String_Const_u8 value){
    ptr->value = value;
    ptr->face = face;
    ptr->fore = fore;
    ptr->pre_margin = pre_margin;
    ptr->post_margin = post_margin;
    return(ptr);
}

function Fancy_String*
push_fancy_string(Arena *arena, Fancy_Line *line, Face_ID face, FColor fore,
                  f32 pre_margin, f32 post_margin, String_Const_u8 value){
    Fancy_String *result = push_array_zero(arena, Fancy_String, 1);
    fill_fancy_string(result, face, fore, pre_margin, post_margin, value);
    if (line != 0){
        push_fancy_string(line, result);
    }
    return(result);
}

function Fancy_String*
push_fancy_string(Arena *arena, Fancy_Line *line, Face_ID face, FColor fore,
                  String_Const_u8 value){
    return(push_fancy_string(arena, line, face, fore, 0, 0, value));
}
function Fancy_String*
push_fancy_string(Arena *arena, Fancy_Line *line, Face_ID face,
                  f32 pre_margin, f32 post_margin, String_Const_u8 value){
    return(push_fancy_string(arena, line, face, fcolor_zero(),
                             pre_margin, post_margin, value));
}
function Fancy_String*
push_fancy_string(Arena *arena, Fancy_Line *line, FColor fore,
                  f32 pre_margin, f32 post_margin, String_Const_u8 value){
    return(push_fancy_string(arena, line, 0, fore, pre_margin, post_margin, value));
}
function Fancy_String*
push_fancy_string(Arena *arena, Fancy_Line *line, Face_ID face, String_Const_u8 value){
    return(push_fancy_string(arena, line, face, fcolor_zero(), 0, 0, value));
}
function Fancy_String*
push_fancy_string(Arena *arena, Fancy_Line *line, FColor color, String_Const_u8 value){
    return(push_fancy_string(arena, line, 0, color, 0, 0, value));
}
function Fancy_String*
push_fancy_string(Arena *arena, Fancy_Line *line, f32 pre_margin, f32 post_margin,
                  String_Const_u8 value){
    return(push_fancy_string(arena, line, 0, fcolor_zero(), pre_margin, post_margin,
                             value));
}
function Fancy_String*
push_fancy_string(Arena *arena, Fancy_Line *line, String_Const_u8 value){
    return(push_fancy_string(arena, line, 0, fcolor_zero(), 0, 0, value));
}

////////////////////////////////

function Fancy_String*
push_fancy_stringfv(Arena *arena, Fancy_Line *line, Face_ID face, FColor fore,
                    f32 pre_margin, f32 post_margin,
                    char *format, va_list args){
    return(push_fancy_string(arena, line, face, fore, pre_margin, post_margin,
                             push_u8_stringfv(arena, format, args)));
}
function Fancy_String*
push_fancy_stringfv(Arena *arena, Fancy_Line *line, Face_ID face, FColor fore,
                    char *format, va_list args){
    return(push_fancy_stringfv(arena, line, face, fore, 0, 0, format, args));
}
function Fancy_String*
push_fancy_stringfv(Arena *arena, Fancy_Line *line, Face_ID face,
                    f32 pre_margin, f32 post_margin,
                    char *format, va_list args){
    return(push_fancy_stringfv(arena, line, face, fcolor_zero(),
                               pre_margin, post_margin, format, args));
}
function Fancy_String*
push_fancy_stringfv(Arena *arena, Fancy_Line *line, FColor fore,
                    f32 pre_margin, f32 post_margin,
                    char *format, va_list args){
    return(push_fancy_stringfv(arena, line, 0, fore, pre_margin, post_margin,
                               format, args));
}
function Fancy_String*
push_fancy_stringfv(Arena *arena, Fancy_Line *line, Face_ID face,
                    char *format, va_list args){
    return(push_fancy_stringfv(arena, line, face, fcolor_zero(), 0, 0,
                               format, args));
}
function Fancy_String*
push_fancy_stringfv(Arena *arena, Fancy_Line *line, FColor color,
                    char *format, va_list args){
    return(push_fancy_stringfv(arena, line, 0, color, 0, 0, format, args));
}
function Fancy_String*
push_fancy_stringfv(Arena *arena, Fancy_Line *line, f32 pre_margin, f32 post_margin,
                    char *format, va_list args){
    return(push_fancy_stringfv(arena, line, 0, fcolor_zero(), pre_margin, post_margin,
                               format, args));
}
function Fancy_String*
push_fancy_stringfv(Arena *arena, Fancy_Line *line,
                    char *format, va_list args){
    return(push_fancy_stringfv(arena, line, 0, fcolor_zero(), 0, 0, format, args));
}

#define StringFBegin() va_list args; va_start(args, format)
#define StringFPass(N) Fancy_String *result = N
#define StringFEnd() va_end(args); return(result)

function Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_Line *line, Face_ID face, FColor fore,
                   f32 pre_margin, f32 post_margin,
                   char *format, ...){
    StringFBegin();
    StringFPass(push_fancy_string(arena, line, face, fore, pre_margin, post_margin,
                                  push_u8_stringfv(arena, format, args)));
    StringFEnd();
}
function Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_Line *line, Face_ID face, FColor fore,
                   char *format, ...){
    StringFBegin();
    StringFPass(push_fancy_stringfv(arena, line, face, fore, 0, 0, format, args));
    StringFEnd();
}
function Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_Line *line, Face_ID face,
                   f32 pre_margin, f32 post_margin,
                   char *format, ...){
    StringFBegin();
    StringFPass(push_fancy_stringfv(arena, line, face, fcolor_zero(),
                                    pre_margin, post_margin, format, args));
    StringFEnd();
}
function Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_Line *line, FColor fore,
                   f32 pre_margin, f32 post_margin,
                   char *format, ...){
    StringFBegin();
    StringFPass(push_fancy_stringfv(arena, line, 0, fore, pre_margin, post_margin,
                                    format, args));
    StringFEnd();
}
function Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_Line *line, Face_ID face,
                   char *format, ...){
    StringFBegin();
    StringFPass(push_fancy_stringfv(arena, line, face, fcolor_zero(), 0, 0,
                                    format, args));
    StringFEnd();
}
function Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_Line *line, FColor color,
                   char *format, ...){
    StringFBegin();
    StringFPass(push_fancy_stringfv(arena, line, 0, color, 0, 0, format, args));
    StringFEnd();
}
function Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_Line *line, f32 pre_margin, f32 post_margin,
                   char *format, ...){
    StringFBegin();
    StringFPass(push_fancy_stringfv(arena, line, 0, fcolor_zero(),
                                    pre_margin, post_margin, format, args));
    StringFEnd();
}
function Fancy_String*
push_fancy_stringf(Arena *arena, Fancy_Line *line,
                   char *format, ...){
    StringFBegin();
    StringFPass(push_fancy_stringfv(arena, line, 0, fcolor_zero(), 0, 0, format, args));
    StringFEnd();
}

////////////////////////////////

function Fancy_String*
push_fancy_string_fixed(Arena *arena, Fancy_Line *line, Face_ID face, FColor fore,
                        f32 pre_margin, f32 post_margin,
                        String_Const_u8 value, i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, face, fore, pre_margin, post_margin,
                                  "%-*.*s", max, string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, face, fore, pre_margin, post_margin,
                                  "%-*.*s...", max - 3, string_expand(value)));
    }
}
function Fancy_String*
push_fancy_string_fixed(Arena *arena, Fancy_Line *line, Face_ID face, FColor fore,
                        String_Const_u8 value, i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, face, fore, 0.f, 0.f,
                                  "%-*.*s", max, string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, face, fore, 0.f, 0.f,
                                  "%-*.*s...", max - 3, string_expand(value)));
    }
}
function Fancy_String*
push_fancy_string_fixed(Arena *arena, Fancy_Line *line, Face_ID face,
                        f32 pre_margin, f32 post_margin, String_Const_u8 value,
                        i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, face, fcolor_zero(),
                                  pre_margin, post_margin,
                                  "%-*.*s", max, string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, face, fcolor_zero(),
                                  pre_margin, post_margin,
                                  "%-*.*s...", max - 3, string_expand(value)));
    }
}
function Fancy_String*
push_fancy_string_fixed(Arena *arena, Fancy_Line *line, FColor fore,
                        f32 pre_margin, f32 post_margin, String_Const_u8 value,
                        i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, 0, fore, pre_margin, post_margin,
                                  "%-*.*s", max, string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, 0, fore, pre_margin, post_margin,
                                  "%-*.*s...", max - 3, string_expand(value)));
    }
}
function Fancy_String*
push_fancy_string_fixed(Arena *arena, Fancy_Line *line, Face_ID face,
                        String_Const_u8 value, i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, face, fcolor_zero(), 0.f, 0.f,
                                  "%-*.*s", max, string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, face, fcolor_zero(), 0.f, 0.f,
                                  "%-*.*s...", max - 3, string_expand(value)));
    }
}
function Fancy_String*
push_fancy_string_fixed(Arena *arena, Fancy_Line *line, FColor fore,
                        String_Const_u8 value, i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, (Face_ID)0, fore, 0.f, 0.f,
                                  "%-*.*s", max, string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, (Face_ID)0, fore, 0.f, 0.f,
                                  "%-*.*s...", max - 3, string_expand(value)));
    }
}
function Fancy_String*
push_fancy_string_fixed(Arena *arena, Fancy_Line *line,
                        f32 pre_margin, f32 post_margin, String_Const_u8 value,
                        i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, (Face_ID)0, fcolor_zero(),
                                  pre_margin, post_margin,
                                  "%-*.*s", max, string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, (Face_ID)0, fcolor_zero(),
                                  pre_margin, post_margin,
                                  "%-*.*s...", max - 3, string_expand(value)));
    }
}
function Fancy_String*
push_fancy_string_fixed(Arena *arena, Fancy_Line *line, String_Const_u8 value,
                        i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, (Face_ID)0, fcolor_zero(), 0.f, 0.f,
                                  "%-*.*s", max, string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, (Face_ID)0, fcolor_zero(), 0.f, 0.f,
                                  "%-*.*s...", max - 3, string_expand(value)));
    }
}

function Fancy_String*
push_fancy_string_trunc(Arena *arena, Fancy_Line *line, Face_ID face, FColor fore,
                        f32 pre_margin, f32 post_margin,
                        String_Const_u8 value, i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, face, fore, pre_margin, post_margin,
                                  "%.*s", string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, face, fore, pre_margin, post_margin,
                                  "%.*s...", max - 3, value.str));
    }
}
function Fancy_String*
push_fancy_string_trunc(Arena *arena, Fancy_Line *line, Face_ID face, FColor fore,
                        String_Const_u8 value, i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, face, fore, 0.f, 0.f,
                                  "%.*s", string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, face, fore, 0.f, 0.f,
                                  "%.*s...", max - 3, value.str));
    }
}
function Fancy_String*
push_fancy_string_trunc(Arena *arena, Fancy_Line *line, Face_ID face,
                        f32 pre_margin, f32 post_margin, String_Const_u8 value,
                        i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, face, fcolor_zero(),
                                  pre_margin, post_margin,
                                  "%.*s", string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, face, fcolor_zero(),
                                  pre_margin, post_margin,
                                  "%.*s...", max - 3, value.str));
    }
}
function Fancy_String*
push_fancy_string_trunc(Arena *arena, Fancy_Line *line, FColor fore,
                        f32 pre_margin, f32 post_margin, String_Const_u8 value,
                        i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, 0, fore, pre_margin, post_margin,
                                  "%.*s", string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, 0, fore, pre_margin, post_margin,
                                  "%.*s...", max - 3, value.str));
    }
}
function Fancy_String*
push_fancy_string_trunc(Arena *arena, Fancy_Line *line, Face_ID face,
                        String_Const_u8 value, i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, face, fcolor_zero(), 0.f, 0.f,
                                  "%.*s", string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, face, fcolor_zero(), 0.f, 0.f,
                                  "%.*s...", max - 3, value.str));
    }
}
function Fancy_String*
push_fancy_string_trunc(Arena *arena, Fancy_Line *line, FColor fore,
                        String_Const_u8 value, i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, (Face_ID)0, fore, 0.f, 0.f,
                                  "%.*s", string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, (Face_ID)0, fore, 0.f, 0.f,
                                  "%.*s...", max - 3, value.str));
    }
}
function Fancy_String*
push_fancy_string_trunc(Arena *arena, Fancy_Line *line,
                        f32 pre_margin, f32 post_margin, String_Const_u8 value,
                        i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, (Face_ID)0, fcolor_zero(),
                                  pre_margin, post_margin,
                                  "%.*s", string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, (Face_ID)0, fcolor_zero(),
                                  pre_margin, post_margin,
                                  "%.*s...", max - 3, value.str));
    }
}
function Fancy_String*
push_fancy_string_trunc(Arena *arena, Fancy_Line *line, String_Const_u8 value,
                        i32 max){
    if (value.size <= max){
        return(push_fancy_stringf(arena, line, (Face_ID)0, fcolor_zero(), 0.f, 0.f,
                                  "%.*s", string_expand(value)));
    }
    else{
        return(push_fancy_stringf(arena, line, (Face_ID)0, fcolor_zero(), 0.f, 0.f,
                                  "%.*s...", max - 3, value.str));
    }
}

////////////////////////////////

function Fancy_Line*
push_fancy_line(Arena *arena, Fancy_Block *block, Face_ID face, FColor fore,
                String_Const_u8 text){
    Fancy_Line *line = push_array_zero(arena, Fancy_Line, 1);
    line->face = face;
    line->fore = fore;
    if (text.size != 0){
        push_fancy_string(arena, line, text);
    }
    if (block != 0){
        push_fancy_line(block, line);
    }
    return(line);
}
function Fancy_Line*
push_fancy_line(Arena *arena, Fancy_Block *block, Face_ID face, FColor fcolor){
    return(push_fancy_line(arena, block, face, fcolor, SCu8()));
}
function Fancy_Line*
push_fancy_line(Arena *arena, Fancy_Block *block, Face_ID face, String_Const_u8 val){
    return(push_fancy_line(arena, block, face, fcolor_zero(), val));
}
function Fancy_Line*
push_fancy_line(Arena *arena, Fancy_Block *block, FColor color, String_Const_u8 val){
    return(push_fancy_line(arena, block, 0, color, val));
}
function Fancy_Line*
push_fancy_line(Arena *arena, Fancy_Block *block, Face_ID face){
    return(push_fancy_line(arena, block, face, fcolor_zero(), SCu8()));
}
function Fancy_Line*
push_fancy_line(Arena *arena, Fancy_Block *block, FColor color){
    return(push_fancy_line(arena, block, 0, color, SCu8()));
}
function Fancy_Line*
push_fancy_line(Arena *arena, Fancy_Block *block, String_Const_u8 val){
    return(push_fancy_line(arena, block, 0, fcolor_zero(), val));
}
function Fancy_Line*
push_fancy_line(Arena *arena, Fancy_Block *block){
    return(push_fancy_line(arena, block, 0, fcolor_zero(), SCu8()));
}

////////////////////////////////

function f32
get_fancy_string_width__inner(Application_Links *app, Face_ID face,
                              Fancy_String *string){
    f32 result = 0.f;
    for (;string != 0;
         string = string->next){
        Face_ID use_face = face;
        if (string->face != 0){
            use_face = string->face;
        }
        if (use_face != 0){
            result += get_string_advance(app, use_face, string->value);
            Face_Metrics metrics = get_face_metrics(app, use_face);
            f32 normal_advance = metrics.normal_advance;
            result += (string->pre_margin + string->post_margin)*normal_advance;
        }
    }
    return(result);
}

function f32
get_fancy_string_height__inner(Application_Links *app, Face_ID face, Fancy_String *string){
    f32 result = 0.f;
    if (face != 0){
        Face_Metrics metrics = get_face_metrics(app, face);
        result = metrics.line_height;
    }
    for (;string != 0;
         string = string->next){
        if (string->face != 0){
            Face_ID use_face = string->face;
            Face_Metrics metrics = get_face_metrics(app, use_face);
            result = Max(result, metrics.line_height);
        }
    }
    return(result);
}

function f32
get_fancy_string_text_height__inner(Application_Links *app, Face_ID face, Fancy_String *string){
    f32 result = 0.f;
    if (face != 0){
        Face_Metrics metrics = get_face_metrics(app, face);
        result = metrics.text_height;
    }
    for (;string != 0;
         string = string->next){
        if (string->face != 0){
            Face_ID use_face = string->face;
            Face_Metrics metrics = get_face_metrics(app, use_face);
            result = Max(result, metrics.text_height);
        }
    }
    return(result);
}

function Vec2_f32
draw_fancy_string__inner(Application_Links *app, Face_ID face, FColor fore, Fancy_String *first_string, Vec2_f32 p, u32 flags, Vec2_f32 delta){
    f32 base_line = 0.f;
    for (Fancy_String *string = first_string;
         string != 0;
         string = string->next){
        Face_ID use_face = face;
        if (string->face != 0){
            use_face = string->face;
        }
        if (use_face != 0){
            Face_Metrics metrics = get_face_metrics(app, use_face);
            base_line = Max(base_line, metrics.ascent);
        }
    }
    
    Vec2_f32 down_delta = V2f32(-delta.y, delta.x);
    for (Fancy_String *string = first_string;
         string != 0;
         string = string->next){
        Face_ID use_face = face;
        if (string->face != 0){
            use_face = string->face;
        }
        FColor use_fore = fore;
        if (fcolor_is_valid(string->fore)){
            use_fore = string->fore;
        }
        if (use_face != 0){
            ARGB_Color use_argb = fcolor_resolve(use_fore);
            Face_Metrics metrics = get_face_metrics(app, use_face);
            f32 down_shift = (base_line - metrics.ascent);
            down_shift = clamp_bot(0.f, down_shift);
            Vec2_f32 p_shift = down_shift*down_delta;
            Vec2_f32 p_shifted = p + p_shift;
            
            if (fcolor_is_valid(use_fore)){
                Vec2_f32 margin_delta = delta*metrics.normal_advance;
                p_shifted += margin_delta*string->pre_margin;
                p_shifted = draw_string_oriented(app, use_face, use_argb, string->value, p_shifted, flags, delta);
                p_shifted += margin_delta*string->post_margin;
            }
            else{
                f32 adv =
                    (string->pre_margin + string->post_margin)*metrics.normal_advance;
                adv += get_string_advance(app, use_face, string->value);
                p_shifted += adv*delta;
            }
            
            p = p_shifted - p_shift;
        }
    }
    return(p);
}

function f32
get_fancy_string_width(Application_Links *app, Face_ID face,
                       Fancy_String *string){
    Fancy_String *next = string->next;
    string->next = 0;
    f32 result = get_fancy_string_width__inner(app, face, string);
    string->next = next;
    return(result);
}

function f32
get_fancy_string_height(Application_Links *app, Face_ID face,
                        Fancy_String *string){
    Fancy_String *next = string->next;
    string->next = 0;
    f32 result = get_fancy_string_height__inner(app, face, string);
    string->next = next;
    return(result);
}

function f32
get_fancy_string_text_height(Application_Links *app, Face_ID face,
                             Fancy_String *string){
    Fancy_String *next = string->next;
    string->next = 0;
    f32 result = get_fancy_string_text_height__inner(app, face, string);
    string->next = next;
    return(result);
}

function Vec2_f32
get_fancy_string_dim(Application_Links *app, Face_ID face, Fancy_String *string){
    Fancy_String *next = string->next;
    string->next = 0;
    Vec2_f32 result = V2f32(get_fancy_string_width__inner(app, face, string),
                            get_fancy_string_height__inner(app, face, string));
    string->next = next;
    return(result);
}

function Vec2_f32
draw_fancy_string(Application_Links *app, Face_ID face, FColor fore,
                  Fancy_String *string, Vec2_f32 p, u32 flags, Vec2_f32 delta){
    Fancy_String *next = string->next;
    string->next = 0;
    Vec2_f32 result = draw_fancy_string__inner(app, face, fore, string, p, flags, delta);
    string->next = next;
    return(result);
}

function f32
get_fancy_line_width(Application_Links *app, Face_ID face, Fancy_Line *line){
    f32 result = 0.f;
    if (line != 0){
        if (line->face != 0){
            face = line->face;
        }
        result = get_fancy_string_width__inner(app, face, line->first);
    }
    return(result);
}

function f32
get_fancy_line_height(Application_Links *app, Face_ID face, Fancy_Line *line){
    f32 result = 0.f;
    if (line != 0){
        if (line->face != 0){
            face = line->face;
        }
        result = get_fancy_string_height__inner(app, face, line->first);
    }
    return(result);
}

function f32
get_fancy_line_text_height(Application_Links *app, Face_ID face, Fancy_Line *line){
    f32 result = 0.f;
    if (line != 0){
        if (line->face != 0){
            face = line->face;
        }
        result = get_fancy_string_text_height__inner(app, face, line->first);
    }
    return(result);
}

function Vec2_f32
get_fancy_line_dim(Application_Links *app, Face_ID face, Fancy_Line *line){
    Vec2_f32 result = {};
    if (line != 0){
        if (line->face != 0){
            face = line->face;
        }
        result = V2f32(get_fancy_string_width__inner(app, face, line->first), get_fancy_string_height__inner(app, face, line->first));
    }
    return(result);
}

function Vec2_f32
draw_fancy_line(Application_Links *app, Face_ID face, FColor fore,
                Fancy_Line *line, Vec2_f32 p, u32 flags, Vec2_f32 delta){
    Vec2_f32 result = {};
    if (line != 0){
        if (line->face != 0){
            face = line->face;
        }
        if (fcolor_is_valid(line->fore)){
            fore = line->fore;
        }
        result = draw_fancy_string__inner(app, face, fore, line->first, p, flags, delta);
    }
    return(result);
}

function f32
get_fancy_block_width(Application_Links *app, Face_ID face, Fancy_Block *block){
    f32 width = 0.f;
    for (Fancy_Line *node = block->first;
         node != 0;
         node = node->next){
        f32 w = get_fancy_line_width(app, face, node);
        width = Max(width, w);
    }
    return(width);
}

function f32
get_fancy_block_height(Application_Links *app, Face_ID face, Fancy_Block *block){
    f32 height = 0.f;
    for (Fancy_Line *node = block->first;
         node != 0;
         node = node->next){
        height += get_fancy_line_height(app, face, node);
    }
    return(height);
}

function Vec2_f32
get_fancy_block_dim(Application_Links *app, Face_ID face, Fancy_Block *block){
    Vec2_f32 result = {};
    result.x = get_fancy_block_width(app, face, block);
    result.y = get_fancy_block_height(app, face, block);
    return(result);
}

function void
draw_fancy_block(Application_Links *app, Face_ID face, FColor fore,
                 Fancy_Block *block, Vec2_f32 p, u32 flags, Vec2_f32 delta){
    for (Fancy_Line *node = block->first;
         node != 0;
         node = node->next){
        draw_fancy_line(app, face, fore, node, p, flags, delta);
        p.y += get_fancy_line_height(app, face, node);
    }
}

function Vec2_f32
draw_fancy_string(Application_Links *app, Face_ID face, FColor fore,
                  Fancy_String *string, Vec2_f32 p){
    return(draw_fancy_string(app, face, fore, string, p, 0, V2f32(1.f, 0.f)));
}

function Vec2_f32
draw_fancy_string(Application_Links *app, Fancy_String *string, Vec2_f32 p){
    return(draw_fancy_string(app, 0, fcolor_zero(), string, p, 0, V2f32(1.f, 0.f)));
}

function Vec2_f32
draw_fancy_line(Application_Links *app, Face_ID face, FColor fore,
                Fancy_Line *line, Vec2_f32 p){
    return(draw_fancy_line(app, face, fore, line, p, 0, V2f32(1.f, 0.f)));
}

function void
draw_fancy_block(Application_Links *app, Face_ID face, FColor fore,
                 Fancy_Block *block, Vec2_f32 p){
    draw_fancy_block(app, face, fore, block, p, 0, V2f32(1.f, 0.f));
}

////////////////////////////////

// TODO(allen): beta: color palette
global FColor f_white      = fcolor_argb(1.0f, 1.0f, 1.0f, 1.0f);
global FColor f_light_gray = fcolor_argb(0.7f, 0.7f, 0.7f, 1.0f);
global FColor f_gray       = fcolor_argb(0.5f, 0.5f, 0.5f, 1.0f);
global FColor f_dark_gray  = fcolor_argb(0.3f, 0.3f, 0.3f, 1.0f);
global FColor f_black      = fcolor_argb(0.0f, 0.0f, 0.0f, 1.0f);
global FColor f_red        = fcolor_argb(1.0f, 0.0f, 0.0f, 1.0f);
global FColor f_green      = fcolor_argb(0.0f, 1.0f, 0.0f, 1.0f);
global FColor f_blue       = fcolor_argb(0.0f, 0.0f, 1.0f, 1.0f);
global FColor f_yellow     = fcolor_argb(1.0f, 1.0f, 0.0f, 1.0f);
global FColor f_pink       = fcolor_argb(1.0f, 0.0f, 1.0f, 1.0f);
global FColor f_cyan       = fcolor_argb(0.0f, 1.0f, 1.0f, 1.0f);

// BOTTOM

