#include "resource_access.slangh"
cbuffer WaterConstants : register(b0)
{
    row_major float4x4 World;
    row_major float4x4 View;
    row_major float4x4 Projection;
    float4 WaterShroudProjection;
    float4 WaterAnimation;
    float4 WaterCameraPosition;
    float4 WaterDisplacementDomain;
    float4 WaterTint;
    float4 WaterEffects;
    float4 WaterSurfaceOptions;
    float4 FogColor;
    float4 WaterFogState;
};
// Explicit reflection-sky material used by the water render object.

struct WaterSkyVertexInput
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float4 color0 : COLOR0;
    float2 texcoord0 : TEXCOORD0;
    float2 texcoord1 : TEXCOORD1;
};

struct WaterSkyVertexOutput
{
    float4 position : SV_POSITION;
    float4 color0 : COLOR0;
    float2 texcoord0 : TEXCOORD0;
};



[shader("vertex")] WaterSkyVertexOutput WaterSkyVertexMain(
    WaterSkyVertexInput input)
{
    WaterSkyVertexOutput output;
    output.position = mul(Projection, mul(View,
        mul(World, float4(input.position, 1.0))));
    output.color0 = input.color0;
    output.texcoord0 = input.texcoord0;
    return output;
}

GRAPHICS_TEXTURE_2D(WaterSkyTexture, float4, 0)
GRAPHICS_SAMPLER(WaterSkySampler, 0)

struct WaterSkyPixelInput
{
    float4 position : SV_POSITION;
    float4 color0 : COLOR0;
    float2 texcoord0 : TEXCOORD0;
};

[shader("pixel")] float4 WaterSkyPixelMain(WaterSkyPixelInput input) : SV_TARGET
{
    return WaterSkyTexture().Sample(WaterSkySampler(), input.texcoord0) *
        input.color0;
}
