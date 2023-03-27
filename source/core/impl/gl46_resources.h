/* date = March 27th 2023 0:59 pm */

#ifndef GL46_RESOURCES_H
#define GL46_RESOURCES_H

typedef struct R_Buffer {
	R_BufferFlags flags;
	u32 handle;
} R_Buffer;

typedef struct R_Shader {
	R_ShaderType type;
	u32 handle;
} R_Shader;

HashTable_Prototype(string, i32);
typedef struct R_ShaderPack {
	hash_table(string, i32) uniforms;
	u32 handle;
} R_ShaderPack;

typedef struct R_Pipeline {
	R_InputAssembly assembly;
	R_Attribute* attributes;
	R_ShaderPack* shader;
	R_BlendMode blend_mode;
	u32 attribute_count;
	
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
	
	u32 sampler_handle;
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
