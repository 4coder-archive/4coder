/*
 * chr  - Andrew Chronister &
 * inso - Alex Baines
 *
 * 12.19.2019
 *
 * Updated linux layer for 4coder
 *
 */

// TOP

#define FPS 60
#define frame_useconds (1000000 / FPS)
#define SLASH '/'
#define DLL "so"

#define container_of(ptr, type, member) ({            \
	const typeof(((type*)0)->member)* __mptr = (ptr); \
	(type*)((char*)__mptr - offsetof(type, member));  \
})

#include "4coder_base_types.h"
#include "4coder_version.h"
#include "4coder_events.h"

#include "4coder_table.h"
#include "4coder_types.h"
#include "4coder_default_colors.h"
#include "4coder_system_types.h"
#include "4ed_font_interface.h"

#define STATIC_LINK_API
#include "generated/system_api.h"

#define STATIC_LINK_API
#include "generated/graphics_api.h"

#define STATIC_LINK_API
#include "generated/font_api.h"

#include "4ed_font_set.h"
#include "4ed_render_target.h"
#include "4ed_search_list.h"
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

#include "4coder_hash_functions.cpp"
#include "4coder_system_allocator.cpp"
#include "4coder_malloc_allocator.cpp"
#include "4coder_codepoint_map.cpp"

#include "4ed_mem.cpp"
#include "4ed_font_set.cpp"
#include "4ed_search_list.cpp"
#include "4ed_font_provider_freetype.h"
#include "4ed_font_provider_freetype.cpp"

#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>
#include <errno.h>
#include <pthread.h>

#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/timerfd.h>

#define Cursor XCursor
#undef function
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xfixes.h>
#define function static
#undef Cursor

#include <GL/glx.h>
#include <GL/glext.h>

#ifdef INSO_DEBUG
    #define LINUX_FN_DEBUG(fmt, ...) do { \
        fprintf(stderr, "%s: " fmt "\n", __func__, ##__VA_ARGS__);\
    } while (0)
#else
    #define LINUX_FN_DEBUG(...)
#endif

////////////////////////////

struct Linux_Vars {
    Display* dpy;
    Window win;

    b32 has_xfixes;
    int xfixes_selection_event;
    XIM xim;
    XIC xic;

    Application_Step_Input input;

    int epoll;
    int step_timer_fd;
    u64 last_step_time;
    b32 is_full_screen;
    b32 should_be_full_screen;

    Application_Mouse_Cursor cursor;
    XCursor hidden_cursor;
    i32 cursor_show;
    i32 prev_cursor_show;

    Node free_linux_objects;
    Node timer_objects;

    Thread_Context tctx;

    System_Mutex global_frame_mutex;

    Arena clipboard_out_arena;
    String_Const_u8 clipboard_contents;
    b32 received_new_clipboard;

    Atom atom_TARGETS;
    Atom atom_CLIPBOARD;
    Atom atom_UTF8_STRING;
    Atom atom__NET_WM_STATE;
    Atom atom__NET_WM_STATE_MAXIMIZED_HORZ;
    Atom atom__NET_WM_STATE_MAXIMIZED_VERT;
    Atom atom__NET_WM_STATE_FULLSCREEN;
    Atom atom__NET_WM_PING;
    Atom atom__NET_WM_WINDOW_TYPE;
    Atom atom__NET_WM_WINDOW_TYPE_NORMAL;
    Atom atom__NET_WM_PID;
    Atom atom_WM_DELETE_WINDOW;
};

global Linux_Vars linuxvars;
global Render_Target render_target;

////////////////////////////

// Defererencing an epoll_event's .data.ptr will always give one of these event types.

typedef i32 Epoll_Kind;
enum {
    EPOLL_FRAME_TIMER,
    EPOLL_X11,
    EPOLL_X11_INTERNAL,
    EPOLL_CLI_PIPE,
    EPOLL_USER_TIMER,
};

// Where per-event epoll data is not needed, .data.ptr will point to one of
// these static vars below.
// If per-event data is needed, container_of can be used on data.ptr
// to access the containing struct and all its other members.

internal Epoll_Kind epoll_tag_frame_timer = EPOLL_FRAME_TIMER;
internal Epoll_Kind epoll_tag_x11 = EPOLL_X11;
internal Epoll_Kind epoll_tag_x11_internal = EPOLL_X11_INTERNAL;
internal Epoll_Kind epoll_tag_cli_pipe = EPOLL_CLI_PIPE;

