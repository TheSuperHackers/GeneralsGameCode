#include "resource_access.slangh"
struct UI2DVertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    uint texture_index : TEXCOORD1;
};

struct UI2DVertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    uint texture_index : TEXCOORD1;
};

GRAPHICS_TEXTURE_ARRAY(UITextures, float4, 0, 128)
GRAPHICS_SAMPLER(UI2DSampler, 0)

[shader("vertex")]
UI2DVertexOutput ui_2d_vertex(UI2DVertexInput input)
{
    UI2DVertexOutput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.uv = input.uv;
    output.texture_index = input.texture_index;
    return output;
}

float4 sample_ui_texture(uint texture_index, float2 uv)
{
#if defined(GRAPHICS_DX12)
    if (texture_index < 128u)
        return UITextures[NonUniformResourceIndex(texture_index)].Sample(UI2DSampler(), uv);
    return float4(1.0, 1.0, 1.0, 1.0);
#else
    switch (texture_index) {
    case 0: return UITextures[0].Sample(UI2DSampler(), uv);
    case 1: return UITextures[1].Sample(UI2DSampler(), uv);
    case 2: return UITextures[2].Sample(UI2DSampler(), uv);
    case 3: return UITextures[3].Sample(UI2DSampler(), uv);
    case 4: return UITextures[4].Sample(UI2DSampler(), uv);
    case 5: return UITextures[5].Sample(UI2DSampler(), uv);
    case 6: return UITextures[6].Sample(UI2DSampler(), uv);
    case 7: return UITextures[7].Sample(UI2DSampler(), uv);
    case 8: return UITextures[8].Sample(UI2DSampler(), uv);
    case 9: return UITextures[9].Sample(UI2DSampler(), uv);
    case 10: return UITextures[10].Sample(UI2DSampler(), uv);
    case 11: return UITextures[11].Sample(UI2DSampler(), uv);
    case 12: return UITextures[12].Sample(UI2DSampler(), uv);
    case 13: return UITextures[13].Sample(UI2DSampler(), uv);
    case 14: return UITextures[14].Sample(UI2DSampler(), uv);
    case 15: return UITextures[15].Sample(UI2DSampler(), uv);
    case 16: return UITextures[16].Sample(UI2DSampler(), uv);
    case 17: return UITextures[17].Sample(UI2DSampler(), uv);
    case 18: return UITextures[18].Sample(UI2DSampler(), uv);
    case 19: return UITextures[19].Sample(UI2DSampler(), uv);
    case 20: return UITextures[20].Sample(UI2DSampler(), uv);
    case 21: return UITextures[21].Sample(UI2DSampler(), uv);
    case 22: return UITextures[22].Sample(UI2DSampler(), uv);
    case 23: return UITextures[23].Sample(UI2DSampler(), uv);
    case 24: return UITextures[24].Sample(UI2DSampler(), uv);
    case 25: return UITextures[25].Sample(UI2DSampler(), uv);
    case 26: return UITextures[26].Sample(UI2DSampler(), uv);
    case 27: return UITextures[27].Sample(UI2DSampler(), uv);
    case 28: return UITextures[28].Sample(UI2DSampler(), uv);
    case 29: return UITextures[29].Sample(UI2DSampler(), uv);
    case 30: return UITextures[30].Sample(UI2DSampler(), uv);
    case 31: return UITextures[31].Sample(UI2DSampler(), uv);
    case 32: return UITextures[32].Sample(UI2DSampler(), uv);
    case 33: return UITextures[33].Sample(UI2DSampler(), uv);
    case 34: return UITextures[34].Sample(UI2DSampler(), uv);
    case 35: return UITextures[35].Sample(UI2DSampler(), uv);
    case 36: return UITextures[36].Sample(UI2DSampler(), uv);
    case 37: return UITextures[37].Sample(UI2DSampler(), uv);
    case 38: return UITextures[38].Sample(UI2DSampler(), uv);
    case 39: return UITextures[39].Sample(UI2DSampler(), uv);
    case 40: return UITextures[40].Sample(UI2DSampler(), uv);
    case 41: return UITextures[41].Sample(UI2DSampler(), uv);
    case 42: return UITextures[42].Sample(UI2DSampler(), uv);
    case 43: return UITextures[43].Sample(UI2DSampler(), uv);
    case 44: return UITextures[44].Sample(UI2DSampler(), uv);
    case 45: return UITextures[45].Sample(UI2DSampler(), uv);
    case 46: return UITextures[46].Sample(UI2DSampler(), uv);
    case 47: return UITextures[47].Sample(UI2DSampler(), uv);
    case 48: return UITextures[48].Sample(UI2DSampler(), uv);
    case 49: return UITextures[49].Sample(UI2DSampler(), uv);
    case 50: return UITextures[50].Sample(UI2DSampler(), uv);
    case 51: return UITextures[51].Sample(UI2DSampler(), uv);
    case 52: return UITextures[52].Sample(UI2DSampler(), uv);
    case 53: return UITextures[53].Sample(UI2DSampler(), uv);
    case 54: return UITextures[54].Sample(UI2DSampler(), uv);
    case 55: return UITextures[55].Sample(UI2DSampler(), uv);
    case 56: return UITextures[56].Sample(UI2DSampler(), uv);
    case 57: return UITextures[57].Sample(UI2DSampler(), uv);
    case 58: return UITextures[58].Sample(UI2DSampler(), uv);
    case 59: return UITextures[59].Sample(UI2DSampler(), uv);
    case 60: return UITextures[60].Sample(UI2DSampler(), uv);
    case 61: return UITextures[61].Sample(UI2DSampler(), uv);
    case 62: return UITextures[62].Sample(UI2DSampler(), uv);
    case 63: return UITextures[63].Sample(UI2DSampler(), uv);
    case 64: return UITextures[64].Sample(UI2DSampler(), uv);
    case 65: return UITextures[65].Sample(UI2DSampler(), uv);
    case 66: return UITextures[66].Sample(UI2DSampler(), uv);
    case 67: return UITextures[67].Sample(UI2DSampler(), uv);
    case 68: return UITextures[68].Sample(UI2DSampler(), uv);
    case 69: return UITextures[69].Sample(UI2DSampler(), uv);
    case 70: return UITextures[70].Sample(UI2DSampler(), uv);
    case 71: return UITextures[71].Sample(UI2DSampler(), uv);
    case 72: return UITextures[72].Sample(UI2DSampler(), uv);
    case 73: return UITextures[73].Sample(UI2DSampler(), uv);
    case 74: return UITextures[74].Sample(UI2DSampler(), uv);
    case 75: return UITextures[75].Sample(UI2DSampler(), uv);
    case 76: return UITextures[76].Sample(UI2DSampler(), uv);
    case 77: return UITextures[77].Sample(UI2DSampler(), uv);
    case 78: return UITextures[78].Sample(UI2DSampler(), uv);
    case 79: return UITextures[79].Sample(UI2DSampler(), uv);
    case 80: return UITextures[80].Sample(UI2DSampler(), uv);
    case 81: return UITextures[81].Sample(UI2DSampler(), uv);
    case 82: return UITextures[82].Sample(UI2DSampler(), uv);
    case 83: return UITextures[83].Sample(UI2DSampler(), uv);
    case 84: return UITextures[84].Sample(UI2DSampler(), uv);
    case 85: return UITextures[85].Sample(UI2DSampler(), uv);
    case 86: return UITextures[86].Sample(UI2DSampler(), uv);
    case 87: return UITextures[87].Sample(UI2DSampler(), uv);
    case 88: return UITextures[88].Sample(UI2DSampler(), uv);
    case 89: return UITextures[89].Sample(UI2DSampler(), uv);
    case 90: return UITextures[90].Sample(UI2DSampler(), uv);
    case 91: return UITextures[91].Sample(UI2DSampler(), uv);
    case 92: return UITextures[92].Sample(UI2DSampler(), uv);
    case 93: return UITextures[93].Sample(UI2DSampler(), uv);
    case 94: return UITextures[94].Sample(UI2DSampler(), uv);
    case 95: return UITextures[95].Sample(UI2DSampler(), uv);
    case 96: return UITextures[96].Sample(UI2DSampler(), uv);
    case 97: return UITextures[97].Sample(UI2DSampler(), uv);
    case 98: return UITextures[98].Sample(UI2DSampler(), uv);
    case 99: return UITextures[99].Sample(UI2DSampler(), uv);
    case 100: return UITextures[100].Sample(UI2DSampler(), uv);
    case 101: return UITextures[101].Sample(UI2DSampler(), uv);
    case 102: return UITextures[102].Sample(UI2DSampler(), uv);
    case 103: return UITextures[103].Sample(UI2DSampler(), uv);
    case 104: return UITextures[104].Sample(UI2DSampler(), uv);
    case 105: return UITextures[105].Sample(UI2DSampler(), uv);
    case 106: return UITextures[106].Sample(UI2DSampler(), uv);
    case 107: return UITextures[107].Sample(UI2DSampler(), uv);
    case 108: return UITextures[108].Sample(UI2DSampler(), uv);
    case 109: return UITextures[109].Sample(UI2DSampler(), uv);
    case 110: return UITextures[110].Sample(UI2DSampler(), uv);
    case 111: return UITextures[111].Sample(UI2DSampler(), uv);
    case 112: return UITextures[112].Sample(UI2DSampler(), uv);
    case 113: return UITextures[113].Sample(UI2DSampler(), uv);
    case 114: return UITextures[114].Sample(UI2DSampler(), uv);
    case 115: return UITextures[115].Sample(UI2DSampler(), uv);
    case 116: return UITextures[116].Sample(UI2DSampler(), uv);
    case 117: return UITextures[117].Sample(UI2DSampler(), uv);
    case 118: return UITextures[118].Sample(UI2DSampler(), uv);
    case 119: return UITextures[119].Sample(UI2DSampler(), uv);
    case 120: return UITextures[120].Sample(UI2DSampler(), uv);
    case 121: return UITextures[121].Sample(UI2DSampler(), uv);
    case 122: return UITextures[122].Sample(UI2DSampler(), uv);
    case 123: return UITextures[123].Sample(UI2DSampler(), uv);
    case 124: return UITextures[124].Sample(UI2DSampler(), uv);
    case 125: return UITextures[125].Sample(UI2DSampler(), uv);
    case 126: return UITextures[126].Sample(UI2DSampler(), uv);
    case 127: return UITextures[127].Sample(UI2DSampler(), uv);
    default: return float4(1.0, 1.0, 1.0, 1.0);
    }
#endif
}

[shader("pixel")]
float4 ui_2d_pixel(UI2DVertexOutput input) : SV_TARGET
{
    const uint grayscale_flag = 0x80000000u;
    const uint texture_index = input.texture_index & 0x7fffffffu;
    float4 sampled = sample_ui_texture(texture_index, input.uv);
    if ((input.texture_index & grayscale_flag) != 0u) {
        const float luminance = dot(sampled.rgb, float3(0.299, 0.587, 0.114));
        sampled.rgb = float3(luminance, luminance, luminance);
    }
    return sampled * input.color;
}
