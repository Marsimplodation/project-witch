#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint texIdx;
layout(location = 4) in vec3 cameraPosition;

layout (location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];


struct Light {
    vec3 pos;
    vec3 color;
    float intensity;
};

float calculateFresnelTerm(float dot, float n1, float n2) {
    float r0 = ((n1 - n2) / (n1 + n2));
    r0 *= r0;
    return r0 + (1 - r0) * pow(1 - dot, 5);
}

float calculateFresnelInAir(float dot) {
    float r0 = 0;
    r0 *= r0;
    return r0 + (1 - r0) * pow(1 - dot, 5);
}

void main() {
        // Calculate view direction
    vec3 viewDir = normalize(cameraPosition - fragPosition);
    Light lights[1];
    
    // Initialize each field manually
    lights[0].pos = vec3(5.0, 2.0, -3.0);
    lights[0].color = vec3(1.0, 1.0, 1.0);
    lights[0].intensity = 1.0;

    vec4 texColor = vec4(1);
    if(texIdx != uint(-1)) {
	texColor = texture(texSamplers[texIdx], fragUV);
    }
    outColor = vec4(vec3(0.1), 1.0) * texColor;

    for(int i = 0; i < 1; ++i) {
	vec3 lightPosition = lights[i].pos;
	vec3 lightDirection = normalize(lightPosition - fragNormal);
	float cosDir = dot(lightDirection, fragNormal);
	cosDir = max(0.1, cosDir);
        outColor.rgb += texColor.rgb * cosDir * lights[i].color * lights[i].intensity;
    }

}
