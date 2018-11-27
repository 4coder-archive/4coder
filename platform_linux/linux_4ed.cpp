/*
 * Mr. 4th Dimention - Allen Webster
 *  (Mostly by insofaras)
 *
 * 14.11.2015
 *
 * Linux layer for 4coder
 *
 */

// TOP

#define IS_PLAT_LAYER
#include "4coder_os_comp_cracking.h"

#include <string.h>
#include "4ed_defines.h"
#include "4coder_API/version.h"

#include "4coder_lib/4coder_utf8.h"

#if defined(FRED_SUPER)
# include "4coder_API/4coder_keycodes.h"
# include "4coder_API/4coder_style.h"

# define FSTRING_IMPLEMENTATION
# include "4coder_lib/4coder_string.h"
# include "4coder_lib/4coder_arena.h"
# include "4coder_lib/4coder_arena.cpp"

# include "4coder_API/types.h"

#else
# include "4coder_default_bindings.cpp"
#endif

#include "4ed_math.h"

#include "4ed_font.h"
#include "4ed_system.h"
#include "4ed_log.h"
#include "4ed_render_target.h"
#include "4ed_render_format.h"
#include "4ed.h"
#include "4ed_linked_node_macros.h"

#include "4ed_file_track.h"
#include "4ed_system_shared.h"

#include "unix_4ed_headers.h"
#include <math.h>
#include <stdlib.h>

#include <locale.h>
#include <xmmintrin.h>
#include <ucontext.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <sys/inotify.h>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xfixes.h>

#include <GL/glx.h>
#include <GL/glext.h>

#include <linux/fs.h>
#include <linux/input.h>

#include "4ed_shared_thread_constants.h"
#include "unix_threading_wrapper.h"
#include "linux_semaphore_wrapper.h"

//
// Linux macros
//

#define FPS 60L
#define frame_useconds (1000000UL / FPS)

#define LINUX_FN_DEBUG(fmt, ...) do { \
    LOGF("%s: " fmt "\n", __func__, ##__VA_ARGS__); \
} while (0)

// TODO(allen): Make an intrinsics header that uses the cracked OS to define a single set of intrinsic names.
#define InterlockedCompareExchange(dest, ex, comp) \
__sync_val_compare_and_swap((dest), (comp), (ex))

//
// Linux structs / enums
//

enum {
    LINUX_4ED_EVENT_X11          = (UINT64_C(1) << 32),
    LINUX_4ED_EVENT_X11_INTERNAL = (UINT64_C(2) << 32),
    LINUX_4ED_EVENT_STEP         = (UINT64_C(3) << 32),
    LINUX_4ED_EVENT_STEP_TIMER   = (UINT64_C(4) << 32),
    LINUX_4ED_EVENT_CLI          = (UINT64_C(5) << 32),
};

//
// Linux forward declarations
//

internal void        LinuxStringDup(String*, void*, size_t);

////////////////////////////////

#define SLASH '/'
#define DLL "so"

global System_Functions sysfunc;
#include "4ed_shared_library_constants.h"
#include "unix_library_wrapper.h"

#include "4ed_standard_libraries.cpp"
#include "4ed_coroutine.cpp"
#include "4ed_font.cpp"

////////////////////////////////

struct Linux_Vars{
    Display *XDisplay;
    Window XWindow;
    
    XIM input_method;
    XIMStyle input_style;
    XIC input_context;
    
    Application_Step_Input input;
    
    String clipboard_contents;
    String clipboard_outgoing;
    b32 new_clipboard;
    
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
    
    b32 has_xfixes;
    int xfixes_selection_event;
    
    int epoll;
    
    int step_timer_fd;
    int step_event_fd;
    int x11_fd;
    int inotify_fd;
    
    u64 last_step;
    
    b32 full_screen;
    b32 do_toggle;
    b32 keep_running;
    
    Application_Mouse_Cursor cursor;
    b32 hide_cursor;
    Cursor hidden_cursor;
    
    sem_t thread_semaphore;
    
    i32 dpi_x, dpi_y;
    
    b32 vsync;
};

////////////////////////////////

global Linux_Vars linuxvars;
global Render_Target target;
global Application_Memory memory_vars;
global Plat_Settings plat_settings;

global Libraries libraries;
global App_Functions app;
global Custom_API custom_api;

global Coroutine_System_Auto_Alloc coroutines;

////////////////////////////////

#include "linux_icon.h"
internal void
linux_set_icon(Display* d, Window w){
    Atom WM_ICON = XInternAtom(d, "_NET_WM_ICON", False);
    XChangeProperty(d, w, WM_ICON, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)linux_icon, sizeof(linux_icon) / sizeof(long));
}

#include "linux_error_box.cpp"

////////////////////////////////

internal sem_t*
handle_sem(Plat_Handle h){
    return(*(sem_t**)&h);
}

internal Plat_Handle
handle_sem(sem_t *sem){
    return(*(Plat_Handle*)&sem);
}

////////////////////////////////

internal
Sys_Get_4ed_Path_Sig(system_get_4ed_path){
    ssize_t size = readlink("/proc/self/exe", out, capacity - 1);
    if (size != -1 && size < capacity - 1){
        String str = make_string(out, size);
        remove_last_folder(&str);
        terminate_with_null(&str);
        size = str.size;
    }
    else{
        size = 0;
    }
    return(size);
}

#include "unix_4ed_functions.cpp"

internal
Sys_Now_Time_Sig(system_now_time){
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    u64 result = (spec.tv_sec*UINT64_C(1000000)) + (spec.tv_nsec/UINT64_C(1000));
    return(result);
}

#include "4ed_shared_file_handling.cpp"

////////////////////////////////

internal void
system_schedule_step(){
    u64 now  = system_now_time();
    u64 diff = (now - linuxvars.last_step);
    
    if (diff > (u64)frame_useconds){
        u64 ev = 1;
        ssize_t size = write(linuxvars.step_event_fd, &ev, sizeof(ev));
        AllowLocal(size);
    }
    else{
        struct itimerspec its = {};
        timerfd_gettime(linuxvars.step_timer_fd, &its);
        
        if (its.it_value.tv_sec == 0 && its.it_value.tv_nsec == 0){
            its.it_value.tv_nsec = (frame_useconds - diff) * 1000UL;
            timerfd_settime(linuxvars.step_timer_fd, 0, &its, NULL);
        }
    }
}

////////////////////////////////

#include "4ed_work_queues.cpp"

////////////////////////////////

internal void
linux_set_wm_state(Display* d, Window w, Atom one, Atom two, int mode){
    //NOTE(inso): this will only work after it is mapped
    
    enum { STATE_REMOVE, STATE_ADD, STATE_TOGGLE };
    
    XEvent e = {};
    
    e.xany.type = ClientMessage;
    e.xclient.message_type = linuxvars.atom__NET_WM_STATE;
    e.xclient.format = 32;
    e.xclient.window = w;
    e.xclient.data.l[0] = mode;
    e.xclient.data.l[1] = one;
    e.xclient.data.l[2] = two;
    e.xclient.data.l[3] = 1L;
    
    XSendEvent(d, RootWindow(d, 0), 0, SubstructureNotifyMask | SubstructureRedirectMask, &e);
}

internal void
linux_maximize_window(b32 maximize){
    linux_set_wm_state(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom__NET_WM_STATE_MAXIMIZED_HORZ, linuxvars.atom__NET_WM_STATE_MAXIMIZED_VERT, maximize?1:0);
}

internal void
linux_toggle_fullscreen(){
    linuxvars.full_screen = !linuxvars.full_screen;
    linux_set_wm_state(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom__NET_WM_STATE_FULLSCREEN, 0, linuxvars.full_screen?1:0);
}

