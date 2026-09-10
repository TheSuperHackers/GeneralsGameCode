#include "resource_access.slangh"
#include "water_constants.slangh"

struct UnderwaterVertexInput
{
    float3 position : POSITION0;
};

struct UnderwaterVertexOutput
{
    float4 position : SV_POSITION;
    float water_level : TEXCOORD0;
};

UnderwaterVertexOutput Transform_Underwater_Vertex(UnderwaterVertexInput input, float4x4 world)
{
    UnderwaterVertexOutput output;
    float4 position = mul(world,float4(input.position,1));
    output.position = mul(Projection,mul(View,position));
    output.water_level = position.z;
    return output;
}

struct UnderwaterInstance { row_major float4x4 world; };
GRAPHICS_STORAGE_BUFFER(UnderwaterInstances, UnderwaterInstance, 127)
[shader("vertex")] UnderwaterVertexOutput UnderwaterVertexMain(UnderwaterVertexInput input)
{
    return Transform_Underwater_Vertex(input,World);
}
[shader("vertex")] UnderwaterVertexOutput UnderwaterVertexInstanced(UnderwaterVertexInput input, uint instance : SV_InstanceID)
{
    return Transform_Underwater_Vertex(input,UnderwaterInstances()[instance].world);
}

GRAPHICS_TEXTURE_2D(UnderwaterSceneTexture, float4, 5)
GRAPHICS_TEXTURE_2D(UnderwaterDepthTexture, float4, 8)
GRAPHICS_TEXTURE_2D(OceanCausticsTexture, float4, 9)
GRAPHICS_TEXTURE_2D(OceanDepthLutTexture, float4, 10)
GRAPHICS_SAMPLER(UnderwaterSceneSampler, 5)
GRAPHICS_SAMPLER(UnderwaterDepthSampler, 8)
GRAPHICS_SAMPLER(OceanCausticsSampler, 9)
GRAPHICS_SAMPLER(OceanDepthLutSampler, 10)

float3 Shade_Underwater(float3 scene, float3 position, float water_level, float3 normal)
{
    float depth_range = water_level > 0.0 ? water_level : 200.0;
    float depth_uv = saturate((1.0 + water_level - position.z) / depth_range);
    float4 depth_lut = OceanDepthLutTexture().SampleLevel(OceanDepthLutSampler(),depth_uv.xx,0);
    scene *= pow(max(depth_lut.rgb,0.0),2.2);
    scene *= 1.0 - 0.1 * depth_uv;
    float3 light = WaterEnvironmentFrame[2].xyz;
    float light_z = abs(light.z) > 0.001 ? light.z : 0.001;
    float2 base = (position.xy - position.z * light.xy / light_z) * 0.004;
    const float cosine = 0.4067366;
    const float sine = 0.9135455;
    float2 uv0 = float2(dot(base,float2(cosine,sine)),dot(base,float2(-sine,cosine)));
    float2 uv1 = float2(dot(base,float2(cosine,-sine)),dot(base,float2(sine,cosine)));
    uv0.x += WaterAnimation.z * 0.12;
    uv1.x += WaterAnimation.z * 0.12;
    float2 distortion = (OceanCausticsTexture().Sample(OceanCausticsSampler(),uv0).rg
        + OceanCausticsTexture().Sample(OceanCausticsSampler(),uv1).rg - 1.0) * 0.3;
    float3 caustics;
    caustics.r = OceanCausticsTexture().Sample(OceanCausticsSampler(),uv0 + distortion * 0.9).a
        * OceanCausticsTexture().Sample(OceanCausticsSampler(),-uv1 - distortion * 0.9).a;
    caustics.g = OceanCausticsTexture().Sample(OceanCausticsSampler(),uv0 + distortion).a
        * OceanCausticsTexture().Sample(OceanCausticsSampler(),-uv1 - distortion).a;
    caustics.b = OceanCausticsTexture().Sample(OceanCausticsSampler(),uv0 + distortion * 1.1).a
        * OceanCausticsTexture().Sample(OceanCausticsSampler(),-uv1 - distortion * 1.1).a;
    caustics = caustics * caustics * depth_lut.a * lerp(WaterSunColor.rgb,1.0,0.75)
        * saturate(dot(normal,light));
    return scene + pow(max(caustics * 5.0,0.0),2.2);
}

[shader("pixel")] float4 UnderwaterPixelMain(UnderwaterVertexOutput input) : SV_TARGET
{
    uint width, height;
    UnderwaterSceneTexture().GetDimensions(width,height);
    float2 uv = input.position.xy / float2(width,height);
    float depth = UnderwaterDepthTexture().SampleLevel(UnderwaterDepthSampler(),uv,0).r;
    float4 projected = mul(WaterInverseViewProjection,float4(uv * float2(2,-2) + float2(-1,1),depth,1));
    float3 position = projected.xyz / projected.w;
    float3 normal = cross(ddy(position),ddx(position));
    normal /= max(length(normal),0.00001);
    if (position.z > input.water_level)
        discard;
    float3 scene = UnderwaterSceneTexture().SampleLevel(UnderwaterSceneSampler(),uv,0).rgb;
    float3 linear_scene = pow(max(scene,0.0),2.2);
    float3 color = Shade_Underwater(linear_scene,position,input.water_level,normal);
    float shoreline_opacity = WaterSurfaceOptions.y > 0.0 ?
        saturate(1.05 * (input.water_level - position.z) / WaterSurfaceOptions.y) : 1.0;
    color = lerp(linear_scene,color,shoreline_opacity);
    return float4(pow(max(color,0.0),1.0/2.2),1);
}
