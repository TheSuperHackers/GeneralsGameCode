module;
#define BOOST_TEST_MODULE OrderedDrawTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>
export module Graphics.Scene.OrderedDraws.Tests;
import Graphics.Scene.OrderedDraws;
import Graphics.Tests.Device;
import Graphics.Scene.Props.Renderer;
using namespace Graphics;

namespace {
struct Object final {
    unsigned value;
    unsigned references = 1;
    unsigned* deaths = nullptr;
    bool visible = true;
    void Add_Ref() { ++references; }
    void Release_Ref() { if (--references == 0) { if (deaths) ++*deaths; delete this; } }
};
struct Recording final {
    OrderedDrawQueue* queue;
    std::vector<unsigned> values;
};
bool Record(Object& object, void* context) {
    auto& recording = *static_cast<Recording*>(context);
    BOOST_CHECK(!recording.queue->Is_Enabled());
    BOOST_CHECK(!recording.queue->Enqueue<Record>(1, object));
    BOOST_CHECK(!recording.queue->Clear());
    BOOST_CHECK(!recording.queue->Drain(context, [] {}));
    recording.values.push_back(object.value);
    return object.visible;
}
bool Throw(Object&, void*) { throw std::runtime_error("draw extraction failed"); }
}

BOOST_AUTO_TEST_CASE(authored_order_duplicates_suppression_and_reentrant_calls_preserve_lifetime)
{
    OrderedDrawQueue queue;
    Object a{1}, b{2}, c{3};
    c.visible = false;
    Recording recording{&queue};
    BOOST_CHECK(!queue.Enqueue<Record>(1, a));
    queue.Set_Enabled(true);
    BOOST_CHECK(!queue.Enqueue<Record>(0, a));
    BOOST_REQUIRE(queue.Enqueue<Record>(1, a));
    BOOST_REQUIRE(queue.Enqueue<Record>(9, b));
    BOOST_REQUIRE(queue.Enqueue<Record>(9, a));
    BOOST_REQUIRE(queue.Enqueue<Record>(9, b));
    BOOST_REQUIRE(queue.Enqueue<Record>(5, c));
    BOOST_CHECK_EQUAL(a.references, 3u);
    BOOST_CHECK_EQUAL(b.references, 3u);
    unsigned flushes = 0;
    BOOST_REQUIRE(queue.Drain(&recording, [&] {
        ++flushes;
        // The completed layer releases all retained sources before flushing.
        BOOST_CHECK_EQUAL(b.references, 1u);
        BOOST_CHECK_EQUAL(a.references, flushes == 1 ? 2u : 1u);
    }));
    const std::vector<unsigned> expected{2,1,2,3,1};
    BOOST_CHECK(recording.values == expected);
    BOOST_CHECK_EQUAL(flushes, 2u);
    BOOST_CHECK_EQUAL(c.references, 1u);
    BOOST_CHECK(queue.Is_Enabled());
    BOOST_REQUIRE(queue.Drain(&recording, [&] { ++flushes; }));
    BOOST_CHECK_EQUAL(flushes, 2u);

    // Disabling new submissions must not discard already retained work.
    BOOST_REQUIRE(queue.Enqueue<Record>(3, c));
    queue.Set_Enabled(false);
    BOOST_REQUIRE(queue.Drain(&recording, [&] { ++flushes; }));
    BOOST_CHECK(!queue.Is_Enabled());
    BOOST_CHECK_EQUAL(recording.values.back(), 3u);
    BOOST_CHECK_EQUAL(c.references, 1u);
}

BOOST_AUTO_TEST_CASE(pending_objects_survive_external_release_and_large_layers_have_no_budget)
{
    unsigned deaths = 0;
    {
        OrderedDrawQueue queue;
        queue.Set_Enabled(true);
        auto* object = new Object{7,1,&deaths};
        for (unsigned i = 0; i < 12000; ++i) BOOST_REQUIRE(queue.Enqueue<Record>(4000000000u, *object));
        object->Release_Ref();
        BOOST_CHECK_EQUAL(deaths, 0u);
        Recording recording{&queue};
        unsigned flushes = 0;
        BOOST_REQUIRE(queue.Drain(&recording, [&] { ++flushes; BOOST_CHECK_EQUAL(deaths, 1u); }));
        BOOST_CHECK_EQUAL(recording.values.size(), 12000u);
        BOOST_CHECK_EQUAL(flushes, 1u);
        object = new Object{8,1,&deaths};
        BOOST_REQUIRE(queue.Enqueue<Record>(2, *object));
        object->Release_Ref();
        BOOST_REQUIRE(queue.Clear());
        BOOST_CHECK_EQUAL(deaths, 2u);
        object = new Object{9,1,&deaths};
        BOOST_REQUIRE(queue.Enqueue<Record>(3, *object));
        object->Release_Ref();
    }
    BOOST_CHECK_EQUAL(deaths, 3u);
}

