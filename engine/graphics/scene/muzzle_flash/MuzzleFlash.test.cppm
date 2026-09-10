module;
#define BOOST_TEST_MODULE MuzzleFlashTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>

export module Graphics.Scene.MuzzleFlash.Tests;
import Graphics.Tests.Device;
import Graphics.RHI;
import Graphics.Scene.MuzzleFlash;
import Graphics.Scene.Props.Renderer;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(muzzle_flash_policy_is_time_driven_and_keeps_the_draw_contract)
{
    const auto disabled = Evaluate_Muzzle_Flash(MuzzleFlashDesignation::None,1000);
    BOOST_CHECK(!disabled.enabled);

    const auto at_zero = Evaluate_Muzzle_Flash(MuzzleFlashDesignation::Rotating,0);
    const auto at_second = Evaluate_Muzzle_Flash(MuzzleFlashDesignation::Rotating,1000);
    BOOST_REQUIRE(at_zero.enabled);
    BOOST_REQUIRE(at_second.enabled);
    BOOST_CHECK_SMALL(at_zero.angle,0.00001f);
    BOOST_CHECK_SMALL(at_second.angle-Muzzle_Flash_Angular_Speed,0.00001f);
    BOOST_CHECK_SMALL(at_second.pivot[0]-0.5f,0.00001f);
    BOOST_CHECK_SMALL(at_second.pivot[1]-0.5f,0.00001f);

    PropParameters parameters;
    parameters.fog_state={0,1,1,0};
    parameters.fog_color={0.4f,0.5f,0.6f,1};
    PropStyle style;
    Prepare_Muzzle_Flash(parameters,style,MuzzleFlashDesignation::Rotating,1000);
    BOOST_CHECK_EQUAL(parameters.muzzle_flash_state[0],1.0f);
    BOOST_CHECK_SMALL(parameters.muzzle_flash_state[1]-Muzzle_Flash_Angular_Speed,0.00001f);
    BOOST_CHECK_EQUAL(parameters.alpha_cutoff,0.0f);
    BOOST_CHECK(style.blend==RHIBlendMode::Additive);
    BOOST_CHECK(style.source_blend==RHIBlendFactor::One);
    BOOST_CHECK(style.destination_blend==RHIBlendFactor::One);
    BOOST_CHECK(!style.depth_write);
    BOOST_CHECK(style.depth_test);
    BOOST_CHECK(style.cull==RHICullMode::None);
    BOOST_CHECK_EQUAL(parameters.fog_color[0],0.0f);
    BOOST_CHECK_EQUAL(parameters.fog_color[1],0.0f);
    BOOST_CHECK_EQUAL(parameters.fog_color[2],0.0f);
}

