module;
#include "../../profiling/Tracy.h"

#define NOMINMAX

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <d3d11.h>
#include <dxgi.h>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <windows.h>
#include "DX11GPUProfiling.h"

export module Graphics.Backends.DX11;

export import Graphics.RHI;

import Graphics.Resources.Pools.ResourcePool;

namespace Graphics
{

template <typename Interface>
class DX11NativeObject final
{
public:
	DX11NativeObject() noexcept = default;
	~DX11NativeObject() noexcept
	{
		Reset();
	}

	DX11NativeObject(const DX11NativeObject &) = delete;
	DX11NativeObject &operator=(const DX11NativeObject &) = delete;

	DX11NativeObject(DX11NativeObject &&other) noexcept
		: m_object(other.m_object)
	{
		other.m_object = nullptr;
	}

	DX11NativeObject &operator=(DX11NativeObject &&other) noexcept
	{
		if (this == &other)
			return *this;

		Reset();
		m_object = other.m_object;
		other.m_object = nullptr;
		return *this;
	}

	void Reset(Interface *object = nullptr) noexcept
	{
		if (m_object != nullptr)
			m_object->Release();
		m_object = object;
	}

	Interface *Get() const noexcept
	{
		return m_object;
	}

private:
	Interface *m_object = nullptr;
};

struct DX11Buffer final
{
	DX11NativeObject<ID3D11Buffer> object;
	DX11NativeObject<ID3D11ShaderResourceView> shader_resource_view;
	RHIBufferUsage usage = RHIBufferUsage::Vertex;
	std::uint32_t byte_size = 0;
	std::uint32_t capacity = 0;
    RHIBufferUpdateMode update_mode = RHIBufferUpdateMode::Preserve;
};

// Recycle native vertex/index storage independently of public handle lifetime.
// Size classes avoid searching live resources. The budget limits idle memory;
// exhausting it never limits allocation or drops a draw.
class DX11BufferCache final
{
public:
    static constexpr std::uint32_t MaxIdleBytes = 64u * 1024u * 1024u;

    static bool Eligible(RHIBufferUsage usage, std::uint32_t size) noexcept
    {
        return (usage == RHIBufferUsage::Vertex || usage == RHIBufferUsage::Index)
            && size > 0 && size <= MaxIdleBytes;
    }

    static unsigned Size_Class(std::uint32_t size) noexcept
    {
        return std::bit_width(size > 256 ? size - 1 : 255u);
    }

    DX11NativeObject<ID3D11Buffer> Take(RHIBufferUsage usage, std::uint32_t size, RHIBufferUpdateMode mode)
    {
        if (!Eligible(usage, size)) return {};
        const auto index = Size_Class(size);
        auto& bucket = m_free[mode == RHIBufferUpdateMode::Discard][usage == RHIBufferUsage::Index][index];
        if (bucket.empty()) return {};
        auto buffer = std::move(bucket.back());
        bucket.pop_back();
        m_idle_bytes -= 1u << index;
        return buffer;
    }

    void Recycle(DX11Buffer& buffer) noexcept
    {
        if (!Eligible(buffer.usage, buffer.capacity)
            || buffer.capacity > MaxIdleBytes - m_idle_bytes) return;
        auto& bucket = m_free[buffer.update_mode == RHIBufferUpdateMode::Discard]
            [buffer.usage == RHIBufferUsage::Index][Size_Class(buffer.capacity)];
        try {
            bucket.push_back(std::move(buffer.object));
            m_idle_bytes += buffer.capacity;
        } catch (...) {
            // Destruction still releases the resource if cache growth fails.
        }
    }

private:
    std::array<std::array<std::array<std::vector<DX11NativeObject<ID3D11Buffer>>, 27>, 2>, 2> m_free;
    std::uint32_t m_idle_bytes = 0;
};

struct DX11TextureMapping final
{
    DX11NativeObject<ID3D11Resource> staging;
    DX11NativeObject<ID3D11DeviceContext> context;
    std::uint32_t subresource = 0;
    std::uint32_t staging_subresource = 0;
    bool read_only = false;
    bool mapped = false;
    ~DX11TextureMapping() {
        if (mapped) context.Get()->Unmap(staging.Get(), staging_subresource);
    }
};

struct DX11Texture final
{
	DX11NativeObject<ID3D11Texture2D> object;
    DX11NativeObject<ID3D11Texture3D> volume;
    RHITexture description{};
    std::uint32_t references = 1;
    std::vector<std::unique_ptr<DX11TextureMapping>> mappings;
    ID3D11Resource* Resource() const noexcept { return object.Get() != nullptr ? static_cast<ID3D11Resource*>(object.Get()) : volume.Get(); }
	DX11NativeObject<ID3D11ShaderResourceView> shader_resource_view;
	DX11NativeObject<ID3D11RenderTargetView> render_target_view;
	DX11NativeObject<ID3D11DepthStencilView> depth_stencil_view;
	DX11NativeObject<ID3D11UnorderedAccessView> unordered_access_view;
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	RHITextureFormat format = RHITextureFormat::RGBA8_UNorm;
};

struct DX11Pipeline final
{
	std::uint64_t key = 0;
	DX11NativeObject<ID3D11VertexShader> vertex_shader;
	DX11NativeObject<ID3D11PixelShader> pixel_shader;
	DX11NativeObject<ID3D11InputLayout> input_layout;
	DX11NativeObject<ID3D11DepthStencilState> depth_stencil_state;
	DX11NativeObject<ID3D11BlendState> blend_state;
	DX11NativeObject<ID3D11RasterizerState> rasterizer_state;
	std::array<DX11NativeObject<ID3D11SamplerState>, 16> sampler_states;
	std::uint8_t sampler_count = 1;
    std::uint8_t stencil_reference = 0;
	RHIPrimitiveTopology topology = RHIPrimitiveTopology::TriangleList;
};

static_assert(std::is_nothrow_move_constructible_v<DX11Buffer>);
static_assert(std::is_nothrow_move_assignable_v<DX11Buffer>);
static_assert(std::is_nothrow_move_constructible_v<DX11Texture>);
static_assert(std::is_nothrow_move_assignable_v<DX11Texture>);
static_assert(std::is_nothrow_move_constructible_v<DX11Pipeline>);
static_assert(std::is_nothrow_move_assignable_v<DX11Pipeline>);

struct DX11DeviceState;

class DX11SwapChain final : public SwapChain
{
public:
	explicit DX11SwapChain(DX11DeviceState *state) noexcept
		: m_state(state)
	{
	}

	~DX11SwapChain() noexcept override;

	bool Is_Valid() const noexcept override;
	RHIBackbuffer Backbuffer() const noexcept override;
	RHIDepthTarget Depth_Target() const noexcept override;
	bool Resize(std::uint32_t width, std::uint32_t height) override;
	bool Present() noexcept override;

	bool Create_Targets(std::uint32_t width, std::uint32_t height);

private:
	DX11DeviceState *m_state = nullptr;
	RHITextureHandle m_backbuffer{};
	RHITextureHandle m_depth_target{};
	std::uint32_t m_width = 0;
	std::uint32_t m_height = 0;
};

class DX11CommandList final : public CommandList
{
public:
	explicit DX11CommandList(DX11DeviceState *state) noexcept
		: m_state(state)
	{
	}

	bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept override;
	bool Set_Bindless_Resources(std::span<const RHIBindlessResource> resources) noexcept override;
	bool Set_Render_Targets(RHITextureHandle color_target, RHITextureHandle depth_target) noexcept override;
	bool Set_Color_Target(RHITextureHandle color_target) noexcept override;
	bool Set_Depth_Target(RHITextureHandle depth_target) noexcept override;
	bool Clear(const std::array<float, 4> &color, float depth) noexcept override;
	bool Clear_Depth(float depth) noexcept override;
    bool Clear_Color_Target(RHITextureHandle texture, const std::array<float, 4>& color) noexcept override;
    bool Clear_Depth_Stencil_Target(RHITextureHandle texture, float depth, std::uint8_t stencil) noexcept override;
	bool Copy_Texture(RHITextureHandle source, RHITextureHandle destination) noexcept override;
	bool Set_Viewport(RHIViewport viewport) noexcept override;
	bool Set_Scissor(RHIScissorRect scissor) noexcept override;
	bool Set_Draw_Constants(std::span<const std::byte> data) noexcept override;
	bool Set_Vertex_Buffer(std::uint32_t slot, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t offset) noexcept override;
	bool Set_Index_Buffer(RHIBufferHandle buffer, RHIIndexFormat format, std::uint32_t offset) noexcept override;
	bool Draw(std::uint32_t vertex_count, std::uint32_t first_vertex, std::uint32_t instance_count, std::uint32_t first_instance) noexcept override;
	bool Draw_Indexed(std::uint32_t index_count, std::uint32_t first_index, std::int32_t base_vertex, std::uint32_t instance_count, std::uint32_t first_instance) noexcept override;
	bool Reset_State() noexcept override;
	void Reset_Frame_State() noexcept;
	void Invalidate_Render_Target_Binding() noexcept { m_targets_bound = false; }

private:
	bool Is_Ready() const noexcept;
	bool Is_Pipeline_Valid() const noexcept;
	bool Bind_Texture_At_Slot(RHIShaderStage stage, std::uint32_t slot, RHITextureHandle texture) noexcept;
	bool Bind_Buffer_At_Slot(RHIShaderStage stage, std::uint32_t slot, RHIBufferHandle buffer) noexcept;

	struct ShaderResourceBinding final {
		RHITextureHandle texture{};
		RHIBufferHandle buffer{};
	};

	DX11DeviceState *m_state = nullptr;
	RHIPipelineHandle m_pipeline{};
	std::array<std::array<RHIBufferHandle, D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT>, 2> m_constant_buffers{};
	std::array<std::array<ShaderResourceBinding, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT>, 2> m_shader_resources{};
	RHIPrimitiveTopology m_topology = RHIPrimitiveTopology::TriangleList;
	RHITextureHandle m_color_target{};
	RHITextureHandle m_depth_target{};
	bool m_targets_bound = false;
	std::span<const RHIBindlessResource> m_bindless_resources{};
	DX11NativeObject<ID3D11Buffer> m_draw_constants;
	std::uint32_t m_draw_constants_size = 0;
	std::array<std::byte, 256> m_draw_constants_data{};
	std::size_t m_draw_constants_data_size = 0;
	std::array<bool, 2> m_draw_constants_bound{};
};

struct DX11DeviceState final
{
	DX11NativeObject<ID3D11Device> device;
	DX11NativeObject<ID3D11DeviceContext> context;
	DX11NativeObject<IDXGISwapChain> native_swap_chain;
	DX11BufferCache buffer_cache;
	ResourcePool<DX11Buffer, RHIBufferHandle> buffers;
	ResourcePool<DX11Texture, RHITextureHandle> textures;
	ResourcePool<DX11Pipeline, RHIPipelineHandle> pipelines;
	DX11SwapChain swap_chain;
	DX11CommandList command_list;
	std::string shader_directory;
	std::string vertex_shader_name;
	std::string fragment_shader_name;
	bool frame_active = false;
	bool ready_to_present = false;
	bool presented = false;
	DX11Detail::GPUProfiler gpu_profiler;

