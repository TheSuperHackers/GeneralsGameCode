module;

#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

export module Graphics.Frame.Runtime;

export import Graphics.FrameOwner;
import Graphics.FrameTargets;
export import Graphics.RHI;
import Graphics.Backends.DX11;
import Graphics.Backends.DX12;
import Graphics.Frame.AttachmentBindings;
import Graphics.Frame.ResourceLifecycle;
import Graphics.Resources.Recreation;
import Graphics.Resources.Loading.Queue;

namespace Graphics
{

export enum class FrameBackend : std::uint8_t
{
	Automatic,
	DX11,
	DX12,
	Invalid
};

export struct FrameDeviceOptions final
{
	FrameBackend backend = FrameBackend::Automatic;
	bool use_warp = false;
	void *window = nullptr;
	std::uint32_t width = 1280;
	std::uint32_t height = 720;
	const char *shader_directory = nullptr;
	const char *vertex_shader_name = "visual_basic.vso";
	const char *fragment_shader_name = "visual_basic.pso";
	RHITextureFormat backbuffer_format = RHITextureFormat::RGBA8_UNorm;
};

namespace
{
	std::unique_ptr<Device> g_frame_device;
	FrameOwner g_frame_owner;
	std::optional<FrameDeviceOptions> g_frame_options;
	FrameBackend g_frame_backend = FrameBackend::Invalid;
	bool g_exclusive_fullscreen = false;
	bool g_reconfiguring = false;
	bool g_restore_resources = false;
	std::uint64_t g_frame_target_identity = 0;

	struct ConfigurationScope final
	{
		ConfigurationScope() { g_reconfiguring = true; }
		~ConfigurationScope() { g_reconfiguring = false; }
	};

	FrameBackend Parse_Backend_Name(const char *value) noexcept
	{
		if (value == nullptr || *value == '\0')
			return FrameBackend::Automatic;
		if (std::string_view(value) == "dx11")
			return FrameBackend::DX11;
		if (std::string_view(value) == "dx12")
			return FrameBackend::DX12;
		return FrameBackend::Invalid;
	}

	FrameBackend Resolve_Backend(const FrameDeviceOptions &options) noexcept
	{
		if (options.backend != FrameBackend::Automatic)
			return options.backend;
		const char *requested = std::getenv("GRAPHICS_BACKEND");
		const FrameBackend environment_backend = Parse_Backend_Name(requested);
		if (environment_backend == FrameBackend::Invalid)
			return FrameBackend::Invalid;
		if (environment_backend != FrameBackend::Automatic)
			return environment_backend;
		return FrameBackend::DX11;
	}

	DX11DeviceOptions Make_DX11_Options(const FrameDeviceOptions &options)
	{
		DX11DeviceOptions result;
		result.use_warp = options.use_warp;
		result.window = options.window;
		result.width = options.width;
		result.height = options.height;
		result.shader_directory = options.shader_directory;
		result.vertex_shader_name = options.vertex_shader_name;
		result.fragment_shader_name = options.fragment_shader_name;
		result.backbuffer_format = options.backbuffer_format;
		return result;
	}

	DX12DeviceOptions Make_DX12_Options(const FrameDeviceOptions &options)
	{
		DX12DeviceOptions result;
		result.use_warp = options.use_warp;
		result.window = options.window;
		result.width = options.width;
		result.height = options.height;
		result.shader_directory = options.shader_directory;
		result.vertex_shader_name = options.vertex_shader_name;
		result.fragment_shader_name = options.fragment_shader_name;
		result.backbuffer_format = options.backbuffer_format;
		return result;
	}

	std::unique_ptr<Device> Create_Device(const FrameDeviceOptions &options, FrameBackend backend)
	{
		if (backend == FrameBackend::DX11)
			return std::make_unique<DX11Device>(Make_DX11_Options(options));
		if (backend == FrameBackend::DX12)
			return std::make_unique<DX12Device>(Make_DX12_Options(options));
		return {};
	}

	bool Set_Exclusive_Fullscreen(Device &device, FrameBackend backend, bool fullscreen) noexcept
	{
		if (backend == FrameBackend::DX11)
			return static_cast<DX11Device &>(device).Set_Exclusive_Fullscreen(fullscreen);
		if (backend == FrameBackend::DX12)
			return static_cast<DX12Device &>(device).Set_Exclusive_Fullscreen(fullscreen);
		return false;
	}

