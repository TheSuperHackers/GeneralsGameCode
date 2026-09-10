module;
#define BOOST_TEST_MODULE MaterialStateTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Materials.State.Tests;
import Graphics.Materials.State;
import Graphics.Materials.Ordering;
import Graphics.Materials.W3DState;
import Graphics.Materials.Fog;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Props.MaterialDrawState;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Geometry;
import Graphics.Tests.Device;
import Graphics.RHI;
import Assets.Adapters.W3D.Materials;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(material_values_preserve_identity_and_independent_presets)
{
    constexpr MaterialState defaults;
    static_assert(defaults.Get_Bits() == 0x0008441b);
    static_assert(MaterialState::Opaque().Get_Bits() == 0x0009441b);
    static_assert(MaterialState::Additive().Get_Bits() == 0x00094433);
    static_assert(MaterialState::Alpha().Get_Bits() == 0x000984b3);
    static_assert(MaterialState::ATestSprite().Get_Bits() == 0x000d401b);
    auto changed = MaterialState::Opaque();
    changed.Set_Depth_Compare(MaterialState::PASS_GREATER);
    changed.Set_Texturing(MaterialState::TEXTURING_DISABLE);
    BOOST_CHECK_EQUAL(changed.Get_Bits(),0x0008441c);
    BOOST_CHECK_EQUAL(MaterialState::Opaque().Get_Bits(),0x0009441b);
    const auto copy = changed;
    BOOST_CHECK(copy == changed);
    changed.Reset();
    BOOST_CHECK(changed == defaults);
    MaterialState reserved{0x80000000};
    reserved.Set_Src_Blend_Func(MaterialState::SRCBLEND_SRC_ALPHA);
    reserved.Set_Dst_Blend_Func(MaterialState::DSTBLEND_ONE_MINUS_SRC_COLOR);
    BOOST_CHECK_EQUAL(reserved.Get_Bits(),0x80008060);
    BOOST_CHECK(reserved.Uses_Alpha());
}

BOOST_AUTO_TEST_CASE(authored_shader_record_keeps_draw_context_and_effective_detail_fields)
{
    const std::array<std::uint8_t,16> bytes{7,0,0,2,3,5,1,3,1,4,3,17,1,9,2,0};
    Assets::W3D::W3DShaderSettings record;
    BOOST_REQUIRE(Assets::W3D::W3DRead_Shader(std::as_bytes(std::span(bytes)),record));
    MaterialState state{0x80020000};
    Apply_W3D_Material_State(state,record);
    BOOST_CHECK_EQUAL(state.Get_Depth_Compare(),MaterialState::PASS_ALWAYS);
    BOOST_CHECK_EQUAL(state.Get_Depth_Mask(),MaterialState::DEPTH_WRITE_DISABLE);
    BOOST_CHECK_EQUAL(state.Get_Color_Mask(),MaterialState::COLOR_WRITE_ENABLE);
    BOOST_CHECK_EQUAL(state.Get_Fog_Func(),MaterialState::FOG_DISABLE);
    BOOST_CHECK_EQUAL(state.Get_Cull_Mode(),MaterialState::CULL_MODE_DISABLE);
    BOOST_CHECK_EQUAL(state.Get_NPatch_Enable(),MaterialState::NPATCH_ENABLE);
    BOOST_CHECK_EQUAL(state.Get_Post_Detail_Color_Func(),MaterialState::DETAILCOLOR_ADD);
    BOOST_CHECK_EQUAL(state.Get_Post_Detail_Alpha_Func(),MaterialState::DETAILALPHA_INVSCALE);
    BOOST_CHECK_EQUAL(state.Get_Bits() & 0x80000000,0x80000000);
    PropParameters parameters;
    SceneDrawParameters scene;
    scene.color_write_mask=7;
    scene.depth_bias=19;
    scene.wireframe=true;
    const auto style=Resolve_Prop_Material_State(state,scene,true,parameters);
    BOOST_CHECK(style.source_blend == RHIBlendFactor::InverseSourceAlpha);
    BOOST_CHECK(style.destination_blend == RHIBlendFactor::SourceColor);
    BOOST_CHECK(style.depth_comparison == RHIComparison::Always);
    BOOST_CHECK(!style.depth_write);
    BOOST_CHECK(!style.front_counter_clockwise);
    BOOST_CHECK_EQUAL(style.color_write_mask,7);
    BOOST_CHECK_EQUAL(style.depth_bias,19);
    BOOST_CHECK(style.wireframe);
    BOOST_CHECK_EQUAL(parameters.primary_gradient,5);
    BOOST_CHECK_EQUAL(parameters.secondary_gradient,1);
    BOOST_CHECK_EQUAL(parameters.detail_color,4);
    BOOST_CHECK_EQUAL(parameters.detail_alpha,3);
    BOOST_CHECK_SMALL(parameters.alpha_cutoff - 96.0f/255.0f,0.000001f);
    BOOST_CHECK_EQUAL(parameters.fog_state[2],0);
    BOOST_CHECK(!Assets::W3D::W3DRead_Shader(std::as_bytes(std::span(bytes)).first(15),record));
}

