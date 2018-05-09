/*
4coder_jumping.h - Types used in jumping.
*/

// TOP

#if !defined(FCODER_JUMPING_H)
#define FCODER_JUMPING_H

struct ID_Pos_Jump_Location{
    Buffer_ID buffer_id;
    int32_t pos;
};

struct Name_Based_Jump_Location{
    String file;
    int32_t line;
    int32_t column;
};

#endif

// BOTTOM

