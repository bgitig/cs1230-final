#version 330 core

layout (location = 0) in vec3 obj_position;
layout (location = 1) in vec3 obj_normal;

out vec3 position;
// out vec3 normal;

uniform mat4 m_mvp;
// uniform mat4 ictm;
// uniform mat4 m_view;
// uniform mat4 m_proj;
uniform mat4 m_model;

void main() {
   // normal = ictm*normalize(obj_position);
   // gl_Position = m_proj * m_view * m_model * vec4(obj_position, 1.0f);
   // vec4 world_pos = m_model * vec4(obj_position, 1.0f);
   // position - world_pos.xyz;

   // position = obj_position;
   // gl_Position = vec4(position,1.0f);

   position = vec3(m_model * vec4(obj_position,1.0f));
   gl_Position = m_mvp * vec4(obj_position,1.0f);
}
