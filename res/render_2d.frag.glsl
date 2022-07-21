#version 330 core

in float v_texindex;
in vec2  v_texcoord;
in vec4  v_color;

layout (location = 0) out vec4 f_color;

uniform sampler2D u_tex[8];

void main() {
    switch (int(v_texindex)) {
        case 0: f_color = v_color * texture(u_tex[0], v_texcoord); break;
        case 1: f_color = v_color * texture(u_tex[1], v_texcoord); break;
        case 2: f_color = v_color * texture(u_tex[2], v_texcoord); break;
        case 3: f_color = v_color * texture(u_tex[3], v_texcoord); break;
        case 4: f_color = v_color * texture(u_tex[4], v_texcoord); break;
        case 5: f_color = v_color * texture(u_tex[5], v_texcoord); break;
        case 6: f_color = v_color * texture(u_tex[6], v_texcoord); break;
        case 7: f_color = v_color * texture(u_tex[7], v_texcoord); break;
        default: discard;
    }
}
