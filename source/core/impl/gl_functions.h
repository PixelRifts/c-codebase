/* date = July 13th 2022 9:50 pm */

#ifndef GL_FUNCTIONS_H
#define GL_FUNCTIONS_H

#include "defines.h"

typedef u32 GLenum;
typedef u8 GLboolean;
typedef u32 GLbitfield;
typedef void GLvoid;
typedef u8 GLubyte;
typedef i16 GLshort;
typedef u16 GLushort;
typedef i32 GLint;
typedef u32 GLuint;
typedef i32 GLclampx;
typedef i32 GLsizei;
typedef f32 GLfloat;
typedef f32 GLclampf;
typedef f64 GLdouble;
typedef f64 GLclampd;
typedef void *GLeglClientBufferEXT;
typedef void *GLeglImageOES;
typedef i8  GLchar;
typedef i8  GLcharARB;
typedef u16 GLhalf;
typedef u16 GLhalfARB;
typedef i32 GLfixed;
typedef u64 GLintptr;
typedef u64 GLintptrARB;
typedef i64 GLsizeiptr;
typedef i64 GLsizeiptrARB;
typedef i64 GLint64;
typedef i64 GLint64EXT;
typedef u64 GLuint64;
typedef u64 GLuint64EXT;

#define GL_FALSE 0
#define GL_TRUE 1

#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893

#define GL_INFO_LOG_LENGTH 0x8B84

#define GL_DELETE_STATUS 0x8B80
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_VALIDATE_STATUS 0x8B83

#define GL_STREAM_DRAW 0x88E0
#define GL_STREAM_READ 0x88E1
#define GL_STREAM_COPY 0x88E2
#define GL_STATIC_DRAW 0x88E4
#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_DYNAMIC_READ 0x88E9
#define GL_DYNAMIC_COPY 0x88EA

#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_GEOMETRY_SHADER 0x8DD9

#define GL_BYTE 0x1400
#define GL_UNSIGNED_BYTE 0x1401
#define GL_SHORT 0x1402
#define GL_UNSIGNED_SHORT 0x1403
#define GL_INT 0x1404
#define GL_UNSIGNED_INT 0x1405
#define GL_FLOAT 0x1406

#define GL_INVALID_ENUM 0x0500

#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_LINE_LOOP 0x0002
#define GL_LINE_STRIP 0x0003
#define GL_TRIANGLES 0x0004
#define GL_TRIANGLE_STRIP 0x0005
#define GL_TRIANGLE_FAN 0x0006
#define GL_QUADS 0x0007

#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000

#define GL_MAP_PERSISTENT_BIT 0x0040
#define GL_MAP_COHERENT_BIT 0x0080
#define GL_DYNAMIC_STORAGE_BIT 0x0100
#define GL_CLIENT_STORAGE_BIT 0x0200

#if defined(BACKEND_GL33)

#  define GL_FUNCTIONS \
X(glGenBuffers, void, (GLsizei count, GLuint* buffer_handles))\
X(glBindBuffer, void, (GLenum target, GLuint buffer_handle))\
X(glBufferData, void, (GLenum target, GLsizeiptr size, const void* data, GLenum usage))\
X(glDeleteBuffers, void, (GLsizei count, const GLuint* buffer_handles))\
X(glCreateShader, u32, (GLenum type))\
X(glShaderSource, void, (GLuint shader_handle, GLsizei count, const GLchar* const* str, const GLint* length))\
X(glCompileShader, void, (GLuint shader_handle))\
X(glGetShaderiv, void, (GLuint shader_handle, GLenum param_name, GLint* values))\
X(glGetShaderInfoLog, void, (GLuint shader_handle, GLsizei buf_size, GLsizei* length, GLchar* log))\
X(glDeleteShader, void, (GLuint shader_handle))\
X(glCreateProgram, u32, (void))\
X(glAttachShader, void, (GLuint program_handle, GLuint shader_handle))\
X(glLinkProgram, void, (GLuint program_handle))\
X(glUseProgram, void, (GLuint program_handle))\
X(glGetProgramiv, void, (GLuint program_handle, GLenum param_name, GLint* values))\
X(glGetProgramInfoLog, void, (GLuint program_handle, GLsizei buf_size, GLsizei* length, GLchar* log))\
X(glDetachShader, void, (GLuint program_handle, GLuint shader_handle))\
X(glDeleteProgram, void, (GLuint program_handle))\
X(glGenVertexArrays, void, (GLsizei count, GLuint* vao_handles))\
X(glBindVertexArray, void, (GLuint vao_handle))\
X(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer))\
X(glEnableVertexAttribArray, void, (GLuint index))\
X(glDeleteVertexArrays, void, (GLsizei count, const GLuint* vao_handles))\

#elif defined(BACKEND_GL46)

#  define GL_FUNCTIONS \
X(glCreateBuffers, void, (GLsizei count, GLuint* buffer_handles))\
X(glNamedBufferStorage, void, (GLuint buffer_handle, GLsizeiptr size, const void* data, GLbitfield flags))\
X(glDeleteBuffers, void, (GLsizei count, const GLuint* buffer_handles))\
X(glCreateShader, u32, (GLenum type))\
X(glShaderSource, void, (GLuint shader_handle, GLsizei count, const GLchar* const* str, const GLint* length))\
X(glCompileShader, void, (GLuint shader_handle))\
X(glGetShaderiv, void, (GLuint shader_handle, GLenum param_name, GLint* values))\
X(glGetShaderInfoLog, void, (GLuint shader_handle, GLsizei buf_size, GLsizei* length, GLchar* log))\
X(glDeleteShader, void, (GLuint shader_handle))\
X(glCreateProgram, u32, (void))\
X(glAttachShader, void, (GLuint program_handle, GLuint shader_handle))\
X(glLinkProgram, void, (GLuint program_handle))\
X(glUseProgram, void, (GLuint program_handle))\
X(glGetProgramiv, void, (GLuint program_handle, GLenum param_name, GLint* values))\
X(glGetProgramInfoLog, void, (GLuint program_handle, GLsizei buf_size, GLsizei* length, GLchar* log))\
X(glDetachShader, void, (GLuint program_handle, GLuint shader_handle))\
X(glDeleteProgram, void, (GLuint program_handle))\
X(glCreateVertexArrays, void, (GLsizei count, GLuint* vao_handles))\
X(glBindVertexArray, void, (GLuint vao_handle))\
X(glVertexArrayAttribFormat, void, (GLuint vao_handle, GLuint attribute_index, GLint size, GLenum type, GLboolean normalized, GLuint relative_offset))\
X(glVertexArrayAttribBinding, void, (GLuint vao_handle, GLuint attribute_index, GLuint binding_index))\
X(glVertexArrayVertexBuffer, void, (GLuint vao_handle, GLuint binding_index, GLuint buffer_handle, GLintptr offset, GLsizei stride))\
X(glEnableVertexArrayAttrib, void, (GLuint vao_handle, GLuint index))\
X(glDeleteVertexArrays, void, (GLsizei count, const GLuint* vao_handles))\


#endif


#define X(Name, Return, Args)\
typedef Return GL_##Name##_Func Args;\
extern GL_##Name##_Func* Name;
GL_FUNCTIONS
#undef X

#endif //GL_FUNCTIONS_H
