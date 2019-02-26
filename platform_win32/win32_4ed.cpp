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
#include "4coder_base_types.h"
#include "4coder_API/4coder_version.h"

#include <string.h>
#include "4coder_lib/4coder_utf8.h"

#if defined(FRED_SUPER)
# include "4coder_lib/4coder_arena.h"
# include "4coder_lib/4coder_heap.h"
# include "4coder_lib/4coder_arena.cpp"
# include "4coder_lib/4coder_heap.cpp"
# define FSTRING_IMPLEMENTATION
# include "4coder_lib/4coder_string.h"

# include "4coder_API/4coder_keycodes.h"
# include "4coder_API/4coder_default_colors.h"
# include "4coder_API/4coder_types.h"
#else
# include "4coder_default_bindings.cpp"
#endif

#include "4coder_base_types.cpp"
//#include "4ed_math.h"

#include "4ed_font.h"
#include "4ed_system.h"
#include "4ed_log.h"
#include "4ed_render_target.h"
#include "4ed_render_format.h"
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
win32_output_error_string(i32 error_string_type);

//////////////////////////////

#include "win32_utf8.h"

#include "4ed_system_shared.h"

#include "4ed_shared_thread_constants.h"
#include "win32_threading_wrapper.h"

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
#include "4ed_coroutine.cpp"
#include "4ed_font.cpp"

#include "4ed_mem.cpp"
#include "4ed_hash_functions.cpp"

////////////////////////////////

typedef i32 Win32_Object_Kind;
enum{
    Win32ObjectKind_ERROR = 0,
    Win32ObjectKind_Timer = 1,
};

struct Win32_Object{
    Node node;
    Win32_Object_Kind kind;
    union{
        // NOTE(allen): Timer object
        struct{
            UINT_PTR id;
        } timer;
    };
};

struct Win32_Vars{
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
    
    String binary_path;
    
    u8 *clip_buffer;
    u32 clip_max;
    String clipboard_contents;
    b32 next_clipboard_is_self;
    DWORD clipboard_sequence;
    
    Partition clip_post_part;
    i32 clip_post_len;
    
    HWND window_handle;
    i32 dpi_x;
    i32 dpi_y;
    
    f64 count_per_usecond;
    b32 first;
    i32 running_cli;
    
