//~ D3D11 Resources

#include "d3d11_resources.h"
#include <stb/stb_image.h>

#include "core/resources.h"
#include "os/impl/win32_window.h"

#include "d3d11_functions.h"

static b8 i32_is_null(i32 value) { return value == 0;  }
static b8 i32_is_tomb(i32 value) { return value == 69; }
HashTable_Impl(string, i32, str_is_null, str_eq, str_hash, 69, i32_is_null, i32_is_tomb);

DArray_Impl(R_BufferAttribCountPack);
DArray_Impl(R_UniformBufferHandle);

// Manual because C-API for the Shader Reflection studd doesn't exist
#define ID3D11ShaderReflection_Release(This) \
( (This)->lpVtbl -> Release(This) ) 

//~ For talking with the backend layer. Not exposed in header

static W32_Window* s_wnd = nullptr;
static f32 s_clear_color[4] = {0};
static R_BlendMode s_last_blend_mode;

void __SetCurrentWindow(W32_Window* window) {
	s_wnd = window;
}

//~ Conversion Routines

static u32 get_bind_flag_of(R_BufferFlags flags) {
	if (flags & BufferFlag_Type_Vertex) return D3D11_BIND_VERTEX_BUFFER;
	if (flags & BufferFlag_Type_Index) return D3D11_BIND_INDEX_BUFFER;
	LogError("[D3D11 Backend] Buffer should have one type defined");
	return 0;
}

static u32 get_primitive_topology_of(R_InputAssembly primitive) {
	AssertTrue(2 == InputAssembly_MAX, "Non Exhaustive switch statement: get_primitive_topology_of in d3d11 backend");
	switch (primitive) {
		case InputAssembly_Triangles: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case InputAssembly_Lines: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	}
	return 0;
}

static u32 get_format_of(R_AttributeType type) {
	AssertTrue(8 == AttributeType_MAX, "Non Exhaustive switch statement: get_component_count_of in gl33 backend");
	switch (type) {
		case AttributeType_Float1: return DXGI_FORMAT_R32_FLOAT;
		case AttributeType_Float2: return DXGI_FORMAT_R32G32_FLOAT;
		case AttributeType_Float3: return DXGI_FORMAT_R32G32B32_FLOAT;
		case AttributeType_Float4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case AttributeType_Integer1: return DXGI_FORMAT_R32_SINT;
		case AttributeType_Integer2: return DXGI_FORMAT_R32G32_SINT;
		case AttributeType_Integer3: return DXGI_FORMAT_R32G32B32_SINT;
		case AttributeType_Integer4: return DXGI_FORMAT_R32G32B32A32_SINT;
	}
	return 0;
}

static u32 get_size_of(R_AttributeType type) {
	AssertTrue(8 == AttributeType_MAX, "Non Exhaustive switch statement: get_component_count_of in gl33 backend");
	switch (type) {
		case AttributeType_Float1: return sizeof(f32) * 1;
		case AttributeType_Float2: return sizeof(f32) * 2;
		case AttributeType_Float3: return sizeof(f32) * 3;
		case AttributeType_Float4: return sizeof(f32) * 4;
		case AttributeType_Integer1: return sizeof(i32) * 1;
		case AttributeType_Integer2: return sizeof(i32) * 2;
		case AttributeType_Integer3: return sizeof(i32) * 3;
		case AttributeType_Integer4: return sizeof(i32) * 4;
	}
	return 0;
}

static string get_compile_target_of(R_ShaderType type) {
	switch (type) {
		case ShaderType_Vertex:   return str_lit("vs_5_0");
		case ShaderType_Fragment: return str_lit("ps_5_0");
		case ShaderType_Geometry: return str_lit("gs_5_0");
	}
	return str_lit("unknown");
}

static u32 get_texture_format_type_of(R_TextureFormat format) {
	AssertTrue(7 == TextureFormat_MAX,
			   "[D3D11 Backend] Non Exhaustive switch statement: get_texture_format_type_of");
	switch (format) {
		case TextureFormat_RInteger: return DXGI_FORMAT_R32_SINT;
		case TextureFormat_R: return DXGI_FORMAT_R8_UNORM;
		case TextureFormat_RG: return DXGI_FORMAT_R8G8_UNORM;
		
		case TextureFormat_RGB:
		LogFatal("[D3D11 Backend] RGB Format textures are not supported :(");
		break;
		
		case TextureFormat_RGBA: return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat_DepthStencil: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	}
	return 0;
}

static u32 get_texture_mutability_of(R_TextureMutability mut) {
	AssertTrue(3 == TextureMutability_MAX,
			   "[D3D11 Backend] Non Exhaustive switch statement: get_texture_mutability_of");
	switch (mut) {
		case TextureMutability_Immutable: return D3D11_USAGE_IMMUTABLE;
		case TextureMutability_Uncommon: return D3D11_USAGE_DEFAULT;
		case TextureMutability_Dynamic: return D3D11_USAGE_DYNAMIC;
	}
	return 0;
}

static u32 get_texture_usage_of(R_TextureUsage usage) {
	u32 ret = 0;
	if (usage & TextureUsage_ShaderResource) ret |= D3D11_BIND_SHADER_RESOURCE;
	if (usage & TextureUsage_Drawable) ret |= D3D11_BIND_RENDER_TARGET;
	return ret;
}

