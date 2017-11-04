/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 28.06.2017
 *
 * Mac C++ layer for 4coder
 *
 */

// TOP

#define IS_PLAT_LAYER

#include <stdio.h>
#define DBG_POINT() fprintf(stdout, "%s\n", __FILE__ ":" LINE_STR ":")

#include "4ed_defines.h"
#include "4coder_API/version.h"

#include "4coder_lib/4coder_utf8.h"

#if defined(FRED_SUPER)
# include "4coder_API/keycodes.h"
# include "4coder_API/style.h"

# define FSTRING_IMPLEMENTATION
# include "4coder_lib/4coder_string.h"
# include "4coder_lib/4coder_mem.h"

# include "4coder_API/types.h"
# include "4ed_os_custom_api.h"

#else
# include "4coder_default_bindings.cpp"
#endif

#include "4ed_math.h"

#include "4ed_system.h"
#include "4ed_log.h"
#include "4ed_rendering.h"
#include "4ed.h"

#include "4ed_file_track.h"
#include "4ed_font_interface_to_os.h"
#include "4ed_system_shared.h"

#include "unix_4ed_headers.h"
#include <sys/syslimits.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#include <mach-o/dyld.h>

#include <stdlib.h>

////////////////////////////////

#include "4ed_shared_thread_constants.h"
#include "unix_threading_wrapper.h"

// TODO(allen): Make an intrinsics header that uses the cracked OS to define a single set of intrinsic names.
#define InterlockedCompareExchange(dest, ex, comp) \
__sync_val_compare_and_swap((dest), (comp), (ex))

////////////////////////////////

#define SLASH '/'
#define DLL "so"

global System_Functions sysfunc;
#include "4ed_shared_library_constants.h"
#include "unix_library_wrapper.h"
#include "4ed_standard_libraries.cpp"

#include "4ed_coroutine.cpp"

////////////////////////////////

#include "osx_objective_c_to_cpp_links.h"
OSX_Vars osx;
global Render_Target target;
global Application_Memory memory_vars;
global Plat_Settings plat_settings;

global Libraries libraries;
global App_Functions app;
global Custom_API custom_api;

global Coroutine_System_Auto_Alloc coroutines;

////////////////////////////////

#include "mac_error_box.cpp"

////////////////////////////////

internal
Sys_Get_4ed_Path_Sig(system_get_4ed_path){
    u32 buf_size = capacity;
    i32 status = _NSGetExecutablePath(out, &buf_size);
    i32 size = 0;
    if (status == 0){
        String str = make_string_slowly(out);
        remove_last_folder(&str);
        terminate_with_null(&str);
        size = str.size;
    }
    return(size);
}

#include "unix_4ed_functions.cpp"
#include "4ed_shared_file_handling.cpp"

////////////////////////////////

internal void
system_schedule_step(){
    // NOTE(allen): It is unclear to me right now what we might need to actually do here.
    // The run loop in a Cocoa app will keep rendering the app anyway, I might just need to set a
    // "do_new_frame" variable of some kind to true here.
}

////////////////////////////////

#include "4ed_work_queues.cpp"

////////////////////////////////

internal
Sys_Show_Mouse_Cursor_Sig(system_show_mouse_cursor){
    // TODO(allen)
}

internal
Sys_Set_Fullscreen_Sig(system_set_fullscreen){
    osx.do_toggle = (osx.full_screen != full_screen);
    return(true);
}

internal
Sys_Is_Fullscreen_Sig(system_is_fullscreen){
    b32 result = (osx.full_screen != osx.do_toggle);
    return(result);
}

// HACK(allen): Why does this work differently from the win32 version!?
internal
Sys_Send_Exit_Signal_Sig(system_send_exit_signal){
    osx.running = false;
}

#include "4ed_coroutine_functions.cpp"

//
// Clipboard
//

internal
Sys_Post_Clipboard_Sig(system_post_clipboard){
    char *string = str.str;
    if (!terminate_with_null(&str)){
        if (osx.clipboard_space_max <= str.size + 1){
            if (osx.clipboard_space != 0){
                system_memory_free(osx.clipboard_space, osx.clipboard_space_max);
            }
            osx.clipboard_space_max = l_round_up_u32(str.size*2 + 1, KB(4096));
            osx.clipboard_space = (char*)system_memory_allocate(osx.clipboard_space_max);
        }
        memcpy(osx.clipboard_space, str.str, str.size);
        osx.clipboard_space[str.size] = 0;
        string = osx.clipboard_space;
    }
    osx_post_to_clipboard(string);
}

