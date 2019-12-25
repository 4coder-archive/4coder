/* Types and functions for communication between C++ and Objective-C layers. */

#if !defined(MAC_OBJECTIVE_C_TO_CPP_LINKS_H)
#define MAC_OBJECTIVE_C_TO_CPP_LINKS_H

// In C++ layer.
external void
mac_init();

// In Objective-C layer.
external i32
mac_get_binary_path(void* buffer, u32 size);

#endif

