/*
 * chr  - Andrew Chronister &
 * inso - Alex Baines
 *
 * 02.02.2020
 *
 * Updated linux layer for 4coder
 *
 */

internal String_Const_u8
system_get_path(Arena* arena, System_Path_Code path_code){
    String_Const_u8 result = {};

    switch (path_code){
        case SystemPath_CurrentDirectory: {
            // glibc extension: getcwd allocates its own memory if passed NULL
            char *working_dir = getcwd(NULL, 0);
            LINUX_FN_DEBUG("cwd = [%s]", working_dir);

            u64 working_dir_len = cstring_length(working_dir);
            u8 *out = push_array(arena, u8, working_dir_len + 1);
            block_copy(out, working_dir, working_dir_len);

            // NOTE: 4ed appears to expect a slash on the end.
            out[working_dir_len] = '/';

            free(working_dir);
            result = SCu8(out, working_dir_len + 1);
        } break;

        case SystemPath_Binary: {
            // linux-specific: binary path symlinked at /proc/self/exe
            // PATH_MAX is probably good enough...
            // read the 'readlink' manpage for some comedy about it being 'broken by design'.

            char* buf = push_array(arena, char, PATH_MAX);
            ssize_t n = readlink("/proc/self/exe", buf, PATH_MAX);

            if(n == -1) {
                perror("readlink");
                *buf = n = 0;
            }

            result = string_remove_last_folder(SCu8(buf, n));

            LINUX_FN_DEBUG("bin dir = [%.*s]", (int)result.size, result.str);
        } break;
    }

    return(result);
}

internal String_Const_u8
system_get_canonical(Arena* arena, String_Const_u8 name){
    // TODO(andrew): Resolve symlinks ?
    // TODO(andrew): Resolve . and .. in paths
    // TODO(andrew): Use realpath(3)
    return name;
}

internal File_List
system_get_file_list(Arena* arena, String_Const_u8 directory){
    File_List result = {};
    String_Const_u8 search_pattern = {};

    if (character_is_slash(string_get_character(directory, directory.size - 1))){
        search_pattern = push_u8_stringf(arena, "%.*s*", string_expand(directory));
    }
    else{
        search_pattern = push_u8_stringf(arena, "%.*s/*", string_expand(directory));
    }

    struct dirent** dir_ents = NULL;
    int num_ents = scandir(
        (const char*)search_pattern.str, &dir_ents, linux_system_get_file_list_filter, alphasort);

    File_Info *first = 0;
    File_Info *last = 0;
    for (int i = 0; i < num_ents; ++i) {
        struct dirent* dirent = dir_ents[i];
        File_Info *info = push_array(arena, File_Info, 1);
        sll_queue_push(first, last, info);

        info->file_name = SCu8((u8*)dirent->d_name);

        struct stat file_stat;
        stat((const char*)dirent->d_name, &file_stat);
        info->attributes = linux_file_attributes_from_struct_stat(file_stat);
    }

    result.infos = push_array(arena, File_Info*, num_ents);
    result.count = num_ents;

    i32 info_index = 0;
    for (File_Info* node = first; node != NULL; node = node->next) {
        result.infos[info_index] = node;
        info_index += 1;
    }

    return result;
}

internal File_Attributes
system_quick_file_attributes(Arena* scratch, String_Const_u8 file_name){
    struct stat file_stat;
    stat((const char*)file_name.str, &file_stat);
    return linux_file_attributes_from_struct_stat(file_stat);
}

internal b32
system_load_handle(Arena* scratch, char* file_name, Plat_Handle* out){
    int fd = open(file_name, O_RDONLY);
    if (fd != -1) {
        *(int*)out = fd;
        return true;
    }
    return false;
}

internal File_Attributes
system_load_attributes(Plat_Handle handle){
    struct stat file_stat;
    fstat(*(int*)&handle, &file_stat);
    return linux_file_attributes_from_struct_stat(file_stat);
}

internal b32
system_load_file(Plat_Handle handle, char* buffer, u32 size){
    int fd = *(int*)&handle;
    int bytes_read = read(fd, buffer, size);
    if (bytes_read == size) {
        return true;
    }
    return false;
}

internal b32
system_load_close(Plat_Handle handle){
    int fd = *(int*)&handle;
    return close(fd) == 0;
}

