/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

module;
#include <cstdint>
export module Graphics.Materials.State;

namespace Graphics {
// Compact material identity shared by model channels and draw preparation.
// This value owns no GPU resources, file parsing, or mutable global presets.
export class MaterialState final {
public:
    enum AlphaTestType { ALPHATEST_DISABLE=0, ALPHATEST_ENABLE };
    enum DepthCompareType { PASS_NEVER=0, PASS_LESS, PASS_EQUAL, PASS_LEQUAL, PASS_GREATER, PASS_NOTEQUAL, PASS_GEQUAL, PASS_ALWAYS };
    enum DepthMaskType { DEPTH_WRITE_DISABLE=0, DEPTH_WRITE_ENABLE };
    enum ColorMaskType { COLOR_WRITE_DISABLE=0, COLOR_WRITE_ENABLE };
    enum DetailAlphaFuncType { DETAILALPHA_DISABLE=0, DETAILALPHA_DETAIL, DETAILALPHA_SCALE, DETAILALPHA_INVSCALE };
    enum DetailColorFuncType { DETAILCOLOR_DISABLE=0, DETAILCOLOR_DETAIL, DETAILCOLOR_SCALE, DETAILCOLOR_INVSCALE, DETAILCOLOR_ADD, DETAILCOLOR_SUB, DETAILCOLOR_SUBR, DETAILCOLOR_BLEND, DETAILCOLOR_DETAILBLEND, DETAILCOLOR_ADDSIGNED, DETAILCOLOR_ADDSIGNED2X, DETAILCOLOR_SCALE2X, DETAILCOLOR_MODALPHAADDCOLOR };
    enum CullModeType { CULL_MODE_DISABLE=0, CULL_MODE_ENABLE };
    enum NPatchEnableType { NPATCH_DISABLE=0, NPATCH_ENABLE };
    enum DstBlendFuncType { DSTBLEND_ZERO=0, DSTBLEND_ONE, DSTBLEND_SRC_COLOR, DSTBLEND_ONE_MINUS_SRC_COLOR, DSTBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA, DSTBLEND_MAX };
    enum FogFuncType { FOG_DISABLE=0, FOG_ENABLE, FOG_SCALE_FRAGMENT, FOG_WHITE };
    enum PriGradientType { GRADIENT_DISABLE=0, GRADIENT_MODULATE, GRADIENT_ADD, GRADIENT_BUMPENVMAP, GRADIENT_BUMPENVMAPLUMINANCE, GRADIENT_MODULATE2X };
    enum SecGradientType { SECONDARY_GRADIENT_DISABLE=0, SECONDARY_GRADIENT_ENABLE };
    enum SrcBlendFuncType { SRCBLEND_ZERO=0, SRCBLEND_ONE, SRCBLEND_SRC_ALPHA, SRCBLEND_ONE_MINUS_SRC_ALPHA, SRCBLEND_MAX };
    enum TexturingType { TEXTURING_DISABLE=0, TEXTURING_ENABLE };
    enum StaticSortCategoryType { SSCAT_OPAQUE=0, SSCAT_ALPHA_TEST, SSCAT_ADDITIVE, SSCAT_SCREEN, SSCAT_OTHER };

    constexpr MaterialState() noexcept = default;
    constexpr MaterialState(std::uint32_t bits) noexcept : m_bits(bits) {}
    constexpr bool operator==(const MaterialState&) const noexcept = default;
    constexpr std::uint32_t Get_Bits() const noexcept { return m_bits; }
    constexpr void Reset() noexcept { m_bits = DefaultBits; }

