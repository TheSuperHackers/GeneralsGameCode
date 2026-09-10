module;
#define BOOST_TEST_MODULE SceneDrawParametersTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>
export module Graphics.Scene.DrawParameters.Tests;
import Graphics.Scene.DrawParameters;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.MaterialPassQueue;
import Graphics.Tests.Device;
using namespace Graphics;

namespace
{
struct Drawing final
{
    GraphicsTestDevice device;
    PropRenderer renderer;
    RHITextureHandle target, depth;
    std::vector<PropMeshHandle> meshes;
    static constexpr unsigned size = 32;

    explicit Drawing(bool software) : device({software})
    {
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        target = device.Create_Texture({size,size,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        depth = device.Create_Texture({size,size,1,RHITextureFormat::D24_UNorm_S8,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(target.Is_Valid());
        BOOST_REQUIRE(depth.Is_Valid());
        BOOST_REQUIRE(Commands().Set_Render_Targets(target,depth));
        BOOST_REQUIRE(Commands().Set_Viewport({0,0,size,size}));
        Clear();
    }
    ~Drawing()
    {
        for (auto mesh : meshes) renderer.Destroy_Mesh(mesh);
        renderer.Shutdown();
        device.Destroy_Texture(target);
        device.Destroy_Texture(depth);
    }
    CommandList& Commands() { return device.Immediate_Command_List(); }
    void Clear()
    {
        BOOST_REQUIRE(Commands().Clear_Color_Target(target,{0,0,0,0}));
        BOOST_REQUIRE(Commands().Clear_Depth_Stencil_Target(depth,1,0));
    }
    PropMeshHandle Quad(std::array<float,4> color, float left = -1, float right = 1, bool reverse = false)
    {
        std::array<PropVertex,4> vertices{};
        vertices[0].position = {left,-1,0.5f}; vertices[1].position = {right,-1,0.5f};
        vertices[2].position = {right,1,0.5f}; vertices[3].position = {left,1,0.5f};
        for (auto& vertex : vertices) vertex.color = color;
        std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
        if (reverse) std::reverse(indices.begin(),indices.end());
        const auto mesh = renderer.Create_Mesh(vertices,indices);
        BOOST_REQUIRE(mesh.Is_Valid());
        meshes.push_back(mesh);
        return mesh;
    }
    PropParameters Parameters() const
    {
        PropParameters parameters;
        parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        parameters.textured = 0;
        return parameters;
    }
    PropStyle Style() const
    {
        PropStyle style;
        style.blend = RHIBlendMode::Disabled;
        return style;
    }
    auto Pixels()
    {
        std::array<std::byte,size*size*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,size*4));
        return pixels;
    }
    void Pixel(unsigned x, unsigned y, std::array<int,4> expected)
    {
        const auto pixels = Pixels();
        for (unsigned channel = 0; channel < 4; ++channel)
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*size+x)*4+channel])-expected[channel],2);
    }
};
}

BOOST_AUTO_TEST_CASE(deferred_draws_retain_fog_and_channel_masks_after_scope_restoration)
{
    for (bool software : {true,false}) {
        Drawing draw(software);
        MaterialPassQueue queue;
        auto& scene = Get_Scene_Draw_Parameters();
        SceneDrawScope restore(scene);
        scene = {};
        const auto mesh = draw.Quad({0.8f,0.4f,0.2f,0.6f});
        {
            SceneDrawScope scope(scene);
            scene.fog = {true,0,1,{0.2f,0.6f,0.8f,1}};
            scene.color_write_mask = 7;
            auto authored = draw.Style();
            authored.color_write_mask = 13; // Material disables green; scene disables alpha.
            auto parameters = draw.Parameters();
            const auto fog = Resolve_Material_Fog(scene.fog,MaterialFogMode::Scene);
            parameters.fog_state = fog.state;
            parameters.fog_color = fog.color;
            BOOST_REQUIRE(queue.Submit(draw.renderer,mesh,Resolve_Prop_Style(authored,scene),parameters,{}));
        }
        BOOST_CHECK_EQUAL(scene.color_write_mask,15);
        BOOST_CHECK(!scene.fog.enabled);
        {
            SceneDrawScope scope(scene);
            scene.color_write_mask = 8;
            BOOST_REQUIRE(queue.Submit(draw.renderer,mesh,Resolve_Prop_Style(draw.Style(),scene),draw.Parameters(),{}));
        }
        scene.color_write_mask = 0;
        scene.wireframe = true;
        scene.fog = {true,0,0.1f,{1,0,1,1}};
        BOOST_REQUIRE(queue.Flush(draw.Commands()));
        draw.Pixel(9,16,{128,0,128,153});
    }
}

