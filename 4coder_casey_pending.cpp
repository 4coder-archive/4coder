/* ========================================================================
$File: work/apps/4coder/custom/4coder_casey_pending.cpp $
$Date: 2019/03/30 23:57:38 UTC $
$Revision: 21 $
$Creator: Casey Muratori $
$Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
======================================================================== */

/* NOTE(casey):

Allen, this is a file full of things that I think 4coder should have provided natively, so unless there's something objectionable
about them, I think they could be merged?

*/

#include <stdarg.h>

static Range
clip_range_to_width(Range range, int32_t max_width) {
    Range result = range;
    if((result.start + max_width) < result.end) {
        result.end = result.start + max_width;
    }
    
    return(result);
}

#define CStrCase(a) case a: return(#a)
#define StringCase(a) case a: return(make_lit_string(#a))

// TODO(casey): Merge this with Allen's Token_Iterator when it's ready?
struct token_iterator
{
    Buffer_ID buffer;
    bool32 valid;
    int32_t index;
};

#if 0
static void
Append(Found_String_List *dest, Found_String_List source)
{
    if(dest->last)
    {
        dest->last->next = source.first;
        dest->last = source.last;
        dest->count += source.count;
    }
    else
    {
        *dest = source;
    }
}
#endif

static Vec2
center_of(f32_Rect a)
{
    Vec2 result;
    
    result.x = 0.5f*(a.x0 + a.x1);
    result.y = 0.5f*(a.y0 + a.y1);
    
    return(result);
}

static f32_Rect
f32_rect_from(i32_Rect a)
{
    f32_Rect result;
    
    result.x0 = (float)a.x0;
    result.x1 = (float)a.x1;
    result.y0 = (float)a.y0;
    result.y1 = (float)a.y1;
    
    return(result);
}

static i32_Rect
i32_rect_from(f32_Rect a)
{
    i32_Rect result;
    
    result.x0 = round32(a.x0);
    result.x1 = round32(a.x1);
    result.y0 = round32(a.y0);
    result.y1 = round32(a.y1);
    
    return(result);
}

static bool32
is_valid(Range range)
{
    bool32 result = (range.start < range.one_past_last);
    return(result);
}

static bool32
is_in(f32_Rect a, Vec2 p)
{
    bool32 result = ((p.x >= a.x0) &&
                     (p.x < a.x1) &&
                     (p.y >= a.y0) &&
                     (p.y < a.y1));
    return(result);
}

static bool32
is_visible(View_Summary *view, Vec2 p)
{
    bool32 result = is_in(f32_rect_from(view->render_region), p);
    return(result);
}

static token_iterator
iterate_tokens(Application_Links *app, Buffer_ID buffer, int32_t absolute_position)
{
    token_iterator iter = {};
    
    iter.buffer = buffer;
    
    Cpp_Get_Token_Result temp;
    iter.valid = buffer_get_token_index(app, buffer, absolute_position, &temp);
    if(iter.valid)
    {
        iter.index = temp.token_index;
    }
    
    return(iter);
}

static Cpp_Token
get_next_token(Application_Links *app, token_iterator *iter)
{
    Cpp_Token result = {};
    
    if(buffer_read_tokens(app, iter->buffer, iter->index, iter->index + 1, &result))
    {
        ++iter->index;
    }
    else
    {
        iter->valid = false;
    }
    
    return(result);
}

static Cpp_Token
peek_token(Application_Links *app, token_iterator *iter)
{
    Cpp_Token result = {};
    
    buffer_read_tokens(app, iter->buffer, iter->index, iter->index + 1, &result);
    
    return(result);
}

static Cpp_Token
get_prev_token(Application_Links *app, token_iterator *iter)
{
    Cpp_Token result = {};
    
    if(buffer_read_tokens(app, iter->buffer, iter->index, iter->index + 1, &result))
    {
        --iter->index;
    }
    else
    {
        iter->valid = false;
    }
    
    return(result);
}

static String
scratch_read(Application_Links *app, Arena *scratch, Buffer_ID buffer, int32_t start, int32_t end)
{
    String result = {};
    
    if(start <= end)
    {
        int32_t len = end - start;
        result = push_string_space(scratch, len);
        if(buffer_read_range(app, buffer, start, end, result.str))
        {
            result.size = len;
        }
    }
    
    return(result);
}