internal File_Attributes
system_save_file(Arena* scratch, char* file_name, String_Const_u8 data){
    File_Attributes result = {};
    int fd = open(file_name, O_WRONLY, O_CREAT);
    if (fd != -1) {
        int bytes_written = write(fd, data.str, data.size);
        if (bytes_written != -1) {
            struct stat file_stat;
            fstat(fd, &file_stat);
            return linux_file_attributes_from_struct_stat(file_stat);
        }
    }
    return result;
}

internal b32
system_load_library(Arena* scratch, String_Const_u8 file_name, System_Library* out){
    void* library = dlopen((const char*)file_name.str, RTLD_LAZY);
    if (library != NULL) {
        *(void**)out = library;
        return true;
    }
    return false;
}

internal b32
system_release_library(System_Library handle){
    return dlclose(*(void**)&handle) == 0;
}

internal Void_Func*
system_get_proc(System_Library handle, char* proc_name){
    return (Void_Func*)dlsym(*(void**)&handle, proc_name);
}

internal u64
system_now_time(void){
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &time);
    return linux_u64_from_timespec(time);
}

internal Plat_Handle
system_wake_up_timer_create(void){
    Linux_Object* object = linux_alloc_object(LinuxObjectKind_Timer);
    dll_insert(&linuxvars.timer_objects, &object->node);

    // NOTE(inso): timers created on-demand to avoid file-descriptor exhaustion.
    object->timer.fd = -1;
}

internal void
system_wake_up_timer_release(Plat_Handle handle){
    Linux_Object* object = handle_to_object(handle);
    if (object->kind == LinuxObjectKind_Timer){
        if(object->timer.fd != -1) {
            epoll_ctl(linuxvars.epoll, EPOLL_CTL_DEL, object->timer.fd, NULL);
            close(object->timer.fd);
            object->timer.fd = -1;
        }
        linux_free_object(object);
    }
}

internal void
system_wake_up_timer_set(Plat_Handle handle, u32 time_milliseconds){
    Linux_Object* object = handle_to_object(handle);

    if (object->kind == LinuxObjectKind_Timer){
        if(object->timer.fd == -1) {
            object->timer.fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
        }

        struct itimerspec it = {};
        it.it_value.tv_sec = time_milliseconds / 1000;
        it.it_value.tv_nsec = (time_milliseconds % 1000) * 1000000UL;

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
        ev.data.ptr = &object->timer.epoll_tag;

        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, object->timer.fd, &ev);
    }

}

internal void
system_signal_step(u32 code){
    linux_schedule_step();
}

internal void
system_sleep(u64 microseconds){
    struct timespec requested;
    struct timespec remaining;
    u64 seconds = microseconds / Million(1);
    requested.tv_sec = seconds;
    requested.tv_nsec = (microseconds - seconds * Million(1)) * Thousand(1);
    nanosleep(&requested, &remaining);
}

internal void
system_post_clipboard(String_Const_u8 str){
    linalloc_clear(&linuxvars.clipboard_out_arena);
    char* p = push_array(&linuxvars.clipboard_out_arena, char, str.size + 1);
    block_copy(p, str.data, str.size);
    p[str.size] = '\0';
    XSetSelectionOwner(linuxvars.dpy, linuxvars.atom_CLIPBOARD, linuxvars.win, CurrentTime);
}

internal b32
system_cli_call(Arena* scratch, char* path, char* script, CLI_Handles* cli_out){
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1){
        perror("system_cli_call: pipe");
        return 0;
    }

    pid_t child_pid = vfork();
    if (child_pid == -1){
        perror("system_cli_call: fork");
        return 0;
    }

    enum { PIPE_FD_READ, PIPE_FD_WRITE };

    // child
    if (child_pid == 0){
        close(pipe_fds[PIPE_FD_READ]);
        dup2(pipe_fds[PIPE_FD_WRITE], STDOUT_FILENO);
        dup2(pipe_fds[PIPE_FD_WRITE], STDERR_FILENO);

        if (chdir(path) == -1){
            perror("system_cli_call: chdir");
            exit(1);
        }

        char* argv[] = { "sh", "-c", script, NULL };

        if (execv("/bin/sh", argv) == -1){
            perror("system_cli_call: execv");
        }
        exit(1);
    }
    else{
        close(pipe_fds[PIPE_FD_WRITE]);

        *(pid_t*)&cli_out->proc = child_pid;
        *(int*)&cli_out->out_read = pipe_fds[PIPE_FD_READ];
        *(int*)&cli_out->out_write = pipe_fds[PIPE_FD_WRITE];

        struct epoll_event e = {};
        e.events = EPOLLIN | EPOLLET;
        e.data.ptr = &epoll_tag_cli_pipe;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, pipe_fds[PIPE_FD_READ], &e);
    }

    return(true);
}

