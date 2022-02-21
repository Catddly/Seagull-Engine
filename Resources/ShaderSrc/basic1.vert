#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormalLS;

layout (location = 0) out vec3 outNormalWS;
layout (location = 1) out vec3 outPosWS;
layout (location = 2) out vec3 outViewPosWS;

layout (binding = 0) uniform UBO
{
	mat4  view;
	mat4  projection;
	vec3  viewPos;
	float pad;
} ubo;

layout(push_constant) uniform pushConstant 
{
	mat4 model;
	mat4 inverseTransposeModel;
} constant;

void main() 
{
	outViewPosWS = ubo.viewPos;
	outPosWS = vec3(constant.model * vec4(inPos, 1.0));
	outNormalWS = mat3(constant.inverseTransposeModel) * inNormalLS; 
    gl_Position = ubo.projection * ubo.view * vec4(outPosWS, 1.0);
}