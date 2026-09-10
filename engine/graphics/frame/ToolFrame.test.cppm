module;
#define BOOST_TEST_MODULE ToolFrameTests
#define NOMINMAX
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstddef>
#include <filesystem>
#include <vector>
#include <windows.h>

export module Graphics.Frame.ToolFrame.Tests;
import Graphics.Frame.ToolFrame;
import Graphics.Frame.Runtime;
import Graphics.Frame.AttachmentBindings;
import Graphics.Renderer2D;
import Graphics.Tests.Device;

using namespace Graphics;

namespace
{
struct Runtime final
{
    HWND window = nullptr;
    Runtime()
    {
        WNDCLASSW description{};
        description.lpfnWndProc = DefWindowProcW;
        description.hInstance = GetModuleHandleW(nullptr);
        description.lpszClassName = L"GraphicsToolFrameTest";
        RegisterClassW(&description);
        window = CreateWindowW(description.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
            0, 0, 64, 64, nullptr, nullptr, description.hInstance, nullptr);
    }
    ~Runtime()
    {
        Shutdown_Tool_Frame();
        Graphics_Shutdown_Shared_Frame();
        if (window) DestroyWindow(window);
    }
};
}

BOOST_AUTO_TEST_CASE(tool_overlay_pixels_survive_abort_resize_and_device_recreation)
{
    for (const bool software : {true, false}) {
        Runtime runtime;
        BOOST_REQUIRE(runtime.window);
        FrameDeviceOptions options;
        options.window = runtime.window;
        options.width = options.height = 16;
        options.use_warp = Graphics_Test_Uses_WARP(software);
        options.backbuffer_format = RHITextureFormat::RGBA8_UNorm;
        BOOST_REQUIRE(Initialize_Frame_Device(options));
        const std::filesystem::path shaders =
            Frame_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
        BOOST_CHECK(!Initialize_Tool_Frame(shaders / "missing"));
        BOOST_CHECK(!Get_Renderer2D().Is_Initialized());
        BOOST_REQUIRE(Initialize_Tool_Frame(shaders));

        for (const unsigned size : {16u, 32u, 24u}) {
            BOOST_REQUIRE(Resize_Frame_Device(size, size, false));
            BOOST_REQUIRE(Begin_Tool_Frame());
            BOOST_REQUIRE(Get_Renderer2D().Add_Rect({0, 0, float(size), float(size)}, {1, 0, 0, 1}));
            const auto vertices = Get_Renderer2D().Vertex_Count();
            BOOST_CHECK(!Begin_Tool_Frame());
            BOOST_CHECK_EQUAL(Get_Renderer2D().Vertex_Count(), vertices);
            BOOST_REQUIRE(Get_Renderer2D().Add_Rect({float(size / 2), 0, float(size), float(size)}, {0, 1, 0, 1}));
            // Inspect the queued executor's pixels before presentation can
            // discard the swap-chain contents.
            BOOST_REQUIRE(Graphics_Execute_Queued_Draws());
            const auto target = Get_Attachment_Bindings().Default().color;
            std::vector<std::byte> pixels(size * size * 4);
            BOOST_REQUIRE(Shared_Frame_Device()->Readback_Texture(target, pixels, size * 4));
            for (unsigned y = 0; y < size; ++y) for (unsigned x = 0; x < size; ++x) {
                const unsigned offset = (y * size + x) * 4;
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset]), x < size / 2 ? 255u : 0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset + 1]), x < size / 2 ? 0u : 255u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset + 2]), 0u);
                BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[offset + 3]), 255u);
            }
            BOOST_REQUIRE(Graphics_End_Frame());
            BOOST_REQUIRE(Graphics_Present());

            BOOST_REQUIRE(Begin_Tool_Frame());
            BOOST_REQUIRE(Get_Renderer2D().Add_Rect({0, 0, float(size), float(size)}, {0, 0, 1, 1}));
            Abort_Tool_Frame();
            BOOST_CHECK(!Get_Renderer2D().Has_Draws());
            BOOST_CHECK(!End_Tool_Frame());
            BOOST_REQUIRE(Begin_Tool_Frame());
            BOOST_CHECK(!Get_Renderer2D().Has_Draws());
            BOOST_REQUIRE(Get_Renderer2D().Add_Rect({0, 0, float(size), float(size)}, {1, 1, 1, 1}));
            BOOST_REQUIRE(End_Tool_Frame());

            BOOST_REQUIRE(Recreate_Frame_Device());
            BOOST_CHECK(!Get_Renderer2D().Is_Initialized());
            BOOST_REQUIRE(Begin_Tool_Frame());
            BOOST_CHECK(Get_Renderer2D().Is_Initialized());
            BOOST_REQUIRE(Get_Renderer2D().Add_Rect({0, 0, float(size), float(size)}, {1, 0, 0, 1}));
            BOOST_REQUIRE(Graphics_Execute_Queued_Draws());
            BOOST_REQUIRE(Shared_Frame_Device()->Readback_Texture(
                Get_Attachment_Bindings().Default().color, pixels, size * 4));
            const unsigned corner = (size * size - 1) * 4;
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[corner]), 255u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[corner + 1]), 0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[corner + 2]), 0u);
            BOOST_CHECK_EQUAL(std::to_integer<unsigned>(pixels[corner + 3]), 255u);
            Abort_Tool_Frame();
        }
        Shutdown_Tool_Frame();
        BOOST_CHECK(!Get_Renderer2D().Is_Initialized());
        BOOST_REQUIRE(Recreate_Frame_Device());
        BOOST_CHECK(!Get_Renderer2D().Is_Initialized());
        BOOST_REQUIRE(Begin_Tool_Frame());
        BOOST_REQUIRE(End_Tool_Frame());
    }
}
