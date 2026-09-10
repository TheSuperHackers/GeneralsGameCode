module;

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <utility>
#include <vector>

export module Graphics.Scene.Models.MeshMaterialBindings;

import Graphics.Materials.MeshMaterial;
import Graphics.Materials.State;
import Graphics.Scene.Models.MaterialSlots;
import Graphics.Scene.Models.SourceRevision;
import Graphics.Scene.Models.Materials;
import Graphics.Scene.Models.VertexChannels;
import Graphics.Scene.Props.Material;

namespace Graphics {

// The mesh-facing material storage is parameterized by the caller's resource
// owner and UV value.  It owns no file, game, or renderer-specific types.
export template<class TextureOwner, class UVValue>
class MeshMaterialBindings final
{
    using MaterialOwner = std::shared_ptr<MeshMaterial>;
    using TextureSlots = MaterialSlots<TextureOwner>;
    using MaterialSlotsType = MaterialSlots<MaterialOwner>;

    struct ShaderStorage final
    {
        ShaderStorage() = default;
        ShaderStorage(const ShaderStorage& source) : values(source.values) {}
        std::vector<MaterialState> values;
        SourceRevision revision;
    };

public:
    static constexpr int MAX_PASSES = 4;
    static constexpr int MAX_TEX_STAGES = 2;
    static constexpr int MAX_COLOR_ARRAYS = 2;

    MeshMaterialBindings() noexcept
        : m_pass_count(1), m_vertex_count(0), m_polygon_count(0)
    {
        Reset_Sources();
    }

    MeshMaterialBindings(const MeshMaterialBindings& source)
        : m_pass_count(source.m_pass_count), m_vertex_count(source.m_vertex_count),
          m_polygon_count(source.m_polygon_count), m_uv(source.m_uv),
          m_colors(source.m_colors), m_uv_sources(source.m_uv_sources),
          m_textures(source.m_textures), m_shaders(source.m_shaders),
          m_dcg_sources(source.m_dcg_sources), m_dig_sources(source.m_dig_sources),
          m_materials(source.m_materials)
    {
        for (int pass = 0; pass < MAX_PASSES; ++pass) {
            for (int stage = 0; stage < MAX_TEX_STAGES; ++stage) {
                m_texture_arrays[pass][stage] = source.m_texture_arrays[pass][stage].Clone();
            }
            m_material_arrays[pass] = source.m_material_arrays[pass].Clone();
            if (source.m_shader_arrays[pass]) {
                m_shader_arrays[pass] = std::make_shared<ShaderStorage>(
                    *source.m_shader_arrays[pass]);
            }
        }
    }

    MeshMaterialBindings& operator=(const MeshMaterialBindings& source)
    {
        if (this == &source) {
            return *this;
        }

        m_pass_count = source.m_pass_count;
        m_vertex_count = source.m_vertex_count;
        m_polygon_count = source.m_polygon_count;
        m_uv = source.m_uv;
        m_colors = source.m_colors;
        m_uv_sources = source.m_uv_sources;
        m_textures = source.m_textures;
        m_shaders = source.m_shaders;
        m_dcg_sources = source.m_dcg_sources;
        m_dig_sources = source.m_dig_sources;
        m_materials = source.m_materials;
        for (int pass = 0; pass < MAX_PASSES; ++pass) {
            for (int stage = 0; stage < MAX_TEX_STAGES; ++stage) {
                m_texture_arrays[pass][stage] = source.m_texture_arrays[pass][stage].Clone();
            }
            m_material_arrays[pass] = source.m_material_arrays[pass].Clone();
            m_shader_arrays[pass].reset();
            if (source.m_shader_arrays[pass]) {
                m_shader_arrays[pass] = std::make_shared<ShaderStorage>(
                    *source.m_shader_arrays[pass]);
            }
        }
        return *this;
    }

    ~MeshMaterialBindings() = default;

