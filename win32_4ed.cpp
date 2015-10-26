/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Win32 layer for project codename "4ed"
 *
 */

// TOP
// TODO(allen):
//
// Fix the OwnDC thing.
// 

#define FRED_PRINT_DEBUG 1
#define FRED_PRINT_DEBUG_FILE_LINE 0
#define FRED_PROFILING 1
#define FRED_PROFILING_OS 0
#define FRED_FULL_ERRORS 0

#ifndef FRED_SLOW
#define FRED_SLOW 0
#else
#undef FRED_SLOW
#define FRED_SLOW 1
#endif

#ifndef FRED_INTERNAL
#define FRED_INTERNAL 0
#else
#undef FRED_INTERNAL
#define FRED_INTERNAL 1
#endif

#define SOFTWARE_RENDER 0

#if FRED_INTERNAL == 0
#undef FRED_PRINT_DEBUG
#define FRED_PRINT_DEBUG 0
#undef FRED_PROFILING
#define FRED_PROFILING 0
#undef FRED_PROFILING_OS
#define FRED_PROFILING_OS 0
#endif

#if FRED_PRINT_DEBUG == 0
#undef FRED_PRINT_DEBUG_FILE_LINE
#define FRED_PRINT_DEBUG_FILE_LINE 0
#undef FRED_PRINT_DEBUG_FILE_LINE
#define FRED_PROFILING_OS 0
#endif

#define FPS 30
#define FRAME_TIME (1000000 / FPS)

#define BUFFER_EXPERIMENT_SCALPEL 1

#include "4ed_meta.h"

#define FCPP_FORBID_MALLOC

#include "4cpp_types.h"
#define FCPP_STRING_IMPLEMENTATION
#include "4cpp_string.h"
#define FCPP_LEXER_IMPLEMENTATION
#include "4cpp_lexer.h"
#include "4ed_math.cpp"
#include "4coder_custom.h"
#include "4ed.h"
#include "4ed_system.h"
#include "4ed_rendering.h"

struct TEMP_BACKDOOR{
    Get_Binding_Data_Function *get_bindings;
    Set_Extra_Font_Function *set_extra_font;
} TEMP;

#if FRED_INTERNAL

struct Sys_Bubble : public Bubble{
    i32 line_number;
    char *file_name;
};

#endif

#include <windows.h>
#include <GL/gl.h>

#include "4ed_internal.h"
#include "4ed_rendering.cpp"
#include "4ed_command.cpp"
#include "4ed_layout.cpp"
#include "4ed_style.cpp"
#if BUFFER_EXPERIMENT_SCALPEL
#include "4ed_file_view_golden_array.cpp"
#else
#include "4ed_file_view.cpp"
#endif
#include "4ed_color_view.cpp"
#include "4ed_interactive_view.cpp"
#include "4ed_menu_view.cpp"
#include "4ed_debug_view.cpp"
#include "4ed.cpp"
#include "4ed_keyboard.cpp"

struct Full_Job_Data{
    Job_Data job;
    
    u32 job_memory_index;
    u32 running_thread;
    bool32 finished;
    u32 id;
};

struct Work_Queue{
    u32 volatile write_position;
    u32 volatile read_position;
    Full_Job_Data jobs[256];
    
    HANDLE semaphore;
};

struct Thread_Context{
    u32 job_id;
    bool32 running;
    
    Work_Queue *queue;
    u32 id;
    u32 windows_id;
    HANDLE handle;
};

struct Thread_Group{
    Thread_Context *threads;
    i32 count;
};

struct Win32_Vars{
	HWND window_handle;
	Key_Codes key_codes, loose_codes;
	Key_Input_Data input_data, previous_data;
    
#if SOFTWARE_RENDER
	BITMAPINFO bmp_info;
	union{
		struct{
			void *pixel_data;
			i32 width, height, pitch;
		};
		Render_Target target;
	};
	i32 true_pixel_size;
#else
    Render_Target target;
#endif
    
    u32 volatile force_redraw;
    