//
// CLI
//

internal
Sys_CLI_Call_Sig(system_cli_call){
    // b32 #(char *path, char *script_name, CLI_Handles *cli_out)
    NotImplemented;
    return(true);
}

internal
Sys_CLI_Begin_Update_Sig(system_cli_begin_update){
    // void #(CLI_Handles *cli)
    NotImplemented;
}

internal
Sys_CLI_Update_Step_Sig(system_cli_update_step){
    // b32 #(CLI_Handles *cli, char *dest, u32 max, u32 *amount)
    NotImplemented;
    return(0);
}

internal
Sys_CLI_End_Update_Sig(system_cli_end_update){
    // b32 #(CLI_Handles *cli)
    NotImplemented;
    return(false);
}

#include "4ed_font_data.h"
#include "4ed_system_shared.cpp"

////////////////////////////////

#include "4ed_link_system_functions.cpp"
#include "4ed_shared_init_logic.cpp"

external void*
osx_allocate(umem size){
    DBG_POINT();
    void *result = system_memory_allocate(size);
    return(result);
}

external void
osx_resize(int width, int height){
    DBG_POINT();
    osx.width = width;
    osx.height = height;
    // TODO
}

external void
osx_character_input(u32 code, OSX_Keyboard_Modifiers modifier_flags){
    DBG_POINT();
    // TODO
}

external void
osx_mouse(i32 mx, i32 my, u32 type){
    DBG_POINT();
    // TODO
}

external void
osx_mouse_wheel(float dx, float dy){
    DBG_POINT();
    // TODO
}

external void
osx_step(){
    DBG_POINT();
    // TODO
}

external void
osx_init(){
    //
    // System Linkage
    //
    
    DBG_POINT();
    link_system_code();
    
    //
    // Memory init
    //
    
    DBG_POINT();
    memset(&target, 0, sizeof(target));
    memset(&memory_vars, 0, sizeof(memory_vars));
    memset(&plat_settings, 0, sizeof(plat_settings));
    
    memset(&libraries, 0, sizeof(libraries));
    memset(&app, 0, sizeof(app));
    memset(&custom_api, 0, sizeof(custom_api));
    
    memory_init();
    
    //
    // HACK(allen): 
    // Previously zipped stuff is here, it should be zipped in the new pattern now.
    //
    
    DBG_POINT();
    init_shared_vars();
    
    //
    // Dynamic Linkage
    //
    
    DBG_POINT();
    load_app_code();
    link_rendering();
#if defined(FRED_SUPER)
    load_custom_code();
#else
    custom_api.get_bindings = get_bindings;
#endif
    
    //
    // Read command line
    //
    
    DBG_POINT();
    read_command_line(osx.argc, osx.argv);
    
    //
    // Threads
    //
    
    DBG_POINT();
    work_system_init();
    
    //
    // Coroutines
    //
    
    DBG_POINT();
    coroutines_init();

    //
    // Font System Init
    //
    
    DBG_POINT();
    system_font_init(&sysfunc.font, 0, 0, plat_settings.font_size, plat_settings.use_hinting);
    
    //
    // App Init
    //
    
    DBG_POINT();
    char cwd[4096];
    u32 size = sysfunc.get_current_path(cwd, sizeof(cwd));
    fprintf(stdout, "cwd = \"%.*s\"\n", size, cwd);
    if (size == 0 || size >= sizeof(cwd)){
        system_error_box("Could not get current directory at launch.");
    }
    String curdir = make_string(cwd, size);
    terminate_with_null(&curdir);
    replace_char(&curdir, '\\', '/');

    DBG_POINT();

    String clipboard_string = {0};
    if (osx.has_clipboard_item){
        clipboard_string = make_string(osx.clipboard_data, osx.clipboard_size);
    }

    DBG_POINT();
    fprintf(stdout, "%p\n", app.init);
    
    LOG("Initializing application variables\n");
    app.init(&sysfunc, &target, &memory_vars, clipboard_string, curdir, custom_api);

    DBG_POINT();
}

#include "4ed_shared_fonts.cpp"
#include "mac_4ed_file_track.cpp"
#include "4ed_font_static_functions.cpp"

// BOTTOM

