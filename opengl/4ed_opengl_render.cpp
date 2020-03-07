/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * OpenGL render implementation
 *
 */

// TOP

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

internal void
gl__error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void *userParam){
    switch (id){
        case 131218:
        {
            // NOTE(allen): performance warning, do nothing.
        }break;
        
        default:
        {
            InvalidPath;
        }break;
    }
}

char *gl__header = R"foo(#version 130
        )foo";

char *gl__vertex = R"foo(
        uniform vec2 view_t;
        uniform mat2x2 view_m;
        in vec2 vertex_p;
        in vec3 vertex_t;
        in uint vertex_c;
        in float vertex_ht;
        smooth out vec4 fragment_color;
        smooth out vec3 uvw;
        smooth out vec2 xy;
        smooth out vec2 adjusted_half_dim;
        smooth out float half_thickness;
        void main(void)
        {
        gl_Position = vec4(view_m*(vertex_p - view_t), 0.0, 1.0);
         fragment_color.b = (float((vertex_c     )&0xFFu))/255.0;
        fragment_color.g = (float((vertex_c>> 8u)&0xFFu))/255.0;
        fragment_color.r = (float((vertex_c>>16u)&0xFFu))/255.0;
        fragment_color.a = (float((vertex_c>>24u)&0xFFu))/255.0;
        uvw = vertex_t;
        vec2 center = vertex_t.xy;
        vec2 half_dim = abs(vertex_p - center);
        adjusted_half_dim = half_dim - vertex_t.zz + vec2(0.5, 0.5);
        half_thickness = vertex_ht;
        xy = vertex_p;
        }
        )foo";

char *gl__fragment = R"foo(
        smooth in vec4 fragment_color;
        smooth in vec3 uvw;
        smooth in vec2 xy;
        smooth in vec2 adjusted_half_dim;
        smooth in float half_thickness;
        uniform sampler2DArray sampler;
        out vec4 out_color;
        
        float rectangle_sd(vec2 p, vec2 b){
        vec2 d = abs(p) - b;
        return(length(max(d, vec2(0.0, 0.0))) + min(max(d.x, d.y), 0.0));
        }
        
        void main(void)
        {
        float has_thickness = (step(0.49, half_thickness));
        float does_not_have_thickness = 1.0 - has_thickness;
        
        float sample_value = texture(sampler, uvw).r;
        sample_value *= does_not_have_thickness;
        
        vec2 center = uvw.xy;
        float roundness = uvw.z;
        float sd = rectangle_sd(xy - center, adjusted_half_dim);
        sd = sd - roundness;
        sd = abs(sd + half_thickness) - half_thickness;
        float shape_value = 1.0 - smoothstep(-1.0, 0.0, sd);
        shape_value *= has_thickness;
        
        out_color = vec4(fragment_color.xyz, fragment_color.a*(sample_value + shape_value));
        }
        )foo";

#define AttributeList(X) \
X(vertex_p) \
X(vertex_t) \
X(vertex_c) \
X(vertex_ht)

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
        os_popup_error("Error", "Shader compilation failed.");
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
gl_render(Render_Target *t){
    Font_Set *font_set = (Font_Set*)t->font_set;
    
    local_persist b32 first_opengl_call = true;
    local_persist u32 attribute_buffer = 0;
    local_persist GL_Program gpu_program = {};
    
    if (first_opengl_call){
        first_opengl_call = false;
        
#if !SHIP_MODE
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        if (glDebugMessageControl){
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, false);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, 0, false);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, 0, true);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, 0, true);
        }
        
        if (glDebugMessageCallback){
            glDebugMessageCallback(gl__error_callback, 0);
        }
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
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
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
    
    for (Render_Group *group = t->group_first;
         group != 0;
         group = group->next){
        Rect_i32 box = Ri32(group->clip_box);
        
        Rect_i32 scissor_box = {
            box.x0, height - box.y1, box.x1 - box.x0, box.y1 - box.y0,
        };
        scissor_box.x0 = clamp_bot(0, scissor_box.x0);
        scissor_box.y0 = clamp_bot(0, scissor_box.y0);
        scissor_box.x1 = clamp_bot(0, scissor_box.x1);
        scissor_box.y1 = clamp_bot(0, scissor_box.y1);
        glScissor(scissor_box.x0, scissor_box.y0, scissor_box.x1, scissor_box.y1);
        
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
            glEnableVertexAttribArray(gpu_program.vertex_ht);
            
            glVertexAttribPointer(gpu_program.vertex_p, 2, GL_FLOAT, true, sizeof(Render_Vertex),
                                  GLOffset(Render_Vertex, xy));
            glVertexAttribPointer(gpu_program.vertex_t, 3, GL_FLOAT, true, sizeof(Render_Vertex),
                                  GLOffset(Render_Vertex, uvw));
            glVertexAttribIPointer(gpu_program.vertex_c, 1, GL_UNSIGNED_INT, sizeof(Render_Vertex),
                                   GLOffset(Render_Vertex, color));
            glVertexAttribPointer(gpu_program.vertex_ht, 1, GL_FLOAT, true, sizeof(Render_Vertex),
                                  GLOffset(Render_Vertex, half_thickness));
            
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
            glDisableVertexAttribArray(gpu_program.vertex_ht);
        }
    }
    
    glFlush();
}

// BOTTOM