internal
Sys_Show_Mouse_Cursor_Sig(system_show_mouse_cursor){
    linuxvars.hide_cursor = !show;
    XDefineCursor(linuxvars.XDisplay, linuxvars.XWindow, show?None:linuxvars.hidden_cursor);
}

internal
Sys_Set_Fullscreen_Sig(system_set_fullscreen){
    b32 success = true;
    linuxvars.do_toggle = (linuxvars.full_screen != full_screen);
    return(success);
}

internal
Sys_Is_Fullscreen_Sig(system_is_fullscreen){
    b32 result = (linuxvars.full_screen != linuxvars.do_toggle);
    return(result);
}

#include "4ed_coroutine_functions.cpp"

#include "4ed_system_shared.cpp"

//
// Clipboard
//

internal
Sys_Post_Clipboard_Sig(system_post_clipboard){
    LinuxStringDup(&linuxvars.clipboard_outgoing, str.str, str.size);
    XSetSelectionOwner(linuxvars.XDisplay, linuxvars.atom_CLIPBOARD, linuxvars.XWindow, CurrentTime);
}

//
// CLI
//

internal
Sys_CLI_Call_Sig(system_cli_call, path, script_name, cli_out){
    LINUX_FN_DEBUG("%s %s", path, script_name);
    
    int pipe_fds[2];
    if (pipe(pipe_fds) == -1){
        perror("system_cli_call: pipe");
        return 0;
    }
    
    pid_t child_pid = fork();
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
        
        char* argv[] = { "sh", "-c", script_name, NULL };
        
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
        e.data.u64 = LINUX_4ED_EVENT_CLI;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, pipe_fds[PIPE_FD_READ], &e);
    }
    
    return(true);
}

internal
Sys_CLI_Begin_Update_Sig(system_cli_begin_update){
    // NOTE(inso): I don't think anything needs to be done here.
}

