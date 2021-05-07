/*
 * chr  - Andrew Chronister &
 * inso - Alex Baines
 *
 * 02.02.2020
 *
 * Updated linux layer for 4coder
 *
 */

global char *lnx_override_user_directory = 0;

internal String_Const_u8
system_get_path(Arena* arena, System_Path_Code path_code){
    String_Const_u8 result = {};
    
    switch (path_code){
        case SystemPath_CurrentDirectory: {
            // glibc extension: getcwd allocates its own memory if passed NULL
            char *working_dir = getcwd(NULL, 0);
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
        } break;
        
        case SystemPath_UserDirectory:
        {
            if (lnx_override_user_directory == 0){
                char *home_cstr = getenv("HOME");
                if (home_cstr != 0){
                    result = push_u8_stringf(arena, "%s/.4coder/", home_cstr);
                }
            }
            else{
                result = SCu8((u8*)lnx_override_user_directory);
            }
        }break;
    }
    
    return(result);
}

internal String_Const_u8
system_get_canonical(Arena* arena, String_Const_u8 name){
    
    // first remove redundant ../, //, ./ parts
    
    const u8* input = (u8*) strndupa((char*)name.str, name.size);
    u8* output = push_array(arena, u8, name.size + 1);
    
    const u8* p = input;
    u8* q = output;
    
    while(*p) {
        
        // not a slash - copy char
        if(p[0] != '/') {
            *q++ = *p++;
            continue;
        }
        
        // two slashes in a row, skip one.
        if(p[1] == '/') {
            ++p;
        }
        else if(p[1] == '.') {
            
            // skip "/./" or trailing "/."
            if(p[2] == '/' || p[2] == '\0') {
                p += 2;
            }
            
            // if we encounter "/../" or trailing "/..", remove last directory instead
            else if(p[2] == '.' && (p[3] == '/' || p[3] == '\0')) {
                while(q > output && *--q != '/'){};
                p += 3;
            }
            
            else {
                *q++ = *p++;
            }
        }
        else {
            *q++ = *p++;
        }
    }
    
#ifdef INSO_DEBUG
    if(name.size != q - output) {
        LINUX_FN_DEBUG("[%.*s] -> [%.*s]", (int)name.size, name.str, (int)(q - output), output);
    }
#endif
    
    // TODO: use realpath at this point to resolve symlinks?
    return SCu8(output, q - output);
}

internal File_List
system_get_file_list(Arena* arena, String_Const_u8 directory){
    //LINUX_FN_DEBUG("%.*s", (int)directory.size, directory.str);
    File_List result = {};
    
    char* path = strndupa((char*)directory.str, directory.size);
    int fd = open(path, O_RDONLY | O_DIRECTORY);
    if(fd == -1) {
        perror("open");
        return result;
    }
    
    DIR* dir = fdopendir(fd);
    struct dirent* d;
    
    File_Info* head = NULL;
    File_Info** fip = &head;
    
    while((d = readdir(dir))) {
        const char* name = d->d_name;
        
        // ignore . and ..
        if(*name == '.' && (name[1] == '\0' || (name[1] == '.' && name[2] == '\0'))) {
            continue;
        }
        
        *fip = push_array_zero(arena, File_Info, 1);
        (*fip)->file_name = push_u8_stringf(arena, "%.*s", d->d_reclen, name);
        
        struct stat st;
        if (fstatat(fd, name, &st, 0) == -1){
            perror("fstatat");
        }
        else{
            (*fip)->attributes = linux_file_attributes_from_struct_stat(&st);
        }
        
        fip = &(*fip)->next;
        result.count++;
    }
    closedir(dir);
    
    if(result.count > 0) {
        result.infos = fip = push_array(arena, File_Info*, result.count);
        
        for(File_Info* f = head;
            f != 0;
            f = f->next) {
            *fip++ = f;
        }
        
        qsort(result.infos, result.count, sizeof(File_Info*), (__compar_fn_t)&linux_compare_file_infos);
        
        for(u32 i = 0; i < result.count - 1; ++i) {
            result.infos[i]->next = result.infos[i+1];
        }
        result.infos[result.count-1]->next = NULL;
    }
    
    return result;
}

