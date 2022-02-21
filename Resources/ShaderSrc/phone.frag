#version 450

layout (location = 0) in vec3 inNormalWS;
layout (location = 1) in vec3 inPosWS;
layout (location = 2) in vec3 inViewPosWS;

layout (location = 0) out vec4 outColor;

void main() 
{
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    // ambient part
    float ambientIntensity = 0.05f;
    vec3 ambient = lightColor * ambientIntensity;

    // diffuse part
    vec3 lightPos = vec3(1.2, 2.0, 1.0);
    vec3 lightDir = normalize(lightPos - inPosWS);
    float diffuseIntensity = max(dot(inNormalWS, lightDir), 0.0);
    vec3 diffuse = diffuseIntensity * lightColor;

    // specular part
    float specularIntensity = 0.15f;
    vec3 viewDir = normalize(inViewPosWS - inPosWS);
    vec3 halfVec = normalize(inViewPosWS + lightDir);
    float spec = pow(max(dot(inNormalWS, halfVec), 0.0), 32);
    vec3 specular = specularIntensity * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * vec3(1.0, 1.0, 1.0);
    outColor = vec4(result, 1.0);
}