internal
Sys_CLI_Update_Step_Sig(system_cli_update_step){
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

internal
Sys_CLI_End_Update_Sig(system_cli_end_update){
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

#include "4ed_font_provider_freetype.h"
global u32 system_font_method = SystemFontMethod_FilePath;
#include "4ed_font_provider_freetype.cpp"

#undef internal
#include <fontconfig/fontconfig.h>
#define internal static

global FcConfig *fc_config = 0;

internal
Sys_Font_Path(name, parameters){
    if (fc_config == 0){
        fc_config = FcInitLoadConfigAndFonts();
    }
    
    Font_Path path = {};
    
    FcPattern *pattern_regular = FcPatternBuild(
        0,
        FC_POSTSCRIPT_NAME, FcTypeString, name,
        FC_SIZE, FcTypeDouble, (double)parameters->pt_size,
        FC_FONTFORMAT, FcTypeString, "TrueType",
        FC_STYLE, FcTypeString, (FcChar8*)"Regular",
        (void*)0);
    
    FcPattern *pattern_styled = 0;
    if (parameters->italics || parameters->bold){
        if (parameters->italics && !parameters->bold){
            pattern_styled = FcPatternBuild(
                0,
                FC_POSTSCRIPT_NAME, FcTypeString, name,
                FC_SIZE, FcTypeDouble, (double)parameters->pt_size,
                FC_FONTFORMAT, FcTypeString, "TrueType",
                FC_STYLE, FcTypeString, (FcChar8*)"Italic",
                (void*)0);
        }
        else if (!parameters->italics && parameters->bold){
            pattern_styled = FcPatternBuild(
                0,
                FC_POSTSCRIPT_NAME, FcTypeString, name,
                FC_SIZE, FcTypeDouble, (double)parameters->pt_size,
                FC_FONTFORMAT, FcTypeString, "TrueType",
                FC_STYLE, FcTypeString, (FcChar8*)"Bold",
                (void*)0);
        }
        else{
            pattern_styled = FcPatternBuild(
                0,
                FC_POSTSCRIPT_NAME, FcTypeString, name,
                FC_SIZE, FcTypeDouble, (double)parameters->pt_size,
                FC_FONTFORMAT, FcTypeString, "TrueType",
                FC_STYLE, FcTypeString, (FcChar8*)"Bold Italic",
                (void*)0);
        }
    }
    
    Partition *part = &shared_vars.font_scratch;
    path.temp = begin_temp_memory(part);
    
    b32 got_font = false;
    if (pattern_styled != 0){
        if (FcConfigSubstitute(fc_config, pattern_styled, FcMatchPattern)){
            FcDefaultSubstitute(pattern_styled);
            
            FcResult res;
            FcPattern *font = FcFontMatch(fc_config, pattern_styled, &res);
            FcChar8 *filename = 0;
            
            if (font != 0){
                FcPatternGetString(font, FC_FILE, 0, &filename);
                if (filename != 0){
                    i32 len = str_size((char*)filename);
                    char *buffer = push_array(part, char, len + 1);
                    if (buffer == 0){
                        sysshared_partition_grow(part, l_round_up_i32(len + 1, KB(4)));
                        buffer = push_array(part, char, len + 1);
                    }
                    
                    if (buffer != 0){
                        push_align(part, 8);
                        memcpy(buffer, filename, len + 1);
                        path.len = len;
                        path.name = buffer;
                    }
                    
                    got_font = true;
                }
                FcPatternDestroy(font);
            }
        }
        
        FcPatternDestroy(pattern_styled);
        pattern_styled = 0;
    }
    
    if (!got_font){
        if (FcConfigSubstitute(fc_config, pattern_regular, FcMatchPattern)){
            FcDefaultSubstitute(pattern_regular);
            
            FcResult res;
            FcPattern *font = FcFontMatch(fc_config, pattern_regular, &res);
            FcChar8 *filename = 0;
            
            if (font != 0){
                FcPatternGetString(font, FC_FILE, 0, &filename);
                if (filename != 0){
                    i32 len = str_size((char*)filename);
                    char *buffer = push_array(part, char, len + 1);
                    if (buffer == 0){
                        sysshared_partition_grow(part, l_round_up_i32(len + 1, KB(4)));
                        buffer = push_array(part, char, len + 1);
                    }
                    
                    if (buffer != 0){
                        push_align(part, 8);
                        memcpy(buffer, filename, len + 1);
                        path.len = len;
                        path.name = buffer;
                    }
                    
                    got_font = true;
                }
                FcPatternDestroy(font);
            }
        }
        
        FcPatternDestroy(pattern_styled);
    }
    
    FcPatternDestroy(pattern_regular);
    
    return(path);
}

Sys_Font_Data_Not_Used;

internal char*
linux_get_loadable_fonts(Partition *part, Font_Setup_List *list){
    if (fc_config == 0){
        fc_config = FcInitLoadConfigAndFonts();
    }
    
    FcPattern* pat = FcPatternBuild(
        0,
        FC_STYLE, FcTypeString, (FcChar8*)"Regular",
        (void*)0);
    FcObjectSet* os = FcObjectSetBuild(FC_FAMILY, FC_FILE, (char*)0);
    FcFontSet* fs = FcFontList(fc_config, pat, os);
    if (fs != 0){
        LOGF("Total matching fonts: %d\n", fs->nfont);
        for (int i=0; fs && i < fs->nfont; ++i) {
            FcPattern* font = fs->fonts[i];
            FcChar8 *file = 0;
            FcChar8 *style = 0;
            FcChar8 *family = 0;
            
            if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch &&
                FcPatternGetString(font, FC_FAMILY, 0, &family) == FcResultMatch)
            {
                Temp_Memory reset = begin_temp_memory(part);
                Font_Setup *setup = push_array(part, Font_Setup, 1);
                if (setup != 0){
                    memset(setup, 0, sizeof(*setup));
                    
                    i32 len = str_size((char*)file);
                    if (len < sizeof(setup->stub.name)){
                        i32 name_len = str_size((char*)family);
                        if (name_len < sizeof(setup->name)){
                            setup->stub.load_from_path = true;
                            memcpy(setup->stub.name, file, len + 1);
                            setup->stub.len = len;
                            setup->has_display_name = true;
                            setup->len = name_len;
                            memcpy(setup->name, family, name_len + 1);
                            sll_push(list->first, list->last, setup);
                        }
                        else{
                            end_temp_memory(reset);
                        }
                    }
                    else{
                        end_temp_memory(reset);
                    }
                }
            }
        }
        FcFontSetDestroy(fs);
    }
}

#include <GL/gl.h>
#include "opengl/4ed_opengl_render.cpp"

//
// End of system funcs
//

//
// Renderer
//

internal void
LinuxResizeTarget(i32 width, i32 height){
    if (width > 0 && height > 0){
        target.width = width;
        target.height = height;
    }
}

//
// OpenGL init
//

// NOTE(allen): Thanks to Casey for providing the linux OpenGL launcher.
static bool ctxErrorOccurred = false;

internal int
ctxErrorHandler( Display *dpy, XErrorEvent *ev ){
    ctxErrorOccurred = true;
    return 0;
}

typedef GLXContext (glXCreateContextAttribsARB_Function)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef void       (glXSwapIntervalEXT_Function)        (Display *dpy, GLXDrawable drawable, int interval);
typedef int        (glXSwapIntervalMESA_Function)       (unsigned int interval);
typedef int        (glXGetSwapIntervalMESA_Function)    (void);
typedef int        (glXSwapIntervalSGI_Function)        (int interval);

internal GLXContext
InitializeOpenGLContext(Display *XDisplay, Window XWindow, GLXFBConfig *best_config){
    
    const char *glxExts = glXQueryExtensionsString(XDisplay, DefaultScreen(XDisplay));
    
    glXCreateContextAttribsARB_Function *glXCreateContextAttribsARB = 0;
    glXSwapIntervalEXT_Function         *glXSwapIntervalEXT = 0;
    glXSwapIntervalMESA_Function        *glXSwapIntervalMESA = 0;
    glXGetSwapIntervalMESA_Function     *glXGetSwapIntervalMESA = 0;
    glXSwapIntervalSGI_Function         *glXSwapIntervalSGI = 0;
    
#define GLXLOAD(f) f = (f##_Function*) glXGetProcAddressARB((const GLubyte*) #f);
    GLXLOAD(glXCreateContextAttribsARB);
    
    GLXContext ctx = 0;
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);
    
    if (glXCreateContextAttribsARB == 0){
        LOG("glXCreateContextAttribsARB() not found, using old-style GLX context\n" );
        ctx = glXCreateNewContext( XDisplay, *best_config, GLX_RGBA_TYPE, 0, True );
    } 
    else{
        int context_attribs[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 2,
            GLX_CONTEXT_MINOR_VERSION_ARB, 1,
            GLX_CONTEXT_PROFILE_MASK_ARB , GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#if defined(FRED_INTERNAL)
            GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
            None
        };
        
        LOG("Creating GL 2.1 context... ");
        ctx = glXCreateContextAttribsARB(XDisplay, *best_config, 0, True, context_attribs);
        
        XSync( XDisplay, False );
        if (!ctxErrorOccurred && ctx){
            LOG("Created GL 2.1 context.\n");
        }
        else{
            LOG("Could not create a context.\n");
            exit(1);
        }
    }
    
    XSync(XDisplay, False);
    XSetErrorHandler(oldHandler);
    
    if (ctxErrorOccurred || !ctx){
        LOG("Failed to create an OpenGL context\n");
        exit(1);
    }
    
    b32 Direct;
    if (!glXIsDirect(XDisplay, ctx)){
        LOG("Indirect GLX rendering context obtained\n");
        Direct = false;
    }
    else{
        LOG("Direct GLX rendering context obtained\n");
        Direct = true;
    }
    
    LOG("Making context current\n");
    glXMakeCurrent( XDisplay, XWindow, ctx );
    
    //TODO(inso): glGetStringi is required in core profile if the GL version is >= 3.0
    //char *Extensions = (char *)glGetString(GL_EXTENSIONS);
    
    //NOTE(inso): enable vsync if available. this should probably be optional
    if (Direct && strstr(glxExts, "GLX_EXT_swap_control ")){
        GLXLOAD(glXSwapIntervalEXT);
        
        if (glXSwapIntervalEXT != 0){
            glXSwapIntervalEXT(XDisplay, XWindow, 1);
            
            unsigned int swap_val = 0;
            glXQueryDrawable(XDisplay, XWindow, GLX_SWAP_INTERVAL_EXT, &swap_val);
            
            linuxvars.vsync = (swap_val == true);
            LOGF("VSync enabled? %s.\n", linuxvars.vsync ? "Yes" : "No");
        }
        
    }
    else if (Direct && strstr(glxExts, "GLX_MESA_swap_control ")){
        
        GLXLOAD(glXSwapIntervalMESA);
        GLXLOAD(glXGetSwapIntervalMESA);
        
        if (glXSwapIntervalMESA != 0){
            glXSwapIntervalMESA(1);
            
            if (glXGetSwapIntervalMESA != 0){
                linuxvars.vsync = glXGetSwapIntervalMESA();
                LOGF("VSync enabled? %s (MESA)\n", linuxvars.vsync ? "Yes" : "No");
            }
            else{
                // NOTE(inso): assume it worked?
                linuxvars.vsync = true;
                LOG("VSync enabled? possibly (MESA)\n");
            }
        }
        
    }
    else if (Direct && strstr(glxExts, "GLX_SGI_swap_control ")){
        GLXLOAD(glXSwapIntervalSGI);
        
        if (glXSwapIntervalSGI){
            glXSwapIntervalSGI(1);
            
            // NOTE(inso): The SGI one doesn't seem to have a way to confirm we got it...
            linuxvars.vsync = true;
            LOG("VSync enabled? hopefully (SGI)\n");
        }
        
    }
    else{
        LOG("VSync enabled? nope, no suitable extension\n");
    }
    
    return(ctx);
}

internal b32
GLXCanUseFBConfig(Display *XDisplay)
{
    b32 Result = false;
    
    int GLXMajor, GLXMinor;
    
    char *XVendor = ServerVendor(XDisplay);
    LOGF("XWindows vendor: %s\n", XVendor);
    if (glXQueryVersion(XDisplay, &GLXMajor, &GLXMinor))
    {
        LOGF("GLX version %d.%d\n", GLXMajor, GLXMinor);
        if (((GLXMajor == 1 ) && (GLXMinor >= 3)) || (GLXMajor > 1))
        {
            Result = true;
        }
    }
    
    return(Result);
}

struct glx_config_result{
    b32 Found;
    GLXFBConfig BestConfig;
    XVisualInfo BestInfo;
};

