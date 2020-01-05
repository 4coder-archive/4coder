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
};

global_const u32 metal_max_vertices = (1<<16);

global_const char *metal__shaders_source = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Buffer index values shared between shader and C code to ensure Metal shader buffer inputs
// match Metal API buffer set calls.
typedef enum AAPLVertexInputIndex
{
    AAPLVertexInputIndexVertices     = 0,
    AAPLVertexInputIndexViewportSize = 1,
} AAPLVertexInputIndex;

//  This structure defines the layout of vertices sent to the vertex
//  shader. This header is shared between the .metal shader and C code, to guarantee that
//  the layout of the vertex array in the C code matches the layout that the .metal
//  vertex shader expects.
typedef struct
{
    vector_float2 position;
    vector_float4 color;
} AAPLVertex;

// Vertex shader outputs and fragment shader inputs
typedef struct
{
    // The [[position]] attribute of this member indicates that this value
    // is the clip space position of the vertex when this structure is
    // returned from the vertex function.
    float4 position [[position]];
    
    // Since this member does not have a special attribute, the rasterizer
    // interpolates its value with the values of the other triangle vertices
    // and then passes the interpolated value to the fragment shader for each
    // fragment in the triangle.
    float4 color;
    
} RasterizerData;

vertex RasterizerData
vertexShader(uint vertexID [[vertex_id]],
             constant AAPLVertex *vertices [[buffer(AAPLVertexInputIndexVertices)]],
             constant float4x4 &projMatrix[[buffer(AAPLVertexInputIndexViewportSize)]])
{
    RasterizerData out;
    
    // Index into the array of positions to get the current vertex.
    // The positions are specified in pixel dimensions (i.e. a value of 100
    // is 100 pixels from the origin).
    float2 pixelSpacePosition = vertices[vertexID].position.xy;
    
    // To convert from positions in pixel space to positions in clip-space,
    //  divide the pixel coordinates by half the size of the viewport.
    out.position = float4(pixelSpacePosition, 0.0, 1.0) * projMatrix;
    
    // Pass the input color directly to the rasterizer.
    out.color = vertices[vertexID].color;
    
    return out;
}

fragment float4 fragmentShader(RasterizerData in [[stage_in]])
{
    // Return the interpolated color.
    return in.color;
}
)";

@interface FCoderMetalRenderer : NSObject<MTKViewDelegate>
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView;
@end

@implementation FCoderMetalRenderer{
    id<MTLDevice> _device;
    
    // The render pipeline generated from the vertex and fragment shaders in the .metal shader file.
    id<MTLRenderPipelineState> _pipelineState;
    
    // The command queue used to pass commands to the device.
    id<MTLCommandQueue> _commandQueue;
    
    // The current size of the view, used as an input to the vertex shader.
    vector_uint2 _viewportSize;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)mtkView{
    self = [super init];
    if(self)
    {
        NSError *error = nil;
        
        _device = mtkView.device;
        
        // Load all the shader files with a .metal file extension in the project.
        id<MTLLibrary> defaultLibrary = [_device newLibraryWithFile:@"shaders/AAPLShaders.metallib"
                error:&error];
        Assert(error == nil);
        
        id<MTLFunction> vertexFunction = [defaultLibrary newFunctionWithName:@"vertexShader"];
        id<MTLFunction> fragmentFunction = [defaultLibrary newFunctionWithName:@"fragmentShader"];
        
        // Configure a pipeline descriptor that is used to create a pipeline state.
        MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipelineStateDescriptor.label = @"Simple Pipeline";
        pipelineStateDescriptor.vertexFunction = vertexFunction;
        pipelineStateDescriptor.fragmentFunction = fragmentFunction;
        pipelineStateDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat;
        
        _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                error:&error];
        
        // Pipeline State creation could fail if the pipeline descriptor isn't set up properly.
        //  If the Metal API validation is enabled, you can find out more information about what
        //  went wrong.  (Metal API validation is enabled by default when a debug build is run
        //  from Xcode.)
        NSAssert(_pipelineState, @"Failed to created pipeline state: %@", error);
        
        // Create the command queue
        _commandQueue = [_device newCommandQueue];
        
        u32 max_buffer_size = (u32)[_device maxBufferLength];
        printf("Max Buffer Size: %u - Which is %lu vertices\n", max_buffer_size, (max_buffer_size / sizeof(Render_Vertex)));
    }
    
    return self;
}

