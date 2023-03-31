/* date = March 27th 2023 3:35 pm */

#ifndef D3D11_RESOURCES_H
#define D3D11_RESOURCES_H

#define D3D11_NO_HELPERS
#define CINTERFACE
#define COBJMACROS
#include <d3d11.h>
#include <d3d11shader.h>
#include <dxgi.h>
#include <d3dcompiler.h>

HashTable_Prototype(string, i32);

typedef struct R_Buffer {
	R_BufferFlags flags;
	ID3D11Buffer* handle;
	u32 v_stride;
} R_Buffer;

typedef struct R_UniformBuffer {
	R_ShaderType stage;
	string name;
	hash_table(string, i32) uniform_offsets;
	u8* cpu_side_buffer;
	b8  dirty;
	u32 size;
	ID3D11Buffer* handle;
} R_UniformBuffer;

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

typedef R_UniformBuffer* R_UniformBufferHandle;
DArray_Prototype(R_UniformBufferHandle);

typedef struct R_Pipeline {
	R_InputAssembly assembly;
	R_Attribute* attributes;
	R_ShaderPack* shader;
	R_BlendMode blend_mode;
	u32 attribute_count;
	
	// yikes!
	darray(R_UniformBufferHandle) vs_uniform_buffers;
	darray(R_UniformBufferHandle) ps_uniform_buffers;
	darray(R_UniformBufferHandle) gs_uniform_buffers;
	
	darray(R_BufferAttribCountPack) buffers;
	ID3D11InputLayout* layout;
	ID3D11BlendState* blend_state;
	b8 layout_changed;
} R_Pipeline;

typedef struct R_Texture2D {
	u32 width;
	u32 height;
	
	R_TextureFormat format;
	R_TextureResizeParam min;
	R_TextureResizeParam mag;
	R_TextureWrapParam wrap_s;
	R_TextureWrapParam wrap_t;
	R_TextureMutability mut;
	R_TextureUsage usage;
	
	ID3D11Texture2D* handle;
	ID3D11ShaderResourceView* shader_resource_view;
	ID3D11RenderTargetView* render_target_view;
	ID3D11SamplerState* sampler;
} R_Texture2D;

typedef struct R_Framebuffer {
	u8 temp;
} R_Framebuffer;

#endif //D3D11_RESOURCES_H
