module;
#include <cstdint>
export module Graphics.Materials.Ordering;
import Graphics.Materials.State;

namespace Graphics {
export constexpr MaterialState::StaticSortCategoryType Classify_Material_Order(const MaterialState& state) noexcept
{
    using S = MaterialState;
    if (state.Get_Dst_Blend_Func() == S::DSTBLEND_ZERO)
        return state.Get_Alpha_Test() == S::ALPHATEST_DISABLE ? S::SSCAT_OPAQUE : S::SSCAT_ALPHA_TEST;
    if (state.Get_Alpha_Test() == S::ALPHATEST_ENABLE
        && state.Get_Src_Blend_Func() == S::SRCBLEND_SRC_ALPHA
        && state.Get_Dst_Blend_Func() == S::DSTBLEND_ONE_MINUS_SRC_ALPHA)
        return S::SSCAT_ALPHA_TEST;
    if (state.Get_Src_Blend_Func() == S::SRCBLEND_ONE) {
        if (state.Get_Dst_Blend_Func() == S::DSTBLEND_ONE) return S::SSCAT_ADDITIVE;
        if (state.Get_Dst_Blend_Func() == S::DSTBLEND_ONE_MINUS_SRC_COLOR) return S::SSCAT_SCREEN;
    }
    return S::SSCAT_OTHER;
}

// Ordered layers are traversed from high to low. Keep alpha-tested geometry
// with the opaque layer, ahead of general transparency, screen and additive.
export constexpr std::uint32_t Material_Ordered_Layer(const MaterialState& state) noexcept
{
    switch (Classify_Material_Order(state)) {
    case MaterialState::SSCAT_OPAQUE:
    case MaterialState::SSCAT_ALPHA_TEST: return 0;
    case MaterialState::SSCAT_ADDITIVE: return 10;
    case MaterialState::SSCAT_SCREEN: return 15;
    default: return 20;
    }
}
}
