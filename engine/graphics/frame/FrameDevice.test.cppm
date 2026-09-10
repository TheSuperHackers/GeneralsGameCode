module;
#define BOOST_TEST_MODULE FrameDeviceTests
#define NOMINMAX
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <vector>
#include <windows.h>
export module Graphics.Frame.Device.Tests;
import Graphics.Frame.Runtime;
import Graphics.FrameTargets;
import Graphics.Frame.AttachmentBindings;
import Graphics.Frame.ResourceLifecycle;
import Graphics.Resources.Recreation;
import Graphics.Resources.Loading.Queue;
import Graphics.Scene.Props.Renderer;
import Graphics.Diagnostics.Render;
import Graphics.Tests.Device;
using namespace Graphics;

namespace
{
struct Window final
{
    HWND handle = nullptr;
    Window()
    {
        WNDCLASSW description{};
        description.lpfnWndProc = DefWindowProcW;
        description.hInstance = GetModuleHandleW(nullptr);
        description.lpszClassName = L"GraphicsFrameDeviceTest";
        RegisterClassW(&description);
        handle = CreateWindowW(description.lpszClassName,L"",WS_OVERLAPPEDWINDOW,
            0,0,64,64,nullptr,nullptr,description.hInstance,nullptr);
    }
    ~Window() { if (handle) DestroyWindow(handle); }
};
struct Runtime final
{
    ~Runtime() { Graphics_Shutdown_Shared_Frame(); }
};
struct LoadQueue final
{
    ~LoadQueue() { Get_Resource_Load_Queue().Shutdown(); }
};
struct PendingImage final : ResourceLoadJob
{
    std::shared_future<void> permit;
    std::vector<int>& events;
    Device* owner = nullptr;
    RHITextureHandle texture;
    PendingImage(std::shared_future<void> signal, std::vector<int>& order) : permit(signal), events(order) {}
    bool Prepare() override
    {
        owner = Shared_Frame_Device();
        texture = owner->Create_Texture({1,1,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::ShaderResource)});
        return texture.Is_Valid();
    }
    bool Decode() override { permit.wait(); return true; }
    void Complete(bool decoded) noexcept override
    {
        BOOST_CHECK(decoded);
        BOOST_CHECK(Shared_Frame_Device() == owner);
        if (Shared_Frame_Device() != owner) return;
        events.push_back(0);
        const std::array<std::byte,4> bytes{};
        BOOST_CHECK(owner->Update_Texture(texture,{bytes,4,4}));
        BOOST_CHECK(owner->Destroy_Texture(texture));
    }
};
}