	bool Bind_Frame_Attachments()
	{
		if (!g_frame_device || !g_frame_device->Get_Swap_Chain().Is_Valid()) return false;
		auto &swapchain = g_frame_device->Get_Swap_Chain();
		const auto color = swapchain.Backbuffer();
		return Get_Attachment_Bindings().Initialize(*g_frame_device,
			{color.texture, swapchain.Depth_Target().texture, {0, 0, color.width, color.height, 0, 1}});
	}

	void Release_Device_Resources()
	{
		if (!g_frame_device) return;
		g_frame_owner.Abort(*g_frame_device);
		Get_Resource_Load_Queue().Drain();
		Get_Frame_Resource_Lifecycle().Release();
		Get_Resource_Recreation_Registry().Release();
		Get_Attachment_Bindings().Reset();
		g_frame_owner.Set_Draw_Executor(nullptr);
		g_frame_device.reset();
		g_restore_resources = true;
	}

	bool Create_Frame_Device(const FrameDeviceOptions &options, FrameBackend backend, bool exclusive)
	{
		if (!options.window || !IsWindow(static_cast<HWND>(options.window))) return false;
		auto device = Create_Device(options, backend);
		if (!device || !device->Is_Valid() || !device->Get_Swap_Chain().Is_Valid()
			|| (exclusive && !Set_Exclusive_Fullscreen(*device, backend, true))) return false;
		g_frame_device = std::move(device);
		if (!Bind_Frame_Attachments()) { g_frame_device.reset(); return false; }
		++g_frame_target_identity;
		return true;
	}