	DX11DeviceState() noexcept
		: swap_chain(this),
		  command_list(this)
	{
	}
};

template <typename Interface>
static Interface *Retain(Interface *object) noexcept
{
	if (object != nullptr)
		object->AddRef();

	return object;
}

static DXGI_FORMAT To_DX11_Format(RHITextureFormat format) noexcept
{
	switch (format) {
    case RHITextureFormat::BGRX8_UNorm: return DXGI_FORMAT_B8G8R8X8_UNORM;
    case RHITextureFormat::BGRA5551_UNorm: return DXGI_FORMAT_B5G5R5A1_UNORM;
    case RHITextureFormat::A8_UNorm: return DXGI_FORMAT_A8_UNORM;
    case RHITextureFormat::RG8_SNorm: return DXGI_FORMAT_R8G8_SNORM;
    case RHITextureFormat::BC1_UNorm: return DXGI_FORMAT_BC1_UNORM;
    case RHITextureFormat::BC2_UNorm: return DXGI_FORMAT_BC2_UNORM;
    case RHITextureFormat::BC3_UNorm: return DXGI_FORMAT_BC3_UNORM;
    case RHITextureFormat::D16_UNorm: return DXGI_FORMAT_D16_UNORM;
	case RHITextureFormat::R8_UNorm:
		return DXGI_FORMAT_R8_UNORM;
	case RHITextureFormat::RG8_UNorm:
		return DXGI_FORMAT_R8G8_UNORM;
	case RHITextureFormat::RGBA8_UNorm:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case RHITextureFormat::BGRA8_UNorm:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case RHITextureFormat::BGR565_UNorm:
		return DXGI_FORMAT_B5G6R5_UNORM;
	case RHITextureFormat::BGRA4444_UNorm:
		return DXGI_FORMAT_B4G4R4A4_UNORM;
	case RHITextureFormat::RGBA16_Float:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case RHITextureFormat::RGBA32_Float:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case RHITextureFormat::R32_Float:
		return DXGI_FORMAT_R32_FLOAT;
	case RHITextureFormat::D24_UNorm_S8:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case RHITextureFormat::D32_Float:
		return DXGI_FORMAT_D32_FLOAT;
	}

	return DXGI_FORMAT_UNKNOWN;
}

static std::uint32_t To_DX11_Bind_Flags(RHIBufferUsage usage) noexcept
{
	switch (usage) {
	case RHIBufferUsage::Vertex:
		return D3D11_BIND_VERTEX_BUFFER;
	case RHIBufferUsage::Index:
		return D3D11_BIND_INDEX_BUFFER;
	case RHIBufferUsage::Constant:
		return D3D11_BIND_CONSTANT_BUFFER;
	case RHIBufferUsage::Storage:
		return D3D11_BIND_SHADER_RESOURCE;
	}

	return 0;
}

static D3D11_BLEND To_DX11_Blend_Factor(RHIBlendFactor factor)
{
    switch(factor) {
    case RHIBlendFactor::Zero: return D3D11_BLEND_ZERO;
    case RHIBlendFactor::One: return D3D11_BLEND_ONE;
    case RHIBlendFactor::SourceColor: return D3D11_BLEND_SRC_COLOR;
    case RHIBlendFactor::InverseSourceColor: return D3D11_BLEND_INV_SRC_COLOR;
    case RHIBlendFactor::SourceAlpha: return D3D11_BLEND_SRC_ALPHA;
    case RHIBlendFactor::InverseSourceAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
    case RHIBlendFactor::DestinationColor: return D3D11_BLEND_DEST_COLOR;
    case RHIBlendFactor::InverseDestinationColor: return D3D11_BLEND_INV_DEST_COLOR;
    case RHIBlendFactor::DestinationAlpha: return D3D11_BLEND_DEST_ALPHA;
    case RHIBlendFactor::InverseDestinationAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
    }
    return static_cast<D3D11_BLEND>(0);
}

D3D11_STENCIL_OP To_DX11_Stencil(RHIStencilOperation operation)
{
    switch(operation) {
    case RHIStencilOperation::Keep: return D3D11_STENCIL_OP_KEEP;
    case RHIStencilOperation::Zero: return D3D11_STENCIL_OP_ZERO;
    case RHIStencilOperation::Replace: return D3D11_STENCIL_OP_REPLACE;
    case RHIStencilOperation::IncrementSaturate: return D3D11_STENCIL_OP_INCR_SAT;
    case RHIStencilOperation::DecrementSaturate: return D3D11_STENCIL_OP_DECR_SAT;
    case RHIStencilOperation::Invert: return D3D11_STENCIL_OP_INVERT;
    case RHIStencilOperation::Increment: return D3D11_STENCIL_OP_INCR;
    case RHIStencilOperation::Decrement: return D3D11_STENCIL_OP_DECR;
    }
    return static_cast<D3D11_STENCIL_OP>(0);
}

D3D11_COMPARISON_FUNC To_DX11_Comparison(RHIComparison comparison) noexcept
{
    switch (comparison) {
    case RHIComparison::Never: return D3D11_COMPARISON_NEVER;
    case RHIComparison::Less: return D3D11_COMPARISON_LESS;
    case RHIComparison::Equal: return D3D11_COMPARISON_EQUAL;
    case RHIComparison::LessEqual: return D3D11_COMPARISON_LESS_EQUAL;
    case RHIComparison::Greater: return D3D11_COMPARISON_GREATER;
    case RHIComparison::NotEqual: return D3D11_COMPARISON_NOT_EQUAL;
    case RHIComparison::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
    case RHIComparison::Always: return D3D11_COMPARISON_ALWAYS;
    }
    return static_cast<D3D11_COMPARISON_FUNC>(0);
}

static D3D11_PRIMITIVE_TOPOLOGY To_DX11_Topology(RHIPrimitiveTopology topology) noexcept
{
	switch (topology) {
	case RHIPrimitiveTopology::TriangleList:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case RHIPrimitiveTopology::TriangleStrip:
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case RHIPrimitiveTopology::PointList:
		return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	}

	return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

static D3D11_CULL_MODE To_DX11_Cull_Mode(RHICullMode mode) noexcept
{
	return mode == RHICullMode::None ? D3D11_CULL_NONE : D3D11_CULL_BACK;
}

static D3D11_BLEND_OP To_DX11_Blend_Operation(RHIBlendOperation operation) noexcept
{
	return operation == RHIBlendOperation::ReverseSubtract
		? D3D11_BLEND_OP_REV_SUBTRACT
		: D3D11_BLEND_OP_ADD;
}

static bool Create_DX11_Pipeline(
	ID3D11Device *device,
	const RHIPipeline &description,
	std::span<const std::byte> vertex_bytecode,
	std::span<const std::byte> pixel_bytecode,
	DX11Pipeline &pipeline) noexcept
{
	if (device == nullptr || (description.topology != RHIPrimitiveTopology::TriangleList
		&& description.topology != RHIPrimitiveTopology::TriangleStrip
		&& description.topology != RHIPrimitiveTopology::PointList)
		|| (description.vertex_format != RHIVertexFormat::Position3Color4UV2
			&& description.vertex_format != RHIVertexFormat::Position3Color4UV2ResourceIndex
			&& description.vertex_format != RHIVertexFormat::Position3Color4UV2Skinned
			&& description.vertex_format != RHIVertexFormat::Position3Color4UV2UV2Normal3))
		return false;
	if (description.sampler_count == 0 || description.sampler_count > description.samplers.size())
		return false;
	if (vertex_bytecode.empty() || pixel_bytecode.empty())
		return false;

	ID3D11VertexShader *native_vertex_shader = nullptr;
	if (FAILED(device->CreateVertexShader(vertex_bytecode.data(), vertex_bytecode.size(), nullptr, &native_vertex_shader)))
		return false;
	pipeline.vertex_shader.Reset(native_vertex_shader);

	ID3D11PixelShader *native_pixel_shader = nullptr;
	if (FAILED(device->CreatePixelShader(pixel_bytecode.data(), pixel_bytecode.size(), nullptr, &native_pixel_shader)))
		return false;
	pipeline.pixel_shader.Reset(native_pixel_shader);

	D3D11_INPUT_ELEMENT_DESC standard_input_elements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 1, DXGI_FORMAT_R32_UINT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	const D3D11_INPUT_ELEMENT_DESC skinned_input_elements[] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	if (description.vertex_format == RHIVertexFormat::Position3Color4UV2UV2Normal3)
		standard_input_elements[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	const bool is_skinned = description.vertex_format == RHIVertexFormat::Position3Color4UV2Skinned;
	const D3D11_INPUT_ELEMENT_DESC *input_elements = is_skinned ? skinned_input_elements : standard_input_elements;
	UINT input_element_count = (is_skinned || description.vertex_format == RHIVertexFormat::Position3Color4UV2UV2Normal3) ? 5u : description.vertex_format == RHIVertexFormat::Position3Color4UV2 ? 3u : 4u;
    std::array<D3D11_INPUT_ELEMENT_DESC,16> custom_elements{};
    if (description.vertex_element_count > 0) {
        if (description.vertex_element_count > custom_elements.size()) return false;
        constexpr const char* semantics[] = {"POSITION","COLOR","NORMAL","TEXCOORD"};
        constexpr DXGI_FORMAT formats[] = {DXGI_FORMAT_R32G32_FLOAT,DXGI_FORMAT_R32G32B32_FLOAT,DXGI_FORMAT_R32G32B32A32_FLOAT,DXGI_FORMAT_R32_FLOAT};
        for (unsigned i=0;i<description.vertex_element_count;++i) {
            const auto& source = description.vertex_elements[i];
            const auto semantic = static_cast<unsigned>(source.semantic);
            const auto format = static_cast<unsigned>(source.format);
            if (semantic >= std::size(semantics) || format >= std::size(formats)) return false;
            custom_elements[i] = {semantics[semantic],source.semantic_index,formats[format],0,source.offset,D3D11_INPUT_PER_VERTEX_DATA,0};
        }
        input_elements = custom_elements.data();
        input_element_count = description.vertex_element_count;
    }
	ID3D11InputLayout *input_layout = nullptr;
	if (FAILED(device->CreateInputLayout(input_elements, input_element_count, vertex_bytecode.data(), vertex_bytecode.size(), &input_layout)))
		return false;
	pipeline.input_layout.Reset(input_layout);

	D3D11_DEPTH_STENCIL_DESC depth_description{};
	depth_description.DepthEnable = description.depth_test ? TRUE : FALSE;
	depth_description.DepthWriteMask = description.depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_description.DepthFunc = To_DX11_Comparison(description.depth_comparison);
    depth_description.StencilEnable = description.stencil.enabled;
    depth_description.StencilReadMask = description.stencil.read_mask;
    depth_description.StencilWriteMask = description.stencil.write_mask;
    const auto stencil_face = [](const RHIStencilFace &face) {
        D3D11_DEPTH_STENCILOP_DESC result{};
        result.StencilFunc = To_DX11_Comparison(face.comparison);
        result.StencilFailOp = To_DX11_Stencil(face.fail);
        result.StencilDepthFailOp = To_DX11_Stencil(face.depth_fail);
        result.StencilPassOp = To_DX11_Stencil(face.pass);
        return result;
    };
    depth_description.FrontFace = stencil_face(description.stencil.front);
    depth_description.BackFace = stencil_face(description.stencil.back);

	ID3D11DepthStencilState *depth_state = nullptr;
	if (FAILED(device->CreateDepthStencilState(&depth_description, &depth_state)))
		return false;
	pipeline.depth_stencil_state.Reset(depth_state);

	D3D11_BLEND_DESC blend_description{};
	D3D11_RENDER_TARGET_BLEND_DESC &render_target_blend = blend_description.RenderTarget[0];
	render_target_blend.BlendEnable = description.blend_mode == RHIBlendMode::Disabled ? FALSE : TRUE;
	switch (description.blend_mode) {
	case RHIBlendMode::Disabled:
	case RHIBlendMode::Additive:
		render_target_blend.SrcBlend = D3D11_BLEND_ONE;
		render_target_blend.DestBlend = description.blend_mode == RHIBlendMode::Additive ? D3D11_BLEND_ONE : D3D11_BLEND_ZERO;
		break;
	case RHIBlendMode::Alpha:
		render_target_blend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		render_target_blend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		break;
	case RHIBlendMode::Multiply:
		render_target_blend.SrcBlend = D3D11_BLEND_ZERO;
		render_target_blend.DestBlend = D3D11_BLEND_SRC_COLOR;
		break;
	case RHIBlendMode::ColorMultiply:
		render_target_blend.SrcBlend = D3D11_BLEND_DEST_COLOR;
		render_target_blend.DestBlend = D3D11_BLEND_SRC_COLOR;
		break;
	}
	render_target_blend.BlendOp = To_DX11_Blend_Operation(description.blend_operation);
    if (description.custom_blend_factors) {
        render_target_blend.SrcBlend = To_DX11_Blend_Factor(description.source_blend);
        render_target_blend.DestBlend = To_DX11_Blend_Factor(description.destination_blend);
    }
	render_target_blend.SrcBlendAlpha = D3D11_BLEND_ONE;
	render_target_blend.DestBlendAlpha = description.blend_mode == RHIBlendMode::Alpha ? D3D11_BLEND_INV_SRC_ALPHA : D3D11_BLEND_ZERO;
    if (description.blend_alpha_like_color) {
        const auto alpha_factor = [](D3D11_BLEND factor) {
            if (factor == D3D11_BLEND_DEST_COLOR) return D3D11_BLEND_DEST_ALPHA;
            if (factor == D3D11_BLEND_SRC_COLOR) return D3D11_BLEND_SRC_ALPHA;
            if (factor == D3D11_BLEND_INV_SRC_COLOR) return D3D11_BLEND_INV_SRC_ALPHA;
            if (factor == D3D11_BLEND_INV_DEST_COLOR) return D3D11_BLEND_INV_DEST_ALPHA;
            return factor;
        };
        render_target_blend.SrcBlendAlpha = alpha_factor(render_target_blend.SrcBlend);
        render_target_blend.DestBlendAlpha = alpha_factor(render_target_blend.DestBlend);
    }

	render_target_blend.BlendOpAlpha = To_DX11_Blend_Operation(description.blend_operation);
	render_target_blend.RenderTargetWriteMask = description.color_write_mask & D3D11_COLOR_WRITE_ENABLE_ALL;
	ID3D11BlendState *blend_state = nullptr;
	if (FAILED(device->CreateBlendState(&blend_description, &blend_state)))
		return false;
	pipeline.blend_state.Reset(blend_state);

	D3D11_RASTERIZER_DESC rasterizer_description{};
	rasterizer_description.FillMode = description.wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	rasterizer_description.CullMode = To_DX11_Cull_Mode(description.cull_mode);
	rasterizer_description.FrontCounterClockwise = description.front_counter_clockwise ? TRUE : FALSE;
	rasterizer_description.DepthClipEnable = TRUE;
    rasterizer_description.DepthBias = description.depth_bias;
	rasterizer_description.ScissorEnable = description.scissor_test ? TRUE : FALSE;
	ID3D11RasterizerState *rasterizer_state = nullptr;
	if (FAILED(device->CreateRasterizerState(&rasterizer_description, &rasterizer_state)))
		return false;
	pipeline.rasterizer_state.Reset(rasterizer_state);

	for (std::size_t slot = 0; slot < description.sampler_count; ++slot) {
        const RHISamplerDescription &sampler = description.samplers[slot];
        D3D11_SAMPLER_DESC sampler_description{};
        sampler_description.Filter = sampler.anisotropy > 1 ? D3D11_FILTER_ANISOTROPIC :
            D3D11_ENCODE_BASIC_FILTER(sampler.minification == RHISamplerFilter::Linear ? D3D11_FILTER_TYPE_LINEAR : D3D11_FILTER_TYPE_POINT,
                sampler.magnification == RHISamplerFilter::Linear ? D3D11_FILTER_TYPE_LINEAR : D3D11_FILTER_TYPE_POINT,
                sampler.mipmap == RHISamplerFilter::Linear ? D3D11_FILTER_TYPE_LINEAR : D3D11_FILTER_TYPE_POINT,
                D3D11_FILTER_REDUCTION_TYPE_STANDARD);
        sampler_description.AddressU = sampler.address[0] == RHISamplerAddress::Clamp ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_description.AddressV = sampler.address[1] == RHISamplerAddress::Clamp ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_description.AddressW = sampler.address[2] == RHISamplerAddress::Clamp ? D3D11_TEXTURE_ADDRESS_CLAMP : D3D11_TEXTURE_ADDRESS_WRAP;
        sampler_description.ComparisonFunc = D3D11_COMPARISON_NEVER;
        sampler_description.MaxAnisotropy = std::clamp<unsigned>(sampler.anisotropy, 1, D3D11_REQ_MAXANISOTROPY);
        sampler_description.MinLOD = sampler.min_lod;
        sampler_description.MaxLOD = sampler.max_lod;
        ID3D11SamplerState *sampler_state = nullptr;
        if (FAILED(device->CreateSamplerState(&sampler_description, &sampler_state)))
            return false;
        pipeline.sampler_states[slot].Reset(sampler_state);
    }

	pipeline.sampler_count = description.sampler_count;
    pipeline.stencil_reference = description.stencil.reference;
	pipeline.key = description.key;
	pipeline.topology = description.topology;
	return true;
}

static bool Has_Texture_Usage(const RHITexture &description, RHITextureUsage usage) noexcept
{
	return (description.usage & static_cast<std::uint32_t>(usage)) != 0;
}

static std::uint32_t To_DX11_Texture_Bind_Flags(const RHITexture &description) noexcept
{
	std::uint32_t flags = 0;
	if (Has_Texture_Usage(description, RHITextureUsage::ShaderResource))
		flags |= D3D11_BIND_SHADER_RESOURCE;
	if (Has_Texture_Usage(description, RHITextureUsage::RenderTarget))
		flags |= D3D11_BIND_RENDER_TARGET;
	if (Has_Texture_Usage(description, RHITextureUsage::DepthStencil))
		flags |= D3D11_BIND_DEPTH_STENCIL;
	if (Has_Texture_Usage(description, RHITextureUsage::UnorderedAccess))
		flags |= D3D11_BIND_UNORDERED_ACCESS;
	return flags;
}

static std::uint32_t To_DX11_Bytes_Per_Pixel(RHITextureFormat format) noexcept
{
	switch (format) {
    case RHITextureFormat::A8_UNorm: return 1;
    case RHITextureFormat::BGRA5551_UNorm:
    case RHITextureFormat::RG8_SNorm:
    case RHITextureFormat::D16_UNorm: return 2;
    case RHITextureFormat::BGRX8_UNorm: return 4;
	case RHITextureFormat::R8_UNorm:
		return 1;
	case RHITextureFormat::RG8_UNorm:
	case RHITextureFormat::BGR565_UNorm:
	case RHITextureFormat::BGRA4444_UNorm:
		return 2;
	case RHITextureFormat::RGBA8_UNorm:
	case RHITextureFormat::BGRA8_UNorm:
	case RHITextureFormat::R32_Float:
	case RHITextureFormat::D24_UNorm_S8:
	case RHITextureFormat::D32_Float:
		return 4;
	case RHITextureFormat::RGBA16_Float:
		return 8;
	case RHITextureFormat::RGBA32_Float:
		return 16;
	}

	return 0;
}

static bool Is_Block_Compressed(RHITextureFormat format) noexcept
{
    return format == RHITextureFormat::BC1_UNorm || format == RHITextureFormat::BC2_UNorm
        || format == RHITextureFormat::BC3_UNorm;
}

struct TextureTransferLayout final
{
    std::uint32_t width, height, depth, rows, row_bytes, row_pitch, slice_pitch, subresource;
};

static bool Texture_Transfer_Layout(const RHITexture& description, std::uint32_t mip,
    std::uint32_t layer, std::uint32_t row_pitch, std::uint32_t slice_pitch,
    std::size_t capacity, TextureTransferLayout& output) noexcept
{
    if (mip >= description.mip_count || mip >= 32 || layer >= description.array_size) return false;
    const auto width = std::max(1u, description.width >> mip);
    const auto height = std::max(1u, description.height >> mip);
    const auto depth = std::max(1u, description.depth >> mip);
    const bool compressed = Is_Block_Compressed(description.format);
    const auto rows = compressed ? (height + 3u) / 4u : height;
    const std::uint64_t row_bytes = compressed
        ? static_cast<std::uint64_t>((width + 3u) / 4u) * (description.format == RHITextureFormat::BC1_UNorm ? 8u : 16u)
        : static_cast<std::uint64_t>(width) * To_DX11_Bytes_Per_Pixel(description.format);
    if (row_bytes == 0 || row_bytes > UINT32_MAX) return false;
    if (row_pitch == 0) row_pitch = static_cast<std::uint32_t>(row_bytes);
    const std::uint64_t minimum_slice = static_cast<std::uint64_t>(row_pitch) * rows;
    if (row_pitch < row_bytes || minimum_slice > UINT32_MAX) return false;
    if (slice_pitch == 0) slice_pitch = static_cast<std::uint32_t>(minimum_slice);
    const auto required = static_cast<std::uint64_t>(slice_pitch) * (depth - 1u)
        + static_cast<std::uint64_t>(row_pitch) * (rows - 1u) + row_bytes;
    if (slice_pitch < minimum_slice || capacity < required) return false;
    output = {width, height, depth, rows, static_cast<std::uint32_t>(row_bytes), row_pitch,
        slice_pitch, mip + layer * description.mip_count};
    return true;
}

export struct DX11DeviceOptions final
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

static bool Create_DX11_Device(DX11DeviceState &state, const DX11DeviceOptions &options) noexcept
{
    if (options.backbuffer_format != RHITextureFormat::RGBA8_UNorm
        && options.backbuffer_format != RHITextureFormat::BGRA8_UNorm) return false;

    const D3D_FEATURE_LEVEL feature_levels[] = {D3D_FEATURE_LEVEL_11_0};
	const D3D_DRIVER_TYPE requested_driver = options.use_warp ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE;
    D3D_FEATURE_LEVEL selected_feature_level = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device *native_device = nullptr;
	ID3D11DeviceContext *native_context = nullptr;
	IDXGISwapChain *native_swap_chain = nullptr;
	HRESULT result = E_FAIL;
	if (options.window != nullptr && options.width != 0 && options.height != 0) {
		DXGI_SWAP_CHAIN_DESC swap_chain_description{};
		swap_chain_description.BufferDesc.Width = options.width;
		swap_chain_description.BufferDesc.Height = options.height;
        swap_chain_description.BufferDesc.Format = options.backbuffer_format == RHITextureFormat::BGRA8_UNorm
            ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
		swap_chain_description.BufferDesc.RefreshRate.Numerator = 60;
		swap_chain_description.BufferDesc.RefreshRate.Denominator = 1;
		swap_chain_description.SampleDesc.Count = 1;
		swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swap_chain_description.BufferCount = 2;
		swap_chain_description.OutputWindow = static_cast<HWND>(options.window);
		swap_chain_description.Windowed = TRUE;
		swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			requested_driver,
			nullptr,
			0,
			feature_levels,
			static_cast<UINT>(std::size(feature_levels)),
			D3D11_SDK_VERSION,
			&swap_chain_description,
			&native_swap_chain,
			&native_device,
			&selected_feature_level,
			&native_context);
	} else {
		result = D3D11CreateDevice(
			nullptr,
			requested_driver,
			nullptr,
			0,
			feature_levels,
			static_cast<UINT>(std::size(feature_levels)),
			D3D11_SDK_VERSION,
			&native_device,
			&selected_feature_level,
			&native_context);
	}

	if (SUCCEEDED(result)) {
		state.device.Reset(native_device);
		state.context.Reset(native_context);
		state.native_swap_chain.Reset(native_swap_chain);
	} else {
		if (native_device != nullptr)
			native_device->Release();
		if (native_context != nullptr)
			native_context->Release();
		if (native_swap_chain != nullptr)
			native_swap_chain->Release();
	}
	return SUCCEEDED(result) && state.device.Get() != nullptr && state.context.Get() != nullptr;
}

static bool Load_Shader_Binary(const std::string &directory, const char *name, std::vector<std::byte> &data)
{
	const std::string path = directory.empty() ? std::string(name) : directory + "/" + name;
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file)
		return false;

	const std::streampos end = file.tellg();
	if (end <= 0)
		return false;

	data.resize(static_cast<std::size_t>(end));
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
	return file.good() || file.eof();
}

export class DX11Device final : public Device
{
public:
	explicit DX11Device(DX11DeviceOptions options = {});
	~DX11Device() noexcept override = default;

