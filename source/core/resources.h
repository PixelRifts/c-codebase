/* date = July 3rd 2022 0:06 pm */

#ifndef RESOURCES_H
#define RESOURCES_H

#include "defines.h"
#include "base/str.h"
#include "base/ds.h"
#include "base/vmath.h"
#include "os/window.h"


//~ Enums
typedef u32 R_BufferFlags;
enum {
	BufferFlag_Dynamic = 0x1,
	
	// Enable only one of these
	BufferFlag_Type_Vertex = 0x2,
	BufferFlag_Type_Index = 0x4,
};

typedef u32 R_ShaderType;
enum {
	ShaderType_NULL,
	
	ShaderType_Vertex,
	ShaderType_Fragment,
	ShaderType_Geometry,
	
	ShaderType_MAX,
};

typedef u32 R_InputAssembly;
enum {
	InputAssembly_Triangles,
	InputAssembly_Lines,
	
	InputAssembly_MAX,
};

typedef u32 R_AttributeType;
enum {
	AttributeType_Float1,
	AttributeType_Float2,
	AttributeType_Float3,
	AttributeType_Float4,
	AttributeType_Integer1,
	AttributeType_Integer2,
	AttributeType_Integer3,
	AttributeType_Integer4,
	
	AttributeType_MAX,
};

typedef struct R_Attribute {
	string name;
	R_AttributeType type;
} R_Attribute;

typedef u32 R_BlendMode;
enum {
	BlendMode_None,
	BlendMode_Alpha,
	
	BlendMode_MAX,
};

typedef u32 R_TextureWrapParam;
enum {
	TextureWrap_ClampToEdge,
	TextureWrap_ClampToBorder,
	TextureWrap_Repeat,
	TextureWrap_MirroredRepeat,
	TextureWrap_MirrorClampToEdge,
	
	TextureWrap_MAX
};

typedef u32 R_TextureResizeParam;
enum {
	TextureResize_Nearest,
	TextureResize_Linear,
	TextureResize_LinearMipmapLinear,
	TextureResize_LinearMipmapNearest,
	TextureResize_NearestMipmapLinear,
	TextureResize_NearestMipmapNearest,
	
	TextureResize_MAX,
};

typedef u32 R_TextureFormat;
enum {
	TextureFormat_Invalid,
	
	TextureFormat_RInteger,
	TextureFormat_R,
	TextureFormat_RG,
	TextureFormat_RGB,
	TextureFormat_RGBA,
	TextureFormat_DepthStencil,
	
	TextureFormat_MAX,
};

typedef i32 R_TextureChannel;
enum {
	TextureChannel_Zero,
	TextureChannel_One,
	TextureChannel_R,
	TextureChannel_G,
	TextureChannel_B,
	TextureChannel_A,
	TextureChannel_MAX,
};

typedef u32 R_TextureMutability;
enum {
	TextureMutability_Immutable,
	TextureMutability_Uncommon,
	TextureMutability_Dynamic,
	
	TextureMutability_MAX,
};

typedef u32 R_TextureUsage;
enum {
	TextureUsage_ShaderResource = 0x1,
	TextureUsage_Drawable = 0x2,
};

typedef u32 R_BufferMask;
enum {
	BufferMask_Color = 0x01,
	BufferMask_Depth = 0x02,
	BufferMask_Stencil = 0x04,
};

//~ Backend specific structures
#if defined(BACKEND_GL33)
#  include "impl/gl33_resources.h"
#elif defined(BACKEND_GL46)
#  include "impl/gl46_resources.h"
#elif defined(BACKEND_D3D11)
#  include "impl/d3d11_resources.h"
#endif


//~ Buffers
void R_BufferAlloc(R_Buffer* buf, R_BufferFlags flags, u32 v_stride);
void R_BufferData(R_Buffer* buf, u64 size, void* data);
void R_BufferUpdate(R_Buffer* buf, u64 offset, u64 size, void* data);
void R_BufferFree(R_Buffer* buf);

void R_UniformBufferAlloc(R_UniformBuffer* buf, string name, string_array member_names,
						  R_ShaderPack* pack, R_ShaderType type);
void R_UniformBufferFree(R_UniformBuffer* buf);

