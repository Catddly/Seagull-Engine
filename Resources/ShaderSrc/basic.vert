#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;

layout (binding = 0) uniform UBO 
{
	mat4 view;
	mat4 projection;
} ubo;

layout(push_constant) uniform pushConstant 
{
	mat4 model;
} constant;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outTexCoord;

void main() 
{
    outColor = inColor;
	outTexCoord = inTexCoord;
    gl_Position = ubo.projection * ubo.view * constant.model * vec4(inPos, 1.0);
}