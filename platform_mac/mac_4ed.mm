/* Mac Objective-C layer for 4coder */

#include "4coder_base_types.h"
#include "4coder_version.h"
#include "4coder_events.h"

#include "4coder_table.h"

// NOTE(allen): This is a very unfortunate hack, but hopefully there will never be a need to use the Marker
// type in the platform layer. If that changes then instead change the name of Marker and make a transition
// macro that is only included in custom code.
#define Marker Marker__SAVE_THIS_IDENTIFIER
#include "4coder_types.h"
#undef Marker

#include "4coder_default_colors.h"

#include "4coder_system_types.h"
#define STATIC_LINK_API
#include "generated/system_api.h"

#include "4ed_font_interface.h"
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

#include "4ed_search_list.cpp"

#include "mac_objective_c_to_cpp_links.h"

#undef function
#undef internal
#undef global
#undef external
#import <Cocoa/Cocoa.h>

#include <libproc.h> // NOTE(yuval): Used for proc_pidpath
#include <mach/mach_time.h> // NOTE(yuval): Used for mach_absolute_time, mach_timebase_info, mach_timebase_info_data_t

#define GL_GLEXT_LEGACY
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#include <dirent.h> // NOTE(yuval): Used for opendir, readdir
#include <dlfcn.h> // NOTE(yuval): Used for dlopen, dlclose, dlsym
#include <errno.h> // NOTE(yuval): Used for errno
#include <fcntl.h> // NOTE(yuval): Used for open
#include <pthread.h> // NOTE(yuval): Used for threads, mutexes, cvs
#include <unistd.h> // NOTE(yuval): Used for getcwd, read, write, getpid
#include <sys/mman.h> // NOTE(yuval): Used for mmap, munmap, mprotect
#include <sys/stat.h> // NOTE(yuval): Used for stat
#include <sys/types.h> // NOTE(yuval): Used for struct stat, pid_t

#include <stdlib.h> // NOTE(yuval): Used for free

#define function static
#define internal static
#define global static
#define external extern "C"

// NOTE(yuval): This is a hack to fix the CALL_CONVENTION not being defined problem in 4coder_base_types.h
#define CALL_CONVENTION

struct Control_Keys{
    b8 l_ctrl;
    b8 r_ctrl;
    b8 l_alt;
    b8 r_alt;
};

struct Mac_Input_Chunk_Transient{
    Input_List event_list;
    b8 mouse_l_press;
    b8 mouse_l_release;
    b8 mouse_r_press;
    b8 mouse_r_release;
    b8 out_of_window;
    i8 mouse_wheel;
    b8 trying_to_kill;
};

struct Mac_Input_Chunk_Persistent{
    Vec2_i32 mouse;
    Control_Keys controls;
    Input_Modifier_Set_Fixed modifiers;
    b8 mouse_l;
    b8 mouse_r;
};

struct Mac_Input_Chunk{
    Mac_Input_Chunk_Transient trans;
    Mac_Input_Chunk_Persistent pers;
};

////////////////////////////////

#define SLASH '/'
#define DLL "so"

#include "4coder_hash_functions.cpp"
#include "4coder_system_allocator.cpp"
#include "4coder_malloc_allocator.cpp"
#include "4coder_codepoint_map.cpp"

#include "4ed_mem.cpp"
#include "4ed_font_set.cpp"

////////////////////////////////

@interface AppDelegate : NSObject<NSApplicationDelegate, NSWindowDelegate>
@end

@interface OpenGLView : NSOpenGLView
- (void)init_opengl;
- (void)requestDisplay;
@end

////////////////////////////////

////////////////////////////////

typedef i32 Mac_Object_Kind;
enum{
    MacObjectKind_ERROR = 0,
    MacObjectKind_Timer = 1,
    MacObjectKind_Thread = 2,
    MacObjectKind_Mutex = 3,
    MacObjectKind_CV = 4,
};

struct Mac_Object{
    Node node;
    Mac_Object_Kind kind;
    
    union{
        NSTimer* timer;
        
        struct{
            pthread_t thread;
            Thread_Function *proc;
            void *ptr;
        } thread;
        
        pthread_mutex_t mutex;
        pthread_cond_t cv;
    };
};

struct Mac_Vars {
    b32 gl_is_initialized;
    
    Thread_Context *tctx;
    
    Arena* frame_arena;
    Mac_Input_Chunk input_chunk;
    
    b8 full_screen;
    b8 do_toggle;
    
    i32 cursor_show;
    i32 prev_cursor_show;
    
