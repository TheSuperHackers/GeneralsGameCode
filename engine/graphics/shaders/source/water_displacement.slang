#include "resource_access.slangh"

cbuffer DisplacementConstants : register(b0)
{
    float4 DisplacementDomain;
    float DisplacementTime;
    float DisplacementTexel;
    uint DisplacementOperation;
    float DisplacementPadding;
};

struct WaveParticle
{
    float2 position;
    float2 velocity;
    float amplitude;
    float creation_time;
    float2 padding;
};
GRAPHICS_STORAGE_BUFFER(WaveParticles, WaveParticle, 127)
GRAPHICS_TEXTURE_2D(DisplacementSource, float4, 0)
GRAPHICS_SAMPLER(DisplacementSampler, 0)

struct DisplacementVertexInput
{
    float3 position : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};
struct DisplacementVertexOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float amplitude : TEXCOORD1;
};

[shader("vertex")] DisplacementVertexOutput WaterDisplacementVertexMain(
    DisplacementVertexInput input, uint instance : SV_InstanceID)
{
    DisplacementVertexOutput output;
    output.position = float4(input.position,1);
    output.uv = input.uv;
    output.amplitude = 0;
    if (DisplacementOperation == 3)
    {
        WaveParticle particle = WaveParticles()[instance];
        float age = max(DisplacementTime - particle.creation_time,0.0);
        float2 position = particle.position + age * particle.velocity;
        float2 uv = (position - DisplacementDomain.xy) / DisplacementDomain.zw;
        uv.y = 1.0 - uv.y;
        uv = (floor(uv / DisplacementTexel) + 0.5) * DisplacementTexel;
        output.position.xy = uv * float2(2,-2) + float2(-1,1)
            + input.position.xy * DisplacementTexel;
        float amplitude = abs(particle.amplitude);
        if (particle.amplitude < 0)
            amplitude *= exp(-age / 4.0) * saturate(age / 2.0);
        output.amplitude = min(amplitude,1.0);
    }
    return output;
}

[shader("pixel")] float4 WaterDisplacementPixelMain(DisplacementVertexOutput input) : SV_TARGET
{
    if (DisplacementOperation == 3)
        return float4(input.amplitude.xxx,0);
    if (DisplacementOperation == 0)
    {
        float2 position = DisplacementDomain.xy
            + float2(input.uv.x,1.0-input.uv.y) * DisplacementDomain.zw;
        float2 uv = position * 0.001;
        const float cosine = 0.2588190;
        const float sine = 0.9659258;
        float2 uv0 = float2(dot(uv,float2(cosine,sine)),dot(uv,float2(-sine,cosine)));
        float2 uv1 = float2(dot(uv,float2(cosine,-sine)),dot(uv,float2(sine,cosine)));
        uv0.x += DisplacementTime * 0.0225;
        uv1.x += DisplacementTime * 0.0225;
        float height = DisplacementSource().Sample(DisplacementSampler(),uv0).a * 2.0 - 1.0;
        height += DisplacementSource().Sample(DisplacementSampler(),uv1).a * 2.0 - 1.0;
        return float4((height * 0.06).xxx,0);
    }
    float2 direction = DisplacementOperation == 1 ? float2(1,0) : float2(0,1);
    float3 result = 0;
    [unroll] for (int i = -10; i <= 10; ++i)
    {
        float phase = 3.14159265 * float(i) / 10.0;
        float transversal = 0.5 * (cos(phase) + 1.0);
        float longitudinal = -sin(phase) * 10.0;
        float3 factor = float3(lerp(float2(1,1),longitudinal.xx,direction),1) * transversal;
        float3 value = DisplacementSource().SampleLevel(DisplacementSampler(),
            input.uv + i * direction * DisplacementTexel,0).xyz;
        result += value * factor;
    }
    result /= 21.0;
    result.xy *= 0.5;
    return float4(result,0);
}
