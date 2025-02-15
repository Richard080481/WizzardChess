#version 450

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform UniformBufferObjectShadowVs
{
    mat4 lightView;
    mat4 lightProj;
} ubo;

layout(push_constant) uniform constants
{
    layout(offset = 0)   mat4 world;
    layout(offset = 64)  mat4 model;
    layout(offset = 128) mat4 normalizeMatrix;
    layout(offset = 192) int  isBlack; // unused
} pushConstant;

void main()
{
    mat4 MLP = ubo.lightProj * ubo.lightView * pushConstant.world * pushConstant.model * pushConstant.normalizeMatrix;
    gl_Position = MLP * vec4(inPosition, 1.0);

    mat4 model     = pushConstant.world * pushConstant.model * pushConstant.normalizeMatrix;
    mat4 modelView = ubo.lightView * model;
    gl_Position    = ubo.lightProj * modelView * vec4(inPosition, 1.0);
}
