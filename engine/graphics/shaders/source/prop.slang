#include "resource_access.slangh"
#include "environment_lighting.slangh"
cbuffer PropViewConstants : register(b0)
{
    row_major float4x4 ViewProjection;
    float4 ShroudProjection;
    float4 FogColor;
    float4 FogState;
    float4 CameraPosition;
    row_major float4x4 View;
};
struct PropLightingData {

    float4 SceneAmbient;
    float4 LightDirection[4];
    float4 LightDiffuse[4];
    float4 LightSpecular[4];
    float4 LightPosition[4];
    float4 LightAttenuation[4];
    float4 LightAmbient[4];
    float4 LightSpot[4];
};
cbuffer PropLightingConstants : register(b1) { PropLightingData SharedLighting; };
cbuffer PropMaterialConstants : register(b2)
{
    float Textured;
    float SecondaryTexture;
    float AlphaCutoff;
    float PrimaryGradient;
    float DetailColor;
    float DetailAlpha;
    float SecondaryGradient;
    float Shroud;
    float ShroudOnly;
    float Opacity;
    float TextureLuminance;
    float NormalInWorldSpace;
    row_major float4x4 UVTransform[2];
    float4 UVSources;
    float4 BumpMatrix;
    float SurfaceModel;
    uint SurfaceMaps;
    float SurfaceNormalScale;
    float SurfaceNormalFlipGreen;
    float SurfaceSpecularScale;
    float SurfaceEmissiveScale;
    float SurfaceRoughness;
    float SurfaceMetallic;
    float SurfaceOcclusionStrength;
    uint SurfaceSpecularChannel;
    uint SurfaceTeamChannel;
    float SurfaceTeamMultiplier;
    float4 SurfaceTeamColor;
    float4 MuzzleFlashState;
};
cbuffer PropObjectConstants : register(b3)
{
    row_major float4x4 World;
};
GRAPHICS_TEXTURE_2D(BaseTexture, float4, 0)
GRAPHICS_TEXTURE_2D(DetailTexture, float4, 1)
GRAPHICS_TEXTURE_2D(ShroudTexture, float4, 3)
GRAPHICS_SAMPLER(BaseSampler, 0)
GRAPHICS_SAMPLER(DetailSampler, 1)
GRAPHICS_SAMPLER(ShroudSampler, 3)
struct PropInput
{
    float3 position : POSITION0;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
    float2 secondary_uv : TEXCOORD1;
    float3 normal : NORMAL0;
    float4 material_ambient : COLOR1;
    float4 material_diffuse : COLOR2;
    float4 material_emissive : COLOR3;
    float4 material_specular : COLOR4;
    float4 secondary_color : COLOR5;
    float4 tangent : TEXCOORD2;
    float bone_index : TEXCOORD3;
};
struct PropOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
    float4 secondary_color : COLOR1;
    float4 material_ambient : COLOR2;
    float4 material_diffuse : COLOR3;
    float4 material_emissive : COLOR4;
    float4 material_specular : COLOR5;
    float3 normal : TEXCOORD4;
    float3 world_position : TEXCOORD5;
    float3 camera_position : TEXCOORD6;
    float3 camera_normal : TEXCOORD7;
    float3 reflection : TEXCOORD8;
    float2 uv : TEXCOORD0;
    float2 secondary_uv : TEXCOORD1;
    float2 shroud_uv : TEXCOORD2;
    float distance : TEXCOORD3;
    float4 tangent : TEXCOORD9;
    nointerpolation uint record_index : TEXCOORD10;
};

float2 Rotate_Muzzle_Flash_UV(float2 uv, float2 pivot, float angle)
{
    float sine, cosine;
    sincos(angle, sine, cosine);
    const float2 centered = uv - pivot;
    // Match the row-vector rotation used by the RA3 source shader.
    return float2(centered.x * cosine + centered.y * sine,
        -centered.x * sine + centered.y * cosine) + pivot;
}

