
#ifndef FCODER_RECT_H
#define FCODER_RECT_H

#include <stdint.h>

struct i32_Rect{
	int32_t x0, y0;
	int32_t x1, y1;
};

struct f32_Rect{
	float x0, y0;
	float x1, y1;
};

inline i32_Rect
i32_rect_zero(){
    i32_Rect rect={0};
    return(rect);
}

inline f32_Rect
f32_rect_zero(){
    f32_Rect rect={0};
    return(rect);
}

inline i32_Rect
i32R(int32_t l, int32_t t, int32_t r, int32_t b){
    i32_Rect rect;
    rect.x0 = l; rect.y0 = t;
    rect.x1 = r; rect.y1 = b;
    return(rect);
}

inline i32_Rect
i32R(f32_Rect r){
    i32_Rect rect;
    rect.x0 = (int32_t)r.x0;
    rect.y0 = (int32_t)r.y0;
    rect.x1 = (int32_t)r.x1;
    rect.y1 = (int32_t)r.y1;
    return(rect);
}

inline i32_Rect
i32XYWH(int32_t x, int32_t y, int32_t w, int32_t h){
    i32_Rect rect;
    rect.x0 = x; rect.y0 = y;
    rect.x1 = x+w; rect.y1 = y+h;
    return(rect);
}

inline f32_Rect
f32R(float l, float t, float r, float b){
    f32_Rect rect;
    rect.x0 = l; rect.y0 = t;
    rect.x1 = r; rect.y1 = b;
    return(rect);
}

inline f32_Rect
f32R(i32_Rect r){
    f32_Rect rect;
    rect.x0 = (float)r.x0;
    rect.y0 = (float)r.y0;
    rect.x1 = (float)r.x1;
    rect.y1 = (float)r.y1;
    return(rect);
}

inline f32_Rect
f32XYWH(float x, float y, float w, float h){
    f32_Rect rect;
    rect.x0 = x; rect.y0 = y;
    rect.x1 = x+w; rect.y1 = y+h;
    return(rect);
}

inline int
rect_equal(i32_Rect r1, i32_Rect r2){
    int result = (r1.x0 == r2.x0 &&
                  r1.y0 == r2.y0 &&
                  r1.x1 == r2.x1 &&
                  r1.y1 == r2.y1);
    return(result);
}

inline int
hit_check(int32_t x, int32_t y, int32_t x0, int32_t y0, int32_t x1, int32_t y1){
    return (x >= x0 && x < x1 && y >= y0 && y < y1);
}

inline int
hit_check(int32_t x, int32_t y, i32_Rect rect){
    return (hit_check(x, y, rect.x0, rect.y0, rect.x1, rect.y1));
}

inline int
hit_check(int32_t x, int32_t y, float x0, float y0, float x1, float y1){
    return (x >= x0 && x < x1 && y >= y0 && y < y1);
}

inline int
hit_check(int32_t x, int32_t y, f32_Rect rect){
    return (hit_check(x, y, rect.x0, rect.y0, rect.x1, rect.y1));
}

inline int
positive_area(i32_Rect rect){
    return (rect.x0 < rect.x1 && rect.y0 < rect.y1);
}

inline i32_Rect
get_inner_rect(i32_Rect outer, int32_t margin){
    i32_Rect r;
    r.x0 = outer.x0 + margin;
    r.y0 = outer.y0 + margin;
    r.x1 = outer.x1 - margin;
    r.y1 = outer.y1 - margin;
    return r;
}

inline int
fits_inside(i32_Rect rect, i32_Rect outer){
    return (rect.x0 >= outer.x0 && rect.x1 <= outer.x1 &&
            rect.y0 >= outer.y0 && rect.y1 <= outer.y1);
}

inline i32_Rect
rect_clamp_to_rect(i32_Rect rect, i32_Rect clamp_box){
	if (rect.x0 < clamp_box.x0) rect.x0 = clamp_box.x0;
	if (rect.y0 < clamp_box.y0) rect.y0 = clamp_box.y0;
	if (rect.x1 > clamp_box.x1) rect.x1 = clamp_box.x1;
	if (rect.y1 > clamp_box.y1) rect.y1 = clamp_box.y1;
	return rect;
}

inline i32_Rect
rect_clamp_to_rect(int32_t left, int32_t top, int32_t right, int32_t bottom, i32_Rect clamp_box){
	return rect_clamp_to_rect(i32R(left, top, right, bottom), clamp_box);
}


#endif