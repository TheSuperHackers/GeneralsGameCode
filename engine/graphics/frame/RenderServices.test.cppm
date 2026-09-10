module;

#define BOOST_TEST_MODULE GraphicsRenderServicesTests
#define NOMINMAX

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>
#include <windows.h>

export module Graphics.Frame.RenderServices.Tests;

import Graphics.Frame.AttachmentBindings;
import Graphics.Frame.RenderServices;
import Graphics.Frame.Runtime;
import Graphics.Resources.Loading.Queue;
import Graphics.RHI;
import Graphics.Scene.OrderedDraws;
import Graphics.Scene.Props.Renderer;
import Graphics.Scene.Props.Submission;
import Graphics.Scene.Shadows.DirectionalRenderer;
import Graphics.Tests.Device;

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
		description.lpszClassName = L"GraphicsRenderServicesTest";
		RegisterClassW(&description);
		handle = CreateWindowW(description.lpszClassName, L"", WS_OVERLAPPEDWINDOW,
			0, 0, 32, 32, nullptr, nullptr, description.hInstance, nullptr);
	}

	~Window()
	{
		if (handle != nullptr)
			DestroyWindow(handle);
	}
};

struct Runtime final
{
	Graphics::PropRenderer renderer;
	Graphics::DirectionalShadowRenderer shadows;
	bool frame_initialized = false;
	bool frame_active = false;

	~Runtime()
	{
		auto &services = Graphics::Get_Render_Services();
		if (services.Is_Initialized())
			services.Shutdown();
		if (frame_active)
			Graphics::Graphics_Abort_Frame();
		Graphics::Get_Prop_Submission().Shutdown();
		shadows.Shutdown();
		renderer.Shutdown();
		if (frame_initialized)
			Graphics::Graphics_Shutdown_Shared_Frame();
	}
};

struct OrderedProbe final
{
	std::vector<unsigned> &events;
	unsigned id;
	unsigned references = 0;

	void Add_Ref() noexcept { ++references; }
	void Release_Ref() noexcept { --references; }
};

struct CallbackProbe final
{
	unsigned resource_progress = 0;
	unsigned evict_unused_textures = 0;
	unsigned invalidate_textures = 0;
	unsigned release_assets = 0;
	bool shutdown_request_accepted = false;
	unsigned shutdown_job_completions = 0;
};

class CallbackResourceJob final : public Graphics::ResourceLoadJob
{
public:
	explicit CallbackResourceJob(bool delay_prepare,
		CallbackProbe *completion_probe = nullptr) noexcept
		: m_delay_prepare(delay_prepare), m_completion_probe(completion_probe)
	{
	}

	bool Prepare() override
	{
		if (m_delay_prepare)
			std::this_thread::sleep_for(std::chrono::milliseconds(25));
		return true;
	}

	bool Decode() override
	{
		return true;
	}

	void Complete(bool) noexcept override
	{
		if (m_completion_probe != nullptr)
			++m_completion_probe->shutdown_job_completions;
	}

private:
	bool m_delay_prepare = false;
	CallbackProbe *m_completion_probe = nullptr;
};

CallbackProbe *active_callback_probe = nullptr;

void Resource_Progress()
{
	++active_callback_probe->resource_progress;
}

void Evict_Unused_Textures()
{
	++active_callback_probe->evict_unused_textures;
}

void Invalidate_Textures()
{
	++active_callback_probe->invalidate_textures;
}

void Release_Assets()
{
	++active_callback_probe->release_assets;
	const auto source = std::make_shared<Graphics::ResourceLoadSource>([] {
		return std::make_unique<CallbackResourceJob>(false, active_callback_probe);
	});
	active_callback_probe->shutdown_request_accepted =
		Graphics::Get_Resource_Load_Queue().Request(source,
			Graphics::ResourceLoadPriority::Immediate);
}

bool Draw_Probe(OrderedProbe &probe, void *)
{
	probe.events.push_back(probe.id);
	return true;
}

}