BOOST_AUTO_TEST_CASE(stencil_marker_masks_preserve_player_bits_and_both_triangle_faces)
{
    for (bool software : {true,false}) for (bool reverse : {false,true}) {
        Drawing draw(software);
        MaterialPassQueue queue;
        SceneDrawParameters scene;
        const auto whole = draw.Quad({0,1,0,0.5f},-1,1,reverse);
        const auto right = draw.Quad({1,0,0,1},0,1,reverse);
        scene.color_write_mask = 0;
        scene.stencil.enabled = true;
        scene.stencil.reference = 0x18;
        scene.stencil.front.pass = scene.stencil.back.pass = RHIStencilOperation::Replace;
        BOOST_REQUIRE(queue.Submit(draw.renderer,whole,Resolve_Prop_Style(draw.Style(),scene),draw.Parameters(),{}));
        scene.stencil.reference = 0xff;
        scene.stencil.write_mask = 0x80;
        BOOST_REQUIRE(queue.Submit(draw.renderer,right,Resolve_Prop_Style(draw.Style(),scene),draw.Parameters(),{}));
        scene.color_write_mask = 15;
        scene.stencil.reference = 0x98;
        scene.stencil.read_mask = 0xf8;
        scene.stencil.write_mask = 0;
        scene.stencil.front.comparison = scene.stencil.back.comparison = RHIComparison::Equal;
        BOOST_REQUIRE(queue.Submit(draw.renderer,whole,Resolve_Prop_Style(draw.Style(),scene),draw.Parameters(),{}));
        scene = {}; // Already queued descriptions must survive this reset.
        BOOST_REQUIRE(queue.Flush(draw.Commands()));
        draw.Pixel(8,16,{0,0,0,0});
        draw.Pixel(24,16,{0,255,0,128});

        draw.Clear();
        scene.stencil.enabled = true;
        scene.stencil.reference = 0x28;
        scene.stencil.front.comparison = scene.stencil.back.comparison = RHIComparison::Never;
        scene.stencil.front.fail = scene.stencil.back.fail = RHIStencilOperation::Replace;
        BOOST_REQUIRE(draw.renderer.Draw(draw.Commands(),whole,Resolve_Prop_Style(draw.Style(),scene),draw.Parameters(),{}));
        draw.Pixel(8,16,{0,0,0,0});
        scene.stencil.front.comparison = scene.stencil.back.comparison = RHIComparison::Equal;
        scene.stencil.front.fail = scene.stencil.back.fail = RHIStencilOperation::Keep;
        BOOST_REQUIRE(draw.renderer.Draw(draw.Commands(),whole,Resolve_Prop_Style(draw.Style(),scene),draw.Parameters(),{}));
        draw.Pixel(8,16,{0,255,0,128});
    }
}

BOOST_AUTO_TEST_CASE(nested_views_restore_wireframe_and_depth_bias_without_a_capacity_limit)
{
    for (bool software : {true,false}) {
        Drawing draw(software);
        SceneDrawParameters scene;
        const auto red = draw.Quad({1,0,0,1});
        const auto green = draw.Quad({0,1,0,1});
        const auto style = draw.Style();
        const auto parameters = draw.Parameters();
        const auto descend = [&](auto&& self, unsigned level) -> void {
            SceneDrawScope scope(scene);
            scene.depth_bias = static_cast<std::int32_t>(level);
            scene.wireframe = level % 2 != 0;
            if (level < 64) self(self,level+1);
            BOOST_CHECK_EQUAL(scene.depth_bias,level);
            BOOST_CHECK_EQUAL(scene.wireframe,level % 2 != 0);
        };
        descend(descend,1);
        BOOST_CHECK_EQUAL(scene.depth_bias,0);
        BOOST_CHECK(!scene.wireframe);
        BOOST_REQUIRE(draw.renderer.Draw(draw.Commands(),red,Resolve_Prop_Style(style,scene),parameters,{}));
        auto pixels = draw.Pixels();
        const auto coverage = [](const auto& values) {
            unsigned count = 0;
            for (unsigned i=0; i<values.size(); i+=4) if (values[i]!=std::byte{0}) ++count;
            return count;
        };
        const auto filled = coverage(pixels);
        draw.Clear();
        {
            SceneDrawScope scope(scene);
            scene.wireframe = true;
            BOOST_REQUIRE(draw.renderer.Draw(draw.Commands(),red,Resolve_Prop_Style(style,scene),parameters,{}));
        }
        pixels = draw.Pixels();
        BOOST_CHECK_GT(coverage(pixels),0);
        BOOST_CHECK_LT(coverage(pixels),filled/2);
        for (int bias : {0,16,-16}) {
            draw.Clear();
            BOOST_REQUIRE(draw.renderer.Draw(draw.Commands(),red,Resolve_Prop_Style(style,scene),parameters,{}));
            SceneDrawScope scope(scene);
            scene.depth_bias = bias;
            auto overlay = style;
            overlay.depth_comparison = RHIComparison::Less;
            BOOST_REQUIRE(draw.renderer.Draw(draw.Commands(),green,Resolve_Prop_Style(overlay,scene),parameters,{}));
            draw.Pixel(9,16,bias < 0 ? std::array{0,255,0,255} : std::array{255,0,0,255});
        }
    }
}
