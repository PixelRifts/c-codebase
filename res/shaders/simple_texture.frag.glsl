#version 330 core

layout (location=0) out vec4 f_color;

in vec2 v_tex_coord;

uniform sampler2D tex;

layout (std140) uniform TESTBUFFER {
	float val;
	vec4 color;
};

void main() {
	f_color = color * texture(tex, v_tex_coord);
}
