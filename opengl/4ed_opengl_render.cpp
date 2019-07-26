/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * OpenGL render implementation
 *
 */

// TOP

//#define GL_FUNC(N,R,P) typedef R (N##_Function) P; N##_Function *P = 0;
//#include "4ed_opengl_funcs.h"
#include "4ed_opengl_defines.h"

internal void
gl__bind_texture(Render_Target *t, i32 texid){
    if (t->bound_texture != texid){
        glBindTexture(GL_TEXTURE_2D_ARRAY, texid);
        t->bound_texture = texid;
    }
}

internal void
gl__bind_any_texture(Render_Target *t){
    if (t->bound_texture == 0){
        Assert(t->fallback_texture_id != 0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, t->fallback_texture_id);
        t->bound_texture = t->fallback_texture_id;
    }
}

internal u32
gl__get_texture(Vec3_i32 dim, Texture_Kind texture_kind){
    u32 tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, dim.x, dim.y, dim.z, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
    return(tex);
}

internal b32
gl__fill_texture(Texture_Kind texture_kind, u32 texture, Vec3_i32 p, Vec3_i32 dim, void *data){
    b32 result = false;
    if (texture != 0){
        glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    }
    if (dim.x > 0 && dim.y > 0 && dim.z > 0){
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, p.x, p.y, p.z, dim.x, dim.y, dim.z, GL_RED, GL_UNSIGNED_BYTE, data);
    }
    return(result);
}

internal void CALL_CONVENTION
gl__error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, char *message, void *userParam){
    InvalidPath;
}

char *gl__header = R"foo(#version 150
)foo";

char *gl__vertex = R"foo(
uniform vec2 view_t;
uniform mat2x2 view_m;
in vec2 vertex_p;
in vec3 vertex_t;
in vec4 vertex_c;
smooth out vec4 fragment_color;
smooth out vec3 uvw;
void main(void)
{
gl_Position = vec4(view_m*(vertex_p - view_t), 0.f, 1.f);
fragment_color = vertex_c;
uvw = vertex_t;
}
)foo";

char *gl__fragment = R"foo(
smooth in vec4 fragment_color;
smooth in vec3 uvw;
uniform sampler2DArray sampler;
out vec4 out_color;
void main(void)
{
out_color = fragment_color*texture(sampler, uvw).r;
}
)foo";

#define AttributeList(X) \
X(vertex_p) \
X(vertex_t) \
X(vertex_c)

#define UniformList(X) \
X(view_t) \
X(view_m) \
X(sampler)

struct GL_Program{
    u32 program;
#define GetAttributeLocation(N) i32 N;
    AttributeList(GetAttributeLocation)
#undef GetAttributeLocation
#define GetUniformLocation(N) i32 N;
        UniformList(GetUniformLocation)
#undef GetUniformLocation
};

internal GL_Program
gl__make_program(char *header, char *vertex, char *fragment){
    if (header == 0){
        header = "";
    }
    
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLchar *vertex_source_array[] = { header, vertex };
    glShaderSource(vertex_shader, ArrayCount(vertex_source_array), vertex_source_array, 0);
    glCompileShader(vertex_shader);
    
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar *fragment_source_array[] = { header, fragment };
    glShaderSource(fragment_shader, ArrayCount(fragment_source_array), fragment_source_array, 0);
    glCompileShader(fragment_shader);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glValidateProgram(program);
    
    GLint success = false;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success){
        GLsizei ignore = 0;
        char vertex_errors[KB(4)];
        char fragment_errors[KB(4)];
        char program_errors[KB(4)];
        glGetShaderInfoLog(vertex_shader, sizeof(vertex_errors), &ignore, vertex_errors);
        glGetShaderInfoLog(fragment_shader, sizeof(fragment_errors), &ignore, fragment_errors);
        glGetProgramInfoLog(program, sizeof(program_errors), &ignore, program_errors);
#if SHIP_MODE
        os_popup_window(string_u8_litexpr("Error"), string_u8_litexpr("Shader compilation failed."));
#endif
        InvalidPath;
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    GL_Program result = {};
    result.program = program;
#define GetAttributeLocation(N) result.N = glGetAttribLocation(program, #N);
    AttributeList(GetAttributeLocation)
#undef GetAttributeLocation
#define GetUniformLocation(N) result.N = glGetUniformLocation(program, #N);
        UniformList(GetUniformLocation)
#undef GetUniformLocation
        return(result);
}

