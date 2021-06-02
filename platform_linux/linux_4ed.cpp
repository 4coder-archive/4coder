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

#include <stdio.h>

#define FPS 60
#define frame_useconds (Million(1) / FPS)
#define frame_nseconds (Billion(1) / FPS)
#define SLASH '/'
#define DLL "so"

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
#include "4coder_search_list.h"
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
#include "4coder_search_list.cpp"
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
#include <sys/eventfd.h>
#include <sys/syscall.h>

#define Cursor XCursor
#undef function
#undef internal
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xfixes.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#define function static
#undef Cursor

//#include <fontconfig/fontconfig.h>
#define internal static

#include <GL/glx.h>
#include <GL/glext.h>

#ifdef INSO_DEBUG
#define LINUX_FN_DEBUG(fmt, ...) do { \
fprintf(stderr, "%s: " fmt "\n", __func__, ##__VA_ARGS__);\
} while (0)

// I want to see a message
#undef AssertBreak
#define AssertBreak(m) ({\
fprintf(stderr, "\n** ASSERTION FAILURE: %s:%d: %s\n\n", __FILE__, __LINE__, #m);\
*((volatile u64*)0) = 0xba771e70ad5;\
})
#else
#define LINUX_FN_DEBUG(...)
#endif

////////////////////////////////

global b32 log_os_enabled = false;
#define log_os(...) \
Stmnt( if (log_os_enabled){ fprintf(stdout, __VA_ARGS__); fflush(stdout); } )

////////////////////////////

struct Linux_Input_Chunk_Transient {
    Input_List event_list;
    b8 mouse_l_press;
    b8 mouse_l_release;
    b8 mouse_r_press;
    b8 mouse_r_release;
    i8 mouse_wheel;
    b8 trying_to_kill;
};

struct Linux_Input_Chunk_Persistent {
    Vec2_i32 mouse;
    Input_Modifier_Set_Fixed modifiers;
    b8 mouse_l;
    b8 mouse_r;
    b8 mouse_out_of_window;
};

struct Linux_Input_Chunk {
    Linux_Input_Chunk_Transient trans;
    Linux_Input_Chunk_Persistent pers;
};

struct Linux_Memory_Tracker_Node {
    Linux_Memory_Tracker_Node* prev;
    Linux_Memory_Tracker_Node* next;
    String_Const_u8 location;
    u64 size;
};

struct Linux_Vars {
    Thread_Context tctx;
    Arena frame_arena;
    
    Display* dpy;
    Window win;
    
    b32 has_xfixes;
    int xfixes_selection_event;
    XIM xim;
    XIC xic;
    //FcConfig* fontconfig;
    XkbDescPtr xkb;
    
    Linux_Input_Chunk input;
    int xkb_event;
    int xkb_group; // active keyboard layout (0-3)
    KeyCode prev_filtered_key;
    
    Key_Mode key_mode;
    
    int epoll;
    int step_timer_fd;
    u64 last_step_time;
    
    XCursor xcursors[APP_MOUSE_CURSOR_COUNT];
    Application_Mouse_Cursor cursor;
    XCursor hidden_cursor;
    i32 cursor_show;
    i32 prev_cursor_show;
    
    Node free_linux_objects;
    Node timer_objects;
    
    System_Mutex global_frame_mutex;
    pthread_mutex_t memory_tracker_mutex;
    Linux_Memory_Tracker_Node* memory_tracker_head;
    Linux_Memory_Tracker_Node* memory_tracker_tail;
    int memory_tracker_count;
    
    Arena clipboard_arena;
    String_Const_u8 clipboard_contents;
    b32 received_new_clipboard;
    b32 clipboard_catch_all;
    
    pthread_mutex_t audio_mutex;
    pthread_cond_t audio_cond;
    void* audio_ctx;
    Audio_Mix_Sources_Function* audio_src_func;
    Audio_Mix_Destination_Function* audio_dst_func;
    System_Thread audio_thread;
    
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
    
    Log_Function *log_string;
};

global Linux_Vars linuxvars;
global Render_Target render_target;

////////////////////////////

// Defererencing an epoll_event's .data.ptr will always give one of these event types.

typedef i32 Epoll_Kind;
enum {
    EPOLL_STEP_TIMER,
    EPOLL_X11,
    EPOLL_X11_INTERNAL,
    EPOLL_CLI_PIPE,
    EPOLL_USER_TIMER,
};

// Where per-event epoll data is not needed, .data.ptr will point to one of
// these static vars below.
// If per-event data is needed, container_of can be used on data.ptr
// to access the containing struct and all its other members.

internal Epoll_Kind epoll_tag_step_timer = EPOLL_STEP_TIMER;
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

Plat_Handle
object_to_handle(Linux_Object* obj) {
    return *(Plat_Handle*)&obj;
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
                                                                      sizeof(Linux_Object) * count,
                                                                      file_name_line_number_lit_u8
                                                                      );
        
        objects[0].node.prev = &linuxvars.free_linux_objects;
        linuxvars.free_linux_objects.next = &objects[0].node;
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
linux_compare_file_infos(File_Info** a, File_Info** b) {
    b32 a_hidden = (*a)->file_name.str[0] == '.';
    b32 b_hidden = (*b)->file_name.str[0] == '.';
    
    // hidden files lower in list
    if(a_hidden != b_hidden) {
        return a_hidden - b_hidden;
    }
    
    // push_stringf seems to null terminate
    return strcoll((char*)(*a)->file_name.str, (char*)(*b)->file_name.str);
}

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

internal u64
linux_us_from_timespec(const struct timespec timespec) {
    return timespec.tv_nsec/Thousand(1) + Million(1) * timespec.tv_sec;
}

internal File_Attribute_Flag
linux_convert_file_attribute_flags(int mode) {
    File_Attribute_Flag result = {};
    MovFlag(mode, S_IFDIR, result, FileAttribute_IsDirectory);
    return result;
}

internal File_Attributes
linux_file_attributes_from_struct_stat(struct stat* file_stat) {
    File_Attributes result = {};
    result.size = file_stat->st_size;
    result.last_write_time = linux_us_from_timespec(file_stat->st_mtim);
    result.flags = linux_convert_file_attribute_flags(file_stat->st_mode);
    return(result);
}

