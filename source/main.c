#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

#include "opt/phys_2d.h"
#include "opt/render_2d.h"

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	M_Arena* global_arena = arena_make();
	
	
	P2D_Collider* a = P2D_ColliderAllocAARect(global_arena, (rect) { 0, 0, 100, 100 });
	P2D_Collider* b = P2D_ColliderAllocRotatedRect(global_arena, (rect) { 50, 50, 100, 100 }, HALF_PI / 2.f);
	
	Log("A, B: %d", P2D_CheckCollision(a, b));
	
	P2D_Collision ab_resolver = P2D_GetCollision(a, b);
	Log("AB-Resolution: (%2.3f, %2.3f)", ab_resolver.resolution.x, ab_resolver.resolution.y);
	
	
	OS_Window* window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	B_BackendInit(window);
	OS_WindowShow(window);
	
	R2D_Renderer renderer = {0};
	R2D_Init(window, &renderer);
	
	while (OS_WindowIsOpen(window)) {
		OS_PollEvents();
		
		R_Viewport(0, 0, window->width, window->height);
		R_Clear(BufferMask_Color);
		
		R2D_BeginDraw(&renderer);
		R2D_DrawQuadC(&renderer, (rect) { 0, 0, 100, 100 }, Color_Red);
		R2D_DrawQuadRotatedC(&renderer, (rect) { 50, 50, 100, 100 }, Color_Blue, HALF_PI / 2.f);
		R2D_DrawLineC(&renderer, b->center_pos, vec2_add(b->center_pos, ab_resolver.resolution), 5, Color_Green);
		R2D_EndDraw(&renderer);
		
		B_BackendSwapchainNext(window);
	}
	
	B_BackendFree(window);
	
	OS_WindowClose(window);
	
	arena_free(global_arena);
	tctx_free(&context);
}
