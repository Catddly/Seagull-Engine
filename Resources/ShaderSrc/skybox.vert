#version 450

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3 outUVW;

layout (set = 0, binding = 0) uniform SkyBoxUBO
{
	mat4 proj;
    mat4 model;
} skyboxUbo;

void main() 
{
    outUVW = inPos;
    gl_Position = skyboxUbo.proj * skyboxUbo.model * vec4(inPos, 1.0);
}