	Mouse_State mouse;
	bool32 focus;
	bool32 keep_playing;
	HCURSOR cursor_ibeam;
	HCURSOR cursor_arrow;
	HCURSOR cursor_leftright;
	HCURSOR cursor_updown;
	Application_Mouse_Cursor prev_mouse_cursor;
	Clipboard_Contents clipboard_contents;
	bool32 next_clipboard_is_self;
	DWORD clipboard_sequence;
    
	Thread_Context main_thread;
    
    Thread_Group groups[THREAD_GROUP_COUNT];
    Work_Queue queues[THREAD_GROUP_COUNT];
    HANDLE locks[LOCK_COUNT];
    HANDLE DEBUG_sysmem_lock;
    Thread_Memory *thread_memory;

    HMODULE custom;
    
    i64 performance_frequency;
    i64 start_pcount;
};

globalvar Win32_Vars win32vars;
globalvar Application_Memory win32memory;

internal void
_OutDbgStr(u8 *msg){
	OutputDebugString((char*)msg);
}

internal void
system_fatal_error(u8 *message){
	MessageBox(0, (char*)message, "4ed Error", MB_OK|MB_ICONERROR);
}

internal File_Data
system_load_file(u8 *filename){
    File_Data result = {};
    HANDLE file;
    file = CreateFile((char*)filename, GENERIC_READ, 0, 0,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (!file){
        return result;
    }
    
    DWORD lo, hi;
    lo = GetFileSize(file, &hi);
    
    if (hi != 0){
        CloseHandle(file);
        return result;
    }
    
    result.size = (lo) + (((u64)hi) << 32);
    result.data = system_get_memory(result.size);
    
    if (!result.data){
        CloseHandle(file);
        result = {};
        return result;
    }
    
    DWORD read_size;
    BOOL read_result = ReadFile(file, result.data, result.size,
                                &read_size, 0);
    if (!read_result || read_size != result.size){
        CloseHandle(file);
        system_free_memory(result.data);
        result = {};
        return result;
    }
    
    CloseHandle(file);
    return result;
}

internal bool32
system_save_file(u8 *filename, void *data, i32 size){
	HANDLE file;
	file = CreateFile((char*)filename, GENERIC_WRITE, 0, 0,
					  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	
	if (!file){
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

internal Time_Stamp
system_file_time_stamp(u8 *filename){
    Time_Stamp result;
    result = {};
    
    FILETIME last_write;
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesEx((char*)filename, GetFileExInfoStandard, &data)){
        last_write = data.ftLastWriteTime;
        
        result.time = ((u64)last_write.dwHighDateTime << 32) | last_write.dwLowDateTime;
        result.success = 1;
    }
    
    return result;
}

internal u64
system_get_now(){
    u64 result;
    SYSTEMTIME sys_now;
    FILETIME file_now;
    GetSystemTime(&sys_now);
    SystemTimeToFileTime(&sys_now, &file_now);
    result = ((u64)file_now.dwHighDateTime << 32) | file_now.dwLowDateTime;
    return result;
}

internal void
system_free_file(File_Data data){
    system_free_memory(data.data);
}

internal i32
system_get_working_directory(u8 *destination, i32 max_size){
	DWORD required = GetCurrentDirectory(0, 0);
	if ((i32) required > max_size){
		// TODO(allen): WHAT NOW? Not enough space in destination for
		// current directory. Two step approach perhaps?
		return 0;
	}
	DWORD written = GetCurrentDirectory(max_size, (char*)destination);
	return (i32)written;
}

internal i32
system_get_easy_directory(u8 *destination){
	persist char easydir[] = "C:\\";
	for (i32 i = 0; i < ArrayCount(easydir); ++i){
		destination[i] = easydir[i];
	}
	return ArrayCount(easydir)-1;
}

internal File_List
system_get_files(String directory){
    File_List result = {};
    
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
            
            result.block = system_get_memory(count + file_count * sizeof(File_Info));
            result.infos = (File_Info*)result.block;
            char *name = (char*)(result.infos + file_count);
            if (result.block){
                search = FindFirstFileA(c_str_dir, &find_data);
                
                if (search != INVALID_HANDLE_VALUE){
                    File_Info *info = result.infos;
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
                    
                    result.count = file_count;
                    
                }else{
                    system_free_memory(result.block);
                    result = {};
                }
            }
        }
    }
    
	return result;
}

internal void
system_free_file_list(File_List list){
    system_free_memory(list.block);
}

#if FRED_INTERNAL
Sys_Bubble INTERNAL_sentinel;

internal Bubble*
INTERNAL_system_sentinel(){
    return &INTERNAL_sentinel;
}
#endif

internal void*
system_get_memory_(i32 size, i32 line_number, char *file_name){
	void *ptr = 0;
    
#if FRED_INTERNAL
    ptr = VirtualAlloc(0, size + sizeof(Sys_Bubble), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    Sys_Bubble *bubble = (Sys_Bubble*)ptr;
    bubble->flags = MEM_BUBBLE_SYS_DEBUG;
    bubble->line_number = line_number;
    bubble->file_name = file_name;
    bubble->size = size;
    WaitForSingleObject(win32vars.DEBUG_sysmem_lock, INFINITE);
    insert_bubble(&INTERNAL_sentinel, bubble);
    ReleaseSemaphore(win32vars.DEBUG_sysmem_lock, 1, 0);
    ptr = bubble + 1;
#else
    ptr = VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#endif
    
	return ptr;
}

internal void
system_free_memory(void *block){
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

internal i64
system_time(){
	i64 result = 0;
	LARGE_INTEGER time;
	if (QueryPerformanceCounter(&time)){
		result = (i64)(time.QuadPart - win32vars.start_pcount) * 1000000 / win32vars.performance_frequency;
	}
	return result;
}

// TODO(allen): Probably best to just drop all system functions here again.
internal void
system_post_clipboard(String str){
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

#if SOFTWARE_RENDER
internal void
Win32RedrawScreen(HDC hdc){
	win32vars.bmp_info.bmiHeader.biHeight =
		-win32vars.bmp_info.bmiHeader.biHeight;
	SetDIBitsToDevice(hdc,
					  0, 0,
					  win32vars.width, win32vars.height,
					  0, 0,
					  0, win32vars.height,
					  win32vars.pixel_data,
					  &win32vars.bmp_info,
					  DIB_RGB_COLORS);
	win32vars.bmp_info.bmiHeader.biHeight =
		-win32vars.bmp_info.bmiHeader.biHeight;
}
#else
internal void
Win32RedrawScreen(HDC hdc){
    glFlush();
    SwapBuffers(hdc);
}
#endif

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

internal void
Win32KeyboardHandle(bool8 current_state, bool8 previous_state, WPARAM wParam){
    switch (wParam){
    case VK_LSHIFT:
    case VK_RSHIFT:
    case VK_SHIFT:
    {
        win32vars.input_data.control_keys[CONTROL_KEY_SHIFT] = current_state;
    }break;
    case VK_LCONTROL:
    case VK_RCONTROL:
    case VK_CONTROL:
    {
        win32vars.input_data.control_keys[CONTROL_KEY_CONTROL] = current_state;
    }break;
    case VK_LMENU:
    case VK_RMENU:
    case VK_MENU:
    {
        win32vars.input_data.control_keys[CONTROL_KEY_ALT] = current_state;
    }break;
    default:
    {
        u16 key = keycode_lookup((u8)wParam);
        if (key != -1){
            if (current_state & !previous_state){
                i32 count = win32vars.input_data.press_count;
                if (count < KEY_INPUT_BUFFER_SIZE){
                    win32vars.input_data.press[count].keycode = key;
                    win32vars.input_data.press[count].loose_keycode = loose_keycode_lookup((u8)wParam);
                    ++win32vars.input_data.press_count;
                }
            }
            else if (current_state){
                i32 count = win32vars.input_data.hold_count;
                if (count < KEY_INPUT_BUFFER_SIZE){
                    win32vars.input_data.hold[count].keycode = key;
                    win32vars.input_data.hold[count].loose_keycode = loose_keycode_lookup((u8)wParam);
                    ++win32vars.input_data.hold_count;
                }
            }
        }
    }break;
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
        bool8 previous_state, current_state;
        previous_state = ((lParam & Bit_30)?(1):(0));
        current_state = ((lParam & Bit_31)?(0):(1));
        Win32KeyboardHandle(current_state, previous_state, wParam);
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
        for (int i = 0; i < CONTROL_KEY_COUNT; ++i){
            win32vars.input_data.control_keys[i] = 0;
        }
    }break;
    
    case WM_SETFOCUS:
    {
        win32vars.focus = 1;
    }break;
    
    case WM_SIZE:
    {
#if SOFTWARE_RENDER
        i32 new_width = LOWORD(lParam);
        i32 new_height = HIWORD(lParam);
        i32 new_pitch = new_width * 4;
        
        if (new_height*new_pitch > win32vars.true_pixel_size){
            system_free_memory(win32vars.pixel_data);
            
            win32vars.pixel_data = system_get_memory(new_height*new_pitch);
            win32vars.true_pixel_size = new_height*new_pitch;
            
            if (!win32vars.pixel_data){
                FatalError("Failure allocating new screen memory");
                win32vars.keep_playing = 0;
            }
        }
        
        win32vars.width = new_width;
        win32vars.height = new_height;
        win32vars.pitch = new_pitch;
        
        win32vars.bmp_info.bmiHeader.biWidth = win32vars.width;
        win32vars.bmp_info.bmiHeader.biHeight = win32vars.height;
#else
        if (win32vars.target.handle){
            i32 new_width = LOWORD(lParam);
            i32 new_height = HIWORD(lParam);
            
            Win32Resize(new_width, new_height);
        }
#endif
    }break;
    
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        
        Clipboard_Contents empty_contents = {};
#if FRED_INTERNAL
        INTERNAL_collecting_events = 0;
#endif
        app_step(&win32vars.main_thread,
                 &win32vars.key_codes,
                 &win32vars.previous_data, &win32vars.mouse,
                 0, &win32vars.target, &win32memory, empty_contents, 0, 1);
#if FRED_INTERNAL
        INTERNAL_collecting_events = 1;
#endif
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

#define THREAD_NOT_ASSIGNED 0xFFFFFFFF

#define JOB_ID_WRAP (ArrayCount(queue->jobs) * 4)
#define QUEUE_WRAP (ArrayCount(queue->jobs))

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
                    Thread_Memory *thread_memory = 0;
                    if (full_job->job.memory_request != 0){
                        thread_memory = win32vars.thread_memory + thread->id - 1;
                        if (thread_memory->size < full_job->job.memory_request){
                            if (thread_memory->data){
                                system_free_memory(thread_memory->data);
                            }
                            i32 new_size = LargeRoundUp(full_job->job.memory_request, Kbytes(4));
                            thread_memory->data = system_get_memory(new_size);
                            thread_memory->size = new_size;
                        }
                    }
                    full_job->job.callback(thread, thread_memory, full_job->job.data);
                    full_job->running_thread = 0;
                    thread->running = 0;
                }
            }
        }
        else{
            WaitForSingleObject(queue->semaphore, INFINITE);
        }
    }
}

