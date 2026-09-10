module;
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Props.SkinBounds;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.SkinPalettes;

namespace Graphics {
// Cache bind-pose bounds by bone once. Evaluating a pose visits bones, not
// every vertex, and conservatively includes every deformed mesh position.
export class PropSkinBounds final {
public:
    void Clear() noexcept { m_bones.clear(); m_prepared=false; }
    bool Evaluate(const PropGeometry& geometry,std::span<const PropBoneTransform> pose,
        std::array<float,3>& minimum,std::array<float,3>& maximum)
    {
        if (pose.empty()) {
            minimum=geometry.Minimum_Position(); maximum=geometry.Maximum_Position();
            return true;
        }
        if (geometry.Maximum_Bone_Index()>=pose.size()) return false;
        if (!m_prepared) {
            m_bones.resize(geometry.Maximum_Bone_Index()+1);
            for (const auto& vertex : geometry.Vertices()) {
                const auto index=static_cast<std::uint32_t>(vertex.bone_index);
                auto& bone=m_bones[index];
                if (!bone.used) {
                    bone.minimum=bone.maximum=vertex.position; bone.index=index; bone.used=true;
                } else for (unsigned axis=0; axis<3; ++axis) {
                    bone.minimum[axis]=(std::min)(bone.minimum[axis],vertex.position[axis]);
                    bone.maximum[axis]=(std::max)(bone.maximum[axis],vertex.position[axis]);
                }
            }
            std::erase_if(m_bones,[](const Bone& bone) { return !bone.used; });
            m_prepared=true;
        }
        minimum.fill((std::numeric_limits<float>::max)());
        maximum.fill(std::numeric_limits<float>::lowest());
        for (const auto& bone : m_bones) for (unsigned corner=0; corner<8; ++corner) {
            std::array<float,3> point;
            for (unsigned axis=0; axis<3; ++axis)
                point[axis]=(corner&(1u<<axis)) ? bone.maximum[axis] : bone.minimum[axis];
            const auto transformed=Transform_Prop_Skin_Position(point,pose[bone.index]);
            for (unsigned axis=0; axis<3; ++axis) {
                const auto& matrix=pose[bone.index];
                const unsigned row=axis*4;
                const double magnitude=std::abs(double(matrix[row])*point[0])
                    +std::abs(double(matrix[row+1])*point[1])+std::abs(double(matrix[row+2])*point[2])
                    +std::abs(double(matrix[row+3]));
                const double error=8*std::numeric_limits<float>::epsilon()*magnitude
                    +8*std::numeric_limits<float>::min();
                const float low=std::nextafter(static_cast<float>(transformed[axis]-error),
                    -std::numeric_limits<float>::infinity());
                const float high=std::nextafter(static_cast<float>(transformed[axis]+error),
                    std::numeric_limits<float>::infinity());
                minimum[axis]=(std::min)(minimum[axis],low);
                maximum[axis]=(std::max)(maximum[axis],high);
            }
        }
        if (m_bones.empty()) minimum=maximum={};
        return true;
    }
private:
    struct Bone {
        std::array<float,3> minimum{},maximum{};
        std::uint32_t index=0;
        bool used=false;
    };
    std::vector<Bone> m_bones;
    bool m_prepared=false;
};
}