internal void
linux_schedule_step(){
    u64 now  = system_now_time();
    u64 diff = (now - linuxvars.last_step_time);
    
    struct itimerspec its = {};
    timerfd_gettime(linuxvars.step_timer_fd, &its);
    
    if (diff > frame_useconds) {
        its.it_value.tv_nsec = 1;
        timerfd_settime(linuxvars.step_timer_fd, 0, &its, NULL);
    } else {
        if (its.it_value.tv_sec == 0 && its.it_value.tv_nsec == 0){
            its.it_value.tv_nsec = (frame_useconds - diff) * 1000UL;
            timerfd_settime(linuxvars.step_timer_fd, 0, &its, NULL);
        }
    }
}

enum wm_state_mode {
    WM_STATE_DEL = 0,
    WM_STATE_ADD = 1,
    WM_STATE_TOGGLE = 2,
};

internal void
linux_set_wm_state(Atom one, Atom two, enum wm_state_mode mode){
    //NOTE(inso): this will only work after the window has been mapped
    
    XEvent e = {};
    e.xany.type = ClientMessage;
    e.xclient.message_type = linuxvars.atom__NET_WM_STATE;
    e.xclient.format = 32;
    e.xclient.window = linuxvars.win;
    e.xclient.data.l[0] = mode;
    e.xclient.data.l[1] = one;
    e.xclient.data.l[2] = two;
    e.xclient.data.l[3] = 1L;
    
    XSendEvent(linuxvars.dpy,
               RootWindow(linuxvars.dpy, 0),
               0, SubstructureNotifyMask | SubstructureRedirectMask, &e);
}

internal void
linux_window_maximize(enum wm_state_mode mode){
    linux_set_wm_state(linuxvars.atom__NET_WM_STATE_MAXIMIZED_HORZ, linuxvars.atom__NET_WM_STATE_MAXIMIZED_VERT, mode);
}

internal void
linux_window_fullscreen(enum wm_state_mode mode) {
    linux_set_wm_state(linuxvars.atom__NET_WM_STATE_FULLSCREEN, 0, mode);
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

#include "linux_icon.h"
internal void
linux_set_icon(Display* d, Window w){
    Atom WM_ICON = XInternAtom(d, "_NET_WM_ICON", False);
    XChangeProperty(d, w, WM_ICON, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)linux_icon, sizeof(linux_icon) / sizeof(long));
}

#include "linux_error_box.cpp"

function void
os_popup_error(char *title, char *message){
    system_error_box(message);
    exit(1);
}

////////////////////////////

#include "linux_4ed_functions.cpp"
#include "linux_4ed_audio.cpp"

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

internal Face*
font_make_face(Arena* arena, Face_Description* description, f32 scale_factor) {
    
    Face_Description local_description = *description;
    String_Const_u8* name = &local_description.font.file_name;
    
    // if description->font.file_name is a relative path, prepend the font directory.
    if(string_get_character(*name, 0) != '/') {
        String_Const_u8 binary = system_get_path(arena, SystemPath_Binary);
        *name = push_u8_stringf(arena, "%.*sfonts/%.*s", string_expand(binary), string_expand(*name));
    }
    
    Face* result = ft__font_make_face(arena, &local_description, scale_factor);
    
    if(!result) {
        // is this fatal? 4ed.cpp:277 (caller) does not check for null.
        char msg[4096];
        snprintf(msg, sizeof(msg), "Unable to load font: %.*s", string_expand(*name));
        system_error_box(msg);
    }
    
    return(result);
}

////////////////////////////

internal b32
glx_init(void) {
    int glx_maj, glx_min;
    
    if(!glXQueryVersion(linuxvars.dpy, &glx_maj, &glx_min)) {
        return false;
    }
    
    return glx_maj > 1 || (glx_maj == 1 && glx_min >= 3);
}

internal b32
glx_get_config(GLXFBConfig* fb_config, XVisualInfo* vi) {
    
    static const int attrs[] = {
        GLX_X_RENDERABLE , True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE  , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE     , 8,
        GLX_GREEN_SIZE   , 8,
        GLX_BLUE_SIZE    , 8,
        GLX_ALPHA_SIZE   , 8,
        GLX_DEPTH_SIZE   , 24,
        GLX_STENCIL_SIZE , 8,
        GLX_DOUBLEBUFFER , True,
        None
    };
    
    int conf_count = 0;
    GLXFBConfig* conf_list = glXChooseFBConfig(linuxvars.dpy, DefaultScreen(linuxvars.dpy), attrs, &conf_count);
    if(!conf_count || conf_count <= 0) {
        return false;
    }
    
    *fb_config = *conf_list;
    XFree(conf_list);
    
    XVisualInfo* xvi = glXGetVisualFromFBConfig(linuxvars.dpy, *fb_config);
    if(!xvi) {
        return false;
    }
    
    *vi = *xvi;
    XFree(xvi);
    
    return true;
}

internal b32 glx_ctx_error;

internal int
glx_error_handler(Display* dpy, XErrorEvent* ev){
    glx_ctx_error = true;
    return 0;
}

typedef GLXContext (glXCreateContextAttribsARB_Function)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef void       (glXSwapIntervalEXT_Function)        (Display *dpy, GLXDrawable drawable, int interval);
typedef int        (glXSwapIntervalMESA_Function)       (unsigned int interval);
typedef int        (glXGetSwapIntervalMESA_Function)    (void);
typedef int        (glXSwapIntervalSGI_Function)        (int interval);

internal b32
glx_create_context(GLXFBConfig fb_config){
    const char *glx_exts = glXQueryExtensionsString(linuxvars.dpy, DefaultScreen(linuxvars.dpy));
    
    glXCreateContextAttribsARB_Function *glXCreateContextAttribsARB = 0;
    glXSwapIntervalEXT_Function         *glXSwapIntervalEXT = 0;
    glXSwapIntervalMESA_Function        *glXSwapIntervalMESA = 0;
    glXGetSwapIntervalMESA_Function     *glXGetSwapIntervalMESA = 0;
    glXSwapIntervalSGI_Function         *glXSwapIntervalSGI = 0;
    
#define GLXLOAD(f) f = (f##_Function*) glXGetProcAddressARB((const GLubyte*) #f);
    GLXLOAD(glXCreateContextAttribsARB);
    
    GLXContext ctx = NULL;
    int (*old_handler)(Display*, XErrorEvent*) = XSetErrorHandler(&glx_error_handler);
    
    if (glXCreateContextAttribsARB == NULL){
        //LOG("glXCreateContextAttribsARB() not found, using old-style GLX context\n" );
        ctx = glXCreateNewContext(linuxvars.dpy, fb_config, GLX_RGBA_TYPE, 0, True);
    } else {
        static const int context_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
            GLX_CONTEXT_MINOR_VERSION_ARB, 1,
            GLX_CONTEXT_PROFILE_MASK_ARB , GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#if GL_DEBUG_MODE
            GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
            None
        };
        
        //LOG("Creating GL 2.1 context... ");
        ctx = glXCreateContextAttribsARB(linuxvars.dpy, fb_config, 0, True, context_attribs);
    }
    
    XSync(linuxvars.dpy, False);
    if(glx_ctx_error || !ctx) {
        return false;
    }
    
    XSync(linuxvars.dpy, False);
    XSetErrorHandler(old_handler);
    
    //b32 direct = glXIsDirect(linuxvars.dpy, ctx);
    
    //LOG("Making context current\n");
    glXMakeCurrent(linuxvars.dpy, linuxvars.win, ctx);
    
    //glx_enable_vsync();
    
    // NOTE(allen): Load gl functions
