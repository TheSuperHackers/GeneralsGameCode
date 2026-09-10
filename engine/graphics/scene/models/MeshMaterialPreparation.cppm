module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

export module Graphics.Scene.Models.MeshMaterialPreparation;

import Assets.Math;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.State;
import Graphics.Materials.Ordering;
import Graphics.Scene.Models.MeshMaterialBindings;
import Graphics.Scene.Props.Material;

namespace Graphics {

// Complete the material and vertex-color conversion that is required after a
// W3D material description has been decoded.  The binding remains generic; the
// caller chooses its texture owner and UV value type.
export template<class TextureOwner, class UVValue>
void Prepare_Mesh_Materials(MeshMaterialBindings<TextureOwner, UVValue>& bindings,
    bool lighting_enabled = true)
{
    struct MaterialValues final
    {
        std::array<float, 3> diffuse{};
        std::array<float, 3> ambient{};
        std::array<float, 3> emissive{};
        float opacity = 1.0f;
    };

    const auto read_values = [](const MeshMaterial* material) {
        MaterialValues values;
        if (material) {
            values.diffuse = material->parameters.diffuse;
            values.ambient = material->parameters.ambient;
            values.emissive = material->parameters.emissive;
            values.opacity = material->parameters.opacity;
        }
        return values;
    };
    const auto has_color = [](const std::array<float, 3>& color) {
        return color[0] != 0.0f || color[1] != 0.0f || color[2] != 0.0f;
    };
    const auto configure_material = [&bindings, lighting_enabled](MeshMaterial* material,
        int pass) {
        if (!material) {
            return;
        }
        material->parameters.diffuse_source = bindings.Get_DCG_Source(pass);
        material->parameters.emissive_source = bindings.Get_DIG_Source(pass);
        material->parameters.lighting = lighting_enabled;
        for (int stage = 0; stage < MeshMaterialBindings<TextureOwner, UVValue>::MAX_TEX_STAGES;
            ++stage) {
            const int source = bindings.Get_UV_Source(pass, stage);
            material->uv_sources[stage] = source < 0 ? 0u : static_cast<std::uint32_t>(source);
        }
    };

    bool set_lighting_to_false = true;
    for (int pass = 0; pass < bindings.Get_Pass_Count(); ++pass) {
        if (!bindings.Peek_Single_Material(pass) && !bindings.Has_Material_Array(pass)) {
            bindings.Set_Single_Material(std::make_shared<MeshMaterial>(), pass);
        }

        if (auto* material = bindings.Peek_Single_Material(pass)) {
            configure_material(material, pass);
        } else {
            MeshMaterial* previous = nullptr;
            for (std::size_t vertex = 0; vertex < bindings.Get_Vertex_Count(); ++vertex) {
                auto* material = bindings.Peek_Material(vertex, pass);
                if (material != previous) {
                    configure_material(material, pass);
                    previous = material;
                }
            }
        }

        if (!bindings.Has_Color_Array(0) && !bindings.Has_Color_Array(1)) {
            continue;
        }

        MaterialValues single_values = read_values(bindings.Peek_Material(0, pass));
        MaterialValues material_values = single_values;
        bool diffuse_used = has_color(single_values.diffuse);
        bool ambient_used = has_color(single_values.ambient);
        bool emissive_used = has_color(single_values.emissive);

        MeshMaterial* previous = nullptr;
        for (std::size_t vertex = 0; vertex < bindings.Get_Vertex_Count(); ++vertex) {
            auto* material = bindings.Peek_Material(vertex, pass);
            if (material != previous) {
                material_values = read_values(material);
                previous = material;
            }
            diffuse_used = diffuse_used || has_color(material_values.diffuse);
            ambient_used = ambient_used || has_color(material_values.ambient);
            emissive_used = emissive_used || has_color(material_values.emissive);
        }
        // The original conversion used the values left by this completed
        // material scan for the ambient and emissive-only cases. Diffuse and
        // opacity continue to follow each vertex material below.
        const MaterialValues scanned_values = material_values;

        // If both DCG and DIG arrays are submitted, multiply them into DCG.
        if (bindings.Get_DCG_Source(pass) != PropColorSource::Material
            && bindings.Has_Color_Array(0)
            && bindings.Get_DIG_Source(pass) != PropColorSource::Material
            && bindings.Has_Color_Array(1)) {
            const auto* diffuse = bindings.Peek_Color_Array(0);
            const auto* emissive = bindings.Peek_Color_Array(1);
            for (std::size_t vertex = 0; vertex < bindings.Get_Vertex_Count(); ++vertex) {
                auto diffuse_value = Assets::Color_From_ARGB(diffuse[vertex]);
                const auto emissive_value = Assets::Color_From_ARGB(emissive[vertex]);
                diffuse_value.r *= emissive_value.r;
                diffuse_value.g *= emissive_value.g;
                diffuse_value.b *= emissive_value.b;
                bindings.Set_Color(0, vertex, Assets::Color_To_ARGB(diffuse_value));
            }
        }
        bindings.Set_DIG_Source(pass, PropColorSource::Material);

        if (bindings.Get_DCG_Source(pass) == PropColorSource::Material
            || !bindings.Has_Color_Array(0)) {
            continue;
        }

        const auto* diffuse = bindings.Peek_Color_Array(0);
        MaterialValues values = read_values(bindings.Peek_Material(0, pass));
        previous = nullptr;
        for (std::size_t vertex = 0; vertex < bindings.Get_Vertex_Count(); ++vertex) {
            auto* material = bindings.Peek_Material(vertex, pass);
            if (material != previous) {
                values = read_values(material);
                previous = material;
            }

            auto multiply_color = [&](const std::array<float, 3>& color) {
                auto diffuse_value = Assets::Color_From_ARGB(diffuse[vertex]);
                diffuse_value.r *= color[0];
                diffuse_value.g *= color[1];
                diffuse_value.b *= color[2];
                diffuse_value.a *= values.opacity;
                bindings.Set_Color(0, vertex, Assets::Color_To_ARGB(diffuse_value));
            };

            // These are the four source combinations supported by the retained
            // SR-lighting conversion.  Other combinations keep their authored
            // material sources and only affect the multi-pass lighting guard.
            if (diffuse_used && !ambient_used && !emissive_used) {
                multiply_color(values.diffuse);
                if (material) {
                    material->parameters.ambient_source = PropColorSource::Material;
                    material->parameters.diffuse_source = PropColorSource::PrimaryColor;
                    material->parameters.emissive_source = PropColorSource::Material;
                }
            }

            if (diffuse_used && ambient_used && !emissive_used) {
                multiply_color(values.diffuse);
                if (material) {
                    material->parameters.ambient_source = PropColorSource::PrimaryColor;
                    material->parameters.diffuse_source = PropColorSource::PrimaryColor;
                    material->parameters.emissive_source = PropColorSource::Material;
                }
            }

            if (!diffuse_used && ambient_used && !emissive_used) {
                multiply_color(scanned_values.ambient);
                if (material) {
                    material->parameters.ambient_source = PropColorSource::PrimaryColor;
                    material->parameters.diffuse_source = PropColorSource::Material;
                    material->parameters.emissive_source = PropColorSource::Material;
                }
            }

            if (!diffuse_used && !ambient_used && emissive_used) {
                multiply_color(scanned_values.emissive);
                if (material) {
                    material->parameters.ambient_source = PropColorSource::Material;
                    material->parameters.diffuse_source = PropColorSource::PrimaryColor;
                    material->parameters.emissive_source = PropColorSource::Material;
                }
            } else if (bindings.Get_Pass_Count() != 1) {
                set_lighting_to_false = false;
            }
        }
    }

    // Disable lighting only when every pass contains emissive color only.
    for (int pass = 0; pass < bindings.Get_Pass_Count(); ++pass) {
        if (!set_lighting_to_false) {
            break;
        }

        MaterialValues values = read_values(bindings.Peek_Material(0, pass));
        bool diffuse_used = has_color(values.diffuse);
        bool ambient_used = has_color(values.ambient);
        bool emissive_used = has_color(values.emissive);
        MeshMaterial* previous = nullptr;
        for (std::size_t vertex = 0; vertex < bindings.Get_Vertex_Count(); ++vertex) {
            auto* material = bindings.Peek_Material(vertex, pass);
            if (material != previous) {
                values = read_values(material);
                previous = material;
            }
            diffuse_used = diffuse_used || has_color(values.diffuse);
            ambient_used = ambient_used || has_color(values.ambient);
            emissive_used = emissive_used || has_color(values.emissive);
        }

        if (bindings.Get_DCG_Source(pass) == PropColorSource::Material
            || !bindings.Has_Color_Array(0)) {
            continue;
        }

        previous = nullptr;
        for (std::size_t vertex = 0; vertex < bindings.Get_Vertex_Count(); ++vertex) {
            auto* material = bindings.Peek_Material(vertex, pass);
            if (material != previous) {
                previous = material;
                if (material && !diffuse_used && !ambient_used && emissive_used) {
                    material->parameters.lighting = false;
                }
            }
        }
    }
}

// Apply the fog state fix-up used by W3D mesh loading.  The two recognized
// two-pass effects need texture, UV, and blend-state adjustments before the
// generic per-pass blend mapping is applied.
export template<class TextureOwner, class UVValue>
void Apply_Mesh_Material_Fog(MeshMaterialBindings<TextureOwner, UVValue>& bindings)
{
    using State = MaterialState;

    if (bindings.Get_Pass_Count() == 2
        && !bindings.Has_Shader_Array(0) && !bindings.Has_Shader_Array(1)) {
        State shader0 = bindings.Get_Single_Shader(0);
        State shader1 = bindings.Get_Single_Shader(1);

        const bool emissive_map_effect =
            shader0.Get_Texturing() == State::TEXTURING_DISABLE
            && shader0.Get_Src_Blend_Func() == State::SRCBLEND_ONE
            && shader0.Get_Dst_Blend_Func() == State::DSTBLEND_ZERO
            && shader0.Get_Primary_Gradient() == State::GRADIENT_MODULATE
            && shader0.Get_Secondary_Gradient() == State::SECONDARY_GRADIENT_DISABLE
            && shader1.Get_Texturing() == State::TEXTURING_ENABLE
            && shader1.Get_Src_Blend_Func() == State::SRCBLEND_SRC_ALPHA
            && shader1.Get_Dst_Blend_Func() == State::DSTBLEND_SRC_COLOR;

        if (emissive_map_effect) {
            shader0.Set_Texturing(State::TEXTURING_ENABLE);
            shader1.Set_Dst_Blend_Func(State::DSTBLEND_ONE);
            shader0.Set_Fog_Func(State::FOG_ENABLE);
            shader1.Set_Fog_Func(State::FOG_SCALE_FRAGMENT);
            bindings.Set_Single_Shader(shader0, 0);
            bindings.Set_Single_Shader(shader1, 1);
            bindings.Set_Single_Texture(bindings.Get_Single_Texture(1, 0), 0, 0);
            bindings.Clone_Texture_Array_If_Absent(0, 0, 1, 0);

            int uv_source = 0;
            if (const auto* materials = bindings.Peek_Material_Array(1)) {
                if (const auto* owner = materials->Peek(0); owner && *owner) {
                    uv_source = static_cast<int>((*owner)->uv_sources[0]);
                }
            }
            if (const auto* materials = bindings.Peek_Material_Array(0)) {
                for (std::size_t vertex = 0; vertex < bindings.Get_Vertex_Count(); ++vertex) {
                    if (const auto* owner = materials->Peek(vertex); owner && *owner) {
                        (*owner)->uv_sources[0] = static_cast<std::uint32_t>(uv_source);
                    }
                }
            } else if (auto* material = bindings.Peek_Single_Material(0)) {
                material->uv_sources[0] = static_cast<std::uint32_t>(uv_source);
            }
            return;
        }

        const bool shiny_mask_effect =
            shader0.Get_Src_Blend_Func() == State::SRCBLEND_ONE
            && shader0.Get_Dst_Blend_Func() == State::DSTBLEND_ZERO
            && shader1.Get_Src_Blend_Func() == State::SRCBLEND_ONE
            && (shader1.Get_Dst_Blend_Func() == State::DSTBLEND_SRC_ALPHA
                || shader1.Get_Dst_Blend_Func() == State::DSTBLEND_ONE_MINUS_SRC_ALPHA);

        if (shiny_mask_effect) {
            shader0.Set_Fog_Func(State::FOG_SCALE_FRAGMENT);
            shader1.Set_Fog_Func(State::FOG_ENABLE);
            bindings.Set_Single_Shader(shader0, 0);
            bindings.Set_Single_Shader(shader1, 1);
            return;
        }
    }

    for (int pass = 0; pass < bindings.Get_Pass_Count(); ++pass) {
        auto shader = bindings.Get_Single_Shader(pass);
        shader.Enable_Fog_For_Blend();
        bindings.Set_Single_Shader(shader, pass);
        if (bindings.Has_Shader_Array(pass)) {
            for (std::size_t polygon = 0; polygon < bindings.Shader_Count(pass); ++polygon) {
                shader = bindings.Get_Shader(polygon, pass);
                shader.Enable_Fog_For_Blend();
                bindings.Set_Shader(polygon, shader, pass);
            }
        }
    }
}

export template<class TextureOwner, class UV>
char Mesh_Material_Sort_Level(const MeshMaterialBindings<TextureOwner, UV>& bindings) {
    const auto pass_flags = [&](int pass) {
        unsigned flags = 0;
        if (bindings.Has_Shader_Array(pass)) {
            for (std::size_t i = 0; i < bindings.Shader_Count(pass); ++i)
                flags |= 1u << Classify_Material_Order(bindings.Get_Shader(i, pass));
        } else flags = 1u << Classify_Material_Order(bindings.Get_Single_Shader(pass));
        return flags;
    };
    if (pass_flags(0) == (1u << MaterialState::SSCAT_OPAQUE)) return 0;
    unsigned flags = 0;
    for (int pass = 0; pass < bindings.Get_Pass_Count(); ++pass) flags |= pass_flags(pass);
    switch (flags) {
    case (1u << MaterialState::SSCAT_OPAQUE) | (1u << MaterialState::SSCAT_ALPHA_TEST): return 0;
    case 1u << MaterialState::SSCAT_ADDITIVE: return 10;
    case 1u << MaterialState::SSCAT_SCREEN: return 15;
    default: return 20;
    }
}
export template<class TextureOwner, class UV>
void Apply_Mesh_Material_Overbright(MeshMaterialBindings<TextureOwner, UV>& bindings) {
    const auto convert = [](MaterialState shader) {
        if (shader.Get_Primary_Gradient() == MaterialState::GRADIENT_MODULATE)
            shader.Set_Primary_Gradient(MaterialState::GRADIENT_MODULATE2X);
        return shader;
    };
    for (int pass = 0; pass < bindings.Get_Pass_Count(); ++pass) {
        bindings.Set_Single_Shader(convert(bindings.Get_Single_Shader(pass)), pass);
        if (bindings.Has_Shader_Array(pass))
            for (std::size_t i = 0; i < bindings.Get_Polygon_Count(); ++i)
                bindings.Set_Shader(i, convert(bindings.Get_Shader(i, pass)), pass);
    }
}
}
