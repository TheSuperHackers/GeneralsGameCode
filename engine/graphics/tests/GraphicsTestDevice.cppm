module;

#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>

export module Graphics.Tests.Device;

export import Graphics.RHI;
import Graphics.Backends.DX11;
import Graphics.Backends.DX12;

namespace Graphics
{

export enum class GraphicsTestBackend
{
	DX11,
	DX12,
	Invalid
};

export struct GraphicsTestDeviceOptions final
{
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
	GraphicsTestBackend Parse_Backend() noexcept
	{
		const char *value = std::getenv("GRAPHICS_TEST_BACKEND");
		if (value == nullptr || *value == '\0' || std::string_view(value) == "dx11")
			return GraphicsTestBackend::DX11;
		if (std::string_view(value) == "dx12")
			return GraphicsTestBackend::DX12;
		return GraphicsTestBackend::Invalid;
	}

	bool Parse_Driver(bool requested_warp, bool &use_warp) noexcept
	{
		const char *value = std::getenv("GRAPHICS_TEST_DRIVER");
		if (value == nullptr || *value == '\0') {
			use_warp = requested_warp;
			return true;
		}
		if (std::string_view(value) == "warp") {
			use_warp = true;
			return true;
		}
		if (std::string_view(value) == "hardware") {
			use_warp = false;
			return true;
		}
		return false;
	}

	std::filesystem::path Resolve_Shader_Directory(std::filesystem::path path)
	{
		if (path.empty())
			return {};

		const char *backend = std::getenv("GRAPHICS_TEST_BACKEND");
		if (path.parent_path().filename() == "precompiled")
			return std::filesystem::path(GRAPHICS_TEST_FIXTURE_DIRECTORY)
				/ (backend != nullptr && std::string_view(backend) == "dx12" ? "dx12" : "dx11");
		if (backend == nullptr || std::string_view(backend) != "dx12")
			return path;
		if (path.filename() == "dx11")
			path.replace_filename("dx12");
		else
			path /= "dx12";
		return path;
	}

	DX11DeviceOptions Make_DX11_Options(const GraphicsTestDeviceOptions &options,
		bool use_warp, const std::string &shader_directory)
	{
		DX11DeviceOptions result;
		result.use_warp = use_warp;
		result.window = options.window;
		result.width = options.width;
		result.height = options.height;
		result.shader_directory = shader_directory.empty() ? options.shader_directory : shader_directory.c_str();
		result.vertex_shader_name = options.vertex_shader_name;
		result.fragment_shader_name = options.fragment_shader_name;
		result.backbuffer_format = options.backbuffer_format;
		return result;
	}

	DX12DeviceOptions Make_DX12_Options(const GraphicsTestDeviceOptions &options,
		bool use_warp, const std::string &shader_directory)
	{
		DX12DeviceOptions result;
		result.use_warp = use_warp;
		result.window = options.window;
		result.width = options.width;
		result.height = options.height;
		result.shader_directory = shader_directory.empty() ? options.shader_directory : shader_directory.c_str();
		result.vertex_shader_name = options.vertex_shader_name;
		result.fragment_shader_name = options.fragment_shader_name;
		result.backbuffer_format = options.backbuffer_format;
		return result;
	}
}

export std::filesystem::path Test_Shader_Directory(std::filesystem::path base)
{
	return Resolve_Shader_Directory(std::move(base));
}

export GraphicsTestBackend Selected_Graphics_Test_Backend() noexcept
{
	return Parse_Backend();
}

export bool Graphics_Test_Uses_WARP(bool requested_warp) noexcept
{
	bool use_warp = requested_warp;
	return Parse_Driver(requested_warp, use_warp) && use_warp;
}

export class GraphicsTestDevice final : public Device
{
public:
	explicit GraphicsTestDevice(GraphicsTestDeviceOptions options = {})
		: m_backend(Parse_Backend())
	{
		bool use_warp = options.use_warp;
		if (m_backend == GraphicsTestBackend::Invalid || !Parse_Driver(options.use_warp, use_warp))
			return;

		if (options.shader_directory != nullptr)
			m_shader_directory = Resolve_Shader_Directory(options.shader_directory).string();
		if (m_backend == GraphicsTestBackend::DX11)
			m_device = std::make_unique<DX11Device>(Make_DX11_Options(options, use_warp, m_shader_directory));
		else
			m_device = std::make_unique<DX12Device>(Make_DX12_Options(options, use_warp, m_shader_directory));
	}

	~GraphicsTestDevice() noexcept override = default;

	GraphicsTestDevice(const GraphicsTestDevice &) = delete;
	GraphicsTestDevice &operator=(const GraphicsTestDevice &) = delete;

