#include "resource_access.slangh"
struct ScreenDistortionVertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct ScreenDistortionVertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

GRAPHICS_TEXTURE_2D(Background, float4, 0)
GRAPHICS_SAMPLER(LinearSampler, 0)

ScreenDistortionVertexOutput screen_distortion_vertex(ScreenDistortionVertexInput input)
{
    ScreenDistortionVertexOutput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

float4 screen_distortion_pixel(ScreenDistortionVertexOutput input) : SV_TARGET
{
    return float4(Background().Sample(LinearSampler(), input.uv).rgb * input.color.rgb, input.color.a);
}
