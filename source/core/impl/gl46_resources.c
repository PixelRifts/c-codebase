//~ OpenGL 4.6 Resources
#include <stb/stb_image.h>

#include "gl_functions.h"

// TODO(voxel): Debug Layer

HashTable_Prototype(string, i32);
b8 str_is_null(string k)  { return k.str == 0 && k.size == 0; }
b8 i32_is_null(i32 value) { return value == 0;  }
b8 i32_is_tomb(i32 value) { return value == 69; }
HashTable_Impl(string, i32, str_is_null, str_eq, str_hash, 69, i32_is_null, i32_is_tomb);

typedef struct R_GL46Buffer {
	R_BufferFlags flags;
	u32 handle;
} R_GL46Buffer;

typedef struct R_GL46Shader {
	R_ShaderType type;
	u32 handle;
} R_GL46Shader;

typedef struct R_GL46ShaderPack {
	hash_table(string, i32) uniforms;
	u32 handle;
} R_GL46ShaderPack;

typedef struct R_GL46Pipeline {
	R_InputAssembly assembly;
	R_Attribute* attributes;
	R_GL46ShaderPack* shader;
	R_BlendMode blend_mode;
	u32 attribute_count;
	
	u32 bindpoint;
	u32 attribpoint;
	u32 handle;
} R_GL46Pipeline;

typedef struct R_GL46Texture2D {
	u32 width;
	u32 height;
	
	R_TextureFormat format;
	R_TextureResizeParam min;
	R_TextureResizeParam mag;
	R_TextureWrapParam wrap_s;
	R_TextureWrapParam wrap_t;
	
	u32 sampler_handle;
	u32 handle;
} R_GL46Texture2D;

typedef struct R_GL46Framebuffer {
	u32 width;
	u32 height;
	
	R_Texture2D* color_attachments;
	u32 color_attachment_count;
	R_Texture2D depth_attachment;
	
	u32 handle;
} R_GL46Framebuffer;


//~ Elpers

static u32 get_size_of(R_AttributeType attrib) {
	AssertTrue(8 == AttributeType_MAX, "Non Exhaustive switch statement: get_size_of in gl46 backend");
	switch (attrib) {
		case AttributeType_Float1: return 1 * sizeof(f32);
		case AttributeType_Float2: return 2 * sizeof(f32);
		case AttributeType_Float3: return 3 * sizeof(f32);
		case AttributeType_Float4: return 4 * sizeof(f32);
		case AttributeType_Integer1: return 1 * sizeof(i32);
		case AttributeType_Integer2: return 2 * sizeof(i32);
		case AttributeType_Integer3: return 3 * sizeof(i32);
		case AttributeType_Integer4: return 4 * sizeof(i32);
	}
	return 0;
}

static u32 get_component_count_of(R_AttributeType attrib) {
	AssertTrue(8 == AttributeType_MAX, "Non Exhaustive switch statement: get_component_count_of in gl46 backend");
	switch (attrib) {
		case AttributeType_Float1: return 1;
		case AttributeType_Float2: return 2;
		case AttributeType_Float3: return 3;
		case AttributeType_Float4: return 4;
		case AttributeType_Integer1: return 1;
		case AttributeType_Integer2: return 2;
		case AttributeType_Integer3: return 3;
		case AttributeType_Integer4: return 4;
	}
	return 0;
}

static u32 get_type_of(R_AttributeType attrib) {
	AssertTrue(8 == AttributeType_MAX, "Non Exhaustive switch statement: get_type_of in gl46 backend");
	switch (attrib) {
		case AttributeType_Float1: return GL_FLOAT;
		case AttributeType_Float2: return GL_FLOAT;
		case AttributeType_Float3: return GL_FLOAT;
		case AttributeType_Float4: return GL_FLOAT;
		case AttributeType_Integer1: return GL_INT;
		case AttributeType_Integer2: return GL_INT;
		case AttributeType_Integer3: return GL_INT;
		case AttributeType_Integer4: return GL_INT;
	}
	return GL_INVALID_ENUM;
}

static u32 get_shader_type_of(R_ShaderType type) {
	AssertTrue(4 == ShaderType_MAX, "Non Exhaustive switch statement: get_shader_type_of in gl46 backend");
	switch (type) {
		case ShaderType_Vertex: return GL_VERTEX_SHADER;
		case ShaderType_Fragment: return GL_FRAGMENT_SHADER;
		case ShaderType_Geometry: return GL_GEOMETRY_SHADER;
	}
	return GL_INVALID_ENUM;
}

