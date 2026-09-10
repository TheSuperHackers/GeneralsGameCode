module;
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <utility>
#include <variant>
#include <vector>
export module Graphics.Scene.Models.W3DMaterialLoading;
import Assets.Math;
import Assets.Identity;
import Assets.Adapters.W3D.Materials;
import Assets.Adapters.W3D.MeshData;
import Assets.Adapters.W3D.PassBindings;
import Graphics.Materials.State;
import Graphics.Materials.Ordering;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.W3DState;
import Graphics.Materials.W3DMeshMaterial;
import Graphics.Scene.Models.Materials;
import Graphics.Scene.Models.MeshMaterialBindings;
import Graphics.Scene.Models.MeshMaterialPreparation;
import Graphics.Scene.Props.Material;

namespace Graphics {
export struct W3DMeshMaterialLoadOptions final {
    std::uint32_t time = 0;
    float (*random_sample)() = nullptr;
    float (*bump_sine)(float) = nullptr;
    float (*bump_cosine)(float) = nullptr;
    bool fog = false;
    bool assign_sort_level = false;
    bool overbright = false;
};
export struct W3DMeshMaterialLoadResult final {
    bool success = false;
    bool sorted = false;
    char sort_level = 0;
    bool animated_material3_texture = false;
};

// One loading session owns the ordered resource tables and alternate bindings.
// Resource acquisition remains with the caller's asset ownership layer.
export template<class TextureOwner, class UV, class ResolveTexture, class TextureName>
class W3DMeshMaterialLoading final {
    using Bindings = MeshMaterialBindings<TextureOwner, UV>;
    struct LegacySlots final {
        std::size_t shader = 0;
        std::size_t material = 0;
        TextureOwner texture;
    };
public:
    W3DMeshMaterialLoading(const Assets::W3D::W3DMeshData& mesh, Bindings& defaults,
        const W3DMeshMaterialLoadOptions& options, ResolveTexture resolve, TextureName name)
        : m_mesh(mesh), m_defaults(defaults), m_options(options),
          m_resolve(std::move(resolve)), m_name(std::move(name))
    {
        m_alternate.Set_Vertex_Count(mesh.header.vertex_count);
        m_alternate.Set_Polygon_Count(mesh.header.triangle_count);
        m_result.sort_level = static_cast<char>(mesh.header.sort_level);
    }

    bool Read() {
        for (const auto& record : m_mesh.materials)
            if (!std::visit([&](const auto& value) { return Install(value); }, record)) return false;
        return true;
    }

