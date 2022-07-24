#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	M_Arena global_arena = {0};
	arena_init(&global_arena);
	
	OS_Window window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	B_BackendInit(&window);
	OS_WindowShow(&window);
	
	f32 start, end, dt;
	start = OS_TimeMicrosecondsNow();
	
	while (OS_WindowIsOpen(&window)) {
		OS_PollEvents();
		
		end = OS_TimeMicrosecondsNow();
		dt = (end - start) / 1e6;
		start = OS_TimeMicrosecondsNow();
		
		B_BackendSwapchainNext(&window);
	}
	
	B_BackendFree(&window);
	OS_WindowClose(&window);
	
	arena_free(&global_arena);
	
	tctx_free(&context);
}
