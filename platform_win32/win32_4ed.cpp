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
#include "4ed_os_comp_cracking.h"

//
// Program setup
//

#define UNICODE

#define FPS 60
#define frame_useconds (1000000 / FPS)

#include "4ed_defines.h"
#include "4coder_API/version.h"

#define WINDOW_NAME L"4coder: " L_VERSION

#include <string.h>
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

#include <Windows.h>
#include <GL/gl.h>
#include "win32_gl.h"

#define GL_TEXTURE_MAX_LEVEL 0x813D

//////////////////////////////

#include "win32_utf8.h"

#include "4ed_file_track.h"
#include "4ed_font_interface_to_os.h"
#include "4ed_system_shared.h"

#include "4ed_shared_thread_constants.h"
#include "win32_threading_wrapper.h"

//
// Win32_Vars structs
//

#define WM_4coder_ANIMATE (WM_USER + 0)

struct Control_Keys{
    b8 l_ctrl;
    b8 r_ctrl;
    b8 l_alt;
    b8 r_alt;
};
global Control_Keys null_control_keys = {0};

struct Win32_Input_Chunk_Transient{
    Key_Input_Data key_data;
    b8 mouse_l_press, mouse_l_release;
    b8 mouse_r_press, mouse_r_release;
    b8 out_of_window;
    i8 mouse_wheel;
    b8 trying_to_kill;
};
global Win32_Input_Chunk_Transient null_input_chunk_transient = {0};

struct Win32_Input_Chunk_Persistent{
    i32 mouse_x, mouse_y;
    b8 mouse_l, mouse_r;
    
    Control_Keys controls;
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

#include "4ed_coroutine.cpp"

////////////////////////////////

struct Win32_Vars{
    Win32_Input_Chunk input_chunk;
    b32 lctrl_lalt_is_altgr;
    b32 got_useful_event;
    
    b32 full_screen;
    b32 do_toggle;
    WINDOWPLACEMENT bordered_win_pos;
    b32 send_exit_signal;
    
    HCURSOR cursor_ibeam;
    HCURSOR cursor_arrow;
    HCURSOR cursor_leftright;
    HCURSOR cursor_updown;
    
    u8 *clip_buffer;
    u32 clip_max;
    String clipboard_contents;
    b32 next_clipboard_is_self;
    DWORD clipboard_sequence;
    
    HWND window_handle;
    i32 dpi_x, dpi_y;
    
    f64 count_per_usecond;
    b32 first;
    i32 running_cli;
    
