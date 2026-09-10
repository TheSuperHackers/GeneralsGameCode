module;

#define BOOST_TEST_MODULE ModelMaterialsTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>

export module Graphics.Scene.Models.Materials.Tests;

import Assets.Math;
import Assets.Materials.TextureMapping;
import Graphics.Materials.MeshMaterial;
import Graphics.Materials.TextureMapping;
import Graphics.Scene.Models.Materials;
import Graphics.Scene.Models.MaterialSlots;
import Graphics.Scene.Props.Geometry;
import Graphics.Scene.Props.Material;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
import Graphics.RHI;

namespace {
struct Resource final { int value = 0; };
using Owner = std::shared_ptr<Resource>;
using Materials = Graphics::ModelMaterials<Owner>;

struct TestTexture final
{
    Graphics::Device* device = nullptr;
    Graphics::RHITextureHandle handle;
    ~TestTexture() { if (device && handle.Is_Valid()) device->Destroy_Texture(handle); }
};
}

BOOST_AUTO_TEST_CASE(cloning_restarts_each_material_mapping_and_shares_textures)
{
    Materials source;
    const auto material = std::make_shared<Graphics::MeshMaterial>();
    material->name = "Tread";
    material->parameters.diffuse = {.25f, .5f, .75f};
    material->parameters.opacity = .625f;
    material->flags = 4;
    material->uv_sources[1] = 3;
    material->Make_Unique();
    const Assets::TextureScrollMapping description{{1,1},{.5f,0},{.25f,.5f}};
    material->mappings[0] = Graphics::TextureMapping::Create(description,1000);
    auto* scroll = material->mappings[0]->Linear_Scroll();
    BOOST_REQUIRE(scroll);
    scroll->offset = {.625f,.875f};
    scroll->rate_per_millisecond = {};
    source.materials = {material,material};
    source.textures.push_back(std::make_shared<Resource>(Resource{7}));
    auto clone = source.Clone(2000);
    BOOST_REQUIRE_EQUAL(clone.materials.size(),2u);
    BOOST_REQUIRE_EQUAL(clone.textures.size(),1u);
    BOOST_CHECK(clone.textures[0] == source.textures[0]);
    BOOST_CHECK(clone.materials[0] != material);
    BOOST_CHECK(clone.materials[1] != clone.materials[0]);
    BOOST_CHECK_EQUAL(clone.materials[0]->name,"Tread");
    BOOST_CHECK(clone.materials[0]->parameters.diffuse == material->parameters.diffuse);
    BOOST_CHECK_EQUAL(clone.materials[0]->parameters.opacity,.625f);
    BOOST_CHECK_EQUAL(clone.materials[0]->flags,4u);
    BOOST_CHECK_EQUAL(clone.materials[0]->uv_sources[1],3u);
    BOOST_CHECK_EQUAL(clone.materials[0]->Content_Key().unique_group,material->Content_Key().unique_group);
    BOOST_REQUIRE(clone.materials[0]->mappings[0]);
    BOOST_CHECK(clone.materials[0]->mappings[0] != material->mappings[0]);
    BOOST_CHECK(clone.materials[0]->mappings[0] != clone.materials[1]->mappings[0]);
    BOOST_CHECK_SMALL(clone.materials[0]->mappings[0]->Evaluate(2000).transform[3]-.25f,.00001f);
    BOOST_CHECK_SMALL(clone.materials[0]->mappings[0]->Evaluate(3000).transform[3]-.25f,.00001f);
    BOOST_CHECK_SMALL(material->mappings[0]->Evaluate(3000).transform[3]-.625f,.00001f);
    auto* clone_scroll = clone.materials[0]->mappings[0]->Linear_Scroll();
    BOOST_REQUIRE(clone_scroll);
    BOOST_CHECK_EQUAL(clone_scroll->rate_per_millisecond.x,0);
    clone_scroll->rate_per_millisecond.x = -.0005f;
    BOOST_CHECK_SMALL(clone.materials[0]->mappings[0]->Evaluate(4000).transform[3]-.75f,.00001f);
    BOOST_CHECK_SMALL(clone.materials[1]->mappings[0]->Evaluate(4000).transform[3]-.25f,.00001f);
    BOOST_CHECK_SMALL(material->mappings[0]->Evaluate(4000).transform[3]-.625f,.00001f);
    clone.materials[0]->parameters.opacity = .125f;
    BOOST_CHECK_EQUAL(material->parameters.opacity,.625f);
    BOOST_CHECK_EQUAL(clone.materials[1]->parameters.opacity,.625f);
    clone.textures[0] = std::make_shared<Resource>(Resource{9});
    BOOST_CHECK_EQUAL(source.textures[0]->value,7);
    source.Reset();
    BOOST_CHECK(source.materials.empty() && source.textures.empty());
    BOOST_CHECK_EQUAL(clone.textures[0]->value,9);
}

