/*
 * Serial inserts helpers
 */

// TOP

#if !defined(FRED_INSERTION_H)
#define FRED_INSERTION_H

struct Buffer_Insertion{
    Application_Links *app;
    Buffer_ID buffer;
    i32 at;
    b32 buffering;
    Partition *part;
    Temp_Memory temp;
};

#endif

// BOTTOM

