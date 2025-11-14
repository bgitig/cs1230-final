#version 330 core

in vec3 position;
in vec3 normal;

out vec4 fragColor;

uniform mat4 m_model;

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

uniform int numLights;

void main() {
   fragColor = vec4(0,0,0,1);

   fragColor += m_ka * m_cAmbient;

   for (int i = 0; i < numLights; i++) {
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
            intensity = intensity;;
         }
         else {
            intensity = vec4(0);
         }
      }

      float attn = type == 1 ? 1 : 1/(m_lightFunction[i].x+distance*m_lightFunction[i].y+distance*distance*m_lightFunction[i].z);
      float fatt = min(1.0f, attn);
      vec4 coef = fatt * intensity;

      vec3 N = normalize(normal);
      float diffuse = clamp(dot(N, directionToLight), 0.0f, 1.0f);
      fragColor += coef * m_kd * m_cDiffuse * diffuse;

      vec3 R = reflect(-directionToLight,N);
      vec3 E = normalize(camera_pos.xyz-position);
      float x = clamp(dot(R,E), 0.0f, 1.0f);
      float specular = x <= 0 ? 0 : m_shininess <= 0 ? 0 : pow(x, m_shininess);
      fragColor += coef * m_ks * m_cSpecular * specular;
   }
}
