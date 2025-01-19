#version 420 core

#define PI 3.1415926535897932384626433832795

uniform vec3 background_color;
uniform float star_density;

out vec4 fragment;

float map(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

uint wang_hash(uint seed)
{
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9;
    seed = seed ^ (seed >> 4u);
    seed *= 0x27d4eb2du;
    seed = seed ^ (seed >> 15u);
    return seed;
}


float rand(vec2 co) {
    uint a = uint(co.x);
    uint b = uint(co.y);
    // Szudzik's function
    uint value_to_hash = a >= b ? a * a + a + b : a + b * b;

    uint hash = wang_hash(value_to_hash);

    return float(hash) / 4294967296.0;
}

float randCutoff(vec2 co, float cutoff) {
    float value = rand(co);
    if (value < cutoff) {
        return 0.0;
    }

    return map(value, cutoff, 1.0, 0.0, 1.0);
}

float perceived_brightness(vec3 color) {
    float r = pow(color.r, 2.218);
    float g = pow(color.g, 2.218);
    float b = pow(color.b, 2.218);

    float luminance = r * 0.2126 + g * 0.7156 + b * 0.0722;

    return pow(luminance, 0.68);
}

void main() {
    vec3 color = vec3(randCutoff(gl_FragCoord.xy,-(star_density*star_density) + 1.0));

    if (perceived_brightness(color) < perceived_brightness(background_color)) {
        color = background_color;
    }

    fragment = vec4(color, 1);
}