//~ OpenGL 3.3 Resources
#include "gl_functions.h"

typedef struct R_GL33Buffer {
	R_BufferFlags flags;
	u32 handle;
} R_GL33Buffer;

typedef struct R_GL33Shader {
	R_ShaderType type;
	u32 handle;
} R_GL33Shader;

typedef struct R_GL33ShaderPack {
	u32 handle;
} R_GL33ShaderPack;

typedef struct R_GL33Pipeline {
	R_InputAssembly assembly;
	R_Attribute* attributes;
	R_GL33ShaderPack* shader;
	u32 attribute_count;
	
	u32 bindpoint;
	u32 attribpoint;
	u32 handle;
} R_GL33Pipeline;


//~ Elpers

static u32 get_size_of(R_Attribute attrib) {
	AssertTrue(8 == Attribute_MAX, "Non Exhaustive switch statement: get_size_of in gl33 backend");
	switch (attrib) {
		case Attribute_Float1: return 1 * sizeof(f32);
		case Attribute_Float2: return 2 * sizeof(f32);
		case Attribute_Float3: return 3 * sizeof(f32);
		case Attribute_Float4: return 4 * sizeof(f32);
		case Attribute_Integer1: return 1 * sizeof(i32);
		case Attribute_Integer2: return 2 * sizeof(i32);
		case Attribute_Integer3: return 3 * sizeof(i32);
		case Attribute_Integer4: return 4 * sizeof(i32);
	}
	return 0;
}

static u32 get_component_count_of(R_Attribute attrib) {
	AssertTrue(8 == Attribute_MAX, "Non Exhaustive switch statement: get_component_count_of in gl33 backend");
	switch (attrib) {
		case Attribute_Float1: return 1;
		case Attribute_Float2: return 2;
		case Attribute_Float3: return 3;
		case Attribute_Float4: return 4;
		case Attribute_Integer1: return 1;
		case Attribute_Integer2: return 2;
		case Attribute_Integer3: return 3;
		case Attribute_Integer4: return 4;
	}
	return 0;
}

static u32 get_type_of(R_Attribute attrib) {
	AssertTrue(8 == Attribute_MAX, "Non Exhaustive switch statement: get_type_of in gl33 backend");
	switch (attrib) {
		case Attribute_Float1: return GL_FLOAT;
		case Attribute_Float2: return GL_FLOAT;
		case Attribute_Float3: return GL_FLOAT;
		case Attribute_Float4: return GL_FLOAT;
		case Attribute_Integer1: return GL_INT;
		case Attribute_Integer2: return GL_INT;
		case Attribute_Integer3: return GL_INT;
		case Attribute_Integer4: return GL_INT;
	}
	return GL_INVALID_ENUM;
}

static u32 get_shader_type_of(R_ShaderType type) {
	AssertTrue(3 == ShaderType_MAX, "Non Exhaustive switch statement: get_shader_type_of in gl33 backend");
	switch (type) {
		case ShaderType_Vertex: return GL_VERTEX_SHADER;
		case ShaderType_Fragment: return GL_FRAGMENT_SHADER;
		case ShaderType_Geometry: return GL_GEOMETRY_SHADER;
	}
	return GL_INVALID_ENUM;
}

//~ Function Implementations

void R_BufferAlloc(R_Buffer* _buf, R_BufferFlags flags) {
	R_GL33Buffer* buf = (R_GL33Buffer*) _buf;
	buf->flags = flags;
	glGenBuffers(1, &buf->handle);
}

