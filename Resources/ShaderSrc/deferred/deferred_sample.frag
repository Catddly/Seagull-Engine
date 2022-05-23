#version 460

layout (location = 0) in vec3 inPosWS;
layout (location = 1) in vec3 inNormalWS;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inTangentWS;
layout (location = 4) in flat uint inId;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;
layout (location = 3) out vec4 outMRAO; // metallic, roughness and ao

struct ObjectRenderData
{
	mat4 model;
	mat4 inverseTransposeModel;
	vec2 mr;
    int instanceId;
    int meshId;
	vec3 albedo;
	uint texFlag;
};

// all object matrices
layout(std140, set = 1, binding = 0) readonly buffer PerObjectBuffer
{
	ObjectRenderData objects[];
} perObjectBuffer;

layout(set = 1, binding = 1) uniform sampler2D sAlbedoMap;
layout(set = 1, binding = 2) uniform sampler2D sMetallicMap;
layout(set = 1, binding = 3) uniform sampler2D sRoughnessMap;
layout(set = 1, binding = 4) uniform sampler2D sAOMap;
layout(set = 1, binding = 5) uniform sampler2D sNormalMap;

#define ALBEDO_TEX_MASK 0x01
#define METALLIC_TEX_MASK 0x02
#define ROUGHNESS_TEX_MASK 0x04
#define NORMAL_TEX_MASK 0x08
#define AO_TEX_MASK 0x10

vec3 CalculateNormal()
{
	vec3 normalFromMap = texture(sNormalMap, inUV).xyz * 2.0 - 1.0; // shift from [0, 1] to [-1, 1] 

	vec3 T = normalize(inTangentWS);
	vec3 N = normalize(inNormalWS);
	vec3 B = normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);
	return normalize(TBN * normalFromMap);
}

void main() 
{
	if ((perObjectBuffer.objects[inId].texFlag & ALBEDO_TEX_MASK) != 0)
	{
		vec4 texAlbedo = texture(sAlbedoMap, inUV);
		if (texAlbedo.a < 0.5)
			discard;

		outAlbedo = vec4(pow(texAlbedo.rgb, vec3(2.2)), texAlbedo.a);
	}
    else
        outAlbedo = vec4(perObjectBuffer.objects[inId].albedo, 1.0);

	outPosition = vec4(inPosWS, 1.0);

	if ((perObjectBuffer.objects[inId].texFlag & NORMAL_TEX_MASK) != 0)
		outNormal = vec4(CalculateNormal(), 1.0);
	else
		outNormal = vec4(normalize(inNormalWS), 1.0);

	if ((perObjectBuffer.objects[inId].texFlag & METALLIC_TEX_MASK) != 0)
		outMRAO.r = texture(sMetallicMap, inUV).r;
	else
		outMRAO.r = perObjectBuffer.objects[inId].mr.r;

	if ((perObjectBuffer.objects[inId].texFlag & ROUGHNESS_TEX_MASK) != 0)
		outMRAO.g = texture(sRoughnessMap, inUV).r;
	else
		outMRAO.g = perObjectBuffer.objects[inId].mr.g;

	if ((perObjectBuffer.objects[inId].texFlag & AO_TEX_MASK) != 0)
		outMRAO.b = texture(sAOMap, inUV).r;
	else
		outMRAO.b = 1.0;
}