/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.11.2015
 *
 * Linux-US Keyboard layer for 4coder
 *
 */

// TOP

#include "4ed_keyboard.cpp"

internal void
keycode_init(Key_Codes *codes){
    set_dynamic_key_names(codes);
    
    keycode_lookup_table[KEY_BACKSPACE] = codes->back;
    keycode_lookup_table[KEY_DELETE] = codes->del;
    keycode_lookup_table[KEY_UP] = codes->up;
    keycode_lookup_table[KEY_DOWN] = codes->down;
    keycode_lookup_table[KEY_LEFT] = codes->left;
    keycode_lookup_table[KEY_RIGHT] = codes->right;
    keycode_lookup_table[KEY_INSERT] = codes->insert;
    keycode_lookup_table[KEY_HOME] = codes->home;
    keycode_lookup_table[KEY_END] = codes->end;
    keycode_lookup_table[KEY_PAGEUP] = codes->page_up;
    keycode_lookup_table[KEY_PAGEDOWN] = codes->page_down;
    keycode_lookup_table[KEY_ESC] = codes->esc;
}

// BOTTOM

