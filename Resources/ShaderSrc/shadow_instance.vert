#version 460
// per vertex attributes
layout (location = 0) in vec3 inPosWS;
layout (location = 1) in vec3 inNormalLS;
// per instance attributes
layout (location = 2) in vec3  inInstancePos;
layout (location = 3) in float inInstanceScale;
layout (location = 4) in float inMetallic;
layout (location = 5) in float inRoughness;
layout (location = 6) in uint  inId;

layout (set = 0, binding = 0) uniform UBO
{
	mat4 lightSpace;
} shadowUbo;

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

	gl_Position = shadowUbo.lightSpace * model * vec4(inPosWS, 1.0);
}