    constexpr DepthCompareType Get_Depth_Compare() const noexcept { return Read<DepthCompareType,0,7>(); }
    constexpr void Set_Depth_Compare(DepthCompareType value) noexcept { Write<0,7>(value); }
    constexpr DepthMaskType Get_Depth_Mask() const noexcept { return Read<DepthMaskType,3,1>(); }
    constexpr void Set_Depth_Mask(DepthMaskType value) noexcept { Write<3,1>(value); }
    constexpr ColorMaskType Get_Color_Mask() const noexcept { return Read<ColorMaskType,4,1>(); }
    constexpr void Set_Color_Mask(ColorMaskType value) noexcept { Write<4,1>(value); }
    constexpr DstBlendFuncType Get_Dst_Blend_Func() const noexcept { return Read<DstBlendFuncType,5,7>(); }
    constexpr void Set_Dst_Blend_Func(DstBlendFuncType value) noexcept { Write<5,7>(value); }
    constexpr FogFuncType Get_Fog_Func() const noexcept { return Read<FogFuncType,8,3>(); }
    constexpr void Set_Fog_Func(FogFuncType value) noexcept { Write<8,3>(value); }
    constexpr PriGradientType Get_Primary_Gradient() const noexcept { return Read<PriGradientType,10,7>(); }
    constexpr void Set_Primary_Gradient(PriGradientType value) noexcept { Write<10,7>(value); }
    constexpr SecGradientType Get_Secondary_Gradient() const noexcept { return Read<SecGradientType,13,1>(); }
    constexpr void Set_Secondary_Gradient(SecGradientType value) noexcept { Write<13,1>(value); }
    constexpr SrcBlendFuncType Get_Src_Blend_Func() const noexcept { return Read<SrcBlendFuncType,14,3>(); }
    constexpr void Set_Src_Blend_Func(SrcBlendFuncType value) noexcept { Write<14,3>(value); }
    constexpr TexturingType Get_Texturing() const noexcept { return Read<TexturingType,16,1>(); }
    constexpr void Set_Texturing(TexturingType value) noexcept { Write<16,1>(value); }
    constexpr NPatchEnableType Get_NPatch_Enable() const noexcept { return Read<NPatchEnableType,17,1>(); }
    constexpr void Set_NPatch_Enable(NPatchEnableType value) noexcept { Write<17,1>(value); }
    constexpr AlphaTestType Get_Alpha_Test() const noexcept { return Read<AlphaTestType,18,1>(); }
    constexpr void Set_Alpha_Test(AlphaTestType value) noexcept { Write<18,1>(value); }
    constexpr CullModeType Get_Cull_Mode() const noexcept { return Read<CullModeType,19,1>(); }
    constexpr void Set_Cull_Mode(CullModeType value) noexcept { Write<19,1>(value); }
    constexpr DetailColorFuncType Get_Post_Detail_Color_Func() const noexcept { return Read<DetailColorFuncType,20,15>(); }
    constexpr void Set_Post_Detail_Color_Func(DetailColorFuncType value) noexcept { Write<20,15>(value); }
    constexpr DetailAlphaFuncType Get_Post_Detail_Alpha_Func() const noexcept { return Read<DetailAlphaFuncType,24,7>(); }
    constexpr void Set_Post_Detail_Alpha_Func(DetailAlphaFuncType value) noexcept { Write<24,7>(value); }

    constexpr bool Uses_Alpha() const noexcept {
        return Get_Alpha_Test() != ALPHATEST_DISABLE
            || Get_Dst_Blend_Func() == DSTBLEND_SRC_ALPHA
            || Get_Dst_Blend_Func() == DSTBLEND_ONE_MINUS_SRC_ALPHA
            || Get_Src_Blend_Func() == SRCBLEND_SRC_ALPHA
            || Get_Src_Blend_Func() == SRCBLEND_ONE_MINUS_SRC_ALPHA;
    }
    constexpr bool Uses_Fog() const noexcept { return Get_Fog_Func() != FOG_DISABLE; }
    constexpr bool Uses_Primary_Gradient() const noexcept { return Get_Primary_Gradient() != GRADIENT_DISABLE; }
    constexpr bool Uses_Secondary_Gradient() const noexcept { return Get_Secondary_Gradient() != SECONDARY_GRADIENT_DISABLE; }
    constexpr bool Uses_Texture() const noexcept { return Get_Texturing() != TEXTURING_DISABLE; }
    constexpr bool Uses_Post_Detail_Texture() const noexcept {
        return Uses_Texture() && (Get_Post_Detail_Color_Func() != DETAILCOLOR_DISABLE
            || Get_Post_Detail_Alpha_Func() != DETAILALPHA_DISABLE);
    }

    // Unsupported blend pairs retain their existing fog selection.
    constexpr bool Enable_Fog_For_Blend() noexcept {
        const auto source = Get_Src_Blend_Func();
        const auto destination = Get_Dst_Blend_Func();
        if (source == SRCBLEND_ZERO && destination == DSTBLEND_SRC_COLOR)
            Set_Fog_Func(FOG_WHITE);
        else if ((source == SRCBLEND_ONE && destination == DSTBLEND_ZERO)
            || (source == SRCBLEND_SRC_ALPHA && destination == DSTBLEND_ONE_MINUS_SRC_ALPHA)
            || (source == SRCBLEND_ONE_MINUS_SRC_ALPHA && destination == DSTBLEND_SRC_ALPHA))
            Set_Fog_Func(FOG_ENABLE);
        else if (source == SRCBLEND_ONE && (destination == DSTBLEND_ONE || destination == DSTBLEND_ONE_MINUS_SRC_COLOR))
            Set_Fog_Func(FOG_SCALE_FRAGMENT);
        else return false;
        return true;
    }

