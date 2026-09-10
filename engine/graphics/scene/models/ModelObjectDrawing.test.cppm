module;
#define BOOST_TEST_MODULE ModelObjectDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>
export module Graphics.Scene.Models.ObjectDrawing.Tests;
import Graphics.RHI;
import Graphics.Tests.Device;
import Graphics.Materials.State;
import Graphics.Materials.MeshMaterial;
import Graphics.Scene.Models.ObjectDrawing;
import Graphics.Scene.Models.MeshMaterialBindings;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Resources.Textures.Sampling;
using namespace Graphics;
BOOST_AUTO_TEST_CASE(object_snapshot_retains_corner_materials_geometry_and_texture_after_callers_release) {
    for (bool warp : {true,false}) {
        GraphicsTestDevice device({warp}); if (!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer; BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const auto target=device.Create_Texture({32,32,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth=device.Create_Texture({32,32,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        auto& commands=device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth)); BOOST_REQUIRE(commands.Set_Viewport({0,0,32,32}));
        using Owner=std::shared_ptr<RHITextureHandle>;
        const std::array<std::uint8_t,4> pixel{255,255,255,255};
        const auto image=device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(pixel)),4});
        BOOST_REQUIRE(image.Is_Valid());
        auto texture=Owner(new RHITextureHandle(image),[&](auto* value) { device.Destroy_Texture(*value); delete value; });
        ModelObjectDrawing<Owner> drawing(renderer);
        constexpr std::array<float,16> identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        {
            MeshMaterialBindings<Owner,std::array<float,2>> bindings; bindings.Reset(2,6,1);
            auto shader=MaterialState::Opaque(); shader.Set_Cull_Mode(MaterialState::CULL_MODE_DISABLE);
            shader.Set_Texturing(MaterialState::TEXTURING_ENABLE); bindings.Set_Single_Shader(shader);
            bindings.Set_Single_Texture(texture);
            auto red=std::make_shared<MeshMaterial>(), green=std::make_shared<MeshMaterial>();
            red->parameters.diffuse={1,0,0}; green->parameters.diffuse={0,1,0};
            for(unsigned i=0;i<6;++i) bindings.Set_Material(i,i<3 ? red : green);
            using Position=std::array<float,3>; using Triangle=std::array<std::uint32_t,3>;
            const std::array<Position,6> positions{{{-.8f,-.8f,.5f},{.8f,-.8f,.5f},{.8f,.8f,.5f},
                {-.8f,-.8f,.5f},{.8f,.8f,.5f},{-.8f,.8f,.5f}}};
            const std::array<Triangle,2> triangles{{{0,1,2},{3,4,5}}};
            BOOST_REQUIRE(drawing.Append(std::span<const Position>(positions),std::span<const Position>{},
                std::span<const Triangle>(triangles),bindings,identity,false));
        }
        texture.reset();
        BOOST_REQUIRE(device.Retain_Texture(image)); device.Destroy_Texture(image);
        ModelObjectDrawContext context; context.view_projection=context.view=context.projection=identity;
        BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
        BOOST_REQUIRE(drawing.Draw(commands,context,[](const RHITextureHandle* handle) { return PropMaterialTexture{*handle,{}}; }));
        std::vector<std::byte> pixels(32*32*4); BOOST_REQUIRE(device.Readback_Texture(target,pixels,32*4));
        const auto check=[&](unsigned x,unsigned y,std::array<int,4> expected) {
            for(unsigned c=0;c<4;++c) BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*32+x)*4+c])-expected[c],2);
        };
        check(24,22,{255,0,0,255}); check(8,10,{0,255,0,255}); check(1,1,{0,0,0,0});
        drawing.Clear(); BOOST_CHECK(!device.Retain_Texture(image));
        renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
    }
}
