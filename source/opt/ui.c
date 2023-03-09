#include "ui.h"

#include <stdarg.h>
#include <stdio.h>
#include "os/input.h"

//- Text Caching Layer 
// TODO(voxel): Switch to freetype soon:tm:

void UI_LoadFont(UI_FontInfo* fontinfo, string filepath, f32 size) {
	M_Scratch scratch = scratch_get();
	
	FILE* ttfile = fopen((char*)filepath.str, "rb");
    AssertTrue(ttfile, "Failed to find Font file %.*s", str_expand(filepath));
	fseek(ttfile, 0, SEEK_END);
	u64 length = ftell(ttfile);
	rewind(ttfile);
	u8* buffer = arena_alloc(scratch.arena, length);
	fread(buffer, length, 1, ttfile);
	fclose(ttfile);
	
	u8* temp_bitmap = arena_alloc(scratch.arena, 512 * 512);
	
	stbtt_fontinfo finfo;
	stbtt_pack_context packctx;
	stbtt_InitFont(&finfo, buffer, 0);
	stbtt_PackBegin(&packctx, temp_bitmap, 512, 512, 0, 1, 0);
	stbtt_PackSetOversampling(&packctx, 1, 1);
	stbtt_PackFontRange(&packctx, buffer, 0, size, 32, 95, fontinfo->cdata);
	stbtt_PackEnd(&packctx);
	
	R_Texture2DAlloc(&fontinfo->font_texture, TextureFormat_R, 512, 512, TextureResize_Linear, TextureResize_Linear, TextureWrap_Repeat, TextureWrap_Repeat);
	R_Texture2DData(&fontinfo->font_texture, temp_bitmap);
	i32 swizzles[4] = {
		TextureChannel_One, TextureChannel_One, TextureChannel_One, TextureChannel_R
	};
	R_Texture2DSwizzle(&fontinfo->font_texture, swizzles);
	
	fontinfo->scale = stbtt_ScaleForPixelHeight(&finfo, size);
	stbtt_GetFontVMetrics(&finfo, &fontinfo->ascent, &fontinfo->descent, nullptr);
	fontinfo->baseline = (i32) (fontinfo->ascent * fontinfo->scale);
	fontinfo->font_size = size;
	
	scratch_return(&scratch);
}

f32 UI_GetStringSize(UI_FontInfo* fontinfo, string str) {
    f32 sz = 0.f;
    for (u32 i = 0; i < str.size; i++) {
        if (str.str[i] >= 32 && str.str[i] < 128) {
            stbtt_packedchar* info = &fontinfo->cdata[str.str[i] - 32];
            sz += info->xadvance;
        }
    }
    return sz;
}

void UI_FreeFont(UI_FontInfo* fontinfo) {
	R_Texture2DFree(&fontinfo->font_texture);
}

//- UI Caching Helpers 

UI_Key UI_KeyNull(void) {
	return (UI_Key) {0};
}

UI_Key UI_KeyFromString(string s) {
	return (UI_Key) { str_hash_64(s) };
}

b8 UI_KeyEquals(UI_Key a, UI_Key b) {
	return a.id == b.id;
}

static b8 UI_KeyIsNull(UI_Key a) { return a.id == 0; }
static u64 UI_KeyHashID(UI_Key a) { return a.id; }

//- UI Box Things 

StableTable_Impl(UI_Key, UI_Box, UI_KeyIsNull, UI_KeyEquals, UI_KeyHashID);