    static constexpr std::uint32_t Make_Bits(DepthCompareType depth, DepthMaskType depth_write,
        ColorMaskType color_write, SrcBlendFuncType source, DstBlendFuncType destination,
        FogFuncType fog, PriGradientType primary, SecGradientType secondary, TexturingType texture,
        AlphaTestType alpha_test, CullModeType cull, DetailColorFuncType detail_color,
        DetailAlphaFuncType detail_alpha) noexcept {
        return static_cast<std::uint32_t>(depth) | (static_cast<std::uint32_t>(depth_write) << 3)
            | (static_cast<std::uint32_t>(color_write) << 4) | (static_cast<std::uint32_t>(destination) << 5)
            | (static_cast<std::uint32_t>(fog) << 8) | (static_cast<std::uint32_t>(primary) << 10)
            | (static_cast<std::uint32_t>(secondary) << 13) | (static_cast<std::uint32_t>(source) << 14)
            | (static_cast<std::uint32_t>(texture) << 16) | (static_cast<std::uint32_t>(alpha_test) << 18)
            | (static_cast<std::uint32_t>(cull) << 19) | (static_cast<std::uint32_t>(detail_color) << 20)
            | (static_cast<std::uint32_t>(detail_alpha) << 24);
    }

    static constexpr MaterialState Opaque() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_ENABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ZERO,
            FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState Additive() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ONE,
            FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState Bumpenvmap() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ONE,
            FOG_DISABLE, GRADIENT_BUMPENVMAP, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_ADD, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState Alpha() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA,
            FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState Multiplicative() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ZERO, DSTBLEND_SRC_COLOR,
            FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState Opaque2D() noexcept {
        return Make_Bits(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ZERO,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState OpaqueSprite() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ZERO,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState Additive2D() noexcept {
        return Make_Bits(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ONE,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState Alpha2D() noexcept {
        return Make_Bits(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState AdditiveSprite() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ONE,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState AlphaSprite() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState OpaqueSolid() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_ENABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ZERO,
            FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_DISABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState AdditiveSolid() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ONE,
            FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_DISABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState AlphaSolid() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA,
            FOG_DISABLE, GRADIENT_MODULATE, SECONDARY_GRADIENT_DISABLE, TEXTURING_DISABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState ATest2D() noexcept {
        return Make_Bits(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ZERO,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_ENABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState ATestSprite() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_ENABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ZERO,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_ENABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState ATestBlend2D() noexcept {
        return Make_Bits(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_ENABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState ATestBlendSprite() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_ENABLE, COLOR_WRITE_ENABLE, SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_ENABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState Screen2D() noexcept {
        return Make_Bits(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ONE_MINUS_SRC_COLOR,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState ScreenSprite() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ONE, DSTBLEND_ONE_MINUS_SRC_COLOR,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState Multiplicative2D() noexcept {
        return Make_Bits(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ZERO, DSTBLEND_SRC_COLOR,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }
    static constexpr MaterialState MultiplicativeSprite() noexcept {
        return Make_Bits(PASS_LEQUAL, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, SRCBLEND_ZERO, DSTBLEND_SRC_COLOR,
            FOG_DISABLE, GRADIENT_DISABLE, SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE);
    }

private:
    template<class Value, unsigned Shift, std::uint32_t Mask>
    constexpr Value Read() const noexcept { return static_cast<Value>((m_bits >> Shift) & Mask); }
    template<unsigned Shift, std::uint32_t Mask, class Value>
    constexpr void Write(Value value) noexcept {
        m_bits = (m_bits & ~(Mask << Shift)) | (static_cast<std::uint32_t>(value) << Shift);
    }
    static constexpr std::uint32_t DefaultBits = 0x0008441b;
    std::uint32_t m_bits = DefaultBits;
};
static_assert(sizeof(MaterialState) == sizeof(std::uint32_t));
}
