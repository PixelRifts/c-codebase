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
	
	M_Arena global_arena;
	arena_init(&global_arena);
	u32* k = arena_alloc(&global_arena, sizeof(u32) * 360);
	k[10] = 50;
	
	vec2 actual_position = { OS_InputGetMouseX(), OS_InputGetMouseY() };
	vec2 corrected_position = { OS_InputGetMouseX(), OS_InputGetMouseY() };
	
	P2D_Collider* a = P2D_ColliderAllocAARect(&global_arena, (rect) { 200, 200, 100, 100 });
	P2D_Collider* b = P2D_ColliderAllocCircle(&global_arena, actual_position, 25);
	
	OS_Window* window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	B_BackendInit(window);
	OS_WindowShow(window);
	
	R2D_Renderer renderer = {0};
	R2D_Init(window, &renderer);
	
	while (OS_WindowIsOpen(window)) {
		OS_PollEvents();
		
		actual_position = (vec2) { OS_InputGetMouseX(), OS_InputGetMouseY() };
		P2D_ColliderMoveTo(b, actual_position);
		P2D_Collision ab_resolver = P2D_GetCollision(a, b);
		corrected_position = vec2_add(actual_position, ab_resolver.resolution);
		
		R_Viewport(0, 0, window->width, window->height);
		R_Clear(BufferMask_Color);
		
		R2D_BeginDraw(&renderer);
		R2D_DrawQuadC(&renderer, (rect) { 200, 200, 100, 100 }, Color_Red);
		R2D_DrawCircle(&renderer, actual_position, 25,P2D_CheckCollision(a, b) ? Color_Blue : Color_Green);
		R2D_DrawCircle(&renderer, corrected_position, 25, Color_Green);
		
		R2D_DrawLineC(&renderer, b->center_pos, vec2_add(b->center_pos, ab_resolver.resolution), 5, Color_Green);
		R2D_EndDraw(&renderer);
		
		B_BackendSwapchainNext(window);
	}
	
	B_BackendFree(window);
	
	OS_WindowClose(window);
	
	arena_free(&global_arena);
	tctx_free(&context);
}
