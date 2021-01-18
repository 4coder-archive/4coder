/* macOS System/Graphics/Font API Implementations */

/********************/
/*    System API    */
/********************/

////////////////////////////////

function
system_get_path_sig(){
    String_Const_u8 result = {};

    switch (path_code){
        case SystemPath_CurrentDirectory:
        {
            char *working_dir = getcwd(NULL, 0);
            u64 working_dir_length = cstring_length(working_dir);

            // TODO(yuval): Maybe use push_string_copy instead
            u8 *out = push_array(arena, u8, working_dir_length);
            block_copy(out, working_dir, working_dir_length);

            free(working_dir);

            result = SCu8(out, working_dir_length);
        } break;

        case SystemPath_Binary:
        {
            local_persist b32 has_stashed_4ed_path = false;
            if (!has_stashed_4ed_path){
                local_const u32 binary_path_capacity = PATH_MAX;
                u8 *memory = (u8*)system_memory_allocate(binary_path_capacity, file_name_line_number_lit_u8);

                pid_t pid = getpid();
                i32 size = proc_pidpath(pid, memory, binary_path_capacity);
                Assert(size < binary_path_capacity);

                mac_vars.binary_path = SCu8(memory, size);
                mac_vars.binary_path = string_remove_last_folder(mac_vars.binary_path);
                mac_vars.binary_path.str[mac_vars.binary_path.size] = 0;

                has_stashed_4ed_path = true;
            }

            result = push_string_copy(arena, mac_vars.binary_path);
        } break;

        case SystemPath_UserDirectory:
        {
            char *home_cstr = getenv("HOME");
            if (home_cstr != 0){
                result = push_u8_stringf(arena, "%s/.4coder/", home_cstr);
            }
        }break;
    }

    return(result);
}