#define GL_FUNC(f,R,P) GLXLOAD(f)
#include "opengl/4ed_opengl_funcs.h"
    
#undef GLXLOAD
    
    return true;
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
        system_error_box("Your XServer's GLX version is too old. GLX 1.3+ is required.");
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
    
    // TEMP
    render_target.width = w;
    render_target.height = h;
    
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
    
    if(!glx_create_context(fb_config)) {
        system_error_box("Unable to create GLX context.");
    }
    
    XRaiseWindow(linuxvars.dpy, linuxvars.win);
    
    if (settings->set_window_pos){
        XMoveWindow(linuxvars.dpy, linuxvars.win, settings->window_x, settings->window_y);
    }
    
    if (settings->maximize_window){
        linux_set_wm_state(linuxvars.atom__NET_WM_STATE_MAXIMIZED_HORZ, linuxvars.atom__NET_WM_STATE_MAXIMIZED_VERT, WM_STATE_ADD);
    } else if (settings->fullscreen_window){
        linux_set_wm_state(linuxvars.atom__NET_WM_STATE_FULLSCREEN, 0, WM_STATE_ADD);
    }
    
    XSync(linuxvars.dpy, False);
    
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
        
        // request notifications for CLIPBOARD updates.
        if(has_xfixes) {
            XFixesSelectSelectionInput(linuxvars.dpy, linuxvars.win, linuxvars.atom_CLIPBOARD, XFixesSetSelectionOwnerNotifyMask);
        }
    }
    
    // Input handling init
    
    setlocale(LC_ALL, "");
    XSetLocaleModifiers("");
    b32 locale_supported = XSupportsLocale();
    
    if (!locale_supported){
        setlocale(LC_ALL, "C");
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
    const XIMStyle style_want = (XIMPreeditNothing | XIMStatusNothing);
    b32 found_style = false;
    
    if (!XGetIMValues(linuxvars.xim, XNQueryInputStyle, &styles, NULL) && styles){
        for (i32 i = 0; i < styles->count_styles; ++i){
            XIMStyle style = styles->supported_styles[i];
            if (style == style_want) {
                found_style = true;
                break;
            }
        }
    }
    
    if(!found_style) {
        system_error_box("Could not find supported X Input style.");
    }
    
    XFree(styles);
    
    linuxvars.xic = XCreateIC(linuxvars.xim,
                              XNInputStyle, style_want,
                              XNClientWindow, linuxvars.win,
                              XNFocusWindow, linuxvars.win,
                              NULL);
    
    if(!linuxvars.xic) {
        system_error_box("Error creating X Input context.");
    }
    
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
        | StructureNotifyMask
        | ExposureMask | VisibilityChangeMask
        | xim_event_mask;
    
    XSelectInput(linuxvars.dpy, linuxvars.win, event_mask);
    
    // init XKB keyboard extension
    
    if(!XkbQueryExtension(linuxvars.dpy, 0, &linuxvars.xkb_event, 0, 0, 0)) {
        system_error_box("XKB Extension not available.");
    }
    
    XkbSelectEvents(linuxvars.dpy, XkbUseCoreKbd, XkbAllEventsMask, XkbAllEventsMask);
    linuxvars.xkb = XkbGetMap(linuxvars.dpy, XkbKeyTypesMask | XkbKeySymsMask, XkbUseCoreKbd);
    if(!linuxvars.xkb) {
        system_error_box("Error getting XKB keyboard map.");
    }
    
    if(XkbGetNames(linuxvars.dpy, XkbKeyNamesMask, linuxvars.xkb) != Success) {
        system_error_box("Error getting XKB key names.");
    }
    
    // closer to windows behaviour (holding key doesn't generate release events)
    XkbSetDetectableAutoRepeat(linuxvars.dpy, True, NULL);
    
    XCursor cursors[APP_MOUSE_CURSOR_COUNT] = {
        None,
        None,
        XCreateFontCursor(linuxvars.dpy, XC_xterm),
        XCreateFontCursor(linuxvars.dpy, XC_sb_h_double_arrow),
        XCreateFontCursor(linuxvars.dpy, XC_sb_v_double_arrow)
    };
    block_copy(linuxvars.xcursors, cursors, sizeof(cursors));
    
    // sneaky invisible cursor
    {
        char data = 0;
        XColor c  = {};
        Pixmap p  = XCreateBitmapFromData(linuxvars.dpy, linuxvars.win, &data, 1, 1);
        
        linuxvars.hidden_cursor = XCreatePixmapCursor(linuxvars.dpy, p, p, &c, &c, 0, 0);
        
        XFreePixmap(linuxvars.dpy, p);
    }
}

global Key_Code keycode_lookup_table_physical[255];
global Key_Code keycode_lookup_table_language[255];

struct SymCode {
    KeySym sym;
    Key_Code code;
};

