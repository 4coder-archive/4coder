/* Mac Renderer Abstraction Implementation */

// TODO(yuval): This should NOT be included here once the renderer is exported to a DLL
#import "mac_4ed_opengl.mm"
#import "mac_4ed_metal.mm"

function Mac_Renderer*
mac_init_renderer(Mac_Renderer_Kind kind, NSWindow *window, Render_Target *target){
    // TODO(yuval): Import renderer load function from a DLL instead of using a switch statement and a renderer kind. This would allow us to switch the renderer backend and implemented new backends with ease.
    
    Mac_Renderer *result = 0;
    
    switch (kind){
        case MacRenderer_OpenGL:
        {
            result = mac_load_opengl_renderer(window, target);
        } break;
        
        case MacRenderer_Metal:
        {
            //result = mac_load_metal_renderer(window, target);
        } break;
        
        default: InvalidPath;
    }
    
    if (!result){
        mac_error_box("Unable to initialize the renderer!");
    }
    
    return result;
}

