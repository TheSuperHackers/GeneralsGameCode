module;
#define BOOST_TEST_MODULE WNDTextureDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>
export module Engine.UI.WND.TextureDrawing.Tests;
import Engine.UI.WND;
import Graphics.Tests.Device;
using namespace Graphics;
using namespace Engine::UI::WND;

BOOST_AUTO_TEST_CASE(viewport_labels_preserve_placement_coverage_and_font_after_renderer_restart)
{
    FontFace font;
    Assets::FontGlyphAsset letter;
    letter.character = 'A';
    letter.width = 4;
    letter.spacing = 6;
    letter.alpha = {0,255,128,0, 0,255,128,0, 0,255,128,0, 0,255,128,0};
    Assets::FontGlyphAsset space;
    space.character = ' ';
    space.spacing = 3;
    BOOST_REQUIRE(font.Build(Assets::FontAsset("Label", 15, false, 4, 0, {letter, space})));
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    Renderer2D renderer;
    for (const unsigned size : {32u, 48u}) {
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const auto target = device.Create_Texture({size,size,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({size,size,1,RHITextureFormat::D32_Float,
            static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
        renderer.Begin(size,size);
        const std::uint16_t label[] = {'A',' ','A',0};
        TextStyle style;
        style.color = {0,1,0,175.0f/255};
        style.drop_color = {0,0,0,0};
        TextRenderer text;
        BOOST_REQUIRE(text.Draw(renderer,font,nullptr,label,4,4,{},style));
        style.color = {1,0,0,175.0f/255};
        BOOST_REQUIRE(text.Draw(renderer,font,nullptr,label,4,19,{},style));
        BOOST_REQUIRE(renderer.Execute(device,commands,target,depth,{0,0,size,size}));
        std::vector<std::byte> pixels(size*size*4);
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,size*4));
        const auto check = [&](unsigned x, unsigned y, std::array<int,3> expected) {
            for (unsigned channel=0;channel<3;++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*size+x)*4+channel])-expected[channel],2);
        };
        check(5,5,{0,175,80});
        check(6,5,{0,88,167});
        check(4,5,{0,0,255});
        check(10,5,{0,0,255});
        check(14,5,{0,175,80});
        check(5,9,{0,0,255});
        check(5,20,{175,0,80});
        renderer.Shutdown();
        BOOST_REQUIRE(device.Destroy_Texture(target));
        BOOST_REQUIRE(device.Destroy_Texture(depth));
    }
}

namespace {
struct Layers {
    ImageRef terrain, overlay, shroud;
    float width, height;
    static bool Extract(void *context, void *, void *, DrawList &list) noexcept
    {
        const auto &data = *static_cast<Layers *>(context);
        return list.Add_Image(data.terrain, {0, 0, data.width, data.height})
            && list.Add_Image(data.overlay, {data.width * 0.25f, data.height * 0.125f,
                data.width * 0.75f, data.height * 0.375f})
            && list.Add_Image(data.shroud, {0, 0, data.width, data.height})
            && list.Add_Gradient_Line({4, data.height - 4}, {data.width - 4, data.height - 4},
                2, {1, 1, 1, 1}, {0, 0, 1, 1});
    }
};
}

BOOST_AUTO_TEST_CASE(generated_map_layers_keep_orientation_alpha_updates_and_resize)
{
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    Renderer2D graphics;
    BOOST_REQUIRE(graphics.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const std::array<std::uint8_t, 8> terrain_pixels{255,0,0,255, 0,255,0,255};
    const std::array<std::uint8_t, 8> shroud_pixels{0,0,0,128, 0,0,0,0};
    const std::array<std::uint8_t, 4> marker_pixels{255,0,0,128};
    const auto terrain = device.Create_Texture_Initialized({1,2}, {std::as_bytes(std::span(terrain_pixels)),4});
    const auto shroud = device.Create_Texture_Initialized({2,1}, {std::as_bytes(std::span(shroud_pixels)),8});
    const auto overlay = device.Create_Texture_Initialized({1,1}, {std::as_bytes(std::span(marker_pixels)),4});
    Layers layers;
    layers.terrain.generated = graphics.Register_Texture(TextureHandle(100,1), terrain);
    layers.terrain.uv = {0,1,1,0};
    layers.overlay.generated = graphics.Register_Texture(TextureHandle(101,1), overlay);
    layers.shroud.generated = graphics.Register_Texture(TextureHandle(102,1), shroud);
    BOOST_REQUIRE(layers.terrain.generated.index.Is_Valid());
    BOOST_REQUIRE(layers.overlay.generated.index.Is_Valid());
    BOOST_REQUIRE(layers.shroud.generated.index.Is_Valid());
    for (const unsigned size : {64u,96u}) {
        layers.width = layers.height = float(size);
        if (size == 96) {
            const std::array<std::uint8_t,4> updated{0,0,255,128};
            BOOST_REQUIRE(device.Update_Texture(overlay, {std::as_bytes(std::span(updated)),4}));
        }
        const auto target = device.Create_Texture({size,size,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({size,size,1,RHITextureFormat::D32_Float,
            static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        auto &commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,size,size}));
        BOOST_REQUIRE(commands.Clear({0,0,0,1},1));
        graphics.Begin(size,size);
        RenderList nodes;
        RenderNode node;
        node.extract = &Layers::Extract;
        node.extract_context = &layers;
        NodeIndex index;
        BOOST_REQUIRE(nodes.Add_Node(node,index));
        nodes.Set_Roots(index,index);
        Renderer wnd;
        BOOST_REQUIRE(wnd.Render(nodes,graphics));
        BOOST_REQUIRE(graphics.Execute(device,commands,target,depth,{0,0,size,size}));
        std::vector<std::byte> pixels(size*size*4);
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,size*4));
        const auto check = [&](unsigned x, unsigned y, std::array<int,3> expected) {
            for (unsigned c=0;c<3;++c)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*size+x)*4+c])-expected[c],3);
        };
        // The generated terrain has reversed V, just like the game radar.
        check(size/8,size/8,{0,127,0});
        check(size*7/8,size/8,{0,255,0});
        check(size*7/8,size*7/8,{255,0,0});
        // The view outline is emitted after the shroud and stays bright.
        BOOST_CHECK_GT(std::to_integer<int>(pixels[((size-4)*size+6)*4+2]),250);
        // Updating the same texture changes the next frame's marker.
        const unsigned marker = (size*3/16*size+size*5/8)*4;
        BOOST_CHECK_GT(std::to_integer<int>(pixels[marker+(size==64?0:2)]),70);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[marker+(size==64?2:0)]),3);
        BOOST_REQUIRE(device.Destroy_Texture(target));
        BOOST_REQUIRE(device.Destroy_Texture(depth));
    }
    graphics.Shutdown();
    for (auto texture : {terrain,overlay,shroud}) BOOST_REQUIRE(device.Destroy_Texture(texture));
}