    void Reset(std::size_t polygon_count, std::size_t vertex_count, int pass_count)
    {
        m_polygon_count = polygon_count;
        m_vertex_count = vertex_count;
        m_pass_count = pass_count;
        m_uv.Clear();
        m_colors.Clear();
        Reset_Sources();
        for (int pass = 0; pass < MAX_PASSES; ++pass) {
            for (int stage = 0; stage < MAX_TEX_STAGES; ++stage) {
                m_texture_arrays[pass][stage].Reset();
            }
            m_material_arrays[pass].Reset();
            m_shader_arrays[pass].reset();
        }
    }

    void Set_Pass_Count(int count) noexcept { m_pass_count = count; }
    int Get_Pass_Count() const noexcept { return m_pass_count; }
    void Set_Vertex_Count(std::size_t count) noexcept { m_vertex_count = count; }
    std::size_t Get_Vertex_Count() const noexcept { return m_vertex_count; }
    void Set_Polygon_Count(std::size_t count) noexcept { m_polygon_count = count; }
    std::size_t Get_Polygon_Count() const noexcept { return m_polygon_count; }

    UVValue* Get_UV_Array(int pass, int stage)
    {
        const int source = m_uv_sources[pass][stage];
        return source < 0 ? nullptr : m_uv.Get(static_cast<std::size_t>(source));
    }

    const UVValue* Peek_UV_Array(int pass, int stage) const noexcept
    {
        const int source = m_uv_sources[pass][stage];
        return source < 0 ? nullptr : m_uv.Peek(static_cast<std::size_t>(source));
    }

    UVValue* Get_UV_Array_By_Index(int index, bool create = true)
    {
        assert(index >= 0);
        if (index < 0) {
            return nullptr;
        }
        return create ? m_uv.Create(static_cast<std::size_t>(index), m_vertex_count)
            : m_uv.Get(static_cast<std::size_t>(index));
    }

    const UVValue* Peek_UV_Array_By_Index(int index) const noexcept
    {
        return index < 0 ? nullptr : m_uv.Peek(static_cast<std::size_t>(index));
    }

    int Get_UV_Array_Count() const noexcept
    {
        return static_cast<int>(m_uv.Count());
    }

    std::uint64_t UV_Revision(int pass, int stage) const noexcept
    {
        const int source = m_uv_sources[pass][stage];
        return source < 0 ? 0 : m_uv.Revision(static_cast<std::size_t>(source));
    }

    void Set_UV_Source(int pass, int stage, int source) noexcept
    {
        assert(pass >= 0 && pass < MAX_PASSES);
        assert(stage >= 0 && stage < MAX_TEX_STAGES);
        m_uv_sources[pass][stage] = source;
    }

    int Get_UV_Source(int pass, int stage) const noexcept
    {
        assert(pass >= 0 && pass < MAX_PASSES);
        assert(stage >= 0 && stage < MAX_TEX_STAGES);
        return m_uv_sources[pass][stage];
    }

    void Install_UV_Array(int pass, int stage, std::span<const UVValue> values)
    {
        Set_UV_Source(pass, stage, static_cast<int>(m_uv.Install(values)));
    }

    void Make_UV_Array_Unique(int pass, int stage)
    {
        const int source = m_uv_sources[pass][stage];
        if (source >= 0) {
            m_uv.Make_Unique(static_cast<std::size_t>(source));
        }
    }

    void Share_UV_Array(std::size_t destination, const MeshMaterialBindings& source,
        std::size_t source_index)
    {
        m_uv.Share(destination, source.m_uv, source_index);
    }

    std::size_t Import_UV_Array(const MeshMaterialBindings& source, std::size_t source_index)
    {
        return m_uv.Import(source.m_uv, source_index);
    }

    bool Has_UV(int pass, int stage) const noexcept
    {
        return m_uv_sources[pass][stage] >= 0;
    }

    unsigned* Get_DCG_Array(int pass)
    {
        return Get_Color_Array_For_Source(m_dcg_sources[pass]);
    }

    unsigned* Get_DIG_Array(int pass)
    {
        return Get_Color_Array_For_Source(m_dig_sources[pass]);
    }

