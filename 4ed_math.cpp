/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 15.05.2015
 *
 * Math functions for 4coder
 *
 */

// TOP

/*
 * Scalar operators
 */

#define C_MATH 1

#define DEG_TO_RAD 0.0174533f

#if C_MATH
#include <math.h>
#endif

inline real32
ABS(real32 x){
#if C_MATH
    return abs(x);
#endif
}

inline real32
MOD(real32 x, i32 m){
#if C_MATH
    real32 whole, frac;
    frac = modf(x, &whole);
    return ((i32)(whole) % m) + frac;
#endif
}

inline real32
SQRT(real32 x){
#if C_MATH
    return sqrt(x);
#endif
}

inline real32
SIN(real32 x_degrees){
#if C_MATH
    return sinf(x_degrees * DEG_TO_RAD);
#endif
}

inline real32
COS(real32 x_degrees){
#if C_MATH
    return cosf(x_degrees * DEG_TO_RAD);
#endif
}

/*
 * Rounding
 */

inline i32
TRUNC32(real32 x) { return (i32)x; }

inline i32
FLOOR32(real32 x) { return (i32)(x)-((x!=(i32)(x) && x<0)?1:0); }

inline i32
CEIL32(real32 x) { return (i32)(x)+((x!=(i32)(x) && x>0)?1:0); }

inline i32
ROUND32(real32 x) { return FLOOR32(x + .5f); }

inline i32
DIVCEIL32(i32 n, i32 d) {
    i32 q = (n/d);
    return q + (q*d < n);
}

inline real32
FRACPART32(real32 x) { return x - (i32)x; }

/*
 * Rectangles
 */

struct i32_Rect{
	i32 x0, y0;
	i32 x1, y1;
};

struct real32_Rect{
	real32 x0, y0;
	real32 x1, y1;
};

inline i32_Rect
i32R(i32 l, i32 t, i32 r, i32 b){
    i32_Rect rect;
    rect.x0 = l; rect.y0 = t;
    rect.x1 = r; rect.y1 = b;
    return rect;
}

inline i32_Rect
i32R(real32_Rect r){
    i32_Rect rect;
    rect.x0 = (i32)r.x0;
    rect.y0 = (i32)r.y0;
    rect.x1 = (i32)r.x1;
    rect.y1 = (i32)r.y1;
    return rect;
}

inline i32_Rect
i32XYWH(i32 x, i32 y, i32 w, i32 h){
    i32_Rect rect;
    rect.x0 = x; rect.y0 = y;
    rect.x1 = x+w; rect.y1 = y+h;
    return rect;
}

inline real32_Rect
real32R(real32 l, real32 t, real32 r, real32 b){
    real32_Rect rect;
    rect.x0 = l; rect.y0 = t;
    rect.x1 = r; rect.y1 = b;
    return rect;
}

inline real32_Rect
real32R(i32_Rect r){
    real32_Rect rect;
    rect.x0 = (real32)r.x0;
    rect.y0 = (real32)r.y0;
    rect.x1 = (real32)r.x1;
    rect.y1 = (real32)r.y1;
    return rect;
}

inline real32_Rect
real32XYWH(real32 x, real32 y, real32 w, real32 h){
    real32_Rect rect;
    rect.x0 = x; rect.y0 = y;
    rect.x1 = x+w; rect.y1 = y+h;
    return rect;
}

inline bool32
hit_check(i32 x, i32 y, i32 x0, i32 y0, i32 x1, i32 y1){
    return (x >= x0 && x < x1 && y >= y0 && y < y1);
}

inline bool32
hit_check(i32 x, i32 y, i32_Rect rect){
    return (hit_check(x, y, rect.x0, rect.y0, rect.x1, rect.y1));
}

inline bool32
hit_check(i32 x, i32 y, real32 x0, real32 y0, real32 x1, real32 y1){
    return (x >= x0 && x < x1 && y >= y0 && y < y1);
}

inline bool32
hit_check(i32 x, i32 y, real32_Rect rect){
    return (hit_check(x, y, rect.x0, rect.y0, rect.x1, rect.y1));
}