internal void
system_cli_begin_update(CLI_Handles* cli){
    // NOTE(inso): I don't think anything needs to be done here.
}

internal b32
system_cli_update_step(CLI_Handles* cli, char* dest, u32 max, u32* amount){
    int pipe_read_fd = *(int*)&cli->out_read;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(pipe_read_fd, &fds);

    struct timeval tv = {};

    size_t space_left = max;
    char* ptr = dest;

    while (space_left > 0 && select(pipe_read_fd + 1, &fds, NULL, NULL, &tv) == 1){
        ssize_t num = read(pipe_read_fd, ptr, space_left);
        if (num == -1){
            perror("system_cli_update_step: read");
        } else if (num == 0){
            // NOTE(inso): EOF
            break;
        } else {
            ptr += num;
            space_left -= num;
        }
    }

    *amount = (ptr - dest);
    return((ptr - dest) > 0);
}

internal b32
system_cli_end_update(CLI_Handles* cli){
    pid_t pid = *(pid_t*)&cli->proc;
    b32 close_me = false;

    int status;
    if (pid && waitpid(pid, &status, WNOHANG) > 0){
        cli->exit = WEXITSTATUS(status);

        close_me = true;
        close(*(int*)&cli->out_read);
        close(*(int*)&cli->out_write);

        struct epoll_event e = {};
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_DEL, *(int*)&cli->out_read, &e);
    }

    return(close_me);
}

internal void
system_open_color_picker(Color_Picker* picker){
    // TODO?
}

internal f32
system_get_screen_scale_factor(void){
    // TODO: correct screen number somehow
    int dpi = linux_get_xsettings_dpi(linuxvars.dpy, 0);
    if(dpi == -1){
        int scr = DefaultScreen(linuxvars.dpy);
        int dw = DisplayWidth(linuxvars.dpy, scr);
        int dh = DisplayHeight(linuxvars.dpy, scr);
        int dw_mm = DisplayWidthMM(linuxvars.dpy, scr);
        int dh_mm = DisplayHeightMM(linuxvars.dpy, scr);
        int dpi_x = dw_mm ? dw / (dw_mm / 25.4) : 96;
        int dpi_y = dh_mm ? dh / (dh_mm / 25.4) : 96;
        dpi = dpi_x > dpi_y ? dpi_x : dpi_y;
    }
    return dpi / 96.0f;
}

internal System_Thread
system_thread_launch(Thread_Function* proc, void* ptr){
    System_Thread result = {};

    Linux_Object* thread_info = linux_alloc_object(LinuxObjectKind_Thread);
    thread_info->thread.proc = proc;
    thread_info->thread.ptr = ptr;

    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    int create_result = pthread_create(
        &thread_info->thread.pthread,
        &thread_attr,
        linux_thread_proc_start,
        thread_info);

    pthread_attr_destroy(&thread_attr);

    // TODO(andrew): Need to wait for thread to confirm it launched?
    if (create_result == 0) {
        static_assert(sizeof(Linux_Object*) <= sizeof(System_Thread));
        *(Linux_Object**)&result = thread_info;
        return result;
    }

    return result;
}

internal void
system_thread_join(System_Thread thread){
    Linux_Object* object = *(Linux_Object**)&thread;
    void* retval_ignored;
    int result = pthread_join(object->thread.pthread, &retval_ignored);
}

internal void
system_thread_free(System_Thread thread){
    Linux_Object* object = *(Linux_Object**)&thread;
    Assert(object->kind == LinuxObjectKind_Thread);
    linux_free_object(object);
}

internal i32
system_thread_get_id(void){
    pthread_t tid = pthread_self();
    Assert(tid <= (u64)max_i32);
    return (i32)tid;
}

