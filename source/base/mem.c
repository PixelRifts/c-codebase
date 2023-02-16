#include "mem.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

// Dependency on the OS. Although it is generic
#include "os/os.h"

// Using the OS Layer here now, gotta implement one for linux
/*
#ifdef PLATFORM_WIN
#include <windows.h>
#elif defined(PLATFORM_LINUX)
#include <sys/mman.h>
#endif

static void* mem_reserve(u64 size) {
#ifdef PLATFORM_WIN
    void* memory = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
#elif defined(PLATFORM_LINUX)
    void* memory = mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
#endif
    return memory;
}

static void mem_release(void* memory, u64 size) {
#ifdef PLATFORM_WIN
    VirtualFree(memory, 0, MEM_RELEASE);
#elif defined(PLATFORM_LINUX)
    munmap(memory, size);
#endif
}

static void mem_commit(void* memory, u64 size) {
#ifdef PLATFORM_WIN
    VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE);
#elif defined(PLATFORM_LINUX)
    mprotect(memory, size, PROT_READ | PROT_WRITE);
#endif
}

static void mem_decommit(void* memory, u64 size) {
#ifdef PLATFORM_WIN
    VirtualFree(memory, size, MEM_DECOMMIT);
#elif defined(PLATFORM_LINUX)
    mprotect(memory, size, PROT_NONE);
#endif
}
*/

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


//~ Arena (No alignment)

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
	return tctx_scratch_reset(ctx, scratch);
}

void scratch_return(M_Scratch* scratch) {
	ThreadContext* ctx = (ThreadContext*) OS_ThreadContextGet();
	return tctx_scratch_return(ctx, scratch);
}