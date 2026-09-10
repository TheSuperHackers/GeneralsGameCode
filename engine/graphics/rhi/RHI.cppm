module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.RHI;

export import Graphics.Resources.Handles.ResourceHandle;

namespace Graphics
{

export struct RHIBufferHandleTag
{
};

export using RHIBufferHandle = ResourceHandle<RHIBufferHandleTag>;

export struct RHITextureHandleTag
{
};

export using RHITextureHandle = ResourceHandle<RHITextureHandleTag>;

export using RHIPipelineHandle = PipelineHandle;

export enum class RHIShaderStage : std::uint8_t
{
	Vertex,
	Fragment
};

export enum class RHIBufferUsage : std::uint8_t
{
	Vertex,
	Index,
	Constant,
	Storage
};

// Discard updates replace a vertex/index buffer's contents starting at offset
// zero. Bytes beyond the supplied range become unspecified; earlier queued draws
// retain the contents they were submitted with. Preserve keeps untouched bytes.
export enum class RHIBufferUpdateMode : std::uint8_t { Preserve, Discard };

export struct RHIBuffer final
{
	std::uint32_t byte_size = 0;
	RHIBufferUsage usage = RHIBufferUsage::Vertex;
	std::uint32_t stride = 0;
	RHIBufferUpdateMode update_mode = RHIBufferUpdateMode::Preserve;
};

export enum class RHITextureFormat : std::uint8_t
{
	R8_UNorm,
	RG8_UNorm,
	RGBA8_UNorm,
	BGRA8_UNorm,
	RGBA16_Float,
	RGBA32_Float,
	R32_Float,
	D24_UNorm_S8,
	D32_Float,
	BGR565_UNorm,
	BGRA4444_UNorm,
	BGRX8_UNorm,
	BGRA5551_UNorm,
	A8_UNorm,
	RG8_SNorm,
	BC1_UNorm,
	BC2_UNorm,
	BC3_UNorm,
	D16_UNorm,
	Unknown
};

export enum class RHITextureUsage : std::uint32_t
{
	ShaderResource = 1u << 0,
	RenderTarget = 1u << 1,
	DepthStencil = 1u << 2,
	UnorderedAccess = 1u << 3
};

export enum class RHITextureDimension : std::uint8_t { Texture2D, Cube, Volume };

export struct RHITexture final
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t mip_count = 1;
	RHITextureFormat format = RHITextureFormat::RGBA8_UNorm;
	std::uint32_t usage = static_cast<std::uint32_t>(RHITextureUsage::ShaderResource);
	std::uint32_t depth = 1;
	std::uint32_t array_size = 1;
	RHITextureDimension dimension = RHITextureDimension::Texture2D;
	bool generate_mips = false;
	// Output views address one array layer; the sampled view covers all layers.
	std::uint32_t output_layer = 0;
};

export struct RHITextureUpload final
{
	std::span<const std::byte> data{};
	std::uint32_t row_pitch = 0;
	std::uint32_t slice_pitch = 0;
	std::uint32_t mip_level = 0;
	std::uint32_t array_layer = 0;
};

export struct RHITextureReadback final
{
	std::span<std::byte> data{};
	std::uint32_t row_pitch = 0;
	std::uint32_t slice_pitch = 0;
	std::uint32_t mip_level = 0;
	std::uint32_t array_layer = 0;
};

export struct RHITextureMapping final
{
	std::span<std::byte> bytes{};
	std::uint32_t row_pitch = 0;
	std::uint32_t slice_pitch = 0;
};

export enum class RHIPrimitiveTopology : std::uint8_t
{
	TriangleList,
	PointList,
	TriangleStrip
};

export enum class RHIVertexFormat : std::uint8_t
{
	Position3Color4UV2,
	Position3Color4UV2ResourceIndex,
	Position3Color4UV2Skinned,
	Position3Color4UV2UV2Normal3
};

export enum class RHIBlendMode : std::uint8_t
{
	Disabled,
	Alpha,
	Additive,
	Multiply,
	ColorMultiply
};

export enum class RHIBlendOperation : std::uint8_t
{
	Add,
	ReverseSubtract
};

export enum class RHICullMode : std::uint8_t
{
	Back,
	None
};

export enum class RHISamplerAddress : std::uint8_t
{
	Wrap,
	Clamp
};

