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
#include <X11/extensions/XInput2.h>
#include <linux/input.h>
#include <time.h>

#include "4ed_internal.h"
#include "4ed_linux_keyboard.cpp"

#include <stdlib.h>

void*
LinuxGetMemory(i32 size){
    // TODO(allen): Implement without stdlib.h
    return (malloc(size));
}

void
LinuxFreeMemory(void *block){
    // TODO(allen): Implement without stdlib.h
    free(block);
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
            if (file_list->block){
                LinuxFreeMemory(file_list->block);
            }
            file_list->block = LinuxGetMemory(required_size);
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
                *cursor++ = 0;
                
                info_ptr->folder = 0;
                info_ptr->filename.str = cursor_start;
                info_ptr->filename.size = (i32)(cursor - cursor_start);
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

        printf("\nAttribs: %d %d %d %d %d\n",
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

internal void
InitializeXInput(Display *dpy, Window XWindow)
{
    int event, error;
    if(XQueryExtension(dpy, "XInputExtension", &XInput2OpCode, &event, &error))
    {
        int major = 2, minor = 0;
        if(XIQueryVersion(dpy, &major, &minor) != BadRequest)
        {
            printf("XInput initialized");
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
}

int
main(int ArgCount, char **Args)
{
    Display *XDisplay = XOpenDisplay(0);
    if(XDisplay && GLXSupportsModernContexts(XDisplay))
    {
        int XScreenCount = ScreenCount(XDisplay);
        for(int XScreenIndex = 0;
            XScreenIndex < XScreenCount;
            ++XScreenIndex)
        {                
            Screen *XScreen = ScreenOfDisplay(XDisplay, XScreenIndex);
                
            int WinWidth = WidthOfScreen(XScreen);
            int WinHeight = HeightOfScreen(XScreen);
                                
            glx_config_result Config = ChooseGLXConfig(XDisplay, XScreenIndex);
            if(Config.Found)
            {
                Colormap cmap;
                XSetWindowAttributes swa;
                swa.colormap = cmap = XCreateColormap(XDisplay,
                                                      RootWindow(XDisplay, Config.BestInfo.screen ), 
                                                      Config.BestInfo.visual, AllocNone);
                swa.background_pixmap = None;
                swa.border_pixel = 0;
                swa.event_mask = StructureNotifyMask;
 
                Window XWindow = XCreateWindow(XDisplay,
                                               RootWindow(XDisplay, Config.BestInfo.screen),
                                               0, 0, WinWidth, WinHeight,
                                               0, Config.BestInfo.depth, InputOutput, 
                                               Config.BestInfo.visual, 
                                               CWBorderPixel|CWColormap|CWEventMask, &swa );
                if(XWindow)
                {
                    XStoreName(XDisplay, XWindow, "4coder");
                    XMapWindow(XDisplay, XWindow);

                    InitializeXInput(XDisplay, XWindow);

                    b32 IsLegacy = false;
                    GLXContext GLContext =
                        InitializeOpenGLContext(XDisplay, XWindow, Config.BestConfig, IsLegacy);

                    XWindowAttributes WinAttribs;
                    if(XGetWindowAttributes(XDisplay, XWindow, &WinAttribs))
                    {
                        WinWidth = WinAttribs.width;
                        WinHeight = WinAttribs.height;
                    }
                    
                    XRaiseWindow(XDisplay, XWindow);
                    XSync(XDisplay, False);

                    for(;;)
                    {
                        while(XPending(XDisplay))
                        {
                            XEvent Event;                    
                            XNextEvent(XDisplay, &Event);

                            if((Event.xcookie.type == GenericEvent) &&
                               (Event.xcookie.extension == XInput2OpCode) &&
                               XGetEventData(XDisplay, &Event.xcookie))
                            {
                                switch(Event.xcookie.evtype)
                                {
                                    case XI_Motion:
                                    {
                                        Window root_return, child_return;
                                        int root_x_return, root_y_return;
                                        int MouseX, MouseY;
                                        unsigned int mask_return;
                                        XQueryPointer(XDisplay,
                                                      XWindow,
                                                      &root_return, &child_return,
                                                      &root_x_return, &root_y_return,
                                                      &MouseX, &MouseY,
                                                      &mask_return);
                                    } break;

                                    case XI_ButtonPress:
                                    case XI_ButtonRelease:
                                    {
                                        b32 Down = (Event.xcookie.evtype == XI_ButtonPress);
                                        XIDeviceEvent *DevEvent = (XIDeviceEvent *)Event.xcookie.data;
                                        int Button = DevEvent->detail;
                                    } break;

                                    case XI_KeyPress:
                                    case XI_KeyRelease:
                                    {
                                        b32 Down = (Event.xcookie.evtype == XI_KeyPress); 
                                        XIDeviceEvent *DevEvent = (XIDeviceEvent *)Event.xcookie.data;
                                        int VK = DevEvent->detail;
                                    } break;
                                }

                                XFreeEventData(XDisplay, &Event.xcookie);
                            }
                        }
                        
                        // Draw some stuff here?
                        
                        glXSwapBuffers(XDisplay, XWindow);                    
                    }
                }
            }
        }
    }
}

// BOTTOM

