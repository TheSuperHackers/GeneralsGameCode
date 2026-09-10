module;
#define NOMINMAX
#define BOOST_TEST_MODULE TextureResidencyTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>
export module Graphics.Resources.Textures.Residency.Tests;
import Assets.Images.PixelEncoding;
import Graphics.Resources.Loading.Queue;
import Graphics.Resources.Recreation;
import Graphics.Resources.Textures.Residency;
import Graphics.Resources.Textures.Resource;
import Graphics.Resources.Textures.Upload;
import Graphics.Scene.Props.Renderer;
import Graphics.Tests.Device;
using namespace Graphics;

namespace
{

TextureResource* Make_Solid_Texture(Device& device, std::array<std::byte, 4> color)
{
    auto* texture = TextureResource::Create(&device, {1, 1, 1}, Assets::PixelEncoding::BGRA8);
    if (!texture)
        return nullptr;
    if (!device.Update_Texture(texture->Handle(), {color, 4})) {
        texture->Release();
        return nullptr;
    }
    return texture;
}

class QueueScope final
{
public:
    QueueScope() : m_queue(Get_Resource_Load_Queue()) { m_started = m_queue.Start(); }
    ~QueueScope() { if (m_started) m_queue.Shutdown(); }
    QueueScope(const QueueScope&) = delete;
    QueueScope& operator=(const QueueScope&) = delete;
    bool Started() const noexcept { return m_started; }

private:
    ResourceLoadQueue& m_queue;
    bool m_started = false;
};

class ReleaseOnExit final
{
public:
    ReleaseOnExit(ResourceLoadQueue& queue, std::atomic<bool>& release)
        : m_queue(queue), m_release(release) {}
    ~ReleaseOnExit()
    {
        m_release.store(true, std::memory_order_release);
        m_queue.Drain();
    }
    ReleaseOnExit(const ReleaseOnExit&) = delete;
    ReleaseOnExit& operator=(const ReleaseOnExit&) = delete;

private:
    ResourceLoadQueue& m_queue;
    std::atomic<bool>& m_release;
};

class BlockingJob final : public ResourceLoadJob
{
public:
    BlockingJob(std::atomic<bool>& started, std::atomic<bool>& release)
        : m_started(started), m_release(release) {}

    bool Prepare() override { return true; }
    bool Decode() override
    {
        m_started.store(true, std::memory_order_release);
        while (!m_release.load(std::memory_order_acquire))
            std::this_thread::yield();
        return true;
    }
    void Complete(bool) noexcept override {}

private:
    std::atomic<bool>& m_started;
    std::atomic<bool>& m_release;
};

}

BOOST_AUTO_TEST_CASE(eviction_is_strict_and_recent_reload_gets_the_threefold_extension)
{
    GraphicsTestDevice device({true});
    std::uint32_t now = 0;
    TextureResidency residency([&] { return now; });
    residency.Set_Inactivation_Time(10);
    residency.Set_Resource(Make_Solid_Texture(device, {std::byte{255}, std::byte{255},
        std::byte{255}, std::byte{255}}));
    residency.Set_Initialized(true);
    BOOST_REQUIRE(residency.Is_Resident());

    BOOST_CHECK(!residency.Evict_If_Old(10));
    BOOST_REQUIRE(residency.Evict_If_Old(11));
    BOOST_CHECK(!residency.Is_Resident());

    unsigned loads = 0;
    residency.Set_Initialize_Callback([&] {
        ++loads;
        residency.Publish(Make_Solid_Texture(device, {std::byte{255}, std::byte{255},
            std::byte{255}, std::byte{255}}), true);
    });
    BOOST_REQUIRE(residency.Ensure_At(15));
    BOOST_CHECK_EQUAL(loads, 1u);
    BOOST_CHECK_EQUAL(residency.Extended_Inactivation_Time(), 30u);
    BOOST_CHECK(!residency.Evict_If_Old(55));
    BOOST_REQUIRE(residency.Evict_If_Old(56));
}

BOOST_AUTO_TEST_CASE(pending_load_prevents_eviction_until_the_worker_finishes)
{
    QueueScope queue_scope;
    BOOST_REQUIRE(queue_scope.Started());
    auto& queue = Get_Resource_Load_Queue();
    std::atomic<bool> started = false;
    std::atomic<bool> release = false;
    ReleaseOnExit release_on_exit(queue, release);
    auto source = std::make_shared<const ResourceLoadSource>([&] {
        return std::make_unique<BlockingJob>(started, release);
    });

    GraphicsTestDevice device({true});
    TextureResidency residency([] { return 100u; });
    residency.Set_Resource(Make_Solid_Texture(device, {std::byte{255}, std::byte{255},
        std::byte{255}, std::byte{255}}));
    residency.Set_Initialized(true);
    residency.Set_Inactivation_Time(1);
    residency.Set_Load_Source(source);
    BOOST_REQUIRE(queue.Request(source, ResourceLoadPriority::Background));
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    while (!started.load(std::memory_order_acquire)
        && std::chrono::steady_clock::now() < deadline)
        std::this_thread::yield();
    BOOST_REQUIRE_MESSAGE(started.load(std::memory_order_acquire),
        "resource load worker did not start within the test deadline");

    BOOST_CHECK(!residency.Invalidate());
    BOOST_CHECK(residency.Is_Resident());
    release.store(true, std::memory_order_release);
    BOOST_REQUIRE(queue.Drain());
    BOOST_CHECK(!residency.Pending());
}

