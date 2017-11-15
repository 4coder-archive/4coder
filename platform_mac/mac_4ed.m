/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 28.06.2017
 *
 * Mac Objective C layer for 4coder
 *
 */

// TOP

#define inline internal
#include "4ed_defines.h"
#undef inline
#include "4coder_API/version.h"
#include "4coder_API/keycodes.h"

#define IS_OBJC_LAYER
#include "4ed_log.h"
#include "4ed_cursor_codes.h"
#include "4ed_doubly_linked_list.cpp"

#define WINDOW_NAME "4coder " VERSION

#undef internal
#undef global
#undef external
#define external

#include "osx_objective_c_to_cpp_links.h"

#include <CoreServices/CoreServices.h>
#import <Cocoa/Cocoa.h>
#import <CoreVideo/CVDisplayLink.h>
#import <IOKit/hid/IOHIDLib.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <time.h>

#include <string.h>

void
osx_post_to_clipboard(char *str){
    NSPasteboard *board = [NSPasteboard generalPasteboard];
    NSString *utf8_type = @"public.utf8-plain-text";
    NSArray<NSString*> *typesArray = [NSArray arrayWithObjects: utf8_type, nil];
    [board declareTypes:typesArray owner:nil];
    NSString *paste_string = [NSString stringWithUTF8String:str];
    [board setString:paste_string forType:utf8_type];
    osx_objc.just_posted_to_clipboard = true;
}


void
osx_error_dialogue(char *str){
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    NSString *text = [NSString stringWithUTF8String:str];
    [alert setMessageText:text];
    [alert setAlertStyle:NSCriticalAlertStyle];
    [alert runModal];
}

//
// Entry point, OpenGL window setup.
//

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@interface My4coderView : NSOpenGLView{
    @public
        //CVDisplayLinkRef displayLink;
}

- (void)requestDisplay;
- (void)checkClipboard;
- (CVReturn)getFrame;
- (void)drawRect:(NSRect)bounds;
@end

#define DISPLINK_SIG(n) CVReturn n(CVDisplayLinkRef link, const CVTimeStamp *now, const CVTimeStamp *output, CVOptionFlags flags_in, CVOptionFlags *flags_out, void *context)
static DISPLINK_SIG(osx_display_link);

@implementation My4coderView

- (void)keyDown:(NSEvent *)event{
    NSString *real = [event charactersIgnoringModifiers];
    NSString *with_mods = [event characters];
    
    b32 is_dead_key = false;
    if (real && !with_mods){
        is_dead_key = true;
    }
    
    OSX_Keyboard_Modifiers mods = {0};
    NSEventModifierFlags flags = [NSEvent modifierFlags];
    mods.shift = ((flags & NSEventModifierFlagShift) != 0);
    mods.command = ((flags & NSEventModifierFlagCommand) != 0);
    mods.control = ((flags & NSEventModifierFlagControl) != 0);
    mods.option = ((flags & NSEventModifierFlagOption) != 0);
    mods.caps = ((flags & NSEventModifierFlagCapsLock) != 0);
    
    // TODO(allen): Not ideal solution, look for realer text
    // input on Mac.  This just makes sure we're getting good
    // results for unmodified keys when cmnd and ctrl aren't down.
    NSString *which = with_mods;
    if (mods.command || mods.control){
        which = real;
    }
    
    u32 length = which.length;
    for (u32 i = 0; i < length; ++i){
        unichar c = [which characterAtIndex:i];
        osx_character_input(c, mods);
    }
}

- (void)mouseDown:(NSEvent*)event{
    NSPoint m = [event locationInWindow];
    osx_mouse(m.x, m.y, MouseType_Press);
}

- (void)mouseDragged:(NSEvent*)event{
    NSPoint m = [event locationInWindow];
    osx_mouse(m.x, m.y, MouseType_Move);
}

- (void)mouseMoved:(NSEvent*)event{
    NSPoint m = [event locationInWindow];
    osx_mouse(m.x, m.y, MouseType_Move);
}

- (void)mouseUp:(NSEvent*)event{
    NSPoint m = [event locationInWindow];
    osx_mouse(m.x, m.y, MouseType_Release);
}

