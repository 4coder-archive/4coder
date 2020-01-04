/* 4coder Metal render implementation */

#undef clamp
#undef function
#import <simd/simd.h>
#import <MetalKit/MetalKit.h>

// Header shared between C code here, which executes Metal API commands, and .metal files, which
// uses these types as inputs to the shaders.
#import "AAPLShaderTypes.h"

#define function static
#define clamp(a,x,b) clamp_((a),(x),(b))

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