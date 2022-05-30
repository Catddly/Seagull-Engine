#version 460

layout (location = 0) in vec3 inPosLS;

layout (set = 0, binding = 0) uniform CameraUBO
{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
    vec3 viewPos;
} cameraUbo;

layout(push_constant) uniform PushConstant 
{
	mat4 model;
} pushConstant;

void main()
{
    gl_Position = cameraUbo.viewProj * pushConstant.model * vec4(inPosLS, 1.0);
}