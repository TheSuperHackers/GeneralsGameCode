module;
#include <cassert>
#include <bit>
#include <cstdint>
#include <cstring>
#include <span>
#include <type_traits>
#include <vector>
export module Graphics.Scene.Models.Deformation;
import Graphics.Scene.Models.GeometryMath;
import Graphics.Scene.Models.Hierarchy;
import Graphics.Scene.Models.SourceRevision;
import Graphics.Scene.AffineTransform;

namespace Graphics {
// Each mesh instance owns its output. Repeated views can share the evaluated
// pose, while changes between views must remain immediately visible.
export template<class Position>
class ModelDeformation final {
public:
    ModelDeformation() = default;
    ModelDeformation(const ModelDeformation&) = delete;
    ModelDeformation& operator=(const ModelDeformation&) = delete;

    void Update(std::span<const Position> positions, std::span<const Position> normals,
        std::span<const std::uint16_t> bones, const ModelHierarchy& hierarchy,
        std::uint64_t source_revision)
    {
        assert(bones.size() == positions.size());
        assert(normals.empty() || normals.size() == positions.size());
        const bool same_source = source_revision != 0
            ? source_revision == m_source_revision && positions.data() == m_position_source
                && normals.data() == m_normal_source && positions.size() == m_positions.size()
                && normals.size() == m_normals.size()
            : m_source_revision == 0 && Equal(positions, m_source_positions)
                && Equal(normals, m_source_normals);
        bool same_pose = m_pose.size() == static_cast<std::size_t>(hierarchy.Bone_Count());
        for (std::size_t bone = 0; same_pose && bone < m_pose.size(); ++bone) {
            const auto& matrix = hierarchy.World_Transform(static_cast<int>(bone)).matrix;
            same_pose = std::memcmp(matrix.data(), m_pose[bone].matrix.data(), sizeof(matrix)) == 0;
        }
        // Bone-link pointers can escape the source revision domain.
        if (m_valid && same_source && same_pose && Equal(bones, m_bones)) return;

        m_positions.resize(positions.size());
        m_normals.resize(normals.size());
        Deform_Model_Geometry(positions, normals, bones, hierarchy,
            std::span<Position>(m_positions), std::span<Position>(m_normals));
        m_source_revision = source_revision;
        m_position_source = positions.data();
        m_normal_source = normals.data();
        if (source_revision == 0) {
            m_source_positions.assign(positions.begin(), positions.end());
            m_source_normals.assign(normals.begin(), normals.end());
        }
        m_bones.assign(bones.begin(), bones.end());
        m_pose.resize(hierarchy.Bone_Count());
        for (std::size_t bone = 0; bone < m_pose.size(); ++bone)
            m_pose[bone] = hierarchy.World_Transform(static_cast<int>(bone));
        m_revision.Invalidate();
        m_valid = true;
    }

    std::span<const Position> Positions() const noexcept { return m_positions; }
    std::span<const Position> Normals() const noexcept { return m_normals; }
    std::uint64_t Revision() const noexcept { return m_revision.Token(); }

private:
    template<class Value>
    static bool Equal(std::span<const Value> source, const std::vector<Value>& cached)
    {
        if (source.size() != cached.size()) return false;
        if constexpr (std::is_arithmetic_v<Value>) {
            return source.empty() || std::memcmp(source.data(), cached.data(), source.size_bytes()) == 0;
        } else {
            for (std::size_t vertex = 0; vertex < source.size(); ++vertex)
                for (unsigned axis = 0; axis < 3; ++axis)
                    if (std::bit_cast<std::uint32_t>(source[vertex][axis])
                        != std::bit_cast<std::uint32_t>(cached[vertex][axis])) return false;
            return true;
        }
    }
    std::vector<Position> m_positions, m_normals;
    std::vector<Position> m_source_positions, m_source_normals;
    std::vector<std::uint16_t> m_bones;
    std::vector<RenderTransform> m_pose;
    const Position* m_position_source = nullptr;
    const Position* m_normal_source = nullptr;
    std::uint64_t m_source_revision = 0;
    SourceRevision m_revision;
    bool m_valid = false;
};
}
