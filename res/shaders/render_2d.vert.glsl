#version 330 core

layout (location = 0) in vec2  a_pos;
layout (location = 1) in vec2  a_texcoord;
layout (location = 2) in float a_texindex;
layout (location = 3) in vec4  a_color;

out float v_texindex;
out vec2  v_texcoord;
out vec4  v_color;


layout (std140) uniform ActualConstants {
	mat4 u_projection;
};

void main() {
    gl_Position = u_projection * vec4(a_pos, 0.0, 1.0);
    v_texindex = a_texindex;
    v_texcoord = a_texcoord;
    v_color = a_color;
}