    const unsigned* Peek_DCG_Array(int pass) const noexcept
    {
        return Peek_Color_Array(Color_Index(m_dcg_sources[pass]));
    }

    const unsigned* Peek_DIG_Array(int pass) const noexcept
    {
        return Peek_Color_Array(Color_Index(m_dig_sources[pass]));
    }

    std::uint64_t DCG_Revision(int pass) const noexcept
    {
        return Color_Revision(m_dcg_sources[pass]);
    }

    std::uint64_t DIG_Revision(int pass) const noexcept
    {
        return Color_Revision(m_dig_sources[pass]);
    }

    void Allocate_Color_Array(int index)
    {
        assert(index >= 0 && index < MAX_COLOR_ARRAYS);
        m_colors.Allocate(static_cast<std::size_t>(index), m_vertex_count);
    }

    void Set_Color(int index, std::size_t vertex, unsigned value)
    {
        assert(index >= 0 && index < MAX_COLOR_ARRAYS);
        m_colors.Set(static_cast<std::size_t>(index), vertex, value);
    }

    void Set_DCG_Source(int pass, PropColorSource source) noexcept
    {
        m_dcg_sources[pass] = source;
    }

    void Set_DIG_Source(int pass, PropColorSource source) noexcept
    {
        m_dig_sources[pass] = source;
    }

    PropColorSource Get_DCG_Source(int pass) const noexcept
    {
        return m_dcg_sources[pass];
    }

    PropColorSource Get_DIG_Source(int pass) const noexcept
    {
        return m_dig_sources[pass];
    }

    unsigned* Get_Color_Array(int index, bool create = true)
    {
        assert(index >= 0 && index < MAX_COLOR_ARRAYS);
        if (index < 0 || index >= MAX_COLOR_ARRAYS) {
            return nullptr;
        }
        if (create) {
            return m_colors.Create(static_cast<std::size_t>(index), m_vertex_count);
        }
        return m_colors.Get(static_cast<std::size_t>(index));
    }

    const unsigned* Peek_Color_Array(int index) const noexcept
    {
        return index < 0 || index >= MAX_COLOR_ARRAYS
            ? nullptr : m_colors.Peek(static_cast<std::size_t>(index));
    }

    bool Has_Color_Array(int index) const noexcept
    {
        return index >= 0 && index < MAX_COLOR_ARRAYS
            && m_colors.Is_Allocated(static_cast<std::size_t>(index));
    }

    void Make_Color_Array_Unique(int index)
    {
        if (index >= 0 && index < MAX_COLOR_ARRAYS) {
            m_colors.Make_Unique(static_cast<std::size_t>(index));
        }
    }

    void Set_Single_Material(const MaterialOwner& material, int pass = 0)
    {
        m_materials[pass] = material;
    }

    MaterialOwner Get_Single_Material(int pass = 0) const
    {
        return m_materials[pass];
    }

    MeshMaterial* Peek_Single_Material(int pass = 0) const noexcept
    {
        return m_materials[pass].get();
    }

    void Set_Material(std::size_t vertex, const MaterialOwner& material, int pass = 0)
    {
        auto* slots = Get_Material_Array(pass, true);
        slots->Set(vertex, material);
    }

    MaterialOwner Get_Material(std::size_t vertex, int pass = 0) const
    {
        if (m_material_arrays[pass].Is_Allocated()) {
            return m_material_arrays[pass].Get(vertex);
        }
        return m_materials[pass];
    }

    MeshMaterial* Peek_Material(std::size_t vertex, int pass = 0) const noexcept
    {
        if (m_material_arrays[pass].Is_Allocated()) {
            if (const auto* owner = m_material_arrays[pass].Peek(vertex)) {
                return owner->get();
            }
        }
        return m_materials[pass].get();
    }

    bool Has_Material_Array(int pass) const noexcept
    {
        return m_material_arrays[pass].Is_Allocated();
    }

    bool Has_Material_Data(int pass) const noexcept
    {
        return static_cast<bool>(m_materials[pass]) || Has_Material_Array(pass);
    }

