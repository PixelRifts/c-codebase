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
#define GL_UNSIGNED_INT_24_8 0x84FA

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

#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE0 0x84C0

#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803

#define GL_CLAMP_TO_EDGE 0x812F
#define GL_CLAMP_TO_BORDER 0x812D
#define GL_REPEAT 0x2901
#define GL_MIRRORED_REPEAT 0x8370
#define GL_MIRROR_CLAMP_TO_EDGE 0x8743

#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703

#define GL_RED_INTEGER 0x8D94
#define GL_RED 0x1903
#define GL_GREEN 0x1904
#define GL_BLUE 0x1905
#define GL_ALPHA 0x1906
#define GL_RG 0x8227
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_DEPTH_STENCIL 0x84F9

#define GL_R32I 0x8235
#define GL_R8 0x8229
#define GL_RG8 0x822B
#define GL_RGB8 0x8051
#define GL_RGBA8 0x8058
#define GL_DEPTH24_STENCIL8 0x88F0

#define GL_BLEND 0x0BE2
#define GL_BLEND_SRC 0x0BE1
#define GL_BLEND_DST 0x0BE0
#define GL_ZERO 0x0
#define GL_ONE 0x1
#define GL_SRC_COLOR 0x0300
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_SRC_ALPHA 0x0302
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_SRC_ALPHA_SATURATE 0x0308

#define GL_TEXTURE_SWIZZLE_R 0x8E42
#define GL_TEXTURE_SWIZZLE_G 0x8E43
#define GL_TEXTURE_SWIZZLE_B 0x8E44
#define GL_TEXTURE_SWIZZLE_A 0x8E45
#define GL_TEXTURE_SWIZZLE_RGBA 0x8E46

#define GL_FRAMEBUFFER 0x8D40
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_STENCIL_ATTACHMENT 0x821A

#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT 0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER 0x8CDB
#define GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER 0x8CDC

#define GL_DEPTH_TEST 0x0B71

#define GL_CULL_FACE 0x0B44
#define GL_NONE 0
#define GL_FRONT_LEFT 0x0400
#define GL_FRONT_RIGHT 0x0401
#define GL_BACK_LEFT 0x0402
#define GL_BACK_RIGHT 0x0403
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_LEFT 0x0406
#define GL_RIGHT 0x0407
#define GL_FRONT_AND_BACK 0x0408

#if defined(BACKEND_GL33)

#  define GL_FUNCTIONS \
X(glGenBuffers, void, (GLsizei count, GLuint* buffer_handles))\
X(glBindBuffer, void, (GLenum target, GLuint buffer_handle))\
X(glBufferData, void, (GLenum target, GLsizeiptr size, const void* data, GLenum usage))\
X(glBufferSubData, void, (GLenum target, GLintptr offset, GLsizeiptr size, const void* data))\
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
X(glGetUniformLocation, i32, (GLuint program_handle, const GLchar *name))\
X(glUniformMatrix4fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))\
X(glUniform1i, void, (GLint location, GLint value))\
X(glUniform1iv, void, (GLint location, GLuint count, GLint* values))\
X(glUniform1f, void, (GLint location, GLfloat value))\
X(glUniform4f, void, (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w))\
X(glGetProgramiv, void, (GLuint program_handle, GLenum param_name, GLint* values))\
X(glGetProgramInfoLog, void, (GLuint program_handle, GLsizei buf_size, GLsizei* length, GLchar* log))\
X(glDetachShader, void, (GLuint program_handle, GLuint shader_handle))\
X(glDeleteProgram, void, (GLuint program_handle))\
X(glGenVertexArrays, void, (GLsizei count, GLuint* vao_handles))\
X(glBindVertexArray, void, (GLuint vao_handle))\
X(glVertexAttribPointer, void, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer))\
X(glEnableVertexAttribArray, void, (GLuint index))\
X(glDeleteVertexArrays, void, (GLsizei count, const GLuint* vao_handles))\
X(glDrawArrays, void, (GLenum mode, GLint first, GLsizei count))\
X(glClear, void, (GLbitfield mask))\
X(glClearColor, void, (GLfloat r, GLfloat g, GLfloat b, GLfloat a))\
X(glGenTextures, void, (GLsizei count, GLuint* texture_handles))\
X(glBindTexture, void, (GLenum target, GLuint texture_handle))\
X(glTexImage2D, void, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* data))\
X(glTexParameteri, void, (GLenum target, GLenum pname, GLint param))\
X(glTexParameteriv, void, (GLenum target, GLenum pname, GLint* params))\
X(glActiveTexture, void, (GLenum texture_handle))\
X(glTexSubImage2D, void, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* data))\
X(glDeleteTextures, void, (GLsizei count, GLuint* texture_handles))\
X(glFlush, void, (void))\
X(glViewport, void, (GLint x, GLint y, GLsizei w, GLsizei h))\
X(glEnable, void, (GLenum feature))\
X(glDisable, void, (GLenum feature))\
X(glBlendFunc, void, (GLenum src_factor, GLenum dst_factor))\
X(glGenFramebuffers, void, (GLsizei count, GLuint* fbo_handles))\
X(glFramebufferTexture2D, void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))\
X(glDrawBuffers, void, (GLsizei count, GLenum* draw_buffers))\
X(glReadBuffer, void, (GLenum read_buffer))\
X(glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels))\
X(glCheckFramebufferStatus, GLenum, (GLenum target))\
X(glBindFramebuffer, void, (GLenum target, GLuint framebuffer_handle))\
X(glBlitFramebuffer, void, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))\
X(glDeleteFramebuffers, void, (GLsizei count, GLuint* fbo_handles))\
X(glCullFace, void, (GLenum mode))\