	DX11Device(const DX11Device &) = delete;
	DX11Device &operator=(const DX11Device &) = delete;

	bool Is_Valid() const noexcept override;
	RHIDeviceStatus Get_Status() const noexcept override;
	bool Get_Adapter_Info(RHIAdapterInfo& info) const noexcept override;
	RHITextureLimits Texture_Limits() const noexcept override;
	RHIBufferHandle Create_Buffer(const RHIBuffer &description) override;
	RHITextureHandle Create_Texture(const RHITexture &description) override;
	RHIPipelineHandle Create_Pipeline(const RHIPipeline &description) override;
	RHIBufferHandle Create_Buffer_Initialized(const RHIBuffer &description, std::span<const std::byte> initial_data) override;
	RHITextureHandle Create_Texture_Initialized(const RHITexture &description, const RHITextureUpload &initial_data) override;
	RHIPipelineHandle Create_Pipeline(const RHIPipeline &description, RHIShaderBytecode vertex_shader, RHIShaderBytecode fragment_shader) override;
	bool Update_Buffer(RHIBufferHandle buffer, std::uint32_t offset, std::span<const std::byte> data) noexcept override;
	bool Update_Texture(RHITextureHandle texture, const RHITextureUpload &data) noexcept override;
	bool Readback_Texture(RHITextureHandle texture, std::span<std::byte> data, std::uint32_t row_pitch) noexcept override;
	bool Readback_Texture_Subresource(RHITextureHandle texture, const RHITextureReadback& data) noexcept override;
    bool Generate_Texture_Mips(RHITextureHandle texture) noexcept override;
    bool Map_Texture(RHITextureHandle texture, std::uint32_t mip, std::uint32_t layer, bool read_only, RHITextureMapping& mapping) override;
    bool Unmap_Texture(RHITextureHandle texture, std::uint32_t mip, std::uint32_t layer) noexcept override;
    bool Retain_Texture(RHITextureHandle texture) noexcept override;
    bool Destroy_Buffer(RHIBufferHandle buffer) noexcept override;
	bool Destroy_Texture(RHITextureHandle texture) noexcept override;
	bool Destroy_Pipeline(RHIPipelineHandle pipeline) noexcept override;
	CommandList &Immediate_Command_List() noexcept override;
	SwapChain &Get_Swap_Chain() noexcept override;
    bool Set_Exclusive_Fullscreen(bool fullscreen) noexcept;
	bool Begin_Frame() noexcept override;
	bool End_Frame() noexcept override;

private:
	std::unique_ptr<DX11DeviceState> m_state;
};

DX11SwapChain::~DX11SwapChain() noexcept
{
	if (m_state == nullptr)
		return;

	m_state->textures.Destroy(m_backbuffer);
	m_state->textures.Destroy(m_depth_target);
}

bool DX11SwapChain::Is_Valid() const noexcept
{
	return m_state != nullptr
		&& m_state->device.Get() != nullptr
		&& m_state->context.Get() != nullptr
		&& m_state->native_swap_chain.Get() != nullptr
		&& m_backbuffer.Is_Valid()
		&& m_depth_target.Is_Valid();
}

RHIBackbuffer DX11SwapChain::Backbuffer() const noexcept
{
	return {m_backbuffer, m_width, m_height};
}

RHIDepthTarget DX11SwapChain::Depth_Target() const noexcept
{
	return {m_depth_target, m_width, m_height};
}

bool DX11SwapChain::Create_Targets(std::uint32_t width, std::uint32_t height)
{
	if (m_state == nullptr || m_state->device.Get() == nullptr || m_state->context.Get() == nullptr || m_state->native_swap_chain.Get() == nullptr || width == 0 || height == 0)
		return false;

	ID3D11DeviceContext *context = m_state->context.Get();
	context->OMSetRenderTargets(0, nullptr, nullptr);
	m_state->command_list.Invalidate_Render_Target_Binding();
	m_state->textures.Destroy(m_backbuffer);
	m_state->textures.Destroy(m_depth_target);
	m_backbuffer = {};
	m_depth_target = {};

	ID3D11Texture2D *native_backbuffer = nullptr;
	if (FAILED(m_state->native_swap_chain.Get()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&native_backbuffer))))
		return false;

	DX11Texture backbuffer;
	backbuffer.object.Reset(native_backbuffer);
	backbuffer.width = width;
	backbuffer.height = height;
	D3D11_TEXTURE2D_DESC backbuffer_description{};
	native_backbuffer->GetDesc(&backbuffer_description);
	backbuffer.format = backbuffer_description.Format == DXGI_FORMAT_B8G8R8A8_UNORM
		? RHITextureFormat::BGRA8_UNorm : RHITextureFormat::RGBA8_UNorm;
    backbuffer.description = {width, height, 1, backbuffer.format};
	ID3D11RenderTargetView *native_render_target = nullptr;
	if (FAILED(m_state->device.Get()->CreateRenderTargetView(native_backbuffer, nullptr, &native_render_target)))
		return false;
	backbuffer.render_target_view.Reset(native_render_target);
	const RHITextureHandle backbuffer_handle = m_state->textures.Create(std::move(backbuffer));
	if (!backbuffer_handle.Is_Valid())
		return false;

	D3D11_TEXTURE2D_DESC depth_description{};
	depth_description.Width = width;
	depth_description.Height = height;
	depth_description.MipLevels = 1;
	depth_description.ArraySize = 1;
	depth_description.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depth_description.SampleDesc.Count = 1;
	depth_description.Usage = D3D11_USAGE_DEFAULT;
	depth_description.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ID3D11Texture2D *native_depth = nullptr;
	if (FAILED(m_state->device.Get()->CreateTexture2D(&depth_description, nullptr, &native_depth))) {
		m_state->textures.Destroy(backbuffer_handle);
		return false;
	}

	DX11Texture depth;
	depth.object.Reset(native_depth);
	depth.width = width;
	depth.height = height;
	depth.format = RHITextureFormat::D24_UNorm_S8;
    depth.description = {width, height, 1, depth.format, static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)};
	ID3D11DepthStencilView *native_depth_view = nullptr;
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_view_description{};
	depth_view_description.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_view_description.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	if (FAILED(m_state->device.Get()->CreateDepthStencilView(native_depth, &depth_view_description, &native_depth_view))) {
		m_state->textures.Destroy(backbuffer_handle);
		return false;
	}
	depth.depth_stencil_view.Reset(native_depth_view);
	const RHITextureHandle depth_handle = m_state->textures.Create(std::move(depth));
	if (!depth_handle.Is_Valid()) {
		m_state->textures.Destroy(backbuffer_handle);
		return false;
	}

	m_backbuffer = backbuffer_handle;
	m_depth_target = depth_handle;
	m_width = width;
	m_height = height;
	return true;
}

