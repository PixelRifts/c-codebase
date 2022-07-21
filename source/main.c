#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

#include "opt/render_2d.h"

string_array tetrominos = {0};

i32 index_with_rotate(i32 x, i32 y, i32 r) {
	switch (r % 4) {
		case 0: return y * 4 + x;
		case 1: return 12 + y - (x * 4);
		case 2: return 15 - (y * 4) - x;
		case 3: return 3 - y + (x * 4);
	}
	return 0;
}

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	OS_Window window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	B_BackendInit(&window);
	OS_WindowShow(&window);
	
	/*//- Init tetrominos 
	string_array_add(&tetrominos, str_lit("..X."
										  "..X."
										  "..X."
										  "..X."));
	string_array_add(&tetrominos, str_lit("..X."
										  ".XX."
										  ".X.."
										  "...."));
	string_array_add(&tetrominos, str_lit(".X.."
										  ".XX."
										  "..X."
										  "...."));
	string_array_add(&tetrominos, str_lit("...."
										  ".XX."
										  ".XX."
										  "...."));
	string_array_add(&tetrominos, str_lit("..X."
										  ".XX."
										  "..X."
										  "...."));
	string_array_add(&tetrominos, str_lit("...."
										  ".XX."
										  "..X."
										  "..X."));
	string_array_add(&tetrominos, str_lit("...."
										  ".XX."
										  ".X.."
										  ".X.."));
	*/
	
	R2D_Renderer renderer = {0};
	R2D_Init(&window, &renderer);
	
	f32 x = 0;
	while (OS_WindowIsOpen(&window)) {
		OS_PollEvents();
		
		R_Clear(BufferMask_Color);
		
		x += 0.1;
		R2D_BeginDraw(&renderer);
		R2D_DrawQuadC(&renderer, (rect) { x, 100, 100, 100 }, (vec4) { 0.8f, 0.2f, 0.3f, 1.f });
		R2D_EndDraw(&renderer);
		
		B_BackendSwapchainNext(&window);
	}
	
	R2D_Free(&renderer);
	
	B_BackendFree(&window);
	OS_WindowClose(&window);
	
	tctx_free(&context);
}
