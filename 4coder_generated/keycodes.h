enum{
key_back = 55296,
key_up = 55297,
key_down = 55298,
key_left = 55299,
key_right = 55300,
key_del = 55301,
key_insert = 55302,
key_home = 55303,
key_end = 55304,
key_page_up = 55305,
key_page_down = 55306,
key_esc = 55307,
key_mouse_left = 55308,
key_mouse_right = 55309,
key_mouse_left_release = 55310,
key_mouse_right_release = 55311,
key_mouse_wheel = 55312,
key_mouse_move = 55313,
key_animate = 55314,
key_view_activate = 55315,
key_f1 = 55316,
key_f2 = 55317,
key_f3 = 55318,
key_f4 = 55319,
key_f5 = 55320,
key_f6 = 55321,
key_f7 = 55322,
key_f8 = 55323,
key_f9 = 55324,
key_f10 = 55325,
key_f11 = 55326,
key_f12 = 55327,
key_f13 = 55328,
key_f14 = 55329,
key_f15 = 55330,
key_f16 = 55331,
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
case key_mouse_left: result = "key_mouse_left"; *size = sizeof("key_mouse_left")-1; break;
case key_mouse_right: result = "key_mouse_right"; *size = sizeof("key_mouse_right")-1; break;
case key_mouse_left_release: result = "key_mouse_left_release"; *size = sizeof("key_mouse_left_release")-1; break;
case key_mouse_right_release: result = "key_mouse_right_release"; *size = sizeof("key_mouse_right_release")-1; break;
case key_mouse_wheel: result = "key_mouse_wheel"; *size = sizeof("key_mouse_wheel")-1; break;
case key_mouse_move: result = "key_mouse_move"; *size = sizeof("key_mouse_move")-1; break;
case key_animate: result = "key_animate"; *size = sizeof("key_animate")-1; break;
case key_view_activate: result = "key_view_activate"; *size = sizeof("key_view_activate")-1; break;
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