BOOST_AUTO_TEST_CASE(render_services_prepare_inside_the_active_frame_and_flush_ordered_draws)
{
	for (const bool software : {true, false})
	{
		Window window;
		BOOST_REQUIRE(window.handle != nullptr);
		CallbackProbe callback_probe;
		std::vector<unsigned> pre_render_events;
		OrderedProbe pre_render_probe{pre_render_events, 3u};
		std::vector<unsigned> events;
		OrderedProbe first{events, 1u};
		OrderedProbe second{events, 2u};
		Runtime runtime;

		Graphics::FrameDeviceOptions options;
		options.window = window.handle;
		options.width = 16;
		options.height = 16;
		options.use_warp = Graphics::Graphics_Test_Uses_WARP(software);
		options.backbuffer_format = Graphics::RHITextureFormat::RGBA8_UNorm;
		BOOST_REQUIRE(Graphics::Initialize_Frame_Device(options));
		runtime.frame_initialized = true;
		BOOST_REQUIRE(Graphics::Frame_Device_Ready());

		const auto shader_directory =
			Graphics::Test_Shader_Directory(GRAPHICS_TERRAIN_SHADER_DIRECTORY);
		BOOST_REQUIRE(runtime.renderer.Initialize(*Graphics::Shared_Frame_Device(),
			shader_directory));
		BOOST_REQUIRE(runtime.shadows.Initialize(*Graphics::Shared_Frame_Device(),
			shader_directory));
		Graphics::Get_Prop_Submission().Initialize(*Graphics::Shared_Frame_Device(),
			runtime.renderer, runtime.shadows);

		auto &services = Graphics::Get_Render_Services();
		BOOST_REQUIRE(services.Initialize());
		active_callback_probe = &callback_probe;
		services.Set_Scene_Draw_Queue_Enabled(true);
		BOOST_REQUIRE(Graphics::Graphics_Begin_Frame());
		runtime.frame_active = true;

		{
			const auto current = Graphics::Get_Attachment_Bindings().Current();
			auto snapshot = Graphics::Get_Attachment_Bindings().Capture();
			BOOST_REQUIRE(snapshot.Owner() == Graphics::Shared_Frame_Device());
			BOOST_REQUIRE(snapshot.Selection().color.Is_Valid());
			snapshot.Reset();
			BOOST_CHECK(snapshot.Owner() == nullptr);
			BOOST_CHECK(!snapshot.Selection().color.Is_Valid());
			BOOST_CHECK(Graphics::Get_Attachment_Bindings().Current().color
				== current.color);
		}

		// Off-screen producers can submit while the shared device frame is
		// active but before the main render-service preparation. This is the
		// ordering used by the water reflection update.
		BOOST_REQUIRE(Graphics::Get_Scene_Draw_Queue().Enqueue<Draw_Probe>(5,
			pre_render_probe));
		BOOST_REQUIRE(services.Flush(nullptr));
		BOOST_CHECK(pre_render_events == std::vector<unsigned>({3u}));
		BOOST_CHECK_EQUAL(pre_render_probe.references, 0u);

		Graphics::RenderBeginOptions begin;
		begin.clear = true;
		begin.clear_depth = true;
		begin.clear_value = {0.125f, 0.25f, 0.5f, 0.75f};
		begin.resource_progress = &Resource_Progress;
		begin.evict_unused_textures = &Evict_Unused_Textures;
		const auto delayed_source = std::make_shared<Graphics::ResourceLoadSource>([] {
			return std::make_unique<CallbackResourceJob>(true);
		});
		const auto quick_source = std::make_shared<Graphics::ResourceLoadSource>([] {
			return std::make_unique<CallbackResourceJob>(false);
		});
		bool delayed_requested = false;
		bool quick_requested = false;
		std::thread request_thread([&] {
			delayed_requested = Graphics::Get_Resource_Load_Queue().Request(
				delayed_source, Graphics::ResourceLoadPriority::Background);
			quick_requested = Graphics::Get_Resource_Load_Queue().Request(
				quick_source, Graphics::ResourceLoadPriority::Background);
		});
		request_thread.join();
		BOOST_REQUIRE(delayed_requested);
		BOOST_REQUIRE(quick_requested);
		BOOST_REQUIRE(services.Begin_Render(begin));
		BOOST_CHECK_GE(callback_probe.resource_progress, 1u);
		BOOST_CHECK_EQUAL(callback_probe.evict_unused_textures, 1u);
		BOOST_CHECK(services.Is_Rendering());
		// Render services must not try to open a second backend frame.
		BOOST_CHECK(!Graphics::Graphics_Begin_Frame());

		const bool sorting_enabled =
			Graphics::Get_Render_Settings().Is_Sorting_Enabled();
		std::array<Graphics::PropVertex, 3> pending_vertices{};
		const std::array<std::uint32_t, 3> pending_indices{0, 1, 2};
		const auto pending_mesh = runtime.renderer.Create_Mesh(
			pending_vertices, pending_indices);
		BOOST_REQUIRE(pending_mesh.Is_Valid());
		Graphics::PropStyle pending_style;
		pending_style.blend = Graphics::RHIBlendMode::Disabled;
		Graphics::PropParameters pending_parameters;
		pending_parameters.textured = 0.0f;
		BOOST_REQUIRE(Graphics::Get_Prop_Submission().Submit(pending_mesh,
			pending_style, pending_parameters, {}, Graphics::PropDrawPhase::Material));
		services.Set_Sorting_Enabled(!sorting_enabled);
		BOOST_CHECK(Graphics::Get_Render_Settings().Is_Sorting_Enabled()
			== !sorting_enabled);
		BOOST_CHECK(runtime.renderer.Destroy_Mesh(pending_mesh));
		BOOST_CHECK(!runtime.renderer.Destroy_Mesh(pending_mesh));
		services.Set_Sorting_Enabled(sorting_enabled);

		BOOST_REQUIRE(Graphics::Get_Scene_Draw_Queue().Enqueue<Draw_Probe>(20, first));
		BOOST_REQUIRE(Graphics::Get_Scene_Draw_Queue().Enqueue<Draw_Probe>(10, second));
		BOOST_REQUIRE(services.Flush(nullptr));
		BOOST_CHECK(events == std::vector<unsigned>({1u, 2u}));
		BOOST_CHECK_EQUAL(first.references, 0u);
		BOOST_CHECK_EQUAL(second.references, 0u);

		const unsigned previous_frame_count = services.Frame_Count();
		BOOST_REQUIRE(services.End_Render());
		BOOST_CHECK(!services.Is_Rendering());
		BOOST_CHECK_EQUAL(services.Frame_Count(), previous_frame_count + 1u);
		BOOST_REQUIRE(Graphics::Graphics_End_Frame());
		runtime.frame_active = false;

		const auto target = Graphics::Shared_Frame_Device()->Get_Swap_Chain().Backbuffer();
		std::vector<std::byte> pixels(target.width * target.height * 4);
		BOOST_REQUIRE(Graphics::Shared_Frame_Device()->Readback_Texture(
			target.texture, pixels, target.width * 4));
		const std::size_t middle = (target.height / 2 * target.width
			+ target.width / 2) * 4;
		const std::array<int, 4> expected{32, 64, 128, 191};
		for (std::size_t channel = 0; channel < expected.size(); ++channel)
			BOOST_CHECK_SMALL(std::to_integer<int>(pixels[middle + channel])
				- expected[channel], 2);

		BOOST_REQUIRE(Graphics::Graphics_Present());

		BOOST_REQUIRE(services.Invalidate_Textures(&Invalidate_Textures));
		BOOST_CHECK_EQUAL(callback_probe.invalidate_textures, 1u);
		BOOST_REQUIRE(services.Shutdown(&Release_Assets));
		BOOST_CHECK_EQUAL(callback_probe.release_assets, 1u);
		BOOST_CHECK(callback_probe.shutdown_request_accepted);
		BOOST_CHECK_EQUAL(callback_probe.shutdown_job_completions, 1u);
		active_callback_probe = nullptr;
	}
}
