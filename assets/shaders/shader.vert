#version 450

#define PUSH_CONSTANT_FLAG_IS_BLACK    0x1
#define PUSH_CONSTANT_FLAG_IS_SELECTED 0x2
#define selected_offset 0.5f

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragPosition;
layout(location = 4) out vec4 lightViewPosition;

layout(binding = 0) uniform UniformBufferObjectVs
{
    mat4 view;
    mat4 lightView;
    mat4 proj;
    mat4 lightProj;
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

    // Transform position
    vec4 position = vec4(inPosition, 1.0);

    fragPosition = vec3(modelView * position);
    gl_Position = ubo.proj * vec4(fragPosition, 1.0);

    // Transform normal properly (important for lighting)
    fragNormal = normalize(mat3(transpose(inverse(modelView))) * inNormal);

    // Apply color modification
    fragColor = inColor;
    fragColor.rgb = fragColor.rgb * 0.8 + 0.2;
    if ((pushConstant.flag & PUSH_CONSTANT_FLAG_IS_BLACK) != 0)
    {
        fragColor.rgb *= 0.05f;
    }

    if ((pushConstant.flag & PUSH_CONSTANT_FLAG_IS_SELECTED) != 0)
    {
        fragColor.rgb *= 0.5f;
        fragColor.rgb += vec3(0.5f, 0.0f, 0.0f);
    }

    // Texture coordinates
    fragTexCoord = inTexCoord;

    // Depth bias to reduce shadow acne (self-shadowing artifacts)
    float bias = -0.001f;

    ///@note Convert X,Y value from NDC [-1, 1] to texture coordinates [0, 1].
    ///      Since shadow map format is D32_SFLOAT, we can keep the depth value in [-1, 1].
    ///@note Column major matrix
    mat4 T = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.5, 0.5, bias, 1.0
    );
    mat4 S = mat4(
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    // Light view position
    mat4 matrixShadow = T * S * ubo.lightProj * ubo.lightView * model;
    lightViewPosition = matrixShadow * vec4(inPosition, 1.0);
}