internal void
linux_keycode_init_common(Display* dpy, Key_Code* keycode_lookup_table, SymCode* sym_table, SymCode* p, size_t sym_table_size){
    
    *p++ = { XK_space, KeyCode_Space };
    *p++ = { XK_Tab, KeyCode_Tab };
    *p++ = { XK_Escape, KeyCode_Escape };
    *p++ = { XK_Pause, KeyCode_Pause };
    *p++ = { XK_Up, KeyCode_Up };
    *p++ = { XK_Down, KeyCode_Down };
    *p++ = { XK_Left, KeyCode_Left };
    *p++ = { XK_Right, KeyCode_Right };
    *p++ = { XK_BackSpace, KeyCode_Backspace };
    *p++ = { XK_Return, KeyCode_Return };
    *p++ = { XK_Delete, KeyCode_Delete };
    *p++ = { XK_Insert, KeyCode_Insert };
    *p++ = { XK_Home, KeyCode_Home };
    *p++ = { XK_End, KeyCode_End };
    *p++ = { XK_Page_Up, KeyCode_PageUp };
    *p++ = { XK_Page_Down, KeyCode_PageDown };
    *p++ = { XK_Caps_Lock, KeyCode_CapsLock };
    *p++ = { XK_Num_Lock, KeyCode_NumLock };
    *p++ = { XK_Scroll_Lock, KeyCode_ScrollLock };
    *p++ = { XK_Menu, KeyCode_Menu };
    *p++ = { XK_Shift_L, KeyCode_Shift };
    *p++ = { XK_Shift_R, KeyCode_Shift };
    *p++ = { XK_Control_L, KeyCode_Control };
    *p++ = { XK_Control_R, KeyCode_Control };
    *p++ = { XK_Alt_L, KeyCode_Alt };
    *p++ = { XK_Alt_R, KeyCode_Alt };
    *p++ = { XK_Super_L, KeyCode_Command };
    *p++ = { XK_Super_R, KeyCode_Command };
    
    for (Key_Code k = KeyCode_F1; k <= KeyCode_F24; ++k){
        *p++ = { XK_F1 + (k - KeyCode_F1), k };
    }
    
    for (Key_Code k = KeyCode_NumPad0; k <= KeyCode_NumPad9; ++k){
        *p++ = { XK_KP_0 + (k - KeyCode_NumPad0), k };
    }
    
    *p++ = { XK_KP_Multiply, KeyCode_NumPadStar };
    *p++ = { XK_KP_Add, KeyCode_NumPadPlus };
    *p++ = { XK_KP_Subtract, KeyCode_NumPadMinus };
    *p++ = { XK_KP_Decimal, KeyCode_NumPadDot };
    *p++ = { XK_KP_Delete, KeyCode_NumPadDot }; // seems to take precedence over Decimal...
    *p++ = { XK_KP_Divide, KeyCode_NumPadSlash };
    *p++ = { XK_KP_Enter, KeyCode_Return }; // NumPadEnter?
    
    const int table_size = p - sym_table;
    Assert(table_size < sym_table_size);
    
    Key_Code next_extra = KeyCode_Ex1;
    const Key_Code max_extra = KeyCode_Ex29;
    
    for(int i = XkbMinLegalKeyCode; i <= XkbMaxLegalKeyCode; ++i) {
        KeySym sym = NoSymbol;
        
        // lookup key in current layout with no modifiers held (0)
        if(!XkbTranslateKeyCode(linuxvars.xkb, i, XkbBuildCoreState(0, linuxvars.xkb_group), NULL, &sym)) {
            continue;
        }
        
        int j;
        for(j = 0; j < table_size; ++j) {
            if(sym_table[j].sym == sym) {
                keycode_lookup_table[i] = sym_table[j].code;
                //printf("lookup %s = %d\n", key_code_name[sym_table[j].code], i);
                break;
            }
        }
        
        if(j != table_size){
            continue;
        }
        
        // nothing found - try with shift held (needed for e.g. belgian numbers to bind).
        KeySym shift_sym = NoSymbol;
        
        if(!XkbTranslateKeyCode(linuxvars.xkb, i, XkbBuildCoreState(ShiftMask, linuxvars.xkb_group), NULL, &shift_sym)) {
            continue;
        }
        
        for(j = 0; j < table_size; ++j) {
            if(sym_table[j].sym == shift_sym) {
                keycode_lookup_table[i] = sym_table[j].code;
                //printf("lookup %s = %d\n", key_code_name[sym_table[j].code], i);
                break;
            }
        }
        
        // something unknown bound, put it in extra
        if(j == table_size && sym != NoSymbol && next_extra <= max_extra && keycode_lookup_table[i] == 0) {
            keycode_lookup_table[i] = next_extra++;
        }
    }
    
}

internal void
linux_keycode_init_language(Display* dpy, Key_Code* keycode_lookup_table){
    SymCode sym_table[300];
    SymCode* p = sym_table;
    
    for(unsigned int i = 0; i < 26; ++i) {
        *p++ = { XK_a + i, KeyCode_A + i};
    }
    
    for(unsigned int i = 0; i < 26; ++i) {
        *p++ = { XK_A + i, KeyCode_A + i};
    }
    
    for(unsigned int i = 0; i <= 9; ++i) {
        *p++ = { XK_0 + i, KeyCode_0 + i};
    }
    
    *p++ = { XK_grave, KeyCode_Tick };
    *p++ = { XK_minus, KeyCode_Minus };
    *p++ = { XK_equal, KeyCode_Equal };
    *p++ = { XK_bracketleft, KeyCode_LeftBracket };
    *p++ = { XK_bracketright, KeyCode_RightBracket };
    *p++ = { XK_semicolon, KeyCode_Semicolon };
    *p++ = { XK_apostrophe, KeyCode_Quote };
    *p++ = { XK_comma, KeyCode_Comma };
    *p++ = { XK_period, KeyCode_Period };
    *p++ = { XK_slash, KeyCode_ForwardSlash };
    *p++ = { XK_backslash, KeyCode_BackwardSlash };
    
    linux_keycode_init_common(dpy, keycode_lookup_table, sym_table, p, ArrayCount(sym_table));
}

