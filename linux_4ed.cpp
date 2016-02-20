/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 14.11.2015
 *
 * Linux layer for project codename "4ed"
 *
 */

// TOP

#include "4ed_config.h"

#include "4ed_meta.h"

#define FCPP_FORBID_MALLOC

#include "4cpp_types.h"
#define FCPP_STRING_IMPLEMENTATION
#include "4coder_string.h"

#include "4ed_mem.cpp"
#include "4ed_math.cpp"

#include "4coder_custom.h"
#include "4ed_system.h"
#include "4ed_rendering.h"
#include "4ed.h"

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <xmmintrin.h>
#include <linux/fs.h>
//#include <X11/extensions/XInput2.h>
#include <X11/XKBlib.h>
#include <linux/input.h>
#include <time.h>
#include <dlfcn.h>
#include <unistd.h>

#include "4ed_internal.h"
#include "4ed_linux_keyboard.cpp"
#include "system_shared.h"

#include <stdlib.h>

struct Linux_Vars{
    Display *XDisplay;
    Window XWindow;
    Render_Target target;

    XIM input_method;
    XIMStyle input_style;
    XIC xic;
    Key_Codes key_codes;
    
    Clipboard_Contents clipboard_contents;
    
    void *app_code;
    void *custom;
    
    Plat_Settings settings;
    System_Functions *system;
    App_Functions app;
    Custom_API custom_api;
    b32 first;
    
#if FRED_INTERNAL
    Sys_Bubble internal_bubble;
#endif

    Font_Load_System fnt;
};

globalvar Linux_Vars linuxvars;
globalvar Application_Memory memory_vars;
globalvar Exchange exchange_vars;

internal
Sys_Get_Memory_Sig(system_get_memory_){
    // TODO(allen): Implement without stdlib.h
    void *result = 0;
    
    Assert(size != 0);
    
#if FRED_INTERNAL
    Sys_Bubble *bubble;
    
    result = malloc(size + sizeof(Sys_Bubble));
    bubble = (Sys_Bubble*)result;
    bubble->flags = MEM_BUBBLE_SYS_DEBUG;
    bubble->line_number = line_number;
    bubble->file_name = file_name;
    bubble->size = size;
    
    // TODO(allen): make Sys_Bubble list thread safe
    insert_bubble(&linuxvars.internal_bubble, bubble);
    result = bubble + 1;
    
#else
    result = malloc(size);
#endif
    
    return(result);
}

internal
Sys_Free_Memory_Sig(system_free_memory){
    // TODO(allen): Implement without stdlib.h
    
    if (block){
#if FRED_INTERNAL
        Sys_Bubble *bubble;
        
        bubble = (Sys_Bubble*)block;
        --bubble;
        Assert((bubble->flags & MEM_BUBBLE_DEBUG_MASK) == MEM_BUBBLE_SYS_DEBUG);

        // TODO(allen): make Sys_Bubble list thread safe
        remove_bubble(bubble);
        
        free(bubble);
#else
        free(block);
#endif
    }
}

#if (defined(_BSD_SOURCE) || defined(_SVID_SOURCE))
#define TimeBySt
#endif
#if (_POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700)
#define TimeBySt
#endif

#ifdef TimeBySt
#define nano_mtime_field st_mtim.tv_nsec
#undef TimeBySt
#else
#define nano_mtime_field st_mtimensec
#endif

Sys_File_Time_Stamp_Sig(system_file_time_stamp){
    struct stat info;
    i64 nanosecond_timestamp;
    u64 result;
    
    stat(filename, &info);
    nanosecond_timestamp = info.nano_mtime_field;
    if (nanosecond_timestamp != 0){
        result = (u64)(nanosecond_timestamp / 1000);
    }
    else{
        result = (u64)(info.st_mtime * 1000000);
    }

    return(result);
}

// TODO(allen): DOES THIS AGREE WITH THE FILESTAMP TIMES?
Sys_Time_Sig(system_time){
    struct timespec spec;
    u64 result;
    
    clock_gettime(CLOCK_MONOTONIC, &spec);
    result = (spec.tv_sec * 1000000) + (spec.tv_nsec / 1000);

    return(result);
}

