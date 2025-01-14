#version 460 core

in vec2 vertex_position;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4( vertex_position.x, vertex_position.y, 1, 1);
}