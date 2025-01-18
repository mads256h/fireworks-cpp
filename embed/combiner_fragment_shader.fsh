#version 420 core

in vec2 uv;

uniform sampler2D lines_frame_buffer;

out vec4 fragment;

void main() {
    vec4 color = texture(lines_frame_buffer, vec2(uv.x, 1.0 - uv.y));

    fragment = color;
}
