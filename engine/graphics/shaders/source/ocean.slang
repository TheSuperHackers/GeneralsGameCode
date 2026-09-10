#include "resource_access.slangh"
#include "environment_lighting.slangh"
#include "water_constants.slangh"

struct OceanVertexInput
{
    float3 position : POSITION0;
    float4 color0 : COLOR0;
    float2 texcoord0 : TEXCOORD0;
};

struct OceanVertexOutput
{
    float4 position : SV_POSITION;
    float4 color0 : COLOR0;
    float2 surface_uv : TEXCOORD0;
    float2 normal_uv0 : TEXCOORD1;
    float2 normal_uv1 : TEXCOORD2;
    float2 foam_uv0 : TEXCOORD3;
    float2 foam_uv1 : TEXCOORD4;
    float2 shroud_uv : TEXCOORD5;
    float2 displacement_uv : TEXCOORD6;
    float3 projective_position : TEXCOORD7;
    float3 world_position : TEXCOORD8;
    float4 depth_projection : TEXCOORD9;
    float foam_height : TEXCOORD10;
    float water_level : TEXCOORD11;
};

float2 Rotate_Ocean_UV(float2 uv)
{
    const float cosine = 0.5735764;
    const float sine = 0.8191520;
    return float2(
        dot(uv, float2(cosine, -sine)),
        dot(uv, float2(sine, cosine)));
}

GRAPHICS_TEXTURE_2D(OceanDisplacementTexture, float4, 1)
GRAPHICS_SAMPLER(OceanDisplacementSampler, 1)

OceanVertexOutput Transform_Ocean_Vertex(OceanVertexInput input, float4x4 world)
{
    OceanVertexOutput output;
    float4 world_position = mul(world, float4(input.position, 1.0));
    output.water_level = world_position.z;
    const float2 material_position = world_position.xy;
    const float phase = WaterAnimation.z;

    float2 displacement_uv = (world_position.xy -
        WaterDisplacementDomain.xy) /
        max(abs(WaterDisplacementDomain.zw), float2(1.0, 1.0));
    displacement_uv = saturate(float2(displacement_uv.x,
        1.0 - displacement_uv.y));
    float3 wave_displacement = OceanDisplacementTexture().SampleLevel(
        OceanDisplacementSampler(), displacement_uv, 0.0).xyz;
    wave_displacement = min(wave_displacement, 0.13);
    wave_displacement.x *= -1.0;
    world_position.xyz += wave_displacement * 300.0;

    output.foam_height = max(wave_displacement.z * 300.0 - 1.0, 0.0);
    float4 camera_position = mul(View, world_position);
    float4 clip_position = mul(Projection, camera_position);
    output.position = clip_position;
    output.color0 = input.color0;

    output.surface_uv = world_position.xy * 0.004 + WaterAnimation.xy * 0.1;
    float2 normal_base = material_position * 0.0025;
    output.normal_uv0 = float2(
        dot(normal_base, float2(0.5735764, 0.8191520)),
        dot(normal_base, float2(-0.8191520, 0.5735764))) +
        float2(phase * 0.015, 0.0);
    output.normal_uv1 = Rotate_Ocean_UV(normal_base) +
        float2(phase * 0.015, 0.0);
    output.foam_uv0 = material_position * 0.005 +
        float2(phase * 0.011, 0.0);
    output.foam_uv1 = material_position * 0.004 +
        float2(-phase * 0.011, 0.0);
    output.shroud_uv = world_position.xy * WaterShroudProjection.xy +
        WaterShroudProjection.zw;
    output.displacement_uv = displacement_uv;

    output.projective_position = float3(
        clip_position.x * 0.5 + clip_position.w * 0.5,
        clip_position.w * 0.5 - clip_position.y * 0.5,
        clip_position.w);
    output.world_position = world_position.xyz;
    output.depth_projection = float4(
        Projection[2][2], Projection[2][3],
        Projection[3][2], Projection[3][3]);
    return output;
}

struct OceanInstance { row_major float4x4 world; };
GRAPHICS_STORAGE_BUFFER(OceanInstances, OceanInstance, 127)
[shader("vertex")] OceanVertexOutput OceanVertexMain(OceanVertexInput input)
{
    return Transform_Ocean_Vertex(input, World);
}
[shader("vertex")] OceanVertexOutput OceanVertexInstanced(OceanVertexInput input, uint instance : SV_InstanceID)
{
    return Transform_Ocean_Vertex(input, OceanInstances()[instance].world);
}

