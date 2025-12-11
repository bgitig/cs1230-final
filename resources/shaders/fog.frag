#version 330 core
uniform sampler2D scene;
uniform sampler2D depth;
uniform sampler2D bloom;
uniform sampler2D godrays;
uniform float near;
uniform float far;

out vec4 fragColor;

in vec2 uv;

float fog_maxdist = 15.0;
float fog_mindist = 0.1;
vec4  fog_color = vec4(0.2, 0.2, 0.2, 0.4);

void main()
{   
    // fragColor = texture(scene, uv);
    vec3 scene = texture(scene, uv).rgb;
    vec3 bloom = texture(bloom, uv).rgb;
    vec3 rays = texture(godrays, uv).rgb;
    vec3 hdr = scene + bloom + rays;

    const float gamma = 2.2;
    vec3 color = vec3(1.0) - exp(-hdr * 0.5);
    vec3 tonemapped = pow(color, vec3(1.0 / gamma));
    vec4 result = vec4(tonemapped, 1.0);

    fragColor = result;
    // float dist = texture(depth, uv).r;
    // float z_ndc = dist * 2.0 - 1.0;
    // float view_z = (2.0 * near * far) / (far + near - z_ndc * (far - near));
    // float fog_factor = (fog_maxdist - view_z) / (fog_maxdist - fog_mindist);
    // fog_factor = clamp(fog_factor, 0.0, 1.0);

    // fragColor = mix(fog_color, result, fog_factor);
}
