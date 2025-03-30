#version 450
// Enable the use of nonuniformEXT(index) to access bindless textures
#extension GL_EXT_nonuniform_qualifier : enable

#include "includes/light.h"
#include "includes/materials.h"
layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint matIdx;
layout(location = 4) in vec3 fragView;
layout(location = 5) in vec3 cameraPos;

layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D texSamplers[MAX_TEXTURES];
layout(binding = 3) uniform LightBuffer {
    DirectionLight light;
};
layout(binding = 5) uniform PointLightBuffer {
    PointLight pointLights[100];
};

layout(binding = 4) uniform MaterialBuffer {
    Material materials[MAX_MATERIALS];
};

vec3 normal = vec3(1.0f);

void loadNormalMap(uint texIdx)  {
    vec4 normalColor = texture(texSamplers[nonuniformEXT(texIdx)], fragUV);
    vec3 textureNormal = vec3(2.0f * normalColor.x, 2.0f * normalColor.y, 2.0f * normalColor.z);
    textureNormal -= vec3(1.0f);
    vec3 arbitrary = abs(normal.z) < 0.99 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(normal.xyz, arbitrary));
    vec3 bitangent = cross(normal.xyz, tangent);

    normal.xyz = textureNormal.x * tangent + textureNormal.y * bitangent + textureNormal.z * normal.xyz;
    normal.xyz = normalize(normal.xyz);
}

vec3 getPointLightsColor() {
    vec3 ret = vec3(0.0);
    for(int i = 0; i < 100; ++i) {
        PointLight pLight = pointLights[i];
        vec3 pos = vec3(pLight.position);
        vec3 color = vec3(pLight.color);
        float radius = pLight.position[3];
        float intensity = pLight.color[3];
        if(radius == 0.0f) continue;

        float distance = length(pos - fragPosition);
        distance -= radius;

        // Compute attenuation
        float attenuation = 1.0 / ((distance * distance));
        attenuation = min(1.0f, attenuation);
        float finalIntensity = intensity * attenuation;

        ret += color * finalIntensity;
    }
    return ret;
}

float calculateShadowAtCascade(int cascadeIndex) {
    vec4 fragPosLightSpace = bias * light.projections[cascadeIndex] * light.views[cascadeIndex] * vec4(fragPosition, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Check if the fragment is outside the shadow map
    if (projCoords.z > 1.0)
        return 0.0; // No shadow outside the cascade

    // Smoother bias calculation
    float bias = max(0.0005 * tan(acos(dot(normal, light.direction.xyz))), 0.005) * 0.1f;

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



bool outLine() {
    vec3 viewDir = normalize(cameraPos - fragPosition);
    float distance = length(cameraPos - fragPosition);
    float minDist = 1.0f;
    float maxDist = 1000.0f;
    float distanceFactor = clamp(1.0 - (distance - minDist) / (maxDist - minDist), 0.2, 1.0);

    // Fresnel term: Highlight edges based on the angle between the normal and view direction
    float fresnelTerm = 1.0 - abs(dot(normal, viewDir));
    fresnelTerm = pow(fresnelTerm, 3.0) * distanceFactor; // Sharpen Fresnel effect

    // Screen-space derivative-based edge detection
    vec3 dNormalX = dFdx(normal);
    vec3 dNormalY = dFdy(normal);

    // Measure the rate of change in normals
    float normalChange = length(dNormalX) + length(dNormalY);

    // Skip edge detection if normal change is not significant
    float normalChangeThreshold = 0.05; // Adjust this value based on your mesh
    outColor = vec4(normalChange, 0,0,1);
    float edgeFactor = 0.6* fresnelTerm + 0.4 * normalChange * distanceFactor; // Weighted combination
    if (normalChange < normalChangeThreshold) {
	edgeFactor = 0.0; 
    }

    // Combine Fresnel and derivative edges
    float outlineThreshold = 0.3; // Final outline threshold

    if (edgeFactor > outlineThreshold) {
        // Render outline as black
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return true;
    }
    return false;

}


void main() {
    // Normalize the fragment normal
    normal = normalize(fragNormal);
    if(outLine()) return;
    vec3 ambient = light.color.rgb * 0.15f;

    // Sample texture
    vec4 texColor = materials[matIdx].albedo;
    texColor.a = 1.0f;
    uint texIdx = materials[matIdx].texInfos[DIFFUSE];
    uint nIdx = materials[matIdx].texInfos[NORMAL];
    if (texIdx != uint(-1)) {
        // 0 is allocated for the shadowMap
        texColor = texture(texSamplers[nonuniformEXT(texIdx + SHADOW_CASCADES)], fragUV);
    }
    if (texColor.a < 0.3) discard;
    if (nIdx != uint(-1)) {
        // 0 is allocated for the shadowMap
        //loadNormalMap(nIdx + SHADOW_CASCADES);
    }

    // Start with ambient term
    vec3 finalColor = (ambient + getPointLightsColor())* texColor.rgb;

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

