#include "../../../shaders/source/resource_access.slangh"

struct VisualVertexInput
{
    float3 position : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VisualVertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

cbuffer MaterialData : register(b0)
{
    float4 MaterialColor;
    uint MaterialTextureIndex;
    uint ShadowTextureIndex;
    uint2 MaterialReserved;
};

GRAPHICS_TEXTURE_ARRAY(BindlessTextures, float4, 0, 16)
GRAPHICS_SAMPLER(VisualSampler, 0)

float4 Sample_Bindless(uint index, float2 coordinates)
{
    switch (index)
    {
    case 0: return BindlessTextures[0].Sample(VisualSampler(), coordinates);
    case 1: return BindlessTextures[1].Sample(VisualSampler(), coordinates);
    case 2: return BindlessTextures[2].Sample(VisualSampler(), coordinates);
    case 3: return BindlessTextures[3].Sample(VisualSampler(), coordinates);
    case 4: return BindlessTextures[4].Sample(VisualSampler(), coordinates);
    case 5: return BindlessTextures[5].Sample(VisualSampler(), coordinates);
    case 6: return BindlessTextures[6].Sample(VisualSampler(), coordinates);
    case 7: return BindlessTextures[7].Sample(VisualSampler(), coordinates);
    default: return float4(0.0, 0.0, 0.0, 1.0);
    }
}

[shader("vertex")] VisualVertexOutput BasicVertex(VisualVertexInput input)
{
    VisualVertexOutput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

[shader("pixel")] float4 BasicPixel(VisualVertexOutput input) : SV_TARGET
{
    return input.color * MaterialColor;
}

[shader("pixel")] float4 TexturedPixel(VisualVertexOutput input) : SV_TARGET
{
    return input.color * MaterialColor * Sample_Bindless(MaterialTextureIndex, input.uv);
}

[shader("vertex")] VisualVertexOutput ShadowVertex(VisualVertexInput input)
{
    VisualVertexOutput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

[shader("pixel")] float4 ShadowPixel(VisualVertexOutput input) : SV_TARGET
{
    return 1.0.xxxx;
}

[shader("pixel")] float4 LitShadowPixel(VisualVertexOutput input) : SV_TARGET
{
    const float shadow_depth = Sample_Bindless(ShadowTextureIndex, input.position.xy * float2(0.5, -0.5) / input.position.w + 0.5).r;
    const float shadow = shadow_depth + 0.01 < input.position.z / input.position.w ? 0.35 : 1.0;
    return float4(input.color.rgb * MaterialColor.rgb * (0.25 + 0.75 * shadow), input.color.a * MaterialColor.a);
}
