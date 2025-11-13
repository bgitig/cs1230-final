#version 330 core

in vec3 position;
in vec3 normal;

out vec4 fragColor;

uniform float m_ka;
uniform float m_kd;
uniform float m_ks;

uniform float m_shininess;
uniform vec4 m_cAmbient;
uniform vec4 m_cDiffuse;
uniform vec4 m_cSpecular;
uniform vec4 m_cReflective;

uniform int m_lightType[8];
uniform vec4 m_lightColor[8];
uniform vec3 m_lightFunction[8];
uniform vec4 m_lightPos[8];
uniform vec4 m_lightDir[8];
uniform float m_lightPenumbra[8];
uniform float m_lightAngle[8];
uniform float directionToLight[8];

uniform vec4 camera_pos;

uniform float white;

void main() {
   fragColor = vec4(m_ka * m_cAmbient);

   for (int i = 0; i < 8; i++) {
      vec4 intensity = m_lightColor[i];

      int type = m_lightType[i];
      vec3 lightPos = vec3(m_lightPos[i]);
      vec3 directionToLight = type == 1 ? normalize(vec3(-m_lightDir[i])) : normalize(vec3(lightPos)-position);
      float distance = length(lightPos-position);

      // spot
      if (type == 2) {
         float theta_o = m_lightAngle[i];
         float theta_i = theta_o - m_lightPenumbra[i];
         float x = acos(clamp(dot(directionToLight, normalize(vec3(-m_lightDir[i]))), -1.0f, 1.0f));
         if (x > theta_i && x <= theta_o) {
            float falloff = (x-theta_i)/(theta_o-theta_i);
            float falloff2 = -2*pow(falloff,3)+3*pow(falloff,2);
            intensity *= (1-falloff2);
         }
         else if (x < theta_i) {
            continue;
         }
         else {
            intensity = vec4(0);
         }
      }

      float attn = type == 1 ? 1 : 1/(m_lightFunction[i].x+distance*m_lightFunction[i].y+distance*distance*m_lightFunction[i].z);
      float fatt = min(1.0f, attn);
      vec4 coef = fatt * intensity;

      vec3 L = normalize(lightPos-position);
      vec3 N = normalize(normal);
      float diffuse = m_kd * clamp(dot(N,L), 0.0f, 1.0f);
      fragColor += coef * diffuse * m_cDiffuse;

      vec3 R = normalize(reflect(-L,N));
      vec3 E = normalize(camera_pos.xyz-position);
      float specular = m_shininess == 0 ? m_ks : m_ks * pow(clamp(dot(R,E), 0.0f, 1.0f), m_shininess);
      fragColor += coef * specular * m_cSpecular;
   }
   fragColor = vec4(m_ka);
}
