#version 460 core

in vec2 uv;

out vec4 fragment;

uniform mat4 model;

void main() {
    vec2 colors = uv;
    fragment = vec4(colors, 0.0, 1.0);
}