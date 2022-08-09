/* date = July 3rd 2022 0:06 pm */

#ifndef RESOURCES_H
#define RESOURCES_H

#include "defines.h"
#include "base/str.h"
#include "base/ds.h"
#include "base/vmath.h"

//~ Buffers

typedef u32 R_BufferFlags;
enum {
	BufferFlag_Dynamic = 0x1,
	
	BufferFlag_Type_Vertex = 0x2,
	BufferFlag_Type_Index = 0x4,
	BufferFlag_Type_Uniform = 0x8,
};

typedef struct R_Buffer {
	R_BufferFlags flags;
	u64 v[2];
} R_Buffer;

void R_BufferAlloc(R_Buffer* buf, R_BufferFlags flags);
void R_BufferData(R_Buffer* buf, u64 size, void* data);
void R_BufferUpdate(R_Buffer* _buf, u64 offset, u64 size, void* data);
void R_BufferFree(R_Buffer* buf);

//~ Shaders

typedef u32 R_ShaderType;
enum {
	ShaderType_Vertex,
	ShaderType_Fragment,
	ShaderType_Geometry,
	
	ShaderType_MAX,
};

typedef struct R_Shader {
	R_ShaderType type;
	u64 v[1];
} R_Shader;

typedef struct R_ShaderPack {
	u64 v[4];
} R_ShaderPack;

void R_ShaderAlloc(R_Shader* shader, string data, R_ShaderType type);
void R_ShaderAllocLoad(R_Shader* shader, string fp, R_ShaderType type);
void R_ShaderFree(R_Shader* shader);

void R_ShaderPackAlloc(R_ShaderPack* pack, R_Shader* shaders, u32 shader_count);
void R_ShaderPackAllocLoad(R_ShaderPack* pack, string fp_prefix);
void R_ShaderPackFree(R_ShaderPack* pack);

void R_ShaderPackUploadMat4(R_ShaderPack* pack, string name, mat4 mat);
void R_ShaderPackUploadInt(R_ShaderPack* pack, string name, i32 val);
void R_ShaderPackUploadIntArray(R_ShaderPack* _pack, string name, i32* vals, u32 count);
void R_ShaderPackUploadFloat(R_ShaderPack* pack, string name, f32 val);
void R_ShaderPackUploadVec4(R_ShaderPack* pack, string name, vec4 val);

//~ Pipelines (VAOs)

typedef u32 R_InputAssembly;
enum {
	InputAssembly_Triangles,
	InputAssembly_Lines,
	
	InputAssembly_MAX,
};

typedef u32 R_Attribute;
enum {
	Attribute_Float1,
	Attribute_Float2,
	Attribute_Float3,
	Attribute_Float4,
	Attribute_Integer1,
	Attribute_Integer2,
	Attribute_Integer3,
	Attribute_Integer4,
	
	Attribute_MAX,
};

typedef struct R_Pipeline {
	R_InputAssembly assembly;
	R_Attribute* attributes;
	R_ShaderPack* shader;
	u32 attribute_count;
	
	u64 v[2];
} R_Pipeline;

void R_PipelineAlloc(R_Pipeline* _in, R_InputAssembly assembly, R_Attribute* attributes, u32 attribute_count, R_ShaderPack* shader);
void R_PipelineAddBuffer(R_Pipeline* in, R_Buffer* _buf, u32 attribute_count);
void R_PipelineBind(R_Pipeline* in);
void R_PipelineFree(R_Pipeline* in);

//~ Textures

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
	TextureFormat_RInteger,
	TextureFormat_R,
	TextureFormat_RG,
	TextureFormat_RGB,
	TextureFormat_RGBA,
	TextureFormat_DepthStencil,
	
	TextureFormat_MAX,
};

typedef struct R_Texture2D {
	u32 width;
	u32 height;
	
	R_TextureFormat format;
	R_TextureResizeParam min;
	R_TextureResizeParam mag;
	R_TextureWrapParam wrap_s;
	R_TextureWrapParam wrap_t;
	
	u64 v[1];
} R_Texture2D;

void R_Texture2DAlloc(R_Texture2D* texture, R_TextureFormat format, u32 width, u32 height, R_TextureResizeParam min, R_TextureResizeParam mag, R_TextureWrapParam wrap_s, R_TextureWrapParam wrap_t);
void R_Texture2DAllocLoad(R_Texture2D* texture, string filepath, R_TextureResizeParam min, R_TextureResizeParam mag, R_TextureWrapParam wrap_s, R_TextureWrapParam wrap_t);
void R_Texture2DData(R_Texture2D* texture, void* data);
void R_Texture2DWhite(R_Texture2D* texture);
b8   R_Texture2DEquals(R_Texture2D* a, R_Texture2D* b);

void R_Texture2DBindTo(R_Texture2D* texture, u32 slot);
void R_Texture2DFree(R_Texture2D* texture);

//~ Other

typedef u32 R_BufferMask;
enum {
	BufferMask_Color = 0x01,
	BufferMask_Depth = 0x02,
	BufferMask_Stencil = 0x04,
};

void R_Clear(R_BufferMask buffer_mask);
void R_ClearColor(f32 r, f32 g, f32 b, f32 a);
void R_Viewport(i32 x, i32 y, i32 w, i32 h);
void R_Draw(R_Pipeline* pipeline, u32 start, u32 count);

#endif //RESOURCES_H
