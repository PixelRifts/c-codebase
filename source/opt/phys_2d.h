/* date = September 10th 2022 9:50 pm */

#ifndef PHYS_2D_H
#define PHYS_2D_H

#include "defines.h"
#include "base/vmath.h"
#include "base/mem.h"

typedef u32 P2D_ColliderType;
enum {
	ColliderType_Invalid,
	ColliderType_Polygon,
	ColliderType_Circle,
	ColliderType_MAX,
};

typedef struct P2D_Collider {
	P2D_ColliderType type;
	
	vec2 center_pos;
	
	union {
		struct { f32 radius; } circle;
		struct { vec2* vertices; u32 vert_count; } polygon;
	};
} P2D_Collider;

typedef struct P2D_Collision {
	vec2 resolution;
	b8 is_colliding;
} P2D_Collision;

b8 P2D_CheckCollision(P2D_Collider* a, P2D_Collider* b);
P2D_Collision P2D_GetCollision(P2D_Collider* a, P2D_Collider* b);

P2D_Collider* P2D_ColliderAllocAARect(M_Arena* arena, rect r);
P2D_Collider* P2D_ColliderAllocRotatedRect(M_Arena* arena, rect r, f32 theta);
P2D_Collider* P2D_ColliderAllocCircle(M_Arena* arena, vec2 c, f32 r);

#endif //PHYS_2D_H
