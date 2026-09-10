export module Graphics.Materials.W3DState;
import Graphics.Materials.State;
import Assets.Adapters.W3D.Materials;

namespace Graphics {
// Asset decoding supplies a bounded record. Color writes start enabled and fog
// starts disabled; their source bytes are not effective draw state. Preserve
// culling and unrelated packed fields in the receiving material.
export void Apply_W3D_Material_State(MaterialState& state, const Assets::W3D::W3DShaderSettings& record) noexcept
{
    using S = MaterialState;
    state.Set_Depth_Compare(static_cast<S::DepthCompareType>(record.depth_compare));
    state.Set_Depth_Mask(static_cast<S::DepthMaskType>(record.depth_mask));
    state.Set_Color_Mask(S::COLOR_WRITE_ENABLE);
    state.Set_Dst_Blend_Func(static_cast<S::DstBlendFuncType>(record.destination_blend));
    state.Set_Fog_Func(S::FOG_DISABLE);
    state.Set_Primary_Gradient(static_cast<S::PriGradientType>(record.primary_gradient));
    state.Set_Secondary_Gradient(static_cast<S::SecGradientType>(record.secondary_gradient));
    state.Set_Src_Blend_Func(static_cast<S::SrcBlendFuncType>(record.source_blend));
    state.Set_Texturing(static_cast<S::TexturingType>(record.texturing));
    state.Set_Alpha_Test(static_cast<S::AlphaTestType>(record.alpha_test));
    // Effective detail state comes from the authored detail fields; raw
    // post-detail bytes belong to the source format and do not override them.
    state.Set_Post_Detail_Color_Func(static_cast<S::DetailColorFuncType>(record.detail_color));
    state.Set_Post_Detail_Alpha_Func(static_cast<S::DetailAlphaFuncType>(record.detail_alpha));
}
}
