#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;

uniform mat4 m_mvp;
uniform mat4 ictm;
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec4 FragPosLightSpace;
out vec2 TexCoords;

void main() {
    gl_Position = m_mvp * vec4(position, 1.0);
    FragPos = vec3(model * vec4(position, 1.0));
    Normal = mat3(ictm) * normal;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    TexCoords = texCoords;  // NEW: Pass through UV coordinates
}