export enum class RHISamplerFilter : std::uint8_t { Point, Linear };

export struct RHISamplerDescription final
{
	std::array<RHISamplerAddress,3> address{RHISamplerAddress::Wrap,RHISamplerAddress::Wrap,RHISamplerAddress::Wrap};
    RHISamplerFilter minification = RHISamplerFilter::Linear;
    RHISamplerFilter magnification = RHISamplerFilter::Linear;
    RHISamplerFilter mipmap = RHISamplerFilter::Linear;
    std::uint8_t anisotropy = 1;
    float min_lod = 0;
    float max_lod = 3.402823466e+38f;
    void Set_Filter(RHISamplerFilter filter) noexcept { minification = magnification = mipmap = filter; }
    bool operator==(const RHISamplerDescription&) const = default;
};

export enum class RHIComparison : std::uint8_t
{
    Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always
};

export enum class RHIStencilOperation : std::uint8_t
{
    Keep, Zero, Replace, IncrementSaturate, DecrementSaturate, Invert, Increment, Decrement
};
export struct RHIStencilFace final
{
    RHIComparison comparison = RHIComparison::Always;
    RHIStencilOperation fail = RHIStencilOperation::Keep;
    RHIStencilOperation depth_fail = RHIStencilOperation::Keep;
    RHIStencilOperation pass = RHIStencilOperation::Keep;
    bool operator==(const RHIStencilFace &) const = default;
};
export struct RHIStencilDescription final
{
    bool enabled = false;
    std::uint8_t read_mask = 255;
    std::uint8_t write_mask = 255;
    std::uint8_t reference = 0;
    RHIStencilFace front{};
    RHIStencilFace back{};
    bool operator==(const RHIStencilDescription &) const = default;
};

export enum class RHIBlendFactor : std::uint8_t
{
    Zero, One, SourceColor, InverseSourceColor, SourceAlpha, InverseSourceAlpha,
    DestinationColor, InverseDestinationColor, DestinationAlpha, InverseDestinationAlpha
};

export enum class RHIVertexSemantic : std::uint8_t { Position, Color, Normal, TexCoord };
export enum class RHIVertexElementFormat : std::uint8_t { Float2, Float3, Float4, Float1 };
export struct RHIVertexElement final
{
    RHIVertexSemantic semantic = RHIVertexSemantic::Position;
    std::uint32_t semantic_index = 0;
    RHIVertexElementFormat format = RHIVertexElementFormat::Float3;
    std::uint32_t offset = 0;
};

export struct RHIPipeline final
{
	std::uint64_t key = 0;
	bool depth_test = true;
	bool depth_write = true;
	RHIPrimitiveTopology topology = RHIPrimitiveTopology::TriangleList;
	RHIVertexFormat vertex_format = RHIVertexFormat::Position3Color4UV2;
	RHIBlendMode blend_mode = RHIBlendMode::Disabled;
	RHICullMode cull_mode = RHICullMode::Back;
	RHIBlendOperation blend_operation = RHIBlendOperation::Add;
	bool scissor_test = false;
	std::uint8_t color_write_mask = 0x0f;
	std::array<RHISamplerDescription, 16> samplers{};
	std::uint8_t sampler_count = 1;
    RHIComparison depth_comparison = RHIComparison::LessEqual;
    bool front_counter_clockwise = false;
    RHIStencilDescription stencil{};
    std::int32_t depth_bias = 0;
    bool wireframe = false;
    bool blend_alpha_like_color = false;
    bool custom_blend_factors = false;
    RHIBlendFactor source_blend = RHIBlendFactor::One;
    RHIBlendFactor destination_blend = RHIBlendFactor::Zero;
    std::array<RHIVertexElement,16> vertex_elements{};
    std::uint8_t vertex_element_count = 0;
};

export struct RHIShaderBytecode final
{
	std::span<const std::byte> data{};
};

export struct RHIViewport final
{
	std::uint32_t x = 0;
	std::uint32_t y = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	float min_depth = 0.0f;
	float max_depth = 1.0f;
};