void R_UniformBufferSetMat4(R_UniformBuffer* buf, string name, mat4 mat);
void R_UniformBufferSetInt(R_UniformBuffer* buf, string name, i32 val);
void R_UniformBufferSetIntArray(R_UniformBuffer* buf, string name, i32* vals, u32 count);
void R_UniformBufferSetFloat(R_UniformBuffer* buf, string name, f32 val);
void R_UniformBufferSetVec4(R_UniformBuffer* buf, string name, vec4 val);

//~ Shaders
void R_ShaderAlloc(R_Shader* shader, string data, R_ShaderType type);
void R_ShaderAllocLoad(R_Shader* shader, string fp, R_ShaderType type);
void R_ShaderFree(R_Shader* shader);

void R_ShaderPackAlloc(R_ShaderPack* pack, R_Shader* shaders, u32 shader_count);
void R_ShaderPackAllocLoad(R_ShaderPack* pack, string fp_prefix);
void R_ShaderPackFree(R_ShaderPack* pack);

void R_ShaderPackUploadMat4(R_ShaderPack* pack, string name, mat4 mat);
void R_ShaderPackUploadInt(R_ShaderPack* pack, string name, i32 val);
void R_ShaderPackUploadIntArray(R_ShaderPack* pack, string name, i32* vals, u32 count);
void R_ShaderPackUploadFloat(R_ShaderPack* pack, string name, f32 val);
void R_ShaderPackUploadVec4(R_ShaderPack* pack, string name, vec4 val);

//~ Pipelines (VAOs OR NOT)
void R_PipelineAlloc(R_Pipeline* in, R_InputAssembly assembly, R_Attribute* attributes, u32 attribute_count, R_ShaderPack* shader, R_BlendMode blending);
void R_PipelineAddBuffer(R_Pipeline* in, R_Buffer* buf, u32 attribute_count);
void R_PipelineAddUniformBuffer(R_Pipeline* in, R_UniformBuffer* buf);
void R_PipelineBind(R_Pipeline* in);
void R_PipelineFree(R_Pipeline* in);



//~ Textures
// NOTE(voxel): Consider converting to desc-style instead of this amount of args :pain:
void R_Texture2DAlloc(R_Texture2D* texture, R_TextureFormat format, u32 width, u32 height, R_TextureResizeParam min, R_TextureResizeParam mag, R_TextureWrapParam wrap_s, R_TextureWrapParam wrap_t, R_TextureMutability mut, R_TextureUsage usage, void* initial_data);
void R_Texture2DAllocLoad(R_Texture2D* texture, string filepath, R_TextureResizeParam min, R_TextureResizeParam mag, R_TextureWrapParam wrap_s, R_TextureWrapParam wrap_t, R_TextureMutability mut, R_TextureUsage usage);
void R_Texture2DData(R_Texture2D* texture, void* data);
void R_Texture2DWhite(R_Texture2D* texture);
b8   R_Texture2DEquals(R_Texture2D* a, R_Texture2D* b);
void R_Texture2DSwizzle(R_Texture2D* texture, i32* swizzles);
void R_Texture2DBindTo(R_Texture2D* texture, u32 slot);
void R_Texture2DFree(R_Texture2D* texture);


//~ Framebuffer
void R_FramebufferCreate(R_Framebuffer* framebuffer, u32 width, u32 height, R_Texture2D* color_attachments, u32 color_attachment_count, R_Texture2D depth_attachment);
void R_FramebufferBind(R_Framebuffer* framebuffer);
void R_FramebufferBindScreen(void);
void R_FramebufferBlitToScreen(OS_Window* window, R_Framebuffer* framebuffer);
void R_FramebufferReadPixel(R_Framebuffer* framebuffer, u32 attachment, u32 x, u32 y,
							void* data);
void R_FramebufferResize(R_Framebuffer* framebuffer, u32 new_width, u32 new_height);
void R_FramebufferFree(R_Framebuffer* framebuffer);


void R_Clear(R_BufferMask buffer_mask);
void R_ClearColor(f32 r, f32 g, f32 b, f32 a);
void R_Viewport(i32 x, i32 y, i32 w, i32 h);
void R_Draw(R_Pipeline* pipeline, u32 start, u32 count);

#endif //RESOURCES_H