    String_Const_u8 binary_path;
    
    String_Const_u8 clipboard_contents;
    
    NSWindow* window;
    OpenGLView* view;
    f32 screen_scale_factor;
    
    mach_timebase_info_data_t timebase_info;
    
    Node free_mac_objects;
    Node timer_objects;
    
    pthread_mutex_t thread_launch_mutex;
    pthread_cond_t thread_launch_cv;
    b32 waiting_for_launch;
    
    System_Mutex global_frame_mutex;
    
    Log_Function *log_string;
};

////////////////////////////////

global Mac_Vars mac_vars;
global Render_Target target;

////////////////////////////////

function Mac_Object*
mac_alloc_object(Mac_Object_Kind kind){
    Mac_Object *result = 0;
    
    if (mac_vars.free_mac_objects.next != &mac_vars.free_mac_objects){
        result = CastFromMember(Mac_Object, node, mac_vars.free_mac_objects.next);
    }
    
    if (!result){
        i32 count = 512;
        Mac_Object *objects = (Mac_Object*)system_memory_allocate(count * sizeof(Mac_Object), file_name_line_number_lit_u8);
        
        // NOTE(yuval): Link the first node of the dll to the sentinel
        objects[0].node.prev = &mac_vars.free_mac_objects;
        mac_vars.free_mac_objects.next = &objects[0].node;
        
        // NOTE(yuval): Link all dll nodes to each other
        for (i32 chain_index = 1; chain_index < count; chain_index += 1){
            objects[chain_index - 1].node.next = &objects[chain_index].node;
            objects[chain_index].node.prev = &objects[chain_index - 1].node;
        }
        
        // NOTE(yuval): Link the last node of the dll to the sentinel
        objects[count - 1].node.next = &mac_vars.free_mac_objects;
        mac_vars.free_mac_objects.prev = &objects[count - 1].node;
        
        result = CastFromMember(Mac_Object, node, mac_vars.free_mac_objects.next);
    }
    
    Assert(result);
    dll_remove(&result->node);
    block_zero_struct(result);
    result->kind = kind;
    
    return(result);
}

function void
mac_free_object(Mac_Object *object){
    if (object->node.next != 0){
        dll_remove(&object->node);
    }
    
    dll_insert(&mac_vars.free_mac_objects, &object->node);
}

function inline Plat_Handle
mac_to_plat_handle(Mac_Object *object){
    Plat_Handle result = *(Plat_Handle*)(&object);
    return(result);
}

function inline Mac_Object*
mac_to_object(Plat_Handle handle){
    Mac_Object *result = *(Mac_Object**)(&handle);
    return(result);
}

////////////////////////////////

#include "4ed_font_provider_freetype.h"
#include "4ed_font_provider_freetype.h"

#include "opengl/4ed_opengl_render.cpp"

#import "mac_4ed_functions.mm"

////////////////////////////////

function void
mac_error_box(char *msg, b32 shutdown = true){
    NSAlert *alert = [[[NSAlert alloc] init] autorelease];
    
    NSString *title_string = @"Error";
    NSString *message_string = [NSString stringWithUTF8String:msg];
    [alert setMessageText:title_string];
    [alert setInformativeText:message_string];
    
    [alert runModal];
    
    if (shutdown){
        exit(1);
    }
}

function b32
mac_file_can_be_made(u8* filename){
    b32 result = access((char*)filename, W_OK) == 0;
    return(result);
}

