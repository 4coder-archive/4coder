/*
 * 4coder base types
 */

// TOP

#define C_MATH 1

static i32
ceil32(f32 v){
    return(((v)>0)?( (v == (i32)(v))?((i32)(v)):((i32)((v)+1.f)) ):( ((i32)(v)) ));
}

static i32
floor32(f32 v){
    return(((v)<0)?( (v == (i32)(v))?((i32)(v)):((i32)((v)-1.f)) ):( ((i32)(v)) ));
}

static i32
round32(f32 v){
    return(floor32(v + 0.5f));
}

static i32
trun32(f32 v){
    return((i32)(v));
}

static i32
div_ceil(i32 n, i32 d){
    return( ((n) % (d) != 0) + ((n) / (d)) );
}

static i32
l_round_up_i32(i32 x, i32 b){
    i32 t = x + b - 1;
    return(t - (t%b));
}

static u32
l_round_up_u32(u32 x, u32 b){
    i32 t = x + b - 1;
    return(t - (t%b));
}

static u32 round_up_pot_u32(u32 x){
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    ++x;
    return(x);
}

////////////////////////////////

// scalars

#define DEG_TO_RAD (0.0174533f)

static f32
ABS(f32 x){
    if (x < 0) x = -x;
    return(x);
}

#if C_MATH
#include <math.h>

static f32
MOD(f32 x, i32 m){
    f32 whole;
    f32 frac = modff(x, &whole);
    f32 r = ((i32)(whole) % m) + frac;
    return(r);
}

static f32
SQRT(f32 x){
    f32 r = sqrtf(x);
    return(r);
}

static f32
SIN(f32 x_degrees){
    f32 r = sinf(x_degrees * DEG_TO_RAD);
    return(r);
}

static f32
COS(f32 x_degrees){
    f32 r = cosf(x_degrees * DEG_TO_RAD);
    return(r);
}
#endif

////////////////////////////////

// vectors

static Vec2
V2(f32 x, f32 y){
    Vec2 result = {};
    result.x = x;
    result.y = y;
    return(result);
}

static Vec3
V3(f32 x, f32 y, f32 z){
    Vec3 result = {};
    result.x = x;
    result.y = y;
    result.z = z;
    return(result);
}

static Vec4
V4(f32 x, f32 y, f32 z, f32 w){
    Vec4 result = {};
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return(result);
}

static Vec2
V2(Vec2_i32 pi){
    return(V2((f32)pi.x, (f32)pi.y));
}

static Vec3
V3(Vec3_i32 pi){
    return(V3((f32)pi.x, (f32)pi.y, (f32)pi.z));
}

static Vec4
V4(Vec4_i32 pi){
    return(V4((f32)pi.x, (f32)pi.y, (f32)pi.z, (f32)pi.w));
}

static Vec2
V2(Vec2 pi){
    return(pi);
}

static Vec3
V3(Vec3 pi){
    return(pi);
}

static Vec4
V4(Vec4 pi){
    return(pi);
}

static Vec2_i32
V2i32(i32 x, i32 y){
    Vec2_i32 result = {};
    result.x = x;
    result.y = y;
    return(result);
}

static Vec3_i32
V3i32(i32 x, i32 y, i32 z){
    Vec3_i32 result = {};
    result.x = x;
    result.y = y;
    result.z = z;
    return(result);
}

static Vec4_i32
V4i32(i32 x, i32 y, i32 z, i32 w){
    Vec4_i32 result = {};
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return(result);
}

static Vec2_i32
V2i32(Vec2_f32 pi){
    return(V2i32((i32)pi.x, (i32)pi.y));
}

static Vec3_i32
V3i32(Vec3_f32 pi){
    return(V3i32((i32)pi.x, (i32)pi.y, (i32)pi.z));
}

static Vec4_i32
V4i32(Vec4_f32 pi){
    return(V4i32((i32)pi.x, (i32)pi.y, (i32)pi.z, (i32)pi.w));
}

static Vec2_i32
V2i32(Vec2_i32 pi){
    return(pi);
}

static Vec3_i32
V3i32(Vec3_i32 pi){
    return(pi);
}

static Vec4_i32
V4i32(Vec4_i32 pi){
    return(pi);
}

static Vec2
operator+(Vec2 a, Vec2 b){
    Vec2 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return(result);
}

static Vec3
operator+(Vec3 a, Vec3 b){
    Vec3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return(result);
}

static Vec4
operator+(Vec4 a, Vec4 b){
    Vec4 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return(result);
}

static Vec2
operator-(Vec2 a, Vec2 b){
    Vec2 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return(result);
}

