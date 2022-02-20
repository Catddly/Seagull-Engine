#version 450

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO 
{
	mat4 view;
	mat4 projection;
} ubo;

layout(push_constant) uniform pushConstant 
{
	mat4 model;
} constant;

void main() 
{
    gl_Position = ubo.projection * ubo.view * constant.model * vec4(inPos, 1.0);
}