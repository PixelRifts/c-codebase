/* date = March 1st 2023 6:37 pm */

//
//  Based on https://www.rfleury.com/p/ui-series-table-of-contents
//

#ifndef UI_H
#define UI_H

#include "defines.h"
#include "base/base.h"
#include "os/window.h"
#include "core/resources.h"

#include <stb/stb_truetype.h>

//~ UI Text Caching Layer
// TODO(voxel): Switch over from temporary STBTTF to Freetype.
// TODO(voxel): Implement a Glyph Cache and Unicode stuff and things
// TODO(voxel): So for now I'll use stb-ttf

typedef struct UI_FontInfo {
	R_Texture2D font_texture;
    stbtt_packedchar cdata[95];
    f32 scale;
    f32 font_size;
    i32 ascent;
    i32 descent;
    i32 baseline;
} UI_FontInfo;

void UI_LoadFont(UI_FontInfo* fontinfo, string filepath, f32 size);
f32  UI_GetStringSize(UI_FontInfo* fontinfo, string str);
void UI_FreeFont(UI_FontInfo* fontinfo);

//~ Main UI Layer

//- UI Caching Helpers 

// not directly typedefed to u64 because it's possible more identification is required
typedef struct UI_Key {
	u64 id;
} UI_Key;

UI_Key UI_KeyNull(void);
UI_Key UI_KeyFromString(string s);
b8     UI_KeyEquals(UI_Key a, UI_Key b);

//- UI Layouting 
typedef u32 UI_SizeKind;
enum {
	SizeKind_Null,
	SizeKind_Pixels,
	SizeKind_TextContent,
	SizeKind_PercentOfParent,
	SizeKind_ChildrenSum,
};

typedef struct UI_Size {
	UI_SizeKind kind;
	f32 value;
	
	// NOTE(voxel): Supposedly required for handling violations
	//f32 strictness;
} UI_Size;

UI_Size UI_Pixels(f32 pixels);
UI_Size UI_TextContent(f32 padding);
UI_Size UI_Percentage(f32 pct);
UI_Size UI_ChildrenSum(void);

//- UI Colors 

typedef struct UI_QuadColorSet {
	u32 bl; // bottom left
	u32 br; // bottom right
	u32 tr; // top right
	u32 tl; // top left
} UI_QuadColorSet;

typedef struct UI_QuadVec4ColorSet {
	vec4 bl; // bottom left
	vec4 br; // bottom right
	vec4 tr; // top right
	vec4 tl; // top left
} UI_QuadVec4ColorSet;

inline static UI_QuadColorSet UI_QuadColorSetMake(u32 bl, u32 br, u32 tr, u32 tl) {
	return (UI_QuadColorSet) { bl, br, tr, tl };
}

inline static UI_QuadVec4ColorSet UI_ColorToVec4Set(u32 color) {
	vec4 a = color_code_to_vec4(color);
	return (UI_QuadVec4ColorSet) { a, a, a, a };
}

UI_QuadVec4ColorSet UI_ColorSetToVec4Set(UI_QuadColorSet set);
UI_QuadVec4ColorSet UI_ColorSetLerp(UI_QuadVec4ColorSet* a, UI_QuadVec4ColorSet* b, f32 t);

//- UI Box Things 

typedef struct UI_Cache UI_Cache;

typedef u32 UI_BoxFlags;
enum {
	BoxFlag_Clickable       = 0x1,    // @done
	BoxFlag_ViewScroll      = 0x2,    // TODO hard
	BoxFlag_DrawText        = 0x4,    // @done
	BoxFlag_DrawBorder      = 0x8,    // @done
	BoxFlag_DrawBackground  = 0x10,   // @done
	BoxFlag_DrawDropShadow  = 0x20,   // @done
	BoxFlag_Clip            = 0x40,   // @done
	BoxFlag_HotAnimation    = 0x80,   // @done
	BoxFlag_ActiveAnimation = 0x100,  // @done
	BoxFlag_CustomRenderer  = 0x200,  // @done
};

typedef struct UI_Box UI_Box;
typedef void UI_RenderFunction(UI_Cache* cache, UI_Box* box);

// n-ary tree node
struct UI_Box {
	// Box tree is rebuilt each frame.
	UI_Box* parent;
	// sibling list
	UI_Box* prev;
	UI_Box* next;
	// children list
	UI_Box* first;
	UI_Box* last;
	
