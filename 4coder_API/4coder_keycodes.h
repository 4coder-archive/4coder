// 55296
enum{
    key_back = 55296 + 0,
    key_up = 55296 + 1,
    key_down = 55296 + 2,
    key_left = 55296 + 3,
    key_right = 55296 + 4,
    key_del = 55296 + 5,
    key_insert = 55296 + 6,
    key_home = 55296 + 7,
    key_end = 55296 + 8,
    key_page_up = 55296 + 9,
    key_page_down = 55296 + 10,
    key_esc = 55296 + 11,
    key_pause = 55296 + 12,
    key_shift = 55296 + 13,
    key_ctrl = 55296 + 14,
    key_alt = 55296 + 15,
    key_cmnd = 55296 + 16,
    key_caps = 55296 + 17,
    key_num_lock = 55296 + 18,
    key_scroll_lock = 55296 + 19,
    key_menu = 55296 + 20,
    key_mouse_left = 55296 + 21,
    key_mouse_right = 55296 + 22,
    key_mouse_left_release = 55296 + 23,
    key_mouse_right_release = 55296 + 24,
    key_mouse_wheel = 55296 + 25,
    key_mouse_move = 55296 + 26,
    key_animate = 55296 + 27,
    key_click_activate_view = 55296 + 28,
    key_click_deactivate_view = 55296 + 29,
    key_f1 = 55296 + 30,
    key_f2 = 55296 + 31,
    key_f3 = 55296 + 32,
    key_f4 = 55296 + 33,
    key_f5 = 55296 + 34,
    key_f6 = 55296 + 35,
    key_f7 = 55296 + 36,
    key_f8 = 55296 + 37,
    key_f9 = 55296 + 38,
    key_f10 = 55296 + 39,
    key_f11 = 55296 + 40,
    key_f12 = 55296 + 41,
    key_f13 = 55296 + 42,
    key_f14 = 55296 + 43,
    key_f15 = 55296 + 44,
    key_f16 = 55296 + 45,
};
static char*
global_key_name(uint32_t key_code, int32_t *size){
    char *result = 0;
    switch(key_code){
        case key_back: result = "key_back"; *size = sizeof("key_back")-1; break;
        case key_up: result = "key_up"; *size = sizeof("key_up")-1; break;
        case key_down: result = "key_down"; *size = sizeof("key_down")-1; break;
        case key_left: result = "key_left"; *size = sizeof("key_left")-1; break;
        case key_right: result = "key_right"; *size = sizeof("key_right")-1; break;
        case key_del: result = "key_del"; *size = sizeof("key_del")-1; break;
        case key_insert: result = "key_insert"; *size = sizeof("key_insert")-1; break;
        case key_home: result = "key_home"; *size = sizeof("key_home")-1; break;
        case key_end: result = "key_end"; *size = sizeof("key_end")-1; break;
        case key_page_up: result = "key_page_up"; *size = sizeof("key_page_up")-1; break;
        case key_page_down: result = "key_page_down"; *size = sizeof("key_page_down")-1; break;
        case key_esc: result = "key_esc"; *size = sizeof("key_esc")-1; break;
        case key_pause: result = "key_pause"; *size = sizeof("key_pause")-1; break;
        case key_shift: result = "key_shift"; *size = sizeof("key_shift")-1; break;
        case key_ctrl: result = "key_ctrl"; *size = sizeof("key_ctrl")-1; break;
        case key_alt: result = "key_alt"; *size = sizeof("key_alt")-1; break;
        case key_cmnd: result = "key_cmnd"; *size = sizeof("key_cmnd")-1; break;
        case key_caps: result = "key_caps"; *size = sizeof("key_caps")-1; break;
        case key_num_lock: result = "key_num_lock"; *size = sizeof("key_num_lock")-1; break;
        case key_scroll_lock: result = "key_scroll_lock"; *size = sizeof("key_scroll_lock")-1; break;
        case key_menu: result = "key_menu"; *size = sizeof("key_menu")-1; break;
        case key_mouse_left: result = "key_mouse_left"; *size = sizeof("key_mouse_left")-1; break;
        case key_mouse_right: result = "key_mouse_right"; *size = sizeof("key_mouse_right")-1; break;
        case key_mouse_left_release: result = "key_mouse_left_release"; *size = sizeof("key_mouse_left_release")-1; break;
        case key_mouse_right_release: result = "key_mouse_right_release"; *size = sizeof("key_mouse_right_release")-1; break;
        case key_mouse_wheel: result = "key_mouse_wheel"; *size = sizeof("key_mouse_wheel")-1; break;
        case key_mouse_move: result = "key_mouse_move"; *size = sizeof("key_mouse_move")-1; break;
        case key_animate: result = "key_animate"; *size = sizeof("key_animate")-1; break;
        case key_click_activate_view: result = "key_click_activate_view"; *size = sizeof("key_click_activate_view")-1; break;
        case key_click_deactivate_view: result = "key_click_deactivate_view"; *size = sizeof("key_click_deactivate_view")-1; break;
        case key_f1: result = "key_f1"; *size = sizeof("key_f1")-1; break;
        case key_f2: result = "key_f2"; *size = sizeof("key_f2")-1; break;
        case key_f3: result = "key_f3"; *size = sizeof("key_f3")-1; break;
        case key_f4: result = "key_f4"; *size = sizeof("key_f4")-1; break;
        case key_f5: result = "key_f5"; *size = sizeof("key_f5")-1; break;
        case key_f6: result = "key_f6"; *size = sizeof("key_f6")-1; break;
        case key_f7: result = "key_f7"; *size = sizeof("key_f7")-1; break;
        case key_f8: result = "key_f8"; *size = sizeof("key_f8")-1; break;
        case key_f9: result = "key_f9"; *size = sizeof("key_f9")-1; break;
        case key_f10: result = "key_f10"; *size = sizeof("key_f10")-1; break;
        case key_f11: result = "key_f11"; *size = sizeof("key_f11")-1; break;
        case key_f12: result = "key_f12"; *size = sizeof("key_f12")-1; break;
        case key_f13: result = "key_f13"; *size = sizeof("key_f13")-1; break;
        case key_f14: result = "key_f14"; *size = sizeof("key_f14")-1; break;
        case key_f15: result = "key_f15"; *size = sizeof("key_f15")-1; break;
        case key_f16: result = "key_f16"; *size = sizeof("key_f16")-1; break;
    }
    return(result);
}
