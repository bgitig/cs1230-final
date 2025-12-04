
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloom;

void main()
{
    // const float exposure = .5;
    // const float gamma = 2.2;

    vec3 hdrColor = texture(scene, TexCoords).rgb;
    vec3 bloomColor = texture(bloom, TexCoords).rgb;

    // vec3 color = hdrColor + bloomColor;
    // vec3 result = vec3(1.0) - exp(-color * exposure);
    // result = pow(result, vec3(1.0 / gamma));
    // FragColor = vec4(result, 1.0);
    FragColor = vec4(max(hdrColor, bloomColor), 1);

}
