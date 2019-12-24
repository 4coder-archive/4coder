#include <Cocoa/Cocoa.h>

@interface App_Delegate : NSObject<NSApplicationDelegate, NSWindowDelegate>
@end

@implementation App_Delegate
- (void)applicationDidFinishLaunching:(id)sender{
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender{
    return YES;
}

- (void)applicationWillTerminate:(NSNotification *)notification{
}

- (NSSize)windowWillResize:(NSWindow*)window toSize:(NSSize)frame_size{
    // frame_size.height = ((f32)frame_size.width / global_aspect_ratio);
    return frame_size;
}

- (void)windowWillClose:(id)sender {
    // global_running = false;
}
@end

int
main(int arg_count, char **args){
    @autoreleasepool{
        // NOTE(yuval): NSApplication & Delegate Creation
        NSApplication* app = [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        App_Delegate* app_delegate = [[App_Delegate alloc] init];
        [app setDelegate:app_delegate];
        
        [NSApp finishLaunching];
        
#if 0
        Thread_Context _tctx = {};
        thread_ctx_init(&_tctx, ThreadKind_Main,
                        get_base_allocator_system(),
                        get_base_allocator_system());
        
        block_zero_struct(&global_mac_vars);
        global_mac_vars.tctx = &_tctx;
        
        // NOTE(yuval): Application Core Update
        Application_Step_Result result = {};
        if (app.step != 0){
            result = app.step(mac_vars.tctx, &target, base_ptr, &input);
        }
#endif
    }
}