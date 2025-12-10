#version 330 core
in vec3 worldSpacePos;
in vec3 worldSpaceNorm;
in vec3 worldSpaceTangent;
in vec2 texCoord;
in vec3 tangentSpaceLight;

out vec4 fragColor;

uniform vec3 matColor;
uniform int bumpPattern;
uniform float bumpScale;
uniform float bumpVariation;
uniform float bumpSeed;
uniform float bumpStrength;

float hash3D(vec3 p) {
    p = fract(p * 0.3183099 + 0.1 + bumpSeed * 0.1);
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float noise3D(vec3 x) {
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);

    return mix(
        mix(mix(hash3D(i + vec3(0,0,0)), hash3D(i + vec3(1,0,0)), f.x),
            mix(hash3D(i + vec3(0,1,0)), hash3D(i + vec3(1,1,0)), f.x), f.y),
        mix(mix(hash3D(i + vec3(0,0,1)), hash3D(i + vec3(1,0,1)), f.x),
            mix(hash3D(i + vec3(0,1,1)), hash3D(i + vec3(1,1,1)), f.x), f.y),
        f.z);
}

float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 5; i++) {
        value += amplitude * noise3D(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float getRockHeight(vec3 pos) {
    vec3 p = normalize(pos) * bumpScale * 5.0;

    if (bumpVariation > 0.1) {
        vec3 warp = vec3(
            noise3D(p * 0.5),
            noise3D(p * 0.5 + vec3(10.0)),
            noise3D(p * 0.5 + vec3(20.0))
        );
        p += warp * bumpVariation * 0.5;
    }

    float h = 0.0;

    h += fbm(p + vec3(bumpSeed)) * 0.5;

    int extraOctaves = int(bumpVariation * 3.0);
    for (int i = 0; i < extraOctaves; i++) {
        float freq = 3.0 + float(i) * 2.0;
        float amp = 0.3 / (float(i) + 1.0);
        h += noise3D(p * freq) * amp;
    }

    h += noise3D(p * 3.0) * 0.3;

    h += noise3D(p * 8.0) * 0.2;

    if (bumpVariation > 0.5) {
        float turbFreq = 5.0 + bumpVariation * 5.0;
        h += abs(noise3D(p * turbFreq) - 0.5) * 0.3 * bumpVariation;
    }

    if (bumpVariation > 1.0) {
        float ridges = noise3D(p * 4.0 + vec3(bumpSeed * 5.0));
        h += pow(abs(ridges - 0.5), 0.3) * 0.4 * (bumpVariation - 1.0);
    }

    if (bumpVariation > 1.5) {
        h += noise3D(p * 0.7 + vec3(bumpSeed * 10.0)) * 0.5 * (bumpVariation - 1.5);
    }

    return h;
}

void main() {
    float h = getRockHeight(worldSpacePos);
    float epsilon = 0.01 * bumpScale * bumpStrength;

    vec3 posRight = worldSpacePos + worldSpaceTangent * epsilon;
    vec3 bitangent = cross(worldSpaceNorm, worldSpaceTangent);
    vec3 posUp = worldSpacePos + bitangent * epsilon;

    float hRight = getRockHeight(posRight);
    float hUp = getRockHeight(posUp);

    float dHdx = (hRight - h) / epsilon;
    float dHdy = (hUp - h) / epsilon;

    dHdx *= bumpStrength;
    dHdy *= bumpStrength;

    vec3 perturbedNormal = normalize(worldSpaceNorm - dHdx * worldSpaceTangent - dHdy * bitangent);

    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));
    float diffuse = max(dot(perturbedNormal, lightDir), 0.0);

    float lighting = diffuse * 0.7 + 0.3;

    fragColor = vec4(matColor * lighting, 1.0);
}
