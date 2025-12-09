#version 330 core
layout(location = 0) in vec3 posVar;
layout(location = 1) in vec3 normVar;
layout(location = 2) in vec3 tangentVar;

out vec3 worldSpacePos;
out vec3 worldSpaceNorm;
out vec3 worldSpaceTangent;
out vec2 texCoord;
out vec3 tangentSpaceLight;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 lightDir;

void main() {
    vec4 worldPos = modelMatrix * vec4(posVar, 1.0);
    worldSpacePos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    vec3 T = normalize(normalMatrix * tangentVar);
    vec3 N = normalize(normalMatrix * normVar);
    vec3 B = normalize(cross(N, T));

    worldSpaceNorm = N;
    worldSpaceTangent = T;

    mat3 TBN_inverse = transpose(mat3(T, B, N));
    tangentSpaceLight = TBN_inverse * normalize(lightDir);

    texCoord = worldSpacePos.xy;

    gl_Position = projMatrix * viewMatrix * worldPos;
}
