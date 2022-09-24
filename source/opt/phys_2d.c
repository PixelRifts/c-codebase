#include "phys_2d.h"

#include "base/ds.h"

DArray_Prototype(vec2);
DArray_Impl(vec2);

Slice_Prototype(vec2);

static vec2 P2D_GJK_FurthestPoint(P2D_Collider* c, vec2 d) {
	switch (c->type) {
		case ColliderType_Polygon: {
			vec2 furthest_vert = {0};
			f32 comparing_dot = 0;
			for (u32 i = 0; i < c->polygon.vert_count; i++) {
				vec2 c_to_vert = vec2_sub(c->polygon.vertices[i], c->center_pos);
				f32 dot = c_to_vert.x * d.x + c_to_vert.y * d.y;
				if (comparing_dot < dot) {
					comparing_dot = dot;
					furthest_vert = c->polygon.vertices[i];
				}
			}
			return furthest_vert;
		} break;
		
		case ColliderType_Circle: {
			return vec2_add(c->center_pos, vec2_scale(d, c->circle.radius));
		} break;
	}
	return (vec2) {0};
}

static vec2 P2D_GJK_Support(P2D_Collider* a, P2D_Collider* b, vec2 d) {
	return vec2_sub(
					P2D_GJK_FurthestPoint(a, d),
					P2D_GJK_FurthestPoint(b, vec2_neg(d))
					);
}

static b8 P2D_GJK_HandleSimplex(darray(vec2)* simplex, vec2* d) {
	if (simplex->len == 2) {
		// Line case
		vec2 b = simplex->elems[0], a = simplex->elems[1];
		vec2 ab = vec2_sub(b, a), ao = vec2_neg(a);
		vec2 ab_perp = vec2_normalize(vec2_triple_product(ab, ao, ab));
		*d = ab_perp;
		return false;
	} else {
		// Triangle case
		vec2 c = simplex->elems[0], b = simplex->elems[1], a = simplex->elems[2];
		vec2 ab = vec2_sub(b, a), ac = vec2_sub(c, a), ao = vec2_neg(a);
		vec2 ab_perp = vec2_normalize(vec2_triple_product(ac, ab, ab));
		vec2 ac_perp = vec2_normalize(vec2_triple_product(ab, ac, ac));
		if (vec2_dot(ab_perp, ao) < 0) {
			darray_remove(vec2, simplex, 0);
			*d = ab_perp;
			return false;
		} else if (vec2_dot(ac_perp, ao) < 0) {
			// erase B from simplex
			darray_remove(vec2, simplex, 1);
			*d = ac_perp;
			return false;
		} else return true;
	}
	return false;
}

b8 P2D_CheckCollision(P2D_Collider* a, P2D_Collider* b) {
	vec2 d = vec2_sub(b->center_pos, a->center_pos);
	if (d.x || d.y) d = vec2_normalize(d);
	else d = (vec2) { 1, 0 };
	
	darray(vec2) simplex = {0};
	darray_reserve(vec2, &simplex, 3);
	darray_add(vec2, &simplex, P2D_GJK_Support(a, b, d));
	
	d = vec2_normalize(vec2_neg(simplex.elems[0]));
	
	while (true) {
		vec2 A = P2D_GJK_Support(a, b, d);
		if (vec2_dot(A, d) <= 0) {
			darray_free(vec2, &simplex);
			return false;
		}
		darray_add(vec2, &simplex, A);
		if (P2D_GJK_HandleSimplex(&simplex, &d)) {
			darray_free(vec2, &simplex);
			return true;
		}
	}
	
	darray_free(vec2, &simplex);
	return false;
}

