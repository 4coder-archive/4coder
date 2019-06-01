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

static u32 cp_min_by_utf8_length[] = {
    0x0,
    0x0,
    0x80,
    0x800,
    0x10000,
};

static u32 surrogate_min = 0xD800;
static u32 surrogate_max = 0xDFFF;

static u32 nonchar_min = 0xFDD0;
static u32 nonchar_max = 0xFDEF;

static b32
codepoint_is_whitespace(u32 codepoint){
    b32 result = false;
    if (codepoint == ' ' || codepoint == '\r' || codepoint == '\n' || codepoint == '\t'){
        result = true;
    }
    return(result);
}

static u32
utf8_to_u32_length_unchecked(u8 *buffer, u32 *length_out){
    u32 result = 0;
    
    if (buffer[0] < 0x80){
        result = (u32)buffer[0];
        *length_out = 1;
    }
    else if (buffer[0] < 0xE0){
        result = ((u32)((buffer[0])&0x1F)) << 6;
        result |= ((u32)((buffer[1])&0x3F));
        *length_out = 2;
    }
    else if (buffer[0] < 0xF0){
        result = ((u32)((buffer[0])&0x0F)) << 12;
        result |= ((u32)((buffer[1])&0x3F)) << 6;
        result |= ((u32)((buffer[2])&0x3F));
        *length_out = 3;
    }
    else{
        result = ((u32)((buffer[0])&0x07)) << 18;
        result |= ((u32)((buffer[1])&0x3F)) << 12;
        result |= ((u32)((buffer[2])&0x3F)) << 6;
        result |= ((u32)((buffer[3])&0x3F));
        *length_out = 4;
    }
    
    if (result < cp_min_by_utf8_length[*length_out] || (result >= surrogate_min && result <= surrogate_max)){
        result = 0;
        *length_out = 0;
    }
    
    return(result);
}

static u32
utf8_to_u32_unchecked(u8 *buffer){
    u32 ignore;
    u32 result = utf8_to_u32_length_unchecked(buffer, &ignore);
    return(result);
}

static u32
utf8_to_u32(u8 **buffer_ptr, u8 *end){
    u8 *buffer = *buffer_ptr;
    u32 limit = (u32)(end - buffer);
    
    u32 length = 0;
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
    else if (buffer[0] < 0xF8){
        length = 4;
    }
    else{
        length = 0;
    }
    
    for (u32 i = 1; i < length; ++i){
        if ((buffer[i] & 0xC0) != 0x80){
            length = 0;
            break;
        }
    }
    
    u32 result = 0;
    if (length != 0 && length <= limit){
        switch (length){
            case 1:
            {
                result = (u32)buffer[0];
            }break;
            
            case 2:
            {
                result = ((u32)((buffer[0])&0x1F)) << 6;
                result |= ((u32)((buffer[1])&0x3F));
            }break;
            
            case 3:
            {
                result = ((u32)((buffer[0])&0x0F)) << 12;
                result |= ((u32)((buffer[1])&0x3F)) << 6;
                result |= ((u32)((buffer[2])&0x3F));
            }break;
            
            case 4:
            {
                result = ((u32)((buffer[0])&0x07)) << 18;
                result |= ((u32)((buffer[1])&0x3F)) << 12;
                result |= ((u32)((buffer[2])&0x3F)) << 6;
                result |= ((u32)((buffer[3])&0x3F));
            }break;
        }
        
        if (result < cp_min_by_utf8_length[length] || (result >= surrogate_min && result <= surrogate_max)){
            result = 0;
            length = 0;
        }
        
        *buffer_ptr = buffer + length;
    }
    else{
        *buffer_ptr = end;
    }
    
    return(result);
}

