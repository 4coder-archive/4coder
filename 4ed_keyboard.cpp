/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.17.2014
 *
 * Win32-US Keyboard layer for project codename "4ed"
 *
 */

// TOP

globalvar u16 keycode_lookup_table[255];
globalvar u16 loose_keycode_lookup_table[255];

internal void
keycode_init(Key_Codes *codes, Key_Codes *loose_codes){
	// NOTE(allen): Assign values to the global keycodes.
	// Skip over the ascii characters that are used as codes.
	u16 code = 1;
	u16 *codes_array = (u16*)codes;
	for (i32 i = 0; i < sizeof(Key_Codes)/2;){
        switch (code){
        case '\n': code++; break;
        case '\t': code++; break;
        case 0x20: code = 0x7F; break;
            
        default:
            codes_array[i++] = code++;
        }
	}
	
	code = 1;
	codes_array = (u16*)loose_codes;
	for (i32 i = 0; i < sizeof(Key_Codes)/2; ++i){
		codes_array[i] = code++;
	}
	
	// NOTE(allen): lookup table for conversion from
	// win32 vk values to fred_keycode values.
	for (u8 i = 0; i < 255; ++i){
		if ((i >= '0' && i <= '9') ||
			(i >= 'A' && i <= 'Z')){
			keycode_lookup_table[i] = i;
			loose_keycode_lookup_table[i] = 0;
		}
		else{
            
            u16 code, loose = 0;
			switch (i){
            case VK_SPACE: code = loose = ' '; break;
            case VK_BACK: code = loose = codes->back; break;
            case VK_OEM_MINUS: code = '-'; break;
            case VK_OEM_PLUS: code = '='; break;
            case VK_SUBTRACT: code = '-'; break;
            case VK_ADD: code = '+'; break;
            case VK_MULTIPLY: code = '*'; break;
            case VK_DIVIDE: code = '/'; break;
                
            case VK_OEM_3: code = '`'; break;
            case VK_OEM_5: code = '\\'; break;
            case VK_OEM_4: code = '['; break;
            case VK_OEM_6: code = ']'; break;
            case VK_TAB: code = loose = '\t'; break;
            case VK_RETURN: code = loose = '\n'; break;
            case VK_OEM_7: code = '\''; break;
                
            case VK_OEM_1: code = ';'; break;
            case VK_OEM_2: code = '/'; break;
            case VK_OEM_PERIOD: code = '.'; break;
            case VK_OEM_COMMA: code = ','; break;
            case VK_UP: code = loose = codes->up; break;
            case VK_DOWN: code = loose = codes->down; break;
            case VK_LEFT: code = loose = codes->left; break;
            case VK_RIGHT: code = loose = codes->right; break;
            case VK_DELETE: code = loose = codes->del; break;
                
            case VK_INSERT: code = loose = codes->insert; break;
            case VK_HOME: code = loose = codes->home; break;
            case VK_END: code = loose = codes->end; break;
            case VK_PRIOR: code = loose = codes->page_up; break;
            case VK_NEXT: code = loose = codes->page_down; break;
            case VK_ESCAPE: code = loose = codes->esc; break;
                
            case VK_NUMPAD0:
            case VK_NUMPAD1: case VK_NUMPAD2: case VK_NUMPAD3:
            case VK_NUMPAD4: case VK_NUMPAD5: case VK_NUMPAD6:
            case VK_NUMPAD7: case VK_NUMPAD8: case VK_NUMPAD9:
                code = (i - VK_NUMPAD0) + '0'; break;
                
            default: code = 0; break;
			}
            
            keycode_lookup_table[i] = code;
            loose_keycode_lookup_table[i] = loose;
		}
	}
}

inline u16
keycode_lookup(u8 virtual_keycode){
	return keycode_lookup_table[virtual_keycode];
}

inline u16
loose_keycode_lookup(u8 virtual_keycode){
	return loose_keycode_lookup_table[virtual_keycode];
}

inline bool32
keycode_has_ascii(u16 keycode){
    return (keycode >= 0x20 && keycode < 0x7F) || keycode == '\n' || keycode == '\t';
}

internal u8
keycode_to_character_ascii(Key_Codes *codes,
                           u16 keycode,
                           bool32 shift,
                           bool32 caps_lock){
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
