/* Mac Metal layer for 4coder */

#import "metal/4ed_metal_render.mm"

////////////////////////////////

struct Mac_Metal{
    Mac_Renderer base;

    Metal_Renderer *renderer;
    MTKView *view;
};

////////////////////////////////

function
mac_render_sig(mac_metal__render){
#if defined(FRED_INTERNAL)
    printf("Redering using Metal!\n");
#endif

    Mac_Metal *metal = (Mac_Metal*)renderer;
    [metal->view draw];
}

function
mac_get_texture_sig(mac_metal__get_texture){
    Mac_Metal *metal = (Mac_Metal*)renderer;
    u32 result = [metal->renderer get_texture_of_dim:dim
            kind:texture_kind];

    return(result);
}

function
mac_fill_texture_sig(mac_metal__fill_texture){
    Mac_Metal *metal = (Mac_Metal*)renderer;
    b32 result = [metal->renderer fill_texture:texture
            kind:texture_kind
            pos:p
            dim:dim
            data:data];

    return(result);
}

function Mac_Metal*
mac_metal__init(NSWindow *window, Render_Target *target){
    // NOTE(yuval): Create the Mac Metal rendere
    Mac_Metal *metal = (Mac_Metal*)system_memory_allocate(sizeof(Mac_Metal),
                                                          file_name_line_number_lit_u8);
    metal->base.render = mac_metal__render;
    metal->base.get_texture = mac_metal__get_texture;
    metal->base.fill_texture = mac_metal__fill_texture;

    // NOTE(yuval): Create the Metal view
    NSView *content_view = [window contentView];

    metal->view = [[MTKView alloc] initWithFrame:[content_view bounds]];
    [metal->view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [metal->view setPaused:YES];
    [metal->view setEnableSetNeedsDisplay:NO];

    metal->view.device = MTLCreateSystemDefaultDevice();

    // NOTE(yuval): Add the Metal view as a subview of the window
    [content_view addSubview:metal->view];

    // NOTE(yuval): Create the Metal renderer and set it as the Metal view's delegate
    metal->renderer = [[Metal_Renderer alloc] initWithMetalKitView:metal->view target:target];
    metal->view.delegate = metal->renderer;

    return(metal);
}

////////////////////////////////

// TODO(yuval): This function should be exported to a DLL
function
mac_load_renderer_sig(mac_load_metal_renderer){
    Mac_Renderer *renderer = (Mac_Renderer*)mac_metal__init(window, target);
    return(renderer);
}