internal File_Attributes
system_quick_file_attributes(Arena* scratch, String_Const_u8 file_name){
    //LINUX_FN_DEBUG("%.*s", (int)file_name.size, file_name.str);
    Temp_Memory_Block temp(scratch);
    file_name = push_string_copy(scratch, file_name);
    File_Attributes result = {};
    struct stat file_stat;
    if (stat((const char*)file_name.str, &file_stat) == 0){
        result = linux_file_attributes_from_struct_stat(&file_stat);
    }
    return(result);
}

internal b32
system_load_handle(Arena* scratch, char* file_name, Plat_Handle* out){
    LINUX_FN_DEBUG("%s", file_name);
    int fd = open(file_name, O_RDONLY);
    if (fd != -1) {
        *(int*)out = fd;
        return true;
    }
    return false;
}

internal File_Attributes
system_load_attributes(Plat_Handle handle){
    LINUX_FN_DEBUG();
    File_Attributes result = {};
    struct stat file_stat;
    if (fstat(*(int*)&handle, &file_stat) == 0){
        result = linux_file_attributes_from_struct_stat(&file_stat);
    }
    return(result);
}

internal b32
system_load_file(Plat_Handle handle, char* buffer, u32 size){
    LINUX_FN_DEBUG("%.*s", size, buffer);
    int fd = *(int*)&handle;
    int bytes_read = read(fd, buffer, size);
    if (bytes_read == size) {
        return true;
    }
    return false;
}

internal b32
system_load_close(Plat_Handle handle){
    LINUX_FN_DEBUG();
    int fd = *(int*)&handle;
    return close(fd) == 0;
}

internal File_Attributes
system_save_file(Arena* scratch, char* file_name, String_Const_u8 data){
    LINUX_FN_DEBUG("%s", file_name);
    File_Attributes result = {};
    
    // TODO(inso): should probably put a \n on the end if it's a text file.
    
    int fd = open(file_name, O_TRUNC|O_WRONLY|O_CREAT, 0666);
    if (fd != -1) {
        int bytes_written = write(fd, data.str, data.size);
        if (bytes_written == -1) {
            perror("write");
        } else if (bytes_written == data.size) {
            struct stat file_stat;
            if (fstat(fd, &file_stat) == 0){
                result = linux_file_attributes_from_struct_stat(&file_stat);
            }
        }
        close(fd);
    } else {
        perror("open");
    }
    
    return result;
}

internal b32
system_load_library(Arena* scratch, String_Const_u8 file_name, System_Library* out){
    LINUX_FN_DEBUG("%.*s", (int)file_name.size, file_name.str);
    void* library = dlopen((const char*)file_name.str, RTLD_LAZY);
    if (library != NULL) {
        *(void**)out = library;
        return true;
    }
    return false;
}

internal b32
system_release_library(System_Library handle){
    LINUX_FN_DEBUG();
    return dlclose(*(void**)&handle) == 0;
}

internal Void_Func*
system_get_proc(System_Library handle, char* proc_name){
    LINUX_FN_DEBUG("%s", proc_name);
    return (Void_Func*)dlsym(*(void**)&handle, proc_name);
}

internal u64
system_now_time(void){
    //LINUX_FN_DEBUG();
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return linux_us_from_timespec(time);
}

function void
linux_date_time_from_tm(Date_Time *out, struct tm *in){
    out->year = in->tm_year + 1900;
    out->mon = in->tm_mon;
    out->day = in->tm_mday - 1;
    out->hour = in->tm_hour;
    out->min = in->tm_min;
    out->sec = in->tm_sec;
    out->msec = 0;
}

function void
linux_tm_from_date_time(struct tm *out, Date_Time *in){
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
    linux_date_time_from_tm(&result, now_tm);
    return(result);
}

function
system_local_date_time_from_universal_sig(){
    struct tm univ_tm = {};
    linux_tm_from_date_time(&univ_tm, date_time);
    time_t utc_time = timegm(&univ_tm);
    struct tm *local_tm = localtime(&utc_time);
    Date_Time result = {};
    linux_date_time_from_tm(&result, local_tm);
    return(result);
}

function
system_universal_date_time_from_local_sig(){
    struct tm local_tm = {};
    linux_tm_from_date_time(&local_tm, date_time);
    time_t loc_time = timelocal(&local_tm);
    struct tm *utc_tm = gmtime(&loc_time);
    Date_Time result = {};
    linux_date_time_from_tm(&result, utc_tm);
    return(result);
}

