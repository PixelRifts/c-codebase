#version 330 core

layout (location = 0) in vec2  a_boxsize;
layout (location = 1) in vec2  a_boxcenter;
layout (location = 2) in vec2  a_uv;
layout (location = 3) in float a_texidx;
layout (location = 4) in vec4  a_color;
layout (location = 5) in vec4  a_clip_quad;
layout (location = 6) in vec3  a_rounding_softness_and_edge_size;

out vec2  v_boxsize;
out vec2  v_boxcenter;
out float v_texindex;
out vec2  v_texcoord;
out vec4  v_color;
out vec4  v_clip_quad;
out vec3  v_rounding_softness_and_edge_size;
out vec2  v_sampling_loc;

const vec2 vertex_multiplers[] = {
    vec2(-.5, -.5),
    vec2(+.5, -.5),
    vec2(+.5, +.5),
    vec2(-.5, -.5),
    vec2(+.5, +.5),
    vec2(-.5, +.5),
};

uniform mat4 u_projection;

void main() {
    vec2 pos = a_boxcenter + (a_boxsize * vertex_multiplers[gl_VertexID % 6]);
	gl_Position = u_projection * vec4(pos, 0.0, 1.0);
	v_boxsize = a_boxsize;
	v_boxcenter = a_boxcenter;
	v_texindex = a_texidx;
	v_texcoord = a_uv;
	v_color = a_color;
	v_clip_quad = a_clip_quad;
	v_rounding_softness_and_edge_size = a_rounding_softness_and_edge_size;
	v_sampling_loc = pos;
}