////////////////////////////

typedef i32 Linux_Object_Kind;
enum {
    LinuxObjectKind_ERROR = 0,
    LinuxObjectKind_Timer = 1,
    LinuxObjectKind_Thread = 2,
    LinuxObjectKind_Mutex = 3,
    LinuxObjectKind_ConditionVariable = 4,
};

struct Linux_Object {
    Linux_Object_Kind kind;
    Node node;
    union {
        struct {
            int fd;
            Epoll_Kind epoll_tag;
        } timer;
        struct {
            pthread_t pthread;
            Thread_Function* proc;
            void* ptr;
        } thread;
        pthread_mutex_t mutex;
        pthread_cond_t condition_variable;
    };
};

Linux_Object*
handle_to_object(Plat_Handle ph){
    return *(Linux_Object**)&ph;
}

internal Linux_Object*
linux_alloc_object(Linux_Object_Kind kind){
    Linux_Object* result = NULL;
    if (linuxvars.free_linux_objects.next != &linuxvars.free_linux_objects) {
        result = CastFromMember(Linux_Object, node, linuxvars.free_linux_objects.next);
    }
    if (result == NULL) {
        i32 count = 512;
        Linux_Object* objects = (Linux_Object*)system_memory_allocate(
            sizeof(Linux_Object), file_name_line_number_lit_u8);
        objects[0].node.prev = &linuxvars.free_linux_objects;
        for (i32 i = 1; i < count; ++i) {
            objects[i - 1].node.next = &objects[i].node;
            objects[i].node.prev = &objects[i - 1].node;
        }
        objects[count - 1].node.next = &linuxvars.free_linux_objects;
        linuxvars.free_linux_objects.prev = &objects[count - 1].node;
        result = CastFromMember(Linux_Object, node, linuxvars.free_linux_objects.next);
    }
    Assert(result != 0);
    dll_remove(&result->node);
    block_zero_struct(result);
    result->kind = kind;
    return result;
}

internal void
linux_free_object(Linux_Object *object){
    if (object->node.next != 0){
        dll_remove(&object->node);
    }
    dll_insert(&linuxvars.free_linux_objects, &object->node);
}

////////////////////////////

internal int
linux_system_get_file_list_filter(const struct dirent *dirent) {
    String_Const_u8 file_name = SCu8((u8*)dirent->d_name);
    if (string_match(file_name, string_u8_litexpr("."))) {
        return 0;
    }
    else if (string_match(file_name, string_u8_litexpr(".."))) {
        return 0;
    }
    return 1;
}

internal int
linux_u64_from_timespec(const struct timespec timespec) {
    return timespec.tv_nsec + 1000000000 * timespec.tv_sec;
}

internal File_Attribute_Flag
linux_convert_file_attribute_flags(int mode) {
    File_Attribute_Flag result = {};
    MovFlag(mode, S_IFDIR, result, FileAttribute_IsDirectory);
    return result;
}

internal File_Attributes
linux_file_attributes_from_struct_stat(struct stat file_stat) {
    File_Attributes result = {};
    result.size = file_stat.st_size;
    result.last_write_time = linux_u64_from_timespec(file_stat.st_mtim);
    result.flags = linux_convert_file_attribute_flags(file_stat.st_mode);
    return result;
}

internal void
linux_schedule_step(){
    u64 now  = system_now_time();
    u64 diff = (now - linuxvars.last_step_time);

    struct itimerspec its = {};
    timerfd_gettime(linuxvars.step_timer_fd, &its);

    if (its.it_value.tv_sec == 0 && its.it_value.tv_nsec == 0){
        if(diff > (u64)frame_useconds) {
            its.it_value.tv_nsec = 1;
        } else {
            its.it_value.tv_nsec = (frame_useconds - diff) * 1000UL;
        }
        timerfd_settime(linuxvars.step_timer_fd, 0, &its, NULL);
    }
}

