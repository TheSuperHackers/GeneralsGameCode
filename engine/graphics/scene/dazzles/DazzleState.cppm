module;
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
export module Graphics.Scene.Dazzles.State;
import Assets.Dazzles;

namespace Graphics {
export struct DazzleIntensity final {
    float intensity = 1;
    float size = 0;
    float halo = 1;
};
export struct DazzleState final {
    float intensity = 0;
    float size = 0;
    float halo = 0;
    std::array<float, 3> screen_position{};
    std::array<float, 3> direction{};
    std::array<float, 3> color{1, 1, 1};
    std::array<float, 3> halo_color{1, 1, 1};
    float lens_flare_intensity = 1;
    float scale = 1;
    float visibility = 0;
    std::uint32_t creation_time = 0;
};
export struct DazzleView final {
    std::array<float, 16> view{};
    std::array<float, 16> projection{};
    std::array<float, 3> camera_position{};
    std::uint32_t milliseconds = 0;
    float frame_milliseconds = 0;
};
namespace DazzleStateDetail {
float Dot(const std::array<float, 3>& a, const std::array<float, 3>& b) {
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
std::array<float, 4> Transform(const std::array<float, 16>& matrix, const std::array<float, 4>& value) {
    std::array<float, 4> result;
    for (unsigned i = 0; i < 4; ++i)
        result[i] = matrix[i*4]*value[0] + matrix[i*4+1]*value[1] + matrix[i*4+2]*value[2] + matrix[i*4+3]*value[3];
    return result;
}
}
export DazzleIntensity Calculate_Dazzle_Intensity(const Assets::DazzleDefinition& definition,
    const std::array<float, 3>& camera_direction, const std::array<float, 3>& dazzle_direction,
    const std::array<float, 3>& toward_camera, float squared_distance)
{
    assert(definition.area > 0);
    DazzleIntensity result;
    const float dot = -DazzleStateDetail::Dot(toward_camera, camera_direction);
    result.intensity = dot;
    if (definition.use_camera_translation && squared_distance > definition.fade_end*definition.fade_end) {
        result.intensity = 0;
        return result;
    }
    result.intensity = std::clamp((result.intensity - (1 - definition.area)) / definition.area, 0.f, 1.f);
    if (definition.direction_area > 0) {
        const float angle = -DazzleStateDetail::Dot(camera_direction, dazzle_direction);
        result.intensity *= std::clamp((angle - (1 - definition.direction_area)) / definition.direction_area, 0.f, 1.f);
    }
    if (result.intensity > 0) {
        result.size = std::pow(result.intensity, definition.size_power);
        result.intensity = std::pow(result.intensity, definition.intensity_power);
    }
    if (definition.halo_intensity_power > .0001f)
        result.halo = dot > 0 ? result.halo * std::pow(dot, definition.halo_intensity_power) : 0;
    result.intensity *= definition.intensity;
    result.halo *= definition.halo_intensity;
    if (definition.use_camera_translation && squared_distance > definition.fade_start*definition.fade_start) {
        const float fade = 1 - (std::sqrt(squared_distance) - definition.fade_start) / (definition.fade_end - definition.fade_start);
        result.intensity *= fade;
        result.halo *= fade;
    }
    return result;
}

export void Set_Dazzle_Direction(DazzleState& state, const Assets::DazzleDefinition& definition,
    const std::array<float, 16>& transform) {
    const auto& direction = definition.direction;
    const auto result = DazzleStateDetail::Transform(transform, {direction[0], direction[1], direction[2], 0});
    state.direction = {result[0], result[1], result[2]};
}

// Visibility is sampled only for positive glare intensity. Halos remain
// independent of occlusion. Blinking preserves the previously prepared frame.
export template<class Visibility>
bool Prepare_Dazzle(DazzleState& state, const Assets::DazzleDefinition& definition,
    const std::array<float, 3>& position, const DazzleView& view, Visibility&& visibility)
{
    state.visibility = 1;
    if (definition.blink_period > 0 && std::fmod(float(view.milliseconds - state.creation_time) / 1000,
        definition.blink_period) > definition.blink_on_time) {
        state.visibility = 0;
        return state.intensity > 0 || state.halo > 0;
    }
    auto projected = DazzleStateDetail::Transform(view.view, {position[0], position[1], position[2], 1});
    projected = DazzleStateDetail::Transform(view.projection, projected);
    assert(projected[3] != 0);
    state.screen_position = {projected[0]/projected[3], projected[1]/projected[3], projected[2]/projected[3]};
    const std::array camera_direction{-view.view[8], -view.view[9], -view.view[10]};
    std::array toward_camera{view.camera_position[0]-position[0], view.camera_position[1]-position[1], view.camera_position[2]-position[2]};
    const float squared_distance = DazzleStateDetail::Dot(toward_camera, toward_camera);
    if (squared_distance != 0) {
        const float reciprocal_length = 1 / std::sqrt(squared_distance);
        for (auto& value : toward_camera) value *= reciprocal_length;
    }
    auto result = Calculate_Dazzle_Intensity(definition, camera_direction, state.direction, toward_camera, squared_distance);
    state.halo = result.halo;
    const float weight = std::pow(definition.history_weight, view.frame_milliseconds);
    if (result.intensity > 0) {
        state.visibility = visibility();
        result.intensity *= state.visibility;
    } else state.visibility = 0;
    if (state.visibility == 0) {
        state.intensity = result.intensity*(1-weight) + state.intensity*weight;
        if (state.intensity < .05f) state.intensity = 0;
        state.size = result.size*(1-weight) + state.size*weight;
    } else {
        state.intensity = result.intensity;
        state.size = result.size;
    }
    return state.intensity > 0 || state.halo > 0;
}
}
