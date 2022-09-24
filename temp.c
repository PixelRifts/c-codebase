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
	
	darray_free(vec2, &simplex);
	
	return (P2D_Collision) {
		.is_colliding = colliding,
		.resolution = resolution,
	};
}