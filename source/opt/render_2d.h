/* date = July 20th 2022 9:08 pm */

#ifndef RENDER_2D_H
#define RENDER_2D_H

#include "defines.h"
#include "base/base.h"
#include "os/window.h"
#include "core/resources.h"

typedef struct R2D_Vertex {
    vec2 pos;
    vec2 tex_coords;
    f32  tex_index;
    vec4 color;
} R2D_Vertex;

#define R2D_MAX_INTERNAL_CACHE_VCOUNT 1024

typedef struct R2D_VertexCache {
    R2D_Vertex* vertices;
    u32 count;
    u32 max_verts;
} R2D_VertexCache;

R2D_VertexCache R2D_VertexCacheCreate(M_Arena* arena, u32 max_verts);
void R2D_VertexCacheReset(R2D_VertexCache* cache);
b8   R2D_VertexCachePush(R2D_VertexCache* cache, R2D_Vertex* vertices, u32 vertex_count);

typedef struct R2D_Batch {
	R2D_VertexCache cache;
    R_Texture2D *textures[8];
    u8 tex_count;
} R2D_Batch;

DArray_Prototype(R2D_Batch);

typedef struct R2D_Renderer {
	M_Arena* arena;
	
	darray(R2D_Batch) batches;
    u8 current_batch;
    rect cull_quad;
    vec2 offset;
    
	R_Texture2D white_texture;
	
	R_Pipeline pipeline;
	R_Buffer buffer;
	R_ShaderPack shader;
} R2D_Renderer;

void R2D_Init(OS_Window* window, R2D_Renderer* renderer);
void R2D_Free(R2D_Renderer* renderer);
void R2D_ResizeProjection(R2D_Renderer* renderer, vec2 render_size);

void R2D_BeginDraw(R2D_Renderer* renderer);
void R2D_EndDraw(R2D_Renderer* renderer);

rect D_PushCullRect(R2D_Renderer* renderer, rect new_quad);
void D_PopCullRect(R2D_Renderer* renderer, rect old_quad);
vec2 D_PushOffset(R2D_Renderer* renderer, vec2 new_offset);
void D_PopOffset(R2D_Renderer* renderer, vec2 old_offset);

void R2D_DrawQuad(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, rect uvs, vec4 color);
void R2D_DrawQuadC(R2D_Renderer* renderer, rect quad, vec4 color);
void R2D_DrawQuadT(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, vec4 tint);
void R2D_DrawQuadST(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, rect uvs, vec4 tint);

void R2D_DrawQuadRotated(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, rect uvs, vec4 color, f32 theta);
void R2D_DrawQuadRotatedC(R2D_Renderer* renderer, rect quad, vec4 color, f32 theta);
void R2D_DrawQuadRotatedT(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, vec4 tint, f32 theta);
void R2D_DrawQuadRotatedST(R2D_Renderer* renderer, rect quad, R_Texture2D* texture, rect uvs, vec4 tint, f32 theta);

// NO CULLING FOR LINES
void R2D_DrawLine(R2D_Renderer* renderer, vec2 start, vec2 end, f32 thickness, R_Texture2D* texture, rect uvs, vec4 color);
void R2D_DrawLineC(R2D_Renderer* renderer, vec2 start, vec2 end, f32 thickness, vec4 color);

#endif //RENDER_2D_H
