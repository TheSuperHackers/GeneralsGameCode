#include "resource_access.slangh"
#include "environment_lighting.slangh"
GRAPHICS_TEXTURE_2D(BaseTexture, float4, 0)
GRAPHICS_TEXTURE_2D(CloudTexture, float4, 1)
GRAPHICS_TEXTURE_2D(LightMapTexture, float4, 2)
GRAPHICS_TEXTURE_2D(ShroudTexture, float4, 3)
GRAPHICS_SAMPLER(BaseSampler, 0)
GRAPHICS_SAMPLER(CloudSampler, 1)
GRAPHICS_SAMPLER(LightMapSampler, 2)
GRAPHICS_SAMPLER(ShroudSampler, 3)

cbuffer SurfaceConstants : register(b0)
{
    row_major float4x4 ViewProjection;
    float4 CloudProjection;
    float4 LightMapProjection;
    float4 ShroudProjection;
    float AlphaCutoff;
    float Textured;
    float Cloud;
    float LightMap;
    float MaskedModulation;
    float Shroud;
    float Opacity;
    float ShroudOnly;
};

struct VertexInput
{
    float3 position : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};
struct PixelInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float2 world_xy : TEXCOORD1;
    float3 world_position : TEXCOORD2;
};
[shader("vertex")] PixelInput surface_vertex(VertexInput input)
{
    PixelInput output;
    output.position = mul(ViewProjection, float4(input.position, 1));
    output.color = input.color;
    output.uv = input.uv;
    output.world_xy = input.position.xy;
    output.world_position = input.position;
    return output;
}

[shader("pixel")] float4 surface_pixel(PixelInput input) : SV_TARGET
{
    ClipEnvironment(input.world_position);
    if (ShroudOnly > 0.5) return float4(ShroudTexture().Sample(ShroudSampler(),
        input.world_xy * ShroudProjection.xy + ShroudProjection.zw).rgb, 1);
    float4 base = Textured > 0.5 ? BaseTexture().Sample(BaseSampler(), input.uv) : float4(1, 1, 1, 1);
    if (MaskedModulation > 0.5) {
        float3 noise = LightMapTexture().Sample(LightMapSampler(),
            input.world_xy * LightMapProjection.xy + LightMapProjection.zw).rgb;
        return float4(lerp(input.color.aaa, noise, base.a), 1);
    }
    float4 color = base * input.color;
    color.a *= Opacity;
    if (CloudOffsetStrength.w > 0 || ShadowOptions.x > 0) {
        float3 ambient = min(EnvironmentAmbient.rgb, input.color.rgb);
        color.rgb = base.rgb*(ambient+(input.color.rgb-ambient)*DirectionalVisibility(input.world_position));
    }
    else if (Cloud > 0.5) color *= CloudTexture().Sample(CloudSampler(),
        input.world_xy * CloudProjection.xy + CloudProjection.zw);
    if (LightMap > 0.5) color *= LightMapTexture().Sample(LightMapSampler(),
        input.world_xy * LightMapProjection.xy + LightMapProjection.zw);
    if (Shroud > 0.5) color.rgb *= ShroudTexture().Sample(ShroudSampler(),
        input.world_xy * ShroudProjection.xy + ShroudProjection.zw).rgb;
    clip(color.a - AlphaCutoff);
    return color;
}
