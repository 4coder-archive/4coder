/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Win32 layer for 4coder
 *
 */

// TOP

#define FPS 60
#define frame_useconds (1000000 / FPS)

#include <stdio.h>

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
#include "4coder_search_list.h"
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

#include "4coder_search_list.cpp"

#undef function
#define UNICODE
#include <Windows.h>
#define function static

#include "win32_utf8.h"
#include "win32_gl.h"

////////////////////////////////

global b32 log_os_enabled = false;
#define log_os(...) \
Stmnt( if (log_os_enabled){ fprintf(stdout, __VA_ARGS__); fflush(stdout); } )

//////////////////////////////

internal String_Const_u8 win32_get_error_string(void);
internal void win32_output_error_string(String_Const_u8 string);

//////////////////////////////

#define WM_4coder_ANIMATE (WM_USER + 0)

struct Control_Keys{
    b8 l_ctrl;
    b8 r_ctrl;
    b8 l_alt;
    b8 r_alt;
};

struct Win32_Input_Chunk_Transient{
    Input_List event_list;
    b8 mouse_l_press;
    b8 mouse_l_release;
    b8 mouse_r_press;
    b8 mouse_r_release;
    b8 out_of_window;
    i8 mouse_wheel;
    b8 trying_to_kill;
};

struct Win32_Input_Chunk_Persistent{
    Vec2_i32 mouse;
    Control_Keys controls;
    Input_Modifier_Set_Fixed modifiers;
    b8 mouse_l;
    b8 mouse_r;
};

struct Win32_Input_Chunk{
    Win32_Input_Chunk_Transient trans;
    Win32_Input_Chunk_Persistent pers;
};

////////////////////////////////

#define SLASH '\\'
#define DLL "dll"

#include "4coder_hash_functions.cpp"
#include "4coder_system_allocator.cpp"
#include "4coder_codepoint_map.cpp"

#include "4ed_mem.cpp"
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
    Thread_Context *tctx;
    
    Arena frame_arena;
    Input_Event *active_key_stroke;
    Input_Event *active_text_input;
    Win32_Input_Chunk input_chunk;
    b8 lctrl_lalt_is_altgr;
    b8 got_useful_event;
    
    Key_Mode key_mode;
    HKL kl_universal;
    
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
    
    b8 clip_catch_all;
    b8 next_clipboard_is_self;
    DWORD clipboard_sequence;
    Plat_Handle clip_wakeup_timer;
    
    Arena clip_post_arena;
    String_Const_u8 clip_post;
    
    HWND window_handle;
    f32 screen_scale_factor;
    
    DWORD audio_thread_id;
    
    void *volatile audio_mix_ctx;
    Audio_Mix_Sources_Function *volatile audio_mix_sources;
    Audio_Mix_Destination_Function *volatile audio_mix_destination;
    
    f64 usecond_per_count;
    b32 first;
    i32 running_cli;
    
    Node free_win32_objects;
    Node timer_objects;
    UINT_PTR timer_counter;
    
    CRITICAL_SECTION thread_launch_mutex;
    CONDITION_VARIABLE thread_launch_cv;
    b32 waiting_for_launch;
    
    System_Mutex global_frame_mutex;
    
    Log_Function *log_string;
};

////////////////////////////////

global Win32_Vars win32vars;
global Render_Target target;

////////////////////////////////

internal void
system_error_box(char *msg){
    MessageBoxA(0, msg, "Error", MB_OK);
    ExitProcess(1);
}

////////////////////////////////

internal String_Const_u8
win32_get_error_string(void){
    String_Const_u8 result = {};
    DWORD error = GetLastError();
    char *str = 0;
    char *str_ptr = (char*)&str;
    if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       0, error, 0, str_ptr, 0, 0)){
        result.str = (u8*)str;
        result.size = strlen(str);
    }
    return(result);
}