bool DX11SwapChain::Resize(std::uint32_t width, std::uint32_t height)
{
	if (m_state == nullptr || m_state->device.Get() == nullptr || m_state->native_swap_chain.Get() == nullptr
		|| m_state->frame_active || width == 0 || height == 0)
		return false;

	const auto* color = m_state->textures.Resolve(m_backbuffer);
	const auto* depth = m_state->textures.Resolve(m_depth_target);
	// A retained frame attachment must remain valid until its consumer releases it.
	if ((color != nullptr && color->references > 1) || (depth != nullptr && depth->references > 1))
		return false;

	m_state->context.Get()->ClearState();
	m_state->command_list.Reset_Frame_State();
	m_state->textures.Destroy(m_backbuffer);
	m_state->textures.Destroy(m_depth_target);
	m_backbuffer = {};
	m_depth_target = {};
	m_width = 0;
	m_height = 0;
	if (FAILED(m_state->native_swap_chain.Get()->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)))
		return false;

	return Create_Targets(width, height);
}

bool DX11SwapChain::Present() noexcept
{
    GRAPHICS_PROFILE_SCOPE("Graphics.DX11.Present");
	if (!Is_Valid() || m_state->frame_active || !m_state->ready_to_present || m_state->presented)
		return false;

	if (FAILED(m_state->native_swap_chain.Get()->Present(0, 0)))
		return false;

	m_state->ready_to_present = false;
	m_state->presented = true;
	return true;
}

bool DX11CommandList::Is_Ready() const noexcept
{
	return m_state != nullptr && m_state->context.Get() != nullptr;
}

bool DX11CommandList::Is_Pipeline_Valid() const noexcept
{
	return Is_Ready() && m_pipeline.Is_Valid() && m_state->pipelines.Resolve(m_pipeline) != nullptr;
}

