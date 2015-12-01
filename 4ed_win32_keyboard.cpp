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

    u16 code, loose;
    for (u8 i = 0; i < 255; ++i){
		if ((i >= '0' && i <= '9') ||
            (i >= 'A' && i <= 'Z')){
			keycode_lookup_table[i] = i;
			loose_keycode_lookup_table[i] = 0;
        }
                
		else{
            loose = 0;
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
                
            case VK_DELETE: code = loose = codes->del; break;
            case VK_UP: code = loose = codes->up; break;
            case VK_DOWN: code = loose = codes->down; break;
            case VK_LEFT: code = loose = codes->left; break;
            case VK_RIGHT: code = loose = codes->right; break;
                
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

// BOTTOM