internal bool32
Win32JobIsPending(Work_Queue *queue, u32 job_id){
    bool32 result;
    u32 job_index;
    Full_Job_Data *full_job;
    
    job_index = job_id % QUEUE_WRAP;
    full_job = queue->jobs + job_index;
    
    Assert(full_job->id == job_id);
    
    result = 0;
    if (full_job->running_thread != 0){
        result = 1;
    }
    
    return result;
}

internal u32
system_thread_get_id(Thread_Context *thread){
    return thread->id;
}

internal u32
system_thread_current_job_id(Thread_Context *thread){
    return thread->job_id;
}

internal u32
system_post_job(Thread_Group_ID group_id, Job_Data job){
    Work_Queue *queue = win32vars.queues + group_id;
    
    Assert((queue->write_position + 1) % QUEUE_WRAP != queue->read_position % QUEUE_WRAP);
    
    bool32 success = 0;
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
    
    ReleaseSemaphore(queue->semaphore, 1, 0);
    
    return result;
}

internal void
system_cancel_job(Thread_Group_ID group_id, u32 job_id){
    Work_Queue *queue = win32vars.queues + group_id;
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
        system_aquire_lock(CANCEL_LOCK0 + thread_id - 1);
        thread = group->threads + thread_id - 1;
        TerminateThread(thread->handle, 0);
        u32 creation_flag = 0;
        thread->handle = CreateThread(0, 0, ThreadProc, thread, creation_flag, (LPDWORD)&thread->windows_id);
        system_release_lock(CANCEL_LOCK0 + thread_id - 1);
        thread->running = 0;
    }
}

