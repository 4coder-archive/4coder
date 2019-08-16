/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Win32 layer for 4coder
 *
 */

// TOP

#define IS_PLAT_LAYER

//
// Program setup
//

#define UNICODE

#define FPS 60
#define frame_useconds (1000000 / FPS)

#include "4coder_base_types.h"
#include "4coder_table.h"
#include "4coder_API/4coder_version.h"

#include <string.h>
#include "4coder_lib/4coder_utf8.h"

#if defined(FRED_SUPER)
# include "4coder_lib/4coder_heap.h"
# include "4coder_lib/4coder_heap.cpp"

# include "4coder_base_types.cpp"
# include "4coder_stringf.cpp"
# include "4coder_hash_functions.cpp"
# include "4coder_table.cpp"
# include "4coder_log.cpp"

# include "4coder_API/4coder_keycodes.h"
# include "4coder_API/4coder_default_colors.h"
# include "4coder_API/4coder_types.h"
#else
# include "4coder_default_bindings.cpp"
#endif

#include "4ed_font_interface.h"
#include "4ed_font_set.h"
#include "4ed_system.h"
#include "4ed_render_target.h"
#include "4ed.h"

#include <Windows.h>
#include "win32_gl.h"

#define GL_TEXTURE_MAX_LEVEL 0x813D

//////////////////////////////

enum{
    ErrorString_UseLog = 0,
    ErrorString_UseErrorBox = 1,
};

internal void
win32_output_error_string(Arena *scratch, i32 error_string_type);

//////////////////////////////

#include "win32_utf8.h"

#include "4ed_system_shared.h"

#define WM_4coder_ANIMATE (WM_USER + 0)

struct Control_Keys{
    b8 l_ctrl;
    b8 r_ctrl;
    b8 l_alt;
    b8 r_alt;
};
global Control_Keys null_control_keys = {};

struct Win32_Input_Chunk_Transient{
    Key_Input_Data key_data;
    b8 mouse_l_press;
    b8 mouse_l_release;
    b8 mouse_r_press;
    b8 mouse_r_release;
    b8 out_of_window;
    i8 mouse_wheel;
    b8 trying_to_kill;
};
global Win32_Input_Chunk_Transient null_input_chunk_transient = {};

struct Win32_Input_Chunk_Persistent{
    Vec2_i32 mouse;
    Control_Keys controls;
    b8 mouse_l;
    b8 mouse_r;
    b8 control_keys[MDFR_INDEX_COUNT];
};

struct Win32_Input_Chunk{
    Win32_Input_Chunk_Transient trans;
    Win32_Input_Chunk_Persistent pers;
};

////////////////////////////////

#define SLASH '\\'
#define DLL "dll"

global System_Functions sysfunc;
#include "4ed_shared_library_constants.h"
#include "win32_library_wrapper.h"

#include "4ed_standard_libraries.cpp"
#include "4ed_font_face.cpp"

#include "4ed_mem.cpp"
#include "4coder_hash_functions.cpp"

#include "4ed_system_allocator.cpp"
#include "4ed_font_set.cpp"

////////////////////////////////

typedef i32 Win32_Object_Kind;
enum{
    Win32ObjectKind_ERROR = 0,
    Win32ObjectKind_Timer = 1,
    Win32ObjectKind_Thread = 2,
    Win32ObjectKind_Mutex = 3,
    Win32ObjectKind_CV = 4,
};

struct Win32_Object{
    Node node;
    Win32_Object_Kind kind;
    union{
        struct{
            UINT_PTR id;
        } timer;
        struct{
            HANDLE thread;
            Thread_Function *proc;
            void *ptr;
        } thread;
        CRITICAL_SECTION mutex;
        CONDITION_VARIABLE cv;
    };
};

struct Win32_Vars{
    Arena arena;
    
    Win32_Input_Chunk input_chunk;
    b8 lctrl_lalt_is_altgr;
    b8 got_useful_event;
    
    b8 full_screen;
    b8 do_toggle;
    WINDOWPLACEMENT bordered_win_pos;
    b32 send_exit_signal;
    
    HCURSOR cursor_ibeam;
    HCURSOR cursor_arrow;
    HCURSOR cursor_leftright;
    HCURSOR cursor_updown;
    i32 cursor_show;
    i32 prev_cursor_show;
    
    String_Const_u8 binary_path;
    
    u8 *clip_buffer;
    u32 clip_max;
    String_Const_u8 clipboard_contents;
    b32 next_clipboard_is_self;
    DWORD clipboard_sequence;
    
    Arena clip_post_arena;
    String_Const_u8 clip_post;
    
    HWND window_handle;
    i32 dpi_x;
    i32 dpi_y;
    
    f64 count_per_usecond;
    b32 first;
    i32 running_cli;
    
    Node free_win32_objects;
    Node timer_objects;
    UINT_PTR timer_counter;
    
    CRITICAL_SECTION thread_launch_mutex;
    CONDITION_VARIABLE thread_launch_cv;
    b32 waiting_for_launch;
    
    Log_Function *log_string;
};

////////////////////////////////

global Win32_Vars win32vars;
global Render_Target target;
global Application_Memory memory_vars;
global Plat_Settings plat_settings;

global Libraries libraries;
global App_Functions app;
global Custom_API custom_api;

////////////////////////////////

internal void
system_error_box(Arena *scratch, char *msg, b32 shutdown = true){
    //LOGF("error box: %s\n", msg);
    MessageBox_utf8(scratch, 0, (u8*)msg, (u8*)"Error", 0);
    if (shutdown){
        exit(1);
    }
}

////////////////////////////////

internal void
win32_output_error_string(Arena *scratch, b32 use_error_box){
    DWORD error = GetLastError();
    
    char *str = 0;
    char *str_ptr = (char*)&str;
    if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, str_ptr, 0, 0)){
        if (use_error_box){
            system_error_box(scratch, str, false);
        }
    }
}

////////////////////////////////

internal HANDLE
handle_type(Plat_Handle h){
    HANDLE result;
    block_copy(&result, &h, sizeof(result));
    return(result);
}

internal Plat_Handle
handle_type(HANDLE h){
    Plat_Handle result = {};
    block_copy(&result, &h, sizeof(h));
    return(result);
}

internal void*
handle_type_ptr(Plat_Handle h){
    void *result;
    block_copy(&result, &h, sizeof(result));
    return(result);
}

internal Plat_Handle
handle_type_ptr(void *ptr){
    Plat_Handle result = {};
    block_copy(&result, &ptr, sizeof(ptr));
    return(result);
}

////////////////////////////////

#include "win32_4ed_functions.cpp"

////////////////////////////////

internal void
system_schedule_step(u32 code){
    PostMessage(win32vars.window_handle, WM_4coder_ANIMATE, code, 0);
}

////////////////////////////////

