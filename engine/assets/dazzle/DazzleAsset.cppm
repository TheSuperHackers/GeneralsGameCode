module;
#include <array>
#include <string>
#include <vector>
export module Assets.Dazzles;

namespace Assets {
export struct LensFlareSprite final {
    float location = 0;
    float size = 1;
    std::array<float, 3> color{1, 1, 1};
    std::array<float, 4> uv{0, 0, 1, 1};
};
export struct LensFlareDefinition final {
    std::string name;
    std::string texture;
    std::vector<LensFlareSprite> sprites;
};
export struct DazzleDefinition final {
    std::string name;
    std::string primary_texture;
    std::string halo_texture;
    std::string lens_flare;
    float halo_intensity = .95f;
    float halo_intensity_power = 0;
    std::array<float, 2> halo_scale{2, 2};
    float area = .05f;
    float direction_area = .5f;
    float intensity = .9f;
    float intensity_power = .9f;
    float size_power = .9f;
    std::array<float, 2> scale{100, 25};
    float fade_start = 25;
    float fade_end = 50;
    float history_weight = .5f;
    bool use_camera_translation = true;
    std::array<float, 3> direction{};
    std::array<float, 3> color{1, 1, 1};
    std::array<float, 3> halo_color{1, 1, 1};
    float radius = 1;
    float blink_period = 0;
    float blink_on_time = 0;
};
export struct DazzleDefinitions final {
    std::vector<LensFlareDefinition> lens_flares;
    std::vector<DazzleDefinition> dazzles;
};
}