    MaterialSlotsType* Get_Material_Array(int pass, bool create = true)
    {
        if (create && !m_material_arrays[pass].Is_Allocated()) {
            m_material_arrays[pass].Allocate(m_vertex_count);
        }
        return m_material_arrays[pass].Is_Allocated() ? &m_material_arrays[pass] : nullptr;
    }

    const MaterialSlotsType* Peek_Material_Array(int pass) const noexcept
    {
        return m_material_arrays[pass].Is_Allocated() ? &m_material_arrays[pass] : nullptr;
    }

    void Set_Single_Texture(const TextureOwner& texture, int pass = 0, int stage = 0)
    {
        m_textures[pass][stage] = texture;
    }

    TextureOwner Get_Single_Texture(int pass = 0, int stage = 0) const
    {
        return m_textures[pass][stage];
    }

    auto Peek_Single_Texture(int pass = 0, int stage = 0) const noexcept
    {
        return Resource_Pointer(m_textures[pass][stage]);
    }

    void Set_Texture(std::size_t polygon, const TextureOwner& texture,
        int pass = 0, int stage = 0)
    {
        auto* slots = Get_Texture_Array(pass, stage, true);
        slots->Set(polygon, texture);
    }

    TextureOwner Get_Texture(std::size_t polygon, int pass = 0, int stage = 0) const
    {
        if (m_texture_arrays[pass][stage].Is_Allocated()) {
            return m_texture_arrays[pass][stage].Get(polygon);
        }
        return m_textures[pass][stage];
    }

    auto Peek_Texture(std::size_t polygon, int pass = 0, int stage = 0) const noexcept
    {
        if (m_texture_arrays[pass][stage].Is_Allocated()) {
            if (const auto* owner = m_texture_arrays[pass][stage].Peek(polygon)) {
                return Resource_Pointer(*owner);
            }
        }
        return Resource_Pointer(m_textures[pass][stage]);
    }

    bool Has_Texture_Array(int pass, int stage) const noexcept
    {
        return m_texture_arrays[pass][stage].Is_Allocated();
    }

    bool Has_Texture_Data(int pass, int stage) const noexcept
    {
        return Peek_Single_Texture(pass, stage) != nullptr
            || m_texture_arrays[pass][stage].Is_Allocated();
    }

    TextureSlots* Get_Texture_Array(int pass, int stage, bool create = true)
    {
        if (create && !m_texture_arrays[pass][stage].Is_Allocated()) {
            m_texture_arrays[pass][stage].Allocate(m_polygon_count);
        }
        return m_texture_arrays[pass][stage].Is_Allocated()
            ? &m_texture_arrays[pass][stage] : nullptr;
    }

    const TextureSlots* Peek_Texture_Array(int pass, int stage) const noexcept
    {
        return m_texture_arrays[pass][stage].Is_Allocated()
            ? &m_texture_arrays[pass][stage] : nullptr;
    }

    // Used by the two-pass fog conversion. The destination is intentionally
    // left alone once it has an authored slot collection.
    void Clone_Texture_Array_If_Absent(int destination_pass, int destination_stage,
        int source_pass, int source_stage)
    {
        if (!m_texture_arrays[destination_pass][destination_stage].Is_Allocated()
            && m_texture_arrays[source_pass][source_stage].Is_Allocated()) {
            m_texture_arrays[destination_pass][destination_stage] =
                m_texture_arrays[source_pass][source_stage].Clone();
        }
    }

    std::optional<std::array<std::uint64_t, 4>> Grouping_Revisions(int pass) const noexcept
    {
        const std::array versions{
            m_shader_arrays[pass] ? m_shader_arrays[pass]->revision.Token() : 0,
            m_material_arrays[pass].Revision(), m_texture_arrays[pass][0].Revision(),
            m_texture_arrays[pass][1].Revision()};
        if ((Has_Shader_Array(pass) && versions[0] == 0)
            || (Has_Material_Array(pass) && versions[1] == 0)
            || (Has_Texture_Array(pass, 0) && versions[2] == 0)
            || (Has_Texture_Array(pass, 1) && versions[3] == 0)) return std::nullopt;
        return versions;
    }