	GraphicsTestBackend Backend() const noexcept { return m_backend; }
	bool Is_Selection_Valid() const noexcept { return m_backend != GraphicsTestBackend::Invalid && m_device != nullptr; }

	bool Is_Valid() const noexcept override { return m_device != nullptr && m_device->Is_Valid(); }
	RHIDeviceStatus Get_Status() const noexcept override { return m_device != nullptr ? m_device->Get_Status() : RHIDeviceStatus::Unavailable; }
	bool Get_Adapter_Info(RHIAdapterInfo &info) const noexcept override { return m_device != nullptr && m_device->Get_Adapter_Info(info); }
	RHITextureLimits Texture_Limits() const noexcept override { return m_device != nullptr ? m_device->Texture_Limits() : RHITextureLimits{}; }
	RHIBufferHandle Create_Buffer(const RHIBuffer &description) override { return m_device != nullptr ? m_device->Create_Buffer(description) : RHIBufferHandle{}; }
	RHITextureHandle Create_Texture(const RHITexture &description) override { return m_device != nullptr ? m_device->Create_Texture(description) : RHITextureHandle{}; }
	RHIPipelineHandle Create_Pipeline(const RHIPipeline &description) override { return m_device != nullptr ? m_device->Create_Pipeline(description) : RHIPipelineHandle{}; }
	RHIBufferHandle Create_Buffer_Initialized(const RHIBuffer &description, std::span<const std::byte> data) override { return m_device != nullptr ? m_device->Create_Buffer_Initialized(description, data) : RHIBufferHandle{}; }
	RHITextureHandle Create_Texture_Initialized(const RHITexture &description, const RHITextureUpload &data) override { return m_device != nullptr ? m_device->Create_Texture_Initialized(description, data) : RHITextureHandle{}; }
	RHIPipelineHandle Create_Pipeline(const RHIPipeline &description, RHIShaderBytecode vertex_shader, RHIShaderBytecode fragment_shader) override { return m_device != nullptr ? m_device->Create_Pipeline(description, vertex_shader, fragment_shader) : RHIPipelineHandle{}; }
	bool Update_Buffer(RHIBufferHandle buffer, std::uint32_t offset, std::span<const std::byte> data) noexcept override { return m_device != nullptr && m_device->Update_Buffer(buffer, offset, data); }
	bool Update_Texture(RHITextureHandle texture, const RHITextureUpload &data) noexcept override { return m_device != nullptr && m_device->Update_Texture(texture, data); }
	bool Readback_Texture(RHITextureHandle texture, std::span<std::byte> data, std::uint32_t row_pitch) noexcept override { return m_device != nullptr && m_device->Readback_Texture(texture, data, row_pitch); }
	bool Readback_Texture_Subresource(RHITextureHandle texture, const RHITextureReadback &data) noexcept override { return m_device != nullptr && m_device->Readback_Texture_Subresource(texture, data); }
	bool Generate_Texture_Mips(RHITextureHandle texture) noexcept override { return m_device != nullptr && m_device->Generate_Texture_Mips(texture); }
	bool Map_Texture(RHITextureHandle texture, std::uint32_t mip, std::uint32_t layer, bool read_only, RHITextureMapping &mapping) override { return m_device != nullptr && m_device->Map_Texture(texture, mip, layer, read_only, mapping); }
	bool Unmap_Texture(RHITextureHandle texture, std::uint32_t mip, std::uint32_t layer) noexcept override { return m_device != nullptr && m_device->Unmap_Texture(texture, mip, layer); }
	bool Retain_Texture(RHITextureHandle texture) noexcept override { return m_device != nullptr && m_device->Retain_Texture(texture); }
	bool Destroy_Buffer(RHIBufferHandle buffer) noexcept override { return m_device != nullptr && m_device->Destroy_Buffer(buffer); }
	bool Destroy_Texture(RHITextureHandle texture) noexcept override { return m_device != nullptr && m_device->Destroy_Texture(texture); }
	bool Destroy_Pipeline(RHIPipelineHandle pipeline) noexcept override { return m_device != nullptr && m_device->Destroy_Pipeline(pipeline); }
	CommandList &Immediate_Command_List() noexcept override { return m_device->Immediate_Command_List(); }
	SwapChain &Get_Swap_Chain() noexcept override { return m_device->Get_Swap_Chain(); }
	bool Begin_Frame() noexcept override { return m_device != nullptr && m_device->Begin_Frame(); }
	bool End_Frame() noexcept override { return m_device != nullptr && m_device->End_Frame(); }

private:
	GraphicsTestBackend m_backend = GraphicsTestBackend::Invalid;
	std::string m_shader_directory;
	std::unique_ptr<Device> m_device;
};

}