BOOST_AUTO_TEST_CASE(resize_retains_scene_resources_and_recreation_restores_pixels_in_owner_order)
{
    for (bool software : {true,false}) {
        Window window;
        BOOST_REQUIRE(window.handle);
        Runtime runtime;
        FrameDeviceOptions options;
        options.window = window.handle;
        options.width = options.height = 16;
        options.use_warp = Graphics_Test_Uses_WARP(software);
        options.backbuffer_format = RHITextureFormat::RGBA8_UNorm;
        BOOST_REQUIRE(Initialize_Frame_Device(options));
        BOOST_REQUIRE(Frame_Device_Ready());
        PropRenderer renderer;
        PropMeshHandle mesh;
        RHITextureHandle texture;
        std::vector<int> events;
        LoadQueue load_queue;
        BOOST_REQUIRE(Get_Resource_Load_Queue().Start());
        const auto acquire_renderer = [&] {
            BOOST_REQUIRE(Frame_Device_Ready());
            BOOST_REQUIRE(renderer.Initialize(*Shared_Frame_Device(),
                Frame_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
            std::array<PropVertex,4> vertices{};
            vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
            vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
            for (auto& vertex : vertices) vertex.color = {1,1,1,1};
            mesh = renderer.Create_Mesh(vertices,std::array<std::uint32_t,6>{0,1,2,0,2,3});
            BOOST_REQUIRE(mesh.Is_Valid());
        };
        const auto acquire_texture = [&] {
            BOOST_REQUIRE(Frame_Device_Ready());
            texture = Shared_Frame_Device()->Create_Texture({1,1,1,RHITextureFormat::RGBA8_UNorm,
                static_cast<unsigned>(RHITextureUsage::ShaderResource)});
            BOOST_REQUIRE(texture.Is_Valid());
            const std::array<std::byte,4> bytes{std::byte{64},std::byte{128},std::byte{192},std::byte{128}};
            BOOST_REQUIRE(Shared_Frame_Device()->Update_Texture(texture,{bytes,4,4}));
        };
        auto frame_registration = Get_Frame_Resource_Lifecycle().Register([&] {
            events.push_back(1);
            BOOST_REQUIRE(Shared_Frame_Device());
            BOOST_CHECK(!Graphics_Begin_Frame());
            BOOST_CHECK(!Recreate_Frame_Device());
            renderer.Shutdown();
        }, [&] {
            events.push_back(3);
            acquire_renderer();
        });
        auto texture_registration = Get_Resource_Recreation_Registry().Register([&] {
            events.push_back(2);
            BOOST_REQUIRE(Shared_Frame_Device());
            BOOST_REQUIRE(Shared_Frame_Device()->Destroy_Texture(texture));
            texture = {};
        }, [&] {
            events.push_back(4);
            acquire_texture();
        });
        acquire_renderer();
        acquire_texture();
        const auto draw = [&] {
            BOOST_REQUIRE(Graphics_Begin_Frame());
            BOOST_CHECK(!Recreate_Frame_Device());
            auto* device = Shared_Frame_Device();
            auto& bindings = Get_Attachment_Bindings();
            const auto screen = bindings.Default().viewport;
            // Camera and screen-space callers use the default frame extent,
            // even after another view selected a smaller drawing viewport.
            BOOST_REQUIRE(bindings.Set_Viewport({1,2,3,4,0.2f,0.8f}));
            BOOST_CHECK_EQUAL(bindings.Default().viewport.width,screen.width);
            BOOST_CHECK_EQUAL(bindings.Default().viewport.height,screen.height);
            BOOST_REQUIRE(bindings.Restore_Default());
            bindings.Clear(true,true,{0,0,0,0});
            PropStyle style;
            style.blend = RHIBlendMode::Disabled;
            PropParameters parameters;
            parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
            BOOST_REQUIRE(renderer.Draw(device->Immediate_Command_List(),mesh,style,parameters,std::array{texture}));
            BOOST_REQUIRE(Graphics_Execute_Queued_Draws());
            const auto target = device->Get_Swap_Chain().Backbuffer();
            BOOST_CHECK_EQUAL(screen.width,target.width);
            BOOST_CHECK_EQUAL(screen.height,target.height);
            std::vector<std::byte> pixels(target.width * target.height * 4);
            BOOST_REQUIRE(device->Readback_Texture(target.texture,pixels,target.width * 4));
            const auto middle = (target.height / 2 * target.width + target.width / 2) * 4;
            const std::array expected{64,128,192,128};
            for (unsigned c = 0; c < 4; ++c)
                BOOST_CHECK_SMALL(std::to_integer<int>(pixels[middle + c]) - expected[c],2);
            for (const unsigned pixel : {0u,target.width-1,
                (target.height-1)*target.width,target.width*target.height-1}) {
                for (unsigned c = 0; c < 4; ++c)
                    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[pixel*4+c])-expected[c],2);
            }
            BOOST_REQUIRE(Graphics_End_Frame());
            BOOST_REQUIRE(Graphics_Present());
        };
        const auto initial_target_identity = Shared_Frame_Targets().identity;
        BOOST_CHECK_NE(initial_target_identity, 0u);
        draw();
        BOOST_CHECK_EQUAL(Shared_Frame_Targets().identity, initial_target_identity);
        auto* original_device = Shared_Frame_Device();
        const auto original_texture = texture;
        {
            auto retained = Get_Attachment_Bindings().Capture();
            BOOST_CHECK(!Resize_Frame_Device(24,12,false));
            BOOST_REQUIRE(Frame_Device_Ready());
            BOOST_CHECK_EQUAL(Get_Attachment_Bindings().Default().viewport.width,16u);
            BOOST_CHECK_EQUAL(Get_Attachment_Bindings().Default().viewport.height,16u);
            BOOST_CHECK(Get_Attachment_Bindings().Default().color == retained.Selection().color);
            draw();
        }
        BOOST_REQUIRE(Recover_Frame_Device());
        BOOST_CHECK_EQUAL(Get_Attachment_Bindings().Default().viewport.width,24u);
        BOOST_CHECK_EQUAL(Get_Attachment_Bindings().Default().viewport.height,12u);
        BOOST_CHECK(Shared_Frame_Device() == original_device);
        BOOST_CHECK(texture == original_texture);
        BOOST_CHECK(events.empty());
        draw();
        for (unsigned width : {24u,16u}) {
            const auto previous_identity = Shared_Frame_Targets().identity;
            BOOST_REQUIRE(Resize_Frame_Device(width,16,false));
            BOOST_CHECK_NE(Shared_Frame_Targets().identity, previous_identity);
            BOOST_CHECK(Shared_Frame_Device() == original_device);
            BOOST_CHECK(texture == original_texture);
            BOOST_CHECK(events.empty());
            BOOST_CHECK_EQUAL(Get_Attachment_Bindings().Default().viewport.width,width);
            draw();
        }
        std::promise<void> permit;
        const auto signal = permit.get_future().share();
        auto source = std::make_shared<const ResourceLoadSource>([&events,signal] {
            return std::make_unique<PendingImage>(signal,events);
        });
        BOOST_REQUIRE(Get_Resource_Load_Queue().Request(source,ResourceLoadPriority::Background));
        BOOST_REQUIRE(Get_Resource_Load_Queue().Pending(source));
        permit.set_value();
        Get_Render_Diagnostics().disable_water = true;
        Get_Render_Diagnostics().console_line_limit = 7;
        const auto previous_identity = Shared_Frame_Targets().identity;
        BOOST_REQUIRE(Recreate_Frame_Device());
        BOOST_CHECK_NE(Shared_Frame_Targets().identity, previous_identity);
        BOOST_CHECK(Get_Render_Diagnostics().disable_water);
        BOOST_CHECK_EQUAL(Get_Render_Diagnostics().console_line_limit,7);
        Get_Render_Diagnostics() = {};
        BOOST_CHECK(!Get_Resource_Load_Queue().Pending(source));
        BOOST_CHECK(events == std::vector<int>({0,1,2,3,4}));
        draw();
        events.clear();
        // A failed replacement must leave restoration pending. A valid new
        // window resumes the same owners without invoking release twice.
        BOOST_REQUIRE(DestroyWindow(window.handle));
        BOOST_REQUIRE(!IsWindow(window.handle));
        window.handle = nullptr;
        BOOST_CHECK(!Recreate_Frame_Device());
        BOOST_CHECK(!Frame_Device_Ready());
        BOOST_CHECK(Shared_Frame_Device() == nullptr);
        BOOST_CHECK(events == std::vector<int>({1,2}));
        Window replacement;
        BOOST_REQUIRE(replacement.handle);
        options.window = replacement.handle;
        BOOST_REQUIRE(Initialize_Frame_Device(options));
        BOOST_CHECK(events == std::vector<int>({1,2,3,4}));
        draw();
        events.clear();
        Graphics_Shutdown_Shared_Frame();
        BOOST_CHECK(events == std::vector<int>({1,2}));
        BOOST_CHECK(!Frame_Device_Ready());
        BOOST_CHECK(!Get_Attachment_Bindings().Default().color.Is_Valid());
        Graphics_Shutdown_Shared_Frame();
        BOOST_CHECK(events == std::vector<int>({1,2}));
    }
}
