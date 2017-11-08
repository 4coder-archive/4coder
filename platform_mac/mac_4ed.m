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
	
	u32 length = real.length;
	for (u32 i = 0; i < length; ++i){
		unichar c = [real characterAtIndex:i];
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
    NSRect rect = CGRectMake(0, 0, osx_objc.width, osx_objc.height);
    [self setNeedsDisplayInRect:rect];
}

- (CVReturn)getFrame{
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
		
		CGLLockContext([[self openGLContext] CGLContextObj]);
		[[self openGLContext] makeCurrentContext];
		osx_step();
		[[self openGLContext] flushBuffer];
		CGLUnlockContext([[self openGLContext] CGLContextObj]);
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
	NSOpenGLPFAOpenGLProfile,
	//NSOpenGLProfileVersion3_2Core,
	NSOpenGLProfileVersionLegacy,
	NSOpenGLPFAColorSize,
	24,
	NSOpenGLPFAAlphaSize,
	8,
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

#if 0
    CVReturn cvreturn = CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    cvreturn = CVDisplayLinkSetOutputCallback(displayLink, &osx_display_link, (__bridge void*)(self));

    CGLContextObj cglContext         = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    cvreturn = CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

    CVDisplayLinkStart(displayLink);
#endif
}

- (void)dealloc
{
    [super dealloc];

#if 0
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
#endif
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

#if 0
static
DISPLINK_SIG(osx_display_link){
    My4coderView* view = (__bridge My4coderView*)context;
    CVReturn result = [view getFrame];
    return result;
}
#endif

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

typedef struct File_Table_Entry{
    u64 hash;
    void *name;
    i32 fd;
} File_Table_Entry;

typedef struct File_Change_Queue{
	i32 kq;
	
	File_Table_Entry *table;
	i32 table_count;
	i32 table_size;
} File_Change_Queue;

static File_Change_Queue file_change_queue = {0};

void*
osx_file_name_prefixed_length(char *name){
	i32 len = 0;
    for (; name[len] != 0; ++len);
    char *name_stored = (char*)malloc(4 + l_round_up_u32(len, 4));
    *(i32*)name_stored = len;
    memcpy(name_stored + 4, name, len);
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
	i32 index = (i32)(hash % file_change_queue.table_size);
	i32 first_index = index;

	for (;;){
		File_Table_Entry *entry = &file_change_queue.table[index];
		if (entry->hash == hash){
			if (osx_name_prefixed_match(name, entry->name)){
				result = entry;
				break;
			}
		}
		if (entry->name == 0){
			break;
		}

		index = (index + 1)%file_change_queue.table_size;
		if (index == first_index){
			break;
		}
	}

	return(result);
}

b32
osx_file_listener_lookup(u64 hash, void *name, i32 *fd_out){
	b32 found = false;
	File_Table_Entry *entry = osx_file_listener_lookup_and_return_pointer(hash, name);
	if (entry != 0){
		found = true;
		if (fd_out != 0){
			*fd_out = entry->fd;
		}
	}
	return(found);
}

b32
osx_file_listener_lookup_and_delete(u64 hash, void *name, i32 *fd_out){
	b32 found = false;
	File_Table_Entry *entry = osx_file_listener_lookup_and_return_pointer(hash, name);
	if (entry != 0){
		found = true;
		if (fd_out != 0){
			*fd_out = entry->fd;
		}
		memset(entry, 0, sizeof(*entry));
		entry->name = (void*)1;
	}
	return(found);
}

typedef struct Hash_Result{
	b32 is_in_table;
	b32 was_already_in_table;
} Hash_Result;

u64
osx_file_hash(void *name){
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

Hash_Result
osx_file_listener_hash(u64 hash, void *name, i32 fd){
	Hash_Result result = {0};
	if (osx_file_listener_lookup(hash, name, 0)){
		result.is_in_table = true;
		result.was_already_in_table = true;
	}
	else if (file_change_queue.table_count * 6 < file_change_queue.table_size * 5){
		i32 index = (i32)(hash % file_change_queue.table_size);
		i32 first_index = index;

		for (;;){
			File_Table_Entry *entry = &file_change_queue.table[index];
			if (entry->name == 0 || entry->name == (void*)1){
				entry->hash = hash;
				entry->name = name;
				entry->fd = fd;
				result.is_in_table = true;
				++file_change_queue.table_count;
				break;
			}

			index = (index + 1)%file_change_queue.table_size;
			if (index == first_index){
				break;
			}
		}

		if (!result.is_in_table){
			fprintf(stdout, "file change listener table error: could not find a free slot in the table\n");
		}
	}

	return(result);
}

Hash_Result
osx_file_listener_hash_bundled(File_Table_Entry entry){
	Hash_Result result = osx_file_listener_hash(entry.hash, entry.name, entry.fd);
	return(result);
}

void
osx_file_listener_grow_table(i32 table_size){
	if (file_change_queue.table_size < table_size){
		File_Table_Entry *old_table = file_change_queue.table;
		i32 old_size = file_change_queue.table_size;

		file_change_queue.table = (File_Table_Entry*)osx_allocate(table_size*sizeof(File_Table_Entry));
		memset(file_change_queue.table, 0, table_size*sizeof(File_Table_Entry));
		file_change_queue.table_size = table_size;

		for (i32 i = 0; i < old_size; ++i){
			void *name = file_change_queue.table[i].name;
			if (name != 0 && name != (void*)1){
				osx_file_listener_hash_bundled(file_change_queue.table[i]);
			}
		}

		if (old_table != 0){
			osx_free(old_table, old_size*sizeof(File_Table_Entry));
		}
	}
}

void
osx_file_listener_double_table(){
	osx_file_listener_grow_table(file_change_queue.table_size*2);
}

b32
osx_file_listener_hash_always(u64 hash, void *name, i32 fd){
	b32 was_already_in_table = false;
	Hash_Result result = osx_file_listener_hash(hash, name, fd);
	if (result.was_already_in_table){
		was_already_in_table = true;
	}
	else if (!result.is_in_table){
		osx_file_listener_double_table();
		osx_file_listener_hash(hash, name, fd);
	}
	return(was_already_in_table);
}

void
osx_file_listener_init(void){
    memset(&file_change_queue, 0, sizeof(file_change_queue));
    file_change_queue.kq = kqueue();
    osx_file_listener_grow_table(1024);
}

void
osx_add_file_listener(char *dir_name){
    DBG_POINT();
    if (file_change_queue.kq < 0){
        return;
    }

    //fprintf(stdout, "ADD_FILE_LISTENER: %s\n", dir_name);

    i32 fd = open(dir_name, O_EVTONLY);
    if (fd <= 0){
        fprintf(stdout, "could not open fd for %s\n", dir_name);
        return;
    }

    // TODO(allen): Decide what to do about these darn string mallocs.
    void *name_stored = osx_file_name_prefixed_length(dir_name);

    struct kevent new_kevent;
    EV_SET(&new_kevent, fd, EVFILT_VNODE, EV_ADD|EV_CLEAR, NOTE_DELETE|NOTE_WRITE, 0, name_stored);

    struct timespec t = {0};
    kevent(file_change_queue.kq, &new_kevent, 1, 0, 0, &t);

    b32 was_already_in_table = osx_file_listener_hash_always(osx_file_hash(name_stored), name_stored, fd);
    if (was_already_in_table){
    	free(name_stored);
    }

    DBG_POINT();
}

void
osx_remove_file_listener(char *dir_name){
	void *name = osx_file_name_prefixed_length(dir_name);
	i32 fd;
	if (osx_file_listener_lookup_and_delete(osx_file_hash(name), name, &fd)){
		close(fd);
	}
	free(name);
}

i32
osx_get_file_change_event(char *buffer, i32 max, i32 *size){
    if (file_change_queue.kq < 0){
        return 0;
    }

    i32 result = 0;
    struct timespec t = {0};
    struct kevent event_out;
    i32 count = kevent(file_change_queue.kq, 0, 0, &event_out, 1, &t);
    if (count < 0 || (count > 0 && event_out.flags == EV_ERROR)){
        fprintf(stdout, "count: %4d error: %s\n", count, strerror(errno));
    }
    else if (count > 0){
        if (event_out.udata != 0){
            i32 len = *(i32*)event_out.udata;
            char *str = (char*)((i32*)event_out.udata + 1);
            fprintf(stdout, "got an event for file: %.*s\n", len, str);
            if (len <= max){
                *size = len;
                memcpy(buffer, str, len);
                result = 1;
            }
            else{
                // TODO(allen): Cache this miss for retrieval???
                // TODO(allen): Better yet, always cache every event on every platform and therefore make two calls per event, but always with enough memory!??
                result = -1;
            }
        }
    }

    return(result);
}

My4coderView* view = 0;

void
osx_schedule_step(void){
    //DBG_POINT();
#if 1
    [NSTimer scheduledTimerWithTimeInterval: 0.0
             target: view
             selector: @selector(requestDisplay)
             userInfo: nil repeats:NO];
#else
    [view requestDisplay];
#endif
}

void
osx_close_app(void){
    [NSApp terminate: nil];
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
		NSWindow* window = [[NSWindow alloc] initWithContentRect:frame styleMask:flags backing:NSBackingStoreBuffered defer:NO];

		[window setAcceptsMouseMovedEvents:YES];

		view = [[My4coderView alloc] init];
		[view setFrame:[[window contentView] bounds]];
		[view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

		[[window contentView] addSubview:view];
		[window setMinSize:NSMakeSize(100, 100)];
		[window setTitle:@WINDOW_NAME];
		[window makeKeyAndOrderFront:nil];

		osx_init();

		[NSApp run];
	}

	return(0);
}

// BOTTOM