Sys_Set_File_List_Sig(system_set_file_list){
    DIR *d;
    struct dirent *entry;
    char *fname, *cursor, *cursor_start;
    File_Info *info_ptr;
    i32 count, file_count, size, required_size;
    
    terminate_with_null(&directory);
    d = opendir(directory.str);
    if (d){
        count = 0;
        for (entry = readdir(d);
             entry != 0;
             entry = readdir(d)){
            fname = entry->d_name;
            ++file_count;            
            for (size = 0; fname[size]; ++size);
            count += size + 1;
        }
        closedir(d);

        required_size = count + file_count * sizeof(File_Info);
        if (file_list->block_size < required_size){
            system_free_memory(file_list->block);
            file_list->block = system_get_memory(required_size);
        }
        
        file_list->infos = (File_Info*)file_list->block;
        cursor = (char*)(file_list->infos + file_count);
        
        d = opendir(directory.str);
        if (d){
            info_ptr = file_list->infos;
            for (entry = readdir(d);
                 entry != 0;
                 entry = readdir(d), ++info_ptr){
                fname = entry->d_name;
                cursor_start = cursor;
                for (; *fname; ) *cursor++ = *fname++;

                // TODO(allen): detect file/folder status
                // (also make sure this even GETS folders!!!)
                info_ptr->folder = 0;
                info_ptr->filename.str = cursor_start;
                info_ptr->filename.size = (i32)(cursor - cursor_start);
                *cursor++ = 0;
                info_ptr->filename.memory_size = info_ptr->filename.size + 1;
            }
        }
        closedir(d);
    }
    closedir(d);
}

Sys_Post_Clipboard_Sig(system_post_clipboard){
    // TODO(allen): Implement
    AllowLocal(str);
}

Sys_CLI_Call_Sig(system_cli_call){
    // TODO(allen): Implement
    AllowLocal(path);
    AllowLocal(script_name);
    AllowLocal(cli_out);
}

Sys_CLI_Begin_Update_Sig(system_cli_begin_update){
    // TODO(allen): Implement
    AllowLocal(cli);
}

Sys_CLI_Update_Step_Sig(system_cli_update_step){
    // TODO(allen): Implement
    AllowLocal(cli);
    AllowLocal(dest);
    AllowLocal(max);
    AllowLocal(amount);
}

Sys_CLI_End_Update_Sig(system_cli_end_update){
    // TODO(allen): Implement
    AllowLocal(cli);
}

Sys_Post_Job_Sig(system_post_job){
    // TODO(allen): Implement
    AllowLocal(group_id);
    AllowLocal(job);
}

Sys_Cancel_Job_Sig(system_cancel_job){
    // TODO(allen): Implement
    AllowLocal(group_id);
    AllowLocal(job_id);
}

Sys_Acquire_Lock_Sig(system_acquire_lock){
    // TODO(allen): Implement
    AllowLocal(id);
}

Sys_Release_Lock_Sig(system_release_lock){
    // TODO(allen): Implement
    AllowLocal(id);
}

Sys_Grow_Thread_Memory_Sig(system_grow_thread_memory){
    void *old_data;
    i32 old_size, new_size;
    
    system_acquire_lock(CANCEL_LOCK0 + memory->id - 1);
    old_data = memory->data;
    old_size = memory->size;
    new_size = LargeRoundUp(memory->size*2, Kbytes(4));
    memory->data = system_get_memory(new_size);
    memory->size = new_size;
    if (old_data){
        memcpy(memory->data, old_data, old_size);
        system_free_memory(old_data);
    }
    system_release_lock(CANCEL_LOCK0 + memory->id - 1);
}

INTERNAL_Sys_Sentinel_Sig(internal_sentinel){
    Bubble *result;
#if FRED_INTERNAL
    result = &linuxvars.internal_bubble;
#else
    result = 0;
#endif
    return(result);
}

INTERNAL_Sys_Get_Thread_States_Sig(internal_get_thread_states){
    // TODO(allen): Implement
    AllowLocal(id);
    AllowLocal(running);
    AllowLocal(pending);
}

INTERNAL_Sys_Debug_Message_Sig(internal_debug_message){
    printf("%s", message);
}

DIRECTORY_HAS_FILE_SIG(system_directory_has_file){
    int result = 0;
    // TODO(allen): Implement
    AllowLocal(dir);
    AllowLocal(filename);
    return(result);
}

