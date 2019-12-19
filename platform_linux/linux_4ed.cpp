/*
 * chr - Andrew Chronister
 *
 * 12.19.2019
 *
 * Updated linux layer for 4coder
 *
 */

// TOP

#define FPS 60
#define frame_useconds (1000000 / FPS)

#include "4coder_base_types.h"
#include "4coder_version.h"
#include "4coder_events.h"

#include "4coder_table.h"
#include "4coder_types.h"
#include "4coder_default_colors.h"

#include "4coder_system_types.h"
#define STATIC_LINK_API
#include "generated/system_api.h"

#include "4ed_font_interface.h"
#define STATIC_LINK_API
#include "generated/graphics_api.h"
#define STATIC_LINK_API
#include "generated/font_api.h"

#include "4ed_font_set.h"
#include "4ed_render_target.h"
#include "4ed_search_list.h"
#include "4ed.h"

#include "generated/system_api.cpp"
#include "generated/graphics_api.cpp"
#include "generated/font_api.cpp"

#include "4coder_base_types.cpp"
#include "4coder_stringf.cpp"
#include "4coder_events.cpp"
#include "4coder_hash_functions.cpp"
#include "4coder_table.cpp"
#include "4coder_log.cpp"

#include "4ed_search_list.cpp"

#include <unistd.h>

internal
system_get_path_sig(){
    // Arena* arena, System_Path_Code path_code
    String_Const_u8 result = {};
    switch (path_code){
        case SystemPath_CurrentDirectory:
        {
            // glibc extension: getcwd allocates its own memory if passed NULL
            char *working_dir = getcwd(NULL, 0);
            u64 working_dir_len = cstring_length(working_dir);
            u8 *out = push_array(arena, u8, working_dir_len);
            block_copy(out, working_dir, working_dir_len);
            free(working_dir);
            result = SCu8(out, working_dir_len);
        }break;

        case SystemPath_Binary:
        {
            // linux-specific: binary path symlinked at /proc/self/exe
            ssize_t binary_path_len = readlink("/proc/self/exe", NULL, 0);
            u8* out = push_array(arena, u8, binary_path_len);
            readlink("/proc/self/exe", (char*)out, binary_path_len);
            String_u8 out_str = Su8(out, binary_path_len);
            out_str.string = string_remove_last_folder(out_str.string);
            string_null_terminate(&out_str);
            result = out_str.string;
        }break;
    }
    return(result);
}

int main(int argc, char **argv){
}
