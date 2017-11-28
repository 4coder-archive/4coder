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
#include "4coder_os_comp_cracking.h"

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
# include "4coder_generated/keycodes.h"
# include "4coder_generated/style.h"

# define FSTRING_IMPLEMENTATION
# include "4coder_lib/4coder_string.h"
# include "4coder_lib/4coder_mem.h"

# include "4coder_API/types.h"
# include "4ed_os_custom_api.h"

#else
# include "4coder_default_bindings.cpp"
#endif

#include "4ed_math.h"

#include "4ed_font.h"
#include "4ed_system.h"
#include "4ed_log.h"
#include "4ed_render_target.h"
#include "4ed_render_format.h"
#include "4ed.h"
#include "4ed_linked_node_macros.h"

#include <Windows.h>
#include "win32_gl.h"

#define GL_TEXTURE_MAX_LEVEL 0x813D

//////////////////////////////

internal void
win32_output_error_string(b32 use_error_box = true);

//////////////////////////////

#include "win32_utf8.h"

#include "4ed_file_track.h"
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
#include "4ed_font.cpp"

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
    i32 cursor_show;
    i32 prev_cursor_show;
    
    u8 *clip_buffer;
    u32 clip_max;
    String clipboard_contents;
    b32 next_clipboard_is_self;
    DWORD clipboard_sequence;
    
    Partition clip_post_part;
    i32 clip_post_len;
    
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

internal void
win32_output_error_string(b32 use_error_box){
    DWORD error = GetLastError();
    
    char *str = 0;
    char *str_ptr = (char*)&str;
    if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, str_ptr, 0, 0)){
        LOGF("win32 error:\n%s\n", str);
        if (use_error_box){
            system_error_box(str, false);
        }
    }
    else{
        LOGF("win32 error raw: %d\n", error);
    }
}

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

internal
Sys_Send_Exit_Signal_Sig(system_send_exit_signal){
    win32vars.send_exit_signal = true;
}

#include "4ed_coroutine_functions.cpp"

#include "4ed_system_shared.cpp"

//
// Clipboard
//