/// Called whenever view changes orientation or is resized
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size{
    // Save the size of the drawable to pass to the vertex shader.
    
}

/// Called whenever the view needs to render a frame.
- (void)drawInMTKView:(nonnull MTKView *)view{
    CGSize size = [view drawableSize];
    _viewportSize.x = size.width;
    _viewportSize.y = size.height;
    
    static const AAPLVertex triangleVertices[] =
    {
        // 2D positions,    RGBA colors
        { {  250,  -250 }, { 1, 0, 0, 1 } },
        { { -250,  -250 }, { 0, 1, 0, 1 } },
        { {    0,   250 }, { 0, 0, 1, 1 } },
    };
    
    // Create a new command buffer for each render pass to the current drawable.
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    commandBuffer.label = @"MyCommand";
    
    // Obtain a renderPassDescriptor generated from the view's drawable textures.
    MTLRenderPassDescriptor *renderPassDescriptor = view.currentRenderPassDescriptor;
    
    if(renderPassDescriptor != nil)
    {
        // Create a render command encoder.
        id<MTLRenderCommandEncoder> renderEncoder =
            [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        renderEncoder.label = @"MyRenderEncoder";
        
        // Set the region of the drawable to draw into.
        [renderEncoder setViewport:(MTLViewport){0.0, 0.0, (double)_viewportSize.x, (double)_viewportSize.y, 0.0, 1.0 }];
        
        [renderEncoder setRenderPipelineState:_pipelineState];
        
        // Pass in the parameter data.
        [renderEncoder setVertexBytes:triangleVertices
                length:sizeof(triangleVertices)
                atIndex:AAPLVertexInputIndexVertices];
        
        [renderEncoder setVertexBytes:&_viewportSize
                length:sizeof(_viewportSize)
                atIndex:AAPLVertexInputIndexViewportSize];
        
        // Draw the triangle.
        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                vertexStart:0
                vertexCount:3];
        
        [renderEncoder endEncoding];
        
        // Schedule a present once the framebuffer is complete using the current drawable.
        [commandBuffer presentDrawable:view.currentDrawable];
    }
    
    // Finalize rendering here & push the command buffer to the GPU.
    [commandBuffer commit];
}
@end

function b32
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
        vertex_function = [shader_library newFunctionWithName:@"vertexShader"];
        fragment_function = [shader_library newFunctionWithName:@"fragmentShader"];
        
        [options release];
    }
    
    if (error != nil){
        return(false);
    }
    
    // NOTE(yuval): Configure the pipeline descriptor
    {
        MTLRenderPipelineDescriptor *pipeline_state_descriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipeline_state_descriptor.label = @"4coder Metal Renderer Pipeline";
        pipeline_state_descriptor.vertexFunction = vertex_function;
        pipeline_state_descriptor.fragmentFunction = fragment_function;
        pipeline_state_descriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
        
        renderer->pipeline_state = [renderer->device newRenderPipelineStateWithDescriptor:pipeline_state_descriptor
                error:&error];
    }
    
    if (error != nil){
        return(false);
    }
    
    // NOTE(yuval): Create the command queue
    renderer->command_queue = [renderer->device newCommandQueue];
    
    // NOTE(yuval): Create the vertex buffer
    {
        u32 buffer_size = (metal_max_vertices * sizeof(Render_Vertex));
        MTLResourceOptions options = MTLCPUCacheModeWriteCombined|MTLResourceStorageModeManaged;
        renderer->buffer = [renderer->device newBufferWithLength:buffer_size
                options:options];
    }
    
    return(true);
}

