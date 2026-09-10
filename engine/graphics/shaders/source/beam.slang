#include "resource_access.slangh"
struct BeamVertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    uint resource_index : TEXCOORD1;
};

struct BeamVertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    uint resource_index : TEXCOORD1;
};

GRAPHICS_TEXTURE_ARRAY(Textures, float4, 0, 128)
GRAPHICS_SAMPLER(BeamSampler, 0)

BeamVertexOutput beam_vertex(BeamVertexInput input)
{
    BeamVertexOutput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.uv = input.uv;
    output.resource_index = input.resource_index;
    return output;
}

float4 beam_pixel(BeamVertexOutput input) : SV_TARGET
{
    float4 texture_color = 1.0.xxxx;
    if (input.resource_index != 0xffffffffu)
    {
#if defined(GRAPHICS_DX12)
        if (input.resource_index < 128u)
            texture_color = Textures[NonUniformResourceIndex(input.resource_index)].Sample(BeamSampler(), input.uv);
#else
        switch (input.resource_index)
        {
            case 0: texture_color = Textures[0].Sample(BeamSampler(), input.uv); break;
            case 1: texture_color = Textures[1].Sample(BeamSampler(), input.uv); break;
            case 2: texture_color = Textures[2].Sample(BeamSampler(), input.uv); break;
            case 3: texture_color = Textures[3].Sample(BeamSampler(), input.uv); break;
            case 4: texture_color = Textures[4].Sample(BeamSampler(), input.uv); break;
            case 5: texture_color = Textures[5].Sample(BeamSampler(), input.uv); break;
            case 6: texture_color = Textures[6].Sample(BeamSampler(), input.uv); break;
            case 7: texture_color = Textures[7].Sample(BeamSampler(), input.uv); break;
            case 8: texture_color = Textures[8].Sample(BeamSampler(), input.uv); break;
            case 9: texture_color = Textures[9].Sample(BeamSampler(), input.uv); break;
            case 10: texture_color = Textures[10].Sample(BeamSampler(), input.uv); break;
            case 11: texture_color = Textures[11].Sample(BeamSampler(), input.uv); break;
            case 12: texture_color = Textures[12].Sample(BeamSampler(), input.uv); break;
            case 13: texture_color = Textures[13].Sample(BeamSampler(), input.uv); break;
            case 14: texture_color = Textures[14].Sample(BeamSampler(), input.uv); break;
            case 15: texture_color = Textures[15].Sample(BeamSampler(), input.uv); break;
            case 16: texture_color = Textures[16].Sample(BeamSampler(), input.uv); break;
            case 17: texture_color = Textures[17].Sample(BeamSampler(), input.uv); break;
            case 18: texture_color = Textures[18].Sample(BeamSampler(), input.uv); break;
            case 19: texture_color = Textures[19].Sample(BeamSampler(), input.uv); break;
            case 20: texture_color = Textures[20].Sample(BeamSampler(), input.uv); break;
            case 21: texture_color = Textures[21].Sample(BeamSampler(), input.uv); break;
            case 22: texture_color = Textures[22].Sample(BeamSampler(), input.uv); break;
            case 23: texture_color = Textures[23].Sample(BeamSampler(), input.uv); break;
            case 24: texture_color = Textures[24].Sample(BeamSampler(), input.uv); break;
            case 25: texture_color = Textures[25].Sample(BeamSampler(), input.uv); break;
            case 26: texture_color = Textures[26].Sample(BeamSampler(), input.uv); break;
            case 27: texture_color = Textures[27].Sample(BeamSampler(), input.uv); break;
            case 28: texture_color = Textures[28].Sample(BeamSampler(), input.uv); break;
            case 29: texture_color = Textures[29].Sample(BeamSampler(), input.uv); break;
            case 30: texture_color = Textures[30].Sample(BeamSampler(), input.uv); break;
            case 31: texture_color = Textures[31].Sample(BeamSampler(), input.uv); break;
            case 32: texture_color = Textures[32].Sample(BeamSampler(), input.uv); break;
            case 33: texture_color = Textures[33].Sample(BeamSampler(), input.uv); break;
            case 34: texture_color = Textures[34].Sample(BeamSampler(), input.uv); break;
            case 35: texture_color = Textures[35].Sample(BeamSampler(), input.uv); break;
            case 36: texture_color = Textures[36].Sample(BeamSampler(), input.uv); break;
            case 37: texture_color = Textures[37].Sample(BeamSampler(), input.uv); break;
            case 38: texture_color = Textures[38].Sample(BeamSampler(), input.uv); break;
            case 39: texture_color = Textures[39].Sample(BeamSampler(), input.uv); break;
            case 40: texture_color = Textures[40].Sample(BeamSampler(), input.uv); break;
            case 41: texture_color = Textures[41].Sample(BeamSampler(), input.uv); break;
            case 42: texture_color = Textures[42].Sample(BeamSampler(), input.uv); break;
            case 43: texture_color = Textures[43].Sample(BeamSampler(), input.uv); break;
            case 44: texture_color = Textures[44].Sample(BeamSampler(), input.uv); break;
            case 45: texture_color = Textures[45].Sample(BeamSampler(), input.uv); break;
            case 46: texture_color = Textures[46].Sample(BeamSampler(), input.uv); break;
            case 47: texture_color = Textures[47].Sample(BeamSampler(), input.uv); break;
            case 48: texture_color = Textures[48].Sample(BeamSampler(), input.uv); break;
            case 49: texture_color = Textures[49].Sample(BeamSampler(), input.uv); break;
            case 50: texture_color = Textures[50].Sample(BeamSampler(), input.uv); break;
            case 51: texture_color = Textures[51].Sample(BeamSampler(), input.uv); break;
            case 52: texture_color = Textures[52].Sample(BeamSampler(), input.uv); break;
            case 53: texture_color = Textures[53].Sample(BeamSampler(), input.uv); break;
            case 54: texture_color = Textures[54].Sample(BeamSampler(), input.uv); break;
            case 55: texture_color = Textures[55].Sample(BeamSampler(), input.uv); break;
            case 56: texture_color = Textures[56].Sample(BeamSampler(), input.uv); break;
            case 57: texture_color = Textures[57].Sample(BeamSampler(), input.uv); break;
            case 58: texture_color = Textures[58].Sample(BeamSampler(), input.uv); break;
            case 59: texture_color = Textures[59].Sample(BeamSampler(), input.uv); break;
            case 60: texture_color = Textures[60].Sample(BeamSampler(), input.uv); break;
            case 61: texture_color = Textures[61].Sample(BeamSampler(), input.uv); break;
            case 62: texture_color = Textures[62].Sample(BeamSampler(), input.uv); break;
            case 63: texture_color = Textures[63].Sample(BeamSampler(), input.uv); break;
            case 64: texture_color = Textures[64].Sample(BeamSampler(), input.uv); break;
            case 65: texture_color = Textures[65].Sample(BeamSampler(), input.uv); break;
            case 66: texture_color = Textures[66].Sample(BeamSampler(), input.uv); break;
            case 67: texture_color = Textures[67].Sample(BeamSampler(), input.uv); break;
            case 68: texture_color = Textures[68].Sample(BeamSampler(), input.uv); break;
            case 69: texture_color = Textures[69].Sample(BeamSampler(), input.uv); break;
            case 70: texture_color = Textures[70].Sample(BeamSampler(), input.uv); break;
            case 71: texture_color = Textures[71].Sample(BeamSampler(), input.uv); break;
            case 72: texture_color = Textures[72].Sample(BeamSampler(), input.uv); break;
            case 73: texture_color = Textures[73].Sample(BeamSampler(), input.uv); break;
            case 74: texture_color = Textures[74].Sample(BeamSampler(), input.uv); break;
            case 75: texture_color = Textures[75].Sample(BeamSampler(), input.uv); break;
            case 76: texture_color = Textures[76].Sample(BeamSampler(), input.uv); break;
            case 77: texture_color = Textures[77].Sample(BeamSampler(), input.uv); break;
            case 78: texture_color = Textures[78].Sample(BeamSampler(), input.uv); break;
            case 79: texture_color = Textures[79].Sample(BeamSampler(), input.uv); break;
            case 80: texture_color = Textures[80].Sample(BeamSampler(), input.uv); break;
            case 81: texture_color = Textures[81].Sample(BeamSampler(), input.uv); break;
            case 82: texture_color = Textures[82].Sample(BeamSampler(), input.uv); break;
            case 83: texture_color = Textures[83].Sample(BeamSampler(), input.uv); break;
            case 84: texture_color = Textures[84].Sample(BeamSampler(), input.uv); break;
            case 85: texture_color = Textures[85].Sample(BeamSampler(), input.uv); break;
            case 86: texture_color = Textures[86].Sample(BeamSampler(), input.uv); break;
            case 87: texture_color = Textures[87].Sample(BeamSampler(), input.uv); break;
            case 88: texture_color = Textures[88].Sample(BeamSampler(), input.uv); break;
            case 89: texture_color = Textures[89].Sample(BeamSampler(), input.uv); break;
            case 90: texture_color = Textures[90].Sample(BeamSampler(), input.uv); break;
            case 91: texture_color = Textures[91].Sample(BeamSampler(), input.uv); break;
            case 92: texture_color = Textures[92].Sample(BeamSampler(), input.uv); break;
            case 93: texture_color = Textures[93].Sample(BeamSampler(), input.uv); break;
            case 94: texture_color = Textures[94].Sample(BeamSampler(), input.uv); break;
            case 95: texture_color = Textures[95].Sample(BeamSampler(), input.uv); break;
            case 96: texture_color = Textures[96].Sample(BeamSampler(), input.uv); break;
            case 97: texture_color = Textures[97].Sample(BeamSampler(), input.uv); break;
            case 98: texture_color = Textures[98].Sample(BeamSampler(), input.uv); break;
            case 99: texture_color = Textures[99].Sample(BeamSampler(), input.uv); break;
            case 100: texture_color = Textures[100].Sample(BeamSampler(), input.uv); break;
            case 101: texture_color = Textures[101].Sample(BeamSampler(), input.uv); break;
            case 102: texture_color = Textures[102].Sample(BeamSampler(), input.uv); break;
            case 103: texture_color = Textures[103].Sample(BeamSampler(), input.uv); break;
            case 104: texture_color = Textures[104].Sample(BeamSampler(), input.uv); break;
            case 105: texture_color = Textures[105].Sample(BeamSampler(), input.uv); break;
            case 106: texture_color = Textures[106].Sample(BeamSampler(), input.uv); break;
            case 107: texture_color = Textures[107].Sample(BeamSampler(), input.uv); break;
            case 108: texture_color = Textures[108].Sample(BeamSampler(), input.uv); break;
            case 109: texture_color = Textures[109].Sample(BeamSampler(), input.uv); break;
            case 110: texture_color = Textures[110].Sample(BeamSampler(), input.uv); break;
            case 111: texture_color = Textures[111].Sample(BeamSampler(), input.uv); break;
            case 112: texture_color = Textures[112].Sample(BeamSampler(), input.uv); break;
            case 113: texture_color = Textures[113].Sample(BeamSampler(), input.uv); break;
            case 114: texture_color = Textures[114].Sample(BeamSampler(), input.uv); break;
            case 115: texture_color = Textures[115].Sample(BeamSampler(), input.uv); break;
            case 116: texture_color = Textures[116].Sample(BeamSampler(), input.uv); break;
            case 117: texture_color = Textures[117].Sample(BeamSampler(), input.uv); break;
            case 118: texture_color = Textures[118].Sample(BeamSampler(), input.uv); break;
            case 119: texture_color = Textures[119].Sample(BeamSampler(), input.uv); break;
            case 120: texture_color = Textures[120].Sample(BeamSampler(), input.uv); break;
            case 121: texture_color = Textures[121].Sample(BeamSampler(), input.uv); break;
            case 122: texture_color = Textures[122].Sample(BeamSampler(), input.uv); break;
            case 123: texture_color = Textures[123].Sample(BeamSampler(), input.uv); break;
            case 124: texture_color = Textures[124].Sample(BeamSampler(), input.uv); break;
            case 125: texture_color = Textures[125].Sample(BeamSampler(), input.uv); break;
            case 126: texture_color = Textures[126].Sample(BeamSampler(), input.uv); break;
            case 127: texture_color = Textures[127].Sample(BeamSampler(), input.uv); break;
            default: break;
        }
#endif
    }
    return input.color * texture_color;
}