static Vec3
operator-(Vec3 a, Vec3 b){
    Vec3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return(result);
}

static Vec4
operator-(Vec4 a, Vec4 b){
    Vec4 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return(result);
}

static Vec2
operator*(Vec2 a, f32 k){
    Vec2 result;
    result.x = a.x * k;
    result.y = a.y * k;
    return(result);
}

static Vec3
operator*(Vec3 a, f32 k){
    Vec3 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    return(result);
}

static Vec4
operator*(Vec4 a, f32 k){
    Vec4 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    result.w = a.w * k;
    return(result);
}

static Vec2
operator*(f32 k, Vec2 a){
    Vec2 result;
    result.x = a.x * k;
    result.y = a.y * k;
    return(result);
}

static Vec3
operator*(f32 k, Vec3 a){
    Vec3 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    return(result);
}

static Vec4
operator*(f32 k, Vec4 a){
    Vec4 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    result.w = a.w * k;
    return(result);
}

static Vec2&
operator+=(Vec2 &a, Vec2 b){
    a = (a + b);
    return(a);
}

static Vec3&
operator+=(Vec3 &a, Vec3 b){
    a = (a + b);
    return(a);
}

static Vec4&
operator+=(Vec4 &a, Vec4 b){
    a = (a + b);
    return(a);
}

static Vec2&
operator-=(Vec2 &a, Vec2 b){
    a = (a - b);
    return(a);
}

static Vec3&
operator-=(Vec3 &a, Vec3 b){
    a = (a - b);
    return(a);
}

static Vec4&
operator-=(Vec4 &a, Vec4 b){
    a = (a - b);
    return(a);
}

static Vec2&
operator*=(Vec2 &a, f32 k){
    a = (a*k);
    return(a);
}

static Vec3&
operator*=(Vec3 &a, f32 k){
    a = (a*k);
    return(a);
}

static Vec4&
operator*=(Vec4 &a, f32 k){
    a = (a*k);
    return(a);
}

static b32
operator==(Vec2 a, Vec2 b){
    return(a.x == b.x && a.y == b.y);
}

static b32
operator!=(Vec2 a, Vec2 b){
    return(!(a.x == b.x && a.y == b.y));
}

static b32
operator==(Vec3 a, Vec3 b){
    return(a.x == b.x && a.y == b.y && a.z == b.z);
}

static b32
operator!=(Vec3 a, Vec3 b){
    return(!(a.x == b.x && a.y == b.y && a.z == b.z));
}