static u32 get_texture_cpuaccess_of(R_TextureMutability mut) {
	AssertTrue(3 == TextureMutability_MAX,
			   "[D3D11 Backend] Non Exhaustive switch statement: get_texture_cpuaccess_of");
	switch (mut) {
		case TextureMutability_Immutable: return 0;
		case TextureMutability_Uncommon: return D3D11_CPU_ACCESS_WRITE;
		case TextureMutability_Dynamic: return D3D11_CPU_ACCESS_WRITE;
	}
	return 0;
}

static u32 get_texture_datatype_size_of(R_TextureFormat format) {
    AssertTrue(7 == TextureFormat_MAX,
			   "[D3D11 Backend] Non Exhaustive switch statement: get_texture_datatype_size_of");
	switch (format) {
		case TextureFormat_RInteger: return sizeof(i32);
		case TextureFormat_R: return sizeof(u8);
		case TextureFormat_RG: return 2 * sizeof(u8);
		
		case TextureFormat_RGB:
		LogFatal("[D3D11 Backend] RGB Format textures are not supported :(");
		break;
		
		case TextureFormat_RGBA: return 4 * sizeof(u8);
		case TextureFormat_DepthStencil: return sizeof(u32);
	}
	return 0;
}

// What a monster function xD
// Switch of switches, using an array lookup somehow is probably more efficient...
// Many of these aren't even unique
static u32 get_texture_filter_of(R_TextureResizeParam min, R_TextureResizeParam mag) {
	AssertTrue(6 == TextureResize_MAX,
			   "[D3D11 Backend] Non Exhaustive switch statement: get_texture_datatype_size_of");
	switch (min) {
		case TextureResize_Nearest: {
			switch (mag) {
				case TextureResize_Nearest: return D3D11_FILTER_MIN_MAG_MIP_POINT;
				case TextureResize_Linear: return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				case TextureResize_LinearMipmapLinear: return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				case TextureResize_LinearMipmapNearest: return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
				case TextureResize_NearestMipmapLinear: return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
				case TextureResize_NearestMipmapNearest: return D3D11_FILTER_MIN_MAG_MIP_POINT;
			}
		} break;
		
		case TextureResize_Linear: {
			switch (mag) {
				case TextureResize_Nearest: return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
				case TextureResize_Linear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				case TextureResize_LinearMipmapLinear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				case TextureResize_LinearMipmapNearest: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				case TextureResize_NearestMipmapLinear: return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				case TextureResize_NearestMipmapNearest: return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			}
		} break;
		
		case TextureResize_LinearMipmapLinear: {
			switch (mag) {
				case TextureResize_Nearest: return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				case TextureResize_Linear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				case TextureResize_LinearMipmapLinear: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				case TextureResize_LinearMipmapNearest: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
				case TextureResize_NearestMipmapLinear: return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				case TextureResize_NearestMipmapNearest: return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
			}
		} break;
		
		case TextureResize_LinearMipmapNearest: {
			switch (mag) {
				case TextureResize_Nearest: return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
				case TextureResize_Linear: return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
				case TextureResize_LinearMipmapLinear: return D3D11_FILTER_MIN_MAG_MIP_POINT;
				case TextureResize_LinearMipmapNearest: return D3D11_FILTER_MIN_MAG_MIP_POINT;
				case TextureResize_NearestMipmapLinear: return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
				case TextureResize_NearestMipmapNearest: return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
			}
		} break;
		
		case TextureResize_NearestMipmapLinear: {
			switch (mag) {
				case TextureResize_Nearest: return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
				case TextureResize_Linear: return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				case TextureResize_LinearMipmapLinear: return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				case TextureResize_LinearMipmapNearest: return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				case TextureResize_NearestMipmapLinear: return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
				case TextureResize_NearestMipmapNearest: return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
			}
		} break;
		
		case TextureResize_NearestMipmapNearest: {
			switch (mag) {
				case TextureResize_Nearest: return D3D11_FILTER_MIN_MAG_MIP_POINT;
				case TextureResize_Linear: return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
				case TextureResize_LinearMipmapLinear: return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
				case TextureResize_LinearMipmapNearest: return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
				case TextureResize_NearestMipmapLinear: return D3D11_FILTER_MIN_MAG_MIP_POINT;
				case TextureResize_NearestMipmapNearest: return D3D11_FILTER_MIN_MAG_MIP_POINT;
			}
		} break;
	}
	return 0;
}

static u32 get_texture_wrap_param_of(R_TextureWrapParam wrap) {
	AssertTrue(5 == TextureWrap_MAX,
			   "[D3D11 Backend] Non Exhaustive switch statement: get_texture_wrap_param_of");
	switch (wrap) {
		case TextureWrap_ClampToEdge: return D3D11_TEXTURE_ADDRESS_CLAMP;
		case TextureWrap_ClampToBorder: return D3D11_TEXTURE_ADDRESS_BORDER;
		case TextureWrap_Repeat: return D3D11_TEXTURE_ADDRESS_WRAP;
		case TextureWrap_MirroredRepeat: return D3D11_TEXTURE_ADDRESS_MIRROR;
		case TextureWrap_MirrorClampToEdge: return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
	}
	return 0;
}

//~ Buffers

void R_BufferAlloc(R_Buffer* buf, R_BufferFlags flags, u32 v_stride) {
	buf->flags = flags;
	buf->v_stride = v_stride;
	// NOTE(voxel): Doesn't actually allocate any data. BufferData handles that
}

