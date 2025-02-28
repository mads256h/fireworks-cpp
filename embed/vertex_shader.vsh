#version 420 core

in vec2 vertex_position;
in vec2 vertex_uv;
in mat4 model_matrix;// Instanced
in vec3 model_color;// Instanced
in vec3 vertex_width;// Instanced

out vec2 uv;
out vec3 color;
out float start_width;
out float end_width;
out float max_width;

uniform mat4 projection_matrix;

void main() {
    uv = vertex_uv;
    color = model_color;
    start_width = vertex_width.x;
    end_width = vertex_width.y;
    max_width = vertex_width.z;
    gl_Position = projection_matrix * model_matrix * vec4(vertex_position.xy, 0, 1);
}