internal Plat_Handle
system_wake_up_timer_create(void){
    LINUX_FN_DEBUG();
    Linux_Object* object = linux_alloc_object(LinuxObjectKind_Timer);
    dll_insert(&linuxvars.timer_objects, &object->node);
    
    // NOTE(inso): timers created on-demand to avoid file-descriptor exhaustion.
    object->timer.fd = -1;
    return object_to_handle(object);
}

internal void
system_wake_up_timer_release(Plat_Handle handle){
    LINUX_FN_DEBUG();
    Linux_Object* object = handle_to_object(handle);
    if (object->kind == LinuxObjectKind_Timer){
        if(object->timer.fd != -1) {
            close(object->timer.fd);
            object->timer.fd = -1;
        }
        linux_free_object(object);
    }
}

internal void
system_wake_up_timer_set(Plat_Handle handle, u32 time_milliseconds){
    //LINUX_FN_DEBUG("%u", time_milliseconds);
    Linux_Object* object = handle_to_object(handle);
    
    if (object->kind == LinuxObjectKind_Timer){
        if(object->timer.fd == -1) {
            object->timer.fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
            
            struct epoll_event ev;
            ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
            ev.data.ptr = &object->timer.epoll_tag;
            epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, object->timer.fd, &ev);
        }
        
        struct itimerspec it = {};
        it.it_value.tv_sec = time_milliseconds / 1000;
        it.it_value.tv_nsec = (time_milliseconds % 1000) * UINT64_C(1000000);
        timerfd_settime(object->timer.fd, 0, &it, NULL);
    }
    
}

internal void
system_signal_step(u32 code){
    LINUX_FN_DEBUG("%d", code);
    linux_schedule_step();
}

internal void
system_sleep(u64 microseconds){
    //LINUX_FN_DEBUG("%" PRIu64, microseconds);
    struct timespec requested;
    struct timespec remaining;
    u64 seconds = microseconds / Million(1);
    requested.tv_sec = seconds;
    requested.tv_nsec = (microseconds - seconds * Million(1)) * Thousand(1);
    nanosleep(&requested, &remaining);
}

internal b32
system_cli_call(Arena* scratch, char* path, char* script, CLI_Handles* cli_out){
    LINUX_FN_DEBUG("%s / %s", path, script);
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
    //LINUX_FN_DEBUG();
}

internal b32
system_cli_update_step(CLI_Handles* cli, char* dest, u32 max, u32* amount){
    LINUX_FN_DEBUG();
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
    LINUX_FN_DEBUG();
    pid_t pid = *(pid_t*)&cli->proc;
    b32 close_me = false;
    
    int status;
    if (pid && waitpid(pid, &status, WNOHANG) > 0){
        cli->exit = WEXITSTATUS(status);
        
        close_me = true;
        close(*(int*)&cli->out_read);
        close(*(int*)&cli->out_write);
    }
    
    return(close_me);
}

internal void
system_open_color_picker(Color_Picker* picker){
    // TODO?
    LINUX_FN_DEBUG();
}

internal f32
system_get_screen_scale_factor(void){
    LINUX_FN_DEBUG();
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
    LINUX_FN_DEBUG();
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
        static_assert(sizeof(Linux_Object*) <= sizeof(System_Thread), "Linux_Object doesn't fit inside System_Thread");
        *(Linux_Object**)&result = thread_info;
        return result;
    }
    
    return result;
}

internal void
system_thread_join(System_Thread thread){
    LINUX_FN_DEBUG();
    Linux_Object* object = *(Linux_Object**)&thread;
    void* retval_ignored;
    int result = pthread_join(object->thread.pthread, &retval_ignored);
}

internal void
system_thread_free(System_Thread thread){
    LINUX_FN_DEBUG();
    Linux_Object* object = *(Linux_Object**)&thread;
    Assert(object->kind == LinuxObjectKind_Thread);
    linux_free_object(object);
}

internal i32
system_thread_get_id(void){
    pid_t id = syscall(__NR_gettid);
    //LINUX_FN_DEBUG("%d", id);
    return id;
}

internal void
system_acquire_global_frame_mutex(Thread_Context* tctx){
    //LINUX_FN_DEBUG();
    if (tctx->kind == ThreadKind_AsyncTasks ||
        tctx->kind == ThreadKind_Main){
        system_mutex_acquire(linuxvars.global_frame_mutex);
    }
}

internal void
system_release_global_frame_mutex(Thread_Context* tctx){
    //LINUX_FN_DEBUG();
    if (tctx->kind == ThreadKind_AsyncTasks ||
        tctx->kind == ThreadKind_Main){
        system_mutex_release(linuxvars.global_frame_mutex);
    }
}