BOOST_AUTO_TEST_CASE(remapping_retains_ordered_identities_and_only_updates_the_active_slot_prefix)
{
    Graphics::MaterialSlots<Owner> source_slots;
    source_slots.Allocate(5);
    Graphics::MaterialSlots<Owner> destination_slots;
    destination_slots.Allocate(5);
    const auto sentinel = std::make_shared<Resource>(Resource{99});
    destination_slots.Set(4,sentinel);
    std::weak_ptr<Resource> source_weak;
    std::weak_ptr<Resource> destination_weak;
    {
        std::array<Owner,3> source{std::make_shared<Resource>(Resource{1}),
            std::make_shared<Resource>(Resource{2}),{}};
        source[2] = source[0];
        std::array<Owner,3> destination{std::make_shared<Resource>(Resource{10}),
            std::make_shared<Resource>(Resource{20}),std::make_shared<Resource>(Resource{30})};
        source_weak = source[0];
        destination_weak = destination[0];
        const auto* first = source[0].get();
        const auto* second = source[1].get();
        source_slots.Set(0,source[1]);
        source_slots.Set(1,source[0]);
        source_slots.Set(3,source[0]);
        source_slots.Set(4,source[1]);
        Graphics::MaterialResourceRemap<Owner> remap(source,destination);
        source = {};
        destination = {};
        BOOST_CHECK_EQUAL(remap.Find(first)->value,10);
        BOOST_CHECK_EQUAL(remap.Find(second)->value,20);
        BOOST_CHECK_EQUAL(remap.Find(first)->value,10);
        BOOST_CHECK_EQUAL(remap.Find(first)->value,10);
        BOOST_CHECK(!remap.Find(static_cast<const Resource*>(nullptr)));
        remap.Remap_Slots(source_slots,destination_slots,4);
        BOOST_CHECK_EQUAL(destination_slots.Get(0)->value,20);
        BOOST_CHECK_EQUAL(destination_slots.Get(1)->value,10);
        BOOST_CHECK(!destination_slots.Get(2));
        BOOST_CHECK_EQUAL(destination_slots.Get(3)->value,10);
        BOOST_CHECK(destination_slots.Get(4) == sentinel);
        BOOST_CHECK_EQUAL(source_slots.Get(0)->value,2);
        source_slots.Reset();
        BOOST_CHECK(!source_weak.expired());
        BOOST_CHECK(!destination_weak.expired());
    }
    BOOST_CHECK(source_weak.expired());
    BOOST_CHECK(!destination_weak.expired());
    destination_slots.Reset();
    BOOST_CHECK(destination_weak.expired());
}

BOOST_AUTO_TEST_CASE(empty_resource_collections_preserve_empty_slot_storage)
{
    Materials source;
    const auto clone = source.Clone(1234);
    BOOST_CHECK(clone.materials.empty() && clone.textures.empty());
    Graphics::MaterialResourceRemap<Owner> remap(source.textures,clone.textures);
    BOOST_CHECK(remap.Empty());
    Graphics::MaterialSlots<Owner> absent;
    Graphics::MaterialSlots<Owner> allocated;
    allocated.Allocate(0);
    remap.Remap_Slots(absent,allocated,0);
    BOOST_CHECK(!absent.Is_Allocated());
    BOOST_CHECK(allocated.Is_Allocated());
}