GRAPHICS_TEXTURE_2D(OceanSurfaceTexture, float4, 0)
GRAPHICS_TEXTURE_2D(OceanNormalTexture, float4, 2)
GRAPHICS_TEXTURE_2D(OceanFoamTexture, float4, 3)
GRAPHICS_TEXTURE_2D(OceanReflectionTexture, float4, 4)
GRAPHICS_TEXTURE_2D(OceanRefractionTexture, float4, 5)
GRAPHICS_TEXTURE_2D(OceanEnvironmentTexture, float4, 6)
GRAPHICS_TEXTURE_2D(OceanShroudTexture, float4, 7)
GRAPHICS_TEXTURE_2D(OceanSceneDepthTexture, float4, 8)
GRAPHICS_TEXTURE_2D(OceanCausticsTexture, float4, 9)
GRAPHICS_TEXTURE_2D(OceanDepthLutTexture, float4, 10)
GRAPHICS_SAMPLER(OceanSurfaceSampler, 0)
GRAPHICS_SAMPLER(OceanNormalSampler, 2)
GRAPHICS_SAMPLER(OceanFoamSampler, 3)
GRAPHICS_SAMPLER(OceanReflectionSampler, 4)
GRAPHICS_SAMPLER(OceanRefractionSampler, 5)
GRAPHICS_SAMPLER(OceanEnvironmentSampler, 6)
GRAPHICS_SAMPLER(OceanShroudSampler, 7)
GRAPHICS_SAMPLER(OceanSceneDepthSampler, 8)
GRAPHICS_SAMPLER(OceanCausticsSampler, 9)
GRAPHICS_SAMPLER(OceanDepthLutSampler, 10)

struct OceanPixelInput
{
    float4 position : SV_POSITION;
    float4 color0 : COLOR0;
    float2 surface_uv : TEXCOORD0;
    float2 normal_uv0 : TEXCOORD1;
    float2 normal_uv1 : TEXCOORD2;
    float2 foam_uv0 : TEXCOORD3;
    float2 foam_uv1 : TEXCOORD4;
    float2 shroud_uv : TEXCOORD5;
    float2 displacement_uv : TEXCOORD6;
    float3 projective_position : TEXCOORD7;
    float3 world_position : TEXCOORD8;
    float4 depth_projection : TEXCOORD9;
    float foam_height : TEXCOORD10;
    float water_level : TEXCOORD11;
};

float3 Decode_Water_Normal(float4 sample_value)
{
    return sample_value.rgb * 2.0 - 1.0;
}

float2 Project_Water_Position(float3 projective_position)
{
    return projective_position.xy / max(abs(projective_position.z), 0.000001);
}

float2 Environment_UV(float3 direction)
{
    const float pi = 3.14159265;
    float longitude = atan2(direction.y, direction.x) / (2.0 * pi);
    float latitude = asin(clamp(direction.z, -1.0, 1.0)) / pi;
    return float2(longitude + 0.5, 0.5 - latitude);
}

float Water_Fog_Factor(float3 world_position)
{
    if (WaterFogState.z < 0.5)
        return 0.0;
    float distance_to_camera = length(WaterCameraPosition.xyz -
        world_position);
    return saturate((distance_to_camera - WaterFogState.x) /
        max(WaterFogState.y - WaterFogState.x, 0.0001));
}

float Linearize_Ocean_Depth(float depth, float4 projection)
{
    float numerator = projection.y - depth * projection.w;
    float denominator = depth * projection.z - projection.x;
    return abs(numerator / (abs(denominator) > 0.00001 ? denominator :
        0.00001));
}

