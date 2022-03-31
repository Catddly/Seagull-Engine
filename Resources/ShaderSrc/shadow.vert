#version 460
// per vertex attributes
layout (location = 0) in vec3 inPosWS;
layout (location = 1) in vec3 inNormalLS;

layout (set = 0, binding = 0) uniform UBO
{
	mat4 lightSpace;
} shadowUbo;

struct ObjectRenderData
{
	mat4 model;
	mat4 inverseTransposeModel;
	vec4 mrxx;
};

// all object matrices
layout(std140, set = 1, binding = 0) readonly buffer PerObjectBuffer
{
	ObjectRenderData objects[];
} perObjectBuffer;

void main()
{
	gl_Position = shadowUbo.lightSpace * perObjectBuffer.objects[gl_BaseInstance].model * vec4(inPosWS, 1.0);
}