#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 uvfrag;

// Task 8: Add a sampler2D uniform
uniform sampler2D sampler;

// Task 29: Add a bool on whether or not to filter the texture
uniform bool toggle;

out vec4 fragColor;

void main()
{
    fragColor = vec4(1);
    // Task 17: Set fragColor using the sampler2D at the UV coordinate
    fragColor = texture(sampler, uvfrag);

    // Task 33: Invert fragColor's r, g, and b color channels if your bool is true
    if (toggle) fragColor = vec4(1-fragColor.x,1-fragColor.y,1-fragColor.z,1);

}