    Node free_win32_objects;
    Node timer_objects;
    UINT_PTR timer_counter;
    
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
#include "4ed_shared_file_handling.cpp"

////////////////////////////////

internal void
system_schedule_step(){
    PostMessage(win32vars.window_handle, WM_4coder_ANIMATE, 0, 0);
}

////////////////////////////////

#include "4ed_work_queues.cpp"

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

#include "4ed_coroutine_functions.cpp"

#include "4ed_system_shared.cpp"

//
// File Change Listener
//

union Directory_Track_Node{
    struct{
        Directory_Track_Node *next;
        Directory_Track_Node *prev;
    };
    struct{
        OVERLAPPED overlapped;
        HANDLE dir_handle;
        char buffer[(32 << 10) + 12];
        String dir_name;
        i32 ref_count;
    };
};

union File_Track_Node{
    struct{
        File_Track_Node *next;
        File_Track_Node *prev;
    };
    struct{
        String file_name;
        i32 ref_count;
        Directory_Track_Node *parent_dir;
    };
};
#if !defined(CString_Key_Reference_GAURD)
#define CString_Key_Reference_GAURD
struct CString_Key_Reference{
    char*key;
    i32 size;
};
#endif
struct CString_Ptr_Lookup_Result{
    b32 success;
    void **val;
};
struct CString_Ptr_Table{
    void *mem;
    u64 *hashes;
    CString_Key_Reference*keys;
    void **vals;
    i32 count;
    i32 dirty_slot_count;
    i32 max;
};

typedef i32 File_Track_Instruction;
enum{
    FileTrackInstruction_None,
    FileTrackInstruction_BeginTracking,
    FileTrackInstruction_Cancel,
};

union File_Track_Instruction_Node{
    struct{
        File_Track_Instruction instruction;
        Directory_Track_Node *dir_node;
    };
    struct{
        File_Track_Instruction_Node *next;
        File_Track_Instruction_Node *prev;
    };
};

struct File_Track_Note_Node{
    File_Track_Note_Node *next;
    File_Track_Note_Node *prev;
    String file_name;
};

Heap file_track_heap = {};
Partition file_track_scratch = {};

CString_Ptr_Table file_track_dir_table = {};
Directory_Track_Node *file_track_dir_free_first = 0;
Directory_Track_Node *file_track_dir_free_last = 0;
CString_Ptr_Table file_track_table = {};
File_Track_Node *file_track_free_first = 0;
File_Track_Node *file_track_free_last = 0;
File_Track_Instruction_Node *file_track_ins_free_first = 0;
File_Track_Instruction_Node *file_track_ins_free_last = 0;
File_Track_Note_Node *file_track_note_first = 0;
File_Track_Note_Node *file_track_note_last = 0;
File_Track_Note_Node *file_track_note_free_first = 0;
File_Track_Note_Node *file_track_note_free_last = 0;

CRITICAL_SECTION file_track_critical_section;
HANDLE file_track_iocp;
HANDLE file_track_thread;

global_const u32 file_track_flags = 0
|FILE_NOTIFY_CHANGE_FILE_NAME
|FILE_NOTIFY_CHANGE_DIR_NAME
|FILE_NOTIFY_CHANGE_ATTRIBUTES
|FILE_NOTIFY_CHANGE_SIZE
|FILE_NOTIFY_CHANGE_LAST_WRITE
|FILE_NOTIFY_CHANGE_CREATION
|FILE_NOTIFY_CHANGE_SECURITY
;

////////////////////////////////

internal CString_Ptr_Table
make_CString_Ptr_table(void *mem, umem size){
    CString_Ptr_Table table = {};
    i32 max = (i32)(size/32ULL);
    if (max > 0){
        table.mem = mem;
        u8 *cursor = (u8*)mem;
        table.hashes = (u64*)cursor;
        cursor += 8*max;
        table.keys = (CString_Key_Reference*)cursor;
        cursor += 16*max;
        table.vals = (void **)cursor;
        table.count = 0;
        table.max = max;
        block_fill_ones(table.hashes, sizeof(*table.hashes)*max);
    }
    return(table);
}

internal i32
max_to_memsize_CString_Ptr_table(i32 max){
    return(max*32ULL);
}

internal b32
at_max_CString_Ptr_table(CString_Ptr_Table *table){
    if (table->max > 0 && (table->count + 1)*8 <= table->max*7){
        return(false);
    }
    return(true);
}

internal b32
insert_CString_Ptr_table(CString_Ptr_Table *table, char*key, i32 key_size, void **val){
    i32 max = table->max;
    if (max > 0){
        i32 count = table->count;
        if ((count + 1)*8 <= max*7){
            u64 hash = table_hash_u8((u8*)key, key_size);
            if (hash >= 18446744073709551614ULL){ hash += 2; }
            i32 first_index = hash%max;
            i32 index = first_index;
            u64 *hashes = table->hashes;
            for (;;){
                if (hashes[index] == 18446744073709551615ULL){
                    table->dirty_slot_count += 1;
                }
                if (hashes[index] == 18446744073709551615ULL || hashes[index] == 18446744073709551614ULL){
                    hashes[index] = hash;
                    CString_Key_Reference new_key = {key, key_size};
                    table->keys[index] = new_key;
                    table->vals[index] = *val;
                    table->count += 1;
                    return(true);
                }
                if (hashes[index] == hash) return(false);
                index = (index + 1)%max;
                if (index == first_index) return(false);
            }
        }
    }
    return(false);
}

internal CString_Ptr_Lookup_Result
lookup_CString_Ptr_table(CString_Ptr_Table *table, char*key, i32 key_size){
    CString_Ptr_Lookup_Result result = {};
    i32 max = table->max;
    if (max > 0){
        u64 hash = table_hash_u8((u8*)key, key_size);
        if (hash >= 18446744073709551614ULL){ hash += 2; }
        i32 first_index = hash%max;
        i32 index = first_index;
        u64 *hashes = table->hashes;
        for (;;){
            if (hashes[index] == 18446744073709551615ULL) break;
            if (hashes[index] == hash){
                CString_Key_Reference *key_check = &table->keys[index];
                b32 key_match = (key_size == key_check->size && block_compare(key, key_check->key, key_size*sizeof(*key)) == 0);
                if (key_match){
                    result.success = true;
                    result.val = &table->vals[index];
                    return(result);
                }
            }
            index = (index + 1)%max;
            if (index == first_index) break;
        }
    }
    return(result);
}

internal b32
erase_CString_Ptr_table(CString_Ptr_Table *table, char*key, i32 key_size){
    i32 max = table->max;
    if (max > 0 && table->count > 0){
        u64 hash = table_hash_u8((u8*)key, key_size);
        if (hash >= 18446744073709551614ULL){ hash += 2; }
        i32 first_index = hash%max;
        i32 index = first_index;
        u64 *hashes = table->hashes;
        for (;;){
            if (hashes[index] == 18446744073709551615ULL) break;
            if (hashes[index] == hash){
                CString_Key_Reference *key_check = &table->keys[index];
                b32 key_match = (key_size == key_check->size && block_compare(key, key_check->key, key_size*sizeof(*key)) == 0);
                if (key_match){
                    hashes[index] = 18446744073709551614ULL;
                    table->count -= 1;
                    return(true);
                }
            }
            index = (index + 1)%max;
            if (index == first_index) break;
        }
    }
    return(false);
}

internal b32
move_CString_Ptr_table(CString_Ptr_Table *dst_table, CString_Ptr_Table *src_table){
    if ((src_table->count + dst_table->count)*8 <= dst_table->max*7){
        i32 max = src_table->max;
        u64 *hashes = src_table->hashes;
        for (i32 index = 0; index < max; index += 1){
            if (hashes[index] != 18446744073709551615ULL && hashes[index] != 18446744073709551614ULL){
                char*key = src_table->keys[index].key;
                i32 key_size = src_table->keys[index].size;
                void **val = &src_table->vals[index];
                insert_CString_Ptr_table(dst_table, key, key_size, val);
            }
        }
        return(true);
    }
    return(false);
}

internal b32
lookup_CString_Ptr_table(CString_Ptr_Table *table, char *key, i32 key_size, void * *val_out){
    CString_Ptr_Lookup_Result result = lookup_CString_Ptr_table(table, key, key_size);
    if (result.success){
        *val_out = *result.val;
    }
    return(result.success);
}

internal b32
insert_CString_Ptr_table(CString_Ptr_Table *table, char*key, i32 key_size, void * val){
    return(insert_CString_Ptr_table(table, key, key_size, &val));
}

internal b32
alloc_insert_CString_Ptr_table(CString_Ptr_Table *table, char*key, i32 key_size, void *val){
    if (at_max_CString_Ptr_table(table)){
        i32 new_max = (table->max + 1)*2;
        i32 new_size = max_to_memsize_CString_Ptr_table(new_max);
        void *new_mem = system_memory_allocate(new_size);
        CString_Ptr_Table new_table = make_CString_Ptr_table(new_mem, new_size);
        if (table->mem != 0){
            i32 old_size = max_to_memsize_CString_Ptr_table(table->max);
            system_memory_free(table->mem, old_size);
        }
        *table = new_table;
    }
    return(insert_CString_Ptr_table(table, key, key_size, val));
}

////////////////////////////////

internal String
file_track_store_string_copy(String string){
    i32 alloc_size = string.size + 1;
    alloc_size = l_round_up_i32(alloc_size, 16);
    char *buffer = (char*)heap_allocate(&file_track_heap, alloc_size);
    if (buffer == 0){
        i32 size = MB(1);
        void *new_block = system_memory_allocate(size);
        heap_extend(&file_track_heap, new_block, size);
        buffer = (char*)heap_allocate(&file_track_heap, alloc_size);
    }
    Assert(buffer != 0);
    memcpy(buffer, string.str, string.size);
    buffer[string.size] = 0;
    return(make_string(buffer, string.size, alloc_size));
}

internal void
file_track_free_string(String string){
    Assert(string.str != 0);
    heap_free(&file_track_heap, string.str);
}

internal Directory_Track_Node*
file_track_store_new_dir_node(String dir_name_string, HANDLE dir_handle){
    if (file_track_dir_free_first == 0){
        u32 size = MB(1);
        void *new_block = system_memory_allocate(size);
        u32 count = size/sizeof(Directory_Track_Node);
        Directory_Track_Node *nodes = (Directory_Track_Node*)new_block;
        Directory_Track_Node *node = nodes;
        node->next = node + 1;
        node->prev = 0;
        node += 1;
        for (u32 i = 1; i < count - 1; i += 1, node += 1){
            node->next = node + 1;
            node->prev = node - 1;
        }
        node->next = 0;
        node->prev = node - 1;
        file_track_dir_free_first = nodes;
        file_track_dir_free_last = node;
    }
    Directory_Track_Node *new_node = file_track_dir_free_first;
    zdll_remove(file_track_dir_free_first, file_track_dir_free_last, new_node);
    alloc_insert_CString_Ptr_table(&file_track_dir_table, dir_name_string.str, dir_name_string.size, new_node);
    memset(&new_node->overlapped, 0, sizeof(new_node->overlapped));
    new_node->dir_handle = dir_handle;
    new_node->dir_name = file_track_store_string_copy(dir_name_string);
    new_node->ref_count = 0;
    return(new_node);
}

internal void
file_track_free_dir_node(Directory_Track_Node *node){
    erase_CString_Ptr_table(&file_track_dir_table, node->dir_name.str, node->dir_name.size);
    file_track_free_string(node->dir_name);
    memset(&node->dir_name, 0, sizeof(node->dir_name));
    zdll_push_back(file_track_dir_free_first, file_track_dir_free_last, node);
}

internal File_Track_Node*
file_track_store_new_file_node(String file_name_string, Directory_Track_Node *existing_dir_node){
    if (file_track_free_first == 0){
        u32 size = KB(16);
        void *new_block = system_memory_allocate(size);
        u32 count = size/sizeof(File_Track_Node);
        File_Track_Node *nodes = (File_Track_Node*)new_block;
        File_Track_Node *node = nodes;
        node->next = node + 1;
        node->prev = 0;
        node += 1;
        for (u32 i = 1; i < count - 1; i += 1, node += 1){
            node->next = node + 1;
            node->prev = node - 1;
        }
        node->next = 0;
        node->prev = node - 1;
        file_track_free_first = nodes;
        file_track_free_last = node;
    }
    File_Track_Node *new_node = file_track_free_first;
    zdll_remove(file_track_free_first, file_track_free_last, new_node);
    alloc_insert_CString_Ptr_table(&file_track_table, file_name_string.str, file_name_string.size, new_node);
    new_node->file_name = file_track_store_string_copy(file_name_string);
    new_node->ref_count = 1;
    new_node->parent_dir = existing_dir_node;
    existing_dir_node->ref_count += 1;
    return(new_node);
}

internal void
file_track_free_file_node(File_Track_Node *node){
    erase_CString_Ptr_table(&file_track_table, node->file_name.str, node->file_name.size);
    file_track_free_string(node->file_name);
    memset(&node->file_name, 0, sizeof(node->file_name));
    zdll_push_back(file_track_free_first, file_track_free_last, node);
}

internal File_Track_Note_Node*
file_track_store_new_note_node(String file_name){
    if (file_track_note_free_first == 0){
        u32 size = KB(16);
        void *new_block = system_memory_allocate(size);
        u32 count = size/sizeof(File_Track_Note_Node);
        File_Track_Note_Node *nodes = (File_Track_Note_Node*)new_block;
        File_Track_Note_Node *node = nodes;
        node->next = node + 1;
        node->prev = 0;
        node += 1;
        for (u32 i = 1; i < count - 1; i += 1, node += 1){
            node->next = node + 1;
            node->prev = node - 1;
        }
        node->next = 0;
        node->prev = node - 1;
        file_track_note_free_first = nodes;
        file_track_note_free_last = node;
    }
    File_Track_Note_Node *new_node = file_track_note_free_first;
    zdll_remove(file_track_note_free_first, file_track_note_free_last, new_node);
    zdll_push_back(file_track_note_first, file_track_note_last, new_node);
    new_node->file_name = file_track_store_string_copy(file_name);
    return(new_node);
}

internal void
file_track_free_note_node(File_Track_Note_Node *node){
    file_track_free_string(node->file_name);
    memset(&node->file_name, 0, sizeof(node->file_name));
    zdll_remove(file_track_note_first, file_track_note_last, node);
    zdll_push_back(file_track_note_free_first, file_track_note_free_last, node);
}

internal File_Track_Instruction_Node*
file_track_new_instruction_node(){
    if (file_track_ins_free_first == 0){
        u32 size = KB(16);
        void *new_block = system_memory_allocate(size);
        u32 count = size/sizeof(File_Track_Instruction_Node);
        File_Track_Instruction_Node *nodes = (File_Track_Instruction_Node*)new_block;
        File_Track_Instruction_Node *node = nodes;
        node->next = node + 1;
        node->prev = 0;
        node += 1;
        for (u32 i = 1; i < count - 1; i += 1, node += 1){
            node->next = node + 1;
            node->prev = node - 1;
        }
        node->next = 0;
        node->prev = node - 1;
        file_track_ins_free_first = nodes;
        file_track_ins_free_last = node;
    }
    File_Track_Instruction_Node *new_node = file_track_ins_free_first;
    zdll_remove(file_track_ins_free_first, file_track_ins_free_last, new_node);
    return(new_node);
}

internal void
file_track_free_instruction_node(File_Track_Instruction_Node *node){
    zdll_push_back(file_track_ins_free_first, file_track_ins_free_last, node);
}

internal Directory_Track_Node*
file_track_dir_lookup(String dir_name_string){
    void *ptr = 0;
    lookup_CString_Ptr_table(&file_track_dir_table, dir_name_string.str, dir_name_string.size, &ptr);
    return((Directory_Track_Node*)ptr);
}

internal File_Track_Node*
file_track_file_lookup(String file_name_string){
    void *ptr = 0;
    lookup_CString_Ptr_table(&file_track_table, file_name_string.str, file_name_string.size, &ptr);
    return((File_Track_Node*)ptr);
}

internal DWORD CALL_CONVENTION
file_track_worker(void*){
    for (;;){
        DWORD number_of_bytes = 0;
        ULONG_PTR key = 0;
        OVERLAPPED *overlapped = 0;
        if (GetQueuedCompletionStatus(file_track_iocp, &number_of_bytes, &key, &overlapped, INFINITE)){
            EnterCriticalSection(&file_track_critical_section);
            if (number_of_bytes == 0 && key == 0){
                File_Track_Instruction_Node *instruction = (File_Track_Instruction_Node*)overlapped;
                switch (instruction->instruction){
                    case FileTrackInstruction_None:
                    {}break;
                    case FileTrackInstruction_BeginTracking:
                    {
                        Directory_Track_Node *dir_node = instruction->dir_node;
                        CreateIoCompletionPort(dir_node->dir_handle, file_track_iocp, (ULONG_PTR)&dir_node->overlapped, 1);
                        ReadDirectoryChangesW(dir_node->dir_handle, dir_node->buffer, sizeof(dir_node->buffer), FALSE,
                                              file_track_flags, 0, &dir_node->overlapped, 0);
                    }break;
                    case FileTrackInstruction_Cancel:
                    {
                        Directory_Track_Node *dir_node = instruction->dir_node;
                        CancelIo(dir_node->dir_handle);
                        CloseHandle(dir_node->dir_handle);
                        file_track_free_dir_node(dir_node);
                    }break;
                }
                file_track_free_instruction_node(instruction);
            }
            else if (number_of_bytes != 0 && key != 0){
                Directory_Track_Node *dir_node = (Directory_Track_Node*)overlapped;
                Directory_Track_Node node_copy = *dir_node;
                memset(&dir_node->overlapped, 0, sizeof(dir_node->overlapped));
                ReadDirectoryChangesW(dir_node->dir_handle, dir_node->buffer, sizeof(dir_node->buffer), FALSE,
                                      file_track_flags, 0, &dir_node->overlapped, 0);
                
                FILE_NOTIFY_INFORMATION *info = (FILE_NOTIFY_INFORMATION*)node_copy.buffer;
                
                i32 len = info->FileNameLength/2;
                i32 dir_len = GetFinalPathNameByHandle_utf8(&file_track_scratch, dir_node->dir_handle, 0, 0, FILE_NAME_NORMALIZED);
                
                i32 req_size = dir_len + (len + 1)*2 + 4;
                
                Temp_Memory temp = begin_temp_memory(&file_track_scratch);
                u8 *buffer = push_array(&file_track_scratch, u8, req_size);
                
                if (buffer != 0){
                    u32 path_pos = GetFinalPathNameByHandle_utf8(&file_track_scratch, dir_node->dir_handle, buffer, req_size, FILE_NAME_NORMALIZED);
                    buffer[path_pos] = '\\';
                    path_pos += 1;
                    
                    u32 name_max = req_size - path_pos;
                    u8 *name_buffer = buffer + path_pos;
                    
                    b32 convert_error = false;
                    u32 name_pos = (u32)utf16_to_utf8_minimal_checking(name_buffer, name_max, (u16*)info->FileName, len, &convert_error);
                    if (name_pos < name_max && !convert_error){
                        u32 pos = path_pos + name_pos;
                        if (buffer[0] == '\\'){
                            for (u32 i = 0; i + 4 < pos; i += 1){
                                buffer[i] = buffer[i + 4];
                            }
                            pos -= 4;
                        }
                        String file_name = make_string((char*)buffer, pos);
                        file_track_store_new_note_node(file_name);
                        
                        system_schedule_step();
                    }
                }
                
                end_temp_memory(temp);
            }
            LeaveCriticalSection(&file_track_critical_section);
        }
    }
}

internal void
file_track_init(){
    heap_init(&file_track_heap);
    i32 scratch_size = KB(128);
    file_track_scratch = make_part(system_memory_allocate(scratch_size), scratch_size);
    InitializeCriticalSection(&file_track_critical_section);
    file_track_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 1);
    file_track_thread = CreateThread(0, 0, file_track_worker, 0, 0, 0);
}