DIRECTORY_CD_SIG(system_directory_cd){
    int result = 0;
    // TODO(allen): Implement
    AllowLocal(dir);
    AllowLocal(rel_path);
    return(result);
}

internal
Sys_File_Can_Be_Made(system_file_can_be_made){
    // TODO(allen): Implement
    AllowLocal(filename);
    return(0);
}

internal
Sys_Load_File_Sig(system_load_file){
    Data result = {};
    // TODO(allen): Implement
    AllowLocal(filename);
    return(result);
}

internal
Sys_Save_File_Sig(system_save_file){
    b32 result = 0;
    // TODO(allen): Implement
    AllowLocal(filename);
    AllowLocal(data);
    AllowLocal(size);
    return(result);
}

// TODO(allen): Implement this.  Also where is this
// macro define? Let's try to organize these functions
// a little better now that they're starting to settle
// into their places.

internal
Font_Load_Sig(system_draw_font_load){
    return(0);
}

internal b32
LinuxLoadAppCode(){
    b32 result = 0;
    App_Get_Functions *get_funcs = 0;

    linuxvars.app_code = dlopen("./4ed_app.so", RTLD_LAZY);
    if (linuxvars.app_code){
        get_funcs = (App_Get_Functions*)
            dlsym(linuxvars.app_code, "app_get_functions");
    }
    
    if (get_funcs){
        result = 1;
        linuxvars.app = get_funcs();
    }
    
    return(result);
}

internal void
LinuxLoadSystemCode(){
    linuxvars.system->file_time_stamp = system_file_time_stamp;
    linuxvars.system->set_file_list = system_set_file_list;

    linuxvars.system->directory_has_file = system_directory_has_file;
    linuxvars.system->directory_cd = system_directory_cd;

    linuxvars.system->post_clipboard = system_post_clipboard;
    linuxvars.system->time = system_time;
    
    linuxvars.system->cli_call = system_cli_call;
    linuxvars.system->cli_begin_update = system_cli_begin_update;
    linuxvars.system->cli_update_step = system_cli_update_step;
    linuxvars.system->cli_end_update = system_cli_end_update;

    linuxvars.system->post_job = system_post_job;
    linuxvars.system->cancel_job = system_cancel_job;
    linuxvars.system->grow_thread_memory = system_grow_thread_memory;
    linuxvars.system->acquire_lock = system_acquire_lock;
    linuxvars.system->release_lock = system_release_lock;
    
    linuxvars.system->internal_sentinel = internal_sentinel;
    linuxvars.system->internal_get_thread_states = internal_get_thread_states;
    linuxvars.system->internal_debug_message = internal_debug_message;
}

#include "system_shared.cpp"
#include "4ed_rendering.cpp"

internal void
LinuxLoadRenderCode(){
    linuxvars.target.push_clip = draw_push_clip;
    linuxvars.target.pop_clip = draw_pop_clip;
    linuxvars.target.push_piece = draw_push_piece;
    
    linuxvars.target.font_set.font_info_load = draw_font_info_load;
    linuxvars.target.font_set.font_load = system_draw_font_load;
    linuxvars.target.font_set.release_font = draw_release_font;
}

internal void
LinuxRedrawTarget(){
    system_acquire_lock(RENDER_LOCK);
    launch_rendering(&linuxvars.target);
    system_release_lock(RENDER_LOCK);
    glFlush();
    glXSwapBuffers(linuxvars.XDisplay, linuxvars.XWindow);
}

internal void
LinuxResizeTarget(i32 width, i32 height){
    if (width > 0 && height > 0){
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width, height, 0, -1, 1);
        glScissor(0, 0, width, height);
        
        linuxvars.target.width = width;
        linuxvars.target.height = height;
    }
}

// NOTE(allen): Thanks to Casey for providing the linux OpenGL launcher.
static bool ctxErrorOccurred = false;
static int XInput2OpCode = 0;
internal int
ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    ctxErrorOccurred = true;
    return 0;
}

