#include "resource_access.slangh"
struct ParticleVertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct ParticleVertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    uint emitter_flags : TEXCOORD1;
    uint texture_index : TEXCOORD2;
    float point_size_value : TEXCOORD3;
    float2 point_center : TEXCOORD4;
    nointerpolation float4 texture_region : TEXCOORD5;
};

struct ParticlePixelInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    uint emitter_flags : TEXCOORD1;
    uint texture_index : TEXCOORD2;
    float point_size_value : TEXCOORD3;
    float2 point_center : TEXCOORD4;
    nointerpolation float4 texture_region : TEXCOORD5;
};

struct GPUParticleData
{
    float4 position_lifetime;
    float4 velocity_size;
    float4 color;
    uint material_index;
    uint emitter_flags;
    float angle;
    uint texture_index;
    float4 texture_region;
};

GRAPHICS_STORAGE_BUFFER(Particles, GPUParticleData, 127)
GRAPHICS_TEXTURE_ARRAY(Textures, float4, 0, 126)
GRAPHICS_SAMPLER(ParticleSampler, 0)

cbuffer MaterialParameters : register(b0)
{
    float4 Values[4];
};

cbuffer ParticleFrame : register(b1)
{
    row_major float4x4 ParticleView;
    row_major float4x4 ParticleProjection;
    uint4 ParticleOffset;
};

ParticleVertexOutput particle_billboard_vertex(ParticleVertexInput input, uint instance_id : SV_InstanceID)
{
    const GPUParticleData particle = Particles()[ParticleOffset.x + instance_id];
    const float cosine = cos(particle.angle);
    const float sine = sin(particle.angle);
    const bool billboard = (particle.emitter_flags & 2u) != 0;
    const bool point_sprite = (particle.emitter_flags & 32u) != 0;
    const float size = particle.velocity_size.w;
    const float4 world_center = float4(particle.position_lifetime.xyz,1);
    const float4 view_center = mul(ParticleView,world_center);
    const float4 clip_center = mul(ParticleProjection,view_center);
    ParticleVertexOutput output;
    if (point_sprite) {
        output.position = clip_center;
        output.position.xy += input.position.xy * max(size,1.0) / Values[1].xy * clip_center.w;
    } else if (billboard) {
        const float2 corner = float2(cosine*input.position.x-sine*input.position.y,
            sine*input.position.x+cosine*input.position.y)*size;
        output.position = mul(ParticleProjection,view_center+float4(corner,0,0));
    } else {
        // Ground quads rotate in world XY. Project each corner to retain
        // perspective interpolation and its depth against scene geometry.
        const float2 corner = float2(cosine*input.position.x+sine*input.position.y,
            -sine*input.position.x+cosine*input.position.y)*size;
        output.position = mul(ParticleProjection,mul(ParticleView,world_center+float4(corner,0,0)));
    }
    output.point_size_value = point_sprite ? max(size,1.0) : 1.0;
    output.point_center = clip_center.xy / max(abs(clip_center.w),0.000001);
    output.color = particle.color * Values[0];
    // Ground effects assign UV(0,0) to the rotated world (+X,+Y)
    // corner; billboards assign it to the upper-left view-space corner.
    output.uv = !billboard && !point_sprite ? float2(1.0-input.uv.x,input.uv.y) : input.uv;
    output.emitter_flags = particle.emitter_flags;
    output.texture_index = particle.texture_index;
    output.texture_region = particle.texture_region;
    return output;
}

