#include "resource_access.slangh"
struct BasicOpaqueVertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct BasicOpaqueVertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float3 world_position : TEXCOORD1;
    nointerpolation uint material_index : TEXCOORD2;
    float2 uv : TEXCOORD3;
};

struct SkinnedOpaqueVertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    uint4 bone_indices : BLENDINDICES0;
    float4 bone_weights : BLENDWEIGHT0;
};

struct GPUInstanceData
{
    row_major float4x4 transform;
    float4 bounds;
    uint mesh_index;
    uint material_index;
    uint flags;
    uint reserved;
    uint bone_matrix_index;
    uint bone_matrix_count;
    uint bone_reserved0;
    uint bone_reserved1;
};

struct GPUBoneMatrixData
{
    row_major float4x4 transform;
};

struct GPUMaterialData
{
    float4 parameters[4];
    uint texture_indices[8];
    uint sampler_indices[8];
    uint flags;
    uint reserved[3];
};

struct GPUDrawData
{
    uint instance_index;
    uint material_index;
    uint mesh_index;
    uint flags;
};

GRAPHICS_STORAGE_BUFFER(Instances, GPUInstanceData, 127)

struct GPULightData
{
    float4 position_range;
    float4 direction_intensity;
    float4 color_inner_angle;
    float outer_angle;
    uint type;
    uint flags;
    uint shadow_data_index;
};

GRAPHICS_STORAGE_BUFFER(Lights, GPULightData, 126)

struct GPUViewData
{
    row_major float4x4 view_projection;
    float4 camera_position;
    float4 fog_color_density;
    float4 fog_start_end;
};

GRAPHICS_STORAGE_BUFFER(Views, GPUViewData, 125)

GRAPHICS_STORAGE_BUFFER(BoneMatrices, GPUBoneMatrixData, 124)
GRAPHICS_STORAGE_BUFFER(Materials, GPUMaterialData, 123)
GRAPHICS_STORAGE_BUFFER(Draws, GPUDrawData, 122)
GRAPHICS_TEXTURE_ARRAY(Textures, float4, 0, 122)
GRAPHICS_SAMPLER(MaterialSampler, 0)

cbuffer DrawConstants : register(b1)
{
    uint draw_index;
    uint3 draw_reserved;
};

BasicOpaqueVertexOutput basic_opaque_vertex(BasicOpaqueVertexInput input)
{
    BasicOpaqueVertexOutput output;
    const GPUDrawData draw = Draws()[draw_index];
    const GPUInstanceData instance = Instances()[draw.instance_index];
    const float4 world_position = mul(instance.transform, float4(input.position, 1.0));
    output.position = mul(Views()[0].view_projection, world_position);
    output.color = input.color;
    output.world_position = world_position.xyz;
    output.material_index = draw.material_index;
    output.uv = input.uv;
    return output;
}

BasicOpaqueVertexOutput basic_opaque_skinned_vertex(SkinnedOpaqueVertexInput input)
{
    BasicOpaqueVertexOutput output;
    const GPUDrawData draw = Draws()[draw_index];
    const GPUInstanceData instance = Instances()[draw.instance_index];
    float4 skinned_position = 0.0.xxxx;
    if (instance.bone_matrix_count != 0)
    {
        skinned_position += mul(BoneMatrices()[instance.bone_matrix_index + input.bone_indices.x].transform, float4(input.position, 1.0)) * input.bone_weights.x;
        skinned_position += mul(BoneMatrices()[instance.bone_matrix_index + input.bone_indices.y].transform, float4(input.position, 1.0)) * input.bone_weights.y;
        skinned_position += mul(BoneMatrices()[instance.bone_matrix_index + input.bone_indices.z].transform, float4(input.position, 1.0)) * input.bone_weights.z;
        skinned_position += mul(BoneMatrices()[instance.bone_matrix_index + input.bone_indices.w].transform, float4(input.position, 1.0)) * input.bone_weights.w;
    }
    else
    {
        skinned_position = float4(input.position, 1.0);
    }
    const float4 world_position = mul(instance.transform, skinned_position);
    output.position = mul(Views()[0].view_projection, world_position);
    output.color = input.color;
    output.world_position = world_position.xyz;
    output.material_index = draw.material_index;
    output.uv = input.uv;
    return output;
}

