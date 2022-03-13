#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

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

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outViewVec;
layout (location = 3) out vec3 outLightVec;
layout (location = 4) out vec4 outShadowCoord;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() 
{
	outColor = vec3(1.0, 1.0, 1.0);
	outNormal = inNormal;

	gl_Position = ubo.projection * ubo.view * constant.model * vec4(inPos.xyz, 1.0);
	
    vec4 pos = constant.model * vec4(inPos, 1.0);
    outNormal = mat3(constant.model) * inNormal;
    outLightVec = normalize(ubo.lightPos - inPos);
    outViewVec = -pos.xyz;			

	outShadowCoord = ( biasMat * ubo.lightSpace * constant.model ) * vec4(inPos, 1.0);	
}

