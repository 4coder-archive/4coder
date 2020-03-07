/* Mac OpenGL functions for 4coder */

// TOP
/* Usage:
#define GL_FUNC(N,R,P) ~~~~
#include "4ed_opengl_funcs.h"
*/

// TODO(allen): eliminate this
GL_FUNC(glDebugMessageControl, void, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled))
GL_FUNC(glDebugMessageCallback, void, (GLDEBUGPROC callback, const void *userParam))

GL_FUNC(glGenVertexArrays,    void, (GLsizei n, GLuint *arrays))
GL_FUNC(glBindVertexArray,    void, (GLuint array))

GL_FUNC(glVertexAttribIPointer, void, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer))

#undef GL_FUNC
