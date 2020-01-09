/* Mac OpenGL layer for 4coder */

#include "mac_4ed_renderer.h"

////////////////////////////////
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#include "opengl/4ed_opengl_defines.h"

#define GL_FUNC(N,R,P) typedef R (CALL_CONVENTION N##_Function)P; N##_Function *N = 0;
#include "mac_4ed_opengl_funcs.h"

////////////////////////////////

#include "opengl/4ed_opengl_render.cpp"

////////////////////////////////

@interface OpenGL_View : NSOpenGLView
- (void)init_gl;
- (void)render:(Render_Target*)target;
@end

////////////////////////////////

struct Mac_OpenGL_Renderer{
    Mac_Renderer base;
    OpenGL_View *opengl_view;
};

////////////////////////////////

@implementation OpenGL_View{
    b32 gl_is_initialized;
}

- (id)init{
    self = [super init];
    if (self == nil){
        return nil;
    }
    
    gl_is_initialized = false;
    [self init_gl];
    
    return self;
}

- (void)dealloc{
    [super dealloc];
}

- (void)prepareOpenGL{
    [super prepareOpenGL];
    
    [[self openGLContext] makeCurrentContext];
    
    // NOTE(yuval): Setup vsync
    GLint swap_int = 1;
    printf("Using vsync: %d\n", swap_int);
    [[self openGLContext] setValues:&swap_int forParameter:NSOpenGLCPSwapInterval];
}

- (void)awakeFromNib{
    [self init_gl];
}

- (void)reshape{
    [super reshape];
    
	[[self openGLContext] makeCurrentContext];
	[[self openGLContext] update];
}

- (void)init_gl{
    if (gl_is_initialized){
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
    
    gl_is_initialized = true;
}

- (void)render:(Render_Target*)target{
    Assert(gl_is_initialized);
    
    CGLLockContext([[self openGLContext] CGLContextObj]);
    [[self openGLContext] makeCurrentContext];
    
    gl_render(target);
    
    [[self openGLContext] flushBuffer];
    CGLUnlockContext([[self openGLContext] CGLContextObj]);
}
@end

////////////////////////////////

function
mac_render_sig(mac_gl_render){
    printf("Rendering using OpenGL!\n");
    
    Mac_OpenGL_Renderer *gl = (Mac_OpenGL_Renderer*)renderer;
    [gl->opengl_view render:target];
}

function
mac_get_texture_sig(mac_gl_get_texture){
    u32 result = gl__get_texture(dim, texture_kind);
    return(result);
}

function
mac_fill_texture_sig(mac_gl_fill_texture){
    b32 result = gl__fill_texture(texture_kind, texture, p, dim, data);
    return(result);
}

function Mac_OpenGL_Renderer*
mac_gl__init(NSWindow *window, Render_Target *target){
    // NOTE(yuval): Create the Mac Renderer
    Mac_OpenGL_Renderer *gl = (Mac_OpenGL_Renderer*)system_memory_allocate(sizeof(Mac_OpenGL_Renderer),
                                                                           file_name_line_number_lit_u8);
    gl->base.render = mac_gl_render;
    gl->base.get_texture = mac_gl_get_texture;
    gl->base.fill_texture = mac_gl_fill_texture;
    
    // NOTE(yuval): Create the OpenGL view
    NSView *content_view = [window contentView];
    
    gl->opengl_view = [[OpenGL_View alloc] init];
    [gl->opengl_view setFrame:[content_view bounds]];
    [gl->opengl_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [gl->opengl_view setWantsBestResolutionOpenGLSurface:YES];
    
    // NOTE(yuval): Add the OpenGL view as a subview of the window
    [content_view addSubview:gl->opengl_view];
    
    // NOTE(yuval): Load gl functions
    void *gl_image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
    
#define GL_FUNC(f,R,P) ((f) = (f##_Function*)dlsym(gl_image, #f));
#include "mac_4ed_opengl_funcs.h"
    
    return(gl);
}

// TODO(yuval): This function should be exported to a DLL
function
mac_load_renderer_sig(mac_load_opengl_renderer){
    printf("Initializing The OpenGL Renderer!\n");
    
    Mac_Renderer *renderer = (Mac_Renderer*)mac_gl__init(window, target);
    return(renderer);
}
