module;
#define BOOST_TEST_MODULE FrameResourceLifecycleTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <memory>
#include <utility>
export module Graphics.Frame.ResourceLifecycle.Tests;
import Graphics.Frame.ResourceLifecycle;
import Graphics.Resources.Textures.Resource;
import Graphics.Tests.Device;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(expired_owner_cannot_receive_device_shutdown_or_recreation_callbacks)
{
    FrameResourceLifecycle lifecycle;
    unsigned releases=0,recreates=0;
    std::weak_ptr<int> captured;
    {
        auto owner=std::make_shared<int>(7);
        captured=owner;
        auto registration=lifecycle.Register([owner,&releases] { releases+=*owner; },[&] { ++recreates; });
        owner.reset();
        lifecycle.Release(); lifecycle.Reacquire();
        BOOST_CHECK_EQUAL(releases,7u); BOOST_CHECK_EQUAL(recreates,1u);
        BOOST_CHECK(!captured.expired());
    }
    BOOST_CHECK(captured.expired());
    lifecycle.Release(); lifecycle.Reacquire();
    BOOST_CHECK_EQUAL(releases,7u); BOOST_CHECK_EQUAL(recreates,1u);
}

BOOST_AUTO_TEST_CASE(replacement_move_and_self_removal_preserve_the_current_owner)
{
    FrameResourceRegistration surviving;
    unsigned previous=0,current=0;
    {
        FrameResourceLifecycle lifecycle;
        auto first=lifecycle.Register([&] { ++previous; },{});
        auto second=lifecycle.Register([&] { ++current; },{});
        first.Reset();
        surviving=std::move(second);
        lifecycle.Release();
        BOOST_CHECK_EQUAL(previous,0u); BOOST_CHECK_EQUAL(current,1u);
        surviving=lifecycle.Register([&] { ++current; surviving.Reset(); },{});
        lifecycle.Release(); lifecycle.Release();
        BOOST_CHECK_EQUAL(current,2u);
        surviving=lifecycle.Register([&] { ++current; },{});
    }
    surviving.Reset(); // A registration may also outlive its lifecycle owner.
    BOOST_CHECK_EQUAL(current,2u);
}

BOOST_AUTO_TEST_CASE(resource_reset_replaces_drawable_generation_and_destruction_expires_callbacks)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        FrameResourceLifecycle lifecycle;
        RHITextureHandle first{},second{};
        unsigned releases=0,recreates=0;
        {
            std::unique_ptr<TextureResource> target;
            auto registration=lifecycle.Register([&] { ++releases; target.reset(); },[&] {
                ++recreates;
                target.reset(TextureResource::Create(&device,{2,2,1,RHITextureFormat::RGBA8_UNorm,
                    static_cast<unsigned>(RHITextureUsage::RenderTarget)},Assets::PixelEncoding::RGBA8));
            });
            lifecycle.Reacquire();
            BOOST_REQUIRE(target); first=target->Handle();
            BOOST_REQUIRE(device.Immediate_Command_List().Clear_Color_Target(first,{1,0,0,0.5f}));
            std::array<std::byte,16> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(first,pixels,8));
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[0]),255u);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-128,1);
            lifecycle.Release();
            BOOST_CHECK(!target); BOOST_CHECK(!device.Retain_Texture(first));
            lifecycle.Reacquire();
            BOOST_REQUIRE(target); second=target->Handle();
            BOOST_CHECK(second!=first);
            BOOST_REQUIRE(device.Immediate_Command_List().Clear_Color_Target(second,{0,1,0,0.25f}));
            BOOST_REQUIRE(device.Readback_Texture(second,pixels,8));
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[0]),0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[1]),255u);
            BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3])-64,1);
        }
        lifecycle.Release(); lifecycle.Reacquire();
        BOOST_CHECK_EQUAL(releases,1u); BOOST_CHECK_EQUAL(recreates,2u);
        BOOST_CHECK(!device.Retain_Texture(second));
    }
}
