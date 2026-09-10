struct RingVertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct RingVertexOutput
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

[shader("vertex")] RingVertexOutput ring_vertex(RingVertexInput input)
{
    RingVertexOutput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

[shader("pixel")] float4 ring_pixel(RingVertexOutput input) : SV_TARGET
{
    return input.color * MaterialColor;
}
