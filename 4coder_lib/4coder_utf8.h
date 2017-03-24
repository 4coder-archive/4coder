/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 17.02.2017
 *
 * Code for converting to and from utf8 to ANSI and utf16 text encodings.
 *
 */

// TOP

#if !defined(FED_UTF8_CONVERSION_H)
#define FED_UTF8_CONVERSION_H

// 4tech_standard_preamble.h
#if !defined(FTECH_INTEGERS)
#define FTECH_INTEGERS
#include <stdint.h>
typedef int8_t i8_4tech;
typedef int16_t i16_4tech;
typedef int32_t i32_4tech;
typedef int64_t i64_4tech;

typedef uint8_t u8_4tech;
typedef uint16_t u16_4tech;
typedef uint32_t u32_4tech;
typedef uint64_t u64_4tech;

typedef u64_4tech umem_4tech;

typedef float f32_4tech;
typedef double f64_4tech;

typedef int8_t b8_4tech;
typedef int32_t b32_4tech;
#endif

#if !defined(Assert)
# define Assert(n) do{ if (!(n)) *(int*)0 = 0xA11E; }while(0)
#endif
// standard preamble end 

static b32_4tech
codepoint_is_whitespace(u32_4tech codepoint){
    b32_4tech result = false;
    if (codepoint == ' ' || codepoint == '\r' || codepoint == '\n' || codepoint == '\t'){
        result = true;
    }
    return(result);
}

static u32_4tech
utf8_to_u32_length_unchecked(u8_4tech *buffer, u32_4tech *length_out){
    u32_4tech result = 0;
    
    if (buffer[0] < 0x80){
        result = (u32_4tech)buffer[0];
        *length_out = 1;
    }
    else if (buffer[0] < 0xE0){
        result = ((u32_4tech)((buffer[0])&0x1F)) << 6;
        result |= ((u32_4tech)((buffer[1])&0x3F));
        *length_out = 2;
    }
    else if (buffer[0] < 0xF0){
        result = ((u32_4tech)((buffer[0])&0x0F)) << 12;
        result |= ((u32_4tech)((buffer[1])&0x3F)) << 6;
        result |= ((u32_4tech)((buffer[2])&0x3F));
        *length_out = 3;
    }
    else{
        result = ((u32_4tech)((buffer[0])&0x07)) << 18;
        result |= ((u32_4tech)((buffer[1])&0x3F)) << 12;
        result |= ((u32_4tech)((buffer[2])&0x3F)) << 6;
        result |= ((u32_4tech)((buffer[3])&0x3F));
        *length_out = 4;
    }
    
    return(result);
}

static u32_4tech
utf8_to_u32_unchecked(u8_4tech *buffer){
    u32_4tech ignore;
    u32_4tech result = utf8_to_u32_length_unchecked(buffer, &ignore);
    return(result);
}

static u32_4tech
utf8_to_u32(u8_4tech **buffer_ptr, u8_4tech *end){
    u8_4tech *buffer = *buffer_ptr;
    u32_4tech limit = (u32_4tech)(end - buffer);
    
    u32_4tech length = 0;
    if (buffer[0] < 0x80){
        length = 1;
    }
    else if (buffer[0] < 0xC0){
        length = 0;
    }
    else if (buffer[0] < 0xE0){
        length = 2;
    }
    else if (buffer[0] < 0xF0){
        length = 3;
    }
    else{
        length = 4;
    }
    
    for (u32_4tech i = 1; i < length; ++i){
        if ((buffer[i] & 0xC0) != 0x80){
            length = 0;
            break;
        }
    }
    
    u32_4tech result = 0;
    if (length != 0 && length <= limit){
        switch (length){
            case 1:
            {
                result = (u32_4tech)buffer[0];
            }break;
            
            case 2:
            {
                result = ((u32_4tech)((buffer[0])&0x1F)) << 6;
                result |= ((u32_4tech)((buffer[1])&0x3F));
            }break;
            
            case 3:
            {
                result = ((u32_4tech)((buffer[0])&0x0F)) << 12;
                result |= ((u32_4tech)((buffer[1])&0x3F)) << 6;
                result |= ((u32_4tech)((buffer[2])&0x3F));
            }break;
            
            case 4:
            {
                result = ((u32_4tech)((buffer[0])&0x07)) << 18;
                result |= ((u32_4tech)((buffer[1])&0x3F)) << 12;
                result |= ((u32_4tech)((buffer[2])&0x3F)) << 6;
                result |= ((u32_4tech)((buffer[3])&0x3F));
            }break;
        }
        
        *buffer_ptr = buffer + length;
    }
    else{
        *buffer_ptr = end;
    }
    
    return(result);
}