internal int
linux_get_xsettings_dpi(Display* dpy, int screen){
    struct XSettingHeader {
        u8 type;
        u8 pad0;
        u16 name_len;
        char name[0];
    };

    struct XSettings {
        u8 byte_order;
        u8 pad[3];
        u32 serial;
        u32 num_settings;
    };

    enum { XSettingsTypeInt, XSettingsTypeString, XSettingsTypeColor };

    int dpi = -1;
    unsigned char* prop = NULL;
    char sel_buffer[64];
    struct XSettings* xs;
    const char* p;

    snprintf(sel_buffer, sizeof(sel_buffer), "_XSETTINGS_S%d", screen);

    Atom XSET_SEL = XInternAtom(dpy, sel_buffer, True);
    Atom XSET_SET = XInternAtom(dpy, "_XSETTINGS_SETTINGS", True);

    if (XSET_SEL == None || XSET_SET == None){
        //LOG("XSETTINGS unavailable.\n");
        return(dpi);
    }

    Window xset_win = XGetSelectionOwner(dpy, XSET_SEL);
    if (xset_win == None){
        // TODO(inso): listen for the ClientMessage about it becoming available?
        //             there's not much point atm if DPI scaling is only done at startup 
        goto out;
    }

    {
        Atom type;
        int fmt;
        unsigned long pad, num;

        if (XGetWindowProperty(dpy, xset_win, XSET_SET, 0, 1024, False, XSET_SET, &type, &fmt, &num, &pad, &prop) != Success){
            //LOG("XSETTINGS: GetWindowProperty failed.\n");
            goto out;
        }

        if (fmt != 8){
            //LOG("XSETTINGS: Wrong format.\n");
            goto out;
        }
    }

    xs = (struct XSettings*)prop;
    p  = (char*)(xs + 1);

    if (xs->byte_order != 0){
        //LOG("FIXME: XSETTINGS not host byte order?\n");
        goto out;
    }

    for (int i = 0; i < xs->num_settings; ++i){
        struct XSettingHeader* h = (struct XSettingHeader*)p;

        p += sizeof(struct XSettingHeader);
        p += h->name_len;
        p += ((4 - (h->name_len & 3)) & 3);
        p += 4; // serial

        switch (h->type){
            case XSettingsTypeInt: {
                if (strncmp(h->name, "Xft/DPI", h->name_len) == 0){
                    dpi = *(i32*)p;
                    if (dpi != -1) dpi /= 1024;
                }
                p += 4;
            } break;

            case XSettingsTypeString: {
                u32 len = *(u32*)p;
                p += 4;
                p += len;
                p += ((4 - (len & 3)) & 3);
            } break;

            case XSettingsTypeColor: {
                p += 8;
            } break;

            default: {
                //LOG("XSETTINGS: Got invalid type...\n");
                goto out;
            } break;
        }
    }

    out:
        if (prop){
            XFree(prop);
        }

    return dpi;
}

internal void*
linux_thread_proc_start(void* arg) {
    Linux_Object* info = (Linux_Object*)arg;
    Assert(info->kind == LinuxObjectKind_Thread);
    info->thread.proc(info->thread.ptr);
    return NULL;
}

internal void
linux_window_maximize(b32 enable){

}

internal void
linux_window_fullscreen_toggle(){

}

#include "linux_icon.h"
internal void
linux_set_icon(Display* d, Window w){
    Atom WM_ICON = XInternAtom(d, "_NET_WM_ICON", False);
    XChangeProperty(d, w, WM_ICON, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)linux_icon, sizeof(linux_icon) / sizeof(long));
}

#include "linux_error_box.cpp"

////////////////////////////

#include "linux_4ed_functions.cpp"

////////////////////////////

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

////////////////////////////

internal
font_make_face_sig() {
    Face* result = ft__font_make_face(arena, description, scale_factor);
    return(result);
}

////////////////////////////

internal b32
glx_init(void) {
    return false;
}

internal b32
glx_get_config(GLXFBConfig* fb_config, XVisualInfo* vi) {
    return false;
}

internal b32
glx_create_context(GLXFBConfig* fb_config){
    return false;
}

////////////////////////////

internal void
linux_x11_init(int argc, char** argv, Plat_Settings* settings) {

    Display* dpy = XOpenDisplay(0);
    if (!dpy){
        fprintf(stderr, "FATAL: Cannot open X11 Display!\n");
        exit(1);
    }

    linuxvars.dpy = dpy;

#define LOAD_ATOM(x) linuxvars.atom_##x = XInternAtom(linuxvars.dpy, #x, False);

    LOAD_ATOM(TARGETS);
    LOAD_ATOM(CLIPBOARD);
    LOAD_ATOM(UTF8_STRING);
    LOAD_ATOM(_NET_WM_STATE);
    LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);
    LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
    LOAD_ATOM(_NET_WM_STATE_FULLSCREEN);
    LOAD_ATOM(_NET_WM_PING);
    LOAD_ATOM(_NET_WM_WINDOW_TYPE);
    LOAD_ATOM(_NET_WM_WINDOW_TYPE_NORMAL);
    LOAD_ATOM(_NET_WM_PID);
    LOAD_ATOM(WM_DELETE_WINDOW);

