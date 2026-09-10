module;
#include <cstdint>
export module Graphics.Materials.W3DMeshMaterial;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.TextureMapping;
import Graphics.Materials.State;
import Assets.Models;
import Assets.Adapters.W3D.Chunks;
import Assets.Adapters.W3D.Materials;

namespace Graphics {
// Asset decoding has already produced semantic color values. Conversion into
// draw parameters does not depend on game vectors, a file reader, or a device.
export void Apply_Mesh_Material_Values(MeshMaterial& target,const Assets::ModelMaterialDesc& source)
{
    auto& parameters=target.parameters;
    parameters.ambient={source.ambient_color.r,source.ambient_color.g,source.ambient_color.b};
    parameters.diffuse={source.base_color.r,source.base_color.g,source.base_color.b};
    parameters.specular={source.specular_color.r,source.specular_color.g,source.specular_color.b};
    parameters.emissive={source.emissive_color.r,source.emissive_color.g,source.emissive_color.b};
    parameters.opacity=source.opacity;
    parameters.shininess=source.shininess;
}

export void Apply_W3D_Mesh_Material(MeshMaterial& target,const Assets::W3D::W3DVertexMaterialData& source,
    std::uint32_t now,float (*random_sample)()=nullptr,
    float (*bump_sine)(float)=nullptr,float (*bump_cosine)(float)=nullptr)
{
    Apply_Mesh_Material_Values(target,source.material);
    if (source.has_name) target.name=source.material.name;
    // Loading enables these authored flags without clearing pre-existing ones.
    // Depth-cue-to-alpha has never been enabled by this file record.
    target.flags|=source.material.source_attributes&5u;
    for (unsigned stage=0;stage<source.mappings.size();++stage)
        if (source.mappings[stage]) target.mappings[stage]=TextureMapping::Create(*source.mappings[stage],now,
            random_sample,bump_sine,bump_cosine);
}

// The mesh loader supplies the resulting shader and selected map to its
// legacy material table. Map handling stays ordered because a later authored
// diffuse or qualifying specular illumination map replaces the earlier one.
export struct W3DMaterial3Runtime final
{
    MaterialState shader;
    const Assets::W3D::W3DMaterial3MapData *texture_map = nullptr;
    bool requires_sort = false;
    bool has_animated_texture = false;
};

export W3DMaterial3Runtime Apply_W3D_Material3(MeshMaterial &target,
    const Assets::W3D::W3DMaterial3Data &source)
{
    Apply_Mesh_Material_Values(target, source.material);
    target.name = source.material.name;

    W3DMaterial3Runtime result;
    if ((source.attributes & Assets::W3D::W3DMaterialUseAlpha) != 0) {
        result.shader.Set_Depth_Mask(MaterialState::DEPTH_WRITE_DISABLE);
        result.shader.Set_Dst_Blend_Func(MaterialState::DSTBLEND_ONE_MINUS_SRC_ALPHA);
        result.shader.Set_Src_Blend_Func(MaterialState::SRCBLEND_SRC_ALPHA);
    }
    result.requires_sort = result.shader.Get_Dst_Blend_Func() != MaterialState::DSTBLEND_ZERO;

    for (const auto &map : source.maps) {
        const bool is_diffuse_map = map.kind == Assets::W3D::W3DMaterial3MapKind::DiffuseColor;
        const bool is_specular_illumination_map =
            map.kind == Assets::W3D::W3DMaterial3MapKind::SpecularIllumination;
        if (!is_diffuse_map && !is_specular_illumination_map)
            continue;
        if (is_specular_illumination_map &&
            (target.parameters.diffuse[0] != 0.0f ||
                target.parameters.diffuse[1] != 0.0f ||
                target.parameters.diffuse[2] != 0.0f))
            continue;

        if (map.frame_count > 1)
            result.has_animated_texture = true;
        result.texture_map = &map;
        result.shader.Set_Texturing(MaterialState::TEXTURING_ENABLE);
        if (is_specular_illumination_map) {
            result.shader.Set_Dst_Blend_Func(MaterialState::DSTBLEND_ONE);
            result.shader.Set_Src_Blend_Func(MaterialState::SRCBLEND_ONE);
            result.shader.Set_Primary_Gradient(MaterialState::GRADIENT_DISABLE);
        }
    }

    // Preserve the Material3 solid-color behavior when no usable map exists.
    if (result.shader.Get_Texturing() == MaterialState::TEXTURING_DISABLE) {
        target.parameters.ambient = target.parameters.diffuse;
        target.parameters.diffuse = {0, 0, 0};
    }
    return result;
}
}
