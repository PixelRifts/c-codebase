#include "mem.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "base/log.h"

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
                OS_MemoryCommit(((void*)arena) + sizeof(M_Arena) + arena->commit_position, commit_size);
                arena->commit_position += commit_size;
            }
        } else {
            assert(0 && "Static-Size Arena is out of memory");
        }
    }
    
    memory = arena + sizeof(M_Arena) + arena->alloc_position;
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

M_Arena* arena_make() {
    M_Arena* arena = OS_MemoryReserve(sizeof(M_Arena) + M_ARENA_MAX);
	OS_MemoryCommit(arena, sizeof(M_Arena));
    arena->max = M_ARENA_MAX;
    arena->alloc_position = 0;
    arena->commit_position = 0;
    arena->static_size = false;
	return arena;
}

M_Arena* arena_make_sized(u64 max) {
    M_Arena* arena = OS_MemoryReserve(sizeof(M_Arena) + max);
	OS_MemoryCommit(arena, sizeof(M_Arena));
	arena->max = max;
    arena->alloc_position = 0;
    arena->commit_position = 0;
    arena->static_size = false;
	return arena;
}

void arena_clear(M_Arena* arena) {
    arena_dealloc(arena, arena->alloc_position);
}

void arena_free(M_Arena* arena) {
    OS_MemoryRelease(arena, sizeof(M_Arena) + arena->max);
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

//~ Pool Allocator

M_Pool* pool_make(u64 element_size) {
	M_Pool* pool = OS_MemoryReserve(sizeof(M_Pool) + M_POOL_MAX);
	OS_MemoryCommit(pool, sizeof(M_Pool));
	pool->max = M_POOL_MAX;
	pool->commit_position = 0;
	pool->element_size = align_forward_u64(element_size, DEFAULT_ALIGNMENT);
	pool->head = nullptr;
	return pool;
}

void pool_clear(M_Pool* pool) {
	for (void *it = (void*)pool + sizeof(M_Pool), *preit = it;
		 it <= (void*)pool + sizeof(M_Pool) + pool->commit_position;
		 preit = it, it += pool->element_size) {
		((M_PoolFreeNode*)preit)->next = (M_PoolFreeNode*)it;
	}
	pool->head = (void*)pool + sizeof(M_Pool);
}

void pool_free(M_Pool* pool) {
	OS_MemoryRelease(pool, sizeof(M_Pool) + pool->max);
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
		void* commit_ptr = ((void*)pool) + sizeof(M_Pool) + pool->commit_position;
		OS_MemoryCommit(commit_ptr, M_POOL_COMMIT_CHUNK * pool->element_size);
		pool_dealloc_range(pool, commit_ptr, M_POOL_COMMIT_CHUNK);
		
		return pool_alloc(pool);
	}
}

void  pool_dealloc(M_Pool* pool, void* ptr) {
	((M_PoolFreeNode*)ptr)->next = pool->head;
	pool->head = ptr;
}

void  pool_dealloc_range(M_Pool* pool, void* ptr, u64 count) {
	void* it = ptr;
	for (u64 k = 0; k < count; k++) {
		((M_PoolFreeNode*)it)->next = pool->head;
		pool->head = it;
		it += pool->element_size;
	}
}

//~ Heap Allocator

M_Heap* heap_make(void) {
	M_Heap* heap = OS_MemoryReserve(sizeof(M_Heap) + M_HEAP_MAX);
	OS_MemoryCommit(heap, sizeof(M_Heap));
	heap->max = M_HEAP_MAX;
	heap->commit_position = 0;
	heap->head = nullptr;
	return heap;
}

void heap_clear(M_Heap* heap) {
	heap->head = ((void*)heap) + sizeof(M_Heap);
	heap->head->next = nullptr;
	heap->head->size = heap->commit_position;
}

void heap_free(M_Heap* heap) {
	OS_MemoryRelease(heap, sizeof(M_Heap) + M_HEAP_MAX);
}

