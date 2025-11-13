#version 330 core

layout (location = 0) in vec3 obj_position;
layout (location = 1) in vec3 obj_normal;

out vec3 position;
out vec3 normal;

uniform mat4 m_mvp;
uniform mat3 ictm;
uniform mat4 m_model;

void main() {
   position = vec3(m_model * vec4(obj_position,1.0f));
   normal = ictm * normalize(obj_normal);

   gl_Position = m_mvp * vec4(obj_position, 1.0f);
}