- (void)scrollWheel:(NSEvent*)event{
    float dx = event.scrollingDeltaX;
    float dy = event.scrollingDeltaY;
    osx_mouse_wheel(dx, dy);
}

- (BOOL)windowShouldClose:(NSWindow*)sender{
    osx_try_to_close();
    return(NO);
}

- (void)requestDisplay{
    CGRect cg_rect = CGRectMake(0, 0, osx_objc.width, osx_objc.height);
    NSRect rect = NSRectFromCGRect(cg_rect);
    [self setNeedsDisplayInRect:rect];
}

static i32 did_update_for_clipboard = true;
- (void)checkClipboard{
    NSPasteboard *board = [NSPasteboard generalPasteboard];
    if (board.changeCount != osx_objc.prev_clipboard_change_count && did_update_for_clipboard){
        [self requestDisplay];
        did_update_for_clipboard = false;
    }
}

- (CVReturn)getFrame{
    did_update_for_clipboard = true;
    
    @autoreleasepool
    {
        if (osx_objc.running){
            osx_objc.has_clipboard_item = false;
            NSPasteboard *board = [NSPasteboard generalPasteboard];
            if (board.changeCount != osx_objc.prev_clipboard_change_count){
                if (!osx_objc.just_posted_to_clipboard){
                    NSString *utf8_type = @"public.utf8-plain-text";
                    NSArray *array = [NSArray arrayWithObjects: utf8_type, nil];
                    NSString *has_string = [board availableTypeFromArray:array];
                    if (has_string != nil){
                        NSData *data = [board dataForType: utf8_type];
                        if (data != nil){
                            u32 copy_length = data.length;
                            if (copy_length > 0){
                                // TODO(allen): Grow clipboard memory if needed.
                                if (copy_length+1 < osx_objc.clipboard_max){
                                    osx_objc.clipboard_size = copy_length;
                                    [data getBytes: osx_objc.clipboard_data length: copy_length];
                                    ((char*)osx_objc.clipboard_data)[copy_length] = 0;
                                    osx_objc.has_clipboard_item = true;
                                }
                            }
                        }
                    }
                }
                else{
                    osx_objc.just_posted_to_clipboard = false;
                }
                osx_objc.prev_clipboard_change_count = board.changeCount;
            }
            
            osx_step();
        }
    }
    return kCVReturnSuccess;
}

- (void)reshape
{
    [super reshape];
    
    NSRect rect = [self bounds];
    osx_resize(rect.size.width, rect.size.height);
}

- (void)init_gl
{
    if(osx_objc.running){
        return;
    }
    
    [self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersionLegacy,
        NSOpenGLPFAColorSize, 24,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        0
    };
    
    NSOpenGLPixelFormat* format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if(format == nil){
        fprintf(stderr, "Error creating OpenGLPixelFormat\n");
        exit(1);
    }
    
    NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
    
    [self setPixelFormat:format];
    [self setOpenGLContext:context];
    
    [context makeCurrentContext];
    
    osx_objc.running = true;
}

- (id)init
{
    self = [super init];
    if(self == nil)
    {
        return nil;
    }
    
    [self init_gl];
    return self;
}

- (void)drawRect: (NSRect) bounds{
    [self getFrame];
}

- (void)awakeFromNib
{
    [self init_gl];
}