void* heap_alloc(M_Heap* heap, u64 size) {
	size += sizeof(M_HeapFreeNode) - 1;
	size -= size % sizeof(M_HeapFreeNode);
	
	if (heap->head) {
		i64 delta = heap->head->size - size;
		if (delta < 0) {
			M_HeapFreeNode* it = heap->head;
			M_HeapFreeNode* prev_it = heap->head;
			while (delta < 0) {
				prev_it = it;
				it = it->next;
				if (!it) goto commit_more;
				
				delta = it->size - size;
			}
			
			if (delta >= 0 && delta < sizeof(M_HeapFreeNode)) {
				void* ret = it;
				prev_it->next = it->next;
				//memset(ret, 0, size);
				return ret;
			} else {
				void* ret = it;
				u64 new_size = it->size - size;
				M_HeapFreeNode* splitter = ret + size;
				splitter->next = it->next;
				prev_it->next = splitter;
				splitter->size = new_size;
				//memset(ret, 0, size);
				return ret;
			}
		} else if (delta >= 0 && delta < sizeof(M_HeapFreeNode)) {
			void* ret = heap->head;
			heap->head = heap->head->next;
			//memset(ret, 0, size);
			return ret;
		} else {
			void* ret = heap->head;
			u64 new_size = heap->head->size - size;
			M_HeapFreeNode* splitter = ret + size;
			splitter->next = heap->head->next;
			heap->head = splitter;
			splitter->size = new_size;
			//memset(ret, 0, size);
			return ret;
		}
	}
	
	commit_more:
	{
		if (heap->commit_position + size >= heap->max) {
			assert(0 && "Heap is out of memory");
			return nullptr;
		}
		
		u64 commit_size = size;
		commit_size += M_HEAP_COMMIT_SIZE - 1;
		commit_size -= commit_size % M_HEAP_COMMIT_SIZE;
		commit_size += M_HEAP_COMMIT_SIZE; // Add a bit more space for the heap
		
		Log("Commit %llu", commit_size);
		
		void* commit_ptr = ((void*)heap) + sizeof(M_Heap) + heap->commit_position;
		OS_MemoryCommit(commit_ptr, commit_size);
		heap->head = commit_ptr;
		heap->head->size = commit_size;
		heap->head->next = nullptr;
		
		heap->commit_position += commit_size;
		return heap_alloc(heap, size);
	}
}

void  heap_dealloc(M_Heap* heap, void* ptr, u64 size) {
	size += sizeof(M_HeapFreeNode) - 1;
	size -= size % sizeof(M_HeapFreeNode);
	
	M_HeapFreeNode* it = heap->head;
	M_HeapFreeNode* prev_it = nullptr;
	
	M_HeapFreeNode* back = nullptr;
	M_HeapFreeNode* front = nullptr;
	M_HeapFreeNode* front_prev = nullptr;
	
	while (it) {
		if (((void*)it) + it->size == ptr) {
			back = it;
		} else if (ptr + size == it) {
			front = it;
			front_prev = prev_it;
		}
		
		if (back && front) break;
		
		prev_it = it;
		it = it->next;
	}
	
	M_HeapFreeNode* new_free_node = (M_HeapFreeNode*)ptr;
	new_free_node->next = heap->head;
	new_free_node->size = size;
	
	// Coalesce
	if (front) {
		new_free_node->next = front->next;
		new_free_node->size += front->size;
		if (front_prev) {
			front_prev->next = new_free_node;
		} else heap->head = new_free_node;
	}
	if (back) {
		if (front) {
			front_prev->next = nullptr;
		}
		back->size += new_free_node->size;
	}
	
	if (!(front || back)) {
		// Accept Dead Section :(
		// Not much we can do there, but alignment should stop this from ever happening
		if (size < sizeof(M_HeapFreeNode)) return;
		
		heap->head = new_free_node;
	}
}
