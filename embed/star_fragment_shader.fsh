#version 460 core

#define PI 3.1415926535897932384626433832795

uniform vec3 background_color;
uniform float star_density;

out vec4 fragment;

float map(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}


float rand(vec2 co){
    return fract(sin(mod(dot(co, vec2(12.9898, 48.233)), PI)) * 43758.5453);
}

float randCutoff(vec2 co, float cutoff) {
    float value = rand(co);
    if (value < cutoff) {
        return 0.0;
    }

    return map(value, cutoff, 1.0, 0.0, 1.0);
}

void main() {
    vec3 color = vec3(randCutoff(gl_FragCoord.xy,star_density));

    if (length(color) < length(background_color)) {
        color = background_color;
    }

    fragment = vec4(color, 1);
}