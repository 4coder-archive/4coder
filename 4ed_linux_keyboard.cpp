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
keycode_init(Display* dpy, Key_Codes *codes){
    set_dynamic_key_names(codes);

   /* NOTE(inso): these are for XInput, currently not used.

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
    */

#define XK_(x) XK_##x
#define XKEY(x) keycode_lookup_table[XKeysymToKeycode(dpy, XK_(x))]

    XKEY(BackSpace) = codes->back;
    XKEY(Delete) = codes->del;
    XKEY(Up) = codes->up;
    XKEY(Down) = codes->down;
    XKEY(Left) = codes->left;
    XKEY(Right) = codes->right;
    XKEY(Insert) = codes->insert;
    XKEY(Home) = codes->home;
    XKEY(End) = codes->end;
    XKEY(Page_Up) = codes->page_up;
    XKEY(Page_Down) = codes->page_down;
    XKEY(Escape) = codes->esc;

#undef XKEY
#undef XK_

}

// BOTTOM

