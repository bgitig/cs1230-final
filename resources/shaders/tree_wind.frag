#version 330 core

in vec3 fragPos;
in vec3 fragNormal;

uniform vec4 cAmbient;
uniform vec4 cDiffuse;
uniform vec4 cSpecular;
uniform float shininess;
uniform vec3 lightDir;
uniform vec3 cameraPos;

out vec4 FragColor;

void main() {
    // Phong lighting
    vec3 ambient = cAmbient.rgb;

    vec3 norm = normalize(fragNormal);
    vec3 lightD = normalize(lightDir);
    float diff = max(dot(norm, lightD), 0.0);
    vec3 diffuse = diff * cDiffuse.rgb;

    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 reflectDir = reflect(-lightD, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * cSpecular.rgb;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