UI_Box* UI_BoxMake(UI_Cache* ui_cache, UI_BoxFlags flags, string str) {
	if (str.size == 0) {
		// If this is an ID-less Box, allocate it on the Frame Arena
		UI_Box* to_ret = arena_alloc(U_GetFrameArena(), sizeof(UI_Box));
		
		UI_Box* parent = (UI_Box*) dstack_peek(u64, &ui_cache->parent_stack);
		to_ret->parent = parent;
		// Add this box to parent's children
		if (!parent->first) {
			parent->first = to_ret;
			to_ret->prev = nullptr;
		}
		if (parent->last) {
			parent->last->next = to_ret;
			to_ret->prev = parent->last;
		}
		parent->last = to_ret;
		
		to_ret->next  = nullptr;
		// clear children list
		to_ret->first = nullptr;
		to_ret->last  = nullptr;
		
		to_ret->semantic_size[axis2_x] = dstack_peek(UI_Size, &ui_cache->box_width_pref_stack);
		to_ret->semantic_size[axis2_y] = dstack_peek(UI_Size, &ui_cache->box_height_pref_stack);
		to_ret->layout_axis = dstack_peek(u32, &ui_cache->box_layout_axis_stack);
		to_ret->identifier = str;
		to_ret->key = (UI_Key) {0};
		
		return to_ret;
	}
	
	// Check cache and return Box from last frame if hit
	// Else add a new Box to the cache
	UI_Key key = UI_KeyFromString(str);
	
	UI_Box* to_ret = nullptr;
	if (stable_table_get(UI_Key, UI_Box, &ui_cache->cache, key, &to_ret)) {
		to_ret->last_frame_touched_index = ui_cache->current_frame_index;
		
		if (ui_cache->parent_stack.len != 0) {
			// setting up box parent based on stack state
			UI_Box* parent = (UI_Box*) dstack_peek(u64, &ui_cache->parent_stack);
			to_ret->parent = parent;
			// Add this box to parent's children
			if (!parent->first) {
				parent->first = to_ret;
				to_ret->prev = nullptr;
			}
			if (parent->last) {
				parent->last->next = to_ret;
				to_ret->prev = parent->last;
			}
			parent->last = to_ret;
		} else to_ret->prev = nullptr;
		
		to_ret->next  = nullptr;
		// clear children list
		to_ret->first = nullptr;
		to_ret->last  = nullptr;
		return to_ret;
	}
	
	UI_Box allocate_one_for_me = {0};
	stable_table_set(UI_Key, UI_Box, &ui_cache->cache, key, allocate_one_for_me);
	stable_table_get(UI_Key, UI_Box, &ui_cache->cache, key, &to_ret);
	
	to_ret->last_frame_touched_index = ui_cache->current_frame_index;
	
	if (ui_cache->parent_stack.len != 0) {
		// setting up box properties based on stack states
		UI_Box* parent = (UI_Box*) dstack_peek(u64, &ui_cache->parent_stack);
		to_ret->parent = parent;
		to_ret->flags = flags;
		to_ret->identifier = str;
		
		// Add this box to parent's children
		if (!parent->first) parent->first = to_ret;
		if (parent->last) {
			parent->last->next = to_ret;
			to_ret->prev = parent->last;
		}
		parent->last = to_ret;
		
		to_ret->font = (UI_FontInfo*) dstack_peek(u64, &ui_cache->font_stack);
		to_ret->color = dstack_peek(UI_QuadColorSet, &ui_cache->box_color_stack);
		to_ret->hot_color = dstack_peek(UI_QuadColorSet, &ui_cache->box_hot_color_stack);
		to_ret->active_color = dstack_peek(UI_QuadColorSet, &ui_cache->box_active_color_stack);
		to_ret->edge_color = dstack_peek(u32, &ui_cache->box_edge_color_stack);
		to_ret->text_color = dstack_peek(u32, &ui_cache->box_text_color_stack);
		to_ret->rounding = dstack_peek(f32, &ui_cache->box_rounding_stack);
		to_ret->softness = dstack_peek(f32, &ui_cache->box_softness_stack);
		to_ret->edge_size = dstack_peek(f32, &ui_cache->box_edge_size_stack);
		to_ret->custom_render =
		(UI_RenderFunction*) dstack_peek(u64, &ui_cache->box_custom_render_stack);
		
		to_ret->semantic_size[axis2_x] = dstack_peek(UI_Size, &ui_cache->box_width_pref_stack);
		to_ret->semantic_size[axis2_y] = dstack_peek(UI_Size, &ui_cache->box_height_pref_stack);
		to_ret->layout_axis = dstack_peek(u32, &ui_cache->box_layout_axis_stack);
	}
	
	return to_ret;
}

UI_Box* UI_BoxMakeF(UI_Cache* ui_cache, UI_BoxFlags flags, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	const char buf[8092];
	vsnprintf(buf, 8092, fmt, args);
	va_end(args);
	u64 size = strlen(buf);
	string s = str_alloc(U_GetFrameArena(), size);
	memmove(s.str, buf, size);
	return UI_BoxMake(ui_cache, flags, s);
}

//- MakeSize helpers 
UI_Size UI_Pixels(f32 pixels) {
	return (UI_Size) { SizeKind_Pixels, pixels };
}

UI_Size UI_TextContent(f32 padding) {
	return (UI_Size) { SizeKind_TextContent, padding };
}

UI_Size UI_Percentage(f32 pct) {
	return (UI_Size) { SizeKind_PercentOfParent, pct };
}

UI_Size UI_ChildrenSum(void) {
	return (UI_Size) { SizeKind_ChildrenSum };
}


//- Helpers 

UI_QuadVec4ColorSet UI_ColorSetToVec4Set(UI_QuadColorSet set) {
	return (UI_QuadVec4ColorSet) {
		color_code_to_vec4(set.bl),
		color_code_to_vec4(set.br),
		color_code_to_vec4(set.tr),
		color_code_to_vec4(set.tl),
	};
}

UI_QuadVec4ColorSet UI_ColorSetLerp(UI_QuadVec4ColorSet* a, UI_QuadVec4ColorSet* b, f32 t) {
	return (UI_QuadVec4ColorSet) {
		vec4_lerp(a->bl, b->bl, t),
		vec4_lerp(a->br, b->br, t),
		vec4_lerp(a->tr, b->tr, t),
		vec4_lerp(a->tl, b->tl, t)
	};
}

static b8 UI_RecurseCheckChildrenMouseTest(UI_Box* box) {
	if (rect_contains_point(box->bounds, v2(OS_InputGetMouseX(), OS_InputGetMouseY())))
		return true;
	UI_Box* curr = box->first;
	while (curr) {
		if (UI_RecurseCheckChildrenMouseTest(curr)) return true;
		curr = curr->next;
	}
	return false;
}

