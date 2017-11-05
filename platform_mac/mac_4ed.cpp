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

#if 0
#define DBG_POINT() fprintf(stdout, "%s\n", __FILE__ ":" LINE_STR ":")
#else
#define DBG_POINT()
#endif

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

struct OSX_Vars{
    Application_Step_Input input;
    String clipboard_contents;
    b32 keep_running;
};

////////////////////////////////

#include "osx_objective_c_to_cpp_links.h"
OSX_Objective_C_Vars osx_objc;
OSX_Vars osxvars;
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
    osx_objc.do_toggle = (osx_objc.full_screen != full_screen);
    return(true);
}

internal
Sys_Is_Fullscreen_Sig(system_is_fullscreen){
    b32 result = (osx_objc.full_screen != osx_objc.do_toggle);
    return(result);
}

// HACK(allen): Why does this work differently from the win32 version!?
internal
Sys_Send_Exit_Signal_Sig(system_send_exit_signal){
    osx_objc.running = false;
}

#include "4ed_coroutine_functions.cpp"

//
// Clipboard
//

internal
Sys_Post_Clipboard_Sig(system_post_clipboard){
    char *string = str.str;
    if (!terminate_with_null(&str)){
        if (osx_objc.clipboard_space_max <= str.size + 1){
            if (osx_objc.clipboard_space != 0){
                system_memory_free(osx_objc.clipboard_space, osx_objc.clipboard_space_max);
            }
            osx_objc.clipboard_space_max = l_round_up_u32(str.size*2 + 1, KB(4096));
            osx_objc.clipboard_space = (char*)system_memory_allocate(osx_objc.clipboard_space_max);
        }
        memcpy(osx_objc.clipboard_space, str.str, str.size);
        osx_objc.clipboard_space[str.size] = 0;
        string = osx_objc.clipboard_space;
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
    osx_objc.width = width;
    osx_objc.height = height;

    if (width > 0 && height > 0){
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        glScissor(0, 0, width, height);
        
        target.width = width;
        target.height = height;
    }
}

internal void
osx_push_key(Key_Code code, Key_Code chr, Key_Code chr_nocaps, b8 *mods)
{
    i32 count = osxvars.input.keys.count;
    
    if (count < KEY_INPUT_BUFFER_SIZE){
        Key_Event_Data *data = osxvars.input.keys.keys;

        data[count].keycode = code;
        data[count].character = chr;
        data[count].character_no_caps_lock = chr_nocaps;
        
        memcpy(data[count].modifiers, mods, sizeof(*mods)*MDFR_INDEX_COUNT);
        
        osxvars.input.keys.count = count + 1;
    }
}

external void
osx_character_input(u32 code, OSX_Keyboard_Modifiers modifier_flags){
    DBG_POINT();
    Key_Code c = 0;
    switch (code){
        // TODO(allen): Find the canonical list of these things.
        case 0x007F: c = key_back; break;
        case 0xF700: c = key_up; break;
        case 0xF701: c = key_down; break;
        case 0xF702: c = key_left; break;
        case 0xF703: c = key_right; break;
        case 0x001B: c = key_esc; break;

        case 0xF704: c = key_f1; break;
        case 0xF705: c = key_f2; break;
        case 0xF706: c = key_f3; break;
        case 0xF707: c = key_f4; break;

        case 0xF708: c = key_f5; break;
        case 0xF709: c = key_f6; break;
        case 0xF70A: c = key_f7; break;
        case 0xF70B: c = key_f8; break;

        case 0xF70C: c = key_f9; break;
        case 0xF70D: c = key_f10; break;
        case 0xF70E: c = key_f11; break;
        case 0xF70F: c = key_f12; break;

        case 0xF710: c = key_f13; break;
        case 0xF711: c = key_f14; break;
        case 0xF712: c = key_f15; break;
        case 0xF713: c = key_f16; break;
    }

    b8 mods[MDFR_INDEX_COUNT] = {0};
    
    if (modifier_flags.shift)   mods[MDFR_SHIFT_INDEX] = 1;
    if (modifier_flags.command) mods[MDFR_CONTROL_INDEX] = 1;
    if (modifier_flags.caps)    mods[MDFR_CAPS_INDEX] = 1;
    if (modifier_flags.control) mods[MDFR_ALT_INDEX] = 1;
    
    if (c != 0){
        osx_push_key(c, 0, 0, mods);
    }
    else if (code != 0){
        if (code == '\r'){
            code = '\n';
        }
        Key_Code nocaps = code;
        if (modifier_flags.caps){
            if ('a' <= nocaps && nocaps <= 'z'){
                nocaps += 'A' - 'a';
            }
            else if ('A' <= nocaps && nocaps <= 'Z'){
                nocaps += 'a' - 'A';
            }
        }
        osx_push_key(code, code, nocaps, mods);
    }
    else{
        osx_push_key(0, 0, 0, mods);
    }
}

external void
osx_mouse(i32 mx, i32 my, u32 type){
    DBG_POINT();
    osxvars.input.mouse.x = mx;
    osxvars.input.mouse.y = my;
    if (type == MouseType_Press){
        osxvars.input.mouse.press_l = true;
        osxvars.input.mouse.l = true;
    }
    if (type == MouseType_Release){
        osxvars.input.mouse.l = false;
    }
}

external void
osx_mouse_wheel(float dx, float dy){
    DBG_POINT();
    if (dy > 0){
        osxvars.input.mouse.wheel = 1;
    }
    else if (dy < 0){
        osxvars.input.mouse.wheel = -1;
    }
}

external void
osx_step(){
    Application_Step_Result result = {};
    result.mouse_cursor_type = APP_MOUSE_CURSOR_DEFAULT;
    result.trying_to_kill = !osxvars.keep_running;
    
    osxvars.input.clipboard = null_string;

    app.step(&sysfunc, &target, &memory_vars, &osxvars.input, &result);
    launch_rendering(&sysfunc, &target);

    osxvars.input.first_step = false;
    osxvars.input.keys = null_key_input_data;
    osxvars.input.mouse.press_l = false;
    osxvars.input.mouse.release_l = false;
    osxvars.input.mouse.press_r = false;
    osxvars.input.mouse.release_r = false;
    osxvars.input.mouse.wheel = 0;
}

external void
osx_init(){
    // TODO(allen): Setup GL DEBUG MESSAGE
#if defined(FRED_INTERNAL) && 0
    //
    // OpenGL Init
    //

    typedef PFNGLDEBUGMESSAGECALLBACKARBPROC glDebugMessageCallbackProc;
    
    GLXLOAD(glDebugMessageCallback);
    
    if (glDebugMessageCallback){
        LOG("Enabling GL Debug Callback\n");
        glDebugMessageCallback(&LinuxGLDebugCallback, 0);
        glEnable(GL_DEBUG_OUTPUT);
    }
#endif
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    osxvars.keep_running = true;
    osxvars.input.first_step = true;
    
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
    read_command_line(osx_objc.argc, osx_objc.argv);
    
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
    if (osx_objc.has_clipboard_item){
        clipboard_string = make_string(osx_objc.clipboard_data, osx_objc.clipboard_size);
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

