#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint texIdx;
layout(location = 4) in vec3 cameraPosition;

layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];

struct Light {
    vec3 pos;
    vec3 color;
    float intensity;
};

void main() {
    // Normalize the fragment normal
    vec3 normal = normalize(fragNormal);

    // Ambient light term
    vec3 ambient = vec3(0.05);

    // Initialize lights
    Light lights[1];
    lights[0].pos = vec3(2.0, 3.0, 0.0);
    lights[0].color = vec3(1.0, 1.0, 1.0);
    lights[0].intensity = 1.0;

    // Sample texture
    vec4 texColor = vec4(1.0);
    if (texIdx != uint(-1)) {
        texColor = texture(texSamplers[texIdx], fragUV);
    }

    // Start with ambient term
    vec3 finalColor = ambient * texColor.rgb;

    // Add diffuse lighting for each light
    for (int i = 0; i < 1; ++i) {
        vec3 lightPosition = lights[i].pos;
        vec3 lightDirection = normalize(lightPosition - fragPosition);

        // Diffuse shading
        float cosDir = max(0.0, dot(normal, lightDirection));
        vec3 diffuse = cosDir * lights[i].color * lights[i].intensity;

        // Accumulate lighting
        finalColor += texColor.rgb * diffuse;
    }

    // Set output color
    outColor = vec4(finalColor, 1.0);
}

