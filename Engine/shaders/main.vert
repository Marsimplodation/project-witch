#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;


layout(binding = 0) uniform CameraBuffer {
    mat4 view;
    mat4 projection;
} camera;

layout(binding = 1) uniform ModelBuffer {
    mat4 models[100];
};

void main() {
    vec4 worldPos= models[gl_InstanceIndex] * vec4(inPosition, 1.0);
    gl_Position = camera.projection *camera.view * worldPos;
    vec3 cameraWorldPos = -transpose(mat3(camera.view)) * camera.view[3].xyz;
    fragColor = inColor * 1.0f/length(inPosition - cameraWorldPos);
}