static void
u32_to_utf8_unchecked(u32 codepoint, u8 *buffer, u32 *length_out){
    if (codepoint <= 0x7F){
        buffer[0] = (u8)codepoint;
        *length_out = 1;
    }
    else if (codepoint <= 0x7FF){
        buffer[0] = (u8)(0xC0 | (codepoint >> 6));
        buffer[1] = (u8)(0x80 | (codepoint & 0x3F));
        *length_out = 2;
    }
    else if (codepoint <= 0xFFFF){
        buffer[0] = (u8)(0xE0 | (codepoint >> 12));
        buffer[1] = (u8)(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[2] = (u8)(0x80 | (codepoint & 0x3F));
        *length_out = 3;
    }
    else if (codepoint <= 0x10FFFF){
        codepoint &= 0x001FFFFF;
        buffer[0] = (u8)(0xF0 | (codepoint >> 18));
        buffer[1] = (u8)(0x80 | ((codepoint >> 12) & 0x3F));
        buffer[2] = (u8)(0x80 | ((codepoint >> 6) & 0x3F));
        buffer[3] = (u8)(0x80 | (codepoint & 0x3F));
        *length_out = 4;
    }
    else{
        *length_out = 0;
    }
}

static umem
utf8_to_utf16_minimal_checking(u16 *dst, umem max_wchars, u8 *src, umem length, b32 *error){
    u8 *s = src;
    u8 *s_end = s + length;
    
    u16 *d = dst;
    u16 *d_end = d + max_wchars;
    umem limit = length;
    
    umem needed_max = 0;
    u32 advance = 1;
    
    *error = false;
    for(; s < s_end;){
        u32 codepoint = 0;
        u32 utf8_size = 0;
        
        if (s[0] < 0x80){
            codepoint = (u32)s[0];
            utf8_size = 1;
        }
        else if (s[0] < 0xE0){
            if (limit <= 1){
                *error = true;
                break;
            }
            
            codepoint = ((u32)((s[0])&0x1F)) << 6;
            codepoint |= ((u32)((s[1])&0x3F));
            utf8_size = 2;
        }
        else if (s[0] < 0xF0){
            if (limit <= 2){
                *error = true;
                break;
            }
            
            codepoint = ((u32)((s[0])&0x0F)) << 12;
            codepoint |= ((u32)((s[1])&0x3F)) << 6;
            codepoint |= ((u32)((s[2])&0x3F));
            utf8_size = 3;
        }
        else if (s[0] < 0xF8){
            if (limit > 3){
                *error = true;
                break;
            }
            
            codepoint = ((u32)((s[0])&0x07)) << 18;
            codepoint |= ((u32)((s[1])&0x3F)) << 12;
            codepoint |= ((u32)((s[2])&0x3F)) << 6;
            codepoint |= ((u32)((s[3])&0x3F));
            utf8_size = 4;
        }
        else{
            *error = true;
            break;
        }
        
        if (codepoint < cp_min_by_utf8_length[utf8_size]){
            *error = true;
            break;
        }
        
        s += utf8_size;
        limit -= utf8_size;
        
        if (codepoint <= 0xD7FF || (codepoint >= 0xE000 && codepoint <= 0xFFFF)){
            *d = (u16)(codepoint);
            d += advance;
            needed_max += 1;
        }
        else if (codepoint >= 0x10000 && codepoint <= 0x10FFFF){
            codepoint -= 0x10000;
            
            u32 high = (codepoint >> 10) & 0x03FF;
            u32 low = (codepoint) & 0x03FF;
            
            high += 0xD800;
            low += 0xDC00;
            
            if (d + advance < d_end){
                *d = (u16)high;
                d += advance;
                *d = (u16)low;
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

static umem
utf16_to_utf8_minimal_checking(u8 *dst, umem max_chars, u16 *src, umem length, b32 *error){
    u16 *s = src;
    u16 *s_end = s + length;
    
    u8 *d = dst;
    u8 *d_end = d + max_chars;
    umem limit = length;
    
    umem needed_max = 0;
    
    *error = false;
    
    for (; s < s_end;){
        u32 codepoint = 0;
        u32 utf16_size = 0;
        
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
                
                u32 high = s[0] - 0xD800;
                u32 low = s[1] - 0xDC00;
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
        
        u8 d_fill[4];
        u32 d_fill_count = 0;
        
        if (codepoint <= 0x7F){
            d_fill[0] = (u8)codepoint;
            d_fill_count = 1;
        }
        else if (codepoint <= 0x7FF){
            d_fill[0] = (u8)(0xC0 | (codepoint >> 6));
            d_fill[1] = (u8)(0x80 | (codepoint & 0x3F));
            d_fill_count = 2;
        }
        else if (codepoint <= 0xFFFF){
            d_fill[0] = (u8)(0xE0 | (codepoint >> 12));
            d_fill[1] = (u8)(0x80 | ((codepoint >> 6) & 0x3F));
            d_fill[2] = (u8)(0x80 | (codepoint & 0x3F));
            d_fill_count = 3;
        }
        else if (codepoint <= 0x10FFFF){
            d_fill[0] = (u8)(0xF0 | (codepoint >> 18));
            d_fill[1] = (u8)(0x80 | ((codepoint >> 12) & 0x3F));
            d_fill[2] = (u8)(0x80 | ((codepoint >> 6) & 0x3F));
            d_fill[3] = (u8)(0x80 | (codepoint & 0x3F));
            d_fill_count = 4;
        }
        else{
            *error = true;
            break;
        }
        
        if (d + d_fill_count <= d_end){
            for (u32 i = 0; i < d_fill_count; ++i){
                *d = d_fill[i];
                ++d;
            }
        }
        needed_max += d_fill_count;
    }
    
    return(needed_max);
}

static void
byte_to_ascii(u8 n, u8 *out){
    u8 C = '0' + (n / 0x10);
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

