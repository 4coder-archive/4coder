/* 4coder Metal render implementation */

#undef clamp
#undef function
#import <simd/simd.h>
#import <MetalKit/MetalKit.h>

#include "AAPLShaderTypes.h"
#define function static

struct Metal_Renderer{
    MTKView *view;
    
    id<MTLDevice> device;
    id<MTLRenderPipelineState> pipeline_state;
    id<MTLCommandQueue> command_queue;
    id<MTLBuffer> buffer;
    id<MTLCaptureScope> capture_scope;
};

global_const u32 metal_max_vertices = (1<<16);

global_const char *metal__shaders_source = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

////////////////////////////////

typedef struct{
packed_float2 xy;
    packed_float3 uvw;
uint32_t color;
float half_thickness;
} Vertex;

// NOTE(yuval): Vertex shader outputs and fragment shader inputs
typedef struct{
// NOTE(yuval): Vertex shader output
float4 position [[position]];

// NOTE(yuval): Fragment shader inputs
    float4 color;
    float3 uvw;
    float2 xy;
    float2 adjusted_half_dim;
    float half_thickness;
} Rasterizer_Data;

////////////////////////////////

vertex Rasterizer_Data
vertex_shader(uint vertex_id [[vertex_id]], constant Vertex *vertices [[buffer(0)]],
constant float4x4 &proj [[buffer(1)]]){
    constant Vertex *in = &vertices[vertex_id];
    Rasterizer_Data out;
    
    // NOTE(yuval): Calculate position in NDC
    out.position = proj * float4(in->xy, 0.0, 1.0);
    
    // NOTE(yuval): Convert color to float4 format
    out.color.b = ((float((in->color       ) & 0xFFu)) / 255.0);
    out.color.g = ((float((in->color >>  8u) & 0xFFu)) / 255.0);
    out.color.r = ((float((in->color >> 16u) & 0xFFu)) / 255.0);
    out.color.a = ((float((in->color >> 24u) & 0xFFu)) / 255.0);
    
    // NOTE(yuval): Pass uvw coordinates to the fragment shader
    out.uvw = in->uvw;
    
    // NOTE(yuval): Calculate adjusted half dim
    float2 center = in->uvw.xy;
    float2 half_dim = abs(in->xy - center);
    out.adjusted_half_dim = (half_dim - in->uvw.zz + float2(0.5, 0.5));
    
    // NOTE(yuval): Pass half_thickness to the fragment shader
    out.half_thickness = in->half_thickness;
    
    // NOTE(yuval): Pass xy to the fragment shader
    out.xy = in->xy;
    
    return(out);
}

////////////////////////////////

float
rectangle_sd(float2 p, float2 b){
float2 d = (abs(p) - b);
float result = (length(max(d, float2(0.0, 0.0))) + min(max(d.x, d.y), 0.0));

return(result);
}

fragment float4
fragment_shader(Rasterizer_Data in [[stage_in]]){
float has_thickness = step(0.49, in.half_thickness);
// float does_not_have_thickness = (1.0 - has_thickness);

// TODO(yuval): Sample texture here.

float2 center = in.uvw.xy;
float roundness = in.uvw.z;
float sd = rectangle_sd(in.xy - center, in.adjusted_half_dim);
sd = sd - roundness;
sd = (abs(sd + in.half_thickness) - in.half_thickness);
float shape_value = (1.0 - smoothstep(-1.0, 0.0, sd));
shape_value *= has_thickness;

    // TOOD(yuval): Add sample_value to alpha
   float4 out_color = in.color;// float4(in.color.xyz, in.color.a * (shape_value));
    return(out_color);
}
)";

function void
metal_init(Metal_Renderer *renderer, MTKView *view){
    NSError *error = nil;
    
    renderer->view = view;
    renderer->device = view.device;
    
    // NOTE(yuval): Compile the shaders
    id<MTLFunction> vertex_function = nil;
    id<MTLFunction> fragment_function = nil;
    {
        NSString *shaders_source_str = [NSString stringWithUTF8String:metal__shaders_source];
        
        MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
        options.fastMathEnabled = YES;
        
        id<MTLLibrary> shader_library = [renderer->device newLibraryWithSource:shaders_source_str
                options:options error:&error];
        vertex_function = [shader_library newFunctionWithName:@"vertex_shader"];
        fragment_function = [shader_library newFunctionWithName:@"fragment_shader"];
        
        [options release];
    }
    
    Assert(error == nil);
    
    // NOTE(yuval): Configure the pipeline descriptor
    {
        MTLRenderPipelineDescriptor *pipeline_state_descriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipeline_state_descriptor.label = @"4coder Metal Renderer Pipeline";
        pipeline_state_descriptor.vertexFunction = vertex_function;
        pipeline_state_descriptor.fragmentFunction = fragment_function;
        pipeline_state_descriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
        pipeline_state_descriptor.colorAttachments[0].blendingEnabled = YES;
        pipeline_state_descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        pipeline_state_descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pipeline_state_descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
        pipeline_state_descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        
        renderer->pipeline_state = [renderer->device newRenderPipelineStateWithDescriptor:pipeline_state_descriptor
                error:&error];
    }
    
    Assert(error == nil);
    
    // NOTE(yuval): Create the command queue
    renderer->command_queue = [renderer->device newCommandQueue];
    
    // NOTE(yuval): Create the vertex buffer
    {
        u32 buffer_size = (metal_max_vertices * sizeof(Render_Vertex));
        MTLResourceOptions options = MTLCPUCacheModeWriteCombined|MTLResourceStorageModeManaged;
        renderer->buffer = [renderer->device newBufferWithLength:buffer_size
                options:options];
    }
    
    // NOTE(yuval): Create a capture scope for gpu frame capture
    renderer->capture_scope = [[MTLCaptureManager sharedCaptureManager]
            newCaptureScopeWithDevice:renderer->device];
    renderer->capture_scope.label = @"4coder Metal Capture Scope";
}

