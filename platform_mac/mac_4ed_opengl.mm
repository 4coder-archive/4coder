/* Mac OpenGL layer for 4coder */

#include "opengl/4ed_opengl_defines.h"

#define GL_FUNC(N,R,P) typedef R (CALL_CONVENTION N##_Function)P; N##_Function *N = 0;
#include "mac_4ed_opengl_funcs.h"

f64 mac_get_time_diff_sec(u64 begin, u64 end){
    f64 result = ((end - begin) / 1000000.0);
    return result;
}

#include "opengl/4ed_opengl_render.cpp"

@interface OpenGLView : NSOpenGLView
- (void)initGL;
- (void)render:(Render_Target*)target;
@end

@implementation OpenGLView{
    b32 glIsInitialized;
}

- (id)init{
    self = [super init];
    if (self == nil){
        return nil;
    }
    
    glIsInitialized = false;
    [self initGL];
    
    return self;
}

- (void)dealloc{
    [super dealloc];
}

- (void)prepareOpenGL{
    [super prepareOpenGL];
    
    [[self openGLContext] makeCurrentContext];
    
    // NOTE(yuval): Setup vsync
    GLint swapInt = 1;
    printf("Using vsync: %d\n", swapInt);
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (void)awakeFromNib{
    [self initGL];
}

- (void)reshape{
    [super reshape];
    
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
}

- (void)initGL{
    if (glIsInitialized){
        return;
    }
    
    // NOTE(yuval): Setup OpenGL
    NSOpenGLPixelFormatAttribute opengl_attrs[] = {
        NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAColorSize, 32,
        NSOpenGLPFAAlphaSize, 8,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFASampleBuffers, 1,
        NSOpenGLPFASamples, 16,
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
    
    glIsInitialized = true;
}

- (void)render:(Render_Target*)target{
    Assert(glIsInitialized);
    
    u64 context_lock_begin = system_now_time();
    CGLLockContext([[self openGLContext] CGLContextObj]);
    u64 context_lock_end = system_now_time();
    printf("Context lock time: %fs\n", mac_get_time_diff_sec(context_lock_begin, context_lock_end));
    
    u64 make_current_context_begin = system_now_time();
    [[self openGLContext] makeCurrentContext];
    u64 make_current_context_end = system_now_time();
    printf("Make current context time: %fs\n", mac_get_time_diff_sec(make_current_context_begin, make_current_context_end));
    
    u64 gl_render_begin = system_now_time();
    gl_render(target);
    u64 gl_render_end = system_now_time();
    printf("GL render time: %fs\n", mac_get_time_diff_sec(gl_render_begin, gl_render_end));
    
    u64 gl_flush_buffer_begin = system_now_time();
    [[self openGLContext] flushBuffer];
    u64 gl_flush_buffer_end = system_now_time();
    printf("GL flush buffer time: %fs\n", mac_get_time_diff_sec(gl_flush_buffer_begin, gl_flush_buffer_end));
    
    u64 context_unlock_begin = system_now_time();
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
    u64 context_unlock_end = system_now_time();
    printf("Context unlock time: %fs\n", mac_get_time_diff_sec(context_unlock_begin, context_unlock_end));
}
@end

global OpenGLView *opengl_view;

function void
mac_gl_init(NSWindow *window){
    // NOTE(yuval): Create OpenGLView
    NSView *content_view = [window contentView];
    
    opengl_view = [[OpenGLView alloc] init];
    [opengl_view setFrame:[content_view bounds]];
    [opengl_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [opengl_view setWantsBestResolutionOpenGLSurface:YES];
    
    // NOTE(yuval): Add the OpenGL view as a subview of the window
    [content_view addSubview:opengl_view];
    
    // NOTE(yuval): Load gl functions
    void *gl_image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
    
#define GL_FUNC(f,R,P) ((f) = (f##_Function*)dlsym(gl_image, #f));
#include "mac_4ed_opengl_funcs.h"
}

function void
mac_gl_render(Render_Target* target){
    u64 begin_time = system_now_time();
    [opengl_view render:target];
    u64 end_time = system_now_time();
    printf("Render Time: %fs\n\n", mac_get_time_diff_sec(begin_time, end_time));
}