BOOST_AUTO_TEST_CASE(fog_selection_preserves_unsupported_pairs_and_neutral_colors)
{
    constexpr int expected[4][6] = {
        {-1,-1,3,-1,-1,-1}, {1,2,-1,2,-1,-1},
        {-1,-1,-1,-1,-1,1}, {-1,-1,-1,-1,1,-1}};
    for (unsigned initial : {0u,3u}) for (unsigned source=0;source<4;++source)
        for (unsigned destination=0;destination<6;++destination) {
            MaterialState state;
            state.Set_Src_Blend_Func(static_cast<MaterialState::SrcBlendFuncType>(source));
            state.Set_Dst_Blend_Func(static_cast<MaterialState::DstBlendFuncType>(destination));
            state.Set_Fog_Func(static_cast<MaterialState::FogFuncType>(initial));
            const auto expected_mode=expected[source][destination];
            BOOST_CHECK_EQUAL(state.Enable_Fog_For_Blend(),expected_mode>=0);
            BOOST_CHECK_EQUAL(state.Get_Fog_Func(),expected_mode>=0 ? expected_mode : initial);
        }
}

BOOST_AUTO_TEST_CASE(material_order_retains_alpha_test_and_game_layer_values)
{
    constexpr unsigned layers[2][4][6] = {
        {{0,20,20,20,20,20},{0,10,20,15,20,20},{0,20,20,20,20,20},{0,20,20,20,20,20}},
        {{0,20,20,20,20,20},{0,10,20,15,20,20},{0,20,20,20,20,0},{0,20,20,20,20,20}}};
    for (unsigned alpha=0;alpha<2;++alpha) for (unsigned source=0;source<4;++source)
        for (unsigned destination=0;destination<6;++destination) {
            MaterialState state;
            state.Set_Alpha_Test(static_cast<MaterialState::AlphaTestType>(alpha));
            state.Set_Src_Blend_Func(static_cast<MaterialState::SrcBlendFuncType>(source));
            state.Set_Dst_Blend_Func(static_cast<MaterialState::DstBlendFuncType>(destination));
            BOOST_CHECK_EQUAL(Material_Ordered_Layer(state),layers[alpha][source][destination]);
        }
}

BOOST_AUTO_TEST_CASE(draw_state_preserves_encoded_blends_alpha_cutoff_and_scene_write_mask)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    for (auto& vertex:vertices) vertex.color={0.8f,0.4f,0.2f,0.25f};
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    struct Case { unsigned source; unsigned destination; bool alpha_test; unsigned writes; std::array<int,4> expected; };
    // Literal expected RGBA follows source and destination equations separately.
    // Inverse source color uses inverse source alpha for the alpha channel.
    const Case cases[]{
        {2,2,false,15,{92,56,33,32}}, {2,3,false,15,{61,71,94,64}},
        {3,2,false,15,{194,107,59,64}}, {3,3,false,15,{163,122,120,96}},
        {1,0,true,15,{51,77,102,64}}, {1,0,false,7,{204,102,51,64}}};
    for (const auto& value:cases) {
        BOOST_REQUIRE(commands.Clear_Color_Target(target,{0.2f,0.3f,0.4f,0.25f}));
        MaterialState state;
        state.Set_Depth_Compare(MaterialState::PASS_ALWAYS);
        state.Set_Depth_Mask(MaterialState::DEPTH_WRITE_DISABLE);
        state.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
        state.Set_Src_Blend_Func(static_cast<MaterialState::SrcBlendFuncType>(value.source));
        state.Set_Dst_Blend_Func(static_cast<MaterialState::DstBlendFuncType>(value.destination));
        state.Set_Alpha_Test(value.alpha_test ? MaterialState::ALPHATEST_ENABLE : MaterialState::ALPHATEST_DISABLE);
        SceneDrawParameters scene;
        scene.color_write_mask=static_cast<std::uint8_t>(value.writes);
        PropParameters parameters;
        parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        parameters.textured=0;
        const auto style=Resolve_Prop_Material_State(state,scene,false,parameters);
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,{}));
        std::array<std::byte,8*8*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,32));
        for (const unsigned pixel : {2u*8+2,5u*8+5})
            for (unsigned channel=0;channel<4;++channel)
                BOOST_CHECK_SMALL(static_cast<float>(std::to_integer<int>(pixels[pixel*4+channel])-value.expected[channel]),2.0f);
    }
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
