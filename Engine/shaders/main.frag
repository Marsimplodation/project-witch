#version 450
// Enable the use of nonuniformEXT(index) to access bindless textures
#extension GL_EXT_nonuniform_qualifier : enable

#include "includes/light.h"
layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint texIdx;
layout(location = 4) in vec3 cameraPosition;

layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];
layout(binding = 3) uniform LightBuffer {
    DirectionLight light;
};


float ShadowCalculation()
{
    // Find the correct cascade
    int cascadeIndex = -1;
    mat4 lBias = bias;
    for (int i = 0; i < SHADOW_CASCADES; i++) {
        vec4 posLightSpace = lBias * light.projections[i] * light.views[i] * vec4(fragPosition, 1.0);
        vec3 projCoords = posLightSpace.xyz / posLightSpace.w;

        if (posLightSpace.z <= -light.splitDepths[i]) {
            cascadeIndex = i;
            break;
        }
    }

    if (cascadeIndex == -1) {
        return 0.0; // No shadow if fragment is outside all cascades
    }
    vec4 fragPosLightSpace = lBias * light.projections[cascadeIndex] * light.views[cascadeIndex] * vec4(fragPosition, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    float shadow = 0.0f;
    if(projCoords.z > 1.0)
        return shadow;
    
    float bias = max(0.0005 * tan(acos(dot(fragNormal, light.direction.xyz))), 0.002);

    float weights[3][3] = float[3][3](
        float[3](0.05, 0.1, 0.05),
        float[3](0.1,  0.4, 0.1),
        float[3](0.05, 0.1, 0.05)
    );

    float totalWeight = 0.0;
    
    for(int i = -1; i <= 1; ++i){
        for(int j = -1; j <= 1; ++j){
            vec2 coords = projCoords.xy + vec2(i, j) / (4096.0 * 2);
            float sampleShadow = texture(texSamplers[cascadeIndex], coords.xy).r < projCoords.z - bias ? 1.0 : 0.0;
            shadow += sampleShadow * weights[i + 1][j + 1];
            totalWeight += weights[i + 1][j + 1];
        }
    }

    return shadow / totalWeight;
}


void main() {
    // Normalize the fragment normal
    vec3 normal = normalize(fragNormal);
    vec3 fragViewDir = normalize(fragPosition - cameraPosition);

    // Ambient light term (higher for anime style)
    vec3 ambient = light.color.rgb * 0.15f * light.intensity;

    // Sample texture
    vec4 texColor = vec4(1.0);
    if (texIdx != uint(-1)) {
        // 0 is allocated for the shadowMap
        texColor = texture(texSamplers[nonuniformEXT(texIdx + SHADOW_CASCADES)], fragUV);
    }
    if (texColor.a < 0.3) discard;

    // Start with ambient term
    vec3 finalColor = ambient * texColor.rgb;

    // Add cel-shaded diffuse lighting for each light
    float cosDir = -dot(normal, light.direction.xyz);



    // Hard threshold for cel shading (Anime Style)
    float shadowThreshold = 0.2;  // Adjust for more or less shadow
    float celShading = step(shadowThreshold, cosDir); 
    celShading = min(celShading, (1.0 - ShadowCalculation()));
    

    // Diffuse shading (cel shaded)
    vec3 diffuse = celShading * light.color.rgb * light.intensity; 


    // Accumulate lighting
    finalColor += texColor.rgb * diffuse;

    // Set output color
    outColor = vec4(finalColor, 1.0);
}