UI_Signal UI_SignalFromBox(UI_Box* box) {
	UI_Signal ret = {0};
	if (!rect_contains_point(box->clipped_bounds, v2(OS_InputGetMouseX(), OS_InputGetMouseY())))
		return ret;
	
	UI_Box* curr = box->first;
	while (curr) {
		if (UI_RecurseCheckChildrenMouseTest(curr))
			return ret;
		curr = curr->next;
	}
	
	ret.hovering = true;
	if (box->flags & BoxFlag_Clickable) {
		ret.pressed  = (b8) OS_InputButtonPressed(Input_MouseButton_Left);
		ret.released = (b8) OS_InputButtonReleased(Input_MouseButton_Left);
		ret.clicked  = ret.released && box->pressed_on_this;
		
		ret.right_clicked = (b8) OS_InputButtonReleased(Input_MouseButton_Right) && box->pressed_on_this;
		
		if (OS_InputButtonPressed(Input_MouseButton_Left) || (b8) OS_InputButtonPressed(Input_MouseButton_Right))
			box->pressed_on_this = true;
	}
	if (!(OS_InputButton(Input_MouseButton_Left) || OS_InputButton(Input_MouseButton_Right)))
		box->pressed_on_this = false;
	
	return ret;
}

//- Stack Manipulations 
Stack_Impl(rect);
Stack_Impl(u64);
Stack_Impl(u32);
Stack_Impl(f32);
Stack_Impl(UI_Size);
Stack_Impl(UI_QuadColorSet);

UI_Box* UI_PushParent(UI_Cache* ui_cache, UI_Box* parent) {
	dstack_push(u64, &ui_cache->parent_stack, (u64)parent);
	return parent;
}
UI_Box* UI_PopParent(UI_Cache* ui_cache) {
	return (UI_Box*) dstack_pop(u64, &ui_cache->parent_stack);
}


UI_FontInfo* UI_PushFont(UI_Cache* ui_cache, UI_FontInfo* font) {
	dstack_push(u64, &ui_cache->font_stack, (u64)font);
	return font;
}
UI_FontInfo* UI_PopFont(UI_Cache* ui_cache) {
	return (UI_FontInfo*) dstack_pop(u64, &ui_cache->font_stack);
}


u32 UI_PushBoxColor(UI_Cache* ui_cache, u32 color) {
	dstack_push(UI_QuadColorSet, &ui_cache->box_color_stack,
				((UI_QuadColorSet) { color, color, color, color }));
	return color;
}
UI_QuadColorSet UI_PopBoxColor(UI_Cache* ui_cache) {
	return dstack_pop(UI_QuadColorSet, &ui_cache->box_color_stack);
}

UI_QuadColorSet UI_PushBoxColorSet(UI_Cache* ui_cache, UI_QuadColorSet colors) {
	dstack_push(UI_QuadColorSet, &ui_cache->box_color_stack, colors);
	return colors;
}
UI_QuadColorSet UI_PopBoxColorSet(UI_Cache* ui_cache) {
	return dstack_pop(UI_QuadColorSet, &ui_cache->box_color_stack);
}


u32 UI_PushBoxHotColor(UI_Cache* ui_cache, u32 color) {
	dstack_push(UI_QuadColorSet, &ui_cache->box_hot_color_stack,
				((UI_QuadColorSet) { color, color, color, color }));
	return color;
}
UI_QuadColorSet UI_PopBoxHotColor(UI_Cache* ui_cache) {
	return dstack_pop(UI_QuadColorSet, &ui_cache->box_hot_color_stack);
}

UI_QuadColorSet UI_PushBoxHotColorSet(UI_Cache* ui_cache, UI_QuadColorSet colors) {
	dstack_push(UI_QuadColorSet, &ui_cache->box_hot_color_stack, colors);
	return colors;
}
UI_QuadColorSet UI_PopBoxHotColorSet(UI_Cache* ui_cache) {
	return dstack_pop(UI_QuadColorSet, &ui_cache->box_hot_color_stack);
}


u32 UI_PushBoxActiveColor(UI_Cache* ui_cache, u32 color) {
	dstack_push(UI_QuadColorSet, &ui_cache->box_active_color_stack,
				((UI_QuadColorSet) { color, color, color, color }));
	return color;
}
UI_QuadColorSet UI_PopBoxActiveColor(UI_Cache* ui_cache) {
	return dstack_pop(UI_QuadColorSet, &ui_cache->box_active_color_stack);
}

UI_QuadColorSet UI_PushBoxActiveColorSet(UI_Cache* ui_cache, UI_QuadColorSet colors) {
	dstack_push(UI_QuadColorSet, &ui_cache->box_active_color_stack, colors);
	return colors;
}
UI_QuadColorSet UI_PopBoxActiveColorSet(UI_Cache* ui_cache) {
	return dstack_pop(UI_QuadColorSet, &ui_cache->box_active_color_stack);
}


u32 UI_PushBoxEdgeColor(UI_Cache* ui_cache, u32 edge_color) {
	dstack_push(u32, &ui_cache->box_edge_color_stack, edge_color);
	return edge_color;
}
u32 UI_PopBoxEdgeColor(UI_Cache* ui_cache) {
	return dstack_pop(u32, &ui_cache->box_edge_color_stack);
}

u32 UI_PushBoxTextColor(UI_Cache* ui_cache, u32 text_color) {
	dstack_push(u32, &ui_cache->box_text_color_stack, text_color);
	return text_color;
}
u32 UI_PopBoxTextColor(UI_Cache* ui_cache) {
	return dstack_pop(u32, &ui_cache->box_text_color_stack);
}


f32 UI_PushBoxRounding(UI_Cache* ui_cache, f32 rounding) {
	dstack_push(f32, &ui_cache->box_rounding_stack, rounding);
	return rounding;
}
f32 UI_PopBoxRounding(UI_Cache* ui_cache) {
	return dstack_pop(f32, &ui_cache->box_rounding_stack);
}


