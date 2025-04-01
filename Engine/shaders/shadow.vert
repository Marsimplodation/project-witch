#version 450
#include "includes/light.h"
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 uv;
layout(location = 3) in uint materialIdx;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;
layout(location = 3) out uint texIdx;

layout(push_constant) uniform PushConstants {
    uint lightIdx;    // Current draw call index
};

layout(binding = 1) readonly buffer ModelBuffer {
    mat4 models[10000];
};

layout(binding = 3) readonly buffer LightBuffer {
    DirectionLight light;
};

void main() {
    uint index = gl_InstanceIndex;
    vec4 worldPos= models[index] * vec4(inPosition, 1.0);
    gl_Position = light.projections[lightIdx] * light.views[lightIdx] * worldPos;
    texIdx = materialIdx;
    fragUV = uv;
    fragNormal = normalize(mat3(models[index]) * inNormal);
    fragPosition = vec3(worldPos);
}

