#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in float iteration;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 normalMatrix;

uniform float windTime;
uniform float windStrength;
uniform vec3 windDirection;

out vec3 fragPos;
out vec3 fragNormal;

void main() {
    vec3 displacedPos = position;

    // Only branches sway (iteration >= 2)
    if (iteration >= 1.5) {
        // Height factor for more natural motion (higher = more sway)
        float height = position.z;
        float heightFactor = clamp(height * 20.0, 0.0, 1.0);

        // Multiple wave frequencies for organic motion
        float wave1 = sin(windTime * 1.5 + position.x * 3.0);
        float wave2 = sin(windTime * 2.3 + position.y * 2.0);
        float wave3 = cos(windTime * 1.8);

        // Combine waves - gentle but visible
        float sway = (wave1 * 0.08 + wave2 * 0.06 + wave3 * 0.04)
                     * windStrength * heightFactor;

        // Apply in wind direction
        vec3 displacement = normalize(windDirection) * sway;
        displacedPos += displacement;
    }

    vec4 worldPos = model * vec4(displacedPos, 1.0);
    fragPos = worldPos.xyz;
    fragNormal = normalize(normalMatrix * normal);

    gl_Position = proj * view * worldPos;
}
