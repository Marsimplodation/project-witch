#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) flat in uint texIdx;
layout (location = 0) out vec4 outColor;
layout(set = 0, binding = 2) uniform sampler2D texSamplers[100];


void main() {
    if(texIdx == uint(-1)) {
    	outColor = vec4(1);
	return;
    }
    vec2 uv = fragUV - floor(fragUV);
    vec4 texColor = texture(texSamplers[texIdx], uv);
    outColor = texColor;
}
