module;
#define BOOST_TEST_MODULE FrameDrawingTests
#define NOMINMAX
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <windows.h>
export module Graphics.Frame.Drawing.Tests;
import Graphics.FrameOwner;
import Graphics.Frame.AttachmentBindings;
import Graphics.Scene.Props.Renderer;
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
        description.lpszClassName = L"GraphicsFrameDrawingTest";
        RegisterClassW(&description);
        handle = CreateWindowW(description.lpszClassName,L"",WS_OVERLAPPEDWINDOW,
            0,0,64,64,nullptr,nullptr,description.hInstance,nullptr);
    }
    ~Window() { if (handle) DestroyWindow(handle); }
};
struct QueuedDraw final
{
    PropRenderer* renderer;
    PropMeshHandle mesh;
    PropStyle style;
    PropParameters parameters;
    bool fail = false;
};
QueuedDraw* queued_draw = nullptr;
bool Execute_Overlay(Device&,CommandList& commands,const FrameTargets& targets) noexcept
{
    const bool drawn = commands.Set_Viewport({0,0,targets.backbuffer.width/2,targets.backbuffer.height})
        && queued_draw->renderer->Draw(commands,queued_draw->mesh,queued_draw->style,queued_draw->parameters,{});
    return drawn && !queued_draw->fail;
}
}

BOOST_AUTO_TEST_CASE(scene_and_queued_pixels_survive_offscreen_passes_and_failed_frames_recover)
{
    for (bool software : {true,false}) {
        Window window;
        BOOST_REQUIRE(window.handle != nullptr);
        GraphicsTestDeviceOptions options;
        options.window = window.handle;
        options.width = options.height = 32;
        options.use_warp = software;
        options.backbuffer_format = RHITextureFormat::RGBA8_UNorm;
        GraphicsTestDevice device(options);
        BOOST_REQUIRE(device.Is_Valid());
        PropRenderer renderer;
        BOOST_REQUIRE(renderer.Initialize(device,Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY)));
        auto& commands = device.Immediate_Command_List();
        const auto color = device.Get_Swap_Chain().Backbuffer();
        const auto depth = device.Get_Swap_Chain().Depth_Target();
        AttachmentBindings attachments;
        BOOST_REQUIRE(attachments.Initialize(device,{color.texture,depth.texture,{0,0,32,32}}));
        const auto offscreen = device.Create_Texture({8,8,1,RHITextureFormat::RGBA8_UNorm,
            static_cast<unsigned>(RHITextureUsage::RenderTarget)});
        const auto offscreen_depth = device.Create_Texture({8,8,1,RHITextureFormat::D32_Float,
            static_cast<unsigned>(RHITextureUsage::DepthStencil)});
        std::array<PropVertex,4> vertices{};
        vertices[0].position = {-1,-1,0.5f}; vertices[1].position = {1,-1,0.5f};
        vertices[2].position = {1,1,0.5f}; vertices[3].position = {-1,1,0.5f};
        const std::array<std::uint32_t,6> indices{0,1,2,0,2,3};
        for (auto& vertex : vertices) vertex.color = {1,0,0,0.8f};
        const auto scene_mesh = renderer.Create_Mesh(vertices,indices);
        for (auto& vertex : vertices) vertex.color = {0,1,0,0.5f};
        const auto overlay_mesh = renderer.Create_Mesh(vertices,indices);
        PropStyle scene_style;
        scene_style.blend = RHIBlendMode::Disabled;
        PropParameters parameters;
        parameters.view_projection = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        parameters.textured = 0;
        QueuedDraw overlay{&renderer,overlay_mesh,{},parameters};
        overlay.style.source_blend = RHIBlendFactor::SourceAlpha;
        overlay.style.destination_blend = RHIBlendFactor::InverseSourceAlpha;
        queued_draw = &overlay;
        FrameOwner owner;
        BOOST_REQUIRE(owner.Set_Draw_Executor(Execute_Overlay));
        for (bool fail : {false,true,false}) {
            overlay.fail = fail;
            BOOST_REQUIRE(owner.Begin_Frame(device));
            const auto color = device.Get_Swap_Chain().Backbuffer();
            const auto depth = device.Get_Swap_Chain().Depth_Target();
            BOOST_REQUIRE(attachments.Initialize(device,{color.texture,depth.texture,{0,0,32,32}}));
            BOOST_REQUIRE(attachments.Restore_Default());
            attachments.Clear(true,true,{0,0,0,0});
            BOOST_REQUIRE(renderer.Draw(commands,scene_mesh,scene_style,parameters,{}));
            {
                const auto saved = attachments.Capture();
                BOOST_REQUIRE(attachments.Bind({offscreen,offscreen_depth,{0,0,8,8}}));
                attachments.Clear(true,true,{0,0,1,1});
                BOOST_REQUIRE(attachments.Restore(saved));
            }
            BOOST_CHECK_EQUAL(owner.Execute_Queued_Draws(device),!fail);
            std::array<std::byte,32*32*4> pixels{};
            BOOST_REQUIRE(device.Readback_Texture(color.texture,pixels,32*4));
            const auto check = [&](unsigned x,std::array<int,4> expected) {
                for (unsigned channel=0;channel<4;++channel)
                    BOOST_CHECK_SMALL(std::to_integer<int>(pixels[(16*32+x)*4+channel])-expected[channel],2);
            };
            check(8,{128,128,0,166});
            check(24,{255,0,0,204});
            if (fail) {
                BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Failed);
                BOOST_CHECK(!owner.End_Frame(device));
                BOOST_CHECK(!owner.Present(device));
                owner.Abort(device);
            } else {
                BOOST_REQUIRE(owner.End_Frame(device));
                BOOST_REQUIRE(owner.Present(device));
            }
            BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Idle);
        }
        queued_draw = nullptr;
        renderer.Destroy_Mesh(scene_mesh); renderer.Destroy_Mesh(overlay_mesh);
        renderer.Shutdown();
        attachments.Reset();
        device.Destroy_Texture(offscreen); device.Destroy_Texture(offscreen_depth);
    }
}
