#version 450

layout (location = 0) in vec3 inNormalWS;
layout (location = 1) in vec3 inPosWS;
layout (location = 2) in vec3 inViewPosWS;
layout (location = 3) in vec4 inShadowMapPos;

layout (set = 0, binding = 1) uniform LightUBO
{
	mat4  lightSpaceVP;
	vec3  viewDirection;
	float gamma;
	vec4  directionalColor;
	vec3  pointLightPos;
	float pointLightRadius;
	vec3  pointLightColor;
    float exposure;
} lightUbo;

layout(set = 0, binding = 2) uniform sampler2D sShadowMap;

layout (location = 0) out vec4 outColor;

float SampleShadowMap(vec4 shadowMapPos)
{
    // for perspective projection, we need to normalized the position
    vec3 shadowCoord = shadowMapPos.xyz / shadowMapPos.w;
    if (shadowCoord.z > 1.0) // exceed the depth texture
        return 0.03;

    float closestDepth = texture(sShadowMap, shadowCoord.xy).r; // closest depth to the light
    float currentDepth = shadowCoord.z;

    float shadow = currentDepth > closestDepth ? 0.7 : 0.03;

	return shadow;
}

float SampleShadowMapPCF(vec4 shadowMapPos)
{
    float shadow = 0.0;
    vec3 shadowCoord = shadowMapPos.xyz / shadowMapPos.w;
    vec2 texelSize = 1.0 / textureSize(sShadowMap, 0);
    // 3x3 kernel
    for(int dx = -1; dx <= 1; ++dx)
    {
        for(int dy = -1; dy <= 1; ++dy)
        {
            float depth = texture(sShadowMap, shadowCoord.xy + vec2(dx, dy) * texelSize).r; 
            shadow += shadowCoord.z > depth ? 0.7 : 0.03;        
        }    
    }
    shadow /= 9.0;
    return shadow;
}

float CalcSpecular(vec3 lightDir)
{
    float specularIntensity = 0.85f;
    vec3 viewDir = normalize(inViewPosWS - inPosWS);
    vec3 halfVec = normalize(viewDir + lightDir);
    return pow(max(dot(normalize(inNormalWS), halfVec), 0.0), 64) * specularIntensity;
}

vec3 CalcPointLightIntensity(vec3 lightDir)
{
    float distanceSqr = max(dot(lightDir, lightDir), 0.00001);
    float radius = max(lightUbo.pointLightRadius, 0.000001);
    float d2r2 = distanceSqr / (radius * radius);
    float rangeAttenuation = max(1.0 - d2r2 * d2r2, 0.0);
	float attenuation = rangeAttenuation * rangeAttenuation / distanceSqr;

    float intensity = max(dot(normalize(inNormalWS), lightDir), 0.0);
    intensity += CalcSpecular(lightDir);

    return intensity * attenuation * lightUbo.pointLightColor * 20.0f;
}

vec3 CalcDirectionalLight(vec3 lightDir)
{
    float intensity = max(dot(normalize(inNormalWS), lightDir), 0.0);
    intensity += CalcSpecular(lightDir);

    return intensity * lightUbo.directionalColor.xyz * 0.3f;
}

void main()
{
    // light radiance
    vec3 light = vec3(0.0);

    light += CalcPointLightIntensity(normalize(lightUbo.pointLightPos - inPosWS));
    light += CalcDirectionalLight(normalize(-lightUbo.viewDirection));

    // ambient part
    float ambientIntensity = 0.03;
    vec3 ambient = vec3(1.0f, 1.0f, 1.0f) * ambientIntensity;

    float shadow = SampleShadowMapPCF(inShadowMapPos);
    vec3 result = (ambient + (1.0 - shadow) * light) * vec3(1.0, 1.0, 1.0);

    // tone mapping
    result = vec3(1.0) - exp(-result * lightUbo.exposure);

    // gamma correction (do the reciprocal to flip the brightness to perceived brightness)
    outColor = vec4(pow(result, vec3(1.0 / lightUbo.gamma)), 1.0);
}