internal
Sys_Add_Listener_Sig(system_add_listener){
    b32 added_new_listener = false;
    
    EnterCriticalSection(&file_track_critical_section);
    
    String file_name_string = make_string_slowly(filename);
    File_Track_Node *existing_file_node = file_track_file_lookup(file_name_string);
    
    if (existing_file_node != 0){
        existing_file_node->ref_count += 1;
        added_new_listener = true;
    }
    else{
        String dir_name_string = path_of_directory(file_name_string);
        Directory_Track_Node *existing_dir_node = file_track_dir_lookup(dir_name_string);
        
        if (existing_dir_node == 0){
            Temp_Memory temp = begin_temp_memory(&file_track_scratch);
            String dir_name_string_terminated = string_push_copy(&file_track_scratch, dir_name_string);
            HANDLE dir_handle = CreateFile_utf8(&file_track_scratch, (u8*)dir_name_string_terminated.str,
                                                FILE_LIST_DIRECTORY,
                                                FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, 0,
                                                OPEN_EXISTING,
                                                FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED, 0);
            
            if (dir_handle != 0 && dir_handle != INVALID_HANDLE_VALUE){
                Directory_Track_Node *new_node = file_track_store_new_dir_node(dir_name_string, dir_handle);
                File_Track_Instruction_Node *instruction_node = file_track_new_instruction_node();
                instruction_node->instruction = FileTrackInstruction_BeginTracking;
                instruction_node->dir_node = new_node;
                PostQueuedCompletionStatus(file_track_iocp, 0, 0, (LPOVERLAPPED)instruction_node);
                existing_dir_node = new_node;
            }
            
            end_temp_memory(temp);
        }
        
        if (existing_dir_node != 0){
            file_track_store_new_file_node(file_name_string, existing_dir_node);
            added_new_listener = true;
        }
    }
    
    LeaveCriticalSection(&file_track_critical_section);
    
    return(added_new_listener);
}

