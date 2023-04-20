#include "mem.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

// Dependency on the OS. Although it is generic
#include "os/os.h"

#define DEFAULT_ALIGNMENT sizeof(void*)

b8 is_power_of_two(uintptr_t x) {
	return (x & (x-1)) == 0;
}

u64 align_forward_u64(u64 ptr, u64 align) {
	u64 p, a, modulo;
    
	assert(is_power_of_two(align));
    
	p = ptr;
	a = (size_t)align;
	// Same as (p % a) but faster as 'a' is a power of two
	modulo = p & (a-1);
    
	if (modulo != 0) {
		// If 'p' address is not aligned, push the address to the
		// next value which is aligned
		p += a - modulo;
	}
	return p;
}


//~ Arena

void* arena_alloc(M_Arena* arena, u64 size) {
    void* memory = 0;
	
	// align!
	size = align_forward_u64(size, DEFAULT_ALIGNMENT);
	
    if (arena->alloc_position + size > arena->commit_position) {
        if (!arena->static_size) {
            u64 commit_size = size;
            
            commit_size += M_ARENA_COMMIT_SIZE - 1;
            commit_size -= commit_size % M_ARENA_COMMIT_SIZE;
            
            if (arena->commit_position >= arena->max) {
                assert(0 && "Arena is out of memory");
            } else {
                OS_MemoryCommit(arena->memory + arena->commit_position, commit_size);
                arena->commit_position += commit_size;
            }
        } else {
            assert(0 && "Static-Size Arena is out of memory");
        }
    }
    
    memory = arena->memory + arena->alloc_position;
    arena->alloc_position += size;
    return memory;
}

void* arena_alloc_zero(M_Arena* arena, u64 size) {
    void* result = arena_alloc(arena, size);
    memset(result, 0, size);
    return result;
}

void arena_dealloc(M_Arena* arena, u64 size) {
    if (size > arena->alloc_position)
        size = arena->alloc_position;
    arena->alloc_position -= size;
}

void arena_dealloc_to(M_Arena* arena, u64 pos) {
    if (pos > arena->max) pos = arena->max;
    if (pos < 0) pos = 0;
    arena->alloc_position = pos;
}

void* arena_raise(M_Arena* arena, void* ptr, u64 size) {
    void* raised = arena_alloc(arena, size);
    memcpy(raised, ptr, size);
    return raised;
}

void* arena_alloc_array_sized(M_Arena* arena, u64 elem_size, u64 count) {
    return arena_alloc(arena, elem_size * count);
}

void arena_init(M_Arena* arena) {
	MemoryZeroStruct(arena, M_Arena);
    arena->max = M_ARENA_MAX;
    arena->memory = OS_MemoryReserve(arena->max);
    arena->alloc_position = 0;
    arena->commit_position = 0;
    arena->static_size = false;
}

void arena_init_sized(M_Arena* arena, u64 max) {
	MemoryZeroStruct(arena, M_Arena);
	arena->max = max;
    arena->memory = OS_MemoryReserve(arena->max);
    arena->alloc_position = 0;
    arena->commit_position = 0;
    arena->static_size = false;
}

void arena_clear(M_Arena* arena) {
    arena_dealloc(arena, arena->alloc_position);
}

void arena_free(M_Arena* arena) {
    OS_MemoryRelease(arena->memory, arena->max);
}

//~ Temp arena

M_ArenaTemp arena_begin_temp(M_Arena* arena) {
    return (M_ArenaTemp) { arena, arena->alloc_position };
}

void arena_end_temp(M_ArenaTemp temp) {
    arena_dealloc_to(temp.arena, temp.pos);
}

//~ Scratch Blocks

M_Scratch scratch_get(void) {
	ThreadContext* ctx = (ThreadContext*) OS_ThreadContextGet();
	return tctx_scratch_get(ctx);
}

void scratch_reset(M_Scratch* scratch) {
	ThreadContext* ctx = (ThreadContext*) OS_ThreadContextGet();
	tctx_scratch_reset(ctx, scratch);
}

void scratch_return(M_Scratch* scratch) {
	ThreadContext* ctx = (ThreadContext*) OS_ThreadContextGet();
	tctx_scratch_return(ctx, scratch);
}

//~ Pool

void pool_init(M_Pool* pool, u64 element_size) {
	MemoryZeroStruct(pool, M_Pool);
	pool->memory = OS_MemoryReserve(M_POOL_MAX);
	pool->max = M_POOL_MAX;
	pool->commit_position = 0;
	pool->element_size = align_forward_u64(element_size, DEFAULT_ALIGNMENT);
	pool->head = nullptr;
}

void pool_clear(M_Pool* pool) {
	for (u8* it = (u8*)pool + sizeof(M_Pool), *preit = it;
		 it <= (u8*)pool->memory + pool->commit_position;
		 preit = it, it += pool->element_size) {
		((M_PoolFreeNode*)preit)->next = (M_PoolFreeNode*)it;
	}
	pool->head = (M_PoolFreeNode*)pool->memory;
}

void pool_free(M_Pool* pool) {
	OS_MemoryRelease(pool->memory, pool->max);
}

void* pool_alloc(M_Pool* pool) {
	if (pool->head) {
		void* ret = pool->head;
		pool->head = pool->head->next;
		return ret;
	} else {
		if (pool->commit_position + M_POOL_COMMIT_CHUNK * pool->element_size >= pool->max) {
			assert(0 && "Pool is out of memory");
			return nullptr;
		}
		void* commit_ptr = pool->memory + pool->commit_position;
		OS_MemoryCommit(commit_ptr, M_POOL_COMMIT_CHUNK * pool->element_size);
		pool_dealloc_range(pool, commit_ptr, M_POOL_COMMIT_CHUNK);
		
		return pool_alloc(pool);
	}
}

void pool_dealloc(M_Pool* pool, void* ptr) {
	((M_PoolFreeNode*)ptr)->next = pool->head;
	pool->head = ptr;
}

void pool_dealloc_range(M_Pool* pool, void* ptr, u64 count) {
	u8* it = ptr;
	for (u64 k = 0; k < count; k++) {
		((M_PoolFreeNode*)it)->next = pool->head;
		pool->head = (M_PoolFreeNode*) it;
		it += pool->element_size;
	}
}