[shader("pixel")] float4 OceanPixelMain(OceanPixelInput input) : SV_TARGET
{
    float phase = WaterAnimation.z;
    float4 surface0 = OceanSurfaceTexture().Sample(OceanSurfaceSampler(),
        input.surface_uv);
    float4 surface1 = OceanSurfaceTexture().Sample(OceanSurfaceSampler(),
        input.surface_uv * 1.37 + float2(0.23, -0.17));
    float3 surface = lerp(surface0.rgb, surface1.rgb, 0.35);

    float3 static_displacement0 = Decode_Water_Normal(OceanNormalTexture().Sample(
        OceanNormalSampler(), input.normal_uv0));
    float3 static_displacement1 = Decode_Water_Normal(OceanNormalTexture().Sample(
        OceanNormalSampler(), input.normal_uv1));
    float3 static_displacement = static_displacement0 +
        static_displacement1;
    uint displacement_width, displacement_height;
    OceanDisplacementTexture().GetDimensions(displacement_width, displacement_height);
    float2 displacement_texel = 1.0 / float2(displacement_width, displacement_height);
    float height_center = OceanDisplacementTexture().SampleLevel(
        OceanDisplacementSampler(), input.displacement_uv, 0.0).z;
    float height_x = OceanDisplacementTexture().SampleLevel(
        OceanDisplacementSampler(), input.displacement_uv +
            float2(displacement_texel.x, 0.0), 0.0).z;
    float height_y = OceanDisplacementTexture().SampleLevel(
        OceanDisplacementSampler(), input.displacement_uv +
            float2(0.0, displacement_texel.y), 0.0).z;
    float3 displacement_normal = float3(height_x - height_center,
        height_y - height_center, 0.01) * 50.0;
    displacement_normal = normalize(displacement_normal +
        static_displacement * 0.2);
    float3 surface_normal = normalize(displacement_normal * 50.0 +
        float3(0.0, 0.0, 1.0));
    float3 view_direction = normalize(WaterCameraPosition.xyz -
        input.world_position);

    float fresnel = 1.0 - dot(surface_normal, view_direction);

    float3 color = pow(max(surface * input.color0.rgb * WaterTint.rgb,0.0),2.2);
    color *= 0.55 + surface_normal.z * 0.45;

    float3 foam = OceanFoamTexture().Sample(OceanFoamSampler(), input.foam_uv0).rgb *
        OceanFoamTexture().Sample(OceanFoamSampler(), input.foam_uv1).rgb;
    foam = foam * foam * clamp(input.foam_height * 0.2, 0.025, 3.0);

    float2 projective_uv = Project_Water_Position(input.projective_position);

    uint capture_width, capture_height;
    OceanRefractionTexture().GetDimensions(capture_width,capture_height);
    float2 capture_uv = input.position.xy / max(float2(capture_width,capture_height),1.0);
    float2 distorted_uv = saturate(capture_uv + surface_normal.xy * 0.15);
    float3 shroud = WaterEffects.y > 0.5 ?
        OceanShroudTexture().Sample(OceanShroudSampler(), input.shroud_uv).rgb : float3(1,1,1);
    color *= shroud;

    float water_depth_fade = 0.0;
    float3 refraction_base = color;
    float authored_alpha = saturate(input.color0.a * WaterTint.a);
    float water_alpha = authored_alpha;
    if (WaterSurfaceOptions.w > 0.5)
    {
        uint depth_width, depth_height;
        OceanSceneDepthTexture().GetDimensions(depth_width,depth_height);
        float2 depth_uv = input.position.xy / max(float2(depth_width,depth_height),1.0);
        float scene_depth = OceanSceneDepthTexture().SampleLevel(
            OceanSceneDepthSampler(), saturate(depth_uv), 0.0).r;
        if (scene_depth >= 0.9999)
        {

            water_depth_fade = 1.0;
            water_alpha = saturate(WaterSurfaceOptions.z);
        }
        else
        {
            float water_view_depth = Linearize_Ocean_Depth(input.position.z,
                input.depth_projection);
            float scene_view_depth = Linearize_Ocean_Depth(scene_depth,
                input.depth_projection);
            float thickness = max(scene_view_depth - water_view_depth, 0.0);
            if (WaterEffects.w > 0.5) {
                float4 scene_position = mul(WaterInverseViewProjection,
                    float4(depth_uv * float2(2,-2) + float2(-1,1),scene_depth,1));
                thickness = max(input.water_level - scene_position.z / scene_position.w,0.0);
            }
            water_depth_fade = saturate(thickness /
                max(WaterSurfaceOptions.y, 0.001));
            if (WaterEffects.w > 0.5)
                water_depth_fade = saturate(water_depth_fade * 1.05);
            if (WaterEffects.w > 0.5 && WaterSurfaceOptions.y <= 0.0)
                water_depth_fade = 1.0;

            water_alpha = water_depth_fade *
                saturate(WaterSurfaceOptions.z);
        }
    }

    if (WaterEffects.z > 0.5)
    {
        float3 refracted = OceanRefractionTexture().SampleLevel(
            OceanRefractionSampler(), distorted_uv, 0.0).rgb;
        float3 refracted_base = OceanRefractionTexture().SampleLevel(
            OceanRefractionSampler(), saturate(capture_uv), 0.0).rgb;
        refracted = pow(max(refracted,0.0),2.2);
        refracted_base = pow(max(refracted_base,0.0),2.2);
        refraction_base = refracted_base;

        bool reject_refraction = distance(refracted, refracted_base) >= 0.1;
        if (reject_refraction) {
            refracted = refracted_base;
            distorted_uv = saturate(capture_uv);
        }

        color = refracted;
        if (WaterEffects.w > 0.5)
            water_alpha = 1.0;
    }

    if (WaterEffects.x > 0.001)
    {
        float2 reflection_uv = saturate(projective_uv + surface_normal.xy * 0.2);
        float3 reflection = OceanReflectionTexture().SampleLevel(
            OceanReflectionSampler(), reflection_uv, 0.0).rgb;
        color += shroud * pow(max(reflection,0.0),2.2) * WaterEffects.x * 0.5;
    }

    float3 environment_direction = reflect(-view_direction, surface_normal);
    environment_direction = float3(dot(WaterEnvironmentFrame[0].xyz,environment_direction),
        dot(WaterEnvironmentFrame[1].xyz,environment_direction),
        dot(WaterEnvironmentFrame[2].xyz,environment_direction));
    float4 environment_sample = OceanEnvironmentTexture().SampleLevel(
        OceanEnvironmentSampler(), saturate(Environment_UV(
            environment_direction)), 0.0);

    float3 environment = environment_sample.rgb * environment_sample.a * 4.0;
    color += shroud * (environment * fresnel * 2.0 *
        DirectionalVisibility(input.world_position) + foam) * WaterSunColor.rgb;

    if (WaterEffects.z > 0.5 && WaterEffects.w > 0.5 && WaterSurfaceOptions.w > 0.5)
        color = lerp(refraction_base,color,water_depth_fade);

    color = pow(max(color,0.0),1.0/2.2);
    color = lerp(color, FogColor.rgb, Water_Fog_Factor(input.world_position));
    return float4(saturate(color), water_alpha);
}
