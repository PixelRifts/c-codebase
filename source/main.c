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
	
	OS_Window window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	B_BackendInit(&window);
	OS_WindowShow(&window);
	
	
	
	while (OS_WindowIsOpen(&window)) {
		OS_PollEvents();
		
		B_BackendSwapchainNext(&window);
	}
	
	B_BackendFree(&window);
	OS_WindowClose(&window);
	
	tctx_free(&context);
}
