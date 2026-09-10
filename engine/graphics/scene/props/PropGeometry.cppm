module;
#include "../../profiling/Tracy.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <vector>
#if defined(_M_X64) || defined(__SSE2__)
#include <emmintrin.h>
#endif

export module Graphics.Scene.Props.Geometry;

namespace Graphics
{
// Vertex attributes have the GPU's IEEE binary32 representation.
static_assert(sizeof(float) == sizeof(std::uint32_t) && std::numeric_limits<float>::is_iec559);
export struct PropVertex final
{
    std::array<float, 3> position{};
    std::array<float, 4> color{1, 1, 1, 1};
    std::array<float, 2> uv{};
    std::array<float, 2> secondary_uv{};
    std::array<float, 3> normal{0,0,1};
    std::array<float,4> material_ambient{1,1,1,0};
    std::array<float,4> material_diffuse{1,1,1,1};
    std::array<float,4> material_emissive{};
    std::array<float,4> material_specular{0,0,0,1};
    std::array<float,4> secondary_color{};
    std::array<float,4> tangent{0,0,0,1};
    float bone_index=0;
};
static_assert(sizeof(PropVertex) == 156);

export bool Finite_Prop_Vertices(std::span<const PropVertex> vertices) noexcept
{
    // All 39 components are contiguous binary32 values, with no padding.
    // Inspect their exponent bits without floating-point comparisons: signed
    // zeros and subnormals remain valid, and every infinity/NaN is rejected.
    static_assert(sizeof(PropVertex) == 39 * sizeof(float));
    constexpr std::uint32_t exponent_mask = 0x7f800000u;
    const auto bytes = std::as_bytes(vertices);
    std::size_t offset = 0;
#if defined(_M_X64) || defined(__SSE2__)
    const auto mask = _mm_set1_epi32(exponent_mask);
    const auto invalid = [&](std::size_t at) {
        __m128i values;
        std::memcpy(&values,bytes.data()+at,sizeof(values));
        return _mm_cmpeq_epi32(_mm_and_si128(values,mask),mask);
    };
    for (; bytes.size()-offset >= 64; offset += 64) {
        const auto first = _mm_or_si128(invalid(offset),invalid(offset+16));
        const auto second = _mm_or_si128(invalid(offset+32),invalid(offset+48));
        if (_mm_movemask_epi8(_mm_or_si128(first,second)) != 0) return false;
    }
    for (; bytes.size()-offset >= 16; offset += 16)
        if (_mm_movemask_epi8(invalid(offset)) != 0) return false;
#endif
    for (; offset < bytes.size(); offset += sizeof(std::uint32_t)) {
        std::uint32_t value;
        std::memcpy(&value,bytes.data()+offset,sizeof(value));
        if ((value & exponent_mask) == exponent_mask) return false;
    }
    return true;
}

// Builds one material batch without expanding shared source vertices into
// triangle corners. Source indices define identity: equal positions with
// different UVs, normals or colors remain distinct. Begin separates materials.
export class PropBatchBuilder final
{
public:
    bool Begin(std::size_t source_vertex_count, std::size_t index_count_hint = 0)
    {
        if (source_vertex_count > std::numeric_limits<std::uint32_t>::max()/sizeof(PropVertex)
            || index_count_hint > std::numeric_limits<std::uint32_t>::max()/sizeof(std::uint32_t)) return false;
        m_vertices.clear();
        m_indices.clear();
        if (index_count_hint != 0) {
            m_vertices.reserve(std::min(source_vertex_count,index_count_hint));
            m_indices.reserve(index_count_hint);
        }
        m_remap.assign(source_vertex_count,InvalidIndex);
        return true;
    }

    template<typename ExtractVertex>
    bool Append(std::uint32_t source_index,const ExtractVertex& extract)
    {
        if (source_index >= m_remap.size()
            || m_indices.size() >= std::numeric_limits<std::uint32_t>::max()/sizeof(std::uint32_t)) return false;
        Append_Validated(source_index, extract);
        return true;
    }

    // Mesh publication validates topology and batch sizing once. Internal
    // extraction can then assert those preconditions without per-corner checks.
    template<typename ExtractVertex>
    void Append_Validated(std::uint32_t source_index, const ExtractVertex& extract)
    {
        assert(source_index < m_remap.size());
        assert(m_indices.size() < (std::numeric_limits<std::uint32_t>::max)()/sizeof(std::uint32_t));
        auto& index = m_remap[source_index];
        if (index == InvalidIndex) {
            const auto next = static_cast<std::uint32_t>(m_vertices.size());
            m_vertices.push_back(extract(source_index));
            index = next;
        }
        m_indices.push_back(index);
    }

