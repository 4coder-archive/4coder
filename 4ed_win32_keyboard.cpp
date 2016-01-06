/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.17.2014
 *
 * Win32-US Keyboard layer for 4coder
 *
 */

// TOP

#include "4ed_keyboard.cpp"

internal void
keycode_init(Key_Codes *codes){
    set_dynamic_key_names(codes);
    
    keycode_lookup_table[VK_BACK] = codes->back;
    keycode_lookup_table[VK_DELETE] = codes->del;
    keycode_lookup_table[VK_UP] = codes->up;
    keycode_lookup_table[VK_DOWN] = codes->down;
    keycode_lookup_table[VK_LEFT] = codes->left;
    keycode_lookup_table[VK_RIGHT] = codes->right;
    keycode_lookup_table[VK_INSERT] = codes->insert;
    keycode_lookup_table[VK_HOME] = codes->home;
    keycode_lookup_table[VK_END] = codes->end;
    keycode_lookup_table[VK_PRIOR] = codes->page_up;
    keycode_lookup_table[VK_NEXT] = codes->page_down;
    keycode_lookup_table[VK_ESCAPE] = codes->esc;
}

// BOTTOM