BOOST_AUTO_TEST_CASE(muzzle_flash_draw_is_additive_depth_tested_and_unlit)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,
        Graphics::Test_Shader_Directory(GRAPHICS_MUZZLE_FLASH_SHADER_DIRECTORY)));

    const auto target=device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    std::array<std::uint8_t,4*4*4> pattern{};
    for (unsigned y=0;y<4;++y) for (unsigned x=0;x<4;++x) {
        const unsigned offset=(y*4+x)*4;
        if (x<2) pattern[offset]=255;
        else pattern[offset+1]=255;
        pattern[offset+3]=255;
    }
    const auto texture=device.Create_Texture_Initialized({4,4,1,RHITextureFormat::RGBA8_UNorm},
        {std::as_bytes(std::span(pattern)),4*4});
    const std::array<std::uint8_t,4> zero_alpha_texel{128,192,64,0};
    const auto zero_alpha_texture=device.Create_Texture_Initialized(
        {1,1,1,RHITextureFormat::RGBA8_UNorm},
        {std::as_bytes(std::span(zero_alpha_texel)),zero_alpha_texel.size()});
    BOOST_REQUIRE(target.Is_Valid());
    BOOST_REQUIRE(depth.Is_Valid());
    BOOST_REQUIRE(texture.Is_Valid());
    BOOST_REQUIRE(zero_alpha_texture.Is_Valid());

    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.25f}; vertices[1].position={1,-1,0.25f};
    vertices[2].position={1,1,0.25f}; vertices[3].position={-1,1,0.25f};
    vertices[0].uv={0,0}; vertices[1].uv={1,0};
    vertices[2].uv={1,1}; vertices[3].uv={0,1};
    for (auto& vertex : vertices) vertex.secondary_uv=vertex.uv;
    for (auto& vertex : vertices) {
        vertex.color={1,1,1,1};
        vertex.material_diffuse={1,1,1,1};
    }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());

    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.camera_position={0,0,10,1};
    parameters.textured=1; parameters.secondary_texture=1;
    PropStyle style;
    for (auto& sampler : style.samplers) {
        sampler.Set_Filter(RHISamplerFilter::Point);
        sampler.address.fill(RHISamplerAddress::Clamp);
    }
    Prepare_Muzzle_Flash(parameters,style,MuzzleFlashDesignation::Rotating,0);
    const std::array<RHITextureHandle,2> textures{texture,texture};
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,32,32}));
    const auto read_color = [&](unsigned x,unsigned y) {
        std::array<std::byte,32*32*4> pixels{};
        BOOST_CHECK(device.Readback_Texture(target,pixels,32*4));
        std::array<int,4> result{};
        const auto offset=(y*32+x)*4;
        for (unsigned channel=0;channel<4;++channel)
            result[channel]=std::to_integer<int>(pixels[offset+channel]);
        return result;
    };
    const auto read_depth = [&]() {
        std::array<float,32*32> values{};
        BOOST_CHECK(device.Readback_Texture(depth,std::as_writable_bytes(std::span(values)),32*sizeof(float)));
        return values[(12*32)+24];
    };

    // At zero rotation the probe is in the green half of the texture. At a
    // right angle the two opposite samples land in red and green, so their
    // per-channel product is black. This independently exercises both UVs.
    BOOST_REQUIRE(commands.Clear({0,0,0,1},0.5f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    const auto zero_angle=read_color(24,12);
    BOOST_CHECK_SMALL(zero_angle[0],2);
    BOOST_CHECK_SMALL(zero_angle[1]-255,2);
    BOOST_CHECK_SMALL(zero_angle[2],2);
    BOOST_CHECK_EQUAL(zero_angle[3],255);
    BOOST_CHECK_SMALL(read_depth()-0.5f,0.01f);

    parameters.muzzle_flash_state[1]=1.57079632679f;
    BOOST_REQUIRE(commands.Clear({0,0,0,1},0.5f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    const auto right_angle=read_color(24,12);
    BOOST_CHECK_SMALL(right_angle[0],2);
    BOOST_CHECK_SMALL(right_angle[1],2);
    BOOST_CHECK_SMALL(right_angle[2],2);
    BOOST_CHECK_EQUAL(right_angle[3],255);

    // A noncentral pivot changes the same probe back to the red half. The
    // pivot is explicit state, never inferred from vertex alpha.
    parameters.muzzle_flash_state[2]=0.25f;
    BOOST_REQUIRE(commands.Clear({0,0,0,1},0.5f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,textures));
    const auto pivoted=read_color(24,12);
    BOOST_CHECK_SMALL(pivoted[0]-255,2);
    BOOST_CHECK_SMALL(pivoted[1],2);
    BOOST_CHECK_SMALL(pivoted[2],2);
    BOOST_CHECK_EQUAL(pivoted[3],255);
    BOOST_CHECK_SMALL(read_depth()-0.5f,0.01f);

    // The zero-alpha texture and vertex alpha still contribute RGB under
    // One/One. The literal values also lock the separate vertex/material tint
    // equation and the duplicated texture sample.
    for (auto& vertex : vertices) {
        vertex.color={0.5f,0.25f,0.75f,0};
        vertex.material_diffuse={0.5f,0.75f,0.5f,0.5f};
        vertex.material_emissive={0,0,0,0};
    }
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    parameters.muzzle_flash_state={1,0,0.5f,0.5f};
    parameters.opacity=1;
    const std::array<RHITextureHandle,2> zero_alpha_textures{zero_alpha_texture,zero_alpha_texture};
    BOOST_REQUIRE(commands.Clear({0.1f,0.2f,0.3f,1},0.5f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,zero_alpha_textures));
    const auto visible=read_color(24,12);
    const std::array<int,4> expected_visible{42,78,83,255};
    for (unsigned channel=0;channel<4;++channel)
        BOOST_CHECK_SMALL(visible[channel]-expected_visible[channel],2);
    BOOST_CHECK_SMALL(read_depth()-0.5f,0.01f);

    // Depth rejects the flash while leaving the existing depth value intact.
    BOOST_REQUIRE(commands.Clear({0.1f,0.2f,0.3f,1},0.0f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,zero_alpha_textures));
    const auto occluded=read_color(24,12);
    const std::array<int,4> expected_occluded{26,51,77,255};
    for (unsigned channel=0;channel<4;++channel)
        BOOST_CHECK_SMALL(occluded[channel]-expected_occluded[channel],2);
    BOOST_CHECK_SMALL(read_depth(),0.01f);

    // Global opacity scales additive RGB explicitly, including zero-alpha
    // source art. The untagged path below retains its ordinary alpha contract.
    parameters.opacity=0;
    BOOST_REQUIRE(commands.Clear({0,0,0,1},0.5f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,zero_alpha_textures));
    const auto invisible=read_color(24,12);
    BOOST_CHECK_SMALL(invisible[0],2);
    BOOST_CHECK_SMALL(invisible[1],2);
    BOOST_CHECK_SMALL(invisible[2],2);
    parameters.opacity=0.5f;
    BOOST_REQUIRE(commands.Clear({0,0,0,1},0.5f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,zero_alpha_textures));
    const auto faded=read_color(24,12);
    const std::array<int,4> expected_faded{8,14,3,255};
    for (unsigned channel=0;channel<4;++channel)
        BOOST_CHECK_SMALL(faded[channel]-expected_faded[channel],2);

    // Emissive RGB remains visible when diffuse and vertex RGB are black.
    parameters.opacity=1;
    for (auto& vertex : vertices) {
        vertex.color={0,0,0,0};
        vertex.material_diffuse={0,0,0,0.5f};
        vertex.material_emissive={0.4f,0.2f,0.1f,0};
    }
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    BOOST_REQUIRE(commands.Clear({0,0,0,1},0.5f));
    BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,zero_alpha_textures));
    const auto emissive=read_color(24,12);
    const std::array<int,4> expected_emissive{26,29,2,255};
    for (unsigned channel=0;channel<4;++channel)
        BOOST_CHECK_SMALL(emissive[channel]-expected_emissive[channel],2);
    BOOST_CHECK_SMALL(read_depth()-0.5f,0.01f);

    // No designation leaves the regular single-stage shader active. Its RGB
    // is the authored texture, while global opacity affects only its alpha.
    for (auto& vertex : vertices) {
        vertex.color={1,1,1,1};
        vertex.material_diffuse={1,1,1,1};
        vertex.material_emissive={0,0,0,0};
    }
    BOOST_REQUIRE(renderer.Update_Mesh(mesh,vertices,indices));
    PropStyle ordinary_style=style;
    ordinary_style.blend=RHIBlendMode::Disabled;
    ordinary_style.source_blend=RHIBlendFactor::One;
    ordinary_style.destination_blend=RHIBlendFactor::Zero;
    parameters.muzzle_flash_state={};
    parameters.textured=1; parameters.secondary_texture=0;
    parameters.primary_gradient=0; parameters.opacity=0.5f;
    BOOST_REQUIRE(commands.Clear({0,0,0,0},0.5f));
    const std::array<RHITextureHandle,2> ordinary_textures{texture,{}};
    BOOST_REQUIRE(renderer.Draw(commands,mesh,ordinary_style,parameters,ordinary_textures));
    const auto ordinary=read_color(24,12);
    const std::array<int,4> expected_ordinary{0,255,0,128};
    for (unsigned channel=0;channel<4;++channel)
        BOOST_CHECK_SMALL(ordinary[channel]-expected_ordinary[channel],2);

    renderer.Destroy_Mesh(mesh);
    renderer.Shutdown();
    device.Destroy_Texture(zero_alpha_texture);
    device.Destroy_Texture(texture);
    device.Destroy_Texture(depth);
    device.Destroy_Texture(target);
}
