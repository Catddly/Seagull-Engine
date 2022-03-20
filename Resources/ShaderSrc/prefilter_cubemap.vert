#version 450

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3  outUVW;
layout (location = 1) out float outRoughness;
layout (location = 2) out uint  outSamples;

layout(push_constant) uniform pushConstant 
{
	mat4  mvp;
	float roughness;
	uint  samples;
} constant;

void main()
{
    outUVW = inPos; // used to sample cubemap
    outRoughness = constant.roughness;
    outSamples = constant.samples;
    gl_Position = constant.mvp * vec4(inPos, 1.0);
}