export struct RHIScissorRect final
{
	std::uint32_t x = 0;
	std::uint32_t y = 0;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

export enum class RHIIndexFormat : std::uint8_t
{
	UInt16,
	UInt32
};

export enum class RHIResourceType : std::uint8_t
{
	Invalid,
	Buffer,
	Texture,
	Sampler,
	Material
};

export struct RHIBindlessResource final
{
	ResourceIndex index{};
	RHIResourceType type = RHIResourceType::Invalid;
	RHIBufferHandle buffer{};
	RHITextureHandle texture{};
    RHIShaderStage stage = RHIShaderStage::Fragment;
    // Constant-buffer register, independent of the resource table index.
    std::uint32_t constant_buffer_slot = 0;
};

export struct RHIBackbuffer final
{
	RHITextureHandle texture{};
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

export struct RHIDepthTarget final
{
	RHITextureHandle texture{};
	std::uint32_t width = 0;
	std::uint32_t height = 0;
};

export class SwapChain
{
public:
	virtual ~SwapChain() noexcept = default;

	virtual bool Is_Valid() const noexcept = 0;
	virtual RHIBackbuffer Backbuffer() const noexcept = 0;
	virtual RHIDepthTarget Depth_Target() const noexcept = 0;
	virtual bool Resize(std::uint32_t width, std::uint32_t height) = 0;
	virtual bool Present() noexcept = 0;
};

export struct RHISubmissionCounts final
{
	std::uint64_t draw_calls = 0;
	// Topology-derived submitted primitives; includes degenerate triangles.
	std::uint64_t triangles = 0;
	// Submitted vertices or indices multiplied by instances, not unique vertices
	// or a hardware measurement of vertex shader executions.
	std::uint64_t vertex_invocations = 0;
};

export class CommandList
{
public:
	virtual ~CommandList() noexcept = default;

	RHISubmissionCounts Submission_Counts() const noexcept
	{
		return m_submission_counts;
	}

	virtual bool Reset_State() noexcept
	{
		return true;
	}

	virtual bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept = 0;
	virtual bool Set_Bindless_Resources(std::span<const RHIBindlessResource> resources) noexcept = 0;
	virtual bool Set_Render_Targets(RHITextureHandle color_target, RHITextureHandle depth_target) noexcept = 0;
	virtual bool Set_Color_Target(RHITextureHandle) noexcept
	{
		return false;
	}
	virtual bool Set_Depth_Target(RHITextureHandle depth_target) noexcept = 0;
	virtual bool Clear(const std::array<float, 4> &color, float depth) noexcept = 0;
	virtual bool Clear_Depth(float depth) noexcept = 0;
	// Clear an explicit attachment without changing the currently bound targets.
	virtual bool Clear_Color_Target(RHITextureHandle, const std::array<float, 4>&) noexcept { return false; }
	virtual bool Clear_Depth_Stencil_Target(RHITextureHandle, float, std::uint8_t) noexcept { return false; }
	virtual bool Copy_Texture(RHITextureHandle source, RHITextureHandle destination) noexcept
	{
		(void)source;
		(void)destination;
		return false;
	}
	virtual bool Set_Viewport(RHIViewport viewport) noexcept = 0;
	// Backends predating the generic scissor capability may keep the default
	// no-op while the active backend provides the real implementation.
	virtual bool Set_Scissor(RHIScissorRect) noexcept
	{
		return true;
	}
	virtual bool Set_Draw_Constants(std::span<const std::byte>) noexcept
	{
		return true;
	}
	virtual bool Set_Vertex_Buffer(std::uint32_t slot, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t offset) noexcept = 0;
	virtual bool Set_Index_Buffer(RHIBufferHandle buffer, RHIIndexFormat format, std::uint32_t offset) noexcept = 0;
	virtual bool Draw(std::uint32_t vertex_count, std::uint32_t first_vertex = 0, std::uint32_t instance_count = 1, std::uint32_t first_instance = 0) noexcept = 0;
	virtual bool Draw_Indexed(std::uint32_t index_count, std::uint32_t first_index = 0, std::int32_t base_vertex = 0, std::uint32_t instance_count = 1, std::uint32_t first_instance = 0) noexcept = 0;

protected:
	void Record_Draw(RHIPrimitiveTopology topology, std::uint32_t element_count, std::uint32_t instance_count) noexcept
	{
		const std::uint64_t elements = element_count;
		const std::uint64_t instances = instance_count;
		++m_submission_counts.draw_calls;
		m_submission_counts.vertex_invocations += elements * instances;
		switch (topology) {
		case RHIPrimitiveTopology::TriangleList:
			m_submission_counts.triangles += (elements / 3u) * instances;
			break;
		case RHIPrimitiveTopology::TriangleStrip:
			if (elements >= 3u)
				m_submission_counts.triangles += (elements - 2u) * instances;
			break;
		case RHIPrimitiveTopology::PointList:
			break;
		}
	}

private:
	RHISubmissionCounts m_submission_counts{};
};

export enum class RHIDeviceStatus : std::uint8_t
{
	Unavailable,
	Ready,
	Removed
};

export struct RHIAdapterInfo final
{
	std::uint32_t vendor_id = 0;
	std::uint32_t device_id = 0;
};

export struct RHITextureLimits final
{
	std::uint32_t max_2d_extent = 0;
	std::uint32_t max_3d_extent = 0;
};

export class Device
{
public:
	virtual ~Device() noexcept = default;

