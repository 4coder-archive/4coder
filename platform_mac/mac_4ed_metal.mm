#import "metal/4ed_metal_render.mm"

global MTKView *metal_view;
global FCoderMetalRenderer *metal_renderer;

function void
mac_metal_init(NSWindow *window){
    // NOTE(yuval): Create Metal view
    NSView *content_view = [window contentView];
    
    metal_view = [[MTKView alloc] initWithFrame:[content_view bounds]];
    [metal_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    
    metal_view.device = MTLCreateSystemDefaultDevice();
    
    // NOTE(yuval): Add the Metal view as a subview of the window
    [content_view addSubview:metal_view];
    
    // NOTE(yuval): Create the Metal renderer
    metal_renderer = [[FCoderMetalRenderer alloc] initWithMetalKitView:metal_view];
}

function void
mac_metal_render(Render_Target* target){
    u64 begin_time = system_now_time();
    [metal_renderer drawInMTKView:metal_view];
    u64 end_time = system_now_time();
    printf("Metal Render Time: %fs\n\n", mac_get_time_diff_sec(begin_time, end_time));
}