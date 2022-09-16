#include "render_2d.h"

DArray_Impl(R2D_Batch);

static R2D_Batch* R2D_NextBatch(R2D_Renderer* renderer) {
    R2D_Batch* next = &renderer->batches.elems[++renderer->current_batch];
    
    if (renderer->current_batch >= renderer->batches.len) {
		darray_add(R2D_Batch, &renderer->batches, (R2D_Batch) {});
		next = &renderer->batches.elems[renderer->current_batch];
        next->cache = R2D_VertexCacheCreate(renderer->arena, R2D_MAX_INTERNAL_CACHE_VCOUNT);
    }
    return next;
}

static b8 R2D_BatchCanAddTexture(R2D_Renderer* renderer, R2D_Batch* batch, R_Texture2D* texture) {
    if (batch->tex_count < 8) return true;
    for (u8 i = 0; i < batch->tex_count; i++) {
        if (R_Texture2DEquals(batch->textures[i], texture))
            return true;
    }
    return false;
}

static u8 R2D_BatchAddTexture(R2D_Renderer* renderer, R2D_Batch* batch, R_Texture2D* tex) {
    for (u8 i = 0; i < batch->tex_count; i++) {
		if (R_Texture2DEquals(batch->textures[i], tex))
            return i;
    }
    batch->textures[batch->tex_count] = tex;
    return batch->tex_count++;
}

static R2D_Batch* R2D_BatchGetCurrent(R2D_Renderer* renderer, int num_verts, R_Texture2D* tex) {
    R2D_Batch* batch = &renderer->batches.elems[renderer->current_batch];
    if (!R2D_BatchCanAddTexture(renderer, batch, tex) || batch->cache.count + num_verts >= batch->cache.max_verts)
        batch = R2D_NextBatch(renderer);
    return batch;
}

//~ Vertex Cache

R2D_VertexCache R2D_VertexCacheCreate(M_Arena* arena, u32 max_verts) {
	return (R2D_VertexCache) {
        .vertices = arena_alloc(arena, sizeof(R2D_Vertex) * max_verts),
        .count = 0,
        .max_verts = max_verts
    };
}

void R2D_VertexCacheReset(R2D_VertexCache* cache) {
	cache->count = 0;
}

b8 R2D_VertexCachePush(R2D_VertexCache* cache, R2D_Vertex* vertices, u32 vertex_count) {
	if (cache->max_verts < cache->count + vertex_count)
        return false;
    memcpy(cache->vertices + cache->count, vertices, sizeof(R2D_Vertex) * vertex_count);
    cache->count += vertex_count;
    return true;
}

//~ Renderer Core

