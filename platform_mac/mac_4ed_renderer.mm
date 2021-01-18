/* Mac Renderer Abstraction Implementation */

// TODO(yuval): This should NOT be included here once the renderer is exported to a DLL
#import "mac_4ed_opengl.mm"
#import "mac_4ed_metal.mm"

// TODO(yuval): Replace this array with an array of the paths to the renderer dlls
global mac_load_renderer_type *mac_renderer_load_functions[MacRenderer_COUNT] = {
    mac_load_opengl_renderer,
    mac_load_metal_renderer
};

function Mac_Renderer*
mac_init_renderer(Mac_Renderer_Kind kind, NSWindow *window, Render_Target *target){
    // TODO(yuval): Import renderer load function from a DLL instead of using an array of the load functions. This would allow us to switch the renderer backend and implemented new backends with ease.
    
    mac_load_renderer_type *load_renderer = mac_renderer_load_functions[kind];
    Mac_Renderer *result = load_renderer(window, target);
    
    if (!result){
        system_error_box("Unable to initialize the renderer!");
    }
    
    return result;
}

