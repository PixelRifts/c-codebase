#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

typedef struct foo {
	i32 a;
	b32 b;
	f32 c;
} foo;

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	Log("====== Arena ======");
	M_Arena* arena = arena_make();
	arena_free(arena);
	
	Log("====== Pool ======");
	M_Pool* pool = pool_make(sizeof(foo));
	for (u32 i = 0; i < 36; i++) {
		foo* f = pool_alloc(pool);
		Log("%d: %llu", i, (u64)f);
		
		if (i % 2 == 0)
			pool_dealloc(pool, f);
	}
	pool_clear(pool);
	pool_free(pool);
	
	Log("");
	Log("====== Heap ======");
	
	M_Heap* heap = heap_make();
	foo* f = heap_alloc(heap, sizeof(foo));
	Log("f: %llu", (u64)f);
	heap_alloc(heap, sizeof(float) * 64);
	heap_dealloc(heap, f, sizeof(foo));
	foo* f2 = heap_alloc(heap, sizeof(foo));
	Log("f2: %llu", (u64)f2);
	heap_free(heap);
	
	tctx_free(&context);
}