static String
scratch_read(Application_Links *app, Arena *scratch, Buffer_ID buffer, Range location)
{
    String result = scratch_read(app, scratch, buffer, location.start, location.one_past_last);
    return(result);
}

static String
scratch_read(Application_Links *app, Arena *scratch, Buffer_ID buffer, Cpp_Token token)
{
    String result = scratch_read(app, scratch, buffer, token.start, token.start + token.size);
    return(result);
}

static bool32
token_text_is(Application_Links *app, Buffer_Summary *buffer, Cpp_Token token, String match)
{
    Scratch_Block scratch(app);
    
    String text = scratch_read(app, scratch, buffer->buffer_id, token.start, token.start + token.size);
    bool32 result = (compare_ss(text, match) == 0);
    
    return(result);
}

static Range
token_range_from_abs(Application_Links *app, Buffer_ID buffer, int32_t pos, bool32 favor_forward)
{
    Range result = {};
    
    Cpp_Get_Token_Result get;
    if(buffer_get_token_index(app, buffer, pos, &get))
    {
        if(favor_forward && get.in_whitespace_after_token)
        {
            Cpp_Token token;
            buffer_read_tokens(app, buffer, get.token_index + 1, get.token_index + 2, &token);
            result = make_range(token.start, token.start + token.size);
        }
        else
        {
            result = make_range(get.token_start, get.token_one_past_last);
        }
    }
    
    return(result);
}

static int32_t
line_from_abs(Application_Links *app, Buffer_ID buffer, int32_t pos)
{
    int32_t Result = 0;
    
    Buffer_Seek seek = {};
    seek.type = buffer_seek_pos;
    seek.pos = pos;
    Partial_Cursor at;
    if(buffer_compute_cursor(app, buffer, seek, &at))
    {
        Result = at.line;
    }
    
    return(Result);
}

static i32
abs_from_seek(Application_Links *app, Buffer_ID buffer, Buffer_Seek seek)
{
    i32 Result = 0;
    
    // TODO(casey): I feel like there need to be faster methods for round-tripping through the (column/line <-> pos) routes.
    Partial_Cursor at;
    if(buffer_compute_cursor(app, buffer, seek, &at))
    {
        Result = at.pos;
    }
    
    return(Result);
}

static int32_t
line_from_abs(Application_Links *app, Buffer_Summary *buffer, int32_t pos)
{
    int32_t Result = 0;
    
    Buffer_Seek seek = {};
    seek.type = buffer_seek_pos;
    seek.pos = pos;
    Partial_Cursor at;
    if(buffer_compute_cursor(app, buffer, seek, &at))
    {
        Result = at.line;
    }
    
    return(Result);
}

static int32_t
line_from_abs(Application_Links *app, View_Summary *view, int32_t pos)
{
    int32_t Result = 0;
    
    Buffer_Seek seek = {};
    seek.type = buffer_seek_pos;
    seek.pos = pos;
    Full_Cursor at;
    if(view_compute_cursor(app, view, seek, &at))
    {
        Result = at.line;
    }
    
    return(Result);
}

static bool32
cursor_from_abs(Application_Links *app, View_Summary *view, int32_t pos, Full_Cursor *cursor)
{
    Buffer_Seek seek = {};
    seek.type = buffer_seek_pos;
    seek.pos = pos;
    bool32 result = view_compute_cursor(app, view, seek, cursor);
    return(result);
}

// TODO(casey): Rename these now that they are "render space", not screen space
static Vec2
screen_p_from(View_Summary *view, Full_Cursor *at)
{
    Vec2 Result = {};
    
    Result.x = (float)at->wrapped_x - (float)view->scroll_vars.scroll_x;
    Result.y = (float)at->wrapped_y - (float)view->scroll_vars.scroll_y;
    
    return(Result);
}

static Vec2
screen_p_from_abs(Application_Links *app, View_Summary *view, int32_t pos)
{
    Vec2 Result = {};
    
    Full_Cursor at;
    if(cursor_from_abs(app, view, pos, &at))
    {
        Result = screen_p_from(view, &at);
    }
    
    return(Result);
}

