/* date = July 3rd 2022 0:06 pm */

#ifndef RESOURCES_H
#define RESOURCES_H

#include "defines.h"
#include "base/str.h"

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
	u64 v[1];
} R_ShaderPack;

void R_ShaderAlloc(R_Shader* shader, string data, R_ShaderType type);
void R_ShaderAllocLoad(R_Shader* shader, string fp, R_ShaderType type);
void R_ShaderFree(R_Shader* shader);

void R_ShaderPackAlloc(R_ShaderPack* pack, R_Shader* shaders, u32 shader_count);
void R_ShaderPackAllocLoad(R_ShaderPack* pack, string fp_prefix);
void R_ShaderPackFree(R_ShaderPack* pack);

//~ Attribute Specifiers (VAOs)

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

void R_PipelineAlloc(R_Pipeline* in, R_InputAssembly assembly, R_Attribute* attributes, R_ShaderPack* shader, u32 attribute_count);
void R_PipelineAddBuffer(R_Pipeline* in, R_Buffer* _buf, u32 attribute_count);
void R_PipelineBind(R_Pipeline* in);
void R_PipelineFree(R_Pipeline* in);

#endif //RESOURCES_H
