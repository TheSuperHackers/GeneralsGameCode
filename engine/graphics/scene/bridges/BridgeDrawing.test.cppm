module;
#define BOOST_TEST_MODULE BridgeDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
export module Graphics.Scene.Bridges.Drawing.Tests;
import Graphics.Tests.Device;
import Graphics.Scene.Bridges.Renderer;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(cutout_threshold_equal_depth_shroud_and_reflected_winding_preserve_coverage)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    SurfaceRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto texture = [&](std::array<std::uint8_t, 4> color) {
        return device.Create_Texture_Initialized({1, 1}, {std::as_bytes(std::span(color)), 4});
    };
    const auto below_cutoff = texture({255, 0, 0, 95});
    const auto at_cutoff = texture({255, 0, 0, 96});
    const auto shroud = texture({128, 128, 128, 255});
    const auto target = device.Create_Texture({16, 16, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({16, 16, 1, RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(below_cutoff.Is_Valid() && at_cutoff.Is_Valid() && shroud.Is_Valid()
        && target.Is_Valid() && depth.Is_Valid());
    std::array<SurfaceVertex, 4> vertices{};
    vertices[0].position = {-1, -1, 0.5f};
    vertices[1].position = {1, -1, 0.5f};
    vertices[2].position = {1, 1, 0.5f};
    vertices[3].position = {-1, 1, 0.5f};
    // This winding was drawn by the base pass but culled by the shroud pass.
    // Reflections reverse the projected winding below.
    const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
    const auto mesh = renderer.Create_Mesh(vertices, indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    SurfaceParameters parameters;
    parameters.view_projection = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    parameters.shroud = 1;
    auto &commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Set_Viewport({0, 0, 16, 16}));
    const auto check = [&](std::array<int, 4> expected) {
        std::array<std::byte, 16 * 16 * 4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, 16 * 4));
        for (std::size_t channel = 0; channel < 4; ++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(8 * 16 + 8) * 4 + channel]) - expected[channel], 2);
    };
    BOOST_REQUIRE(commands.Clear({0, 0, 1, 0.75f}, 1));
    std::array<BridgeDraw, 1> bridges{{{mesh, below_cutoff}}};
    BOOST_REQUIRE(Draw_Bridges(renderer, commands, bridges, parameters, {}, shroud));
    check({0, 0, 255, 191});
    bridges[0].texture = at_cutoff;
    BOOST_REQUIRE(Draw_Bridges(renderer, commands, bridges, parameters, {}, shroud));
    check({48, 0, 80, 191});
    BOOST_REQUIRE(commands.Clear({0, 0, 1, 0.75f}, 1));
    parameters.view_projection[0] = -1;
    BOOST_REQUIRE(Draw_Bridges(renderer, commands, bridges, parameters, {}, shroud));
    check({48, 0, 80, 191});
    // The accepted bridge fragments must have written depth.
    for (auto &vertex : vertices) vertex.position[2] = 0.75f;
    const auto behind = renderer.Create_Mesh(vertices, indices);
    SurfaceParameters solid = parameters;
    solid.textured = 0;
    solid.shroud = 0;
    SurfaceStyle opaque;
    opaque.blend = RHIBlendMode::Disabled;
    BOOST_REQUIRE(renderer.Draw(commands, behind, opaque, solid, {}));
    check({48, 0, 80, 191});
    BOOST_CHECK(renderer.Destroy_Mesh(mesh));
    BOOST_CHECK(renderer.Destroy_Mesh(behind));
    renderer.Shutdown();
    for (auto handle : {below_cutoff, at_cutoff, shroud, target, depth}) BOOST_CHECK(device.Destroy_Texture(handle));
}
