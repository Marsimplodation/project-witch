#version 450
// Enable the use of nonuniformEXT(index) to access bindless textures
#extension GL_EXT_nonuniform_qualifier : enable

#include "includes/light.h"
layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint texIdx;
layout(location = 4) in vec3 fragView;

layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];
layout(binding = 3) uniform LightBuffer {
    DirectionLight light;
};


float calculateShadowAtCascade(int cascadeIndex) {
    vec4 fragPosLightSpace = bias * light.projections[cascadeIndex] * light.views[cascadeIndex] * vec4(fragPosition, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Check if the fragment is outside the shadow map
    if (projCoords.z > 1.0)
        return 0.0; // No shadow outside the cascade

    // Smoother bias calculation
    float bias = max(0.0005 * tan(acos(dot(fragNormal, light.direction.xyz))), 0.002);

    // Larger 5x5 kernel for smoother shadow filtering
    float weights[5][5] = float[5][5](
        float[5](0.002216, 0.01774, 0.0808, 0.01774, 0.002216),
        float[5](0.01774, 0.1331, 0.6150, 0.1331, 0.01774),
        float[5](0.0808, 0.6150, 2.0970, 0.6150, 0.0808),
        float[5](0.01774, 0.1331, 0.6150, 0.1331, 0.01774),
        float[5](0.002216, 0.01774, 0.0808, 0.01774, 0.002216)
    );

    float shadow = 0.0;
    float totalWeight = 0.0;

    // Perform PCF with 5x5 kernel for smoother shadows
    for (int i = -2; i <= 2; ++i) {
        for (int j = -2; j <= 2; ++j) {
            vec2 coords = projCoords.xy + (vec2(i, j) / SHADOW_MAP_RES[cascadeIndex]);
            float sampleShadow = texture(texSamplers[nonuniformEXT(cascadeIndex)], coords.xy).r < projCoords.z - bias ? 1.0 : 0.0;
            shadow += sampleShadow * weights[i + 2][j + 2];
            totalWeight += weights[i + 2][j + 2];
        }
    }

    return shadow / totalWeight;
}

float ShadowCalculation() {
    // Initialize variables
    int primaryCascade = SHADOW_CASCADES - 1;
    int secondaryCascade = SHADOW_CASCADES - 1;

    // Find the two closest cascades
    for (int i = 0; i < SHADOW_CASCADES; i++) {
        if (abs(fragView.z) <= light.splitDepths[i]) {
            primaryCascade = i;
            secondaryCascade = max(0, i - 1); // Use the previous cascade for blending
            break;
        }
    }

    if (primaryCascade == -1) {
        return 0.0; // No shadow if fragment is outside all cascades
    }

    // Compute blending factor between the two cascades
    float depthRatio = (abs(fragView.z) - light.splitDepths[secondaryCascade]) / 
                       (light.splitDepths[primaryCascade] - light.splitDepths[secondaryCascade]);
    depthRatio = clamp(depthRatio, 0.0, 1.0);

    // Calculate shadows for both cascades
    float shadowPrimary = calculateShadowAtCascade(primaryCascade);
    float shadowSecondary = calculateShadowAtCascade(secondaryCascade);
    float shadow = mix(shadowSecondary, shadowPrimary, depthRatio);
    shadow = max(shadow, shadowPrimary);

    // Blend shadows smoothly
    return shadow;
}






void main() {
    // Normalize the fragment normal
    vec3 normal = normalize(fragNormal);

    // Ambient light term (higher for anime style)
    vec3 ambient = light.color.rgb * 0.15f;

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
    if(light.castShadows)celShading = min(celShading, abs((1.0 - ShadowCalculation())));
    

    // Diffuse shading (cel shaded)
    vec3 diffuse = celShading * light.color.rgb ; 


    // Accumulate lighting
    finalColor += texColor.rgb * diffuse;

    // Set output color
    outColor = vec4(finalColor, 1.0);
}

