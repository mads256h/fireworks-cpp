#version 420 core

in vec2 vertex_position;
in vec2 vertex_uv;

uniform mat4 projection_matrix;

out vec2 uv;

void main() {
    uv = vertex_uv;
    gl_Position = projection_matrix * vec4(vertex_position.xy, 0, 1);
}