internal void
system_acquire_global_frame_mutex(Thread_Context* tctx){
    if (tctx->kind == ThreadKind_AsyncTasks){
        system_mutex_acquire(linuxvars.global_frame_mutex);
    }
}

internal void
system_release_global_frame_mutex(Thread_Context* tctx){
    if (tctx->kind == ThreadKind_AsyncTasks){
        system_mutex_release(linuxvars.global_frame_mutex);
    }
}

internal System_Mutex
system_mutex_make(void){
    System_Mutex result = {};
    Linux_Object* object = linux_alloc_object(LinuxObjectKind_Mutex);
    pthread_mutex_init(&object->mutex, NULL);
    *(Linux_Object**)&result = object;
    return result;
}

internal void
system_mutex_acquire(System_Mutex mutex){
    Linux_Object* object = *(Linux_Object**)&mutex;
    Assert(object->kind == LinuxObjectKind_Mutex);
    pthread_mutex_lock(&object->mutex);
}

internal void
system_mutex_release(System_Mutex mutex){
    Linux_Object* object = *(Linux_Object**)&mutex;
    Assert(object->kind == LinuxObjectKind_Mutex);
    pthread_mutex_unlock(&object->mutex);
}

internal void
system_mutex_free(System_Mutex mutex){
    Linux_Object* object = *(Linux_Object**)&mutex;
    Assert(object->kind == LinuxObjectKind_Mutex);
    pthread_mutex_destroy(&object->mutex);
    linux_free_object(object);
}

internal System_Condition_Variable
system_condition_variable_make(void){
    System_Condition_Variable result = {};
    Linux_Object* object = linux_alloc_object(LinuxObjectKind_ConditionVariable);
    pthread_cond_init(&object->condition_variable, NULL);
    *(Linux_Object**)&result = object;
    return result;
}

internal void
system_condition_variable_wait(System_Condition_Variable cv, System_Mutex mutex){
    Linux_Object* cv_object = *(Linux_Object**)&cv;
    Linux_Object* mutex_object = *(Linux_Object**)&mutex;
    Assert(cv_object->kind == LinuxObjectKind_ConditionVariable);
    Assert(mutex_object->kind == LinuxObjectKind_Mutex);
    pthread_cond_wait(&cv_object->condition_variable, &mutex_object->mutex);
}

internal void
system_condition_variable_signal(System_Condition_Variable cv){
    Linux_Object* object = *(Linux_Object**)&cv;
    Assert(object->kind == LinuxObjectKind_ConditionVariable);
    pthread_cond_signal(&object->condition_variable);
}

internal void
system_condition_variable_free(System_Condition_Variable cv){
    Linux_Object* object = *(Linux_Object**)&cv;
    Assert(object->kind == LinuxObjectKind_ConditionVariable);
    pthread_cond_destroy(&object->condition_variable);
    linux_free_object(object);
}

internal void*
system_memory_allocate(u64 size, String_Const_u8 location){
    void* result = mmap(
        NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    // TODO(andrew): Allocation tracking?
    return result;
}

internal b32
system_memory_set_protection(void* ptr, u64 size, u32 flags){
    int protect = 0;
    MovFlag(flags, MemProtect_Read, protect, PROT_READ);
    MovFlag(flags, MemProtect_Write, protect, PROT_WRITE);
    MovFlag(flags, MemProtect_Execute, protect, PROT_EXEC);
    int result = mprotect(ptr, size, protect);
    return result == 0;
}

internal void
system_memory_free(void* ptr, u64 size){
    munmap(ptr, size);
}

internal Memory_Annotation
system_memory_annotation(Arena* arena){
    // TODO;
}

internal void
system_show_mouse_cursor(i32 show){
    linuxvars.cursor_show = show;

    XDefineCursor(
        linuxvars.dpy,
        linuxvars.win,
        show ? None : linuxvars.hidden_cursor);
}

internal b32
system_set_fullscreen(b32 full_screen){
    linuxvars.should_be_full_screen = full_screen;
    return true;
}

internal b32
system_is_fullscreen(void){
    return linuxvars.is_full_screen;
}

internal Input_Modifier_Set
system_get_keyboard_modifiers(Arena* arena){
    // TODO:
    //return(copy_modifier_set(arena, &linuxvars.input_chunk.pers.modifiers));
}
