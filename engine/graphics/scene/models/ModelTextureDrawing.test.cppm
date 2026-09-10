module;
#define BOOST_TEST_MODULE ModelTextureDrawingTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <vector>
export module Graphics.Scene.Models.TextureDrawing.Tests;
import Assets.Cache;
import Assets.Identity;
import Assets.Models;
import Assets.Materials;
import Graphics.Scene.Models.ModelAssetBinding;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(model_binding_loads_named_texture_and_preserves_cutout)
{
    std::vector<std::byte> encoded(26);
    encoded[2]=std::byte{2}; encoded[12]=std::byte{2}; encoded[14]=std::byte{1};
    encoded[16]=std::byte{32}; encoded[17]=std::byte{0x28};
    encoded[19]=std::byte{255}; encoded[23]=std::byte{255}; encoded[25]=std::byte{255};
    Assets::AssetCache cache([&](const Assets::AssetIdentity&) { return encoded; });
    Assets::MaterialAssetDesc material;
    material.name="cutout"; material.primary_texture="cutout.tga";
    material.base_color={1,1,1,1}; material.render_mode=Assets::MaterialRenderMode::AlphaTest;
    const auto material_asset=cache.Request_Material(material);
    cache.Wait(material_asset);
    Assets::ModelAssetDesc description;
    description.name="quad"; description.bounds={{-0.8f,-0.8f,0.5f},{0.8f,0.8f,0.5f}};
    description.vertices={
        {{-0.8f,-0.8f,0.5f},{0,0,1},{0,1},{1,1,1,1}},
        {{0.8f,-0.8f,0.5f},{0,0,1},{1,1},{1,1,1,1}},
        {{0.8f,0.8f,0.5f},{0,0,1},{1,0},{1,1,1,1}},
        {{-0.8f,0.8f,0.5f},{0,0,1},{0,0},{1,1,1,1}}};
    description.indices={0,1,2,0,2,3}; description.submeshes={{0,6,0,"quad"}};
    description.materials={material};
    const std::array material_handles{material_asset};
    const Assets::ModelAsset model({Assets::AssetType::Model,"quad"},std::move(description),material_handles);
    GraphicsTestDevice device({true}); BOOST_REQUIRE(device.Is_Valid());
    StaticMeshRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_MODEL_SHADER_DIRECTORY),2,1,16,4));
    renderer.Set_View({Matrix4x4::Identity(),Matrix4x4::Identity(),{},{0,0,128,72,0,1}});
    StaticMeshBinding binding;
    RenderTransform transform; transform.matrix=Matrix4x4::Identity().values;
    std::vector<MaterialHandle> materials;
    BOOST_CHECK(!Create_Model_Asset_Binding(renderer,model,transform,RenderInstanceFlags::DoubleSided,
        All_Submeshes_Visible,binding,materials));
    std::vector<TextureHandle> textures;
    BOOST_REQUIRE(Create_Model_Asset_Textures(renderer,cache,model,textures));
    BOOST_REQUIRE(Create_Model_Asset_Binding(renderer,model,transform,RenderInstanceFlags::DoubleSided,
        All_Submeshes_Visible,binding,materials,textures));
    const auto target=device.Create_Texture({128,72,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({128,72,1,RHITextureFormat::D32_Float,
        static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Clear({0,0,1,1},1));
    BOOST_REQUIRE(renderer.Render(commands,{{target,128,72},{depth,128,72}},false));
    std::vector<std::byte> pixels(128*72*4);
    BOOST_REQUIRE(device.Readback_Texture(target,pixels,128*4));
    const auto check=[&](unsigned x,std::array<int,4> expected) {
        for (unsigned c=0;c<4;++c) {
            BOOST_TEST_CONTEXT("x=" << x << " channel=" << c) {
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(36*128+x)*4+c])-expected[c],2);
            }
        }
    };
    check(40,{0,0,255,255});
    // The wrap sampler blends across the two-texel edge; alpha survives the
    // cutoff without being forced opaque (u=0.7392578 at this pixel center).
    check(88,{0,255,0,249});
    binding.Destroy(renderer);
    for (auto handle : materials) BOOST_REQUIRE(renderer.Destroy_Material(handle));
    for (auto handle : textures) BOOST_REQUIRE(renderer.Destroy_Texture(handle));
    renderer.Shutdown(); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
