module;
#include "../../profiling/Tracy.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <numeric>
#include <span>
#include <utility>
#include <vector>
export module Graphics.Scene.Models.MeshDrawing;
import Graphics.Materials.State;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.MeshTextureMapping;
import Graphics.Scene.Models.MeshMaterialBindings;
import Graphics.Scene.Models.SourceRevision;
import Graphics.Scene.MuzzleFlash;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Material;
import Graphics.Scene.Props.Extraction;
import Graphics.Scene.Props.MeshSet;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Props.MaterialSubmission;

namespace Graphics {
export struct ModelMeshDrawOverrides final {
    float opacity = 1;
    float pass_opacity = 1;
    float pass_emissive = 1;
    bool shadow_capture = false;
};

export struct ModelMeshDrawContext final {
    PropParameters parameters;
    std::array<float, 16> projection{};
    std::uint32_t milliseconds = 0;
    bool sorted = false;
    bool additional_only = false;
    bool shadow = false;
    bool two_sided = false;
    bool additive = false;
    bool translucent = false;
    MuzzleFlashDesignation muzzle_flash = MuzzleFlashDesignation::None;
    std::optional<std::array<float, 2>> uv_offset;
    ModelMeshDrawOverrides overrides;
    std::span<const std::uint16_t> bone_links;
};

// Bind-pose geometry and material packets belong to the shared mesh source.
// Instance records supply transforms, lighting and animation palettes at draw time.
export template<class TextureOwner>
struct ModelMaterialGroups final {
    struct Key final {
        std::array<std::uint64_t, 4> versions{};
        std::array<const void*, 3> single_resources{};
        const void* triangles = nullptr;
        std::size_t triangle_count = 0;
        std::uint64_t topology_revision = 0;
        std::uint32_t single_shader = 0;
        bool operator==(const Key& other) const noexcept
        {
            return std::equal(versions.begin(), versions.end(), other.versions.begin())
                && std::equal(single_resources.begin(), single_resources.end(), other.single_resources.begin())
                && triangles == other.triangles
                && triangle_count == other.triangle_count
                && topology_revision == other.topology_revision
                && single_shader == other.single_shader;
        }
    };
    Key key;
    struct Packet final {
        std::size_t end = 0;
        MaterialState shader{0};
        std::shared_ptr<MeshMaterial> material;
        std::array<TextureOwner, 2> textures;
        PropMeshHandle mesh{};
        std::optional<PropMaterial> geometry_material;
        PropMaterialPreparation preparation;
    };
    struct GeometryKey final {
        std::array<const void*, 6> sources{};
        std::array<std::uint64_t, 5> revisions{};
        std::size_t vertices = 0;
        std::size_t normals = 0;
        float opacity = 1;
        bool sorted = false;
        bool lighting = true;
        bool additive = false;
        std::uint64_t bone_revision = 0;
        bool operator==(const GeometryKey& other) const noexcept
        {
            return std::equal(sources.begin(), sources.end(), other.sources.begin())
                && std::equal(revisions.begin(), revisions.end(), other.revisions.begin())
                && vertices == other.vertices
                && normals == other.normals
                && opacity == other.opacity
                && sorted == other.sorted
                && lighting == other.lighting
                && additive == other.additive
                && bone_revision == other.bone_revision;
        }
    };
    GeometryKey geometry_key;
    std::vector<Packet> packets;
    bool valid = false;
    bool geometry_valid = false;
};

export template<class TextureOwner>
struct ModelMeshState final {
    PropMeshSet base;
    PropMeshSet additional;
    std::array<ModelMaterialGroups<TextureOwner>, 4> groups;
    std::vector<PropMaterialPreparation> additional_materials;
    std::span<const std::uint32_t> Complete_Polygons(std::size_t count) {
        assert(count <= (std::numeric_limits<std::uint32_t>::max)());
        if (m_polygons.size() != count) {
            m_polygons.resize(count);
            std::iota(m_polygons.begin(),m_polygons.end(),std::uint32_t{0});
            ++m_polygon_revision;
        }
        return m_polygons;
    }
    std::uint64_t Complete_Polygon_Revision() const noexcept { return m_polygon_revision; }
    std::uint64_t Update_Bone_Links(std::span<const std::uint16_t> bones) {
        // Bone-link pointers can escape the model's geometry revision domain.
        if ((!m_bones && !bones.empty()) || (m_bones && (bones.size()!=m_bones->size() || (!bones.empty()
            && std::memcmp(bones.data(),m_bones->data(),bones.size_bytes())!=0)))) {
            m_bones=std::make_shared<const std::vector<std::uint16_t>>(bones.begin(),bones.end());
            m_bone_revision.Invalidate();
        }
        return m_bone_revision.Token();
    }
    std::shared_ptr<const std::vector<std::uint16_t>> Bone_Links() const noexcept { return m_bones; }
private:
    std::vector<std::uint32_t> m_polygons;
    std::shared_ptr<const std::vector<std::uint16_t>> m_bones;
    SourceRevision m_bone_revision;
    std::uint64_t m_polygon_revision = 0;
};

namespace MeshDrawingDetail {
std::array<float, 4> Unpack_Color(unsigned color) {
    return {((color >> 16) & 255) / 255.0f, ((color >> 8) & 255) / 255.0f,
        (color & 255) / 255.0f, ((color >> 24) & 255) / 255.0f};
}
}

// The session owns its extraction lease across base and procedural passes.
// Borrowed geometry is converted lazily for tracked sources, and snapshotted
// immediately for writable/deformed sources before any submission callback.
// Deformed vertices may supply a separate source-topology revision so pose
// changes do not rebuild unchanged material ranges and resource ownership.
export template<class Position, class Triangle, class TextureOwner, class UV>
class ModelMeshDrawing final {
public:
    ModelMeshDrawing(std::span<const Position> positions, std::span<const Position> normals,
        std::span<const Triangle> triangles, std::uint64_t revision,
        MeshMaterialBindings<TextureOwner, UV>& materials, ModelMeshState<TextureOwner>& state,
        PropRenderer& renderer, PropExtractionCache& cache, ModelMeshDrawContext context,
        std::optional<std::uint64_t> topology_revision = std::nullopt)
        : m_positions(positions), m_normals(normals), m_triangles(triangles), m_revision(revision),
          m_topology_revision(topology_revision.value_or(revision)),
          m_materials(materials), m_state(state), m_renderer(renderer), m_context(std::move(context)),
          m_cache(cache)
    {
        assert(m_context.bone_links.empty() || m_context.bone_links.size()==positions.size());
        m_bone_revision=m_state.Update_Bone_Links(m_context.bone_links);
        m_bones=m_state.Bone_Links();
        if (m_bones) m_context.bone_links=*m_bones;
        if (m_revision == 0) Prepare_Source();
    }

