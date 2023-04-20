#include "defines.h"
#include "os/os.h"
#include "base/tctx.h"
#include "base/log.h"
#include "os/window.h"
#include "os/input.h"

void mybtncallback(OS_Window* window, u8 btn, i32 action) {
}

int main() {
	OS_Init();
	ThreadContext context = {0};
	tctx_init(&context);
	M_Arena global_arena = {0};
	arena_init(&global_arena);
	
	OS_Window* window = OS_WindowCreate(1080, 720, str_lit("Yeet"));
	window->button_callback = mybtncallback;
	OS_WindowShow(window);
	while (OS_WindowIsOpen(window)) {
		OS_PollEvents();
		
		if (OS_InputKeyPressed(Input_Key_Apostrophe)) {
			Log("Foo");
		}
		if (OS_InputButtonPressed(Input_MouseButton_Left)) {
			Log("Bar");
		}
		
		f32 yscroll = OS_InputGetMouseScrollY();
		if (yscroll) Log("%2.3f", yscroll);
	}
	OS_WindowClose(window);
	
	arena_free(&global_arena);
	tctx_free(&context);
}
