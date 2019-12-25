/* Mac Objective C layer for 4coder */

#include "4coder_base_types.h"

#include "mac_objective_c_to_cpp_links.h"

#undef function
#undef internal
#undef global
#undef external
#include <Cocoa/Cocoa.h>

#include <libproc.h> // NOTE(yuval): Used for proc_pidpath

#include <sys/types.h> // NOTE(yuval): Used for pid_t
#include <unistd.h> // NOTE(yuval): Used for getpid

#define external extern "C"

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

- (void)windowWillClose:(id)sender{
    // global_running = false;
}
@end

external String_Const_u8
mac_standardize_path(Arena* arena, String_Const_u8 path){
    NSString *path_ns_str =
        [[NSString alloc] initWithBytes:path.data length:path.size encoding:NSUTF8StringEncoding];
    
    NSString *standardized_path_ns_str = [path_ns_str stringByStandardizingPath];
    String_Const_u8 standardized_path = SCu8([standardized_path_ns_str UTF8String],[standardized_path_ns_str lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
    
    String_Const_u8 result = push_string_copy(arena, standardized_path);
    
    [path release];
    
    return result;
}

external i32
mac_get_binary_path(void *buffer, u32 size){
    pid_t pid = getpid();
    i32 bytes_read = proc_pidpath(pid, buffer, size);
    
    return bytes_read;
}

int
main(int arg_count, char **args){
    @autoreleasepool{
        NSFileManager *fileManager = [[NSFileManager alloc] init];
        NSString *displayNameAtPath = [fileManager displayNameAtPath:@"build"];
        NSLog(@"Display Name: %@", displayNameAtPath);
        
        // NOTE(yuval): Create NSApplication & Delegate
        NSApplication* app = [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        App_Delegate* app_delegate = [[App_Delegate alloc] init];
        [app setDelegate:app_delegate];
        
        [NSApp finishLaunching];
        
        mac_init();
        
#if 0
        // NOTE(yuval): Application Core Update
        Application_Step_Result result = {};
        if (app.step != 0){
            result = app.step(mac_vars.tctx, &target, base_ptr, &input);
        }
#endif
    }
}