internal
Sys_Remove_Listener_Sig(system_remove_listener){
    b32 removed_listener = false;
    
    EnterCriticalSection(&file_track_critical_section);
    
    String file_name_string = make_string_slowly(filename);
    File_Track_Node *existing_file_node = file_track_file_lookup(file_name_string);
    
    if (existing_file_node != 0){
        existing_file_node->ref_count -= 1;
        if (existing_file_node->ref_count == 0){
            Directory_Track_Node *existing_dir_node = existing_file_node->parent_dir;
            existing_dir_node->ref_count -= 1;
            if (existing_dir_node->ref_count == 0){
                File_Track_Instruction_Node *instruction_node = file_track_new_instruction_node();
                instruction_node->instruction = FileTrackInstruction_Cancel;
                instruction_node->dir_node = existing_dir_node;
                PostQueuedCompletionStatus(file_track_iocp, 0, 0, (LPOVERLAPPED)instruction_node);
            }
            file_track_free_file_node(existing_file_node);
        }
        removed_listener = true;
    }
    
    LeaveCriticalSection(&file_track_critical_section);
    
    return(removed_listener);
}

internal
Sys_Get_File_Change_Sig(system_get_file_change){
    b32 has_or_got_a_change = false;
    
    EnterCriticalSection(&file_track_critical_section);
    
    if (file_track_note_first != 0){
        has_or_got_a_change = true;
        File_Track_Note_Node *node = file_track_note_first;
        *required_size = node->file_name.size + 1;
        if (node->file_name.size < max){
            memcpy(buffer, node->file_name.str, node->file_name.size);
            buffer[node->file_name.size] = 0;
            file_track_free_note_node(node);
        }
        else{
            *mem_too_small = true;
        }
    }
    
    LeaveCriticalSection(&file_track_critical_section);
    
    return(has_or_got_a_change);
}