bool DX11CommandList::Bind_Pipeline(RHIPipelineHandle pipeline) noexcept
{
    GRAPHICS_PROFILE_SCOPE("Graphics.DX11.BindPipeline");
	if (!Is_Ready())
		return false;

	DX11Pipeline *resource = m_state->pipelines.Resolve(pipeline);
	if (resource == nullptr || resource->vertex_shader.Get() == nullptr || resource->pixel_shader.Get() == nullptr || resource->input_layout.Get() == nullptr || resource->depth_stencil_state.Get() == nullptr || resource->blend_state.Get() == nullptr || resource->rasterizer_state.Get() == nullptr || resource->sampler_states[0].Get() == nullptr)
		return false;
	if (m_pipeline == pipeline)
		return true;

    const DX11Pipeline* previous = m_state->pipelines.Resolve(m_pipeline);
    ID3D11DeviceContext* context = m_state->context.Get();
    if (!previous || previous->input_layout.Get() != resource->input_layout.Get())
        context->IASetInputLayout(resource->input_layout.Get());
    if (!previous || previous->vertex_shader.Get() != resource->vertex_shader.Get())
        context->VSSetShader(resource->vertex_shader.Get(), nullptr, 0);
    if (!previous || previous->pixel_shader.Get() != resource->pixel_shader.Get())
        context->PSSetShader(resource->pixel_shader.Get(), nullptr, 0);
    std::array<ID3D11SamplerState*, 16> samplers{};
    bool samplers_changed = !previous || previous->sampler_count < resource->sampler_count;
    for (std::size_t slot = 0; slot < resource->sampler_count; ++slot) {
        samplers[slot] = resource->sampler_states[slot].Get();
        samplers_changed |= !previous || previous->sampler_states[slot].Get() != samplers[slot];
    }
    if (samplers_changed) {
        context->PSSetSamplers(0, resource->sampler_count, samplers.data());
        context->VSSetSamplers(0, resource->sampler_count, samplers.data());
    }
    if (!previous || previous->depth_stencil_state.Get() != resource->depth_stencil_state.Get()
        || previous->stencil_reference != resource->stencil_reference)
        context->OMSetDepthStencilState(resource->depth_stencil_state.Get(), resource->stencil_reference);
    if (!previous || previous->blend_state.Get() != resource->blend_state.Get())
        context->OMSetBlendState(resource->blend_state.Get(), nullptr, 0xffffffffu);
    if (!previous || previous->rasterizer_state.Get() != resource->rasterizer_state.Get())
        context->RSSetState(resource->rasterizer_state.Get());
    if (!previous || previous->topology != resource->topology)
        context->IASetPrimitiveTopology(To_DX11_Topology(resource->topology));
	m_pipeline = pipeline;
	m_topology = resource->topology;
	return true;
}

bool DX11CommandList::Bind_Texture_At_Slot(RHIShaderStage stage, std::uint32_t slot, RHITextureHandle texture) noexcept
{
	if (!Is_Ready())
		return false;

	DX11Texture *resource = m_state->textures.Resolve(texture);
	if (resource == nullptr || resource->shader_resource_view.Get() == nullptr)
		return false;

	auto& binding = m_shader_resources[stage == RHIShaderStage::Vertex ? 0 : 1][slot];
	if (binding.texture == texture)
		return true;

	ID3D11ShaderResourceView *view = resource->shader_resource_view.Get();
	if (stage == RHIShaderStage::Vertex)
		m_state->context.Get()->VSSetShaderResources(slot, 1, &view);
	else
		m_state->context.Get()->PSSetShaderResources(slot, 1, &view);
	// An output conflict makes DX11 bind null instead of the requested view.
	// Such a request must never become a remembered successful binding.
	binding = texture == m_color_target || texture == m_depth_target
		? ShaderResourceBinding{} : ShaderResourceBinding{texture, {}};
	return true;
}

bool DX11CommandList::Bind_Buffer_At_Slot(RHIShaderStage stage, std::uint32_t slot, RHIBufferHandle buffer) noexcept
{
	if (!Is_Ready())
		return false;

	DX11Buffer *resource = m_state->buffers.Resolve(buffer);
	if (resource == nullptr || resource->object.Get() == nullptr)
		return false;

	ID3D11DeviceContext *context = m_state->context.Get();
	if (resource->usage == RHIBufferUsage::Storage) {
		if (resource->shader_resource_view.Get() == nullptr)
			return false;
		auto& binding = m_shader_resources[stage == RHIShaderStage::Vertex ? 0 : 1][slot];
		if (binding.buffer == buffer)
			return true;
		ID3D11ShaderResourceView *view = resource->shader_resource_view.Get();
		if (stage == RHIShaderStage::Vertex)
			context->VSSetShaderResources(slot, 1, &view);
		else
			context->PSSetShaderResources(slot, 1, &view);
		binding = {{}, buffer};
		return true;
	}

	if (resource->usage != RHIBufferUsage::Constant)
		return false;
	auto& bindings = m_constant_buffers[stage == RHIShaderStage::Vertex ? 0 : 1];
	if (slot < bindings.size() && bindings[slot] == buffer)
		return true;
	ID3D11Buffer *native_buffer = resource->object.Get();
	if (stage == RHIShaderStage::Vertex)
		context->VSSetConstantBuffers(slot, 1, &native_buffer);
	else
		context->PSSetConstantBuffers(slot, 1, &native_buffer);
	if (slot < bindings.size()) bindings[slot] = buffer;
	if (slot == 1) m_draw_constants_bound[stage == RHIShaderStage::Vertex ? 0 : 1] = false;
	return true;
}

bool DX11CommandList::Set_Bindless_Resources(std::span<const RHIBindlessResource> resources) noexcept
{
	if (!Is_Ready())
		return false;

	m_bindless_resources = resources;
	std::uint32_t storage_buffer_slot = 127;
	for (const RHIBindlessResource &resource : resources) {
		switch (resource.type) {
		case RHIResourceType::Buffer:
			if (!Bind_Buffer_At_Slot(RHIShaderStage::Vertex, storage_buffer_slot, resource.buffer)
				|| !Bind_Buffer_At_Slot(RHIShaderStage::Fragment, storage_buffer_slot, resource.buffer))
				return false;
			if (storage_buffer_slot == 0)
				return false;
			--storage_buffer_slot;
			break;
		case RHIResourceType::Texture:
			if (resource.index.Get_Index() >= 128 || !Bind_Texture_At_Slot(resource.stage, resource.index.Get_Index(), resource.texture))
				return false;
			break;
		case RHIResourceType::Material:
		{
			const auto slot = resource.constant_buffer_slot;
			if (slot >= D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT
				|| !Bind_Buffer_At_Slot(RHIShaderStage::Vertex, slot, resource.buffer)
				|| !Bind_Buffer_At_Slot(RHIShaderStage::Fragment, slot, resource.buffer))
				return false;
			break;
		}
		case RHIResourceType::Sampler:
		case RHIResourceType::Invalid:
			break;
		}
	}
	return true;
}

bool DX11CommandList::Set_Render_Targets(RHITextureHandle color_target, RHITextureHandle depth_target) noexcept
{
	if (!Is_Ready())
		return false;

	DX11Texture *color = m_state->textures.Resolve(color_target);
	DX11Texture *depth = m_state->textures.Resolve(depth_target);
	if (color == nullptr || color->render_target_view.Get() == nullptr || depth == nullptr || depth->depth_stencil_view.Get() == nullptr)
		return false;

	if (m_targets_bound && m_color_target == color_target && m_depth_target == depth_target)
		return true;

	ID3D11RenderTargetView *color_view = color->render_target_view.Get();
	m_state->context.Get()->OMSetRenderTargets(1, &color_view, depth->depth_stencil_view.Get());
	// Output binding can implicitly unbind overlapping shader resources.
	m_shader_resources = {};
	m_targets_bound = true;
	m_color_target = color_target;
	m_depth_target = depth_target;
	return true;
}

bool DX11CommandList::Set_Color_Target(RHITextureHandle color_target) noexcept
{
	if (!Is_Ready())
		return false;

	DX11Texture *color = m_state->textures.Resolve(color_target);
	if (color == nullptr || color->render_target_view.Get() == nullptr)
		return false;

	if (m_targets_bound && m_color_target == color_target && !m_depth_target.Is_Valid())
		return true;

	ID3D11RenderTargetView *color_view = color->render_target_view.Get();
	m_state->context.Get()->OMSetRenderTargets(1, &color_view, nullptr);
	m_shader_resources = {};
	m_targets_bound = true;
	m_color_target = color_target;
	m_depth_target = {};
	return true;
}

bool DX11CommandList::Set_Depth_Target(RHITextureHandle depth_target) noexcept
{
	if (!Is_Ready())
		return false;

	DX11Texture *depth = m_state->textures.Resolve(depth_target);
	if (depth == nullptr || depth->depth_stencil_view.Get() == nullptr)
		return false;

	if (m_targets_bound && !m_color_target.Is_Valid() && m_depth_target == depth_target)
		return true;

	m_state->context.Get()->OMSetRenderTargets(0, nullptr, depth->depth_stencil_view.Get());
	m_shader_resources = {};
	m_targets_bound = true;
	m_color_target = {};
	m_depth_target = depth_target;
	return true;
}

bool DX11CommandList::Clear(const std::array<float, 4> &color, float depth) noexcept
{
	if (!Is_Ready() || !m_color_target.Is_Valid() || !m_depth_target.Is_Valid())
		return false;

	DX11Texture *color_target = m_state->textures.Resolve(m_color_target);
	DX11Texture *depth_target = m_state->textures.Resolve(m_depth_target);
	if (color_target == nullptr || color_target->render_target_view.Get() == nullptr || depth_target == nullptr || depth_target->depth_stencil_view.Get() == nullptr)
		return false;

	m_state->context.Get()->ClearRenderTargetView(color_target->render_target_view.Get(), color.data());
	m_state->context.Get()->ClearDepthStencilView(depth_target->depth_stencil_view.Get(), D3D11_CLEAR_DEPTH, depth, 0);
	return true;
}

bool DX11CommandList::Clear_Depth(float depth) noexcept
{
	if (!Is_Ready() || !m_depth_target.Is_Valid())
		return false;

	DX11Texture *depth_target = m_state->textures.Resolve(m_depth_target);
	if (depth_target == nullptr || depth_target->depth_stencil_view.Get() == nullptr)
		return false;

	m_state->context.Get()->ClearDepthStencilView(depth_target->depth_stencil_view.Get(), D3D11_CLEAR_DEPTH, depth, 0);
	return true;
}

bool DX11CommandList::Clear_Color_Target(RHITextureHandle texture, const std::array<float, 4>& color) noexcept
{
    if (!Is_Ready()) return false;
    auto* resource = m_state->textures.Resolve(texture);
    if (resource == nullptr || resource->render_target_view.Get() == nullptr) return false;
    m_state->context.Get()->ClearRenderTargetView(resource->render_target_view.Get(), color.data());
    return true;
}

bool DX11CommandList::Clear_Depth_Stencil_Target(RHITextureHandle texture, float depth, std::uint8_t stencil) noexcept
{
    if (!Is_Ready()) return false;
    auto* resource = m_state->textures.Resolve(texture);
    if (resource == nullptr || resource->depth_stencil_view.Get() == nullptr) return false;
    const auto flags = D3D11_CLEAR_DEPTH | (resource->format == RHITextureFormat::D24_UNorm_S8 ? D3D11_CLEAR_STENCIL : 0);
    m_state->context.Get()->ClearDepthStencilView(resource->depth_stencil_view.Get(), flags, depth, stencil);
    return true;
}

