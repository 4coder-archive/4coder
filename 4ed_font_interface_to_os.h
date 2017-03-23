/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 13.03.2017
 *
 * Font system interface to the OS layer.
 *
 */

// TOP

#if !defined(FCODER_FONT_INTERFACE_TO_OS_H)
#define FCODER_FONT_INTERFACE_TO_OS_H

#define Sys_Font_Init_Sig(name_) void (name_)(Font_Functions *font, void *memory, umem memory_size, u32 font_size, b32 use_hinting)
internal Sys_Font_Init_Sig(system_font_init);

#endif

// BOTTOM



