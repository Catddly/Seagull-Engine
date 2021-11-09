#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inColor;

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

void main() 
{
    outColor = inColor;
    gl_Position = ubo.projection * ubo.view * constant.model * vec4(inPos, 1.0);
}