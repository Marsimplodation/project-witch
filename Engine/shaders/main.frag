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

    // Fresnel term: Highlight edges based on the angle between the normal and view direction
    float fresnelTerm = 1.0 - abs(dot(normalize(fragNormal), viewDir));
    fresnelTerm = pow(fresnelTerm, 3.0); // Sharpen Fresnel effect

    // Screen-space derivative-based edge detection
    vec3 dNormalX = dFdx(fragNormal);
    vec3 dNormalY = dFdy(fragNormal);

    // Measure the rate of change in normals
    float normalChange = length(dNormalX) + length(dNormalY);

    // Skip edge detection if normal change is not significant
    float normalChangeThreshold = 0.1; // Adjust this value based on your mesh
    float edgeFactor = 0.7* fresnelTerm + 0.3 * normalChange; // Weighted combination
    if (normalChange < normalChangeThreshold) {
	edgeFactor = 0.0; 
    }

    // Combine Fresnel and derivative edges
    float outlineThreshold = 0.3; // Final outline threshold

    if (edgeFactor > outlineThreshold) {
        // Render outline as black
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
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

	// Quantize cosDir to create a cel-shading effect
	int numSteps = 2; // Number of shading levels
	float step = 1.0 / float(numSteps);
	cosDir = floor(cosDir / step) * step;
        outColor.rgb += texColor.rgb * cosDir * lights[i].color * lights[i].intensity;
    }

}
