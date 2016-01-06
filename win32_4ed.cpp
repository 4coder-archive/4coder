/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Win32 layer for project codename "4ed"
 *
 */

// TOP

#include "4ed_config.h"

#include "4ed_meta.h"

#define FCPP_FORBID_MALLOC

#include "4cpp_types.h"
#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4ed_mem.cpp"
#include "4ed_math.cpp"

#include "4ed_dll_reader.h"
#include "4coder_custom.h"
#include "4ed_system.h"
#include "4ed_rendering.h"
#include "4ed.h"

#include <windows.h>
#include <GL/gl.h>

#include "4ed_dll_reader.cpp"
#include "4ed_internal.h"
#include "4ed_win32_keyboard.cpp"

#define FPS 30
#define frame_useconds (1000000 / FPS)

struct Thread_Context{
    u32 job_id;
    b32 running;
    
    Work_Queue *queue;
    u32 id;
    u32 windows_id;
    HANDLE handle;
};

struct Thread_Group{
    Thread_Context *threads;
    i32 count;
};

#define UseWinDll 1
#define UseThreadMemory 1

struct Win32_Vars{
	HWND window_handle;
	Key_Codes key_codes;
	Key_Input_Data input_data, previous_data;
    
    Render_Target target;
    
	Mouse_State mouse;
	b32 focus;
	b32 keep_playing;
	HCURSOR cursor_ibeam;
	HCURSOR cursor_arrow;
	HCURSOR cursor_leftright;
	HCURSOR cursor_updown;
	Application_Mouse_Cursor prev_mouse_cursor;
	Clipboard_Contents clipboard_contents;
	b32 next_clipboard_is_self;
	DWORD clipboard_sequence;
    
	Thread_Context main_thread;
    
    Thread_Group groups[THREAD_GROUP_COUNT];
    HANDLE locks[LOCK_COUNT];
    HANDLE DEBUG_sysmem_lock;

#if UseThreadMemory
    Thread_Memory *thread_memory;
#endif

    u64 performance_frequency;
    u64 start_pcount;
    u64 start_time;
    
    HMODULE custom;
#if UseWinDll
    HMODULE app_code;
#else
    DLL_Loaded app_dll;
#endif
    
    System_Functions *system;
    App_Functions app;
    Config_API config_api;
    
#if FRED_INTERNAL
    Sys_Bubble internal_bubble;
#endif
};

globalvar Win32_Vars win32vars;
globalvar Application_Memory memory_vars;
globalvar Exchange exchange_vars;

#if FRED_INTERNAL
internal Bubble*
INTERNAL_system_sentinel(){
    return (&win32vars.internal_bubble);
}

internal void
INTERNAL_system_debug_message(char *message){
    OutputDebugString(message);
}

#endif

