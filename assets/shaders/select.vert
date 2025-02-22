#version 450

#define PUSH_CONSTANT_FLAG_IS_BLACK    0x1
#define PUSH_CONSTANT_FLAG_IS_SELECTED 0x2
#define selected_offset 0.5f

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;

layout(binding = 0) uniform UniformBufferObjectVs
{
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform constants
{
    ///@note VS part
    layout(offset = 0)   mat4 world;
    layout(offset = 64)  mat4 model;
    layout(offset = 128) mat4 normalizeMatrix;
    layout(offset = 192) int  flag;

    ///@note FS part
    ///      layout(offset = 196) int renderMode;
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
    mat4 model     = pushConstant.world * pushConstant.model * pushConstant.normalizeMatrix;

    if ((pushConstant.flag & PUSH_CONSTANT_FLAG_IS_SELECTED) != 0)
    {
        model = translationMatrix(vec3(0.0, selected_offset, 0.0)) * model;
    }

    mat4 modelView = ubo.view * model;

    // Position
    gl_Position = ubo.proj * modelView * vec4(inPosition, 1.0);

    fragColor = vec3(inTexCoord.x, 1.0 - inTexCoord.y, 0.0);
}
