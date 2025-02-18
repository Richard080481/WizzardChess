#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform constants
{
    ///@note vs
    ///      layout(offset = 0)   mat4 world;
    ///      layout(offset = 64)  mat4 model;
    ///      layout(offset = 128) mat4 normalizeMatrix;
    ///      layout(offset = 192) int  isBlack;

    ///@note fs
    layout(offset = 196) int fileRank;
} pushConstant;

void main()
{
    if (pushConstant.fileRank == 0)
    {
        // Extract the x and y coordinates
        vec2 v2color = fragColor.xy;

        // Scale the coordinates to map them to a [0, 9.5] range
        v2color = v2color * 9.5f;

        // Set boundary condition for yellow coloring
        if (v2color.x <= 0.75f || v2color.x >= 8.75f || v2color.y <= 0.75f || v2color.y >= 8.75f)
        {
            // Yellow color outside the center
            outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }
        else
        {
            float file = v2color.x - 0.75f + 1.0f;
            float rank = v2color.y - 0.75f + 1.0f;

            float fileValue = floor(file) / 8.0f;
            float rankValue = floor(rank) / 8.0f;

            outColor = vec4(fileValue, rankValue, 0.0f, 1.0f);
        }
    }
    else
    {
        float file = float(pushConstant.fileRank & 0xffff);
        float rank = float((pushConstant.fileRank >> 16) & 0xffff);
        outColor = vec4(file / 8.0f,
                        rank / 8.0f,
                        0.0f,
                        1.0f);
    }
}
