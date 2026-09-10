struct FullscreenOverlayVertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct FullscreenOverlayVertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

cbuffer MaterialData : register(b0)
{
    float4 MaterialColor;
    float4 MaterialReserved0;
    float4 MaterialReserved1;
    float4 MaterialReserved2;
};

[shader("vertex")] FullscreenOverlayVertexOutput fullscreen_overlay_vertex(FullscreenOverlayVertexInput input)
{
    FullscreenOverlayVertexOutput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

[shader("pixel")] float4 fullscreen_overlay_pixel(FullscreenOverlayVertexOutput input) : SV_TARGET
{
    return input.color * MaterialColor;
}
