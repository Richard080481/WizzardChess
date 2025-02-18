#version 450

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
    layout(offset = 192) int  isBlack; // unused

    ///@note FS part
    ///      layout(offset = 196) int renderMode;
} pushConstant;

void main()
{
    mat4 model     = pushConstant.world * pushConstant.model * pushConstant.normalizeMatrix;
    mat4 modelView = ubo.view * model;

    // Position
    vec3 fragPosition = vec3(modelView * vec4(inPosition, 1.0));
    gl_Position = ubo.proj * vec4(fragPosition, 1.0);

    fragColor = vec3(inTexCoord.x, 1.0 - inTexCoord.y, 0.0);
    /*
    if (pushConstant.fileRank == 0)
    {
    }
    else
    {
        float file = float(pushConstant.fileRank & 0xffff);
        float rank = float((pushConstant.fileRank >> 16) & 0xffff);
        fragColor = vec3(file / 8.0f + 0.75f,
                         rank / 8.0f + 0.75f,
                         0.0f);
        fragColor = vec3(0.5f, 0.5f, 0.0);
    }
    */
}
