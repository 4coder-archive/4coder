/* 4coder Metal render implementation */

#undef clamp
#undef function
#import <simd/simd.h>
#import <MetalKit/MetalKit.h>
#define function static

////////////////////////////////

struct Metal_Buffer{
    Node node;
    
    id<MTLBuffer> buffer;
    u32 size;
    u64 last_reuse_time;
};

////////////////////////////////

typedef id<MTLTexture> Metal_Texture;

// NOTE(yuval): This is a locator used to describe where a specific slot is located.
union Metal_Texture_Slot_Locator{
    u32 packed;
    
    struct{
        u16 bucket_index;
        u16 slot_index;
    };
};

// NOTE(yuval): This is the ACTUAL texture slot. Each slot contains the texture handle, the slot locator, and a pointer to the next slot in the free list (in case the slot if not occupied).
struct Metal_Texture_Slot{
    // NOTE(yuval): This is a pointer to the next texture in the free texture slots list
    Metal_Texture_Slot *next;
    
    Metal_Texture texture;
    Metal_Texture_Slot_Locator locator;
};

global_const u32 metal__texture_slots_per_bucket = 256;

// NOTE(yuval): This a bucket of ACTUAL texture slots.
struct Metal_Texture_Slot_Bucket{
    Metal_Texture_Slot_Bucket *next;
    Metal_Texture_Slot slots[metal__texture_slots_per_bucket];
};

// NOTE(yuval): This a struct contaning all texture slot buckets and a list of the currently free slots.
struct Metal_Texture_Slot_List{
    Metal_Texture_Slot_Bucket *first_bucket;
    Metal_Texture_Slot_Bucket *last_bucket;
    u16 bucket_count;
    
    Metal_Texture_Slot *first_free_slot;
    Metal_Texture_Slot *last_free_slot;
};

global_const u32 metal__invalid_texture_slot_locator = (u32)-1;

////////////////////////////////

@interface Metal_Renderer : NSObject<MTKViewDelegate>
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)mtkView target:(Render_Target*)target;

- (u32)get_texture_of_dim:(Vec3_i32)dim kind:(Texture_Kind)kind;
- (b32)fill_texture:(u32)texture kind:(Texture_Kind)kind pos:(Vec3_i32)p dim:(Vec3_i32)dim data:(void*)data;
- (void)bind_texture:(u32)handle encoder:(id<MTLRenderCommandEncoder>)render_encoder;
- (Metal_Texture_Slot*)get_texture_slot_at_locator:(Metal_Texture_Slot_Locator)locator;
- (Metal_Texture_Slot*)get_texture_slot_at_handle:(u32)handle;

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
fragment_shader(Rasterizer_Data in [[stage_in]],
texture2d_array<half> in_texture [[texture(0)]]){
float has_thickness = step(0.49, in.half_thickness);
float does_not_have_thickness = (1.0 - has_thickness);

constexpr sampler texture_sampler(coord::normalized, min_filter::linear, mag_filter::linear, mip_filter::linear);
half sample_value = in_texture.sample(texture_sampler, in.uvw.xy, in.uvw.z).r;
sample_value *= does_not_have_thickness;

float2 center = in.uvw.xy;
float roundness = in.uvw.z;
float sd = rectangle_sd(in.xy - center, in.adjusted_half_dim);
sd = sd - roundness;
sd = (abs(sd + in.half_thickness) - in.half_thickness);
float shape_value = (1.0 - smoothstep(-1.0, 0.0, sd));
shape_value *= has_thickness;

float4 out_color = float4(in.color.xyz, in.color.a * (sample_value + shape_value));
return(out_color);
}
)";

////////////////////////////////

function Metal_Buffer*
metal__make_buffer(u32 size, id<MTLDevice> device){
    Metal_Buffer *result = (Metal_Buffer*)malloc(sizeof(Metal_Buffer));
    
    // NOTE(yuval): Create the vertex buffer
    MTLResourceOptions options = MTLCPUCacheModeWriteCombined|MTLResourceStorageModeManaged;
    result->buffer = [device newBufferWithLength:size options:options];
    result->size = size;
    
    // NOTE(yuval): Set the last_reuse_time to the current time
    result->last_reuse_time = system_now_time();
    
    return result;
}

////////////////////////////////

@implementation Metal_Renderer{
    Render_Target *_target;
    
    id<MTLDevice> _device;
    id<MTLRenderPipelineState> _pipeline_state;
    id<MTLCommandQueue> _command_queue;
    id<MTLCaptureScope> _capture_scope;
    
    Node _buffer_cache;
    u64 _last_buffer_cache_purge_time;
    
    Metal_Texture_Slot_List _texture_slots;
}

- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)mtk_view target:(Render_Target*)target{
    self = [super init];
    if (self == nil){
        return(nil);
    }
    
    _target = target;
    
    NSError *error = nil;
    
    _device = mtk_view.device;
    
    // NOTE(yuval): Compile the shaders
    id<MTLFunction> vertex_function = nil;
    id<MTLFunction> fragment_function = nil;
    {
        NSString *shaders_source_str = [NSString stringWithUTF8String:metal__shaders_source];
        
        MTLCompileOptions *options = [[MTLCompileOptions alloc] init];
        options.fastMathEnabled = YES;
        
        id<MTLLibrary> shader_library = [_device newLibraryWithSource:shaders_source_str
                options:options error:&error];
        vertex_function = [shader_library newFunctionWithName:@"vertex_shader"];
        fragment_function = [shader_library newFunctionWithName:@"fragment_shader"];
        
        [options release];
    }
    
    Assert(error == nil);
    Assert((vertex_function != nil) && (fragment_function != nil));
    
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
        pipeline_state_descriptor.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
        pipeline_state_descriptor.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        
        _pipeline_state = [_device newRenderPipelineStateWithDescriptor:pipeline_state_descriptor
                error:&error];
    }
    
    Assert(error == nil);
    
    // NOTE(yuval): Create the command queue
    _command_queue = [_device newCommandQueue];
    
    // NOTE(yuval): Initialize buffer caching
    dll_init_sentinel(&_buffer_cache);
    _last_buffer_cache_purge_time = system_now_time();
    
    // NOTE(yuval): Initialize the texture slot list
    block_zero_struct(&_texture_slots);
    
    // NOTE(yuval): Create the fallback texture
    _target->fallback_texture_id = [self get_texture_of_dim:V3i32(2, 2, 1)
            kind:TextureKind_Mono];
    u8 white_block[] = {0xFF, 0xFF, 0xFF, 0xFF};
    [self fill_texture:_target->fallback_texture_id
            kind:TextureKind_Mono
            pos:V3i32(0, 0, 0)
            dim:V3i32(2, 2, 1)
            data:white_block];
    
    // NOTE(yuval): Create a capture scope for gpu frame capture
    _capture_scope = [[MTLCaptureManager sharedCaptureManager]
            newCaptureScopeWithDevice:_device];
    _capture_scope.label = @"4coder Metal Capture Scope";
    
    return(self);
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size{
    // NOTE(yuval): Nothing to do here because we use the render target's dimentions for rendering
}

