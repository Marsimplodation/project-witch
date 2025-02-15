#version 450
// Enable the use of nonuniformEXT(index) to access bindless textures
#extension GL_EXT_nonuniform_qualifier : enable

#include "includes/light.h"
layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint texIdx;
layout(location = 4) in vec3 cameraPosition;
layout(location = 5) in vec4 positionInLight;

layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];
layout(binding = 3) uniform LightBuffer {
    DirectionLight light;
};

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    float shadow = 0.0f;
    if(projCoords.z > 1.0)
        return shadow;
        
    float bias = max(0.05 * (1.0 - dot(fragNormal, light.direction.xyz)), 0.005);
    shadow += texture(texSamplers[0], projCoords.xy).r < projCoords.z - bias ? 1.0 : 0.0;
    return shadow;
}

void main() {
    // Normalize the fragment normal
    vec3 normal = normalize(fragNormal);
    vec3 fragViewDir = normalize(fragPosition - cameraPosition);

    // Ambient light term (higher for anime style)
    vec3 ambient = vec3(0.2);

    // Sample texture
    vec4 texColor = vec4(1.0);
    if (texIdx != uint(-1)) {
        // 0 is allocated for the shadowMap
        texColor = texture(texSamplers[nonuniformEXT(texIdx + 1)], fragUV);
    }
    if (texColor.a < 0.3) discard;

    // Start with ambient term
    vec3 finalColor = ambient * texColor.rgb;

    // Add cel-shaded diffuse lighting for each light
    float cosDir = -dot(normal, light.direction.xyz);



    // Hard threshold for cel shading (Anime Style)
    float shadowThreshold = 0.5;  // Adjust for more or less shadow
    float celShading = step(shadowThreshold, cosDir); 
    celShading = min(celShading, (1.0 - ShadowCalculation(positionInLight)));
    

    // Diffuse shading (cel shaded)
    vec3 diffuse = celShading * light.color.rgb * light.intensity; 


    // Accumulate lighting
    finalColor += texColor.rgb * diffuse;

    // Set output color
    outColor = vec4(finalColor, 1.0);
}