inline bool32
positive_area(i32_Rect rect){
    return (rect.x0 < rect.x1 && rect.y0 < rect.y1);
}

inline i32_Rect
get_inner_rect(i32_Rect outer, i32 margin){
    i32_Rect r;
    r.x0 = outer.x0 + margin;
    r.y0 = outer.y0 + margin;
    r.x1 = outer.x1 - margin;
    r.y1 = outer.y1 - margin;
    return r;
}

inline bool32
fits_inside(i32_Rect rect, i32_Rect outer){
    return (rect.x0 >= outer.x0 && rect.x1 <= outer.x1 &&
            rect.y0 >= outer.y0 && rect.y1 <= outer.y1);
}

/*
 * Vectors
 */

struct Vec2{
    union{
        struct{
            real32 x, y;
        };
        struct{
            real32 v[2];
        };
    };
};

struct Vec3{
    union{
        struct{
            real32 x, y, z;
        };
        struct{
            real32 r, g, b;
        };
        struct{
            Vec2 xy;
            real32 _z;
        };
        struct{
            real32 _x;
            Vec2 yz;
        };
        struct{
            real32 v[3];
        };
    };
};

struct Vec4{
    union{
        struct{
            real32 r, g, b, a;
        };
        
        struct{
            real32 h, s, l, __a;
        };
        struct{
            real32 x, y, z, w;
        };
        struct{
            Vec3 rgb;
            real32 _a;
        };
        struct{
            Vec3 xyz;
            real32 _w;
        };
        struct{
            real32 _x;
            Vec3 yzw;
        };
        struct{
            real32 v[4];
        };
    };
};

inline internal Vec2
V2(real32 x, real32 y){
    Vec2 result;
    result.x = x;
    result.y = y;
    return result;
}