float4 particle_billboard_pixel(ParticlePixelInput input) : SV_TARGET
{
    const bool point_sprite = (input.emitter_flags & 32u) != 0;
    const float2 point_center_pixels = float2(
        Values[1].z + (input.point_center.x + 1.0) * 0.5 * Values[1].x,
        Values[1].w + (1.0 - input.point_center.y) * 0.5 * Values[1].y);
    const float2 sprite_uv = point_sprite
        ? (input.position.xy - point_center_pixels) / max(input.point_size_value, 1.0) + 0.5
        : input.uv;
    const float2 point_sample_uv = lerp(input.texture_region.xy, input.texture_region.zw, sprite_uv);
    float4 texture_color = float4(1.0, 1.0, 1.0, 1.0);
    if (input.texture_index != 0xffffffffu)
    {
#if defined(GRAPHICS_DX12)
        if (input.texture_index < 126u)
            texture_color = Textures[NonUniformResourceIndex(input.texture_index)].Sample(ParticleSampler(), point_sample_uv);
#else
        switch (input.texture_index)
        {
        case 0: texture_color = Textures[0].Sample(ParticleSampler(), point_sample_uv); break;
        case 1: texture_color = Textures[1].Sample(ParticleSampler(), point_sample_uv); break;
        case 2: texture_color = Textures[2].Sample(ParticleSampler(), point_sample_uv); break;
        case 3: texture_color = Textures[3].Sample(ParticleSampler(), point_sample_uv); break;
        case 4: texture_color = Textures[4].Sample(ParticleSampler(), point_sample_uv); break;
        case 5: texture_color = Textures[5].Sample(ParticleSampler(), point_sample_uv); break;
        case 6: texture_color = Textures[6].Sample(ParticleSampler(), point_sample_uv); break;
        case 7: texture_color = Textures[7].Sample(ParticleSampler(), point_sample_uv); break;
        case 8: texture_color = Textures[8].Sample(ParticleSampler(), point_sample_uv); break;
        case 9: texture_color = Textures[9].Sample(ParticleSampler(), point_sample_uv); break;
        case 10: texture_color = Textures[10].Sample(ParticleSampler(), point_sample_uv); break;
        case 11: texture_color = Textures[11].Sample(ParticleSampler(), point_sample_uv); break;
        case 12: texture_color = Textures[12].Sample(ParticleSampler(), point_sample_uv); break;
        case 13: texture_color = Textures[13].Sample(ParticleSampler(), point_sample_uv); break;
        case 14: texture_color = Textures[14].Sample(ParticleSampler(), point_sample_uv); break;
        case 15: texture_color = Textures[15].Sample(ParticleSampler(), point_sample_uv); break;
        case 16: texture_color = Textures[16].Sample(ParticleSampler(), point_sample_uv); break;
        case 17: texture_color = Textures[17].Sample(ParticleSampler(), point_sample_uv); break;
        case 18: texture_color = Textures[18].Sample(ParticleSampler(), point_sample_uv); break;
        case 19: texture_color = Textures[19].Sample(ParticleSampler(), point_sample_uv); break;
        case 20: texture_color = Textures[20].Sample(ParticleSampler(), point_sample_uv); break;
        case 21: texture_color = Textures[21].Sample(ParticleSampler(), point_sample_uv); break;
        case 22: texture_color = Textures[22].Sample(ParticleSampler(), point_sample_uv); break;
        case 23: texture_color = Textures[23].Sample(ParticleSampler(), point_sample_uv); break;
        case 24: texture_color = Textures[24].Sample(ParticleSampler(), point_sample_uv); break;
        case 25: texture_color = Textures[25].Sample(ParticleSampler(), point_sample_uv); break;
        case 26: texture_color = Textures[26].Sample(ParticleSampler(), point_sample_uv); break;
        case 27: texture_color = Textures[27].Sample(ParticleSampler(), point_sample_uv); break;
        case 28: texture_color = Textures[28].Sample(ParticleSampler(), point_sample_uv); break;
        case 29: texture_color = Textures[29].Sample(ParticleSampler(), point_sample_uv); break;
        case 30: texture_color = Textures[30].Sample(ParticleSampler(), point_sample_uv); break;
        case 31: texture_color = Textures[31].Sample(ParticleSampler(), point_sample_uv); break;
        case 32: texture_color = Textures[32].Sample(ParticleSampler(), point_sample_uv); break;
        case 33: texture_color = Textures[33].Sample(ParticleSampler(), point_sample_uv); break;
        case 34: texture_color = Textures[34].Sample(ParticleSampler(), point_sample_uv); break;
        case 35: texture_color = Textures[35].Sample(ParticleSampler(), point_sample_uv); break;
        case 36: texture_color = Textures[36].Sample(ParticleSampler(), point_sample_uv); break;
        case 37: texture_color = Textures[37].Sample(ParticleSampler(), point_sample_uv); break;
        case 38: texture_color = Textures[38].Sample(ParticleSampler(), point_sample_uv); break;
        case 39: texture_color = Textures[39].Sample(ParticleSampler(), point_sample_uv); break;
        case 40: texture_color = Textures[40].Sample(ParticleSampler(), point_sample_uv); break;
        case 41: texture_color = Textures[41].Sample(ParticleSampler(), point_sample_uv); break;
        case 42: texture_color = Textures[42].Sample(ParticleSampler(), point_sample_uv); break;
        case 43: texture_color = Textures[43].Sample(ParticleSampler(), point_sample_uv); break;
        case 44: texture_color = Textures[44].Sample(ParticleSampler(), point_sample_uv); break;
        case 45: texture_color = Textures[45].Sample(ParticleSampler(), point_sample_uv); break;
        case 46: texture_color = Textures[46].Sample(ParticleSampler(), point_sample_uv); break;
        case 47: texture_color = Textures[47].Sample(ParticleSampler(), point_sample_uv); break;
        case 48: texture_color = Textures[48].Sample(ParticleSampler(), point_sample_uv); break;
        case 49: texture_color = Textures[49].Sample(ParticleSampler(), point_sample_uv); break;
        case 50: texture_color = Textures[50].Sample(ParticleSampler(), point_sample_uv); break;
        case 51: texture_color = Textures[51].Sample(ParticleSampler(), point_sample_uv); break;
        case 52: texture_color = Textures[52].Sample(ParticleSampler(), point_sample_uv); break;
        case 53: texture_color = Textures[53].Sample(ParticleSampler(), point_sample_uv); break;
        case 54: texture_color = Textures[54].Sample(ParticleSampler(), point_sample_uv); break;
        case 55: texture_color = Textures[55].Sample(ParticleSampler(), point_sample_uv); break;
        case 56: texture_color = Textures[56].Sample(ParticleSampler(), point_sample_uv); break;
        case 57: texture_color = Textures[57].Sample(ParticleSampler(), point_sample_uv); break;
        case 58: texture_color = Textures[58].Sample(ParticleSampler(), point_sample_uv); break;
        case 59: texture_color = Textures[59].Sample(ParticleSampler(), point_sample_uv); break;
        case 60: texture_color = Textures[60].Sample(ParticleSampler(), point_sample_uv); break;
        case 61: texture_color = Textures[61].Sample(ParticleSampler(), point_sample_uv); break;
        case 62: texture_color = Textures[62].Sample(ParticleSampler(), point_sample_uv); break;
        case 63: texture_color = Textures[63].Sample(ParticleSampler(), point_sample_uv); break;
        case 64: texture_color = Textures[64].Sample(ParticleSampler(), point_sample_uv); break;
        case 65: texture_color = Textures[65].Sample(ParticleSampler(), point_sample_uv); break;
        case 66: texture_color = Textures[66].Sample(ParticleSampler(), point_sample_uv); break;
        case 67: texture_color = Textures[67].Sample(ParticleSampler(), point_sample_uv); break;
        case 68: texture_color = Textures[68].Sample(ParticleSampler(), point_sample_uv); break;
        case 69: texture_color = Textures[69].Sample(ParticleSampler(), point_sample_uv); break;
        case 70: texture_color = Textures[70].Sample(ParticleSampler(), point_sample_uv); break;
        case 71: texture_color = Textures[71].Sample(ParticleSampler(), point_sample_uv); break;
        case 72: texture_color = Textures[72].Sample(ParticleSampler(), point_sample_uv); break;
        case 73: texture_color = Textures[73].Sample(ParticleSampler(), point_sample_uv); break;
        case 74: texture_color = Textures[74].Sample(ParticleSampler(), point_sample_uv); break;
        case 75: texture_color = Textures[75].Sample(ParticleSampler(), point_sample_uv); break;
        case 76: texture_color = Textures[76].Sample(ParticleSampler(), point_sample_uv); break;
        case 77: texture_color = Textures[77].Sample(ParticleSampler(), point_sample_uv); break;
        case 78: texture_color = Textures[78].Sample(ParticleSampler(), point_sample_uv); break;
        case 79: texture_color = Textures[79].Sample(ParticleSampler(), point_sample_uv); break;
        case 80: texture_color = Textures[80].Sample(ParticleSampler(), point_sample_uv); break;
        case 81: texture_color = Textures[81].Sample(ParticleSampler(), point_sample_uv); break;
        case 82: texture_color = Textures[82].Sample(ParticleSampler(), point_sample_uv); break;
        case 83: texture_color = Textures[83].Sample(ParticleSampler(), point_sample_uv); break;
        case 84: texture_color = Textures[84].Sample(ParticleSampler(), point_sample_uv); break;
        case 85: texture_color = Textures[85].Sample(ParticleSampler(), point_sample_uv); break;
        case 86: texture_color = Textures[86].Sample(ParticleSampler(), point_sample_uv); break;
        case 87: texture_color = Textures[87].Sample(ParticleSampler(), point_sample_uv); break;
        case 88: texture_color = Textures[88].Sample(ParticleSampler(), point_sample_uv); break;
        case 89: texture_color = Textures[89].Sample(ParticleSampler(), point_sample_uv); break;
        case 90: texture_color = Textures[90].Sample(ParticleSampler(), point_sample_uv); break;
        case 91: texture_color = Textures[91].Sample(ParticleSampler(), point_sample_uv); break;
        case 92: texture_color = Textures[92].Sample(ParticleSampler(), point_sample_uv); break;
        case 93: texture_color = Textures[93].Sample(ParticleSampler(), point_sample_uv); break;
        case 94: texture_color = Textures[94].Sample(ParticleSampler(), point_sample_uv); break;
        case 95: texture_color = Textures[95].Sample(ParticleSampler(), point_sample_uv); break;
        case 96: texture_color = Textures[96].Sample(ParticleSampler(), point_sample_uv); break;
        case 97: texture_color = Textures[97].Sample(ParticleSampler(), point_sample_uv); break;
        case 98: texture_color = Textures[98].Sample(ParticleSampler(), point_sample_uv); break;
        case 99: texture_color = Textures[99].Sample(ParticleSampler(), point_sample_uv); break;
        case 100: texture_color = Textures[100].Sample(ParticleSampler(), point_sample_uv); break;
        case 101: texture_color = Textures[101].Sample(ParticleSampler(), point_sample_uv); break;
        case 102: texture_color = Textures[102].Sample(ParticleSampler(), point_sample_uv); break;
        case 103: texture_color = Textures[103].Sample(ParticleSampler(), point_sample_uv); break;
        case 104: texture_color = Textures[104].Sample(ParticleSampler(), point_sample_uv); break;
        case 105: texture_color = Textures[105].Sample(ParticleSampler(), point_sample_uv); break;
        case 106: texture_color = Textures[106].Sample(ParticleSampler(), point_sample_uv); break;
        case 107: texture_color = Textures[107].Sample(ParticleSampler(), point_sample_uv); break;
        case 108: texture_color = Textures[108].Sample(ParticleSampler(), point_sample_uv); break;
        case 109: texture_color = Textures[109].Sample(ParticleSampler(), point_sample_uv); break;
        case 110: texture_color = Textures[110].Sample(ParticleSampler(), point_sample_uv); break;
        case 111: texture_color = Textures[111].Sample(ParticleSampler(), point_sample_uv); break;
        case 112: texture_color = Textures[112].Sample(ParticleSampler(), point_sample_uv); break;
        case 113: texture_color = Textures[113].Sample(ParticleSampler(), point_sample_uv); break;
        case 114: texture_color = Textures[114].Sample(ParticleSampler(), point_sample_uv); break;
        case 115: texture_color = Textures[115].Sample(ParticleSampler(), point_sample_uv); break;
        case 116: texture_color = Textures[116].Sample(ParticleSampler(), point_sample_uv); break;
        case 117: texture_color = Textures[117].Sample(ParticleSampler(), point_sample_uv); break;
        case 118: texture_color = Textures[118].Sample(ParticleSampler(), point_sample_uv); break;
        case 119: texture_color = Textures[119].Sample(ParticleSampler(), point_sample_uv); break;
        case 120: texture_color = Textures[120].Sample(ParticleSampler(), point_sample_uv); break;
        case 121: texture_color = Textures[121].Sample(ParticleSampler(), point_sample_uv); break;
        case 122: texture_color = Textures[122].Sample(ParticleSampler(), point_sample_uv); break;
        case 123: texture_color = Textures[123].Sample(ParticleSampler(), point_sample_uv); break;
        case 124: texture_color = Textures[124].Sample(ParticleSampler(), point_sample_uv); break;
        case 125: texture_color = Textures[125].Sample(ParticleSampler(), point_sample_uv); break;
        default: break;
        }
#endif
    }

    const float4 color = input.color * texture_color;
    if ((input.emitter_flags & 4u) != 0 && color.a < (96.0 / 255.0))
        discard;
    return color;
}
