//~ D3D11 Resources

#include "d3d11_resources.h"

#include "core/resources.h"
#include "os/impl/win32_window.h"

#include "d3d11_functions.h"

DArray_Impl(R_BufferAttribCountPack);

//~ For talking with the backend layer. Not exposed in header

static W32_Window* s_wnd = nullptr;
static f32 s_clear_color[4] = {0};

void __SetCurrentWindow(W32_Window* window) {
	s_wnd = window;
}

//~ Conversion Routines

static u32 get_bind_flag_of(R_BufferFlags flags) {
	if (flags & BufferFlag_Type_Vertex) return D3D11_BIND_VERTEX_BUFFER;
	if (flags & BufferFlag_Type_Index) return D3D11_BIND_INDEX_BUFFER;
	if (flags & BufferFlag_Type_Uniform) return D3D11_BIND_CONSTANT_BUFFER;
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
	desc.Usage = buf->flags & BufferFlag_Dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_IMMUTABLE;
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




void R_ShaderPackUploadMat4(R_ShaderPack* pack, string name, mat4 mat) {
	LogFatal("[D3D11 Backend] Unimplemented: Uniforms/Push Constants not supported for D3D11");
}

void R_ShaderPackUploadInt(R_ShaderPack* pack, string name, i32 val) {
	LogFatal("[D3D11 Backend] Unimplemented: Uniforms/Push Constants not supported for D3D11");
}

void R_ShaderPackUploadIntArray(R_ShaderPack* pack, string name, i32* vals, u32 count) {
	LogFatal("[D3D11 Backend] Unimplemented: Uniforms/Push Constants not supported for D3D11");
}

void R_ShaderPackUploadFloat(R_ShaderPack* pack, string name, f32 val) {
	LogFatal("[D3D11 Backend] Unimplemented: Uniforms/Push Constants not supported for D3D11");
}

void R_ShaderPackUploadVec4(R_ShaderPack* pack, string name, vec4 val) {
	LogFatal("[D3D11 Backend] Unimplemented: Uniforms/Push Constants not supported for D3D11");
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
	
	// TODO(voxel): Add Blending
	
	scratch_return(&scratch);
}

void R_PipelineFree(R_Pipeline* in) {
	SAFE_RELEASE(ID3D11InputLayout, in->layout);
	darray_free(R_BufferAttribCountPack, &in->buffers);
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