internal glx_config_result
ChooseGLXConfig(Display *XDisplay, int XScreenIndex)
{
    glx_config_result Result = {};
    
    int DesiredAttributes[] = {
        GLX_X_RENDERABLE    , True,
        GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
        GLX_RENDER_TYPE     , GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
        GLX_RED_SIZE        , 8,
        GLX_GREEN_SIZE      , 8,
        GLX_BLUE_SIZE       , 8,
        GLX_ALPHA_SIZE      , 8,
        GLX_DEPTH_SIZE      , 24,
        GLX_STENCIL_SIZE    , 8,
        GLX_DOUBLEBUFFER    , True,
        None
    };    
    
    int ConfigCount = 0;
    GLXFBConfig *Configs = glXChooseFBConfig(XDisplay, XScreenIndex, DesiredAttributes, &ConfigCount);
    if (Configs != 0 && ConfigCount > 0){
        XVisualInfo* VI = glXGetVisualFromFBConfig(XDisplay, Configs[0]);
        if (VI != 0){
            Result.Found = true;
            Result.BestConfig = Configs[0];
            Result.BestInfo = *VI;
            
            int id = 0;
            glXGetFBConfigAttrib(XDisplay, Result.BestConfig, GLX_FBCONFIG_ID, &id);
            LOGF("Using FBConfig: %d (0x%x)\n", id, id);
            
            XFree(VI);
        }
        
        XFree(Configs);
    }
    
    return(Result);
}

//
// X11 input / events init
//

struct Init_Input_Result{
    XIM input_method;
    XIMStyle best_style;
    XIC xic;
};
static Init_Input_Result null_init_input_result = {};

internal Init_Input_Result
LinuxInputInit(Display *dpy, Window XWindow){
    Init_Input_Result result = {};
    XIMStyles *styles = 0;
    XIMStyle style;
    unsigned long xim_event_mask = 0;
    
    setlocale(LC_ALL, "");
    XSetLocaleModifiers("");
    b32 locale_supported = XSupportsLocale();
    LOGF("Supported locale?: %s.\n", locale_supported ? "Yes" : "No");
    if (!locale_supported){
        LOG("Reverting to 'C' ... ");
        setlocale(LC_ALL, "C");
        locale_supported = XSupportsLocale();
        LOGF("C is supported? %s.\n", locale_supported ? "Yes" : "No");
    }
    
    result.input_method = XOpenIM(dpy, 0, 0, 0);
    if (!result.input_method){
        // NOTE(inso): Try falling back to the internal XIM implementation that
        // should in theory always exist.
        
        XSetLocaleModifiers("@im=none");
        result.input_method = XOpenIM(dpy, 0, 0, 0);
    }
    
    if (result.input_method){
        if (!XGetIMValues(result.input_method, XNQueryInputStyle, &styles, NULL) && styles){
            for (i32 i = 0; i < styles->count_styles; ++i){
                style = styles->supported_styles[i];
                if (style == (XIMPreeditNothing | XIMStatusNothing)){
                    result.best_style = style;
                    break;
                }
            }
        }
        
        if (result.best_style){
            XFree(styles);
            
            result.xic = XCreateIC(result.input_method, XNInputStyle, result.best_style, XNClientWindow, XWindow, XNFocusWindow, XWindow, NULL);
            
            if (XGetICValues(result.xic, XNFilterEvents, &xim_event_mask, NULL)){
                xim_event_mask = 0;
            }
        }
        else{
            result = null_init_input_result;
            LOG("Could not get minimum required input style.\n");
            exit(1);
        }
    }
    else{
        result = null_init_input_result;
        LOG("Could not open X Input Method.\n");
        exit(1);
    }
    
    u32 flags = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | FocusChangeMask | StructureNotifyMask | MappingNotify | ExposureMask | VisibilityChangeMask | xim_event_mask;
    
    XSelectInput(linuxvars.XDisplay, linuxvars.XWindow, flags);
    
    return(result);
}

//
// Keyboard handling funcs
//

#if 0
internal void
LinuxKeycodeInit(Display* dpy){
    
    // NOTE(inso): This looks a bit dumb, but it's the best way I can think of to do it, since:
    // KeySyms are the type representing "virtual" keys, like XK_BackSpace, but they are 32-bit ints.
    // KeyCodes are guaranteed to fit in 1 byte (and therefore the keycode_lookup_table) but
    // have dynamic numbers assigned by the XServer.
    //  There is XKeysymToKeycode, but it only returns 1 KeyCode for a KeySym. I have my capslock
    // rebound to esc, so there are two KeyCodes for the XK_Escape KeyCode but XKeysymToKeycode only
    // gets one of them, hence the need for this crazy lookup which works correctly with rebound keys.
    
    memset(keycode_lookup_table, 0, sizeof(keycode_lookup_table));
    
    struct SymMapping {
        KeySym sym;
        u16 code;
    };
    
    SymMapping sym_table[] = {
        { XK_BackSpace, key_back },
        { XK_Delete, key_del },
        { XK_Up, key_up },
        { XK_Down, key_down },
        { XK_Left, key_left },
        { XK_Right, key_right },
        { XK_Insert, key_insert },
        { XK_Home, key_home },
        { XK_End, key_end },
        { XK_Page_Up, key_page_up },
        { XK_Page_Down, key_page_down },
        { XK_Escape, key_esc },
        { XK_F1, key_f1 },
        { XK_F2, key_f2 },
        { XK_F3, key_f3 },
        { XK_F4, key_f4 },
        { XK_F5, key_f5 },
        { XK_F6, key_f6 },
        { XK_F7, key_f7 },
        { XK_F8, key_f8 },
        { XK_F9, key_f9 },
        { XK_F10, key_f10 },
        { XK_F11, key_f11 },
        { XK_F12, key_f12 },
        { XK_F13, key_f13 },
        { XK_F14, key_f14 },
        { XK_F15, key_f15 },
        { XK_F16, key_f16 },
    };
    
    const int table_size = sizeof(sym_table) / sizeof(struct SymMapping);
    
    int key_min, key_max, syms_per_code;
    XDisplayKeycodes(dpy, &key_min, &key_max);
    
    int key_count = (key_max - key_min) + 1;
    
    KeySym* syms = XGetKeyboardMapping(dpy, key_min, key_count, &syms_per_code);
    
    if (syms == 0){
        return;
    }
    
    int key = key_min;
    for(int i = 0; i < key_count * syms_per_code; ++i){
        for(int j = 0; j < table_size; ++j){
            if (sym_table[j].sym == syms[i]){
                keycode_lookup_table[key + (i/syms_per_code)] = sym_table[j].code;
                break;
            }
        }
    }
    
    XFree(syms);
}
#endif

internal void
LinuxPushKey(Key_Code code, Key_Code chr, Key_Code chr_nocaps, b8 *mods)
{
    i32 *count = &linuxvars.input.keys.count;
    Key_Event_Data *data = linuxvars.input.keys.keys;
    
    if (*count < KEY_INPUT_BUFFER_SIZE){
        data[*count].keycode = code;
        data[*count].character = chr;
        data[*count].character_no_caps_lock = chr_nocaps;
        
        memcpy(data[*count].modifiers, mods, sizeof(*mods)*MDFR_INDEX_COUNT);
        
        ++(*count);
    }
}

//
// Misc utility funcs
//

internal void
LinuxStringDup(String* str, void* data, size_t size){
    if (str->memory_size < size){
        if (str->str){
            system_memory_free(str->str, str->memory_size);
        }
        str->memory_size = size;
        str->str = (char*)system_memory_allocate(size);
        //TODO(inso): handle alloc failure case
    }
    
    str->size = size;
    memcpy(str->str, data, size);
}

//
// X11 utility funcs
//

internal void
LinuxX11ConnectionWatch(Display* dpy, XPointer cdata, int fd, Bool opening, XPointer* wdata){
    struct epoll_event e = {};
    e.events = EPOLLIN | EPOLLET;
    e.data.u64 = LINUX_4ED_EVENT_X11_INTERNAL | fd;
    
    int op = opening ? EPOLL_CTL_ADD : EPOLL_CTL_DEL;
    epoll_ctl(linuxvars.epoll, op, fd, &e);
}

