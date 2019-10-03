#define system_get_canonical_sig() String_Const_u8 system_get_canonical(Arena* arena, String_Const_u8 name)
#define system_get_file_list_sig() File_List system_get_file_list(Arena* arena, String_Const_u8 directory)
#define system_quick_file_attributes_sig() File_Attributes system_quick_file_attributes(Arena* scratch, String_Const_u8 file_name)
#define system_load_handle_sig() b32 system_load_handle(Arena* scratch, char* file_name, Plat_Handle* out)
#define system_load_attributes_sig() File_Attributes system_load_attributes(Plat_Handle handle)
#define system_load_file_sig() b32 system_load_file(Plat_Handle handle, char* buffer, u32 size)
#define system_load_close_sig() b32 system_load_close(Plat_Handle handle)
#define system_save_file_sig() File_Attributes system_save_file(Arena* scratch, char* file_name, String_Const_u8 data)
#define system_load_library_sig() b32 system_load_library(Arena* scratch, String_Const_u8 file_name, System_Library* out)
#define system_release_library_sig() b32 system_release_library(System_Library handle)
#define system_get_proc_sig() Void_Func system_get_proc(System_Library handle, char* proc_name)
#define system_now_time_sig() u64 system_now_time(void)
#define system_wake_up_timer_create_sig() Plat_Handle system_wake_up_timer_create(void)
#define system_wake_up_timer_release_sig() void system_wake_up_timer_release(Plat_Handle handle)
#define system_wake_up_timer_set_sig() void system_wake_up_timer_set(Plat_Handle handle, u32 time_milliseconds)
#define system_signal_step_sig() void system_signal_step(u32 code)
#define system_sleep_sig() void system_sleep(u64 microseconds)
#define system_post_clipboard_sig() void system_post_clipboard(String_Const_u8 str)
#define system_cli_call_sig() b32 system_cli_call(Arena* scratch, char* path, char* script, CLI_Handles* cli_out)
#define system_cli_begin_update_sig() void system_cli_begin_update(CLI_Handles* cli)
#define system_cli_update_step_sig() b32 system_cli_update_step(CLI_Handles* cli, char* dest, u32 max, u32* amount)
#define system_cli_end_update_sig() b32 system_cli_end_update(CLI_Handles* cli)
#define system_open_color_picker_sig() void system_open_color_picker(Color_Picker* picker)
#define system_get_screen_scale_factor_sig() f32 system_get_screen_scale_factor(void)
#define system_thread_launch_sig() System_Thread system_thread_launch(Thread_Function* proc, void* ptr)
#define system_thread_join_sig() void system_thread_join(System_Thread thread)
#define system_thread_free_sig() void system_thread_free(System_Thread thread)
#define system_thread_get_id_sig() i32 system_thread_get_id(void)
#define system_mutex_make_sig() System_Mutex system_mutex_make(void)
#define system_mutex_acquire_sig() void system_mutex_acquire(System_Mutex mutex)
#define system_mutex_release_sig() void system_mutex_release(System_Mutex mutex)
#define system_mutex_free_sig() void system_mutex_free(System_Mutex mutex)
#define system_condition_variable_make_sig() System_Condition_Variable system_condition_variable_make(void)
#define system_condition_variable_wait_sig() void system_condition_variable_wait(System_Condition_Variable cv, System_Mutex mutex)
#define system_condition_variable_signal_sig() void system_condition_variable_signal(System_Condition_Variable cv)
#define system_condition_variable_free_sig() void system_condition_variable_free(System_Condition_Variable cv)
#define system_memory_allocate_sig() void* system_memory_allocate(umem size)
#define system_memory_set_protection_sig() b32 system_memory_set_protection(void* ptr, umem size, u32 flags)
#define system_memory_free_sig() void system_memory_free(void* ptr, umem size)
#define system_show_mouse_cursor_sig() void system_show_mouse_cursor(i32 show)
#define system_set_fullscreen_sig() b32 system_set_fullscreen(b32 full_screen)
#define system_is_fullscreen_sig() b32 system_is_fullscreen(void)
typedef String_Const_u8 system_get_canonical_type(Arena* arena, String_Const_u8 name);
typedef File_List system_get_file_list_type(Arena* arena, String_Const_u8 directory);
typedef File_Attributes system_quick_file_attributes_type(Arena* scratch, String_Const_u8 file_name);
typedef b32 system_load_handle_type(Arena* scratch, char* file_name, Plat_Handle* out);
typedef File_Attributes system_load_attributes_type(Plat_Handle handle);
typedef b32 system_load_file_type(Plat_Handle handle, char* buffer, u32 size);
typedef b32 system_load_close_type(Plat_Handle handle);
typedef File_Attributes system_save_file_type(Arena* scratch, char* file_name, String_Const_u8 data);
typedef b32 system_load_library_type(Arena* scratch, String_Const_u8 file_name, System_Library* out);
typedef b32 system_release_library_type(System_Library handle);
typedef Void_Func system_get_proc_type(System_Library handle, char* proc_name);
typedef u64 system_now_time_type(void);
typedef Plat_Handle system_wake_up_timer_create_type(void);
typedef void system_wake_up_timer_release_type(Plat_Handle handle);
typedef void system_wake_up_timer_set_type(Plat_Handle handle, u32 time_milliseconds);
typedef void system_signal_step_type(u32 code);
typedef void system_sleep_type(u64 microseconds);
typedef void system_post_clipboard_type(String_Const_u8 str);
typedef b32 system_cli_call_type(Arena* scratch, char* path, char* script, CLI_Handles* cli_out);
typedef void system_cli_begin_update_type(CLI_Handles* cli);
typedef b32 system_cli_update_step_type(CLI_Handles* cli, char* dest, u32 max, u32* amount);
typedef b32 system_cli_end_update_type(CLI_Handles* cli);
typedef void system_open_color_picker_type(Color_Picker* picker);
typedef f32 system_get_screen_scale_factor_type(void);
typedef System_Thread system_thread_launch_type(Thread_Function* proc, void* ptr);
typedef void system_thread_join_type(System_Thread thread);
typedef void system_thread_free_type(System_Thread thread);
typedef i32 system_thread_get_id_type(void);
typedef System_Mutex system_mutex_make_type(void);
typedef void system_mutex_acquire_type(System_Mutex mutex);
typedef void system_mutex_release_type(System_Mutex mutex);
typedef void system_mutex_free_type(System_Mutex mutex);
typedef System_Condition_Variable system_condition_variable_make_type(void);
typedef void system_condition_variable_wait_type(System_Condition_Variable cv, System_Mutex mutex);
typedef void system_condition_variable_signal_type(System_Condition_Variable cv);
typedef void system_condition_variable_free_type(System_Condition_Variable cv);
typedef void* system_memory_allocate_type(umem size);
typedef b32 system_memory_set_protection_type(void* ptr, umem size, u32 flags);
typedef void system_memory_free_type(void* ptr, umem size);
typedef void system_show_mouse_cursor_type(i32 show);
typedef b32 system_set_fullscreen_type(b32 full_screen);
typedef b32 system_is_fullscreen_type(void);
struct API_VTable_system{
system_get_canonical_type *get_canonical;
system_get_file_list_type *get_file_list;
system_quick_file_attributes_type *quick_file_attributes;
system_load_handle_type *load_handle;
system_load_attributes_type *load_attributes;
system_load_file_type *load_file;
system_load_close_type *load_close;
system_save_file_type *save_file;
system_load_library_type *load_library;
system_release_library_type *release_library;
system_get_proc_type *get_proc;
system_now_time_type *now_time;
system_wake_up_timer_create_type *wake_up_timer_create;
system_wake_up_timer_release_type *wake_up_timer_release;
system_wake_up_timer_set_type *wake_up_timer_set;
system_signal_step_type *signal_step;
system_sleep_type *sleep;
system_post_clipboard_type *post_clipboard;
system_cli_call_type *cli_call;
system_cli_begin_update_type *cli_begin_update;
system_cli_update_step_type *cli_update_step;
system_cli_end_update_type *cli_end_update;
system_open_color_picker_type *open_color_picker;
system_get_screen_scale_factor_type *get_screen_scale_factor;
system_thread_launch_type *thread_launch;
system_thread_join_type *thread_join;
system_thread_free_type *thread_free;
system_thread_get_id_type *thread_get_id;
system_mutex_make_type *mutex_make;
system_mutex_acquire_type *mutex_acquire;
system_mutex_release_type *mutex_release;
system_mutex_free_type *mutex_free;
system_condition_variable_make_type *condition_variable_make;
system_condition_variable_wait_type *condition_variable_wait;
system_condition_variable_signal_type *condition_variable_signal;
system_condition_variable_free_type *condition_variable_free;
system_memory_allocate_type *memory_allocate;
system_memory_set_protection_type *memory_set_protection;
system_memory_free_type *memory_free;
system_show_mouse_cursor_type *show_mouse_cursor;
system_set_fullscreen_type *set_fullscreen;
system_is_fullscreen_type *is_fullscreen;
};
#if defined(STATIC_LINK_API)
internal String_Const_u8 system_get_canonical(Arena* arena, String_Const_u8 name);
internal File_List system_get_file_list(Arena* arena, String_Const_u8 directory);
internal File_Attributes system_quick_file_attributes(Arena* scratch, String_Const_u8 file_name);
internal b32 system_load_handle(Arena* scratch, char* file_name, Plat_Handle* out);
internal File_Attributes system_load_attributes(Plat_Handle handle);
internal b32 system_load_file(Plat_Handle handle, char* buffer, u32 size);
internal b32 system_load_close(Plat_Handle handle);
internal File_Attributes system_save_file(Arena* scratch, char* file_name, String_Const_u8 data);
internal b32 system_load_library(Arena* scratch, String_Const_u8 file_name, System_Library* out);
internal b32 system_release_library(System_Library handle);
internal Void_Func system_get_proc(System_Library handle, char* proc_name);
internal u64 system_now_time(void);
internal Plat_Handle system_wake_up_timer_create(void);
internal void system_wake_up_timer_release(Plat_Handle handle);
internal void system_wake_up_timer_set(Plat_Handle handle, u32 time_milliseconds);
internal void system_signal_step(u32 code);
internal void system_sleep(u64 microseconds);
internal void system_post_clipboard(String_Const_u8 str);
internal b32 system_cli_call(Arena* scratch, char* path, char* script, CLI_Handles* cli_out);
internal void system_cli_begin_update(CLI_Handles* cli);
internal b32 system_cli_update_step(CLI_Handles* cli, char* dest, u32 max, u32* amount);
internal b32 system_cli_end_update(CLI_Handles* cli);
internal void system_open_color_picker(Color_Picker* picker);
internal f32 system_get_screen_scale_factor(void);
internal System_Thread system_thread_launch(Thread_Function* proc, void* ptr);
internal void system_thread_join(System_Thread thread);
internal void system_thread_free(System_Thread thread);
internal i32 system_thread_get_id(void);
internal System_Mutex system_mutex_make(void);
internal void system_mutex_acquire(System_Mutex mutex);
internal void system_mutex_release(System_Mutex mutex);
internal void system_mutex_free(System_Mutex mutex);
internal System_Condition_Variable system_condition_variable_make(void);
internal void system_condition_variable_wait(System_Condition_Variable cv, System_Mutex mutex);
internal void system_condition_variable_signal(System_Condition_Variable cv);
internal void system_condition_variable_free(System_Condition_Variable cv);
internal void* system_memory_allocate(umem size);
internal b32 system_memory_set_protection(void* ptr, umem size, u32 flags);
internal void system_memory_free(void* ptr, umem size);
internal void system_show_mouse_cursor(i32 show);
internal b32 system_set_fullscreen(b32 full_screen);
internal b32 system_is_fullscreen(void);
#undef STATIC_LINK_API
#elif defined(DYNAMIC_LINK_API)
global system_get_canonical_type *system_get_canonical = 0;
global system_get_file_list_type *system_get_file_list = 0;
global system_quick_file_attributes_type *system_quick_file_attributes = 0;
global system_load_handle_type *system_load_handle = 0;
global system_load_attributes_type *system_load_attributes = 0;
global system_load_file_type *system_load_file = 0;
global system_load_close_type *system_load_close = 0;
global system_save_file_type *system_save_file = 0;
global system_load_library_type *system_load_library = 0;
global system_release_library_type *system_release_library = 0;
global system_get_proc_type *system_get_proc = 0;
global system_now_time_type *system_now_time = 0;
global system_wake_up_timer_create_type *system_wake_up_timer_create = 0;
global system_wake_up_timer_release_type *system_wake_up_timer_release = 0;
global system_wake_up_timer_set_type *system_wake_up_timer_set = 0;
global system_signal_step_type *system_signal_step = 0;
global system_sleep_type *system_sleep = 0;
global system_post_clipboard_type *system_post_clipboard = 0;
global system_cli_call_type *system_cli_call = 0;
global system_cli_begin_update_type *system_cli_begin_update = 0;
global system_cli_update_step_type *system_cli_update_step = 0;
global system_cli_end_update_type *system_cli_end_update = 0;
global system_open_color_picker_type *system_open_color_picker = 0;
global system_get_screen_scale_factor_type *system_get_screen_scale_factor = 0;
global system_thread_launch_type *system_thread_launch = 0;
global system_thread_join_type *system_thread_join = 0;
global system_thread_free_type *system_thread_free = 0;
global system_thread_get_id_type *system_thread_get_id = 0;
global system_mutex_make_type *system_mutex_make = 0;
global system_mutex_acquire_type *system_mutex_acquire = 0;
global system_mutex_release_type *system_mutex_release = 0;
global system_mutex_free_type *system_mutex_free = 0;
global system_condition_variable_make_type *system_condition_variable_make = 0;
global system_condition_variable_wait_type *system_condition_variable_wait = 0;
global system_condition_variable_signal_type *system_condition_variable_signal = 0;
global system_condition_variable_free_type *system_condition_variable_free = 0;
global system_memory_allocate_type *system_memory_allocate = 0;
global system_memory_set_protection_type *system_memory_set_protection = 0;
global system_memory_free_type *system_memory_free = 0;
global system_show_mouse_cursor_type *system_show_mouse_cursor = 0;
global system_set_fullscreen_type *system_set_fullscreen = 0;
global system_is_fullscreen_type *system_is_fullscreen = 0;
#undef DYNAMIC_LINK_API
#endif
