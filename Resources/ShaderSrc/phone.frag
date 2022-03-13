#version 450

layout (location = 0) in vec3 inNormalWS;
layout (location = 1) in vec3 inPosWS;
layout (location = 2) in vec3 inViewPosWS;
layout (location = 3) in vec4 inShadowMapPos;

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

layout(set = 0, binding = 1) uniform sampler2D sShadowMap;

layout (location = 0) out vec4 outColor;

float SampleShadowMap(vec4 shadowMapPos)
{
    // for perspective projection, we need to normalized the position
    vec3 shadowCoord = shadowMapPos.xyz / shadowMapPos.w;

    float closestDepth = texture(sShadowMap, shadowCoord.xy).r; // closest depth to the light
    float currentDepth = shadowCoord.z;

    float shadow = currentDepth > closestDepth ? 0.7 : 0.03;
	return shadow;
}

void main()
{
    // ambient part
    float ambientIntensity = 0.03;
    vec3 ambient = vec3(1.0f, 1.0f, 1.0f) * ambientIntensity;

    // diffuse part
    vec3 lightDir = ubo.lightPos - inPosWS;
    float distanceSqr = max(dot(lightDir, lightDir), 0.00001);
    float radius = max(ubo.lightRadius, 0.000001);
    float d2r2 = distanceSqr / (radius * radius);
    float rangeAttenuation = max(1.0 - d2r2 * d2r2, 0.0);
	float lightAttenuation = (rangeAttenuation * rangeAttenuation) / distanceSqr;

    float diffuseIntensity = max(dot(inNormalWS, lightDir), 0.0);
    vec3 diffuse = diffuseIntensity * ubo.lightColor;

    // specular part
    float specularIntensity = 0.15f;
    vec3 viewDir = normalize(inViewPosWS - inPosWS);
    vec3 halfVec = normalize(viewDir + lightDir);
    float spec = pow(max(dot(inNormalWS, halfVec), 0.0), 32);
    vec3 specular = specularIntensity * spec * ubo.lightColor;

    float shadow = SampleShadowMap(inShadowMapPos);

    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular) * lightAttenuation) * vec3(1.0, 1.0, 1.0);
    outColor = vec4(result, 1.0);
}