void Prepare_Muzzle_Flash_UV(float2 source_uv,
    out float2 primary_uv, out float2 secondary_uv)
{
    const float2 pivot = MuzzleFlashState.zw;
    primary_uv = Rotate_Muzzle_Flash_UV(source_uv, pivot, MuzzleFlashState.y);
    // The second sample counter-rotates around the same authored pivot.
    secondary_uv = Rotate_Muzzle_Flash_UV(source_uv, pivot, -MuzzleFlashState.y);
}
PropOutput Transform_Prop_Vertex(PropInput input, row_major float4x4 WorldTransform)
{
    PropOutput output;
    output.record_index = 0;
    float3 world_position = mul(WorldTransform,float4(input.position,1)).xyz;
    float3 world_normal = NormalInWorldSpace > 0.5 ? input.normal : mul(WorldTransform,float4(input.normal,0)).xyz;
    output.position = mul(ViewProjection,float4(world_position,1));
    output.color = input.color;
    output.secondary_color = input.secondary_color;
    output.normal = normalize(world_normal);
    output.tangent = float4(mul(WorldTransform,float4(input.tangent.xyz,0)).xyz,input.tangent.w);
    if (SurfaceModel > .5 && NormalInWorldSpace < .5) {
        const float3 r0=WorldTransform[0].xyz, r1=WorldTransform[1].xyz, r2=WorldTransform[2].xyz;
        const float determinant=dot(r0,cross(r1,r2));
        const float orientation=determinant < 0 ? -1 : 1;
        const float3x3 cofactor=float3x3(cross(r1,r2),cross(r2,r0),cross(r0,r1));
        output.normal=normalize(mul(cofactor,input.normal)*orientation);
        output.tangent.w*=orientation;
    }
    output.camera_position = mul(View,float4(world_position,1)).xyz;
    output.camera_normal = normalize(mul(View,float4(world_normal,0)).xyz);
    output.reflection = reflect(normalize(output.camera_position),output.camera_normal);
    output.world_position = world_position;
    output.material_ambient = input.material_ambient;
    output.material_diffuse = input.material_diffuse;
    output.material_emissive = input.material_emissive;
    output.material_specular = input.material_specular;
    if (MuzzleFlashState.x > 0.5)
        Prepare_Muzzle_Flash_UV(input.uv, output.uv, output.secondary_uv);
    else {
        output.uv = input.uv;
        output.secondary_uv = input.secondary_uv;
    }
    if ((SurfaceMaps & 128u) != 0) {
        output.uv.x += input.color.a;
        output.color.a = 1.0;
    }
    output.shroud_uv = world_position.xy*ShroudProjection.xy+ShroudProjection.zw;
    output.distance = length(CameraPosition.xyz-world_position);
    return output;
}
struct PropInstance { row_major float4x4 world; };
GRAPHICS_STORAGE_BUFFER(PropInstances, PropInstance, 127)
[shader("vertex")] PropOutput prop_vertex(PropInput input) {
    return Transform_Prop_Vertex(input, World);
}
[shader("vertex")] PropOutput prop_vertex_instanced(PropInput input, uint instance : SV_InstanceID) {
    return Transform_Prop_Vertex(input, PropInstances()[instance].world);
}

struct PropInstanceRecord {
    row_major float4x4 world;
    PropLightingData lighting;
    uint4 skin;
};
GRAPHICS_STORAGE_BUFFER(PropRecords, PropInstanceRecord, 127)
GRAPHICS_STORAGE_BUFFER(PropRecordIndices, uint, 126)
struct PropBone { row_major float3x4 transform; };
GRAPHICS_STORAGE_BUFFER(PropSkinPalette, PropBone, 125)
[shader("vertex")] PropOutput prop_vertex_records(PropInput input, uint instance : SV_InstanceID) {
    const uint index = PropRecordIndices()[instance];
    const PropInstanceRecord record = PropRecords()[index];
    if (record.skin.y != 0) {
        const row_major float3x4 bone = PropSkinPalette()[record.skin.x+uint(input.bone_index)].transform;
        input.position = mul(bone,float4(input.position,1));
        if (NormalInWorldSpace < .5) input.normal = mul(bone,float4(input.normal,0));
        input.tangent.xyz = mul(bone,float4(input.tangent.xyz,0));
    }
    PropOutput output = Transform_Prop_Vertex(input, record.world);
    output.record_index = index;
    return output;
}

