#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

#include "opt/phys_2d.h"

typedef struct foo {
	i32 a;
	b32 b;
	f32 c;
} foo;

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	M_Arena* global_arena = arena_make();
	
	P2D_Collider* a = P2D_ColliderAllocAARect(global_arena, (rect) { 0, 0, 10, 10 });
	P2D_Collider* b = P2D_ColliderAllocRotatedRect(global_arena, (rect) { 0, 0, 10, 10 }, HALF_PI);
	P2D_Collider* c = P2D_ColliderAllocCircle(global_arena, (vec2) { 50, 10 }, 15);
	
	Log("A, B: %d", P2D_CheckCollision(a, b));
	Log("B, C: %d", P2D_CheckCollision(b, c));
	Log("C, A: %d", P2D_CheckCollision(c, a));
	
	arena_free(global_arena);
	
	tctx_free(&context);
}
