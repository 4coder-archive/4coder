#import "metal/4ed_metal_render.mm"

global Metal_Renderer *metal_renderer;
global MTKView *metal_view;

function void
mac_metal_init(NSWindow *window){
    // NOTE(yuval): Create Metal view
    NSView *content_view = [window contentView];
    
    metal_view = [[MTKView alloc] initWithFrame:[content_view bounds]];
    [metal_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [metal_view setPaused:YES];
    [metal_view setEnableSetNeedsDisplay:NO];
    [metal_view setSampleCount:4];
    
    metal_view.device = MTLCreateSystemDefaultDevice();
    
    // NOTE(yuval): Add the Metal view as a subview of the window
    [content_view addSubview:metal_view];
    
    // NOTE(yuval): Create the Metal renderer and set it as the Metal view's delegate
    metal_renderer = [[Metal_Renderer alloc] initWithMetalKitView:metal_view];
    metal_view.delegate = metal_renderer;
}

function void
mac_metal_render(Render_Target* target){
    u64 begin_time = system_now_time();
    metal_renderer.target = target;
    [metal_view draw];
    u64 end_time = system_now_time();
    printf("Metal Render Time: %fs\n\n", mac_get_time_diff_sec(begin_time, end_time));
}

function u32
mac_metal_get_texture(Vec3_i32 dim, Texture_Kind texture_kind){
    u32 result = [metal_renderer get_texture_of_dim:dim
            kind:texture_kind];
    
    return result;
}

function b32
mac_metal_fill_texture(Texture_Kind texture_kind, u32 texture, Vec3_i32 p, Vec3_i32 dim, void *data){
    b32 result = [metal_renderer fill_texture:texture
            kind:texture_kind
            pos:p
            dim:dim
            data:data];
    
    return result;
}