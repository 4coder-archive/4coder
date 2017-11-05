/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 28.06.2017
 *
 * Mac Objective C layer for 4coder
 *
 */

// TOP

#include "4ed_defines.h"
#include "4coder_API/version.h"
#include "4coder_API/keycodes.h"

#define WINDOW_NAME "4coder" VERSION

#undef internal
#undef global
#undef external
#define external

#include "osx_objective_c_to_cpp_links.h"

#import <Cocoa/Cocoa.h>
#import <CoreVideo/CVDisplayLink.h>
#import <IOKit/hid/IOHIDLib.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

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
	CVDisplayLinkRef displayLink;
}

- (CVReturn)getFrameForTime:(const CVTimeStamp*)time;
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

- (CVReturn)getFrameForTime:(const CVTimeStamp*)time{
    @autoreleasepool
    {
	if (osx_objc.running){
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
    if(osx_objc.running)
    {
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
    if(format == nil)
    {
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

    CVReturn cvreturn = CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    cvreturn = CVDisplayLinkSetOutputCallback(displayLink, &osx_display_link, (__bridge void*)(self));

    CGLContextObj cglContext         = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = [[self pixelFormat] CGLPixelFormatObj];
    cvreturn = CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);

    CVDisplayLinkStart(displayLink);
}

- (void)dealloc
{
    [super dealloc];

    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
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

static
DISPLINK_SIG(osx_display_link){
    My4coderView* view = (__bridge My4coderView*)context;
    CVReturn result = [view getFrameForTime:output];
    return result;
}

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

typedef struct File_Change_Queue{
	char *buffer;
	char *read_ptr;
	char *write_ptr;
	char *end;
} File_Change_Queue;

static File_Change_Queue file_change_queue = {0};

void
osx_add_file_listener(char *file_name){
	NotImplemented;
}

void
osx_remove_file_listener(char *file_name){
	NotImplemented;
}

void
block_split_copy(void *dst, void *src1, i32 size1, void *src2, i32 size2){
	memcpy(dst, src1, size1);
	memcpy((u8*)dst + size1, src2, size2);
}

i32
osx_get_file_change_event(char *buffer, i32 max, i32 *size){
	i32 result = 0;
	if (file_change_queue.read_ptr != file_change_queue.write_ptr){
		i32 change_size = *(i32*)file_change_queue.read_ptr;
		if (max <= change_size){
            char *b1 = file_change_queue.read_ptr + 4;
		    char *b2 = file_change_queue.buffer;
		    i32 b1_size = Min(change_size, (i32)(file_change_queue.end - b1));
            i32 b2_size = change_size - b1_size;
		    block_split_copy(buffer, b1, b1_size, b2, b2_size);
		    if (b1 < file_change_queue.end){
		    	file_change_queue.read_ptr = b1 + change_size;
		    }
		    else{
		    	file_change_queue.read_ptr = b2 + b2_size;
		    }
		    result = 1;
		}
		else{
			result = -1;
		}
	}
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

		My4coderView* view = [[My4coderView alloc] init];
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

