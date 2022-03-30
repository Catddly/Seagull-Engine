#version 460
// per vertex attributes
layout (location = 0) in vec3 inPosWS;
layout (location = 1) in vec3 inNormalLS;
// per instance attributes
layout (location = 2) in vec3  inInstancePos;
layout (location = 3) in float inInstanceScale;
layout (location = 4) in float inMetallic;
layout (location = 5) in float inRoughness;

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

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() 
{
	mat4 translationMat;
	translationMat[0] = vec4(1.0, 0.0, 0.0, 0.0);
	translationMat[1] = vec4(0.0, 1.0, 0.0, 0.0);
	translationMat[2] = vec4(0.0, 0.0, 1.0, 0.0);
	translationMat[3] = vec4(inInstancePos.x, inInstancePos.y, inInstancePos.z, 1.0);

	mat4 scaleMat;
	scaleMat[0] = vec4(inInstanceScale, 0.0, 0.0, 0.0);
	scaleMat[1] = vec4(0.0, inInstanceScale, 0.0, 0.0);
	scaleMat[2] = vec4(0.0, 0.0, inInstanceScale, 0.0);
	scaleMat[3] = vec4(0.0, 0.0, 0.0, 1.0);

	mat4 model = translationMat * scaleMat;
	mat4 ivModel = transpose(inverse(model));

	outViewPosWS = cameraUbo.viewPos;
	outPosWS = vec3(model * vec4(inPosWS, 1.0));
	outNormalWS = mat3(ivModel) * inNormalLS;
	outShadowMapPos = biasMat * lightUbo.lightSpaceVP * vec4(outPosWS, 1.0);
	outMetallic = inMetallic;
	outRoughness = inRoughness;

    gl_Position = cameraUbo.viewProj * vec4(outPosWS, 1.0);
}