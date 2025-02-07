#version 450
// Enable the use of nonuniformEXT(index) to access bindless textures
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint texIdx;
layout(location = 4) in vec3 cameraPosition;

layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];


//opengl tutorial
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(texSamplers[nonuniformEXT(0)], projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}

void main() {
    // Normalize the fragment normal
    vec3 normal = normalize(fragNormal);
    vec3 fragViewDir = normalize(fragPosition - cameraPosition);

    // Sample texture
    vec4 texColor = vec4(1.0);
    if (texIdx != uint(-1)) {
        // 0 is allocated for the shadowMap
        texColor = texture(texSamplers[nonuniformEXT(texIdx + 1)], fragUV);
    }
    if (texColor.a < 0.3) discard;

}