    template<class Submit>
    bool Draw_Base(Submit&& submit) {
        const auto& overrides = m_context.overrides;
        const auto first_shader = m_materials.Get_Single_Shader();
        const bool render_base = !m_context.additional_only || (m_context.shadow &&
            (first_shader.Get_Alpha_Test() == MaterialState::ALPHATEST_ENABLE ||
             first_shader.Get_Src_Blend_Func() == MaterialState::SRCBLEND_SRC_ALPHA));
        std::size_t batch_index = 0;
        bool success = true;
        for (int pass = 0; render_base && pass < m_materials.Get_Pass_Count() &&
            (!overrides.shadow_capture || pass == 0); ++pass) {
            const auto* primary = m_materials.Peek_DCG_Array(pass);
            const auto* secondary = m_materials.Peek_DIG_Array(pass);
            const auto* uv = m_materials.Peek_UV_Array(pass, 0);
            const auto* secondary_uv = m_materials.Peek_UV_Array(pass, 1);
            const bool uniform = !m_materials.Has_Shader_Array(pass) && !m_materials.Has_Material_Array(pass)
                && !m_materials.Has_Texture_Array(pass, 0) && !m_materials.Has_Texture_Array(pass, 1);
            auto& groups = m_state.groups[pass];
            bool cache_groups = false;
            bool reuse_groups = false;
            if (m_topology_revision != 0) {
                if (const auto versions = m_materials.Grouping_Revisions(pass)) {
                    const typename ModelMaterialGroups<TextureOwner>::Key key{*versions,
                        {m_materials.Peek_Single_Material(pass), m_materials.Peek_Single_Texture(pass, 0),
                            m_materials.Peek_Single_Texture(pass, 1)},
                        m_triangles.data(), m_triangles.size(), m_topology_revision, m_materials.Get_Single_Shader(pass).Get_Bits()};
                    cache_groups = true;
                    reuse_groups = groups.valid && groups.key == key;
                    if (!reuse_groups) groups.key = key;
                }
            }
            if (!reuse_groups) {
                groups.valid = false;
            }
            const typename ModelMaterialGroups<TextureOwner>::GeometryKey geometry_key{
                {m_positions.data(), m_normals.data(), primary, secondary, uv, secondary_uv},
                {m_revision, m_materials.DCG_Revision(pass), m_materials.DIG_Revision(pass),
                    m_materials.UV_Revision(pass, 0), m_materials.UV_Revision(pass, 1)},
                m_positions.size(), m_normals.size(), overrides.opacity, m_context.sorted,
                Get_Prop_Draw_Settings().lighting, m_context.additive, m_bone_revision};
            const bool tracked_geometry = m_revision != 0
                && (!primary || geometry_key.revisions[1] != 0)
                && (!secondary || geometry_key.revisions[2] != 0)
                && (!uv || geometry_key.revisions[3] != 0)
                && (!secondary_uv || geometry_key.revisions[4] != 0);
            const bool reuse_geometry = reuse_groups && tracked_geometry && groups.geometry_valid
                && groups.geometry_key == geometry_key;
            groups.geometry_valid = false;
            std::size_t group_index = 0;
            for (std::size_t first = 0; first < m_triangles.size();) {
                const auto slot = batch_index++;
                if (!reuse_groups) {
                    if (group_index == groups.packets.size()) groups.packets.emplace_back();
                    auto& packet = groups.packets[group_index];
                    packet.shader = m_materials.Get_Shader(first, pass);
                    const auto vertex = m_triangles[first][0];
                    if (packet.material.get() != m_materials.Peek_Material(vertex, pass)) {
                        const auto* slots = m_materials.Peek_Material_Array(pass);
                        const auto* owner = slots ? slots->Peek(vertex) : nullptr;
                        packet.material = owner ? *owner : m_materials.Get_Single_Material(pass);
                    }
                    for (unsigned stage = 0; stage < 2; ++stage) {
                        if (std::to_address(packet.textures[stage]) != m_materials.Peek_Texture(first, pass, stage)) {
                            const auto* slots = m_materials.Peek_Texture_Array(pass, stage);
                            const auto* owner = slots ? slots->Peek(first) : nullptr;
                            packet.textures[stage] = owner ? *owner : m_materials.Get_Single_Texture(pass, stage);
                        }
                    }
                    packet.end = uniform ? m_triangles.size() : first + 1;
                    while (packet.end < m_triangles.size() && m_materials.Get_Shader(packet.end, pass) == packet.shader
                        && m_materials.Peek_Material(m_triangles[packet.end][0], pass) == packet.material.get()
                        && m_materials.Peek_Texture(packet.end, pass, 0) == std::to_address(packet.textures[0])
                        && m_materials.Peek_Texture(packet.end, pass, 1) == std::to_address(packet.textures[1])) ++packet.end;
                }
                auto& packet = groups.packets[group_index++];
                if (!reuse_geometry) packet.mesh = {};
                const auto end = packet.end;
                auto shader = packet.shader;
                auto* material = packet.material.get();
                const std::array textures{std::to_address(packet.textures[0]), std::to_address(packet.textures[1])};
                PropMaterialDrawOverrides draw;
                draw.preparation = &packet.preparation;
                draw.force_multiply = Get_Prop_Draw_Settings().force_multiply &&
                    shader.Get_Dst_Blend_Func() == MaterialState::DSTBLEND_ZERO && overrides.opacity == 1;
                if (!m_context.sorted && overrides.opacity != 1) {
                    if (!m_context.additive) {
                        shader.Set_Src_Blend_Func(MaterialState::SRCBLEND_SRC_ALPHA);
                        shader.Set_Dst_Blend_Func(MaterialState::DSTBLEND_ONE_MINUS_SRC_ALPHA);
                    }
                    draw.alpha_cutoff = static_cast<unsigned>(96 * overrides.opacity) / 255.0f;
                }
                if (m_context.two_sided) shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
                const auto* vertex_material = material ? &material->parameters : nullptr;
                const std::array preparation_state{overrides.opacity, m_context.sorted ? 1.0f : 0.0f,
                    Get_Prop_Draw_Settings().lighting ? 1.0f : 0.0f, m_context.additive ? 1.0f : 0.0f};
                const auto bytes = [&](const auto* values) {
                    return std::as_bytes(std::span(values, values ? m_positions.size() : 0));
                };
                const bool same_material = vertex_material
                    ? packet.geometry_material && std::memcmp(&*packet.geometry_material, vertex_material, sizeof(PropMaterial)) == 0
                    : !packet.geometry_material;
                const auto* geometry = reuse_geometry && same_material ? m_renderer.Mesh_Geometry(packet.mesh) : nullptr;
                auto mesh = geometry ? packet.mesh : PropMeshHandle{};
                std::optional<std::array<PropPreparationInput, 10>> inputs;
                if (!mesh.Is_Valid()) {
                    inputs.emplace(std::array<PropPreparationInput, 10>{{
                        {m_revision != 0 ? std::as_bytes(m_positions) : std::as_bytes(m_source), m_revision},
                        {m_revision != 0 ? std::as_bytes(m_normals) : std::span<const std::byte>{}, m_revision},
                        {std::as_bytes(m_triangles.subspan(first, end - first)), m_topology_revision},
                        {bytes(primary), geometry_key.revisions[1]}, {bytes(secondary), geometry_key.revisions[2]},
                        {bytes(uv), geometry_key.revisions[3]}, {bytes(secondary_uv), geometry_key.revisions[4]},
                        {std::as_bytes(std::span(vertex_material, vertex_material ? 1u : 0u))},
                        {std::as_bytes(std::span(preparation_state))},
                        {std::as_bytes(m_context.bone_links),m_bone_revision}}});
                    mesh = m_state.base.Find_Versioned(m_renderer, slot, *inputs);
                }
                const auto extract = [&](unsigned index) {
                    auto vertex = m_source[index].Make_Vertex();
                if (!m_context.bone_links.empty()) vertex.bone_index=m_context.bone_links[index];
                    if (primary) vertex.color = MeshDrawingDetail::Unpack_Color(primary[index]);
                    if (secondary) vertex.secondary_color = MeshDrawingDetail::Unpack_Color(secondary[index]);
                    if (uv) vertex.uv = {uv[index][0], uv[index][1]};
                    if (secondary_uv) vertex.secondary_uv = {secondary_uv[index][0], secondary_uv[index][1]};
                    if (vertex_material) Apply_Prop_Material(vertex, *vertex_material);
                    if (!Get_Prop_Draw_Settings().lighting) vertex.material_ambient[3] = 0;
                    if (!m_context.sorted && overrides.opacity != 1 && material &&
                        (!material->parameters.lighting || material->parameters.diffuse_source == PropColorSource::Material)) {
                        vertex.material_diffuse[3] = overrides.opacity;
                        if (m_context.additive)
                            vertex.material_diffuse[0] = vertex.material_diffuse[1] = vertex.material_diffuse[2] = overrides.opacity;
                    }
                    return vertex;
                };
                if (!mesh.Is_Valid()) {
                    GRAPHICS_PROFILE_SCOPE("Graphics.Mesh.BuildMaterialBatch");
                    Prepare_Source();
                    auto& batch = m_workspace->Workspace().Batch();
                    if (!batch.Begin(m_source.size(), (end - first) * 3)) return false;
                    for (std::size_t polygon = first; polygon < end; ++polygon)
                        for (unsigned corner = 0; corner < 3; ++corner)
                            batch.Append_Validated(m_triangles[polygon][corner], extract);
                }
                auto parameters = m_context.parameters;
                Extract_Mesh_Texture_Mappings(parameters, material, m_context.milliseconds,
                    parameters.view, m_context.projection, m_context.sorted ? std::nullopt : m_context.uv_offset);
                draw.shadow_capture = overrides.shadow_capture;
                draw.muzzle_flash = m_context.muzzle_flash;
                if (draw.muzzle_flash != MuzzleFlashDesignation::None) parameters.opacity = overrides.opacity;
                if (overrides.shadow_capture) {
                    if (shader.Get_Dst_Blend_Func() == MaterialState::DSTBLEND_ONE ||
                        shader.Get_Src_Blend_Func() == MaterialState::SRCBLEND_ZERO) {
                        first = end;
                        continue;
                    }
                    if (shader.Get_Src_Blend_Func() == MaterialState::SRCBLEND_SRC_ALPHA) {
                        shader.Set_Alpha_Test(MaterialState::ALPHATEST_ENABLE);
                        draw.alpha_cutoff = 96.0f / 255.0f;
                    }
                }
                if (!mesh.Is_Valid()) {
                    auto& batch = m_workspace->Workspace().Batch();
                    mesh = m_state.base.Publish_Versioned(m_renderer, slot, *inputs, batch.Vertices(), batch.Indices());
                }
                if (!mesh.Is_Valid()) return false;
                packet.mesh = mesh;
                if (!same_material)
                    packet.geometry_material = vertex_material ? std::optional(*vertex_material) : std::nullopt;
                draw.mesh = mesh;
                if (!geometry) geometry = m_renderer.Mesh_Geometry(mesh);
                if (!submit(geometry->Vertices(), geometry->Indices(), shader, textures, parameters, draw)) success = false;
                first = end;
            }
            if (cache_groups) {
                groups.valid = true;
            }
            if (!reuse_groups) groups.packets.resize(group_index);
            if (!reuse_geometry) groups.geometry_key = geometry_key;
            groups.geometry_valid = tracked_geometry;
        }
        return success;
    }

