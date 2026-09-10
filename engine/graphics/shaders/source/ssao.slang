#include "resource_access.slangh"
// Right-handed view space, -Z forward, [0,1] hardware depth.
cbuffer Parameters : register(b0)
{
    row_major float4x4 Projection;
    row_major float4x4 InverseProjection;
    float Radius;
    float Bias;
    float Strength;
    uint Operation;
};
GRAPHICS_TEXTURE_2D(Scene, float4, 0)
GRAPHICS_TEXTURE_2D(Depth, float, 1)
GRAPHICS_TEXTURE_2D(Occlusion, float4, 2)
struct Input { float3 position : POSITION; float4 color : COLOR0; float2 uv : TEXCOORD0; };
struct Output { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
[shader("vertex")] Output ssao_vertex(Input input)
{
    Output output;
    output.position = float4(input.position,1);
    output.uv = input.uv;
    return output;
}
float3 ViewPosition(int2 pixel, int2 size)
{
    float2 uv = (float2(pixel)+0.5)/float2(size);
    float4 position = mul(InverseProjection,float4(uv.x*2-1,1-uv.y*2,Depth().Load(int3(pixel,0)),1));
    return position.xyz/position.w;
}
float3 SurfaceNormal(int2 pixel, int2 size, float3 position)
{
    float3 left = position-ViewPosition(max(pixel-int2(1,0),0),size);
    float3 right = ViewPosition(min(pixel+int2(1,0),size-1),size)-position;
    float3 up = ViewPosition(max(pixel-int2(0,1),0),size)-position;
    float3 down = position-ViewPosition(min(pixel+int2(0,1),size-1),size);
    // Choose the neighbor on the same surface at depth discontinuities.
    float3 dx = pixel.x==0 ? right : pixel.x==size.x-1 ? left : abs(left.z)<abs(right.z) ? left : right;
    float3 dy = pixel.y==0 ? down : pixel.y==size.y-1 ? up : abs(up.z)<abs(down.z) ? up : down;
    float3 normal = cross(dx,dy);
    float lengthSquared = dot(normal,normal);
    if (lengthSquared<1e-12) return float3(0,0,1);
    return normal*rsqrt(lengthSquared)*(normal.z<0 ? -1.0 : 1.0);
}
// Fixed stratified hemisphere: no simulation RNG or frame-time noise.
static const float3 Samples[16] = {
    float3(0.1992065, 0.0000000, 0.0250977),
    float3(-0.1415181, 0.1296422, 0.0776367),
    float3(0.0149823, -0.1707154, 0.1372070),
    float3(0.0701879, 0.0915477, 0.2084961),
    float3(-0.2572232, -0.0454992, 0.0329102),
    float3(0.2303771, -0.1465470, 0.1104492),
    float3(-0.0672873, 0.2503054, 0.2075195),
    float3(-0.0838502, -0.1614484, 0.3288086),
    float3(0.3968085, 0.1449138, 0.0532227),
    float3(-0.4130484, 0.1705004, 0.1807617),
    float3(0.1801662, -0.3850046, 0.3403320),
    float3(0.0888584, 0.2832947, 0.5366211),
    float3(-0.5908379, -0.3424027, 0.0860352),
    float3(0.6967357, -0.1531754, 0.2885742),
    float3(-0.3847730, 0.5473000, 0.5356445),
    float3(-0.0591528, -0.4564783, 0.8319336)
};
[shader("pixel")] float4 ssao_pixel(Output input) : SV_TARGET
{
    uint width,height;
    Depth().GetDimensions(width,height);
    int2 size = int2(width,height);
    int2 pixel = clamp(int2(input.uv*float2(size)),0,size-1);
    float depth = Depth().Load(int3(pixel,0));
    if (depth>=1.0)
        return Operation==0 ? float4(1,0,0,0) : Scene().Load(int3(pixel,0));
    float3 position = ViewPosition(pixel,size);
    if (Operation==0)
    {
        float3 normal = SurfaceNormal(pixel,size,position);
        float3 tangent = normalize(cross(abs(normal.z)>0.9 ? float3(0,1,0) : float3(0,0,1),normal));
        float3 bitangent = cross(normal,tangent);
        uint2 tile = uint2(input.position.xy)&3u;
        float sine,cosine;
        sincos(float(tile.x+4*tile.y)*0.3926990817,sine,cosine);
        float occlusion = 0;
        [loop] for (int i=0;i<16;++i)
        {
            float3 kernelSample = Samples[i];
            float2 rotated = float2(kernelSample.x*cosine-kernelSample.y*sine,kernelSample.x*sine+kernelSample.y*cosine);
            float3 probe = position+Radius*(tangent*rotated.x+bitangent*rotated.y+normal*kernelSample.z);
            float4 clip = mul(Projection,float4(probe,1));
            if (clip.w<=0) continue;
            float3 ndc = clip.xyz/clip.w;
            float2 uv = float2(ndc.x*0.5+0.5,0.5-ndc.y*0.5);
            // Offscreen samples and cleared depth cannot occlude a surface.
            if (any(uv<0) || any(uv>=1) || ndc.z<0 || ndc.z>1) continue;
            int2 neighbor = int2(uv*float2(size));
            if (Depth().Load(int3(neighbor,0))>=1.0) continue;
            float3 surface = ViewPosition(neighbor,size);
            float3 delta = surface-position;
            if (surface.z>probe.z+Bias && dot(normal,delta)>Bias)
                occlusion += saturate(1.0-length(delta)/Radius);
        }
        return float4(saturate(1.0-occlusion/8.0),position.z,0,1);
    }
    uint aoWidth,aoHeight;
    Occlusion().GetDimensions(aoWidth,aoHeight);
    float2 aoPixel = input.uv*float2(aoWidth,aoHeight)-0.5;
    int2 center = int2(floor(aoPixel+0.5));
    float total = 0, weighted = 0;
    [loop] for (int y=-1;y<=1;++y) [loop] for (int x=-1;x<=1;++x)
    {
        int2 samplePixel = center+int2(x,y);
        if (any(samplePixel<0) || any(samplePixel>=int2(aoWidth,aoHeight))) continue;
        float4 aoSample = Occlusion().Load(int3(samplePixel,0));
        float distanceZ = abs(aoSample.y-position.z);
        // Bilateral blur/upsampling must reject other surfaces and sky.
        if (aoSample.a==0 || distanceZ>max(Radius*0.1,Bias*2)) continue;
        float2 offset = float2(samplePixel)-aoPixel;
        float weight = exp(-0.5*dot(offset,offset)-distanceZ*4.0/Radius);
        weighted += aoSample.x*weight;
        total += weight;
    }
    float visibility = total>0 ? weighted/total : 1.0;
    float4 scene = Scene().Load(int3(pixel,0));
    return float4(scene.rgb*lerp(1.0,visibility,Strength),scene.a);
}