internal int
LinuxGetXSettingsDPI(Display* dpy, int screen)
{
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
        LOG("XSETTINGS unavailable.\n");
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
            LOG("XSETTINGS: GetWindowProperty failed.\n");
            goto out;
        }
        
        if (fmt != 8){
            LOG("XSETTINGS: Wrong format.\n");
            goto out;
        }
    }
    
    xs = (struct XSettings*)prop;
    p  = (char*)(xs + 1);
    
    if (xs->byte_order != 0){
        LOG("FIXME: XSETTINGS not host byte order?\n");
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
                LOG("XSETTINGS: Got invalid type...\n");
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

//
// X11 window init
//

internal f32
size_change(i32 x, i32 y){
    f32 xs = x/96.f;
    f32 ys = y/96.f;
    f32 s = Min(xs, ys);
    return(s);
}

#define BASE_W 800
#define BASE_H 600

internal b32
LinuxX11WindowInit(int argc, char** argv, int* window_width, int* window_height){
    if (plat_settings.set_window_size){
        *window_width = plat_settings.window_w;
        *window_height = plat_settings.window_h;
    } else {
        f32 schange = size_change(linuxvars.dpi_x, linuxvars.dpi_y);
        *window_width = ceil32(BASE_W * schange);
        *window_height = ceil32(BASE_H * schange);
    }
    *window_width = Max(*window_width, 1);
    *window_height = Max(*window_height, 1);
    
    if (!GLXCanUseFBConfig(linuxvars.XDisplay)){
        system_error_box("Your XServer's GLX version is too old. GLX 1.3+ is required.");
    }
    
    glx_config_result Config = ChooseGLXConfig(linuxvars.XDisplay, DefaultScreen(linuxvars.XDisplay));
    if (!Config.Found){
        system_error_box("Could not get a matching GLX FBConfig. Check your OpenGL drivers are installed correctly.");
    }
    
    XSetWindowAttributes swa = {};
    swa.backing_store = WhenMapped;
    swa.event_mask = StructureNotifyMask;
    swa.bit_gravity = NorthWestGravity;
    swa.colormap = XCreateColormap(linuxvars.XDisplay, RootWindow(linuxvars.XDisplay, Config.BestInfo.screen), Config.BestInfo.visual, AllocNone);
    
    u32 CWflags = CWBackingStore|CWBitGravity|CWBackPixel|CWBorderPixel|CWColormap|CWEventMask;
    linuxvars.XWindow = XCreateWindow(linuxvars.XDisplay, RootWindow(linuxvars.XDisplay, Config.BestInfo.screen), 0, 0, *window_width, *window_height, 0, Config.BestInfo.depth, InputOutput, Config.BestInfo.visual, CWflags, &swa);
    
    if (!linuxvars.XWindow){
        system_error_box("XCreateWindow failed. Make sure your display is set up correctly.");
    }
    
    //NOTE(inso): Set the window's type to normal
    XChangeProperty(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom__NET_WM_WINDOW_TYPE, XA_ATOM, 32, PropModeReplace, (unsigned char*)&linuxvars.atom__NET_WM_WINDOW_TYPE_NORMAL, 1);
    
    //NOTE(inso): window managers want the PID as a window property for some reason.
    pid_t pid = getpid();
    XChangeProperty(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom__NET_WM_PID, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&pid, 1);
    
    //NOTE(inso): set wm properties
    XStoreName(linuxvars.XDisplay, linuxvars.XWindow, WINDOW_NAME);
    
    XSizeHints *sz_hints = XAllocSizeHints();
    XWMHints   *wm_hints = XAllocWMHints();
    XClassHint *cl_hints = XAllocClassHint();
    
    sz_hints->flags = PMinSize | PMaxSize | PWinGravity;
    
    sz_hints->min_width = 50;
    sz_hints->min_height = 50;
    
    sz_hints->max_width = sz_hints->max_height = (1UL << 16UL);
    
    /* NOTE(inso): fluxbox thinks this is minimum, so don't set it
        sz_hints->base_width = BASE_W;
        sz_hints->base_height = BASE_H;
    */
    sz_hints->win_gravity = NorthWestGravity;
    
    if (plat_settings.set_window_pos){
        sz_hints->flags |= USPosition;
        sz_hints->x = plat_settings.window_x;
        sz_hints->y = plat_settings.window_y;
    }
    
    wm_hints->flags |= InputHint | StateHint;
    wm_hints->input = True;
    wm_hints->initial_state = NormalState;
    
    cl_hints->res_name = "4coder";
    cl_hints->res_class = "4coder";
    
    char* win_name_list[] = { WINDOW_NAME };
    XTextProperty win_name;
    XStringListToTextProperty(win_name_list, 1, &win_name);
    
    XSetWMProperties(linuxvars.XDisplay, linuxvars.XWindow, &win_name, NULL, argv, argc, sz_hints, wm_hints, cl_hints);
    
    XFree(win_name.value);
    
    XFree(sz_hints);
    XFree(wm_hints);
    XFree(cl_hints);
    
    linux_set_icon(linuxvars.XDisplay, linuxvars.XWindow);
    
    // NOTE(inso): make the window visible
    XMapWindow(linuxvars.XDisplay, linuxvars.XWindow);
    
    GLXContext GLContext =
        InitializeOpenGLContext(linuxvars.XDisplay, linuxvars.XWindow, &Config.BestConfig);
    
    XRaiseWindow(linuxvars.XDisplay, linuxvars.XWindow);
    
    if (plat_settings.set_window_pos){
        XMoveWindow(linuxvars.XDisplay, linuxvars.XWindow, plat_settings.window_x, plat_settings.window_y);
    }
    
    if (plat_settings.maximize_window){
        linux_maximize_window(true);
    }
    else if (plat_settings.fullscreen_window){
        linux_toggle_fullscreen();
    }
    
    XSync(linuxvars.XDisplay, False);
    
    XWindowAttributes WinAttribs;
    if (XGetWindowAttributes(linuxvars.XDisplay, linuxvars.XWindow, &WinAttribs)){
        *window_width = WinAttribs.width;
        *window_height = WinAttribs.height;
    }
    
    Atom wm_protos[] = {
        linuxvars.atom_WM_DELETE_WINDOW,
        linuxvars.atom__NET_WM_PING
    };
    
    XSetWMProtocols(linuxvars.XDisplay, linuxvars.XWindow, wm_protos, 2);
    
    return(true);
}

internal void
LinuxHandleX11Events(void)
{
    static XEvent prev_event = {};
    b32 should_step = false;
    
    while (XPending(linuxvars.XDisplay))
    {
        XEvent event;
        XNextEvent(linuxvars.XDisplay, &event);
        
        if (XFilterEvent(&event, None) == True){
            continue;
        }
        
        switch (event.type){
            case KeyPress: {
                should_step = true;
                
                b32 is_hold = (prev_event.type         == KeyRelease &&
                               prev_event.xkey.time    == event.xkey.time &&
                               prev_event.xkey.keycode == event.xkey.keycode);
                
                b8 mods[MDFR_INDEX_COUNT] = {};
                mods[MDFR_HOLD_INDEX] = is_hold;
                
                if (event.xkey.state & ShiftMask) mods[MDFR_SHIFT_INDEX] = 1;
                if (event.xkey.state & ControlMask) mods[MDFR_CONTROL_INDEX] = 1;
                if (event.xkey.state & LockMask) mods[MDFR_CAPS_INDEX] = 1;
                if (event.xkey.state & Mod1Mask) mods[MDFR_ALT_INDEX] = 1;
                
                event.xkey.state &= ~(ControlMask);
                
                Status status;
                KeySym keysym = NoSymbol;
                u8 buff[32] = {};
                
                Xutf8LookupString(linuxvars.input_context, &event.xkey, (char*)buff, sizeof(buff) - 1, &keysym, &status);
                
                if (status == XBufferOverflow){
                    //TODO(inso): handle properly
                    Xutf8ResetIC(linuxvars.input_context);
                    XSetICFocus(linuxvars.input_context);
                    LOG("FIXME: XBufferOverflow from LookupString.\n");
                }
                
                // don't push modifiers
                if (keysym >= XK_Shift_L && keysym <= XK_Hyper_R){
                    break;
                }
                
                u32 key = utf8_to_u32_unchecked(buff);
                u32 key_no_caps = key;
                
                if (mods[MDFR_CAPS_INDEX] && status == XLookupBoth && event.xkey.keycode){
                    u8 buff_no_caps[32] = {};
                    event.xkey.state &= ~(LockMask);
                    
                    Xutf8LookupString(linuxvars.input_context, &event.xkey, (char*)buff_no_caps, sizeof(buff_no_caps) - 1, NULL, &status);
                    
                    if (status == XBufferOverflow){
                        //TODO(inso): handle properly
                        Xutf8ResetIC(linuxvars.input_context);
                        XSetICFocus(linuxvars.input_context);
                        LOG("FIXME: XBufferOverflow from LookupString.\n");
                    }
                    
                    if (*buff_no_caps){
                        key_no_caps = utf8_to_u32_unchecked(buff_no_caps);
                    }
                }
                
                if (key         == '\r') key         = '\n';
                if (key_no_caps == '\r') key_no_caps = '\n';
                
                if (keysym == XK_ISO_Left_Tab){
                    key = key_no_caps = '\t';
                    mods[MDFR_SHIFT_INDEX] = 1;
                }
                
                //Key_Code special_key = keycode_lookup_table[keysym];
                
                Key_Code special_key = 0;
                switch (keysym){
                    case XK_BackSpace:
                    {
                        special_key = key_back;
                    }break;
                    
                    case XK_Delete:
                    {
                        special_key = key_del;
                    }break;
                    
                    case XK_Up:
                    {
                        special_key = key_up;
                    }break;
                    
                    case XK_Down:
                    {
                        special_key = key_down;
                    }break;
                    
                    case XK_Left:
                    {
                        special_key = key_left;
                    }break;
                    
                    case XK_Right:
                    {
                        special_key = key_right;
                    }break;
                    
                    case XK_Insert:
                    {
                        special_key = key_insert;
                    }break;
                    
                    case XK_Home:
                    {
                        special_key = key_home;
                    }break;
                    
                    case XK_End:
                    {
                        special_key = key_end;
                    }break;
                    
                    case XK_Page_Up:
                    {
                        special_key = key_page_up;
                    }break;
                    
                    case XK_Page_Down:
                    {
                        special_key = key_page_down;
                    }break;
                    
                    case XK_Escape:
                    {
                        special_key = key_esc;
                    }break;
                    
                    case XK_F1:
                    {
                        special_key = key_f1;
                    }break;
                    
                    case XK_F2:
                    {
                        special_key = key_f2;
                    }break;
                    
                    case XK_F3:
                    {
                        special_key = key_f3;
                    }break;
                    
                    case XK_F4:
                    {
                        special_key = key_f4;
                    }break;
                    
                    case XK_F5:
                    {
                        special_key = key_f5;
                    }break;
                    
                    case XK_F6:
                    {
                        special_key = key_f6;
                    }break;
                    
                    case XK_F7:
                    {
                        special_key = key_f7;
                    }break;
                    
                    case XK_F8:
                    {
                        special_key = key_f8;
                    }break;
                    
                    case XK_F9:
                    {
                        special_key = key_f9;
                    }break;
                    
                    case XK_F10:
                    {
                        special_key = key_f10;
                    }break;
                    
                    case XK_F11:
                    {
                        special_key = key_f11;
                    }break;
                    
                    case XK_F12:
                    {
                        special_key = key_f12;
                    }break;
                    
                    case XK_F13:
                    {
                        special_key = key_f13;
                    }break;
                    
                    case XK_F14:
                    {
                        special_key = key_f14;
                    }break;
                    
                    case XK_F15:
                    {
                        special_key = key_f15;
                    }break;
                    
                    case XK_F16:
                    {
                        special_key = key_f16;
                    }break;
                }
                
                
                if (special_key){
                    LinuxPushKey(special_key, 0, 0, mods);
                } else if (key != 0){
                    LinuxPushKey(key, key, key_no_caps, mods);
                } else {
                    LinuxPushKey(0, 0, 0, mods);
                }
            }break;
            
            case KeyRelease: {
                should_step = true;
            }break;
            
            case MotionNotify: {
                should_step = true;
                linuxvars.input.mouse.x = event.xmotion.x;
                linuxvars.input.mouse.y = event.xmotion.y;
            }break;
            
            case ButtonPress: {
                should_step = true;
                switch(event.xbutton.button){
                    case Button1: {
                        linuxvars.input.mouse.press_l = 1;
                        linuxvars.input.mouse.l = 1;
                    } break;
                    case Button3: {
                        linuxvars.input.mouse.press_r = 1;
                        linuxvars.input.mouse.r = 1;
                    } break;
                    
                    //NOTE(inso): scroll up
                    case Button4: {
                        linuxvars.input.mouse.wheel = -100;
                    }break;
                    
                    //NOTE(inso): scroll down
                    case Button5: {
                        linuxvars.input.mouse.wheel =  100;
                    }break;
                }
            }break;
            
            case ButtonRelease: {
                should_step = true;
                switch(event.xbutton.button){
                    case Button1: {
                        linuxvars.input.mouse.release_l = 1;
                        linuxvars.input.mouse.l = 0;
                    } break;
                    case Button3: {
                        linuxvars.input.mouse.release_r = 1;
                        linuxvars.input.mouse.r = 0;
                    } break;
                }
            }break;
            
            case EnterNotify: {
                should_step = true;
                linuxvars.input.mouse.out_of_window = 0;
            }break;
            
            case LeaveNotify: {
                should_step = true;
                linuxvars.input.mouse.out_of_window = 1;
            }break;
            
            case FocusIn:
            case FocusOut: {
                should_step = true;
                linuxvars.input.mouse.l = 0;
                linuxvars.input.mouse.r = 0;
            }break;
            
            case ConfigureNotify: {
                should_step = true;
                i32 w = event.xconfigure.width;
                i32 h = event.xconfigure.height;
                
                if (w != target.width || h != target.height){
                    LinuxResizeTarget(w, h);
                }
            }break;
            
            case MappingNotify: {
                if (event.xmapping.request == MappingModifier || event.xmapping.request == MappingKeyboard){
                    XRefreshKeyboardMapping(&event.xmapping);
                    //LinuxKeycodeInit(linuxvars.XDisplay);
                }
            }break;
            
            case ClientMessage: {
                if ((Atom)event.xclient.data.l[0] == linuxvars.atom_WM_DELETE_WINDOW) {
                    should_step = true;
                    linuxvars.keep_running = 0;
                }
                else if ((Atom)event.xclient.data.l[0] == linuxvars.atom__NET_WM_PING) {
                    event.xclient.window = DefaultRootWindow(linuxvars.XDisplay);
                    XSendEvent(
                        linuxvars.XDisplay,
                        event.xclient.window,
                        False,
                        SubstructureRedirectMask | SubstructureNotifyMask,
                        &event);
                }
            }break;
            
            // NOTE(inso): Someone wants us to give them the clipboard data.
            case SelectionRequest: {
                XSelectionRequestEvent request = event.xselectionrequest;
                
                XSelectionEvent response = {};
                response.type = SelectionNotify;
                response.requestor = request.requestor;
                response.selection = request.selection;
                response.target = request.target;
                response.time = request.time;
                response.property = None;
                
                if (linuxvars.clipboard_outgoing.size &&
                    request.selection == linuxvars.atom_CLIPBOARD &&
                    request.property != None &&
                    request.display &&
                    request.requestor){
                    Atom atoms[] = {
                        XA_STRING,
                        linuxvars.atom_UTF8_STRING
                    };
                    
                    if (request.target == linuxvars.atom_TARGETS){
                        XChangeProperty(
                            request.display,
                            request.requestor,
                            request.property,
                            XA_ATOM,
                            32,
                            PropModeReplace,
                            (u8*)atoms,
                            ArrayCount(atoms));
                        
                        response.property = request.property;
                        
                    } else {
                        b32 found = false;
                        for(int i = 0; i < ArrayCount(atoms); ++i){
                            if (request.target == atoms[i]){
                                found = true;
                                break;
                            }
                        }
                        
                        if (found){
                            XChangeProperty(
                                request.display,
                                request.requestor,
                                request.property,
                                request.target,
                                8,
                                PropModeReplace,
                                (u8*)linuxvars.clipboard_outgoing.str,
                                linuxvars.clipboard_outgoing.size
                                );
                            
                            response.property = request.property;
                        }
                    }
                }
                
                XSendEvent(request.display, request.requestor, True, 0, (XEvent*)&response);
                
            }break;
            
            // NOTE(inso): Another program is now the clipboard owner.
            case SelectionClear: {
                if (event.xselectionclear.selection == linuxvars.atom_CLIPBOARD){
                    linuxvars.clipboard_outgoing.size = 0;
                }
            }break;
            
            // NOTE(inso): A program is giving us the clipboard data we asked for.
            case SelectionNotify: {
                XSelectionEvent* e = (XSelectionEvent*)&event;
                if (e->selection == linuxvars.atom_CLIPBOARD && e->target == linuxvars.atom_UTF8_STRING && e->property != None){
                    Atom type;
                    int fmt;
                    unsigned long nitems;
                    unsigned long bytes_left;
                    u8 *data;
                    
                    
#define PASTE_MAX_WORDS 0x20000000L
                    int result = XGetWindowProperty(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom_CLIPBOARD, 0L, PASTE_MAX_WORDS, False, linuxvars.atom_UTF8_STRING, &type, &fmt, &nitems, &bytes_left, &data);
                    
                    if (result == Success && fmt == 8){
                        LinuxStringDup(&linuxvars.clipboard_contents, data, nitems);
                        should_step = true;
                        linuxvars.new_clipboard = true;
                        XFree(data);
                        XDeleteProperty(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom_CLIPBOARD);
                    }
                }
            }break;
            
            case Expose:
            case VisibilityNotify: {
                should_step = true;
            }break;
            
            default: {
                if (event.type == linuxvars.xfixes_selection_event){
                    XFixesSelectionNotifyEvent* sne = (XFixesSelectionNotifyEvent*)&event;
                    if (sne->subtype == XFixesSelectionNotify && sne->owner != linuxvars.XWindow){
                        XConvertSelection(linuxvars.XDisplay, linuxvars.atom_CLIPBOARD, linuxvars.atom_UTF8_STRING, linuxvars.atom_CLIPBOARD, linuxvars.XWindow, CurrentTime);
                    }
                }
            }break;
        }
        
        prev_event = event;
    }
    
    if (should_step){
        system_schedule_step();
    }
}

#include "4ed_link_system_functions.cpp"
#include "4ed_shared_init_logic.cpp"

int
main(int argc, char **argv){
    //
    // System Linkage
    //
    
    link_system_code();
    
    //
    // Memory init
    //
    
    memset(&linuxvars, 0, sizeof(linuxvars));
    memset(&target, 0, sizeof(target));
    memset(&memory_vars, 0, sizeof(memory_vars));
    memset(&plat_settings, 0, sizeof(plat_settings));
    
    memset(&libraries, 0, sizeof(libraries));
    memset(&app, 0, sizeof(app));
    memset(&custom_api, 0, sizeof(custom_api));
    
    memory_init();
    
    //
    // HACK(allen): 
    // Previously zipped stuff is here, it should be zipped in the new pattern now.
    //
    
    init_shared_vars();
    
    //
    // Load Core Code
    //
    load_app_code();
    
    //
    // Read command line
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
    // X11 init
    //
    
    linuxvars.XDisplay = XOpenDisplay(0);
    if (!linuxvars.XDisplay){
        // NOTE(inso): probably not worth trying the popup in this case...
        LOG("Can't open display!\n");
        exit(1);
    }
    
#define LOAD_ATOM(x) linuxvars.atom_##x = XInternAtom(linuxvars.XDisplay, #x, False);
    
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
    
    linuxvars.dpi_x = linuxvars.dpi_y = LinuxGetXSettingsDPI(linuxvars.XDisplay, DefaultScreen(linuxvars.XDisplay));
    
    // fallback
    if (linuxvars.dpi_x == -1){
        int scr = DefaultScreen(linuxvars.XDisplay);
        
        int dw = DisplayWidth(linuxvars.XDisplay, scr);
        int dh = DisplayHeight(linuxvars.XDisplay, scr);
        
        int dw_mm = DisplayWidthMM(linuxvars.XDisplay, scr);
        int dh_mm = DisplayHeightMM(linuxvars.XDisplay, scr);
        
        linuxvars.dpi_x = dw_mm ? dw / (dw_mm / 25.4) : 96;
        linuxvars.dpi_y = dh_mm ? dh / (dh_mm / 25.4) : 96;
        
        LOGF("%dx%d - %dmmx%dmm DPI: %dx%d\n", dw, dh, dw_mm, dh_mm, linuxvars.dpi_x, linuxvars.dpi_y);
    }
    else{
        LOGF("DPI from XSETTINGS: %d\n", linuxvars.dpi_x);
    }
    
    int window_width, window_height;
    if (!LinuxX11WindowInit(argc, argv, &window_width, &window_height)){
        return 1;
    }
    
    int xfixes_version_unused, xfixes_err_unused;
    b32 xquery_extension_r = XQueryExtension(linuxvars.XDisplay, "XFIXES", &xfixes_version_unused, &linuxvars.xfixes_selection_event, &xfixes_err_unused);
    linuxvars.has_xfixes = (xquery_extension_r == True);
    
    if (linuxvars.has_xfixes){
        XFixesSelectSelectionInput(linuxvars.XDisplay, linuxvars.XWindow, linuxvars.atom_CLIPBOARD, XFixesSetSelectionOwnerNotifyMask);
    }
    else{
        LOG("Your X server doesn't support XFIXES, mention this fact if you report any clipboard-related issues.\n");
    }
    
    Init_Input_Result input_result = LinuxInputInit(linuxvars.XDisplay, linuxvars.XWindow);
    
    linuxvars.input_method = input_result.input_method;
    linuxvars.input_style = input_result.best_style;
    linuxvars.input_context = input_result.xic;
    
    //LinuxKeycodeInit(linuxvars.XDisplay);
    
    Cursor xcursors[APP_MOUSE_CURSOR_COUNT] = {
        None,
        None,
        XCreateFontCursor(linuxvars.XDisplay, XC_xterm),
        XCreateFontCursor(linuxvars.XDisplay, XC_sb_h_double_arrow),
        XCreateFontCursor(linuxvars.XDisplay, XC_sb_v_double_arrow)
    };
    
    {
        char data = 0;
        XColor c  = {};
        Pixmap p  = XCreateBitmapFromData(linuxvars.XDisplay, linuxvars.XWindow, &data, 1, 1);
        
        linuxvars.hidden_cursor = XCreatePixmapCursor(linuxvars.XDisplay, p, p, &c, &c, 0, 0);
        
        XFreePixmap(linuxvars.XDisplay, p);
    }
    
    
    //
    // Font System Init
    //
    
    Partition *scratch = &shared_vars.scratch;
    Temp_Memory temp = begin_temp_memory(scratch);
    Font_Setup_List font_setup = system_font_get_local_stubs(scratch);
    linux_get_loadable_fonts(scratch, &font_setup);
    system_font_init(&sysfunc.font, plat_settings.font_size, plat_settings.use_hinting, font_setup);
    end_temp_memory(temp);
    
    //
    // Epoll init
    //
    
    linuxvars.x11_fd        = ConnectionNumber(linuxvars.XDisplay);
    linuxvars.inotify_fd    = inotify_init1(IN_NONBLOCK);
    linuxvars.step_event_fd = eventfd(0, EFD_NONBLOCK);
    linuxvars.step_timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    
    linuxvars.epoll = epoll_create(16);
    
    {
        struct epoll_event e = {};
        e.events = EPOLLIN | EPOLLET;
        
        e.data.u64 = LINUX_4ED_EVENT_X11;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, linuxvars.x11_fd, &e);
        
        e.data.u64 = LINUX_4ED_EVENT_STEP;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, linuxvars.step_event_fd, &e);
        
        e.data.u64 = LINUX_4ED_EVENT_STEP_TIMER;
        epoll_ctl(linuxvars.epoll, EPOLL_CTL_ADD, linuxvars.step_timer_fd, &e);
    }
    
    //
    // App init
    //
    
    XAddConnectionWatch(linuxvars.XDisplay, &LinuxX11ConnectionWatch, NULL);
    
    char cwd[4096];
    u32 size = sysfunc.get_current_path(cwd, sizeof(cwd));
    if (size == 0 || size >= sizeof(cwd)){
        system_error_box("Could not get current directory at launch.");
    }
    String curdir = make_string(cwd, size);
    terminate_with_null(&curdir);
    replace_char(&curdir, '\\', '/');
    
    LOG("Initializing application variables\n");
    app.init(&sysfunc, &target, &memory_vars, linuxvars.clipboard_contents, curdir, custom_api);
    
    LinuxResizeTarget(window_width, window_height);
    
    //
    // Main loop
    //
    
    system_acquire_lock(FRAME_LOCK);
    
    system_schedule_step();
    
    linuxvars.keep_running = true;
    linuxvars.input.first_step = true;
    linuxvars.input.dt = (frame_useconds / 1000000.f);
    
    for (;;){
        if (XEventsQueued(linuxvars.XDisplay, QueuedAlready)){
            LinuxHandleX11Events();
        }
        
        system_release_lock(FRAME_LOCK);
        
        struct epoll_event events[16];
        int num_events = epoll_wait(linuxvars.epoll, events, ArrayCount(events), -1);
        
        system_acquire_lock(FRAME_LOCK);
        
        if (num_events == -1){
            if (errno != EINTR){
                LOG("epoll_wait\n");
            }
            continue;
        }
        
        b32 do_step = false;
        
        for (int i = 0; i < num_events; ++i){
            int fd   = events[i].data.u64 & UINT32_MAX;
            u64 type = events[i].data.u64 & ~fd;
            
            switch (type){
                case LINUX_4ED_EVENT_X11: {
                    LinuxHandleX11Events();
                } break;
                
                case LINUX_4ED_EVENT_X11_INTERNAL: {
                    XProcessInternalConnection(linuxvars.XDisplay, fd);
                } break;
                
                case LINUX_4ED_EVENT_STEP: {
                    u64 ev;
                    int ret;
                    do {
                        ret = read(linuxvars.step_event_fd, &ev, 8);
                    } while (ret != -1 || errno != EAGAIN);
                    do_step = true;
                } break;
                
                case LINUX_4ED_EVENT_STEP_TIMER: {
                    u64 count;
                    int ret;
                    do {
                        ret = read(linuxvars.step_timer_fd, &count, 8);
                    } while (ret != -1 || errno != EAGAIN);
                    do_step = true;
                } break;
                
                case LINUX_4ED_EVENT_CLI: {
                    system_schedule_step();
                } break;
            }
        }
        
        if (do_step){
            linuxvars.last_step = system_now_time();
            
            // NOTE(allen): Frame Clipboard Input
            if (linuxvars.input.first_step || !linuxvars.has_xfixes){
                XConvertSelection(linuxvars.XDisplay, linuxvars.atom_CLIPBOARD, linuxvars.atom_UTF8_STRING, linuxvars.atom_CLIPBOARD, linuxvars.XWindow, CurrentTime);
            }
            
            if (linuxvars.new_clipboard){
                linuxvars.input.clipboard = linuxvars.clipboard_contents;
                linuxvars.new_clipboard = 0;
            } else {
                linuxvars.input.clipboard = null_string;
            }
            
            Application_Step_Input frame_input = linuxvars.input;
            frame_input.trying_to_kill = !linuxvars.keep_running;
            
            // HACK(allen): THIS SHIT IS FUCKED (happens on mac too)
            b32 keep_running = linuxvars.keep_running;
            
            // NOTE(allen): Application Core Update
            target.buffer.pos = 0;
            Application_Step_Result result = {};
            if (app.step != 0){
                result = app.step(&sysfunc, &target, &memory_vars, &frame_input);
            }
            else{
                LOG("app.step == 0 -- skipping\n");
            }
            
            
            // NOTE(allen): Finish the Loop
            if (result.perform_kill){
                break;
            }
            else if (!keep_running && !linuxvars.keep_running){
                linuxvars.keep_running = true;
            }
            
            // NOTE(NAME): Switch to New Title
            if (result.has_new_title){
                XStoreName(linuxvars.XDisplay, linuxvars.XWindow, result.title_string);
            }
            
            // NOTE(allen): Switch to New Cursor
            if (result.mouse_cursor_type != linuxvars.cursor && !linuxvars.input.mouse.l){
                Cursor c = xcursors[result.mouse_cursor_type];
                if (!linuxvars.hide_cursor){
                    XDefineCursor(linuxvars.XDisplay, linuxvars.XWindow, c);
                }
                linuxvars.cursor = result.mouse_cursor_type;
            }
            
            // NOTE(allen): Render
            interpret_render_buffer(&target, &shared_vars.pixel_scratch);
            glXSwapBuffers(linuxvars.XDisplay, linuxvars.XWindow);
            
            // NOTE(allen): Toggle Full Screen
            if (linuxvars.do_toggle){
                linux_toggle_fullscreen();
                linuxvars.do_toggle = false;
            }
            
            // NOTE(allen): Schedule Another Step if Needed
            if (result.animating){
                system_schedule_step();
            }
            
            flush_thread_group(BACKGROUND_THREADS);
            
            // TODO(allen): CROSS REFERENCE WITH WIN32 "TIC898989"
            memset(&linuxvars.input.keys, 0, sizeof(linuxvars.input.keys));
            linuxvars.input.mouse.press_l = 0;
            linuxvars.input.mouse.release_l = 0;
            linuxvars.input.mouse.press_r = 0;
            linuxvars.input.mouse.release_r = 0;
            linuxvars.input.mouse.wheel = 0;
            
            linuxvars.input.first_step = 0;
        }
    }
    
    if (linuxvars.XDisplay){
        if (linuxvars.XWindow){
            XDestroyWindow(linuxvars.XDisplay, linuxvars.XWindow);
        }
        XCloseDisplay(linuxvars.XDisplay);
    }
    
    return(0);
}

#include "linux_4ed_file_track.cpp"

// BOTTOM
// vim: expandtab:ts=4:sts=4:sw=4