- (void)prepareOpenGL
{
    [super prepareOpenGL];
    
    [[self openGLContext] makeCurrentContext];
    
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (void)dealloc
{
    [super dealloc];
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)becomeFirstResponder
{
    return YES;
}

- (BOOL)resignFirstResponder
{
    return YES;
}
@end

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(id)sender
{
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    return YES;
}
- (void)applicationWillTerminate:(NSApplication*)sender
{
}
@end

//////////////////

typedef struct File_Change_Node File_Change_Node;
struct File_Change_Node{
    File_Change_Node *next;
    char *name;
    i32 len;
};

typedef struct{
    File_Change_Node *first;
    File_Change_Node *last;
    volatile i64 lock;
} File_Change_Queue;

static File_Change_Queue file_queue = {0};

File_Change_Node*
file_change_node(char *name){
    i32 len = strlen(name);
    void *block = malloc(len + 1 + sizeof(File_Change_Node));
    File_Change_Node *node = (File_Change_Node*)block;
    memset(node, 0, sizeof(*node));
    node->name = (char*)(node + 1);
    node->len = len;
    memcpy(node->name, name, len + 1);
    return(node);
}

void
file_change_node_free(File_Change_Node *node){
    free(node);
}

#define file_queue_lock() for(;;){i64 v=__sync_val_compare_and_swap(&file_queue.lock,0,1);if(v==0){break;}}

#define file_queue_unlock() __sync_lock_test_and_set(&file_queue.lock, 0)

void
file_watch_callback(ConstFSEventStreamRef stream, void *callbackInfo, size_t numEvents, void *evPaths, const FSEventStreamEventFlags *evFlags, const FSEventStreamEventId *evIds){
    char **paths = (char**)evPaths;
    for (int i = 0; i < numEvents; ++i){
        File_Change_Node *node = file_change_node(paths[i]);
        file_queue_lock();
        sll_push(file_queue.first, file_queue.last, node);
        file_queue_unlock();
    }
}

//////////////////

typedef struct{
    FSEventStreamRef stream;
} File_Watching_Handle;

File_Watching_Handle
schedule_file_watching(char *f){
    File_Watching_Handle handle = {0};
    
    CFStringRef arg = CFStringCreateWithCString(0, f, kCFStringEncodingUTF8);
    
    CFArrayRef paths = CFArrayCreate(0, (const void**)&arg, 1, 0);
    
    void *callbackInfo = 0;
    CFAbsoluteTime latency = 2.0;
    
    handle.stream = FSEventStreamCreate(0, &file_watch_callback,  0, paths, kFSEventStreamEventIdSinceNow, latency, kFSEventStreamCreateFlagFileEvents);
    
    FSEventStreamScheduleWithRunLoop(handle.stream, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
    
    if (!FSEventStreamStart(handle.stream)){
        fprintf(stdout, "BAD SCHED: %s\n", f);
    }
    
    return(handle);
}

void
unschedule_file_watching(File_Watching_Handle handle){
    FSEventStreamStop(handle.stream);
    FSEventStreamInvalidate(handle.stream);
    FSEventStreamRelease(handle.stream);
}

typedef struct File_Table_Entry{
    u64 hash;
    void *name;
    i32 counter;
    File_Watching_Handle handle;
} File_Table_Entry;

typedef struct File_Change_Table{
    File_Table_Entry *table;
    i32 count;
    i32 size;
} File_Change_Table;

static File_Change_Table file_change_table = {0};

void*
osx_file_name_prefixed_length(char *name){
    i32 len = 0;
    for (; name[len] != 0; ++len);
    char *name_stored = (char*)malloc(4 + l_round_up_u32(len + 1, 4));
    *(i32*)name_stored = len;
    memcpy(name_stored + 4, name, len);
    name_stored[4 + len] = 0;
    return(name_stored);
}

b32
osx_name_prefixed_match(void *a, void *b){
    b32 result = false;
    i32 *len_a = (i32*)a;
    i32 *len_b = (i32*)b;
    if (*len_a == *len_b){
        char *str_a = (char*)(len_a + 1);
        char *str_b = (char*)(len_b + 1);
        if (strncmp(str_a, str_b, *len_a) == 0){
            result = true;
        }
    }
    return(result);
}

File_Table_Entry*
osx_file_listener_lookup_and_return_pointer(u64 hash, void *name){
    File_Table_Entry *result = 0;
    i32 index = (i32)(hash % file_change_table.size);
    i32 first_index = index;
    
    for (;;){
        File_Table_Entry *entry = &file_change_table.table[index];
        if (entry->hash == hash){
            if (osx_name_prefixed_match(name, entry->name)){
                result = entry;
                break;
            }
        }
        if (entry->name == 0){
            break;
        }
        
        index = (index + 1)%file_change_table.size;
        if (index == first_index){
            break;
        }
    }
    
    return(result);
}

void
osx_file_listener_table_entry_erase(File_Table_Entry *entry){
    free(entry->name);
    unschedule_file_watching(entry->handle);
    memset(entry, 0, sizeof(*entry));
    entry->name = (void*)1;
}

b32
osx_file_listener_lookup_and_decrement(u64 hash, void *name){
    b32 found = false;
    File_Table_Entry *entry = osx_file_listener_lookup_and_return_pointer(hash, name);
    if (entry != 0){
        found = true;
        --entry->counter;
        if (entry->counter <= 0){
            osx_file_listener_table_entry_erase(entry);
        }
    }
    return(found);
}

b32
osx_file_listener_hash(u64 hash, void *name, i32 counter, File_Watching_Handle **handle_address_out){
    b32 result = 0;
    if (file_change_table.count*6 < file_change_table.size*5){
        i32 index = (i32)(hash % file_change_table.size);
        i32 first_index = index;
        
        for (;;){
            File_Table_Entry *entry = &file_change_table.table[index];
            if (entry->name == 0 || entry->name == (void*)1){
                entry->hash = hash;
                entry->name = name;
                entry->counter = counter;
                *handle_address_out = &entry->handle;
                result = true;
                ++file_change_table.count;
                break;
            }
            
            index = (index + 1)%file_change_table.size;
            if (index == first_index){
                break;
            }
        }
        
        if (!result){
            LOG("file change listener table error: could not find a free slot in the table\n");
        }
    }
    
    return(result);
}

void
osx_file_listener_grow_table(i32 size){
    if (file_change_table.size < size){
        File_Table_Entry *old_table = file_change_table.table;
        i32 old_size = file_change_table.size;
        
        file_change_table.table = (File_Table_Entry*)osx_allocate(size*sizeof(File_Table_Entry));
        memset(file_change_table.table, 0, size*sizeof(File_Table_Entry));
        file_change_table.size = size;
        
        for (i32 i = 0; i < old_size; ++i){
            void *name = file_change_table.table[i].name;
            if (name != 0 && name != (void*)1){
                File_Table_Entry *e = &file_change_table.table[i];
                File_Watching_Handle *handle_address = 0;
                osx_file_listener_hash(e->hash, e->name, e->counter, &handle_address);
                *handle_address = e->handle;
            }
        }
        
        if (old_table != 0){
            osx_free(old_table, old_size*sizeof(File_Table_Entry));
        }
    }
}

void
osx_file_listener_double_table(){
    osx_file_listener_grow_table(file_change_table.size*2);
}

b32
osx_file_listener_insert_or_increment_always(u64 hash, void *name, File_Watching_Handle **handle_address_out){
    b32 was_already_in_table = false;
    File_Table_Entry *entry = osx_file_listener_lookup_and_return_pointer(hash, name);
    if (entry != 0){
        ++entry->counter;
        was_already_in_table = true;
    }
    else{
        b32 result = osx_file_listener_hash(hash, name, 1, handle_address_out);
        if (!result){
            osx_file_listener_double_table();
            osx_file_listener_hash(hash, name, 1, handle_address_out);
        }
    }
    return(was_already_in_table);
}

u64
osx_get_file_hash(void *name){
    u32 count = *(u32*)(name);
    char *str = (char*)name + 4;
    u64 hash = 0;
    u64 state = count;
    u64 inc = 1 + 2*count;
    for (u32 i = 0; i <= count; ++i){
        u64 old_state = state;
        state = state*6364136223846783005ULL + inc;
        u32 xorshifted = ((old_state >> 18u) ^ old_state) >> 27u;
        u32 rot = old_state >> 59u;
        hash = (hash << 3) + (hash & 1) + ((xorshifted >> rot) | (xorshifted << ((-rot) & 31)));
        if (i < count){
            inc = 1 + 2*(((inc - 1) << 7) | (u8)str[i]);
        }
    }
    return(hash);
}

void
osx_file_listener_init(void){
    osx_file_listener_grow_table(4096);
}

void
osx_add_file_listener(char *dir_name){
    // TODO(allen): Decide what to do about these darn string mallocs.
    void *name_stored = osx_file_name_prefixed_length(dir_name);
    File_Watching_Handle *handle_address = 0;
    b32 was_already_in_table = osx_file_listener_insert_or_increment_always(osx_get_file_hash(name_stored), name_stored, &handle_address);
    if (was_already_in_table){
        free(name_stored);
    }
    else{
        *handle_address = schedule_file_watching(dir_name);
    }
}

void
osx_remove_file_listener(char *dir_name){
    void *name_stored = osx_file_name_prefixed_length(dir_name);
    osx_file_listener_lookup_and_decrement(osx_get_file_hash(name_stored), name_stored);
    free(name_stored);
}

i32
osx_get_file_change_event(char *buffer, i32 max, i32 *size){
    file_queue_lock();
    File_Change_Node *node = file_queue.first;
    sll_pop(file_queue.first, file_queue.last);
    file_queue_unlock();
    
    i32 result = 0;
    if (node != 0){
        if (node->len < max){
            result = 1;
            memcpy(buffer, node->name, node->len);
            *size = node->len;
        }
        else{
            result = -1;
            // TODO(allen): Somehow save the node?
        }
        file_change_node_free(node);
    }
    
    return(result);
}

void
osx_show_cursor(i32 show, i32 cursor_type){
    local_persist b32 cursor_is_shown = 1;
    if (show == 1){
        if (!cursor_is_shown){
            [NSCursor unhide];
            cursor_is_shown = true;
        }
    }
    else if (show == -1){
        if (cursor_is_shown){
            [NSCursor hide];
            cursor_is_shown = false;
        }
    }
    
    if (cursor_type > 0){
        switch (cursor_type){
            case APP_MOUSE_CURSOR_ARROW:
            {
                [[NSCursor arrowCursor] set];
            }break;
            
            case APP_MOUSE_CURSOR_IBEAM:
            {
                [[NSCursor IBeamCursor] set];
            }break;
            
            case APP_MOUSE_CURSOR_LEFTRIGHT:
            {
                [[NSCursor resizeLeftRightCursor] set];
            }break;
            
            case APP_MOUSE_CURSOR_UPDOWN:
            {
                [[NSCursor resizeUpDownCursor] set];
            }break;
        }
    }
}

My4coderView* view = 0;
NSWindow* window = 0;

void
osx_begin_render(){
    CGLLockContext([[view openGLContext] CGLContextObj]);
    [[view openGLContext] makeCurrentContext];
}

void
osx_end_render(){
    [[view openGLContext] flushBuffer];
    CGLUnlockContext([[view openGLContext] CGLContextObj]);
}

void
osx_schedule_step(void){
    [NSTimer scheduledTimerWithTimeInterval: 0.0
            target: view
            selector: @selector(requestDisplay)
            userInfo: nil repeats:NO];
}

void
osx_toggle_fullscreen(void){
    [window toggleFullScreen:nil];
}

b32
osx_is_fullscreen(void){
    b32 result = (([window styleMask] & NSFullScreenWindowMask) != 0);
    return(result);
}

void
osx_close_app(void){
    [NSApp terminate: nil];
}

f32
osx_timer_seconds(void){
    f32 result = CACurrentMediaTime();
    return(result);
}

int
main(int argc, char **argv){
    memset(&osx_objc, 0, sizeof(osx_objc));
    
    umem clipboard_size = MB(4);
    osx_objc.clipboard_data = osx_allocate(clipboard_size);
    osx_objc.clipboard_max = clipboard_size;
    osx_objc.argc = argc;
    osx_objc.argv = argv;
    
    osx_file_listener_init();
    
    @autoreleasepool{
        NSApplication *app = [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        
        [app setDelegate:[[AppDelegate alloc] init]];
        NSRect screenRect = [[NSScreen mainScreen] frame];
        float w           = 800.f;
        float h           = 600.f;
        NSRect frame      = NSMakeRect((screenRect.size.width - w) * 0.5, (screenRect.size.height - h) * 0.5, w, h);
        
        u32 flags = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
        window = [[NSWindow alloc] initWithContentRect:frame styleMask:flags backing:NSBackingStoreBuffered defer:NO];
        
        [window setAcceptsMouseMovedEvents:YES];
        
        view = [[My4coderView alloc] init];
        [view setFrame:[[window contentView] bounds]];
        [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
        
        [[window contentView] addSubview:view];
        [window setMinSize:NSMakeSize(100, 100)];
        [window setTitle:@WINDOW_NAME];
        [window makeKeyAndOrderFront:nil];
        
        [NSTimer scheduledTimerWithTimeInterval: 0.5
                target: view
                selector: @selector(checkClipboard)
                userInfo: nil repeats:YES];
        
        osx_init();
        
        [NSApp run];
    }
    
    return(0);
}

// BOTTOM

