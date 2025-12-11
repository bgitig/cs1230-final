#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace;
in vec2 TexCoords;  // NEW: Add this input for UV coordinates

struct Light {
    int type;
    vec4 color;
    vec3 function;
    vec4 pos;
    vec4 dir;
    float penumbra;
    float angle;
};

uniform vec3 cameraPos;
uniform int numLights;
uniform Light lights[20];

uniform float m_ka;
uniform float m_kd;
uniform float m_ks;
uniform vec4 m_cAmbient;
uniform vec4 m_cDiffuse;
uniform vec4 m_cSpecular;
uniform float shininess;

// NEW: Texture uniforms
uniform sampler2D flagTexture;
uniform bool useTexture;

// shadows!!
uniform sampler2D shadowMap;

// yet again, shadows
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform range
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    // check if we're outside the shadow map bounds
    if(projCoords.x < 0.0 || projCoords.x > 1.0 ||
       projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;

    // get depth from light
    float currentDepth = projCoords.z;
    // bias
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // an "attempt" at PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y){
            // sample shadow map
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            // compare depths
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0; // average the samples

    return shadow;
}

vec3 calculatePointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 baseColor) {
    vec3 lightDir = normalize(light.pos.xyz - fragPos);

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    // f-att
    float distance = length(light.pos.xyz - fragPos);
    float attenuation = 1.0 / (light.function.x + light.function.y * distance + light.function.z * (distance * distance));

    // accumulation - MODIFIED to use baseColor
    vec3 ambient = m_ka * m_cAmbient.rgb * light.color.rgb;
    vec3 diffuse = m_kd * baseColor.rgb * diff * light.color.rgb;  // Use baseColor
    vec3 specular = m_ks * m_cSpecular.rgb * spec * light.color.rgb;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec3 calculateDirectionalLight(Light light, vec3 normal, vec3 viewDir, float shadow, vec4 baseColor) {
    vec3 lightDir = normalize(-light.dir.xyz);

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

    // accumulation - MODIFIED to use baseColor
    vec3 ambient = m_ka * m_cAmbient.rgb * light.color.rgb;
    vec3 diffuse = m_kd * baseColor.rgb * diff * light.color.rgb;  // Use baseColor
    vec3 specular = m_ks * m_cSpecular.rgb * spec * light.color.rgb;

    // shadow time!
    diffuse *= (1.0 - shadow);
    specular *= (1.0 - shadow);
    // no ambient effect

    return ambient + diffuse + specular;
}

vec3 calculateSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 baseColor) {
    vec3 lightDir = normalize(light.pos.xyz - fragPos);
    vec3 spotDir = normalize(-light.dir.xyz);
    float theta = dot(lightDir, spotDir);

    float outerCutoff = cos(light.angle);
    float innerCutoff = cos(light.angle * 0.8);

    float intensity = 0.0;
    if(theta > outerCutoff) {
        if(theta > innerCutoff) {
            intensity = 1.0;
        } else {
            intensity = (theta - outerCutoff) / (innerCutoff - outerCutoff);
        }
    }

    if(intensity > 0.0) {
        // diffuse
        float diff = max(dot(normal, lightDir), 0.0);

        // specular
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);

        // f-att
        float distance = length(light.pos.xyz - fragPos);
        float attenuation = 1.0 / (light.function.x + light.function.y * distance +
                                  light.function.z * (distance * distance));

        // accumulate - MODIFIED to use baseColor
        vec3 ambient = m_ka * m_cAmbient.rgb * light.color.rgb;
        vec3 diffuse = m_kd * baseColor.rgb * diff * light.color.rgb;  // Use baseColor
        vec3 specular = m_ks * m_cSpecular.rgb * spec * light.color.rgb;

        ambient *= attenuation * intensity;
        diffuse *= attenuation * intensity;
        specular *= attenuation * intensity;

        return ambient + diffuse + specular;
    }

    return vec3(0.0);
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(cameraPos - FragPos);


    // NEW: Get base color from texture or uniform
    vec4 baseColor;
    if (useTexture) {
        baseColor = texture(flagTexture, TexCoords);
    } else {
        baseColor = m_cDiffuse;
    }

    // Simple lighting for testing
    vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 color = baseColor.rgb * diff + vec3(0.1, 0.1, 0.1); // Use baseColor with ambient

    // Use proper lighting if lights exist
    if (numLights > 0) {
        color = vec3(0.0);
        for (int i = 0; i < numLights && i < 8; i++) {
            if (lights[i].type == 1) { // directional
                float shadow = ShadowCalculation(FragPosLightSpace, norm, normalize(-lights[i].dir.xyz));
                color += calculateDirectionalLight(lights[i], norm, viewDir, shadow, baseColor);
            } else if (lights[i].type == 0) { // point
                color += calculatePointLight(lights[i], norm, FragPos, viewDir, baseColor);
            } else if (lights[i].type == 2) { // spot
                color += calculateSpotLight(lights[i], norm, FragPos, viewDir, baseColor);
            }
        }
    }

    FragColor = vec4(color, baseColor.a);
}
