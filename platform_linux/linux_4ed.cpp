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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

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


internal
system_get_canonical_sig(){
    // TODO(andrew): Resolve symlinks ?
    // TODO(andrew): Resolve . and .. in paths
    // TODO(andrew): Use realpath(3)
	return name;
}


internal int
linux_system_get_file_list_filter(const struct dirent *dirent) {
    String_Const_u8 file_name = SCu8((u8*)dirent->d_name);
    if (string_match(file_name, string_u8_litexpr("."))) {
        return 0;
    }
    else if (string_match(file_name, string_u8_litexpr(".."))) {
        return 0;
    }
    return 1;
}

internal int
linux_u64_from_timespec(const struct timespec timespec) {
    return timespec.tv_nsec + 1000000000 * timespec.tv_sec;
}

internal File_Attribute_Flag
linux_convert_file_attribute_flags(int mode) {
    File_Attribute_Flag result = {};
    MovFlag(mode, S_IFDIR, result, FileAttribute_IsDirectory);
    return result;
}

internal File_Attributes
linux_get_file_attributes(String_Const_u8 file_name) {
    struct stat file_stat;
    stat((const char*)file_name.str, &file_stat);

    File_Attributes result = {};
    result.size = file_stat.st_size;
    result.last_write_time = linux_u64_from_timespec(file_stat.st_mtim);
    result.flags = linux_convert_file_attribute_flags(file_stat.st_mode);
    return result;
}

internal
system_get_file_list_sig(){
    File_List result = {};
    String_Const_u8 search_pattern = {};
    if (character_is_slash(string_get_character(directory, directory.size - 1))){
        search_pattern = push_u8_stringf(arena, "%.*s*", string_expand(directory));
    }
    else{
        search_pattern = push_u8_stringf(arena, "%.*s/*", string_expand(directory));
    }

    struct dirent** dir_ents = NULL;
    int num_ents = scandir(
        (const char*)search_pattern.str, &dir_ents, linux_system_get_file_list_filter, alphasort);

    File_Info *first = 0;
    File_Info *last = 0;
    for (int i = 0; i < num_ents; ++i) {
        struct dirent* dirent = dir_ents[i];
        File_Info *info = push_array(arena, File_Info, 1);
        sll_queue_push(first, last, info);

        info->file_name = SCu8((u8*)dirent->d_name);
        info->attributes = linux_get_file_attributes(info->file_name);
    }

    result.infos = push_array(arena, File_Info*, num_ents);
    result.count = num_ents;
    i32 info_index = 0;
    for (File_Info* node = first; node != NULL; node = node->next) {
        result.infos[info_index] = node;
        info_index += 1;
    }
    return result;
}

internal
system_quick_file_attributes_sig(){
    return linux_get_file_attributes(file_name);
}

int main(int argc, char **argv){
}