void R_BufferData(R_Buffer* buf, u64 size, void* data) {
	// Do a temporary allocation for initializing the buffer if no explicit data is passed in
	// This is so that there is consistency with the OpenGL backend
	if (!data) data = arena_alloc(U_GetFrameArena(), size);
	
	D3D11_SUBRESOURCE_DATA sd = {0};
	sd.pSysMem = data;
	
	D3D11_BUFFER_DESC desc = {0};
	desc.ByteWidth = size;
	desc.Usage = buf->flags & BufferFlag_Dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	desc.BindFlags = get_bind_flag_of(buf->flags);
	desc.CPUAccessFlags = buf->flags & BufferFlag_Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	desc.StructureByteStride = buf->v_stride;
	
	HRESULT hr;
	CHECK_HR(ID3D11Device_CreateBuffer(s_wnd->device, &desc, &sd, &buf->handle));
}

void R_BufferUpdate(R_Buffer* buf, u64 offset, u64 size, void* data) {
	D3D11_MAPPED_SUBRESOURCE mapped_res;
	ID3D11DeviceContext_Map(s_wnd->context, (ID3D11Resource*) buf->handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
	memmove(mapped_res.pData + offset, data, size);
	ID3D11DeviceContext_Unmap(s_wnd->context, (ID3D11Resource*) buf->handle, 0);
}

void R_BufferFree(R_Buffer* buf) {
	SAFE_RELEASE(ID3D11Buffer, buf->handle);
}



void R_UniformBufferAlloc(R_UniformBuffer* buf, string name, string_array member_names,
						  R_ShaderPack* pack, R_ShaderType type) {
	hash_table_init(string, i32, &buf->uniform_offsets);
	buf->name = name;
	buf->dirty = false;
	buf->stage = type;
	
	ID3D10Blob* blob = nullptr;
	switch (type) {
		case ShaderType_Vertex:   blob = pack->vs.bytecode_blob; break;
		case ShaderType_Fragment: blob = pack->ps.bytecode_blob; break;
		case ShaderType_Geometry: blob = pack->gs.bytecode_blob; break;
	}
	ID3D11ShaderReflection* shader_reflection;
	D3DReflect(ID3D10Blob_GetBufferPointer(blob), ID3D10Blob_GetBufferSize(blob), &IID_ID3D11ShaderReflection, (void**) &shader_reflection);
	ID3D11ShaderReflectionConstantBuffer* cb = shader_reflection->lpVtbl->GetConstantBufferByName(shader_reflection, (const char*) name.str);
	D3D11_SHADER_BUFFER_DESC cbdesc;
	cb->lpVtbl->GetDesc(cb, &cbdesc);
	
	buf->size = cbdesc.Size;
	buf->cpu_side_buffer = malloc(cbdesc.Size);
	MemoryZero(buf->cpu_side_buffer, cbdesc.Size);
	
	Iterate(member_names, i) {
		ID3D11ShaderReflectionVariable* var =
			cb->lpVtbl->GetVariableByName(cb, (const char*) member_names.elems[i].str);
		D3D11_SHADER_VARIABLE_DESC vardesc;
		var->lpVtbl->GetDesc(var, &vardesc);
		hash_table_set(string, i32, &buf->uniform_offsets, member_names.elems[i], (i32)vardesc.StartOffset);
	}
	
	SAFE_RELEASE(ID3D11ShaderReflection, shader_reflection);
	
	D3D11_BUFFER_DESC desc = {0};
	desc.ByteWidth = buf->size;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.StructureByteStride = 0;
	
	D3D11_SUBRESOURCE_DATA sd = {0};
	sd.pSysMem = buf->cpu_side_buffer;
	
	HRESULT hr;
	CHECK_HR(ID3D11Device_CreateBuffer(s_wnd->device, &desc, &sd, &buf->handle));
}

void R_UniformBufferFree(R_UniformBuffer* buf) {
	hash_table_free(string, i32, &buf->uniform_offsets);
	free(buf->cpu_side_buffer);
	SAFE_RELEASE(ID3D11Buffer, buf->handle);
}


void R_UniformBufferSetMat4(R_UniformBuffer* buf, string name, mat4 mat) {
	i32 offset = -1;
	if (!hash_table_get(string, i32, &buf->uniform_offsets, name, &offset)) {
		LogError("[GL33 Backend] Tried to set member '%.*s' of uniform buffer '%.*s' that doesn't exist", str_expand(name), str_expand(buf->name));
		return;
	}
	memmove(buf->cpu_side_buffer + offset, &mat, sizeof(mat4));
	buf->dirty = true;
}

void R_UniformBufferSetInt(R_UniformBuffer* buf, string name, i32 val) {
	i32 offset = -1;
	if (!hash_table_get(string, i32, &buf->uniform_offsets, name, &offset)) {
		LogError("[GL33 Backend] Tried to set member '%.*s' of uniform buffer '%.*s' that doesn't exist", str_expand(name), str_expand(buf->name));
		return;
	}
	memmove(buf->cpu_side_buffer + offset, &val, sizeof(i32));
	buf->dirty = true;
}

void R_UniformBufferSetIntArray(R_UniformBuffer* buf, string name, i32* vals, u32 count) {
	i32 offset = -1;
	if (!hash_table_get(string, i32, &buf->uniform_offsets, name, &offset)) {
		LogError("[GL33 Backend] Tried to set member '%.*s' of uniform buffer '%.*s' that doesn't exist", str_expand(name), str_expand(buf->name));
		return;
	}
	memmove(buf->cpu_side_buffer + offset, vals, sizeof(i32) * count);
	buf->dirty = true;
}

void R_UniformBufferSetFloat(R_UniformBuffer* buf, string name, f32 val) {
	i32 offset = -1;
	if (!hash_table_get(string, i32, &buf->uniform_offsets, name, &offset)) {
		LogError("[GL33 Backend] Tried to set member '%.*s' of uniform buffer '%.*s' that doesn't exist", str_expand(name), str_expand(buf->name));
		return;
	}
	memmove(buf->cpu_side_buffer + offset, &val, sizeof(f32));
	buf->dirty = true;
}

void R_UniformBufferSetVec4(R_UniformBuffer* buf, string name, vec4 val) {
	i32 offset = -1;
	if (!hash_table_get(string, i32, &buf->uniform_offsets, name, &offset)) {
		LogError("[GL33 Backend] Tried to set member '%.*s' of uniform buffer '%.*s' that doesn't exist", str_expand(name), str_expand(buf->name));
		return;
	}
	memmove(buf->cpu_side_buffer + offset, &val, sizeof(vec4));
	buf->dirty = true;
}

//~ Shaders

void R_ShaderAlloc(R_Shader* shader, string data, R_ShaderType type) {
	shader->type = type;
	
	HRESULT hr;
	
	ID3DBlob* error_blob;
	u32 flags = 0; // @debug add the debug flag here
	
	CHECK_HR(D3DCompile(data.str, data.size, nullptr, nullptr, nullptr, "main",
						(const char*) get_compile_target_of(type).str, flags, 0, &shader->bytecode_blob,
						&error_blob));
	
	if (FAILED(hr)) {
		LogError("[D3D11 Backend] Shader Compilation Failure:\n %.*s",
				 (i32)ID3D10Blob_GetBufferSize(error_blob),
				 (const char*) ID3D10Blob_GetBufferPointer(error_blob));
	} else if (error_blob) {
		LogError("[D3D11 Backend] Shader Compilation Warning:\n %.*s",
				 (i32)ID3D10Blob_GetBufferSize(error_blob),
				 (const char*) ID3D10Blob_GetBufferPointer(error_blob));
	}
	
	switch (type) {
		case ShaderType_Vertex: {
			CHECK_HR(ID3D11Device_CreateVertexShader(s_wnd->device,
													 ID3D10Blob_GetBufferPointer(shader->bytecode_blob),
													 ID3D10Blob_GetBufferSize(shader->bytecode_blob),
													 nullptr, &shader->vs));
		} break;
		
		case ShaderType_Fragment: {
			CHECK_HR(ID3D11Device_CreatePixelShader(s_wnd->device,
													ID3D10Blob_GetBufferPointer(shader->bytecode_blob),
													ID3D10Blob_GetBufferSize(shader->bytecode_blob),
													nullptr, &shader->ps));
		} break;
		
		case ShaderType_Geometry: {
			CHECK_HR(ID3D11Device_CreateGeometryShader(s_wnd->device,
													   ID3D10Blob_GetBufferPointer(shader->bytecode_blob),
													   ID3D10Blob_GetBufferSize(shader->bytecode_blob),
													   nullptr, &shader->gs));
		} break;
		
		default: {
			LogFatal("[D3D11 Backend] Unsupported Shader Type: %u", type);
		} break;
	}
	
	if (FAILED(hr)) {
		SAFE_RELEASE(ID3D10Blob, shader->bytecode_blob);
		LogFatal("[D3D11 Backend] CreateShader failed: %u", type);
	}
}

void R_ShaderAllocLoad(R_Shader* shader, string fp, R_ShaderType type) {
	string source_code = OS_FileRead(U_GetFrameArena(), fp);
	R_ShaderAlloc(shader, source_code, type);
}

void R_ShaderFree(R_Shader* shader) {
	SAFE_RELEASE(ID3D10Blob, shader->bytecode_blob);
	switch (shader->type) {
		case ShaderType_Vertex: SAFE_RELEASE(ID3D11VertexShader, shader->vs); break;
		case ShaderType_Fragment: SAFE_RELEASE(ID3D11PixelShader, shader->ps); break;
		case ShaderType_Geometry: SAFE_RELEASE(ID3D11GeometryShader, shader->gs); break;
	}
}



void R_ShaderPackAlloc(R_ShaderPack* pack, R_Shader* shaders, u32 shader_count) {
	for (u32 i = 0; i < shader_count; i++) {
		switch (shaders[i].type) {
			case ShaderType_Vertex:   pack->vs = shaders[i]; break;
			case ShaderType_Fragment: pack->ps = shaders[i]; break;
			case ShaderType_Geometry: pack->gs = shaders[i]; break;
		}
	}
}

void R_ShaderPackAllocLoad(R_ShaderPack* pack, string fp_prefix) {
	M_Scratch scratch = scratch_get();
	
	string vsfp = str_cat(&scratch.arena, fp_prefix, str_lit(".vert.hlsl"));
	string fsfp = str_cat(&scratch.arena, fp_prefix, str_lit(".frag.hlsl"));
	string gsfp = str_cat(&scratch.arena, fp_prefix, str_lit(".geom.hlsl"));
	
	
	R_Shader* shader_buffer = arena_alloc(&scratch.arena, sizeof(R_Shader) * 3);
	u32 shader_count = 0;
	
	if (!OS_FileExists(vsfp))
		LogError("[D3D11 Backend] The Vertex Shader File '%s.vert.hlsl' doesn't exist",
				 fp_prefix.str);
	else Log("[D3D11 Backend] Loading Vertex Shader '%s.vert.hlsl'", fp_prefix.str);
	R_ShaderAllocLoad(&shader_buffer[shader_count++], vsfp, ShaderType_Vertex);
	
	if (!OS_FileExists(fsfp))
		LogError("[D3D11 Backend] The Fragment Shader File '%s.frag.hlsl' doesn't exist",
				 fp_prefix.str);
	else Log("[D3D11 Backend] Loading Fragment Shader '%s.frag.hlsl'", fp_prefix.str);
	R_ShaderAllocLoad(&shader_buffer[shader_count++], fsfp, ShaderType_Fragment);
	
	if (OS_FileExists(gsfp)) {
		Log("[D3D11 Backend] Loading Geometry Shader '%s.geom.hlsl'", fp_prefix.str);
		R_ShaderAllocLoad(&shader_buffer[shader_count++], gsfp, ShaderType_Geometry);
	}
	
	R_ShaderPackAlloc(pack, shader_buffer, shader_count);
	
	scratch_return(&scratch);
}

void R_ShaderPackFree(R_ShaderPack* pack) {
	if (pack->vs.type) R_ShaderFree((R_Shader*) &pack->vs);
	if (pack->ps.type) R_ShaderFree((R_Shader*) &pack->ps);
	if (pack->gs.type) R_ShaderFree((R_Shader*) &pack->gs);
}


// TODO(voxel): Change to Asserts once debug break is a thing

void R_ShaderPackUploadMat4(R_ShaderPack* pack, string name, mat4 mat) {
	LogError("[D3D11 backend] Global Shader Uniforms are not supported");
	LogFatal("[D3D11 backend] use a #if defined(BACKEND_D3D11) and handle this differently");
}

void R_ShaderPackUploadInt(R_ShaderPack* pack, string name, i32 val) {
	LogError("[D3D11 backend] Global Shader Uniforms are not supported");
	LogFatal("[D3D11 backend] use a #if defined(BACKEND_D3D11) and handle this differently");
}

void R_ShaderPackUploadIntArray(R_ShaderPack* pack, string name, i32* vals, u32 count) {
	LogError("[D3D11 backend] Global Shader Uniforms are not supported");
	LogFatal("[D3D11 backend] use a #if defined(BACKEND_D3D11) and handle this differently");
}

void R_ShaderPackUploadFloat(R_ShaderPack* pack, string name, f32 val) {
	LogError("[D3D11 backend] Global Shader Uniforms are not supported");
	LogFatal("[D3D11 backend] use a #if defined(BACKEND_D3D11) and handle this differently");
}

void R_ShaderPackUploadVec4(R_ShaderPack* pack, string name, vec4 val) {
	LogError("[D3D11 backend] Global Shader Uniforms are not supported");
	LogFatal("[D3D11 backend] use a #if defined(BACKEND_D3D11) and handle this differently");
}


//~ Pipelines

void R_PipelineAlloc(R_Pipeline* in, R_InputAssembly assembly, R_Attribute* attributes, u32 attribute_count, R_ShaderPack* shader, R_BlendMode blending) {
	in->assembly = assembly;
	in->attributes = attributes;
	in->attribute_count = attribute_count;
	in->shader = (R_ShaderPack*) shader;
	in->blend_mode = blending;
}

void R_PipelineAddBuffer(R_Pipeline* in, R_Buffer* buf, u32 attribute_count) {
	in->layout_changed = true;
	darray_add(R_BufferAttribCountPack, &in->buffers,
			   ((R_BufferAttribCountPack) { .b = buf, .attrib_count = attribute_count }));
	
	// TODO(voxel): Handle Index Buffers
}

void R_PipelineAddUniformBuffer(R_Pipeline* in, R_UniformBuffer* buf) {
	switch (buf->stage) {
		case ShaderType_Vertex: {
			darray_add(R_UniformBufferHandle, &in->vs_uniform_buffers, buf);
		} break;
		
		case ShaderType_Fragment: {
			darray_add(R_UniformBufferHandle, &in->ps_uniform_buffers, buf);
		} break;
		
		case ShaderType_Geometry: {
			darray_add(R_UniformBufferHandle, &in->gs_uniform_buffers, buf);
		} break;
	}
}

void R_PipelineBind(R_Pipeline* in) {
	M_Scratch scratch = scratch_get();
	
	ID3D11Buffer** buf_arr = arena_alloc(&scratch.arena, sizeof(ID3D11Buffer*) * in->buffers.len);
	u32* strides_arr = arena_alloc(&scratch.arena, sizeof(u32) * in->buffers.len);
	u32* offsets_arr = arena_alloc(&scratch.arena, sizeof(u32) * in->buffers.len);
	u32 total_attrib_ct = 0;
	Iterate(in->buffers, i) {
		R_Buffer* curr = in->buffers.elems[i].b;
		total_attrib_ct += in->buffers.elems[i].attrib_count;
		buf_arr[i] = curr->handle;
		strides_arr[i] = curr->v_stride;
		offsets_arr[i] = 0;
	}
	
	ID3D11DeviceContext_IASetVertexBuffers(s_wnd->context, 0, in->buffers.len, buf_arr, strides_arr, offsets_arr);
	
	
	ID3D11DeviceContext_VSSetShader(s_wnd->context, in->shader->vs.vs, nullptr, 0);
	if (in->shader->gs.type)
		ID3D11DeviceContext_GSSetShader(s_wnd->context, in->shader->gs.gs, nullptr, 0);
	ID3D11DeviceContext_PSSetShader(s_wnd->context, in->shader->ps.ps, nullptr, 0);
	
	// Vertex Shader Uniform Buffers
	if (in->vs_uniform_buffers.len) {
		buf_arr = arena_alloc(&scratch.arena, sizeof(ID3D11Buffer*) * in->vs_uniform_buffers.len);
		// Pack and update Uniform Buffers
		Iterate(in->vs_uniform_buffers, i) {
			R_UniformBuffer* curr = in->vs_uniform_buffers.elems[i];
			buf_arr[i] = curr->handle;
			
			// Update UBO if dirty
			if (curr->dirty) {
				D3D11_MAPPED_SUBRESOURCE mapped_res;
				ID3D11DeviceContext_Map(s_wnd->context, (ID3D11Resource*) curr->handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
				memmove(mapped_res.pData, curr->cpu_side_buffer, curr->size);
				ID3D11DeviceContext_Unmap(s_wnd->context, (ID3D11Resource*) curr->handle, 0);
				
				curr->dirty = false;
			}
		}
		ID3D11DeviceContext_VSSetConstantBuffers(s_wnd->context, 0, in->vs_uniform_buffers.len, buf_arr);
	}
	// Pixel Shader Uniform Buffers
	if (in->ps_uniform_buffers.len) {
		buf_arr = arena_alloc(&scratch.arena, sizeof(ID3D11Buffer*) * in->ps_uniform_buffers.len);
		// Pack and update Uniform Buffers
		Iterate(in->ps_uniform_buffers, i) {
			R_UniformBuffer* curr = in->ps_uniform_buffers.elems[i];
			buf_arr[i] = curr->handle;
			
			// Update UBO if dirty
			if (curr->dirty) {
				D3D11_MAPPED_SUBRESOURCE mapped_res;
				ID3D11DeviceContext_Map(s_wnd->context, (ID3D11Resource*) curr->handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
				memmove(mapped_res.pData, curr->cpu_side_buffer, curr->size);
				ID3D11DeviceContext_Unmap(s_wnd->context, (ID3D11Resource*) curr->handle, 0);
				
				curr->dirty = false;
			}
		}
		ID3D11DeviceContext_PSSetConstantBuffers(s_wnd->context, 0, in->ps_uniform_buffers.len, buf_arr);
	}
	// Geometry Shader Uniform Buffers
	if (in->gs_uniform_buffers.len) {
		buf_arr = arena_alloc(&scratch.arena, sizeof(ID3D11Buffer*) * in->gs_uniform_buffers.len);
		// Pack and update Uniform Buffers
		Iterate(in->gs_uniform_buffers, i) {
			R_UniformBuffer* curr = in->gs_uniform_buffers.elems[i];
			buf_arr[i] = curr->handle;
			
			// Update UBO if dirty
			if (curr->dirty) {
				D3D11_MAPPED_SUBRESOURCE mapped_res;
				ID3D11DeviceContext_Map(s_wnd->context, (ID3D11Resource*) curr->handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
				memmove(mapped_res.pData, curr->cpu_side_buffer, curr->size);
				ID3D11DeviceContext_Unmap(s_wnd->context, (ID3D11Resource*) curr->handle, 0);
				
				curr->dirty = false;
			}
		}
		ID3D11DeviceContext_GSSetConstantBuffers(s_wnd->context, 0, in->gs_uniform_buffers.len, buf_arr);
	}
	
	// Recreate Layout if necessary
	if (in->layout_changed || !in->layout) {
		D3D11_INPUT_ELEMENT_DESC* elements = arena_alloc(&scratch.arena, sizeof(D3D11_INPUT_ELEMENT_DESC) * total_attrib_ct);
		u32 curr_buf_idx = 0;
		u32 ticker = 0;
		u32 offset = 0;
		
		for (u32 i = 0; i < total_attrib_ct; i++) {
			elements[i].SemanticName = (const char*) in->attributes[i].name.str;
			
			// @check
			elements[i].SemanticIndex = 0;
			
			elements[i].Format = get_format_of(in->attributes[i].type);
			elements[i].InputSlot = curr_buf_idx;
			elements[i].AlignedByteOffset = offset;
			elements[i].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elements[i].InstanceDataStepRate = 0;
			ticker += 1;
			offset += get_size_of(in->attributes[i].type);
			if (in->buffers.elems[curr_buf_idx].attrib_count == ticker) {
				ticker = 0;
				offset = 0;
				curr_buf_idx += 1;
			}
		}
		
		in->layout_changed = false;
		HRESULT hr;
		CHECK_HR(ID3D11Device_CreateInputLayout(s_wnd->device, elements, total_attrib_ct, ID3D10Blob_GetBufferPointer(in->shader->vs.bytecode_blob),
												ID3D10Blob_GetBufferSize(in->shader->vs.bytecode_blob), &in->layout));
	}
	
	ID3D11DeviceContext_IASetInputLayout(s_wnd->context, in->layout);
	
	// TODO(voxel): Offload to Framebuffer API
	ID3D11DeviceContext_OMSetRenderTargets(s_wnd->context, 1, &s_wnd->rtv, nullptr);
	
	ID3D11DeviceContext_IASetPrimitiveTopology(s_wnd->context, get_primitive_topology_of(in->assembly));
	
	if (in->blend_mode != s_last_blend_mode) {
		s_last_blend_mode = in->blend_mode;
		HRESULT hr;
		switch (s_last_blend_mode) {
			case BlendMode_None: {
				D3D11_BLEND_DESC desc = {0};
				desc.RenderTarget[0].BlendEnable = false;
				SAFE_RELEASE(ID3D11BlendState, in->blend_state);
				CHECK_HR(ID3D11Device_CreateBlendState(s_wnd->device, &desc, &in->blend_state));
			} break;
			
			case BlendMode_Alpha: {
				D3D11_BLEND_DESC desc = {0};
				desc.RenderTarget[0].BlendEnable = true;
				desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
				desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
				desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
				desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
				desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
				desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
				desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
				SAFE_RELEASE(ID3D11BlendState, in->blend_state);
				CHECK_HR(ID3D11Device_CreateBlendState(s_wnd->device, &desc, &in->blend_state));
			} break;
			
			default: {
				D3D11_BLEND_DESC desc = {0};
				desc.RenderTarget[0].BlendEnable = false;
				SAFE_RELEASE(ID3D11BlendState, in->blend_state);
				CHECK_HR(ID3D11Device_CreateBlendState(s_wnd->device, &desc, &in->blend_state));
			} break;
		}
		ID3D11DeviceContext_OMSetBlendState(s_wnd->context, in->blend_state, nullptr, 0xFFFFFF);
	}
	
	// TODO(voxel): Add Blending
	scratch_return(&scratch);
}

void R_PipelineFree(R_Pipeline* in) {
	darray_free(R_UniformBufferHandle, &in->vs_uniform_buffers);
	darray_free(R_UniformBufferHandle, &in->ps_uniform_buffers);
	darray_free(R_UniformBufferHandle, &in->gs_uniform_buffers);
	darray_free(R_BufferAttribCountPack, &in->buffers);
	SAFE_RELEASE(ID3D11BlendState, in->blend_state);
	SAFE_RELEASE(ID3D11InputLayout, in->layout);
}

//~ Textures

void R_Texture2DAlloc(R_Texture2D* texture,  R_TextureFormat format, u32 width, u32 height,
					  R_TextureResizeParam min, R_TextureResizeParam mag, R_TextureWrapParam wrap_s,
					  R_TextureWrapParam wrap_t, R_TextureMutability mut, R_TextureUsage usage,
					  void* initial_data) {
	texture->width = width;
	texture->height = height;
	texture->format = format;
	texture->min = min;
	texture->mag = mag;
	texture->wrap_s = wrap_s;
	texture->wrap_t = wrap_t;
	texture->mut = mut;
	texture->usage = usage;
	
	D3D11_TEXTURE2D_DESC desc = {0};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = get_texture_format_type_of(format);
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = get_texture_mutability_of(mut);
	desc.BindFlags = get_texture_usage_of(usage);
	desc.CPUAccessFlags = get_texture_cpuaccess_of(mut);
	desc.MiscFlags = 0;
	
	// Do a temporary allocation for initializing the texture if no explicit data is passed in
	// This is so that there is consistency with the OpenGL backend
	if (!initial_data) {
		initial_data =
			arena_alloc(U_GetFrameArena(), width * height * get_texture_datatype_size_of(format));
	}
	
	D3D11_SUBRESOURCE_DATA sd = {0};
	sd.pSysMem = initial_data;
	sd.SysMemPitch = width * get_texture_datatype_size_of(format);
	
	HRESULT hr;
	CHECK_HR(ID3D11Device_CreateTexture2D(s_wnd->device, &desc, &sd, &texture->handle));
	
	if (usage & TextureUsage_ShaderResource) {
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {0};
		srv_desc.Format = desc.Format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MostDetailedMip = 0;
		srv_desc.Texture2D.MipLevels = 1;
		CHECK_HR(ID3D11Device_CreateShaderResourceView(s_wnd->device, (ID3D11Resource*) texture->handle, &srv_desc, &texture->shader_resource_view));
	}
	
	if (usage & TextureUsage_Drawable) {
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc = {0};
		rtv_desc.Format = desc.Format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtv_desc.Texture2D.MipSlice = 0;
		CHECK_HR(ID3D11Device_CreateRenderTargetView(s_wnd->device, (ID3D11Resource*) texture->handle, &rtv_desc, &texture->render_target_view));
	}
	
	D3D11_SAMPLER_DESC sampler_desc = {0};
	sampler_desc.Filter = get_texture_filter_of(min, mag);
	sampler_desc.AddressU = get_texture_wrap_param_of(wrap_s);
	sampler_desc.AddressV = get_texture_wrap_param_of(wrap_t);
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	
	CHECK_HR(ID3D11Device_CreateSamplerState(s_wnd->device, &sampler_desc, &texture->sampler));
}

void R_Texture2DAllocLoad(R_Texture2D* texture, string filepath, R_TextureResizeParam min,
						  R_TextureResizeParam mag, R_TextureWrapParam wrap_s,
						  R_TextureWrapParam wrap_t, R_TextureMutability mut, R_TextureUsage usage) {
	i32 width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	u8* data = stbi_load((const char*)filepath.str, &width, &height, &channels, STBI_rgb_alpha);
	
	// Regardless of channels, we HAVE to use RGBA textures since RGB is literally not
	// supported by d3d11 for some bizarre reason. I hate this :(((
	R_Texture2DAlloc(texture, TextureFormat_RGBA, width, height, min, mag, wrap_s, wrap_t, mut, usage, data);
	
	stbi_image_free(data);
}


void R_Texture2DSwizzle(R_Texture2D* texture, i32* swizzles) {
	// TODO(voxel): Not possible with D3D11. Remove this
}

void R_Texture2DData(R_Texture2D* texture, void* data) {
	// TODO(voxel): Uncommon v dynamic updates
	if (texture->mut == TextureMutability_Dynamic) {
		D3D11_MAPPED_SUBRESOURCE mapped_res;
		ID3D11DeviceContext_Map(s_wnd->context, (ID3D11Resource*) texture->handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_res);
		memmove(mapped_res.pData, data, texture->width * texture->height * get_texture_datatype_size_of(texture->format));
		ID3D11DeviceContext_Unmap(s_wnd->context, (ID3D11Resource*) texture->handle, 0);
	} else if (texture->mut == TextureMutability_Uncommon) {
		// TODO(voxel): Check if this actually works
		
		D3D11_BOX update_box = (D3D11_BOX) {
			.left = 0,
			.right = texture->width * get_texture_datatype_size_of(texture->format),
			.top = 0,
			.bottom = texture->height * get_texture_datatype_size_of(texture->format),
			.front = 0,
			.back = 0,
		};
		ID3D11DeviceContext_UpdateSubresource(s_wnd->context, (ID3D11Resource*) texture->handle, 0,
											  &update_box, data,
											  texture->width * get_texture_datatype_size_of(texture->format), 1);
	} else if (texture->mut == TextureMutability_Immutable) {
		LogError("[D3D11 Backend] Tried to change an Immutable Texture");
	}
}

b8 R_Texture2DEquals(R_Texture2D* a, R_Texture2D* b) {
	return a->handle == b->handle;
}

void R_Texture2DBindTo(R_Texture2D* texture, u32 slot) {
	ID3D11DeviceContext_VSSetShaderResources(s_wnd->context, slot, 1, &texture->shader_resource_view);
	ID3D11DeviceContext_VSSetSamplers(s_wnd->context, slot, 1, &texture->sampler);
	ID3D11DeviceContext_PSSetShaderResources(s_wnd->context, slot, 1, &texture->shader_resource_view);
	ID3D11DeviceContext_PSSetSamplers(s_wnd->context, slot, 1, &texture->sampler);
	ID3D11DeviceContext_GSSetShaderResources(s_wnd->context, slot, 1, &texture->shader_resource_view);
	ID3D11DeviceContext_GSSetSamplers(s_wnd->context, slot, 1, &texture->sampler);
}

void R_Texture2DFree(R_Texture2D* texture) {
	SAFE_RELEASE(ID3D11Texture2D, texture->handle);
	SAFE_RELEASE(ID3D11ShaderResourceView, texture->shader_resource_view);
	SAFE_RELEASE(ID3D11RenderTargetView, texture->render_target_view);
	SAFE_RELEASE(ID3D11SamplerState, texture->sampler);
}

//~ Other

void R_ClearColor(f32 r, f32 g, f32 b, f32 a) {
	s_clear_color[0] = r;
	s_clear_color[1] = g;
	s_clear_color[2] = b;
	s_clear_color[3] = a;
}

void R_Clear(R_BufferMask buffer_mask) {
	if (buffer_mask & BufferMask_Color)
		ID3D11DeviceContext_ClearRenderTargetView(s_wnd->context, s_wnd->rtv, s_clear_color);
	
	// TODO(voxel): DepthAndStencil Clear
}

void R_Viewport(i32 x, i32 y, i32 w, i32 h) {
	HRESULT hr;
	
	SAFE_RELEASE(ID3D11RenderTargetView, s_wnd->rtv);
	
	CHECK_HR(IDXGISwapChain_ResizeBuffers(s_wnd->swapchain, 1, (u32)w, (u32)h, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	
	ID3D11Resource* back_buffer_resource = nullptr;
	IDXGISwapChain_GetBuffer(s_wnd->swapchain, 0, &IID_ID3D11Resource,
							 (void**) &back_buffer_resource);
	CHECK_HR(ID3D11Device_CreateRenderTargetView(s_wnd->device, back_buffer_resource, nullptr, &s_wnd->rtv));
	ID3D11Resource_Release(back_buffer_resource);
	
	D3D11_VIEWPORT view = {0};
	view.Width = (f32) w;
	view.Height = (f32) h;
	view.MinDepth = 0.f;
	view.MaxDepth = 1.f;
	view.TopLeftX = (f32) x;
	view.TopLeftY = (f32) y;
	ID3D11DeviceContext_RSSetViewports(s_wnd->context, 1, &view);
}

void R_Draw(R_Pipeline* pipeline, u32 start, u32 count) {
	ID3D11DeviceContext_Draw(s_wnd->context, count, start);
}
