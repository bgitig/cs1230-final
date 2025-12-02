#version 330 core

out vec4 fragColor;

uniform sampler2D sceneTex;
uniform sampler2D occTex;

uniform vec2 lightScreenPos;
uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform int NUM_SAMPLES;

in vec2 uv;

void main() {
    vec3 scene = texture(sceneTex, uv).rgb;

    vec2 delta = (lightScreenPos - uv) * density / float(NUM_SAMPLES);
    vec2 coord = uv;

    float illuminationDecay = 1.0;
    vec3 color = vec3(0.0);

    for (int i = 0; i < NUM_SAMPLES; i++) {
        coord += delta;
        float x = texture(occTex, coord).r;

        x *= illuminationDecay * weight;

        color += vec3(x);
        illuminationDecay *= decay;
    }

    fragColor = vec4(scene + color * exposure, 1.0);
}