internal void*
Win32GetMemory_(i32 size, i32 line_number, char *file_name){
	void *ptr = 0;
    
#if FRED_INTERNAL
    ptr = VirtualAlloc(0, size + sizeof(Sys_Bubble), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    Sys_Bubble *bubble = (Sys_Bubble*)ptr;
    bubble->flags = MEM_BUBBLE_SYS_DEBUG;
    bubble->line_number = line_number;
    bubble->file_name = file_name;
    bubble->size = size;
    WaitForSingleObject(win32vars.DEBUG_sysmem_lock, INFINITE);
    insert_bubble(&win32vars.internal_bubble, bubble);
    ReleaseSemaphore(win32vars.DEBUG_sysmem_lock, 1, 0);
    ptr = bubble + 1;
#else
    ptr = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#endif
    
	return ptr;
}

#define Win32GetMemory(size) Win32GetMemory_(size, __LINE__, __FILE__)

internal void
Win32FreeMemory(void *block){
    if (block){
#if FRED_INTERNAL
        Sys_Bubble *bubble = ((Sys_Bubble*)block) - 1;
        Assert((bubble->flags & MEM_BUBBLE_DEBUG_MASK) == MEM_BUBBLE_SYS_DEBUG);
        WaitForSingleObject(win32vars.DEBUG_sysmem_lock, INFINITE);
        remove_bubble(bubble);
        ReleaseSemaphore(win32vars.DEBUG_sysmem_lock, 1, 0);
        VirtualFree(bubble, 0, MEM_RELEASE);
#else
        VirtualFree(block, 0, MEM_RELEASE);
#endif
    }
}

inline void
system_free_memory(void *block){
    Win32FreeMemory(block);
}

internal Data
system_load_file(char *filename){
    Data result = {};
    HANDLE file;
    file = CreateFile((char*)filename, GENERIC_READ, 0, 0,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (!file || file == INVALID_HANDLE_VALUE){
        return result;
    }
    
    DWORD lo, hi;
    lo = GetFileSize(file, &hi);
    
    if (hi != 0){
        CloseHandle(file);
        return result;
    }
    
    result.size = (lo) + (((u64)hi) << 32);
    result.data = (byte*)Win32GetMemory(result.size);
    
    if (!result.data){
        CloseHandle(file);
        result = {};
        return result;
    }
    
    DWORD read_size;
    BOOL read_result = ReadFile(file, result.data, result.size,
                                &read_size, 0);
    if (!read_result || read_size != (u32)result.size){
        CloseHandle(file);
        Win32FreeMemory(result.data);
        result = {};
        return result;
    }
    
    CloseHandle(file);
    return result;
}

#include "4ed_rendering.cpp"

internal b32
system_save_file(char *filename, void *data, i32 size){
	HANDLE file;
	file = CreateFile((char*)filename, GENERIC_WRITE, 0, 0,
					  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	
	if (!file || file == INVALID_HANDLE_VALUE){
		return 0;
	}
	
	BOOL write_result;
	DWORD bytes_written;
	write_result = WriteFile(file, data, size, &bytes_written, 0);
	
	CloseHandle(file);
	
	if (!write_result || bytes_written != (u32)size){
		return 0;
	}
	
	return 1;
}

internal
Sys_File_Time_Stamp_Sig(system_file_time_stamp){
    u64 result;
    result = 0;
    
    FILETIME last_write;
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx((char*)filename, GetFileExInfoStandard, &data)){
        last_write = data.ftLastWriteTime;
        
        result = ((u64)last_write.dwHighDateTime << 32) | (last_write.dwLowDateTime);
        result /= 10;
    }
    
    return result;
}

internal
Sys_Time_Sig(system_time){
	u64 result = 0;
	LARGE_INTEGER time;
	if (QueryPerformanceCounter(&time)){
		result = (u64)(time.QuadPart - win32vars.start_pcount) * 1000000 / win32vars.performance_frequency;
        result += win32vars.start_time;
	}
	return result;
}

internal
Sys_Set_File_List_Sig(system_set_file_list){
    if (directory.size > 0){
        char dir_space[MAX_PATH + 32];
        String dir = make_string(dir_space, 0, MAX_PATH + 32);
        append(&dir, directory);
        char trail_str[] = "\\*";
        append(&dir, trail_str);
        
        char *c_str_dir = make_c_str(dir);
        
        WIN32_FIND_DATA find_data;
        HANDLE search;
        search = FindFirstFileA(c_str_dir, &find_data);
        
        if (search != INVALID_HANDLE_VALUE){            
            i32 count = 0;
            i32 file_count = 0;
            BOOL more_files = 1;
            do{
                if (!match(find_data.cFileName, ".") &&
                    !match(find_data.cFileName, "..")){
                    ++file_count;
                    i32 size = 0;
                    for(;find_data.cFileName[size];++size);
                    count += size + 1;
                }
                more_files = FindNextFile(search, &find_data);
            }while(more_files);
            FindClose(search);

            i32 required_size = count + file_count * sizeof(File_Info);
            if (file_list->block_size < required_size){
                if (file_list->block){
                    Win32FreeMemory(file_list->block);
                }
                
                file_list->block = Win32GetMemory(required_size);
            }
            
            file_list->infos = (File_Info*)file_list->block;
            char *name = (char*)(file_list->infos + file_count);
            if (file_list->block){
                search = FindFirstFileA(c_str_dir, &find_data);
                
                if (search != INVALID_HANDLE_VALUE){
                    File_Info *info = file_list->infos;
                    more_files = 1;
                    do{
                        if (!match(find_data.cFileName, ".") &&
                            !match(find_data.cFileName, "..")){
                            info->folder = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                            info->filename.str = name;
                            
                            char *name_base = name;
                            i32 i = 0;
                            for(;find_data.cFileName[i];++i) *name++ = find_data.cFileName[i];
                            info->filename.size = (i32)(name - name_base);
                            info->filename.memory_size = info->filename.size + 1;
                            *name++ = 0;
                            ++info;
                        }
                        more_files = FindNextFile(search, &find_data);
                    }while(more_files);
                    FindClose(search);
                    
                    file_list->count = file_count;
                    
                }else{
                    Win32FreeMemory(file_list->block);
                    file_list->block = 0;
                    file_list->block_size = 0;
                }
            }
        }
    }
}

internal
DIRECTORY_HAS_FILE_SIG(system_directory_has_file){
    char *full_filename;
    char space[1024];
    HANDLE file;
    b32 result;
    i32 len;

    full_filename = 0;
    len = str_size(filename);
    if (dir.memory_size - dir.size - 1 >= len){
        full_filename = dir.str;
        memcpy(dir.str + dir.size, filename, len + 1);
    }
    else if (dir.size + len + 1 < 1024){
        full_filename = space;
        memcpy(full_filename, dir.str, dir.size);
        memcpy(full_filename + dir.size, filename, len + 1);
    }

    result = 0;
    if (full_filename){
        file = CreateFile((char*)full_filename, GENERIC_READ, 0, 0,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (file != INVALID_HANDLE_VALUE){
            CloseHandle(file);
            result = 1;
        }
        dir.str[dir.size] = 0;
    }
    
    return(result);
}

b32 Win32DirectoryExists(char *path){
    DWORD attrib = GetFileAttributesA(path);
    return (attrib != INVALID_FILE_ATTRIBUTES &&
            (attrib & FILE_ATTRIBUTE_DIRECTORY));
}

internal
DIRECTORY_CD_SIG(system_directory_cd){
    b32 result = 0;
    i32 old_size;
    i32 len;
    
    if (rel_path[0] != 0){
        if (rel_path[0] == '.' && rel_path[1] == 0){
            result = 1;
        }
        else if (rel_path[0] == '.' && rel_path[1] == '.' && rel_path[2] == 0){
            result = remove_last_folder(dir);
            terminate_with_null(dir);
        }
        else{
            len = str_size(rel_path);
            if (dir->size + len + 1 > dir->memory_size){
                old_size = dir->size;
                append_partial(dir, rel_path);
                append_partial(dir, "\\");
                if (Win32DirectoryExists(dir->str)){
                    result = 1;
                }
                else{
                    dir->size = old_size;
                }
            }
        }
    }
    
    return(result);
}

internal
Sys_Post_Clipboard_Sig(system_post_clipboard){
	if (OpenClipboard(win32vars.window_handle)){
		EmptyClipboard();
		HANDLE memory_handle;
		memory_handle = GlobalAlloc(GMEM_MOVEABLE, str.size+1);
		if (memory_handle){
			char *dest = (char*)GlobalLock(memory_handle);
            copy_fast_unsafe(dest, str);
			GlobalUnlock(memory_handle);
			SetClipboardData(CF_TEXT, memory_handle);
			win32vars.next_clipboard_is_self = 1;
		}
		CloseClipboard();
	}
}

internal void
Win32RedrawScreen(HDC hdc){
    launch_rendering(&win32vars.target);
    win32vars.target.size = 0;
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
        
        win32vars.target.width = width;
        win32vars.target.height = height;
    }
}

#define HOTKEY_ALT_ID 0

internal LRESULT
Win32Callback(HWND hwnd, UINT uMsg,
              WPARAM wParam, LPARAM lParam){
    LRESULT result = {};
    switch (uMsg){
    case WM_MENUCHAR:
    case WM_SYSCHAR:break;
    
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:
    {
        switch (wParam){
        case VK_CONTROL:case VK_LCONTROL:case VK_RCONTROL:
        case VK_MENU:case VK_LMENU:case VK_RMENU:
        case VK_SHIFT:case VK_LSHIFT:case VK_RSHIFT: break;
            
        default:
            b8 previous_state, current_state;
            previous_state = ((lParam & Bit_30)?(1):(0));
            current_state = ((lParam & Bit_31)?(0):(1));
        
            if (current_state){
                u8 key = keycode_lookup((u8)wParam);
                
                i32 *count = 0;
                Key_Event_Data *data = 0;
                
                if (!previous_state){
                    count = &win32vars.input_data.press_count;
                    data = win32vars.input_data.press;
                }
                else{
                    count = &win32vars.input_data.hold_count;
                    data = win32vars.input_data.hold;
                }
                
                if (*count < KEY_INPUT_BUFFER_SIZE){
                    if (!key){
                        UINT vk = (UINT)wParam;
                        UINT scan = (UINT)((lParam >> 16) & 0x7F);
                        BYTE state[256];
                        WORD x;
                        int result;
                        
                        GetKeyboardState(state);
                        if (win32vars.input_data.control_keys[CONTROL_KEY_CONTROL] &&
                            !win32vars.input_data.control_keys[CONTROL_KEY_ALT])
                            state[VK_CONTROL] = 0;
                        result = ToAscii(vk, scan, state, &x, 0);
                        if (result == 1 && x < 128){
                            key = (u8)x;
                            if (key == '\r') key = '\n';
                            data[*count].character = key;
                            
                            state[VK_CAPITAL] = 0;
                            result = ToAscii(vk, scan, state, &x, 0);
                            if (result == 1 && x < 128){
                                key = (u8)x;
                                if (key == '\r') key = '\n';
                                data[*count].character_no_caps_lock = key;
                                data[*count].keycode = key;
                            }
                        }
                        if (result != 1 || x >= 128){
                            data[*count].character = 0;
                            data[*count].character_no_caps_lock = 0;
                            data[*count].keycode = 0;
                        }
                    }
                    else{
                        data[*count].character = 0;
                        data[*count].character_no_caps_lock = 0;
                        data[*count].keycode = key;
                    }
                    ++(*count);
                }
            }
            
            result = DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }break;

    case WM_INPUT:
    {
        char buffer[sizeof(RAWINPUT)] = {};
        UINT size = sizeof(RAWINPUT);
        GetRawInputData((HRAWINPUT)(lParam), RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER));
        
        RAWINPUT *rawinput = (RAWINPUT*)(buffer);
        if (rawinput->header.dwType == RIM_TYPEKEYBOARD){
            RAWKEYBOARD *raw = &rawinput->data.keyboard;
            UINT vk = raw->VKey;
            UINT flags = raw->Flags;
            b8 down = !(flags & 1);
            
            if (vk != 255){
                if (vk == VK_SHIFT){
                    win32vars.input_data.control_keys[CONTROL_KEY_SHIFT] = down;
                }
                else if (vk == VK_CONTROL){
                    win32vars.input_data.control_keys[CONTROL_KEY_CONTROL] = down;
                }
                else if (vk == VK_MENU){
                    UINT is_gr = ((flags & RI_KEY_E0) != 0);
                    if (!is_gr){
                        win32vars.input_data.control_keys[CONTROL_KEY_ALT] = down;
                    }
                }
            }
        }
        
        result = DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }break;

    case WM_MOUSEMOVE:
    {
        win32vars.mouse.x = LOWORD(lParam);
        win32vars.mouse.y = HIWORD(lParam);
    }break;
    
    case WM_MOUSEWHEEL:
    {
        i16 rotation = GET_WHEEL_DELTA_WPARAM(wParam);
        if (rotation > 0){
            win32vars.mouse.wheel = 1;
        }
        else{
            win32vars.mouse.wheel = -1;
        }
    }break;
    
    case WM_LBUTTONDOWN:
    {
        win32vars.mouse.left_button = true;
    }break;
    
    case WM_RBUTTONDOWN:
    {
        win32vars.mouse.right_button = true;
    }break;
    
    case WM_LBUTTONUP:
    {
        win32vars.mouse.left_button = false;
    }break;
    
    case WM_RBUTTONUP:
    {
        win32vars.mouse.right_button = false;
    }break;
    
    case WM_KILLFOCUS:
    {
        win32vars.focus = 0;
        win32vars.mouse.left_button = false;
        win32vars.mouse.right_button = false;
    }break;
    
    case WM_SETFOCUS:
    {
        win32vars.focus = 1;
        for (int i = 0; i < CONTROL_KEY_COUNT; ++i){
            win32vars.input_data.control_keys[i] = 0;
        }
    }break;
    
    case WM_SIZE:
    {
        if (win32vars.target.handle){
            i32 new_width = LOWORD(lParam);
            i32 new_height = HIWORD(lParam);
            
            Win32Resize(new_width, new_height);
        }
    }break;
    
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        Clipboard_Contents empty_contents = {};
        win32vars.app.step(win32vars.system,
                           &win32vars.key_codes,
                           &win32vars.previous_data,
                           &win32vars.mouse,
                           &win32vars.target,
                           &memory_vars,
                           &exchange_vars,
                           empty_contents,
                           0, 0, 1);
        
        Win32RedrawScreen(hdc);
        EndPaint(hwnd, &ps);
    }break;
    
    case WM_CLOSE: // NOTE(allen): I expect WM_CLOSE not WM_DESTROY
    case WM_DESTROY:
    {
        win32vars.keep_playing = 0;
    }break;
    
    default:
    {
        result = DefWindowProc(hwnd, uMsg, wParam, lParam);
    }break;
	}
	return result;
}

internal HANDLE
Win32Handle(Plat_Handle h){
    HANDLE result;
    result = {};
    result = *(HANDLE*)(&h);
    return(result);
}

internal Plat_Handle
Win32GenHandle(HANDLE h){
    Assert(sizeof(Plat_Handle) >= sizeof(HANDLE));
    Plat_Handle result;
    result = *(Plat_Handle*)(&h);
    return(result);
}

internal DWORD WINAPI
ThreadProc(LPVOID lpParameter){
    Thread_Context *thread = (Thread_Context*)lpParameter;
    Work_Queue *queue = thread->queue;
    
    for (;;){
        u32 read_index = queue->read_position;
        u32 write_index = queue->write_position;
        
        if (read_index != write_index){
            u32 next_read_index = (read_index + 1) % JOB_ID_WRAP;
            u32 safe_read_index =
                InterlockedCompareExchange(&queue->read_position,
                                           next_read_index, read_index);
            
            if (safe_read_index == read_index){
                Full_Job_Data *full_job = queue->jobs + (safe_read_index % QUEUE_WRAP);
                // NOTE(allen): This is interlocked so that it plays nice
                // with the cancel job routine, which may try to cancel this job
                // at the same time that we try to run it
                
                i32 safe_running_thread =
                    InterlockedCompareExchange(&full_job->running_thread,
                                               thread->id, THREAD_NOT_ASSIGNED);
                
                if (safe_running_thread == THREAD_NOT_ASSIGNED){
                    thread->job_id = full_job->id;
                    thread->running = 1;
#if UseThreadMemory
                    Thread_Memory *thread_memory = 0;
#endif
                    
                    // TODO(allen): remove memory_request
                    if (full_job->job.memory_request != 0){
#if UseThreadMemory
                        thread_memory = win32vars.thread_memory + thread->id - 1;
                        if (thread_memory->size < full_job->job.memory_request){
                            if (thread_memory->data){
                                Win32FreeMemory(thread_memory->data);
                            }
                            i32 new_size = LargeRoundUp(full_job->job.memory_request, Kbytes(4));
                            thread_memory->data = Win32GetMemory(new_size);
                            thread_memory->size = new_size;
                        }
#endif
                    }
                    full_job->job.callback(win32vars.system, thread, thread_memory,
                                           &exchange_vars.thread, full_job->job.data);
                    full_job->running_thread = 0;
                    thread->running = 0;
                }
            }
        }
        else{
            WaitForSingleObject(Win32Handle(queue->semaphore), INFINITE);
        }
    }
}

internal
Sys_Post_Job_Sig(system_post_job){
    Work_Queue *queue = exchange_vars.thread.queues + group_id;
    
    Assert((queue->write_position + 1) % QUEUE_WRAP != queue->read_position % QUEUE_WRAP);
    
    b32 success = 0;
    u32 result = 0;
    while (!success){
        u32 write_index = queue->write_position;
        u32 next_write_index = (write_index + 1) % JOB_ID_WRAP;
        u32 safe_write_index =
            InterlockedCompareExchange(&queue->write_position,
                                       next_write_index, write_index);
        if (safe_write_index  == write_index){
            result = write_index;
            write_index = write_index % QUEUE_WRAP;
            queue->jobs[write_index].job = job;
            queue->jobs[write_index].running_thread = THREAD_NOT_ASSIGNED;
            queue->jobs[write_index].id = result;
            success = 1;
        }
    }
    
    ReleaseSemaphore(Win32Handle(queue->semaphore), 1, 0);
    
    return result;
}

internal
Sys_Acquire_Lock_Sig(system_acquire_lock){
    WaitForSingleObject(win32vars.locks[id], INFINITE);
}

internal
Sys_Release_Lock_Sig(system_release_lock){
    ReleaseSemaphore(win32vars.locks[id], 1, 0);
}

internal
Sys_Cancel_Job_Sig(system_cancel_job){
    Work_Queue *queue = exchange_vars.thread.queues + group_id;
    Thread_Group *group = win32vars.groups + group_id;
    
    u32 job_index;
    u32 thread_id;
    Full_Job_Data *full_job;
    Thread_Context *thread;
    
    job_index = job_id % QUEUE_WRAP;
    full_job = queue->jobs + job_index;
    
    Assert(full_job->id == job_id);
    thread_id =
        InterlockedCompareExchange(&full_job->running_thread,
                                   0, THREAD_NOT_ASSIGNED);
    
    if (thread_id != THREAD_NOT_ASSIGNED){
        system_acquire_lock(CANCEL_LOCK0 + thread_id - 1);
        thread = group->threads + thread_id - 1;
        TerminateThread(thread->handle, 0);
        u32 creation_flag = 0;
        thread->handle = CreateThread(0, 0, ThreadProc, thread, creation_flag, (LPDWORD)&thread->windows_id);
        system_release_lock(CANCEL_LOCK0 + thread_id - 1);
        thread->running = 0;
    }
}

#if UseThreadMemory
internal void
system_grow_thread_memory(Thread_Memory *memory){
    system_acquire_lock(CANCEL_LOCK0 + memory->id - 1);
    void *old_data = memory->data;
    i32 old_size = memory->size;
    i32 new_size = LargeRoundUp(memory->size*2, Kbytes(4));
    memory->data = Win32GetMemory(new_size);
    memory->size = new_size;
    if (old_data){
        memcpy(memory->data, old_data, old_size);
        Win32FreeMemory(old_data);
    }
    system_release_lock(CANCEL_LOCK0 + memory->id - 1);
}
#endif

#if FRED_INTERNAL
internal void
INTERNAL_get_thread_states(Thread_Group_ID id, bool8 *running, i32 *pending){
    Work_Queue *queue = exchange_vars.thread.queues + id;
    u32 write = queue->write_position;
    u32 read = queue->read_position;
    if (write < read) write += JOB_ID_WRAP;
    *pending = (i32)(write - read);
    
    Thread_Group *group = win32vars.groups + id;
    for (i32 i = 0; i < group->count; ++i){
        running[i] = (group->threads[i].running != 0);
    }
}
#endif

internal
Sys_CLI_Call_Sig(system_cli_call){
    char cmd[] = "c:\\windows\\system32\\cmd.exe";
    char *env_variables = 0;
    char command_line[2048];
    
    b32 success = 1;
    String s = make_fixed_width_string(command_line);
    copy(&s, make_lit_string("/C "));
    append_partial(&s, script_name);
    success = terminate_with_null(&s);
    
    if (success){
        success = 0;
        
        SECURITY_ATTRIBUTES sec_attributes;
        HANDLE out_read;
        HANDLE out_write;
        
        sec_attributes = {};
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
                if (CreateProcess(cmd, command_line,
                                  0, 0, TRUE, 0,
                                  env_variables, path,
                                  &startup, &info)){
                    success = 1;
                    CloseHandle(info.hThread);
                    *(HANDLE*)&cli_out->proc = info.hProcess;
                    *(HANDLE*)&cli_out->out_read = out_read;
                    *(HANDLE*)&cli_out->out_write = out_write;
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
    
    return has_more;
}

internal
Sys_CLI_End_Update_Sig(system_cli_end_update){
    b32 close_me = 0;
    if (WaitForSingleObject(*(HANDLE*)&cli->proc, 0) == WAIT_OBJECT_0){
        close_me = 1;
        CloseHandle(*(HANDLE*)&cli->proc);
        CloseHandle(*(HANDLE*)&cli->out_read);
        CloseHandle(*(HANDLE*)&cli->out_write);
    }
    return close_me;
}

internal b32
Win32LoadAppCode(){
    b32 result = 0;

#if UseWinDll
    win32vars.app_code = LoadLibraryA("4ed_app.dll");
    if (win32vars.app_code){
        result = 1;
        App_Get_Functions *get_funcs = (App_Get_Functions*)
            GetProcAddress(win32vars.app_code, "app_get_functions");

        win32vars.app = get_funcs();
    }

#else
    Data file = system_load_file("4ed_app.dll");

    if (file.data){
        i32 error;
        DLL_Data dll_data;
        if (dll_parse_headers(file, &dll_data, &error)){
            Data img;
            img.size = dll_total_loaded_size(&dll_data);
            img.data = (byte*)
                VirtualAlloc((LPVOID)Tbytes(3), img.size,
                             MEM_COMMIT | MEM_RESERVE,
                             PAGE_READWRITE);

            dll_load(img, &win32vars.app_dll, file, &dll_data);

            DWORD extra_;
            VirtualProtect(img.data + win32vars.app_dll.text_start,
                           win32vars.app_dll.text_size,
                           PAGE_EXECUTE_READ,
                           &extra_);

            result = 1;
            App_Get_Functions *get_functions = (App_Get_Functions*)
                dll_load_function(&win32vars.app_dll, "app_get_functions", 17);
        }
        else{
            // TODO(allen): file loading error
        }

        system_free(file.data);
        
        DUMP((byte*)(Tbytes(3)), Kbytes(400));
    }
    else{
        // TODO(allen): file loading error
    }
    
#endif

    return result;
}

internal void
Win32LoadSystemCode(){
    win32vars.system->file_time_stamp = system_file_time_stamp;
    win32vars.system->set_file_list = system_set_file_list;

    win32vars.system->directory_has_file = system_directory_has_file;
    win32vars.system->directory_cd = system_directory_cd;

    win32vars.system->post_clipboard = system_post_clipboard;
    win32vars.system->time = system_time;
    
    win32vars.system->cli_call = system_cli_call;
    win32vars.system->cli_begin_update = system_cli_begin_update;
    win32vars.system->cli_update_step = system_cli_update_step;
    win32vars.system->cli_end_update = system_cli_end_update;

    win32vars.system->post_job = system_post_job;
    win32vars.system->cancel_job = system_cancel_job;
    win32vars.system->grow_thread_memory = system_grow_thread_memory;
    win32vars.system->acquire_lock = system_acquire_lock;
    win32vars.system->release_lock = system_release_lock;
    
    win32vars.system->internal_sentinel = INTERNAL_system_sentinel;
    win32vars.system->internal_get_thread_states = INTERNAL_get_thread_states;
    win32vars.system->internal_debug_message = INTERNAL_system_debug_message;
}

void
ex__file_insert(File_Slot *pos, File_Slot *file){
    file->next = pos->next;
    file->next->prev = file;
    file->prev = pos;
    pos->next = file;
}

void
ex__insert_range(File_Slot *start, File_Slot *end, File_Slot *pos){
    end->next->prev = start->prev;
    start->prev->next = end->next;
    
    end->next = pos->next;
    start->prev = pos;
    pos->next->prev = end;
    pos->next = start;
}

internal void
ex__check_file(File_Slot *pos){
    File_Slot *file = pos;
    
    Assert(pos == pos->next->prev);
    
    for (pos = pos->next;
         file != pos;
         pos = pos->next){
        Assert(pos == pos->next->prev);
    }
}

internal void
ex__check(File_Exchange *file_exchange){
    ex__check_file(&file_exchange->available);
    ex__check_file(&file_exchange->active);
    ex__check_file(&file_exchange->free_list);
}

int
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow){
    win32vars = {};
    
    exchange_vars = {};
    
#if FRED_INTERNAL
    win32vars.internal_bubble.next = &win32vars.internal_bubble;
    win32vars.internal_bubble.prev = &win32vars.internal_bubble;
    win32vars.internal_bubble.flags = MEM_BUBBLE_SYS_DEBUG;
#endif
    
    if (!Win32LoadAppCode()){
        // TODO(allen): Failed to load app code, serious problem.
        return 99;
    }
    
    System_Functions system_;
    System_Functions *system = &system_;
    win32vars.system = system;
    Win32LoadSystemCode();
    
    LARGE_INTEGER lpf;
    QueryPerformanceFrequency(&lpf);
    win32vars.performance_frequency = lpf.QuadPart;
    QueryPerformanceCounter(&lpf);
    win32vars.start_pcount = lpf.QuadPart;
    
    FILETIME filetime;
    GetSystemTimeAsFileTime(&filetime);
    win32vars.start_time = ((u64)filetime.dwHighDateTime << 32) | (filetime.dwLowDateTime);
    win32vars.start_time /= 10;
    
    keycode_init(&win32vars.key_codes);
    
#ifdef FRED_SUPER
    win32vars.custom = LoadLibraryA("4coder_custom.dll");
    if (win32vars.custom){
        win32vars.config_api.get_bindings = (Get_Binding_Data_Function*)
            GetProcAddress(win32vars.custom, "get_bindings");
        
        win32vars.config_api.set_extra_font = (Set_Extra_Font_Function*)
            GetProcAddress(win32vars.custom, "set_extra_font");
    }
#endif
    
    Thread_Context background[4];
    memset(background, 0, sizeof(background));
    win32vars.groups[BACKGROUND_THREADS].threads = background;
    win32vars.groups[BACKGROUND_THREADS].count = ArrayCount(background);

    
#if UseThreadMemory
    Thread_Memory thread_memory[ArrayCount(background)];
    win32vars.thread_memory = thread_memory;
#endif

    
    exchange_vars.thread.queues[BACKGROUND_THREADS].semaphore =
        Win32GenHandle(
            CreateSemaphore(0, 0, win32vars.groups[BACKGROUND_THREADS].count, 0)
                       );
    
    u32 creation_flag = 0;
    for (i32 i = 0; i < win32vars.groups[BACKGROUND_THREADS].count; ++i){
        Thread_Context *thread = win32vars.groups[BACKGROUND_THREADS].threads + i;
        thread->id = i + 1;

#if UseThreadMemory
        Thread_Memory *memory = win32vars.thread_memory + i;
        *memory = {};
        memory->id = thread->id;
#endif
        
        thread->queue = &exchange_vars.thread.queues[BACKGROUND_THREADS];
        thread->handle = CreateThread(0, 0, ThreadProc, thread, creation_flag, (LPDWORD)&thread->windows_id);
    }
    
    Assert(win32vars.locks);
    for (i32 i = 0; i < LOCK_COUNT; ++i){
        win32vars.locks[i] = CreateSemaphore(0, 1, 1, 0);
    }
    win32vars.DEBUG_sysmem_lock = CreateSemaphore(0, 1, 1, 0);
    
    win32vars.cursor_ibeam = LoadCursor(NULL, IDC_IBEAM);
    win32vars.cursor_arrow = LoadCursor(NULL, IDC_ARROW);
    win32vars.cursor_leftright = LoadCursor(NULL, IDC_SIZEWE);
    win32vars.cursor_updown = LoadCursor(NULL, IDC_SIZENS);
    win32vars.prev_mouse_cursor = APP_MOUSE_CURSOR_ARROW;
    
	WNDCLASS window_class = {};
	window_class.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	window_class.lpfnWndProc = Win32Callback;
	window_class.hInstance = hInstance;
	window_class.lpszClassName = "4coder-win32-wndclass";
    
	if (!RegisterClass(&window_class)){
		return 1;
	}
    
    RECT window_rect = {};
    window_rect.right = 800;
    window_rect.bottom = 600;
    
    if (!AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, false)){
        // TODO(allen): non-fatal diagnostics
    }

#define WINDOW_NAME "4coder-window"
    
    HWND window_handle = {};
    window_handle = CreateWindowA(
        window_class.lpszClassName,
        WINDOW_NAME,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        0, 0, hInstance, 0);
    
    if (window_handle == 0){
        return 2;
    }
    
    // TODO(allen): errors?
    win32vars.window_handle = window_handle;
    HDC hdc = GetDC(window_handle);
    
    GetClientRect(window_handle, &window_rect);
    
    RAWINPUTDEVICE device;
    device.usUsagePage = 0x1;
    device.usUsage = 0x6;
    device.dwFlags = 0;
    device.hwndTarget = window_handle;
    RegisterRawInputDevices(&device, 1, sizeof(device));
    
    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0 };
    
    i32 pixel_format;
    pixel_format = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixel_format, &pfd);
    
    win32vars.target.handle = hdc;
    win32vars.target.context = wglCreateContext(hdc);
    wglMakeCurrent(hdc, (HGLRC)win32vars.target.context);
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Win32Resize(window_rect.right - window_rect.left, window_rect.bottom - window_rect.top);
    
    LPVOID base;
