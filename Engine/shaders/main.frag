#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) flat in uint texIdx;
layout (location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];



void main() {
    vec3 lightPosition = vec3(2,1,3);
    vec3 lightDirection = normalize(lightPosition - fragNormal);
    float cosDir = dot(lightDirection, fragNormal);
    cosDir = max(0.0, cosDir);
    if(texIdx == uint(-1)) {
    	outColor = vec4(1) * cosDir;
	return;
    }
    vec2 uv = fragUV - floor(fragUV);
    vec4 texColor = texture(texSamplers[texIdx], uv);
    outColor = texColor * cosDir;
}