    std::span<const PropVertex> Vertices() const noexcept { return m_vertices; }
    std::span<const std::uint32_t> Indices() const noexcept { return m_indices; }

    std::size_t Allocated_Bytes() const noexcept
    {
        return m_vertices.capacity()*sizeof(PropVertex)
            +(m_remap.capacity()+m_indices.capacity())*sizeof(std::uint32_t);
    }

private:
    static constexpr std::uint32_t InvalidIndex = std::numeric_limits<std::uint32_t>::max();
    std::vector<std::uint32_t> m_remap;
    std::vector<PropVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
};

// Indexed material geometry shared by terrain decorations. The input
// topology is preserved, including segment boundaries and overlapping layers.
export class PropGeometry final
{
public:
    bool Matches(std::span<const PropVertex> vertices, std::span<const std::uint32_t> indices) const noexcept
    {
        return vertices.size() == m_vertices.size() && indices.size() == m_indices.size()
            && (vertices.empty() || std::memcmp(vertices.data(),m_vertices.data(),vertices.size_bytes()) == 0)
            && (indices.empty() || std::memcmp(indices.data(),m_indices.data(),indices.size_bytes()) == 0);
    }

    bool Assign(std::span<const PropVertex> vertices, std::span<const std::uint32_t> indices)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Props.AssignGeometry");
        if (!Valid(vertices, indices)) return false;
        m_vertices.assign(vertices.begin(), vertices.end());
        m_indices.assign(indices.begin(), indices.end());
        m_bounds_valid = false;
        return true;
    }

    // Append complete material batches, retaining shared vertices and rebasing
    // only their indices. Validation precedes mutation so a rejected batch
    // cannot alter earlier draw ranges.
    bool Append(std::span<const PropVertex> vertices, std::span<const std::uint32_t> indices)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Props.AppendGeometry");
        constexpr auto maximum_bytes = std::numeric_limits<std::uint32_t>::max();
        if (vertices.size() > maximum_bytes / sizeof(PropVertex) - m_vertices.size()
            || indices.size() > maximum_bytes / sizeof(std::uint32_t) - m_indices.size()
            || !Valid(vertices, indices)) return false;
        const auto base_vertex = static_cast<std::uint32_t>(m_vertices.size());
        const auto first_index = m_indices.size();
        m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
        m_indices.resize(first_index + indices.size());
        for (std::size_t index = 0; index < indices.size(); ++index)
            m_indices[first_index + index] = base_vertex + indices[index];
        if (!vertices.empty()) m_bounds_valid = false;
        return true;
    }

    std::span<const PropVertex> Vertices() const noexcept { return m_vertices; }
    std::span<const std::uint32_t> Indices() const noexcept { return m_indices; }

    std::uint32_t Maximum_Bone_Index() const noexcept { Update_Bounds(); return m_maximum_bone; }
    const std::array<float,3>& Minimum_Position() const noexcept { Update_Bounds(); return m_minimum; }
    const std::array<float,3>& Maximum_Position() const noexcept { Update_Bounds(); return m_maximum; }

private:
    void Update_Bounds() const noexcept
    {
        if (m_bounds_valid) return;
        m_minimum = m_maximum = m_vertices.empty() ? std::array<float,3>{} : m_vertices.front().position;
        for (const auto& vertex : m_vertices) for (unsigned axis=0;axis<3;++axis) {
            m_minimum[axis] = std::min(m_minimum[axis],vertex.position[axis]);
            m_maximum[axis] = std::max(m_maximum[axis],vertex.position[axis]);
        }
        m_maximum_bone = 0;
        for (const auto& vertex : m_vertices)
            m_maximum_bone = (std::max)(m_maximum_bone,static_cast<std::uint32_t>(vertex.bone_index));
        m_bounds_valid = true;
    }

    static bool Valid(std::span<const PropVertex> vertices, std::span<const std::uint32_t> indices)
    {
        constexpr auto maximum_bytes = std::numeric_limits<std::uint32_t>::max();
        if (vertices.size() > maximum_bytes / sizeof(PropVertex)
            || indices.size() > maximum_bytes / sizeof(std::uint32_t)
            || indices.size() % 3 != 0) return false;
        if (!Finite_Prop_Vertices(vertices)) return false;
        for (const auto& vertex : vertices)
            if (vertex.bone_index < 0 || vertex.bone_index > 65535
                || vertex.bone_index != static_cast<float>(static_cast<std::uint32_t>(vertex.bone_index))) return false;
        for (std::uint32_t index : indices) if (index >= vertices.size()) return false;
        return true;
    }
    std::vector<PropVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
    mutable std::array<float,3> m_minimum{}, m_maximum{};
    mutable bool m_bounds_valid = false;
    mutable std::uint32_t m_maximum_bone = 0;
};
}
