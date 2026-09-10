module;
#define BOOST_TEST_MODULE AttachmentBindingsTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <memory>
#include <vector>
export module Graphics.Frame.AttachmentBindings.Tests;
import Graphics.Frame.AttachmentBindings;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
using namespace Graphics;

namespace
{
std::unique_ptr<TextureResource> Target(Device& device,unsigned size)
{
    return std::unique_ptr<TextureResource>(TextureResource::Create(&device,
        {size,size,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)},
        Assets::PixelEncoding::RGBA8,RHITextureFormat::D24_UNorm_S8));
}
void Draw(Device& device,std::array<float,4> color)
{
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    std::array<PropVertex,3> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={3,-1,0.5f}; vertices[2].position={-1,3,0.5f};
    for (auto& vertex : vertices) vertex.color=color;
    const auto mesh=renderer.Create_Mesh(vertices,std::array<std::uint32_t,3>{0,1,2});
    PropParameters parameters; parameters.textured=0;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    PropStyle style; style.blend=RHIBlendMode::Disabled; style.depth_test=true; style.depth_write=true;
    BOOST_REQUIRE(renderer.Draw(device.Immediate_Command_List(),mesh,style,parameters,{}));
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
}
std::vector<std::byte> Pixels(Device& device,RHITextureHandle texture,unsigned size)
{
    std::vector<std::byte> pixels(size*size*4);
    BOOST_REQUIRE(device.Readback_Texture(texture,pixels,size*4));
    return pixels;
}
}

BOOST_AUTO_TEST_CASE(nested_passes_retain_destroyed_texture_owners_and_restore_viewport_and_depth)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        auto main=Target(device,4),first=Target(device,2),second=Target(device,4);
        BOOST_REQUIRE(main && first && second);
        AttachmentBindings bindings;
        BOOST_REQUIRE(bindings.Initialize(device,{main->Handle(),main->Depth_Attachment(),{1,1,2,2,0.2f,0.8f}}));
        bindings.Clear(true,true,{0,0,0,0});
        const auto first_handle=first->Handle(),second_handle=second->Handle();
        {
            AttachmentScope outer(bindings,first.get());
            BOOST_REQUIRE(outer.Active()); BOOST_CHECK(bindings.Offscreen());
            BOOST_REQUIRE(bindings.Set_Viewport({0,0,1,2,0,1}));
            bindings.Clear(true,true,{0,0,0,0});
            first.reset();
            Draw(device,{1,0,0,0.5f});
            {
                AttachmentScope inner(bindings,second.get());
                BOOST_REQUIRE(inner.Active()); second.reset();
                bindings.Clear(true,true,{0,0,0,0}); Draw(device,{0,1,0,0.25f});
                const auto pixels=Pixels(device,second_handle,4);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[1]),255u);
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-64,1);
            }
            BOOST_CHECK(bindings.Current().color==first_handle);
            BOOST_CHECK_EQUAL(bindings.Current().viewport.width,1u);
            bindings.Clear(false,true,{}); Draw(device,{0,0,1,0.75f});
            const auto pixels=Pixels(device,first_handle,2);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[2]),255u);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-191,1);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[7]),0u);
        }
        BOOST_CHECK(!device.Retain_Texture(first_handle));
        BOOST_CHECK(!device.Retain_Texture(second_handle));
        BOOST_CHECK(!bindings.Offscreen());
        BOOST_CHECK_EQUAL(bindings.Current().viewport.x,1u);
        BOOST_CHECK_EQUAL(bindings.Current().viewport.min_depth,0.2f);
        Draw(device,{0,1,0,0.5f});
        const auto pixels=Pixels(device,main->Handle(),4);
        for (unsigned y=0;y<4;++y) for (unsigned x=0;x<4;++x) {
            const auto offset=(y*4+x)*4;
            const bool inside=x>=1 && x<3 && y>=1 && y<3;
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset+1]),inside ? 255u : 0u);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[offset+3])-(inside ? 128 : 0),1);
        }
    }
}

BOOST_AUTO_TEST_CASE(invalid_and_foreign_bindings_preserve_the_active_drawing_target)
{
    GraphicsTestDevice device({true}),foreign({true});
    auto main=Target(device,2),other=Target(foreign,2);
    BOOST_REQUIRE(main && other);
    AttachmentBindings bindings,foreign_bindings;
    BOOST_REQUIRE(bindings.Initialize(device,{main->Handle(),main->Depth_Attachment(),{0,0,2,2}}));
    BOOST_REQUIRE(foreign_bindings.Initialize(foreign,{other->Handle(),other->Depth_Attachment(),{0,0,2,2}}));
    BOOST_CHECK(!bindings.Bind(other.get()));
    BOOST_CHECK(!bindings.Restore(foreign_bindings.Capture()));
    auto invalid=bindings.Current(); invalid.viewport.width=0;
    BOOST_CHECK(!bindings.Bind(invalid));
    auto transient=Target(device,2); BOOST_REQUIRE(transient);
    invalid=bindings.Current(); invalid.color=transient->Handle(); transient.reset();
    BOOST_CHECK(!bindings.Bind(invalid));
    BOOST_CHECK(bindings.Current().color==main->Handle());
    bindings.Clear(true,true,{0,0,0,0}); Draw(device,{1,0,0,0.5f});
    const auto pixels=Pixels(device,main->Handle(),2);
    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[0]),255u);
    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-128,1);
}

BOOST_AUTO_TEST_CASE(default_replacement_and_snapshot_lifetime_preserve_saved_gpu_generations)
{
    GraphicsTestDevice device({true});
    AttachmentSnapshot saved;
    RHITextureHandle old_handle{};
    {
        AttachmentBindings bindings;
        auto original=Target(device,2);
        BOOST_REQUIRE(original); old_handle=original->Handle();
        BOOST_REQUIRE(bindings.Initialize(device,{old_handle,original->Depth_Attachment(),{0,0,2,2}}));
        bindings.Clear(true,true,{1,0,0,0.25f});
        saved=bindings.Capture(); original.reset(); bindings.Reset();
        auto replacement=Target(device,4); BOOST_REQUIRE(replacement);
        BOOST_REQUIRE(bindings.Initialize(device,{replacement->Handle(),replacement->Depth_Attachment(),{0,0,4,4}}));
        BOOST_REQUIRE(bindings.Restore(saved));
        BOOST_CHECK(bindings.Offscreen());
        const auto old_pixels=Pixels(device,old_handle,2);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(old_pixels[0]),255u);
        BOOST_CHECK_SMALL(std::to_integer<int>(old_pixels[3])-64,1);
        BOOST_REQUIRE(bindings.Restore_Default());
        bindings.Clear(true,true,{0,0,0,0}); Draw(device,{0,0,1,0.75f});
        const auto pixels=Pixels(device,replacement->Handle(),4);
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[2]),255u);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-191,1);
    }
    auto copy=saved; saved.Reset();
    BOOST_CHECK(device.Retain_Texture(old_handle)); device.Destroy_Texture(old_handle);
    copy.Reset(); BOOST_CHECK(!device.Retain_Texture(old_handle));
}