internal bool32
system_job_is_pending(Thread_Group_ID group_id, u32 job_id){
    Work_Queue *queue = win32vars.queues + group_id;;
    return Win32JobIsPending(queue, job_id);
}

internal void
system_aquire_lock(Lock_ID id){
    WaitForSingleObject(win32vars.locks[id], INFINITE);
}

internal void
system_release_lock(Lock_ID id){
    ReleaseSemaphore(win32vars.locks[id], 1, 0);
}

internal void
system_aquire_lock(i32 id){
    WaitForSingleObject(win32vars.locks[id], INFINITE);
}

internal void
system_release_lock(i32 id){
    ReleaseSemaphore(win32vars.locks[id], 1, 0);
}

internal void
system_grow_thread_memory(Thread_Memory *memory){
    system_aquire_lock(CANCEL_LOCK0 + memory->id - 1);
    void *old_data = memory->data;
    i32 old_size = memory->size;
    i32 new_size = LargeRoundUp(memory->size*2, Kbytes(4));
    memory->data = system_get_memory(new_size);
    memory->size = new_size;
    if (old_data){
        memcpy(memory->data, old_data, old_size);
        system_free_memory(old_data);
    }
    system_release_lock(CANCEL_LOCK0 + memory->id - 1);
}

internal void
system_force_redraw(){
    InterlockedExchange(&win32vars.force_redraw, 1);
}