function
system_get_canonical_sig(){
    NSString *path_ns_str =
        [[NSString alloc] initWithBytes:name.str length:name.size encoding:NSUTF8StringEncoding];

    NSString *standardized_path_ns_str = [path_ns_str stringByStandardizingPath];
    String_Const_u8 standardized_path = SCu8((u8*)[standardized_path_ns_str UTF8String],[standardized_path_ns_str lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);

    String_Const_u8 result = push_string_copy(arena, standardized_path);

    [path_ns_str release];

    return(result);
}

////////////////////////////////

function File_Attributes
mac_get_file_attributes(struct stat file_stat) {
    File_Attributes result;
    result.size = file_stat.st_size;
    result.last_write_time = file_stat.st_mtimespec.tv_sec;

    result.flags = 0;
    if (S_ISDIR(file_stat.st_mode)) {
        result.flags |= FileAttribute_IsDirectory;
    }

    return(result);
}

function inline File_Attributes
mac_file_attributes_from_path(char *path) {
    File_Attributes result = {};

    struct stat file_stat;
    if (stat(path, &file_stat) == 0){
        result = mac_get_file_attributes(file_stat);
    }

    return(result);
}

function inline File_Attributes
mac_file_attributes_from_fd(i32 fd) {
    File_Attributes result = {};

    struct stat file_stat;
    if (fstat(fd, &file_stat) == 0){
        result = mac_get_file_attributes(file_stat);
    }

    return(result);
}

function
system_get_file_list_sig(){
    File_List result = {};

    u8 *c_directory = push_array(arena, u8, directory.size + 1);
    block_copy(c_directory, directory.str, directory.size);
    c_directory[directory.size] = 0;

    DIR *dir = opendir((char*)c_directory);
    if (dir){
        File_Info* first = 0;
        File_Info* last = 0;
        i32 count = 0;

        for (struct dirent *entry = readdir(dir);
             entry;
             entry = readdir(dir)){
            char *c_file_name = entry->d_name;
            String_Const_u8 file_name = SCu8(c_file_name);

            if (string_match(file_name, string_u8_litexpr(".")) || string_match(file_name, string_u8_litexpr(".."))){
                continue;
            }

            File_Info *info = push_array(arena, File_Info, 1);
            sll_queue_push(first, last, info);
            count += 1;

            info->file_name = push_string_copy(arena, file_name);

            // NOTE(yuval): Get file attributes
            {
                Temp_Memory temp = begin_temp(arena);

                b32 append_slash = false;
                u64 file_path_size = directory.size + file_name.size;
                if (string_get_character(directory, directory.size - 1) != '/'){
                    append_slash = true;
                    file_path_size += 1;
                }

                char *file_path = push_array(arena, char, file_path_size + 1);
                char *file_path_at = file_path;

                block_copy(file_path_at, directory.str, directory.size);
                file_path_at += directory.size;

                if (append_slash){
                    *file_path_at = '/';
                    file_path_at += 1;
                }

                block_copy(file_path_at, file_name.str, file_name.size);
                file_path_at += file_name.size;

                *file_path_at = 0;

                info->attributes = mac_file_attributes_from_path(file_path);

                end_temp(temp);
            }
        }

        closedir(dir);

        result.infos = push_array(arena, File_Info*, count);
        result.count = count;

        i32 index = 0;
        for (File_Info *node = first;
             node != 0;
             node = node->next){
            result.infos[index] = node;
            index += 1;
        }
    }

    return(result);
}

function
system_quick_file_attributes_sig(){
    Temp_Memory temp = begin_temp(scratch);

    char *c_file_name = push_array(scratch, char, file_name.size + 1);
    block_copy(c_file_name, file_name.str, file_name.size);
    c_file_name[file_name.size] = 0;

    File_Attributes result = mac_file_attributes_from_path(c_file_name);

    end_temp(temp);

    return(result);
}

function inline Plat_Handle
mac_to_plat_handle(i32 fd){
    Plat_Handle result = *(Plat_Handle*)(&fd);
    return(result);
}

function inline i32
mac_to_fd(Plat_Handle handle){
    i32 result = *(i32*)(&handle);
    return(result);
}

function
system_load_handle_sig(){
    b32 result = false;

    i32 fd = open(file_name, O_RDONLY);
    if ((fd != -1) && (fd != 0)) {
        *out = mac_to_plat_handle(fd);
        result = true;
    }

    return(result);
}

function
system_load_attributes_sig(){
    i32 fd = mac_to_fd(handle);
    File_Attributes result = mac_file_attributes_from_fd(fd);

    return(result);
}

function
system_load_file_sig(){
    i32 fd = mac_to_fd(handle);

    do{
        ssize_t bytes_read = read(fd, buffer, size);
        if (bytes_read == -1){
            if (errno != EINTR){
                // NOTE(yuval): An error occured while reading from the file descriptor
                break;
            }
        } else{
            size -= bytes_read;
            buffer += bytes_read;
        }
    } while (size > 0);

    b32 result = (size == 0);
    return(result);
}

function
system_load_close_sig(){
    b32 result = true;

    i32 fd = mac_to_fd(handle);
    if (close(fd) == -1){
        // NOTE(yuval): An error occured while close the file descriptor
        result = false;
    }

    return(result);
}

function
system_save_file_sig(){
    File_Attributes result = {};

    i32 fd = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 00640);
    if (fd != -1) {
        do{
            ssize_t bytes_written = write(fd, data.str, data.size);
            if (bytes_written == -1){
                if (errno != EINTR){
                    // NOTE(yuval): An error occured while writing to the file descriptor
                    break;
                }
            } else{
                data.size -= bytes_written;
                data.str += bytes_written;
            }
        } while (data.size > 0);

        if (data.size == 0) {
            result = mac_file_attributes_from_fd(fd);
        }

        close(fd);
    }

    return(result);
}

////////////////////////////////

function inline System_Library
mac_to_system_library(void *dl_handle){
    System_Library result = *(System_Library*)(&dl_handle);
    return(result);
}

function inline void*
mac_to_dl_handle(System_Library system_lib){
    void *result = *(void**)(&system_lib);
    return(result);
}

function
system_load_library_sig(){
    b32 result = false;

    void *lib = 0;

    // NOTE(yuval): Open library handle
    {
        Temp_Memory temp = begin_temp(scratch);

        char *c_file_name = push_array(scratch, char, file_name.size + 1);
        block_copy(c_file_name, file_name.str, file_name.size);
        c_file_name[file_name.size] = 0;

        lib = dlopen(c_file_name, RTLD_LAZY | RTLD_GLOBAL);

        end_temp(temp);
    }

    if (lib){
        *out = mac_to_system_library(lib);
        result = true;
    }

    return(result);
}

