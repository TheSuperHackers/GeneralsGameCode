#include "resource_access.slangh"
struct VertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};
struct VertexOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};
cbuffer BloomParameters : register(b0)
{
    float2 TexelSize;
    uint Operation;
    float Intensity;
    float3 Exposure;
    uint HalfKernelSize;
};
GRAPHICS_TEXTURE_2D(Source, float4, 0)
GRAPHICS_TEXTURE_2D(Scene, float4, 1)
GRAPHICS_SAMPLER(LinearClamp, 0)

float3 SampleLinearScene(float2 uv)
{
    uint width, height;
    Source().GetDimensions(width,height);
    float2 position = uv*float2(width,height)-0.5;
    int2 base = int2(floor(position));
    float2 fraction = frac(position);
    int2 minimum = clamp(base,int2(0,0),int2(width-1,height-1));
    int2 maximum = clamp(base+1,int2(0,0),int2(width-1,height-1));
    float3 top_left = pow(max(Source().Load(int3(minimum,0)).rgb,0),2.2);
    float3 top_right = pow(max(Source().Load(int3(maximum.x,minimum.y,0)).rgb,0),2.2);
    float3 bottom_left = pow(max(Source().Load(int3(minimum.x,maximum.y,0)).rgb,0),2.2);
    float3 bottom_right = pow(max(Source().Load(int3(maximum,0)).rgb,0),2.2);
    return lerp(lerp(top_left,top_right,fraction.x),lerp(bottom_left,bottom_right,fraction.x),fraction.y);
}

[shader("vertex")] VertexOutput bloom_vertex(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position, 1);
    output.uv = input.uv;
    return output;
}

[shader("pixel")] float4 bloom_pixel(VertexOutput input) : SV_TARGET
{
    if (Operation == 0 || Operation == 4)
    {
        float3 color = 0;
        if (Operation == 0) {
            int radius = int(HalfKernelSize)-1;
            [loop] for (int y=-radius;y<=radius;y+=2)
                [loop] for (int x=-radius;x<=radius;x+=2)
                    color += SampleLinearScene(input.uv + TexelSize*float2(x,y));
            color /= float(HalfKernelSize*HalfKernelSize);
        } else {
            color = (Source().Sample(LinearClamp(), input.uv + TexelSize * float2(-1,-1)).rgb
            + Source().Sample(LinearClamp(), input.uv + TexelSize * float2(1,-1)).rgb
            + Source().Sample(LinearClamp(), input.uv + TexelSize * float2(-1,1)).rgb
            + Source().Sample(LinearClamp(), input.uv + TexelSize * float2(1,1)).rgb) * 0.25;
            return float4(color,0);
        }
        float brightness = dot(color, float3(0.3,0.3,0.3));
        float excess = max(brightness - 0.11, 0.0);
        return float4(color * (excess * excess / (brightness + 0.0001)) * Exposure, 0);
    }
    if (Operation == 3)
    {
        float4 scene = Scene().Load(int3(int2(input.position.xy),0));
        float3 bloom = Source().Sample(LinearClamp(), input.uv).rgb * Intensity;
        float3 color = pow(max(pow(max(scene.rgb,0),2.2) + bloom,0),1.0/2.2);
        return float4(color,scene.a);
    }
    if (Operation == 5)
        return float4(Scene().Sample(LinearClamp(),input.uv).rgb
            + Source().Sample(LinearClamp(),input.uv).rgb,0);
    float2 direction = Operation == 1 ? float2(TexelSize.x,0) : float2(0,TexelSize.y);
    float3 color = Source().Sample(LinearClamp(), input.uv).rgb * 252.0;
    color += (Source().Sample(LinearClamp(), input.uv - direction * 1.65).rgb
        + Source().Sample(LinearClamp(), input.uv + direction * 1.65).rgb) * 120.0;
    color += (Source().Sample(LinearClamp(), input.uv - direction * 3.55).rgb
        + Source().Sample(LinearClamp(), input.uv + direction * 3.55).rgb) * 10.0;
    return float4(color / 512.0, 0);
}