#define GLOffsetStruct(p,m) ((void*)(OffsetOfMemberStruct(p,m)))
#define GLOffset(S,m) ((void*)(OffsetOfMember(S,m)))

internal void
gl_render(Render_Target *t, Arena *scratch){
    Font_Set *font_set = (Font_Set*)t->font_set;
    
    local_persist b32 first_opengl_call = true;
    local_persist u32 attribute_buffer = 0;
    local_persist GL_Program gpu_program = {};
    
    if (first_opengl_call){
        first_opengl_call = false;
        
#if !SHIP_MODE
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, false);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, 0, true);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, 0, true);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, 0, true);
        glDebugMessageCallback(gl__error_callback, 0);
#endif
        
        ////////////////////////////////
        
        GLuint dummy_vao = 0;
        glGenVertexArrays(1, &dummy_vao);
        glBindVertexArray(dummy_vao);
        
        ////////////////////////////////
        
        glGenBuffers(1, &attribute_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, attribute_buffer);
        
        ////////////////////////////////
        
        glEnable(GL_SCISSOR_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        
        ////////////////////////////////
        
        gpu_program = gl__make_program(gl__header, gl__vertex, gl__fragment);
        glUseProgram(gpu_program.program);
        
        ////////////////////////////////
        
        {        
            t->fallback_texture_id = gl__get_texture(V3i32(2, 2, 1), TextureKind_Mono);
            u8 white_block[] = { 0xFF, 0xFF, 0xFF, 0xFF, };
            gl__fill_texture(TextureKind_Mono, 0, V3i32(0, 0, 0), V3i32(2, 2, 1), white_block);
        }
    }
    
    i32 width = t->width;
    i32 height = t->height;
    
    glViewport(0, 0, width, height);
    glScissor(0, 0, width, height);
    glClearColor(1.f, 0.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    t->bound_texture = 0;
    
    for (Render_Free_Texture *free_texture = t->free_texture_first;
         free_texture != 0;
         free_texture = free_texture->next){
        glDeleteTextures(1, &free_texture->tex_id);
    }
    t->free_texture_first = 0;
    t->free_texture_last = 0;
    
#if 0
    i32 glyph_counter = 0;
    
    u8 *start = (u8*)t->buffer.base;
    u8 *end = (u8*)t->buffer.base + t->buffer.pos;
    Render_Command_Header *header = 0;
    for (u8 *p = start; p < end; p += header->size){
        header = (Render_Command_Header*)p;
        
        i32 type = header->type;
        switch (type){
            case RenCom_Rectangle:
            {
                Render_Command_Rectangle *rectangle = (Render_Command_Rectangle*)header;
                gl__bind_any_texture(t);
                
                i32 vertex_count = ArrayCount(rectangle->vertices);
                glBufferData(GL_ARRAY_BUFFER, sizeof(rectangle->vertices), rectangle->vertices, GL_STREAM_DRAW);
                glEnableVertexAttribArray(gpu_program.vertex_p);
                glEnableVertexAttribArray(gpu_program.vertex_t);
                glEnableVertexAttribArray(gpu_program.vertex_c);
                //glVertexAttribPointer(gpu_program.vertex_p, 2, GL_FLOAT, true, sizeof(rectangle->vertices[0]), 0);
                Render_Vertex *vertices = rectangle->vertices;
                glVertexAttribPointer(gpu_program.vertex_p, 2, GL_FLOAT, true, sizeof(*vertices), GLOffset(vertices, xy));
                glVertexAttribPointer(gpu_program.vertex_t, 3, GL_FLOAT, true, sizeof(*vertices), GLOffset(vertices, uvw));
                glVertexAttribPointer(gpu_program.vertex_c, 4, GL_FLOAT, true, sizeof(*vertices), GLOffset(vertices, color));
                
                glUniform2f(gpu_program.view_t, width/2.f, height/2.f);
                f32 m[4] = {
                    2.f/width, 0.f,
                    0.f, -2.f/height,
                };
                glUniformMatrix2fv(gpu_program.view_m, 1, GL_FALSE, m);
                glUniform1i(gpu_program.sampler, 0);
                
                glDrawArrays(GL_TRIANGLES, 0, vertex_count);
                glDisableVertexAttribArray(gpu_program.vertex_p);
                glDisableVertexAttribArray(gpu_program.vertex_t);
                glDisableVertexAttribArray(gpu_program.vertex_c);
            }break;
            
            case RenCom_Glyph:
            {
#if 1
                if ((glyph_counter += 1) > 10){
                    break;
                }
#endif
                
                Render_Command_Glyph *glyph = (Render_Command_Glyph*)header;
                Face *face = font_set_face_from_id(font_set, glyph->font_id);
                if (face == 0){
                    break;
                }
                
                u32 codepoint = glyph->codepoint;
                
                u16 glyph_index = 0;
                if (!codepoint_index_map_read(&face->codepoint_to_index_map, codepoint, &glyph_index)){
                    glyph_index = 0;
                }
                Glyph_Bounds bounds = face->bounds[glyph_index];
                GLuint tex = face->texture;
                Vec3_f32 texture_dim = face->texture_dim;
                
                f32 x = glyph->pos.x;
                f32 y = glyph->pos.y;
                
                f32_Rect uv = Rf32(bounds.uv.x0, bounds.uv.y0,
                                   bounds.uv.x1, bounds.uv.y1);
                f32_Rect xy = Rf32(x + bounds.xy_off.x0, y + bounds.xy_off.y0,
                                   x + bounds.xy_off.x1, y + bounds.xy_off.y1);
                
                Render_Vertex vertices[4];
                if (!HasFlag(glyph->flags, GlyphFlag_Rotate90)){
                    vertices[0].xy = V2(xy.x0, xy.y1); vertices[0].uvw = V3(uv.x0, uv.y1, bounds.w);
                    vertices[1].xy = V2(xy.x1, xy.y1); vertices[1].uvw = V3(uv.x1, uv.y1, bounds.w);
                    vertices[2].xy = V2(xy.x0, xy.y0); vertices[2].uvw = V3(uv.x0, uv.y0, bounds.w);
                    vertices[3].xy = V2(xy.x1, xy.y0); vertices[3].uvw = V3(uv.x1, uv.y0, bounds.w);
                }
                else{
                    vertices[0].xy = V2(xy.x0, xy.y1); vertices[0].uvw = V3(uv.x1, uv.y1, bounds.w);
                    vertices[1].xy = V2(xy.x1, xy.y1); vertices[1].uvw = V3(uv.x1, uv.y0, bounds.w);
                    vertices[2].xy = V2(xy.x0, xy.y0); vertices[2].uvw = V3(uv.x0, uv.y1, bounds.w);
                    vertices[3].xy = V2(xy.x1, xy.y0); vertices[3].uvw = V3(uv.x0, uv.y0, bounds.w);
                }
                
                Vec4 c = unpack_color4(glyph->color);
                for (i32 i = 0; i < 4; i += 1){
                    vertices[i].color = c;
                }
                
                gl__bind_texture(t, tex);
                
                i32 vertex_count = ArrayCount(vertices);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
                glEnableVertexAttribArray(gpu_program.vertex_p);
                glEnableVertexAttribArray(gpu_program.vertex_t);
                glEnableVertexAttribArray(gpu_program.vertex_c);
                glVertexAttribPointer(gpu_program.vertex_p, 2, GL_FLOAT, true, sizeof(vertices[0]), GLOffset(&vertices[0], xy));
                glVertexAttribPointer(gpu_program.vertex_t, 3, GL_FLOAT, true, sizeof(vertices[0]), GLOffset(&vertices[0], uvw));
                glVertexAttribPointer(gpu_program.vertex_c, 4, GL_FLOAT, true, sizeof(vertices[0]), GLOffset(&vertices[0], color));
                
                glUniform2f(gpu_program.view_t, width/2.f, height/2.f);
                f32 m[4] = {
                    2.f/width, 0.f,
                    0.f, -2.f/height,
                };
                glUniformMatrix2fv(gpu_program.view_m, 1, GL_FALSE, m);
                glUniform1i(gpu_program.sampler, 0);
                
                glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
                glDisableVertexAttribArray(gpu_program.vertex_p);
                glDisableVertexAttribArray(gpu_program.vertex_t);
                glDisableVertexAttribArray(gpu_program.vertex_c);
                
#if 0
                if (codepoint != ' ' && font.settings->parameters.underline){
                    // TODO(allen): recover this feature
                    glDisable(GL_TEXTURE_2D);
                    
                    f32 x0 = x;
                    f32 x1 = x + page->advance[glyph_index];
                    f32 yoff1 = y + font.metrics->underline_yoff1;
                    f32 yoff2 = y + font.metrics->underline_yoff2;
                    
                    glBegin(GL_QUADS);
                    {
                        glVertex2f(x0, yoff1);
                        glVertex2f(x1, yoff1);
                        glVertex2f(x1, yoff2);
                        glVertex2f(x0, yoff2);
                    }
                    glEnd();
                    
                    glEnable(GL_TEXTURE_2D);
                }
            }break;
#endif
            
            case RenCom_ChangeClip:
            {
                Render_Command_Change_Clip *clip = (Render_Command_Change_Clip*)header;
                i32_Rect box = clip->box;
                glScissor(box.x0, height - box.y1, box.x1 - box.x0, box.y1 - box.y0);
            }break;
            
            case RenCom_Group:
            {
                Render_Command_Group *group = (Render_Command_Group*)header;
                Rect_i32 box = group->clip_box;
                glScissor(box.x0, height - box.y1, box.x1 - box.x0, box.y1 - box.y0);
                
                
            }break;
        }
    }
#else
    for (Render_Group *group = t->group_first;
         group != 0;
         group = group->next){
        Rect_i32 box = group->clip_box;
        glScissor(box.x0, height - box.y1, box.x1 - box.x0, box.y1 - box.y0);
        
        i32 vertex_count = group->vertex_list.vertex_count;
        if (vertex_count > 0){
            Face *face = font_set_face_from_id(font_set, group->face_id);
            if (face != 0){
                gl__bind_texture(t, face->texture);
            }
            else{
                gl__bind_any_texture(t);
            }
            
            glBufferData(GL_ARRAY_BUFFER, vertex_count*sizeof(Render_Vertex), 0, GL_STREAM_DRAW);
            i32 cursor = 0;
            for (Render_Vertex_Array_Node *node = group->vertex_list.first;
                 node != 0;
                 node = node->next){
                i32 size = node->vertex_count*sizeof(*node->vertices);
                glBufferSubData(GL_ARRAY_BUFFER, cursor, size, node->vertices);
                cursor += size;
            }
            
            glEnableVertexAttribArray(gpu_program.vertex_p);
            glEnableVertexAttribArray(gpu_program.vertex_t);
            glEnableVertexAttribArray(gpu_program.vertex_c);
            glVertexAttribPointer(gpu_program.vertex_p, 2, GL_FLOAT, true, sizeof(Render_Vertex), GLOffset(Render_Vertex, xy));
            glVertexAttribPointer(gpu_program.vertex_t, 3, GL_FLOAT, true, sizeof(Render_Vertex), GLOffset(Render_Vertex, uvw));
            glVertexAttribPointer(gpu_program.vertex_c, 4, GL_FLOAT, true, sizeof(Render_Vertex), GLOffset(Render_Vertex, color));
            
            glUniform2f(gpu_program.view_t, width/2.f, height/2.f);
            f32 m[4] = {
                2.f/width, 0.f,
                0.f, -2.f/height,
            };
            glUniformMatrix2fv(gpu_program.view_m, 1, GL_FALSE, m);
            glUniform1i(gpu_program.sampler, 0);
            
            glDrawArrays(GL_TRIANGLES, 0, vertex_count);
            glDisableVertexAttribArray(gpu_program.vertex_p);
            glDisableVertexAttribArray(gpu_program.vertex_t);
            glDisableVertexAttribArray(gpu_program.vertex_c);
        }
    }
#endif
    
    glFlush();
}

// BOTTOM

