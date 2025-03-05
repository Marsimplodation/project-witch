#version 450
// Enable the use of nonuniformEXT(index) to access bindless textures
#extension GL_EXT_nonuniform_qualifier : enable
#include "includes/light.h"
layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint texIdx;

layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];

void main() {
    // Normalize the fragment normal
    vec3 normal = normalize(fragNormal);
    // Sample texture
    //skip alpha cutout in here for now
    /*vec4 texColor = vec4(1.0);
    if (texIdx != uint(-1)) {
        // 0 is allocated for the shadowMap
        texColor = texture(texSamplers[nonuniformEXT(texIdx)], fragUV);
    }
    if (texColor.a < 0.3) discard;*/

}

