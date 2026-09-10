module;
#define BOOST_TEST_MODULE ResourceRecreationTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>
export module Graphics.Resources.Recreation.Tests;
import Graphics.Resources.Recreation;
import Graphics.Resources.Textures.Resource;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(growing_owner_storage_preserves_notification_order_and_expired_owners)
{
    ResourceRecreationRegistry registry;
    std::vector<ResourceRecreationRegistration> owners;
    std::vector<unsigned> released,recreated;
    for (unsigned i=0;i<4096;++i)
        owners.push_back(registry.Register([&,i] { released.push_back(i); },[&,i] { recreated.push_back(i); }));
    for (unsigned i=0;i<4096;i+=3) owners[i].Reset();
    registry.Release(); registry.Recreate();
    BOOST_REQUIRE_EQUAL(released.size(),2730u);
    BOOST_REQUIRE_EQUAL(recreated.size(),released.size());
    unsigned index=0;
    for (unsigned i=0;i<4096;++i) if (i%3) {
        BOOST_CHECK_EQUAL(released[index],i);
        BOOST_CHECK_EQUAL(recreated[index++],i);
    }
    owners.clear(); registry.Release(); registry.Recreate();
    BOOST_CHECK_EQUAL(released.size(),2730u);
    BOOST_CHECK_EQUAL(recreated.size(),2730u);
}

BOOST_AUTO_TEST_CASE(callback_removal_movement_and_new_registration_do_not_invalidate_dispatch)
{
    ResourceRecreationRegistry registry;
    ResourceRecreationRegistration first,second,moved,third,added;
    std::vector<unsigned> visited;
    first=registry.Register([&] {
        visited.push_back(1); first.Reset(); second.Reset(); moved=std::move(third);
        added=registry.Register([&] { visited.push_back(4); },{});
    },{});
    second=registry.Register([&] { visited.push_back(2); },{});
    third=registry.Register([&] { visited.push_back(3); },{});
    registry.Release();
    const std::vector<unsigned> first_expected{1,3};
    BOOST_CHECK(visited==first_expected);
    registry.Release();
    const std::vector<unsigned> second_expected{1,3,3,4};
    BOOST_CHECK(visited==second_expected);
    moved=registry.Register([] { throw std::runtime_error("test callback"); },{});
    BOOST_CHECK_THROW(registry.Release(),std::runtime_error);
    moved.Reset(); added.Reset(); registry.Release();
    ResourceRecreationRegistration survivor;
    std::weak_ptr<int> captured;
    {
        auto owner=std::make_shared<int>(1); captured=owner;
        ResourceRecreationRegistry transient;
        survivor=transient.Register([owner] {},{});
        owner.reset();
        BOOST_CHECK(!captured.expired());
    }
    BOOST_CHECK(captured.expired());
    survivor.Reset();
}

BOOST_AUTO_TEST_CASE(recreated_targets_preserve_rgb_alpha_and_independent_resource_owners)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        ResourceRecreationRegistry registry;
        std::array<std::unique_ptr<TextureResource>,3> targets;
        std::array<RHITextureHandle,3> old{};
        std::array<ResourceRecreationRegistration,3> registrations;
        for (unsigned i=0;i<targets.size();++i) {
            registrations[i]=registry.Register([&,i] { targets[i].reset(); },[&,i] {
                if (!targets[i]) targets[i].reset(TextureResource::Create(&device,
                    {2,2,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)},
                    Assets::PixelEncoding::RGBA8,RHITextureFormat::D24_UNorm_S8));
            });
        }
        registry.Recreate();
        for (unsigned i=0;i<targets.size();++i) {
            BOOST_REQUIRE(targets[i]); old[i]=targets[i]->Handle();
            BOOST_CHECK(targets[i]->Depth_Attachment().Is_Valid());
        }
        registrations[1].Reset(); targets[1].reset();
        registry.Release();
        for (auto handle : old) BOOST_CHECK(!device.Retain_Texture(handle));
        registry.Recreate();
        BOOST_CHECK(!targets[1]);
        for (const unsigned i : {0u,2u}) {
            BOOST_REQUIRE(targets[i]); BOOST_CHECK(targets[i]->Handle()!=old[i]);
            auto& commands=device.Immediate_Command_List();
            BOOST_REQUIRE(commands.Set_Render_Targets(targets[i]->Handle(),targets[i]->Depth_Attachment()));
            BOOST_REQUIRE(commands.Clear({i==0 ? 1.0f : 0.0f,0,i==2 ? 1.0f : 0.0f,0.25f},1));
            std::array<std::byte,16> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(targets[i]->Handle(),pixels,8));
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[i]),255u);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-64,1);
        }
        registry.Release(); registry.Recreate();
        BOOST_CHECK(targets[0] && targets[2]);
    }
}

BOOST_AUTO_TEST_CASE(atlas_aliases_retain_the_recreated_generation_across_owner_notification_order)
{
    GraphicsTestDevice device({true});
    ResourceRecreationRegistry registry;
    TextureResource* atlas=nullptr;
    TextureResource* alias=nullptr;
    const auto recreate_atlas=[&] {
        if (!atlas) atlas=TextureResource::Create(&device,
            {2,2,1,RHITextureFormat::RGBA8_UNorm,static_cast<unsigned>(RHITextureUsage::RenderTarget)},
            Assets::PixelEncoding::RGBA8);
    };
    // The alias can appear before its source during reset notification.
    auto alias_registration=registry.Register([&] {
        Release_Texture_Resource(alias); alias=nullptr;
    },[&] {
        recreate_atlas();
        if (!alias) alias=Retain_Texture_Resource(atlas);
    });
    auto atlas_registration=registry.Register([&] {
        Release_Texture_Resource(atlas); atlas=nullptr;
    },recreate_atlas);
    RHITextureHandle previous{};
    for (unsigned cycle=0;cycle<3;++cycle) {
        registry.Recreate();
        BOOST_REQUIRE(atlas); BOOST_REQUIRE(alias==atlas);
        BOOST_CHECK_EQUAL(atlas->Reference_Count(),2u);
        BOOST_CHECK(atlas->Handle()!=previous); previous=atlas->Handle();
        BOOST_REQUIRE(device.Immediate_Command_List().Clear_Color_Target(atlas->Handle(),{0,1,0,0.5f}));
        std::array<std::byte,16> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(alias->Handle(),pixels,8));
        BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[1]),255u);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-128,1);
        registry.Release();
        BOOST_CHECK(!atlas && !alias); BOOST_CHECK(!device.Retain_Texture(previous));
    }
}
