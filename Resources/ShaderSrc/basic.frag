#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inTexCoord;

layout (binding = 1) uniform sampler2D logoSampler;

layout (location = 0) out vec4 outColor;

void main() 
{
    outColor = texture(logoSampler, inTexCoord);
    //outColor = vec4(inTexCoord, 0.0, 1.0);
}