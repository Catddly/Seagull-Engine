#version 450

layout (location = 0) in vec3 inPosWS;
layout (location = 1) in vec3 inNormalLS;

layout (binding = 0) uniform UBO
{
	mat4 lightSpace;
} shadowUbo;

layout(push_constant) uniform pushConstant 
{
	mat4 model;
} constant;

void main()
{
	gl_Position = shadowUbo.lightSpace * constant.model * vec4(inPosWS, 1.0);
}