inline internal Vec3
V3(real32 x, real32 y, real32 z){
    Vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

inline internal Vec4
V4(real32 x, real32 y, real32 z, real32 w){
    Vec4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
}

inline internal Vec2
operator+(Vec2 a, Vec2 b){
    Vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

inline internal Vec3
operator+(Vec3 a, Vec3 b){
    Vec3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

inline internal Vec4
operator+(Vec4 a, Vec4 b){
    Vec4 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

inline internal Vec2
operator-(Vec2 a, Vec2 b){
    Vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return result;
}

inline internal Vec3
operator-(Vec3 a, Vec3 b){
    Vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

inline internal Vec4
operator-(Vec4 a, Vec4 b){
    Vec4 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return result;
}

inline internal Vec2
operator*(Vec2 a, real32 k){
    Vec2 result;
    result.x = a.x * k;
    result.y = a.y * k;
    return result;
}

inline internal Vec3
operator*(Vec3 a, real32 k){
    Vec3 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    return result;
}

inline internal Vec4
operator*(Vec4 a, real32 k){
    Vec4 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    result.w = a.w * k;
    return result;
}

inline internal Vec2
operator*(real32 k, Vec2 a){
    Vec2 result;
    result.x = a.x * k;
    result.y = a.y * k;
    return result;
}

inline internal Vec3
operator*(real32 k, Vec3 a){
    Vec3 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    return result;
}

inline internal Vec4
operator*(real32 k, Vec4 a){
    Vec4 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    result.w = a.w * k;
    return result;
}

inline internal Vec2&
operator+=(Vec2 &a, Vec2 b){
    a = (a + b);
    return a;
}

inline internal Vec3&
operator+=(Vec3 &a, Vec3 b){
    a = (a + b);
    return a;
}

inline internal Vec4&
operator+=(Vec4 &a, Vec4 b){
    a = (a + b);
    return a;
}

inline internal Vec2&
operator-=(Vec2 &a, Vec2 b){
    a = (a - b);
    return a;
}

inline internal Vec3&
operator-=(Vec3 &a, Vec3 b){
    a = (a - b);
    return a;
}

inline internal Vec4&
operator-=(Vec4 &a, Vec4 b){
    a = (a - b);
    return a;
}

inline internal Vec2&
operator*=(Vec2 &a, real32 k){
    a = (a * k);
    return a;
}

inline internal Vec3&
operator*=(Vec3 &a, real32 k){
    a = (a * k);
    return a;
}

inline internal Vec4&
operator*=(Vec4 &a, real32 k){
    a = (a * k);
    return a;
}

inline internal real32
dot(Vec2 a, Vec2 b){
    real32 result;
    result = a.x*b.x + a.y*b.y;
    return result;
}

inline internal real32
dot(Vec3 a, Vec3 b){
    real32 result;
    result = a.x*b.x + a.y*b.y + a.z*b.z;
    return result;
}

inline internal real32
dot(Vec4 a, Vec4 b){
    real32 result;
    result = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    return result;
}

inline internal Vec3
cross(Vec3 a, Vec3 b){
    Vec3 result;
    result.x = a.y*b.z - b.y*a.z;
    result.y = a.z*b.x - b.z*a.x;
    result.z = a.x*b.y - b.x*a.y;
    return result;
}

inline internal Vec2
hadamard(Vec2 a, Vec2 b){
    Vec2 result;
    result.x = a.x*b.x;
    result.y = a.y*b.y;
    return result;
}

inline internal Vec3
hadamard(Vec3 a, Vec3 b){
    Vec3 result;
    result.x = a.x*b.x;
    result.y = a.y*b.y;
    result.z = a.z*b.z;
    return result;
}

inline internal Vec4
hadamard(Vec4 a, Vec4 b){
    Vec4 result;
    result.x = a.x*b.x;
    result.y = a.y*b.y;
    result.z = a.z*b.z;
    result.w = a.w*b.w;
    return result;
}

inline internal Vec2
perp(Vec2 v){
    Vec2 result;
    result.x = -v.y;
    result.y = v.x;
    return result;
}

inline Vec2
polar_to_cartesian(real32 theta_degrees, real32 length){
	Vec2 result;
	result.x = COS(theta_degrees)*length;
	result.y = SIN(theta_degrees)*length;
	return result;
}

inline Vec2
rotate(Vec2 v, real32 theta_degrees){
    Vec2 result;
    real32 c, s;
    c = COS(theta_degrees);
    s = SIN(theta_degrees);
    result.x = v.x*c - v.y*s;
    result.y = v.x*s + v.y*c;
    return result;
}

/*
 * Coordinates
 */

struct Matrix2{
    Vec2 x_axis;
    Vec2 y_axis;
};

internal Matrix2
invert(Vec2 x_axis, Vec2 y_axis){
    Matrix2 result = {};
    real32 det = 1.f / (x_axis.x*y_axis.y - x_axis.y*y_axis.x);
    result.x_axis.x = y_axis.y*det;
    result.y_axis.x = -y_axis.x*det;
    result.x_axis.y = -x_axis.y*det;
    result.y_axis.y = x_axis.x*det;
    return result;
}

internal Matrix2
invert(Matrix2 m){
    Matrix2 result = {};
    real32 det = 1.f / (m.x_axis.x*m.y_axis.y - m.x_axis.y*m.y_axis.x);
    result.x_axis.x = m.y_axis.y*det;
    result.y_axis.x = -m.y_axis.x*det;
    result.x_axis.y = -m.x_axis.y*det;
    result.y_axis.y = m.x_axis.x*det;
    return result;
}

/*
 * Lerps, Clamps, Thresholds, Etc
 */

inline real32
lerp(real32 a, real32 t, real32 b){
    return a + (b-a)*t;
}

inline Vec2
lerp(Vec2 a, real32 t, Vec2 b){
    return a + (b-a)*t;
}

inline Vec3
lerp(Vec3 a, real32 t, Vec3 b){
    return a + (b-a)*t;
}

inline Vec4
lerp(Vec4 a, real32 t, Vec4 b){
    return a + (b-a)*t;
}

inline real32
unlerp(real32 a, real32 x, real32 b){
    return (x - a) / (b - a);
}

inline real32
clamp(real32 a, real32 n, real32 z){
    return (n<a)?(a):((n>z)?(z):n);
}

/*
 * Color
 */

// TODO(allen): Convert colors to Vec4
inline internal u32
color_blend(u32 a, real32 t, u32 b){
    union{
        u8 byte[4];
        u32 comp;
    } A, B, R;
     
    A.comp = a;
    B.comp = b;
    
    R.byte[0] = (u8)lerp(A.byte[0], t, B.byte[0]);
    R.byte[1] = (u8)lerp(A.byte[1], t, B.byte[1]);
    R.byte[2] = (u8)lerp(A.byte[2], t, B.byte[2]);
    R.byte[3] = (u8)lerp(A.byte[3], t, B.byte[3]);
    
    return R.comp;
}

internal Vec3
unpack_color3(u32 color){
    Vec3 result;
    result.r = ((color >> 16) & 0xFF) / 255.f;
    result.g = ((color >> 8) & 0xFF) / 255.f;
    result.b = ((color >> 0) & 0xFF) / 255.f;
    return result;
}

internal Vec4
unpack_color4(u32 color){
    Vec4 result;
    result.a = ((color >> 24) & 0xFF) / 255.f;
    result.r = ((color >> 16) & 0xFF) / 255.f;
    result.g = ((color >> 8) & 0xFF) / 255.f;
    result.b = ((color >> 0) & 0xFF) / 255.f;
    return result;
}

internal u32
pack_color4(Vec4 color){
    u32 result =
        ((u8)(color.a * 255) << 24) |
        ((u8)(color.r * 255) << 16) |
        ((u8)(color.g * 255) << 8) |
        ((u8)(color.b * 255) << 0);
    return result;
}

internal Vec4
rgba_to_hsla(Vec4 rgba){
    Vec4 hsla = {};
    real32 max, min, delta;
    i32 maxc;
    hsla.a = rgba.a;
    max = rgba.r; min = rgba.r;
    maxc = 0;
    if (rgba.r < rgba.g){
        max = rgba.g;
        maxc = 1;
    }
    if (rgba.b > max){
        max = rgba.b;
        maxc = 2;
    }
    if (rgba.r > rgba.g){
        min = rgba.g;
    }
    if (rgba.b < min){
        min = rgba.b;
    }
    delta = max - min;
    
    hsla.z = (max + min) * .5f;
    if (delta == 0){
        hsla.x = 0.f;
        hsla.y = 0.f;
    }
    else{
        switch (maxc){
        case 0:
        {
            hsla.x = (rgba.g - rgba.b) / delta;
            hsla.x += (rgba.g < rgba.b) * 6.f;
        }break;
        
        case 1:
        {
            hsla.x = (rgba.b - rgba.r) / delta;
            hsla.x += 2.f;
        }break;
        
        case 2:
        {
            hsla.x = (rgba.r - rgba.g) / delta;
            hsla.x += 4.f;
        }break;
        }
        hsla.x *= (1/6.f); // * 60 / 360
        hsla.y = delta / (1.f - ABS(2.f*hsla.z - 1.f));
    }
    
    return hsla;
}

internal Vec4
hsla_to_rgba(Vec4 hsla){
    if (hsla.h >= 1.f) hsla.h = 0.f;
    Vec4 rgba = {};
    real32 C, X, m;
    i32 H;
    rgba.a = hsla.a;
    C = (1.f - ABS(2*hsla.z - 1.f)) * hsla.y;
    X = C * (1.f-ABS(MOD(hsla.x*6.f, 2)-1.f));
    m = hsla.z - C*.5f;
    H = FLOOR32(hsla.x * 6.f);
    switch (H){
    case 0:
        rgba.r = C; rgba.g = X; rgba.b = 0;
        break;
        
    case 1:
        rgba.r = X; rgba.g = C; rgba.b = 0;
        break;
        
    case 2:
        rgba.r = 0; rgba.g = C; rgba.b = X;
        break;
        
    case 3:
        rgba.r = 0; rgba.g = X; rgba.b = C;
        break;
        
    case 4:
        rgba.r = X; rgba.g = 0; rgba.b = C;
        break;
        
    case 5:
        rgba.r = C; rgba.g = 0; rgba.b = X;
        break;
    }
    rgba.r += m;
    rgba.g += m;
    rgba.b += m;
    return rgba;
}

// BOTTOM

