#version 330 core

in vec3 fragPos;
in vec3 fragNormal;

uniform vec4 cAmbient;
uniform vec4 cDiffuse;
uniform vec4 cSpecular;
uniform float shininess;
uniform vec3 lightDir;
uniform vec3 cameraPos;

out vec4 fragColor;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 lightDirection = normalize(lightDir);

    // Ambient
    vec3 ambient = cAmbient.rgb;

    // Diffuse
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = diff * cDiffuse.rgb;

    // Specular
    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = spec * cSpecular.rgb;

    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0);
}
