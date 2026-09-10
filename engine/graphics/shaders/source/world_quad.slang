#include "resource_access.slangh"
struct WorldQuadVertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct WorldQuadVertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    uint material_index : TEXCOORD1;
};

struct GPUWorldQuadData
{
    float4 corners[4];
    float4 color;
    uint material_index;
    uint reserved0;
    uint reserved1;
    uint reserved2;
};

struct GPUMaterialData
{
    float4 parameters[4];
    uint texture_indices[8];
    uint sampler_indices[8];
    uint flags;
    uint reserved[3];
};

struct WorldQuadViewData
{
    row_major float4x4 view_projection;
    float4 reserved0;
    float4 reserved1;
    float4 reserved2;
};

GRAPHICS_STORAGE_BUFFER(Quads, GPUWorldQuadData, 127)
GRAPHICS_STORAGE_BUFFER(Materials, GPUMaterialData, 126)
GRAPHICS_STORAGE_BUFFER(Views, WorldQuadViewData, 125)
GRAPHICS_TEXTURE_ARRAY(Textures, float4, 0, 125)
GRAPHICS_SAMPLER(WorldQuadSampler, 0)

WorldQuadVertexOutput world_quad_vertex(WorldQuadVertexInput input, uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID)
{
    const GPUWorldQuadData quad = Quads()[instance_id];
    uint corner_index = vertex_id;
    if (vertex_id == 3)
        corner_index = 0;
    else if (vertex_id == 4)
        corner_index = 2;
    else if (vertex_id == 5)
        corner_index = 3;

    WorldQuadVertexOutput output;
    output.position = mul(quad.corners[corner_index], Views()[0].view_projection);
    output.color = quad.color;
    output.uv = input.uv;
    output.material_index = quad.material_index;
    return output;
}

