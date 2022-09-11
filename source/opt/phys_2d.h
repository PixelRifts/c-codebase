/* date = September 10th 2022 9:50 pm */

#ifndef PHYS_2D_H
#define PHYS_2D_H

typedef u32 P2D_ColliderType;
enum {
	ColliderType_Invalid,
	ColliderType_Polygon,
	ColliderType_Circle,
	ColliderType_MAX,
};

typedef struct P2D_Collider {
	P2D_ColliderType;
	
	union {
		struct { vec2 pos; f32 radius; } circle;
		struct { u32 vert_count; vec2* vertices; } polygon;
	};
} P2D_Collider;

#endif //PHYS_2D_H