internal void
linux_keycode_init_physical(Display* dpy, Key_Code* keycode_lookup_table){
    
    // Find common keys by their key label
    SymCode sym_table[100];
    linux_keycode_init_common(dpy, keycode_lookup_table, sym_table, sym_table, ArrayCount(sym_table));
    
    // Find these keys by physical position, and map to QWERTY KeyCodes
#define K(k) glue(KeyCode_, k)
    static const u8 positional_keys[] = {
        K(1), K(2), K(3), K(4), K(5), K(6), K(7), K(8), K(9), K(0), K(Minus), K(Equal),
        K(Q), K(W), K(E), K(R), K(T), K(Y), K(U), K(I), K(O), K(P), K(LeftBracket), K(RightBracket),
        K(A), K(S), K(D), K(F), K(G), K(H), K(J), K(K), K(L), K(Semicolon), K(Quote), /*uk hash*/0,
        K(Z), K(X), K(C), K(V), K(B), K(N), K(M), K(Comma), K(Period), K(ForwardSlash), 0, 0
    };
#undef K
    
    // XKB gives the alphanumeric keys names like AE01 -> E is the row (from B-E), 01 is the column (01-12).
    // to get key names in .ps file: setxkbmap -print | xkbcomp - - | xkbprint -label name - out.ps
    
    static const int ncols = 12;
    static const int nrows = 4;
    
    for(int i = XkbMinLegalKeyCode; i <= XkbMaxLegalKeyCode; ++i) {
        const char* name = linuxvars.xkb->names->keys[i].name;
        
        // alphanumeric keys
        
        if(name[0] == 'A' && name[1] >= 'B' && name[1] <= 'E') {
            int row = (nrows - 1) - (name[1] - 'B');
            int col = (name[2] - '0') * 10 + (name[3] - '0') - 1;
            
            if(row >= 0 && row < nrows && col >= 0 && col < ncols) {
                keycode_lookup_table[i] = positional_keys[row * ncols + col];
            }
        }
        
        // numpad
        
        else if(name[0] == 'K' && name[1] == 'P' && name[2] >= '0' && name[2] <= '9' && !name[3]) {
            
            // don't overwrite - for e.g. laptops with numpad keys embedded in the normal ones, toggling with numlock
            if(keycode_lookup_table[i] == 0) {
                keycode_lookup_table[i] = KeyCode_NumPad0 + name[2] - '0';
            }
        }
        
        // a few special cases:
        
        else if(memcmp(name, "TLDE", XkbKeyNameLength) == 0) {
            keycode_lookup_table[i] = KeyCode_Tick;
        } else if(memcmp(name, "BKSL", XkbKeyNameLength) == 0) {
            keycode_lookup_table[i] = KeyCode_BackwardSlash;
        } else if(memcmp(name, "LSGT", XkbKeyNameLength) == 0) {
            // UK extra key between left shift and Z
            // it prints \ and | with shift. KeyCode_Backslash will be where UK # is.
            keycode_lookup_table[i] = KeyCode_Ex0;
        }
    }
}

internal void
linux_keycode_init(Display* dpy){
    block_zero_array(keycode_lookup_table_physical);
    block_zero_array(keycode_lookup_table_language);
    
    linux_keycode_init_physical(dpy, keycode_lookup_table_physical);
    linux_keycode_init_language(dpy, keycode_lookup_table_language);
}

internal void
linux_epoll_init(void) {
    struct epoll_event e = {};
    e.events = EPOLLIN | EPOLLET;
    
    linuxvars.step_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    linuxvars.epoll = epoll_create(16);
    
    e.data.ptr = &epoll_tag_x11;
    epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, ConnectionNumber(linuxvars.dpy), &e);
    
    e.data.ptr = &epoll_tag_step_timer;
    epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, linuxvars.step_timer_fd, &e);
}

internal void
linux_clipboard_send(XSelectionRequestEvent* req) {
    
    XSelectionEvent rsp = {};
    rsp.type = SelectionNotify;
    rsp.requestor = req->requestor;
    rsp.selection = req->selection;
    rsp.target = req->target;
    rsp.time = req->time;
    rsp.property = None;
    
    Atom formats[] = {
        linuxvars.atom_UTF8_STRING,
        XA_STRING,
    };
    
    if(linuxvars.clipboard_contents.size == 0) {
        goto done;
    }
    
    if(req->selection != linuxvars.atom_CLIPBOARD || req->property == None) {
        goto done;
    }
    
    if (req->target == linuxvars.atom_TARGETS){
        
        XChangeProperty(
                        req->display,
                        req->requestor,
                        req->property,
                        XA_ATOM,
                        32,
                        PropModeReplace,
                        (u8*)formats,
                        ArrayCount(formats));
        
        rsp.property = req->property;
        
    } else {
        
        int i;
        for(i = 0; i < ArrayCount(formats); ++i){
            if (req->target == formats[i]){
                break;
            }
        }
        
        if (i != ArrayCount(formats)){
            XChangeProperty(
                            req->display,
                            req->requestor,
                            req->property,
                            req->target,
                            8,
                            PropModeReplace,
                            linuxvars.clipboard_contents.str,
                            linuxvars.clipboard_contents.size
                            );
            
            rsp.property = req->property;
        }
    }
    
    done:
    XSendEvent(req->display, req->requestor, True, 0, (XEvent*)&rsp);
}

internal String_Const_u8
linux_clipboard_recv(Arena *arena){
    Atom type;
    int fmt;
    unsigned long nitems;
    unsigned long bytes_left;
    u8 *data;
    
    int result = XGetWindowProperty(linuxvars.dpy,
                                    linuxvars.win,
                                    linuxvars.atom_CLIPBOARD,
                                    0L, 0x20000000L, False,
                                    linuxvars.atom_UTF8_STRING,
                                    &type, &fmt, &nitems,
                                    &bytes_left, &data);
    
    String_Const_u8 clip = {};
    if(result == Success && fmt == 8){
        clip= push_string_copy(arena, SCu8(data, nitems));
        XFree(data);
        XDeleteProperty(linuxvars.dpy, linuxvars.win, linuxvars.atom_CLIPBOARD);
    }
    
    return(clip);
}

internal void
linux_clipboard_recv(XSelectionEvent* ev) {
    
    if(ev->selection != linuxvars.atom_CLIPBOARD ||
       ev->target != linuxvars.atom_UTF8_STRING ||
       ev->property == None) {
        return;
    }
    
    Scratch_Block scratch(&linuxvars.tctx);
    String_Const_u8 clip = linux_clipboard_recv(scratch);
    if (clip.size > 0){
        linalloc_clear(&linuxvars.clipboard_arena);
        linuxvars.clipboard_contents = push_string_copy(&linuxvars.clipboard_arena, clip);
        linuxvars.received_new_clipboard = true;
        linux_schedule_step();
    }
}

internal
system_get_clipboard_sig(){
    // TODO(inso): index?
    return(push_string_copy(arena, linuxvars.clipboard_contents));
}

internal void
system_post_clipboard(String_Const_u8 str, i32 index){
    // TODO(inso): index?
    //LINUX_FN_DEBUG("%.*s", string_expand(str));
    linalloc_clear(&linuxvars.clipboard_arena);
    linuxvars.clipboard_contents = push_u8_stringf(&linuxvars.clipboard_arena, "%.*s", string_expand(str));
    XSetSelectionOwner(linuxvars.dpy, linuxvars.atom_CLIPBOARD, linuxvars.win, CurrentTime);
}