#undef LOAD_ATOM

    if (!glx_init()){
        //system_error_box("Your XServer's GLX version is too old. GLX 1.3+ is required.");
        system_error_box("TODO: Everything.");
    }

    GLXFBConfig fb_config;
    XVisualInfo vi;
    if (!glx_get_config(&fb_config, &vi)){
        system_error_box("Could not get a matching GLX FBConfig. Check your OpenGL drivers are installed correctly.");
    }

    // TODO: window size
#define WINDOW_W_DEFAULT 800
#define WINDOW_H_DEFAULT 600
    int w = WINDOW_W_DEFAULT;
    int h = WINDOW_H_DEFAULT;

    XSetWindowAttributes swa = {};
    swa.backing_store = WhenMapped;
    swa.event_mask = StructureNotifyMask;
    swa.bit_gravity = NorthWestGravity;
    swa.colormap = XCreateColormap(dpy, RootWindow(dpy, vi.screen), vi.visual, AllocNone);

    u32 CWflags = CWBackingStore|CWBitGravity|CWBackPixel|CWBorderPixel|CWColormap|CWEventMask;
    linuxvars.win = XCreateWindow(dpy, RootWindow(dpy, vi.screen), 0, 0, w, h, 0, vi.depth, InputOutput, vi.visual, CWflags, &swa);

    if (!linuxvars.win){
        system_error_box("XCreateWindow failed. Make sure your display is set up correctly.");
    }

    //NOTE(inso): Set the window's type to normal
    XChangeProperty(linuxvars.dpy, linuxvars.win, linuxvars.atom__NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char*)&linuxvars.atom__NET_WM_WINDOW_TYPE_NORMAL, 1);

    //NOTE(inso): window managers want the PID as a window property for some reason.
    pid_t pid = getpid();
    XChangeProperty(linuxvars.dpy, linuxvars.win, linuxvars.atom__NET_WM_PID, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&pid, 1);

    //NOTE(inso): set wm properties
    XStoreName(linuxvars.dpy, linuxvars.win, WINDOW_NAME);

    XSizeHints *sz_hints = XAllocSizeHints();
    XWMHints   *wm_hints = XAllocWMHints();
    XClassHint *cl_hints = XAllocClassHint();

    sz_hints->flags = PMinSize | PMaxSize | PWinGravity;

    sz_hints->min_width = 50;
    sz_hints->min_height = 50;

    sz_hints->max_width = sz_hints->max_height = (1UL << 16UL);
    sz_hints->win_gravity = NorthWestGravity;

    if (settings->set_window_pos){
        sz_hints->flags |= USPosition;
        sz_hints->x = settings->window_x;
        sz_hints->y = settings->window_y;
    }

    wm_hints->flags |= InputHint | StateHint;
    wm_hints->input = True;
    wm_hints->initial_state = NormalState;

    cl_hints->res_name = "4coder";
    cl_hints->res_class = "4coder";

    char* win_name_list[] = { WINDOW_NAME };
    XTextProperty win_name;
    XStringListToTextProperty(win_name_list, 1, &win_name);

    XSetWMProperties(linuxvars.dpy, linuxvars.win, &win_name, NULL, argv, argc, sz_hints, wm_hints, cl_hints);

    XFree(win_name.value);
    XFree(sz_hints);
    XFree(wm_hints);
    XFree(cl_hints);

    linux_set_icon(linuxvars.dpy, linuxvars.win);

    // NOTE(inso): make the window visible
    XMapWindow(linuxvars.dpy, linuxvars.win);

    if(!glx_create_context(&fb_config)) {
        system_error_box("Unable to create GLX context.");
    }

    XRaiseWindow(linuxvars.dpy, linuxvars.win);

    if (settings->set_window_pos){
        XMoveWindow(linuxvars.dpy, linuxvars.win, settings->window_x, settings->window_y);
    }

    if (settings->maximize_window){
        linux_window_maximize(true);
    } else if (settings->fullscreen_window){
        linux_window_fullscreen_toggle();
    }

    XSync(linuxvars.dpy, False);

    XWindowAttributes attr;
    if (XGetWindowAttributes(linuxvars.dpy, linuxvars.win, &attr)){
        //*window_width = WinAttribs.width;
        //*window_height = WinAttribs.height;
    }

    Atom wm_protos[] = {
        linuxvars.atom_WM_DELETE_WINDOW,
        linuxvars.atom__NET_WM_PING
    };

    XSetWMProtocols(linuxvars.dpy, linuxvars.win, wm_protos, 2);

    // XFixes extension for clipboard notification.
    {
        int xfixes_version_unused, xfixes_err_unused;
        Bool has_xfixes = XQueryExtension(linuxvars.dpy, "XFIXES", &xfixes_version_unused, &linuxvars.xfixes_selection_event, &xfixes_err_unused);
        linuxvars.has_xfixes = (has_xfixes == True);
    }

    // Input handling init

    setlocale(LC_ALL, "");
    XSetLocaleModifiers("");
    b32 locale_supported = XSupportsLocale();

    //LOGF("Supported locale?: %s.\n", locale_supported ? "Yes" : "No");
    if (!locale_supported){
        //LOG("Reverting to 'C' ... ");
        setlocale(LC_ALL, "C");
        locale_supported = XSupportsLocale();
        //LOGF("C is supported? %s.\n", locale_supported ? "Yes" : "No");
    }

    linuxvars.xim = XOpenIM(dpy, 0, 0, 0);
    if (!linuxvars.xim){
        // NOTE(inso): Try falling back to the internal XIM implementation that
        // should in theory always exist.
        XSetLocaleModifiers("@im=none");
        linuxvars.xim = XOpenIM(dpy, 0, 0, 0);
    }

    // If it still isn't there we're screwed.
    if (!linuxvars.xim){
        system_error_box("Could not initialize X Input.");
    }

    XIMStyles *styles = NULL;
    XIMStyle got_style = None;

    if (!XGetIMValues(linuxvars.xim, XNQueryInputStyle, &styles, NULL) && styles){
        for (i32 i = 0; i < styles->count_styles; ++i){
            XIMStyle style = styles->supported_styles[i];
            if (style == (XIMPreeditNothing | XIMStatusNothing)){
                got_style = true;
                break;
            }
        }
    }

    if(got_style == None) {
        system_error_box("Could not find supported X Input style.");
    }

    XFree(styles);

    linuxvars.xic = XCreateIC(linuxvars.xim,
                              XNInputStyle, got_style,
                              XNClientWindow, linuxvars.win,
                              XNFocusWindow, linuxvars.win,
                              NULL);

    int xim_event_mask;
    if (XGetICValues(linuxvars.xic, XNFilterEvents, &xim_event_mask, NULL)){
        xim_event_mask = 0;
    }

    u32 event_mask = ExposureMask
        | KeyPressMask | KeyReleaseMask
        | ButtonPressMask | ButtonReleaseMask
        | EnterWindowMask | LeaveWindowMask
        | PointerMotionMask
        | FocusChangeMask
        | StructureNotifyMask | MappingNotify
        | ExposureMask | VisibilityChangeMask
        | xim_event_mask;

    XSelectInput(linuxvars.dpy, linuxvars.win, event_mask);
}