- (void)drawInMTKView:(nonnull MTKView*)view{
#if FRED_INTERNAL
    [_capture_scope beginScope];
#endif
    
    // HACK(yuval): This is the best way I found to force valid width and height without drawing on the next draw cycle (1 frame delay).
    CGSize drawable_size = [view drawableSize];
    i32 width = (i32)Min(_target->width, drawable_size.width);
    i32 height = (i32)Min(_target->height, drawable_size.height);
    
    Font_Set *font_set = (Font_Set*)_target->font_set;
    
    // NOTE(yuval): Free any textures in the target's texture free list
    for (Render_Free_Texture *free_texture = _target->free_texture_first;
         free_texture;
         free_texture = free_texture->next){
        Metal_Texture_Slot *texture_slot = [self get_texture_slot_at_handle:free_texture->tex_id];
        if (texture_slot){
            sll_queue_push(_texture_slots.first_free_slot, _texture_slots.last_free_slot, texture_slot);
        }
    }
    _target->free_texture_first = 0;
    _target->free_texture_last = 0;
    
    // NOTE(yuval): Create the command buffer
    id<MTLCommandBuffer> command_buffer = [_command_queue commandBuffer];
    command_buffer.label = @"4coder Metal Render Command";
    
    // NOTE(yuval): Obtain the render pass descriptor from the renderer's view
    MTLRenderPassDescriptor *render_pass_descriptor = view.currentRenderPassDescriptor;
    if (render_pass_descriptor != nil){
        render_pass_descriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0f, 0.0f, 0.0f, 1.0f);
        
        // NOTE(yuval): Create the render command encoder
        id<MTLRenderCommandEncoder> render_encoder =
            [command_buffer renderCommandEncoderWithDescriptor:render_pass_descriptor];
        render_encoder.label = @"4coder Render Encoder";
        
        // NOTE(yuval): Set the region of the drawable to draw into
        [render_encoder setViewport:(MTLViewport){0.0, 0.0, (double)width, (double)height, 0.0, 1.0}];
        
        // NOTE(yuval): Set the render pipeline to use for drawing
        [render_encoder setRenderPipelineState:_pipeline_state];
        
        // NOTE(yuval): Calculate the projection matrix
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
        [render_encoder setVertexBuffer:buffer->buffer
                offset:0
                atIndex:0];
        
        // NOTE(yuval): Pass the projection matrix to the vertex shader
        [render_encoder setVertexBytes:&proj
                length:sizeof(proj)
                atIndex:1];
        
        u32 buffer_offset = 0;
        for (Render_Group *group = _target->group_first;
             group;
             group = group->next){
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
                // NOTE(yuval): Bind a texture
                {
                    Face* face = font_set_face_from_id(font_set, group->face_id);
                    if (face != 0){
                        // NOTE(yuval): Bind face texture
                        [self bind_texture:face->texture
                                encoder:render_encoder];
                    } else{
                        // NOTE(yuval): Bind fallback texture
                        [self bind_texture:_target->fallback_texture_id
                                encoder:render_encoder];
                    }
                }
                
                // NOTE(yuval): Copy the vertex data to the vertex buffer
                {
                    
                    u8 *group_buffer_contents = (u8*)[buffer->buffer contents] + buffer_offset;
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
                    [buffer->buffer didModifyRange:modify_range];
                }
                
                // NOTE(yuval): Set the vertex buffer offset to the beginning of the group's vertices
                [render_encoder setVertexBufferOffset:buffer_offset atIndex:0];
                
                // NOTE(yuval): Draw the vertices
                [render_encoder drawPrimitives:MTLPrimitiveTypeTriangle
                        vertexStart:0
                        vertexCount:vertex_count];
                
                buffer_offset += (vertex_count * sizeof(Render_Vertex));
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
    
#if FRED_INTERNAL
    [_capture_scope endScope];
#endif
}

- (u32)get_texture_of_dim:(Vec3_i32)dim kind:(Texture_Kind)kind{
    u32 handle = metal__invalid_texture_slot_locator;
    
    // NOTE(yuval): Check for a free texture slot and allocate another slot bucket if no free slot has been found
    if (!_texture_slots.first_free_slot){
        // NOTE(yuval): Assert that the next bucket's index can fit in a u16
        Assert(_texture_slots.bucket_count < ((u16)-1));
        
        Metal_Texture_Slot_Bucket *bucket = (Metal_Texture_Slot_Bucket*)system_memory_allocate(sizeof(Metal_Texture_Slot_Bucket),  file_name_line_number_lit_u8);
        
        for (u16 slot_index = 0;
             slot_index < ArrayCount(bucket->slots);
             ++slot_index){
            Metal_Texture_Slot *slot = &bucket->slots[slot_index];
            block_zero_struct(slot);
            slot->locator.bucket_index = _texture_slots.bucket_count;
            slot->locator.slot_index = slot_index;
            
            sll_queue_push(_texture_slots.first_free_slot, _texture_slots.last_free_slot, slot);
        }
        
        sll_queue_push(_texture_slots.first_bucket, _texture_slots.last_bucket, bucket);
        _texture_slots.bucket_count += 1;
    }
    
    // NOTE(yuval): Get the first free texture slot and remove it from the free list (a slot is guarenteed to exist because we assert that above).
    if (_texture_slots.first_free_slot){
        Metal_Texture_Slot *texture_slot = _texture_slots.first_free_slot;
        sll_queue_pop(_texture_slots.first_free_slot, _texture_slots.last_free_slot);
        texture_slot->next = 0;
        
        // NOTE(yuval): Create a texture descriptor.
        MTLTextureDescriptor *texture_descriptor = [[MTLTextureDescriptor alloc] init];
        texture_descriptor.textureType = MTLTextureType2DArray;
        texture_descriptor.pixelFormat = MTLPixelFormatR8Unorm;
        texture_descriptor.width = dim.x;
        texture_descriptor.height = dim.y;
        texture_descriptor.depth = dim.z;
        
        // NOTE(yuval): Create the texture from the device using the descriptor and add it to the textures array.
        Metal_Texture texture = [_device newTextureWithDescriptor:texture_descriptor];
        texture_slot->texture = texture;
        
        handle = texture_slot->locator.packed;
    }
    
    return handle;
}

- (b32)fill_texture:(u32)handle kind:(Texture_Kind)kind pos:(Vec3_i32)p dim:(Vec3_i32)dim data:(void*)data{
    b32 result = false;
    
    if (data){
        Metal_Texture_Slot *texture_slot = [self get_texture_slot_at_handle:handle];
        if (texture_slot){
            Metal_Texture texture = texture_slot->texture;
            
            if (texture != 0){
                MTLRegion replace_region = {
                    {(NSUInteger)p.x, (NSUInteger)p.y, (NSUInteger)p.z},
                    {(NSUInteger)dim.x, (NSUInteger)dim.y, (NSUInteger)dim.z}
                };
                
                // NOTE(yuval): Fill the texture with data
                [texture replaceRegion:replace_region
                        mipmapLevel:0
                        withBytes:data
                        bytesPerRow:dim.x];
                
                result = true;
            }
        }
    }
    
    return(result);
}

- (void)bind_texture:(u32)handle encoder:(id<MTLRenderCommandEncoder>)render_encoder{
    Metal_Texture_Slot *texture_slot = [self get_texture_slot_at_handle:handle];
    if (texture_slot){
        Metal_Texture texture = texture_slot->texture;
        if (texture != 0){
            [render_encoder setFragmentTexture:texture
                    atIndex:0];
        }
    }
}

- (Metal_Texture_Slot*)get_texture_slot_at_locator:(Metal_Texture_Slot_Locator)locator{
    Metal_Texture_Slot *result = 0;
    
    if (locator.packed != metal__invalid_texture_slot_locator){
        Metal_Texture_Slot_Bucket *bucket = _texture_slots.first_bucket;
        for (u16 bucket_index = 0;
             (bucket_index < locator.bucket_index) && bucket;
             ++bucket_index, bucket = bucket->next);
        
        if (bucket && (locator.slot_index < metal__texture_slots_per_bucket)){
            result = &bucket->slots[locator.slot_index];
        }
    }
    
    return(result);
}

- (Metal_Texture_Slot*)get_texture_slot_at_handle:(u32)handle{
    Metal_Texture_Slot_Locator locator;
    locator.packed = handle;
    
    Metal_Texture_Slot *result = [self get_texture_slot_at_locator:locator];
    return(result);
}

- (Metal_Buffer*)get_reusable_buffer_with_size:(NSUInteger)size{
    // NOTE(yuval): This routine is a modified version of Dear ImGui's MetalContext::dequeueReusableBufferOfLength in imgui_impl_metal.mm
    
    u64 now = system_now_time();
    
    // NOTE(yuval): Purge old buffers that haven't been useful for a while
    if ((now - _last_buffer_cache_purge_time) > 1000000){
        Node prev_buffer_cache = _buffer_cache;
        dll_init_sentinel(&_buffer_cache);
        
        for (Node *node = prev_buffer_cache.next;
             node != &_buffer_cache;
             node = node->next){
            Metal_Buffer *candidate = CastFromMember(Metal_Buffer, node, node);
            if (candidate->last_reuse_time > _last_buffer_cache_purge_time){
                dll_insert(&_buffer_cache, node);
            }
        }
        
        _last_buffer_cache_purge_time = now;
    }
    
    // NOTE(yuval): See if we have a buffer we can reuse
    Metal_Buffer *best_candidate = 0;
    for (Node *node = _buffer_cache.next;
         node != &_buffer_cache;
         node = node->next){
        Metal_Buffer *candidate = CastFromMember(Metal_Buffer, node, node);
        if ((candidate->size >= size) && ((!best_candidate) || (best_candidate->last_reuse_time > candidate->last_reuse_time))){
            best_candidate = candidate;
        }
    }
    
    Metal_Buffer *result;
    if (best_candidate){
        // NOTE(yuval): A best candidate has been found! Remove it from the buffer list and set its last reuse time.
        dll_remove(&best_candidate->node);
        best_candidate->last_reuse_time = now;
        result = best_candidate;
    } else{
        // NOTE(yuval): No luck; make a new buffer.
        result = metal__make_buffer(size, _device);
    }
    
    return(result);
}

- (void)add_reusable_buffer:(Metal_Buffer*)buffer{
    // NOTE(yuval): This routine is a modified version of Dear ImGui's MetalContext::enqueueReusableBuffer in imgui_impl_metal.mm
    
    dll_insert(&_buffer_cache, &buffer->node);
}
@end