function void
metal_render(Metal_Renderer *renderer, Render_Target *t){
    [renderer->capture_scope beginScope];
    
    i32 width = t->width;
    i32 height = t->height;
    
    Font_Set* font_set = (Font_Set*)t->font_set;
    
    // NOTE(yuval): Create the command buffer
    id<MTLCommandBuffer> command_buffer = [renderer->command_queue commandBuffer];
    command_buffer.label = @"4coder Metal Render Command";
    
    // NOTE(yuval): Obtain the render pass descriptor from the renderer's view
    MTLRenderPassDescriptor *render_pass_descriptor = renderer->view.currentRenderPassDescriptor;
    if (render_pass_descriptor != nil){
        render_pass_descriptor.colorAttachments[0].clearColor = MTLClearColorMake(1.0f, 0.0f, 1.0f, 1.0f);
        
        // NOTE(yuval): Create the render command encoder
        id<MTLRenderCommandEncoder> render_encoder
            = [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
        render_encoder.label = @"4coder Render Encoder";
        
        // NOTE(yuval): Set the region of the drawable to draw into
        [render_encoder setViewport:(MTLViewport){0.0, 0.0, (double)width, (double)height, 0.0, 1.0}];
        
        // NOTE(yuval): Set the render pipeline to use for drawing
        [render_encoder setRenderPipelineState:renderer->pipeline_state];
        
        // NOTE(yuval): Calculate and pass in the projection matrix
        float left = 0, right = (float)width;
        float bottom = (float)height, top = 0;
        float near_depth = -1.0f, far_depth = 1.0f;
        float proj[16] = {
            2.0f / (right - left), 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f / (far_depth - near_depth), 0.0f,
            -((right + left) / (right - left)), -((top + bottom) / (top - bottom)),
            -(near_depth / (far_depth - near_depth)), 1.0f
        };
        
        for (Render_Group *group = t->group_first;
             group;
             group = group->next){
            // NOTE(yuval): Set scissor rect
            {
                Rect_i32 box = Ri32(group->clip_box);
                MTLScissorRect scissor_rect;
                
                CGSize frame = [renderer->view drawableSize];
                printf("Drawable Size - w:%f  h:%f\n", frame.width, frame.height);
                
                NSUInteger x0 = (NSUInteger)Min(Max(0, box.x0), frame.width - 1);
                NSUInteger x1 = (NSUInteger)Min(Max(0, box.x1), frame.width);
                NSUInteger y0 = (NSUInteger)Min(Max(0, box.y0), frame.height - 1);
                NSUInteger y1 = (NSUInteger)Min(Max(0, box.y1), frame.height);
                
                scissor_rect.x = x0;
                scissor_rect.y = y0;
                scissor_rect.width = (x1 - x0);
                scissor_rect.height = (y1 - y0);
                
                [render_encoder setScissorRect:scissor_rect];
            }
            
            i32 vertex_count = group->vertex_list.vertex_count;
            if (vertex_count > 0){
                // TODO(yuval): Bind a texture
                {
                    Face* face = font_set_face_from_id(font_set, group->face_id);
                    if (face != 0){
                        // TODO(yuval): Bind face texture
                    } else{
                        // TODO(yuval): Bind default texture
                    }
                }
                
                // NOTE(yuval): Copy the vertex data to the vertex buffer
                {
                    u8 *cursor = (u8*)[renderer->buffer contents];
                    for (Render_Vertex_Array_Node *node = group->vertex_list.first;
                         node;
                         node = node->next){
                        i32 size = node->vertex_count * sizeof(*node->vertices);
                        memcpy(cursor, node->vertices, size);
                        cursor += size;
                    }
                }
                
                // NOTE(yuval): Pass the vertex buffer to the vertex shader
                [render_encoder setVertexBuffer:renderer->buffer
                        offset:0
                        atIndex:0];
                
                // NOTE(yuval): Pass the projection matrix to the vertex shader
                [render_encoder setVertexBytes:&proj
                        length:sizeof(proj)
                        atIndex:1];
                
                // NOTE(yuval): Draw the vertices
                [render_encoder drawPrimitives:MTLPrimitiveTypeTriangle
                        vertexStart:0
                        vertexCount:vertex_count];
            }
        }
        
        [render_encoder endEncoding];
        
        // NOTE(yuval): Schedule a present once the framebuffer is complete using the current drawable
        [command_buffer presentDrawable:renderer->view.currentDrawable];
    }
    
    [command_buffer commit];
    
    [renderer->capture_scope endScope];
}