//
// Clipboard
//

internal void
win32_post_clipboard(char *text, i32 len){
    if (OpenClipboard(win32vars.window_handle)){
        if (!EmptyClipboard()){
            win32_output_error_string(ErrorString_UseLog);
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
win32_read_clipboard_contents(void){
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    
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
                            win32vars.clip_max = l_round_up_u32(clip_8_len + 1, KB(4));
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
                            win32vars.clip_max = l_round_up_u32(clip_ascii_len + 1, KB(4));
                            win32vars.clip_buffer = (u8*)system_memory_allocate(win32vars.clip_max);
                        }
                        memcpy(win32vars.clip_buffer, clip_ascii, clip_ascii_len + 1);
                        contents_length = clip_ascii_len + 1;
                    }
                }
            }
            
            if (contents_length > 0){
                win32vars.clipboard_contents = make_string_cap(win32vars.clip_buffer, contents_length - 1, win32vars.clip_max);
            }
            
            GlobalUnlock(clip_data);
            
            CloseClipboard();
        }
    }
    
    end_temp_memory(temp);
    
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
                if (CreateProcess_utf8(&shared_vars.scratch, (u8*)cmd, (u8*)command_line, 0, 0, TRUE, 0, env_variables, (u8*)path, &startup, &info)){
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
    Font_Raw_Data data = {};
    
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
    
    LOGFONT log_font = {};
    log_font.lfCharSet = ANSI_CHARSET;
    log_font.lfFaceName[0] = 0;
    
    Win32_Font_Enum p = {};
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
Win32KeycodeInit(void){
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

internal Win32_Object*
win32_alloc_object(void){
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
    return(result);
}

internal void
win32_free_object(Win32_Object *object){
    if (object->node.next != 0){
        dll_remove(&object->node);
    }
    dll_insert(&win32vars.free_win32_objects, &object->node);
}

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
    Win32_Object *object = win32_alloc_object();
    block_zero_struct(object);
    dll_insert(&win32vars.timer_objects, &object->node);
    object->kind = Win32ObjectKind_Timer;
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
                    
                    b8 down = ((lParam & bit_31)?(0):(1));
                    b8 is_right = ((lParam & bit_24)?(1):(0));
                    
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
            }
            
            b8 current_state = ((lParam & bit_31)?(0):(1));
            if (current_state){
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
    
    dll_init_sentinel(&win32vars.free_win32_objects);
    dll_init_sentinel(&win32vars.timer_objects);
    
    //
    // HACK(allen):
    // Previously zipped stuff is here, it should be zipped in the new pattern now.
    //
    
    init_shared_vars();
    
    //
    // Init Filetrack
    //
    file_track_init();
    
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
    
    WNDCLASS window_class = {};
    window_class.style = CS_HREDRAW|CS_VREDRAW;
    window_class.lpfnWndProc = (WNDPROC)(win32_proc);
    window_class.hInstance = hInstance;
    window_class.lpszClassName = L"4coder-win32-wndclass";
    window_class.hIcon = LoadIcon(hInstance, L"main");
    
    if (!RegisterClass(&window_class)){
        exit(1);
    }
    
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
    win32vars.window_handle = CreateWindowEx(0, window_class.lpszClassName, L_WINDOW_NAME, window_style, window_x, window_y, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, 0, 0, hInstance, 0);
    
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
        win32_output_error_string(ErrorString_UseLog);
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
    u64 timer_start = system_now_time();
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
        memset(&win32vars.clipboard_contents, 0, sizeof(win32vars.clipboard_contents));
        if (win32vars.clipboard_sequence != 0){
            DWORD new_number = GetClipboardSequenceNumber();
            if (new_number != win32vars.clipboard_sequence){
                if (win32vars.next_clipboard_is_self){
                    win32vars.next_clipboard_is_self = false;
                    win32vars.clipboard_sequence = new_number;
                }
                else{
                    b32 got_contents = false;
                    for (i32 R = 0; R < 4; ++R){
                        if (win32_read_clipboard_contents()){
                            win32vars.clipboard_sequence = new_number;
                            got_contents = true;
                            break;
                        }
                    }
                }
            }
        }
        input.clipboard = win32vars.clipboard_contents;
        
        win32vars.clip_post_len = 0;
        
        
        // NOTE(allen): Application Core Update
        Application_Step_Result result = {};
        if (app.step != 0){
            result = app.step(&sysfunc, &target, &memory_vars, &input);
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
        
        // NOTE(allen): Switch to New Title
        if (result.has_new_title){
            SetWindowText_utf8(&shared_vars.scratch, win32vars.window_handle, (u8*)result.title_string);
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
        interpret_render_buffer(&target, &shared_vars.pixel_scratch);
        SwapBuffers(hdc);
        ReleaseDC(win32vars.window_handle, hdc);
        
        // NOTE(allen): toggle full screen
        if (win32vars.do_toggle){
            win32_toggle_fullscreen();
            win32vars.do_toggle = false;
        }
        
        // NOTE(allen): schedule another step if needed
        if (result.animating){
            system_schedule_step();
        }
        
        // NOTE(allen): sleep a bit to cool off :)
        flush_thread_group(BACKGROUND_THREADS);
        
        u64 timer_end = system_now_time();
        u64 end_target = timer_start + frame_useconds;
        
        system_release_lock(FRAME_LOCK);
        for (;timer_end < end_target;){
            DWORD samount = (DWORD)((end_target - timer_end)/1000);
            if (samount > 0){
                Sleep(samount);
            }
            timer_end = system_now_time();
        }
        system_acquire_lock(FRAME_LOCK);
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

