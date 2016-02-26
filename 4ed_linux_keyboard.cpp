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
keycode_init(Display* dpy){
#if 0
    // NOTE(inso): these are for XInput, currently not used.

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
#endif

    // NOTE(inso): This looks a bit dumb, but it's the best way I can think of to do it, since:
    // KeySyms are the type representing "virtual" keys, like XK_BackSpace, but they are 32-bit ints.
    // KeyCodes are guaranteed to fit in 1 byte (and therefore the keycode_lookup_table) but
    // have dynamic numbers assigned by the XServer.
    //  There is XKeysymToKeycode, but it only returns 1 KeyCode for a KeySym. I have my capslock
    // rebound to esc, so there are two KeyCodes for the XK_Escape KeyCode but XKeysymToKeycode only
    // gets one of them, hence the need for this crazy lookup which works correctly with rebound keys.

    memset(keycode_lookup_table, 0, sizeof(keycode_lookup_table));

    struct SymMapping {
        KeySym sym;
        Code code;
    } sym_table[] = {
        { XK_BackSpace, key_back },
        { XK_Delete, key_del },
        { XK_Up, key_up },
        { XK_Down, key_down },
        { XK_Left, key_left },
        { XK_Right, key_right },
        { XK_Insert, key_insert },
        { XK_Home, key_home },
        { XK_End, key_end },
        { XK_Page_Up, key_page_up },
        { XK_Page_Down, key_page_down },
        { XK_Escape, key_esc }
    };

    const int table_size = sizeof(sym_table) / sizeof(struct SymMapping);

    int key_min, key_max, syms_per_code;
    XDisplayKeycodes(dpy, &key_min, &key_max);

    int key_count = (key_max - key_min) + 1;

    KeySym* syms = XGetKeyboardMapping(
        dpy,
        key_min,
        key_count,
        &syms_per_code
    );

    if(!syms) return;

    int key = key_min;
    for(int i = 0; i < key_count * syms_per_code; ++i){
        for(int j = 0; j < table_size; ++j){
            if(sym_table[j].sym == syms[i]){
                keycode_lookup_table[key + (i/syms_per_code)] = sym_table[j].code;
                break;
            }
        }
    }

    XFree(syms);

}

// BOTTOM

