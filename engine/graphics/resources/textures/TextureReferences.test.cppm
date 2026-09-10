module;
#define BOOST_TEST_MODULE TextureReferencesTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <span>
export module Graphics.Resources.Textures.References.Tests;
import Graphics.Resources.Textures.References;
import Graphics.Renderer2D;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(shared_texture_versions_keep_their_pixels_until_the_resource_scope_ends)
{
    GraphicsTestDevice device({true});
    Renderer2D renderer;
    TextureReferences references;
    BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target = device.Create_Texture({8, 4, 1, RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth = device.Create_Texture({8, 4, 1, RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    const std::array<std::byte, 4> red{std::byte{255}, {}, {}, std::byte{128}};
    const std::array<std::byte, 4> green{std::byte{}, std::byte{255}, std::byte{}, std::byte{128}};
    const auto old_version = device.Create_Texture_Initialized({1, 1}, {red, 4});
    for (unsigned i = 0; i < 1000; ++i) BOOST_REQUIRE(references.Retain(device, old_version) == old_version);
    BOOST_CHECK_EQUAL(references.Size(), 1u);
    const auto replacement = device.Create_Texture_Initialized({1, 1}, {green, 4});
    BOOST_REQUIRE(references.Retain(device, replacement) == replacement);
    BOOST_REQUIRE(device.Destroy_Texture(old_version));
    BOOST_REQUIRE(device.Destroy_Texture(replacement));
    BOOST_CHECK_EQUAL(references.Size(), 2u);
    const auto old_image = renderer.Register_Texture(TextureHandle(1, 1), old_version);
    const auto new_image = renderer.Register_Texture(TextureHandle(2, 1), replacement);
    auto& commands = device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
    BOOST_REQUIRE(commands.Clear({0, 0, 1, 1}, 1));
    renderer.Begin(8, 4);
    BOOST_REQUIRE(renderer.Add_Quad(Rect2D{0, 0, 4, 4}, {0, 0, 1, 1}, old_image, Color2D{}));
    BOOST_REQUIRE(renderer.Add_Quad(Rect2D{4, 0, 8, 4}, {0, 0, 1, 1}, new_image, Color2D{}));
    BOOST_REQUIRE(renderer.Execute(device, commands, target, depth, {0, 0, 8, 4}));
    std::array<std::byte, 8 * 4 * 4> pixels{};
    BOOST_REQUIRE(device.Readback_Texture(target, pixels, 32));
    for (unsigned x = 0; x < 8; ++x) {
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[x * 4]) - (x < 4 ? 128 : 0), 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[x * 4 + 1]) - (x < 4 ? 0 : 128), 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[x * 4 + 2]) - 127, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[x * 4 + 3]) - 255, 2);
    }
    renderer.Shutdown();
    references.Clear();
    BOOST_CHECK_EQUAL(references.Size(), 0u);
    BOOST_CHECK(!device.Retain_Texture(old_version));
    BOOST_CHECK(!device.Retain_Texture(replacement));
    const auto next_version = device.Create_Texture_Initialized({1, 1}, {red, 4});
    BOOST_REQUIRE(next_version != old_version && next_version != replacement);
    BOOST_REQUIRE(references.Retain(device, next_version) == next_version);
    BOOST_REQUIRE(device.Destroy_Texture(next_version));
    references.Clear();
    BOOST_CHECK(!device.Retain_Texture(next_version));
    device.Destroy_Texture(target); device.Destroy_Texture(depth);
}

BOOST_AUTO_TEST_CASE(texture_reference_scopes_reject_cross_device_handles_and_allow_rebinding_after_clear)
{
    GraphicsTestDevice first({true}), second({true});
    const auto a = first.Create_Texture({1, 1});
    const auto b = second.Create_Texture({1, 1});
    BOOST_REQUIRE(a.Is_Valid()); BOOST_REQUIRE(b.Is_Valid());
    TextureReferences references;
    BOOST_CHECK(!references.Retain(first, {}).Is_Valid());
    BOOST_REQUIRE(references.Retain(first, a) == a);
    BOOST_CHECK(!references.Retain(second, b).Is_Valid());
    BOOST_REQUIRE(first.Destroy_Texture(a));
    references.Clear();
    BOOST_CHECK(!first.Retain_Texture(a));
    BOOST_REQUIRE(references.Retain(second, b) == b);
    BOOST_REQUIRE(second.Destroy_Texture(b));
    references.Clear();
    BOOST_CHECK(!second.Retain_Texture(b));
}