internal void
linux_handle_x11_events() {
    static XEvent prev_event = {};
    b32 should_step = false;
    while (XPending(linuxvars.dpy)) {
        XEvent event;
        XNextEvent(linuxvars.dpy, &event);

        if (XFilterEvent(&event, None) == True){
            continue;
        }

        switch (event.type) {
        }
    }
}

internal b32
linux_epoll_process(struct epoll_event* events, int num_events) {
    b32 do_step = false;

    for (int i = 0; i < num_events; ++i){
        struct epoll_event* ev = events + i;
        Epoll_Kind* tag = (Epoll_Kind*)ev->data.ptr;

        switch (*tag){
            case EPOLL_X11: {
                linux_handle_x11_events();
            } break;

            case EPOLL_X11_INTERNAL: {
                //XProcessInternalConnection(linuxvars.dpy, fd);
            } break;

            case EPOLL_FRAME_TIMER: {
                u64 count;
                int ret;
                do {
                    ret = read(linuxvars.step_timer_fd, &count, 8);
                } while (ret != -1 || errno != EAGAIN);
                do_step = true;
            } break;

            case EPOLL_CLI_PIPE: {
                linux_schedule_step();
            } break;

            case EPOLL_USER_TIMER: {
                Linux_Object* obj = container_of(tag, Linux_Object, timer.epoll_tag);
                close(obj->timer.fd);
                obj->timer.fd = -1;
                do_step = true;
            } break;
        }
    }

    return do_step;
}