    void Set_Single_Shader(MaterialState shader, int pass = 0) noexcept
    {
        m_shaders[pass] = shader;
    }

    MaterialState Get_Single_Shader(int pass = 0) const noexcept
    {
        return m_shaders[pass];
    }

    bool Has_Shader_Array(int pass) const noexcept
    {
        return static_cast<bool>(m_shader_arrays[pass]);
    }

    bool Has_Shader_Data(int pass) const noexcept
    {
        return m_shaders[pass] != MaterialState{0} || Has_Shader_Array(pass);
    }

    MaterialState Get_Shader(std::size_t polygon, int pass = 0) const noexcept
    {
        if (m_shader_arrays[pass] && polygon < m_shader_arrays[pass]->values.size()) {
            return m_shader_arrays[pass]->values[polygon];
        }
        return m_shader_arrays[pass] ? MaterialState{0} : m_shaders[pass];
    }

    void Set_Shader(std::size_t polygon, MaterialState shader, int pass = 0)
    {
        Allocate_Shader_Array(pass);
        if (polygon < m_shader_arrays[pass]->values.size()) {
            m_shader_arrays[pass]->revision.Invalidate();
            m_shader_arrays[pass]->values[polygon] = shader;
        }
    }

    MaterialState* Get_Shader_Array(int pass, bool create = true)
    {
        if (create) Allocate_Shader_Array(pass);
        if (!m_shader_arrays[pass] || m_shader_arrays[pass]->values.empty()) {
            return nullptr;
        }
        m_shader_arrays[pass]->revision.Expose_Writable();
        return m_shader_arrays[pass]->values.data();
    }

    const MaterialState* Peek_Shader_Array(int pass) const noexcept
    {
        if (!m_shader_arrays[pass] || m_shader_arrays[pass]->values.empty()) {
            return nullptr;
        }
        return m_shader_arrays[pass]->values.data();
    }

    std::size_t Shader_Count(int pass) const noexcept
    {
        return m_shader_arrays[pass] ? m_shader_arrays[pass]->values.size() : 0;
    }

    template<class Function>
    void For_Each_Shader(int pass, Function&& function)
    {
        if (m_shader_arrays[pass]) {
            m_shader_arrays[pass]->revision.Expose_Writable();
            for (auto& shader : m_shader_arrays[pass]->values) {
                function(shader);
            }
        } else {
            function(m_shaders[pass]);
        }
    }

    template<class Function>
    void For_Each_Shader(int pass, Function&& function) const
    {
        if (m_shader_arrays[pass]) {
            for (const auto& shader : m_shader_arrays[pass]->values) {
                function(shader);
            }
        } else {
            function(m_shaders[pass]);
        }
    }

    void Share_Shader_Array(int destination_pass, const MeshMaterialBindings& source,
        int source_pass)
    {
        m_shader_arrays[destination_pass] = source.m_shader_arrays[source_pass];
    }

    bool Is_Empty() const noexcept
    {
        for (int index = 0; index < MAX_COLOR_ARRAYS; ++index) {
            if (Has_Color_Array(index)) {
                return false;
            }
        }
        if (!m_uv.Empty()) {
            return false;
        }
        for (int pass = 0; pass < MAX_PASSES; ++pass) {
            for (int stage = 0; stage < MAX_TEX_STAGES; ++stage) {
                if (Peek_Single_Texture(pass, stage) != nullptr
                    || Has_Texture_Array(pass, stage)) {
                    return false;
                }
            }
            // Shader-only state intentionally does not make the collection
            // non-empty; this is the retained alternate-description contract.
            if (Peek_Single_Material(pass) != nullptr || Has_Material_Array(pass)) {
                return false;
            }
        }
        return true;
    }

