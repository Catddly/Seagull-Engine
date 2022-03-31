#version 460

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormalLS;

layout (location = 0) out vec3 outNormalWS;
layout (location = 1) out vec3 outPosWS;
layout (location = 2) out vec3 outViewPosWS;
layout (location = 3) out vec4 outShadowMapPos;
layout (location = 4) out float outMetallic;
layout (location = 5) out float outRoughness;

layout (set = 0, binding = 0) uniform CameraUBO
{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
    vec3 viewPos;
} cameraUbo;

layout (set = 0, binding = 1) uniform LightUBO
{
	mat4  lightSpaceVP;
	vec3  viewDirection;
	float pad;
	vec4  directionalColor;
	vec3  pointLightPos;
	float pointLightRadius;
	vec3  pointLightColor;
} lightUbo;

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

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() 
{
	outViewPosWS = cameraUbo.viewPos;
	outPosWS = vec3(perObjectBuffer.objects[gl_BaseInstance].model * vec4(inPos, 1.0));
	outNormalWS = mat3(perObjectBuffer.objects[gl_BaseInstance].inverseTransposeModel) * inNormalLS; 
	outShadowMapPos = biasMat * lightUbo.lightSpaceVP * vec4(outPosWS, 1.0);
	outMetallic = perObjectBuffer.objects[gl_BaseInstance].mrxx.x;
	outRoughness = perObjectBuffer.objects[gl_BaseInstance].mrxx.y;

    gl_Position = cameraUbo.viewProj * vec4(outPosWS, 1.0);
}