function void
metal_render(Metal_Renderer *renderer, Render_Target *t){
    static const AAPLVertex triangleVertices[] = {
        // 2D positions,    RGBA colors
        { {  200,  100 }, { 1, 0, 0, 1 } },
        { { 100,  100 }, { 0, 1, 0, 1 } },
        { {    150, 200  }, { 0, 0, 1, 1 } },
    };
    
    // NOTE(yuval): Create the command buffer
    id<MTLCommandBuffer> command_buffer = [renderer->command_queue commandBuffer];
    command_buffer.label = @"4coder Metal Render Command";
    
    // NOTE(yuval): Obtain the render pass descriptor from the renderer's view
    MTLRenderPassDescriptor *render_pass_descriptor = renderer->view.currentRenderPassDescriptor;
    if (render_pass_descriptor != nil){
        // NOTE(yuval): Create the render command encoder
        id<MTLRenderCommandEncoder> render_encoder
            = [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
        render_encoder.label = @"4coder Render Encoder";
        
        // NOTE(yuval): Set the region of the drawable to draw into
        [render_encoder setViewport:(MTLViewport){0.0, 0.0, (double)t->width, (double)t->height, 0.0, 1.0}];
        
        // NOTE(yuval): Set the render pipeline to use for drawing
        [render_encoder setRenderPipelineState:renderer->pipeline_state];
        
        // NOTE(yuval): Pass in the parameter data
        [render_encoder setVertexBytes:triangleVertices
                length:sizeof(triangleVertices)
                atIndex:AAPLVertexInputIndexVertices];
        
#if 0
        vector_uint2 viewport_size = {(u32)t->width, (u32)t->height};
        [render_encoder setVertexBytes:&viewport_size
                length:sizeof(viewport_size)
                atIndex:AAPLVertexInputIndexViewportSize];
#else
        float left = 0, right = (float)t->width;
        float bottom = 0, top = (float)t->height;
        float near_depth = -1.0f, far_depth = 1.0f;
        float m[16] = {
            2.0f / (right - left), 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
            0.0f, 0.0f, -1.0f / (far_depth - near_depth), 0.0f,
            -((right + left) / (right - left)), -((top + bottom) / (top - bottom)),
            (-near_depth) * (far_depth - near_depth), 1.0f
        };
        
        float sLength = 1.0f / (right - left);
        float sHeight = 1.0f / (top   - bottom);
        float sDepth  = 1.0f / (far_depth   - near_depth);
        
        simd::float4 P;
        simd::float4 Q;
        simd::float4 R;
        simd::float4 S;
        
        P.x = 2.0f * sLength;
        P.y = 0.0f;
        P.z = 0.0f;
        P.w = -((right + left) / (right - left));
        
        Q.x = 0.0f;
        Q.y = 2.0f * sHeight;
        Q.z = 0.0f;
        Q.w = -((top + bottom) / (top - bottom));
        
        R.x = 0.0f;
        R.y = 0.0f;
        R.z = sDepth;
        R.w = -near_depth  * sDepth;
        
        S.x =  0.0f;
        S.y =  0.0f;
        S.z =  0.0f;
        S.w =  1.0f;
        
        simd_float4x4 proj = simd::float4x4(P, Q, R, S);
        
        [render_encoder setVertexBytes:&proj
                length:sizeof(proj)
                atIndex:AAPLVertexInputIndexViewportSize];
#endif
        
        // NOTE(yuval): Draw the triangle
        [render_encoder drawPrimitives:MTLPrimitiveTypeTriangle
                vertexStart:0
                vertexCount:3];
        
        [render_encoder endEncoding];
        
        // NOTE(yuval): Schedule a present once the framebuffer is complete using the current drawable
        [command_buffer presentDrawable:renderer->view.currentDrawable];
    }
    
    [command_buffer commit];
}