void R_BufferData(R_Buffer* _buf, u64 size, void* data) {
	R_GL33Buffer* buf = (R_GL33Buffer*) _buf;
	u32 usage = buf->flags & BufferFlag_Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;
	glBindBuffer(GL_ARRAY_BUFFER, buf->handle);
	glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void R_BufferFree(R_Buffer* _buf) {
	R_GL33Buffer* buf = (R_GL33Buffer*) _buf;
	glDeleteBuffers(1, &buf->handle);
}

//~ Shaders

void R_ShaderAlloc(R_Shader* _shader, string data, R_ShaderType type) {
	R_GL33Shader* shader = (R_GL33Shader*) _shader;
	shader->type = type;
	
	shader->handle = glCreateShader(get_shader_type_of(type));
	i32 size = data.size;
	glShaderSource(shader->handle, 1, (const GLchar *const *)&data.str, &size);
	glCompileShader(shader->handle);
	
	i32 ret = 0;
	glGetShaderiv(shader->handle, GL_COMPILE_STATUS, &ret);
    if (ret == GL_FALSE) {
		LogError("Shader Compilation Failure: ");
		
        i32 length;
        glGetShaderiv(shader->handle, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = calloc(length, sizeof(GLchar));
        glGetShaderInfoLog(shader->handle, length, NULL, info);
        LogError("%s\n", info);
        free(info);
    }
}

void R_ShaderAllocLoad(R_Shader* _shader, string fp, R_ShaderType type) {
	M_Arena arena = {0};
	arena_init(&arena);
	string source_code = OS_FileRead(&arena, fp);
	R_ShaderAlloc(_shader, source_code, type);
	arena_free(&arena);
}

void R_ShaderFree(R_Shader* _shader) {
	R_GL33Shader* shader = (R_GL33Shader*) _shader;
	glDeleteShader(shader->handle);
}

void R_ShaderPackAlloc(R_ShaderPack* _pack, R_Shader* shaders, u32 shader_count) {
	R_GL33ShaderPack* pack = (R_GL33ShaderPack*) _pack;
	
	pack->handle = glCreateProgram();
	for (u32 i = 0; i < shader_count; i++) {
		R_GL33Shader* shader = (R_GL33Shader*) &shaders[i];
		glAttachShader(pack->handle, shader->handle);
	}
	glLinkProgram(pack->handle);
	
	i32 ret = 0;
	glGetProgramiv(pack->handle, GL_LINK_STATUS, &ret);
    if (ret == GL_FALSE) {
		LogError("Shader Compilation Failure: ");
		
        i32 length;
        glGetProgramiv(pack->handle, GL_INFO_LOG_LENGTH, &length);
        GLchar *info = calloc(length, sizeof(GLchar));
        glGetProgramInfoLog(pack->handle, length, NULL, info);
        LogError("%s\n", info);
        free(info);
    }
	
	for (u32 i = 0; i < shader_count; i++) {
		R_GL33Shader* shader = (R_GL33Shader*) &shaders[i];
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
		LogError("The Vertex Shader File '%s.vert.glsl' doesn't exist", fp_prefix.str);
	else Log("Loading Vertex Shader '%s.vert.glsl'", fp_prefix.str);
	R_ShaderAllocLoad(&shader_buffer[shader_count++], vsfp, ShaderType_Vertex);
	
	if (!OS_FileExists(fsfp))
		LogError("The Fragment Shader File '%s.frag.glsl' doesn't exist", fp_prefix.str);
	else Log("Loading Fragment Shader '%s.frag.glsl'", fp_prefix.str);
	R_ShaderAllocLoad(&shader_buffer[shader_count++], fsfp, ShaderType_Fragment);
	
	if (OS_FileExists(gsfp)) {
		Log("Loading Geometry Shader '%s.geom.glsl'", fp_prefix.str);
		R_ShaderAllocLoad(&shader_buffer[shader_count++], gsfp, ShaderType_Geometry);
	}
	
	R_ShaderPackAlloc(_pack, shader_buffer, shader_count);
	
	for (u32 i = 0; i < shader_count; i++) {
		R_ShaderFree(&shader_buffer[i]);
	}
	
	scratch_return(&scratch);
}

void R_ShaderPackFree(R_ShaderPack* _pack) {
	R_GL33ShaderPack* pack = (R_GL33ShaderPack*) _pack;
	glDeleteProgram(pack->handle);
}

//~ Vertex Input (VAOs)

void R_PipelineAlloc(R_Pipeline* _in, R_InputAssembly assembly, R_Attribute* attributes, R_ShaderPack* shader, u32 attribute_count) {
	R_GL33Pipeline* in = (R_GL33Pipeline*) _in;
	in->assembly = assembly;
	in->attributes = attributes;
	in->shader = (R_GL33ShaderPack*) shader;
	in->attribute_count = attribute_count;
	glGenVertexArrays(1, &in->handle);
}

void R_PipelineAddBuffer(R_Pipeline* _in, R_Buffer* _buf, u32 attribute_count) {
	R_GL33Pipeline* in = (R_GL33Pipeline*) _in;
	R_GL33Buffer* buf = (R_GL33Buffer*) _buf;
	
	glBindVertexArray(in->handle);
	u32 stride = 0;
	for (u32 i = in->attribpoint; i < in->attribpoint + attribute_count; i++) {
		stride += get_size_of(in->attributes[i]);
	}
	
	glBindBuffer(GL_ARRAY_BUFFER, buf->handle);
	u32 offset = 0;
	for (u32 i = in->attribpoint; i < in->attribpoint + attribute_count; i++) {
		glVertexAttribPointer(i, get_component_count_of(in->attributes[i]), get_type_of(in->attributes[i]), GL_FALSE, stride, (void*) offset);
		glEnableVertexAttribArray(i);
		offset += get_size_of(in->attributes[i]);
	}
}

void R_PipelineBind(R_Pipeline* _in) {
	R_GL33Pipeline* in = (R_GL33Pipeline*) _in;
	glUseProgram(in->shader->handle);
	glBindVertexArray(in->handle);
}

void R_PipelineFree(R_Pipeline* _in) {
	R_GL33Pipeline* in = (R_GL33Pipeline*) _in;
	glDeleteVertexArrays(1, &in->handle);
}
