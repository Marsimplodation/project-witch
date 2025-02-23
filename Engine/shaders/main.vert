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
layout(location = 4) out vec3 fragView;



layout(binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 projection;
} camera;

layout(push_constant) uniform PushConstants {
    uint startModelIndex;    // Current draw call index
    uint lightIdx;    // Current draw call index
};

layout(binding = 1) uniform ModelBuffer {
    mat4 models[10000];
};

layout(binding = 3) uniform LightBuffer {
    DirectionLight light;
};

// Pseudo-random number generator
float random(vec2 seed) {
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}




void main() {
    uint index = startModelIndex + gl_InstanceIndex;
    vec4 worldPos= models[index] * vec4(inPosition, 1.0);
    gl_Position = camera.projection *camera.view * worldPos;
    vec4 viewM = (camera.view * worldPos);
    fragView = viewM.xyz;
    fragView.z *= -1;
    texIdx = materialIdx;
    fragUV = uv;
    fragNormal = normalize(mat3(models[index]) * inNormal);
    fragPosition = vec3(worldPos);
}