bool DX11CommandList::Copy_Texture(RHITextureHandle source, RHITextureHandle destination) noexcept
{
	if (!Is_Ready() || !source.Is_Valid() || !destination.Is_Valid())
		return false;

	DX11Texture *source_texture = m_state->textures.Resolve(source);
	DX11Texture *destination_texture = m_state->textures.Resolve(destination);
	if (source_texture == nullptr || destination_texture == nullptr
		|| source_texture->Resource() == nullptr || destination_texture->Resource() == nullptr
		|| source_texture->width != destination_texture->width || source_texture->height != destination_texture->height
		|| source_texture->format != destination_texture->format
        || source_texture->description.depth != destination_texture->description.depth
        || source_texture->description.mip_count != destination_texture->description.mip_count
        || source_texture->description.array_size != destination_texture->description.array_size
        || source_texture->description.dimension != destination_texture->description.dimension)
		return false;

	m_state->context.Get()->CopyResource(destination_texture->Resource(), source_texture->Resource());
	return true;
}

bool DX11CommandList::Set_Viewport(RHIViewport viewport) noexcept
{
	if (!Is_Ready() || viewport.width == 0 || viewport.height == 0)
		return false;

	D3D11_VIEWPORT native_viewport{};
	native_viewport.TopLeftX = static_cast<float>(viewport.x);
	native_viewport.TopLeftY = static_cast<float>(viewport.y);
	native_viewport.Width = static_cast<float>(viewport.width);
	native_viewport.Height = static_cast<float>(viewport.height);
	native_viewport.MinDepth = viewport.min_depth;
	native_viewport.MaxDepth = viewport.max_depth;
	m_state->context.Get()->RSSetViewports(1, &native_viewport);
	return true;
}

bool DX11CommandList::Set_Scissor(RHIScissorRect scissor) noexcept
{
	if (!Is_Ready() || scissor.width == 0 || scissor.height == 0
		|| scissor.x > static_cast<std::uint32_t>(LONG_MAX)
		|| scissor.y > static_cast<std::uint32_t>(LONG_MAX)
		|| scissor.width > static_cast<std::uint32_t>(LONG_MAX)
		|| scissor.height > static_cast<std::uint32_t>(LONG_MAX)
		|| scissor.x > static_cast<std::uint32_t>(LONG_MAX) - scissor.width
		|| scissor.y > static_cast<std::uint32_t>(LONG_MAX) - scissor.height)
		return false;

	D3D11_RECT native_scissor{};
	native_scissor.left = static_cast<LONG>(scissor.x);
	native_scissor.top = static_cast<LONG>(scissor.y);
	native_scissor.right = static_cast<LONG>(scissor.x + scissor.width);
	native_scissor.bottom = static_cast<LONG>(scissor.y + scissor.height);
	m_state->context.Get()->RSSetScissorRects(1, &native_scissor);
	return true;
}

bool DX11CommandList::Set_Draw_Constants(std::span<const std::byte> data) noexcept
{
	if (!Is_Ready() || data.empty() || data.size() > 256)
		return false;

	const std::uint32_t byte_size = static_cast<std::uint32_t>((data.size() + 15u) & ~std::size_t(15u));
	if (m_draw_constants.Get() == nullptr || m_draw_constants_size != byte_size) {
		D3D11_BUFFER_DESC description{};
		description.ByteWidth = byte_size;
		description.Usage = D3D11_USAGE_DEFAULT;
		description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		ID3D11Buffer *native_buffer = nullptr;
		if (FAILED(m_state->device.Get()->CreateBuffer(&description, nullptr, &native_buffer)))
			return false;
		m_draw_constants.Reset(native_buffer);
		m_draw_constants_size = byte_size;
		m_draw_constants_data_size = 0;
		m_draw_constants_bound = {};
	}

	if (m_draw_constants_data_size != data.size()
		|| std::memcmp(m_draw_constants_data.data(), data.data(), data.size()) != 0) {
		m_draw_constants_data.fill(std::byte{});
		std::memcpy(m_draw_constants_data.data(), data.data(), data.size());
		m_state->context.Get()->UpdateSubresource(m_draw_constants.Get(), 0, nullptr,
			m_draw_constants_data.data(), 0, 0);
		m_draw_constants_data_size = data.size();
	}
	ID3D11Buffer *native_buffer = m_draw_constants.Get();
	if (!m_draw_constants_bound[0]) {
		m_state->context.Get()->VSSetConstantBuffers(1, 1, &native_buffer);
		m_constant_buffers[0][1] = {};
		m_draw_constants_bound[0] = true;
	}
	if (!m_draw_constants_bound[1]) {
		m_state->context.Get()->PSSetConstantBuffers(1, 1, &native_buffer);
		m_constant_buffers[1][1] = {};
		m_draw_constants_bound[1] = true;
	}
	return true;
}

bool DX11CommandList::Set_Vertex_Buffer(std::uint32_t slot, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t offset) noexcept
{
    GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX11.SetVertexBuffer");
	if (!Is_Ready())
		return false;

	DX11Buffer *resource = m_state->buffers.Resolve(buffer);
	if (resource == nullptr || resource->object.Get() == nullptr || stride == 0)
		return false;

	ID3D11Buffer *native_buffer = resource->object.Get();
	const UINT native_stride = stride;
	const UINT native_offset = offset;
	m_state->context.Get()->IASetVertexBuffers(slot, 1, &native_buffer, &native_stride, &native_offset);
	return true;
}

bool DX11CommandList::Set_Index_Buffer(RHIBufferHandle buffer, RHIIndexFormat format, std::uint32_t offset) noexcept
{
    GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX11.SetIndexBuffer");
	if (!Is_Ready())
		return false;

	DX11Buffer *resource = m_state->buffers.Resolve(buffer);
	if (resource == nullptr || resource->object.Get() == nullptr)
		return false;

	const DXGI_FORMAT native_format = format == RHIIndexFormat::UInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	m_state->context.Get()->IASetIndexBuffer(resource->object.Get(), native_format, offset);
	return true;
}

bool DX11CommandList::Draw(std::uint32_t vertex_count, std::uint32_t first_vertex, std::uint32_t instance_count, std::uint32_t first_instance) noexcept
{
    GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX11.Draw");
	if (!Is_Pipeline_Valid() || vertex_count == 0 || instance_count == 0)
		return false;

	if (instance_count == 1 && first_instance == 0)
		m_state->context.Get()->Draw(vertex_count, first_vertex);
	else
		m_state->context.Get()->DrawInstanced(vertex_count, instance_count, first_vertex, first_instance);
	Record_Draw(m_topology, vertex_count, instance_count);
	return true;
}

bool DX11CommandList::Draw_Indexed(std::uint32_t index_count, std::uint32_t first_index, std::int32_t base_vertex, std::uint32_t instance_count, std::uint32_t first_instance) noexcept
{
    GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX11.DrawIndexed");
	if (!Is_Pipeline_Valid() || index_count == 0 || instance_count == 0)
		return false;

	if (instance_count == 1 && first_instance == 0)
		m_state->context.Get()->DrawIndexed(index_count, first_index, base_vertex);
	else
		m_state->context.Get()->DrawIndexedInstanced(index_count, instance_count, first_index, base_vertex, first_instance);
	Record_Draw(m_topology, index_count, instance_count);
	return true;
}

void DX11CommandList::Reset_Frame_State() noexcept
{
	m_pipeline = {};
	m_constant_buffers = {};
	m_draw_constants_bound = {};
	m_shader_resources = {};
	m_topology = RHIPrimitiveTopology::TriangleList;
	m_color_target = {};
	m_depth_target = {};
	m_targets_bound = false;
	m_bindless_resources = {};
}

bool DX11CommandList::Reset_State() noexcept
{
	if (!Is_Ready())
		return false;

	Reset_Frame_State();
	return true;
}

DX11Device::DX11Device(DX11DeviceOptions options)
	: m_state(std::make_unique<DX11DeviceState>())
{
	m_state->shader_directory = options.shader_directory != nullptr ? options.shader_directory : "";
	m_state->vertex_shader_name = options.vertex_shader_name != nullptr ? options.vertex_shader_name : "";
	m_state->fragment_shader_name = options.fragment_shader_name != nullptr ? options.fragment_shader_name : "";
	if (!Create_DX11_Device(*m_state, options))
		return;

	if (m_state->native_swap_chain.Get() != nullptr && !m_state->swap_chain.Create_Targets(options.width, options.height))
		m_state->native_swap_chain.Reset();
	if (m_state->native_swap_chain.Get() != nullptr)
		m_state->gpu_profiler.Initialize(m_state->device.Get(),m_state->context.Get());
}

bool DX11Device::Is_Valid() const noexcept
{
	return m_state != nullptr && m_state->device.Get() != nullptr && m_state->context.Get() != nullptr;
}

RHIDeviceStatus DX11Device::Get_Status() const noexcept
{
	if (!Is_Valid()) return RHIDeviceStatus::Unavailable;
	return FAILED(m_state->device.Get()->GetDeviceRemovedReason())
		? RHIDeviceStatus::Removed : RHIDeviceStatus::Ready;
}

bool DX11Device::Get_Adapter_Info(RHIAdapterInfo& info) const noexcept
{
	info = {};
	if (!Is_Valid()) return false;
	DX11NativeObject<IDXGIDevice> dxgi_device;
	DX11NativeObject<IDXGIAdapter> adapter;
	IDXGIDevice* queried_device = nullptr;
	const HRESULT device_result = m_state->device.Get()->QueryInterface(__uuidof(IDXGIDevice),
		reinterpret_cast<void**>(&queried_device));
	dxgi_device.Reset(queried_device);
	if (FAILED(device_result)) return false;
	IDXGIAdapter* queried_adapter = nullptr;
	const HRESULT adapter_result = dxgi_device.Get()->GetAdapter(&queried_adapter);
	adapter.Reset(queried_adapter);
	if (FAILED(adapter_result)) return false;
	DXGI_ADAPTER_DESC description{};
	if (FAILED(adapter.Get()->GetDesc(&description))) return false;
	info = {description.VendorId, description.DeviceId};
	return true;
}

RHITextureLimits DX11Device::Texture_Limits() const noexcept
{
	return Is_Valid() ? RHITextureLimits{D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION,
		D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION} : RHITextureLimits{};
}

RHIBufferHandle DX11Device::Create_Buffer(const RHIBuffer &description)
{
	return Create_Buffer_Initialized(description, {});
}