static u32 get_input_assembly_type_of(R_InputAssembly assembly) {
	AssertTrue(2 == InputAssembly_MAX, "Non Exhaustive switch statement: get_input_assembly_type_of in gl46 backend");
	switch (assembly) {
		case InputAssembly_Triangles: return GL_TRIANGLES;
		case InputAssembly_Lines: return GL_LINES;
	}
	return GL_INVALID_ENUM;
}

static u32 get_texture_wrap_param_type_of(R_TextureWrapParam param) {
	AssertTrue(5 == TextureWrap_MAX, "Non Exhaustive switch statement: get_texture_wrap_param_type_of in gl46 backend");
	switch (param) {
		case TextureWrap_ClampToEdge: return GL_CLAMP_TO_EDGE;
		case TextureWrap_ClampToBorder: return GL_CLAMP_TO_BORDER;
		case TextureWrap_Repeat: return GL_REPEAT;
		case TextureWrap_MirroredRepeat: return GL_MIRRORED_REPEAT;
		case TextureWrap_MirrorClampToEdge: return GL_MIRROR_CLAMP_TO_EDGE;
	}
	return 0;
}

static u32 get_texture_resize_param_type_of(R_TextureResizeParam param) {
	AssertTrue(6 == TextureResize_MAX,
			   "Non Exhaustive switch statement: get_texture_resize_param_type_of in gl46 backend");
	switch (param) {
		case TextureResize_Nearest: return GL_NEAREST;
		case TextureResize_Linear: return GL_LINEAR;
		case TextureResize_LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
		case TextureResize_LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
		case TextureResize_NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
		case TextureResize_NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
	}
	return 0;
}

static u32 get_texture_format_type_of(R_TextureFormat format) {
	AssertTrue(7 == TextureFormat_MAX,
			   "Non Exhaustive switch statement: get_texture_format_type_of in gl46 backend");
	switch (format) {
		case TextureFormat_RInteger: return GL_RED_INTEGER;
		case TextureFormat_R: return GL_RED;
		case TextureFormat_RG: return GL_RG;
		case TextureFormat_RGB: return GL_RGB;
		case TextureFormat_RGBA: return GL_RGBA;
		case TextureFormat_DepthStencil: return GL_DEPTH_STENCIL;
	}
	return 0;
}

static u32 get_texture_datatype_of(R_TextureFormat format) {
	AssertTrue(7 == TextureFormat_MAX,
			   "Non Exhaustive switch statement: get_texture_datatype_of in gl46 backend");
	switch (format) {
		case TextureFormat_RInteger: return GL_INT;
		case TextureFormat_R: return GL_UNSIGNED_BYTE;
		case TextureFormat_RG: return GL_UNSIGNED_BYTE;
		case TextureFormat_RGB: return GL_UNSIGNED_BYTE;
		case TextureFormat_RGBA: return GL_UNSIGNED_BYTE;
		case TextureFormat_DepthStencil: return GL_UNSIGNED_INT_24_8;
	}
	return 0;
}

static u32 get_texture_internal_format_type_of(R_TextureFormat format) {
    AssertTrue(7 == TextureFormat_MAX,
			   "Non Exhaustive switch statement: get_texture_internal_format_type_of in gl46 backend");
	switch (format) {
        case TextureFormat_RInteger: return GL_R32I;
        case TextureFormat_R: return GL_R8;
        case TextureFormat_RG: return GL_RG8;
        case TextureFormat_RGB: return GL_RGB8;
        case TextureFormat_RGBA: return GL_RGBA8;
        case TextureFormat_DepthStencil: return GL_DEPTH24_STENCIL8;
    }
    return 0;
}

static u32 get_texture_channel_of(R_TextureChannel format) {
    AssertTrue(6 == TextureChannel_MAX,
			   "Non Exhaustive switch statement: get_texture_channel_of in gl46 backend");
	switch (format) {
		case TextureChannel_Zero: return GL_ZERO;
		case TextureChannel_One: return GL_ONE;
        case TextureChannel_R: return GL_RED;
		case TextureChannel_G: return GL_GREEN;
		case TextureChannel_B: return GL_BLUE;
		case TextureChannel_A: return GL_ALPHA;
	}
    return 0;
}

//~ Function Implementations

