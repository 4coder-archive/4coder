/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 24.01.2018
 *
 * Buffer types
 *
 */

// TOP

#if !defined(FRED_TRANSLATION_H)
#define FRED_TRANSLATION_H

struct Translation_State{
    u8 fill_buffer[4];
    u32 fill_start_i;
    u8 fill_i;
    u8 fill_expected;
};

enum{
    TranLBH_None,
    TranLBH_Rebuffer,
    TranLBH_EmitAsCP,
};
struct Translation_Byte_Description{
    u8 byte_class;
    u8 last_byte_handler;
    u8 prelim_emit_type;
};

struct Translation_Emit_Rule{
    u8 byte_class;
    u8 last_byte_handler;
    u8 emit_type;
    
    u32 codepoint;
    u32 codepoint_length;
};

struct Translation_Emits{
    Buffer_Model_Step steps[5];
    u32 step_count;
};

#endif

// BOTTOM

