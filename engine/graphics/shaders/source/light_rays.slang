#include "resource_access.slangh"
// Based on Electronic Arts' Kane's Wrath PostFX_LightRays.fx (2006):
// https://github.com/electronicarts/CnC_Modding_Support/blob/main/Kanes%20Wrath/Shaders/PostFX_LightRays.fx
// Depth-limited world-space integration of cloud light and shadow visibility,
// followed by an additive overlay. No D3D9 half-texel offset is needed here.
#include "environment_lighting.slangh"

struct VertexInput { float3 position : POSITION; float4 color : COLOR0; float2 uv : TEXCOORD0; };
struct VertexOutput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
cbuffer LightRaysParameters : register(b0)
{
    row_major float4x4 InverseViewProjection;
    float4 ShroudProjection;
    float3 Brightness;
    uint Operation;
    uint ShroudEnabled;
    float3 Padding;
};
GRAPHICS_TEXTURE_2D(Scene, float4, 0)
GRAPHICS_TEXTURE_2D(SceneDepth, float, 1)
GRAPHICS_TEXTURE_2D(Rays, float4, 2)
GRAPHICS_TEXTURE_2D(Shroud, float4, 3)
GRAPHICS_SAMPLER(LinearClamp, 0)

[shader("vertex")] VertexOutput light_rays_vertex(VertexInput input)
{
    VertexOutput output;
    output.position = float4(input.position,1);
    output.uv = input.uv;
    return output;
}

float3 Unproject(float2 uv, float depth)
{
    float4 world = mul(InverseViewProjection,float4(uv*float2(2,-2)+float2(-1,1),depth,1));
    return world.xyz / world.w;
}

float RayShadowSample(Texture2D<float> map, float3 projected)
{
    float2 uv = projected.xy*float2(0.5,-0.5)+0.5;
    if (any(uv < 0) || any(uv > 1) || projected.z < 0 || projected.z > 1) return 1;
    uint width, height;
    map.GetDimensions(width,height);
    int2 texel = clamp(int2(uv*float2(width,height)),int2(0,0),int2(width-1,height-1));
    return projected.z - ShadowOptions.y <= map.Load(int3(texel,0)) ? 1 : 0;
}

// Volume samples have no receiving surface or screen-space slope. Like the
// reference shader, compare one shadow sample, without surface PCF derivatives.
float RayShadow(float3 world)
{
    uint count = uint(ShadowOptions.x);
    if (count == 0) return 1;
    float depth = dot(ShadowViewDepth,float4(world,1));
    if (depth > ShadowSplits[count-1]) return 1;
    uint cascade = 0;
    while (cascade+1 < count && depth > ShadowSplits[cascade]) ++cascade;
    float4 clip = mul(ShadowViewProjection[cascade],float4(world,1));
    float3 projected = clip.xyz / clip.w;
    if (cascade == 0) return RayShadowSample(EnvironmentShadowTextures[0],projected);
    if (cascade == 1) return RayShadowSample(EnvironmentShadowTextures[1],projected);
    if (cascade == 2) return RayShadowSample(EnvironmentShadowTextures[2],projected);
    return RayShadowSample(EnvironmentShadowTextures[3],projected);
}

float3 UpsampleRays(float2 uv, float ray_length)
{
    uint width, height;
    Rays().GetDimensions(width,height);
    float2 position = uv*float2(width,height)-0.5;
    int2 base = int2(floor(position));
    float2 fraction = frac(position);
    float3 sum = 0;
    float weights = 0;
    [unroll] for (int y=0;y<2;++y) [unroll] for (int x=0;x<2;++x) {
        int2 texel = clamp(base+int2(x,y),int2(0,0),int2(width-1,height-1));
        float4 sample = Rays().Load(int3(texel,0));
        float weight = (x != 0 ? fraction.x : 1-fraction.x)*(y != 0 ? fraction.y : 1-fraction.y);
        // Avoid carrying long background rays across a foreground silhouette.
        weight *= exp2(-abs(sample.a-ray_length)/max(1.0,ray_length*0.02));
        sum += sample.rgb*weight;
        weights += weight;
    }
    return weights > 0.00001 ? sum/weights : float3(0,0,0);
}

[shader("pixel")] float4 light_rays_pixel(VertexOutput input) : SV_TARGET
{
    uint width, height;
    SceneDepth().GetDimensions(width,height);
    int2 pixel = clamp(int2(input.uv*float2(width,height)),int2(0,0),int2(width-1,height-1));
    float depth = SceneDepth().Load(int3(pixel,0));
    float3 start = Unproject(input.uv,0);
    float3 end = Unproject(input.uv,depth);
    float ray_length = distance(start,end);
    if (Operation == 1) {
        float4 scene = Scene().Load(int3(pixel,0));
        // Unwritten background does not define a finite illuminated surface.
        if (depth >= 1) return scene;
        float visibility = 1;
        if (ShroudEnabled != 0)
            visibility = Shroud().SampleLevel(LinearClamp(),end.xy*ShroudProjection.xy+ShroudProjection.zw,0).r;
        return float4(scene.rgb+UpsampleRays(input.uv,ray_length)*visibility,scene.a);
    }
    if (depth >= 1) return float4(0,0,0,0);
    const uint steps = 100;
    float3 delta = (end-start)/steps;
    float3 world = start;
    float3 light = 0;
    [loop] for (uint step=0;step<steps;++step) {
        world += delta;
        light += CloudVisibility(world)*RayShadow(world);
    }
    // Keep the reference's distance response near the camera, but cap its
    // energy in this display-color framebuffer. Unbounded far-plane distances
    // otherwise turn the additive overlay into an opaque white wash.
    return float4(light*(Brightness/400)*(min(ray_length,400.0)/steps),ray_length);
}
