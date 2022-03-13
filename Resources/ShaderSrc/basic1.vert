#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormalLS;

layout (location = 0) out vec3 outNormalWS;
layout (location = 1) out vec3 outPosWS;
layout (location = 2) out vec3 outViewPosWS;
layout (location = 3) out vec4 outShadowMapPos;

layout (set = 0, binding = 0) uniform UBO
{
	mat4  view;
	mat4  projection;
	mat4  lightSpace;
	vec3  viewPos;
	float lightRadius;
	vec3  lightPos;
	float pad;
	vec3  lightColor;
} ubo;

layout(push_constant) uniform pushConstant 
{
	mat4 model;
	mat4 inverseTransposeModel;
} constant;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() 
{
	outViewPosWS = ubo.viewPos;
	outPosWS = vec3(constant.model * vec4(inPos, 1.0));
	outNormalWS = mat3(constant.inverseTransposeModel) * inNormalLS; 
	outShadowMapPos = biasMat * ubo.lightSpace * vec4(outPosWS, 1.0);

    gl_Position = ubo.projection * ubo.view * vec4(outPosWS, 1.0);
}