#version 450

#define PUSH_CONSTANT_FLAG_IS_BLACK    0x1
#define PUSH_CONSTANT_FLAG_IS_SELECTED 0x2
#define selected_offset 0.1f

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
    layout(offset = 192) int  flag;
} pushConstant;

mat4 translationMatrix(vec3 translation)
{
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        translation.x, translation.y, translation.z, 1.0
    );
}

void main()
{
    mat4 model     = pushConstant.model * pushConstant.normalizeMatrix;

    if ((pushConstant.flag & PUSH_CONSTANT_FLAG_IS_SELECTED) != 0)
    {
        model = translationMatrix(vec3(0.0, selected_offset, 0.0)) * model;
    }

    model = pushConstant.world * model;

    mat4 modelView = ubo.lightView * model;
    gl_Position    = ubo.lightProj * modelView * vec4(inPosition, 1.0);
}