    void Init_Alternate(const MeshMaterialBindings& defaults,
        const MeshMaterialBindings& alternate, std::uint32_t now)
    {
        Reset(defaults.m_polygon_count, defaults.m_vertex_count, defaults.m_pass_count);

        for (int index = 0; index < MAX_COLOR_ARRAYS; ++index) {
            if (alternate.Has_Color_Array(index)) {
                m_colors.Share(static_cast<std::size_t>(index), alternate.m_colors,
                    static_cast<std::size_t>(index));
            } else if (defaults.Has_Color_Array(index)) {
                m_colors.Share(static_cast<std::size_t>(index), defaults.m_colors,
                    static_cast<std::size_t>(index));
            }
        }

        for (std::size_t index = 0; index < alternate.m_uv.Count(); ++index) {
            m_uv.Share(index, alternate.m_uv, index);
        }

        for (int pass = 0; pass < MAX_PASSES; ++pass) {
            for (int stage = 0; stage < MAX_TEX_STAGES; ++stage) {
                if (alternate.m_uv_sources[pass][stage] < 0) {
                    if (defaults.m_uv_sources[pass][stage] >= 0) {
                        m_uv_sources[pass][stage] = static_cast<int>(m_uv.Import(
                            defaults.m_uv,
                            static_cast<std::size_t>(defaults.m_uv_sources[pass][stage])));
                    }
                } else {
                    m_uv_sources[pass][stage] = alternate.m_uv_sources[pass][stage];
                }

                if (alternate.Peek_Single_Texture(pass, stage) != nullptr
                    || alternate.Has_Texture_Array(pass, stage)) {
                    m_textures[pass][stage] = alternate.m_textures[pass][stage];
                    m_texture_arrays[pass][stage] = alternate.m_texture_arrays[pass][stage];
                } else {
                    m_textures[pass][stage] = defaults.m_textures[pass][stage];
                    m_texture_arrays[pass][stage] = defaults.m_texture_arrays[pass][stage];
                }
            }

            m_dcg_sources[pass] = alternate.m_dcg_sources[pass] == PropColorSource::Material
                ? defaults.m_dcg_sources[pass] : alternate.m_dcg_sources[pass];
            // Default shaders always win. Alternate shader arrays share the
            // default array just as the former description did.
            m_shaders[pass] = defaults.m_shaders[pass];
            Share_Shader_Array(pass, defaults, pass);

            if (alternate.m_materials[pass] || alternate.Has_Material_Array(pass)) {
                m_materials[pass] = alternate.m_materials[pass];
                m_material_arrays[pass] = alternate.m_material_arrays[pass];
            } else if (defaults.m_materials[pass]) {
                m_materials[pass] = defaults.m_materials[pass]->Clone(now);
            } else {
                // A default per-vertex material array without an alternate
                // array remains unsupported and therefore empty here.
                m_materials[pass].reset();
            }
        }
    }

    bool Do_Mappers_Need_Normals() const noexcept
    {
        for (int pass = 0; pass < m_pass_count; ++pass) {
            if (m_materials[pass]) {
                if (m_materials[pass]->Mappings_Need_Normals()) {
                    return true;
                }
                continue;
            }
            MeshMaterial* previous = nullptr;
            for (std::size_t vertex = 0; vertex < m_vertex_count; ++vertex) {
                auto* material = Peek_Material(vertex, pass);
                if (material != previous && material) {
                    if (material->Mappings_Need_Normals()) {
                        return true;
                    }
                    previous = material;
                }
            }
        }
        return false;
    }

    void Set_Two_Sided() noexcept
    {
        for (int pass = 0; pass < m_pass_count; ++pass) {
            m_shaders[pass].Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
            if (m_shader_arrays[pass]) {
                m_shader_arrays[pass]->revision.Invalidate();
                for (auto& shader : m_shader_arrays[pass]->values) {
                    shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
                }
            }
        }
    }

