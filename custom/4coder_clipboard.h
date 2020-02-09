/*
4coder_clipboard.cpp - Copy paste commands and clipboard related setup.
*/

// TOP

#ifndef FCODER_CLIPBOARD_H
#define FCODER_CLIPBOARD_H

struct Clipboard{
    Arena arena;
    Heap heap;
    String_Const_u8 *clips;
    u32 clip_index;
    u32 clip_capacity;
};

#endif //4CODER_CLIPBOARD_H

// BOTTOM

