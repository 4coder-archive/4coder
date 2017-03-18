/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.03.2017
 *
 * Abstract model for the describing the characters of a buffer.
 *
 */

// TOP

#if !defined(FRED_BUFFER_MODEL_H)
#define FRED_BUFFER_MODEL_H

struct Buffer_Model_Step{
    u32 type;
    u32 value;
    i32 i;
    u32 byte_length;
};

struct Buffer_Model_Behavior{
    b32 do_newline;
    b32 do_codepoint_advance;
    b32 do_number_advance;
};

enum{
    BufferModelUnit_None,
    BufferModelUnit_Codepoint,
    BufferModelUnit_Numbers,
};

#endif

// BOTTOM