function
system_release_library_sig(){
    void *lib = mac_to_dl_handle(handle);
    i32 rc = dlclose(lib);

    b32 result = (rc == 0);
    return(result);
}

function
system_get_proc_sig(){
    void *lib = mac_to_dl_handle(handle);
    Void_Func *result = (Void_Func*)dlsym(lib, proc_name);

    return(result);
}

////////////////////////////////

function
system_now_time_sig(){
    u64 now = mach_absolute_time();

    // NOTE(yuval): Now time nanoseconds conversion
    f64 now_nano = (f64)((f64)now *
                         ((f64)mac_vars.timebase_info.numer /
                          (f64)mac_vars.timebase_info.denom));

    // NOTE(yuval): Conversion to useconds
    u64 result = (u64)(now_nano * 1.0E-3);
    return(result);
}

function void
mac_date_time_from_tm(Date_Time *out, struct tm *in){
    out->year = in->tm_year + 1900;
    out->mon = in->tm_mon;
    out->day = in->tm_mday - 1;
    out->hour = in->tm_hour;
    out->min = in->tm_min;
    out->sec = in->tm_sec;
    out->msec = 0;
}

function void
mac_tm_from_date_time(struct tm *out, Date_Time *in){
    out->tm_year = in->year - 1900;
    out->tm_mon = in->mon;
    out->tm_mday = in->day + 1;
    out->tm_hour = in->hour;
    out->tm_min = in->min;
    out->tm_sec = in->sec;
}

function
system_now_date_time_universal_sig(){
    time_t now_time = time(0);
    struct tm *now_tm = gmtime(&now_time);
    Date_Time result = {};
    mac_date_time_from_tm(&result, now_tm);
    return(result);
}

function
system_local_date_time_from_universal_sig(){
    struct tm univ_tm = {};
    mac_tm_from_date_time(&univ_tm, date_time);
    time_t utc_time = timegm(&univ_tm);
    struct tm *local_tm = localtime(&utc_time);
    Date_Time result = {};
    mac_date_time_from_tm(&result, local_tm);
    return(result);
}

function
system_universal_date_time_from_local_sig(){
    struct tm local_tm = {};
    mac_tm_from_date_time(&local_tm, date_time);
    time_t loc_time = timelocal(&local_tm);
    struct tm *utc_tm = gmtime(&loc_time);
    Date_Time result = {};
    mac_date_time_from_tm(&result, utc_tm);
    return(result);
}

function
system_wake_up_timer_create_sig(){
    Mac_Object *object = mac_alloc_object(MacObjectKind_Timer);
    dll_insert(&mac_vars.timer_objects, &object->node);

    object->timer = nil;

    Plat_Handle result = mac_to_plat_handle(object);
    return(result);
}

function
system_wake_up_timer_release_sig(){
    Mac_Object *object = mac_to_object(handle);
    if (object->kind == MacObjectKind_Timer){
        if ((object->timer != nil) && [object->timer isValid]) {
            [object->timer invalidate];
            mac_free_object(object);
        }
    }
}

function
system_wake_up_timer_set_sig(){
    Mac_Object *object = mac_to_object(handle);
    if (object->kind == MacObjectKind_Timer){
        f64 time_seconds = ((f64)time_milliseconds / 1000.0);
        object->timer = [NSTimer scheduledTimerWithTimeInterval:time_seconds
                target:mac_vars.view
                selector:@selector(request_display)
                userInfo:nil repeats:NO];
    }
}

function
system_signal_step_sig(){
#if 0
    if (!mac_vars.step_requested){
        [NSTimer scheduledTimerWithTimeInterval:0.0
                target:mac_vars.view
                selector:@selector(request_display)
                userInfo:nil repeats:NO];

        mac_vars.step_requested = true;
    }
#else
    mac_vars.step_requested = true;
    dispatch_async(dispatch_get_main_queue(),
                   ^{
                        [NSTimer scheduledTimerWithTimeInterval:0.0
                         target:mac_vars.view
                         selector:@selector(request_display)
                         userInfo:nil repeats:NO];
                   });
#endif
}

function
system_sleep_sig(){
    u64 nanoseconds = (microseconds * Thousand(1));
    u64 abs_sleep_time = (u64)((f64)nanoseconds *
                               ((f64)mac_vars.timebase_info.denom /
                                (f64)mac_vars.timebase_info.numer));

    u64 now = mach_absolute_time();
    mach_wait_until(now + abs_sleep_time);
}

