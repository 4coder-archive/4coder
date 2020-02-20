/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 10.11.2017
 *
 * OpenGL functions for 4coder
 *
 */

// TOP
/* Usage:
#define GL_FUNC(N,R,P) ~~~~
#include "4ed_opengl_funcs.h"
*/

#if OS_WINDOWS || OS_LINUX
GL_FUNC(glDebugMessageControl, void, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled))
GL_FUNC(glDebugMessageCallback, void, (GLDEBUGPROC callback, const void *userParam))

GL_FUNC(glGenVertexArrays,    void, (GLsizei n, GLuint *arrays))
GL_FUNC(glBindVertexArray,    void, (GLuint array))

GL_FUNC(glDeleteVertexArrays, void, (GLsizei n, const GLuint *arrays))

#if !OS_LINUX
GL_FUNC(glTexImage3D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels))
GL_FUNC(glTexSubImage3D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels))
GL_FUNC(glActiveTexture, void, (GLenum texture))
#endif

GL_FUNC(glGenBuffers, void, (GLsizei n, GLuint *buffers))
GL_FUNC(glBindBuffer, void, (GLenum target, GLuint buffer))
GL_FUNC(glBufferData, void, (GLenum target, GLsizeiptr size, const void *data, GLenum usage))
GL_FUNC(glBufferSubData, void, (GLenum target, GLsizeiptr offset, GLsizeiptr size, const void *data))

GL_FUNC(glCreateShader, GLuint, (GLenum type))
GL_FUNC(glShaderSource, void, (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length))
GL_FUNC(glCompileShader, void, (GLuint shader))
GL_FUNC(glGetShaderiv, void, (GLuint shader, GLenum pname, GLint *params))
GL_FUNC(glGetShaderInfoLog, void, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog))
GL_FUNC(glDeleteShader, void, (GLuint shader))
GL_FUNC(glCreateProgram, GLuint, (void))
GL_FUNC(glAttachShader, void, (GLuint program, GLuint shader))
GL_FUNC(glLinkProgram, void, (GLuint program))
GL_FUNC(glUseProgram, void, (GLuint program))
GL_FUNC(glGetProgramiv, void, (GLuint program, GLenum pname, GLint *params))
GL_FUNC(glGetProgramInfoLog, void, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog))
GL_FUNC(glDeleteProgram, void, (GLuint program))
GL_FUNC(glValidateProgram, void, (GLuint program))

GL_FUNC(glGetUniformLocation, GLint, (GLuint program, const GLchar *name))
GL_FUNC(glGetAttribLocation, GLint, (GLuint program, const GLchar *name))

GL_FUNC(glBindAttribLocation, void, (GLuint program, GLuint index, const GLchar *name))
GL_FUNC(glDisableVertexAttribArray, void, (GLuint index))
GL_FUNC(glEnableVertexAttribArray, void, (GLuint index))

GL_FUNC(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer))

GL_FUNC(glVertexAttribIPointer, void, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer))

GL_FUNC(glUniform1f, void, (GLint location, GLfloat v0))
GL_FUNC(glUniform2f, void, (GLint location, GLfloat v0, GLfloat v1))
GL_FUNC(glUniform3f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2))
GL_FUNC(glUniform4f, void, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3))
GL_FUNC(glUniform1i, void, (GLint location, GLint v0))
GL_FUNC(glUniform2i, void, (GLint location, GLint v0, GLint v1))
GL_FUNC(glUniform3i, void, (GLint location, GLint v0, GLint v1, GLint v2))
GL_FUNC(glUniform4i, void, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3))
GL_FUNC(glUniform1fv, void, (GLint location, GLsizei count, const GLfloat *value))
GL_FUNC(glUniform2fv, void, (GLint location, GLsizei count, const GLfloat *value))
GL_FUNC(glUniform3fv, void, (GLint location, GLsizei count, const GLfloat *value))
GL_FUNC(glUniform4fv, void, (GLint location, GLsizei count, const GLfloat *value))
GL_FUNC(glUniform1iv, void, (GLint location, GLsizei count, const GLint *value))
GL_FUNC(glUniform2iv, void, (GLint location, GLsizei count, const GLint *value))
GL_FUNC(glUniform3iv, void, (GLint location, GLsizei count, const GLint *value))
GL_FUNC(glUniform4iv, void, (GLint location, GLsizei count, const GLint *value))
GL_FUNC(glUniformMatrix2fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
GL_FUNC(glUniformMatrix3fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))
GL_FUNC(glUniformMatrix4fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))

GL_FUNC(glGenFramebuffers, void, (GLsizei n, GLuint *framebuffers))
GL_FUNC(glBindFramebuffer, void, (GLenum target, GLuint framebuffer))
GL_FUNC(glCheckFramebufferStatus, GLenum, (GLenum target))
GL_FUNC(glFramebufferTexture1D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))
GL_FUNC(glFramebufferTexture2D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))
GL_FUNC(glFramebufferTexture3D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset))

GL_FUNC(glBlitFramebuffer, void, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))

GL_FUNC(glTexImage2DMultisample, void, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations))

#elif OS_MAC

GL_FUNC(glDebugMessageControl, void, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled))
GL_FUNC(glDebugMessageCallback, void, (GLDEBUGPROC callback, const void *userParam))

GL_FUNC(glGenVertexArrays,    void, (GLsizei n, GLuint *arrays))
GL_FUNC(glBindVertexArray,    void, (GLuint array))

GL_FUNC(glVertexAttribIPointer, void, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer))

#endif

#undef GL_FUNC

// BOTTOM



