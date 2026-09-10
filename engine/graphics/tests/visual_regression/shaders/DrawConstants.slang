struct VertexInput { float3 position : POSITION0; float4 color : COLOR0; float2 uv : TEXCOORD0; };
struct VertexOutput { float4 position : SV_POSITION; float4 color : COLOR0; };
cbuffer DrawValues : register(b1) { float4 VertexColor; float4 FragmentColor; };
[shader("vertex")] VertexOutput DrawConstantsVertex(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position, 1);
    output.color = input.color * VertexColor;
    return output;
}
[shader("pixel")] float4 DrawConstantsPixel(VertexOutput input) : SV_TARGET
{
    return input.color * FragmentColor;
}
