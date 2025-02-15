#version 450

#define USE_SOFT_SHADOW true

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragPosition;
layout(location = 4) in vec4 lightViewPosition;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UniformBufferObjectFs
{
    vec3 lightPos;
    vec3 lightColor;
    vec3 cameraPos;
} ubo;

layout(binding = 2) uniform sampler2D texSampler;
layout(binding = 3) uniform sampler2D shadowSampler;

layout(push_constant) uniform constants
{
    ///@note vs
    ///      layout(offset = 0)   mat4 world;
    ///      layout(offset = 64)  mat4 model;
    ///      layout(offset = 128) mat4 normalizeMatrix;
    ///      layout(offset = 192) int  isBlack;

    ///@note fs
    layout(offset = 196) int renderMode;
} pushConstant;


// Function to calculate soft shadows using PCF (Percentage Closer Filtering)
float PCFSoftShadow(sampler2D shadowMap, vec3 projCoords, int kernelSize)
{
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);  // Get texel size for sampling

    float currentDepth = projCoords.z;

    int samples = 0;

    for (int x = -kernelSize; x <= kernelSize; x++)
    {
        for (int y = -kernelSize; y <= kernelSize; y++)
        {
            vec2 offset = vec2(x, y) * texelSize;
            float closestDepth = texture(shadowMap, projCoords.xy + offset).r;
            float bias = 0.001f;
            shadow += (currentDepth > closestDepth + bias) ? 0.0 : 1.0;
            samples++;
        }
    }
    return shadow / float(samples);
}


void main()
{
    if (pushConstant.renderMode == 0)
    {
        vec3 N = normalize(fragNormal);
        vec3 L = normalize(ubo.lightPos - fragPosition);
        vec3 V = normalize(ubo.cameraPos - fragPosition);
        vec3 R = reflect(-L, N);

        // Ambient lighting
        float ambientStrength = 0.1;
        vec3 ambient = ambientStrength * ubo.lightColor;

        // Diffuse lighting (Lambertian reflection)
        float diff = max(dot(N, L), 0.0);
        vec3 diffuse = diff * ubo.lightColor;

        // Specular lighting (Phong reflection model)
        float specularStrength = 0.5;
        float shininess = 32.0;
        float spec = pow(max(dot(V, R), 0.0), shininess);
        vec3 specular = specularStrength * spec * ubo.lightColor;

        vec3 lighting = ambient + diffuse + specular;

        // Color Mode - Apply lighting to vertex color
        outColor = vec4(lighting * fragColor, 1.0);
    }
    else if (pushConstant.renderMode == 1)
    {
        outColor = texture(texSampler, fragTexCoord);
    }

    if (!USE_SOFT_SHADOW)
    {
        // Hard shadow
        vec3 p = lightViewPosition.xyz / lightViewPosition.w;
        float shadowMapDepth = texture(shadowSampler, p.xy).r;
        outColor *= (shadowMapDepth < p.z) ? 0.2 : 1;
    }
    else
    {
        // Soft shadow
        vec3 p = lightViewPosition.xyz / lightViewPosition.w;
        float shadowFactor = PCFSoftShadow(shadowSampler, p, 5);
        outColor *= mix(0.1, 1.0, shadowFactor);
    }
}
