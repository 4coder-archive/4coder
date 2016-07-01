enum Key_Code_Names{
key_back = 1,
key_up = 2,
key_down = 3,
key_left = 4,
key_right = 5,
key_del = 6,
key_insert = 7,
key_home = 8,
key_end = 11,
key_page_up = 12,
key_page_down = 13,
key_esc = 14,
key_mouse_left = 15,
key_mouse_right = 16,
key_f1 = 127,
key_f2 = 128,
key_f3 = 129,
key_f4 = 130,
key_f5 = 131,
key_f6 = 132,
key_f7 = 133,
key_f8 = 134,
key_f9 = 135,
key_f10 = 136,
key_f11 = 137,
key_f12 = 138,
key_f13 = 139,
key_f14 = 140,
key_f15 = 141,
key_f16 = 142,
};
static char*
global_key_name(int key_code, int *size){
char *result = 0;
switch(key_code){
case key_back: result = "back"; *size = sizeof("back")-1; break;
case key_up: result = "up"; *size = sizeof("up")-1; break;
case key_down: result = "down"; *size = sizeof("down")-1; break;
case key_left: result = "left"; *size = sizeof("left")-1; break;
case key_right: result = "right"; *size = sizeof("right")-1; break;
case key_del: result = "del"; *size = sizeof("del")-1; break;
case key_insert: result = "insert"; *size = sizeof("insert")-1; break;
case key_home: result = "home"; *size = sizeof("home")-1; break;
case key_end: result = "end"; *size = sizeof("end")-1; break;
case key_page_up: result = "page_up"; *size = sizeof("page_up")-1; break;
case key_page_down: result = "page_down"; *size = sizeof("page_down")-1; break;
case key_esc: result = "esc"; *size = sizeof("esc")-1; break;
case key_mouse_left: result = "mouse_left"; *size = sizeof("mouse_left")-1; break;
case key_mouse_right: result = "mouse_right"; *size = sizeof("mouse_right")-1; break;
case key_f1: result = "f1"; *size = sizeof("f1")-1; break;
case key_f2: result = "f2"; *size = sizeof("f2")-1; break;
case key_f3: result = "f3"; *size = sizeof("f3")-1; break;
case key_f4: result = "f4"; *size = sizeof("f4")-1; break;
case key_f5: result = "f5"; *size = sizeof("f5")-1; break;
case key_f6: result = "f6"; *size = sizeof("f6")-1; break;
case key_f7: result = "f7"; *size = sizeof("f7")-1; break;
case key_f8: result = "f8"; *size = sizeof("f8")-1; break;
case key_f9: result = "f9"; *size = sizeof("f9")-1; break;
case key_f10: result = "f10"; *size = sizeof("f10")-1; break;
case key_f11: result = "f11"; *size = sizeof("f11")-1; break;
case key_f12: result = "f12"; *size = sizeof("f12")-1; break;
case key_f13: result = "f13"; *size = sizeof("f13")-1; break;
case key_f14: result = "f14"; *size = sizeof("f14")-1; break;
case key_f15: result = "f15"; *size = sizeof("f15")-1; break;
case key_f16: result = "f16"; *size = sizeof("f16")-1; break;
}
return(result);
}