float4 world_quad_pixel(WorldQuadVertexOutput input) : SV_TARGET
{
    const GPUMaterialData material = Materials()[input.material_index];
    const uint texture_index = material.texture_indices[0];
    float4 texture_color = 1.0.xxxx;
    if (texture_index != 0xffffffffu)
    {
#if defined(GRAPHICS_DX12)
        if (texture_index < 125u)
            texture_color = Textures[NonUniformResourceIndex(texture_index)].Sample(WorldQuadSampler(), input.uv);
#else
        switch (texture_index)
        {
        case 0: texture_color = Textures[0].Sample(WorldQuadSampler(), input.uv); break;
        case 1: texture_color = Textures[1].Sample(WorldQuadSampler(), input.uv); break;
        case 2: texture_color = Textures[2].Sample(WorldQuadSampler(), input.uv); break;
        case 3: texture_color = Textures[3].Sample(WorldQuadSampler(), input.uv); break;
        case 4: texture_color = Textures[4].Sample(WorldQuadSampler(), input.uv); break;
        case 5: texture_color = Textures[5].Sample(WorldQuadSampler(), input.uv); break;
        case 6: texture_color = Textures[6].Sample(WorldQuadSampler(), input.uv); break;
        case 7: texture_color = Textures[7].Sample(WorldQuadSampler(), input.uv); break;
        case 8: texture_color = Textures[8].Sample(WorldQuadSampler(), input.uv); break;
        case 9: texture_color = Textures[9].Sample(WorldQuadSampler(), input.uv); break;
        case 10: texture_color = Textures[10].Sample(WorldQuadSampler(), input.uv); break;
        case 11: texture_color = Textures[11].Sample(WorldQuadSampler(), input.uv); break;
        case 12: texture_color = Textures[12].Sample(WorldQuadSampler(), input.uv); break;
        case 13: texture_color = Textures[13].Sample(WorldQuadSampler(), input.uv); break;
        case 14: texture_color = Textures[14].Sample(WorldQuadSampler(), input.uv); break;
        case 15: texture_color = Textures[15].Sample(WorldQuadSampler(), input.uv); break;
        case 16: texture_color = Textures[16].Sample(WorldQuadSampler(), input.uv); break;
        case 17: texture_color = Textures[17].Sample(WorldQuadSampler(), input.uv); break;
        case 18: texture_color = Textures[18].Sample(WorldQuadSampler(), input.uv); break;
        case 19: texture_color = Textures[19].Sample(WorldQuadSampler(), input.uv); break;
        case 20: texture_color = Textures[20].Sample(WorldQuadSampler(), input.uv); break;
        case 21: texture_color = Textures[21].Sample(WorldQuadSampler(), input.uv); break;
        case 22: texture_color = Textures[22].Sample(WorldQuadSampler(), input.uv); break;
        case 23: texture_color = Textures[23].Sample(WorldQuadSampler(), input.uv); break;
        case 24: texture_color = Textures[24].Sample(WorldQuadSampler(), input.uv); break;
        case 25: texture_color = Textures[25].Sample(WorldQuadSampler(), input.uv); break;
        case 26: texture_color = Textures[26].Sample(WorldQuadSampler(), input.uv); break;
        case 27: texture_color = Textures[27].Sample(WorldQuadSampler(), input.uv); break;
        case 28: texture_color = Textures[28].Sample(WorldQuadSampler(), input.uv); break;
        case 29: texture_color = Textures[29].Sample(WorldQuadSampler(), input.uv); break;
        case 30: texture_color = Textures[30].Sample(WorldQuadSampler(), input.uv); break;
        case 31: texture_color = Textures[31].Sample(WorldQuadSampler(), input.uv); break;
        case 32: texture_color = Textures[32].Sample(WorldQuadSampler(), input.uv); break;
        case 33: texture_color = Textures[33].Sample(WorldQuadSampler(), input.uv); break;
        case 34: texture_color = Textures[34].Sample(WorldQuadSampler(), input.uv); break;
        case 35: texture_color = Textures[35].Sample(WorldQuadSampler(), input.uv); break;
        case 36: texture_color = Textures[36].Sample(WorldQuadSampler(), input.uv); break;
        case 37: texture_color = Textures[37].Sample(WorldQuadSampler(), input.uv); break;
        case 38: texture_color = Textures[38].Sample(WorldQuadSampler(), input.uv); break;
        case 39: texture_color = Textures[39].Sample(WorldQuadSampler(), input.uv); break;
        case 40: texture_color = Textures[40].Sample(WorldQuadSampler(), input.uv); break;
        case 41: texture_color = Textures[41].Sample(WorldQuadSampler(), input.uv); break;
        case 42: texture_color = Textures[42].Sample(WorldQuadSampler(), input.uv); break;
        case 43: texture_color = Textures[43].Sample(WorldQuadSampler(), input.uv); break;
        case 44: texture_color = Textures[44].Sample(WorldQuadSampler(), input.uv); break;
        case 45: texture_color = Textures[45].Sample(WorldQuadSampler(), input.uv); break;
        case 46: texture_color = Textures[46].Sample(WorldQuadSampler(), input.uv); break;
        case 47: texture_color = Textures[47].Sample(WorldQuadSampler(), input.uv); break;
        case 48: texture_color = Textures[48].Sample(WorldQuadSampler(), input.uv); break;
        case 49: texture_color = Textures[49].Sample(WorldQuadSampler(), input.uv); break;
        case 50: texture_color = Textures[50].Sample(WorldQuadSampler(), input.uv); break;
        case 51: texture_color = Textures[51].Sample(WorldQuadSampler(), input.uv); break;
        case 52: texture_color = Textures[52].Sample(WorldQuadSampler(), input.uv); break;
        case 53: texture_color = Textures[53].Sample(WorldQuadSampler(), input.uv); break;
        case 54: texture_color = Textures[54].Sample(WorldQuadSampler(), input.uv); break;
        case 55: texture_color = Textures[55].Sample(WorldQuadSampler(), input.uv); break;
        case 56: texture_color = Textures[56].Sample(WorldQuadSampler(), input.uv); break;
        case 57: texture_color = Textures[57].Sample(WorldQuadSampler(), input.uv); break;
        case 58: texture_color = Textures[58].Sample(WorldQuadSampler(), input.uv); break;
        case 59: texture_color = Textures[59].Sample(WorldQuadSampler(), input.uv); break;
        case 60: texture_color = Textures[60].Sample(WorldQuadSampler(), input.uv); break;
        case 61: texture_color = Textures[61].Sample(WorldQuadSampler(), input.uv); break;
        case 62: texture_color = Textures[62].Sample(WorldQuadSampler(), input.uv); break;
        case 63: texture_color = Textures[63].Sample(WorldQuadSampler(), input.uv); break;
        case 64: texture_color = Textures[64].Sample(WorldQuadSampler(), input.uv); break;
        case 65: texture_color = Textures[65].Sample(WorldQuadSampler(), input.uv); break;
        case 66: texture_color = Textures[66].Sample(WorldQuadSampler(), input.uv); break;
        case 67: texture_color = Textures[67].Sample(WorldQuadSampler(), input.uv); break;
        case 68: texture_color = Textures[68].Sample(WorldQuadSampler(), input.uv); break;
        case 69: texture_color = Textures[69].Sample(WorldQuadSampler(), input.uv); break;
        case 70: texture_color = Textures[70].Sample(WorldQuadSampler(), input.uv); break;
        case 71: texture_color = Textures[71].Sample(WorldQuadSampler(), input.uv); break;
        case 72: texture_color = Textures[72].Sample(WorldQuadSampler(), input.uv); break;
        case 73: texture_color = Textures[73].Sample(WorldQuadSampler(), input.uv); break;
        case 74: texture_color = Textures[74].Sample(WorldQuadSampler(), input.uv); break;
        case 75: texture_color = Textures[75].Sample(WorldQuadSampler(), input.uv); break;
        case 76: texture_color = Textures[76].Sample(WorldQuadSampler(), input.uv); break;
        case 77: texture_color = Textures[77].Sample(WorldQuadSampler(), input.uv); break;
        case 78: texture_color = Textures[78].Sample(WorldQuadSampler(), input.uv); break;
        case 79: texture_color = Textures[79].Sample(WorldQuadSampler(), input.uv); break;
        case 80: texture_color = Textures[80].Sample(WorldQuadSampler(), input.uv); break;
        case 81: texture_color = Textures[81].Sample(WorldQuadSampler(), input.uv); break;
        case 82: texture_color = Textures[82].Sample(WorldQuadSampler(), input.uv); break;
        case 83: texture_color = Textures[83].Sample(WorldQuadSampler(), input.uv); break;
        case 84: texture_color = Textures[84].Sample(WorldQuadSampler(), input.uv); break;
        case 85: texture_color = Textures[85].Sample(WorldQuadSampler(), input.uv); break;
        case 86: texture_color = Textures[86].Sample(WorldQuadSampler(), input.uv); break;
        case 87: texture_color = Textures[87].Sample(WorldQuadSampler(), input.uv); break;
        case 88: texture_color = Textures[88].Sample(WorldQuadSampler(), input.uv); break;
        case 89: texture_color = Textures[89].Sample(WorldQuadSampler(), input.uv); break;
        case 90: texture_color = Textures[90].Sample(WorldQuadSampler(), input.uv); break;
        case 91: texture_color = Textures[91].Sample(WorldQuadSampler(), input.uv); break;
        case 92: texture_color = Textures[92].Sample(WorldQuadSampler(), input.uv); break;
        case 93: texture_color = Textures[93].Sample(WorldQuadSampler(), input.uv); break;
        case 94: texture_color = Textures[94].Sample(WorldQuadSampler(), input.uv); break;
        case 95: texture_color = Textures[95].Sample(WorldQuadSampler(), input.uv); break;
        case 96: texture_color = Textures[96].Sample(WorldQuadSampler(), input.uv); break;
        case 97: texture_color = Textures[97].Sample(WorldQuadSampler(), input.uv); break;
        case 98: texture_color = Textures[98].Sample(WorldQuadSampler(), input.uv); break;
        case 99: texture_color = Textures[99].Sample(WorldQuadSampler(), input.uv); break;
        case 100: texture_color = Textures[100].Sample(WorldQuadSampler(), input.uv); break;
        case 101: texture_color = Textures[101].Sample(WorldQuadSampler(), input.uv); break;
        case 102: texture_color = Textures[102].Sample(WorldQuadSampler(), input.uv); break;
        case 103: texture_color = Textures[103].Sample(WorldQuadSampler(), input.uv); break;
        case 104: texture_color = Textures[104].Sample(WorldQuadSampler(), input.uv); break;
        case 105: texture_color = Textures[105].Sample(WorldQuadSampler(), input.uv); break;
        case 106: texture_color = Textures[106].Sample(WorldQuadSampler(), input.uv); break;
        case 107: texture_color = Textures[107].Sample(WorldQuadSampler(), input.uv); break;
        case 108: texture_color = Textures[108].Sample(WorldQuadSampler(), input.uv); break;
        case 109: texture_color = Textures[109].Sample(WorldQuadSampler(), input.uv); break;
        case 110: texture_color = Textures[110].Sample(WorldQuadSampler(), input.uv); break;
        case 111: texture_color = Textures[111].Sample(WorldQuadSampler(), input.uv); break;
        case 112: texture_color = Textures[112].Sample(WorldQuadSampler(), input.uv); break;
        case 113: texture_color = Textures[113].Sample(WorldQuadSampler(), input.uv); break;
        case 114: texture_color = Textures[114].Sample(WorldQuadSampler(), input.uv); break;
        case 115: texture_color = Textures[115].Sample(WorldQuadSampler(), input.uv); break;
        case 116: texture_color = Textures[116].Sample(WorldQuadSampler(), input.uv); break;
        case 117: texture_color = Textures[117].Sample(WorldQuadSampler(), input.uv); break;
        case 118: texture_color = Textures[118].Sample(WorldQuadSampler(), input.uv); break;
        case 119: texture_color = Textures[119].Sample(WorldQuadSampler(), input.uv); break;
        case 120: texture_color = Textures[120].Sample(WorldQuadSampler(), input.uv); break;
        case 121: texture_color = Textures[121].Sample(WorldQuadSampler(), input.uv); break;
        case 122: texture_color = Textures[122].Sample(WorldQuadSampler(), input.uv); break;
        case 123: texture_color = Textures[123].Sample(WorldQuadSampler(), input.uv); break;
        case 124: texture_color = Textures[124].Sample(WorldQuadSampler(), input.uv); break;
        default: break;
        }
#endif
    }
    return input.color * material.parameters[0] * texture_color;
}