#elif defined(BACKEND_GL46)

#  define GL_FUNCTIONS \
X(glCreateBuffers, void, (GLsizei count, GLuint* buffer_handles))\
X(glNamedBufferStorage, void, (GLuint buffer_handle, GLsizeiptr size, const void* data, GLbitfield flags))\
X(glNamedBufferSubData, void, (GLuint buffer_handle, GLintptr offset, GLsizeiptr size, const void* data))\
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
X(glGetUniformLocation, i32, (GLuint program_handle, const GLchar *name))\
X(glUniformMatrix4fv, void, (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value))\
X(glUniform1i, void, (GLint location, GLint value))\
X(glUniform1iv, void, (GLint location, GLuint count, GLint* values))\
X(glUniform1f, void, (GLint location, GLfloat value))\
X(glUniform4f, void, (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w))\
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
X(glDrawArrays, void, (GLenum mode, GLint first, GLsizei count))\
X(glClear, void, (GLbitfield mask))\
X(glClearColor, void, (GLfloat r, GLfloat g, GLfloat b, GLfloat a))\
X(glCreateTextures, void, (GLenum type, GLsizei count, GLuint* texture_handles))\
X(glBindTextureUnit, void, (GLint slot, GLuint texture_handle))\
X(glTextureStorage2D, void, (GLuint texture_handle, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height))\
X(glTextureParameteri, void, (GLuint texture_handle, GLenum pname, GLint param))\
X(glTextureParameteriv, void, (GLuint texture_handle, GLenum pname, GLint* params))\
X(glTextureSubImage2D, void, (GLuint texture_handle, GLint level, GLint xoffset, GLint yoffset, GLsizei width,\
GLsizei height, GLenum format, GLenum type, const void* data))\
X(glDeleteTextures, void, (GLsizei count, GLuint* texture_handles))\
X(glFlush, void, (void))\
X(glViewport, void, (GLint x, GLint y, GLsizei w, GLsizei h))\
X(glEnable, void, (GLenum feature))\
X(glDisable, void, (GLenum feature))\
X(glBlendFunc, void, (GLenum src_factor, GLenum dst_factor))\
X(glCreateFramebuffers, void, (GLsizei count, GLuint* fbo_handles))\
X(glNamedFramebufferTexture, void, (GLuint fbo_handle, GLenum attachment, GLuint texture, GLint level, GLint layer))\
X(glNamedFramebufferDrawBuffers, void, (GLuint fbo_handle, GLsizei count, GLenum* draw_buffers))\
X(glNamedFramebufferReadBuffer, void, (GLuint fbo_handle, GLenum read_buffer))\
X(glReadPixels, void, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels))\
X(glCheckNamedFramebufferStatus, GLenum, (GLuint fbo_handle, GLenum target))\
X(glBindFramebuffer, void, (GLenum target, GLuint framebuffer_handle))\
X(glBlitNamedFramebuffer, void, (GLuint read_fbo_handle, GLuint draw_fbo_handle, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))\
X(glDeleteFramebuffers, void, (GLsizei count, GLuint* fbo_handles))\
X(glCullFace, void, (GLenum mode))\

#endif


#if defined(GL_FUNCTIONS)
#  define X(Name, Return, Args)\
typedef Return GL_##Name##_Func Args;\
extern GL_##Name##_Func* Name;
GL_FUNCTIONS
#  undef X
#endif // defined(GL_FUNCTIONS)

#endif //GL_FUNCTIONS_H
