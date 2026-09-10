module;
#include <cstdint>
#include <variant>
export module Assets.Materials.TextureMapping;
import Assets.Math;

namespace Assets {
export enum class TextureEnvironmentSource { None, Normal, Reflection };
export enum class TextureMappingAxis { X, Y, Z };
export struct TextureScaleMapping final { Vector2f scale{1,1}; };
export struct TextureScrollMapping final {
    Vector2f scale{1,1};
    Vector2f rate_per_second{};
    Vector2f start_offset{};
    bool clamp = false;
    bool screen_projection = false;
};
export struct TextureEnvironmentMapping final {
    TextureEnvironmentSource source = TextureEnvironmentSource::Normal;
    bool world_space = false;
    TextureMappingAxis axis = TextureMappingAxis::Z;
};
export struct TextureGridMapping final {
    float frames_per_second = 1;
    std::uint32_t width_log2 = 1;
    std::uint32_t last_frame = 0;
    std::uint32_t start_frame = 0;
    TextureEnvironmentMapping environment{TextureEnvironmentSource::None};
};
export struct TextureRotateMapping final {
    Vector2f scale{1,1};
    Vector2f center{};
    float turns_per_second = 0.1f;
};
export struct TextureSineMapping final {
    Vector2f scale{1,1};
    // Amplitude, frequency, and phase in half turns for each axis.
    Vector3f u{1,1,0};
    Vector3f v{1,1,0};
};
export struct TextureStepMapping final {
    Vector2f scale{1,1};
    Vector2f step{};
    float steps_per_second = 0;
    bool clamp = false;
};
export struct TextureZigZagMapping final {
    Vector2f scale{1,1};
    Vector2f rate_per_second{};
    float period_seconds = 0;
};
export struct TextureEdgeMapping final {
    float rate_per_second = 0;
    float start_offset = 0;
    bool reflection = false;
};
export struct TextureRandomMapping final {
    Vector2f scale{1,1};
    Vector2f rate_per_second{};
    float frames_per_second = 0;
};
export struct TextureBumpMapping final {
    TextureScrollMapping scroll;
    float turns_per_second = 0;
    float scale = 1;
};
// Store only the parameters used by the selected mapping. Mutable animation
// state, the clock, camera matrices, and random sampling belong to graphics.
export using TextureMappingDescription = std::variant<TextureScaleMapping,
    TextureScrollMapping, TextureEnvironmentMapping, TextureGridMapping,
    TextureRotateMapping, TextureSineMapping, TextureStepMapping,
    TextureZigZagMapping, TextureEdgeMapping, TextureRandomMapping, TextureBumpMapping>;
}