internal void
system_set_clipboard_catch_all(b32 enabled){
    LINUX_FN_DEBUG("%d", enabled);
    linuxvars.clipboard_catch_all = !!enabled;
}

internal b32
system_get_clipboard_catch_all(void){
    return linuxvars.clipboard_catch_all;
}

internal String_Const_u8
linux_filter_text(Arena* arena, u8* buf, int len) {
    u8* const result = push_array(arena, u8, len);
    u8* outp = result;
    
    for(int i = 0; i < len; ++i) {
        u8 c = buf[i];
        
        if(c == '\r') {
            *outp++ = '\n';
        } else if(c > 127 || (' ' <= c && c <= '~') || c == '\t') {
            *outp++ = c;
        }
    }
    
    return SCu8(result, outp - result);
}

internal KeyCode
linux_numlock_convert(KeyCode in){
    static const KeyCode lookup[] = {
        KeyCode_Insert,
        KeyCode_End,
        KeyCode_Down,
        KeyCode_PageDown,
        KeyCode_Left,
        0,
        KeyCode_Right,
        KeyCode_Home,
        KeyCode_Up,
        KeyCode_PageUp,
        0, 0, 0,
        KeyCode_Delete,
    };
    
    if(in >= KeyCode_NumPad0 && in <= KeyCode_NumPadDot) {
        KeyCode ret = lookup[in - KeyCode_NumPad0];
        if(ret != 0) {
            return ret;
        }
    }
    
    return in;
}