static String
read_entire_file(Partition *arena, char *filename)
{
    String result = {};
    
    FILE *file = fopen(filename, "rb");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        // TODO(casey): Can't we have 64-bit sized strings?  :(
        i32_4tech size = (i32_4tech)ftell(file);
        fseek(file, 0, SEEK_SET);
        
        result = string_push(arena, (i32_4tech)size);
        if(result.str)
        {
            result.size = size;
            fread(result.str, result.size, 1, file);
        }
        
        fclose(file);
    }
    
    return(result);
}

static void
draw_line(Application_Links *app, Vec2 from, Vec2 to, int_color color, float thickness)
{
    // TODO(casey): Allen, this should eventually be an actual line primitive, for non-rectangular lines
    float half = 0.5f*thickness;
    f32_Rect rect;
    rect.x0 = Min(from.x, to.x) - half;
    rect.x1 = Max(from.x, to.x) + half;
    rect.y0 = Min(from.y, to.y) - half;
    rect.y1 = Max(from.y, to.y) + half;
    draw_rectangle(app, rect, color);
}

static void
draw_line_loop(Application_Links *app, int32_t p_count, Vec2 *p, int_color color, float thickness)
{
    if(p_count)
    {
        int32_t prev = p_count - 1;
        for(int32_t i = 0; i < p_count; ++i)
        {
            draw_line(app, p[prev], p[i], color, thickness);
            prev = i;
        }
    }
}

static float
get_dpi_scaling_value(Application_Links *app)
{
    // TODO(casey): Allen, this should return the multiplier for the display relative to whatever 4coder
    // gets tuned to.
    float result = 2.0f;
    return(result);
}

static f32_Rect
minkowski_sum(f32_Rect a, Vec2 b)
{
    f32_Rect r = a;
    
    r.x0 -= b.x;
    r.x1 += b.x;
    
    r.x0 -= b.y;
    r.x1 += b.y;
    
    return(r);
}

static f32_Rect
minkowski_sum(f32_Rect a, f32 b)
{
    f32_Rect r = minkowski_sum(a, V2(b, b));
    
    return(r);
}

static void
clear_buffer(Application_Links *app, Buffer_ID buffer_id)
{
    Buffer_Summary buffer = get_buffer(app, buffer_id, AccessAll);
    if(buffer.exists)
    {
        buffer_replace_range(app, buffer_id, 0, buffer.size, empty_string);
    }
}

static int32_t
get_end_of_line(Application_Links *app, Buffer_ID buffer, int32_t from)
{
    int32_t result = from;
    
    Partial_Cursor line;
    if(buffer_compute_cursor(app, buffer, seek_pos(from), &line))
    {
        Partial_Cursor eol;
        if(buffer_compute_cursor(app, buffer, seek_line_char(line.line, max_i32), &eol))
        {
            result = eol.pos;
        }
    }
    
    return(result);
}

enum Coordinate
{
    Coordinate_X = 0,
    Coordinate_Y = 1,
};

enum Side
{
    Side_Min = 0,
    Side_Max = 1,
};

static f32_Rect_Pair
split_rect(f32_Rect rect, View_Split_Kind kind, Coordinate coord, Side from_side, f32 t)
{
    f32_Rect_Pair result;
    
    if(kind == ViewSplitKind_FixedPixels)
    {
        result.E[0] = rect;
        result.E[1] = rect;
        
        if(coord == Coordinate_X)
        {
            result.E[0].x1 = (from_side == Side_Max) ? (rect.x1 - t) : (rect.x0 + t);
            result.E[1].x0 = result.E[0].x1;
        }
        else
        {
            Assert(coord == Coordinate_Y);
            result.E[0].y1 = (from_side == Side_Max) ? (rect.y1 - t) : (rect.y0 + t);
            result.E[1].y0 = result.E[0].y1;
        }
    }
    else
    {
        Assert(kind == ViewSplitKind_Ratio);
        
        f32 pixel_count;
        if(coord == Coordinate_X)
        {
            pixel_count = t*(rect.x1 - rect.x0);
        }
        else
        {
            Assert(coord == Coordinate_Y);
            pixel_count = t*(rect.y1 - rect.y0);
        }
        result = split_rect(rect, ViewSplitKind_FixedPixels, coord, from_side, pixel_count);
    }
    
    return(result);
}
