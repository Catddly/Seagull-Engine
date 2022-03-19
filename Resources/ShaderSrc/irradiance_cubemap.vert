#version 450

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3  outUVW;
layout (location = 1) out float outDeltaPhi;
layout (location = 2) out float outDeltaTheta;

layout(push_constant) uniform pushConstant 
{
	mat4  mvp;
	float deltaPhi;
	float deltaTheta;
} constant;

void main()
{
    outUVW = inPos; // used to sample cubemap
    outDeltaPhi = constant.deltaPhi;
    outDeltaTheta = constant.deltaTheta;
    gl_Position = constant.mvp * vec4(inPos, 1.0);
}