/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 16.11.2014
 *
 * Win32-US Keyboard layer for 4coder
 *
 */

// TOP

globalvar u16 keycode_lookup_table[255];
globalvar u16 loose_keycode_lookup_table[255];

internal void
set_dynamic_key_names(Key_Codes *codes){
	u16 code = 1;
	u16 *codes_array = (u16*)codes;
	for (i32 i = 0; i < sizeof(*codes)/sizeof(codes->up);){
        switch (code){
        case '\n': code++; break;
        case '\t': code++; break;
        case 0x20: code = 0x7F; break;
        }
        codes_array[i++] = code++;
	}
}

inline u16
keycode_lookup(u8 system_code){
	return keycode_lookup_table[system_code];
}

inline u16
loose_keycode_lookup(u8 system_code){
	return loose_keycode_lookup_table[system_code];
}

inline b32
keycode_has_ascii(u16 keycode){
    return (keycode >= 0x20 && keycode < 0x7F) || keycode == '\n' || keycode == '\t';
}

internal u8
translate_key(u16 keycode,
              b32 shift,
              b32 caps_lock){
    u8 character = 0;
    if (keycode >= 'A' && keycode <= 'Z'){
        if (caps_lock) shift = !shift;
        if (!shift) character = char_to_lower((char)keycode);
        else character = (u8)keycode;
    }
    else if (keycode >= '0' && keycode <= '9'){
        persist u8 shift_number_table[10] = {
            ')', '!', '@', '#', '$', '%', '^', '&', '*', '('
            //0   1    2    3    4    5    6    7    8    9
        };
        if (shift){
            character = shift_number_table[keycode - '0'];
        }
        else{
            character = (u8)keycode;
        }
    }
    else{
        if (keycode_has_ascii(keycode)){
            character = (u8)keycode;
            u8 shift_character = character;
            switch (keycode){
            case '-': character = '-'; shift_character = '_'; break;
            case '=': character = '='; shift_character = '+'; break;
            case '`': character = '`'; shift_character = '~'; break;
            case '\\': character = '\\'; shift_character = '|'; break;
            case '[': character = '['; shift_character = '{'; break;
            case ']': character = ']'; shift_character = '}'; break;
            case '\'': character = '\''; shift_character = '"'; break;
            case ';': character = ';'; shift_character = ':'; break;
            case '/': character = '/'; shift_character = '?'; break;
            case '.': character = '.'; shift_character = '>'; break;
            case ',': character = ','; shift_character = '<'; break;
            case ' ': character = ' '; shift_character = ' '; break;
            case '\n': character = '\n'; shift_character = '\n'; break;
            case '\t': character = '\t'; shift_character = '\t'; break;
            }
            if (shift) character = shift_character;
        }
    }
    return character;
}

// BOTTOM

