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

#include <dirent.h> // NOTE(yuval): Used for opendir, readdir
#include <dlfcn.h> // NOTE(yuval): Used for dlopen, dlclose, dlsym
#include <errno.h> // NOTE(yuval): Used for errno
#include <fcntl.h> // NOTE(yuval): Used for open
#include <unistd.h> // NOTE(yuval): Used for getcwd, read, write, getpid
#include <sys/stat.h> // NOTE(yuval): Used for stat
#include <sys/types.h> // NOTE(yuval): Used for struct stat, pid_t

#include <stdlib.h> // NOTE(yuval): Used for free

#define function static
#define internal static
#define global static
#define external extern "C"

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
        struct{
            NSTimer* timer;
        } timer;
    };
};

struct Mac_Vars {
    Thread_Context *tctx;
    
    Arena* frame_arena;
    
    String_Const_u8 binary_path;
    
    NSWindow* window;
    OpenGLView* view;
    
    mach_timebase_info_data_t timebase_info;
    
    Node free_mac_objects;
    Node timer_objects;
};

////////////////////////////////

global Mac_Vars mac_vars;
global Render_Target target;

////////////////////////////////

function inline Plat_Handle
mac_to_plat_handle(Mac_Object *object){
    Plat_Handle result = *(Plat_Handle*)(&object);
    return(result);
}

function inline Mac_Object*
mac_to_object(Plat_Handle handle){
    Mac_Object* result = *(Mac_Object**)(&handle);
    return(result);
}

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

////////////////////////////////

#import "mac_4ed_functions.mm"

////////////////////////////////

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(id)sender{
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender{
    return YES;
}

- (void)applicationWillTerminate:(NSNotification*)notification{
}

- (NSSize)windowWillResize:(NSWindow*)window toSize:(NSSize)frame_size{
    // frame_size.height = ((f32)frame_size.width / global_aspect_ratio);
    return frame_size;
}

- (void)windowWillClose:(id)sender{
    // global_running = false;
}
@end

@implementation OpenGLView
- (id)init {
    self = [super init];
    return self;
}

- (void)prepareOpenGL{
    [super prepareOpenGL];
    [[self openGLContext] makeCurrentContext];
}

- (void)reshape{
    [super reshape];
    
    NSRect bounds = [self bounds];
    // [global_opengl_context makeCurrentContext];
    // [global_opengl_context update];
    // glViewport(0, 0, (GLsizei)bounds.size.width,
    // (GLsizei)bounds.size.height);
}

- (void)requestDisplay{
    printf("Display Requested\n");
}
@end

////////////////////////////////

int
main(int arg_count, char **args){
    block_zero_struct(&mac_vars);
    
    @autoreleasepool{
        // NOTE(yuval): Create NSApplication & Delegate
        NSApplication *app = [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        AppDelegate *app_delegate = [[AppDelegate alloc] init];
        [app setDelegate:app_delegate];
        
        // NOTE(yuval): Create NSWindow
        float w = 1280.0f;
        float h = 720.0f;
        NSRect screen_rect = [[NSScreen mainScreen] frame];
        NSRect initial_frame = NSMakeRect((screen_rect.size.width - w) * 0.5f, (screen_rect.size.height - h) * 0.5f, w, h);
        
        NSWindow* window = [[NSWindow alloc] initWithContentRect: initial_frame
                styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                backing: NSBackingStoreBuffered
                defer: NO];
        
        [window setBackgroundColor: NSColor.blackColor];
        [window setDelegate: app_delegate];
        [window setTitle: @"4coder"];
        [window makeKeyAndOrderFront: nil];
        
        // NOTE(yuval): Create OpenGLView
        NSView* content_view = [window contentView];
        
        // TODO(yuval): Finish view setup!
        mac_vars.view = [[OpenGLView alloc] init];
        [mac_vars.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        [mac_vars.view setFrame:[content_view bounds]];
        
        [content_view addSubview:mac_vars.view];
        
        dll_init_sentinel(&mac_vars.free_mac_objects);
        dll_init_sentinel(&mac_vars.timer_objects);
        
        Arena test_arena = make_arena_malloc();
        Plat_Handle timer = system_wake_up_timer_create();
        system_wake_up_timer_set(timer, 5000);
        
        // NOTE(yuval): Start the app's run loop
        [NSApp run];
        
        
        
#if 0
        // NOTE(yuval): Context Setup
        Thread_Context _tctx = {};
        thread_ctx_init(&_tctx, ThreadKind_Main,
                        get_base_allocator_system(),
                        get_base_allocator_system());
        
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
        
        // NOTE(yuval): Get the timebase info
        mach_timebase_info(&mac_vars.timebase_info);
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