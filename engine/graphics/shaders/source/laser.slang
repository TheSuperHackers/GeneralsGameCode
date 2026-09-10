#include "resource_access.slangh"
cbuffer LaserConstants : register(b0)
{
    row_major float4x4 ViewProjection;
    float4 ShroudProjection;
};

struct LaserVertexInput
{
    float3 position : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float2 detail_uv : TEXCOORD1;
    float3 normal : NORMAL0;
    float2 distortion : TEXCOORD2;
};

struct LaserVertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float2 detail_uv : TEXCOORD1;
    float3 normal : TEXCOORD2;
    float2 shroud_uv : TEXCOORD3;
    float distortion : TEXCOORD4;
};

GRAPHICS_TEXTURE_2D(BaseTexture, float4, 0)
GRAPHICS_TEXTURE_2D(DetailTexture, float4, 1)
GRAPHICS_TEXTURE_2D(ShroudTexture, float4, 2)
GRAPHICS_TEXTURE_2D(BackgroundTexture, float4, 3)
GRAPHICS_SAMPLER(BaseSampler, 0)
GRAPHICS_SAMPLER(DetailSampler, 1)
GRAPHICS_SAMPLER(ShroudSampler, 2)
GRAPHICS_SAMPLER(BackgroundSampler, 3)

float3 BiasShroud(float3 value)
{
    return max((value - 0.75.xxx) * 4.0.xxx, 0.0.xxx);
}

float BeamFootprint(float across)
{
    const float edge_distance = saturate(1.0 - abs(across * 2.0 - 1.0));
    return smoothstep(0.0, 1.0, edge_distance);
}

LaserVertexOutput laser_vertex(LaserVertexInput input)
{
    LaserVertexOutput output;
    output.position = mul(ViewProjection, float4(input.position, 1.0));
    output.color = input.color;
    output.uv = input.uv;
    output.detail_uv = input.detail_uv;
    // The CPU supplies the camera-facing side tangent in world space.  The
    // projection-free matrix transform keeps its screen orientation available
    // to the distortion pass without inventing a fixed screen-space tangent.
    output.normal = mul((float3x3)ViewProjection, input.normal);
    output.shroud_uv = input.position.xy * ShroudProjection.xy +
        ShroudProjection.zw;
    output.distortion = max(input.distortion.x, 0.0);
    return output;
}

[shader("vertex")] LaserVertexOutput laser_core_vertex(LaserVertexInput input)
{
    return laser_vertex(input);
}

[shader("pixel")] float4 laser_core_pixel(LaserVertexOutput input) : SV_TARGET
{
    // Match the authored laser shader: both RGBA textures participate in the
    // product, while the shroud is biased independently for effects.
    const float4 base = BaseTexture().Sample(BaseSampler(), input.uv);
    const float4 detail = DetailTexture().Sample(DetailSampler(), input.detail_uv);
    const float3 shroud = BiasShroud(ShroudTexture().Sample(ShroudSampler(),
        input.shroud_uv).rgb);
    const float footprint = BeamFootprint(input.uv.x);
    float4 result = base * detail * input.color;
    result.rgb *= shroud;
    result.a *= footprint;
    return result;
}

[shader("vertex")] LaserVertexOutput laser_distortion_vertex(
    LaserVertexInput input)
{
    return laser_vertex(input);
}

[shader("pixel")] float4 laser_distortion_pixel(
    LaserVertexOutput input) : SV_TARGET
{
    const float4 base = BaseTexture().Sample(BaseSampler(), input.uv);
    const float4 normal_sample_a = DetailTexture().Sample(DetailSampler(),
        input.detail_uv * float2(1.71, 0.93) + float2(0.13, 0.07));
    const float4 normal_sample_b = DetailTexture().Sample(DetailSampler(),
        input.detail_uv * float2(0.83, 1.89) + float2(0.47, 0.31));

    const float3 bump_a = normalize(normal_sample_a.rgb * 2.0 - 1.0);
    const float3 bump_b = normalize(normal_sample_b.rgb * 2.0 - 1.0);
    const float3 bump_normal = normalize(bump_a + bump_b);

    // Build a view-facing tangent frame from the actual beam side tangent.
    // The second axis follows the projected beam, so the normal maps do not
    // always distort in fixed screen X/Y directions.
    float3 view_side = normalize(input.normal);
    if (dot(view_side, view_side) < 0.0001)
        view_side = float3(1.0, 0.0, 0.0);
    const float3 view_forward = float3(0.0, 0.0, 1.0);
    float3 view_along = normalize(cross(view_forward, view_side));
    if (dot(view_along, view_along) < 0.0001)
        view_along = float3(0.0, 1.0, 0.0);

    const float2 distortion_direction = normalize(
        view_side.xy * bump_normal.x + view_along.xy * bump_normal.y);
    const float3 biased_shroud = BiasShroud(ShroudTexture().Sample(
        ShroudSampler(), input.shroud_uv).rgb);
    const float shroud_mask = saturate(min(biased_shroud.r,
        min(biased_shroud.g, biased_shroud.b)));
    const float normal_mask = saturate(normal_sample_a.a * normal_sample_b.a);
    const float mask = saturate(base.a * input.color.a
        * BeamFootprint(input.uv.x) * normal_mask * shroud_mask
        * input.distortion);

    uint background_width;
    uint background_height;
    BackgroundTexture().GetDimensions(background_width, background_height);
    const float2 dimensions = float2(max(background_width, 1u),
        max(background_height, 1u));
    const float2 screen_uv = input.position.xy / dimensions;
    // Keep the displacement bounded to a small, resolution-independent pixel
    // radius.  A zero strength or a fogged/transparent beam has zero mask.
    const float2 pixel_offset = distortion_direction * (1.5 * mask);
    const float2 distorted_uv = saturate(screen_uv + pixel_offset / dimensions);
    const float4 background = BackgroundTexture().Sample(BackgroundSampler(),
        distorted_uv);
    return float4(background.rgb, mask);
}