static b32
operator==(Vec4 a, Vec4 b){
    return(a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
}

static b32
operator!=(Vec4 a, Vec4 b){
    return(!(a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w));
}

static Vec2_i32
operator+(Vec2_i32 a, Vec2_i32 b){
    Vec2_i32 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return(result);
}

static Vec3_i32
operator+(Vec3_i32 a, Vec3_i32 b){
    Vec3_i32 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return(result);
}

static Vec4_i32
operator+(Vec4_i32 a, Vec4_i32 b){
    Vec4_i32 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return(result);
}

static Vec2_i32
operator-(Vec2_i32 a, Vec2_i32 b){
    Vec2_i32 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    return(result);
}

static Vec3_i32
operator-(Vec3_i32 a, Vec3_i32 b){
    Vec3_i32 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return(result);
}

static Vec4_i32
operator-(Vec4_i32 a, Vec4_i32 b){
    Vec4_i32 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    result.w = a.w - b.w;
    return(result);
}

static Vec2_i32
operator*(Vec2_i32 a, i32 k){
    Vec2_i32 result;
    result.x = a.x * k;
    result.y = a.y * k;
    return(result);
}

static Vec3_i32
operator*(Vec3_i32 a, i32 k){
    Vec3_i32 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    return(result);
}

static Vec4_i32
operator*(Vec4_i32 a, i32 k){
    Vec4_i32 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    result.w = a.w * k;
    return(result);
}

static Vec2_i32
operator*(i32 k, Vec2_i32 a){
    Vec2_i32 result;
    result.x = a.x * k;
    result.y = a.y * k;
    return(result);
}

static Vec3_i32
operator*(i32 k, Vec3_i32 a){
    Vec3_i32 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    return(result);
}

static Vec4_i32
operator*(i32 k, Vec4_i32 a){
    Vec4_i32 result;
    result.x = a.x * k;
    result.y = a.y * k;
    result.z = a.z * k;
    result.w = a.w * k;
    return(result);}

static Vec2_i32&
operator+=(Vec2_i32 &a, Vec2_i32 b){
    a = (a + b);
    return(a);
}

static Vec3_i32&
operator+=(Vec3_i32 &a, Vec3_i32 b){
    a = (a + b);
    return(a);
}

static Vec4_i32&
operator+=(Vec4_i32 &a, Vec4_i32 b){
    a = (a + b);
    return(a);
}

static Vec2_i32&
operator-=(Vec2_i32 &a, Vec2_i32 b){
    a = (a - b);
    return(a);
}

static Vec3_i32&
operator-=(Vec3_i32 &a, Vec3_i32 b){
    a = (a - b);
    return(a);
}

static Vec4_i32&
operator-=(Vec4_i32 &a, Vec4_i32 b){
    a = (a - b);
    return(a);
}

static Vec2_i32&
operator*=(Vec2_i32 &a, i32 k){
    a = (a*k);
    return(a);
}

static Vec3_i32&
operator*=(Vec3_i32 &a, i32 k){
    a = (a*k);
    return(a);
}

static Vec4_i32&
operator*=(Vec4_i32 &a, i32 k){
    a = (a*k);
    return(a);
}

static b32
operator==(Vec2_i32 a, Vec2_i32 b){
    return(a.x == b.x && a.y == b.y);
}

static b32
operator!=(Vec2_i32 a, Vec2_i32 b){
    return(!(a.x == b.x && a.y == b.y));
}

static b32
operator==(Vec3_i32 a, Vec3_i32 b){
    return(a.x == b.x && a.y == b.y && a.z == b.z);
}

static b32
operator!=(Vec3_i32 a, Vec3_i32 b){
    return(!(a.x == b.x && a.y == b.y && a.z == b.z));
}

static b32
operator==(Vec4_i32 a, Vec4_i32 b){
    return(a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w);
}

static b32
operator!=(Vec4_i32 a, Vec4_i32 b){
    return(!(a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w));
}

static f32
dot(Vec2 a, Vec2 b){
    f32 result;
    result = a.x*b.x + a.y*b.y;
    return(result);
}

static f32
dot(Vec3 a, Vec3 b){
    f32 result;
    result = a.x*b.x + a.y*b.y + a.z*b.z;
    return(result);
}

static f32
dot(Vec4 a, Vec4 b){
    f32 result;
    result = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
    return(result);
}

static Vec3
cross(Vec3 a, Vec3 b){
    Vec3 result;
    result.x = a.y*b.z - b.y*a.z;
    result.y = a.z*b.x - b.z*a.x;
    result.z = a.x*b.y - b.x*a.y;
    return(result);
}

static Vec2
hadamard(Vec2 a, Vec2 b){
    a.x *= b.x;
    a.y *= b.y;
    return(a);
}

static Vec3
hadamard(Vec3 a, Vec3 b){
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    return(a);
}

static Vec4
hadamard(Vec4 a, Vec4 b){
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    a.w *= b.w;
    return(a);
}

static Vec2
perp(Vec2 v){
    return(V2(-v.y, v.x));
}

static Vec2
polar_to_cartesian(f32 theta_degrees, f32 length){
    return(V2(COS(theta_degrees), SIN(theta_degrees))*length);
}

static Vec2
rotate(Vec2 v, f32 theta_degrees){
    f32 c = COS(theta_degrees);
    f32 s = SIN(theta_degrees);
    return(V2(v.x*c - v.y*s,
              v.x*s + v.y*c));
}

////////////////////////////////

static f32
lerp(f32 a, f32 t, f32 b){
    return(a + (b-a)*t);
}

static i32
lerp(i32 a, f32 t, i32 b){
    return((i32)(lerp((f32)a, t, (f32)b)));
}

static Vec2
lerp(Vec2 a, f32 t, Vec2 b){
    return(a + (b-a)*t);
}

static Vec3
lerp(Vec3 a, f32 t, Vec3 b){
    return(a + (b-a)*t);
}

static Vec4
lerp(Vec4 a, f32 t, Vec4 b){
    return(a + (b-a)*t);
}

static f32
unlerp(f32 a, f32 x, f32 b){
    f32 r = x;
    if (b != a){
        r = (x - a) / (b - a);
    }
    return(r);
}

////////////////////////////////

// color

// TODO(allen): Convert colors to Vec4
static u32
color_blend(u32 a, f32 t, u32 b){
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
    
    return(R.comp);
}

static Vec3
unpack_color3(u32 color){
    Vec3 result;
    result.r = ((color >> 16) & 0xFF) / 255.f;
    result.g = ((color >> 8) & 0xFF) / 255.f;
    result.b = ((color >> 0) & 0xFF) / 255.f;
    return(result);
}

static Vec4
unpack_color4(u32 color){
    Vec4 result;
    result.a = ((color >> 24) & 0xFF) / 255.f;
    result.r = ((color >> 16) & 0xFF) / 255.f;
    result.g = ((color >> 8) & 0xFF) / 255.f;
    result.b = ((color >> 0) & 0xFF) / 255.f;
    return(result);
}

static u32
pack_color4(Vec4 color){
    u32 result =
        ((u8)(color.a*255) << 24) |
        ((u8)(color.r*255) << 16) |
        ((u8)(color.g*255) << 8) |
        ((u8)(color.b*255) << 0);
    return(result);
}

static Vec4
rgba_to_hsla(Vec4 rgba){
    Vec4 hsla = {};
    f32 max, min, delta;
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
    
    hsla.z = (max + min)*.5f;
    if (delta == 0){
        hsla.x = 0.f;
        hsla.y = 0.f;
    }
    else{
        switch (maxc){
            case 0:
            {
                hsla.x = (rgba.g - rgba.b) / delta;
                hsla.x += (rgba.g < rgba.b)*6.f;
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
        hsla.x *= (1/6.f); //*60 / 360
        hsla.y = delta / (1.f - ABS(2.f*hsla.z - 1.f));
    }
    
    return(hsla);
}

static Vec4
hsla_to_rgba(Vec4 hsla){
    if (hsla.h >= 1.f){
        hsla.h = 0.f;
    }
    f32 C = (1.f - ABS(2*hsla.z - 1.f))*hsla.y;
    f32 X = C*(1.f-ABS(MOD(hsla.x*6.f, 2)-1.f));
    f32 m = hsla.z - C*.5f;
    i32 H = floor32(hsla.x*6.f);
    Vec4 rgba = {};
    rgba.a = hsla.a;
    switch (H){
        case 0: rgba.r = C; rgba.g = X; rgba.b = 0; break;
        case 1: rgba.r = X; rgba.g = C; rgba.b = 0; break;
        case 2: rgba.r = 0; rgba.g = C; rgba.b = X; break;
        case 3: rgba.r = 0; rgba.g = X; rgba.b = C; break;
        case 4: rgba.r = X; rgba.g = 0; rgba.b = C; break;
        case 5: rgba.r = C; rgba.g = 0; rgba.b = X; break;
    }
    rgba.r += m;
    rgba.g += m;
    rgba.b += m;
    return(rgba);
}

////////////////////////////////

static i32
get_width(Range range){
    i32 result = range.end - range.start;
    if (result < 0){
        result = 0;
    }
    return(result);
}

static Range
make_range(i32 p1, i32 p2){
    Range range;
    if (p1 < p2){
        range.min = p1;
        range.max = p2;
    }
    else{
        range.min = p2;
        range.max = p1;
    }
    return(range);
}

static Range
rectify(Range range) {
    return(make_range(range.min, range.max));
}

static b32
interval_overlap(i32 a0, i32 a1, i32 b0, i32 b1){
    return(!(a1 < b0 || b1 < a0));
}

static b32
interval_overlap(Range a, Range b){
    return(interval_overlap(a.first, a.one_past_last, b.first, b.one_past_last));
}

static b32
interval_contains(i32 a0, i32 a1, i32 b){
    return((a0 <= b) && (b < a1));
}

static b32
interval_contains(Range range, i32 b){
    return(interval_contains(range.start, range.one_past_last, b));
}

////////////////////////////////

static i32_Rect
i32R(i32 l, i32 t, i32 r, i32 b){
    i32_Rect rect = {};
    rect.x0 = l;
    rect.y0 = t;
    rect.x1 = r;
    rect.y1 = b;
    return(rect);
}

static i32_Rect
i32R_xy_wh(i32 x, i32 y, i32 w, i32 h){
    return(i32R(x, y, x + w, y + h));
}

static i32_Rect
i32R(f32_Rect r){
    i32_Rect rect = {};
    rect.x0 = (i32)r.x0;
    rect.y0 = (i32)r.y0;
    rect.x1 = (i32)r.x1;
    rect.y1 = (i32)r.y1;
    return(rect);
}

static f32_Rect
f32R(f32 l, f32 t, f32 r, f32 b){
    f32_Rect rect = {};
    rect.x0 = l;
    rect.y0 = t;
    rect.x1 = r;
    rect.y1 = b;
    return(rect);
}

static f32_Rect
f32R(Vec2 p0, Vec2 p1){
    f32_Rect rect = {};
    rect.p0 = p0;
    rect.p1 = p1;
    return(rect);
}

static f32_Rect
f32R(i32_Rect r){
    f32_Rect rect = {};
    rect.x0 = (f32)r.x0;
    rect.y0 = (f32)r.y0;
    rect.x1 = (f32)r.x1;
    rect.y1 = (f32)r.y1;
    return(rect);
}

static i32
rect_equal(i32_Rect r1, i32_Rect r2){
    return(r1.x0 == r2.x0 && r1.y0 == r2.y0 && r1.x1 == r2.x1 && r1.y1 == r2.y1);
}

static b32
hit_check(f32 x, f32 y, f32 x0, f32 y0, f32 x1, f32 y1){
    return(x >= x0 && x < x1 && y >= y0 && y < y1);
}

static b32
hit_check(f32 x, f32 y, f32_Rect rect){
    return(hit_check(x, y, rect.x0, rect.y0, rect.x1, rect.y1));
}

static b32
hit_check(Rect_f32 rect, Vec2_f32 p){
    return(hit_check(p.x, p.y, rect.x0, rect.y0, rect.x1, rect.y1));
}

static b32
hit_check(i32 x, i32 y, i32 x0, i32 y0, i32 x1, i32 y1){
    return(x >= x0 && x < x1 && y >= y0 && y < y1);
}

static b32
hit_check(i32 x, i32 y, i32_Rect rect){
    return(hit_check(x, y, rect.x0, rect.y0, rect.x1, rect.y1));
}

static b32
hit_check(Rect_i32 rect, Vec2_i32 p){
    return(hit_check(p.x, p.y, rect.x0, rect.y0, rect.x1, rect.y1));
}

static i32_Rect
get_inner_rect(i32_Rect outer, i32 margin){
    i32_Rect r = {};
    r.x0 = outer.x0 + margin;
    r.y0 = outer.y0 + margin;
    r.x1 = outer.x1 - margin;
    r.y1 = outer.y1 - margin;
    return(r);
}

static f32_Rect
get_inner_rect(f32_Rect outer, f32 margin){
    f32_Rect r = {};
    r.x0 = outer.x0 + margin;
    r.y0 = outer.y0 + margin;
    r.x1 = outer.x1 - margin;
    r.y1 = outer.y1 - margin;
    return(r);
}

static i32
rect_height(i32_Rect rect){
    return(rect.y1 - rect.y0);
}

static i32
rect_width(i32_Rect rect){
    return(rect.x1 - rect.x0);
}

static i32
fits_inside(i32_Rect rect, i32_Rect outer){
    return(rect.x0 >= outer.x0 && rect.x1 <= outer.x1 && rect.y0 >= outer.y0 && rect.y1 <= outer.y1);
}

static i32
interval_overlap(f32 a0, f32 a1, f32 b0, f32 b1){
    return(a0 < b1 && b0 < a1);
}

static i32
rect_overlap(f32_Rect a, f32_Rect b){
    return(interval_overlap(a.x0, a.x1, b.x0, b.x1) &&
           interval_overlap(a.y0, a.y1, b.y0, b.y1));
}

static f32
rect_height(f32_Rect rect){
    return(rect.y1 - rect.y0);
}

static f32
rect_width(f32_Rect rect){
    return(rect.x1 - rect.x0);
}

static Vec2
rect_dim(f32_Rect rect){
    return(V2(rect.x1 - rect.x0, rect.y1 - rect.y0));
}

static i32_Rect
intersection_of(i32_Rect a, i32_Rect b)
{
    i32_Rect result;
    result.x0 = Max(a.x0, b.x0);
    result.y0 = Max(a.y0, b.y0);
    result.x1 = Min(a.x1, b.x1);
    result.y1 = Min(a.y1, b.y1);
    return(result);
}

static i32_Rect
union_of(i32_Rect a, i32_Rect b)
{
    i32_Rect result;
    result.x0 = Max(a.x0, b.x0);
    result.y0 = Max(a.y0, b.y0);
    result.x1 = Min(a.x1, b.x1);
    result.y1 = Min(a.y1, b.y1);
    return(result);
}

static f32_Rect
intersection_of(f32_Rect a, f32_Rect b)
{
    f32_Rect result;
    result.x0 = Max(a.x0, b.x0);
    result.y0 = Max(a.y0, b.y0);
    result.x1 = Min(a.x1, b.x1);
    result.y1 = Min(a.y1, b.y1);
    return(result);
}

static f32_Rect
union_of(f32_Rect a, f32_Rect b)
{
    f32_Rect result;
    result.x0 = Max(a.x0, b.x0);
    result.y0 = Max(a.y0, b.y0);
    result.x1 = Min(a.x1, b.x1);
    result.y1 = Min(a.y1, b.y1);
    return(result);
}

// BOTTOM

