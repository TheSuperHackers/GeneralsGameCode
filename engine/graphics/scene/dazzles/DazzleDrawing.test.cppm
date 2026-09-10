module;
// MSVC needs Tracy's static helpers when instantiating instrumented module templates.
#include "../../profiling/Tracy.h"
#define BOOST_TEST_MODULE DazzleDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>
export module Graphics.Scene.Dazzles.Drawing.Tests;
import Assets.Dazzles;
import Graphics.RHI;
import Graphics.Tests.Device;
import Graphics.Scene.Dazzles.Drawing;
import Graphics.Scene.Dazzles.State;
import Graphics.Scene.Dazzles.Layer;
import Graphics.Scene.Dazzles.Resources;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Props.MaterialSubmission;
import Graphics.Scene.Shadows.DirectionalRenderer;
import Graphics.Resources.Textures.Sampling;
using namespace Graphics;
namespace {
using TextureOwner=std::shared_ptr<RHITextureHandle>;
TextureOwner Create_Test_Texture(GraphicsTestDevice& device,std::span<const std::uint8_t> pixels,unsigned width=1,unsigned height=1) {
    const auto handle=device.Create_Texture_Initialized({width,height},{std::as_bytes(pixels),width*4});
    BOOST_REQUIRE(handle.Is_Valid());
    return TextureOwner(new RHITextureHandle(handle),[&device](auto* value) { device.Destroy_Texture(*value); delete value; });
}
std::optional<PropMaterialTexture> Resolve(const RHITextureHandle* source,bool) {
    // Isolate authored UV orientation from bilinear blending across wrapped edges.
    TextureSampling sampling;
    sampling.minification=sampling.magnification=sampling.mipmap=SamplingFilter::Disabled;
    return PropMaterialTexture{*source,sampling};
}
struct Target {
    GraphicsTestDevice& device; unsigned width; RHITextureHandle color,depth;
    Target(GraphicsTestDevice& device,unsigned width):device(device),width(width) {
        color=device.Create_Texture({width,32,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        depth=device.Create_Texture({width,32,1,RHITextureFormat::D32_Float,static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(color.Is_Valid()); BOOST_REQUIRE(depth.Is_Valid());
        BOOST_REQUIRE(device.Immediate_Command_List().Set_Render_Targets(color,depth));
        BOOST_REQUIRE(device.Immediate_Command_List().Set_Viewport({0,0,width,32}));
    }
    ~Target() { device.Destroy_Texture(color); device.Destroy_Texture(depth); }
    void Clear(float depth_value) { BOOST_REQUIRE(device.Immediate_Command_List().Clear({0,0,0,0},depth_value)); }
    void Pixel(unsigned x,unsigned y,std::array<int,4> expected) {
        std::vector<std::byte> pixels(width*32*4);
        BOOST_REQUIRE(device.Readback_Texture(color,pixels,width*4));
        for(unsigned c=0;c<4;++c) BOOST_TEST_CONTEXT("pixel "<<x<<","<<y<<" channel "<<c) {
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*width+x)*4+c])-expected[c],2);
        }
    }
};
}
BOOST_AUTO_TEST_CASE(geometry_keeps_quad_uv_orientation_quantized_colors_aspect_and_unscaled_halo) {
    Assets::DazzleDefinition definition;
    definition.scale={.1f,.2f}; definition.halo_scale={.3f,.4f}; definition.color={.5f,.25f,.75f};
    DazzleState state; state.screen_position={.4f,-.2f,.5f}; state.intensity=.5f; state.halo=.5f; state.size=.5f;
    state.scale=2; state.color={1,.5f,1}; state.halo_color={.1f,.2f,.3f}; state.lens_flare_intensity=2;
    DazzleDrawing drawing;
    {
        const std::array<Assets::LensFlareSprite,1> flares{{{-.5f,.1f,{.25f,.5f,1},{.25f,.1f,.75f,.9f}}}};
        drawing.Prepare(definition,state,flares,64,32);
    }
    const auto glare=drawing.Vertices(DazzleImage::Glare),halo=drawing.Vertices(DazzleImage::Halo),flare=drawing.Vertices(DazzleImage::LensFlare);
    BOOST_REQUIRE_EQUAL(glare.size(),4); BOOST_REQUIRE_EQUAL(halo.size(),4); BOOST_REQUIRE_EQUAL(flare.size(),4);
    BOOST_CHECK_SMALL(glare[0].position[0]-.5f,1e-6f); BOOST_CHECK_SMALL(glare[0].position[1]+.6f,1e-6f);
    BOOST_CHECK_SMALL(halo[0].position[0]-.7f,1e-6f); BOOST_CHECK_SMALL(halo[0].position[1]+1.f,1e-6f);
    BOOST_CHECK_EQUAL(glare[0].color[0],64/255.f); BOOST_CHECK_EQUAL(glare[0].color[1],16/255.f); BOOST_CHECK_EQUAL(glare[0].color[2],96/255.f);
    BOOST_CHECK_EQUAL(halo[0].color[0],13/255.f); BOOST_CHECK_EQUAL(halo[0].color[1],26/255.f); BOOST_CHECK_EQUAL(halo[0].color[2],38/255.f);
    BOOST_CHECK_SMALL(flare[0].position[0]+.05527864f,1e-6f); BOOST_CHECK_SMALL(flare[0].position[1]+.18944272f,1e-6f);
    BOOST_CHECK_EQUAL(flare[0].color[0],64/255.f); BOOST_CHECK_EQUAL(flare[0].color[1],128/255.f); BOOST_CHECK_EQUAL(flare[0].color[2],1);
    BOOST_CHECK((flare[0].uv==std::array<float,2>{.25f,.1f})); BOOST_CHECK((flare[1].uv==std::array<float,2>{.75f,.1f}));
    BOOST_CHECK((flare[2].uv==std::array<float,2>{.75f,.9f})); BOOST_CHECK((flare[3].uv==std::array<float,2>{.25f,.9f}));
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    BOOST_CHECK_EQUAL_COLLECTIONS(drawing.Indices(DazzleImage::Glare).begin(),drawing.Indices(DazzleImage::Glare).end(),indices.begin(),indices.end());
}
BOOST_AUTO_TEST_CASE(lens_flare_geometry_exceeds_sixteen_bit_vertex_indices) {
    Assets::DazzleDefinition definition; DazzleState state; state.intensity=1; state.size=1;
    DazzleDrawing drawing;
    {
        std::vector<Assets::LensFlareSprite> flares(17000);
        drawing.Prepare(definition,state,flares,32,32);
    }
    BOOST_REQUIRE_EQUAL(drawing.Vertices(DazzleImage::LensFlare).size(),68000);
    BOOST_REQUIRE_EQUAL(drawing.Indices(DazzleImage::LensFlare).size(),102000);
    BOOST_CHECK_EQUAL(drawing.Indices(DazzleImage::LensFlare).back(),67999);
}
BOOST_AUTO_TEST_CASE(halo_depth_and_glare_uvs_draw_both_triangles_immediately_after_renderer_recreation) {
    for(const bool warp:{true,false}) {
        GraphicsTestDevice device({warp}); if(!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        const std::array<std::uint8_t,16> quadrants{255,0,0,255,0,0,0,255,0,0,0,255,0,0,255,255};
        const std::array<std::uint8_t,4> green{0,255,0,255};
        auto glare=Create_Test_Texture(device,quadrants,2,2),halo=Create_Test_Texture(device,green);
        Assets::DazzleDefinition definition; definition.scale={.4f,.4f}; definition.halo_scale={.75f,.75f};
        DazzleState state; state.screen_position={0,0,.5f}; state.intensity=state.size=state.halo=1;
        DazzleDrawing drawing; PropRenderer renderer; PropSubmission submission; DirectionalShadowRenderer shadows;
        for(const unsigned width:{32u,64u}) {
            BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY))); submission.Initialize(device,renderer,shadows);
            Target target(device,width); drawing.Prepare(definition,state,{},width,32);
            for(const float depth:{.25f,1.f}) {
                target.Clear(depth);
                PropMaterialDrawContext context; context.sorting_depth={0,0,1,0};
                BOOST_REQUIRE(drawing.Draw(device,renderer,submission,[&](DazzleImage image) { return image==DazzleImage::Halo ? halo.get() : glare.get(); },Resolve,context));
                target.Pixel(width/2+width/8,16+width/10,{255,depth==1 ? 255 : 0,0,255});
                target.Pixel(width/2-width/8-1,15-width/10,{0,depth==1 ? 255 : 0,255,255});
                target.Pixel(width*4/5,16,depth==1 ? std::array<int,4>{0,255,0,255} : std::array<int,4>{0,0,0,0});
                target.Pixel(0,0,{0,0,0,0});
            }
            submission.Shutdown(); renderer.Shutdown();
        }
    }
}
BOOST_AUTO_TEST_CASE(queued_glare_and_flare_keep_sources_until_layer_draw_and_release_texture_generations_afterward) {
    for(const bool warp:{true,false}) {
        GraphicsTestDevice device({warp}); if(!warp && !device.Is_Valid()) continue;
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer; BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        PropSubmission submission; DirectionalShadowRenderer shadows; submission.Initialize(device,renderer,shadows);
        Target target(device,32); target.Clear(1);
        struct Source { DazzleMembership membership; DazzleState state; DazzleResources<TextureOwner> resources; };
        auto source=std::make_shared<Source>();
        {
            Assets::DazzleDefinitions definitions;
            definitions.lens_flares.push_back({"Flare","green",{{-1,.08f}}});
            auto& definition=definitions.dazzles.emplace_back(); definition.name="Lamp";
            definition.primary_texture="red"; definition.lens_flare="Flare"; definition.scale={.1f,.1f};
            source->resources.Initialize(std::move(definitions));
        }
        const auto acquire=[&](const std::string& name) {
            const std::array<std::uint8_t,4> color=name=="red" ? std::array<std::uint8_t,4>{255,0,0,255} : std::array<std::uint8_t,4>{0,255,0,255};
            return Create_Test_Texture(device,color);
        };
        const auto glare=*source->resources.Texture(0,DazzleImage::Glare,acquire);
        const auto flare=*source->resources.Texture(0,DazzleImage::LensFlare,acquire);
        source->state.screen_position={.6f,0,.5f}; source->state.intensity=.5f; source->state.size=1; source->state.lens_flare_intensity=2;
        DazzleLayer<std::shared_ptr<Source>> layer(1);
        layer.Queue(0,source->membership,[&] { return source; }); source.reset();
        target.Pixel(7,16,{0,0,0,0}); target.Pixel(26,16,{0,0,0,0});
        DazzleDrawing drawing;
        layer.Draw_All([&](const auto& queued) {
            drawing.Prepare(queued->resources.Definition(0),queued->state,queued->resources.Sprites(0),32,32);
            BOOST_REQUIRE(drawing.Draw(device,renderer,submission,[&](DazzleImage image) {
                return queued->resources.Texture(0,image,acquire);
            },Resolve,{}));
        });
        target.Pixel(7,16,{0,255,0,255}); target.Pixel(5,15,{0,255,0,255});
        target.Pixel(26,16,{128,0,0,255}); target.Pixel(24,15,{128,0,0,255}); target.Pixel(16,16,{0,0,0,0});
        BOOST_CHECK(!device.Retain_Texture(glare)); BOOST_CHECK(!device.Retain_Texture(flare));
        submission.Shutdown(); renderer.Shutdown();
    }
}