#if FRED_INTERNAL
    base = (LPVOID)Tbytes(1);
#else
    base = (LPVOID)0;
#endif
    
	memory_vars.vars_memory_size = Mbytes(2);
    memory_vars.vars_memory = VirtualAlloc(base, memory_vars.vars_memory_size,
                                           MEM_COMMIT | MEM_RESERVE,
                                           PAGE_READWRITE);
    
#if FRED_INTERNAL
    base = (LPVOID)Tbytes(2);
#else
    base = (LPVOID)0;
#endif
    memory_vars.target_memory_size = Mbytes(512);
    memory_vars.target_memory = VirtualAlloc(base, memory_vars.target_memory_size,
                                             MEM_COMMIT | MEM_RESERVE,
                                             PAGE_READWRITE);
    
    if (!memory_vars.vars_memory){
        return 4;
    }
    
    win32vars.clipboard_sequence = GetClipboardSequenceNumber();
    
    if (win32vars.clipboard_sequence == 0){
        system_post_clipboard(make_lit_string(""));
        
        win32vars.clipboard_sequence = GetClipboardSequenceNumber();
        win32vars.next_clipboard_is_self = 0;
        
        if (win32vars.clipboard_sequence == 0){
            // TODO(allen): diagnostics
        }
	}
    
    else{
        if (IsClipboardFormatAvailable(CF_TEXT)){
            if (OpenClipboard(win32vars.window_handle)){
                HANDLE clip_data;
                clip_data = GetClipboardData(CF_TEXT);
                if (clip_data){
                    win32vars.clipboard_contents.str = (u8*)GlobalLock(clip_data);
                    if (win32vars.clipboard_contents.str){
                        win32vars.clipboard_contents.size = str_size((char*)win32vars.clipboard_contents.str);
                        GlobalUnlock(clip_data);
                    }
                }
                CloseClipboard();
            }
        }
    }

    win32vars.target.push_clip = draw_push_clip;
    win32vars.target.pop_clip = draw_pop_clip;
    win32vars.target.push_piece = draw_push_piece;
    
    win32vars.target.font_set.font_info_load = draw_font_info_load;
    win32vars.target.font_set.font_load = draw_font_load;
    win32vars.target.font_set.release_font = draw_release_font;

    win32vars.target.max = Mbytes(1);
    win32vars.target.push_buffer = (byte*)Win32GetMemory(win32vars.target.max);

    File_Slot file_slots[32];
    exchange_vars.file.max = sizeof(file_slots) / sizeof(file_slots[0]);
    exchange_vars.file.available = {};
    exchange_vars.file.available.next = &exchange_vars.file.available;
    exchange_vars.file.available.prev = &exchange_vars.file.available;
    
    exchange_vars.file.active = {};
    exchange_vars.file.active.next = &exchange_vars.file.active;
    exchange_vars.file.active.prev = &exchange_vars.file.active;
        
    exchange_vars.file.free_list = {};
    exchange_vars.file.free_list.next = &exchange_vars.file.free_list;
    exchange_vars.file.free_list.prev = &exchange_vars.file.free_list;

    exchange_vars.file.files = file_slots;
    memset(file_slots, 0, sizeof(file_slots));

    char *filename_space = (char*)
        Win32GetMemory(FileNameMax*exchange_vars.file.max);
    
    for (int i = 0; i < exchange_vars.file.max; ++i){
        File_Slot *slot = file_slots + i;
        ex__file_insert(&exchange_vars.file.available, slot);
        slot->filename = filename_space;
        filename_space += FileNameMax;
    }
    
    DWORD required = GetCurrentDirectory(0, 0);
    required += 1;
    required *= 4;
    char *current_directory_mem = (char*)Win32GetMemory(required);
    DWORD written = GetCurrentDirectory(required, current_directory_mem);

    String current_directory = make_string(current_directory_mem, written, required);
    terminate_with_null(&current_directory);
    
	if (!win32vars.app.init(win32vars.system, &win32vars.target,
                            &memory_vars, &exchange_vars, &win32vars.key_codes,
                            win32vars.clipboard_contents, current_directory,
                            win32vars.config_api)){
		return 5;
	}
	
	win32vars.keep_playing = 1;
	timeBeginPeriod(1);
	
    system_acquire_lock(FRAME_LOCK);
    Thread_Context *thread = &win32vars.main_thread;
    AllowLocal(thread);
	b32 first = 1;
	i64 timer_start = system_time();
    
	while (win32vars.keep_playing){
        ProfileStart(OS_input);
		win32vars.previous_data = win32vars.input_data;
		win32vars.input_data.press_count = 0;
		win32vars.input_data.hold_count = 0;
		win32vars.input_data.caps_lock = GetKeyState(VK_CAPITAL) & 0x1;
        
		win32vars.mouse.left_button_prev = win32vars.mouse.left_button;
		win32vars.mouse.right_button_prev = win32vars.mouse.right_button;
		win32vars.mouse.wheel = 0;
		
		MSG msg;
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE) && win32vars.keep_playing){
			if (msg.message == WM_QUIT){
				win32vars.keep_playing = 0;
			}else{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		
		if (!win32vars.keep_playing){
			break;
		}
		
		win32vars.mouse.out_of_window = 0;
		POINT mouse_point;
		if (GetCursorPos(&mouse_point) &&
			ScreenToClient(window_handle, &mouse_point)){
			
			if (mouse_point.x < 0 || mouse_point.x >= win32vars.target.width ||
				mouse_point.y < 0 || mouse_point.y >= win32vars.target.height){
				win32vars.mouse.out_of_window = 1;
			}
		}
		else{
			win32vars.mouse.out_of_window = 1;
		}
		
		win32vars.clipboard_contents = {};
		if (win32vars.clipboard_sequence != 0){
			DWORD new_number = GetClipboardSequenceNumber();
			if (new_number != win32vars.clipboard_sequence){
				win32vars.clipboard_sequence = new_number;
				if (win32vars.next_clipboard_is_self){
					win32vars.next_clipboard_is_self = 0;
				}
				else if (IsClipboardFormatAvailable(CF_TEXT)){
					if (OpenClipboard(win32vars.window_handle)){
						HANDLE clip_data;
						clip_data = GetClipboardData(CF_TEXT);
						if (clip_data){
							win32vars.clipboard_contents.str = (u8*)GlobalLock(clip_data);
							if (win32vars.clipboard_contents.str){
								win32vars.clipboard_contents.size = str_size((char*)win32vars.clipboard_contents.str);
								GlobalUnlock(clip_data);
							}
						}
						CloseClipboard();
					}
				}
			}
		}
        
        u32 redraw = exchange_vars.thread.force_redraw;
        if (redraw){
            exchange_vars.thread.force_redraw = 0;
        }
        
        ProfileEnd(OS_input);
        
        Application_Step_Result result =
            win32vars.app.step(win32vars.system,
                               &win32vars.key_codes,
                               &win32vars.input_data,
                               &win32vars.mouse,
                               &win32vars.target,
                               &memory_vars,
                               &exchange_vars,
                               win32vars.clipboard_contents,
                               1, first, redraw);
        
        ProfileStart(OS_frame_out);
		first = 0;
		switch (result.mouse_cursor_type){
			case APP_MOUSE_CURSOR_ARROW:
				SetCursor(win32vars.cursor_arrow); break;
			case APP_MOUSE_CURSOR_IBEAM:
				SetCursor(win32vars.cursor_ibeam); break;
			case APP_MOUSE_CURSOR_LEFTRIGHT:
				SetCursor(win32vars.cursor_leftright); break;
			case APP_MOUSE_CURSOR_UPDOWN:
                SetCursor(win32vars.cursor_updown); break;
		}
		
		if (result.redraw) Win32RedrawScreen(hdc);
        ProfileEnd(OS_frame_out);
        
        ProfileStart(OS_file_process);
        {
            File_Slot *file;
            int d = 0;
            
            for (file = exchange_vars.file.active.next;
                 file != &exchange_vars.file.active;
                 file = file->next){
                ++d;
                
                if (file->flags & FEx_Save){
                    Assert((file->flags & FEx_Request) == 0);
                    file->flags &= (~FEx_Save);
                    system_save_file(file->filename, file->data, file->size);
                    file->flags |= FEx_Save_Complete;
                }
                
                if (file->flags & FEx_Request){
                    Assert((file->flags & FEx_Save) == 0);
                    file->flags &= (~FEx_Request);
                    Data sysfile =
                        system_load_file(file->filename);
                    if (sysfile.data == 0){
                        file->flags |= FEx_Not_Exist;
                    }
                    else{
                        file->flags |= FEx_Ready;
                        file->data = sysfile.data;
                        file->size = sysfile.size;
                    }
                }
            }
            
            Assert(d == exchange_vars.file.num_active);

            for (file = exchange_vars.file.free_list.next;
                 file != &exchange_vars.file.free_list;
                 file = file->next){
                if (file->data){
                    system_free_memory(file->data);
                }
            }

            if (exchange_vars.file.free_list.next != &exchange_vars.file.free_list){
                ex__insert_range(exchange_vars.file.free_list.next, exchange_vars.file.free_list.prev,
                                 &exchange_vars.file.available);
            }

            ex__check(&exchange_vars.file);
        }
        ProfileEnd(OS_file_process);
        
        ProfileStart(frame_sleep);
		i64 timer_end = system_time();
		i64 end_target = (timer_start + frame_useconds);
        
        system_release_lock(FRAME_LOCK);
		while (timer_end < end_target){
            DWORD samount = (DWORD)((end_target - timer_end) / 1000);
            if (samount > 0) Sleep(samount);
            timer_end = system_time();
		}
        system_acquire_lock(FRAME_LOCK);
		timer_start = system_time();7
        ProfileEnd(frame_sleep);
	}
	
	return 0;
}

// BOTTOM


