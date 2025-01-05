#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint texIdx;
layout (location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];


struct Light {
    vec3 pos;
    vec3 color;
    float intensity;
};

void main() {
    Light lights[1];
    
    // Initialize each field manually
    lights[0].pos = vec3(-2.0, 10.0, -3.0);
    lights[0].color = vec3(1.0, 1.0, 1.0);
    lights[0].intensity = 1.0;

    vec4 texColor = vec4(1);
    if(texIdx != uint(-1)) {
	vec2 uv = fragUV - floor(fragUV);
	texColor = texture(texSamplers[texIdx], uv);
    }
    outColor = vec4(vec3(0.0), 1.0) * texColor;

    for(int i = 0; i < 1; ++i) {
	vec3 lightPosition = lights[i].pos;
	vec3 lightDirection = normalize(lightPosition - fragNormal);
	float cosDir = dot(lightDirection, fragNormal);
	cosDir = max(0.0, cosDir);

	// Quantize cosDir to create a cel-shading effect
	int numSteps = 4; // Number of shading levels
	float step = 1.0 / float(numSteps);
	cosDir = floor(cosDir / step) * step;
        outColor.rgb += texColor.rgb * cosDir * lights[i].color * lights[i].intensity;
    }

}