BOOST_AUTO_TEST_CASE(cloned_resource_slots_draw_material_and_texture_pixels_after_source_release_and_recreation)
{
    using namespace Graphics;
    using TextureOwner = std::shared_ptr<TestTexture>;
    GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    PropRenderer renderer;
    const auto shaders = Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
    BOOST_REQUIRE(renderer.Initialize(device,shaders));
    MaterialSlots<std::shared_ptr<MeshMaterial>> materials;
    MaterialSlots<TextureOwner> textures;
    std::weak_ptr<MeshMaterial> original_material;
    std::weak_ptr<TestTexture> retained_texture;
    {
        ModelMaterials<TextureOwner> source;
        for (unsigned index = 0; index < 2; ++index) {
            auto texture = std::make_shared<TestTexture>();
            texture->device = &device;
            const std::array<std::uint8_t,4> pixel = index == 0
                ? std::array<std::uint8_t,4>{128,255,64,255}
                : std::array<std::uint8_t,4>{255,64,128,128};
            texture->handle = device.Create_Texture_Initialized({1,1},{std::as_bytes(std::span(pixel)),4});
            BOOST_REQUIRE(texture->handle.Is_Valid());
            source.textures.push_back(texture);
            source.materials.push_back(std::make_shared<MeshMaterial>());
        }
        source.materials[0]->parameters.diffuse = {.5f,.25f,1};
        source.materials[0]->parameters.opacity = .5f;
        source.materials[1]->parameters.diffuse = {1,.5f,.25f};
        source.materials[1]->parameters.opacity = .75f;
        original_material = source.materials[0];
        retained_texture = source.textures[0];
        auto clone = source.Clone(0);
        clone.materials[0]->parameters.diffuse = {.25f,.5f,1};
        clone.materials[1]->parameters.opacity = .25f;
        MaterialResourceRemap<std::shared_ptr<MeshMaterial>> material_remap(source.materials,clone.materials);
        MaterialResourceRemap<TextureOwner> texture_remap(source.textures,clone.textures);
        materials.Allocate(2);
        textures.Allocate(2);
        for (unsigned index = 0; index < 2; ++index) {
            materials.Set(index,material_remap.Find(source.materials[index].get()));
            textures.Set(index,texture_remap.Find(source.textures[index].get()));
        }
    }
    BOOST_CHECK(original_material.expired());
    BOOST_CHECK(!retained_texture.expired());
    std::array<PropMeshHandle,2> meshes;
    for (unsigned quad = 0; quad < 2; ++quad) {
        std::array<PropVertex,4> vertices{};
        const float left = quad == 0 ? -.875f : .125f;
        vertices[0].position = {left,-.75f,.5f};
        vertices[1].position = {left+.75f,-.75f,.5f};
        vertices[2].position = {left+.75f,.75f,.5f};
        vertices[3].position = {left,.75f,.5f};
        for (auto& vertex : vertices) {
            vertex.color = {1,1,1,1};
            vertex.uv = {.5f,.5f};
            Apply_Prop_Material(vertex,materials.Get(quad)->parameters);
        }
        const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
        meshes[quad] = renderer.Create_Mesh(vertices,indices);
        BOOST_REQUIRE(meshes[quad].Is_Valid());
    }
    PropParameters parameters;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    parameters.textured = 1;
    PropStyle style;
    style.blend = RHIBlendMode::Disabled;
    style.cull = RHICullMode::None;
    for (unsigned size : {16u,32u}) {
        const auto target = device.Create_Texture({size,size,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({size,size,1,RHITextureFormat::D32_Float,
            static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(target.Is_Valid() && depth.Is_Valid());
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
        BOOST_REQUIRE(commands.Set_Viewport({0,0,size,size}));
        BOOST_REQUIRE(commands.Clear({.125f,.25f,.5f,.75f},1));
        for (unsigned quad = 0; quad < 2; ++quad) {
            const std::array draw_textures{textures.Get(quad)->handle};
            BOOST_REQUIRE(renderer.Draw(commands,meshes[quad],style,parameters,draw_textures));
        }
        std::array<std::byte,32*32*4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,size*4));
        const auto check = [&](unsigned x,unsigned y,std::array<int,4> expected) {
            x = x*size/16;
            y = y*size/16;
            for (unsigned channel = 0; channel < 4; ++channel)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(y*size+x)*4+channel])-expected[channel],2);
        };
        check(2,4,{32,128,64,128}); check(5,11,{32,128,64,128});
        check(10,4,{255,32,32,32}); check(13,11,{255,32,32,32});
        check(8,8,{32,64,128,191}); check(0,0,{32,64,128,191});
        BOOST_CHECK(device.Destroy_Texture(target));
        BOOST_CHECK(device.Destroy_Texture(depth));
        if (size == 16) {
            renderer.Shutdown();
            BOOST_REQUIRE(renderer.Initialize(device,shaders));
        }
    }
    for (auto mesh : meshes) BOOST_CHECK(renderer.Destroy_Mesh(mesh));
    renderer.Shutdown();
    materials.Reset();
    textures.Reset();
    BOOST_CHECK(retained_texture.expired());
}
