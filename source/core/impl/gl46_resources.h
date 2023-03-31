/* date = March 27th 2023 0:59 pm */

#ifndef GL46_RESOURCES_H
#define GL46_RESOURCES_H

HashTable_Prototype(string, i32);

typedef struct R_Buffer {
	R_BufferFlags flags;
	u32 handle;
	
	union {
		string buffer_name;
	};
} R_Buffer;

typedef struct R_UniformBuffer {
	R_ShaderType stage;
	string name;
	hash_table(string, i32) uniform_offsets;
	u8* cpu_side_buffer;
	b8  dirty;
	u32 size;
	u32 bindpoint;
	u32 handle;
} R_UniformBuffer;

typedef struct R_Shader {
	R_ShaderType type;
	u32 handle;
} R_Shader;

typedef struct R_ShaderPack {
	u32 handle;
} R_ShaderPack;

typedef R_UniformBuffer* R_UniformBufferHandle;
DArray_Prototype(R_UniformBufferHandle);

typedef struct R_Pipeline {
	R_InputAssembly assembly;
	R_Attribute* attributes;
	R_ShaderPack* shader;
	R_BlendMode blend_mode;
	u32 attribute_count;
	
	darray(R_UniformBufferHandle) uniform_buffers;
	
	u32 bindpoint;
	u32 attribpoint;
	u32 handle;
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
	
	u32 handle;
} R_Texture2D;

typedef struct R_Framebuffer {
	u32 width;
	u32 height;
	
	R_Texture2D* color_attachments;
	u32 color_attachment_count;
	R_Texture2D depth_attachment;
	
	u32 handle;
} R_Framebuffer;


#endif //GL46_RESOURCES_H
