/* Types and functions for communication between C++ and Objective-C layers. */

#if !defined(MAC_OBJECTIVE_C_TO_CPP_LINKS_H)
#define MAC_OBJECTIVE_C_TO_CPP_LINKS_H

// In C++ layer
external String_Const_u8
mac_SCu8(u8* str, u64 size);

external String_Const_u8
mac_push_string_copy(Arena *arena, String_Const_u8 src);

external void
mac_init();

// In Objective-C layer
external String_Const_u8
mac_standardize_path(Arena* arena, String_Const_u8 path);

external i32
mac_get_binary_path(void* buffer, u32 size);

#endif