RHIBufferHandle DX11Device::Create_Buffer_Initialized(const RHIBuffer &description, std::span<const std::byte> initial_data)
{
    GRAPHICS_PROFILE_SCOPE("Graphics.DX11.CreateBuffer");
	if (!Is_Valid() || description.byte_size == 0)
		return {};
    if (description.update_mode != RHIBufferUpdateMode::Preserve
        && description.update_mode != RHIBufferUpdateMode::Discard) return {};
    const bool discard = description.update_mode == RHIBufferUpdateMode::Discard;
    if (discard && description.usage != RHIBufferUsage::Vertex && description.usage != RHIBufferUsage::Index) return {};
	if (!initial_data.empty() && (initial_data.size() != description.byte_size || initial_data.size() > std::numeric_limits<std::uint32_t>::max()))
		return {};

	const std::uint32_t bind_flags = To_DX11_Bind_Flags(description.usage);
	if (bind_flags == 0)
		return {};
	if (description.usage == RHIBufferUsage::Storage && (description.stride == 0 || description.byte_size % description.stride != 0))
		return {};

	D3D11_BUFFER_DESC native_description{};
	if (description.usage == RHIBufferUsage::Constant && description.byte_size > std::numeric_limits<std::uint32_t>::max() - 15u)
		return {};
	native_description.ByteWidth = description.usage == RHIBufferUsage::Constant
		? (description.byte_size + 15u) & ~15u
		: description.byte_size;
	const auto logical_size = native_description.ByteWidth;
	const bool pooled = DX11BufferCache::Eligible(description.usage, description.byte_size);
	if (pooled)
		native_description.ByteWidth = 1u << DX11BufferCache::Size_Class(description.byte_size);
	native_description.Usage = discard ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    native_description.CPUAccessFlags = discard ? D3D11_CPU_ACCESS_WRITE : 0;
	native_description.BindFlags = bind_flags;
	if (description.usage == RHIBufferUsage::Storage) {
		native_description.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		native_description.StructureByteStride = description.stride;
	}
	DX11Buffer resource;
	resource.usage = description.usage;
	resource.byte_size = logical_size;
	resource.capacity = native_description.ByteWidth;
    resource.update_mode = description.update_mode;
	resource.object = m_state->buffer_cache.Take(description.usage, description.byte_size,description.update_mode);
	ID3D11Buffer *native_buffer = nullptr;
	D3D11_SUBRESOURCE_DATA native_data{};
	native_data.pSysMem = initial_data.data();
	if (resource.object.Get() == nullptr) {
        GRAPHICS_PROFILE_SCOPE("Graphics.DX11.NativeCreateBuffer");
		if (FAILED(m_state->device.Get()->CreateBuffer(&native_description,
            initial_data.empty() || pooled || discard ? nullptr : &native_data, &native_buffer))) return {};
		resource.object.Reset(native_buffer);
	}
	if ((pooled || discard) && !initial_data.empty()) {
        GRAPHICS_PROFILE_SCOPE("Graphics.DX11.InitialBufferUpload");
        // Discard mapping gives pending draws their previous storage while the
        // new handle receives independent contents, even when recycling buffers.
        if (discard) {
            D3D11_MAPPED_SUBRESOURCE mapped{};
            if (FAILED(m_state->context.Get()->Map(resource.object.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&mapped))) return {};
            std::memcpy(mapped.pData,initial_data.data(),initial_data.size());
            m_state->context.Get()->Unmap(resource.object.Get(),0);
        } else {
            const D3D11_BOX box{0,0,0,description.byte_size,1,1};
            m_state->context.Get()->UpdateSubresource(resource.object.Get(),0,&box,initial_data.data(),0,0);
        }
	}
	if (description.usage == RHIBufferUsage::Storage) {
		D3D11_SHADER_RESOURCE_VIEW_DESC view_description{};
		view_description.Format = DXGI_FORMAT_UNKNOWN;
		view_description.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		view_description.BufferEx.FirstElement = 0;
		view_description.BufferEx.NumElements = description.byte_size / description.stride;
		ID3D11ShaderResourceView *native_view = nullptr;
		if (FAILED(m_state->device.Get()->CreateShaderResourceView(native_buffer, &view_description, &native_view)))
			return {};
		resource.shader_resource_view.Reset(native_view);
	}
	return m_state->buffers.Create(std::move(resource));
}

RHITextureHandle DX11Device::Create_Texture(const RHITexture &description)
{
	return Create_Texture_Initialized(description, {});
}

RHITextureHandle DX11Device::Create_Texture_Initialized(const RHITexture &description, const RHITextureUpload &initial_data)
{
    if (!Is_Valid() || description.width == 0 || description.height == 0 || description.depth == 0
        || description.array_size == 0 || description.mip_count == 0 || description.mip_count > 15
        || description.output_layer >= description.array_size) return {};
    const bool volume = description.dimension == RHITextureDimension::Volume;
    const bool cube = description.dimension == RHITextureDimension::Cube;
    if (volume ? description.array_size != 1 : description.depth != 1) return {};
    if (cube && (description.width != description.height || description.array_size != 6)) return {};
    const bool compressed = Is_Block_Compressed(description.format);
    if (compressed && (volume || description.generate_mips
        || Has_Texture_Usage(description, RHITextureUsage::RenderTarget)
        || Has_Texture_Usage(description, RHITextureUsage::UnorderedAccess))) return {};
    const auto native_format = To_DX11_Format(description.format);
    auto bind_flags = To_DX11_Texture_Bind_Flags(description);
    if (native_format == DXGI_FORMAT_UNKNOWN || bind_flags == 0) return {};
    if (description.generate_mips) {
        if (!Has_Texture_Usage(description, RHITextureUsage::ShaderResource)
            || Has_Texture_Usage(description, RHITextureUsage::DepthStencil)) return {};
        bind_flags |= D3D11_BIND_RENDER_TARGET;
    }
    const bool depth = description.format == RHITextureFormat::D32_Float
        || description.format == RHITextureFormat::D24_UNorm_S8 || description.format == RHITextureFormat::D16_UNorm;
    if (depth && (volume || description.generate_mips || !initial_data.data.empty())) return {};
    const bool sampled_depth = depth && Has_Texture_Usage(description, RHITextureUsage::ShaderResource);
    const auto storage_format = sampled_depth ? (description.format == RHITextureFormat::D32_Float
        ? DXGI_FORMAT_R32_TYPELESS : description.format == RHITextureFormat::D16_UNorm
        ? DXGI_FORMAT_R16_TYPELESS : DXGI_FORMAT_R24G8_TYPELESS) : native_format;
    const UINT misc = (cube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0)
        | (description.generate_mips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0);
    DX11Texture resource;
    resource.width = description.width;
    resource.height = description.height;
    resource.format = description.format;
    resource.description = description;
    if (volume) {
        D3D11_TEXTURE3D_DESC native{};
        native.Width = description.width;
        native.Height = description.height;
        native.Depth = description.depth;
        native.MipLevels = description.mip_count;
        native.Format = storage_format;
        native.Usage = D3D11_USAGE_DEFAULT;
        native.BindFlags = bind_flags;
        native.MiscFlags = misc;
        ID3D11Texture3D* texture = nullptr;
        if (FAILED(m_state->device.Get()->CreateTexture3D(&native, nullptr, &texture))) return {};
        resource.volume.Reset(texture);
    } else {
        D3D11_TEXTURE2D_DESC native{};
        native.Width = description.width;
        native.Height = description.height;
        native.MipLevels = description.mip_count;
        native.ArraySize = description.array_size;
        native.Format = storage_format;
        native.SampleDesc.Count = 1;
        native.Usage = D3D11_USAGE_DEFAULT;
        native.BindFlags = bind_flags;
        native.MiscFlags = misc;
        ID3D11Texture2D* texture = nullptr;
        if (FAILED(m_state->device.Get()->CreateTexture2D(&native, nullptr, &texture))) return {};
        resource.object.Reset(texture);
    }
    auto* native_texture = resource.Resource();
    if (Has_Texture_Usage(description, RHITextureUsage::ShaderResource)) {
        D3D11_SHADER_RESOURCE_VIEW_DESC view{};
        view.Format = sampled_depth ? (description.format == RHITextureFormat::D32_Float
            ? DXGI_FORMAT_R32_FLOAT : description.format == RHITextureFormat::D16_UNorm
            ? DXGI_FORMAT_R16_UNORM : DXGI_FORMAT_R24_UNORM_X8_TYPELESS) : native_format;
        if (volume) {
            view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
            view.Texture3D.MipLevels = description.mip_count;
        } else if (cube) {
            view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
            view.TextureCube.MipLevels = description.mip_count;
        } else if (description.array_size > 1) {
            view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            view.Texture2DArray.MipLevels = description.mip_count;
            view.Texture2DArray.ArraySize = description.array_size;
        } else {
            view.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            view.Texture2D.MipLevels = description.mip_count;
        }
        ID3D11ShaderResourceView* native_view = nullptr;
        if (FAILED(m_state->device.Get()->CreateShaderResourceView(native_texture, &view, &native_view))) return {};
        resource.shader_resource_view.Reset(native_view);
    }
    if (Has_Texture_Usage(description, RHITextureUsage::RenderTarget)) {
        D3D11_RENDER_TARGET_VIEW_DESC view{};
        view.Format = native_format;
        if (volume) {
            view.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
            view.Texture3D.WSize = description.depth;
        } else if (description.array_size > 1) {
            view.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            view.Texture2DArray.FirstArraySlice = description.output_layer;
            view.Texture2DArray.ArraySize = 1;
        } else view.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        ID3D11RenderTargetView* native_view = nullptr;
        if (FAILED(m_state->device.Get()->CreateRenderTargetView(native_texture, &view, &native_view))) return {};
        resource.render_target_view.Reset(native_view);
    }
    if (Has_Texture_Usage(description, RHITextureUsage::DepthStencil)) {
        D3D11_DEPTH_STENCIL_VIEW_DESC view{};
        view.Format = native_format;
        if (description.array_size > 1) {
            view.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            view.Texture2DArray.FirstArraySlice = description.output_layer;
            view.Texture2DArray.ArraySize = 1;
        } else view.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        ID3D11DepthStencilView* native_view = nullptr;
        if (FAILED(m_state->device.Get()->CreateDepthStencilView(native_texture, &view, &native_view))) return {};
        resource.depth_stencil_view.Reset(native_view);
    }
    if (Has_Texture_Usage(description, RHITextureUsage::UnorderedAccess)) {
        ID3D11UnorderedAccessView* native_view = nullptr;
        if (FAILED(m_state->device.Get()->CreateUnorderedAccessView(native_texture, nullptr, &native_view))) return {};
        resource.unordered_access_view.Reset(native_view);
    }
    const auto handle = m_state->textures.Create(std::move(resource));
    if (!initial_data.data.empty() && !Update_Texture(handle, initial_data)) {
        m_state->textures.Destroy(handle);
        return {};
    }
    return handle;
}

bool DX11Device::Update_Buffer(RHIBufferHandle buffer, std::uint32_t offset, std::span<const std::byte> data) noexcept
{
    GRAPHICS_PROFILE_SCOPE("Graphics.DX11.UpdateBuffer");
	if (!Is_Valid() || data.empty() || data.size() > std::numeric_limits<std::uint32_t>::max())
		return false;

	DX11Buffer *resource = m_state->buffers.Resolve(buffer);
	if (resource == nullptr || resource->object.Get() == nullptr || offset > resource->byte_size || data.size() > resource->byte_size - offset)
		return false;
    if (resource->update_mode == RHIBufferUpdateMode::Discard) {
        if (offset != 0) return false;
        D3D11_MAPPED_SUBRESOURCE mapped{};
        if (FAILED(m_state->context.Get()->Map(resource->object.Get(),0,D3D11_MAP_WRITE_DISCARD,0,&mapped))) return false;
        std::memcpy(mapped.pData,data.data(),data.size());
        m_state->context.Get()->Unmap(resource->object.Get(),0);
        return true;
    }
	if (resource->usage == RHIBufferUsage::Constant) {
		if (offset != 0 || data.size() != resource->byte_size)
			return false;
		m_state->context.Get()->UpdateSubresource(resource->object.Get(), 0, nullptr, data.data(), 0, 0);
		return true;
	}

	D3D11_BOX destination_box{};
	destination_box.left = offset;
	destination_box.right = offset + static_cast<UINT>(data.size());
	destination_box.top = 0;
	destination_box.bottom = 1;
	destination_box.front = 0;
	destination_box.back = 1;
	m_state->context.Get()->UpdateSubresource(resource->object.Get(), 0, &destination_box, data.data(), 0, 0);
	return true;
}

