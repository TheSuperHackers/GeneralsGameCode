module;
#include <array>
export module Graphics.Scene.Props.MaterialDrawState;
import Graphics.Materials.State;
import Graphics.Materials.Fog;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Props.Renderer;
import Graphics.RHI;

namespace Graphics {
// Material policy resolves into generic draw state before GPU submission.
// Scene inputs are copied now so deferred draws keep their original context.
export PropStyle Resolve_Prop_Material_State(const MaterialState& material,
    const SceneDrawParameters& scene, bool reflected, PropParameters& parameters) noexcept
{
    using S = MaterialState;
    parameters.primary_gradient = static_cast<float>(material.Get_Primary_Gradient());
    parameters.secondary_gradient = static_cast<float>(material.Get_Secondary_Gradient());
    parameters.detail_color = static_cast<float>(material.Get_Post_Detail_Color_Func());
    parameters.detail_alpha = static_cast<float>(material.Get_Post_Detail_Alpha_Func());
    parameters.alpha_cutoff = material.Get_Alpha_Test() == S::ALPHATEST_ENABLE ? 96.0f / 255.0f : 0;
    constexpr std::array fog_modes{MaterialFogMode::Disabled, MaterialFogMode::Scene,
        MaterialFogMode::Black, MaterialFogMode::White};
    const auto fog = Resolve_Material_Fog(scene.fog, fog_modes[material.Get_Fog_Func()]);
    parameters.fog_state = fog.state;
    parameters.fog_color = fog.color;
    PropStyle style;
    style.depth_write = material.Get_Depth_Mask() == S::DEPTH_WRITE_ENABLE;
    constexpr std::array comparisons{RHIComparison::Never, RHIComparison::Less,
        RHIComparison::Equal, RHIComparison::LessEqual, RHIComparison::Greater,
        RHIComparison::NotEqual, RHIComparison::GreaterEqual, RHIComparison::Always};
    style.depth_comparison = comparisons[material.Get_Depth_Compare()];
    // Source and destination values have different encodings for 2 and 3.
    constexpr std::array sources{RHIBlendFactor::Zero, RHIBlendFactor::One,
        RHIBlendFactor::SourceAlpha, RHIBlendFactor::InverseSourceAlpha};
    constexpr std::array destinations{RHIBlendFactor::Zero, RHIBlendFactor::One,
        RHIBlendFactor::SourceColor, RHIBlendFactor::InverseSourceColor,
        RHIBlendFactor::SourceAlpha, RHIBlendFactor::InverseSourceAlpha};
    style.source_blend = sources[material.Get_Src_Blend_Func()];
    style.destination_blend = destinations[material.Get_Dst_Blend_Func()];
    style.cull = material.Get_Cull_Mode() == S::CULL_MODE_ENABLE ? RHICullMode::Back : RHICullMode::None;
    style.front_counter_clockwise = !reflected;
    style.color_write_mask = material.Get_Color_Mask() == S::COLOR_WRITE_ENABLE ? 15 : 0;
    return Resolve_Prop_Style(style, scene);
}
}
