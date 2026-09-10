#include "resource_access.slangh"
struct VideoVertexInput
{
    float3 position : POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

struct VideoVertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

cbuffer MaterialData : register(b0)
{
    float4 MaterialColor;
    float4 VideoData;
    float4 MaterialReserved0;
    float4 MaterialReserved1;
};

GRAPHICS_TEXTURE_ARRAY(VideoTextures, float4, 0, 32)
GRAPHICS_SAMPLER(VideoSampler, 0)

float4 Sample_Video(uint index, float2 coordinates)
{
#if defined(GRAPHICS_DX12)
    if (index < 32u)
        return VideoTextures[NonUniformResourceIndex(index)].Sample(VideoSampler(), coordinates);
    return 0.0.xxxx;
#else
    switch (index)
    {
    case 0: return VideoTextures[0].Sample(VideoSampler(), coordinates);
    case 1: return VideoTextures[1].Sample(VideoSampler(), coordinates);
    case 2: return VideoTextures[2].Sample(VideoSampler(), coordinates);
    case 3: return VideoTextures[3].Sample(VideoSampler(), coordinates);
    case 4: return VideoTextures[4].Sample(VideoSampler(), coordinates);
    case 5: return VideoTextures[5].Sample(VideoSampler(), coordinates);
    case 6: return VideoTextures[6].Sample(VideoSampler(), coordinates);
    case 7: return VideoTextures[7].Sample(VideoSampler(), coordinates);
    case 8: return VideoTextures[8].Sample(VideoSampler(), coordinates);
    case 9: return VideoTextures[9].Sample(VideoSampler(), coordinates);
    case 10: return VideoTextures[10].Sample(VideoSampler(), coordinates);
    case 11: return VideoTextures[11].Sample(VideoSampler(), coordinates);
    case 12: return VideoTextures[12].Sample(VideoSampler(), coordinates);
    case 13: return VideoTextures[13].Sample(VideoSampler(), coordinates);
    case 14: return VideoTextures[14].Sample(VideoSampler(), coordinates);
    case 15: return VideoTextures[15].Sample(VideoSampler(), coordinates);
    case 16: return VideoTextures[16].Sample(VideoSampler(), coordinates);
    case 17: return VideoTextures[17].Sample(VideoSampler(), coordinates);
    case 18: return VideoTextures[18].Sample(VideoSampler(), coordinates);
    case 19: return VideoTextures[19].Sample(VideoSampler(), coordinates);
    case 20: return VideoTextures[20].Sample(VideoSampler(), coordinates);
    case 21: return VideoTextures[21].Sample(VideoSampler(), coordinates);
    case 22: return VideoTextures[22].Sample(VideoSampler(), coordinates);
    case 23: return VideoTextures[23].Sample(VideoSampler(), coordinates);
    case 24: return VideoTextures[24].Sample(VideoSampler(), coordinates);
    case 25: return VideoTextures[25].Sample(VideoSampler(), coordinates);
    case 26: return VideoTextures[26].Sample(VideoSampler(), coordinates);
    case 27: return VideoTextures[27].Sample(VideoSampler(), coordinates);
    case 28: return VideoTextures[28].Sample(VideoSampler(), coordinates);
    case 29: return VideoTextures[29].Sample(VideoSampler(), coordinates);
    case 30: return VideoTextures[30].Sample(VideoSampler(), coordinates);
    case 31: return VideoTextures[31].Sample(VideoSampler(), coordinates);
    default: return 0.0.xxxx;
    }
#endif
}

[shader("vertex")] VideoVertexOutput video_vertex(VideoVertexInput input)
{
    VideoVertexOutput output;
    output.position = float4(input.position, 1.0);
    output.color = input.color;
    output.uv = input.uv;
    return output;
}

[shader("pixel")] float4 video_pixel(VideoVertexOutput input) : SV_TARGET
{
    const uint texture_index = (uint)(VideoData.x + 0.5);
    return Sample_Video(texture_index, input.uv) * input.color * MaterialColor;
}
