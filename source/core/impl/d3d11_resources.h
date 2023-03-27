/* date = March 27th 2023 3:35 pm */

#ifndef D3D11_RESOURCES_H
#define D3D11_RESOURCES_H

#define D3D11_NO_HELPERS
#define CINTERFACE
#define COBJMACROS
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>

typedef struct R_Buffer {
	R_BufferFlags flags;
	ID3D11Buffer* handle;
	u32 v_stride;
} R_Buffer;

typedef struct R_Shader {
	R_ShaderType type;
	ID3D10Blob* bytecode_blob;
	union {
		ID3D11VertexShader* vs;
		ID3D11PixelShader* ps;
		ID3D11GeometryShader* gs;
	};
} R_Shader;

typedef struct R_ShaderPack {
	R_Shader vs;
	R_Shader ps;
	R_Shader gs;
} R_ShaderPack;

typedef struct R_BufferAttribCountPack {
	R_Buffer* b;
	u32 attrib_count;
} R_BufferAttribCountPack;

DArray_Prototype(R_BufferAttribCountPack);

typedef struct R_Pipeline {
	R_InputAssembly assembly;
	R_Attribute* attributes;
	R_ShaderPack* shader;
	R_BlendMode blend_mode;
	u32 attribute_count;
	
	darray(R_BufferAttribCountPack) buffers;
	ID3D11InputLayout* layout;
	b8 layout_changed;
} R_Pipeline;

typedef struct R_Texture2D {
	u8 temp;
} R_Texture2D;

typedef struct R_Framebuffer {
	u8 temp;
} R_Framebuffer;

#endif //D3D11_RESOURCES_H