internal void
win32_post_clipboard(char *text, i32 len){
    if (OpenClipboard(win32vars.window_handle)){
        if (!EmptyClipboard()){
            win32_output_error_string(false);
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
    Partition *part = &win32vars.clip_post_part;
    part->pos = 0;
    u8 *post = (u8*)sysshared_push_block(part, str.size + 1);
    if (post != 0){
        memcpy(post, str.str, str.size);
        post[str.size] = 0;
        win32vars.clip_post_len = str.size;
    }
    else{
        LOGF("Failed to allocate buffer for clipboard post (%d)\n", str.size + 1);
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
                    
                    b32 error = false;
                    u32 clip_8_len = (u32)utf16_to_utf8_minimal_checking(win32vars.clip_buffer, win32vars.clip_max-1, clip_16, clip_16_len, &error);
                    
                    for (;clip_8_len >= win32vars.clip_max && !error;){
                        system_memory_free(win32vars.clip_buffer, win32vars.clip_max);
                        win32vars.clip_max = l_round_up_u32(clip_8_len, KB(4));
                        win32vars.clip_buffer = (u8*)system_memory_allocate(win32vars.clip_max);
                        
                        clip_8_len = (u32)utf16_to_utf8_minimal_checking(win32vars.clip_buffer, win32vars.clip_max - 1, clip_16, clip_16_len, &error);
                    }
                    
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
Sys_CLI_Call_Sig(system_cli_call, path, script_name, cli_out){
    Assert(sizeof(Plat_Handle) >= sizeof(HANDLE));
    
    char cmd[] = "c:\\windows\\system32\\cmd.exe";
    char *env_variables = 0;
    char command_line[2048];
    
    String s = make_fixed_width_string(command_line);
    copy_ss(&s, make_lit_string("/C "));
    append_partial_sc(&s, script_name);
    b32 success = terminate_with_null(&s);
    
    if (success){
        success = false;
        
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
global u32 system_font_method = SystemFontMethod_RawData;
#include "4ed_font_provider_freetype.cpp"

Sys_Font_Path_Not_Used;

internal
Sys_Font_Data(name, parameters){
    Font_Raw_Data data = {0};
    
    int weight = FW_REGULAR;
    if (parameters->bold){
        weight = FW_BOLD;
    }
    
    HFONT hfont = CreateFontA(
        0,
        0,
        0,
        0,
        weight, // Weight
        parameters->italics, // Italic
        FALSE, // Underline
        FALSE, // Strikeout
        ANSI_CHARSET,
        OUT_DEVICE_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        name
        );
    
    if (hfont != 0){
        HDC hdc = CreateCompatibleDC(NULL);
        if (hdc != 0){
            SelectObject(hdc, hfont);
            DWORD size = GetFontData(hdc, 0, 0, NULL, 0);
            if (size > 0){
                Partition *part = &shared_vars.font_scratch;
                data.temp = begin_temp_memory(part);
                u8 *buffer = push_array(part, u8, size);
                if (buffer == 0){
                    sysshared_partition_grow(part, l_round_up_i32(size, KB(4)));
                    buffer = push_array(part, u8, size);
                }
                
                if (buffer != 0){
                    push_align(part, 8);
                    if (GetFontData(hdc, 0, 0, buffer, size) == size){
                        data.data = buffer;
                        data.size = size;
                    }
                }
            }
            DeleteDC(hdc);
        }
    }
    
    return(data);
}

struct Win32_Font_Enum{
    Partition *part;
    Font_Setup_List *list;
};

int CALL_CONVENTION
win32_font_enum_callback(
const LOGFONT    *lpelfe,
const TEXTMETRIC *lpntme,
DWORD      FontType,
LPARAM     lParam
){
    
    if ((FontType & TRUETYPE_FONTTYPE) != 0){
        ENUMLOGFONTEXDV *log_font = (ENUMLOGFONTEXDV*)lpelfe;
        TCHAR *name = ((log_font)->elfEnumLogfontEx).elfLogFont.lfFaceName;
        
        if ((char)name[0] == '@'){
            return(1);
        }
        
        i32 len = 0;
        for (;name[len]!=0;++len);
        
        if (len >= sizeof(((Font_Loadable_Stub*)0)->name)){
            return(1);
        }
        
        Win32_Font_Enum p = *(Win32_Font_Enum*)lParam;
        
        Temp_Memory reset = begin_temp_memory(p.part);
        Font_Setup *setup = push_array(p.part, Font_Setup, 1);
        if (setup != 0){
            memset(setup, 0, sizeof(*setup));
            
            b32 good = true;
            for (i32 i = 0; i < len; ++i){
                if (name[i] >= 128){
                    good = false;
                    break;
                }
                setup->stub.name[i] = (char)name[i];
            }
            
            if (good){
                setup->stub.load_from_path = false;
                setup->stub.len = len;
                sll_push(p.list->first, p.list->last, setup);
            }
            else{
                end_temp_memory(reset);
            }
        }
    }
    
    return(1);
}

internal void
win32_get_loadable_fonts(Partition *part, Font_Setup_List *list){
    HDC hdc= GetDC(0);
    
    LOGFONT log_font = {0};
    log_font.lfCharSet = ANSI_CHARSET;
    log_font.lfFaceName[0] = 0;
    
    Win32_Font_Enum p = {0};
    p.part = part;
    p.list = list;
    
    int result = EnumFontFamiliesEx(hdc, &log_font, win32_font_enum_callback, (LPARAM)&p,0);
    AllowLocal(result);
    
    ReleaseDC(0, hdc);
}

#include <GL/gl.h>
#include "opengl/4ed_opengl_render.cpp"

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
Win32Resize(i32 width, i32 height){
    if (width > 0 && height > 0){
        target.width = width;
        target.height = height;
    }
}

#define GLFuncGood(f) (((f)!=0)&&((f)!=(void*)1)&&((f)!=(void*)2)&&((f)!=(void*)3)&&((f)!=(void*)-11))

typedef HGLRC (CALL_CONVENTION wglCreateContextAttribsARB_Function)(HDC,HGLRC,i32*);
typedef BOOL  (CALL_CONVENTION wglChoosePixelFormatARB_Function)(HDC,i32*,f32*,u32,i32*,u32*);
typedef char* (CALL_CONVENTION wglGetExtensionsStringEXT_Function)();
typedef VOID  (CALL_CONVENTION wglSwapIntervalEXT_Function)(i32);

global wglCreateContextAttribsARB_Function *wglCreateContextAttribsARB = 0;
global wglChoosePixelFormatARB_Function *wglChoosePixelFormatARB = 0;
global wglGetExtensionsStringEXT_Function *wglGetExtensionsStringEXT = 0;
global wglSwapIntervalEXT_Function *wglSwapIntervalEXT = 0;

internal void
win32_init_gl(HDC hdc){
    LOG("trying to load wgl extensions...\n");
    
#define GLInitFail(s) system_error_box(FNLN "\nOpenGL init fail - " s )
    
    // Init First Context
    WNDCLASSA wglclass = {0};
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
    
    PIXELFORMATDESCRIPTOR format = {0};
    format.nSize = sizeof(format);
    format.nVersion = 1;
    format.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    format.iPixelType = PFD_TYPE_RGBA;
    format.cColorBits = 32;
    format.cAlphaBits = 8;
    format.cDepthBits = 24;
    format.iLayerType = PFD_MAIN_PLANE;
    i32 suggested_format_index = ChoosePixelFormat(hwgldc, &format);
    if (suggested_format_index == 0){
        win32_output_error_string();
        GLInitFail("ChoosePixelFormat");
    }
    
    DescribePixelFormat(hwgldc, suggested_format_index, sizeof(format), &format);
    if (!SetPixelFormat(hwgldc, suggested_format_index, &format)){
        win32_output_error_string();
        GLInitFail("SetPixelFormat");
    }
    
    HGLRC wglcontext = wglCreateContext(hwgldc);
    if (wglcontext == 0){
        win32_output_error_string();
        GLInitFail("wglCreateContext");
    }
    
    if (!wglMakeCurrent(hwgldc, wglcontext)){
        win32_output_error_string();
        GLInitFail("wglMakeCurrent");
    }
    
    // Load wgl Extensions
#define LoadWGL(f, s) f = (f##_Function*)wglGetProcAddress(#f); b32 got_##f = GLFuncGood(f); \
    if (!got_##f) { if (s) { GLInitFail(#f " missing"); } else { f = 0; } }
    LoadWGL(wglCreateContextAttribsARB, true);
    LoadWGL(wglChoosePixelFormatARB, true);
    LoadWGL(wglGetExtensionsStringEXT, true);
    
    LOG("got wgl functions\n");
    
    char *extensions_c = wglGetExtensionsStringEXT();
    String extensions = make_string_slowly(extensions_c);
    if (has_substr(extensions, make_lit_string("WGL_EXT_swap_interval"))){
        LoadWGL(wglSwapIntervalEXT, false);
        if (wglSwapIntervalEXT != 0){
            LOG("got wglSwapIntervalEXT\n");
        }
    }
    
    // Init the Second Context
    int pixel_attrib_list[] = {
        WGL_DRAW_TO_WINDOW_ARB, TRUE,
        WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
        WGL_SUPPORT_OPENGL_ARB, TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
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
        LOGF("setting swap interval %d\n", 1);
        wglSwapIntervalEXT(1);
    }
    
    ReleaseDC(hwglwnd, hwgldc);
    DestroyWindow(hwglwnd);
    wglDeleteContext(wglcontext);
    
    LOG("successfully enabled opengl\n");
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
win32_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
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
            // NOTE(allen): Do nothing?
            AllowLocal(hdc);
            EndPaint(hwnd, &ps);
        }break;
        
        case WM_CLIPBOARDUPDATE:
        {
            win32vars.got_useful_event = true;
        }break;
        
        case WM_CLOSE:
        case WM_DESTROY:
        {
            win32vars.got_useful_event = true;
            win32vars.input_chunk.trans.trying_to_kill = true;
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

#include "4ed_link_system_functions.cpp"
#include "4ed_shared_init_logic.cpp"

int CALL_CONVENTION
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
    i32 argc = __argc;
    char **argv = __argv;
    
    //
    // System Linkage
    //
    
    link_system_code();
    
    //
    // Memory init
    //
    
    memset(&win32vars, 0, sizeof(win32vars));
    memset(&target, 0, sizeof(target));
    memset(&memory_vars, 0, sizeof(memory_vars));
    memset(&plat_settings, 0, sizeof(plat_settings));
    
    memset(&libraries, 0, sizeof(libraries));
    memset(&app, 0, sizeof(app));
    memset(&custom_api, 0, sizeof(custom_api));
    
    memory_init();
    
    win32vars.cursor_show = MouseCursorShow_Always;
    win32vars.prev_cursor_show = MouseCursorShow_Always;
    
    //
    // HACK(allen): 
    // Previously zipped stuff is here, it should be zipped in the new pattern now.
    //
    
    init_shared_vars();
    
    //
    // Load Core Code
    //
    load_app_code();
    
    //
    // Read Command Line
    //
    read_command_line(argc, argv);
    
    //
    // Load Custom Code
    //
    
#if defined(FRED_SUPER)
    load_custom_code();
#else
    custom_api.get_bindings = get_bindings;
#endif
    
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
    window_class.lpfnWndProc = (WNDPROC)(win32_proc);
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
    i32 window_style = WS_OVERLAPPEDWINDOW;
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
        
        win32_init_gl(hdc);
        
        ReleaseDC(win32vars.window_handle, hdc);
    }
    
    Win32Resize(window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
    
    //
    // Font System Init
    //
    
    LOG("Initializing fonts\n");
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    Font_Setup_List font_setup = system_font_get_local_stubs(scratch);
    win32_get_loadable_fonts(scratch, &font_setup);
    system_font_init(&sysfunc.font, plat_settings.font_size, plat_settings.use_hinting, font_setup);
    end_temp_memory(temp);
    
    //
    // Misc System Initializations
    //
    
    LOG("Initializing clipboard\n");
    if (!AddClipboardFormatListener(win32vars.window_handle)){
        win32_output_error_string(false);
    }
    
    win32vars.clip_max = KB(16);
    win32vars.clip_buffer = (u8*)system_memory_allocate(win32vars.clip_max);
    
    win32vars.clipboard_sequence = GetClipboardSequenceNumber();
    if (win32vars.clipboard_sequence == 0){
        win32_post_clipboard("", 0);
        
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
    // App init
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
    
    LOG("Beginning main loop\n");
    u64 timer_start = Win32HighResolutionTime();
    system_acquire_lock(FRAME_LOCK);
    MSG msg;
    for (;keep_running;){
        // TODO(allen): Find a good way to wait on a pipe
        // without interfering with the reading process.
        // NOTE(allen): Looks like we can ReadFile with a 
        // size of zero in an IOCP for this effect.
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
            
            win32vars.input_chunk.pers.mouse_x = mouse_point.x;
            win32vars.input_chunk.pers.mouse_y = mouse_point.y;
        }
        else{
            win32vars.input_chunk.trans.out_of_window = true;
        }
        
        // NOTE(allen): Prepare the Frame Input
        
        // TODO(allen): CROSS REFERENCE WITH LINUX SPECIAL CODE "TIC898989"
        Win32_Input_Chunk input_chunk = win32vars.input_chunk;
        win32vars.input_chunk.trans = null_input_chunk_transient;
        
        input_chunk.pers.control_keys[MDFR_CAPS_INDEX] = GetKeyState(VK_CAPITAL) & 0x1;
        
        Application_Step_Input input = {0};
        
        input.first_step = win32vars.first;
        
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
        
        // NOTE(allen): Frame Clipboard Input
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
        input.clipboard = win32vars.clipboard_contents;
        
        win32vars.clip_post_len = 0;
        
        // NOTE(allen): Initialize result So the Core Doesn't Have to Fill Things it Doesn't Care About
        Application_Step_Result result = {0};
        result.mouse_cursor_type = APP_MOUSE_CURSOR_DEFAULT;
        result.lctrl_lalt_is_altgr = win32vars.lctrl_lalt_is_altgr;
        result.trying_to_kill = input_chunk.trans.trying_to_kill;
        
        // TODO(allen): Not really appropriate to round trip this all the way to the OS layer, redo this system.
        // NOTE(allen): Ask the Core About Exiting if We Have an Exit Signal
        if (win32vars.send_exit_signal){
            result.trying_to_kill = true;
            win32vars.send_exit_signal = false;
        }
        
        // NOTE(allen): Application Core Update
        target.buffer.pos = 0;
        if (app.step != 0){
            app.step(&sysfunc, &target, &memory_vars, &input, &result);
        }
        else{
            LOG("app.step == 0 -- skipping\n");
        }
        
        // NOTE(allen): Finish the Loop
        if (result.perform_kill){
            keep_running = false;
        }
        
        // NOTE(allen): Post New Clipboard Content
        if (win32vars.clip_post_len > 0){
            win32_post_clipboard((char*)win32vars.clip_post_part.base, win32vars.clip_post_len);
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
        
        // NOTE(allen): Update lctrl_lalt_is_altgr Status
        win32vars.lctrl_lalt_is_altgr = result.lctrl_lalt_is_altgr;
        
        // NOTE(allen): Render
        HDC hdc = GetDC(win32vars.window_handle);
        interpret_render_buffer(&target, &shared_vars.pixel_scratch);
        SwapBuffers(hdc);
        ReleaseDC(win32vars.window_handle, hdc);
        
        // NOTE(allen): Toggle Full Screen
        if (win32vars.do_toggle){
            win32_toggle_fullscreen();
            win32vars.do_toggle = false;
        }
        
        // NOTE(allen): Schedule Another Step if Needed
        if (result.animating){
            system_schedule_step();
        }
        
        // NOTE(allen): Sleep a Bit to Cool Off :)
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
        
        // TODO(allen): Only rely on version right inside input?
        win32vars.first = 0;
    }
    
    return(0);
}

#include "win32_4ed_file_track.cpp"
#include "win32_utf8.cpp"

#if 0
// NOTE(allen): In case I want to switch back to a console application at some point.
int main(int argc, char **argv){
    HINSTANCE hInstance = GetModuleHandle(0);
}
#endif

// BOTTOM