P2D_Collision P2D_GetCollision(P2D_Collider* a, P2D_Collider* b) {
	// GJK
	
	b8 colliding = false;
	vec2 resolution = {0};
	
	vec2 d = vec2_sub(b->center_pos, a->center_pos);
	if (d.x || d.y) d = vec2_normalize(d);
	else d = (vec2) { 1, 0 };
	
	darray(vec2) simplex = {0};
	darray_reserve(vec2, &simplex, 3);
	darray_add(vec2, &simplex, P2D_GJK_Support(a, b, d));
	
	d = vec2_normalize(vec2_neg(simplex.elems[0]));
	
	while (true) {
		vec2 A = P2D_GJK_Support(a, b, d);
		if (vec2_dot(A, d) <= 0) {
			colliding = false;
			break;
		}
		darray_add(vec2, &simplex, A);
		if (P2D_GJK_HandleSimplex(&simplex, &d)) {
			colliding = true;
			break;
		}
	}
	
	// EPA
	if (colliding) {
		
		u32 min_index = 0;
		f32 min_distance = FLOAT_MAX;
		vec2 min_normal = {0};
		
		while (EpsilonEquals(min_distance, FLOAT_MAX)) {
			for (u32 i = 0; i < simplex.len; i++) {
				u32 j = (i + 1) % simplex.len;
				
				vec2 vert_i = simplex.elems[i];
				if (!vert_i.x && !vert_i.y) break;
				vec2 vert_j = simplex.elems[j];
				if (!vert_j.x && !vert_j.y) break;
				
				vec2 ij = vec2_sub(vert_j, vert_i);
				vec2 normal = { -ij.y, ij.x };
				
				normal = vec2_normalize(normal);
				f32 dist = vec2_dot(normal, vert_i);
				if (dist < 0) {
					dist *= -1;
					normal = vec2_neg(normal);
				}
				
				if (dist < min_distance) {
					min_distance = dist;
					min_normal = normal;
					min_index = j;
				}
			}
			
			vec2 support = P2D_GJK_Support(a, b, min_normal);
			f32 s_dist = vec2_dot(min_normal, support);
			if (!EpsilonEquals(s_dist, min_distance)) {
				min_distance = FLOAT_MAX;
				darray_add_at(vec2, &simplex, support, min_index);
			}
		}
		
		resolution = vec2_scale(min_normal, min_distance + EPSILON);
	}
	
	return (P2D_Collision) {
		.is_colliding = colliding,
		.resolution = resolution,
		.simplex_verts = simplex.elems,
		.simplex_vert_count = simplex.len,
	};
}

P2D_Collider* P2D_ColliderAllocAARect(M_Arena* arena, rect r) {
	P2D_Collider* collider = arena_alloc(arena, sizeof(P2D_Collider));
	collider->type = ColliderType_Polygon;
	collider->polygon.vertices = arena_alloc(arena, sizeof(vec2) * 4);
	collider->polygon.vert_count = 4;
	collider->polygon.vertices[0] = (vec2) { r.x, r.y };
	collider->polygon.vertices[1] = (vec2) { r.x + r.w, r.y };
	collider->polygon.vertices[2] = (vec2) { r.x + r.w, r.y + r.h };
	collider->polygon.vertices[3] = (vec2) { r.x, r.y + r.h };
	collider->center_pos = (vec2) { r.x + r.w / 2.f, r.y + r.h / 2.f };
	return collider;
}

P2D_Collider* P2D_ColliderAllocRotatedRect(M_Arena* arena, rect r, f32 theta) {
	P2D_Collider* collider = arena_alloc(arena, sizeof(P2D_Collider));
	collider->type = ColliderType_Polygon;
	collider->polygon.vertices = arena_alloc(arena, sizeof(vec2) * 4);
	collider->polygon.vert_count = 4;
	collider->polygon.vertices[0] = (vec2) { -r.w / 2.f, -r.h / 2.f };
	collider->polygon.vertices[1] = (vec2) {  r.w / 2.f, -r.h / 2.f };
	collider->polygon.vertices[2] = (vec2) {  r.w / 2.f,  r.h / 2.f };
	collider->polygon.vertices[3] = (vec2) { -r.w / 2.f,  r.h / 2.f };
	for (u32 i = 0; i < collider->polygon.vert_count; i++) {
		vec2 old = collider->polygon.vertices[i];
		collider->polygon.vertices[i].x = r.x + r.w / 2.f + (cosf(theta) * old.x - sinf(theta) * old.y);
		collider->polygon.vertices[i].y = r.y + r.h / 2.f + (sinf(theta) * old.x + cosf(theta) * old.y);
	}
	collider->center_pos = (vec2) { r.x + r.w / 2.f, r.y + r.h / 2.f };
	return collider;
}

P2D_Collider* P2D_ColliderAllocCircle(M_Arena* arena, vec2 c, f32 r) {
	P2D_Collider* collider = arena_alloc(arena, sizeof(P2D_Collider));
	collider->type = ColliderType_Circle;
	collider->circle.radius = r;
	collider->center_pos = c;
	return collider;
}

void P2D_ColliderMoveTo(P2D_Collider* collider, vec2 new_pos) {
	switch (collider->type) {
		case ColliderType_Polygon: {
			for (u32 i = 0; i < collider->polygon.vert_count; i++) {
				collider->polygon.vertices[i] = vec2_add(new_pos, vec2_sub(collider->polygon.vertices[i], collider->center_pos));
			}
		} break;
	}
	collider->center_pos = new_pos;
}
