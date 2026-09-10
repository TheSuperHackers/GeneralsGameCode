module;
#define NOMINMAX
#define BOOST_TEST_MODULE ResourceLoadQueueTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <chrono>
#include <cstddef>
#include <future>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>
export module Graphics.Resources.Loading.Queue.Tests;
import Graphics.Resources.Loading.Queue;
import Graphics.Resources.Textures.Upload;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
using namespace Graphics;

namespace
{
struct Record
{
    unsigned created=0,prepared=0,decoded=0,completed=0,destroyed=0;
    std::thread::id factory_thread,prepare_thread,decode_thread,complete_thread,destroy_thread;
    bool success=false;
};
struct Job final : ResourceLoadJob
{
    Record& record;
    bool prepare_ok=true,decode_ok=true,throw_decode=false;
    std::promise<void>* entered=nullptr;
    std::shared_future<void> release;
    explicit Job(Record& value) : record(value) {}
    ~Job() override { ++record.destroyed; record.destroy_thread=std::this_thread::get_id(); }
    bool Prepare() override { ++record.prepared; record.prepare_thread=std::this_thread::get_id(); return prepare_ok; }
    bool Decode() override {
        ++record.decoded; record.decode_thread=std::this_thread::get_id();
        if (entered) entered->set_value();
        if (release.valid()) release.wait();
        if (throw_decode) throw std::runtime_error("decode failure");
        return decode_ok;
    }
    void Complete(bool success) noexcept override {
        ++record.completed; record.complete_thread=std::this_thread::get_id(); record.success=success;
    }
};
auto Source(Record& record)
{
    return std::make_shared<const ResourceLoadSource>([&record] {
        ++record.created; record.factory_thread=std::this_thread::get_id();
        return std::make_unique<Job>(record);
    });
}
void Check_Owner_Phases(const Record& record, std::thread::id owner)
{
    BOOST_CHECK(record.factory_thread==owner);
    BOOST_CHECK(record.prepare_thread==owner);
    BOOST_CHECK(record.complete_thread==owner);
    BOOST_CHECK(record.destroy_thread==owner);
    BOOST_CHECK_EQUAL(record.created,1);
    BOOST_CHECK_EQUAL(record.completed,1);
    BOOST_CHECK_EQUAL(record.destroyed,1);
}
}

BOOST_AUTO_TEST_CASE(producer_requests_deduplicate_without_constructing_jobs_off_thread)
{
    ResourceLoadQueue queue;
    BOOST_REQUIRE(queue.Start());
    const auto owner=std::this_thread::get_id();
    Record record;
    const auto source=Source(record);
    bool producer_owner=true,accepted=false;
    std::thread producer([&] {
        producer_owner=queue.Is_Owner_Thread();
        accepted=true;
        for (unsigned i=0;i<20;++i) accepted=queue.Request(source,ResourceLoadPriority::Background) && accepted;
    });
    producer.join();
    BOOST_CHECK(accepted);
    BOOST_CHECK(!producer_owner);
    BOOST_CHECK_EQUAL(record.created,0);
    BOOST_CHECK(queue.Pending(source));
    BOOST_REQUIRE(queue.Drain());
    Check_Owner_Phases(record,owner);
    BOOST_CHECK_EQUAL(record.decoded,1);
    BOOST_CHECK(record.decode_thread!=owner);
    BOOST_CHECK(record.success);
    BOOST_CHECK(!queue.Pending(source));
    BOOST_REQUIRE(queue.Shutdown());
    BOOST_CHECK(!queue.Is_Owner_Thread());
}

BOOST_AUTO_TEST_CASE(expired_sources_are_discarded_before_factory_or_gpu_setup)
{
    ResourceLoadQueue queue;
    BOOST_REQUIRE(queue.Start());
    Record record;
    auto source=Source(record);
    bool accepted=false;
    std::thread producer([&] { accepted=queue.Request(source,ResourceLoadPriority::Immediate); });
    producer.join();
    BOOST_REQUIRE(accepted);
    source.reset();
    BOOST_REQUIRE(queue.Drain());
    BOOST_CHECK_EQUAL(record.created,0);
    BOOST_CHECK_EQUAL(record.prepared,0);
    BOOST_CHECK_EQUAL(record.completed,0);
}

BOOST_AUTO_TEST_CASE(prepare_and_decode_failures_complete_and_destroy_on_owner)
{
    ResourceLoadQueue queue;
    BOOST_REQUIRE(queue.Start());
    for (unsigned failure=0;failure<3;++failure) {
        Record record;
        const auto source=std::make_shared<const ResourceLoadSource>([&] {
            ++record.created; record.factory_thread=std::this_thread::get_id();
            auto job=std::make_unique<Job>(record);
            job->prepare_ok=failure!=0; job->decode_ok=false; job->throw_decode=failure==2;
            return job;
        });
        BOOST_REQUIRE(queue.Request(source,ResourceLoadPriority::Background));
        BOOST_REQUIRE(queue.Drain());
        Check_Owner_Phases(record,std::this_thread::get_id());
        BOOST_CHECK_EQUAL(record.decoded,failure==0 ? 0 : 1);
        BOOST_CHECK(!record.success);
    }
}