#if FRED_INTERNAL
internal void
INTERNAL_get_thread_states(Thread_Group_ID id, bool8 *running, i32 *pending){
    Work_Queue *queue = win32vars.queues + id;
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

int
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow){
    win32vars = {};
    TEMP = {};
    LARGE_INTEGER lpf;
    QueryPerformanceFrequency(&lpf);
    win32vars.performance_frequency = lpf.QuadPart;
    QueryPerformanceCounter(&lpf);
    win32vars.start_pcount = lpf.QuadPart;
    
#if FRED_INTERNAL
    memset(INTERNAL_event_hits, 0, INTERNAL_event_index_count * sizeof(u32));
    INTERNAL_frame_index = 0;
    INTERNAL_updating_profile = 1;
    INTERNAL_collecting_events = 1;
    
    INTERNAL_sentinel.next = &INTERNAL_sentinel;
    INTERNAL_sentinel.prev = &INTERNAL_sentinel;
    INTERNAL_sentinel.flags = MEM_BUBBLE_SYS_DEBUG;
#endif
    
    keycode_init(&win32vars.key_codes, &win32vars.loose_codes);
    
#ifdef FRED_SUPER
    win32vars.custom = LoadLibraryA("4coder_custom.dll");
    if (win32vars.custom){
        TEMP.get_bindings = (Get_Binding_Data_Function*)
            GetProcAddress(win32vars.custom, "get_bindings");
        
        TEMP.set_extra_font = (Set_Extra_Font_Function*)
            GetProcAddress(win32vars.custom, "set_extra_font");
    }
#endif
    
    Thread_Context background[4];
    memset(background, 0, sizeof(background));
    win32vars.groups[BACKGROUND_THREADS].threads = background;
    win32vars.groups[BACKGROUND_THREADS].count = ArrayCount(background);
    
    Thread_Memory thread_memory[ArrayCount(background)];
    win32vars.thread_memory = thread_memory;
    
    win32vars.queues[BACKGROUND_THREADS].semaphore =
        CreateSemaphore(0, 0, win32vars.groups[BACKGROUND_THREADS].count, 0);
    
    u32 creation_flag = 0;
    for (i32 i = 0; i < win32vars.groups[BACKGROUND_THREADS].count; ++i){
        Thread_Context *thread = win32vars.groups[BACKGROUND_THREADS].threads + i;
        thread->id = i + 1;
        
        Thread_Memory *memory = win32vars.thread_memory + i;
        *memory = {};
        memory->id = thread->id;
        
        thread->queue = &win32vars.queues[BACKGROUND_THREADS];
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
		// TODO(allen): diagnostics
		FatalError("Failed to create window class");
		return 1;
	}
    
    RECT window_rect = {};
    window_rect.right = 800;
    window_rect.bottom = 600;
    
    if (!AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, false)){
        // TODO(allen): non-fatal diagnostics
    }
    
