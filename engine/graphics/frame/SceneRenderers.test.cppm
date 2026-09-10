module;
#define BOOST_TEST_MODULE SceneRendererLifetimeTests
#include <boost/test/included/unit_test.hpp>
#include <filesystem>

export module Graphics.Frame.SceneRenderers.Tests;
import Graphics.Frame.SceneRenderers;
import Graphics.Scene.Particles.Renderer;
import Graphics.Tests.Device;

BOOST_AUTO_TEST_CASE(scene_resources_recover_from_missing_shaders_and_restart)
{
    Graphics::GraphicsTestDevice device({true});
    BOOST_REQUIRE(device.Is_Valid());
    const std::filesystem::path shaders(Graphics::Test_Shader_Directory(GRAPHICS_SCENE_SHADER_DIRECTORY));
    BOOST_CHECK(!Graphics::Initialize_Scene_Renderers(device, shaders / "missing"));
    BOOST_CHECK(!Graphics::GetParticleRenderer().Is_Initialized());
    for (unsigned cycle=0;cycle<2;++cycle) {
        BOOST_REQUIRE(Graphics::Initialize_Scene_Renderers(device,shaders));
        BOOST_CHECK(Graphics::GetParticleRenderer().Is_Initialized());
        Graphics::Shutdown_Scene_Renderers();
        BOOST_CHECK(!Graphics::GetParticleRenderer().Is_Initialized());
    }
}