internal GLXContext
InitializeOpenGLContext(Display *XDisplay, Window XWindow, GLXFBConfig &bestFbc, b32 &IsLegacy)
{
    IsLegacy = false;
    
    typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    const char *glxExts = glXQueryExtensionsString(XDisplay, DefaultScreen(XDisplay));
    
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
        glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
 
    GLXContext ctx = 0;
    ctxErrorOccurred = false;
    int (*oldHandler)(Display*, XErrorEvent*) =
        XSetErrorHandler(&ctxErrorHandler);
    if (!glXCreateContextAttribsARB)
    {
        printf( "glXCreateContextAttribsARB() not found"
                " ... using old-style GLX context\n" );
        ctx = glXCreateNewContext( XDisplay, bestFbc, GLX_RGBA_TYPE, 0, True );
    } 
    else
    {
        int context_attribs[] =
        {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            None
        };

        printf("Attribs: %d %d %d %d %d\n",
               context_attribs[0],
               context_attribs[1],
               context_attribs[2],
               context_attribs[3],
               context_attribs[4]);
 
        printf( "Creating context\n" );
        ctx = glXCreateContextAttribsARB( XDisplay, bestFbc, 0,
                                          True, context_attribs );
 
        XSync( XDisplay, False );
        if ( !ctxErrorOccurred && ctx )
        {
            printf( "Created GL 4.3 context\n" );
        }
        else
        {
            ctxErrorOccurred = false;

            context_attribs[1] = 4;
            context_attribs[3] = 0;
 
            printf( "Creating context\n" );
            ctx = glXCreateContextAttribsARB( XDisplay, bestFbc, 0,
                                              True, context_attribs );
 
            XSync( XDisplay, False );

            if ( !ctxErrorOccurred && ctx )
            {
                printf( "Created GL 4.0 context\n" );
            }
            else
            {
                context_attribs[1] = 1;
                context_attribs[3] = 0;
 
                ctxErrorOccurred = false;
 
                printf( "Failed to create GL 4.0 context"
                        " ... using old-style GLX context\n" );
                ctx = glXCreateContextAttribsARB( XDisplay, bestFbc, 0, 
                                                  True, context_attribs );

                IsLegacy = true;
            }
        }
    }
 
    XSync( XDisplay, False );
    XSetErrorHandler( oldHandler );
 
    if ( ctxErrorOccurred || !ctx )
    {
        printf( "Failed to create an OpenGL context\n" );
        exit(1);
    }
 
    if ( ! glXIsDirect ( XDisplay, ctx ) )
    {
        printf( "Indirect GLX rendering context obtained\n" );
    }
    else
    {
        printf( "Direct GLX rendering context obtained\n" );
    }
    
    printf( "Making context current\n" );
    glXMakeCurrent( XDisplay, XWindow, ctx );
    
    GLint n;
    char *Vendor = (char *)glGetString(GL_VENDOR);
    char *Renderer = (char *)glGetString(GL_RENDERER);
    char *Version = (char *)glGetString(GL_VERSION);
    char *Extensions = (char *)glGetString(GL_EXTENSIONS);
    
    printf("GL_VENDOR: %s\n", Vendor);
    printf("GL_RENDERER: %s\n", Renderer);
    printf("GL_VERSION: %s\n", Version);
    printf("GL_EXTENSIONS: %s\n", Extensions);
    
    return(ctx);
}

internal b32
GLXSupportsModernContexts(Display *XDisplay)
{
    b32 Result = false;
    
    int GLXMajor, GLXMinor;

    char *XVendor = ServerVendor(XDisplay);
    printf("XWindows vendor: %s\n", XVendor);
    if(glXQueryVersion(XDisplay, &GLXMajor, &GLXMinor))
    {
        printf("GLX version %d.%d\n", GLXMajor, GLXMinor);
        if(((GLXMajor == 1 ) && (GLXMinor >= 3)) ||
           (GLXMajor > 1))
        {
            Result = true;
        }
    }

    return(Result);
}

typedef struct glx_config_result{
    b32 Found;
    GLXFBConfig BestConfig;
    XVisualInfo BestInfo;
} glx_config_result;