    bool Accepts_Additional_Pass(bool enabled_on_translucent) const noexcept {
        return !m_context.overrides.shadow_capture && (!m_context.translucent || enabled_on_translucent);
    }

    template<class Description, class Index, class Submit>
    bool Draw_Additional(Description description, std::span<const Index> polygons,
        std::size_t slot, Submit&& submit, std::uint64_t polygon_revision = 0) {
        if (polygons.empty()) return true;
        const auto* vertex_material = description.material ? &description.material->parameters : nullptr;
        const auto* primary = m_materials.Peek_DCG_Array(0);
        const auto* secondary = m_materials.Peek_DIG_Array(0);
        const auto* uv = m_materials.Peek_UV_Array(0, 0);
        const auto* secondary_uv = m_materials.Peek_UV_Array(0, 1);
        const auto bytes = [&](const auto* values) {
            return std::as_bytes(std::span(values, values ? m_positions.size() : 0));
        };
        const std::array preparation_state{m_context.overrides.pass_opacity, m_context.overrides.pass_emissive};
        const std::array<PropPreparationInput,11> inputs{{
            {m_revision != 0 ? std::as_bytes(m_positions) : std::as_bytes(m_source), m_revision},
            {m_revision != 0 ? std::as_bytes(m_normals) : std::span<const std::byte>{}, m_revision},
            {std::as_bytes(m_triangles), m_topology_revision},
            {std::as_bytes(polygons), polygon_revision},
            {bytes(primary), m_materials.DCG_Revision(0)}, {bytes(secondary), m_materials.DIG_Revision(0)},
            {bytes(uv), m_materials.UV_Revision(0,0)}, {bytes(secondary_uv), m_materials.UV_Revision(0,1)},
            {std::as_bytes(std::span(vertex_material, vertex_material ? 1u : 0u))},
            {std::as_bytes(std::span(preparation_state))},
            {std::as_bytes(m_context.bone_links),m_bone_revision}}};
        auto mesh = m_state.additional.Find_Versioned(m_renderer,slot,inputs);
        if (!mesh.Is_Valid()) {
            Prepare_Source();
            auto& batch = m_workspace->Workspace().Batch();
            if (!batch.Begin(m_source.size(), polygons.size() * 3)) return false;
            const bool override_opacity = vertex_material && m_context.overrides.pass_opacity != 1
                && (!vertex_material->lighting || vertex_material->diffuse_source == PropColorSource::Material);
            const bool scale_emissive = vertex_material && vertex_material->emissive_source == PropColorSource::Material;
            const auto extract = [&](unsigned index) {
                auto vertex = m_source[index].Make_Vertex();
                if (!m_context.bone_links.empty()) vertex.bone_index=m_context.bone_links[index];
                if (primary) vertex.color = MeshDrawingDetail::Unpack_Color(primary[index]);
                if (secondary) vertex.secondary_color = MeshDrawingDetail::Unpack_Color(secondary[index]);
                if (uv) vertex.uv = {uv[index][0], uv[index][1]};
                if (secondary_uv) vertex.secondary_uv = {secondary_uv[index][0], secondary_uv[index][1]};
                if (vertex_material) Apply_Prop_Material(vertex, *vertex_material);
                if (override_opacity) vertex.material_diffuse[3] = m_context.overrides.pass_opacity;
                if (scale_emissive)
                    for (unsigned c = 0; c < 3; ++c) vertex.material_emissive[c] *= m_context.overrides.pass_emissive;
                return vertex;
            };
            for (const auto polygon : polygons) {
                assert(static_cast<std::size_t>(polygon) < m_triangles.size());
                for (unsigned corner = 0; corner < 3; ++corner)
                    batch.Append_Validated(m_triangles[polygon][corner], extract);
            }
            mesh = m_state.additional.Publish_Versioned(m_renderer,slot,inputs,batch.Vertices(),batch.Indices());
            if (!mesh.Is_Valid()) return false;
        }
        auto parameters = m_context.parameters;
        if (description.world_coordinates) {
            parameters.uv_sources[0] = 4;
            parameters.uv_transform[0] = description.world_texture_transform;
        } else Extract_Mesh_Texture_Mappings(parameters, description.material, m_context.milliseconds,
            parameters.view, m_context.projection);
        if (m_context.two_sided) description.shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
        PropMaterialDrawOverrides draw;
        if (slot >= m_state.additional_materials.size()) m_state.additional_materials.resize(slot + 1);
        draw.preparation = &m_state.additional_materials[slot];
        draw.color_write_mask = description.color_write_mask;
        draw.deferred_pass = m_context.additional_only;
        draw.mesh = mesh;
        const auto* geometry = m_renderer.Mesh_Geometry(mesh);
        assert(geometry != nullptr);
        return submit(geometry->Vertices(), geometry->Indices(), description.shader,
            description.textures, parameters, draw);
    }

private:
    void Prepare_Source() {
        if (!m_source.empty()) return;
        GRAPHICS_PROFILE_SCOPE("Graphics.Mesh.TransformVertices");
        if (!m_workspace) m_workspace.emplace(m_cache.Acquire());
        assert(m_normals.empty() || m_normals.size() == m_positions.size());
        const auto prepare = [&]<bool HasNormals>() {
            m_source = m_workspace->Workspace().Prepare_Source(m_positions.size(), [&](PropSourceVertex& vertex, std::size_t i) {
                vertex.position = {m_positions[i][0], m_positions[i][1], m_positions[i][2]};
                if constexpr (HasNormals) vertex.normal = {m_normals[i][0], m_normals[i][1], m_normals[i][2]};
                else vertex.normal = {0,0,1};
            });
        };
        if (m_normals.empty()) prepare.template operator()<false>();
        else prepare.template operator()<true>();
    }
    std::uint64_t m_bone_revision=0;
    std::shared_ptr<const std::vector<std::uint16_t>> m_bones;
    std::span<const Position> m_positions;
    std::span<const Position> m_normals;
    std::span<const Triangle> m_triangles;
    std::uint64_t m_revision;
    std::uint64_t m_topology_revision;
    MeshMaterialBindings<TextureOwner, UV>& m_materials;
    ModelMeshState<TextureOwner>& m_state;
    PropRenderer& m_renderer;
    ModelMeshDrawContext m_context;
    PropExtractionCache& m_cache;
    std::optional<PropExtractionCache::Lease> m_workspace;
    std::span<PropSourceVertex> m_source;
};
}
