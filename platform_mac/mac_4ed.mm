/* Mac Objective C layer for 4coder */

#include "4coder_base_types.h"

#if 1
#include "4coder_table.h"
#include "4coder_events.h"
// NOTE(allen): This is a very unfortunate hack, but hopefully there will never be a need to use the Marker
// type in the platform layer. If that changes then instead change the name of Marker and make a transition
// macro that is only included in custom code.
#define Marker Marker__SAVE_THIS_IDENTIFIER
#include "4coder_types.h"
#undef Marker
#endif

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

#if 0
external File_List
mac_get_file_list(Arena* arena, String_Const_u8 directory){
    File_List result = {};
    
    NSString *directory_ns_string =
        [[NSString alloc]
            initWithBytes:directory.data length:directory.size encoding:NSUTF8StringEncoding];
    
    NSFileManager *file_manager = [NSFileManager defaultManager];
    NSDirectoryEnumerator *dirEnum = [file_manager enumeratorAtPath:directory_ns_string];
    
    NSString *filename;
    while ((filename = [dirEnum nextObject])){
        NSLog(@"%@", filename);
    }
    
    [directory_ns_string release];
}
#endif

external String_Const_u8
mac_standardize_path(Arena* arena, String_Const_u8 path){
    NSString *path_ns_str =
        [[NSString alloc] initWithBytes:path.data length:path.size encoding:NSUTF8StringEncoding];
    
    NSString *standardized_path_ns_str = [path_ns_str stringByStandardizingPath];
    String_Const_u8 standardized_path = mac_SCu8((u8*)[standardized_path_ns_str UTF8String],[standardized_path_ns_str lengthOfBytesUsingEncoding:NSUTF8StringEncoding]);
    
    String_Const_u8 result = mac_push_string_copy(arena, standardized_path);
    
    [path_ns_str release];
    
    return(result);
}

external i32
mac_get_binary_path(void *buffer, u32 size){
    pid_t pid = getpid();
    i32 bytes_read = proc_pidpath(pid, buffer, size);
    
    return(bytes_read);
}

int
main(int arg_count, char **args){
    @autoreleasepool{
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