internal void
win32_toggle_fullscreen(){
    HWND win = win32vars.window_handle;
    DWORD style = GetWindowLongW(win, GWL_STYLE);
    b32 is_full = ((style & WS_OVERLAPPEDWINDOW) == 0);
    if (!is_full){
        MONITORINFO info = {sizeof(MONITORINFO)};
        if (GetWindowPlacement(win, &win32vars.bordered_win_pos) && GetMonitorInfo(MonitorFromWindow(win, MONITOR_DEFAULTTOPRIMARY), &info)){
            SetWindowLongW(win, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            
            i32 x = info.rcMonitor.left;
            i32 y = info.rcMonitor.top;
            i32 w = info.rcMonitor.right - info.rcMonitor.left;
            i32 h = info.rcMonitor.bottom - info.rcMonitor.top;
            
            SetWindowPos(win, HWND_TOP, x, y, w, h, SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
            win32vars.full_screen = true;
        }
    }
    else{
        SetWindowLongW(win, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(win, &win32vars.bordered_win_pos);
        SetWindowPos(win, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        win32vars.full_screen = false;
    }
}

// TODO(allen): add a "shown but auto-hides on timer" setting here.
internal
Sys_Show_Mouse_Cursor_Sig(system_show_mouse_cursor){
    win32vars.cursor_show = show;
}

internal
Sys_Set_Fullscreen_Sig(system_set_fullscreen){
    // NOTE(allen): If the new value of full_screen does not match the current value,
    // set toggle to true.
    win32vars.do_toggle = (win32vars.full_screen != full_screen);
    b32 success = true;
    return(success);
}

internal
Sys_Is_Fullscreen_Sig(system_is_fullscreen){
    // NOTE(allen): Report the fullscreen status as it would be set at the beginning of the next frame.
    // That is, take into account all fullscreen toggle requests that have come in already this frame.
    // Read: "full_screen XOR do_toggle"
    b32 result = (win32vars.full_screen != win32vars.do_toggle);
    return(result);
}

#include "4ed_system_shared.cpp"

//
// Clipboard
//

internal void
win32_post_clipboard(Arena *scratch, char *text, i32 len){
    if (OpenClipboard(win32vars.window_handle)){
        if (!EmptyClipboard()){
            win32_output_error_string(scratch, ErrorString_UseLog);
        }
        HANDLE memory_handle = GlobalAlloc(GMEM_MOVEABLE, len  + 1);
        if (memory_handle){
            char *dest = (char*)GlobalLock(memory_handle);
            memmove(dest, text, len);
            dest[len] = 0;
            GlobalUnlock(memory_handle);
            SetClipboardData(CF_TEXT, memory_handle);
            win32vars.next_clipboard_is_self = true;
        }
        CloseClipboard();
    }
}

internal
Sys_Post_Clipboard_Sig(system_post_clipboard){
    Arena *arena = &win32vars.clip_post_arena;
    if (arena->base_allocator == 0){
        *arena = make_arena_system(&sysfunc);
    }
    else{
        linalloc_clear(arena);
    }
    win32vars.clip_post.str = push_array(arena, u8, str.size + 1);
    if (win32vars.clip_post.str != 0){
        block_copy(win32vars.clip_post.str, str.str, str.size);
        win32vars.clip_post.str[str.size] = 0;
        win32vars.clip_post.size = str.size;
    }
    else{
        //LOGF("Failed to allocate buffer for clipboard post (%d)\n", (i32)str.size + 1);
    }
}

internal b32
win32_read_clipboard_contents(Arena *scratch){
    Temp_Memory temp = begin_temp(scratch);
    
    b32 result = false;
    
    b32 has_text = false;
    b32 has_unicode = IsClipboardFormatAvailable(CF_UNICODETEXT);
    if (!has_unicode){
        has_text = IsClipboardFormatAvailable(CF_TEXT);
    }
    b32 can_read = has_unicode || has_text;
    
    if (can_read){
        if (OpenClipboard(win32vars.window_handle)){
            result = true;
            HANDLE clip_data = 0;
            i32 contents_length = 0;
            if (has_unicode){
                clip_data = GetClipboardData(CF_UNICODETEXT);
                if (clip_data != 0){
                    u16 *clip_16 = (u16*)GlobalLock(clip_data);
                    if (clip_16 != 0){
                        u32 clip_16_len = 0;
                        for(;clip_16[clip_16_len];++clip_16_len);
                        
                        b32 error = false;
                        u32 clip_8_len = (u32)utf16_to_utf8_minimal_checking(win32vars.clip_buffer, win32vars.clip_max-1, clip_16, clip_16_len, &error);
                        
                        for (;clip_8_len >= win32vars.clip_max && !error;){
                            system_memory_free(win32vars.clip_buffer, win32vars.clip_max);
                            win32vars.clip_max = round_up_u32(clip_8_len + 1, KB(4));
                            win32vars.clip_buffer = (u8*)system_memory_allocate(win32vars.clip_max);
                            clip_8_len = (u32)utf16_to_utf8_minimal_checking(win32vars.clip_buffer, win32vars.clip_max - 1, clip_16, clip_16_len, &error);
                        }
                        
                        if (clip_8_len < win32vars.clip_max && !error){
                            win32vars.clip_buffer[clip_8_len] = 0;
                            contents_length = clip_8_len + 1;
                        }
                    }
                }
            }
            else{
                clip_data = GetClipboardData(CF_TEXT);
                if (clip_data != 0){
                    char *clip_ascii = (char*)GlobalLock(clip_data);
                    if (clip_ascii != 0){
                        u32 clip_ascii_len = 0;
                        for(;clip_ascii[clip_ascii_len];++clip_ascii_len);
                        
                        if (clip_ascii_len >= win32vars.clip_max){
                            system_memory_free(win32vars.clip_buffer, win32vars.clip_max);
                            win32vars.clip_max = round_up_u32(clip_ascii_len + 1, KB(4));
                            win32vars.clip_buffer = (u8*)system_memory_allocate(win32vars.clip_max);
                        }
                        memcpy(win32vars.clip_buffer, clip_ascii, clip_ascii_len + 1);
                        contents_length = clip_ascii_len + 1;
                    }
                }
            }
            
            if (contents_length > 0){
                win32vars.clipboard_contents = SCu8(win32vars.clip_buffer, contents_length - 1);
            }
            
            GlobalUnlock(clip_data);
            
            CloseClipboard();
        }
    }
    
    end_temp(temp);
    
    return(result);
}


//
// Command Line Exectuion
//

internal
Sys_CLI_Call_Sig(system_cli_call, scratch, path, script_name, cli_out){
    Assert(sizeof(Plat_Handle) >= sizeof(HANDLE));
    
    char cmd[] = "c:\\windows\\system32\\cmd.exe";
    char *env_variables = 0;
    
    Temp_Memory temp = begin_temp(scratch);
    String_Const_u8 s = push_u8_stringf(scratch, "/C %s", script_name);
    
    b32 success = false;
    
    *(HANDLE*)&cli_out->proc = INVALID_HANDLE_VALUE;
    *(HANDLE*)&cli_out->out_read = INVALID_HANDLE_VALUE;
    *(HANDLE*)&cli_out->out_write = INVALID_HANDLE_VALUE;
    *(HANDLE*)&cli_out->in_read = INVALID_HANDLE_VALUE;
    *(HANDLE*)&cli_out->in_write = INVALID_HANDLE_VALUE;
    
    SECURITY_ATTRIBUTES security_atrb = {};
    security_atrb.nLength = sizeof(SECURITY_ATTRIBUTES);
    security_atrb.bInheritHandle = TRUE;
    
    HANDLE out_read = INVALID_HANDLE_VALUE;
    HANDLE out_write = INVALID_HANDLE_VALUE;
    if (CreatePipe(&out_read, &out_write, &security_atrb, 0)){
        if (SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0)){
            STARTUPINFO startup = {};
            startup.cb = sizeof(STARTUPINFO);
            startup.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
            
            HANDLE in_read = CreateFileA("nul", GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, &security_atrb, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            
            startup.hStdInput = in_read;
            startup.hStdOutput = out_write;
            startup.hStdError = out_write;
            startup.wShowWindow = SW_HIDE;
            
            PROCESS_INFORMATION info = {};
            if (CreateProcess_utf8(scratch, (u8*)cmd, s.str, 0, 0, TRUE, 0, env_variables, (u8*)path, &startup, &info)){
                success = true;
                CloseHandle(info.hThread);
                *(HANDLE*)&cli_out->proc = info.hProcess;
                *(HANDLE*)&cli_out->out_read = out_read;
                *(HANDLE*)&cli_out->out_write = out_write;
                
                ++win32vars.running_cli;
            }
            else{
                CloseHandle(out_read);
                CloseHandle(out_write);
                CloseHandle(in_read);
            }
        }
        else{
            // TODO(allen): failed SetHandleInformation
            CloseHandle(out_read);
            CloseHandle(out_write);
        }
    }
    else{
        // TODO(allen): failed CreatePipe
    }
    
    end_temp(temp);
    
    return(success);
}

struct CLI_Loop_Control{
    u32 remaining_amount;
};

internal
Sys_CLI_Begin_Update_Sig(system_cli_begin_update){
    Assert(sizeof(cli->scratch_space) >= sizeof(CLI_Loop_Control));
    CLI_Loop_Control *loop = (CLI_Loop_Control*)cli->scratch_space;
    loop->remaining_amount = 0;
}

internal
Sys_CLI_Update_Step_Sig(system_cli_update_step){
    HANDLE handle = *(HANDLE*)&cli->out_read;
    CLI_Loop_Control *loop = (CLI_Loop_Control*)cli->scratch_space;
    b32 has_more = 0;
    DWORD remaining = loop->remaining_amount;
    u32 pos = 0;
    DWORD read_amount = 0;
    
    for (;;){
        if (remaining == 0){
            if (!PeekNamedPipe(handle, 0, 0, 0, &remaining, 0)) break;
            if (remaining == 0) break;
        }
        
        if (remaining + pos < max){
            has_more = 1;
            ReadFile(handle, dest + pos, remaining, &read_amount, 0);
            Assert(remaining == read_amount);
            pos += remaining;
            remaining = 0;
        }
        else{
            has_more = 1;
            ReadFile(handle, dest + pos, max - pos, &read_amount, 0);
            Assert(max - pos == read_amount);
            loop->remaining_amount = remaining - (max - pos);
            pos = max;
            break;
        }
    }
    *amount = pos;
    
    return(has_more);
}

internal
Sys_CLI_End_Update_Sig(system_cli_end_update){
    b32 close_me = false;
    HANDLE proc = *(HANDLE*)&cli->proc;
    DWORD result = 0;
    
    if (WaitForSingleObject(proc, 0) == WAIT_OBJECT_0){
        if (GetExitCodeProcess(proc, &result) == 0){
            cli->exit = -1;
        }
        else{
            cli->exit = (i32)result;
        }
        
        close_me = true;
        CloseHandle(*(HANDLE*)&cli->proc);
        CloseHandle(*(HANDLE*)&cli->out_read);
        CloseHandle(*(HANDLE*)&cli->out_write);
        if (*(HANDLE*)&cli->in_read != INVALID_HANDLE_VALUE){
            CloseHandle(*(HANDLE*)&cli->in_read);
        }
        if (*(HANDLE*)&cli->in_write != INVALID_HANDLE_VALUE){
            CloseHandle(*(HANDLE*)&cli->in_write);
        }
        
        --win32vars.running_cli;
    }
    
    return(close_me);
}

#include "4ed_font_provider_freetype.h"
#include "4ed_font_provider_freetype.cpp"

#include <GL/gl.h>
#include "opengl/4ed_opengl_render.cpp"

//
// Helpers
//

global Key_Code keycode_lookup_table[255];

internal void
win32_keycode_init(void){
    keycode_lookup_table[VK_BACK] = key_back;
    keycode_lookup_table[VK_DELETE] = key_del;
    keycode_lookup_table[VK_UP] = key_up;
    keycode_lookup_table[VK_DOWN] = key_down;
    keycode_lookup_table[VK_LEFT] = key_left;
    keycode_lookup_table[VK_RIGHT] = key_right;
    keycode_lookup_table[VK_INSERT] = key_insert;
    keycode_lookup_table[VK_HOME] = key_home;
    keycode_lookup_table[VK_END] = key_end;
    keycode_lookup_table[VK_PRIOR] = key_page_up;
    keycode_lookup_table[VK_NEXT] = key_page_down;
    keycode_lookup_table[VK_ESCAPE] = key_esc;
    
    keycode_lookup_table[VK_CONTROL] = key_ctrl;
    keycode_lookup_table[VK_LCONTROL] = key_ctrl;
    keycode_lookup_table[VK_RCONTROL] = key_ctrl;
    
    keycode_lookup_table[VK_MENU] = key_alt;
    keycode_lookup_table[VK_LMENU] = key_alt;
    keycode_lookup_table[VK_RMENU] = key_alt;
    
    keycode_lookup_table[VK_SHIFT] = key_shift;
    keycode_lookup_table[VK_LSHIFT] = key_shift;
    keycode_lookup_table[VK_RSHIFT] = key_shift;
    
    keycode_lookup_table[VK_PAUSE] = key_pause;
    keycode_lookup_table[VK_CAPITAL] = key_caps;
    keycode_lookup_table[VK_NUMLOCK] = key_num_lock;
    keycode_lookup_table[VK_SCROLL] = key_scroll_lock;
    keycode_lookup_table[VK_APPS] = key_menu;
    
    keycode_lookup_table[VK_F1] = key_f1;
    keycode_lookup_table[VK_F2] = key_f2;
    keycode_lookup_table[VK_F3] = key_f3;
    keycode_lookup_table[VK_F4] = key_f4;
    keycode_lookup_table[VK_F5] = key_f5;
    keycode_lookup_table[VK_F6] = key_f6;
    keycode_lookup_table[VK_F7] = key_f7;
    keycode_lookup_table[VK_F8] = key_f8;
    keycode_lookup_table[VK_F9] = key_f9;
    
    keycode_lookup_table[VK_F10] = key_f10;
    keycode_lookup_table[VK_F11] = key_f11;
    keycode_lookup_table[VK_F12] = key_f12;
    keycode_lookup_table[VK_F13] = key_f13;
    keycode_lookup_table[VK_F14] = key_f14;
    keycode_lookup_table[VK_F15] = key_f15;
    keycode_lookup_table[VK_F16] = key_f16;
}

internal void
win32_resize(i32 width, i32 height){
    if (width > 0 && height > 0){
        target.width = width;
        target.height = height;
    }
}

#if 0
internal void
win32_init_gl(HDC hdc){
    //LOG("trying to load wgl extensions...\n");
    
#define GLInitFail(s) system_error_box(FNLN "\nOpenGL init fail - " s )
    
    // Init First Context
    WNDCLASSA wglclass = {};
    wglclass.lpfnWndProc = DefWindowProcA;
    wglclass.hInstance = GetModuleHandle(0);
    wglclass.lpszClassName = "4ed-wgl-loader";
    if (RegisterClassA(&wglclass) == 0){
        GLInitFail("RegisterClassA");
    }
    
    HWND hwglwnd = CreateWindowExA(0, wglclass.lpszClassName, "", 0, 0, 0, 0, 0, 0, 0, wglclass.hInstance, 0);
    if (hwglwnd == 0){
        GLInitFail("CreateWindowExA");
    }
    
    HDC hwgldc = GetDC(hwglwnd);
    
    PIXELFORMATDESCRIPTOR format = {};
    format.nSize = sizeof(format);
    format.nVersion = 1;
    //format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    format.iPixelType = PFD_TYPE_RGBA;
    format.cColorBits = 32;
    format.cAlphaBits = 8;
    format.cDepthBits = 24;
    format.iLayerType = PFD_MAIN_PLANE;
    i32 suggested_format_index = ChoosePixelFormat(hwgldc, &format);
    if (suggested_format_index == 0){
        win32_output_error_string(ErrorString_UseErrorBox);
        GLInitFail("ChoosePixelFormat");
    }
    
    DescribePixelFormat(hwgldc, suggested_format_index, sizeof(format), &format);
    if (!SetPixelFormat(hwgldc, suggested_format_index, &format)){
        win32_output_error_string(ErrorString_UseErrorBox);
        GLInitFail("SetPixelFormat");
    }
    
    HGLRC wglcontext = wglCreateContext(hwgldc);
    if (wglcontext == 0){
        win32_output_error_string(ErrorString_UseErrorBox);
        GLInitFail("wglCreateContext");
    }
    
    if (!wglMakeCurrent(hwgldc, wglcontext)){
        win32_output_error_string(ErrorString_UseErrorBox);
        GLInitFail("wglMakeCurrent");
    }
    
    // Load wgl Extensions
#define LoadWGL(f, s) f = (f##_Function*)wglGetProcAddress(#f); b32 got_##f = GLFuncGood(f); \
    if (!got_##f) { if (s) { GLInitFail(#f " missing"); } else { f = 0; } }
    LoadWGL(wglCreateContextAttribsARB, true);
    LoadWGL(wglChoosePixelFormatARB, true);
    LoadWGL(wglGetExtensionsStringEXT, true);
    
    //LOG("got wgl functions\n");
    
    char *extensions_c = wglGetExtensionsStringEXT();
    String extensions = make_string_slowly(extensions_c);
    if (has_substr(extensions, make_lit_string("WGL_EXT_swap_interval"))){
        LoadWGL(wglSwapIntervalEXT, false);
        if (wglSwapIntervalEXT != 0){
            //LOG("got wglSwapIntervalEXT\n");
        }
    }
    
    // Init the Second Context
    int pixel_attrib_list[] = {
        WGL_DRAW_TO_WINDOW_ARB, TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_SUPPORT_OPENGL_ARB, TRUE,
        //WGL_DOUBLE_BUFFER_ARB, TRUE,
        WGL_DOUBLE_BUFFER_ARB, FALSE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        0,
    };
    
    u32 ignore = 0;
    if (!wglChoosePixelFormatARB(hdc, pixel_attrib_list, 0, 1, &suggested_format_index, &ignore)){
        GLInitFail("wglChoosePixelFormatARB");
    }
    
    DescribePixelFormat(hdc, suggested_format_index, sizeof(format), &format);
    if (!SetPixelFormat(hdc, suggested_format_index, &format)){
        GLInitFail("SetPixelFormat");
    }
    
    i32 context_attrib_list[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_FLAGS_ARB, 0,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    
    HGLRC context = wglCreateContextAttribsARB(hdc, 0, context_attrib_list);
    if (context == 0){
        GLInitFail("wglCreateContextAttribsARB");
    }
    
    wglMakeCurrent(hdc, context);
    
    if (wglSwapIntervalEXT != 0){
        //LOGF("setting swap interval %d\n", 1);
        wglSwapIntervalEXT(1);
    }
    
    ReleaseDC(hwglwnd, hwgldc);
    DestroyWindow(hwglwnd);
    wglDeleteContext(wglcontext);
    
    //LOG("successfully enabled opengl\n");
}
#endif

internal void
Win32SetCursorFromUpdate(Application_Mouse_Cursor cursor){
    switch (cursor){
        case APP_MOUSE_CURSOR_ARROW:
        {
            SetCursor(win32vars.cursor_arrow);
        }break;
        
        case APP_MOUSE_CURSOR_IBEAM:
        {
            SetCursor(win32vars.cursor_ibeam);
        }break;
        
        case APP_MOUSE_CURSOR_LEFTRIGHT:
        {
            SetCursor(win32vars.cursor_leftright);
        }break;
        
        case APP_MOUSE_CURSOR_UPDOWN:
        {
            SetCursor(win32vars.cursor_updown);
        }break;
    }
}

internal Win32_Object*
win32_alloc_object(Win32_Object_Kind kind){
    Win32_Object *result = 0;
    if (win32vars.free_win32_objects.next != &win32vars.free_win32_objects){
        result = CastFromMember(Win32_Object, node, win32vars.free_win32_objects.next);
    }
    if (result == 0){
        i32 count = 512;
        Win32_Object *objects = (Win32_Object*)system_memory_allocate(count*sizeof(Win32_Object));
        objects[0].node.prev = &win32vars.free_win32_objects;
        win32vars.free_win32_objects.next = &objects[0].node;
        for (i32 i = 1; i < count; i += 1){
            objects[i - 1].node.next = &objects[i].node;
            objects[i].node.prev = &objects[i - 1].node;
        }
        objects[count - 1].node.next = &win32vars.free_win32_objects;
        win32vars.free_win32_objects.prev = &objects[count - 1].node;
        result = CastFromMember(Win32_Object, node, win32vars.free_win32_objects.next);
    }
    Assert(result != 0);
    dll_remove(&result->node);
    block_zero_struct(result);
    result->kind = kind;
    return(result);
}

internal void
win32_free_object(Win32_Object *object){
    if (object->node.next != 0){
        dll_remove(&object->node);
    }
    dll_insert(&win32vars.free_win32_objects, &object->node);
}

////////////////////////////////

internal
Sys_Now_Time_Sig(system_now_time){
    u64 result = 0;
    LARGE_INTEGER t;
    if (QueryPerformanceCounter(&t)){
        result = (u64)(t.QuadPart/win32vars.count_per_usecond);
    }
    return(result);
}

internal
Sys_Wake_Up_Timer_Create_Sig(system_wake_up_timer_create){
    Win32_Object *object = win32_alloc_object(Win32ObjectKind_Timer);
    dll_insert(&win32vars.timer_objects, &object->node);
    object->timer.id = ++win32vars.timer_counter;
    return(handle_type(object));
}

internal
Sys_Wake_Up_Timer_Release_Sig(system_wake_up_timer_release){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(handle);
    if (object->kind == Win32ObjectKind_Timer){
        KillTimer(win32vars.window_handle, object->timer.id);
        win32_free_object(object);
    }
}

internal
Sys_Wake_Up_Timer_Set_Sig(system_wake_up_timer_set){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(handle);
    if (object->kind == Win32ObjectKind_Timer){
        object->timer.id = SetTimer(win32vars.window_handle, object->timer.id, time_milliseconds, 0);
    }
}

internal
Sys_Signal_Step_Sig(system_signal_step){
    system_schedule_step(code);
}

internal
Sys_Sleep_Sig(system_sleep){
    u32 milliseconds = (u32)(microseconds/Thousand(1));
    Sleep(milliseconds);
}

////////////////////////////////

internal DWORD
win32_thread_wrapper(void *ptr){
    Win32_Object *object = (Win32_Object*)ptr;
    Thread_Function *proc = object->thread.proc;
    void *object_ptr = object->thread.ptr;
    win32vars.waiting_for_launch = false;
    WakeConditionVariable(&win32vars.thread_launch_cv);
    proc(object_ptr);
    return(0);
}

internal
Sys_Thread_Launch_Sig(system_thread_launch){
    Win32_Object *object = win32_alloc_object(Win32ObjectKind_Thread);
    object->thread.proc = proc;
    object->thread.ptr = ptr;
    EnterCriticalSection(&win32vars.thread_launch_mutex);
    win32vars.waiting_for_launch = true;
    object->thread.thread = CreateThread(0, 0, win32_thread_wrapper, object, 0, 0);
    for (;win32vars.waiting_for_launch;){
        SleepConditionVariableCS(&win32vars.thread_launch_cv, &win32vars.thread_launch_mutex, INFINITE);
    }
    LeaveCriticalSection(&win32vars.thread_launch_mutex);
    return(handle_type(object));
}

internal
Sys_Thread_Join_Sig(system_thread_join){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(thread);
    if (object->kind == Win32ObjectKind_Thread){
        WaitForSingleObject(object->thread.thread, INFINITE);
    }
}

internal
Sys_Thread_Free_Sig(system_thread_free){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(thread);
    if (object->kind == Win32ObjectKind_Thread){
        CloseHandle(object->thread.thread);
        win32_free_object(object);
    }
}

internal
Sys_Thread_Get_ID_Sig(system_thread_get_id){
    DWORD result = GetCurrentThreadId();
    return((i32)result);
}

internal
Sys_Mutex_Make_Sig(system_mutex_make){
    Win32_Object *object = win32_alloc_object(Win32ObjectKind_Mutex);
    InitializeCriticalSection(&object->mutex);
    return(handle_type(object));
}

internal
Sys_Mutex_Acquire_Sig(system_mutex_acquire){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(mutex);
    if (object->kind == Win32ObjectKind_Mutex){
        EnterCriticalSection(&object->mutex);
    }
}

internal
Sys_Mutex_Release_Sig(system_mutex_release){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(mutex);
    if (object->kind == Win32ObjectKind_Mutex){
        LeaveCriticalSection(&object->mutex);
    }
}

internal
Sys_Mutex_Free_Sig(system_mutex_free){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(mutex);
    if (object->kind == Win32ObjectKind_Mutex){
        DeleteCriticalSection(&object->mutex);
        win32_free_object(object);
    }
}

internal
Sys_Condition_Variable_Make_Sig(system_condition_variable_make){
    Win32_Object *object = win32_alloc_object(Win32ObjectKind_CV);
    InitializeConditionVariable(&object->cv);
    return(handle_type(object));
}

internal
Sys_Condition_Variable_Wait_Sig(system_condition_variable_wait){
    Win32_Object *object_cv = (Win32_Object*)handle_type_ptr(cv);
    Win32_Object *object_mutex = (Win32_Object*)handle_type_ptr(mutex);
    if (object_cv->kind == Win32ObjectKind_CV &&
        object_mutex->kind == Win32ObjectKind_Mutex){
        SleepConditionVariableCS(&object_cv->cv, &object_mutex->mutex, INFINITE);
    }
}

internal
Sys_Condition_Variable_Signal_Sig(system_condition_variable_signal){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(cv);
    if (object->kind == Win32ObjectKind_CV){
        WakeConditionVariable(&object->cv);
    }
}

internal
Sys_Condition_Variable_Free_Sig(system_condition_variable_free){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(cv);
    if (object->kind == Win32ObjectKind_CV){
        win32_free_object(object);
    }
}

////////////////////////////////

internal LRESULT
win32_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;
    Arena *scratch = &win32vars.arena;
    
    switch (uMsg){
        case WM_MENUCHAR:break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            b8 release = HasFlag(lParam, bit_32);
            b8 down = !release;
            b8 is_right = HasFlag(lParam, bit_25);
            
            switch (wParam){
                case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
                case VK_MENU:case VK_LMENU:case VK_RMENU:
                case VK_SHIFT:case VK_LSHIFT:case VK_RSHIFT:
                {
                    Control_Keys *controls = &win32vars.input_chunk.pers.controls;
                    b8 *control_keys = win32vars.input_chunk.pers.control_keys;
                    
                    if (wParam != 255){
                        switch (wParam){
                            case VK_SHIFT:
                            {
                                control_keys[MDFR_SHIFT_INDEX] = down;
                            }break;
                            case VK_CONTROL:
                            {
                                if (is_right){
                                    controls->r_ctrl = down;
                                }
                                else{
                                    controls->l_ctrl = down;
                                }
                            }break;
                            case VK_MENU:
                            {
                                if (is_right){
                                    controls->r_alt = down;
                                }
                                else{
                                    controls->l_alt = down;
                                }
                            }break;
                        }
                        
                        b8 ctrl = (controls->r_ctrl || (controls->l_ctrl && !controls->r_alt));
                        b8 alt = (controls->l_alt || (controls->r_alt && !controls->l_ctrl));
                        if (win32vars.lctrl_lalt_is_altgr && controls->l_alt && controls->l_ctrl){
                            ctrl = false;
                            alt = false;
                        }
                        control_keys[MDFR_CONTROL_INDEX] = ctrl;
                        control_keys[MDFR_ALT_INDEX] = alt;
                    }
                }break;
            }
            
            if (down){
                Key_Code key = keycode_lookup_table[(u8)wParam];
                if (key != 0){
                    i32 *count = &win32vars.input_chunk.trans.key_data.count;
                    Key_Event_Data *data = win32vars.input_chunk.trans.key_data.keys;
                    b8 *control_keys = win32vars.input_chunk.pers.control_keys;
                    i32 control_keys_size = sizeof(win32vars.input_chunk.pers.control_keys);
                    
                    if (*count < KEY_INPUT_BUFFER_SIZE){
                        data[*count].character = 0;
                        data[*count].character_no_caps_lock = 0;
                        data[*count].keycode = key;
                        memcpy(data[*count].modifiers, control_keys, control_keys_size);
                        ++(*count);
                    }
                    
                    win32vars.got_useful_event = true;
                }
            }
        }break;
        
        case WM_CHAR: case WM_SYSCHAR: case WM_UNICHAR:
        {
            u16 character = (u16)wParam;
            
            if (character == '\r'){
                character = '\n';
            }
            else if (character == '\t'){
                // Do nothing
            }
            else if (character < 32 || character == 127){
                break;
            }
            
            u16 character_no_caps_lock = character;
            
            i32 *count = &win32vars.input_chunk.trans.key_data.count;
            Key_Event_Data *data = win32vars.input_chunk.trans.key_data.keys;
            b8 *control_keys = win32vars.input_chunk.pers.control_keys;
            i32 control_keys_size = sizeof(win32vars.input_chunk.pers.control_keys);
            
            BYTE state[256];
            GetKeyboardState(state);
            if (state[VK_CAPITAL]){
                if (character_no_caps_lock >= 'a' && character_no_caps_lock <= 'z'){
                    character_no_caps_lock -= (u8)('a' - 'A');
                }
                else if (character_no_caps_lock >= 'A' && character_no_caps_lock <= 'Z'){
                    character_no_caps_lock += (u8)('a' - 'A');
                }
            }
            
            if (*count < KEY_INPUT_BUFFER_SIZE){
                data[*count].character = character;
                data[*count].character_no_caps_lock = character_no_caps_lock;
                data[*count].keycode = character_no_caps_lock;
                memcpy(data[*count].modifiers, control_keys, control_keys_size);
                ++(*count);
            }
            
            win32vars.got_useful_event = true;
        }break;
        
        case WM_MOUSEMOVE:
        {
            Vec2_i32 new_m = V2i32(LOWORD(lParam), HIWORD(lParam));
            if (new_m != win32vars.input_chunk.pers.mouse){
                win32vars.input_chunk.pers.mouse = new_m;
                win32vars.got_useful_event = true;
            }
        }break;
        
        case WM_MOUSEWHEEL:
        {
            win32vars.got_useful_event = true;
            i32 rotation = GET_WHEEL_DELTA_WPARAM(wParam);
            if (rotation > 0){
                win32vars.input_chunk.trans.mouse_wheel = -100;
            }
            else{
                win32vars.input_chunk.trans.mouse_wheel =  100;
            }
        }break;
        
        case WM_LBUTTONDOWN:
        {
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.mouse_l_press = 1;
            win32vars.input_chunk.pers.mouse_l = 1;
        }break;
        
        case WM_RBUTTONDOWN:
        {
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.mouse_r_press = 1;
            win32vars.input_chunk.pers.mouse_r = 1;
        }break;
        
        case WM_LBUTTONUP:
        {
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.mouse_l_release = 1;
            win32vars.input_chunk.pers.mouse_l = 0;
        }break;
        
        case WM_RBUTTONUP:
        {
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.mouse_r_release = 1;
            win32vars.input_chunk.pers.mouse_r = 0;
        }break;
        
        case WM_KILLFOCUS:
        case WM_SETFOCUS:
        {
            win32vars.got_useful_event = true;
            win32vars.input_chunk.pers.mouse_l = 0;
            win32vars.input_chunk.pers.mouse_r = 0;
            for (i32 i = 0; i < MDFR_INDEX_COUNT; ++i){
                win32vars.input_chunk.pers.control_keys[i] = 0;
            }
            win32vars.input_chunk.pers.controls = null_control_keys;
        }break;
        
        case WM_SIZE:
        {
            win32vars.got_useful_event = true;
            i32 new_width = LOWORD(lParam);
            i32 new_height = HIWORD(lParam);
            
            win32_resize(new_width, new_height);
        }break;
        
        case WM_DISPLAYCHANGE:
        {
            win32vars.got_useful_event = true;
            
            LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
            if (!(style & WS_OVERLAPPEDWINDOW)){
                MONITORINFO monitor_info = {sizeof(MONITORINFO)};
                if(GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
                {
                    SetWindowPos(hwnd, HWND_TOP,
                                 monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                                 monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                                 monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                                 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
                }
            }
        }break;
        
        case WM_PAINT:
        {
            win32vars.got_useful_event = true;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // NOTE(allen): Do nothing?
            EndPaint(hwnd, &ps);
        }break;
        
        case WM_CLIPBOARDUPDATE:
        {
            win32vars.got_useful_event = true;
            LogEventLit(win32vars.log_string(M), scratch, 0, 0, system_thread_get_id(),
                        "new clipboard contents");
        }break;
        
        case WM_CLOSE:
        case WM_DESTROY:
        {
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.trying_to_kill = true;
        }break;
        
        case WM_TIMER:
        {
            UINT_PTR timer_id = (UINT_PTR)wParam;
            KillTimer(win32vars.window_handle, timer_id);
            win32vars.got_useful_event = true;
        }break;
        
        case WM_4coder_ANIMATE:
        {
            win32vars.got_useful_event = true;
        }break;
        
        default:
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
    }
    
    return(result);
}

////////////////////////////////

internal b32
win32_wgl_good(Void_Func *f){
    return(f != 0 &&
           f != (Void_Func*)1 &&
           f != (Void_Func*)2 &&
           f != (Void_Func*)3 &&
           f != (Void_Func*)-1);
}

typedef HGLRC (wglCreateContextAttribsARB_Function)(HDC,HGLRC,i32*);
typedef BOOL (wglChoosePixelFormatARB_Function)(HDC,i32*,f32*,u32,i32*,u32*);
typedef char* (wglGetExtensionsStringEXT_Function)();
typedef VOID (wglSwapIntervalEXT_Function)(i32);

global wglCreateContextAttribsARB_Function *wglCreateContextAttribsARB = 0;
global wglChoosePixelFormatARB_Function *wglChoosePixelFormatARB = 0;
global wglGetExtensionsStringEXT_Function *wglGetExtensionsStringEXT = 0;
global wglSwapIntervalEXT_Function *wglSwapIntervalEXT = 0;

// TODO(allen): This requires all windows to be handled on a single thread.
// We would need a platform side thread context to get around this which would
// probably mean thread local storage would have to get involved.
global HWND win32_current_gl_window = 0;

internal b32
win32_gl_create_window(HWND *wnd_out, HGLRC *context_out, DWORD style, RECT rect){
    HINSTANCE this_instance = GetModuleHandle(0);
    
    local_persist b32 srgb_support = false;
    local_persist b32 register_success = true;
    local_persist b32 first_call = true;
    if (first_call){
        first_call = false;
        
        // NOTE(allen): Create the GL bootstrap window
        WNDCLASSW wglclass = {};
        wglclass.lpfnWndProc = DefWindowProcW;
        wglclass.hInstance = this_instance;
        wglclass.lpszClassName = L"wgl-loader";
        if (RegisterClassW(&wglclass) == 0){
            register_success = false;
            goto fail_register;
        }
        
        HWND wglwindow = CreateWindowW(wglclass.lpszClassName, L"", 0, 0, 0, 0, 0,
                                       0, 0, this_instance, 0);
        if (wglwindow == 0){
            register_success = false;
            goto fail_register;
        }
        
        // NOTE(allen): Create the GL bootstrap context
        HDC wgldc = GetDC(wglwindow);
        
        PIXELFORMATDESCRIPTOR format = {};
        format.nSize = sizeof(format);
        format.nVersion = 1;
        format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        format.iPixelType = PFD_TYPE_RGBA;
        format.cColorBits = 32;
        format.cAlphaBits = 8;
        format.cDepthBits = 24;
        format.iLayerType = PFD_MAIN_PLANE;
        i32 suggested_format_index = ChoosePixelFormat(wgldc, &format);
        if (!SetPixelFormat(wgldc, suggested_format_index, &format)){
            register_success = false;
            goto fail_register;
        }
        
        HGLRC wglcontext = wglCreateContext(wgldc);
        if (wglcontext == 0){
            register_success = false;
            goto fail_register;
        }
        
        if (!wglMakeCurrent(wgldc, wglcontext)){
            register_success = false;
            goto fail_register;
        }
        
        // NOTE(allen): Load wgl extensions
#define LoadWGL(f,l) Stmnt((f) = (f##_Function*)wglGetProcAddress(#f); \
        (l) = (l) && win32_wgl_good((Void_Func*)(f));)
        
        b32 load_success = true;
        LoadWGL(wglCreateContextAttribsARB, load_success);
        LoadWGL(wglChoosePixelFormatARB, load_success);
        LoadWGL(wglGetExtensionsStringEXT, load_success);
        
        if (!load_success){
            register_success = false;
            goto fail_register;
        }
        
        char *extensions_c = wglGetExtensionsStringEXT();
        String_Const_u8 extensions = SCu8((u8*)extensions_c);
        
        {
            String_Const_u8 s = string_skip_whitespace(extensions);
            for (;s.size > 0;){
                umem end = string_find_first_whitespace(s);
                String_Const_u8 m = string_prefix(s, end);
                if (string_match(m, string_u8_litexpr("WGL_EXT_framebuffer_sRGB")) ||
                    string_match(m, string_u8_litexpr("WGL_ARB_framebuffer_sRGB"))){
                    srgb_support = true;
                }
                else if (string_match(m, string_u8_litexpr("WGL_EXT_swap_interval"))){
                    b32 wgl_swap_interval_ext = true;
                    LoadWGL(wglSwapIntervalEXT, wgl_swap_interval_ext);
                    if (!wgl_swap_interval_ext){
                        wglSwapIntervalEXT = 0;
                    }
                }
                s = string_skip_whitespace(string_skip(s, end));
            }
        }
        
        // NOTE(allen): Load gl functions
#define GL_FUNC(f,R,P) LoadWGL(f,load_success);
#include "opengl/4ed_opengl_funcs.h"
        
        if (!load_success){
            register_success = false;
            goto fail_register;
        }
        
        // NOTE(allen): Cleanup the GL bootstrap resources
        ReleaseDC(wglwindow, wgldc);
        DestroyWindow(wglwindow);
        wglDeleteContext(wglcontext);
        
        // NOTE(allen): Register the graphics window class
        WNDCLASSW wndclass = {};
        wndclass.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
        wndclass.lpfnWndProc = win32_proc;
        wndclass.hInstance = this_instance;
        wndclass.lpszClassName = L"GRAPHICS-WINDOW-NAME";
        if (RegisterClassW(&wndclass) == 0){
            register_success = false;
            goto fail_register;
        }
    }
    fail_register:;
    
    b32 result = false;
    if (register_success){
        // NOTE(allen): Create the graphics window
        HWND wnd = CreateWindowExW(0, L"GRAPHICS-WINDOW-NAME", L"GRAPHICS", style,
                                   CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                                   0, 0, this_instance, 0);
        
        *wnd_out = 0;
        *context_out = 0;
        if (wnd != 0){
            HDC dc = GetDC(wnd);
            
            PIXELFORMATDESCRIPTOR format = {};
            
            i32 pixel_attrib_list[] = {
                /* 0*/WGL_DRAW_TO_WINDOW_ARB, TRUE,
                /* 2*/WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
                /* 4*/WGL_SUPPORT_OPENGL_ARB, TRUE,
                /* 6*/WGL_DOUBLE_BUFFER_ARB, false,
                /* 8*/WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
                /*10*/WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
                /*12*/0,
            };
            if (!srgb_support){
                pixel_attrib_list[10] = 0;
            }
            
            i32 suggested_format_index = 0;
            u32 ignore = 0;
            if (!wglChoosePixelFormatARB(dc, pixel_attrib_list, 0, 1, &suggested_format_index, &ignore)){
                goto fail_window_init;
            }
            
            DescribePixelFormat(dc, suggested_format_index, sizeof(format), &format);
            if (!SetPixelFormat(dc, suggested_format_index, &format)){
                goto fail_window_init;
            }
            
#if 1
            i32 context_attrib_list[] = {
                /*0*/WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                /*2*/WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                /*4*/WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if GL_DEBUG_MODE
                    |WGL_CONTEXT_DEBUG_BIT_ARB
#endif
                    ,
                /*6*/WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                /*8*/0
            };
#else
            i32 context_attrib_list[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 2,
                WGL_CONTEXT_MINOR_VERSION_ARB, 1,
                WGL_CONTEXT_FLAGS_ARB, 0,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                0
            };
#endif
            
            HGLRC context = wglCreateContextAttribsARB(dc, 0, context_attrib_list);
            if (context == 0){
                goto fail_window_init;
            }
            
            wglMakeCurrent(dc, context);
            win32_current_gl_window = wnd;
            if (wglSwapIntervalEXT != 0){
                wglSwapIntervalEXT(1);
            }
            *wnd_out = wnd;
            *context_out = context;
            result = true;
            
            if (false){
                fail_window_init:;
                DWORD error = GetLastError();
                ReleaseDC(wnd, dc);
                DestroyWindow(wnd);
                SetLastError(error);
            }
        }
    }
    
    return(result);
}

////////////////////////////////

#include "4ed_link_system_functions.cpp"
#include "4ed_shared_init_logic.cpp"

int CALL_CONVENTION
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    i32 argc = __argc;
    char **argv = __argv;
    
    //
    // System Linkage
    //
    
    sysfunc.font_make_face = ft__font_make_face;
    sysfunc.get_texture = gl__get_texture;
    sysfunc.fill_texture = gl__fill_texture;
    link_system_code();
    
    //
    // Memory init
    //
    
    
    memset(&win32vars, 0, sizeof(win32vars));
    memset(&target, 0, sizeof(target));
    memset(&memory_vars, 0, sizeof(memory_vars));
    memset(&plat_settings, 0, sizeof(plat_settings));
    
	win32vars.arena = make_arena_system(&sysfunc);
    
    memset(&libraries, 0, sizeof(libraries));
    memset(&app, 0, sizeof(app));
    memset(&custom_api, 0, sizeof(custom_api));
    
    memory_init(&win32vars.arena);
    
    win32vars.cursor_show = MouseCursorShow_Always;
    win32vars.prev_cursor_show = MouseCursorShow_Always;
    
    dll_init_sentinel(&win32vars.free_win32_objects);
    dll_init_sentinel(&win32vars.timer_objects);
    
    InitializeCriticalSection(&win32vars.thread_launch_mutex);
    InitializeConditionVariable(&win32vars.thread_launch_cv);
    
    //
    // HACK(allen):
    // Previously zipped stuff is here, it should be zipped in the new pattern now.
    //
    
    init_shared_vars();
    
    load_app_code(&win32vars.arena);
    win32vars.log_string = app.get_logger(&sysfunc);
    read_command_line(&win32vars.arena, argc, argv);
    
    //
    // Load Custom Code
    //
    
#if defined(FRED_SUPER)
    load_custom_code(&win32vars.arena);
#else
    custom_api.get_bindings = get_bindings;
#endif
    
    //
    // Window and GL Initialization
    //
    
    RECT window_rect = {};
    if (plat_settings.set_window_size){
        window_rect.right = plat_settings.window_w;
        window_rect.bottom = plat_settings.window_h;
    }
    else{
        window_rect.right = 800;
        window_rect.bottom = 600;
    }
    
    if (!AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, false)){
        //LOG("Could not get adjusted window.\n");
    }
    
    i32 window_style = WS_OVERLAPPEDWINDOW;
    if (!plat_settings.fullscreen_window && plat_settings.maximize_window){
        window_style |= WS_MAXIMIZE;
    }
    
    HGLRC window_opengl_context = 0;
    if (!win32_gl_create_window(&win32vars.window_handle, &window_opengl_context, window_style, window_rect)){
        exit(1);
    }
    
    GetClientRect(win32vars.window_handle, &window_rect);
    win32_resize(window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
    
    //
    // Misc System Initializations
    //
    
    if (!AddClipboardFormatListener(win32vars.window_handle)){
        win32_output_error_string(&win32vars.arena, ErrorString_UseLog);
    }
    
    win32vars.clip_max = KB(16);
    win32vars.clip_buffer = (u8*)system_memory_allocate(win32vars.clip_max);
    
    win32vars.clipboard_sequence = GetClipboardSequenceNumber();
    if (win32vars.clipboard_sequence == 0){
        win32_post_clipboard(&win32vars.arena, "", 0);
        
        win32vars.clipboard_sequence = GetClipboardSequenceNumber();
        win32vars.next_clipboard_is_self = 0;
        
        if (win32vars.clipboard_sequence == 0){
            OutputDebugStringA("Failure while initializing clipboard\n");
        }
    }
    else{
        win32_read_clipboard_contents(&win32vars.arena);
    }
    
    win32_keycode_init();
    
    win32vars.cursor_ibeam = LoadCursor(NULL, IDC_IBEAM);
    win32vars.cursor_arrow = LoadCursor(NULL, IDC_ARROW);
    win32vars.cursor_leftright = LoadCursor(NULL, IDC_SIZEWE);
    win32vars.cursor_updown = LoadCursor(NULL, IDC_SIZENS);
    
    LARGE_INTEGER f;
    if (QueryPerformanceFrequency(&f)){
        win32vars.count_per_usecond = (f32)f.QuadPart / 1000000.f;
    }
    else{
        // NOTE(allen): Just guess.
        win32vars.count_per_usecond = 1.f;
    }
    Assert(win32vars.count_per_usecond > 0.f);
    
    //
    // App init
    //
    
    {
        Temp_Memory temp = begin_temp(&win32vars.arena);
        String_Const_u8 curdir = sysfunc.get_current_path(&win32vars.arena);
        curdir = string_mod_replace_character(curdir, '\\', '/');
        app.init(&sysfunc, &target, &memory_vars, win32vars.clipboard_contents, curdir, custom_api);
        end_temp(temp);
    }
    
    //
    // Main loop
    //
    
    b32 keep_running = true;
    win32vars.first = true;
    timeBeginPeriod(1);
    
    if (plat_settings.fullscreen_window){
        win32_toggle_fullscreen();
    }
    
    SetForegroundWindow(win32vars.window_handle);
    SetActiveWindow(win32vars.window_handle);
    ShowWindow(win32vars.window_handle, SW_SHOW);
    
    //LOG("Beginning main loop\n");
    u64 timer_start = system_now_time();
    MSG msg;
    for (;keep_running;){
        // TODO(allen): Find a good way to wait on a pipe
        // without interfering with the reading process.
        // NOTE(allen): Looks like we can ReadFile with a
        // size of zero in an IOCP for this effect.
        if (!win32vars.first){
            if (win32vars.running_cli == 0){
                win32vars.got_useful_event = false;
            }
            
            b32 get_more_messages = true;
            do{
                if (win32vars.got_useful_event == 0){
                    get_more_messages = GetMessage(&msg, 0, 0, 0);
                }
                else{
                    get_more_messages = PeekMessage(&msg, 0, 0, 0, 1);
                }
                
                if (get_more_messages){
                    if (msg.message == WM_QUIT){
                        keep_running = false;
                    }else{
                        b32 treat_normally = true;
                        if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN){
                            switch (msg.wParam){
                                case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
                                case VK_MENU:case VK_LMENU:case VK_RMENU:
                                case VK_SHIFT:case VK_LSHIFT:case VK_RSHIFT:break;
                                
                                default: treat_normally = false; break;
                            }
                        }
                        
                        if (treat_normally){
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                        else{
                            Control_Keys *controls = &win32vars.input_chunk.pers.controls;
                            
                            b8 ctrl = (controls->r_ctrl || (controls->l_ctrl && !controls->r_alt));
                            b8 alt = (controls->l_alt || (controls->r_alt && !controls->l_ctrl));
                            
                            if (win32vars.lctrl_lalt_is_altgr){
                                if (controls->l_alt && controls->l_ctrl){
                                    ctrl = 0;
                                    alt = 0;
                                }
                            }
                            
                            BYTE ctrl_state = 0, alt_state = 0;
                            BYTE state[256];
                            if (ctrl || alt){
                                GetKeyboardState(state);
                                if (ctrl){
                                    ctrl_state = state[VK_CONTROL];
                                    state[VK_CONTROL] = 0;
                                }
                                if (alt){
                                    alt_state = state[VK_MENU];
                                    state[VK_MENU] = 0;
                                }
                                SetKeyboardState(state);
                                
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                                
                                if (ctrl){
                                    state[VK_CONTROL] = ctrl_state;
                                }
                                if (alt){
                                    state[VK_MENU] = alt_state;
                                }
                                SetKeyboardState(state);
                            }
                            else{
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }
                    }
                }
            }while (get_more_messages);
        }
        
        // NOTE(allen): Mouse Out of Window Detection
        POINT mouse_point;
        if (GetCursorPos(&mouse_point) &&
            ScreenToClient(win32vars.window_handle, &mouse_point)){
            
            i32_Rect screen;
            screen.x0 = 0;
            screen.y0 = 0;
            screen.x1 = target.width;
            screen.y1 = target.height;
            
            i32 mx = mouse_point.x;
            i32 my = mouse_point.y;
            
            b32 is_hit = false;
            if (mx >= screen.x0 && mx < screen.x1 && my >= screen.y0 && my < screen.y1){
                is_hit = true;
            }
            
            if (!is_hit){
                win32vars.input_chunk.trans.out_of_window = true;
            }
            
            win32vars.input_chunk.pers.mouse = V2i32(mouse_point.x, mouse_point.y);
        }
        else{
            win32vars.input_chunk.trans.out_of_window = true;
        }
        
        // NOTE(allen): Prepare the Frame Input
        
        // TODO(allen): CROSS REFERENCE WITH LINUX SPECIAL CODE "TIC898989"
        Win32_Input_Chunk input_chunk = win32vars.input_chunk;
        win32vars.input_chunk.trans = null_input_chunk_transient;
        
        input_chunk.pers.control_keys[MDFR_CAPS_INDEX] = GetKeyState(VK_CAPITAL) & 0x1;
        
        Application_Step_Input input = {};
        
        input.first_step = win32vars.first;
        input.dt = frame_useconds/1000000.f;
        
        input.keys = input_chunk.trans.key_data;
        memcpy(input.keys.modifiers, input_chunk.pers.control_keys, sizeof(input.keys.modifiers));
        
        input.mouse.out_of_window = input_chunk.trans.out_of_window;
        
        input.mouse.l = input_chunk.pers.mouse_l;
        input.mouse.press_l = input_chunk.trans.mouse_l_press;
        input.mouse.release_l = input_chunk.trans.mouse_l_release;
        
        input.mouse.r = input_chunk.pers.mouse_r;
        input.mouse.press_r = input_chunk.trans.mouse_r_press;
        input.mouse.release_r = input_chunk.trans.mouse_r_release;
        
        input.mouse.wheel = input_chunk.trans.mouse_wheel;
        input.mouse.p = input_chunk.pers.mouse;
        
        input.trying_to_kill = input_chunk.trans.trying_to_kill;
        
        // TODO(allen): Not really appropriate to round trip this all the way to the OS layer, redo this system.
        // NOTE(allen): Ask the Core About Exiting if We Have an Exit Signal
        if (win32vars.send_exit_signal){
            input.trying_to_kill = true;
            win32vars.send_exit_signal = false;
        }
        
        // NOTE(allen): Frame Clipboard Input
        block_zero_struct(&win32vars.clipboard_contents);
        input.clipboard_changed = false;
        if (win32vars.clipboard_sequence != 0){
            DWORD new_number = GetClipboardSequenceNumber();
            if (new_number != win32vars.clipboard_sequence){
                if (win32vars.next_clipboard_is_self){
                    win32vars.next_clipboard_is_self = false;
                }
                else{
                    for (i32 R = 0; R < 4; ++R){
                        if (win32_read_clipboard_contents(&win32vars.arena)){
                            input.clipboard_changed = true;
                            break;
                        }
                    }
                }
                win32vars.clipboard_sequence = new_number;
            }
        }
        input.clipboard = win32vars.clipboard_contents;
        
        win32vars.clip_post.size = 0;
        
        
        // NOTE(allen): Application Core Update
        Application_Step_Result result = {};
        if (app.step != 0){
            result = app.step(&sysfunc, &target, &memory_vars, &input);
        }
        else{
            //LOG("app.step == 0 -- skipping\n");
        }
        
        // NOTE(allen): Finish the Loop
        if (result.perform_kill){
            keep_running = false;
        }
        
        // NOTE(allen): Post New Clipboard Content
        if (win32vars.clip_post.size > 0){
            win32_post_clipboard(&win32vars.arena, (char*)win32vars.clip_post.str, (i32)win32vars.clip_post.size);
        }
        
        // NOTE(allen): Switch to New Title
        if (result.has_new_title){
            SetWindowText_utf8(&win32vars.arena, win32vars.window_handle, (u8*)result.title_string);
        }
        
        // NOTE(allen): Switch to New Cursor
        Win32SetCursorFromUpdate(result.mouse_cursor_type);
        if (win32vars.cursor_show != win32vars.prev_cursor_show){
            win32vars.prev_cursor_show = win32vars.cursor_show;
            switch (win32vars.cursor_show){
                case MouseCursorShow_Never:
                {
                    i32 counter = 0;
                    do{
                        counter = ShowCursor(false);
                    }while(counter >= 0);
                }break;
                
                case MouseCursorShow_Always:
                {
                    i32 counter = 0;
                    do{
                        counter = ShowCursor(true);
                    }while(counter <= 0);
                }break;
                
                // TODO(allen): MouseCursorShow_HideWhenStill
            }
        }
        
        // NOTE(allen): update lctrl_lalt_is_altgr status
        win32vars.lctrl_lalt_is_altgr = (b8)result.lctrl_lalt_is_altgr;
        
        // NOTE(allen): render
        HDC hdc = GetDC(win32vars.window_handle);
        gl_render(&target, &shared_vars.pixel_scratch);
        SwapBuffers(hdc);
        ReleaseDC(win32vars.window_handle, hdc);
        
        // NOTE(allen): toggle full screen
        if (win32vars.do_toggle){
            win32_toggle_fullscreen();
            win32vars.do_toggle = false;
        }
        
        // NOTE(allen): schedule another step if needed
        if (result.animating){
            system_schedule_step(0);
        }
        
        // NOTE(allen): sleep a bit to cool off :)
        u64 timer_end = system_now_time();
        u64 end_target = timer_start + frame_useconds;
        
        for (;timer_end < end_target;){
            DWORD samount = (DWORD)((end_target - timer_end)/1000);
            if (samount > 0){
                Sleep(samount);
            }
            timer_end = system_now_time();
        }
        timer_start = system_now_time();
        
        win32vars.first = false;
    }
    
    return(0);
}

#include "win32_utf8.cpp"

#if 0
// NOTE(allen): In case I want to switch back to a console application at some point.
int main(int argc, char **argv){
    HINSTANCE hInstance = GetModuleHandle(0);
}
#endif

// BOTTOM

