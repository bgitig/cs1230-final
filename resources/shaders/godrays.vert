#version 330 core
layout (location = 0) in vec3 position;
layout (location = 0) in vec2 vuv;

uniform mat4 mvp;
out vec2 uv;

void main() {
    uv = vuv;
    gl_Position = mvp * vec4(position, 1.0);
}