	// caching stuff
	UI_Box* hash_next;
	UI_Box* hash_prev;
	UI_Key key;
	u64 last_frame_touched_index;
	b32 direct_set;
	
	// main things
	UI_BoxFlags flags;
	string identifier;
	
	// input things
	b32 pressed_on_this;
	
	// layouting
	UI_Size semantic_size[axis2_count];
	f32 computed_size[axis2_count];
	axis2 layout_axis;
	f32 computed_rel_position[axis2_count];
	rect target_bounds;
	rect bounds;
	rect clipped_bounds;
	
	// Properties!!
	f32 hot_t;
	u32 hot_color;
	f32 active_t;
	u32 active_color;
	b8 is_on;
	
	UI_FontInfo* font;
	u32 color;
	u32 edge_color;
	u32 text_color;
	f32 rounding;
	f32 softness;
	f32 edge_size;
	UI_RenderFunction* custom_render;
};

UI_Box* UI_BoxMake(UI_Cache* cache, UI_BoxFlags flags, string str);
UI_Box* UI_BoxMakeF(UI_Cache* cache, UI_BoxFlags flags, const char* fmt, ...);

//- Stacks and Stacks of things 
#define UI_DeferLoop(begin, end) for(int _i_ = ((begin), 0); !_i_; _i_ += 1, (end))

#include "meta/ui_stacks.h"

//~ UI Signals

typedef struct UI_Signal {
	b8 clicked;        // @done
	b8 double_clicked; // TODO
	b8 right_clicked;  // @done
	b8 pressed;        // @done
	b8 released;       // @done
	b8 dragging;       // TODO
	b8 hovering;       // @done
} UI_Signal;

UI_Signal UI_SignalFromBox(UI_Box* box);

//~ UI Rendering Layer
// TODO(voxel): This exposed API will be for custom rendering procedures

#define MAX_UI_QUADS 2048

typedef struct UI_Vertex {
	// We get box size and center instead of vertex position, since vertex pos can be calculated
	// and box size/center cannot. Compression is being done basically
    vec2 box_size;
	vec2 box_center;
	vec2 uv;
	f32  tex_idx;
	vec4 color;
	vec4 clip_quad;
	vec3 rounding_softness_and_edge_size;
} UI_Vertex;

void UI_PushQuad(UI_Cache* ui_cache, rect bounds, rect uvs, R_Texture2D* texture, UI_QuadVec4ColorSet colors, f32 rounding, f32 softness, f32 edge_size);

//~ UI Main Things 

// UI_Box* doesn't really work ;-; so let's just use u64
// Some things about these macros are not nice, I really gotta make the names not tied to type again

StableTable_Prototype(UI_Key, UI_Box);

struct UI_Cache {
	stable_table(UI_Key, UI_Box) cache;
	
	UI_STACK_DECLS;
	
	UI_FontInfo default_font;
	
	UI_Box* root;
	u64 current_frame_index;
	
	UI_Key hot_key;
	UI_Key active_key;
	
	// Rendering stuff
	R_ShaderPack shaderpack;
	R_Pipeline pipeline;
	R_Buffer gpu_side_buffer;
	UI_Vertex cpu_side_buffer[MAX_UI_QUADS * 6];
	u32 quad_count;
	R_Texture2D textures[8];
	u32 textures_count;
	
	R_Texture2D white_texture;
};

void UI_Init(OS_Window* window, UI_Cache* ui_cache);
void UI_BeginFrame(OS_Window* window, UI_Cache* cache);
void UI_EndFrame(UI_Cache* ui_cache, f32 delta_time);
void UI_Free(UI_Cache* ui_cache);

void UI_Resize(UI_Cache* ui_cache, i32 w, i32 h);
void UI_ButtonEvent(UI_Cache* ui_cache, i32 button, i32 action);


//~ UI Builder Layer

UI_Signal UI_Button(UI_Cache* ui_cache, string id);
UI_Signal UI_ButtonF(UI_Cache* ui_cache, const char* fmt, ...);
b8        UI_Checkbox(UI_Cache* ui_cache, string id);
b8        UI_CheckboxF(UI_Cache* ui_cache, const char* fmt, ...);
void      UI_Spacer(UI_Cache* ui_cache, UI_Size size);


#endif //UI_H
