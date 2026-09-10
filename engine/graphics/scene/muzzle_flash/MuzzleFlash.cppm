module;
#include <array>
#include <cmath>
#include <cstdint>

export module Graphics.Scene.MuzzleFlash;
export import Graphics.Scene.Props.Renderer;

namespace Graphics
{
// The designation is stored on a render-object instance. It deliberately does
// not encode material or vertex data: the model adapter owns identification,
// while this component owns the effect's draw policy.
export enum class MuzzleFlashDesignation : std::uint8_t
{
    None,
    Rotating,
};

export struct MuzzleFlashRenderState final
{
    bool enabled = false;
    float angle = 0;
    std::array<float,2> pivot{0.5f,0.5f};
};

export inline constexpr float Muzzle_Flash_Angular_Speed = 0.75f;

export MuzzleFlashRenderState Evaluate_Muzzle_Flash(
    MuzzleFlashDesignation designation, std::uint32_t sync_time_ms) noexcept
{
    if (designation == MuzzleFlashDesignation::None) return {};
    constexpr float two_pi = 6.2831853071795864769f;
    const float elapsed = static_cast<float>(sync_time_ms) / 1000.0f;
    return {true,std::fmod(elapsed*Muzzle_Flash_Angular_Speed,two_pi),{0.5f,0.5f}};
}

// Apply the complete effect contract after the material adapter has translated
// the authored shader. The shader consumes the state for the two opposite UV
// rotations; the style keeps the flash additive while depth-tested and leaves
// the depth buffer unchanged.
export void Prepare_Muzzle_Flash(PropParameters& parameters, PropStyle& style,
    MuzzleFlashDesignation designation, std::uint32_t sync_time_ms) noexcept
{
    const auto state = Evaluate_Muzzle_Flash(designation,sync_time_ms);
    if (!state.enabled) return;
    parameters.muzzle_flash_state = {1.0f,state.angle,state.pivot[0],state.pivot[1]};
    parameters.alpha_cutoff = 0;
    if (parameters.fog_state[2] > 0.5f) parameters.fog_color = {0,0,0,1};
    style.blend = RHIBlendMode::Additive;
    style.source_blend = RHIBlendFactor::One;
    style.destination_blend = RHIBlendFactor::One;
    style.depth_test = true;
    style.depth_write = false;
    style.cull = RHICullMode::None;
}
}