internal glx_config_result
ChooseGLXConfig(Display *XDisplay, int XScreenIndex)
{
    glx_config_result Result = {0};
    
    int DesiredAttributes[] =
    {
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
        //GLX_SAMPLE_BUFFERS  , 1,
        //GLX_SAMPLES         , 4,
        None
    };    

    {
        int ConfigCount;
        GLXFBConfig *Configs = glXChooseFBConfig(XDisplay,
                                                 XScreenIndex,
                                                 DesiredAttributes,
                                                 &ConfigCount);

#if 0
        int DiffValues[GLXValueCount];
#endif
        {for(int ConfigIndex = 0;
             ConfigIndex < ConfigCount;
             ++ConfigIndex)
        {
            GLXFBConfig &Config = Configs[ConfigIndex];
            XVisualInfo *VisualInfo = glXGetVisualFromFBConfig(XDisplay, Config);

#if 0
            printf("  Option %d:\n", ConfigIndex);
            printf("    Depth: %d\n", VisualInfo->depth);
            printf("    Bits per channel: %d\n", VisualInfo->bits_per_rgb);
            printf("    Mask: R%06x G%06x B%06x\n",
                   (uint32)VisualInfo->red_mask,
                   (uint32)VisualInfo->green_mask,
                   (uint32)VisualInfo->blue_mask);
            printf("    Class: %d\n", VisualInfo->c_class);
#endif
            
#if 0
            {for(int ValueIndex = 0;
                 ValueIndex < GLXValueCount;
                 ++ValueIndex)
            {
                glx_value_info &ValueInfo = GLXValues[ValueIndex];
                int Value;
                glXGetFBConfigAttrib(XDisplay, Config, ValueInfo.ID, &Value);
                if(DiffValues[ValueIndex] != Value)
                {
                    printf("    %s: %d\n", ValueInfo.Name, Value);
                    DiffValues[ValueIndex] = Value;
                }
            }}
#endif
            
            // TODO(casey): How the hell are you supposed to pick a config here??
            if(ConfigIndex == 0)
            {
                Result.Found = true;
                Result.BestConfig = Config;
                Result.BestInfo = *VisualInfo;
            }
                        
            XFree(VisualInfo);
        }}
                    
        XFree(Configs);
    }

    return(Result);
}

struct Init_Input_Result{
    XIM input_method;
    XIMStyle best_style;
    XIC xic;
};

