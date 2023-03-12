#include "mem.h"
// Dependency on the OS. Although it is generic
#include "os/os.h"

void tctx_init(ThreadContext* ctx) {
	arena_init(&ctx->arena);
	OS_ThreadContextSet(ctx);
}

void tctx_free(ThreadContext* ctx) {
	arena_free(&ctx->arena);
	OS_ThreadContextSet(ctx);
}

M_Scratch tctx_scratch_get(ThreadContext* ctx) {
	if (!ctx->free_list) {
        M_Scratch scratch = {0};
        
        void* ptr = arena_alloc(&ctx->arena, M_SCRATCH_SIZE);
        scratch.arena.memory = ptr;
        scratch.arena.max = M_SCRATCH_SIZE;
        scratch.arena.alloc_position = 0;
        scratch.arena.commit_position = M_SCRATCH_SIZE;
        scratch.arena.static_size = true;
        
        ctx->max_created++;
        return scratch;
    } else {
        M_Scratch scratch = {0};
        
        scratch.arena.memory = (u8*) ctx->free_list;
        scratch.arena.max = M_SCRATCH_SIZE;
        scratch.arena.alloc_position = 0;
        scratch.arena.commit_position = M_SCRATCH_SIZE;
        scratch.arena.static_size = true;
        
        ctx->free_list = ctx->free_list->next;
        return scratch;
    }
}

void tctx_scratch_reset(ThreadContext* ctx, M_Scratch* scratch) {
	scratch->arena.alloc_position = 0;
}

void tctx_scratch_return(ThreadContext* ctx, M_Scratch* scratch) {
	scratch_free_list_node* prev_head = ctx->free_list;
    ctx->free_list = (scratch_free_list_node*) scratch->arena.memory;
    ctx->free_list->next = prev_head;
}
