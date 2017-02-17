/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 17.02.2017
 *
 * File for reading and writing utf8.
 *
 */

// TOP

#if !defined(FED_UTF8_CONVERSION_H)
#define FED_UTF8_CONVERSION_H

internal u32
utf8_to_u32_unchecked(u8 *buffer){
    u32 result = 0;
    
    if (buffer[0] <= 0x7F){
        result = (u32)buffer[0];
    }
    else if (buffer[0] <= 0xE0){
        result = ((u32)((buffer[0])&0x1F)) << 6;
        result |= ((u32)((buffer[1])&0x3F));
    }
    else if (buffer[0] <= 0xF0){
        result = ((u32)((buffer[0])&0x0F)) << 12;
        result |= ((u32)((buffer[1])&0x3F)) << 6;
        result |= ((u32)((buffer[2])&0x3F));
    }
    else{
        result = ((u32)((buffer[0])&0x07)) << 18;
        result |= ((u32)((buffer[1])&0x3F)) << 12;
        result |= ((u32)((buffer[2])&0x3F)) << 6;
        result |= ((u32)((buffer[3])&0x3F));
    }
    
    return(result);
}


#endif

// BOTTOM

