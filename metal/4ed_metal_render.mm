/* 4coder Metal render implementation */

#undef clamp
#undef function
#import <simd/simd.h>
#import <MetalKit/MetalKit.h>

#include "AAPLShaderTypes.h"
#define function static

////////////////////////////////

// TODO(yuval): Convert this to a struct when I handle my own caching solution
@interface Metal_Buffer : NSObject
@property (nonatomic, strong) id<MTLBuffer> buffer;
@property (nonatomic, readonly) u32 size;
@property (nonatomic, assign) NSTimeInterval last_reuse_time;

- (instancetype)initWithSize:(u32)size usingDevice:(id<MTLDevice>)device;
@end

@interface Metal_Renderer : NSObject<MTKViewDelegate>
@property (nonatomic) Render_Target *target;

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)mtkView;
- (Metal_Buffer*)get_reusable_buffer_with_size:(NSUInteger)size;
- (void)add_reusable_buffer:(Metal_Buffer*)buffer;
@end

////////////////////////////////

global_const char *metal__shaders_source = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

////////////////////////////////

typedef struct{
float2 xy [[attribute(0)]];
    float3 uvw [[attribute(1)]];
uint32_t color [[attribute(2)]];
float half_thickness [[attribute(3)]];
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
vertex_shader(Vertex in [[stage_in]],
constant float4x4 &proj [[buffer(1)]]){
    Rasterizer_Data out;
    
    // NOTE(yuval): Calculate position in NDC
    out.position = proj * float4(in.xy, 0.0, 1.0);
    
    // NOTE(yuval): Convert color to float4 format
    out.color.b = ((float((in.color       ) & 0xFFu)) / 255.0);
    out.color.g = ((float((in.color >>  8u) & 0xFFu)) / 255.0);
    out.color.r = ((float((in.color >> 16u) & 0xFFu)) / 255.0);
    out.color.a = ((float((in.color >> 24u) & 0xFFu)) / 255.0);
    
    // NOTE(yuval): Pass uvw coordinates to the fragment shader
    out.uvw = in.uvw;
    
    // NOTE(yuval): Calculate adjusted half dim
    float2 center = in.uvw.xy;
    float2 half_dim = abs(in.xy - center);
    out.adjusted_half_dim = (half_dim - in.uvw.zz + float2(0.5, 0.5));
    
    // NOTE(yuval): Pass half_thickness to the fragment shader
    out.half_thickness = in.half_thickness;
    
    // NOTE(yuval): Pass xy to the fragment shader
    out.xy = in.xy;
    
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

////////////////////////////////

@implementation Metal_Buffer
- (instancetype)initWithSize:(u32)size usingDevice:(id<MTLDevice>)device{
    self = [super init];
    if (self == nil){
        return(nil);
    }
    
    // NOTE(yuval): Create the vertex buffer
    MTLResourceOptions options = MTLCPUCacheModeWriteCombined|MTLResourceStorageModeManaged;
    _buffer = [device newBufferWithLength:size options:options];
    _size = size;
    
    // NOTE(yuval): Set the last_reuse_time to the current time
    _last_reuse_time = [NSDate date].timeIntervalSince1970;
    
    return(self);
}
@end

////////////////////////////////

@implementation Metal_Renderer{
    id<MTLDevice> device;
    id<MTLRenderPipelineState> pipeline_state;
    id<MTLCommandQueue> command_queue;
    id<MTLCaptureScope> capture_scope;
    
    NSMutableArray<Metal_Buffer*> *buffer_cache;
    NSTimeInterval last_buffer_cache_purge_time;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)mtk_view{
    self = [super init];
    if (self == nil){
        return(nil);
    }
    
    NSError *error = nil;
    
    device = mtk_view.device;
    
    // NOTE(yuval): Compile the shaders
    id<MTLFunction> vertex_function = nil;
    id<MTLFunction> fragment_function = nil;
    {
        NSString *shaders_source_str = [NSString stringWithUTF8String:metal__shaders_source];
        
        MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
        options.fastMathEnabled = YES;
        
        id<MTLLibrary> shader_library = [device newLibraryWithSource:shaders_source_str
                options:options error:&error];
        vertex_function = [shader_library newFunctionWithName:@"vertex_shader"];
        fragment_function = [shader_library newFunctionWithName:@"fragment_shader"];
        
        [options release];
    }
    
    Assert(error == nil);
    
    // NOTE(yuval): Configure the pipeline descriptor
    {
        MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
        vertexDescriptor.attributes[0].offset = OffsetOfMember(Render_Vertex, xy);
        vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2; // position
        vertexDescriptor.attributes[0].bufferIndex = 0;
        vertexDescriptor.attributes[1].offset = OffsetOfMember(Render_Vertex, uvw);
        vertexDescriptor.attributes[1].format = MTLVertexFormatFloat3; // texCoords
        vertexDescriptor.attributes[1].bufferIndex = 0;
        vertexDescriptor.attributes[2].offset = OffsetOfMember(Render_Vertex, color);
        vertexDescriptor.attributes[2].format = MTLVertexFormatUInt; // color
        vertexDescriptor.attributes[2].bufferIndex = 0;
        vertexDescriptor.attributes[3].offset = OffsetOfMember(Render_Vertex, half_thickness);
        vertexDescriptor.attributes[3].format = MTLVertexFormatFloat; // position
        vertexDescriptor.attributes[3].bufferIndex = 0;
        vertexDescriptor.layouts[0].stepRate = 1;
        vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        vertexDescriptor.layouts[0].stride = sizeof(Render_Vertex);
        
        MTLRenderPipelineDescriptor *pipeline_state_descriptor = [[MTLRenderPipelineDescriptor alloc] init];
        pipeline_state_descriptor.label = @"4coder Metal Renderer Pipeline";
        pipeline_state_descriptor.vertexFunction = vertex_function;
        pipeline_state_descriptor.fragmentFunction = fragment_function;
        pipeline_state_descriptor.vertexDescriptor = vertexDescriptor;
        pipeline_state_descriptor.colorAttachments[0].pixelFormat = mtk_view.colorPixelFormat;
        pipeline_state_descriptor.colorAttachments[0].blendingEnabled = YES;
        pipeline_state_descriptor.colorAttachments[0].alphaBlendOperation = MTLBlendOperationAdd;
        pipeline_state_descriptor.colorAttachments[0].rgbBlendOperation = MTLBlendOperationAdd;
        pipeline_state_descriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        pipeline_state_descriptor.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        /*pipeline_state_descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
        pipeline_state_descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;*/
        
        pipeline_state = [device newRenderPipelineStateWithDescriptor:pipeline_state_descriptor
                error:&error];
    }
    
    Assert(error == nil);
    
    // NOTE(yuval): Create the command queue
    command_queue = [device newCommandQueue];
    
    // NOTE(yuval): Initialize buffer caching
    buffer_cache = [NSMutableArray array];
    last_buffer_cache_purge_time = [NSDate date].timeIntervalSince1970;
    
    // NOTE(yuval): Create a capture scope for gpu frame capture
    capture_scope = [[MTLCaptureManager sharedCaptureManager]
            newCaptureScopeWithDevice:device];
    capture_scope.label = @"4coder Metal Capture Scope";
    
    return(self);
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size{
    // NOTE(yuval): Nothing to do here because we use the render target's dimentions for rendering
}

- (void)drawInMTKView:(nonnull MTKView*)view{
    printf("Metal Renderer Draw!\n");
    
    [capture_scope beginScope];
    
    i32 width = _target->width;
    i32 height = _target->height;
    
    Font_Set* font_set = (Font_Set*)_target->font_set;
    
    // NOTE(yuval): Create the command buffer
    id<MTLCommandBuffer> command_buffer = [command_queue commandBuffer];
    command_buffer.label = @"4coder Metal Render Command";
    
    // NOTE(yuval): Obtain the render pass descriptor from the renderer's view
    MTLRenderPassDescriptor *render_pass_descriptor = view.currentRenderPassDescriptor;
    if (render_pass_descriptor != nil){
        render_pass_descriptor.colorAttachments[0].clearColor = MTLClearColorMake(1.0f, 0.0f, 1.0f, 1.0f);
        
        // NOTE(yuval): Create the render command encoder
        id<MTLRenderCommandEncoder> render_encoder
            = [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
        render_encoder.label = @"4coder Render Encoder";
        
        // NOTE(yuval): Set the region of the drawable to draw into
        [render_encoder setViewport:(MTLViewport){0.0, 0.0, (double)width, (double)height, 0.0, 1.0}];
        
        // NOTE(yuval): Set the render pipeline to use for drawing
        [render_encoder setRenderPipelineState:pipeline_state];
        
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
        
        // NOTE(yuval): Calculate required vertex buffer size
        i32 all_vertex_count = 0;
        for (Render_Group *group = _target->group_first;
             group;
             group = group->next){
            all_vertex_count += group->vertex_list.vertex_count;
        }
        
        u32 vertex_buffer_size = (all_vertex_count * sizeof(Render_Vertex));
        
        // NOTE(yuval): Find & Get a vertex buffer matching the required size
        Metal_Buffer *buffer = [self get_reusable_buffer_with_size:vertex_buffer_size];
        
        // NOTE(yuval): Pass the vertex buffer to the vertex shader
        [render_encoder setVertexBuffer:buffer.buffer
                offset:0
                atIndex:0];
        
        // NOTE(yuval): Pass the projection matrix to the vertex shader
        [render_encoder setVertexBytes:&proj
                length:sizeof(proj)
                atIndex:1];
        
        u32 buffer_offset = 0;
        i32 count = 0;
        for (Render_Group *group = _target->group_first;
             group;
             group = group->next, ++count){
            // NOTE(yuval): Set scissor rect
            {
                Rect_i32 box = Ri32(group->clip_box);
                
                NSUInteger x0 = (NSUInteger)Min(Max(0, box.x0), width - 1);
                NSUInteger x1 = (NSUInteger)Min(Max(0, box.x1), width);
                NSUInteger y0 = (NSUInteger)Min(Max(0, box.y0), height - 1);
                NSUInteger y1 = (NSUInteger)Min(Max(0, box.y1), height);
                
                MTLScissorRect scissor_rect;
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
                    
                    u8 *group_buffer_contents = (u8*)[buffer.buffer contents] + buffer_offset;
                    u8 *cursor = group_buffer_contents;
                    for (Render_Vertex_Array_Node *node = group->vertex_list.first;
                         node;
                         node = node->next){
                        i32 size = node->vertex_count * sizeof(*node->vertices);
                        memcpy(cursor, node->vertices, size);
                        cursor += size;
                    }
                    
                    NSUInteger data_size = (NSUInteger)(cursor - group_buffer_contents);
                    NSRange modify_range = NSMakeRange(buffer_offset, data_size);
                    [buffer.buffer didModifyRange:modify_range];
                }
                
                // NOTE(yuval): Set the vertex buffer offset to the beginning of the group's vertices
                [render_encoder setVertexBufferOffset:buffer_offset atIndex:0];
                
                // NOTE(yuval): Draw the vertices
                [render_encoder drawPrimitives:MTLPrimitiveTypeTriangle
                        vertexStart:0
                        vertexCount:vertex_count];
                
                buffer_offset += (vertex_count * sizeof(Render_Vertex)); //((((vertex_count * sizeof(Render_Vertex)) + 256) / 256) * 256);
            }
        }
        
        [render_encoder endEncoding];
        
        // NOTE(yuval): Schedule a present once the framebuffer is complete using the current drawable
        [command_buffer presentDrawable:view.currentDrawable];
        
        [command_buffer addCompletedHandler:^(id<MTLCommandBuffer>){
                dispatch_async(dispatch_get_main_queue(), ^{
                               [self add_reusable_buffer:buffer];
                               });
            }];
    }
    
    // NOTE(yuval): Finalize rendering here and push the command buffer to the GPU
    [command_buffer commit];
    
    [capture_scope endScope];
}

- (Metal_Buffer*)get_reusable_buffer_with_size:(NSUInteger)size{
    // NOTE(yuval): This routine is a modified version of Dear ImGui's MetalContext::dequeueReusableBufferOfLength in imgui_impl_metal.mm
    
    NSTimeInterval now = [NSDate date].timeIntervalSince1970;
    
    // NOTE(yuval): Purge old buffers that haven't been useful for a while
    if ((now - last_buffer_cache_purge_time) > 1.0){
        NSMutableArray *survivors = [NSMutableArray array];
        for (Metal_Buffer *candidate in buffer_cache){
            if (candidate.last_reuse_time > last_buffer_cache_purge_time){
                [survivors addObject:candidate];
            }
        }
        
        buffer_cache = [survivors mutableCopy];
        last_buffer_cache_purge_time = now;
    }
    
    // NOTE(yuval): See if we have a buffer we can reuse
    Metal_Buffer *best_candidate = 0;
    for (Metal_Buffer *candidate in buffer_cache){
        if ((candidate.size >= size) && ((!best_candidate) || (best_candidate.last_reuse_time > candidate.last_reuse_time))){
            best_candidate = candidate;
        }
    }
    
    Metal_Buffer *result;
    if (best_candidate){
        [buffer_cache removeObject:best_candidate];
        best_candidate.last_reuse_time = now;
        result = best_candidate;
    } else{
        result = [[Metal_Buffer alloc] initWithSize:size usingDevice:device];
    }
    
    // NOTE(yuval): No luck; make a new buffer
    
    return result;
}

- (void)add_reusable_buffer:(Metal_Buffer*)buffer{
    // NOTE(yuval): This routine is a modified version of Dear ImGui's MetalContext::enqueueReusableBuffer in imgui_impl_metal.mm
    
    [buffer_cache addObject:buffer];
}
@end