////////////////////////////////

function
system_cli_call_sig(){
    b32 result = false;

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1){
        perror("system_cli_call: pipe");
        return(false);
    }

    pid_t child_pid = fork();
    if (child_pid == -1){
        perror("system_cli_call: fork");
        return(false);
    }

    enum { PIPE_FD_READ, PIPE_FD_WRITE };

    if (child_pid == 0){
        // NOTE(yuval): Child Process
        close(pipe_fds[PIPE_FD_READ]);
        dup2(pipe_fds[PIPE_FD_WRITE], STDOUT_FILENO);
        dup2(pipe_fds[PIPE_FD_WRITE], STDERR_FILENO);

        if (chdir(path) == -1){
            perror("system_cli_call: chdir");
            exit(1);
        }

        char* argv[] = {"sh", "-c", script, 0};

        if (execv("/bin/sh", argv) == -1){
            perror("system_cli_call: execv");
        }

        exit(1);
    } else{
        // NOTE(yuval): Parent Process
        close(pipe_fds[PIPE_FD_WRITE]);

        *(pid_t*)&cli_out->proc = child_pid;
        *(int*)&cli_out->out_read = pipe_fds[PIPE_FD_READ];
        *(int*)&cli_out->out_write = pipe_fds[PIPE_FD_WRITE];

        mac_vars.running_cli += 1;
    }

    return(true);
}

function
system_cli_begin_update_sig(){
    // NOTE(yuval): Nothing to do here.
}

function
system_cli_update_step_sig(){
    int pipe_read_fd = *(int*)&cli->out_read;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(pipe_read_fd, &fds);

    struct timeval tv = {};

    size_t space_left = max;
    char* ptr = dest;

    while (space_left > 0 && (select(pipe_read_fd + 1, &fds, NULL, NULL, &tv) == 1)){
        ssize_t num = read(pipe_read_fd, ptr, space_left);
        if (num == -1){
            perror("system_cli_update_step: read");
        } else if (num == 0){
            // NOTE(inso): EOF
            break;
        } else{
            ptr += num;
            space_left -= num;
        }
    }

    *amount = (ptr - dest);

    b32 result = ((ptr - dest) > 0);
    return(result);
}

function
system_cli_end_update_sig(){
    b32 close_me = false;

    pid_t pid = *(pid_t*)&cli->proc;

    int status;
    if (pid && (waitpid(pid, &status, WNOHANG) > 0)){
        cli->exit = WEXITSTATUS(status);

        close(*(int*)&cli->out_read);
        close(*(int*)&cli->out_write);

        mac_vars.running_cli -= 1;

        close_me = true;
    }

    return(close_me);
}

////////////////////////////////

function
system_open_color_picker_sig(){
    NotImplemented;
}

function
system_get_screen_scale_factor_sig(){
    f32 result = mac_vars.screen_scale_factor;
    return(result);
}

////////////////////////////////

function void*
mac_thread_wrapper(void *ptr){
    Mac_Object *object = (Mac_Object*)ptr;
    Thread_Function *proc = object->thread.proc;
    void *object_ptr = object->thread.ptr;

    pthread_mutex_lock(&mac_vars.thread_launch_mutex);
    {
        mac_vars.waiting_for_launch = false;
        pthread_cond_signal(&mac_vars.thread_launch_cv);
    }
    pthread_mutex_unlock(&mac_vars.thread_launch_mutex);

    proc(object_ptr);

    return(0);
}

function
system_thread_launch_sig(){
    Mac_Object *object = mac_alloc_object(MacObjectKind_Thread);
    object->thread.proc = proc;
    object->thread.ptr = ptr;

    pthread_mutex_lock(&mac_vars.thread_launch_mutex);
    {
        mac_vars.waiting_for_launch = true;
        pthread_create(&object->thread.thread, 0, mac_thread_wrapper, object);

        while (mac_vars.waiting_for_launch){
            pthread_cond_wait(&mac_vars.thread_launch_cv, &mac_vars.thread_launch_mutex);
        }
    }
    pthread_mutex_unlock(&mac_vars.thread_launch_mutex);

    System_Thread result = mac_to_plat_handle(object);
    return(result);
}

