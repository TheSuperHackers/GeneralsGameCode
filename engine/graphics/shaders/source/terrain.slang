#include "resource_access.slangh"
#include "environment_lighting.slangh"
GRAPHICS_TEXTURE_2D(TerrainDiffuseTexture, float4, 0)
GRAPHICS_TEXTURE_2D(TerrainBlendTexture, float4, 1)
GRAPHICS_TEXTURE_2D(TerrainCloudTexture, float4, 2)
GRAPHICS_TEXTURE_2D(TerrainLightMapTexture, float4, 3)
GRAPHICS_TEXTURE_2D(TerrainShroudTexture, float4, 4)

GRAPHICS_SAMPLER(TerrainDiffuseSampler, 0)
GRAPHICS_SAMPLER(TerrainBlendSampler, 1)
GRAPHICS_SAMPLER(TerrainCloudSampler, 2)
GRAPHICS_SAMPLER(TerrainLightMapSampler, 3)
GRAPHICS_SAMPLER(TerrainShroudSampler, 4)

struct TerrainVertexInput
{
    float3 position : POSITION0;
    float4 color0 : COLOR0;
    float2 texcoord0 : TEXCOORD0;
    float2 texcoord1 : TEXCOORD1;
    float3 normal : NORMAL0;
};

struct TerrainVertexOutput
{
    float4 position : SV_POSITION;
    float4 color0 : COLOR0;
    float2 diffuse_uv : TEXCOORD0;
    float2 blend_uv : TEXCOORD1;
    float2 cloud_uv : TEXCOORD2;
    float2 lightmap_uv : TEXCOORD3;
    float2 shroud_uv : TEXCOORD4;
    float3 world_position : TEXCOORD5;
};

struct TerrainLight
{
    float4 position_range;
    float4 diffuse_inner;
    float4 ambient_kind;
    float4 direction;
};

cbuffer TerrainConstants : register(b0)
{
    row_major float4x4 ViewProjection;
    float4 TerrainCloudProjection;
    float4 TerrainLightMapProjection;
    float4 TerrainShroudProjection;
    float4 TerrainFeatures;
    float4 TerrainLighting;
    float4 TerrainOptions;
    float4 TerrainLightOptions;
    TerrainLight TerrainLights[20];
};

float2 TerrainProjectedCoordinate(float3 world_position, float4 projection)
{
    return world_position.xy * projection.xy + projection.zw;
}

[shader("vertex")] TerrainVertexOutput terrain_vertex(TerrainVertexInput input)
{
    TerrainVertexOutput output;
    float4 world_position = float4(input.position, 1.0);

    output.position = mul(ViewProjection, world_position);
    output.color0 = input.color0;
    float3 lighting = input.color0.rgb;
    for (int index = 0; index < min(int(TerrainLightOptions.x), 20); ++index)
    {
        TerrainLight light = TerrainLights[index];
        float3 direction = light.direction.xyz;
        float attenuation = 1.0;
        if (light.ambient_kind.w < 0.5)
        {
            direction = input.position - light.position_range.xyz;
            float distance = length(direction);
            if (distance >= light.position_range.w || light.diffuse_inner.w < 0.1) continue;
            attenuation = saturate(1.0 - (distance - light.diffuse_inner.w) /
                max(light.position_range.w - light.diffuse_inner.w, 0.00001));
            direction = distance > 0.0 ? direction / distance : float3(0, 0, -1);
        }
        float shade = saturate(dot(-direction, input.normal) * attenuation);
        lighting += shade * light.diffuse_inner.rgb + attenuation * light.ambient_kind.rgb;
    }
    if (TerrainLightOptions.x > 0.0)
        output.color0.rgb = floor(saturate(lighting) * 255.0) / 255.0;
    output.diffuse_uv = input.texcoord0;
    output.blend_uv = input.texcoord1;
    output.cloud_uv = TerrainProjectedCoordinate(world_position.xyz,
        TerrainCloudProjection);
    output.lightmap_uv = TerrainProjectedCoordinate(world_position.xyz,
        TerrainLightMapProjection);
    output.shroud_uv = TerrainProjectedCoordinate(world_position.xyz,
        TerrainShroudProjection);
    output.world_position = world_position.xyz;
    return output;
}