    u32 log_position;
};

////////////////////////////////

global Win32_Vars win32vars;
global Render_Target target;
global Application_Memory memory_vars;
global Plat_Settings plat_settings;

global Libraries libraries;
global App_Functions app;
global Custom_API custom_api;

global Coroutine_System_Auto_Alloc coroutines;

////////////////////////////////

#include "win32_error_box.cpp"

////////////////////////////////

internal HANDLE
handle_type(Plat_Handle h){
    HANDLE result;
    memcpy(&result, &h, sizeof(result));
    return(result);
}

internal Plat_Handle
handle_type(HANDLE h){
    Plat_Handle result = {0};
    memcpy(&result, &h, sizeof(h));
    return(result);
}

////////////////////////////////

#include "win32_4ed_functions.cpp"
#include "4ed_shared_file_handling.cpp"

////////////////////////////////

internal void
system_schedule_step(){
    PostMessage(win32vars.window_handle, WM_4coder_ANIMATE, 0, 0);
}

////////////////////////////////

#include "4ed_work_queues.cpp"

////////////////////////////////

// HACK(allen): Get this shit working more properly (look at pens)
internal void
win32_toggle_fullscreen(){
    HWND win = win32vars.window_handle;
    DWORD style = GetWindowLongW(win, GWL_STYLE);
    b32 is_full = ((style & WS_OVERLAPPEDWINDOW) != 0);
    if (is_full){
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
    switch (show){
        case MouseCursorShow_Never: ShowCursor(false); break;
        case MouseCursorShow_Always: ShowCursor(true); break;
        // TODO(allen): MouseCursor_HideWhenStill
    }
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

internal
Sys_Send_Exit_Signal_Sig(system_send_exit_signal){
    win32vars.send_exit_signal = true;
}

//
// Coroutines
//

internal
Sys_Create_Coroutine_Sig(system_create_coroutine){
    Coroutine *coroutine = coroutine_system_alloc(&coroutines);
    Coroutine_Head *result = 0;
    if (coroutine != 0){
        coroutine_set_function(coroutine, func);
        result = &coroutine->head;
    }
    return(result);
}

internal
Sys_Launch_Coroutine_Sig(system_launch_coroutine){
    Coroutine *coroutine = (Coroutine*)head;
    coroutine->head.in = in;
    coroutine->head.out = out;
    
    Coroutine *active = coroutine->sys->active;
    Assert(active != 0);
    coroutine_launch(active, coroutine);
    Assert(active == coroutine->sys->active);
    
    Coroutine_Head *result = &coroutine->head;
    if (coroutine->state == CoroutineState_Dead){
        coroutine_system_free(&coroutines, coroutine);
        result = 0;
    }
    return(result);
}

Sys_Resume_Coroutine_Sig(system_resume_coroutine){
    Coroutine *coroutine = (Coroutine*)head;
    coroutine->head.in = in;
    coroutine->head.out = out;
    
    Coroutine *active = coroutine->sys->active;
    Assert(active != 0);
    coroutine_resume(active, coroutine);
    Assert(active == coroutine->sys->active);
    
    Coroutine_Head *result = &coroutine->head;
    if (coroutine->state == CoroutineState_Dead){
        coroutine_system_free(&coroutines, coroutine);
        result = 0;
    }
    return(result);
}

Sys_Yield_Coroutine_Sig(system_yield_coroutine){
    Coroutine *coroutine = (Coroutine*)head;
    coroutine_yield(coroutine);
}

//
// Clipboard
//

internal
Sys_Post_Clipboard_Sig(system_post_clipboard){
    if (OpenClipboard(win32vars.window_handle)){
        EmptyClipboard();
        HANDLE memory_handle;
        memory_handle = GlobalAlloc(GMEM_MOVEABLE, str.size+1);
        if (memory_handle){
            char *dest = (char*)GlobalLock(memory_handle);
            copy_fast_unsafe_cs(dest, str);
            GlobalUnlock(memory_handle);
            SetClipboardData(CF_TEXT, memory_handle);
            win32vars.next_clipboard_is_self = 1;
        }
        CloseClipboard();
    }
}

internal b32
win32_read_clipboard_contents(){
    b32 result = false;
    
    if (IsClipboardFormatAvailable(CF_UNICODETEXT)){
        result = true;
        if (OpenClipboard(win32vars.window_handle)){
            HANDLE clip_data = GetClipboardData(CF_UNICODETEXT);
            if (clip_data != 0){
                Partition *scratch = &shared_vars.scratch;
                Temp_Memory temp = begin_temp_memory(scratch);
                
                u16 *clip_16 = (u16*)GlobalLock(clip_data);
                
                if (clip_16 != 0){
                    u32 clip_16_len = 0;
                    for(;clip_16[clip_16_len];++clip_16_len);
                    
                    // TODO(allen): Extend the buffer if it is too small.
                    b32 error = false;
                    u32 clip_8_len = (u32)utf16_to_utf8_minimal_checking(win32vars.clip_buffer, win32vars.clip_max-1, clip_16, clip_16_len, &error);
                    
                    if (clip_8_len < win32vars.clip_max && !error){
                        win32vars.clip_buffer[clip_8_len] = 0;
                        
                        win32vars.clipboard_contents = make_string_cap(win32vars.clip_buffer, clip_8_len, win32vars.clip_max);
                    }
                    
                    GlobalUnlock(clip_data);
                }
                
                if (win32vars.clipboard_contents.str){
                    win32vars.clipboard_contents.size = str_size((char*)win32vars.clipboard_contents.str);
                }
                
                end_temp_memory(temp);
            }
            CloseClipboard();
        }
    }
    
    return(result);
}


//
// Command Line Exectuion
//

internal
Sys_CLI_Call_Sig(system_cli_call){
    char cmd[] = "c:\\windows\\system32\\cmd.exe";
    char *env_variables = 0;
    char command_line[2048];
    
    String s = make_fixed_width_string(command_line);
    copy_ss(&s, make_lit_string("/C "));
    append_partial_sc(&s, script_name);
    b32 success = terminate_with_null(&s);
    
    if (success){
        success = 0;
        
        SECURITY_ATTRIBUTES sec_attributes = {};
        HANDLE out_read;
        HANDLE out_write;
        
        sec_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        sec_attributes.bInheritHandle = TRUE;
        
        if (CreatePipe(&out_read, &out_write, &sec_attributes, 0)){
            if (SetHandleInformation(out_read, HANDLE_FLAG_INHERIT, 0)){
                STARTUPINFO startup = {};
                startup.cb = sizeof(STARTUPINFO);
                startup.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
                startup.hStdError = out_write;
                startup.hStdOutput = out_write;
                startup.wShowWindow = SW_HIDE;
                
                PROCESS_INFORMATION info = {};
                
                Assert(sizeof(Plat_Handle) >= sizeof(HANDLE));
                if (CreateProcess_utf8((u8*)cmd, (u8*)command_line, 0, 0, TRUE, 0, env_variables, (u8*)path, &startup, &info)){
                    success = 1;
                    CloseHandle(info.hThread);
                    *(HANDLE*)&cli_out->proc = info.hProcess;
                    *(HANDLE*)&cli_out->out_read = out_read;
                    *(HANDLE*)&cli_out->out_write = out_write;
                    
                    ++win32vars.running_cli;
                }
                else{
                    CloseHandle(out_read);
                    CloseHandle(out_write);
                    *(HANDLE*)&cli_out->proc = INVALID_HANDLE_VALUE;
                    *(HANDLE*)&cli_out->out_read = INVALID_HANDLE_VALUE;
                    *(HANDLE*)&cli_out->out_write = INVALID_HANDLE_VALUE;
                }
            }
            else{
                // TODO(allen): failed SetHandleInformation
                CloseHandle(out_read);
                CloseHandle(out_write);
                *(HANDLE*)&cli_out->proc = INVALID_HANDLE_VALUE;
                *(HANDLE*)&cli_out->out_read = INVALID_HANDLE_VALUE;
                *(HANDLE*)&cli_out->out_write = INVALID_HANDLE_VALUE;
            }
        }
        else{
            // TODO(allen): failed CreatePipe
        }
    }
    
    return success;
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
            TentativeAssert(remaining == read_amount);
            pos += remaining;
            remaining = 0;
        }
        else{
            has_more = 1;
            ReadFile(handle, dest + pos, max - pos, &read_amount, 0);
            TentativeAssert(max - pos == read_amount);
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
    b32 close_me = 0;
    HANDLE proc = *(HANDLE*)&cli->proc;
    DWORD result = 0;
    
    if (WaitForSingleObject(proc, 0) == WAIT_OBJECT_0){
        if (GetExitCodeProcess(proc, &result) == 0){
            cli->exit = -1;
        }
        else{
            cli->exit = (i32)result;
        }
        
        close_me = 1;
        CloseHandle(*(HANDLE*)&cli->proc);
        CloseHandle(*(HANDLE*)&cli->out_read);
        CloseHandle(*(HANDLE*)&cli->out_write);
        
        --win32vars.running_cli;
    }
    return(close_me);
}

//
// Linkage to Custom and Application
//

#include "4ed_font_data.h"
#include "4ed_system_shared.cpp"

//
// Helpers
//

global Key_Code keycode_lookup_table[255];

internal void
Win32KeycodeInit(){
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
Win32RedrawScreen(HDC hdc){
    launch_rendering(&sysfunc, &target);
    glFlush();
    SwapBuffers(hdc);
}

internal void
Win32Resize(i32 width, i32 height){
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

internal void*
win32_load_gl_always(char *name, HMODULE module){
    void *p = (void *)wglGetProcAddress(name), *r = 0;
    if(p == 0 ||
       (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
       (p == (void*)-1) ){
        r = (void *)GetProcAddress(module, name);
    }
    else{
        r = p;
    }
    return(r);
}

internal void CALL_CONVENTION
OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam){
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
}

internal void
Win32InitGL(){
    // GL context initialization
    PIXELFORMATDESCRIPTOR format;
    format.nSize = sizeof(format);
    format.nVersion = 1;
    format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    format.iPixelType = PFD_TYPE_RGBA;
    format.cColorBits = 32;
    format.cRedBits = 0;
    format.cRedShift = 0;
    format.cGreenBits = 0;
    format.cGreenShift = 0;
    format.cBlueBits = 0;
    format.cBlueShift = 0;
    format.cAlphaBits = 0;
    format.cAlphaShift = 0;
    format.cAccumBits = 0;
    format.cAccumRedBits = 0;
    format.cAccumGreenBits = 0;
    format.cAccumBlueBits = 0;
    format.cAccumAlphaBits = 0;
    format.cDepthBits = 24;
    format.cStencilBits = 8;
    format.cAuxBuffers = 0;
    format.iLayerType = PFD_MAIN_PLANE;
    format.bReserved = 0;
    format.dwLayerMask = 0;
    format.dwVisibleMask = 0;
    format.dwDamageMask = 0;
    
    HDC dc = GetDC(win32vars.window_handle);
    Assert(dc);
    int format_id = ChoosePixelFormat(dc, &format);
    Assert(format_id != 0);
    BOOL success = SetPixelFormat(dc, format_id, &format);
    Assert(success == TRUE); AllowLocal(success);
    
    HGLRC glcontext = wglCreateContext(dc);
    wglMakeCurrent(dc, glcontext);
    
    HMODULE module = LoadLibraryA("opengl32.dll");
    AllowLocal(module);
    
    wglCreateContextAttribsARB_Function *wglCreateContextAttribsARB = 0;
    wglCreateContextAttribsARB = (wglCreateContextAttribsARB_Function*)
        win32_load_gl_always("wglCreateContextAttribsARB", module);
    
    wglChoosePixelFormatARB_Function *wglChoosePixelFormatARB = 0;
    wglChoosePixelFormatARB = (wglChoosePixelFormatARB_Function*)
        win32_load_gl_always("wglChoosePixelFormatARB", module);
    
    if (wglCreateContextAttribsARB != 0 && wglChoosePixelFormatARB != 0){
        const int choosePixel_attribList[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, TRUE,
            WGL_SUPPORT_OPENGL_ARB, TRUE,
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
            WGL_COLOR_BITS_ARB, 32,
            WGL_DEPTH_BITS_ARB, 24,
            WGL_STENCIL_BITS_ARB, 8,
            0,
        };
        
        i32 extended_format_id = 0;
        u32 num_formats = 0;
        BOOL result =  wglChoosePixelFormatARB(dc, choosePixel_attribList, 0, 1, &extended_format_id, &num_formats);
        
        if (result != 0 && num_formats > 0){
            const int createContext_attribList[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 2,
                WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                0
            };
            
            if (extended_format_id == format_id){
                HGLRC extended_context = wglCreateContextAttribsARB(dc, 0, createContext_attribList);
                if (extended_context){
                    wglMakeCurrent(dc, extended_context);
                    wglDeleteContext(glcontext);
                    glcontext = extended_context;
                }
            }
        }
    }
    
#if (defined(BUILD_X64) && 1) || (defined(BUILD_X86) && 0)
#if defined(FRED_INTERNAL)
    // NOTE(casey): This slows down GL but puts error messages to
    // the debug console immediately whenever you do something wrong
    glDebugMessageCallback_type *glDebugMessageCallback = 
        (glDebugMessageCallback_type *)win32_load_gl_always("glDebugMessageCallback", module);
    glDebugMessageControl_type *glDebugMessageControl = 
        (glDebugMessageControl_type *)win32_load_gl_always("glDebugMessageControl", module);
    if(glDebugMessageCallback != 0 && glDebugMessageControl != 0)
    {
        glDebugMessageCallback(OpenGLDebugCallback, 0);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }
#endif
#endif
    
    ReleaseDC(win32vars.window_handle, dc);
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

internal void
Win32SetCursorFromUpdate(Application_Mouse_Cursor cursor){
    switch (cursor){
        case APP_MOUSE_CURSOR_ARROW:
        SetCursor(win32vars.cursor_arrow); break;
        
        case APP_MOUSE_CURSOR_IBEAM:
        SetCursor(win32vars.cursor_ibeam); break;
        
        case APP_MOUSE_CURSOR_LEFTRIGHT:
        SetCursor(win32vars.cursor_leftright); break;
        
        case APP_MOUSE_CURSOR_UPDOWN:
        SetCursor(win32vars.cursor_updown); break;
    }
}

internal u64
Win32HighResolutionTime(){
    u64 result = 0;
    LARGE_INTEGER t;
    if (QueryPerformanceCounter(&t)){
        result = (u64) (t.QuadPart / win32vars.count_per_usecond);
    }
    return(result);
}

internal LRESULT
Win32Callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;
    
    switch (uMsg){
        case WM_MENUCHAR:break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            switch (wParam){
                case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
                case VK_MENU:case VK_LMENU:case VK_RMENU:
                case VK_SHIFT:case VK_LSHIFT:case VK_RSHIFT:
                {
                    Control_Keys *controls = &win32vars.input_chunk.pers.controls;
                    b8 *control_keys = win32vars.input_chunk.pers.control_keys;
                    
                    b8 down = ((lParam & Bit_31)?(0):(1));
                    b8 is_right = ((lParam & Bit_24)?(1):(0));
                    
                    if (wParam != 255){
                        switch (wParam){
                            case VK_SHIFT:
                            {
                                control_keys[MDFR_SHIFT_INDEX] = down;
                            }break;
                            
                            case VK_CONTROL:
                            {
                                if (is_right) controls->r_ctrl = down;
                                else controls->l_ctrl = down;
                            }break;
                            
                            case VK_MENU:
                            {
                                if (is_right) controls->r_alt = down;
                                else controls->l_alt = down;
                            }break;
                        }
                        
                        b8 ctrl = (controls->r_ctrl || (controls->l_ctrl && !controls->r_alt));
                        b8 alt = (controls->l_alt || (controls->r_alt && !controls->l_ctrl));
                        
                        if (win32vars.lctrl_lalt_is_altgr){
                            if (controls->l_alt && controls->l_ctrl){
                                ctrl = 0;
                                alt = 0;
                            }
                        }
                        
                        control_keys[MDFR_CONTROL_INDEX] = ctrl;
                        control_keys[MDFR_ALT_INDEX] = alt;
                    }
                }break;
                
                default:
                {
                    b8 current_state = ((lParam & Bit_31)?(0):(1));
                    
                    if (current_state){
                        Key_Code key = keycode_lookup_table[(u8)wParam];
                        
                        if (key != 0){
                            i32 *count = &win32vars.input_chunk.trans.key_data.count;
                            Key_Event_Data *data = win32vars.input_chunk.trans.key_data.keys;
                            b8 *control_keys = win32vars.input_chunk.pers.control_keys;
                            i32 control_keys_size = sizeof(win32vars.input_chunk.pers.control_keys);
                            
                            Assert(*count < KEY_INPUT_BUFFER_SIZE);
                            data[*count].character = 0;
                            data[*count].character_no_caps_lock = 0;
                            data[*count].keycode = key;
                            memcpy(data[*count].modifiers, control_keys, control_keys_size);
                            ++(*count);
                            
                            win32vars.got_useful_event = true;
                        }
                    }
                }break;
            }/* switch */
        }break;
        
        case WM_CHAR: case WM_SYSCHAR: case WM_UNICHAR:
        {
            u16 character = (u16)wParam;
            
            if (character == '\r'){
                character = '\n';
            }
            else if (character == '\t'){
                character = '\t';
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
            
            Assert(*count < KEY_INPUT_BUFFER_SIZE);
            data[*count].character = character;
            data[*count].character_no_caps_lock = character_no_caps_lock;
            data[*count].keycode = character_no_caps_lock;
            memcpy(data[*count].modifiers, control_keys, control_keys_size);
            ++(*count);
            
            win32vars.got_useful_event = true;
        }break;
        
        case WM_MOUSEMOVE:
        {
            i32 new_x = LOWORD(lParam);
            i32 new_y = HIWORD(lParam);
            
            if (new_x != win32vars.input_chunk.pers.mouse_x || new_y != win32vars.input_chunk.pers.mouse_y){
                win32vars.input_chunk.pers.mouse_x = new_x;
                win32vars.input_chunk.pers.mouse_y = new_y;
                
                win32vars.got_useful_event = true;
            }
        }break;
        
        case WM_MOUSEWHEEL:
        {
            win32vars.got_useful_event = true;
            i32 rotation = GET_WHEEL_DELTA_WPARAM(wParam);
            if (rotation > 0){
                win32vars.input_chunk.trans.mouse_wheel = 1;
            }
            else{
                win32vars.input_chunk.trans.mouse_wheel = -1;
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
            
            Win32Resize(new_width, new_height);
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
            Win32RedrawScreen(hdc);
            EndPaint(hwnd, &ps);
        }break;
        
        case WM_CLOSE:
        case WM_DESTROY:
        {
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.trying_to_kill = 1;
        }break;
        
        case WM_4coder_ANIMATE:
        {
            win32vars.got_useful_event = true;
        }break;
        
        case WM_CANCELMODE:
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
        
        default:
        {
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }break;
    }
    
    return(result);
}

#include "4ed_link_system_functions.cpp"
#include "4ed_shared_init_logic.cpp"

int CALL_CONVENTION
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    i32 argc = __argc;
    char **argv = __argv;
    
    memset(&win32vars, 0, sizeof(win32vars));
    memset(&target, 0, sizeof(target));
    memset(&memory_vars, 0, sizeof(memory_vars));
    memset(&plat_settings, 0, sizeof(plat_settings));
    
    memset(&libraries, 0, sizeof(libraries));
    memset(&app, 0, sizeof(app));
    memset(&custom_api, 0, sizeof(custom_api));
    
    init_shared_vars();
    
    //
    // Linkage
    //
    
    link_system_code();
    load_app_code();
    link_rendering();
#if defined(FRED_SUPER)
    load_custom_code();
#else
    custom_api.get_bindings = get_bindings;
#endif
    
    //
    // Memory init
    //
    
    memory_init();
    
    //
    // Read Command Line
    //
    
    read_command_line(argc, argv);
    
    //
    // Threads
    //
    
    work_system_init();
    
    //
    // Coroutines
    //
    
    coroutines_init();
    
    //
    // Window and GL Initialization
    //
    
    WNDCLASS window_class = {0};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = (WNDPROC)(Win32Callback);
    window_class.hInstance = hInstance;
    window_class.lpszClassName = L"4coder-win32-wndclass";
    window_class.hIcon = LoadIcon(hInstance, L"main");
    
    if (!RegisterClass(&window_class)){
        exit(1);
    }
    
    RECT window_rect = {0};
    if (plat_settings.set_window_size){
        window_rect.right = plat_settings.window_w;
        window_rect.bottom = plat_settings.window_h;
    }
    else{
        window_rect.right = 800;
        window_rect.bottom = 600;
    }
    
    if (!AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, false)){
        LOG("Could not get adjusted window.\n");
    }
    
    i32 window_x = CW_USEDEFAULT;
    i32 window_y = CW_USEDEFAULT;
    
    if (plat_settings.set_window_pos){
        window_x = plat_settings.window_x;
        window_y = plat_settings.window_y;
        LOGF("Setting window position (%d, %d)\n", window_x, window_y);
    }
    
    LOG("Creating window... ");
    i32 window_style = WS_CAPTION|WS_MINIMIZEBOX|WS_BORDER;
    if (!plat_settings.fullscreen_window && plat_settings.maximize_window){
        window_style |= WS_MAXIMIZE;
    }
    win32vars.window_handle = CreateWindowEx(0, window_class.lpszClassName, WINDOW_NAME, window_style, window_x, window_y, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, 0, 0, hInstance, 0);
    
    if (win32vars.window_handle == 0){
        LOG("Failed\n");
        exit(1);
    }
    else{
        LOG("Success\n");
    }
    
    {
        HDC hdc = GetDC(win32vars.window_handle);
        
        // TODO(allen): not Windows XP compatible, how do I handle that?
        SetProcessDPIAware();
        win32vars.dpi_x = GetDeviceCaps(hdc, LOGPIXELSX);
        win32vars.dpi_y = GetDeviceCaps(hdc, LOGPIXELSY);
        
        GetClientRect(win32vars.window_handle, &window_rect);
        ReleaseDC(win32vars.window_handle, hdc);
    }
    
    Win32InitGL();
    Win32Resize(window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
    
    //
    // Font System Init
    //
    
    LOG("Initializing fonts\n");
    system_font_init(&sysfunc.font, 0, 0, plat_settings.font_size, plat_settings.use_hinting);
    
    //
    // Misc System Initializations
    //
    
    LOG("Initializing clipboard\n");
    win32vars.clip_max = KB(16);
    win32vars.clip_buffer = (u8*)system_memory_allocate(win32vars.clip_max);
    
    win32vars.clipboard_sequence = GetClipboardSequenceNumber();
    if (win32vars.clipboard_sequence == 0){
        system_post_clipboard(make_lit_string(""));
        
        win32vars.clipboard_sequence = GetClipboardSequenceNumber();
        win32vars.next_clipboard_is_self = 0;
        
        if (win32vars.clipboard_sequence == 0){
            OutputDebugStringA("Failure while initializing clipboard\n");
        }
    }
    else{
        win32_read_clipboard_contents();
    }
    
    Win32KeycodeInit();
    
    win32vars.cursor_ibeam = LoadCursor(NULL, IDC_IBEAM);
    win32vars.cursor_arrow = LoadCursor(NULL, IDC_ARROW);
    win32vars.cursor_leftright = LoadCursor(NULL, IDC_SIZEWE);
    win32vars.cursor_updown = LoadCursor(NULL, IDC_SIZENS);
    
    LARGE_INTEGER f;
    if (QueryPerformanceFrequency(&f)){
        win32vars.count_per_usecond = (f32)f.QuadPart / 1000000.f;
        LOGF("Got performance frequency %f\n", win32vars.count_per_usecond);
    }
    else{
        // NOTE(allen): Just guess.
        win32vars.count_per_usecond = 1.f;
        LOG("Did not get performance frequency, just guessing with 1.\n");
    }
    Assert(win32vars.count_per_usecond > 0.f);
    
    //
    // Main Loop
    //
    
    char cwd[4096];
    u32 size = sysfunc.get_current_path(cwd, sizeof(cwd));
    if (size == 0 || size >= sizeof(cwd)){
        system_error_box("Could not get current directory at launch.");
    }
    String curdir = make_string(cwd, size);
    terminate_with_null(&curdir);
    replace_char(&curdir, '\\', '/');
    
    LOG("Initializing application variables\n");
    app.init(&sysfunc, &target, &memory_vars, win32vars.clipboard_contents, curdir, custom_api);
    
    b32 keep_running = true;
    win32vars.first = true;
    timeBeginPeriod(1);
    
    if (plat_settings.fullscreen_window){
        win32_toggle_fullscreen();
    }
    
    SetForegroundWindow(win32vars.window_handle);
    SetActiveWindow(win32vars.window_handle);
    ShowWindow(win32vars.window_handle, SW_SHOW);
    
    LOG("Beginning main loop\n");
    u64 timer_start = Win32HighResolutionTime();
    system_acquire_lock(FRAME_LOCK);
    MSG msg;
    for (;keep_running;){
        // TODO(allen): Find a good way to wait on a pipe
        // without interfering with the reading process
        //  Looks like we can ReadFile with a size of zero
        // in an IOCP for this effect.
        if (!win32vars.first){
            system_release_lock(FRAME_LOCK);
            
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
            
            system_acquire_lock(FRAME_LOCK);
        }
        
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
                win32vars.input_chunk.trans.out_of_window = 1;
            }
            
            win32vars.input_chunk.pers.mouse_x = mouse_point.x;
            win32vars.input_chunk.pers.mouse_y = mouse_point.y;
        }
        else{
            win32vars.input_chunk.trans.out_of_window = 1;
        }
        
        Win32_Input_Chunk input_chunk = win32vars.input_chunk;
        win32vars.input_chunk.trans = null_input_chunk_transient;
        
        input_chunk.pers.control_keys[MDFR_CAPS_INDEX] = GetKeyState(VK_CAPITAL) & 0x1;
        
        win32vars.clipboard_contents = null_string;
        if (win32vars.clipboard_sequence != 0){
            DWORD new_number = GetClipboardSequenceNumber();
            if (new_number != win32vars.clipboard_sequence){
                win32vars.clipboard_sequence = new_number;
                if (win32vars.next_clipboard_is_self){
                    win32vars.next_clipboard_is_self = 0;
                }
                else{
                    win32_read_clipboard_contents();
                }
            }
        }
        
        Application_Step_Input input = {0};
        
        input.first_step = win32vars.first;
        
        // NOTE(allen): The expected dt given the frame limit in seconds.
        input.dt = frame_useconds / 1000000.f;
        
        input.keys = input_chunk.trans.key_data;
        
        input.mouse.out_of_window = input_chunk.trans.out_of_window;
        
        input.mouse.l = input_chunk.pers.mouse_l;
        input.mouse.press_l = input_chunk.trans.mouse_l_press;
        input.mouse.release_l = input_chunk.trans.mouse_l_release;
        
        input.mouse.r = input_chunk.pers.mouse_r;
        input.mouse.press_r = input_chunk.trans.mouse_r_press;
        input.mouse.release_r = input_chunk.trans.mouse_r_release;
        
        input.mouse.wheel = input_chunk.trans.mouse_wheel;
        input.mouse.x = input_chunk.pers.mouse_x;
        input.mouse.y = input_chunk.pers.mouse_y;
        
        input.clipboard = win32vars.clipboard_contents;
        
        
        Application_Step_Result result = {(Application_Mouse_Cursor)0};
        result.mouse_cursor_type = APP_MOUSE_CURSOR_DEFAULT;
        result.lctrl_lalt_is_altgr = win32vars.lctrl_lalt_is_altgr;
        result.trying_to_kill = input_chunk.trans.trying_to_kill;
        result.perform_kill = 0;
        
        if (win32vars.send_exit_signal){
            result.trying_to_kill = true;
            win32vars.send_exit_signal = false;
        }
        
        app.step(&sysfunc, &target, &memory_vars, &input, &result);
        
        if (result.perform_kill){
            keep_running = false;
        }
        
        if (win32vars.do_toggle){
            win32_toggle_fullscreen();
            win32vars.do_toggle = false;
        }
        
        Win32SetCursorFromUpdate(result.mouse_cursor_type);
        win32vars.lctrl_lalt_is_altgr = result.lctrl_lalt_is_altgr;
        
        HDC hdc = GetDC(win32vars.window_handle);
        Win32RedrawScreen(hdc);
        ReleaseDC(win32vars.window_handle, hdc);
        
        win32vars.first = 0;
        
        if (result.animating){
            system_schedule_step();
        }
        
        flush_thread_group(BACKGROUND_THREADS);
        
        u64 timer_end = Win32HighResolutionTime();
        u64 end_target = timer_start + frame_useconds;
        
        system_release_lock(FRAME_LOCK);
        while (timer_end < end_target){
            DWORD samount = (DWORD)((end_target - timer_end) / 1000);
            if (samount > 0) Sleep(samount);
            timer_end = Win32HighResolutionTime();
        }
        system_acquire_lock(FRAME_LOCK);
        timer_start = Win32HighResolutionTime();
    }
    
    return(0);
}

#include "4ed_shared_fonts.cpp"
#include "win32_4ed_file_track.cpp"
#include "4ed_font_static_functions.cpp"
#include "win32_utf8.cpp"

#if 0
// NOTE(allen): In case I want to switch back to a console application at some point.
int main(int argc, char **argv){
    HINSTANCE hInstance = GetModuleHandle(0);
}
#endif

// BOTTOM