f32 UI_PushBoxEdgeSoftness(UI_Cache* ui_cache, f32 softness) {
	dstack_push(f32, &ui_cache->box_softness_stack, softness);
	return softness;
}
f32 UI_PopBoxEdgeSoftness(UI_Cache* ui_cache) {
	return dstack_pop(f32, &ui_cache->box_softness_stack);
}

f32 UI_PushBoxEdgeSize(UI_Cache* ui_cache, f32 size) {
	dstack_push(f32, &ui_cache->box_edge_size_stack, size);
	return size;
}
f32 UI_PopBoxEdgeSize(UI_Cache* ui_cache) {
	return dstack_pop(f32, &ui_cache->box_edge_size_stack);
}


UI_Size UI_PushBoxPrefWidth(UI_Cache* ui_cache, UI_Size width) {
	dstack_push(UI_Size, &ui_cache->box_width_pref_stack, width);
	return width;
}
UI_Size UI_PopBoxPrefWidth(UI_Cache* ui_cache) {
	return dstack_pop(UI_Size, &ui_cache->box_width_pref_stack);
}

UI_Size UI_PushBoxPrefHeight(UI_Cache* ui_cache, UI_Size height) {
	dstack_push(UI_Size, &ui_cache->box_height_pref_stack, height);
	return height;
}
UI_Size UI_PopBoxPrefHeight(UI_Cache* ui_cache) {
	return dstack_pop(UI_Size, &ui_cache->box_height_pref_stack);
}


u32 UI_PushBoxLayoutAxis(UI_Cache* ui_cache, u32 layout_axis) {
	dstack_push(u32, &ui_cache->box_layout_axis_stack, layout_axis);
	return layout_axis;
}
u32 UI_PopBoxLayoutAxis(UI_Cache* ui_cache) {
	return dstack_pop(u32, &ui_cache->box_layout_axis_stack);
}

UI_RenderFunction* UI_PushBoxRenderFunction(UI_Cache* ui_cache, UI_RenderFunction* fn) {
	dstack_push(u64, &ui_cache->box_custom_render_stack, (u64)fn);
	return fn;
}
UI_RenderFunction* UI_PopBoxRenderFunction(UI_Cache* ui_cache) {
	return (UI_RenderFunction*) dstack_pop(u64, &ui_cache->box_custom_render_stack);
}

//~ UI Rendering Layer

