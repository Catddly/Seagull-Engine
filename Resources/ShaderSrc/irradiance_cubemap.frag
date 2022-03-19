#version 450

layout (location = 0) in vec3  inUVW;
layout (location = 1) in float inDeltaPhi;
layout (location = 2) in float inDeltaTheta;

layout(set = 0, binding = 0) uniform samplerCube sCubeMap;

layout (location = 0) out vec4 outColor;

#define PI 3.1415926535897932384626433832795

void main()
{
    vec3 normal = normalize(inUVW);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal));
	up = cross(normal, right);

	const float TWO_PI = PI * 2.0;
	const float HALF_PI = PI * 0.5;

	vec3 color = vec3(0.0);
	uint sampleCount = 0u;

	// sample map (semisphere integral)
	for (float phi = 0.0; phi < TWO_PI; phi += inDeltaPhi) 
    {
		for (float theta = 0.0; theta < HALF_PI; theta += inDeltaTheta) 
        {
			vec3 tempVec = cos(phi) * right + sin(phi) * up;
			vec3 sampleVector = cos(theta) * normal + sin(theta) * tempVec;

			color += texture(sCubeMap, sampleVector).rgb * cos(theta) * sin(theta);
			sampleCount++;
		}
	}
	outColor = vec4(PI * color / float(sampleCount), 1.0);
	//outColor = vec4(texture(sCubeMap, normal).rgb, 1.0);
}