internal Init_Input_Result
InitializeXInput(Display *dpy, Window XWindow)
{
#if 0
    int event, error;
    if(XQueryExtension(dpy, "XInputExtension", &XInput2OpCode, &event, &error))
    {
        int major = 2, minor = 0;
        if(XIQueryVersion(dpy, &major, &minor) != BadRequest)
        {
            printf("XInput initialized version %d.%d\n", major, minor);
        }
        else
        {
            printf("XI2 not available. Server supports %d.%d\n", major, minor);
        }
    }
    else
    {
        printf("X Input extension not available.\n");
    }

    /*
      TODO(casey): So, this is all one big clusterfuck I guess.

      The problem here is that you want to be able to get input
      from all possible devices that could be a mouse or keyboard
      (or gamepad, or whatever).  So you'd like to be able to
      register events for XIAllDevices, so that when a new
      input device is connected, you will start receiving
      input from it automatically, without having to periodically
      poll XIQueryDevice to see if a new device has appeared.

      UNFORTUNATELY, this is not actually possible in Linux because
      there was a bug in Xorg (as of early 2013, it is still not
      fixed in most distributions people are using, AFAICT) which
      makes the XServer return an error if you actually try to
      do this :(

      But EVENTUALLY, if that shit gets fixed, then that is
      the way this should work.
    */

#if 0
    int DeviceCount;
    XIDeviceInfo *DeviceInfo = XIQueryDevice(dpy, XIAllDevices, &DeviceCount);
    {for(int32x DeviceIndex = 0;
         DeviceIndex < DeviceCount;
         ++DeviceIndex)
    {
        XIDeviceInfo *Device = DeviceInfo + DeviceIndex;
        printf("Device %d: %s\n", Device->deviceid, Device->name);
    }}
    XIFreeDeviceInfo(DeviceInfo);
#endif
    
    XIEventMask Mask = {0};
    Mask.deviceid = XIAllDevices;
    Mask.mask_len = XIMaskLen(XI_RawMotion);
    size_t MaskSize = Mask.mask_len * sizeof(char unsigned);
    Mask.mask = (char unsigned *)alloca(MaskSize);
    memset(Mask.mask, 0, MaskSize);
    if(Mask.mask)
    {
        XISetMask(Mask.mask, XI_ButtonPress);
        XISetMask(Mask.mask, XI_ButtonRelease);
        XISetMask(Mask.mask, XI_KeyPress);
        XISetMask(Mask.mask, XI_KeyRelease);
        XISetMask(Mask.mask, XI_Motion);
        XISetMask(Mask.mask, XI_DeviceChanged);
        XISetMask(Mask.mask, XI_Enter);
        XISetMask(Mask.mask, XI_Leave);
        XISetMask(Mask.mask, XI_FocusIn);
        XISetMask(Mask.mask, XI_FocusOut);
        XISetMask(Mask.mask, XI_HierarchyChanged);
        XISetMask(Mask.mask, XI_PropertyEvent);
        XISelectEvents(dpy, XWindow, &Mask, 1);
        XSync(dpy, False);

        Mask.deviceid = XIAllMasterDevices;
        memset(Mask.mask, 0, MaskSize);
        XISetMask(Mask.mask, XI_RawKeyPress);
        XISetMask(Mask.mask, XI_RawKeyRelease);
        XISetMask(Mask.mask, XI_RawButtonPress);
        XISetMask(Mask.mask, XI_RawButtonRelease);
        XISetMask(Mask.mask, XI_RawMotion);
        XISelectEvents(dpy, DefaultRootWindow(dpy), &Mask, 1);
    }
#endif
    
    // NOTE(allen): Annnndddd... here goes some guess work of my own.
    Init_Input_Result result = {};
    XIMStyle style;
    XIMStyles *styles = 0;
    i32 i, count;
    
    XSetLocaleModifiers("");
	printf("supported locales: %d\n", XSupportsLocale());
    
    XSelectInput(linuxvars.XDisplay, linuxvars.XWindow, ExposureMask | KeyPressMask);
    
    result.input_method = XOpenIM(dpy, 0, 0, 0);
    if (result.input_method){
        if (!XGetIMValues(result.input_method, XNQueryInputStyle, &styles, NULL) &&
            styles){
            result.best_style = 0;
            count = styles->count_styles;
            for (i = 0; i < count; ++i){
                style = styles->supported_styles[i];
                if (style == (XIMPreeditNothing | XIMStatusNothing)){
                    result.best_style = style;
                    break;
                }
            }
            
            if (result.best_style){
                XFree(styles);

                result.xic =
                    XCreateIC(result.input_method, XNInputStyle, result.best_style,
                              XNClientWindow, XWindow,
                              XNFocusWindow, XWindow,
                              0, 0,
                              NULL);
            }
            else{
                result = {};
                printf("Could not get minimum required input style");
            }
        }
    }
    else{
        result = {};
        printf("Could not open X Input Method\n");
    }

    return(result);
}

