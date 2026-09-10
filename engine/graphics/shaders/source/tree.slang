#include "resource_access.slangh"
#include "environment_lighting.slangh"
cbuffer TreeConstants : register(b0)
{
    row_major float4x4 ViewProjection;
    float4 Sway[10];
    float4 ShroudProjection;
    float4 Options;
};
GRAPHICS_TEXTURE_2D(TreeTexture, float4, 0)
GRAPHICS_TEXTURE_2D(ShroudTexture, float4, 1)
GRAPHICS_SAMPLER(TreeSampler, 0)
GRAPHICS_SAMPLER(ShroudSampler, 1)
struct TreeInput
{
    float3 position : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float2 reserved : TEXCOORD1;
    float3 sway : NORMAL0;
};
struct TreeOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR0;
    float2 uv : TEXCOORD0;
    float2 shroud_uv : TEXCOORD1;
    float3 world_position : TEXCOORD2;
};
[shader("vertex")] TreeOutput tree_vertex(TreeInput input)
{
    TreeOutput output;
    uint sway_index = uint(clamp(input.sway.x,1.0,10.0))-1;
    float3 world = input.position + (input.position.z-input.sway.z)*Sway[sway_index].xyz;
    output.position = mul(ViewProjection,float4(world,1));
    output.world_position = world;
    output.color = input.color.rgb*input.sway.y;
    output.uv = input.uv;
    output.shroud_uv = world.xy*ShroudProjection.xy+ShroudProjection.zw;
    return output;
}
[shader("pixel")] float4 tree_pixel(TreeOutput input) : SV_TARGET
{
    ClipEnvironment(input.world_position);
    float4 texel = TreeTexture().Sample(TreeSampler(),input.uv);
    clip(texel.a-Options.y);
    float3 visibility = Options.x > 0.5 ? ShroudTexture().Sample(ShroudSampler(),input.shroud_uv).rgb : float3(1,1,1);
    float3 ambient = min(EnvironmentAmbient.rgb, input.color);
    float3 lighting = ambient + (input.color-ambient)*DirectionalVisibility(input.world_position);
    return float4(lighting*texel.rgb*visibility*Options.z,texel.a);
}
