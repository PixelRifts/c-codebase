/* date = June 30th 2022 0:24 pm */

#ifndef TCTX_H
#define TCTX_H

#include "defines.h"

#define M_SCRATCH_SIZE Kilobytes(16)

typedef struct M_ArenaTemp M_Scratch;

typedef struct ThreadContext {
	M_Arena* arena;
	u64 pop_to;
} ThreadContext;

void tctx_init(ThreadContext* ctx);
void tctx_free(ThreadContext* ctx);

M_Scratch tctx_scratch_get(ThreadContext* ctx);
void tctx_scratch_reset(ThreadContext* ctx, M_Scratch* scratch);
void tctx_scratch_return(ThreadContext* ctx, M_Scratch* scratch);

#endif //TCTX_H