	virtual bool Is_Valid() const noexcept = 0;
	virtual RHIDeviceStatus Get_Status() const noexcept
	{
		return Is_Valid() ? RHIDeviceStatus::Ready : RHIDeviceStatus::Unavailable;
	}
	virtual bool Get_Adapter_Info(RHIAdapterInfo& info) const noexcept
	{
		info = {};
		return false;
	}
	virtual RHITextureLimits Texture_Limits() const noexcept { return {}; }
	virtual RHIBufferHandle Create_Buffer(const RHIBuffer &description) = 0;
	virtual RHITextureHandle Create_Texture(const RHITexture &description) = 0;
	virtual RHIPipelineHandle Create_Pipeline(const RHIPipeline &description) = 0;
	virtual RHIPipelineHandle Create_Pipeline(const RHIPipeline &description, RHIShaderBytecode vertex_shader, RHIShaderBytecode fragment_shader)
	{
		return vertex_shader.data.empty() && fragment_shader.data.empty() ? Create_Pipeline(description) : RHIPipelineHandle{};
	}
	virtual RHIBufferHandle Create_Buffer_Initialized(const RHIBuffer &description, std::span<const std::byte> initial_data)
	{
		return initial_data.empty() ? Create_Buffer(description) : RHIBufferHandle{};
	}
	virtual RHITextureHandle Create_Texture_Initialized(const RHITexture &description, const RHITextureUpload &initial_data)
	{
		return initial_data.data.empty() ? Create_Texture(description) : RHITextureHandle{};
	}
	virtual bool Update_Buffer(RHIBufferHandle, std::uint32_t, std::span<const std::byte>) noexcept
	{
		return false;
	}
	virtual bool Update_Texture(RHITextureHandle, const RHITextureUpload &) noexcept
	{
		return false;
	}
	virtual bool Readback_Texture(RHITextureHandle, std::span<std::byte>, std::uint32_t) noexcept
	{
		return false;
	}
	virtual bool Readback_Texture_Subresource(RHITextureHandle, const RHITextureReadback &) noexcept
	{
		return false;
	}
	virtual bool Generate_Texture_Mips(RHITextureHandle) noexcept { return false; }
	// Mappings preserve existing contents. Read-only mappings discard CPU writes.
	// Only one mapping per subresource may be active; destroying its last owner cancels it.
	virtual bool Map_Texture(RHITextureHandle, std::uint32_t, std::uint32_t, bool, RHITextureMapping &) { return false; }
	virtual bool Unmap_Texture(RHITextureHandle, std::uint32_t, std::uint32_t) noexcept { return false; }
	// Each retained reference requires Destroy_Texture; the device must outlive them.
	virtual bool Retain_Texture(RHITextureHandle) noexcept { return false; }
	virtual bool Destroy_Buffer(RHIBufferHandle buffer) noexcept = 0;
	virtual bool Destroy_Texture(RHITextureHandle texture) noexcept = 0;
	virtual bool Destroy_Pipeline(RHIPipelineHandle pipeline) noexcept = 0;
	virtual CommandList &Immediate_Command_List() noexcept = 0;
	virtual SwapChain &Get_Swap_Chain() noexcept = 0;
	virtual bool Begin_Frame() noexcept = 0;
	virtual bool End_Frame() noexcept = 0;
};

}
