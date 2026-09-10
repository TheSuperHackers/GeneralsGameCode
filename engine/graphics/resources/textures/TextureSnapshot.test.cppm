module;
#define BOOST_TEST_MODULE TextureSnapshotTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>

export module Graphics.Resources.Textures.Snapshot.Tests;
import Graphics.Tests.Device;
import Graphics.Resources.Textures.Snapshot;
import Graphics.Scene.Props.Renderer;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(snapshot_sampling_preserves_depth_color_and_contents_across_resize)
{
    GraphicsTestDevice device({true});
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    auto& commands = device.Immediate_Command_List();
    const auto output = device.Create_Texture({8, 8, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    BOOST_REQUIRE(output.Is_Valid());
    const auto output_depth = device.Create_Texture({8, 8, 1, RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    BOOST_REQUIRE(output_depth.Is_Valid());
    std::array<PropVertex, 4> vertices{};
    vertices[0].position = {-1, -1, 0}; vertices[1].position = {1, -1, 0};
    vertices[2].position = {1, 1, 0}; vertices[3].position = {-1, 1, 0};
    for (auto& vertex : vertices) vertex.uv = {0.5f, 0.5f};
    const std::array<std::uint32_t, 6> indices{0, 1, 2, 0, 2, 3};
    const auto quad = renderer.Create_Mesh(vertices, indices);
    BOOST_REQUIRE(quad.Is_Valid());
    PropParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    style.depth_test = style.depth_write = false;
    TextureSnapshot snapshot;

    const auto check_pixels = [&](float expected) {
        BOOST_REQUIRE(commands.Set_Render_Targets(output, output_depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,8,8}));
        BOOST_REQUIRE(commands.Clear({0,1,0,1}, 1));
        const std::array<RHITextureHandle, 2> textures{snapshot.Texture(), {}};
        BOOST_REQUIRE(renderer.Draw(commands, quad, style, parameters, textures));
        std::array<std::byte, 8 * 8 * 4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(output, pixels, 8 * 4));
        for (std::size_t offset = 0; offset < pixels.size(); offset += 4) {
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset]) - static_cast<int>(std::lround(expected * 255)), 2);
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[offset + 1]), 0);
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[offset + 2]), 0);
            BOOST_CHECK_EQUAL(std::to_integer<int>(pixels[offset + 3]), 255);
        }
    };

    for (auto format : {RHITextureFormat::D24_UNorm_S8, RHITextureFormat::D32_Float,
        RHITextureFormat::BGRA8_UNorm, RHITextureFormat::RGBA8_UNorm}) {
        const bool depth = format == RHITextureFormat::D24_UNorm_S8 || format == RHITextureFormat::D32_Float;
        for (const std::uint32_t extent : {8u, 16u}) {
            const auto source = device.Create_Texture({extent, extent, 1, format,
                static_cast<std::uint32_t>(depth ? RHITextureUsage::DepthStencil : RHITextureUsage::RenderTarget)});
            BOOST_REQUIRE(source.Is_Valid());
            const auto source_depth = depth ? source : device.Create_Texture({extent, extent, 1,
                RHITextureFormat::D32_Float, static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(source_depth.Is_Valid());
            const auto clear_source = [&](float value) {
                if (depth) {
                    BOOST_REQUIRE(commands.Set_Depth_Target(source));
                    BOOST_REQUIRE(commands.Clear_Depth(value));
                } else {
                    BOOST_REQUIRE(commands.Set_Render_Targets(source, source_depth));
                    BOOST_REQUIRE(commands.Clear({value,0,0,1}, 1));
                }
            };
            clear_source(0.25f);
            BOOST_REQUIRE(snapshot.Capture(device, commands, source, extent, extent, format));
            const auto captured = snapshot.Texture();
            clear_source(0.75f);
            check_pixels(0.25f);
            BOOST_REQUIRE(snapshot.Capture(device, commands, source, extent, extent, format));
            BOOST_CHECK(snapshot.Texture() == captured);
            check_pixels(0.75f);
            BOOST_CHECK(!snapshot.Capture(device, commands, source, extent + 1, extent, format));
            BOOST_CHECK(snapshot.Texture() == captured);
            BOOST_REQUIRE(device.Destroy_Texture(source));
            if (!depth) device.Destroy_Texture(source_depth);
            check_pixels(0.75f);
        }
    }
    const auto captured = snapshot.Texture();
    snapshot.Shutdown();
    BOOST_CHECK(!snapshot.Texture().Is_Valid());
    BOOST_CHECK(!device.Destroy_Texture(captured));
    renderer.Destroy_Mesh(quad);
    renderer.Shutdown();
    device.Destroy_Texture(output);
    device.Destroy_Texture(output_depth);
}