float2 Prop_Coordinate(uint stage,PropOutput input)
{
    float mode = stage==0 ? UVSources.x : UVSources.z;
    float projected = stage==0 ? UVSources.y : UVSources.w;
    float4 coordinate = float4(stage==0 ? input.uv : input.secondary_uv,0,1);
    if (mode==1) coordinate = float4(input.camera_position,1);
    else if (mode==2) coordinate = float4(input.camera_normal,1);
    else if (mode==3) coordinate = float4(input.reflection,1);
    else if (mode==4) coordinate = float4(input.world_position,1);
    coordinate = mul(UVTransform[stage],coordinate);
    if (projected > 0.5) coordinate.xyz /= max(abs(coordinate.w),0.000001);
    return coordinate.xy;
}
#include "prop_surface.slangh"
float4 Shade_Prop_Pixel(PropOutput input, PropLightingData lighting)
{
    ClipEnvironment(input.world_position);
    if (ShroudOnly > 0.5) return float4(ShroudTexture().Sample(ShroudSampler(),input.shroud_uv).rgb,1);
    if (MuzzleFlashState.x > 0.5 && Textured > 0.5 && SecondaryTexture > 0.5) {
        float4 color = BaseTexture().Sample(BaseSampler(),Prop_Coordinate(0,input));
        color *= DetailTexture().Sample(DetailSampler(),Prop_Coordinate(1,input));
        const float vertex_alpha = input.color.a;
        const float material_alpha = input.material_diffuse.a;
        const float3 authored_rgb = input.color.rgb*input.material_diffuse.rgb
            + input.material_emissive.rgb;
        color.rgb *= authored_rgb*Opacity;
        color.a *= vertex_alpha*material_alpha*Opacity;
        if (Shroud > 0.5) color.rgb *= ShroudTexture().Sample(ShroudSampler(),input.shroud_uv).rgb;
        if (FogState.z > 0.5) {
            const float fog = saturate((abs(input.camera_position.z)-FogState.x)/max(FogState.y-FogState.x,0.0001));
            color.rgb = lerp(color.rgb,FogColor.rgb,fog);
        }
        return saturate(color);
    }
    if (SurfaceModel > .5) {
        float4 surface=Shade_Prop_Surface(input, lighting);
        surface.a*=Opacity;
        clip(surface.a-AlphaCutoff);
        if (Shroud > .5) surface.rgb*=ShroudTexture().Sample(ShroudSampler(),input.shroud_uv).rgb;
        if (FogState.z > .5) {
            const float fog=saturate((abs(input.camera_position.z)-FogState.x)/max(FogState.y-FogState.x,.0001));
            surface.rgb=lerp(surface.rgb,FogColor.rgb,fog);
        }
        return saturate(surface);
    }
    float4 diffuse = input.color*input.material_diffuse;
    if (input.material_ambient.w > 0.5) {
        float3 normal = normalize(input.normal);
        float3 view_direction = normalize(CameraPosition.xyz-input.world_position);
        float3 lit = lighting.SceneAmbient.rgb*input.material_ambient.rgb+input.material_emissive.rgb;
        float3 specular = 0;
        const float3 cloud_visibility = DirectionalVisibility(input.world_position);
        for (uint light=0;light<4;++light) {
            if (lighting.LightDirection[light].w < 0.5) continue;
            float3 direction = normalize(lighting.LightDirection[light].xyz);
            float attenuation = 1;
            if (lighting.LightPosition[light].w > 0.5) {
                const float3 delta = lighting.LightPosition[light].xyz-input.world_position;
                const float distance = length(delta);
                direction = distance > 0.0001 ? delta/distance : float3(0,0,1);
                const float4 coefficients = lighting.LightAttenuation[light];
                attenuation = 1/max(coefficients.x+coefficients.y*distance
                    +coefficients.z*distance*distance,0.0001);
                if (coefficients.w > 0 && distance > coefficients.w) attenuation = 0;
                if (lighting.LightPosition[light].w > 1.5) {
                    const float cosine = dot(-direction,normalize(lighting.LightDirection[light].xyz));
                    const float inner = cos(0.5*lighting.LightSpot[light].y);
                    const float outer = cos(0.5*lighting.LightSpot[light].z);
                    attenuation *= pow(saturate((cosine-outer)/max(inner-outer,0.0001)),
                        max(lighting.LightSpot[light].x,0));
                }
            }
            float amount = saturate(dot(normal,direction));
            const float3 visibility = lighting.LightPosition[light].w < 0.5 ? cloud_visibility : float3(1,1,1);
            lit += attenuation*(lighting.LightAmbient[light].rgb*input.material_ambient.rgb
                +amount*lighting.LightDiffuse[light].rgb*input.material_diffuse.rgb*visibility);
            if (SecondaryGradient > 0.5 && amount > 0) {
                float3 half_vector = normalize(direction+view_direction);
                specular += pow(saturate(dot(normal,half_vector)),max(input.material_specular.w,1.0))
                    *lighting.LightSpecular[light].rgb*input.material_specular.rgb*attenuation*visibility;
            }
        }
        diffuse = float4(saturate(saturate(lit)+specular),input.material_diffuse.a);
    }
    float4 color = Textured > 0.5 ? BaseTexture().Sample(BaseSampler(),Prop_Coordinate(0,input)) : float4(1,1,1,1);
    float2 bump_offset = 0;
    if (Textured < 0.5) color = diffuse;
    else if (PrimaryGradient == 1) color *= diffuse;
    else if (PrimaryGradient == 2) color = float4(color.rgb+diffuse.rgb,diffuse.a);
    else if (PrimaryGradient == 5) color *= diffuse*float4(2,2,2,1);
    else if (PrimaryGradient == 3 || PrimaryGradient == 4) {
        float2 signed_bump = color.rg*2-1;
        bump_offset = float2(dot(signed_bump,BumpMatrix.xy),dot(signed_bump,BumpMatrix.zw));
        color = diffuse;
    }
    if (TextureLuminance > 0.5) {
        // Preserve the packed texture-factor multiply-add / dot-product material.
        const float3 factor = float3(165,202,142)/255.0;
        const float3 shifted = BaseTexture().Sample(BaseSampler(),Prop_Coordinate(0,input)).rgb*(128.0/255.0)+(128.0/255.0)-0.5;
        color.rgb = saturate(4*dot(shifted,factor-0.5)).xxx;
    }
    if (SecondaryTexture > 0.5) {
        float4 detail = DetailTexture().Sample(DetailSampler(),Prop_Coordinate(1,input)+bump_offset);
        float3 local = color.rgb;
        if (DetailColor == 1) color.rgb = detail.rgb;
        else if (DetailColor == 2) color.rgb *= detail.rgb;
        else if (DetailColor == 3) color.rgb = local+(1-local)*detail.rgb;
        else if (DetailColor == 4) color.rgb += detail.rgb;
        else if (DetailColor == 5) color.rgb -= detail.rgb;
        else if (DetailColor == 6) color.rgb = detail.rgb-local;
        else if (DetailColor == 7) color.rgb = lerp(detail.rgb,local,color.a);
        else if (DetailColor == 8) color.rgb = lerp(detail.rgb,local,detail.a);
        else if (DetailColor == 9) color.rgb += detail.rgb-0.5;
        else if (DetailColor == 10) color.rgb = (local+detail.rgb-0.5)*2;
        else if (DetailColor == 11) color.rgb *= detail.rgb*2;
        else if (DetailColor == 12) color.rgb += color.a*detail.rgb;
        if (DetailAlpha == 1) color.a = detail.a;
        else if (DetailAlpha == 2) color.a *= detail.a;
        else if (DetailAlpha == 3) color.a += (1-color.a)*detail.a;
    }
    if (SecondaryGradient > 0.5) color.rgb += input.secondary_color.rgb;
    color.a *= Opacity;
    clip(color.a-AlphaCutoff);
    if (Shroud > 0.5) color.rgb *= ShroudTexture().Sample(ShroudSampler(),input.shroud_uv).rgb;
    if (FogState.z > 0.5) {
        const float fog = saturate((abs(input.camera_position.z)-FogState.x)/max(FogState.y-FogState.x,0.0001));
        color.rgb = lerp(color.rgb,FogColor.rgb,fog);
    }
    return saturate(color);
}

[shader("pixel")] float4 prop_pixel(PropOutput input) : SV_TARGET {
    return Shade_Prop_Pixel(input, SharedLighting);
}
[shader("pixel")] float4 prop_pixel_records(PropOutput input) : SV_TARGET {
    return Shade_Prop_Pixel(input, PropRecords()[input.record_index].lighting);
}
