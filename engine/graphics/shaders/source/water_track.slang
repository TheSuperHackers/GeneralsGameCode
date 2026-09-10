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
// programmable shoreline-breaking-wave material.
//
// The shoreline strips are not river or ocean surfaces.  Their authored
// texture is an alpha-shaped wave decal, and their CPU geometry supplies the
// animation, width, and fade.  Keep this pass deliberately limited to that
// contract so the decal is not interpreted as a normal/foam/environment map.

struct WaterTrackVertexInput
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float4 color0 : COLOR0;
    float2 texcoord0 : TEXCOORD0;
    float2 texcoord1 : TEXCOORD1;
};

struct WaterTrackVertexOutput
{
    float4 position : SV_POSITION;
    float4 color0 : COLOR0;
    float2 texcoord0 : TEXCOORD0;
};



[shader("vertex")] WaterTrackVertexOutput WaterTrackVertexMain(
    WaterTrackVertexInput input)
{
    WaterTrackVertexOutput output;
    output.position = mul(Projection, mul(View,
        mul(World, float4(input.position, 1.0))));
    output.color0 = input.color0;
    output.texcoord0 = input.texcoord0;
    return output;
}

GRAPHICS_TEXTURE_2D(WaterTrackTexture, float4, 0)
GRAPHICS_SAMPLER(WaterTrackSampler, 0)

struct WaterTrackPixelInput
{
    float4 position : SV_POSITION;
    float4 color0 : COLOR0;
    float2 texcoord0 : TEXCOORD0;
};

[shader("pixel")] float4 WaterTrackPixelMain(
    WaterTrackPixelInput input) : SV_TARGET
{
    // wave256.tga contains the shoreline shape in its alpha channel.  The
    // vertex alpha is the time-dependent fade supplied by WaterTracksObj.
    return WaterTrackTexture().Sample(WaterTrackSampler(), input.texcoord0) *
        input.color0;
}
