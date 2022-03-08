#version 450

layout (location = 0) in vec3 inNormalWS;
layout (location = 1) in vec3 inPosWS;
layout (location = 2) in vec3 inViewPosWS;
layout (location = 3) in vec4 inShadowMapPos;

layout (set = 0, binding = 0) uniform UBO
{
	mat4  view;
	mat4  projection;
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
    shadowMapPos = shadowMapPos / shadowMapPos.w;

    float shadow = 1.0;
	if (shadowMapPos.z > -1.0 && shadowMapPos.z < 1.0) 
	{
		float dist = texture(sShadowMap, shadowMapPos.xy).r; // closest depth to the light
		if (shadowMapPos.w > 0.0 && dist < shadowMapPos.z) // actual depth in world space
		{
			shadow = 0.05;
		}
	}
	return shadow;
}

void main()
{
    float shadow = SampleShadowMap(inShadowMapPos);

    // ambient part
    float ambientIntensity = 0.05;
    vec3 ambient = ubo.lightColor * ambientIntensity;

    // diffuse part
    vec3 lightDir = normalize(ubo.lightPos - inPosWS);
    float diffuseIntensity = max(dot(inNormalWS, lightDir), 0.0);
    vec3 diffuse = diffuseIntensity * ubo.lightColor;

    // specular part
    float specularIntensity = 0.15f;
    vec3 viewDir = normalize(inViewPosWS - inPosWS);
    vec3 halfVec = normalize(viewDir + lightDir);
    float spec = pow(max(dot(inNormalWS, halfVec), 0.0), 32);
    vec3 specular = specularIntensity * spec * ubo.lightColor;

    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * vec3(1.0, 1.0, 1.0);
    outColor = vec4(result, 1.0);
}