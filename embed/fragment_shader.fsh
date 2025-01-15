#version 460 core

in vec2 uv;
in vec3 color;
in float start_width;
in float end_width;

out vec4 fragment;

void main() {
    float alpha = uv.x * 2;
    if (alpha > 1.0) {
        alpha = (alpha - 1.0) * -1.0 + 1.0;
    }

    alpha = pow(alpha, mix(start_width, end_width, uv.y) / 50.0);
    vec4 colors = vec4(color, alpha);
    fragment = colors;
}