bool DX11Device::Update_Texture(RHITextureHandle texture, const RHITextureUpload &data) noexcept
{
    if (!Is_Valid()) return false;
    auto* resource = m_state->textures.Resolve(texture);
    TextureTransferLayout layout{};
    if (resource == nullptr || resource->Resource() == nullptr
        || Has_Texture_Usage(resource->description, RHITextureUsage::DepthStencil)
        || !Texture_Transfer_Layout(resource->description, data.mip_level, data.array_layer,
            data.row_pitch, data.slice_pitch, data.data.size(), layout)) return false;
    m_state->context.Get()->UpdateSubresource(resource->Resource(), layout.subresource, nullptr,
        data.data.data(), layout.row_pitch, layout.slice_pitch);
    return true;
}

bool DX11Device::Readback_Texture(RHITextureHandle texture, std::span<std::byte> data, std::uint32_t row_pitch) noexcept
{
    return Readback_Texture_Subresource(texture, {data, row_pitch});
}

bool DX11Device::Map_Texture(RHITextureHandle texture, std::uint32_t mip, std::uint32_t layer,
    bool read_only, RHITextureMapping& output)
{
    output = {};
    if (!Is_Valid()) return false;
    auto* resource = m_state->textures.Resolve(texture);
    TextureTransferLayout layout{};
    if (resource == nullptr || resource->Resource() == nullptr
        || (!read_only && Has_Texture_Usage(resource->description, RHITextureUsage::DepthStencil))
        || !Texture_Transfer_Layout(resource->description, mip, layer, 0, 0, SIZE_MAX, layout)) return false;
    for (const auto& mapping : resource->mappings)
        if (mapping->subresource == layout.subresource) return false;
    DX11NativeObject<ID3D11Resource> staging;
    std::uint32_t staging_mip = 0;
    if (resource->volume.Get() != nullptr) {
        D3D11_TEXTURE3D_DESC native{};
        resource->volume.Get()->GetDesc(&native);
        native.Width = layout.width;
        native.Height = layout.height;
        native.Depth = layout.depth;
        native.MipLevels = 1;
        native.Usage = D3D11_USAGE_STAGING;
        native.BindFlags = native.MiscFlags = 0;
        native.CPUAccessFlags = D3D11_CPU_ACCESS_READ | (read_only ? 0 : D3D11_CPU_ACCESS_WRITE);
        ID3D11Texture3D* object = nullptr;
        if (FAILED(m_state->device.Get()->CreateTexture3D(&native, nullptr, &object))) return false;
        staging.Reset(object);
    } else {
        D3D11_TEXTURE2D_DESC native{};
        resource->object.Get()->GetDesc(&native);
        native.Width = layout.width;
        native.Height = layout.height;
        // Keep the mapped mip's logical dimensions exact for copies in both directions.
        // A BC staging resource needs a block-aligned base level even for a 1x1 mip.
        if (Is_Block_Compressed(resource->format)) {
            while (native.Width < 4 || native.Height < 4) {
                native.Width *= 2; native.Height *= 2; ++staging_mip;
            }
        }
        native.MipLevels = staging_mip + 1;
        native.ArraySize = 1;
        native.Usage = D3D11_USAGE_STAGING;
        native.BindFlags = native.MiscFlags = 0;
        native.CPUAccessFlags = D3D11_CPU_ACCESS_READ | (read_only ? 0 : D3D11_CPU_ACCESS_WRITE);
        ID3D11Texture2D* object = nullptr;
        if (FAILED(m_state->device.Get()->CreateTexture2D(&native, nullptr, &object))) return false;
        staging.Reset(object);
    }
    auto mapping = std::make_unique<DX11TextureMapping>();
    mapping->staging = std::move(staging);
    mapping->context.Reset(Retain(m_state->context.Get()));
    mapping->subresource = layout.subresource;
    mapping->staging_subresource = staging_mip;
    mapping->read_only = read_only;
    m_state->context.Get()->CopySubresourceRegion(mapping->staging.Get(), staging_mip, 0, 0, 0,
        resource->Resource(), layout.subresource, nullptr);
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(m_state->context.Get()->Map(mapping->staging.Get(), staging_mip,
        read_only ? D3D11_MAP_READ : D3D11_MAP_READ_WRITE, 0, &mapped))) return false;
    mapping->mapped = true;
    const auto slice_pitch = layout.depth > 1 ? mapped.DepthPitch : mapped.RowPitch * layout.rows;
    const auto size = static_cast<std::size_t>(slice_pitch) * (layout.depth - 1u)
        + static_cast<std::size_t>(mapped.RowPitch) * (layout.rows - 1u) + layout.row_bytes;
    resource->mappings.push_back(std::move(mapping));
    output = {{static_cast<std::byte*>(mapped.pData), size}, mapped.RowPitch, slice_pitch};
    return true;
}

bool DX11Device::Unmap_Texture(RHITextureHandle texture, std::uint32_t mip, std::uint32_t layer) noexcept
{
    if (!Is_Valid()) return false;
    auto* resource = m_state->textures.Resolve(texture);
    if (resource == nullptr || mip >= resource->description.mip_count || layer >= resource->description.array_size) return false;
    const auto subresource = mip + layer * resource->description.mip_count;
    const auto found = std::find_if(resource->mappings.begin(), resource->mappings.end(),
        [subresource](const auto& mapping) { return mapping->subresource == subresource; });
    if (found == resource->mappings.end()) return false;
    auto& mapping = **found;
    m_state->context.Get()->Unmap(mapping.staging.Get(), mapping.staging_subresource);
    mapping.mapped = false;
    if (!mapping.read_only)
        m_state->context.Get()->CopySubresourceRegion(resource->Resource(), subresource, 0, 0, 0,
            mapping.staging.Get(), mapping.staging_subresource, nullptr);
    resource->mappings.erase(found);
    return true;
}

bool DX11Device::Readback_Texture_Subresource(RHITextureHandle texture, const RHITextureReadback& data) noexcept
{
    if (!Is_Valid()) return false;
    const auto* resource = m_state->textures.Resolve(texture);
    TextureTransferLayout layout{};
    if (resource == nullptr || !Texture_Transfer_Layout(resource->description, data.mip_level,
        data.array_layer, data.row_pitch, data.slice_pitch, data.data.size(), layout)) return false;
    RHITextureMapping mapped;
    try {
        if (!Map_Texture(texture, data.mip_level, data.array_layer, true, mapped)) return false;
    } catch (...) { return false; }
    for (std::uint32_t slice = 0; slice < layout.depth; ++slice)
        for (std::uint32_t row = 0; row < layout.rows; ++row)
            std::memcpy(data.data.data() + static_cast<std::size_t>(slice) * layout.slice_pitch
                + static_cast<std::size_t>(row) * layout.row_pitch,
                mapped.bytes.data() + static_cast<std::size_t>(slice) * mapped.slice_pitch
                + static_cast<std::size_t>(row) * mapped.row_pitch, layout.row_bytes);
    return Unmap_Texture(texture, data.mip_level, data.array_layer);
}

bool DX11Device::Generate_Texture_Mips(RHITextureHandle texture) noexcept
{
    if (!Is_Valid()) return false;
    auto* resource = m_state->textures.Resolve(texture);
    if (resource == nullptr || !resource->description.generate_mips
        || resource->shader_resource_view.Get() == nullptr) return false;
    m_state->context.Get()->GenerateMips(resource->shader_resource_view.Get());
    return true;
}


RHIPipelineHandle DX11Device::Create_Pipeline(const RHIPipeline &description)
{
	if (!Is_Valid())
		return {};

	std::vector<std::byte> vertex_shader;
	std::vector<std::byte> pixel_shader;
	if (!Load_Shader_Binary(m_state->shader_directory, m_state->vertex_shader_name.c_str(), vertex_shader)
		|| !Load_Shader_Binary(m_state->shader_directory, m_state->fragment_shader_name.c_str(), pixel_shader))
		return {};

	return Create_Pipeline(description, {vertex_shader}, {pixel_shader});
}

RHIPipelineHandle DX11Device::Create_Pipeline(const RHIPipeline &description, RHIShaderBytecode vertex_shader, RHIShaderBytecode fragment_shader)
{
	if (!Is_Valid())
		return {};

	DX11Pipeline pipeline;
	if (!Create_DX11_Pipeline(m_state->device.Get(), description, vertex_shader.data, fragment_shader.data, pipeline))
		return {};

	return m_state->pipelines.Create(std::move(pipeline));
}

bool DX11Device::Destroy_Buffer(RHIBufferHandle buffer) noexcept
{
	if (m_state == nullptr) return false;
	auto* resource = m_state->buffers.Resolve(buffer);
	if (resource == nullptr) return false;
	m_state->buffer_cache.Recycle(*resource);
	return m_state->buffers.Destroy(buffer);
}

bool DX11Device::Retain_Texture(RHITextureHandle texture) noexcept
{
    if (!Is_Valid()) return false;
    auto* resource = m_state->textures.Resolve(texture);
    if (resource == nullptr || resource->references == UINT32_MAX) return false;
    ++resource->references;
    return true;
}

bool DX11Device::Destroy_Texture(RHITextureHandle texture) noexcept
{
    if (m_state == nullptr) return false;
    auto* resource = m_state->textures.Resolve(texture);
    if (resource == nullptr) return false;
    if (resource->references > 1) { --resource->references; return true; }
    return m_state->textures.Destroy(texture);
}

bool DX11Device::Destroy_Pipeline(RHIPipelineHandle pipeline) noexcept
{
	return m_state != nullptr && m_state->pipelines.Destroy(pipeline);
}

CommandList &DX11Device::Immediate_Command_List() noexcept
{
	return m_state->command_list;
}

SwapChain &DX11Device::Get_Swap_Chain() noexcept
{
	return m_state->swap_chain;
}

bool DX11Device::Set_Exclusive_Fullscreen(bool fullscreen) noexcept
{
    return Is_Valid() && m_state->native_swap_chain.Get() != nullptr && !m_state->frame_active
        && SUCCEEDED(m_state->native_swap_chain.Get()->SetFullscreenState(fullscreen ? TRUE : FALSE, nullptr));
}

bool DX11Device::Begin_Frame() noexcept
{
	if (!Is_Valid() || m_state->frame_active || !m_state->swap_chain.Is_Valid())
		return false;

	m_state->command_list.Reset_Frame_State();
	const RHIBackbuffer backbuffer = m_state->swap_chain.Backbuffer();
	const RHIDepthTarget depth_target = m_state->swap_chain.Depth_Target();
	if (!m_state->command_list.Set_Render_Targets(backbuffer.texture, depth_target.texture))
		return false;

	m_state->frame_active = true;
	m_state->gpu_profiler.Begin_Frame();
	m_state->ready_to_present = false;
	m_state->presented = false;
	return true;
}

bool DX11Device::End_Frame() noexcept
{
	if (!Is_Valid() || !m_state->frame_active)
		return false;

	m_state->context.Get()->OMSetRenderTargets(0, nullptr, nullptr);
	m_state->command_list.Invalidate_Render_Target_Binding();
	m_state->gpu_profiler.End_Frame();
	m_state->frame_active = false;
	m_state->ready_to_present = true;
	m_state->presented = false;
	return true;
}

}
