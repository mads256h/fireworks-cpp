#version 460 core

in vec2 vertex_position;
in vec2 vertex_uv;

out vec2 uv;

uniform mat4 projection;
uniform mat4 model;

void main() {
    uv = vertex_uv;
    gl_Position = projection * model * vec4( vertex_position.xy, 1, 1);
}