function
system_thread_join_sig(){
    Mac_Object *object = mac_to_object(thread);
    if (object->kind == MacObjectKind_Thread){
        pthread_join(object->thread.thread, 0);
    }
}

function
system_thread_free_sig(){
    Mac_Object* object = mac_to_object(thread);
    if (object->kind == MacObjectKind_Thread){
        mac_free_object(object);
    }
}

function
system_thread_get_id_sig(){
    pthread_t id = pthread_self();
    i32 result = *(i32*)(&id);
    return(result);
}

function
system_mutex_make_sig(){
    Mac_Object *object = mac_alloc_object(MacObjectKind_Mutex);
    mac_init_recursive_mutex(&object->mutex);

    System_Mutex result = mac_to_plat_handle(object);
    return(result);
}

function
system_mutex_acquire_sig(){
    Mac_Object *object = mac_to_object(mutex);
    if (object->kind == MacObjectKind_Mutex){
        pthread_mutex_lock(&object->mutex);
    }
}

function
system_mutex_release_sig(){
    Mac_Object *object = mac_to_object(mutex);
    if (object->kind == MacObjectKind_Mutex){
        pthread_mutex_unlock(&object->mutex);
    }
}

function
system_mutex_free_sig(){
    Mac_Object *object = mac_to_object(mutex);
    if (object->kind == MacObjectKind_Mutex){
        pthread_mutex_destroy(&object->mutex);
        mac_free_object(object);
    }
}

function
system_acquire_global_frame_mutex_sig(){
    if (tctx->kind == ThreadKind_AsyncTasks ||
        tctx->kind == ThreadKind_Main){
        system_mutex_acquire(mac_vars.global_frame_mutex);
    }
}

function
system_release_global_frame_mutex_sig(){
    if (tctx->kind == ThreadKind_AsyncTasks ||
        tctx->kind == ThreadKind_Main){
        system_mutex_release(mac_vars.global_frame_mutex);
    }
}

function
system_condition_variable_make_sig(){
    Mac_Object *object = mac_alloc_object(MacObjectKind_CV);
    pthread_cond_init(&object->cv, 0);

    System_Condition_Variable result = mac_to_plat_handle(object);
    return(result);
}

function
system_condition_variable_wait_sig(){
    Mac_Object *object_cv = mac_to_object(cv);
    Mac_Object *object_mutex = mac_to_object(mutex);
    if ((object_cv->kind == MacObjectKind_CV) && (object_mutex->kind == MacObjectKind_Mutex)){
        pthread_cond_wait(&object_cv->cv, &object_mutex->mutex);
    }
}

function
system_condition_variable_signal_sig(){
    Mac_Object *object = mac_to_object(cv);
    if (object->kind == MacObjectKind_CV){
        pthread_cond_signal(&object->cv);
    }
}

function
system_condition_variable_free_sig(){
    Mac_Object *object = mac_to_object(cv);
    if (object->kind == MacObjectKind_CV){
        pthread_cond_destroy(&object->cv);
        mac_free_object(object);
    }
}

////////////////////////////////

struct Memory_Annotation_Tracker_Node{
    Memory_Annotation_Tracker_Node *next;
    Memory_Annotation_Tracker_Node *prev;
    String_Const_u8 location;
    u64 size;
};

struct Memory_Annotation_Tracker{
    Memory_Annotation_Tracker_Node *first;
    Memory_Annotation_Tracker_Node *last;
    i32 count;
};

global Memory_Annotation_Tracker memory_tracker = {};
global pthread_mutex_t memory_tracker_mutex;

global_const u64 ALLOCATION_SIZE_ADJUSTMENT = 64;