static void UI_InitializeRenderer(OS_Window* window, UI_Cache* ui_cache) {
	R_Attribute attributes[] = {
		Attribute_Float2,
		Attribute_Float2,
		Attribute_Float2,
		Attribute_Float1,
		Attribute_Float4,
		Attribute_Float4,
		Attribute_Float3,
	};
	R_ShaderPackAllocLoad(&ui_cache->shaderpack, str_lit("res/ui"));
	R_PipelineAlloc(&ui_cache->pipeline, InputAssembly_Triangles, attributes, ArrayCount(attributes),
					&ui_cache->shaderpack, BlendMode_Alpha);
	
	R_BufferAlloc(&ui_cache->gpu_side_buffer, BufferFlag_Dynamic | BufferFlag_Type_Vertex);
	R_BufferData(&ui_cache->gpu_side_buffer, MAX_UI_QUADS * 6 * sizeof(UI_Vertex), nullptr);
	R_PipelineAddBuffer(&ui_cache->pipeline, &ui_cache->gpu_side_buffer, ArrayCount(attributes));
	
	R_PipelineBind(&ui_cache->pipeline);
	i32 textures[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	R_ShaderPackUploadIntArray(&ui_cache->shaderpack, str_lit("u_tex"), textures, 8);
	mat4 projection = mat4_transpose(mat4_ortho(0, window->width, 0, window->height, -1, 1000));
	R_ShaderPackUploadMat4(&ui_cache->shaderpack, str_lit("u_projection"), projection);
	
	R_Texture2DWhite(&ui_cache->white_texture);
}

static void UI_FreeRenderer(UI_Cache* ui_cache) {
	R_BufferFree(&ui_cache->gpu_side_buffer);
	R_ShaderPackFree(&ui_cache->shaderpack);
	R_PipelineFree(&ui_cache->pipeline);
}

static void UI_BeginRendererFrame(UI_Cache* ui_cache) {
	ui_cache->quad_count = 0;
	ui_cache->textures_count = 0;
}

static void UI_EndRendererFrame(UI_Cache* ui_cache) {
	for (u32 i = 0; i < ui_cache->textures_count; i++) {
		R_Texture2DBindTo(&ui_cache->textures[i], i);
	}
	R_PipelineBind(&ui_cache->pipeline);
	R_BufferUpdate(&ui_cache->gpu_side_buffer, 0, ui_cache->quad_count * 6 * sizeof(UI_Vertex), ui_cache->cpu_side_buffer); 
	R_Draw(&ui_cache->pipeline, 0, ui_cache->quad_count * 6);
}

static i32 UI_GetTextureIndex(UI_Cache* ui_cache, R_Texture2D* texture) {
	for (i32 i = 0; i < ui_cache->textures_count; i++) {
		if (R_Texture2DEquals(&ui_cache->textures[i], texture))
			return i;
	}
	ui_cache->textures[ui_cache->textures_count] = *texture;
	return ui_cache->textures_count++;
}

void UI_PushQuad(UI_Cache* ui_cache, rect bounds, rect uvs, R_Texture2D* texture, UI_QuadVec4ColorSet colors, f32 rounding, f32 softness, f32 edge_size) {
	if (ui_cache->quad_count == MAX_UI_QUADS || ui_cache->textures_count == 8) {
		UI_EndRendererFrame(ui_cache);
		UI_BeginRendererFrame(ui_cache);
	}
	
	rect clipping_quad = dstack_peek(rect, &ui_cache->clipping_stack);
	i32 tex_idx = UI_GetTextureIndex(ui_cache, texture);
	vec2 size = (vec2) { bounds.w, bounds.h };
	vec2 center = (vec2) { bounds.x + bounds.w/2.f, bounds.y + bounds.h/2.f };
	vec2 uv_vertices[] = {
		v2(uvs.x, uvs.y),
		v2(uvs.x + uvs.w, uvs.y),
		v2(uvs.x + uvs.w, uvs.y + uvs.h),
		v2(uvs.x, uvs.y),
		v2(uvs.x + uvs.w, uvs.y + uvs.h),
		v2(uvs.x, uvs.y + uvs.h),
	};
	vec4 vertex_colors[] = {
		colors.tl,
		colors.tr,
		colors.br,
		colors.tl,
		colors.br,
		colors.bl,
	};
	for (u32 i = 0; i < 6; i++) {
		ui_cache->cpu_side_buffer[ui_cache->quad_count * 6 + i] = (UI_Vertex) {
			size, center, uv_vertices[i], tex_idx, vertex_colors[i], *((vec4*)&clipping_quad),
			v3(rounding, softness, edge_size)
		};
	}
	
	ui_cache->quad_count++;
}

#define UI_Vec4ColorSetUniform(color) ((UI_QuadVec4ColorSet) { color, color, color, color })

static void UI_PushBox(UI_Cache* ui_cache, UI_Box* box) {
	if (box->flags & BoxFlag_CustomRenderer) {
		box->custom_render(ui_cache, box);
		return;
	}
	
	if (box->flags & BoxFlag_DrawDropShadow) {
		UI_PushQuad(ui_cache, (rect) {box->bounds.x + 5, box->bounds.y + 5, box->bounds.w, box->bounds.h}, rect_init(0, 0, 1, 1),
					&ui_cache->white_texture, UI_Vec4ColorSetUniform(v4(0.05, 0.05, 0.05, 1.0)),
					box->rounding, 5,
					0.f);
	}
	
	if (box->flags & BoxFlag_DrawBackground) {
		UI_QuadVec4ColorSet color = UI_ColorSetToVec4Set(box->color);
		if (box->flags & BoxFlag_HotAnimation) {
			UI_QuadVec4ColorSet hot_color = UI_ColorSetToVec4Set(box->hot_color);
			color = UI_ColorSetLerp(&color, &hot_color, box->hot_t);
		}
		if (box->flags & BoxFlag_ActiveAnimation) {
			UI_QuadVec4ColorSet active_color = UI_ColorSetToVec4Set(box->active_color);
			color = UI_ColorSetLerp(&color, &active_color, box->active_t);
		}
		
		UI_PushQuad(ui_cache, box->bounds, rect_init(0, 0, 1, 1),
					&ui_cache->white_texture, color, box->rounding, box->softness,
					0.f);
	}
	
	if (box->flags & BoxFlag_DrawText) {
		f32 string_size = UI_GetStringSize(box->font, box->identifier);
		vec2 pos = v2(box->bounds.x + (box->bounds.w / 2.f), box->bounds.y + (box->bounds.h / 2.f));
		pos.x -= string_size / 2.f;
		pos.y += box->font->baseline / 2.f;
		vec4 vcolor = color_code_to_vec4(box->text_color);
		
		for (u32 i = 0; i < box->identifier.size; i++) {
			if (box->identifier.str[i] >= 32 && box->identifier.str[i] < 128) {
				stbtt_packedchar* info = &box->font->cdata[box->identifier.str[i] - 32];
				rect uvs = {
					info->x0 / 512.f,
					info->y0 / 512.f,
					(info->x1 - info->x0) / 512.f,
					(info->y1 - info->y0) / 512.f
				};
				rect loc = { pos.x + info->xoff, pos.y + info->yoff, info->x1 - info->x0, info->y1 - info->y0 };
				UI_PushQuad(ui_cache, loc, uvs, &box->font->font_texture,
							UI_Vec4ColorSetUniform(vcolor), 0.f, 0.f, 0.f);
				pos.x += info->xadvance;
			}
		}
	}
	
	if (box->flags & BoxFlag_DrawBorder) {
		vec4 bcolor = color_code_to_vec4(box->edge_color);
		UI_QuadVec4ColorSet bcolor_set = { bcolor, bcolor, bcolor, bcolor };
		UI_PushQuad(ui_cache, box->bounds, rect_init(0, 0, 1, 1),
					&ui_cache->white_texture, bcolor_set, box->rounding,
					box->softness, box->edge_size);
	}
}


//- Layouting Helpers TODO(voxel): @rework replace recursives with queue/stack 

static void UI_LayoutRecurseForward(UI_Cache* ui_cache, UI_Box* box, u32 axis) {
	f32 edge_correction_factor = box->parent ?
		box->parent->edge_size + (box->parent->edge_size)*0.25 : 0;
	
	if (box->semantic_size[axis].kind == SizeKind_Pixels) {
		box->computed_size[axis] = box->semantic_size[axis].value;
	} else if (box->semantic_size[axis].kind == SizeKind_TextContent) {
		if (axis == axis2_x) {
			box->computed_size[axis] = UI_GetStringSize(box->font, box->identifier) + box->semantic_size[axis].value * 2;
		} else {
			box->computed_size[axis] = box->font->font_size + box->semantic_size[axis].value * 2;
		}
	} else if (box->semantic_size[axis].kind == SizeKind_PercentOfParent) {
		box->computed_size[axis] = (box->parent->computed_size[axis] - edge_correction_factor*2) * 
			box->semantic_size[axis].value / 100.f;
	}
	
	UI_Box* curr = box->first;
	while (curr) {
		UI_LayoutRecurseForward(ui_cache, curr, axis);
		curr = curr->next;
	}
}

static b8 UI_StateRecurseCheckHotAndActive(UI_Cache* ui_cache, UI_Box* box) {
	UI_Box* curr = box->first;
	while (curr) {
		if (UI_StateRecurseCheckHotAndActive(ui_cache, curr)) return true;
		curr = curr->next;
	}
	
	if (!box->parent || UI_KeyIsNull(box->key)) return false;
	if (rect_contains_point(box->clipped_bounds, v2(OS_InputGetMouseX(), OS_InputGetMouseY()))) {
		if (OS_InputButton(Input_MouseButton_Left)) {
			ui_cache->hot_key = (UI_Key) {0};
			ui_cache->active_key = box->key;
		} else {
			ui_cache->hot_key = box->key;
			ui_cache->active_key = (UI_Key) {0};
		}
		return true;
	}
	return false;
}

static void UI_LayoutRecurseBackward(UI_Cache* ui_cache, UI_Box* box, u32 axis) {
	UI_Box* curr = box->first;
	f32 size = 0.f;
	while (curr) {
		UI_LayoutRecurseBackward(ui_cache, curr, axis);
		size += curr->computed_size[axis];
		
		curr = curr->next;
	}
	if (box->semantic_size[axis].kind == SizeKind_ChildrenSum) {
		box->computed_size[axis] = size;
	}
}

static void UI_LayoutRecursePositionForward(UI_Cache* ui_cache, UI_Box* box, f32 depth) {
	f32 edge_correction_factor = box->parent ? (box->parent->edge_size)*0.25 : 0;
	f32 child_depth = box->edge_size + edge_correction_factor;
	UI_Box* curr = box->first;
	while (curr) {
		UI_LayoutRecursePositionForward(ui_cache, curr, child_depth);
		child_depth += curr->computed_size[box->layout_axis];
		curr = curr->next;
	}
	
	if (box->parent) {
		box->computed_rel_position[box->parent->layout_axis] = depth;
		box->computed_rel_position[!box->parent->layout_axis] = box->parent->edge_size + edge_correction_factor;
	}
}

static void UI_LayoutRecurseCalculateBounds(UI_Cache* ui_cache, UI_Box* box, f32 xoff, f32 yoff) {
	xoff += box->computed_rel_position[axis2_x];
	yoff += box->computed_rel_position[axis2_y];
	
	box->bounds.x = xoff;
	box->bounds.y = yoff;
	box->bounds.w = box->computed_size[axis2_x];
	box->bounds.h = box->computed_size[axis2_y];
	
	rect clippable_bounds = box->bounds;
	if (box->flags & BoxFlag_DrawBorder) {
		f32 edge_correction_factor = box->parent ? (box->parent->edge_size)*0.25 : 0;
		clippable_bounds.x += box->edge_size + edge_correction_factor;
		clippable_bounds.y += box->edge_size + edge_correction_factor;
		clippable_bounds.w -= box->edge_size * 2 + edge_correction_factor;
		clippable_bounds.h -= box->edge_size * 2 + edge_correction_factor;
	}
	
	rect clipping_quad = dstack_peek(rect, &ui_cache->clipping_stack);
	box->clipped_bounds = rect_get_overlap(clippable_bounds, clipping_quad);
	
	if (box->flags & BoxFlag_Clip) {
		dstack_push(rect, &ui_cache->clipping_stack, box->clipped_bounds);
	}
	
	UI_Box* curr = box->first;
	while (curr) {
		UI_LayoutRecurseCalculateBounds(ui_cache, curr, xoff, yoff);
		curr = curr->next;
	}
	
	if (box->flags & BoxFlag_Clip) {
		dstack_pop(rect, &ui_cache->clipping_stack);
	}
}

// Required?
//static void UI_LayoutRecurseSolveViolations(UI_Cache* ui_cache, UI_Box* box, u32 axis) {}

//~ UI Main Things 

void UI_Init(OS_Window* window, UI_Cache* ui_cache) {
	MemoryZeroStruct(ui_cache, UI_Cache);
	stable_table_init(UI_Key, UI_Box, &ui_cache->cache, 64);
	UI_LoadFont(&ui_cache->default_font, str_lit("res/Inconsolata.ttf"), 24);
	dstack_push(rect, &ui_cache->clipping_stack, rect_init(0, 0, window->width, window->height));
	UI_InitializeRenderer(window, ui_cache);
}

void UI_Free(UI_Cache* ui_cache) {
	UI_FreeFont(&ui_cache->default_font);
	stable_table_free(UI_Key, UI_Box, &ui_cache->cache);
	UI_FreeRenderer(ui_cache);
}

void UI_Resize(UI_Cache* ui_cache, i32 w, i32 h) {
	R_PipelineBind(&ui_cache->pipeline);
	
	ui_cache->root->computed_size[0] = w;
	ui_cache->root->computed_size[1] = h;
	ui_cache->clipping_stack.elems[0].w = w;
	ui_cache->clipping_stack.elems[0].h = h;
	
	mat4 projection = mat4_transpose(mat4_ortho(0, w, 0, h, -1, 1000));
	R_ShaderPackUploadMat4(&ui_cache->shaderpack, str_lit("u_projection"), projection);
}

void UI_BeginFrame(OS_Window* window, UI_Cache* ui_cache) {
	// NOTE(voxel): EVICTION PASS
	
	Iterate (ui_cache->cache, i) {
		UI_Box* curr = &ui_cache->cache.elems[i];
		while (curr) {
			if (UI_KeyIsNull(curr->key) && curr->hash_next == nullptr) break;
			if (curr->last_frame_touched_index < ui_cache->current_frame_index) {
				UI_Key todel = curr->key;
				curr = curr->hash_next;
				stable_table_del(UI_Key, UI_Box, &ui_cache->cache, todel);
				continue;
			}
			
			curr = curr->hash_next;
		}
	}
	ui_cache->current_frame_index++;
	
	// NOTE(voxel): Reset all the stacks and push default values
	dstack_clear(u64, &ui_cache->parent_stack);
	dstack_clear(u64, &ui_cache->font_stack);
	dstack_clear(UI_QuadColorSet, &ui_cache->box_color_stack);
	dstack_clear(UI_QuadColorSet, &ui_cache->box_hot_color_stack);
	dstack_clear(UI_QuadColorSet, &ui_cache->box_active_color_stack);
	dstack_clear(u32, &ui_cache->box_edge_color_stack);
	dstack_clear(u32, &ui_cache->box_text_color_stack);
	dstack_clear(f32, &ui_cache->box_rounding_stack);
	dstack_clear(f32, &ui_cache->box_softness_stack);
	dstack_clear(f32, &ui_cache->box_edge_size_stack);
	dstack_clear(UI_Size, &ui_cache->box_width_pref_stack);
	dstack_clear(UI_Size, &ui_cache->box_height_pref_stack);
	dstack_clear(u32, &ui_cache->box_layout_axis_stack);
	dstack_clear(u64, &ui_cache->box_custom_render_stack);
	ui_cache->clipping_stack.len = 1;
	
	// NOTE(voxel): Default stack states
	UI_Box* container = UI_BoxMake(ui_cache, BoxFlag_Clip, str_lit("__MainContainer"));
	container->computed_size[0] = window->width;
	container->computed_size[1] = window->height;
	container->layout_axis = axis2_y;
	container->bounds = (rect) { 0.f, 0.f, window->width, window->height };
	UI_PushParent(ui_cache, container);
	
	UI_PushFont(ui_cache, &ui_cache->default_font);
	UI_PushBoxColor(ui_cache, 0x111111FF);
	UI_PushBoxHotColorSet(ui_cache,
						  UI_QuadColorSetMake(0x111111FF, 0x111111FF, 0x232323FF, 0x232323FF));
	UI_PushBoxActiveColorSet(ui_cache,
							 UI_QuadColorSetMake(0x232323FF, 0x232323FF, 0x111111FF, 0x111111FF));
	UI_PushBoxEdgeColor(ui_cache, 0x9A5EBDFF);
	UI_PushBoxTextColor(ui_cache, 0xFFAAFFFF);
	UI_PushBoxRounding(ui_cache, 5.f);
	UI_PushBoxEdgeSoftness(ui_cache, 2.f);
	UI_PushBoxEdgeSize(ui_cache, 2.f);
	UI_PushBoxPrefWidth(ui_cache, UI_Pixels(100));
	UI_PushBoxPrefHeight(ui_cache, UI_Pixels(100));
	UI_PushBoxLayoutAxis(ui_cache, axis2_y);
	UI_PushBoxRenderFunction(ui_cache, nullptr);
	
	ui_cache->root = container;
}

static void UI_PushBoxRecursive(UI_Cache* ui_cache, UI_Box* box) {
	UI_PushBox(ui_cache, box);
	
	if (box->flags & BoxFlag_Clip) {
		dstack_push(rect, &ui_cache->clipping_stack, box->clipped_bounds);
	}
	
	UI_Box* curr = box->first;
	while (curr) {
		UI_PushBoxRecursive(ui_cache, curr);
		curr = curr->next;
	}
	
	if (box->flags & BoxFlag_Clip) {
		dstack_pop(rect, &ui_cache->clipping_stack);
	}
}

void UI_EndFrame(UI_Cache* ui_cache, f32 delta_time) {
	// TODO(voxel): @rework Use a queue and stack to bypass recusive functions
	//              I have yet to implement a queue in ds.h so I'll hold off on that for now
	for (u32 i = axis2_x; i < axis2_count; i++) {
		UI_LayoutRecurseForward(ui_cache, ui_cache->root, i);
		UI_LayoutRecurseBackward(ui_cache, ui_cache->root, i);
		// NOTE(voxel): What the hell is this supposed to do even. I'll implement this
		//              if I find things to be weird
		//UI_LayoutRecurseSolveViolations(ui_cache, ui_cache->root, i);
	}
	UI_LayoutRecursePositionForward(ui_cache, ui_cache->root, 0.f);
	UI_LayoutRecurseCalculateBounds(ui_cache, ui_cache->root, 0.f, 0.f);
	
	ui_cache->hot_key = (UI_Key) {0};
	ui_cache->active_key = (UI_Key) {0};
	UI_StateRecurseCheckHotAndActive(ui_cache, ui_cache->root);
	
	// UI_Animate inlined
	f32 fast_rate = 1 - pow(2.f, -50.f * delta_time);
	f32 slow_rate = 1 - pow(2.f, -20.f * delta_time);
	Iterate (ui_cache->cache, i) {
		UI_Box* curr = &ui_cache->cache.elems[i];
		while (curr) {
			if (UI_KeyIsNull(curr->key) && curr->hash_next == nullptr) break;
			b8 is_hot        = UI_KeyEquals(ui_cache->hot_key, curr->key);
			b8 is_active     = UI_KeyEquals(ui_cache->active_key, curr->key);
			curr->hot_t     += ((f32)!!is_hot - curr->hot_t) * fast_rate;
			curr->active_t  += ((f32)!!is_active - curr->active_t) * fast_rate;
			
			curr = curr->hash_next;
		}
	}
	
	// NOTE(voxel): RENDERING PASS
	UI_BeginRendererFrame(ui_cache);
	UI_PushBoxRecursive(ui_cache, ui_cache->root);
	UI_EndRendererFrame(ui_cache);
}

//~ UI Builder

static void UI_ButtonRenderFunction(UI_Cache* ui_cache, UI_Box* box) {
	UI_PushQuad(ui_cache, (rect) {box->bounds.x + 5, box->bounds.y + 5, box->bounds.w, box->bounds.h}, rect_init(0, 0, 1, 1),
				&ui_cache->white_texture, UI_Vec4ColorSetUniform(v4(0.05, 0.05, 0.05, 1.0)),
				box->rounding, 5,
				0.f);
	
	UI_QuadVec4ColorSet color = UI_ColorSetToVec4Set(box->color);
	UI_QuadVec4ColorSet hot_color = UI_ColorSetToVec4Set(box->hot_color);
	color = UI_ColorSetLerp(&color, &hot_color, box->hot_t);
	UI_QuadVec4ColorSet active_color = UI_ColorSetToVec4Set(box->active_color);
	color = UI_ColorSetLerp(&color, &active_color, box->active_t);
	
	UI_PushQuad(ui_cache, box->bounds, rect_init(0, 0, 1, 1),
				&ui_cache->white_texture, color, box->rounding, box->softness,
				0.f);
	
	f32 string_size = UI_GetStringSize(box->font, box->identifier);
	vec2 pos = v2(box->bounds.x + (box->bounds.w / 2.f), box->bounds.y + (box->bounds.h / 2.f));
	pos.x -= string_size / 2.f;
	pos.y += box->hot_t ? box->font->baseline / 2.f - 2.f : box->font->baseline / 2.f;
	vec4 vcolor = color_code_to_vec4(box->text_color);
	
	for (u32 i = 0; i < box->identifier.size; i++) {
		if (box->identifier.str[i] >= 32 && box->identifier.str[i] < 128) {
			stbtt_packedchar* info = &box->font->cdata[box->identifier.str[i] - 32];
			rect uvs = {
				info->x0 / 512.f,
				info->y0 / 512.f,
				(info->x1 - info->x0) / 512.f,
				(info->y1 - info->y0) / 512.f
			};
			rect loc = { pos.x + info->xoff, pos.y + info->yoff, info->x1 - info->x0, info->y1 - info->y0 };
			UI_PushQuad(ui_cache, loc, uvs, &box->font->font_texture,
						UI_Vec4ColorSetUniform(vcolor), 0.f, 0.f, 0.f);
			pos.x += info->xadvance;
		}
	}
	
	vec4 bcolor = color_code_to_vec4(box->edge_color);
	UI_QuadVec4ColorSet bcolor_set = { bcolor, bcolor, bcolor, bcolor };
	UI_PushQuad(ui_cache, box->bounds, rect_init(0, 0, 1, 1),
				&ui_cache->white_texture, bcolor_set, box->rounding,
				box->softness, box->edge_size);
}

UI_Signal UI_Button(UI_Cache* ui_cache, string id) {
	UI_PushBoxRenderFunction(ui_cache, UI_ButtonRenderFunction);
	UI_Box* the_box =
		UI_BoxMake(ui_cache, BoxFlag_DrawBackground | BoxFlag_DrawBorder | BoxFlag_HotAnimation |
				   BoxFlag_ActiveAnimation | BoxFlag_DrawDropShadow | BoxFlag_Clickable |
				   BoxFlag_DrawText | BoxFlag_CustomRenderer, id);
	UI_PopBoxRenderFunction(ui_cache);
	return UI_SignalFromBox(the_box);
}

void UI_Spacer(UI_Cache* ui_cache, UI_Size size) {
	UI_Box* parent = (UI_Box*) dstack_peek(u64, &ui_cache->parent_stack);
	if (parent->layout_axis == axis2_x)
		UI_PushBoxPrefWidth(ui_cache, size);
	else
		UI_PushBoxPrefHeight(ui_cache, size);
	
	UI_BoxMake(ui_cache, 0, str_lit(""));
	
	if (parent->layout_axis == axis2_x)
		UI_PopBoxPrefWidth(ui_cache);
	else
		UI_PopBoxPrefHeight(ui_cache);
}
