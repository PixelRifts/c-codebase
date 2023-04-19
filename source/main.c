#include "defines.h"
#include "os/os.h"
#include "base/tctx.h"
#include "base/log.h"
#include "os/window.h"

int main() {
	OS_Init();
	ThreadContext context = {0};
	tctx_init(&context);
	M_Arena global_arena = {0};
	arena_init(&global_arena);
	
	OS_Window* window = OS_WindowCreate(1080, 720, str_lit("Yeet"));
	OS_WindowShow(window);
	while (OS_WindowIsOpen(window)) {
		OS_PollEvents();
	}
	OS_WindowClose(window);
	
	arena_free(&global_arena);
	tctx_free(&context);
}