internal void
linux_handle_x11_events() {
    static XEvent prev_event = {};
    b32 should_step = false;
    
    while (XPending(linuxvars.dpy)) {
        XEvent event;
        XNextEvent(linuxvars.dpy, &event);
        
        b32 filtered = false;
        if (XFilterEvent(&event, None) == True){
            filtered = true;
            if(event.type != KeyPress && event.type != KeyRelease) {
                continue;
            }
        }
        
        u64 event_id = (u64)event.xkey.serial << 32 | event.xkey.time;
        
        switch(event.type) {
            case KeyPress: {
                should_step = true;
                
                Input_Modifier_Set_Fixed* mods = &linuxvars.input.pers.modifiers;
                
                int state = event.xkey.state;
                set_modifier(mods, KeyCode_Shift, state & ShiftMask);
                set_modifier(mods, KeyCode_Control, state & ControlMask);
                set_modifier(mods, KeyCode_CapsLock, state & LockMask);
                set_modifier(mods, KeyCode_Alt, state & Mod1Mask);
                
                event.xkey.state &= ~(ControlMask);
                
                Status status;
                KeySym keysym = NoSymbol;
                u8 buf[256] = {};
                
                int len = Xutf8LookupString(linuxvars.xic, &event.xkey, (char*)buf, sizeof(buf) - 1, &keysym, &status);
                
                if (status == XBufferOverflow){
                    Xutf8ResetIC(linuxvars.xic);
                    XSetICFocus(linuxvars.xic);
                }
                
                if (keysym == XK_ISO_Left_Tab){
                    add_modifier(mods, KeyCode_Shift);
                }
                
                Key_Code key;
                if(linuxvars.key_mode == KeyMode_Physical) {
                    key = keycode_lookup_table_physical[(u8)event.xkey.keycode];
                } else {
                    key = keycode_lookup_table_language[(u8)event.xkey.keycode];
                }
                
                if(!(state & Mod2Mask)) {
                    key = linux_numlock_convert(key);
                }
                
                //printf("key %d = %s (f:%d)\n", event.xkey.keycode, key_code_name[key], filtered);
                
                b32 is_dead = false;
                if (keysym >= XK_dead_grave && keysym <= XK_dead_greek && len == 0) {
                    is_dead = true;
                }
                
                if(!is_dead && filtered) {
                    linuxvars.prev_filtered_key = key;
                    break;
                }
                
                // send a keycode for the key after the dead key
                if(!key && linuxvars.prev_filtered_key) {
                    key = linuxvars.prev_filtered_key;
                    linuxvars.prev_filtered_key = 0;
                }
                
                Input_Event* key_event = NULL;
                if(key) {
                    add_modifier(mods, key);
                    // printf(" push key %d\n", key);
                    
                    key_event = push_input_event(&linuxvars.frame_arena, &linuxvars.input.trans.event_list);
                    key_event->kind = InputEventKind_KeyStroke;
                    key_event->key.code = key;
                    key_event->key.modifiers = copy_modifier_set(&linuxvars.frame_arena, mods);
                    key_event->key.flags = 0;
                    if (is_dead){
                        key_event->key.flags |= KeyFlag_IsDeadKey;
                    }
                }
                
                Input_Event* text_event = NULL;
                if(status == XLookupChars || status == XLookupBoth) {
                    String_Const_u8 str = linux_filter_text(&linuxvars.frame_arena, buf, len);
                    if(str.size) {
                        // printf(" push txt %d\n", key);
                        text_event = push_input_event(&linuxvars.frame_arena, &linuxvars.input.trans.event_list);
                        text_event->kind = InputEventKind_TextInsert;
                        text_event->text.string = str;
                    }
                }
                
                if(key_event && text_event) {
                    key_event->key.first_dependent_text = text_event;
                }
            } break;
            
            case KeyRelease: {
                should_step = true;
                
                Input_Modifier_Set_Fixed* mods = &linuxvars.input.pers.modifiers;
                
                int state = event.xkey.state;
                set_modifier(mods, KeyCode_Shift, state & ShiftMask);
                set_modifier(mods, KeyCode_Control, state & ControlMask);
                set_modifier(mods, KeyCode_CapsLock, state & LockMask);
                set_modifier(mods, KeyCode_Alt, state & Mod1Mask);
                
                Key_Code key;
                if(linuxvars.key_mode == KeyMode_Physical) {
                    key = keycode_lookup_table_physical[(u8)event.xkey.keycode];
                } else {
                    key = keycode_lookup_table_language[(u8)event.xkey.keycode];
                }
                
                // num lock off -> convert KP keys to Insert, Home, End etc.
                if(!(state & Mod2Mask)) {
                    key = linux_numlock_convert(key);
                }
                
                Input_Event* key_event = NULL;
                if(key) {
                    remove_modifier(mods, key);
                    key_event = push_input_event(&linuxvars.frame_arena, &linuxvars.input.trans.event_list);
                    key_event->kind = InputEventKind_KeyRelease;
                    key_event->key.code = key;
                    key_event->key.modifiers = copy_modifier_set(&linuxvars.frame_arena, mods);
                }
            } break;
            
            case MotionNotify: {
                int x = clamp(0, event.xmotion.x, render_target.width - 1);
                int y = clamp(0, event.xmotion.y, render_target.height - 1);
                linuxvars.input.pers.mouse = { x, y };
                should_step = true;
            } break;
            
            case ButtonPress: {
                should_step = true;
                switch(event.xbutton.button) {
                    case Button1: {
                        linuxvars.input.trans.mouse_l_press = true;
                        linuxvars.input.pers.mouse_l = true;
                        
                        // NOTE(inso): improves selection dragging (especially in notepad-like mode).
                        // we will still get mouse events when the pointer leaves the window if it's dragging.
                        XGrabPointer(
                                     linuxvars.dpy,
                                     linuxvars.win,
                                     True, PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                                     GrabModeAsync, GrabModeAsync,
                                     None, None, CurrentTime);
                        
                    } break;
                    
                    case Button3: {
                        linuxvars.input.trans.mouse_r_press = true;
                        linuxvars.input.pers.mouse_r = true;
                    } break;
                    
                    case Button4: {
                        linuxvars.input.trans.mouse_wheel = -100;
                    } break;
                    
                    case Button5: {
                        linuxvars.input.trans.mouse_wheel = +100;
                    } break;
                }
            } break;
            
            case ButtonRelease: {
                should_step = true;
                switch(event.xbutton.button) {
                    case Button1: {
                        linuxvars.input.trans.mouse_l_release = true;
                        linuxvars.input.pers.mouse_l = false;
                        
                        XUngrabPointer(linuxvars.dpy, CurrentTime);
                    } break;
                    
                    case Button3: {
                        linuxvars.input.trans.mouse_r_release = true;
                        linuxvars.input.pers.mouse_r = false;
                    } break;
                }
            } break;
            
            case FocusIn:
            case FocusOut: {
                linuxvars.input.pers.mouse_l = false;
                linuxvars.input.pers.mouse_r = false;
                block_zero_struct(&linuxvars.input.pers.modifiers);
            } break;
            
            case EnterNotify: {
                linuxvars.input.pers.mouse_out_of_window = 0;
            } break;
            
            case LeaveNotify: {
                linuxvars.input.pers.mouse_out_of_window = 1;
            } break;
            
            case ConfigureNotify: {
                i32 w = event.xconfigure.width;
                i32 h = event.xconfigure.height;
                
                if (w != render_target.width || h != render_target.height){
                    should_step = true;
                    render_target.width = w;
                    render_target.height = h;
                }
            } break;
            
            case ClientMessage: {
                Atom atom = event.xclient.data.l[0];
                
                // Window X button clicked
                if(atom == linuxvars.atom_WM_DELETE_WINDOW) {
                    should_step = true;
                    linuxvars.input.trans.trying_to_kill = true;
                }
                
                // Notify WM that we're still responding (don't grey our window out).
                else if(atom == linuxvars.atom__NET_WM_PING) {
                    event.xclient.window = DefaultRootWindow(linuxvars.dpy);
                    XSendEvent(linuxvars.dpy,
                               event.xclient.window,
                               False,
                               SubstructureRedirectMask | SubstructureNotifyMask,
                               &event);
                }
            } break;
            
            case SelectionRequest: {
                linux_clipboard_send((XSelectionRequestEvent*)&event);
            } break;
            
            case SelectionNotify: {
                linux_clipboard_recv((XSelectionEvent*)&event);
            } break;
            
            case SelectionClear: {
                if(event.xselectionclear.selection == linuxvars.atom_CLIPBOARD) {
                    linalloc_clear(&linuxvars.clipboard_arena);
                    block_zero_struct(&linuxvars.clipboard_contents);
                }
            } break;
            
            case Expose:
            case VisibilityNotify: {
                should_step = true;
            } break;
            
            default: {
                // clipboard update notification - ask for the new content
                if (event.type == linuxvars.xfixes_selection_event) {
                    XFixesSelectionNotifyEvent* sne = (XFixesSelectionNotifyEvent*)&event;
                    if (sne->subtype == XFixesSelectionNotify && sne->owner != linuxvars.win){
                        XConvertSelection(linuxvars.dpy,
                                          linuxvars.atom_CLIPBOARD,
                                          linuxvars.atom_UTF8_STRING,
                                          linuxvars.atom_CLIPBOARD,
                                          linuxvars.win,
                                          CurrentTime);
                    }
                }
                
                else if(event.type == linuxvars.xkb_event) {
                    XkbEvent* kb = (XkbEvent*)&event;
                    
                    // Keyboard layout changed, refresh lookup table.
                    if(kb->any.xkb_type == XkbStateNotify && kb->state.group != linuxvars.xkb_group) {
                        linuxvars.xkb_group = kb->state.group;
                        XkbRefreshKeyboardMapping((XkbMapNotifyEvent*)kb);
                        linux_keycode_init(linuxvars.dpy);
                    }
                }
            } break;
        }
    }
    
    if(should_step) {
        linux_schedule_step();
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
            
            case EPOLL_STEP_TIMER: {
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
                Linux_Object* obj = CastFromMember(Linux_Object, timer.epoll_tag, tag);
                close(obj->timer.fd);
                obj->timer.fd = -1;
                linux_schedule_step();
            } break;
        }
    }
    
    return do_step;
}