    void Remap_Resources(const MeshMaterialBindings& source,
        const ModelMaterials<TextureOwner>& source_resources,
        const ModelMaterials<TextureOwner>& destination_resources)
    {
        MaterialResourceRemap<MaterialOwner> materials(
            source_resources.materials, destination_resources.materials);
        MaterialResourceRemap<TextureOwner> textures(
            source_resources.textures, destination_resources.textures);

        if (!materials.Empty()) {
            for (int pass = 0; pass < source.m_pass_count; ++pass) {
                if (source.m_material_arrays[pass].Is_Allocated()) {
                    materials.Remap_Slots(source.m_material_arrays[pass],
                        m_material_arrays[pass], source.m_vertex_count);
                } else {
                    m_materials[pass] = materials.Find(source.m_materials[pass].get());
                }
            }
        }
        if (!textures.Empty()) {
            for (int pass = 0; pass < source.m_pass_count; ++pass) {
                for (int stage = 0; stage < MAX_TEX_STAGES; ++stage) {
                    if (source.m_texture_arrays[pass][stage].Is_Allocated()) {
                        textures.Remap_Slots(source.m_texture_arrays[pass][stage],
                            m_texture_arrays[pass][stage], source.m_polygon_count);
                    } else {
                        m_textures[pass][stage] = textures.Find(
                            Resource_Pointer(source.m_textures[pass][stage]));
                    }
                }
            }
        }
    }

private:
    void Reset_Sources() noexcept
    {
        for (int index = 0; index < MAX_PASSES; ++index) {
            m_dcg_sources[index] = PropColorSource::Material;
            m_dig_sources[index] = PropColorSource::Material;
            m_shaders[index] = MaterialState{0};
            m_materials[index].reset();
            for (int stage = 0; stage < MAX_TEX_STAGES; ++stage) {
                m_uv_sources[index][stage] = -1;
                m_textures[index][stage] = TextureOwner{};
            }
        }
    }

    void Allocate_Shader_Array(int pass)
    {
        if (!m_shader_arrays[pass]) {
            m_shader_arrays[pass] = std::make_shared<ShaderStorage>();
            // Newly allocated entries retain the authored all-zero default.
            m_shader_arrays[pass]->values.assign(m_polygon_count, MaterialState{0});
        }
    }

    static int Color_Index(PropColorSource source) noexcept
    {
        if (source == PropColorSource::PrimaryColor) return 0;
        if (source == PropColorSource::SecondaryColor) return 1;
        return -1;
    }

    std::uint64_t Color_Revision(PropColorSource source) const noexcept
    {
        const int index = Color_Index(source);
        return index < 0 ? 0 : m_colors.Revision(static_cast<std::size_t>(index));
    }

    unsigned* Get_Color_Array_For_Source(PropColorSource source)
    {
        if (source == PropColorSource::PrimaryColor) {
            return Get_Color_Array(0, false);
        }
        if (source == PropColorSource::SecondaryColor) {
            return Get_Color_Array(1, false);
        }
        return nullptr;
    }

    static auto Resource_Pointer(const TextureOwner& owner) noexcept
    {
        return std::to_address(owner);
    }

    int m_pass_count;
    std::size_t m_vertex_count;
    std::size_t m_polygon_count;
    VertexChannels<UVValue> m_uv;
    VertexChannels<unsigned> m_colors;
    std::array<std::array<int, MAX_TEX_STAGES>, MAX_PASSES> m_uv_sources{};
    std::array<std::array<TextureOwner, MAX_TEX_STAGES>, MAX_PASSES> m_textures{};
    std::array<MaterialState, MAX_PASSES> m_shaders{};
    std::array<MaterialOwner, MAX_PASSES> m_materials{};
    std::array<PropColorSource, MAX_PASSES> m_dcg_sources{};
    std::array<PropColorSource, MAX_PASSES> m_dig_sources{};
    std::array<std::array<TextureSlots, MAX_TEX_STAGES>, MAX_PASSES> m_texture_arrays{};
    std::array<MaterialSlotsType, MAX_PASSES> m_material_arrays{};
    std::array<std::shared_ptr<ShaderStorage>, MAX_PASSES> m_shader_arrays{};
};

}