void R_BufferAlloc(R_Buffer* _buf, R_BufferFlags flags, u32 v_stride) {
	R_GL46Buffer* buf = (R_GL46Buffer*) _buf;
	buf->flags = flags;
	glCreateBuffers(1, &buf->handle);
}

void R_BufferData(R_Buffer* _buf, u64 size, void* data) {
	R_GL46Buffer* buf = (R_GL46Buffer*) _buf;
	u32 flags = 0;
	flags |= buf->flags & BufferFlag_Dynamic ? GL_DYNAMIC_STORAGE_BIT : 0;
	glNamedBufferStorage(buf->handle, size, data, flags);
}

void R_BufferUpdate(R_Buffer* _buf, u64 offset, u64 size, void* data) {
	R_GL46Buffer* buf = (R_GL46Buffer*) _buf;
	glNamedBufferSubData(buf->handle, offset, size, data);
}

void R_BufferFree(R_Buffer* _buf) {
	R_GL46Buffer* buf = (R_GL46Buffer*) _buf;
	glDeleteBuffers(1, &buf->handle);
}

//~ Shaders

void R_ShaderAlloc(R_Shader* _shader, string data, R_ShaderType type) {
	R_GL46Shader* shader = (R_GL46Shader*) _shader;
	shader->type = type;
	
	shader->handle = glCreateShader(get_shader_type_of(type));
	i32 size = data.size;
	glShaderSource(shader->handle, 1, (const GLchar *const *)&data.str, &size);
	glCompileShader(shader->handle);
	
	i32 ret = 0;
	glGetShaderiv(shader->handle, GL_COMPILE_STATUS, &ret);
    if (ret == GL_FALSE) {
		LogError("[GL46 Backend] Shader Compilation Failure:\n");
		
        i32 length;
        glGetShaderiv(shader->handle, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = calloc(length, sizeof(GLchar));
        glGetShaderInfoLog(shader->handle, length, NULL, info);
        LogError("%s\n", info);
        free(info);
    }
}

void R_ShaderAllocLoad(R_Shader* _shader, string fp, R_ShaderType type) {
	M_Arena arena;
	arena_init(&arena);
	string source_code = OS_FileRead(&arena, fp);
	R_ShaderAlloc(_shader, source_code, type);
	arena_free(&arena);
}

void R_ShaderFree(R_Shader* _shader) {
	R_GL46Shader* shader = (R_GL46Shader*) _shader;
	glDeleteShader(shader->handle);
}

void R_ShaderPackAlloc(R_ShaderPack* _pack, R_Shader* shaders, u32 shader_count) {
	R_GL46ShaderPack* pack = (R_GL46ShaderPack*) _pack;
	hash_table_init(string, i32, &pack->uniforms);
	
	pack->handle = glCreateProgram();
	for (u32 i = 0; i < shader_count; i++) {
		R_GL46Shader* shader = (R_GL46Shader*) &shaders[i];
		glAttachShader(pack->handle, shader->handle);
	}
	glLinkProgram(pack->handle);
	
	i32 ret = 0;
	glGetProgramiv(pack->handle, GL_LINK_STATUS, &ret);
	if (ret == GL_FALSE) {
		LogError("[GL46 Backend] Shader Compilation Failure:\n");
		
		i32 length;
		glGetProgramiv(pack->handle, GL_INFO_LOG_LENGTH, &length);
		GLchar *info = calloc(length, sizeof(GLchar));
		glGetProgramInfoLog(pack->handle, length, NULL, info);
		LogError("%s\n", info);
		free(info);
	}
	
	for (u32 i = 0; i < shader_count; i++) {
		R_GL46Shader* shader = (R_GL46Shader*) &shaders[i];
		glDetachShader(pack->handle, shader->handle);
	}
}

void R_ShaderPackAllocLoad(R_ShaderPack* _pack, string fp_prefix) {
	M_Scratch scratch = scratch_get();
	
	string vsfp = str_cat(&scratch.arena, fp_prefix, str_lit(".vert.glsl"));
	string fsfp = str_cat(&scratch.arena, fp_prefix, str_lit(".frag.glsl"));
	string gsfp = str_cat(&scratch.arena, fp_prefix, str_lit(".geom.glsl"));
	
	R_Shader* shader_buffer = arena_alloc(&scratch.arena, sizeof(R_Shader) * 3);
	u32 shader_count = 0;
	
	if (!OS_FileExists(vsfp))
		LogError("[GL46 Backend] The Vertex Shader File '%s.vert.glsl' doesn't exist", fp_prefix.str);
	else Log("[GL46 Backend] Loading Vertex Shader '%s.vert.glsl'", fp_prefix.str);
	R_ShaderAllocLoad(&shader_buffer[shader_count++], vsfp, ShaderType_Vertex);
	
	if (!OS_FileExists(fsfp))
		LogError("[GL46 Backend] The Fragment Shader File '%s.frag.glsl' doesn't exist", fp_prefix.str);
	else Log("[GL46 Backend] Loading Fragment Shader '%s.frag.glsl'", fp_prefix.str);
	R_ShaderAllocLoad(&shader_buffer[shader_count++], fsfp, ShaderType_Fragment);
	
	if (OS_FileExists(gsfp)) {
		Log("[GL46 Backend] Loading Geometry Shader '%s.geom.glsl'", fp_prefix.str);
		R_ShaderAllocLoad(&shader_buffer[shader_count++], gsfp, ShaderType_Geometry);
	}
	
	R_ShaderPackAlloc(_pack, shader_buffer, shader_count);
	
	for (u32 i = 0; i < shader_count; i++) {
		R_ShaderFree(&shader_buffer[i]);
	}
	
	scratch_return(&scratch);
}


void R_ShaderPackUploadMat4(R_ShaderPack* _pack, string name, mat4 mat) {
	R_GL46ShaderPack* pack = (R_GL46ShaderPack*) _pack;
	i32 loc;
    if (!hash_table_get(string, i32, &pack->uniforms, name, &loc)) {
        loc = glGetUniformLocation(pack->handle, (const GLchar*)name.str);
        hash_table_set(string, i32, &pack->uniforms, name, loc);
    }
    glUniformMatrix4fv(loc, 1, GL_TRUE, mat.a);
}

void R_ShaderPackUploadInt(R_ShaderPack* _pack, string name, i32 val) {
	R_GL46ShaderPack* pack = (R_GL46ShaderPack*) _pack;
	i32 loc;
    if (!hash_table_get(string, i32, &pack->uniforms, name, &loc)) {
        loc = glGetUniformLocation(pack->handle, (const GLchar*)name.str);
        hash_table_set(string, i32, &pack->uniforms, name, loc);
    }
    glUniform1i(loc, val);
}

void R_ShaderPackUploadIntArray(R_ShaderPack* _pack, string name, i32* vals, u32 count) {
	R_GL46ShaderPack* pack = (R_GL46ShaderPack*) _pack;
	i32 loc;
    if (!hash_table_get(string, i32,&pack->uniforms, name, &loc)) {
        loc = glGetUniformLocation(pack->handle, (const GLchar*)name.str);
        hash_table_set(string, i32, &pack->uniforms, name, loc);
    }
    glUniform1iv(loc, count, vals);
}

void R_ShaderPackUploadFloat(R_ShaderPack* _pack, string name, f32 val) {
	R_GL46ShaderPack* pack = (R_GL46ShaderPack*) _pack;
	i32 loc;
    if (!hash_table_get(string, i32, &pack->uniforms, name, &loc)) {
        loc = glGetUniformLocation(pack->handle, (const GLchar*)name.str);
        hash_table_set(string, i32, &pack->uniforms, name, loc);
    }
    glUniform1f(loc, val);
}

void R_ShaderPackUploadVec4(R_ShaderPack* _pack, string name, vec4 val) {
	R_GL46ShaderPack* pack = (R_GL46ShaderPack*) _pack;
	i32 loc;
    if (!hash_table_get(string, i32, &pack->uniforms, name, &loc)) {
        loc = glGetUniformLocation(pack->handle, (const GLchar*)name.str);
        hash_table_set(string, i32, &pack->uniforms, name, loc);
    }
    glUniform4f(loc, val.x, val.y, val.z, val.w);
}


void R_ShaderPackFree(R_ShaderPack* _pack) {
	R_GL46ShaderPack* pack = (R_GL46ShaderPack*) _pack;
	hash_table_free(string, i32, &pack->uniforms);
	glDeleteProgram(pack->handle);
}

//~ Vertex Input (VAOs)

void R_PipelineAlloc(R_Pipeline* _in, R_InputAssembly assembly, R_Attribute* attributes, u32 attribute_count, R_ShaderPack* shader, R_BlendMode blending) {
	R_GL46Pipeline* in = (R_GL46Pipeline*) _in;
	in->assembly = assembly;
	in->attributes = attributes;
	in->shader = (R_GL46ShaderPack*) shader;
	in->attribute_count = attribute_count;
	in->blend_mode = blending;
	glCreateVertexArrays(1, &in->handle);
}

void R_PipelineAddBuffer(R_Pipeline* _in, R_Buffer* _buf, u32 attribute_count) {
	R_GL46Pipeline* in = (R_GL46Pipeline*) _in;
	R_GL46Buffer* buf = (R_GL46Buffer*) _buf;
	
	glBindVertexArray(in->handle);
	u32 stride = 0;
	for (u32 i = in->attribpoint; i < in->attribpoint + attribute_count; i++) {
		stride += get_size_of(in->attributes[i].type);
	}
	
	u32 offset = 0;
	for (u32 i = in->attribpoint; i < in->attribpoint + attribute_count; i++) {
		glEnableVertexArrayAttrib(in->handle, i);
		glVertexArrayAttribFormat(in->handle, i, get_component_count_of(in->attributes[i].type), get_type_of(in->attributes[i].type), GL_FALSE, offset);
		glVertexArrayAttribBinding(in->handle, i, in->bindpoint);
		offset += get_size_of(in->attributes[i].type);
	}
	
	glVertexArrayVertexBuffer(in->handle, in->bindpoint, buf->handle, 0, stride);
	
	in->bindpoint++;
}

void R_PipelineBind(R_Pipeline* _in) {
	R_GL46Pipeline* in = (R_GL46Pipeline*) _in;
	glUseProgram(in->shader->handle);
	glBindVertexArray(in->handle);
	switch (in->blend_mode) {
		case BlendMode_None: {
			glDisable(GL_BLEND);
			break;
		}
		
		case BlendMode_Alpha: {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		} break;
		
		default: {
			glDisable(GL_BLEND);
		} break;
	}
}

void R_PipelineFree(R_Pipeline* _in) {
	R_GL46Pipeline* in = (R_GL46Pipeline*) _in;
	glDeleteVertexArrays(1, &in->handle);
}

//~ Textures

void R_Texture2DAlloc(R_Texture2D* _texture, R_TextureFormat format, u32 width, u32 height, R_TextureResizeParam min, R_TextureResizeParam mag, R_TextureWrapParam wrap_s, R_TextureWrapParam wrap_t) {
	R_GL46Texture2D* texture = (R_GL46Texture2D*) _texture;
	texture->width = width;
	texture->height = height;
	texture->min = min;
	texture->mag = mag;
	texture->wrap_s = wrap_s;
	texture->wrap_t = wrap_t;texture->format = format;
	glCreateTextures(GL_TEXTURE_2D, 1, &texture->handle);
	
	glTextureParameteri(texture->handle, GL_TEXTURE_WRAP_S, get_texture_wrap_param_type_of(wrap_s));
	glTextureParameteri(texture->handle, GL_TEXTURE_WRAP_T, get_texture_wrap_param_type_of(wrap_t));
	glTextureParameteri(texture->handle, GL_TEXTURE_MIN_FILTER, get_texture_resize_param_type_of(min));
	glTextureParameteri(texture->handle, GL_TEXTURE_MAG_FILTER, get_texture_resize_param_type_of(mag));
	
	glTextureStorage2D(texture->handle, 1,
					   get_texture_internal_format_type_of(format), width, height);
	
	AssertTrue(mag == TextureResize_Nearest || mag == TextureResize_Linear, "Magnification Filter for texture can only be Nearest or Linear");
}

void R_Texture2DAllocLoad(R_Texture2D* _texture, string filepath, R_TextureResizeParam min, R_TextureResizeParam mag, R_TextureWrapParam wrap_s, R_TextureWrapParam wrap_t) {
	i32 width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	u8* data = stbi_load((const char*)filepath.str, &width, &height, &channels, 0);
	
	if (channels == 3) {
		R_Texture2DAlloc(_texture, TextureFormat_RGB, width, height, min, mag, wrap_s, wrap_t);
	} else if (channels == 4) {
		R_Texture2DAlloc(_texture, TextureFormat_RGBA, width, height, min, mag, wrap_s, wrap_t);
	}
	
	R_Texture2DData(_texture, data);
	stbi_image_free(data);
}

void R_Texture2DSwizzle(R_Texture2D* _texture, i32* swizzles) {
	R_GL46Texture2D* texture = (R_GL46Texture2D*) _texture;
	GLint fixed[4] = {
		get_texture_channel_of(swizzles[0]),
		get_texture_channel_of(swizzles[1]),
		get_texture_channel_of(swizzles[2]),
		get_texture_channel_of(swizzles[3]),
	};
	glTextureParameteriv(texture->handle, GL_TEXTURE_SWIZZLE_RGBA, fixed);
}

void R_Texture2DData(R_Texture2D* _texture, void* data) {
	R_GL46Texture2D* texture = (R_GL46Texture2D*) _texture;
	u32 datatype = get_texture_datatype_of(texture->format);
	glTextureSubImage2D(texture->handle, 0, 0, 0, texture->width, texture->height, get_texture_format_type_of(texture->format), datatype, data);
}

b8 R_Texture2DEquals(R_Texture2D* _a, R_Texture2D* _b) {
	R_GL46Texture2D* a = (R_GL46Texture2D*) _a;
	R_GL46Texture2D* b = (R_GL46Texture2D*) _b;
	return a->handle == b->handle;
}

void R_Texture2DBindTo(R_Texture2D* _texture, u32 slot) {
	R_GL46Texture2D* texture = (R_GL46Texture2D*) _texture;
	glBindTextureUnit(slot, texture->handle);
}

void R_Texture2DFree(R_Texture2D* _texture) {
	R_GL46Texture2D* texture = (R_GL46Texture2D*) _texture;
	glDeleteTextures(1, &texture->handle);
}

//~ Framebuffers

void R_FramebufferCreate(R_Framebuffer* _framebuffer, u32 width, u32 height, R_Texture2D* color_attachments, u32 color_attachment_count, R_Texture2D depth_attachment) {
	if (!width) width = 1;
	if (!height) height = 1;
	R_GL46Framebuffer* ret = (R_GL46Framebuffer*) _framebuffer;
	glCreateFramebuffers(1, &ret->handle);
    glBindFramebuffer(GL_FRAMEBUFFER, ret->handle);
	ret->width = width;
    ret->height = height;
	ret->color_attachments = malloc(sizeof(R_Texture2D) * color_attachment_count);
	MemoryZero(ret->color_attachments, sizeof(R_Texture2D) * color_attachment_count);
	ret->depth_attachment = depth_attachment;
    ret->color_attachment_count = color_attachment_count;
	
	u32 draw_buffers_active[color_attachment_count];
    for (u32 i = 0; i < color_attachment_count; i++) {
        ret->color_attachments[i] = color_attachments[i];
        draw_buffers_active[i] = GL_COLOR_ATTACHMENT0 + i;
        glNamedFramebufferTexture(ret->handle, GL_COLOR_ATTACHMENT0 + i, ((R_GL46Texture2D*)&color_attachments[i])->handle, 0, 0);
    }
    glNamedFramebufferDrawBuffers(ret->handle, color_attachment_count, draw_buffers_active);
	
	if (depth_attachment.format != TextureFormat_Invalid) {
        AssertTrue(depth_attachment.format == TextureFormat_DepthStencil, "Depth Texture format is not TextureFormat_DepthStencil");
        glNamedFramebufferTexture(ret->handle, GL_DEPTH_STENCIL_ATTACHMENT, ((R_GL46Texture2D*)&depth_attachment)->handle, 0, 0);
    }
    
	if (glCheckNamedFramebufferStatus(ret->handle, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LogError("[GL46 Backend] Incomplete framebuffer with code %x", glCheckNamedFramebufferStatus(ret->handle, GL_FRAMEBUFFER));
	}
}

void R_FramebufferBind(R_Framebuffer* _framebuffer) {
	R_GL46Framebuffer* framebuffer = (R_GL46Framebuffer*) _framebuffer;
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle);
}

void R_FramebufferBindScreen(void) {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void R_FramebufferBlitToScreen(OS_Window* window, R_Framebuffer* _framebuffer) {
	R_GL46Framebuffer* framebuffer = (R_GL46Framebuffer*) _framebuffer;
	glBlitNamedFramebuffer(framebuffer->handle, 0,
						   0, 0, framebuffer->width, framebuffer->height,
						   0, 0, window->width, window->height,
						   GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void R_FramebufferReadPixel(R_Framebuffer* _framebuffer, u32 attachment, u32 x, u32 y,
							void* data) {
	R_GL46Framebuffer* framebuffer = (R_GL46Framebuffer*) _framebuffer;
    glNamedFramebufferReadBuffer(framebuffer->handle, GL_COLOR_ATTACHMENT0 + attachment);
	R_TextureFormat format = framebuffer->color_attachments[attachment].format;
    glReadPixels(x, y, 1, 1, get_texture_format_type_of(format), get_texture_datatype_of(format), data);
}

static void R_FramebufferDeleteInternal(R_Framebuffer* _framebuffer) {
	R_GL46Framebuffer* framebuffer = (R_GL46Framebuffer*) _framebuffer;
	for (u32 i = 0; i < framebuffer->color_attachment_count; i++) {
        R_Texture2DFree(&framebuffer->color_attachments[i]);
    }
    if (framebuffer->depth_attachment.format != TextureFormat_Invalid)
        R_Texture2DFree(&framebuffer->depth_attachment);
    glDeleteFramebuffers(1, &framebuffer->handle);
}

void R_FramebufferResize(R_Framebuffer* _framebuffer, u32 new_width, u32 new_height) {
	R_FramebufferDeleteInternal(_framebuffer);
	R_GL46Framebuffer* framebuffer = (R_GL46Framebuffer*) _framebuffer;
    
	if (!new_width) new_width = 1;
    if (!new_height) new_height = 1;
    glCreateFramebuffers(1, &framebuffer->handle);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->handle);
    framebuffer->width = new_width;
    framebuffer->height = new_height;
	
    for (u32 i = 0; i < framebuffer->color_attachment_count; i++) {
        R_Texture2D old_spec = framebuffer->color_attachments[i];
		R_Texture2DAlloc(&framebuffer->color_attachments[i], old_spec.format, new_width, new_height, old_spec.min, old_spec.mag, old_spec.wrap_s, old_spec.wrap_t);
        glNamedFramebufferTexture(framebuffer->handle, GL_COLOR_ATTACHMENT0 + i, ((R_GL46Texture2D*)&framebuffer->color_attachments[i])->handle, 0, 0);
    }
    
    if (framebuffer->depth_attachment.format != TextureFormat_Invalid) {
        AssertTrue(framebuffer->depth_attachment.format == TextureFormat_DepthStencil, "Depth Texture format is not TextureFormat_DepthStencil");
        
        R_Texture2D old_spec = framebuffer->depth_attachment;
		R_Texture2DAlloc(&framebuffer->depth_attachment, old_spec.format, new_width, new_height, old_spec.min, old_spec.mag, old_spec.wrap_s, old_spec.wrap_t);
        glNamedFramebufferTexture(framebuffer->handle, GL_DEPTH_STENCIL_ATTACHMENT, ((R_GL46Texture2D*)&framebuffer->depth_attachment)->handle, 0, 0);
    }
    
    if (glCheckNamedFramebufferStatus(framebuffer->handle, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        AssertTrue(false, "Framebuffer Incomplete");
    }
}

void R_FramebufferFree(R_Framebuffer* _framebuffer) {
	R_GL46Framebuffer* framebuffer = (R_GL46Framebuffer*) _framebuffer;
	for (u32 i = 0; i < framebuffer->color_attachment_count; i++) {
        R_Texture2DFree(&framebuffer->color_attachments[i]);
    }
	free(framebuffer->color_attachments);
    if (framebuffer->depth_attachment.format != TextureFormat_Invalid)
        R_Texture2DFree(&framebuffer->depth_attachment);
    glDeleteFramebuffers(1, &framebuffer->handle);
}

//~ Other

void R_Clear(R_BufferMask buffer_mask) {
	u32 mask = 0;
	mask |= buffer_mask & BufferMask_Color ? GL_COLOR_BUFFER_BIT : 0;
	mask |= buffer_mask & BufferMask_Depth ? GL_DEPTH_BUFFER_BIT : 0;
	mask |= buffer_mask & BufferMask_Stencil ? GL_STENCIL_BUFFER_BIT : 0;
	glClear(mask);
}

void R_ClearColor(f32 r, f32 g, f32 b, f32 a) {
	glClearColor(r, g, b, a);
}

void R_Viewport(i32 x, i32 y, i32 w, i32 h) {
	glViewport(x, y, w, h);
}

void R_Draw(R_Pipeline* _in, u32 start, u32 count) {
	R_GL46Pipeline* in = (R_GL46Pipeline*) _in;
	glDrawArrays(get_input_assembly_type_of(in->assembly), start, count);
}