BOOST_AUTO_TEST_CASE(immediate_promotion_steals_waiting_work_without_double_decode)
{
    Record blocked,urgent;
    ResourceLoadQueue queue;
    BOOST_REQUIRE(queue.Start());
    // Promises are destroyed before the queue on assertion failure, releasing
    // the worker so cleanup cannot hang waiting for a test gate.
    std::promise<void> entered,release;
    const auto ready=entered.get_future();
    const auto blocker=std::make_shared<const ResourceLoadSource>([&] {
        ++blocked.created; blocked.factory_thread=std::this_thread::get_id();
        auto job=std::make_unique<Job>(blocked);
        job->entered=&entered; job->release=release.get_future().share();
        return job;
    });
    const auto source=Source(urgent);
    BOOST_REQUIRE(queue.Request(blocker,ResourceLoadPriority::Background));
    BOOST_REQUIRE(ready.wait_for(std::chrono::seconds(5))==std::future_status::ready);
    BOOST_REQUIRE(queue.Request(source,ResourceLoadPriority::Background));
    BOOST_REQUIRE(queue.Request(source,ResourceLoadPriority::Immediate));
    BOOST_CHECK_EQUAL(urgent.decoded,1);
    BOOST_CHECK(urgent.decode_thread==std::this_thread::get_id());
    BOOST_CHECK(urgent.success);
    BOOST_CHECK_EQUAL(blocked.completed,0);
    release.set_value();
    BOOST_REQUIRE(queue.Drain());
    BOOST_CHECK_EQUAL(urgent.decoded,1);
    BOOST_CHECK_EQUAL(blocked.decoded,1);
    Check_Owner_Phases(urgent,std::this_thread::get_id());
    Check_Owner_Phases(blocked,std::this_thread::get_id());
}

BOOST_AUTO_TEST_CASE(shutdown_drains_pending_jobs_and_allows_clean_restart)
{
    ResourceLoadQueue queue;
    for (unsigned cycle=0;cycle<2;++cycle) {
        Record record;
        const auto source=Source(record);
        BOOST_REQUIRE(queue.Start());
        bool accepted=false;
        std::thread producer([&] { accepted=queue.Request(source,ResourceLoadPriority::Background); });
        producer.join();
        BOOST_REQUIRE(accepted);
        BOOST_REQUIRE(queue.Shutdown());
        Check_Owner_Phases(record,std::this_thread::get_id());
        BOOST_CHECK(record.success);
        BOOST_CHECK(!queue.Request(source,ResourceLoadPriority::Immediate));
    }
}

BOOST_AUTO_TEST_CASE(worker_preparation_publishes_complete_texture_rgb_and_alpha)
{
    GraphicsTestDevice device({true});
    ResourceLoadQueue queue;
    BOOST_REQUIRE(queue.Start());
    PropRenderer renderer;
    BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
    const auto target=device.Create_Texture({1,1,1,RHITextureFormat::RGBA8_UNorm,
        static_cast<unsigned>(RHITextureUsage::RenderTarget)});
    const auto depth=device.Create_Texture({1,1,1,RHITextureFormat::D32_Float,
        static_cast<unsigned>(RHITextureUsage::DepthStencil)});
    auto& commands=device.Immediate_Command_List();
    BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
    BOOST_REQUIRE(commands.Set_Viewport({0,0,1,1}));
    std::array<PropVertex,4> vertices{};
    vertices[0].position={-1,-1,0.5f}; vertices[1].position={1,-1,0.5f};
    vertices[2].position={1,1,0.5f}; vertices[3].position={-1,1,0.5f};
    for (auto& vertex : vertices) { vertex.color={1,1,1,1}; vertex.uv={0.5f,0.5f}; }
    const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
    const auto mesh=renderer.Create_Mesh(vertices,indices);
    BOOST_REQUIRE(mesh.Is_Valid());
    PropStyle style;
    style.blend=RHIBlendMode::Disabled; style.depth_test=false; style.depth_write=false;
    PropParameters parameters;
    parameters.view_projection={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    RHITextureHandle published;
    struct ImageJob final : ResourceLoadJob {
        Device& device; RHITextureHandle& published; bool fail;
        TextureUpload upload; RHITextureHandle texture;
        ImageJob(Device& d,RHITextureHandle& p,bool f) : device(d),published(p),fail(f) {}
        ~ImageJob() override { if (texture.Is_Valid()) device.Destroy_Texture(texture); }
        bool Prepare() override { texture=device.Create_Texture({1,1,1}); return upload.Begin(device,texture,1); }
        bool Decode() override {
            const std::array<unsigned char,4> color=fail ? std::array<unsigned char,4>{255,0,0,255} : std::array<unsigned char,4>{32,96,160,80};
            const auto mapping=upload.Mapping(0);
            for (unsigned i=0;i<4;++i) mapping.bytes[i]=std::byte(color[i]);
            return !fail;
        }
        void Complete(bool decoded) noexcept override {
            const bool uploaded=upload.Finish();
            if (decoded && uploaded) {
                if (published.Is_Valid()) device.Destroy_Texture(published);
                published=texture; texture={};
            }
        }
    };
    for (unsigned pass=0;pass<3;++pass) {
        const bool fail=pass==1;
        const auto source=std::make_shared<const ResourceLoadSource>([&] { return std::make_unique<ImageJob>(device,published,fail); });
        BOOST_REQUIRE(queue.Request(source,ResourceLoadPriority::Background));
        if (pass==2) BOOST_REQUIRE(queue.Shutdown());
        else BOOST_REQUIRE(queue.Drain());
        BOOST_REQUIRE(published.Is_Valid());
        BOOST_REQUIRE(renderer.Draw(commands,mesh,style,parameters,std::array{published}));
        std::array<std::byte,4> pixels{};
        BOOST_REQUIRE(device.Readback_Texture(target,pixels,4));
        const std::array expected{32,96,160,80};
        for (unsigned i=0;i<4;++i) BOOST_CHECK_SMALL(std::to_integer<int>(pixels[i])-expected[i],1);
    }
    BOOST_REQUIRE(queue.Shutdown());
    renderer.Destroy_Mesh(mesh); renderer.Shutdown();
    device.Destroy_Texture(published); device.Destroy_Texture(target); device.Destroy_Texture(depth);
}
