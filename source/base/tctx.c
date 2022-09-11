#include "mem.h"
// Dependency on the OS. Although it is generic
#include "os/os.h"

void tctx_init(ThreadContext* ctx) {
	ctx->arena = arena_make();
	OS_ThreadContextSet(ctx);
}

void tctx_free(ThreadContext* ctx) {
	arena_free(ctx->arena);
	OS_ThreadContextSet(ctx);
}

M_Scratch tctx_scratch_get(ThreadContext* ctx) {
	ctx->pop_to = ctx->arena->alloc_position;
	return (M_Scratch) {
		.arena = ctx->arena,
		.pos = ctx->pop_to,
	};
}

void tctx_scratch_reset(ThreadContext* ctx, M_Scratch* scratch) {
	arena_clear(scratch->arena);
}

void tctx_scratch_return(ThreadContext* ctx, M_Scratch* scratch) {
	arena_dealloc_to(scratch->arena, scratch->pos);
}