void R2D_Init(OS_Window* window, R2D_Renderer* renderer) {
	renderer->arena = arena_make();
	
	renderer->current_batch = 0;
	renderer->cull_quad = (rect) { 0, 0, window->width, window->height };
    renderer->offset = (vec2) { 0.f, 0.f };
	darray_add(R2D_Batch, &renderer->batches, (R2D_Batch) {0});
	renderer->batches.elems[renderer->current_batch].cache = R2D_VertexCacheCreate(renderer->arena, R2D_MAX_INTERNAL_CACHE_VCOUNT);
	
	R_ShaderPackAllocLoad(&renderer->shader, str_lit("res/render_2d"));
	R_Attribute attributes[] = { Attribute_Float2, Attribute_Float2, Attribute_Float1, Attribute_Float4 };
	R_PipelineAlloc(&renderer->pipeline, InputAssembly_Triangles, attributes, ArrayCount(attributes), &renderer->shader, BlendMode_Alpha);
	R_BufferAlloc(&renderer->buffer, BufferFlag_Dynamic | BufferFlag_Type_Vertex);
	R_BufferData(&renderer->buffer, R2D_MAX_INTERNAL_CACHE_VCOUNT * sizeof(R2D_Vertex), nullptr);
	R_PipelineAddBuffer(&renderer->pipeline, &renderer->buffer, ArrayCount(attributes));
	
	R_PipelineBind(&renderer->pipeline);
	i32 textures[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	R_ShaderPackUploadIntArray(&renderer->shader, str_lit("u_tex"), textures, 8);
	mat4 projection = mat4_transpose(mat4_ortho(0, window->width, 0, window->height, -1, 1000));
	R_ShaderPackUploadMat4(&renderer->shader, str_lit("u_projection"), projection);
	
	R_Texture2DWhite(&renderer->white_texture);
	R_Texture2DAllocLoad(&renderer->circle_texture, str_lit("res/circle.png"), TextureResize_Linear, TextureResize_Linear, TextureWrap_ClampToEdge, TextureWrap_ClampToEdge);
}

void R2D_Free(R2D_Renderer* renderer) {
	R_Texture2DFree(&renderer->white_texture);
	R_BufferFree(&renderer->buffer);
	R_PipelineFree(&renderer->pipeline);
	R_ShaderPackFree(&renderer->shader);
	arena_free(renderer->arena);
}

void R2D_ResizeProjection(R2D_Renderer* renderer, vec2 render_size) {
	R_PipelineBind(&renderer->pipeline);
	mat4 projection = mat4_transpose(mat4_ortho(0, render_size.x, 0, render_size.y, -1, 1000));
	R_ShaderPackUploadMat4(&renderer->shader, str_lit("u_projection"), projection);
}

void R2D_BeginDraw(R2D_Renderer* renderer) {
	Iterate(renderer->batches, i) {
		R2D_VertexCacheReset(&renderer->batches.elems[i].cache);
		renderer->batches.elems[i].tex_count = 0;
	}
	renderer->current_batch = 0;
}

void R2D_EndDraw(R2D_Renderer* renderer) {
	R_PipelineBind(&renderer->pipeline);
	for (u32 i = 0; i < renderer->current_batch+1; i++) {
		for (u32 t = 0; t < renderer->batches.elems[i].tex_count; t++) {
			R_Texture2DBindTo(renderer->batches.elems[i].textures[t], t);
		}
		R2D_VertexCache* cache = &renderer->batches.elems[i].cache;
		R_BufferUpdate(&renderer->buffer, 0, cache->count * sizeof(R2D_Vertex), (void*) cache->vertices);
		R_Draw(&renderer->pipeline, 0, cache->count);
	}
}

rect D_PushCullRect(R2D_Renderer* renderer, rect new_quad) {
	rect ret = renderer->cull_quad;
	renderer->cull_quad = new_quad;
	return ret;
}

void D_PopCullRect(R2D_Renderer* renderer, rect old_quad) {
	renderer->cull_quad = old_quad;
}

vec2 D_PushOffset(R2D_Renderer* renderer, vec2 new_offset) {
	vec2 ret = renderer->offset;
	renderer->offset = new_offset;
	return ret;
}

void D_PopOffset(R2D_Renderer* renderer, vec2 old_offset) {
	renderer->offset = old_offset;
}

void R2D_DrawQuad(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, rect uvs, vec4 color) {
	quad.x += renderer->offset.x;
	quad.y += renderer->offset.y;
	
	if (!rect_overlaps(quad, renderer->cull_quad)) return;
	
	R2D_Batch* batch = R2D_BatchGetCurrent(renderer, 6, texture);
	i32 idx = R2D_BatchAddTexture(renderer, batch, texture);
	rect uv_culled = rect_uv_cull(quad, uvs, renderer->cull_quad);
	
	R2D_Vertex vertices[] = {
		{
            .pos = vec2_clamp(vec2_init(quad.x, quad.y), renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x, uv_culled.y),
            .color = color,
		},
        {
            .pos = vec2_clamp(vec2_init(quad.x + quad.w, quad.y), renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x + uv_culled.w, uv_culled.y),
            .color = color,
		},
        {
            .pos = vec2_clamp(vec2_init(quad.x + quad.w, quad.y + quad.h), renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x + uv_culled.w, uv_culled.y + uv_culled.h),
            .color = color,
		},
        {
            .pos = vec2_clamp(vec2_init(quad.x, quad.y), renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x, uv_culled.y),
            .color = color,
		},
        {
            .pos = vec2_clamp(vec2_init(quad.x + quad.w, quad.y + quad.h), renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x + uv_culled.w, uv_culled.y + uv_culled.h),
            .color = color,
        },
        {
            .pos = vec2_clamp(vec2_init(quad.x, quad.y + quad.h), renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x, uv_culled.y + uv_culled.h),
            .color = color,
        },
	};
	R2D_VertexCachePush(&batch->cache, vertices, 6);
}

void R2D_DrawQuadC(R2D_Renderer* renderer, rect quad, vec4 color) {
	R2D_DrawQuad(renderer, quad, &renderer->white_texture, rect_init(0.f, 0.f, 1.f, 1.f), color);
}

void R2D_DrawQuadT(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, vec4 tint) {
	R2D_DrawQuad(renderer, quad, texture, rect_init(0.f, 0.f, 1.f, 1.f), tint);
}

void R2D_DrawQuadST(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, rect uvs, vec4 tint) {
	R2D_DrawQuad(renderer, quad, texture, uvs, tint);
}

void R2D_DrawCircle(R2D_Renderer* renderer, vec2 pos, f32 radius, vec4 color) {
	R2D_DrawQuad(renderer, (rect) { pos.x - radius, pos.y - radius, radius * 2.f, radius * 2.f }, &renderer->circle_texture, rect_init(0.f, 0.f, 1.f, 1.f), color);
}



void R2D_DrawQuadRotated(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, rect uvs,
						 vec4 color, f32 theta) {
	quad.x += renderer->offset.x;
	quad.y += renderer->offset.y;
	
	vec2 vertices[4] = {
		{ -quad.w / 2.f, -quad.h / 2.f },
		{  quad.w / 2.f, -quad.h / 2.f },
		{  quad.w / 2.f,  quad.h / 2.f },
		{ -quad.w / 2.f,  quad.h / 2.f },
	};
	for (u32 i = 0; i < 4; i++) {
		vec2 old = vertices[i];
		vertices[i].x = quad.x + quad.w / 2.f + (cosf(theta) * old.x - sinf(theta) * old.y);
		vertices[i].y = quad.y + quad.h / 2.f + (sinf(theta) * old.x + cosf(theta) * old.y);
	}
	
	if (!rect_overlaps(quad, renderer->cull_quad)) return;
	
	R2D_Batch* batch = R2D_BatchGetCurrent(renderer, 6, texture);
	i32 idx = R2D_BatchAddTexture(renderer, batch, texture);
	rect uv_culled = rect_uv_cull(quad, uvs, renderer->cull_quad);
	
	R2D_Vertex vertices_to_batch[] = {
		{
            .pos = vec2_clamp(vertices[0], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x, uv_culled.y),
            .color = color,
		},
        {
            .pos = vec2_clamp(vertices[1], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x + uv_culled.w, uv_culled.y),
            .color = color,
		},
        {
            .pos = vec2_clamp(vertices[2], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x + uv_culled.w, uv_culled.y + uv_culled.h),
            .color = color,
		},
        {
            .pos = vec2_clamp(vertices[0], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x, uv_culled.y),
            .color = color,
		},
        {
            .pos = vec2_clamp(vertices[2], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x + uv_culled.w, uv_culled.y + uv_culled.h),
            .color = color,
        },
        {
            .pos = vec2_clamp(vertices[3], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x, uv_culled.y + uv_culled.h),
            .color = color,
        },
	};
	R2D_VertexCachePush(&batch->cache, vertices_to_batch, 6);
}

void R2D_DrawQuadRotatedC(R2D_Renderer* renderer, rect quad, vec4 color, f32 theta) {
	R2D_DrawQuadRotated(renderer, quad, &renderer->white_texture, rect_init(0.f, 0.f, 1.f, 1.f), color, theta);
}

void R2D_DrawQuadRotatedT(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, vec4 tint, f32 theta) {
	R2D_DrawQuadRotated(renderer, quad, texture, rect_init(0.f, 0.f, 1.f, 1.f), tint, theta);
}

void R2D_DrawQuadRotatedST(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, rect uvs, vec4 tint, f32 theta) {
	R2D_DrawQuadRotated(renderer, quad, texture, uvs, tint, theta);
}

// NO CULLING FOR LINES

void R2D_DrawLine(R2D_Renderer* renderer, vec2 start, vec2 end, f32 thickness, R_Texture2D* texture, rect uvs, vec4 color) {
	start.x += renderer->offset.x;
	end.x += renderer->offset.x;
	start.y += renderer->offset.y;
	end.y += renderer->offset.y;
	
	vec2 line_vector = vec2_sub(end, start);
	vec2 r = { -line_vector.y,  line_vector.x };
	vec2 l = {  line_vector.y, -line_vector.x };
	r = vec2_scale(vec2_normalize(r), thickness / 2.f);
	l = vec2_scale(vec2_normalize(l), thickness / 2.f);
	
	vec2 vertices[4] = {
		vec2_add(start, l),
		vec2_add(start, r),
		vec2_add(end, r),
		vec2_add(end, l),
	};
	
	
	R2D_Batch* batch = R2D_BatchGetCurrent(renderer, 6, texture);
	i32 idx = R2D_BatchAddTexture(renderer, batch, texture);
	rect uv_culled = uvs;
	
	R2D_Vertex vertices_to_batch[] = {
		{
            .pos = vec2_clamp(vertices[0], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x, uv_culled.y),
            .color = color,
		},
        {
            .pos = vec2_clamp(vertices[1], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x + uv_culled.w, uv_culled.y),
            .color = color,
		},
        {
            .pos = vec2_clamp(vertices[2], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x + uv_culled.w, uv_culled.y + uv_culled.h),
            .color = color,
		},
        {
            .pos = vec2_clamp(vertices[0], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x, uv_culled.y),
            .color = color,
		},
        {
            .pos = vec2_clamp(vertices[2], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x + uv_culled.w, uv_culled.y + uv_culled.h),
            .color = color,
        },
        {
            .pos = vec2_clamp(vertices[3], renderer->cull_quad),
            .tex_index = idx,
            .tex_coords = vec2_init(uv_culled.x, uv_culled.y + uv_culled.h),
            .color = color,
        },
	};
	R2D_VertexCachePush(&batch->cache, vertices_to_batch, 6);
}

void R2D_DrawLineC(R2D_Renderer* renderer, vec2 start, vec2 end, f32 thickness, vec4 color) {
	R2D_DrawLine(renderer, start, end, thickness, &renderer->white_texture, rect_init(0.f, 0.f, 1.f, 1.f), color);
}