    W3DMeshMaterialLoadResult Finish(std::unique_ptr<Bindings>& alternate,
        ModelMaterials<TextureOwner>& materials)
    {
        if (!m_alternate.Is_Empty()) {
            alternate = std::make_unique<Bindings>();
            alternate->Init_Alternate(m_defaults, m_alternate, m_options.time);
        }
        Prepare_Mesh_Materials(m_defaults, m_mesh.prelit_chunk != 0x24);
        if (alternate) Prepare_Mesh_Materials(*alternate, m_mesh.prelit_chunk != 0x24);
        materials.textures.insert(materials.textures.end(), m_materials.textures.begin(), m_materials.textures.end());
        materials.materials.insert(materials.materials.end(), m_materials.materials.begin(), m_materials.materials.end());
        if (m_mesh.header.attributes & 0x2000u) {
            m_defaults.Set_Two_Sided();
            if (alternate) alternate->Set_Two_Sided();
        }
        if (m_options.fog) Apply_Mesh_Material_Fog(m_defaults);
        if (m_result.sorted && m_result.sort_level == 0 && m_options.assign_sort_level)
            m_result.sort_level = Mesh_Material_Sort_Level(m_defaults);
        if (m_options.overbright) Apply_Mesh_Material_Overbright(m_defaults);
        m_result.success = true;
        return m_result;
    }

private:
    bool Install(const Assets::W3D::W3DMaterialInfo& info) {
        if (info.pass_count == 0 || info.pass_count > Bindings::MAX_PASSES) return false;
        m_defaults.Set_Pass_Count(static_cast<int>(info.pass_count));
        return true;
    }
    bool Install(const std::vector<Assets::W3D::W3DShaderSettings>& records) {
        for (const auto& record : records) {
            MaterialState shader;
            Apply_W3D_Material_State(shader, record);
            m_shaders.push_back(shader);
        }
        return true;
    }
    bool Install(const std::vector<Assets::W3D::W3DVertexMaterialData>& records) {
        for (const auto& record : records) {
            auto material = std::make_shared<MeshMaterial>();
            Apply_W3D_Mesh_Material(*material, record, m_options.time,
                m_options.random_sample, m_options.bump_sine, m_options.bump_cosine);
            m_materials.materials.push_back(std::move(material));
        }
        return true;
    }
    bool Install(const std::vector<Assets::W3D::W3DTextureData>& records) {
        for (const auto& record : records) {
            auto texture = m_resolve(record);
            if (!texture) return false;
            m_materials.textures.push_back(std::move(texture));
        }
        return true;
    }
    bool Install(const std::vector<Assets::Vector2f>& records) {
        if (m_pass >= Bindings::MAX_PASSES || m_stage >= Bindings::MAX_TEX_STAGES) return false;
        m_uv.resize(records.size());
        for (std::size_t i = 0; i < records.size(); ++i) m_uv[i] = UV{records[i].x, 1.0f - records[i].y};
        m_defaults.Install_UV_Array(m_pass, m_stage, std::span<const UV>(m_uv));
        return true;
    }
    bool Install(const std::vector<Assets::Color4f>& records) {
        if (m_pass >= Bindings::MAX_PASSES) return false;
        if (!m_defaults.Has_Color_Array(0)) {
            m_defaults.Allocate_Color_Array(0);
            for (std::size_t i = 0; i < records.size(); ++i) m_defaults.Set_Color(0, i, Assets::Color_To_ARGB(records[i]));
        }
        m_defaults.Set_DCG_Source(m_pass, PropColorSource::PrimaryColor);
        return true;
    }
    bool Install(const std::vector<Assets::W3D::W3DMaterial3Data>& records) {
        for (const auto& record : records) {
            auto material = std::make_shared<MeshMaterial>();
            const auto runtime = Apply_W3D_Material3(*material, record);
            m_result.sorted |= runtime.requires_sort;
            m_result.animated_material3_texture |= runtime.has_animated_texture;
            auto shader = std::find(m_shaders.begin(), m_shaders.end(), runtime.shader);
            const auto shader_index = static_cast<std::size_t>(shader - m_shaders.begin());
            if (shader == m_shaders.end()) m_shaders.push_back(runtime.shader);
            const auto key = material->Content_Key();
            std::size_t material_index = 0;
            for (; material_index < m_materials.materials.size(); ++material_index)
                if (m_materials.materials[material_index]->Content_Key() == key) break;
            if (material_index == m_materials.materials.size()) m_materials.materials.push_back(material);
            TextureOwner texture;
            if (runtime.texture_map) {
                Assets::W3D::W3DTextureData source;
                source.name = runtime.texture_map->filename;
                texture = m_resolve(source);
                if (!texture) return false;
                auto found = std::find_if(m_materials.textures.begin(), m_materials.textures.end(),
                    [&](const auto& value) { return std::to_address(value) == std::to_address(texture)
                        || Assets::Asset_Name_Equals_No_Case(m_name(value), m_name(texture)); });
                if (found == m_materials.textures.end()) m_materials.textures.push_back(texture);
                else texture = *found;
            }
            m_legacy.push_back({shader_index, material_index, std::move(texture)});
        }
        if (!m_materials.materials.empty()) m_defaults.Set_Single_Material(m_materials.materials[0]);
        if (!m_materials.textures.empty()) m_defaults.Set_Single_Texture(m_materials.textures[0]);
        if (!m_shaders.empty()) m_defaults.Set_Single_Shader(m_shaders[0]);
        return true;
    }
    bool Install(const std::vector<std::uint16_t>& records) {
        if (m_legacy.empty()) return false;
        for (const auto id : records) if (id >= m_legacy.size()) return false;
        const bool multiple_materials = m_materials.materials.size() > 1;
        const bool multiple_textures = m_materials.textures.size() > 1;
        const bool multiple_shaders = m_shaders.size() > 1;
        if (!multiple_materials) m_defaults.Set_Single_Material(m_materials.materials[m_legacy[0].material]);
        if (!multiple_textures) m_defaults.Set_Single_Texture(m_legacy[0].texture);
        if (!multiple_shaders) m_defaults.Set_Single_Shader(m_shaders[m_legacy[0].shader]);
        for (std::size_t face = 0; face < records.size(); ++face) {
            const auto& slots = m_legacy[records[face]];
            if (multiple_shaders) m_defaults.Set_Shader(face, m_shaders[slots.shader]);
            if (multiple_textures) m_defaults.Set_Texture(face, slots.texture);
            if (multiple_materials)
                for (const auto vertex : m_mesh.triangles[face].indices)
                    m_defaults.Set_Material(vertex, m_materials.materials[slots.material]);
        }
        return true;
    }
    bool Install(const Assets::W3D::W3DPassBindings& bindings) {
        const int pass = m_pass;
        if (pass < 0 || pass >= Bindings::MAX_PASSES
            || bindings.stages.size() > Bindings::MAX_TEX_STAGES)
            return false;
        // Validate references before changing either material set.
        for (const auto id : bindings.vertex_material_ids)
            if (id >= static_cast<unsigned>(m_materials.materials.size())) return false;
        for (const auto id : bindings.shader_ids)
            if (id >= static_cast<unsigned>(m_shaders.size())) return false;
        for (const auto &stage : bindings.stages)
            for (const auto id : stage.texture_ids)
                if (id != 0xffffffffu && id >= static_cast<unsigned>(m_materials.textures.size()))
                    return false;
    
        if (!bindings.vertex_material_ids.empty()) {
            auto *materials = m_defaults.Has_Material_Data(pass)
                ? &m_alternate : &m_defaults;
            const auto &ids = bindings.vertex_material_ids;
            if (ids.size() == 1) materials->Set_Single_Material(m_materials.materials[ids[0]], pass);
            else for (std::size_t vertex = 0; vertex < ids.size(); ++vertex)
                materials->Set_Material(vertex, m_materials.materials[ids[vertex]], pass);
        }
        if (!bindings.shader_ids.empty()) {
            auto *materials = m_defaults.Has_Shader_Data(pass)
                ? &m_alternate : &m_defaults;
            const auto &ids = bindings.shader_ids;
            for (std::size_t face = 0; face < ids.size(); ++face) {
                const auto shader = m_shaders[ids[face]];
                if (ids.size() == 1) materials->Set_Single_Shader(shader, pass);
                else materials->Set_Shader(face, shader, pass);
                if (pass == 0 && shader.Get_Dst_Blend_Func() != Graphics::MaterialState::DSTBLEND_ZERO
                    && shader.Get_Alpha_Test() == Graphics::MaterialState::ALPHATEST_DISABLE && m_result.sort_level == 0)
                    m_result.sorted = true;
            }
        }
    
        // Prelit illumination and authored alpha may share the same color array.
        // Applying their decoded arrays in file order preserves alternate sets and
        // the existing multiplication/alpha replacement semantics.
        for (const auto source : bindings.color_order) {
            using Assets::W3D::W3DPassColorSource;
            if (source == W3DPassColorSource::Diffuse) {
                auto *materials = m_defaults.Get_DCG_Source(pass) != Graphics::PropColorSource::Material
                    ? &m_alternate : &m_defaults;
                const bool replace_rgb = !materials->Has_Color_Array(0);
                if (replace_rgb || m_mesh.prelit_chunk == 0x24) {
                    materials->Allocate_Color_Array(0);
                    const auto* colors = materials->Peek_Color_Array(0);
                    for (std::size_t vertex = 0; vertex < bindings.diffuse_colors.size(); ++vertex) {
                        const auto &input = bindings.diffuse_colors[vertex];
                        auto value = replace_rgb ? Assets::Color4f{input.r, input.g, input.b, input.a}
                            : Assets::Color_From_ARGB(colors[vertex]);
                        value.a = input.a;
                        materials->Set_Color(0, vertex, Assets::Color_To_ARGB(value));
                    }
                }
                materials->Set_DCG_Source(pass, Graphics::PropColorSource::PrimaryColor);
            } else if (source == W3DPassColorSource::Illumination) {
                auto *materials = m_loaded_illumination
                    ? &m_alternate : &m_defaults;
                m_loaded_illumination = true;
                const bool multiply = materials->Has_Color_Array(0);
                materials->Allocate_Color_Array(0);
                const auto* colors = materials->Peek_Color_Array(0);
                for (std::size_t vertex = 0; vertex < bindings.diffuse_illumination.size(); ++vertex) {
                    const auto &input = bindings.diffuse_illumination[vertex];
                    auto value = multiply ? Assets::Color_From_ARGB(colors[vertex]) : Assets::Color4f{};
                    value.r *= input.r;
                    value.g *= input.g;
                    value.b *= input.b;
                    materials->Set_Color(0, vertex, Assets::Color_To_ARGB(value));
                }
                materials->Set_DCG_Source(pass, Graphics::PropColorSource::PrimaryColor);
            }
            // Specular arrays remain in the decoded asset. The existing material
            // description has no per-pass specular array binding.
        }
    
        for (std::size_t stage_index = 0; stage_index < bindings.stages.size(); ++stage_index) {
            const int stage = static_cast<int>(stage_index);
            const auto &input = bindings.stages[stage_index];
            if (!input.texture_ids.empty()) {
                auto *materials = m_defaults.Has_Texture_Data(pass,stage)
                    ? &m_alternate : &m_defaults;
                if (input.texture_ids.size() == 1) {
                    const auto id = input.texture_ids[0];
                    const auto texture = id == 0xffffffffu ? TextureOwner{} : m_materials.textures[id];
                    materials->Set_Single_Texture(texture, pass, stage);
                } else for (std::size_t face = 0; face < input.texture_ids.size(); ++face) {
                    const auto id = input.texture_ids[face];
                    if (id != 0xffffffffu) {
                        const auto& texture = m_materials.textures[id];
                        materials->Set_Texture(face, texture, pass, stage);
                    }
                }
            }
            if (!input.texcoords.empty()) {
                auto *materials = m_defaults.Has_UV(pass,stage)
                    ? &m_alternate : &m_defaults;
                const int count = static_cast<int>(input.texcoords.size());
                m_uv.resize(count);
                auto* uvs = m_uv.data();
                for (int vertex = 0; vertex < count; ++vertex)
                    uvs[vertex] = UV{input.texcoords[vertex].x, 1.0f-input.texcoords[vertex].y};
                materials->Install_UV_Array(pass,stage,
                    std::span<const UV>(uvs,static_cast<std::size_t>(count)));
            }
            // Indexed corner UVs are retained and validated by the decoder. The
            // current mesh description consumes vertex-indexed UV arrays only.
        }
        m_stage = static_cast<int>(bindings.stages.size());
        ++m_pass;
        return true;
    }
    const Assets::W3D::W3DMeshData& m_mesh;
    Bindings& m_defaults;
    W3DMeshMaterialLoadOptions m_options;
    ResolveTexture m_resolve;
    TextureName m_name;
    Bindings m_alternate;
    ModelMaterials<TextureOwner> m_materials;
    std::vector<MaterialState> m_shaders;
    std::vector<LegacySlots> m_legacy;
    std::vector<UV> m_uv;
    int m_pass = 0;
    int m_stage = 0;
    bool m_loaded_illumination = false;
    W3DMeshMaterialLoadResult m_result;
};
}
