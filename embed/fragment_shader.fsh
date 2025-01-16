#version 460 core

in vec2 uv;
in vec3 color;
in float start_width;
in float end_width;

out vec4 fragment;

float map(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main() {
    float width = mix(start_width, end_width, uv.y);
    float clamp_value = 1.0 - (width / 50.0);
    float value = uv.x * 2;
    if (value > 1.0) {
        value = (value - 1.0) * -1.0 + 1.0;
    }

    value = clamp(value, clamp_value, 1.0);
    value = map(value, clamp_value, 1.0, 0.0, 1.0);

    float alpha = value;
    vec4 colors = vec4(color, alpha);
    fragment = colors;
}