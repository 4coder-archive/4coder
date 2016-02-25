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

inline u8
keycode_lookup(u8 system_code){
	return keycode_lookup_table[system_code];
}

// BOTTOM