	void Restore_Device_Resources()
	{
		if (!g_restore_resources) return;
		Get_Frame_Resource_Lifecycle().Reacquire();
		Get_Resource_Recreation_Registry().Recreate();
		g_restore_resources = false;
	}
}

export Device *Shared_Frame_Device() noexcept
{
	return g_frame_device.get();
}

export FrameTargets Shared_Frame_Targets() noexcept
{
	if (!g_frame_device || !g_frame_device->Get_Swap_Chain().Is_Valid()) return {};
	auto &swapchain = g_frame_device->Get_Swap_Chain();
	return {swapchain.Backbuffer(), swapchain.Depth_Target(), g_frame_target_identity};
}

export FrameBackend Active_Frame_Backend() noexcept
{
	return g_frame_backend;
}

export std::filesystem::path Frame_Shader_Directory(std::filesystem::path base)
{
	if (g_frame_backend == FrameBackend::DX12)
		base /= "dx12";
	return base;
}

export bool Initialize_Frame_Device(const FrameDeviceOptions &options)
{
	if (g_reconfiguring || g_frame_device != nullptr || options.window == nullptr
		|| g_frame_owner.Phase() != FrameOwnerPhase::Idle) return false;
	const FrameBackend backend = Resolve_Backend(options);
	if (backend == FrameBackend::Invalid) return false;
	ConfigurationScope scope;
	if (!Create_Frame_Device(options, backend, false)) return false;
	g_frame_options = options;
	g_frame_options->backend = backend;
	g_frame_backend = backend;
	g_exclusive_fullscreen = false;
	Restore_Device_Resources();
	return true;
}

export bool Frame_Device_Ready() noexcept
{
	return g_frame_device && g_frame_device->Get_Status() == RHIDeviceStatus::Ready
		&& g_frame_device->Get_Swap_Chain().Is_Valid()
		&& Get_Attachment_Bindings().Default().color.Is_Valid();
}

export bool Recreate_Frame_Device()
{
	if (g_reconfiguring || !g_frame_options || g_frame_owner.Phase() != FrameOwnerPhase::Idle
		|| Get_Attachment_Bindings().Offscreen()) return false;
	ConfigurationScope scope;
	Release_Device_Resources();
	if (!Create_Frame_Device(*g_frame_options, g_frame_backend, g_exclusive_fullscreen)) return false;
	Restore_Device_Resources();
	return true;
}

export bool Resize_Frame_Device(std::uint32_t width, std::uint32_t height, bool exclusive_fullscreen)
{
	if (g_reconfiguring || !g_frame_options || g_frame_owner.Phase() != FrameOwnerPhase::Idle
		|| Get_Attachment_Bindings().Offscreen() || width == 0 || height == 0) return false;
	g_frame_options->width = width;
	g_frame_options->height = height;
	g_exclusive_fullscreen = exclusive_fullscreen;
	if (!g_frame_device || g_frame_device->Get_Status() != RHIDeviceStatus::Ready)
		return Recreate_Frame_Device();
	ConfigurationScope scope;
	if (!Set_Exclusive_Fullscreen(*g_frame_device, g_frame_backend, false)) return false;
	Get_Attachment_Bindings().Reset();
	const bool resized = g_frame_device->Get_Swap_Chain().Resize(width, height);
	// Failed resize can still recreate the previous-size attachments.
	++g_frame_target_identity;
	const bool bound = Bind_Frame_Attachments();
	const bool fullscreen = !exclusive_fullscreen
		|| Set_Exclusive_Fullscreen(*g_frame_device, g_frame_backend, true);
	return resized && bound && fullscreen;
}

export bool Recover_Frame_Device()
{
	return g_frame_options && Resize_Frame_Device(g_frame_options->width,
		g_frame_options->height, g_exclusive_fullscreen);
}

export bool Set_Frame_Exclusive_Fullscreen(bool fullscreen) noexcept
{
	if (g_reconfiguring || !g_frame_device || g_frame_owner.Phase() != FrameOwnerPhase::Idle
		|| !Set_Exclusive_Fullscreen(*g_frame_device, g_frame_backend, fullscreen)) return false;
	g_exclusive_fullscreen = fullscreen;
	return true;
}

export void Detach_Frame_Draw_Executor() noexcept
{
	if (g_frame_device != nullptr) g_frame_owner.Abort(*g_frame_device);
	g_frame_owner.Set_Draw_Executor(nullptr);
}

export extern "C" bool Graphics_Begin_Frame() noexcept
{
	if (g_reconfiguring) return false;
	if (!Frame_Device_Ready() && !Recover_Frame_Device()) return false;
	if (g_frame_owner.Phase() == FrameOwnerPhase::Idle
		&& !Get_Attachment_Bindings().Offscreen()) {
		auto &swapchain = g_frame_device->Get_Swap_Chain();
		const auto color = swapchain.Backbuffer();
		const auto depth = swapchain.Depth_Target();
		auto &bindings = Get_Attachment_Bindings();
		const auto &defaults = bindings.Default();
		const auto &current = bindings.Current();
		if (defaults.color != color.texture || defaults.depth != depth.texture
			|| current.color != color.texture || current.depth != depth.texture
			|| defaults.viewport.width != color.width || defaults.viewport.height != color.height) {
			if (!Bind_Frame_Attachments()) return false;
		}
	}
	return g_frame_owner.Begin_Frame(*g_frame_device);
}

export extern "C" bool Graphics_Execute_Queued_Draws() noexcept
{
	return g_frame_device != nullptr && g_frame_owner.Execute_Queued_Draws(*g_frame_device);
}

export bool Register_Frame_Draw_Executor(FrameRendererInitializer initializer, FrameDrawExecutor executor)
{
	if (g_frame_device == nullptr || g_frame_owner.Phase() != FrameOwnerPhase::Idle)
		return false;
	if (initializer != nullptr && !initializer(*g_frame_device))
		return false;
	return g_frame_owner.Set_Draw_Executor(executor);
}

export extern "C" bool Graphics_End_Frame() noexcept
{
	return g_frame_device != nullptr && g_frame_owner.End_Frame(*g_frame_device);
}

export extern "C" bool Graphics_Present() noexcept
{
	if (g_frame_device == nullptr || !g_frame_owner.Present(*g_frame_device))
		return false;
	// Loading screens can bind the default target between frames. Publish the
	// next swap-chain buffer now, before those callers record any GPU commands.
	auto &bindings = Get_Attachment_Bindings();
	if (!bindings.Offscreen()
		&& bindings.Default().color != g_frame_device->Get_Swap_Chain().Backbuffer().texture)
		return Bind_Frame_Attachments();
	return true;
}

export extern "C" void Graphics_Abort_Frame() noexcept
{
	if (g_frame_device != nullptr)
		g_frame_owner.Abort(*g_frame_device);
}

export extern "C" std::uint32_t Graphics_Frame_Invalid_Operation_Count() noexcept
{
	return g_frame_owner.Invalid_Operation_Count();
}

export extern "C" void Graphics_Shutdown_Shared_Frame() noexcept
{
	if (g_reconfiguring) return;
	ConfigurationScope scope;
	Release_Device_Resources();
	g_frame_owner.Set_Draw_Executor(nullptr);
	g_frame_options.reset();
	g_frame_backend = FrameBackend::Invalid;
	g_restore_resources = false;
	g_exclusive_fullscreen = false;
}

}
