#include "resource_access.slangh"
cbuffer Parameters : register(b0)
{
    float4 Tint;
    float Operation;
    float Fade;
    float VertexAlpha;
    float Textured;
};
GRAPHICS_TEXTURE_2D(Source, float4, 0)
GRAPHICS_TEXTURE_2D(Mask, float4, 1)
GRAPHICS_SAMPLER(SourceSampler, 0)
GRAPHICS_SAMPLER(MaskSampler, 1)
struct Input
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float2 maskUV : TEXCOORD1;
    float3 normal : NORMAL;
};
struct Output
{
    float4 position : SV_Position;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float2 maskUV : TEXCOORD1;
};
Output screen_filter_vertex(Input input)
{
    Output output;
    output.position=float4(input.position,1);
    output.color=input.color; output.uv=input.uv; output.maskUV=input.maskUV;
    return output;
}
float4 screen_filter_pixel(Output input) : SV_Target
{
    float4 color=Textured>0.5 ? Source().Sample(SourceSampler(),input.uv)*input.color : input.color;
    if (Operation==1) {
        float grey=dot(color.rgb,float3(0.3,0.59,0.11));
        color=float4(lerp(color.rgb,grey*Tint.rgb,Fade),grey*Tint.a);
    }
    if (VertexAlpha>0.5) color.a=input.color.a;
    if (Operation==2) color*=Mask().Sample(MaskSampler(),input.maskUV);
    if (Operation==3) color.rgb=color.bgr;
    return color;
}
