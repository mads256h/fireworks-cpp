#version 460 core

in vec2 uv;

out vec4 fragment;

void main() {
    float alpha = uv.x * 2;
    if (alpha > 1.0) {
        alpha = (alpha - 1.0) * -1.0 + 1.0;
    }

    alpha = pow(alpha, 4);
    vec4 colors = vec4(1.0, 1.0f, 1.0f, alpha);
    fragment = colors;
}