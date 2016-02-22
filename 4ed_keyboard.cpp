/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 16.11.2014
 *
 * Keyboard layer for 4coder
 *
 */

// TOP

globalvar u8 keycode_lookup_table[255];

internal void
set_dynamic_key_names(Key_Codes *codes){
	u8 code = 1;
	u8 *codes_array = (u8*)codes;
	for (i32 i = 0; i < sizeof(*codes)/sizeof(codes->up);){
        switch (code){
        case '\n': code++; break;
        case '\t': code++; break;
        case 0x20: code = 0x7F; break;
        default:
            codes_array[i++] = code++;
        }
	}
}

inline u8
keycode_lookup(u8 system_code){
	return keycode_lookup_table[system_code];
}

// BOTTOM