BOOST_AUTO_TEST_CASE(procedural_registration_is_opt_in)
{
    GraphicsTestDevice device({true});
    TextureResidency resident;
    resident.Set_Procedural(true);
    resident.Set_Resource(Make_Solid_Texture(device, {std::byte{255}, std::byte{255},
        std::byte{255}, std::byte{255}}));
    BOOST_REQUIRE(resident.Is_Resident());
    BOOST_CHECK(!resident.Is_Initialized());
    BOOST_REQUIRE(resident.Ensure());
    BOOST_CHECK(!resident.Is_Initialized());

    TextureResidency editable;
    editable.Set_Procedural(true);
    BOOST_CHECK(!editable.Recreation_Registered());

    TextureResidency recreated;
    recreated.Set_Procedural(true);
    recreated.Set_Recreate_Callback([] { return true; });
    recreated.Register_For_Recreation();
    BOOST_CHECK(recreated.Recreation_Registered());
}

BOOST_AUTO_TEST_CASE(evicted_texture_reloads_and_draws_through_the_native_renderer)
{
    for (const bool warp : {true, false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid())
            continue;

        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device,
            Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        const auto target = device.Create_Texture({1, 1, 1, RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto depth = device.Create_Texture({1, 1, 1, RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        BOOST_REQUIRE(target.Is_Valid());
        BOOST_REQUIRE(depth.Is_Valid());

        std::array<PropVertex, 4> vertices{};
        vertices[0].position = {-1, -1, 0.5f}; vertices[1].position = {1, -1, 0.5f};
        vertices[2].position = {1, 1, 0.5f}; vertices[3].position = {-1, 1, 0.5f};
        for (auto& vertex : vertices) {
            vertex.color = {1, 1, 1, 1};
            vertex.uv = {0.5f, 0.5f};
        }
        const auto mesh = renderer.Create_Mesh(vertices,
            std::array<std::uint32_t, 6>{0, 1, 2, 0, 2, 3});
        BOOST_REQUIRE(mesh.Is_Valid());
        PropParameters parameters;
        parameters.view_projection = {1, 0, 0, 0, 0, 1, 0, 0,
            0, 0, 1, 0, 0, 0, 0, 1};
        parameters.textured = 1;
        PropStyle style;
        style.blend = RHIBlendMode::Disabled;
        style.depth_test = false;
        style.depth_write = false;
        auto& commands = device.Immediate_Command_List();
        BOOST_REQUIRE(commands.Set_Render_Targets(target, depth));
        BOOST_REQUIRE(commands.Set_Viewport({0, 0, 1, 1}));

        std::uint32_t now = 0;
        TextureResidency residency([&] { return now; });
        residency.Set_Inactivation_Time(10);
        residency.Set_Resource(Make_Solid_Texture(device, {std::byte{0}, std::byte{0},
            std::byte{255}, std::byte{128}}));
        residency.Set_Initialized(true);
        const auto old_handle = residency.Handle();
        BOOST_REQUIRE(commands.Clear({0.05f, 0.1f, 0.15f, 0.2f}, 1));
        BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters,
            std::array{old_handle}));
        std::array<std::byte, 4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, 4));
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[0]) - 255, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[1]) - 0, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[2]) - 0, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3]) - 128, 2);

        now = 11;
        BOOST_REQUIRE(residency.Evict_If_Old(now));
        BOOST_CHECK(!residency.Handle().Is_Valid());
        residency.Set_Initialize_Callback([&] {
            residency.Publish(Make_Solid_Texture(device, {std::byte{0}, std::byte{255},
                std::byte{0}, std::byte{64}}), true);
        });
        now = 12;
        BOOST_REQUIRE(residency.Ensure_At(now));
        const auto new_handle = residency.Handle();
        BOOST_CHECK(new_handle != old_handle);
        BOOST_REQUIRE(commands.Clear({0.05f, 0.1f, 0.15f, 0.2f}, 1));
        BOOST_REQUIRE(renderer.Draw(commands, mesh, style, parameters,
            std::array{new_handle}));
        BOOST_REQUIRE(device.Readback_Texture(target, pixels, 4));
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[0]) - 0, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[1]) - 255, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[2]) - 0, 2);
        BOOST_CHECK_SMALL(std::to_integer<int>(pixels[3]) - 64, 2);

        renderer.Destroy_Mesh(mesh);
        renderer.Shutdown();
        BOOST_REQUIRE(device.Destroy_Texture(target));
        BOOST_REQUIRE(device.Destroy_Texture(depth));
    }
}
