/* date = September 27th 2021 11:45 am */

#ifndef MEM_H
#define MEM_H

#include <stdlib.h>
#include "defines.h"

//~ Arena (Linear Allocator)

typedef struct M_Arena {
    u8* memory;
    u64 max;
    u64 alloc_position;
    u64 commit_position;
    b8 static_size;
} M_Arena;

#define M_ARENA_MAX Gigabytes(1)
#define M_ARENA_COMMIT_SIZE Kilobytes(8)

void* arena_alloc(M_Arena* arena, u64 size);
void* arena_alloc_zero(M_Arena* arena, u64 size);
void  arena_dealloc(M_Arena* arena, u64 size);
void  arena_dealloc_to(M_Arena* arena, u64 pos);
void* arena_raise(M_Arena* arena, void* ptr, u64 size);
void* arena_alloc_array_sized(M_Arena* arena, u64 elem_size, u64 count);

#define arena_alloc_array(arena, elem_type, count) \
arena_alloc_array_sized(arena, sizeof(elem_type), count)

void arena_init(M_Arena* arena);
void arena_init_sized(M_Arena* arena, u64 max);
void arena_clear(M_Arena* arena);
void arena_free(M_Arena* arena);

typedef struct M_ArenaTemp {
    M_Arena* arena;
    u64 pos;
} M_ArenaTemp;

M_ArenaTemp arena_begin_temp(M_Arena* arena);
void        arena_end_temp(M_ArenaTemp temp);

//~ Scratch Helpers
// A scratch block is just a view into an arena
#include "tctx.h"

M_Scratch scratch_get(void);
void scratch_reset(M_Scratch* scratch);
void scratch_return(M_Scratch* scratch);

//~ Pool (Pool Allocator)

typedef struct M_PoolFreeNode M_PoolFreeNode;
struct M_PoolFreeNode { M_PoolFreeNode* next; };

typedef struct M_Pool {
	u8* memory;
	u64 max;
	u64 commit_position;
	u64 element_size;
	
	M_PoolFreeNode* head;
} M_Pool;

#define M_POOL_MAX Gigabytes(1)
#define M_POOL_COMMIT_CHUNK 32

void pool_init(M_Pool* pool, u64 element_size);
void pool_clear(M_Pool* pool);
void pool_free(M_Pool* pool);

void* pool_alloc(M_Pool* pool);
void  pool_dealloc(M_Pool* pool, void* ptr);
void  pool_dealloc_range(M_Pool* pool, void* ptr, u64 count);

#endif //MEM_H