float4 basic_opaque_pixel(BasicOpaqueVertexOutput input) : SV_TARGET
{
    float3 light_color = 1.0.xxx;
    const GPUMaterialData material = Materials()[input.material_index];
    float4 albedo = 1.0.xxxx;
    const uint texture_index = material.texture_indices[0];
#if defined(GRAPHICS_DX12)
    if (texture_index < 122u)
        albedo = Textures[NonUniformResourceIndex(texture_index)].Sample(MaterialSampler(), input.uv);
#else
    switch (texture_index)
    {
        case 0: albedo = Textures[0].Sample(MaterialSampler(), input.uv); break;
        case 1: albedo = Textures[1].Sample(MaterialSampler(), input.uv); break;
        case 2: albedo = Textures[2].Sample(MaterialSampler(), input.uv); break;
        case 3: albedo = Textures[3].Sample(MaterialSampler(), input.uv); break;
        case 4: albedo = Textures[4].Sample(MaterialSampler(), input.uv); break;
        case 5: albedo = Textures[5].Sample(MaterialSampler(), input.uv); break;
        case 6: albedo = Textures[6].Sample(MaterialSampler(), input.uv); break;
        case 7: albedo = Textures[7].Sample(MaterialSampler(), input.uv); break;
        case 8: albedo = Textures[8].Sample(MaterialSampler(), input.uv); break;
        case 9: albedo = Textures[9].Sample(MaterialSampler(), input.uv); break;
        case 10: albedo = Textures[10].Sample(MaterialSampler(), input.uv); break;
        case 11: albedo = Textures[11].Sample(MaterialSampler(), input.uv); break;
        case 12: albedo = Textures[12].Sample(MaterialSampler(), input.uv); break;
        case 13: albedo = Textures[13].Sample(MaterialSampler(), input.uv); break;
        case 14: albedo = Textures[14].Sample(MaterialSampler(), input.uv); break;
        case 15: albedo = Textures[15].Sample(MaterialSampler(), input.uv); break;
        case 16: albedo = Textures[16].Sample(MaterialSampler(), input.uv); break;
        case 17: albedo = Textures[17].Sample(MaterialSampler(), input.uv); break;
        case 18: albedo = Textures[18].Sample(MaterialSampler(), input.uv); break;
        case 19: albedo = Textures[19].Sample(MaterialSampler(), input.uv); break;
        case 20: albedo = Textures[20].Sample(MaterialSampler(), input.uv); break;
        case 21: albedo = Textures[21].Sample(MaterialSampler(), input.uv); break;
        case 22: albedo = Textures[22].Sample(MaterialSampler(), input.uv); break;
        case 23: albedo = Textures[23].Sample(MaterialSampler(), input.uv); break;
        case 24: albedo = Textures[24].Sample(MaterialSampler(), input.uv); break;
        case 25: albedo = Textures[25].Sample(MaterialSampler(), input.uv); break;
        case 26: albedo = Textures[26].Sample(MaterialSampler(), input.uv); break;
        case 27: albedo = Textures[27].Sample(MaterialSampler(), input.uv); break;
        case 28: albedo = Textures[28].Sample(MaterialSampler(), input.uv); break;
        case 29: albedo = Textures[29].Sample(MaterialSampler(), input.uv); break;
        case 30: albedo = Textures[30].Sample(MaterialSampler(), input.uv); break;
        case 31: albedo = Textures[31].Sample(MaterialSampler(), input.uv); break;
        case 32: albedo = Textures[32].Sample(MaterialSampler(), input.uv); break;
        case 33: albedo = Textures[33].Sample(MaterialSampler(), input.uv); break;
        case 34: albedo = Textures[34].Sample(MaterialSampler(), input.uv); break;
        case 35: albedo = Textures[35].Sample(MaterialSampler(), input.uv); break;
        case 36: albedo = Textures[36].Sample(MaterialSampler(), input.uv); break;
        case 37: albedo = Textures[37].Sample(MaterialSampler(), input.uv); break;
        case 38: albedo = Textures[38].Sample(MaterialSampler(), input.uv); break;
        case 39: albedo = Textures[39].Sample(MaterialSampler(), input.uv); break;
        case 40: albedo = Textures[40].Sample(MaterialSampler(), input.uv); break;
        case 41: albedo = Textures[41].Sample(MaterialSampler(), input.uv); break;
        case 42: albedo = Textures[42].Sample(MaterialSampler(), input.uv); break;
        case 43: albedo = Textures[43].Sample(MaterialSampler(), input.uv); break;
        case 44: albedo = Textures[44].Sample(MaterialSampler(), input.uv); break;
        case 45: albedo = Textures[45].Sample(MaterialSampler(), input.uv); break;
        case 46: albedo = Textures[46].Sample(MaterialSampler(), input.uv); break;
        case 47: albedo = Textures[47].Sample(MaterialSampler(), input.uv); break;
        case 48: albedo = Textures[48].Sample(MaterialSampler(), input.uv); break;
        case 49: albedo = Textures[49].Sample(MaterialSampler(), input.uv); break;
        case 50: albedo = Textures[50].Sample(MaterialSampler(), input.uv); break;
        case 51: albedo = Textures[51].Sample(MaterialSampler(), input.uv); break;
        case 52: albedo = Textures[52].Sample(MaterialSampler(), input.uv); break;
        case 53: albedo = Textures[53].Sample(MaterialSampler(), input.uv); break;
        case 54: albedo = Textures[54].Sample(MaterialSampler(), input.uv); break;
        case 55: albedo = Textures[55].Sample(MaterialSampler(), input.uv); break;
        case 56: albedo = Textures[56].Sample(MaterialSampler(), input.uv); break;
        case 57: albedo = Textures[57].Sample(MaterialSampler(), input.uv); break;
        case 58: albedo = Textures[58].Sample(MaterialSampler(), input.uv); break;
        case 59: albedo = Textures[59].Sample(MaterialSampler(), input.uv); break;
        case 60: albedo = Textures[60].Sample(MaterialSampler(), input.uv); break;
        case 61: albedo = Textures[61].Sample(MaterialSampler(), input.uv); break;
        case 62: albedo = Textures[62].Sample(MaterialSampler(), input.uv); break;
        case 63: albedo = Textures[63].Sample(MaterialSampler(), input.uv); break;
        case 64: albedo = Textures[64].Sample(MaterialSampler(), input.uv); break;
        case 65: albedo = Textures[65].Sample(MaterialSampler(), input.uv); break;
        case 66: albedo = Textures[66].Sample(MaterialSampler(), input.uv); break;
        case 67: albedo = Textures[67].Sample(MaterialSampler(), input.uv); break;
        case 68: albedo = Textures[68].Sample(MaterialSampler(), input.uv); break;
        case 69: albedo = Textures[69].Sample(MaterialSampler(), input.uv); break;
        case 70: albedo = Textures[70].Sample(MaterialSampler(), input.uv); break;
        case 71: albedo = Textures[71].Sample(MaterialSampler(), input.uv); break;
        case 72: albedo = Textures[72].Sample(MaterialSampler(), input.uv); break;
        case 73: albedo = Textures[73].Sample(MaterialSampler(), input.uv); break;
        case 74: albedo = Textures[74].Sample(MaterialSampler(), input.uv); break;
        case 75: albedo = Textures[75].Sample(MaterialSampler(), input.uv); break;
        case 76: albedo = Textures[76].Sample(MaterialSampler(), input.uv); break;
        case 77: albedo = Textures[77].Sample(MaterialSampler(), input.uv); break;
        case 78: albedo = Textures[78].Sample(MaterialSampler(), input.uv); break;
        case 79: albedo = Textures[79].Sample(MaterialSampler(), input.uv); break;
        case 80: albedo = Textures[80].Sample(MaterialSampler(), input.uv); break;
        case 81: albedo = Textures[81].Sample(MaterialSampler(), input.uv); break;
        case 82: albedo = Textures[82].Sample(MaterialSampler(), input.uv); break;
        case 83: albedo = Textures[83].Sample(MaterialSampler(), input.uv); break;
        case 84: albedo = Textures[84].Sample(MaterialSampler(), input.uv); break;
        case 85: albedo = Textures[85].Sample(MaterialSampler(), input.uv); break;
        case 86: albedo = Textures[86].Sample(MaterialSampler(), input.uv); break;
        case 87: albedo = Textures[87].Sample(MaterialSampler(), input.uv); break;
        case 88: albedo = Textures[88].Sample(MaterialSampler(), input.uv); break;
        case 89: albedo = Textures[89].Sample(MaterialSampler(), input.uv); break;
        case 90: albedo = Textures[90].Sample(MaterialSampler(), input.uv); break;
        case 91: albedo = Textures[91].Sample(MaterialSampler(), input.uv); break;
        case 92: albedo = Textures[92].Sample(MaterialSampler(), input.uv); break;
        case 93: albedo = Textures[93].Sample(MaterialSampler(), input.uv); break;
        case 94: albedo = Textures[94].Sample(MaterialSampler(), input.uv); break;
        case 95: albedo = Textures[95].Sample(MaterialSampler(), input.uv); break;
        case 96: albedo = Textures[96].Sample(MaterialSampler(), input.uv); break;
        case 97: albedo = Textures[97].Sample(MaterialSampler(), input.uv); break;
        case 98: albedo = Textures[98].Sample(MaterialSampler(), input.uv); break;
        case 99: albedo = Textures[99].Sample(MaterialSampler(), input.uv); break;
        case 100: albedo = Textures[100].Sample(MaterialSampler(), input.uv); break;
        case 101: albedo = Textures[101].Sample(MaterialSampler(), input.uv); break;
        case 102: albedo = Textures[102].Sample(MaterialSampler(), input.uv); break;
        case 103: albedo = Textures[103].Sample(MaterialSampler(), input.uv); break;
        case 104: albedo = Textures[104].Sample(MaterialSampler(), input.uv); break;
        case 105: albedo = Textures[105].Sample(MaterialSampler(), input.uv); break;
        case 106: albedo = Textures[106].Sample(MaterialSampler(), input.uv); break;
        case 107: albedo = Textures[107].Sample(MaterialSampler(), input.uv); break;
        case 108: albedo = Textures[108].Sample(MaterialSampler(), input.uv); break;
        case 109: albedo = Textures[109].Sample(MaterialSampler(), input.uv); break;
        case 110: albedo = Textures[110].Sample(MaterialSampler(), input.uv); break;
        case 111: albedo = Textures[111].Sample(MaterialSampler(), input.uv); break;
        case 112: albedo = Textures[112].Sample(MaterialSampler(), input.uv); break;
        case 113: albedo = Textures[113].Sample(MaterialSampler(), input.uv); break;
        case 114: albedo = Textures[114].Sample(MaterialSampler(), input.uv); break;
        case 115: albedo = Textures[115].Sample(MaterialSampler(), input.uv); break;
        case 116: albedo = Textures[116].Sample(MaterialSampler(), input.uv); break;
        case 117: albedo = Textures[117].Sample(MaterialSampler(), input.uv); break;
        case 118: albedo = Textures[118].Sample(MaterialSampler(), input.uv); break;
        case 119: albedo = Textures[119].Sample(MaterialSampler(), input.uv); break;
        case 120: albedo = Textures[120].Sample(MaterialSampler(), input.uv); break;
        case 121: albedo = Textures[121].Sample(MaterialSampler(), input.uv); break;
    default: break;
    }
#endif
    if (material.parameters[1].x > 0.5)
    {
        const GPULightData light = Lights()[0];
        const float distance_to_light = length(input.world_position - light.position_range.xyz);
        const float attenuation = saturate(1.0 - distance_to_light / max(light.position_range.w, 0.0001));
        const float enabled = (light.flags & 1) != 0 ? 1.0 : 0.0;
        light_color = light.color_inner_angle.rgb * light.direction_intensity.w * (0.2 + 0.8 * attenuation) * enabled;
    }
    const float4 color = float4(input.color.rgb * material.parameters[0].rgb * albedo.rgb * light_color,
        input.color.a * material.parameters[0].a * albedo.a);
    if ((material.flags & (1u << 1)) != 0 && color.a < 0.5)
        discard;

    float4 output = color;
    if ((material.flags & (1u << 7)) != 0 && Views()[0].fog_start_end.z > 0.5)
    {
        const float fog_distance = length(input.world_position - Views()[0].camera_position.xyz);
        const float fog_factor = saturate((fog_distance - Views()[0].fog_start_end.x)
            / max(Views()[0].fog_start_end.y - Views()[0].fog_start_end.x, 0.0001));
        output.rgb = lerp(output.rgb, Views()[0].fog_color_density.rgb, fog_factor);
    }
    return output;
}