function void*
mac_memory_allocate_extended(void *base, u64 size, String_Const_u8 location){
    u64 adjusted_size = size + ALLOCATION_SIZE_ADJUSTMENT;
    void *memory = mmap(base, adjusted_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    Assert(memory != MAP_FAILED);

    Memory_Annotation_Tracker_Node *node = (Memory_Annotation_Tracker_Node*)memory;

    pthread_mutex_lock(&memory_tracker_mutex);
    {
        zdll_push_back(memory_tracker.first, memory_tracker.last, node);
        memory_tracker.count += 1;
    }
    pthread_mutex_unlock(&memory_tracker_mutex);

    node->location = location;
    node->size = size;

    void* result = (node + 1);
    return(result);
}

function void
mac_memory_free_extended(void *ptr){
    Memory_Annotation_Tracker_Node *node = (Memory_Annotation_Tracker_Node*)ptr;
    node -= 1;

    pthread_mutex_lock(&memory_tracker_mutex);
    {
        zdll_remove(memory_tracker.first, memory_tracker.last, node);
        memory_tracker.count -= 1;
    }
    pthread_mutex_unlock(&memory_tracker_mutex);

    munmap(node, node->size + ALLOCATION_SIZE_ADJUSTMENT);
}

function
system_memory_allocate_sig(){
    void* result = mac_memory_allocate_extended(0, size, location);
    return(result);
}

function
system_memory_set_protection_sig(){
    b32 result = true;

    int protect = 0;
    switch (flags & 0x7){
        case 0:
        {
            protect = PROT_NONE;
        } break;

        case MemProtect_Read:
        {
            protect = PROT_READ;
        } break;

        case MemProtect_Write:
        case MemProtect_Read | MemProtect_Write:
        {
            protect = PROT_READ | PROT_WRITE;
        } break;

        case MemProtect_Execute:
        {
            protect = PROT_EXEC;
        } break;

        case MemProtect_Execute | MemProtect_Read:
        {
            protect = PROT_READ | PROT_EXEC;
        } break;

        // NOTE(inso): some W^X protection things might be unhappy about this one
        case MemProtect_Execute | MemProtect_Write:
        case MemProtect_Execute | MemProtect_Write | MemProtect_Read:
        {
            protect = PROT_READ | PROT_WRITE | PROT_EXEC;
        } break;
    }

    Memory_Annotation_Tracker_Node *node = (Memory_Annotation_Tracker_Node*)ptr;
    node -= 1;

    if(mprotect(node, size, protect) == -1){
        result = false;
    }

    return(result);
}

function
system_memory_free_sig(){
    mac_memory_free_extended(ptr);
}

function
system_memory_annotation_sig(){
    Memory_Annotation result = {};

    pthread_mutex_lock(&memory_tracker_mutex);
    {
        for (Memory_Annotation_Tracker_Node *node = memory_tracker.first;
             node != 0;
             node = node->next){
            Memory_Annotation_Node *r_node = push_array(arena, Memory_Annotation_Node, 1);
            sll_queue_push(result.first, result.last, r_node);
            result.count += 1;

            r_node->location = node->location;
            r_node->address = node + 1;
            r_node->size = node->size;
        }

    }
    pthread_mutex_unlock(&memory_tracker_mutex);

    return(result);
}

////////////////////////////////

function
system_show_mouse_cursor_sig(){
    mac_vars.cursor_show = show;
}

function
system_set_fullscreen_sig(){
    // NOTE(yuval): Read comment in system_set_fullscreen_sig in win32_4ed.cpp
    mac_vars.do_toggle = (mac_vars.full_screen != full_screen);

    b32 success = true;
    return(success);
}

function
system_is_fullscreen_sig(){
    // NOTE(yuval): Read comment in system_is_fullscreen_sig in win32_4ed.cpp
    b32 result = (mac_vars.full_screen != mac_vars.do_toggle);
    return(result);
}

function
system_get_keyboard_modifiers_sig(){
    Input_Modifier_Set result = copy_modifier_set(arena, &mac_vars.input_chunk.pers.modifiers);
    return(result);
}

function
system_set_key_mode_sig(){
    mac_vars.key_mode = mode;
}

internal void
system_set_source_mixer(void* ctx, Audio_Mix_Sources_Function* mix_func){
    // TODO(allen): Audio on Mac
}

internal void
system_set_destination_mixer(Audio_Mix_Destination_Function* mix_func){
    // TODO(allen): Audio on Mac
}

////////////////////////////////

/**********************/
/*    Graphics API    */
/**********************/

////////////////////////////////

function
graphics_get_texture_sig(){
    u32 result = renderer->get_texture(renderer, dim, texture_kind);
    return(result);
}

function
graphics_fill_texture_sig(){
    b32 result = renderer->fill_texture(renderer, texture_kind, texture, p, dim, data);
    return(result);
}

////////////////////////////////

/******************/
/*    Font API    */
/******************/

////////////////////////////////

function
font_make_face_sig(){
    Face* result = ft__font_make_face(arena, description, scale_factor);
    return(result);
}

////////////////////////////////
