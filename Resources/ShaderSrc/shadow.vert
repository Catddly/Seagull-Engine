#version 460
// per vertex attributes
layout (location = 0) in vec3 inPosWS;
layout (location = 1) in vec3 inNormalLS;

layout (set = 0, binding = 0) uniform UBO
{
	mat4 lightSpace;
} shadowUbo;

//struct PerMeshRenderData
//{
//	mat4 model;
//	mat4 inverseTransposeModel;
//	float metallic;
//	float roughness;
//	float pad1;
//	float pad2;
//};

//// all object matrices
//layout(std140, set = 1, binding = 0) readonly buffer PerMeshBuffer
//{
//	PerMeshRenderData objects[];
//} perMeshBuffer;

layout(push_constant) uniform PushConstant 
{
	mat4 model;
} pushConstant;

void main()
{
	gl_Position = shadowUbo.lightSpace * pushConstant.model * vec4(inPosWS, 1.0);
}