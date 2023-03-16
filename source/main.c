#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

void MyResizeCallback(OS_Window* window, i32 w, i32 h) {
	// TODO(voxel): @awkward Add a "first resize" to Win32Window so that This if isn't required
	if (window->user_data) {
		R_Viewport(0, 0, w, h);
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
	
	f32 start = 0.f; f32 end = 0.016f;
	f32 delta = 0.016f;
	
	while (OS_WindowIsOpen(window)) {
		delta = end - start;
		start = OS_TimeMicrosecondsNow();
		
		U_ResetFrameArena();
		OS_PollEvents();
		R_Clear(BufferMask_Color);
		
		
		B_BackendSwapchainNext(window);
		end = OS_TimeMicrosecondsNow();
	}
	
	B_BackendFree(window);
	
	OS_WindowClose(window);
	
	U_FrameArenaFree();
	arena_free(&global_arena);
	tctx_free(&context);
}
