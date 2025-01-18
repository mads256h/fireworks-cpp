#version 420 core

in vec2 vertex_position;

uniform mat4 projection_matrix;

void main() {
    gl_Position = projection_matrix * vec4(vertex_position.xy, 0, 1);
}