internal void
win32_output_error_string(String_Const_u8 error_string){
    system_error_box((char*)error_string.str);
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
#include "win32_audio.cpp"

////////////////////////////////

internal
system_load_library_sig(){
    HMODULE lib = LoadLibrary_utf8String(scratch, file_name);
    b32 result = false;
    if (lib != 0){
        result = true;
        *out = handle_type(lib);
    }
    return(result);
}

internal
system_release_library_sig(){
    HMODULE lib = (HMODULE)handle_type(handle);
    return(FreeLibrary(lib));
}

internal
system_get_proc_sig(){
    HMODULE lib = (HMODULE)handle_type(handle);
    return((Void_Func*)(GetProcAddress(lib, proc_name)));
}

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
system_show_mouse_cursor_sig(){
    win32vars.cursor_show = show;
}

internal
system_set_fullscreen_sig(){
    // NOTE(allen): If the new value of full_screen does not match the current value,
    // set toggle to true.
    win32vars.do_toggle = (win32vars.full_screen != full_screen);
    b32 success = true;
    return(success);
}

internal
system_is_fullscreen_sig(){
    // NOTE(allen): Report the fullscreen status as it would be set at the beginning of the
    // next frame. That is, take into account all fullscreen toggle requests that have come in
    // already this frame. Read: "full_screen XOR do_toggle"
    b32 result = (win32vars.full_screen != win32vars.do_toggle);
    return(result);
}

internal
system_get_keyboard_modifiers_sig(){
    return(copy_modifier_set(arena, &win32vars.input_chunk.pers.modifiers));
}

internal
system_set_key_mode_sig(){
    win32vars.key_mode = mode;
}

////////////////////////////////
// NOTE(allen): Clipboard

internal String_Const_u8
win32_read_clipboard_contents(Thread_Context *tctx, Arena *arena){
    Scratch_Block scratch(tctx, arena);
    
    String_Const_u8 result = {};
    
    if (OpenClipboard(win32vars.window_handle)){
        b32 got_result = false;
        if (!got_result){
            HANDLE clip_data = GetClipboardData(CF_UNICODETEXT);
            if (clip_data != 0){
                u16 *clip_16_ptr = (u16*)GlobalLock(clip_data);
                if (clip_16_ptr != 0){
                    String_Const_u16 clip_16 = SCu16(clip_16_ptr);
                    got_result = true;
                    result = string_u8_from_string_u16(arena, clip_16, StringFill_NullTerminate).string;
                }
                GlobalUnlock(clip_data);
            }
        }
        if (!got_result){
            HANDLE clip_data = GetClipboardData(CF_TEXT);
            if (clip_data != 0){
                char *clip_ascii_ptr = (char*)GlobalLock(clip_data);
                if (clip_ascii_ptr != 0){
                    String_Const_char clip_ascii = SCchar(clip_ascii_ptr);
                    got_result = true;
                    result = string_u8_from_string_char(arena, clip_ascii, StringFill_NullTerminate).string;
                }
                GlobalUnlock(clip_data);
            }
        }
        CloseClipboard();
    }
    
    
    return(result);
}

internal void
win32_post_clipboard(Arena *scratch, char *text, i32 len){
    if (OpenClipboard(win32vars.window_handle)){
        if (!EmptyClipboard()){
            String_Const_u8 error_string = win32_get_error_string();
            win32_output_error_string(error_string);
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
system_get_clipboard_sig(){
    String_Const_u8 result = {};
    DWORD new_number = GetClipboardSequenceNumber();
    if (new_number != win32vars.clipboard_sequence){
        win32vars.clipboard_sequence = new_number;
        if (win32vars.next_clipboard_is_self){
            win32vars.next_clipboard_is_self = false;
        }
        else{
            for (i32 R = 0; R < 8; ++R){
                result = win32_read_clipboard_contents(win32vars.tctx, arena);
                if (result.str == 0){
                    break;
                }
            }
        }
    }
    return(result);
}

internal
system_post_clipboard_sig(){
    Arena *arena = &win32vars.clip_post_arena;
    if (arena->base_allocator == 0){
        *arena = make_arena_system();
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
        log_os("Failed to allocate buffer for clipboard post (%d)\n", (i32)str.size + 1);
    }
}

internal
system_set_clipboard_catch_all_sig(){
    win32vars.clip_catch_all = enabled?true:false;
}

internal
system_get_clipboard_catch_all_sig(){
    return(win32vars.clip_catch_all);
}

//
// Command Line Exectuion
//

internal
system_cli_call_sig(){
    Assert(sizeof(Plat_Handle) >= sizeof(HANDLE));
    
    char cmd[] = "c:\\windows\\system32\\cmd.exe";
    char *env_variables = 0;
    
    Temp_Memory temp = begin_temp(scratch);
    String_Const_u8 s = push_u8_stringf(scratch, "/C %s", script);
    
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
system_cli_begin_update_sig(){
    Assert(sizeof(cli->scratch_space) >= sizeof(CLI_Loop_Control));
    CLI_Loop_Control *loop = (CLI_Loop_Control*)cli->scratch_space;
    loop->remaining_amount = 0;
}

internal
system_cli_update_step_sig(){
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
system_cli_end_update_sig(){
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

function void
os_popup_error(char *title, char *message){
    MessageBoxA(0, title, message, MB_OK);
    ExitProcess(1);
}

#include "4ed_font_provider_freetype.h"
#include "4ed_font_provider_freetype.cpp"

#include <GL/gl.h>
#include "opengl/4ed_opengl_defines.h"
#define GL_FUNC(N,R,P) typedef R (CALL_CONVENTION N##_Function)P; N##_Function *N = 0;
#include "opengl/4ed_opengl_funcs.h"
#include "opengl/4ed_opengl_render.cpp"

internal
graphics_get_texture_sig(){
    return(gl__get_texture(dim, texture_kind));
}

internal
graphics_fill_texture_sig(){
    return(gl__fill_texture(texture_kind, texture, p, dim, data));
}

internal
font_make_face_sig(){
    return(ft__font_make_face(arena, description, scale_factor));
}

//
// Helpers
//

global Key_Code keycode_lookup_table[255];

internal void
win32_keycode_init(void){
    for (u32 i = 'A'; i <= 'Z'; i += 1){
        keycode_lookup_table[i] = KeyCode_A + i - 'A';
    }
    for (u32 i = '0'; i <= '9'; i += 1){
        keycode_lookup_table[i] = KeyCode_0 + i - '0';
    }
    
    keycode_lookup_table[VK_SPACE] = KeyCode_Space;
    keycode_lookup_table[VK_OEM_3] = KeyCode_Tick;
    keycode_lookup_table[VK_OEM_MINUS] = KeyCode_Minus;
    keycode_lookup_table[VK_OEM_PLUS] = KeyCode_Equal;
    keycode_lookup_table[VK_OEM_4] = KeyCode_LeftBracket;
    keycode_lookup_table[VK_OEM_6] = KeyCode_RightBracket;
    keycode_lookup_table[VK_OEM_1] = KeyCode_Semicolon;
    keycode_lookup_table[VK_OEM_7] = KeyCode_Quote;
    keycode_lookup_table[VK_OEM_COMMA] = KeyCode_Comma;
    keycode_lookup_table[VK_OEM_PERIOD] = KeyCode_Period;
    keycode_lookup_table[VK_OEM_2] = KeyCode_ForwardSlash;
    keycode_lookup_table[VK_OEM_5] = KeyCode_BackwardSlash;
    
    keycode_lookup_table[VK_TAB] = KeyCode_Tab;
    keycode_lookup_table[VK_PAUSE] = KeyCode_Pause;
    keycode_lookup_table[VK_ESCAPE] = KeyCode_Escape;
    
    keycode_lookup_table[VK_UP] = KeyCode_Up;
    keycode_lookup_table[VK_DOWN] = KeyCode_Down;
    keycode_lookup_table[VK_LEFT] = KeyCode_Left;
    keycode_lookup_table[VK_RIGHT] = KeyCode_Right;
    
    keycode_lookup_table[VK_BACK] = KeyCode_Backspace;
    keycode_lookup_table[VK_RETURN] = KeyCode_Return;
    
    keycode_lookup_table[VK_DELETE] = KeyCode_Delete;
    keycode_lookup_table[VK_INSERT] = KeyCode_Insert;
    keycode_lookup_table[VK_HOME] = KeyCode_Home;
    keycode_lookup_table[VK_END] = KeyCode_End;
    keycode_lookup_table[VK_PRIOR] = KeyCode_PageUp;
    keycode_lookup_table[VK_NEXT] = KeyCode_PageDown;
    
    keycode_lookup_table[VK_CAPITAL] = KeyCode_CapsLock;
    keycode_lookup_table[VK_NUMLOCK] = KeyCode_NumLock;
    keycode_lookup_table[VK_SCROLL] = KeyCode_ScrollLock;
    keycode_lookup_table[VK_APPS] = KeyCode_Menu;
    
    keycode_lookup_table[VK_SHIFT] = KeyCode_Shift;
    keycode_lookup_table[VK_LSHIFT] = KeyCode_Shift;
    keycode_lookup_table[VK_RSHIFT] = KeyCode_Shift;
    
    keycode_lookup_table[VK_CONTROL] = KeyCode_Control;
    keycode_lookup_table[VK_LCONTROL] = KeyCode_Control;
    keycode_lookup_table[VK_RCONTROL] = KeyCode_Control;
    
    keycode_lookup_table[VK_MENU] = KeyCode_Alt;
    keycode_lookup_table[VK_LMENU] = KeyCode_Alt;
    keycode_lookup_table[VK_RMENU] = KeyCode_Alt;
    
    keycode_lookup_table[VK_F1] = KeyCode_F1;
    keycode_lookup_table[VK_F2] = KeyCode_F2;
    keycode_lookup_table[VK_F3] = KeyCode_F3;
    keycode_lookup_table[VK_F4] = KeyCode_F4;
    keycode_lookup_table[VK_F5] = KeyCode_F5;
    keycode_lookup_table[VK_F6] = KeyCode_F6;
    keycode_lookup_table[VK_F7] = KeyCode_F7;
    keycode_lookup_table[VK_F8] = KeyCode_F8;
    
    keycode_lookup_table[VK_F9] = KeyCode_F9;
    keycode_lookup_table[VK_F10] = KeyCode_F10;
    keycode_lookup_table[VK_F11] = KeyCode_F11;
    keycode_lookup_table[VK_F12] = KeyCode_F12;
    keycode_lookup_table[VK_F13] = KeyCode_F13;
    keycode_lookup_table[VK_F14] = KeyCode_F14;
    keycode_lookup_table[VK_F15] = KeyCode_F15;
    keycode_lookup_table[VK_F16] = KeyCode_F16;
    
    keycode_lookup_table[VK_F17] = KeyCode_F17;
    keycode_lookup_table[VK_F18] = KeyCode_F18;
    keycode_lookup_table[VK_F19] = KeyCode_F19;
    keycode_lookup_table[VK_F20] = KeyCode_F20;
    keycode_lookup_table[VK_F21] = KeyCode_F21;
    keycode_lookup_table[VK_F22] = KeyCode_F22;
    keycode_lookup_table[VK_F23] = KeyCode_F23;
    keycode_lookup_table[VK_F24] = KeyCode_F24;
    
    keycode_lookup_table[VK_NUMPAD0] = KeyCode_NumPad0;
    keycode_lookup_table[VK_NUMPAD1] = KeyCode_NumPad1;
    keycode_lookup_table[VK_NUMPAD2] = KeyCode_NumPad2;
    keycode_lookup_table[VK_NUMPAD3] = KeyCode_NumPad3;
    keycode_lookup_table[VK_NUMPAD4] = KeyCode_NumPad4;
    keycode_lookup_table[VK_NUMPAD5] = KeyCode_NumPad5;
    keycode_lookup_table[VK_NUMPAD6] = KeyCode_NumPad6;
    keycode_lookup_table[VK_NUMPAD7] = KeyCode_NumPad7;
    keycode_lookup_table[VK_NUMPAD8] = KeyCode_NumPad8;
    keycode_lookup_table[VK_NUMPAD9] = KeyCode_NumPad9;
    
    for (i32 i = 0xDF; i < 0xFF; i += 1){
        keycode_lookup_table[i] = KeyCode_Ex0 + 1;
    }
}

internal b32
keycode_physical_translaion_is_wrong(u64 vk){
    b32 result = false;
    switch (vk){
        case VK_UP: case VK_DOWN: case VK_LEFT: case VK_RIGHT:
        case VK_DELETE:
        case VK_INSERT:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_NUMPAD0:
        case VK_NUMPAD1:
        case VK_NUMPAD2:
        case VK_NUMPAD3:
        case VK_NUMPAD4:
        case VK_NUMPAD5:
        case VK_NUMPAD6:
        case VK_NUMPAD7:
        case VK_NUMPAD8:
        case VK_NUMPAD9:
        case VK_MULTIPLY:
        case VK_ADD     :
        case VK_SUBTRACT:
        case VK_DECIMAL :
        case VK_DIVIDE  :
        {
            result = true;
        }break;
        
    }
    return(result);
}

internal void
win32_resize(i32 width, i32 height){
    if (width > 0 && height > 0){
        target.width = width;
        target.height = height;
    }
}

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
        Win32_Object *objects = (Win32_Object*)system_memory_allocate(count*sizeof(Win32_Object), file_name_line_number_lit_u8);
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
system_now_time_sig(){
    u64 result = 0;
    LARGE_INTEGER t;
    if (QueryPerformanceCounter(&t)){
        result = (u64)(t.QuadPart*win32vars.usecond_per_count);
    }
    return(result);
}

internal void
date_time_from_win32_system_time(Date_Time *out, SYSTEMTIME *in){
    out->year = in->wYear;
    out->mon = (u8)(in->wMonth - 1);
	out->day = (u8)(in->wDay - 1);
	out->hour = (u8)(in->wHour);
	out->min = (u8)(in->wMinute);
	out->sec = (u8)(in->wSecond);
    out->msec = in->wMilliseconds;
}

internal void
win32_system_time_from_date_time(SYSTEMTIME *out, Date_Time *in){
    out->wYear = (WORD)(in->year);
    out->wMonth = in->mon + 1;
    out->wDay = in->day + 1;
    out->wHour = in->hour;
    out->wMinute = in->min;
    out->wSecond = in->sec;
    out->wMilliseconds = in->msec;
}

internal
system_now_date_time_universal_sig(){
    SYSTEMTIME systime = {};
    GetSystemTime(&systime);
    Date_Time result = {};
    date_time_from_win32_system_time(&result, &systime);
    return(result);
}

internal
system_local_date_time_from_universal_sig(){
    SYSTEMTIME systime = {};
    win32_system_time_from_date_time(&systime, date_time);
    FILETIME ftime = {};
    SystemTimeToFileTime(&systime, &ftime);
    FILETIME ftime_local = {};
    FileTimeToLocalFileTime(&ftime, &ftime_local);
    FileTimeToSystemTime(&ftime_local, &systime);
    Date_Time result = {};
    date_time_from_win32_system_time(&result, &systime);
    return(result);
}

internal
system_universal_date_time_from_local_sig(){
    SYSTEMTIME systime = {};
    win32_system_time_from_date_time(&systime, date_time);
    FILETIME ftime = {};
    SystemTimeToFileTime(&systime, &ftime);
    FILETIME ftime_local = {};
    LocalFileTimeToFileTime(&ftime, &ftime_local);
    FileTimeToSystemTime(&ftime_local, &systime);
    Date_Time result = {};
    date_time_from_win32_system_time(&result, &systime);
    return(result);
}

internal
system_wake_up_timer_create_sig(){
    Win32_Object *object = win32_alloc_object(Win32ObjectKind_Timer);
    dll_insert(&win32vars.timer_objects, &object->node);
    object->timer.id = ++win32vars.timer_counter;
    return(handle_type(object));
}

internal
system_wake_up_timer_release_sig(){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(handle);
    if (object->kind == Win32ObjectKind_Timer){
        KillTimer(win32vars.window_handle, object->timer.id);
        win32_free_object(object);
    }
}

internal
system_wake_up_timer_set_sig(){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(handle);
    if (object->kind == Win32ObjectKind_Timer){
        object->timer.id = SetTimer(win32vars.window_handle, object->timer.id, time_milliseconds, 0);
    }
}

internal
system_signal_step_sig(){
    system_schedule_step(code);
}

internal
system_sleep_sig(){
    u32 milliseconds = (u32)(microseconds/Thousand(1));
    Sleep(milliseconds);
}

////////////////////////////////

internal DWORD CALL_CONVENTION
win32_thread_wrapper(void *ptr){
    Win32_Object *object = (Win32_Object*)ptr;
    Thread_Function *proc = object->thread.proc;
    void *object_ptr = object->thread.ptr;
    EnterCriticalSection(&win32vars.thread_launch_mutex);
    win32vars.waiting_for_launch = false;
    WakeConditionVariable(&win32vars.thread_launch_cv);
    LeaveCriticalSection(&win32vars.thread_launch_mutex);
    proc(object_ptr);
    return(0);
}

internal
system_thread_launch_sig(){
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
system_thread_join_sig(){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(thread);
    if (object->kind == Win32ObjectKind_Thread){
        WaitForSingleObject(object->thread.thread, INFINITE);
    }
}

internal
system_thread_free_sig(){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(thread);
    if (object->kind == Win32ObjectKind_Thread){
        CloseHandle(object->thread.thread);
        win32_free_object(object);
    }
}

internal
system_thread_get_id_sig(){
    DWORD result = GetCurrentThreadId();
    return((i32)result);
}

internal
system_mutex_make_sig(){
    Win32_Object *object = win32_alloc_object(Win32ObjectKind_Mutex);
    InitializeCriticalSection(&object->mutex);
    return(handle_type(object));
}

internal
system_mutex_acquire_sig(){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(mutex);
    if (object->kind == Win32ObjectKind_Mutex){
        EnterCriticalSection(&object->mutex);
    }
}

internal
system_mutex_release_sig(){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(mutex);
    if (object->kind == Win32ObjectKind_Mutex){
        LeaveCriticalSection(&object->mutex);
    }
}

global i32 global_frame_mutex_state_ticker = 0;

internal
system_acquire_global_frame_mutex_sig(){
    if (tctx->kind == ThreadKind_AsyncTasks ||
        tctx->kind == ThreadKind_Main){
        system_mutex_acquire(win32vars.global_frame_mutex);
        Assert(global_frame_mutex_state_ticker == 0);
        global_frame_mutex_state_ticker = 1;
    }
}

internal
system_release_global_frame_mutex_sig(){
    if (tctx->kind == ThreadKind_AsyncTasks ||
        tctx->kind == ThreadKind_Main){
        Assert(global_frame_mutex_state_ticker == 1);
        global_frame_mutex_state_ticker = 0;
        system_mutex_release(win32vars.global_frame_mutex);
    }
}

internal
system_mutex_free_sig(){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(mutex);
    if (object->kind == Win32ObjectKind_Mutex){
        DeleteCriticalSection(&object->mutex);
        win32_free_object(object);
    }
}

internal
system_condition_variable_make_sig(){
    Win32_Object *object = win32_alloc_object(Win32ObjectKind_CV);
    InitializeConditionVariable(&object->cv);
    return(handle_type(object));
}

internal
system_condition_variable_wait_sig(){
    Win32_Object *object_cv = (Win32_Object*)handle_type_ptr(cv);
    Win32_Object *object_mutex = (Win32_Object*)handle_type_ptr(mutex);
    if (object_cv->kind == Win32ObjectKind_CV &&
        object_mutex->kind == Win32ObjectKind_Mutex){
        SleepConditionVariableCS(&object_cv->cv, &object_mutex->mutex, INFINITE);
    }
}

internal
system_condition_variable_signal_sig(){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(cv);
    if (object->kind == Win32ObjectKind_CV){
        WakeConditionVariable(&object->cv);
    }
}

internal
system_condition_variable_free_sig(){
    Win32_Object *object = (Win32_Object*)handle_type_ptr(cv);
    if (object->kind == Win32ObjectKind_CV){
        win32_free_object(object);
    }
}

////////////////////////////////

internal LRESULT CALL_CONVENTION
win32_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;
    Scratch_Block scratch(win32vars.tctx);
    
    switch (uMsg){
        case WM_MENUCHAR:
        {
            result = (MNC_CLOSE << 16);
        }break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            b8 release = HasFlag(lParam, bit_32);
            b8 down = !release;
            b8 is_right = HasFlag(lParam, bit_25);
            
            u64 vk = wParam;
            
            if (win32vars.key_mode == KeyMode_Physical &&
                !keycode_physical_translaion_is_wrong(vk)){
                UINT scan_code = ((lParam >> 16) & bitmask_8);
                vk = MapVirtualKeyEx(scan_code, MAPVK_VSC_TO_VK_EX, win32vars.kl_universal);
            }
            
            Input_Modifier_Set_Fixed *mods = &win32vars.input_chunk.pers.modifiers;
            
            Control_Keys *controls = &win32vars.input_chunk.pers.controls;
            switch (vk){
                case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
                case VK_MENU:case VK_LMENU:case VK_RMENU:
                {
                    switch (vk){
                        case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
                        {
                            if (is_right){
                                controls->r_ctrl = down;
                            }
                            else{
                                controls->l_ctrl = down;
                            }
                        }break;
                        case VK_MENU:case VK_LMENU:case VK_RMENU:
                        {
                            if (is_right){
                                controls->r_alt = down;
                            }
                            else{
                                controls->l_alt = down;
                            }
                        }break;
                    }
                }break;
            }
			
            b8 ctrl = (controls->r_ctrl || (controls->l_ctrl && !controls->r_alt));
            b8 alt = (controls->l_alt || (controls->r_alt && !controls->l_ctrl));
            if (win32vars.lctrl_lalt_is_altgr && controls->l_alt && controls->l_ctrl){
                ctrl = false;
                alt = false;
            }
            set_modifier(mods, KeyCode_Control, ctrl);
            set_modifier(mods, KeyCode_Alt, alt);
            
            {
                b8 shift = ((GetKeyState(VK_SHIFT) & bit_16) != 0);
                set_modifier(mods, KeyCode_Shift, shift);
            }
            
            Key_Code key = keycode_lookup_table[(u8)vk];
            if (down){
                if (key != 0){
                    add_modifier(mods, key);
                    
                    Input_Event *event = push_input_event(&win32vars.frame_arena, &win32vars.input_chunk.trans.event_list);
                    event->kind = InputEventKind_KeyStroke;
                    event->key.code = key;
                    event->key.modifiers = copy_modifier_set(&win32vars.frame_arena, mods);
                    win32vars.active_key_stroke = event;
                    
                    win32vars.got_useful_event = true;
                }
            }
            else{
                win32vars.active_key_stroke = 0;
                win32vars.active_text_input = 0;
                win32vars.got_useful_event = true;
                
                if (key != 0){
                    Input_Event *event = push_input_event(&win32vars.frame_arena, &win32vars.input_chunk.trans.event_list);
                    event->kind = InputEventKind_KeyRelease;
                    event->key.code = key;
                    event->key.modifiers = copy_modifier_set(&win32vars.frame_arena, mods);
                    
                    remove_modifier(mods, key);
                }
            }
        }break;
        
        case WM_CHAR:
        {
            u16 c = wParam & bitmask_16;
            if (c == '\r'){
                c = '\n';
            }
            if (c > 127 || (' ' <= c && c <= '~') || c == '\t' || c == '\n'){
                String_Const_u16 str_16 = SCu16(&c, 1);
                String_Const_u8 str_8 = string_u8_from_string_u16(&win32vars.frame_arena, str_16).string;
                Input_Event *event = push_input_event(&win32vars.frame_arena, &win32vars.input_chunk.trans.event_list);
                event->kind = InputEventKind_TextInsert;
                event->text.string = str_8;
                event->text.next_text = 0;
                event->text.blocked = false;
                if (win32vars.active_text_input != 0){
                    win32vars.active_text_input->text.next_text = event;
                }
                else if (win32vars.active_key_stroke != 0){
                    win32vars.active_key_stroke->key.first_dependent_text = event;
                }
                win32vars.active_text_input = event;
                
                win32vars.got_useful_event = true;
            }
        }break;
        
        case WM_DEADCHAR:
        {
            if (win32vars.active_key_stroke != 0){
                AddFlag(win32vars.active_key_stroke->key.flags, KeyFlag_IsDeadKey);
            }
        }break;
        
        case WM_UNICHAR:
        {
            if (wParam == UNICODE_NOCHAR){
                result = true;
            }
            else{
                u32 c = (u32)wParam;
                if (c == '\r'){
                    c= '\n';
                }
                if (c > 127 || (' ' <= c && c <= '~') || c == '\t' || c == '\n'){
                    String_Const_u32 str_32 = SCu32(&c, 1);
                    String_Const_u8 str_8 = string_u8_from_string_u32(&win32vars.frame_arena, str_32).string;
                    Input_Event event = {};
                    event.kind = InputEventKind_TextInsert;
                    event.text.string = str_8;
                    push_input_event(&win32vars.frame_arena, &win32vars.input_chunk.trans.event_list, &event);
                    win32vars.got_useful_event = true;
                }
            }
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
            SetCapture(hwnd);
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.mouse_l_press = true;
            win32vars.input_chunk.pers.mouse_l = true;
        }break;
        
        case WM_RBUTTONDOWN:
        {
            SetCapture(hwnd);
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.mouse_r_press = true;
            win32vars.input_chunk.pers.mouse_r = true;
        }break;
        
        case WM_LBUTTONUP:
        {
            ReleaseCapture();
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.mouse_l_release = true;
            win32vars.input_chunk.pers.mouse_l = false;
        }break;
        
        case WM_RBUTTONUP:
        {
            ReleaseCapture();
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.mouse_r_release = true;
            win32vars.input_chunk.pers.mouse_r = false;
        }break;
        
        case WM_KILLFOCUS:
        case WM_SETFOCUS:
        {
            ReleaseCapture();
            win32vars.got_useful_event = true;
            win32vars.input_chunk.pers.mouse_l = false;
            win32vars.input_chunk.pers.mouse_r = false;
            block_zero_struct(&win32vars.input_chunk.pers.controls);
            block_zero_struct(&win32vars.input_chunk.pers.modifiers);
            win32vars.active_key_stroke = 0;
            win32vars.active_text_input = 0;
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
            if (win32vars.clip_catch_all){
                win32vars.got_useful_event = true;
                LogEventLit(win32vars.log_string(M), scratch, 0, 0, system_thread_get_id(),
                            "new clipboard contents");
            }
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

typedef HGLRC (CALL_CONVENTION wglCreateContextAttribsARB_Function)(HDC,HGLRC,i32*);
typedef BOOL  (CALL_CONVENTION wglChoosePixelFormatARB_Function)(HDC,i32*,f32*,u32,i32*,u32*);
typedef char* (CALL_CONVENTION wglGetExtensionsStringEXT_Function)();
typedef VOID  (CALL_CONVENTION wglSwapIntervalEXT_Function)(i32);

global wglCreateContextAttribsARB_Function *wglCreateContextAttribsARB = 0;
global wglChoosePixelFormatARB_Function *wglChoosePixelFormatARB = 0;
global wglGetExtensionsStringEXT_Function *wglGetExtensionsStringEXT = 0;
global wglSwapIntervalEXT_Function *wglSwapIntervalEXT = 0;

internal b32
win32_gl_create_window(HWND *wnd_out, HGLRC *context_out, DWORD style, RECT rect){
    HINSTANCE this_instance = GetModuleHandle(0);
    
    local_persist b32 srgb_support = false;
    local_persist b32 register_success = true;
    local_persist b32 first_call = true;
    if (first_call){
        log_os(" GL bootstrapping...\n");
        
        first_call = false;
        
        // NOTE(allen): Create the GL bootstrap window
        log_os(" registering bootstrap class...\n");
        WNDCLASSW wglclass = {};
        wglclass.lpfnWndProc = DefWindowProcW;
        wglclass.hInstance = this_instance;
        wglclass.lpszClassName = L"wgl-loader";
        if (RegisterClassW(&wglclass) == 0){
            register_success = false;
            goto fail_register;
        }
        
        log_os(" creating bootstrap window...\n");
        HWND wglwindow = CreateWindowW(wglclass.lpszClassName, L"", 0, 0, 0, 0, 0,
                                       0, 0, this_instance, 0);
        if (wglwindow == 0){
            register_success = false;
            goto fail_register;
        }
        
        // NOTE(allen): Create the GL bootstrap context
        log_os(" setting bootstrap pixel format...\n");
        
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
        
        log_os(" creating bootstrap GL context...\n");
        HGLRC wglcontext = wglCreateContext(wgldc);
        if (wglcontext == 0){
            register_success = false;
            goto fail_register;
        }
        
        log_os(" making bootstrap GL context current...\n");
        if (!wglMakeCurrent(wgldc, wglcontext)){
            register_success = false;
            goto fail_register;
        }
        
        // NOTE(allen): Load wgl extensions
        log_os(" loading wgl extensions...\n");
        
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
        
        log_os(" checking wgl extensions...\n");
        char *extensions_c = wglGetExtensionsStringEXT();
        String_Const_u8 extensions = SCu8((u8*)extensions_c);
        
        {
            String_Const_u8 s = string_skip_whitespace(extensions);
            for (;s.size > 0;){
                u64 end = string_find_first_whitespace(s);
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
        log_os(" loading core GL functions...\n");
        
#define GL_FUNC(f,R,P) LoadWGL(f,load_success);
#include "opengl/4ed_opengl_funcs.h"
        
        if (!load_success){
            register_success = false;
            goto fail_register;
        }
        
        // NOTE(allen): Cleanup the GL bootstrap resources
        log_os(" cleaning up boostrap resources...\n");
        
        ReleaseDC(wglwindow, wgldc);
        DestroyWindow(wglwindow);
        wglDeleteContext(wglcontext);
        
        // NOTE(allen): Register the graphics window class
        log_os(" registering graphics class...\n");
        
        WNDCLASSW wndclass = {};
        wndclass.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
        wndclass.lpfnWndProc = win32_proc;
        wndclass.hIcon = LoadIconW(GetModuleHandle(0), L"main");
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
        log_os(" creating graphics window...\n");
        
        HWND wnd = CreateWindowExW(0, L"GRAPHICS-WINDOW-NAME", L"GRAPHICS", style,
                                   CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
                                   0, 0, this_instance, 0);
        
        *wnd_out = 0;
        *context_out = 0;
        if (wnd != 0){
            log_os(" setting graphics pixel format...\n");
            
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
            
            log_os(" setting graphics attributes...\n");
            
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
            
            log_os(" creating graphics GL context...\n");
            HGLRC context = wglCreateContextAttribsARB(dc, 0, context_attrib_list);
            if (context == 0){
                goto fail_window_init;
            }
            
            log_os(" making graphics GL context current...\n");
            wglMakeCurrent(dc, context);
            
            
            if (wglSwapIntervalEXT != 0){
                log_os(" setting swap interval...\n");
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
            else{
                ReleaseDC(wnd, dc);
            }
        }
    }
    
    return(result);
}

////////////////////////////////

int CALL_CONVENTION
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    i32 argc = __argc;
    char **argv = __argv;
    
    // NOTE(allen): someone get my shit togeth :(er for me
    
    for (i32 i = 0; i < argc; i += 1){
        String_Const_u8 arg = SCu8(argv[i]);
        printf("arg[%d]: %.*s\n", i, string_expand(arg));
        if (string_match(arg, str8_lit("-L"))){
            log_os_enabled = true;
        }
    }
    
    log_os("Logging startup...\n");
    
    log_os("Initializing thread context...\n");
    
    // NOTE(allen): This thing
    InitializeCriticalSection(&memory_tracker_mutex);
    
    // NOTE(allen): context setup
    Thread_Context _tctx = {};
    thread_ctx_init(&_tctx, ThreadKind_Main, get_base_allocator_system(), get_base_allocator_system());
    
    block_zero_struct(&win32vars);
    win32vars.tctx = &_tctx;
    
    log_os("Filling API v-tables...\n");
    
    API_VTable_system system_vtable = {};
    system_api_fill_vtable(&system_vtable);
    
    API_VTable_graphics graphics_vtable = {};
    graphics_api_fill_vtable(&graphics_vtable);
    
    API_VTable_font font_vtable = {};
    font_api_fill_vtable(&font_vtable);
    
    log_os("Setting up memory management...\n");
    
    // NOTE(allen): memory
    win32vars.frame_arena = make_arena_system();
    // TODO(allen): *arena;
    target.arena = make_arena_system(KB(256));
    
    win32vars.cursor_show = MouseCursorShow_Always;
    win32vars.prev_cursor_show = MouseCursorShow_Always;
    
    log_os("Setting up threading primitives...\n");
    
    dll_init_sentinel(&win32vars.free_win32_objects);
    dll_init_sentinel(&win32vars.timer_objects);
    
    InitializeCriticalSection(&win32vars.thread_launch_mutex);
    InitializeConditionVariable(&win32vars.thread_launch_cv);
    
    log_os("Setting up DPI awareness...\n");
    
    SetProcessDPIAware();
    
    {
        HDC dc = GetDC(0);
        i32 x_dpi = GetDeviceCaps(dc, LOGPIXELSX);
        i32 y_dpi = GetDeviceCaps(dc, LOGPIXELSY);
        i32 max_dpi = max(x_dpi, y_dpi);
        win32vars.screen_scale_factor = ((f32)max_dpi)/96.f;
        ReleaseDC(0, dc);
        log_os(" detected dpi %f\n", win32vars.screen_scale_factor);
    }
    
    // NOTE(allen): load core
    log_os("Loading 4ed core...\n");
    
    System_Library core_library = {};
    App_Functions app = {};
    {
        App_Get_Functions *get_funcs = 0;
        Scratch_Block scratch(win32vars.tctx);
        
        List_String_Const_u8 search_list = {};
        def_search_list_add_system_path(scratch, &search_list, SystemPath_Binary);
        
        String_Const_u8 core_path =
            def_search_get_full_path(scratch, &search_list, SCu8("4ed_app.dll"));
        
        log_os(" path to core: '%.*s'\n", string_expand(core_path));
        
        if (system_load_library(scratch, core_path, &core_library)){
            get_funcs = (App_Get_Functions*)system_get_proc(core_library, "app_get_functions");
            if (get_funcs != 0){
                app = get_funcs();
            }
            else{
                char msg[] = "Failed to get application code from '4ed_app.dll'.";
                system_error_box(msg);
            }
        }
        else{
            char msg[] = "Could not load '4ed_app.dll'. This file should be in the same directory as the main '4ed' executable.";
            system_error_box(msg);
        }
    }
    
    log_os(" core loaded\n");
    
    // NOTE(allen): send system vtable to core
    log_os("Linking vtables...\n");
    
    app.load_vtables(&system_vtable, &font_vtable, &graphics_vtable);
    win32vars.log_string = app.get_logger();
    
    // NOTE(allen): init & command line parameters
    log_os("Parsing command line...\n");
    
    Plat_Settings plat_settings = {};
    void *base_ptr = 0;
    {
        Scratch_Block scratch(win32vars.tctx);
        String_Const_u8 curdir = system_get_path(scratch, SystemPath_CurrentDirectory);
        curdir = string_mod_replace_character(curdir, '\\', '/');
        char **files = 0;
        i32 *file_count = 0;
        base_ptr = app.read_command_line(win32vars.tctx, curdir, &plat_settings, &files, &file_count, argc, argv);
        {
            i32 end = *file_count;
            i32 i = 0, j = 0;
            for (; i < end; ++i){
                if (system_file_can_be_made(scratch, (u8*)files[i])){
                    files[j] = files[i];
                    ++j;
                }
            }
            *file_count = j;
        }
    }
    
    // NOTE(allen): setup user directory override
    log_os("User directory override: '%s'\n", plat_settings.user_directory);
    
    if (plat_settings.user_directory != 0){
        w32_override_user_directory = SCu8((u8*)plat_settings.user_directory);
    }
    
    // NOTE(allen): load custom layer
    log_os("Loading custom layer...\n");
    
    System_Library custom_library = {};
    Custom_API custom = {};
    {
        char custom_not_found_msg[] = "Did not find a library for the custom layer.";
        char custom_fail_version_msg[] = "Failed to load custom code due to missing version information or a version mismatch.  Try rebuilding with buildsuper.";
        char custom_fail_init_apis[] = "Failed to load custom code due to missing 'init_apis' symbol.  Try rebuilding with buildsuper";
        
        Scratch_Block scratch(win32vars.tctx);
        String_Const_u8 default_file_name = string_u8_litexpr("custom_4coder.dll");
        List_String_Const_u8 search_list = {};
        def_search_list_add_system_path(scratch, &search_list, SystemPath_UserDirectory);
        def_search_list_add_system_path(scratch, &search_list, SystemPath_Binary);
        
        if (log_os_enabled){
            log_os(" search list (paths):");
            for (Node_String_Const_u8 *node = search_list.first;
                 node != 0;
                 node = node->next){
                log_os("'%.*s'", string_expand(node->string));
                if (node->next != 0){
                    log_os(", ");
                }
            }
            log_os("\n");
        }
        
        String_Const_u8 custom_file_names[2] = {};
        i32 custom_file_count = 1;
        if (plat_settings.custom_dll != 0){
            custom_file_names[0] = SCu8(plat_settings.custom_dll);
            if (!plat_settings.custom_dll_is_strict){
                custom_file_names[1] = default_file_name;
                custom_file_count += 1;
            }
        }
        else{
            custom_file_names[0] = default_file_name;
        }
        
        log_os(" search list (file names): '%.*s', '%.*s'\n",
               string_expand(custom_file_names[0]), string_expand(custom_file_names[1]));
        
        String_Const_u8 custom_file_name = {};
        for (i32 i = 0; i < custom_file_count; i += 1){
            custom_file_name = def_search_get_full_path(scratch, &search_list, custom_file_names[i]);
            if (custom_file_name.size > 0){
                break;
            }
        }
        
        log_os(" trying to load: '%.*s'\n", string_expand(custom_file_name));
        
        b32 has_library = false;
        if (custom_file_name.size > 0){
            if (system_load_library(scratch, custom_file_name, &custom_library)){
                has_library = true;
            }
        }
        
        if (!has_library){
            system_error_box(custom_not_found_msg);
        }
        custom.get_version = (_Get_Version_Type*)system_get_proc(custom_library, "get_version");
        if (custom.get_version == 0 || custom.get_version(MAJOR, MINOR, PATCH) == 0){
            system_error_box(custom_fail_version_msg);
        }
        custom.init_apis = (_Init_APIs_Type*)system_get_proc(custom_library, "init_apis");
        if (custom.init_apis == 0){
            system_error_box(custom_fail_init_apis);
        }
    }
    
    log_os(" loaded successfully\n");
    
    // NOTE(allen): Window Init
    log_os("Initializing graphical window...\n");
    
    log_os(" getting initial settings...\n");
    RECT window_rect = {};
    if (plat_settings.set_window_size){
        window_rect.right = plat_settings.window_w;
        window_rect.bottom = plat_settings.window_h;
    }
    else{
        window_rect.right = 800;
        window_rect.bottom = 600;
    }
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, false);
    i32 window_style = WS_OVERLAPPEDWINDOW;
    if (!plat_settings.fullscreen_window && plat_settings.maximize_window){
        window_style |= WS_MAXIMIZE;
    }
    log_os(" windowed dimensions: %d, %d\n"
           " initially maximized: %d",
           window_rect.right - window_rect.left,
           window_rect.bottom - window_rect.top,
           ((window_style & WS_MAXIMIZE) != 0));
    
    HGLRC window_opengl_context = 0;
    if (!win32_gl_create_window(&win32vars.window_handle, &window_opengl_context, window_style, window_rect)){
        exit(1);
    }
    
    log_os(" window created successfully\n");
    
    GetClientRect(win32vars.window_handle, &window_rect);
    win32_resize(window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
    
    // NOTE(allen): Audio Init
    log_os("Initializing audio...\n");
    win32vars.audio_thread_id = win32_audio_init();
    
    // NOTE(allen): Misc Init
    log_os("Initializing clipboard listener...\n");
    if (!AddClipboardFormatListener(win32vars.window_handle)){
        String_Const_u8 error_string = win32_get_error_string();
        win32_output_error_string(error_string);
    }
    win32vars.clip_wakeup_timer = system_wake_up_timer_create();
    win32vars.clipboard_sequence = 0;
    win32vars.next_clipboard_is_self = 0;
#if 0
    if (win32vars.clipboard_sequence == 0){
        Scratch_Block scratch(win32vars.tctx);
        win32_post_clipboard(scratch, "", 0);
        win32vars.clipboard_sequence = GetClipboardSequenceNumber();
        win32vars.next_clipboard_is_self = 0;
        if (win32vars.clipboard_sequence == 0){
            log_os(" failure\n");
        }
        else{
            log_os(" got first sequence number\n");
        }
    }
    else{
        log_os(" no initial sequence number\n");
    }
#endif
    
    log_os("Setting up keyboard layout...\n");
    win32vars.kl_universal = LoadKeyboardLayoutW(L"00000409", 0);
    win32_keycode_init();
    
    log_os("Loading cursors...\n");
    win32vars.cursor_ibeam = LoadCursor(NULL, IDC_IBEAM);
    win32vars.cursor_arrow = LoadCursor(NULL, IDC_ARROW);
    win32vars.cursor_leftright = LoadCursor(NULL, IDC_SIZEWE);
    win32vars.cursor_updown = LoadCursor(NULL, IDC_SIZENS);
    
    log_os("Initializing performance counter...\n");
    LARGE_INTEGER f;
    if (QueryPerformanceFrequency(&f)){
        win32vars.usecond_per_count = 1000000.f/(f32)f.QuadPart;
    }
    else{
        // NOTE(allen): Just guess.
        win32vars.usecond_per_count = 1.f;
        log_os(" load failed, guessing usecond_per_count = 1\n");
    }
    if (win32vars.usecond_per_count <= 0.f){
        win32vars.usecond_per_count = 1.f;
    }
    
    //
    // App init
    //
    
    log_os("Initializing 4ed core...\n");
    {
        Scratch_Block scratch(win32vars.tctx);
        String_Const_u8 curdir = system_get_path(scratch, SystemPath_CurrentDirectory);
        curdir = string_mod_replace_character(curdir, '\\', '/');
        app.init(win32vars.tctx, &target, base_ptr, curdir, custom);
    }
    
    //
    // Main loop
    //
    
    log_os("Starting main loop...\n");
    
    b32 keep_running = true;
    win32vars.first = true;
    timeBeginPeriod(1);
    
    if (plat_settings.fullscreen_window){
        win32_toggle_fullscreen();
    }
    
    SetForegroundWindow(win32vars.window_handle);
    SetActiveWindow(win32vars.window_handle);
    ShowWindow(win32vars.window_handle, SW_SHOW);
    
    win32vars.global_frame_mutex = system_mutex_make();
    system_acquire_global_frame_mutex(win32vars.tctx);
    
    u64 timer_start = system_now_time();
    MSG msg;
    for (;keep_running;){
        linalloc_clear(&win32vars.frame_arena);
        block_zero_struct(&win32vars.input_chunk.trans);
        win32vars.active_key_stroke = 0;
        win32vars.active_text_input = 0;
        
        // TODO(allen): Find a good way to wait on a pipe
        // without interfering with the reading process.
        // NOTE(allen): Looks like we can ReadFile with a
        // size of zero in an IOCP for this effect.
        if (!win32vars.first){
            if (win32vars.running_cli == 0){
                win32vars.got_useful_event = false;
            }
            
            // NOTE(allen): while we're doing this (and possibly sleeping)
            // we can let async processes get there time in.
			system_release_global_frame_mutex(win32vars.tctx);
            
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
                    }
                    else{
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
            
			system_acquire_global_frame_mutex(win32vars.tctx);
        }
        
        // NOTE(allen): Mouse Out of Window Detection
        POINT mouse_point;
        if (GetCursorPos(&mouse_point) &&
            ScreenToClient(win32vars.window_handle, &mouse_point)){
            Rect_i32 screen = Ri32(0, 0, target.width, target.height);
            Vec2_i32 mp = V2i32(mouse_point.x, mouse_point.y);
            win32vars.input_chunk.trans.out_of_window = (!rect_contains_point(screen, mp));
            win32vars.input_chunk.pers.mouse = mp;
        }
        else{
            win32vars.input_chunk.trans.out_of_window = true;
        }
        
        // NOTE(allen): Prepare the Frame Input
        
        // TODO(allen): CROSS REFERENCE WITH LINUX SPECIAL CODE "TIC898989"
        Win32_Input_Chunk input_chunk = win32vars.input_chunk;
        
        Scratch_Block scratch(win32vars.tctx);
        Application_Step_Input input = {};
        
        input.first_step = win32vars.first;
        input.dt = frame_useconds/1000000.f;
        input.events = input_chunk.trans.event_list;
        
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
        if (win32vars.clip_catch_all){
            input.clipboard = system_get_clipboard(scratch, 0);
        }
        
        win32vars.clip_post.size = 0;
        
        
        // NOTE(allen): Application Core Update
        Application_Step_Result result = app.step(win32vars.tctx, &target, base_ptr, &input);
        
        // NOTE(allen): Finish the Loop
        if (result.perform_kill){
            keep_running = false;
        }
        
        // NOTE(allen): Post New Clipboard Content
        if (win32vars.clip_post.size > 0){
            win32_post_clipboard(scratch, (char*)win32vars.clip_post.str, (i32)win32vars.clip_post.size);
        }
        
        // NOTE(allen): Switch to New Title
        if (result.has_new_title){
            SetWindowText_utf8(scratch, win32vars.window_handle, (u8*)result.title_string);
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
        gl_render(&target);
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
        else if (win32vars.clip_catch_all){
            system_wake_up_timer_set(win32vars.clip_wakeup_timer, 250);
        }
        
        // NOTE(allen): sleep a bit to cool off :)
		system_release_global_frame_mutex(win32vars.tctx);
        
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
        
        system_acquire_global_frame_mutex(win32vars.tctx);
        
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