BOOST_AUTO_TEST_CASE(failed_extraction_releases_current_entry_and_restores_queue_state)
{
    OrderedDrawQueue queue;
    queue.Set_Enabled(true);
    unsigned deaths = 0;
    auto* object = new Object{1,1,&deaths};
    BOOST_REQUIRE(queue.Enqueue<Throw>(2, *object));
    object->Release_Ref();
    Object pending{2};
    BOOST_REQUIRE(queue.Enqueue<Record>(1, pending));
    unsigned flushes = 0;
    BOOST_CHECK_THROW(queue.Drain(nullptr, [&] { ++flushes; }), std::runtime_error);
    BOOST_CHECK_EQUAL(deaths, 1u);
    BOOST_CHECK(queue.Is_Enabled());
    BOOST_CHECK_EQUAL(flushes, 0u);
    Recording recording{&queue};
    BOOST_REQUIRE(queue.Drain(&recording, [&] { ++flushes; }));
    BOOST_CHECK_EQUAL(recording.values.size(), 1u);
    BOOST_CHECK_EQUAL(recording.values.front(), 2u);
    BOOST_CHECK_EQUAL(pending.references, 1u);
    BOOST_CHECK_EQUAL(flushes, 1u);
}

namespace {
struct Drawing final {
    PropRenderer* renderer;
    CommandList* commands;
};
bool Draw(Object& object, void* context) {
    if (!object.visible) return false;
    auto& drawing = *static_cast<Drawing*>(context);
    std::array<PropVertex,3> vertices{};
    vertices[0].position = {-1,-1,.5f}; vertices[1].position = {3,-1,.5f}; vertices[2].position = {-1,3,.5f};
    for (auto& vertex : vertices) vertex.color = object.value
        ? std::array<float,4>{0,1,0,.5f} : std::array<float,4>{1,0,0,.5f};
    PropStyle style;
    style.depth_test = false; style.depth_write = false;
    style.source_blend = RHIBlendFactor::SourceAlpha;
    style.destination_blend = RHIBlendFactor::InverseSourceAlpha;
    PropParameters parameters;
    parameters.textured = 0;
    parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
    const auto mesh = drawing.renderer->Create_Mesh(vertices, std::array<std::uint32_t,3>{0,1,2});
    const bool drawn = drawing.renderer->Draw(*drawing.commands, mesh, style, parameters, {});
    drawing.renderer->Destroy_Mesh(mesh);
    BOOST_REQUIRE(drawn);
    return drawn;
}
}

BOOST_AUTO_TEST_CASE(alpha_drawing_retains_layer_priority_and_equal_layer_submission_order_after_resize)
{
    for (const bool warp : {true,false}) {
        GraphicsTestDevice device({warp});
        if (!warp && !device.Is_Valid()) continue;
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device, Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        auto& commands = device.Immediate_Command_List();
        OrderedDrawQueue queue;
        queue.Set_Enabled(true);
        Object red{0}, green{1}, suppressed{0};
        suppressed.visible = false;
        Drawing drawing{&renderer,&commands};
        for (const unsigned size : {1,17,5}) {
            const auto target = device.Create_Texture({size,size,1,RHITextureFormat::RGBA8_UNorm,
                static_cast<unsigned>(RHITextureUsage::RenderTarget)});
            const auto depth = device.Create_Texture({size,size,1,RHITextureFormat::D32_Float,
                static_cast<unsigned>(RHITextureUsage::DepthStencil)});
            BOOST_REQUIRE(commands.Set_Render_Targets(target,depth));
            BOOST_REQUIRE(commands.Set_Viewport({0,0,size,size}));
            for (unsigned pass = 0; pass < 2; ++pass) {
                BOOST_REQUIRE(commands.Clear({0,0,0,0},1));
                BOOST_REQUIRE(queue.Enqueue<Draw>(3,red));
                BOOST_REQUIRE(queue.Enqueue<Draw>(pass ? 7 : 3,green));
                BOOST_REQUIRE(queue.Enqueue<Draw>(1,suppressed));
                unsigned flushes = 0;
                BOOST_REQUIRE(queue.Drain(&drawing, [&] { ++flushes; }));
                BOOST_CHECK_EQUAL(flushes, pass ? 2u : 1u);
                std::vector<std::byte> pixels(size*size*4);
                BOOST_REQUIRE(device.Readback_Texture(target,pixels,size*4));
                for (unsigned i = 0; i < size*size; ++i) {
                    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[i*4]) - (pass ? 128 : 64), 1);
                    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[i*4+1]) - (pass ? 64 : 128), 1);
                    BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[i*4+2]), 0u);
                    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[i*4+3]) - 96, 1);
                }
            }
            device.Destroy_Texture(target); device.Destroy_Texture(depth);
        }
        renderer.Shutdown();
    }
}
