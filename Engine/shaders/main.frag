#version 450
// Enable the use of nonuniformEXT(index) to access bindless textures
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint texIdx;
layout(location = 4) in vec3 cameraPosition;

layout(location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];


struct Light {
    vec3 direction;
    vec3 color;
    float intensity;
};

void main() {
    // Normalize the fragment normal
    vec3 normal = normalize(fragNormal);
    vec3 fragViewDir = normalize(fragPosition - cameraPosition);

    // Ambient light term (higher for anime style)
    vec3 ambient = vec3(0.2);

    // Initialize lights
    Light lights[1];
    lights[0].direction = normalize(-vec3(3.0, 5.0, -2.0));
    lights[0].color = vec3(1.0, 1.0, 1.0);
    lights[0].intensity = 0.9;

    // Sample texture
    vec4 texColor = vec4(1.0);
    if (texIdx != uint(-1)) {
        texColor = texture(texSamplers[nonuniformEXT(texIdx)], fragUV);
    }
    if (texColor.a < 0.3) discard;

    // Start with ambient term
    vec3 finalColor = ambient * texColor.rgb;

    // Add cel-shaded diffuse lighting for each light
    for (int i = 0; i < 1; ++i) {
        float cosDir = -dot(normal, lights[i].direction);
        cosDir = max(0.0, cosDir);

        // Hard threshold for cel shading (Anime Style)
        float shadowThreshold = 0.5;  // Adjust for more or less shadow
        float celShading = step(shadowThreshold, cosDir); 

        // Diffuse shading (cel shaded)
        vec3 diffuse = celShading * lights[i].color * lights[i].intensity;

        // Accumulate lighting
        finalColor += texColor.rgb * diffuse;
    }

    // Set output color
    outColor = vec4(finalColor, 1.0);
}

