#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

#include "opt/ui.h"
//#include <freetype/freetype.h>

void MyResizeCallback(OS_Window* window, i32 w, i32 h) {
	// TODO(voxel): @awkward Add a "first resize" to Win32Window so that This if isn't required
	if (window->user_data) {
		R_Viewport(0, 0, w, h);
		UI_Resize((UI_Cache*) window->user_data, w, h);
	}
}

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	M_Arena global_arena;
	arena_init(&global_arena);
	
	U_FrameArenaInit();
	
	OS_Window* window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	window->resize_callback = MyResizeCallback;
	
	B_BackendInit(window);
	OS_WindowShow(window);
	
	UI_Cache* ui_cache = arena_alloc(&global_arena, sizeof(UI_Cache));
	UI_Init(window, ui_cache);
	window->user_data = ui_cache;
	
	f32 start = 0.f; f32 end = 0.016f;
	f32 delta = 0.016f;
	b8 show_btn = false;
	
	while (OS_WindowIsOpen(window)) {
		delta = end - start;
		start = OS_TimeMicrosecondsNow();
		
		U_ResetFrameArena();
		OS_PollEvents();
		
		R_Clear(BufferMask_Color);
		UI_BeginFrame(window, ui_cache);
		
		UI_PrefWidth(ui_cache, UI_Percentage(65))
			UI_PrefHeight(ui_cache, UI_Percentage(65))
			UI_LayoutAxis(ui_cache, axis2_y)
		{
			UI_Box* box = UI_BoxMake(ui_cache, BoxFlag_DrawBackground | BoxFlag_DrawBorder
									 | BoxFlag_HotAnimation | BoxFlag_ActiveAnimation | BoxFlag_Clip, str_lit("foo"));
			
			UI_Parent(ui_cache, box)
				UI_EdgeSize(ui_cache, 2)
				UI_PrefWidth(ui_cache, UI_TextContent(100))
				UI_PrefHeight(ui_cache, UI_Pixels(50))
				UI_LayoutAxis(ui_cache, axis2_x)
			{
				if (UI_Button(ui_cache, str_lit("fooA")).clicked) {
					Log("A Hit");
				}
				if (UI_Button(ui_cache, str_lit("fooB")).clicked) {
					Log("B Hit");
				}
				if (UI_Button(ui_cache, str_lit("fooC")).clicked) {
					Log("C Hit");
				}
				
				UI_Spacer(ui_cache, UI_Pixels(50));
				if (UI_Button(ui_cache, str_lit("fooD")).clicked) {
					Log("D Hit");
					show_btn = !show_btn;
				}
				
				if (show_btn) {
					if (UI_Button(ui_cache, str_lit("fooE")).double_clicked) {
						Log("YES!");
					}
				}
			}
		}
		
		UI_PrefWidth(ui_cache, UI_Percentage(85))
			UI_PrefHeight(ui_cache, UI_Percentage(15))
			UI_LayoutAxis(ui_cache, axis2_y) {
			UI_BoxMake(ui_cache, BoxFlag_DrawBackground | BoxFlag_DrawBorder, str_lit("barzz"));
		}
		
		UI_EndFrame(ui_cache, delta);
		
		B_BackendSwapchainNext(window);
		
		end = OS_TimeMicrosecondsNow();
	}
	
	B_BackendFree(window);
	
	OS_WindowClose(window);
	
	U_FrameArenaFree();
	arena_free(&global_arena);
	tctx_free(&context);
}
