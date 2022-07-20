#version 330 core

in vec2 v_uv;

layout (location=0) out vec4 f_color;

uniform sampler2D u_texture;

void main() {
	f_color = texture(u_texture, v_uv);
}