static void
u32_to_utf8_unchecked(u32_4tech codepoint, u8_4tech *buffer, u32_4tech *length_out){
    if (codepoint <= 0x7F){
        buffer[0] = (u8_4tech)codepoint;
        *length_out = 1;
    }
    else if (codepoint <= 0x7FF){
        buffer[0] = (u8_4tech)(0xC0 | (codepoint >> 6));
        buffer[1] = (u8_4tech)(0x80 | (codepoint & 0x3F));
        *length_out = 2;
    }
    else if (codepoint <= 0xFFFF){
        buffer[0] = (u8_4tech)(0xE0 | (codepoint >> 12));
        buffer[1] = (u8_4tech)(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[2] = (u8_4tech)(0x80 | (codepoint & 0x3F));
        *length_out = 3;
    }
    else{
        codepoint &= 0x001FFFFF;
        buffer[0] = (u8_4tech)(0xF0 | (codepoint >> 18));
        buffer[1] = (u8_4tech)(0x80 | ((codepoint >> 12) & 0x3F));
        buffer[2] = (u8_4tech)(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[3] = (u8_4tech)(0x80 | (codepoint & 0x3F));
        *length_out = 4;
    }
}

static umem_4tech
utf8_to_utf16_minimal_checking(u16_4tech *dst, umem_4tech max_wchars, u8_4tech *src, umem_4tech length, b32_4tech *error){
    u8_4tech *s = src;
    u8_4tech *s_end = s + length;
    
    u16_4tech *d = dst;
    u16_4tech *d_end = d + max_wchars;
    umem_4tech limit = length;
    
    umem_4tech needed_max = 0;
    u32_4tech advance = 1;
    
    *error = false;
    for(; s < s_end;){
        u32_4tech codepoint = 0;
        u32_4tech utf8_size = 0;
        
        if (s[0] < 0x80){
            codepoint = (u32_4tech)s[0];
            utf8_size = 1;
        }
        else if (s[0] < 0xE0){
            if (limit <= 1){
                *error = true;
                break;
            }
            
            codepoint = ((u32_4tech)((s[0])&0x1F)) << 6;
            codepoint |= ((u32_4tech)((s[1])&0x3F));
            utf8_size = 2;
        }
        else if (s[0] < 0xF0){
            if (limit <= 2){
                *error = true;
                break;
            }
            
            codepoint = ((u32_4tech)((s[0])&0x0F)) << 12;
            codepoint |= ((u32_4tech)((s[1])&0x3F)) << 6;
            codepoint |= ((u32_4tech)((s[2])&0x3F));
            utf8_size = 3;
        }
        else{
            if (limit > 3){
                *error = true;
                break;
            }
            
            codepoint = ((u32_4tech)((s[0])&0x07)) << 18;
            codepoint |= ((u32_4tech)((s[1])&0x3F)) << 12;
            codepoint |= ((u32_4tech)((s[2])&0x3F)) << 6;
            codepoint |= ((u32_4tech)((s[3])&0x3F));
            utf8_size = 4;
        }
        
        s += utf8_size;
        limit -= utf8_size;
        
        if (codepoint <= 0xD7FF || (codepoint >= 0xE000 && codepoint <= 0xFFFF)){
            *d = (u16_4tech)(codepoint);
            d += advance;
            needed_max += 1;
        }
        else if (codepoint >= 0x10000 && codepoint <= 0x10FFFF){
            codepoint -= 0x10000;
            
            u32_4tech high = (codepoint >> 10) & 0x03FF;
            u32_4tech low = (codepoint) & 0x03FF;
            
            high += 0xD800;
            low += 0xDC00;
            
            if (d + advance < d_end){
                *d = (u16_4tech)high;
                d += advance;
                *d = (u16_4tech)low;
                d += advance;
            }
            else{
                advance = 0;
            }
            
            needed_max += 2;
        }
        else{
            *error = true;
            break;
        }
        
        if (d >= d_end){
            advance = 0;
        }
    }
    
    return(needed_max);
}

static umem_4tech
utf16_to_utf8_minimal_checking(u8_4tech *dst, umem_4tech max_chars, u16_4tech *src, umem_4tech length, b32_4tech *error){
    u16_4tech *s = src;
    u16_4tech *s_end = s + length;
    
    u8_4tech *d = dst;
    u8_4tech *d_end = d + max_chars;
    umem_4tech limit = length;
    
    umem_4tech needed_max = 0;
    
    *error = false;
    
    for (; s < s_end;){
        u32_4tech codepoint = 0;
        u32_4tech utf16_size = 0;
        
        if (s[0] <= 0xD7FF || (s[0] >= 0xE000 && s[0] <= 0xFFFF)){
            codepoint = s[0];
            utf16_size = 1;
        }
        else{
            if (s[0] >= 0xD800 && s[0] <= 0xDBFF){
                if (limit <= 1){
                    *error = true;
                    break;
                }
                
                u32_4tech high = s[0] - 0xD800;
                u32_4tech low = s[1] - 0xDC00;
                codepoint = ((high << 10) | (low)) + 0x10000;
                utf16_size = 2;
            }
            else{
                *error = true;
                break;
            }
        }
        
        s += utf16_size;
        limit -= utf16_size;
        
        u8_4tech d_fill[4];
        u32_4tech d_fill_count = 0;
        
        if (codepoint <= 0x7F){
            d_fill[0] = (u8_4tech)codepoint;
            d_fill_count = 1;
        }
        else if (codepoint <= 0x7FF){
            d_fill[0] = (u8_4tech)(0xC0 | (codepoint >> 6));
            d_fill[1] = (u8_4tech)(0x80 | (codepoint & 0x3F));
            d_fill_count = 2;
        }
        else if (codepoint <= 0xFFFF){
            d_fill[0] = (u8_4tech)(0xE0 | (codepoint >> 12));
            d_fill[1] = (u8_4tech)(0x80 | ((codepoint >> 6) & 0x3F));
            d_fill[2] = (u8_4tech)(0x80 | (codepoint & 0x3F));
            d_fill_count = 3;
        }
        else if (codepoint <= 0x10FFFF){
            d_fill[0] = (u8_4tech)(0xF0 | (codepoint >> 18));
            d_fill[1] = (u8_4tech)(0x80 | ((codepoint >> 12) & 0x3F));
            d_fill[2] = (u8_4tech)(0x80 | ((codepoint >> 6) & 0x3F));
            d_fill[3] = (u8_4tech)(0x80 | (codepoint & 0x3F));
            d_fill_count = 4;
        }
        else{
            *error = true;
            break;
        }
        
        if (d + d_fill_count <= d_end){
            for (u32_4tech i = 0; i < d_fill_count; ++i){
                *d = d_fill[i];
                ++d;
            }
        }
        needed_max += d_fill_count;
    }
    
    return(needed_max);
}

static void
byte_to_ascii(u8_4tech n, u8_4tech *out){
    u8_4tech C = '0' + (n / 0x10);
    if ((n / 0x10) > 0x9){
        C = ('A' - 0xA) + (n / 0x10);
    }
    out[0] = C;
    
    n = (n % 0x10);
    C = '0' + n;
    if (n > 0x9){
        C = ('A' - 0xA) + n;
    }
    out[1] = C;
}

#endif

// BOTTOM