////////////////////////////////

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(id)sender{
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender{
    return YES;
}

- (void)applicationWillTerminate:(NSNotification*)notification{
}
@end

@implementation OpenGLView
- (id)init{
    self = [super init];
    if (self == nil){
        return nil;
    }
    
    [self init_opengl];
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void)prepareOpenGL
{
    [super prepareOpenGL];
    
    [[self openGLContext] makeCurrentContext];
    
    // NOTE(yuval): Setup vsync
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (void)awakeFromNib
{
    [self init_opengl];
}

- (void)reshape{
    [super reshape];
    
    NSRect bounds = [self bounds];
    // mac_resize(rect.size.width, rect.size.height);
}

- (void)drawRect:(NSRect)bounds{
    // [self getFrame];
    printf("Draw Rect!\n");
}

- (BOOL)windowShouldClose:(NSWindow*)sender{
    // osx_try_to_close();
    return(NO);
}

- (BOOL)acceptsFirstResponder{
    return YES;
}

- (BOOL)becomeFirstResponder{
    return YES;
}

- (BOOL)resignFirstResponder{
    return YES;
}

- (void)keyDown:(NSEvent *)event{
    [self requestDisplay];
}

- (void)mouseMoved:(NSEvent*)event{
    [self requestDisplay];
}

- (void)mouseDown:(NSEvent*)event{
    [self requestDisplay];
}

- (void)init_opengl{
    if (mac_vars.gl_is_initialized){
        return;
    }
    
    [self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    
    // NOTE(yuval): Setup OpenGL
    NSOpenGLPixelFormatAttribute opengl_attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADepthSize, 24,
        // NSOpenGLPFASampleBuffers, 1,
        // NSOpenGLPFASamples, 16,
        0
    };
    
    NSOpenGLPixelFormat *pixel_format = [[NSOpenGLPixelFormat alloc] initWithAttributes:opengl_attrs];
    if (pixel_format == nil){
        fprintf(stderr, "Error creating OpenGLPixelFormat\n");
        exit(1);
    }
    
    NSOpenGLContext *context = [[NSOpenGLContext alloc] initWithFormat:pixel_format shareContext:nil];
    
    [self setPixelFormat:pixel_format];
    [self setOpenGLContext:context];
    
    [context makeCurrentContext];
    
    [pixel_format release];
    
    mac_vars.gl_is_initialized = true;
}

- (void)requestDisplay{
    printf("Display Requested\n");
    
    [self setNeedsDisplayInRect:[mac_vars.window frame]];
}
@end

////////////////////////////////

int
main(int arg_count, char **args){
    @autoreleasepool{
        // NOTE(yuval): Create NSApplication & Delegate
        NSApplication *ns_app = [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        AppDelegate *app_delegate = [[AppDelegate alloc] init];
        [ns_app setDelegate:app_delegate];
        
        pthread_mutex_init(&memory_tracker_mutex, 0);
        
        // NOTE(yuval): Context Setup
        Thread_Context _tctx = {};
        thread_ctx_init(&_tctx, ThreadKind_Main,
                        get_base_allocator_system(),
                        get_base_allocator_system());
        
        block_zero_struct(&mac_vars);
        mac_vars.tctx = &_tctx;
        
        API_VTable_system system_vtable = {};
        system_api_fill_vtable(&system_vtable);
        
        API_VTable_graphics graphics_vtable = {};
        graphics_api_fill_vtable(&graphics_vtable);
        
        API_VTable_font font_vtable = {};
        font_api_fill_vtable(&font_vtable);
        
        // NOTE(yuval): Memory
        mac_vars.frame_arena = reserve_arena(mac_vars.tctx);
        target.arena = make_arena_system(KB(256));
        
        mac_vars.cursor_show = MouseCursorShow_Always;
        mac_vars.prev_cursor_show = MouseCursorShow_Always;
        
        dll_init_sentinel(&mac_vars.free_mac_objects);
        dll_init_sentinel(&mac_vars.timer_objects);
        
        pthread_mutex_init(&mac_vars.thread_launch_mutex, 0);
        pthread_cond_init(&mac_vars.thread_launch_cv, 0);
        
        // NOTE(yuval): Screen scale factor calculation
        {
            NSScreen* screen = [NSScreen mainScreen];
            NSDictionary* desc = [screen deviceDescription];
            NSSize size = [[desc valueForKey:NSDeviceResolution] sizeValue];
            f32 max_dpi = Max(size.width, size.height);
            mac_vars.screen_scale_factor = (max_dpi / 72.0f);
        }
        
        // NOTE(yuval): Load core
        System_Library core_library = {};
        App_Functions app = {};
        {
            App_Get_Functions *get_funcs = 0;
            Scratch_Block scratch(mac_vars.tctx, Scratch_Share);
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
                    mac_error_box(msg);
                }
            }
            else{
                char msg[] = "Could not load '4ed_app.so'. This file should be in the same directory as the main '4ed' executable.";
                mac_error_box(msg);
            }
        }
        
        // NOTE(yuval): Send api vtables to core
        app.load_vtables(&system_vtable, &font_vtable, &graphics_vtable);
        mac_vars.log_string = app.get_logger();
        
        // NOTE(yuval): Init & command line parameters
        Plat_Settings plat_settings = {};
        void *base_ptr = 0;
        {
            Scratch_Block scratch(mac_vars.tctx, Scratch_Share);
            String_Const_u8 curdir = system_get_path(scratch, SystemPath_CurrentDirectory);
            curdir = string_mod_replace_character(curdir, '\\', '/');
            char **files = 0;
            i32 *file_count = 0;
            base_ptr = app.read_command_line(mac_vars.tctx, curdir, &plat_settings, &files, &file_count, arg_count, args);
            {
                i32 end = *file_count;
                i32 i = 0, j = 0;
                for (; i < end; ++i){
                    if (mac_file_can_be_made((u8*)files[i])){
                        files[j] = files[i];
                        ++j;
                    }
                }
                *file_count = j;
            }
        }
        
        // NOTE(yuval): Load custom layer
        System_Library custom_library = {};
        Custom_API custom = {};
        {
            char custom_not_found_msg[] = "Did not find a library for the custom layer.";
            char custom_fail_version_msg[] = "Failed to load custom code due to missing version information or a version mismatch.  Try rebuilding with buildsuper.";
            char custom_fail_init_apis[] = "Failed to load custom code due to missing 'init_apis' symbol.  Try rebuilding with buildsuper";
            
            Scratch_Block scratch(mac_vars.tctx, Scratch_Share);
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
                mac_error_box(custom_not_found_msg);
            }
            custom.get_version = (_Get_Version_Type*)system_get_proc(custom_library, "get_version");
            if (custom.get_version == 0 || custom.get_version(MAJOR, MINOR, PATCH) == 0){
                mac_error_box(custom_fail_version_msg);
            }
            custom.init_apis = (_Init_APIs_Type*)system_get_proc(custom_library, "init_apis");
            if (custom.init_apis == 0){
                mac_error_box(custom_fail_init_apis);
            }
        }
        
        //
        // Window and GL View Initialization
        //
        
        // NOTE(yuval): Create NSWindow
        float w;
        float h;
        if (plat_settings.set_window_size){
            w = (float)plat_settings.window_w;
            h = (float)plat_settings.window_h;
        } else{
            w = 800.0f;
            h = 800.0f;
        }
        
        NSRect screen_rect = [[NSScreen mainScreen] frame];
        NSRect initial_frame = NSMakeRect((screen_rect.size.width - w) * 0.5f, (screen_rect.size.height - h) * 0.5f, w, h);
        
        u32 style_mask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
        
        mac_vars.window = [[NSWindow alloc] initWithContentRect:initial_frame
                styleMask:style_mask
                backing:NSBackingStoreBuffered
                defer:NO];
        
        [mac_vars.window setMinSize:NSMakeSize(100, 100)];
        [mac_vars.window setBackgroundColor:NSColor.blackColor];
        [mac_vars.window setDelegate:app_delegate];
        [mac_vars.window setTitle:@"4coder"];
        [mac_vars.window setAcceptsMouseMovedEvents:YES];
        
        // NOTE(yuval): Create OpenGLView
        NSView* content_view = [mac_vars.window contentView];
        
        mac_vars.gl_is_initialized = false;
        
        mac_vars.view = [[OpenGLView alloc] init];
        [mac_vars.view setFrame:[content_view bounds]];
        [mac_vars.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        
        // NOTE(yuval): Display opengl view and window
        [content_view addSubview:mac_vars.view];
        [mac_vars.window makeKeyAndOrderFront:nil];
        
        //
        // TODO(yuval): Misc System Initializations
        
        // NOTE(yuval): Get the timebase info
        mach_timebase_info(&mac_vars.timebase_info);
        
        //
        // App init
        //
        
        {
            Scratch_Block scratch(mac_vars.tctx, Scratch_Share);
            String_Const_u8 curdir = system_get_path(scratch, SystemPath_CurrentDirectory);
            curdir = string_mod_replace_character(curdir, '\\', '/');
            app.init(mac_vars.tctx, &target, base_ptr, mac_vars.clipboard_contents, curdir, custom);
        }
        
        // NOTE(yuval): Start the app's run loop
#if 1
        printf("Running using NSApp run\n");
        [NSApp run];
#else
        printf("Running using manual event loop\n");
        
        for (;;) {
            u64 count = 0;
            
            NSEvent* event;
            do {
                event = [NSApp nextEventMatchingMask:NSEventMaskAny
                        untilDate:[NSDate distantFuture]
                        inMode:NSDefaultRunLoopMode
                        dequeue:YES];
                
                [NSApp sendEvent:event];
            } while (event != nil);
        }
#endif
        
#if 0
        // NOTE(yuval): Application Core Update
        Application_Step_Result result = {};
        if (app.step != 0){
            result = app.step(mac_vars.tctx, &target, base_ptr, &input);
        }
#endif
    }
}