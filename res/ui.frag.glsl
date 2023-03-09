#version 330 core

in vec2  v_boxsize;
in vec2  v_boxcenter;
in float v_texindex;
in vec2  v_texcoord;
in vec4  v_color;
in vec4  v_clip_quad;
in vec3  v_rounding_softness_and_edge_size;
in vec2  v_sampling_loc;

layout (location = 0) out vec4 f_color;

uniform sampler2D u_tex[8];

// Thanks a LOT Ryan :D. This is so much nicer rounding than I was using before
// +Edges is a nice feature

float RoundedRectSDF(vec2 sample_pos, vec2 rect_center, vec2 rect_half_size, float r) {
	vec2 d2 = (abs(rect_center - sample_pos) - rect_half_size + vec2(r, r));
	return min(max(d2.x, d2.y), 0.0) + length(max(d2, 0.0)) - r;
}

bool rect_contains_point(vec4 a, vec2 p) {
    return a.x <= p.x && a.y <= p.y && a.x + a.z >= p.x && a.y + a.w >= p.y;
}

void main() {
	vec2 half_boxsize = v_boxsize / 2.0;
	if (!rect_contains_point(v_clip_quad, v_sampling_loc)) discard;
	
	// Texture
	switch (int(v_texindex)) {
		case 0: f_color = texture(u_tex[0], v_texcoord); break;
		case 1: f_color = texture(u_tex[1], v_texcoord); break;
		case 2: f_color = texture(u_tex[2], v_texcoord); break;
		case 3: f_color = texture(u_tex[3], v_texcoord); break;
		case 4: f_color = texture(u_tex[4], v_texcoord); break;
		case 5: f_color = texture(u_tex[5], v_texcoord); break;
		case 6: f_color = texture(u_tex[6], v_texcoord); break;
		case 7: f_color = texture(u_tex[7], v_texcoord); break;
		default: discard;
	}
	
	
	// Color
	f_color *= v_color;
	
	
	// Rounding
	
	float softness = v_rounding_softness_and_edge_size.y;
	vec2 softness_padding = vec2(max(0, softness*2-1), max(0, softness*2-1));
	
	float dist = RoundedRectSDF(v_sampling_loc, v_boxcenter,
								half_boxsize-softness_padding,
								v_rounding_softness_and_edge_size.x);
	float sdf_factor = 1.0 - smoothstep(0, 2*softness, dist);
	
	f_color *= sdf_factor;
	
	
	// Edges
	float edge_size = v_rounding_softness_and_edge_size.z;
	if (edge_size != 0) {
		vec2 interior_half_size = half_boxsize - vec2(edge_size);
		
		float interior_radius_reduce_f = min(interior_half_size.x/half_boxsize.x,
											 interior_half_size.y/half_boxsize.y);
		
		float interior_corner_radius = (v_rounding_softness_and_edge_size.x
										* interior_radius_reduce_f * interior_radius_reduce_f);
		
		float inside_d = RoundedRectSDF(v_sampling_loc, v_boxcenter,
										interior_half_size-softness_padding,
										interior_corner_radius);
		f_color *= smoothstep(0, 2*softness, inside_d);
	}
	
	
	//f_color = vec4(edge_size / 100.f, 0.2, 0.3, 1.0);
}
