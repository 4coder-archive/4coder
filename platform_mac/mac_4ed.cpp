/* Mac C++ layer for 4coder */

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

#include "mac_objective_c_to_cpp_links.h"

#include <dirent.h> // NOTE(yuval): Used for opendir, readdir
#include <errno.h> // NOTE(yuval): Used for errno
#include <fcntl.h> // NOTE(yuval): Used for open
#include <unistd.h> // NOTE(yuval): Used for getcwd, read, write
#include <sys/stat.h> // NOTE(yuval): Used for stat
#include <sys/types.h> // NOTE(yuval): Used for struct stat

#include <stdlib.h> // NOTE(yuval): Used for free

////////////////////////////////

#define SLASH '\\'
#define DLL "dll"

#include "4coder_hash_functions.cpp"
#include "4coder_system_allocator.cpp"
#include "4coder_malloc_allocator.cpp"
#include "4coder_codepoint_map.cpp"

#include "4ed_mem.cpp"
#include "4ed_font_set.cpp"

////////////////////////////////

struct Mac_Vars {
    Thread_Context *tctx;
    
    Arena* frame_arena;
    
    String_Const_u8 binary_path;
};

////////////////////////////////

Mac_Vars mac_vars;
global Render_Target target;

////////////////////////////////

#include "mac_4ed_functions.cpp"

////////////////////////////////

external String_Const_u8
mac_SCu8(u8* str, u64 size){
    String_Const_u8 result = SCu8(str, size);
    return(result);
}

external String_Const_u8
mac_push_string_copy(Arena *arena, String_Const_u8 src){
    String_Const_u8 result = push_string_copy(arena, src);
    return(result);
}

external void
mac_init(){
    Arena test_arena = make_arena_malloc();
    File_List list = system_get_file_list(&test_arena,
                                          string_u8_litexpr("/Users/yuvaldolev/Desktop"));
    
    for (u32 index = 0; index < list.count; ++index) {
        File_Info* info = list.infos[index];
        
        printf("File_Info{file_name:'%.*s', "
               "attributes:{size:%llu, last_write_time:%llu, flags:{IsDirectory:%d}}}\n",
               (i32)info->file_name.size, info->file_name.str,
               info->attributes.size, info->attributes.last_write_time,
               ((info->attributes.flags & FileAttribute_IsDirectory) != 0));
    }
    
#if 0
    // NOTE(yuval): Context Setup
    Thread_Context _tctx = {};
    thread_ctx_init(&_tctx, ThreadKind_Main,
                    get_base_allocator_system(),
                    get_base_allocator_system());
    
    block_zero_struct(&mac_vars);
    mac_vars.tctx = &_tctx;
    
    API_VTable_system system_vtable = {};
    system_api_fill_vtable(&system_vtable);
    
    API_VTable_graphics graphics_vtable = {};
    graphics_api_fill_vtable(&graphics_vtable);
    
    API_VTable_font font_vtable = {};
    font_api_fill_vtable(&font_vtable);
    
    // NOTE(yuval): Memory
    mac_vars.frame_arena = reserve_arena(mac_vars.tctx);
    target.arena = make_arena_system(KB(256));
    
    mac_vars.cursor_show = MouseCursorShow_Always;
    mac_vars.prev_cursor_show = MouseCursorShow_Always;
    
    dll_init_sentinel(&mac_vars.free_mac_objects);
    dll_init_sentinel(&mac_vars.timer_objects);
#endif
}