internal System_Mutex
system_mutex_make(void){
    System_Mutex result = {};
    Linux_Object* object = linux_alloc_object(LinuxObjectKind_Mutex);
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&object->mutex, &attr);
    *(Linux_Object**)&result = object;
    //LINUX_FN_DEBUG("%p", object);
    return result;
}

internal void
system_mutex_acquire(System_Mutex mutex){
    Linux_Object* object = *(Linux_Object**)&mutex;
    //LINUX_FN_DEBUG("%p", object);
    Assert(object->kind == LinuxObjectKind_Mutex);
    pthread_mutex_lock(&object->mutex);
}

internal void
system_mutex_release(System_Mutex mutex){
    Linux_Object* object = *(Linux_Object**)&mutex;
    //LINUX_FN_DEBUG("%p", object);
    Assert(object->kind == LinuxObjectKind_Mutex);
    pthread_mutex_unlock(&object->mutex);
}

internal void
system_mutex_free(System_Mutex mutex){
    Linux_Object* object = *(Linux_Object**)&mutex;
    //LINUX_FN_DEBUG("%p", object);
    Assert(object->kind == LinuxObjectKind_Mutex);
    pthread_mutex_destroy(&object->mutex);
    linux_free_object(object);
}

internal System_Condition_Variable
system_condition_variable_make(void){
    System_Condition_Variable result = {};
    Linux_Object* object = linux_alloc_object(LinuxObjectKind_ConditionVariable);
    //LINUX_FN_DEBUG("%p", object);
    pthread_cond_init(&object->condition_variable, NULL);
    *(Linux_Object**)&result = object;
    return result;
}

internal void
system_condition_variable_wait(System_Condition_Variable cv, System_Mutex mutex){
    Linux_Object* cv_object = *(Linux_Object**)&cv;
    Linux_Object* mutex_object = *(Linux_Object**)&mutex;
    //LINUX_FN_DEBUG("%p / %p", cv_object, mutex_object);
    Assert(cv_object->kind == LinuxObjectKind_ConditionVariable);
    Assert(mutex_object->kind == LinuxObjectKind_Mutex);
    pthread_cond_wait(&cv_object->condition_variable, &mutex_object->mutex);
}

internal void
system_condition_variable_signal(System_Condition_Variable cv){
    Linux_Object* object = *(Linux_Object**)&cv;
    //LINUX_FN_DEBUG("%p", object);
    Assert(object->kind == LinuxObjectKind_ConditionVariable);
    pthread_cond_signal(&object->condition_variable);
}

internal void
system_condition_variable_free(System_Condition_Variable cv){
    Linux_Object* object = *(Linux_Object**)&cv;
    LINUX_FN_DEBUG("%p", &object->condition_variable);
    Assert(object->kind == LinuxObjectKind_ConditionVariable);
    pthread_cond_destroy(&object->condition_variable);
    linux_free_object(object);
}

#define MEMORY_PREFIX_SIZE 64

