#define system_error_box_sig() void system_error_box(char* msg)
#define system_get_path_sig() String_Const_u8 system_get_path(Arena* arena, System_Path_Code path_code)
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
#define system_get_proc_sig() Void_Func* system_get_proc(System_Library handle, char* proc_name)
#define system_now_time_sig() u64 system_now_time(void)
#define system_now_date_time_universal_sig() Date_Time system_now_date_time_universal(void)
#define system_local_date_time_from_universal_sig() Date_Time system_local_date_time_from_universal(Date_Time* date_time)
#define system_universal_date_time_from_local_sig() Date_Time system_universal_date_time_from_local(Date_Time* date_time)
#define system_wake_up_timer_create_sig() Plat_Handle system_wake_up_timer_create(void)
#define system_wake_up_timer_release_sig() void system_wake_up_timer_release(Plat_Handle handle)
#define system_wake_up_timer_set_sig() void system_wake_up_timer_set(Plat_Handle handle, u32 time_milliseconds)
#define system_signal_step_sig() void system_signal_step(u32 code)
#define system_sleep_sig() void system_sleep(u64 microseconds)
#define system_get_clipboard_sig() String_Const_u8 system_get_clipboard(Arena* arena, i32 index)
#define system_post_clipboard_sig() void system_post_clipboard(String_Const_u8 str, i32 index)
#define system_set_clipboard_catch_all_sig() void system_set_clipboard_catch_all(b32 enabled)
#define system_get_clipboard_catch_all_sig() b32 system_get_clipboard_catch_all(void)
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
#define system_acquire_global_frame_mutex_sig() void system_acquire_global_frame_mutex(Thread_Context* tctx)
#define system_release_global_frame_mutex_sig() void system_release_global_frame_mutex(Thread_Context* tctx)
#define system_mutex_make_sig() System_Mutex system_mutex_make(void)
#define system_mutex_acquire_sig() void system_mutex_acquire(System_Mutex mutex)
#define system_mutex_release_sig() void system_mutex_release(System_Mutex mutex)
#define system_mutex_free_sig() void system_mutex_free(System_Mutex mutex)
#define system_condition_variable_make_sig() System_Condition_Variable system_condition_variable_make(void)
#define system_condition_variable_wait_sig() void system_condition_variable_wait(System_Condition_Variable cv, System_Mutex mutex)
#define system_condition_variable_signal_sig() void system_condition_variable_signal(System_Condition_Variable cv)
#define system_condition_variable_free_sig() void system_condition_variable_free(System_Condition_Variable cv)
#define system_memory_allocate_sig() void* system_memory_allocate(u64 size, String_Const_u8 location)
#define system_memory_set_protection_sig() b32 system_memory_set_protection(void* ptr, u64 size, u32 flags)
#define system_memory_free_sig() void system_memory_free(void* ptr, u64 size)
#define system_memory_annotation_sig() Memory_Annotation system_memory_annotation(Arena* arena)
#define system_show_mouse_cursor_sig() void system_show_mouse_cursor(i32 show)
#define system_set_fullscreen_sig() b32 system_set_fullscreen(b32 full_screen)
#define system_is_fullscreen_sig() b32 system_is_fullscreen(void)
#define system_get_keyboard_modifiers_sig() Input_Modifier_Set system_get_keyboard_modifiers(Arena* arena)
#define system_set_key_mode_sig() void system_set_key_mode(Key_Mode mode)
#define system_set_source_mixer_sig() void system_set_source_mixer(void* ctx, Audio_Mix_Sources_Function* mix_func)
#define system_set_destination_mixer_sig() void system_set_destination_mixer(Audio_Mix_Destination_Function* mix_func)
typedef void system_error_box_type(char* msg);
typedef String_Const_u8 system_get_path_type(Arena* arena, System_Path_Code path_code);
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
typedef Void_Func* system_get_proc_type(System_Library handle, char* proc_name);
typedef u64 system_now_time_type(void);
typedef Date_Time system_now_date_time_universal_type(void);
typedef Date_Time system_local_date_time_from_universal_type(Date_Time* date_time);
typedef Date_Time system_universal_date_time_from_local_type(Date_Time* date_time);
typedef Plat_Handle system_wake_up_timer_create_type(void);
typedef void system_wake_up_timer_release_type(Plat_Handle handle);
typedef void system_wake_up_timer_set_type(Plat_Handle handle, u32 time_milliseconds);
typedef void system_signal_step_type(u32 code);
typedef void system_sleep_type(u64 microseconds);
typedef String_Const_u8 system_get_clipboard_type(Arena* arena, i32 index);
typedef void system_post_clipboard_type(String_Const_u8 str, i32 index);
typedef void system_set_clipboard_catch_all_type(b32 enabled);
typedef b32 system_get_clipboard_catch_all_type(void);
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
typedef void system_acquire_global_frame_mutex_type(Thread_Context* tctx);
typedef void system_release_global_frame_mutex_type(Thread_Context* tctx);
typedef System_Mutex system_mutex_make_type(void);
typedef void system_mutex_acquire_type(System_Mutex mutex);
typedef void system_mutex_release_type(System_Mutex mutex);
typedef void system_mutex_free_type(System_Mutex mutex);
typedef System_Condition_Variable system_condition_variable_make_type(void);
typedef void system_condition_variable_wait_type(System_Condition_Variable cv, System_Mutex mutex);
typedef void system_condition_variable_signal_type(System_Condition_Variable cv);
typedef void system_condition_variable_free_type(System_Condition_Variable cv);
typedef void* system_memory_allocate_type(u64 size, String_Const_u8 location);
typedef b32 system_memory_set_protection_type(void* ptr, u64 size, u32 flags);
typedef void system_memory_free_type(void* ptr, u64 size);
typedef Memory_Annotation system_memory_annotation_type(Arena* arena);
typedef void system_show_mouse_cursor_type(i32 show);
typedef b32 system_set_fullscreen_type(b32 full_screen);
typedef b32 system_is_fullscreen_type(void);
typedef Input_Modifier_Set system_get_keyboard_modifiers_type(Arena* arena);
typedef void system_set_key_mode_type(Key_Mode mode);
typedef void system_set_source_mixer_type(void* ctx, Audio_Mix_Sources_Function* mix_func);
typedef void system_set_destination_mixer_type(Audio_Mix_Destination_Function* mix_func);
struct API_VTable_system{
system_error_box_type *error_box;
system_get_path_type *get_path;
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
system_now_date_time_universal_type *now_date_time_universal;
system_local_date_time_from_universal_type *local_date_time_from_universal;
system_universal_date_time_from_local_type *universal_date_time_from_local;
system_wake_up_timer_create_type *wake_up_timer_create;
system_wake_up_timer_release_type *wake_up_timer_release;
system_wake_up_timer_set_type *wake_up_timer_set;
system_signal_step_type *signal_step;
system_sleep_type *sleep;
system_get_clipboard_type *get_clipboard;
system_post_clipboard_type *post_clipboard;
system_set_clipboard_catch_all_type *set_clipboard_catch_all;
system_get_clipboard_catch_all_type *get_clipboard_catch_all;
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
system_acquire_global_frame_mutex_type *acquire_global_frame_mutex;
system_release_global_frame_mutex_type *release_global_frame_mutex;
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
system_memory_annotation_type *memory_annotation;
system_show_mouse_cursor_type *show_mouse_cursor;
system_set_fullscreen_type *set_fullscreen;
system_is_fullscreen_type *is_fullscreen;
system_get_keyboard_modifiers_type *get_keyboard_modifiers;
system_set_key_mode_type *set_key_mode;
system_set_source_mixer_type *set_source_mixer;
system_set_destination_mixer_type *set_destination_mixer;
};
#if defined(STATIC_LINK_API)
internal void system_error_box(char* msg);
internal String_Const_u8 system_get_path(Arena* arena, System_Path_Code path_code);
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
internal Void_Func* system_get_proc(System_Library handle, char* proc_name);
internal u64 system_now_time(void);
internal Date_Time system_now_date_time_universal(void);
internal Date_Time system_local_date_time_from_universal(Date_Time* date_time);
internal Date_Time system_universal_date_time_from_local(Date_Time* date_time);
internal Plat_Handle system_wake_up_timer_create(void);
internal void system_wake_up_timer_release(Plat_Handle handle);
internal void system_wake_up_timer_set(Plat_Handle handle, u32 time_milliseconds);
internal void system_signal_step(u32 code);
internal void system_sleep(u64 microseconds);
internal String_Const_u8 system_get_clipboard(Arena* arena, i32 index);
internal void system_post_clipboard(String_Const_u8 str, i32 index);
internal void system_set_clipboard_catch_all(b32 enabled);
internal b32 system_get_clipboard_catch_all(void);
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
internal void system_acquire_global_frame_mutex(Thread_Context* tctx);
internal void system_release_global_frame_mutex(Thread_Context* tctx);
internal System_Mutex system_mutex_make(void);
internal void system_mutex_acquire(System_Mutex mutex);
internal void system_mutex_release(System_Mutex mutex);
internal void system_mutex_free(System_Mutex mutex);
internal System_Condition_Variable system_condition_variable_make(void);
internal void system_condition_variable_wait(System_Condition_Variable cv, System_Mutex mutex);
internal void system_condition_variable_signal(System_Condition_Variable cv);
internal void system_condition_variable_free(System_Condition_Variable cv);
internal void* system_memory_allocate(u64 size, String_Const_u8 location);
internal b32 system_memory_set_protection(void* ptr, u64 size, u32 flags);
internal void system_memory_free(void* ptr, u64 size);
internal Memory_Annotation system_memory_annotation(Arena* arena);
internal void system_show_mouse_cursor(i32 show);
internal b32 system_set_fullscreen(b32 full_screen);
internal b32 system_is_fullscreen(void);
internal Input_Modifier_Set system_get_keyboard_modifiers(Arena* arena);
internal void system_set_key_mode(Key_Mode mode);
internal void system_set_source_mixer(void* ctx, Audio_Mix_Sources_Function* mix_func);
internal void system_set_destination_mixer(Audio_Mix_Destination_Function* mix_func);
#undef STATIC_LINK_API
#elif defined(DYNAMIC_LINK_API)
global system_error_box_type *system_error_box = 0;
global system_get_path_type *system_get_path = 0;
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
global system_now_date_time_universal_type *system_now_date_time_universal = 0;
global system_local_date_time_from_universal_type *system_local_date_time_from_universal = 0;
global system_universal_date_time_from_local_type *system_universal_date_time_from_local = 0;
global system_wake_up_timer_create_type *system_wake_up_timer_create = 0;
global system_wake_up_timer_release_type *system_wake_up_timer_release = 0;
global system_wake_up_timer_set_type *system_wake_up_timer_set = 0;
global system_signal_step_type *system_signal_step = 0;
global system_sleep_type *system_sleep = 0;
global system_get_clipboard_type *system_get_clipboard = 0;
global system_post_clipboard_type *system_post_clipboard = 0;
global system_set_clipboard_catch_all_type *system_set_clipboard_catch_all = 0;
global system_get_clipboard_catch_all_type *system_get_clipboard_catch_all = 0;
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
global system_acquire_global_frame_mutex_type *system_acquire_global_frame_mutex = 0;
global system_release_global_frame_mutex_type *system_release_global_frame_mutex = 0;
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
global system_memory_annotation_type *system_memory_annotation = 0;
global system_show_mouse_cursor_type *system_show_mouse_cursor = 0;
global system_set_fullscreen_type *system_set_fullscreen = 0;
global system_is_fullscreen_type *system_is_fullscreen = 0;
global system_get_keyboard_modifiers_type *system_get_keyboard_modifiers = 0;
global system_set_key_mode_type *system_set_key_mode = 0;
global system_set_source_mixer_type *system_set_source_mixer = 0;
global system_set_destination_mixer_type *system_set_destination_mixer = 0;
#undef DYNAMIC_LINK_API
#endif