int
main(int argc, char **argv){

    // NOTE(allen): context setup
    {
        Base_Allocator* alloc = get_base_allocator_system();
        thread_ctx_init(&linuxvars.tctx, ThreadKind_Main, alloc, alloc);
    }

    API_VTable_system system_vtable = {};
    system_api_fill_vtable(&system_vtable);

    API_VTable_graphics graphics_vtable = {};
    graphics_api_fill_vtable(&graphics_vtable);

    API_VTable_font font_vtable = {};
    font_api_fill_vtable(&font_vtable);

    // NOTE(allen): memory
    //win32vars.frame_arena = reserve_arena(win32vars.tctx);
    // TODO(allen): *arena;
    //target.arena = make_arena_system(KB(256));

    linuxvars.cursor_show = MouseCursorShow_Always;
    linuxvars.prev_cursor_show = MouseCursorShow_Always;

    dll_init_sentinel(&linuxvars.free_linux_objects);
    dll_init_sentinel(&linuxvars.timer_objects);

    //InitializeCriticalSection(&win32vars.thread_launch_mutex);
    //InitializeConditionVariable(&win32vars.thread_launch_cv);

    // dpi ?

    // NOTE(allen): load core
    System_Library core_library = {};
    App_Functions app = {};
    {
        App_Get_Functions *get_funcs = 0;
        Scratch_Block scratch(&linuxvars.tctx, Scratch_Share);
        Path_Search_List search_list = {};
        search_list_add_system_path(scratch, &search_list, SystemPath_Binary);

        String_Const_u8 core_path = get_full_path(scratch, &search_list, SCu8("4ed_app.so"));
        if (system_load_library(scratch, core_path, &core_library)){
            get_funcs = (App_Get_Functions*)system_get_proc(core_library, "app_get_functions");
            if (get_funcs != 0){
                app = get_funcs();
            }
            else{
                char msg[] = "Failed to get application code from '4ed_app.so'.";
                system_error_box(msg);
            }
        }
        else{
            char msg[] = "Could not load '4ed_app.so'. This file should be in the same directory as the main '4ed' executable.";
            system_error_box(msg);
        }
    }

    // NOTE(allen): send system vtable to core
    app.load_vtables(&system_vtable, &font_vtable, &graphics_vtable);
    //linuxvars.log_string = app.get_logger();

    // NOTE(allen): init & command line parameters
    Plat_Settings plat_settings = {};
    void *base_ptr = 0;
    {
        Scratch_Block scratch(&linuxvars.tctx, Scratch_Share);
        String_Const_u8 curdir = system_get_path(scratch, SystemPath_CurrentDirectory);

        char **files = 0;
        i32 *file_count = 0;
        base_ptr = app.read_command_line(&linuxvars.tctx, curdir, &plat_settings, &files, &file_count, argc, argv);
        /* TODO(inso): what is this doing?
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
        }*/
    }

    // NOTE(allen): load custom layer
    System_Library custom_library = {};
    Custom_API custom = {};
    {
        char custom_not_found_msg[] = "Did not find a library for the custom layer.";
        char custom_fail_version_msg[] = "Failed to load custom code due to missing version information or a version mismatch.  Try rebuilding with buildsuper.";
        char custom_fail_init_apis[] = "Failed to load custom code due to missing 'init_apis' symbol.  Try rebuilding with buildsuper";

        Scratch_Block scratch(&linuxvars.tctx, Scratch_Share);
        String_Const_u8 default_file_name = string_u8_litexpr("custom_4coder.so");
        Path_Search_List search_list = {};
        search_list_add_system_path(scratch, &search_list, SystemPath_CurrentDirectory);
        search_list_add_system_path(scratch, &search_list, SystemPath_Binary);
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
        String_Const_u8 custom_file_name = {};
        for (i32 i = 0; i < custom_file_count; i += 1){
            custom_file_name = get_full_path(scratch, &search_list, custom_file_names[i]);
            if (custom_file_name.size > 0){
                break;
            }
        }
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

    linux_x11_init(argc, argv, &plat_settings);

    // TODO(inso): move to x11 init?
    XCursor xcursors[APP_MOUSE_CURSOR_COUNT] = {
        None,
        None,
        XCreateFontCursor(linuxvars.dpy, XC_xterm),
        XCreateFontCursor(linuxvars.dpy, XC_sb_h_double_arrow),
        XCreateFontCursor(linuxvars.dpy, XC_sb_v_double_arrow)
    };

    // sneaky invisible cursor
    {
        char data = 0;
        XColor c  = {};
        Pixmap p  = XCreateBitmapFromData(linuxvars.dpy, linuxvars.win, &data, 1, 1);

        linuxvars.hidden_cursor = XCreatePixmapCursor(linuxvars.dpy, p, p, &c, &c, 0, 0);

        XFreePixmap(linuxvars.dpy, p);
    }

    // epoll init
    {
        struct epoll_event e = {};
        e.events = EPOLLIN | EPOLLET;

        //linuxvars.inotify_fd    = inotify_init1(IN_NONBLOCK);
        linuxvars.step_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        linuxvars.epoll         = epoll_create(16);

        e.data.ptr = &epoll_tag_x11;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, ConnectionNumber(linuxvars.dpy), &e);

        e.data.ptr = &epoll_tag_frame_timer;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, linuxvars.step_timer_fd, &e);
    }

    // app init
    {
        Scratch_Block scratch(&linuxvars.tctx, Scratch_Share);
        String_Const_u8 curdir = system_get_path(scratch, SystemPath_CurrentDirectory);
        //app.init(&linuxvars.tctx, &render_target, base_ptr, linuxvars.clipboard_contents, curdir, custom);
    }

    linuxvars.global_frame_mutex = system_mutex_make();
    system_mutex_acquire(linuxvars.global_frame_mutex);

    linux_schedule_step();

    b32 keep_running = true;
    for (;keep_running;){

        if (XEventsQueued(linuxvars.dpy, QueuedAlready)){
            linux_handle_x11_events();
        }

        system_mutex_release(linuxvars.global_frame_mutex);

        struct epoll_event events[16];
        int num_events = epoll_wait(linuxvars.epoll, events, ArrayCount(events), -1);

        system_mutex_acquire(linuxvars.global_frame_mutex);

        if (num_events == -1){
            if (errno != EINTR){
                perror("epoll_wait");
                //LOG("epoll_wait\n");
            }
            continue;
        }

        if(!linux_epoll_process(events, num_events)) {
            continue;
        }

        b32 first_step = true;
        linuxvars.last_step_time = system_now_time();

        // NOTE(allen): Frame Clipboard Input
        // Request clipboard contents from X11 on first step, or every step if they don't have XFixes notification ability.
        if (first_step || !linuxvars.has_xfixes){
            XConvertSelection(linuxvars.dpy, linuxvars.atom_CLIPBOARD, linuxvars.atom_UTF8_STRING, linuxvars.atom_CLIPBOARD, linuxvars.win, CurrentTime);
        }

        if (linuxvars.received_new_clipboard){
            linuxvars.input.clipboard = linuxvars.clipboard_contents;
            linuxvars.received_new_clipboard = false;
        } else {
            linuxvars.input.clipboard = {};
        }

        Application_Step_Input frame_input = linuxvars.input;
        // TODO:
        frame_input.trying_to_kill = !keep_running;

        // NOTE(allen): Application Core Update
        // render_target.buffer.pos = 0;
        Application_Step_Result result = {};
        if (app.step != 0){
            result = app.step(&linuxvars.tctx, &render_target, base_ptr, &frame_input);
        }
        else{
            //LOG("app.step == 0 -- skipping\n");
        }

        // NOTE(allen): Finish the Loop
        if (result.perform_kill){
            break;
        }
        // TODO
#if 0
        else if (!keep_running && !linuxvars.keep_running){
            linuxvars.keep_running = true;
        }
#endif

        // NOTE(NAME): Switch to New Title
        if (result.has_new_title){
            XStoreName(linuxvars.dpy, linuxvars.win, result.title_string);
        }

        // NOTE(allen): Switch to New Cursor
        if (result.mouse_cursor_type != linuxvars.cursor && !linuxvars.input.mouse.l){
            XCursor c = xcursors[result.mouse_cursor_type];
            if (linuxvars.cursor_show){
                XDefineCursor(linuxvars.dpy, linuxvars.win, c);
            }
            linuxvars.cursor = result.mouse_cursor_type;
        }

        first_step = false;
    }

    return 0;
}
