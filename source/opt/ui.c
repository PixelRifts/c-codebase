#include "ui.h"

UI_Item g_ui_null_item = {
	.first  = &g_ui_null_item,
	.last   = &g_ui_null_item,
	.next   = &g_ui_null_item,
	.prev   = &g_ui_null_item,
	.parent = &g_ui_null_item,
};

// TODO(voxel)
//u32 g_cache_capacity;
//UI_Item* g_item_cache;
//
//static void CachePut(UI_Key key, UI_Item* item) {
//u64 index = str_hash(key.str) % g_cache_capacity;
//UI_Item* ret = &g_item_cache[index];
//if (UI_ItemIsNull(ret)) return 0;
//}
//
//static UI_Item* CacheLookup(UI_Key key) {
//u64 index = str_hash(key.str) % g_cache_capacity;
//UI_Item ret = g_item_cache[index];
//while (!str_eq(ret.key, key) || !UI_ItemIsNull(ret)) ret = ret->hash_next;
//return ret;
//}

//~ Middle Layer - Core Item Codepath

UI_Item* UI_ItemNull(void) {
	return &g_ui_null_item;
}

b8 UI_ItemIsNull(UI_Item* item) {
	return item == nullptr || item == &g_ui_null_item;
}

UI_Item* UI_ItemMake(UI_ItemFlags flags, string text) {
	return &g_ui_null_item;
}

UI_Item* UI_ItemMakeF(UI_ItemFlags flags, char* fmt, ...) {
	return &g_ui_null_item;
}

UI_Item* UI_PushParent(UI_Item* parent) {
	return &g_ui_null_item;
}

UI_Item* UI_PopParent() {
	return &g_ui_null_item;
}