#if SOFTWARE_RENDER
#define WINDOW_NAME "4coder-softrender-window"
#else
#define WINDOW_NAME "4coder-window"
#endif
    
    HWND window_handle = {};
    window_handle = CreateWindowA(
        window_class.lpszClassName,
        WINDOW_NAME,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT/*x*/, CW_USEDEFAULT/*y*/,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        0, 0, hInstance, 0);
    
    if (!window_handle){
        // TODO(allen): diagnostics
        FatalError("Failed to create window");
        return 2;
    }
    
    // TODO(allen): errors?
    win32vars.window_handle = window_handle;
    HDC hdc = GetDC(window_handle);
    
    GetClientRect(window_handle, &window_rect);
    
#if SOFTWARE_RENDER
    win32vars.width = window_rect.right - window_rect.left;
    win32vars.height = window_rect.bottom - window_rect.top;
    
    win32vars.pitch = win32vars.width*4;
#define bmi_header win32vars.bmp_info.bmiHeader
    bmi_header = {};
    bmi_header.biSize = sizeof(BITMAPINFOHEADER);
    bmi_header.biWidth = win32vars.width;
    bmi_header.biHeight = win32vars.height;
    bmi_header.biPlanes = 1;
    bmi_header.biBitCount = 32;
    bmi_header.biCompression = BI_RGB;
#undef bmi_header
    
	win32vars.true_pixel_size = win32vars.height*win32vars.pitch;
	win32vars.pixel_data = system_get_memory(win32vars.true_pixel_size);
    
	if (!win32vars.pixel_data){
		FatalError("Failure allocating screen memory");
		return 3;
	}
#else
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
#endif
    
    LPVOID base;
#if FRED_INTERNAL
    base = (LPVOID)Tbytes(1);
#else
    base = (LPVOID)0;
#endif
    
	win32memory.vars_memory_size = Mbytes(2);
    win32memory.vars_memory = VirtualAlloc(base, win32memory.vars_memory_size,
                                           MEM_COMMIT | MEM_RESERVE,
                                           PAGE_READWRITE);
    
#if FRED_INTERNAL
    base = (LPVOID)Tbytes(2);
#else
    base = (LPVOID)0;
