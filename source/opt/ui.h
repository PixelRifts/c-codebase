/* date = August 6th 2022 9:38 am */

#ifndef UI_H
#define UI_H

#include "defines.h"
#include "base/base.h"

//~ Bottommost Layer - Renderer Interface
// TODO(voxel): This is going to be tedious

//~ Middle Layer - Core Item Codepath

typedef u32 UI_SizeType;
enum {
	UI_SizeType_Null,
	UI_SizeType_Pixels,
	UI_SizeType_TextContent,
	UI_SizeType_PercentOfParent,
	UI_SizeType_ChildrenSum,
	UI_SizeType_Count,
};

typedef struct UI_Size {
	UI_SizeType type;
	f32 value;
	f32 strictness;
} UI_Size;

typedef u32 UI_ItemFlags;
enum {
	UI_ItemFlag_Clickable       = 0x1,
	UI_ItemFlag_DrawText        = 0x2,
	UI_ItemFlag_DrawBorder      = 0x4,
	UI_ItemFlag_Clip            = 0x8,
	UI_ItemFlag_DrawBackground  = 0x10,
	UI_ItemFlag_HotAnimation    = 0x20, // Hover
	UI_ItemFlag_ActiveAnimation = 0x40, // Click/Drag/Whatever
};
typedef string UI_Key;

typedef struct UI_Item UI_Item;
struct UI_Item {
	UI_Item* first;  // First child
	UI_Item* last;   // Last child
	UI_Item* next;   // Next sibling
	UI_Item* prev;   // Prev sibling
	UI_Item* parent; // Parent
	
	UI_Item* hash_prev;
	UI_Item* hash_next;
	
	UI_Key key;
	i32 last_frame_touched_index;
	
	UI_ItemFlags flags;
	string text;
	
	UI_Size semantic_size[axis2_count];
	
	f32 computed_rel_position[axis2_count];
	f32 computed_size[axis2_count];
	rect quad;
	
	f32 hot_transition;
	f32 active_transition;
};

UI_Item* UI_ItemNull(void);
b8       UI_ItemIsNull(UI_Item* item);

UI_Item* UI_ItemMake(UI_ItemFlags flags, string text);
UI_Item* UI_ItemMakeF(UI_ItemFlags flags, char* fmt, ...);

UI_Item* UI_PushParent(UI_Item* parent);
UI_Item* UI_PopParent();

//~ Top Layer - Layouting

#endif //UI_H