internal void*
system_memory_allocate(u64 size, String_Const_u8 location){
    
    static_assert(MEMORY_PREFIX_SIZE >= sizeof(Memory_Annotation_Node), "MEMORY_PREFIX_SIZE is not enough to contain Memory_Annotation_Node");
    u64 adjusted_size = size + MEMORY_PREFIX_SIZE;
    
    Assert(adjusted_size > size);
    
    const int prot  = PROT_READ | PROT_WRITE;
    const int flags = MAP_PRIVATE | MAP_ANONYMOUS;
    
    void* result = mmap(NULL, adjusted_size, prot, flags, -1, 0);
    
    if(result == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    
    Linux_Memory_Tracker_Node* node = (Linux_Memory_Tracker_Node*)result;
    node->location = location;
    node->size = size;
    
    pthread_mutex_lock(&linuxvars.memory_tracker_mutex);
    zdll_push_back(linuxvars.memory_tracker_head, linuxvars.memory_tracker_tail, node);
    linuxvars.memory_tracker_count++;
    pthread_mutex_unlock(&linuxvars.memory_tracker_mutex);
    
    return (u8*)result + MEMORY_PREFIX_SIZE;
}

internal b32
system_memory_set_protection(void* ptr, u64 size, u32 flags){
    LINUX_FN_DEBUG("%p / %ld / %d", ptr, size, flags);
    int protect = 0;
    MovFlag(flags, MemProtect_Read, protect, PROT_READ);
    MovFlag(flags, MemProtect_Write, protect, PROT_WRITE);
    MovFlag(flags, MemProtect_Execute, protect, PROT_EXEC);
    int result = mprotect(ptr, size, protect);
    return result == 0;
}

internal void
system_memory_free(void* ptr, u64 size){
    u64 adjusted_size = size + MEMORY_PREFIX_SIZE;
    Linux_Memory_Tracker_Node* node = (Linux_Memory_Tracker_Node*)((u8*)ptr - MEMORY_PREFIX_SIZE);
    
    pthread_mutex_lock(&linuxvars.memory_tracker_mutex);
    zdll_remove(linuxvars.memory_tracker_head, linuxvars.memory_tracker_tail, node);
    linuxvars.memory_tracker_count--;
    pthread_mutex_unlock(&linuxvars.memory_tracker_mutex);
    
    if(munmap(node, adjusted_size) == -1) {
        perror("munmap");
    }
}

internal Memory_Annotation
system_memory_annotation(Arena* arena){
    LINUX_FN_DEBUG();
    
    Memory_Annotation result;
    Memory_Annotation_Node** ptr = &result.first;
    
    pthread_mutex_lock(&linuxvars.memory_tracker_mutex);
    
    for(Linux_Memory_Tracker_Node* node = linuxvars.memory_tracker_head; node; node = node->next) {
        *ptr = push_array(arena, Memory_Annotation_Node, 1);
        (*ptr)->location = node->location;
        (*ptr)->size = node->size;
        (*ptr)->address = (u8*)node + MEMORY_PREFIX_SIZE;
        ptr = &(*ptr)->next;
        result.count++;
    }
    
    pthread_mutex_unlock(&linuxvars.memory_tracker_mutex);
    
    *ptr = NULL;
    result.last = CastFromMember(Memory_Annotation_Node, next, ptr);
    
    return result;
}

internal void
system_show_mouse_cursor(i32 show){
    LINUX_FN_DEBUG("%d", show);
    
    linuxvars.cursor_show = show;
    
    XDefineCursor(
                  linuxvars.dpy,
                  linuxvars.win,
                  show ? None : linuxvars.hidden_cursor);
}

internal b32
system_set_fullscreen(b32 full_screen){
    linux_window_fullscreen(full_screen ? WM_STATE_ADD : WM_STATE_DEL);
    return true;
}

internal b32
system_is_fullscreen(void){
    b32 result = 0;
    
    // NOTE(inso): This will get the "true" state of fullscreen,
    // even if it was toggled outside of 4coder.
    // (e.g. super-F11 on some WMs sets fullscreen for any window/program)
    
    Atom type, *prop;
    unsigned long nitems, pad;
    int fmt;
    int ret = XGetWindowProperty(linuxvars.dpy,
                                 linuxvars.win,
                                 linuxvars.atom__NET_WM_STATE,
                                 0, 32, False, XA_ATOM,
                                 &type, &fmt, &nitems, &pad,
                                 (unsigned char**)&prop);
    
    if(ret == Success && prop){
        result = *prop == linuxvars.atom__NET_WM_STATE_FULLSCREEN;
        XFree((unsigned char*)prop);
    }
    
    return result;
}

internal Input_Modifier_Set
system_get_keyboard_modifiers(Arena* arena){
    //LINUX_FN_DEBUG();
    return(copy_modifier_set(arena, &linuxvars.input.pers.modifiers));
}

function
system_set_key_mode_sig(){
    linuxvars.key_mode = mode;
}

internal void
system_set_source_mixer(void* ctx, Audio_Mix_Sources_Function* mix_func){
    pthread_mutex_lock(&linuxvars.audio_mutex);
    linuxvars.audio_ctx = ctx;
    linuxvars.audio_src_func = mix_func;
    pthread_mutex_unlock(&linuxvars.audio_mutex);
}

internal void
system_set_destination_mixer(Audio_Mix_Destination_Function* mix_func){
    pthread_mutex_lock(&linuxvars.audio_mutex);
    linuxvars.audio_dst_func = mix_func;
    pthread_mutex_unlock(&linuxvars.audio_mutex);
}

// NOTE(inso): to prevent me continuously messing up indentation
// vim: et:ts=4:sts=4:sw=4