#endif
    win32memory.target_memory_size = Mbytes(512);
    win32memory.target_memory = VirtualAlloc(base, win32memory.target_memory_size,
                                             MEM_COMMIT | MEM_RESERVE,
                                             PAGE_READWRITE);
    
    if (!win32memory.vars_memory){
        FatalError("Failure allocating application memory");
        return 4;
    }
    
    win32vars.clipboard_sequence = GetClipboardSequenceNumber();
    
    if (win32vars.clipboard_sequence == 0){
        system_post_clipboard(make_lit_string(""));
        
        win32vars.clipboard_sequence = GetClipboardSequenceNumber();
        win32vars.next_clipboard_is_self = 0;
        
        if (win32vars.clipboard_sequence == 0){
            FatalError("Failure to access platform's clipboard");
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
    
	if (!app_init(&win32vars.main_thread,
				  &win32memory, &win32vars.key_codes,
                  win32vars.clipboard_contents)){
		return 5;
	}
	
	win32vars.keep_playing = 1;
	timeBeginPeriod(1);
	
    system_aquire_lock(FRAME_LOCK);
    Thread_Context *thread = &win32vars.main_thread;
    AllowLocal(thread);
	bool32 first = 1;
	i64 timer_start = system_time();
	while (win32vars.keep_playing){
#if FRED_INTERNAL
        i64 dbg_procing_start = system_time();
        if (!first){
            if (INTERNAL_updating_profile){
                i32 j = (INTERNAL_frame_index % 30);
                Profile_Frame *frame = past_frames + j;
                
                sort(&profile_frame.events);
                
                frame->events.count = profile_frame.events.count;
                memcpy(frame->events.e, profile_frame.events.e, sizeof(Debug_Event)*profile_frame.events.count);
                
                past_frames[j].dbg_procing_start = profile_frame.dbg_procing_start;
                past_frames[j].dbg_procing_end = profile_frame.dbg_procing_end;
                past_frames[j].index = profile_frame.index;
                past_frames[j].first_key = profile_frame.first_key;
                
                ++INTERNAL_frame_index;
                if (INTERNAL_frame_index < 0){
                    INTERNAL_frame_index = ((INTERNAL_frame_index - 1) % 30) + 1;
                }
                memset(INTERNAL_event_hits, 0, INTERNAL_event_index_count * sizeof(u32));
            }
        }
        profile_frame.events.count = 0;
        profile_frame.first_key = -1;
        profile_frame.index = INTERNAL_frame_index;
        INTERNAL_frame_start_time = timer_start;
        profile_frame.dbg_procing_start = (i32)(dbg_procing_start - INTERNAL_frame_start_time);
        profile_frame.dbg_procing_end = (i32)(system_time() - INTERNAL_frame_start_time);
#endif
        
        ProfileStart(OS_input);
		win32vars.previous_data = win32vars.input_data;
		win32vars.input_data.press_count = 0;
		win32vars.input_data.hold_count = 0;
		win32vars.input_data.caps_lock = GetKeyState(VK_CAPITAL) & 0x1;
		win32vars.mouse.left_button_prev = win32vars.mouse.left_button;
		win32vars.mouse.right_button_prev = win32vars.mouse.right_button;
		win32vars.mouse.wheel = 0;
		
		MSG msg;
		while (PeekMessage(&msg, window_handle, 0, 0, PM_REMOVE) && win32vars.keep_playing){
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
		
		bool32 shift = win32vars.input_data.control_keys[CONTROL_KEY_SHIFT];
		bool32 caps_lock = win32vars.input_data.caps_lock;
        for (i32 i = 0; i < win32vars.input_data.press_count; ++i){
            i16 keycode = win32vars.input_data.press[i].keycode;
			win32vars.input_data.press[i].character =
				keycode_to_character_ascii(&win32vars.key_codes, keycode,
										   shift, caps_lock);
            
			win32vars.input_data.press[i].character_no_caps_lock =
				keycode_to_character_ascii(&win32vars.key_codes, keycode,
										   shift, 0);
		}
        
        for (i32 i = 0; i < win32vars.input_data.hold_count; ++i){
            i16 keycode = win32vars.input_data.hold[i].keycode;
			win32vars.input_data.hold[i].character =
				keycode_to_character_ascii(&win32vars.key_codes, keycode,
										   shift, caps_lock);
			
			win32vars.input_data.hold[i].character_no_caps_lock =
				keycode_to_character_ascii(&win32vars.key_codes, keycode,
										   shift, 0);
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
        
        i32 redraw = InterlockedExchange(&win32vars.force_redraw, 0);
        ProfileEnd(OS_input);
        
		Application_Step_Result result =
			app_step(&win32vars.main_thread,
					 &win32vars.key_codes,
					 &win32vars.input_data, &win32vars.mouse,
					 1, &win32vars.target,
					 &win32memory,
					 win32vars.clipboard_contents,
					 first, redraw);
        
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
        
        ProfileStart(frame_sleep);
		i64 timer_end = system_time();
		i64 end_target = (timer_start + FRAME_TIME);
        
        system_release_lock(FRAME_LOCK);
		while (timer_end < end_target){
            DWORD samount = (DWORD)((end_target - timer_end) / 1000);
            if (samount > 0) Sleep(samount);
            timer_end = system_time();
		}
        system_aquire_lock(FRAME_LOCK);
		timer_start = system_time();
        ProfileEnd(frame_sleep);
	}
	
	return 0;
}

#if FRED_INTERNAL
const i32 INTERNAL_event_index_count = __COUNTER__;
u32 INTERNAL_event_hits[__COUNTER__];
#endif

// BOTTOM