struct TerrainPixelInput
{
    float4 position : SV_POSITION;
    float4 color0 : COLOR0;
    float2 diffuse_uv : TEXCOORD0;
    float2 blend_uv : TEXCOORD1;
    float2 cloud_uv : TEXCOORD2;
    float2 lightmap_uv : TEXCOORD3;
    float2 shroud_uv : TEXCOORD4;
    float3 world_position : TEXCOORD5;
};

[shader("pixel")] float4 terrain_pixel(TerrainPixelInput input) : SV_TARGET
{
    ClipEnvironment(input.world_position);
    if (TerrainFeatures.w > 4.5)
        return float4(0.0, 0.0, 0.0, TerrainShroudTexture().SampleLevel(TerrainShroudSampler(), input.shroud_uv, 0.0).a);

	if (TerrainFeatures.w > 3.5)
		return float4(0.0, 0.0, 0.0, TerrainDiffuseTexture().SampleLevel(TerrainDiffuseSampler(), input.diffuse_uv, 0.0).a);

	if (TerrainFeatures.z > 0.5)
	{
		return float4(input.color0.rgb * TerrainLighting.rgb, 1.0);
	}

    float4 color = float4(0.0, 0.0, 0.0, 1.0);
    if (TerrainFeatures.w > 2.5)
    {
        // Third-layer tiles are an alpha overlay over the already-composed
        // terrain. Their vertex alpha is the overlay coverage.
        color = TerrainDiffuseTexture().Sample(TerrainDiffuseSampler(), input.diffuse_uv);
        color.rgb *= input.color0.rgb;
        color.a = input.color0.a;
    }
    else if (TerrainFeatures.w > 1.5)
    {
        // The shroud texture stores its modulation in RGB (R5G6B5 in the
        // release path), so this pass is intentionally independent of alpha.
        float3 shroud = TerrainShroudTexture().SampleLevel(TerrainShroudSampler(),
            input.shroud_uv, 0.0).rgb;
        return float4(shroud * TerrainLighting.rgb, 1.0);
    }

    else
    {
        float4 base = TerrainDiffuseTexture().Sample(TerrainDiffuseSampler(), input.diffuse_uv);
        float4 blend = TerrainBlendTexture().Sample(TerrainBlendSampler(), input.blend_uv);
        color = lerp(base, blend, saturate(input.color0.a));
        color.rgb *= input.color0.rgb;
    }

    if (CloudOffsetStrength.w > 0 || ShadowOptions.x > 0)
    {
        float3 illumination = max(input.color0.rgb, 0.00001);
        float3 ambient = min(EnvironmentAmbient.rgb, illumination);
        color.rgb *= (ambient + (illumination - ambient) *
            DirectionalVisibility(input.world_position)) / illumination;
    }
    else if (TerrainFeatures.x > 0.5)
    {
        color.rgb *= TerrainCloudTexture().SampleLevel(TerrainCloudSampler(),
            input.cloud_uv, 0.0).rgb;
    }
    if (TerrainFeatures.y > 0.5)
    {
        color.rgb *= TerrainLightMapTexture().SampleLevel(TerrainLightMapSampler(),
            input.lightmap_uv, 0.0).rgb;
    }
    color.rgb *= TerrainLighting.rgb;

    // Shroud modulation shares the surface depth and coverage.
    if (TerrainOptions.y > 0.5)
    {
        color.rgb *= TerrainShroudTexture().SampleLevel(TerrainShroudSampler(),
            input.shroud_uv, 0.0).rgb;
    }
    return color;
}
