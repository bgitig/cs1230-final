#version 330 core

in vec3 position;
// in vec3 normal;

out vec4 fragColor;

// uniform float m_ka;
// uniform float m_kd;
// uniform float m_ks;
// uniform float m_shininess;

// uniform vec4 m_lightPos;
// uniform vec4 camera_pos;

// uniform vec4 m_lightPosArray[8];


void main() {
   fragColor = vec4(1.0);

   // vec3 lightPos = m_lightPosArray[i];

   // vec3 L = normalize(m_lightPos.xyz-position);
   // vec3 N = normalize(normal);
   // float diffuse = m_kd * clamp(dot(N,L), 0.0f, 1.0f);
   // fragColor += diffuse;

   // vec3 R = normalize(reflect(-L,N));
   // vec3 E = normalize(camera_pos.xyz-position);
   // float specular = m_ks * pow(clamp(dot(R,E), 0.0f, 1.0f), m_shininess);
   // fragColor += specular;
}
