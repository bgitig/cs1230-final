#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat3 normalMatrix;

// Wind parameters
uniform float windTime;
uniform float windStrength;
uniform vec3 windDirection;

out vec3 fragPos;
out vec3 fragNormal;

void main() {
    // Calculate wind displacement based on height
    // After rotation, Z is the "up" direction in terrain space
    float height = position.z;

    // MUCH stronger height factor
    float heightFactor = clamp(height * 2.0, 0.0, 1.0);  // Doubled
    heightFactor = heightFactor * heightFactor;  // Quadratic

    // MUCH bigger waves
    float wave1 = sin(windTime * 1.5 + position.x * 0.5) * 0.1;  // 10x bigger!
    float wave2 = sin(windTime * 2.3 + position.y * 0.3) * 0.08;  // 10x bigger!
    float wave3 = cos(windTime * 1.8) * 0.05;  // 10x bigger!

    // Combine waves with MUCH stronger effect
    float sway = (wave1 + wave2 + wave3) * windStrength * heightFactor * 5.0;  // 5x multiplier!

    // Apply displacement - make it VERY obvious
    vec3 displacement = vec3(sway, sway * 0.5, 0.0);  // Displace in X and Y
    vec3 displacedPos = position + displacement;

    // Transform to world space
    vec4 worldPos = model * vec4(displacedPos, 1.0);
    fragPos = worldPos.xyz;

    // Transform normal
    fragNormal = normalize(normalMatrix * normal);

    gl_Position = proj * view * worldPos;
}
