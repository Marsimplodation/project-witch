#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;


layout(binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 projection;
} camera;

layout(push_constant) uniform PushConstants {
    uint startModelIndex;    // Current draw call index
};

layout(binding = 1) uniform ModelBuffer {
    mat4 models[10000];
};

// Pseudo-random number generator
float random(vec2 seed) {
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
}


void main() {
    uint index = startModelIndex + gl_InstanceIndex;
    vec4 worldPos= models[index] * vec4(inPosition, 1.0);
    gl_Position = camera.projection *camera.view * worldPos;
    vec3 cameraWorldPos = -transpose(mat3(camera.view)) * camera.view[3].xyz;
    float r = random(vec2(float(index), 0.0)); // Red
    float g = random(vec2(float(index), 1.0)); // Green
    float b = random(vec2(float(index), 2.0)); // Blue

    // Combine into a color
    fragColor = vec3(r, g, b);
}

