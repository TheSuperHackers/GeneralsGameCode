module;
#include "../../profiling/Tracy.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

export module Graphics.Scene.Lighting.Local;
import Graphics.Scene.Lighting;

namespace Graphics
{
// Material lighting retains the authored ambient and attenuation interval in
// addition to the spatial light description used by scene light storage.
export struct MaterialLightSource final
{
    RenderLightType type = RenderLightType::Directional;
    std::array<float,3> position{};
    std::array<float,3> direction{};
    std::array<float,3> ambient{};
    std::array<float,3> diffuse{};
    float intensity = 1;
    double attenuation_start = 0;
    double attenuation_end = 0;
    bool attenuate = false;
    float cone_cosine = 0;
};

export struct LocalLightSample final
{
    std::array<float,3> direction{};
    std::array<float,3> diffuse{};
    std::array<float,3> position{};
    std::array<float,3> source_ambient{};
    std::array<float,3> source_diffuse{};
    float inner_radius = 0;
    float outer_radius = 0;
    bool point = false;
};

// This is the material shader's direct-light capacity, not a shadow-caster limit.
export inline constexpr std::size_t Material_Light_Count = 4;

export class LocalLighting final
{
public:
    std::array<float,3> ambient{};
    std::array<LocalLightSample,Material_Light_Count> lights{};
    std::size_t count = 0;

    void Reset(std::array<float,3> center, std::array<float,3> scene_ambient)
    {
        m_center = center;
        ambient = scene_ambient;
        count = 0;
    }

    void Add(const MaterialLightSource& source)
    {
        GRAPHICS_PROFILE_SCOPE("Graphics.Lighting.Add");
        // Preserve the authored near-black rejection before intensity scaling.
        if (source.diffuse[0]<0.05f && source.diffuse[1]<0.05f && source.diffuse[2]<0.05f)
            return;
        LocalLightSample sample;
        auto contribution_ambient = source.ambient;
        sample.diffuse = source.diffuse;
        bool rejected = false;
        if (source.type == RenderLightType::Directional) {
            sample.direction = source.direction;
        } else {
            for (unsigned axis=0;axis<3;++axis)
                sample.direction[axis] = source.position[axis]-m_center[axis];
            const float distance = std::sqrt(Energy(sample.direction));
            if (distance>0)
                for (auto& axis : sample.direction) axis /= distance;
            float attenuation = 1;
            if (source.attenuate) {
                const double interval = source.attenuation_end-source.attenuation_start;
                if (std::abs(interval)<0.0001f)
                    attenuation = distance>source.attenuation_start ? 0.0f : 1.0f;
                else
                    attenuation = std::clamp(static_cast<float>(1-(distance-source.attenuation_start)/interval),0.0f,1.0f);
            }
            if (source.type == RenderLightType::Spot) {
                float cosine = 0;
                for (unsigned axis=0;axis<3;++axis)
                    cosine -= source.direction[axis]*sample.direction[axis];
                const float width = 1-source.cone_cosine;
                attenuation *= width>0 ? (cosine-source.cone_cosine)/width : (cosine>=1 ? 1.0f : 0.0f);
                attenuation = std::clamp(attenuation,0.0f,1.0f);
            }
            sample.point = source.type == RenderLightType::Point;
            sample.position = source.position;
            sample.inner_radius = static_cast<float>(source.attenuation_start);
            sample.outer_radius = static_cast<float>(source.attenuation_end);
            for (unsigned axis=0;axis<3;++axis) {
                sample.source_ambient[axis] = source.ambient[axis]*source.intensity;
                sample.source_diffuse[axis] = source.diffuse[axis]*source.intensity;
                contribution_ambient[axis] = sample.source_ambient[axis]*attenuation;
                sample.diffuse[axis] = sample.source_diffuse[axis]*attenuation;
            }
            // The shipped diffuse-to-ambient cutoff was zero. Points remain
            // eligible at zero contribution because their per-pixel range varies.
            rejected = Energy(sample.source_diffuse)==0;
        }
        for (unsigned axis=0;axis<3;++axis) ambient[axis] += contribution_ambient[axis];
        if (rejected && !sample.point) return;
        std::size_t insert = 0;
        const float energy = Energy(sample.diffuse);
        while (insert<count && energy<=Energy(lights[insert].diffuse)) ++insert;
        if (insert==lights.size()) return;
        const auto end = std::min(count,lights.size()-1);
        for (auto index=end;index>insert;--index) lights[index] = lights[index-1];
        lights[insert] = sample;
        count = std::min(count+1,lights.size());
    }

    void Finalize()
    {
        for (auto& channel : ambient) channel = std::clamp(channel,0.0f,1.0f);
    }

private:
    static float Energy(const std::array<float,3>& value)
    {
        return value[0]*value[0]+value[1]*value[1]+value[2]*value[2];
    }
    std::array<float,3> m_center{};
};
}