int
main(int argc, char **argv){
    // NOTE(allen): fucking bullshit. someone get my shit togeth :(er
    
    for (i32 i = 0; i < argc; i += 1){
        String_Const_u8 arg = SCu8(argv[i]);
        if (string_match(arg, str8_lit("-L"))){
            log_os_enabled = true;
        }
    }
    
    // NOTE(allen): All of This thing
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&linuxvars.memory_tracker_mutex, &attr);
    
    pthread_mutex_init(&linuxvars.audio_mutex, &attr);
    pthread_cond_init(&linuxvars.audio_cond, NULL);
    
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
    linuxvars.frame_arena = make_arena_system();
    linuxvars.clipboard_arena = make_arena_system();
    render_target.arena = make_arena_system(KB(256));
    
    //linuxvars.fontconfig = FcInitLoadConfigAndFonts();
    
    linuxvars.cursor_show = MouseCursorShow_Always;
    linuxvars.prev_cursor_show = MouseCursorShow_Always;
    
    dll_init_sentinel(&linuxvars.free_linux_objects);
    dll_init_sentinel(&linuxvars.timer_objects);
    
    //InitializeCriticalSection(&win32vars.thread_launch_mutex);
    //InitializeConditionVariable(&win32vars.thread_launch_cv);
    
    linuxvars.clipboard_catch_all = false;
    
    // NOTE(allen): load core
    System_Library core_library = {};
    App_Functions app = {};
    {
        App_Get_Functions *get_funcs = 0;
        Scratch_Block scratch(&linuxvars.tctx);
        List_String_Const_u8 search_list = {};
        def_search_list_add_system_path(scratch, &search_list, SystemPath_Binary);
        
        String_Const_u8 core_path = def_search_get_full_path(scratch, &search_list, SCu8("4ed_app.so"));
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
    // get_logger calls log_init which is needed.
    //app.get_logger();
    linuxvars.log_string = app.get_logger();
    
    // NOTE(allen): init & command line parameters
    Plat_Settings plat_settings = {};
    void *base_ptr = 0;
    {
        Scratch_Block scratch(&linuxvars.tctx);
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
    
    // NOTE(allen): setup user directory override
    if (plat_settings.user_directory != 0){
        lnx_override_user_directory = plat_settings.user_directory;
    }
    
    // NOTE(allen): load custom layer
    System_Library custom_library = {};
    Custom_API custom = {};
    {
        char custom_not_found_msg[] = "Did not find a library for the custom layer.";
        char custom_fail_load_msg[] = "Failed to load custom code due to missing version information.  Try rebuilding with buildsuper.";
        char custom_fail_version_msg[] = "Failed to load custom code due to a version mismatch.  Try rebuilding with buildsuper.";
        char custom_fail_init_apis[] = "Failed to load custom code due to missing 'init_apis' symbol.  Try rebuilding with buildsuper";
        
        Scratch_Block scratch(&linuxvars.tctx);
        String_Const_u8 default_file_name = string_u8_litexpr("custom_4coder.so");
        List_String_Const_u8 search_list = {};
        def_search_list_add_system_path(scratch, &search_list, SystemPath_UserDirectory);
        def_search_list_add_system_path(scratch, &search_list, SystemPath_Binary);
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
            custom_file_name = def_search_get_full_path(scratch, &search_list, custom_file_names[i]);
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
        if (custom.get_version == 0){
            system_error_box(custom_fail_load_msg);
        }
        else if (custom.get_version(MAJOR, MINOR, PATCH) == 0){
            system_error_box(custom_fail_version_msg);
        }
        custom.init_apis = (_Init_APIs_Type*)system_get_proc(custom_library, "init_apis");
        if (custom.init_apis == 0){
            system_error_box(custom_fail_init_apis);
        }
    }
    
    linux_x11_init(argc, argv, &plat_settings);
    linux_keycode_init(linuxvars.dpy);
    linux_epoll_init();
    
    linuxvars.audio_thread = system_thread_launch(&linux_audio_main, NULL);
    
    
    // app init
    {
        Scratch_Block scratch(&linuxvars.tctx);
        String_Const_u8 curdir = system_get_path(scratch, SystemPath_CurrentDirectory);
        app.init(&linuxvars.tctx, &render_target, base_ptr, curdir, custom);
    }
    
    linuxvars.global_frame_mutex = system_mutex_make();
    system_mutex_acquire(linuxvars.global_frame_mutex);
    
    linux_schedule_step();
    b32 first_step = true;
    u64 timer_start = system_now_time();
    
    for (;;) {
        
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
        
        linuxvars.last_step_time = system_now_time();
        
        // NOTE(allen): Frame Clipboard Input
        // Request clipboard contents from X11 on first step, or every step if they don't have XFixes notification ability.
        if (first_step || (!linuxvars.has_xfixes && linuxvars.clipboard_catch_all)){
            XConvertSelection(linuxvars.dpy, linuxvars.atom_CLIPBOARD, linuxvars.atom_UTF8_STRING, linuxvars.atom_CLIPBOARD, linuxvars.win, CurrentTime);
        }
        
        Application_Step_Input input = {};
        
        if (linuxvars.received_new_clipboard && linuxvars.clipboard_catch_all){
            input.clipboard = linuxvars.clipboard_contents;
        }
        linuxvars.received_new_clipboard = false;
        
        input.first_step = first_step;
        input.dt = frame_useconds/1000000.f; // variable?
        input.events = linuxvars.input.trans.event_list;
        input.trying_to_kill = linuxvars.input.trans.trying_to_kill;
        
        input.mouse.out_of_window = linuxvars.input.pers.mouse_out_of_window;
        input.mouse.p = linuxvars.input.pers.mouse;
        input.mouse.l = linuxvars.input.pers.mouse_l;
        input.mouse.r = linuxvars.input.pers.mouse_r;
        input.mouse.press_l = linuxvars.input.trans.mouse_l_press;
        input.mouse.release_l = linuxvars.input.trans.mouse_l_release;
        input.mouse.press_r = linuxvars.input.trans.mouse_r_press;
        input.mouse.release_r = linuxvars.input.trans.mouse_r_release;
        input.mouse.wheel = linuxvars.input.trans.mouse_wheel;
        
        // NOTE(allen): Application Core Update
        Application_Step_Result result = {};
        if (app.step != 0){
            result = app.step(&linuxvars.tctx, &render_target, base_ptr, &input);
        }
        
        // NOTE(allen): Finish the Loop
        if (result.perform_kill){
            break;
        }
        
        // NOTE(NAME): Switch to New Title
        if (result.has_new_title){
            XStoreName(linuxvars.dpy, linuxvars.win, result.title_string);
        }
        
        // NOTE(allen): Switch to New Cursor
        if (result.mouse_cursor_type != linuxvars.cursor && !linuxvars.input.pers.mouse_l){
            XCursor c = linuxvars.xcursors[result.mouse_cursor_type];
            if (linuxvars.cursor_show){
                XDefineCursor(linuxvars.dpy, linuxvars.win, c);
            }
            linuxvars.cursor = result.mouse_cursor_type;
        }
        
        gl_render(&render_target);
        glXSwapBuffers(linuxvars.dpy, linuxvars.win);
        
        // TODO(allen): don't let the screen size change until HERE after the render
        
        // NOTE(allen): Schedule a step if necessary
        if (result.animating){
            linux_schedule_step();
        }
        
        first_step = false;
        
        linalloc_clear(&linuxvars.frame_arena);
        block_zero_struct(&linuxvars.input.trans);
    }
    
    return 0;
}

// NOTE(inso): to prevent me continuously messing up indentation
// vim: et:ts=4:sts=4:sw=4