int
main(int argc, char **argv)
{
    i32 COUNTER = 0;
    linuxvars = {};
    exchange_vars = {};
    
#if FRED_INTERNAL
    linuxvars.internal_bubble.next = &linuxvars.internal_bubble;
    linuxvars.internal_bubble.prev = &linuxvars.internal_bubble;
    linuxvars.internal_bubble.flags = MEM_BUBBLE_SYS_DEBUG;
#endif
    
    if (!LinuxLoadAppCode()){
        // TODO(allen): Failed to load app code, serious problem.
        return 99;
    }
    
    System_Functions system_;
    System_Functions *system = &system_;
    linuxvars.system = system;
    LinuxLoadSystemCode();
    
	memory_vars.vars_memory_size = Mbytes(2);
    memory_vars.vars_memory = system_get_memory(memory_vars.vars_memory_size);
    memory_vars.target_memory_size = Mbytes(512);
    memory_vars.target_memory = system_get_memory(memory_vars.target_memory_size);
    
    String current_directory;
    i32 curdir_req, curdir_size;
    char *curdir_mem;
    
    curdir_req = (1 << 9);
    curdir_mem = (char*)system_get_memory(curdir_req);
    for (; getcwd(curdir_mem, curdir_req) == 0 && curdir_req < (1 << 13);){
        system_free_memory(curdir_mem);
        curdir_req *= 4;
        curdir_mem = (char*)system_get_memory(curdir_req);
    }
    
    if (curdir_req >= (1 << 13)){
        // TODO(allen): bullshit string APIs makin' me pissed
        return 57;
    }
   
    for (curdir_size = 0; curdir_mem[curdir_size]; ++curdir_size);
    
    current_directory = make_string(curdir_mem, curdir_size, curdir_req);
    
    Command_Line_Parameters clparams;
    clparams.argv = argv;
    clparams.argc = argc;
    
    char **files;
    i32 *file_count;
    i32 output_size;

    output_size =
        linuxvars.app.read_command_line(system,
                                        &memory_vars,
                                        current_directory,
                                        &linuxvars.settings,
                                        &files, &file_count,
                                        clparams);
    
    if (output_size > 0){
        // TODO(allen): crt free version
        printf("%.*s", output_size, (char*)memory_vars.target_memory);
    }
    if (output_size != 0) return 0;

    sysshared_filter_real_files(files, file_count);
    
    keycode_init(&linuxvars.key_codes);

#ifdef FRED_SUPER
    char *custom_file_default = "4coder_custom.so";
    char *custom_file;
    if (linuxvars.settings.custom_dll) custom_file = linuxvars.settings.custom_dll;
    else custom_file = custom_file_default;
    
    linuxvars.custom = dlopen(custom_file, RTLD_LAZY);
    if (!linuxvars.custom && custom_file != custom_file_default){
        if (!linuxvars.settings.custom_dll_is_strict){
            linuxvars.custom = dlopen(custom_file_default, RTLD_LAZY);
        }
    }
    
    if (linuxvars.custom){
        linuxvars.custom_api.get_bindings = (Get_Binding_Data_Function*)
            dlsym(linuxvars.custom, "get_bindings");
    }
#endif
    
    // TODO(allen): Setup background threads and locks
    
    LinuxLoadRenderCode();
    linuxvars.target.max = Mbytes(1);
    linuxvars.target.push_buffer = (byte*)system_get_memory(linuxvars.target.max);
    
    File_Slot file_slots[32];
    sysshared_init_file_exchange(&exchange_vars, file_slots, ArrayCount(file_slots), 0);
    
    Font_Load_Parameters params[8];
    sysshared_init_font_params(&linuxvars.fnt, params, ArrayCount(params));
    
    linuxvars.app.init(linuxvars.system, &linuxvars.target,
                       &memory_vars, &exchange_vars, &linuxvars.key_codes,
                       linuxvars.clipboard_contents, current_directory,
                       linuxvars.custom_api);
    
    // NOTE(allen): Here begins the linux screen setup stuff.
    // Behold the true nature of this wonderful OS:
    // (thanks again to Casey for providing this stuff)
    Colormap cmap;
    XSetWindowAttributes swa;
    int WinWidth, WinHeight;
    b32 window_setup_success = 0;

    WinWidth = 800;
    WinHeight = 600;
    
    i32 xkb_ev_code = 0, xkb_err_code = 0;
    
    linuxvars.XDisplay = XkbOpenDisplay(0, &xkb_ev_code, &xkb_err_code, 0, 0, 0);
    if(linuxvars.XDisplay && GLXSupportsModernContexts(linuxvars.XDisplay))
    {
        int XScreenCount = ScreenCount(linuxvars.XDisplay);
        for(int XScreenIndex = 0;
            XScreenIndex < XScreenCount;
            ++XScreenIndex)
        {
            Screen *XScreen = ScreenOfDisplay(linuxvars.XDisplay, XScreenIndex);

            i32 ScrnWidth, ScrnHeight;
            ScrnWidth = WidthOfScreen(XScreen);
            ScrnHeight = HeightOfScreen(XScreen);
            
            if (ScrnWidth + 50 < WinWidth) WinWidth = ScrnWidth + 50;
            if (ScrnHeight + 50 < WinHeight) WinHeight = ScrnHeight + 50;
            
            glx_config_result Config = ChooseGLXConfig(linuxvars.XDisplay, XScreenIndex);
            if(Config.Found)
            {
                swa.colormap = cmap = XCreateColormap(linuxvars.XDisplay,
                                                      RootWindow(linuxvars.XDisplay, Config.BestInfo.screen ), 
                                                      Config.BestInfo.visual, AllocNone);
                swa.background_pixmap = None;
                swa.border_pixel = 0;
                swa.event_mask = StructureNotifyMask;
 
                linuxvars.XWindow =
                    XCreateWindow(linuxvars.XDisplay,
                                  RootWindow(linuxvars.XDisplay, Config.BestInfo.screen),
                                  0, 0, WinWidth, WinHeight,
                                  0, Config.BestInfo.depth, InputOutput, 
                                  Config.BestInfo.visual, 
                                  CWBorderPixel|CWColormap|CWEventMask, &swa );
                
                if(linuxvars.XWindow)
                {
                    XStoreName(linuxvars.XDisplay, linuxvars.XWindow, "4coder-window");
                    XMapWindow(linuxvars.XDisplay, linuxvars.XWindow);

                    Init_Input_Result input_result = 
                        InitializeXInput(linuxvars.XDisplay, linuxvars.XWindow);

                    linuxvars.input_method = input_result.input_method;
                    linuxvars.input_style = input_result.best_style;
                    linuxvars.xic = input_result.xic;

                    b32 IsLegacy = false;
                    GLXContext GLContext =
                        InitializeOpenGLContext(linuxvars.XDisplay, linuxvars.XWindow, Config.BestConfig, IsLegacy);

                    XWindowAttributes WinAttribs;
                    if(XGetWindowAttributes(linuxvars.XDisplay, linuxvars.XWindow, &WinAttribs))
                    {
                        WinWidth = WinAttribs.width;
                        WinHeight = WinAttribs.height;
                    }
                    
                    XRaiseWindow(linuxvars.XDisplay, linuxvars.XWindow);
                    XSync(linuxvars.XDisplay, False);

                    window_setup_success = 1;
                }
            }
        }
    }
    
    XSetICFocus(linuxvars.xic);
    
    if (window_setup_success){
        LinuxResizeTarget(WinWidth, WinHeight);
        
        for(;;)
        {
            while(XPending(linuxvars.XDisplay))
            {
                XEvent Event;            
                XNextEvent(linuxvars.XDisplay, &Event);
                                
                if (XFilterEvent(&Event, None) == True){
                    continue;
                }
                
                switch (Event.type){
                case KeyPress:
                {
                    KeySym ks = NoSymbol;
                    XkbLookupKeySym(linuxvars.XDisplay, Event.xkey.keycode, Event.xkey.state, NULL, &ks);
                    printf("keysym: %s\n", XKeysymToString(ks));
			
                    Status st;
                    char buff[256];
                    memset(buff, 0, sizeof(buff));
                    XmbLookupString(linuxvars.xic, &Event.xkey, buff, sizeof(buff), &ks, &st);
                    printf("IC status: %d, code: %u, sym: %s, str: %s\n", st, Event.xkey.keycode, XKeysymToString(ks), buff);
                }break;
                
                }
            }
            
            b32 redraw = 1;
            
            Key_Input_Data input_data;
            Mouse_State mouse;
            Application_Step_Result result;
            
            input_data = {};
            mouse = {};
            
            result.mouse_cursor_type = APP_MOUSE_CURSOR_DEFAULT;
            result.redraw = redraw;
            result.lctrl_lalt_is_altgr = 0;
            
            linuxvars.app.step(linuxvars.system,
                               &linuxvars.key_codes,
                               &input_data,
                               &mouse,
                               &linuxvars.target,
                               &memory_vars,
                               &exchange_vars,
                               linuxvars.clipboard_contents,
                               1, linuxvars.first, redraw,
                               &result);
            
            if (result.redraw){
                LinuxRedrawTarget();
            }
                        
            glBegin(GL_QUADS);
            {
                glVertex2f(.5f, .5f);
                glVertex2f(1.f, .5f);
                glVertex2f(1.f, 1.f);
                glVertex2f(.5f, 1.f);
                            
                glVertex2f(0.f, 0.f);
                glVertex2f(.2f, 0.f);
                glVertex2f(.2f, .2f);
                glVertex2f(0.f, .2f);
            }
            glEnd();
            glXSwapBuffers(linuxvars.XDisplay, linuxvars.XWindow);
        }
    }
}

// BOTTOM

