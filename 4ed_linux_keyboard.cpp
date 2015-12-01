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

    u16 code, loose;
    for (u16 i = 0; i < 255; ++i){
        if (i >= 'a' && i <= 'z'){
			keycode_lookup_table[i] = i + ('A' - 'a');
			loose_keycode_lookup_table[i] = 0;
        }
        
        else if (i >= '0' && i <= '9'){
			keycode_lookup_table[i] = i;
			loose_keycode_lookup_table[i] = 0;
        }
        
        else{
            loose = 0;
            switch (i){
            }
            
            keycode_lookup_table[i] = code;
            loose_keycode_lookup_table[i] = loose;
        }
    }
}

// BOTTOM

