module;

#define NOMINMAX

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <fstream>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <windows.h>
#include <wrl/client.h>
#include "../../profiling/Tracy.h"

export module Graphics.Backends.DX12;

export import Graphics.RHI;

import Graphics.Resources.Pools.ResourcePool;

namespace Graphics
{

template <typename Interface>
class DX12NativeObject final
{
public:
	DX12NativeObject() noexcept = default;
	~DX12NativeObject() noexcept { Reset(); }
	DX12NativeObject(const DX12NativeObject &) = delete;
	DX12NativeObject &operator=(const DX12NativeObject &) = delete;
	DX12NativeObject(DX12NativeObject &&other) noexcept : m_object(other.m_object)
	{
		other.m_object = nullptr;
	}
	DX12NativeObject &operator=(DX12NativeObject &&other) noexcept
	{
		if (this != &other) {
			Reset();
			m_object = other.m_object;
			other.m_object = nullptr;
		}
		return *this;
	}
	void Reset(Interface *object = nullptr) noexcept
	{
		if (m_object != nullptr)
			m_object->Release();
		m_object = object;
	}
	Interface *Get() const noexcept { return m_object; }
	Interface **Put() noexcept
	{
		Reset();
		return &m_object;
	}
	Interface *Detach() noexcept
	{
		Interface *object = m_object;
		m_object = nullptr;
		return object;
	}

private:
	Interface *m_object = nullptr;
};

class DX12DeferredReference final
{
public:
	DX12DeferredReference() noexcept = default;
	explicit DX12DeferredReference(IUnknown *object) noexcept : m_object(object)
	{
		if (m_object != nullptr)
			m_object->AddRef();
	}
	~DX12DeferredReference() noexcept
	{
		if (m_object != nullptr)
			m_object->Release();
	}
	DX12DeferredReference(const DX12DeferredReference &) = delete;
	DX12DeferredReference &operator=(const DX12DeferredReference &) = delete;
	DX12DeferredReference(DX12DeferredReference &&other) noexcept : m_object(other.m_object)
	{
		other.m_object = nullptr;
	}
	DX12DeferredReference &operator=(DX12DeferredReference &&other) noexcept
	{
		if (this != &other) {
			if (m_object != nullptr)
				m_object->Release();
			m_object = other.m_object;
			other.m_object = nullptr;
		}
		return *this;
	}

private:
	IUnknown *m_object = nullptr;
};

static_assert(std::is_nothrow_move_constructible_v<DX12DeferredReference>);
static_assert(std::is_nothrow_move_assignable_v<DX12DeferredReference>);

struct DX12DescriptorRange final
{
	std::uint32_t index = std::numeric_limits<std::uint32_t>::max();
	std::uint32_t count = 0;
	bool Is_Valid() const noexcept
	{
		return index != std::numeric_limits<std::uint32_t>::max() && count != 0;
	}
};

class DX12DescriptorHeap final
{
public:
	bool Create(ID3D12Device *device, D3D12_DESCRIPTOR_HEAP_TYPE type,
		std::uint32_t capacity, bool shader_visible) noexcept
	{
		if (device == nullptr || capacity == 0)
			return false;
		D3D12_DESCRIPTOR_HEAP_DESC description{};
		description.Type = type;
		description.NumDescriptors = capacity;
		description.Flags = shader_visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
			: D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (FAILED(device->CreateDescriptorHeap(&description, IID_PPV_ARGS(m_heap.Put()))))
			return false;
		m_type = type;
		m_capacity = capacity;
		m_increment = device->GetDescriptorHandleIncrementSize(type);
		return true;
	}
	D3D12_CPU_DESCRIPTOR_HANDLE Cpu(std::uint32_t index) const noexcept
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heap.Get()
			? m_heap.Get()->GetCPUDescriptorHandleForHeapStart() : D3D12_CPU_DESCRIPTOR_HANDLE{};
		handle.ptr += static_cast<SIZE_T>(index) * m_increment;
		return handle;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE Gpu(std::uint32_t index) const noexcept
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = m_heap.Get()
			? m_heap.Get()->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{};
		handle.ptr += static_cast<UINT64>(index) * m_increment;
		return handle;
	}
	ID3D12DescriptorHeap *Get() const noexcept { return m_heap.Get(); }
	std::uint32_t Capacity() const noexcept { return m_capacity; }
	std::uint32_t Increment() const noexcept { return m_increment; }

private:
	DX12NativeObject<ID3D12DescriptorHeap> m_heap;
	D3D12_DESCRIPTOR_HEAP_TYPE m_type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	std::uint32_t m_capacity = 0;
	std::uint32_t m_increment = 0;
};

struct DX12MappedBufferPage final
{
	DX12NativeObject<ID3D12Resource> resource;
	std::byte *cpu = nullptr;
	std::uint64_t gpu = 0;
	std::uint64_t retirement_fence = 0;
	std::uint32_t capacity = 0;
	std::uint32_t used = 0;
	~DX12MappedBufferPage() noexcept
	{
		if (cpu != nullptr)
			resource.Get()->Unmap(0, nullptr);
	}
};

struct DX12MappedBufferSlice final
{
	std::shared_ptr<DX12MappedBufferPage> page;
	std::uint32_t offset = 0;
	std::uint64_t GPU_Address() const noexcept { return page ? page->gpu + offset : 0; }
	void Retire(std::uint64_t fence) noexcept
	{
		if (page)
			page->retirement_fence = (std::max)(page->retirement_fence, fence);
	}
};

// Live buffer versions pin their page independently of frame rotation. Once all
// versions are retired, the GPU fence protects reuse of their recorded addresses.
class DX12MappedBufferPool final
{
public:
	void Initialize(ID3D12Device *device) noexcept
	{
		D3D12_FEATURE_DATA_D3D12_OPTIONS16 options{};
		if (SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16,
			&options, sizeof(options))) && options.GPUUploadHeapSupported)
			m_heap_type = D3D12_HEAP_TYPE_GPU_UPLOAD;
	}
	DX12MappedBufferSlice Allocate(ID3D12Device *device, std::uint32_t size,
		std::uint64_t completed) noexcept
	{
		assert(device != nullptr && size != 0 && size % 256u == 0);
		try {
			if (m_current < m_pages.size()) {
				auto &page = m_pages[m_current];
				if (size <= page->capacity - page->used) {
					const std::uint32_t offset = page->used;
					page->used += size;
					return {page, offset};
				}
			}
			for (std::size_t index = 0; index < m_pages.size(); ++index) {
				auto &page = m_pages[index];
				if (page.use_count() == 1 && page->retirement_fence <= completed
					&& page->capacity >= size) {
					page->used = size;
					m_current = index;
					return {page, 0};
				}
			}
			auto page = std::make_shared<DX12MappedBufferPage>();
			page->capacity = (std::max)(size, 1024u * 1024u);
			D3D12_HEAP_PROPERTIES heap{};
			heap.Type = m_heap_type;
			D3D12_RESOURCE_DESC description{};
			description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			description.Width = page->capacity;
			description.Height = 1;
			description.DepthOrArraySize = 1;
			description.MipLevels = 1;
			description.SampleDesc.Count = 1;
			description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			if (FAILED(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
				&description, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
				IID_PPV_ARGS(page->resource.Put()))))
				return {};
			const D3D12_RANGE no_reads{0, 0};
			if (FAILED(page->resource.Get()->Map(0, &no_reads,
				reinterpret_cast<void **>(&page->cpu))))
				return {};
			page->gpu = page->resource.Get()->GetGPUVirtualAddress();
			page->used = size;
			m_pages.push_back(page);
			m_current = m_pages.size() - 1;
			return {std::move(page), 0};
		} catch (...) {
			return {};
		}
	}
private:
	std::vector<std::shared_ptr<DX12MappedBufferPage>> m_pages;
	std::size_t m_current = 0;
	D3D12_HEAP_TYPE m_heap_type = D3D12_HEAP_TYPE_UPLOAD;
};

struct DX12Buffer final
{
	DX12NativeObject<ID3D12Resource> object;
	DX12MappedBufferSlice constants;
	std::vector<std::byte> constant_data;
	DX12DescriptorRange shader_resource_view;
	RHIBufferUsage usage = RHIBufferUsage::Vertex;
	RHIBufferUpdateMode update_mode = RHIBufferUpdateMode::Preserve;
	std::uint32_t byte_size = 0;
	std::uint32_t capacity = 0;
	std::uint32_t stride = 0;
	D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
};

struct DX12TextureMapping final
{
	DX12NativeObject<ID3D12Resource> readback;
	DX12NativeObject<ID3D12Resource> upload;
	std::uint32_t subresource = 0;
	std::uint32_t depth = 1;
	std::uint32_t rows = 1;
	std::uint32_t row_bytes = 0;
	std::uint32_t row_pitch = 0;
	std::uint32_t slice_pitch = 0;
	std::uint64_t footprint_offset = 0;
	D3D12_RESOURCE_STATES return_state = D3D12_RESOURCE_STATE_COMMON;
	bool read_only = false;
	bool readback_mapped = false;
	bool upload_mapped = false;
	~DX12TextureMapping() noexcept
	{
		if (readback_mapped && readback.Get() != nullptr)
			readback.Get()->Unmap(0, nullptr);
		if (upload_mapped && upload.Get() != nullptr)
			upload.Get()->Unmap(0, nullptr);
	}
};

struct DX12Texture final
{
	DX12NativeObject<ID3D12Resource> object;
	RHITexture description{};
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	RHITextureFormat format = RHITextureFormat::RGBA8_UNorm;
	std::uint32_t references = 1;
	DX12DescriptorRange shader_resource_view;
	DX12DescriptorRange render_target_view;
	DX12DescriptorRange depth_stencil_view;
	DX12DescriptorRange unordered_access_view;
	std::vector<DX12DescriptorRange> mip_shader_resource_views;
	std::vector<DX12DescriptorRange> mip_unordered_access_views;
	DX12DescriptorRange mip_render_targets;
	bool compute_mips = false;
	std::vector<D3D12_RESOURCE_STATES> states;
	std::vector<std::unique_ptr<DX12TextureMapping>> mappings;
};

struct DX12SamplerSlot final
{
	D3D12_SAMPLER_DESC description{};
	std::uint32_t references = 0;
	std::uint64_t retirement_fence = 0;
};

struct DX12Pipeline final
{
	std::uint64_t key = 0;
	std::array<DX12NativeObject<ID3D12PipelineState>, 24> pipeline_states;
	std::array<std::uint32_t, 16> sampler_indices{};
	std::uint8_t stencil_reference = 0;
	RHIPrimitiveTopology topology = RHIPrimitiveTopology::TriangleList;
	bool scissor_test = false;
	std::array<std::byte, 1> reserved{};
};

static_assert(std::is_nothrow_move_constructible_v<DX12Buffer>);
static_assert(std::is_nothrow_move_assignable_v<DX12Buffer>);
static_assert(std::is_nothrow_move_constructible_v<DX12Texture>);
static_assert(std::is_nothrow_move_assignable_v<DX12Texture>);
static_assert(std::is_nothrow_move_constructible_v<DX12Pipeline>);
static_assert(std::is_nothrow_move_assignable_v<DX12Pipeline>);

struct DX12UploadSlice final
{
	ID3D12Resource *resource = nullptr;
	std::byte *cpu = nullptr;
	std::uint64_t offset = 0;
	std::uint64_t gpu_address = 0;
	std::uint64_t size = 0;
};

struct DX12UploadArena final
{
	DX12NativeObject<ID3D12Resource> resource;
	std::byte *mapped = nullptr;
	std::uint64_t capacity = 0;
	std::uint64_t offset = 0;

	void Reset() noexcept { offset = 0; }
	DX12UploadSlice Allocate(std::uint64_t size, std::uint64_t alignment) noexcept
	{
		if (resource.Get() == nullptr || mapped == nullptr || size == 0)
			return {};
		const std::uint64_t aligned = (offset + alignment - 1u) & ~(alignment - 1u);
		if (aligned > capacity || size > capacity - aligned)
			return {};
		offset = aligned + size;
		return {resource.Get(), mapped + aligned, aligned,
			resource.Get()->GetGPUVirtualAddress() + aligned, size};
	}
};

struct DX12FrameContext final
{
	DX12NativeObject<ID3D12CommandAllocator> allocator;
	DX12UploadArena uploads;
	std::vector<DX12NativeObject<ID3D12Resource>> transient_uploads;
	std::uint64_t fence = 0;
	std::uint32_t descriptor_base = 0;
	std::uint32_t descriptor_offset = 0;
};

struct DX12DeferredResources final
{
	std::uint64_t fence = 0;
	std::vector<DX12DeferredReference> objects;
};

class DX12BufferCache final
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
		return std::bit_width(size > 256 ? size - 1u : 255u);
	}
	struct Entry final
	{
		DX12NativeObject<ID3D12Resource> object;
		std::uint32_t capacity = 0;
		RHIBufferUsage usage = RHIBufferUsage::Vertex;
		RHIBufferUpdateMode update_mode = RHIBufferUpdateMode::Preserve;
		std::uint64_t fence = 0;
	};
	DX12NativeObject<ID3D12Resource> Take(RHIBufferUsage usage, std::uint32_t capacity,
		RHIBufferUpdateMode update_mode, std::uint64_t completed) noexcept
	{
		if (!Eligible(usage, capacity))
			return {};
		const unsigned first_class = Size_Class(capacity);
		for (unsigned size_class = first_class; size_class < m_free[0].size(); ++size_class) {
			auto &bucket = m_free[(update_mode == RHIBufferUpdateMode::Discard ? 1u : 0u)
			* 2u + (usage == RHIBufferUsage::Index ? 1u : 0u)][size_class];
			for (auto it = bucket.begin(); it != bucket.end(); ++it) {
				if (it->fence <= completed && it->capacity >= capacity) {
					auto object = std::move(it->object);
					m_idle_bytes -= it->capacity;
					bucket.erase(it);
					return object;
				}
			}
		}
		return {};
	}
	void Recycle(DX12Buffer &buffer, std::uint64_t fence) noexcept
	{
		if (!Eligible(buffer.usage, buffer.capacity)
			|| buffer.capacity > MaxIdleBytes - m_idle_bytes || buffer.object.Get() == nullptr)
			return;
		try {
			m_free[ (buffer.update_mode == RHIBufferUpdateMode::Discard ? 1u : 0u) * 2u
				+ (buffer.usage == RHIBufferUsage::Index ? 1u : 0u)][Size_Class(buffer.capacity)]
				.push_back({std::move(buffer.object), buffer.capacity, buffer.usage,
				buffer.update_mode, fence});
			m_idle_bytes += buffer.capacity;
		} catch (...) {
		}
	}

private:
	std::array<std::array<std::vector<Entry>, 27>, 4> m_free;
	std::uint32_t m_idle_bytes = 0;
};

struct DX12DeviceState;

struct DX12DebugCallbackRegistration final
{
	DX12DeviceState *state = nullptr;
	explicit DX12DebugCallbackRegistration(DX12DeviceState *owner) noexcept : state(owner) {}
	~DX12DebugCallbackRegistration() noexcept;
};

static constexpr std::uint32_t SwapChainBufferCount = 2;

class DX12SwapChain final : public SwapChain
{
public:
	explicit DX12SwapChain(DX12DeviceState *state) noexcept : m_state(state) {}
	~DX12SwapChain() noexcept override;
	bool Is_Valid() const noexcept override;
	RHIBackbuffer Backbuffer() const noexcept override;
	RHIDepthTarget Depth_Target() const noexcept override;
	bool Resize(std::uint32_t width, std::uint32_t height) override;
	bool Present() noexcept override;
	bool Create_Targets(std::uint32_t width, std::uint32_t height);

private:
	void Release_Targets() noexcept;
	std::uint32_t Current_Buffer_Index() const noexcept;

	DX12DeviceState *m_state = nullptr;
	std::array<RHITextureHandle, SwapChainBufferCount> m_backbuffers{};
	std::array<RHITextureHandle, SwapChainBufferCount> m_depth_targets{};
	std::uint32_t m_width = 0;
	std::uint32_t m_height = 0;
};

class DX12CommandList final : public CommandList
{
public:
	explicit DX12CommandList(DX12DeviceState *state) noexcept : m_state(state) {}
	bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept override;
	bool Set_Bindless_Resources(std::span<const RHIBindlessResource> resources) noexcept override;
	bool Set_Render_Targets(RHITextureHandle color_target, RHITextureHandle depth_target) noexcept override;
	bool Set_Color_Target(RHITextureHandle color_target) noexcept override;
	bool Set_Depth_Target(RHITextureHandle depth_target) noexcept override;
	bool Clear(const std::array<float, 4> &color, float depth) noexcept override;
	bool Clear_Depth(float depth) noexcept override;
	bool Clear_Color_Target(RHITextureHandle texture, const std::array<float, 4> &color) noexcept override;
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
	void On_New_Command_List() noexcept;
	void Mark_Graphics_State_Dirty() noexcept { m_graphics_state_dirty = true; }
	void Release_Pipeline_Binding(RHIPipelineHandle pipeline) noexcept
	{
		if (m_pipeline == pipeline) {
			m_pipeline = {};
			m_graphics_state_dirty = true;
		}
	}
	void Invalidate_Constants() noexcept
	{
		m_constants_dirty = true;
		m_bindless_page = std::numeric_limits<std::uint32_t>::max();
	}
	void Release_Buffer_Bindings(RHIBufferHandle buffer) noexcept
	{
		if (std::erase_if(m_bindless_cache, [buffer](const RHIBindlessResource &entry) {
			return (entry.type == RHIResourceType::Buffer || entry.type == RHIResourceType::Material)
				&& entry.buffer == buffer;
		}) != 0)
			Invalidate_Constants();
	}
	void Release_Texture_Bindings(RHITextureHandle texture) noexcept
	{
		if (std::erase_if(m_bindless_cache, [texture](const RHIBindlessResource &entry) {
			return entry.type == RHIResourceType::Texture && entry.texture == texture;
		}) != 0)
			Invalidate_Constants();
	}

private:
	bool Is_Ready() const noexcept;
	bool Is_Pipeline_Valid() const noexcept;
	bool Bindless_Resources_Internal(std::span<const RHIBindlessResource> resources, bool cache) noexcept;
	bool Select_Pipeline_State() noexcept;
	bool Apply_Scissor() noexcept;
	bool Rebind_Targets() noexcept;
	void Rebind_Input_Assembly() noexcept;

	DX12DeviceState *m_state = nullptr;
	RHIPipelineHandle m_pipeline{};
	RHIPrimitiveTopology m_topology = RHIPrimitiveTopology::TriangleList;
	RHITextureHandle m_color_target{};
	RHITextureHandle m_depth_target{};
	std::vector<RHIBindlessResource> m_bindless_cache;
	std::uint64_t m_bindless_hash = 0;
	std::array<std::uint32_t, 256> m_resource_indices{};
	std::array<std::uint64_t, 2> m_resource_index_addresses{};
	std::array<std::uint64_t, 128> m_constant_addresses{};
	std::uint32_t m_bindless_page = std::numeric_limits<std::uint32_t>::max();
	std::array<std::byte, 256> m_draw_constant_data{};
	std::uint32_t m_draw_constant_size = 0;
	std::uint64_t m_draw_constant_gpu_address = 0;
	struct VertexBinding final { RHIBufferHandle buffer{}; std::uint32_t stride = 0; std::uint32_t offset = 0; };
	std::array<VertexBinding, 16> m_vertex_bindings{};
	RHIBufferHandle m_index_buffer{};
	RHIIndexFormat m_index_format = RHIIndexFormat::UInt16;
	std::uint32_t m_index_offset = 0;
	RHIViewport m_viewport{};
	RHIScissorRect m_scissor{};
	bool m_has_viewport = false;
	bool m_has_scissor = false;
	bool m_graphics_state_dirty = true;
	bool m_constants_dirty = false;
};

struct DX12DeviceState final
{
	DX12NativeObject<IDXGIFactory6> factory;
	DX12NativeObject<IDXGIAdapter1> adapter;
	DX12NativeObject<ID3D12Device> device;
	DX12NativeObject<ID3D12InfoQueue1> debug_info_queue;
	DWORD debug_message_callback_cookie = 0;
	bool debug_message_callback_registered = false;
	DX12DebugCallbackRegistration debug_callback_registration;
	DX12NativeObject<ID3D12CommandQueue> queue;
	DX12NativeObject<ID3D12GraphicsCommandList> command_list;
	DX12NativeObject<ID3D12Fence> fence;
	DX12NativeObject<IDXGISwapChain3> native_swap_chain;
	DX12NativeObject<ID3D12RootSignature> graphics_root_signature;
	DX12NativeObject<ID3D12RootSignature> mip_root_signature;
	std::array<DX12NativeObject<ID3D12PipelineState>, 64> mip_pipeline_states;
	std::array<DX12NativeObject<ID3D12PipelineState>, 64> raster_mip_pipeline_states;
	DX12NativeObject<ID3D12Resource> null_constant_buffer;
	DX12DescriptorHeap cpu_resources;
	DX12DescriptorHeap cpu_render_targets;
	DX12DescriptorHeap cpu_depth_targets;
	DX12DescriptorHeap gpu_resources;
	DX12DescriptorHeap gpu_samplers;
	std::vector<DX12SamplerSlot> sampler_slots;
	std::array<DX12FrameContext, 3> frames;
	DX12BufferCache buffer_cache;
	DX12MappedBufferPool constant_memory;
	ResourcePool<DX12Buffer, RHIBufferHandle> buffers;
	ResourcePool<DX12Texture, RHITextureHandle> textures;
	ResourcePool<DX12Pipeline, RHIPipelineHandle> pipelines;
	DX12SwapChain swap_chain;
	DX12CommandList command_list_facade;
	std::vector<DX12DeferredResources> deferred;
	std::string shader_directory;
	std::string vertex_shader_name;
	std::string fragment_shader_name;
	std::uint32_t resource_descriptor_offset = 0;
	std::uint32_t render_descriptor_offset = 0;
	std::uint32_t depth_descriptor_offset = 0;
	std::uint32_t null_srv_base = 0;
	std::uint32_t null_cbv_base = 0;
	std::uint64_t next_fence = 0;
	std::uint64_t completed_fence = 0;
	std::uint64_t last_submitted_fence = 0;
	HANDLE fence_event = nullptr;
	std::uint32_t current_frame = 2;
	bool recording = false;
	bool frame_active = false;
	bool ready_to_present = false;
	bool presented = false;
	bool removed = false;
	bool debug_layer = false;
	bool allow_tearing = false;
	DX12DeviceState() noexcept
		: debug_callback_registration(this), swap_chain(this), command_list_facade(this) {}
	~DX12DeviceState() noexcept;
	void Unregister_Debug_Messages() noexcept;

	bool Ensure_Recording() noexcept;
	std::uint64_t Submit_Current(bool wait) noexcept;
	std::uint64_t Retirement_Fence() const noexcept
	{
		// The next submission covers every use recorded in the open list.
		return recording ? next_fence + 1 : last_submitted_fence;
	}
	bool Wait_For_Fence(std::uint64_t value) noexcept;
	void Collect_Deferred() noexcept;
	bool Defer(IUnknown *object, std::uint64_t fence_value) noexcept;
	bool Defer_Buffer(DX12Buffer &resource, std::uint64_t fence_value) noexcept;
	std::uint32_t Acquire_Sampler(const D3D12_SAMPLER_DESC &description);
	void Release_Samplers(std::span<const std::uint32_t> indices, std::uint64_t fence_value) noexcept;
	DX12UploadSlice Allocate_Upload(std::uint64_t size, std::uint64_t alignment) noexcept;
	DX12UploadSlice Allocate_Transient_Upload(std::uint64_t size, std::uint64_t alignment) noexcept;
	bool Transition(DX12Texture &texture, D3D12_RESOURCE_STATES desired, std::uint32_t subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) noexcept;
	D3D12_CPU_DESCRIPTOR_HANDLE CpuResource(std::uint32_t index) const noexcept { return cpu_resources.Cpu(index); }
	D3D12_CPU_DESCRIPTOR_HANDLE CpuRTV(std::uint32_t index) const noexcept { return cpu_render_targets.Cpu(index); }
	D3D12_CPU_DESCRIPTOR_HANDLE CpuDSV(std::uint32_t index) const noexcept { return cpu_depth_targets.Cpu(index); }
};

static constexpr std::uint32_t InvalidDescriptor = std::numeric_limits<std::uint32_t>::max();
static constexpr std::uint32_t FrameCount = 3;
static constexpr std::uint32_t FrameResourceDescriptors = 32768;
static constexpr std::uint32_t PersistentSamplerCount = 2048;
static constexpr std::uint32_t BindlessSRVCount = 128;
static constexpr std::uint32_t BindlessCBVCount = 128;
static constexpr std::uint32_t RootCBVCount = 8;
static constexpr std::uint32_t PersistentDescriptorBase = FrameCount * FrameResourceDescriptors;
static constexpr std::uint32_t PersistentDescriptorCount = 262144;

static std::uint32_t Constant_Root_Parameter(std::uint32_t slot) noexcept
{
	assert(slot < RootCBVCount);
	return slot == 0 ? 5 : slot == 1 ? 4 : slot + 4;
}

static void Publish_Shader_Resource(DX12DeviceState &state, DX12DescriptorRange view) noexcept
{
	assert(view.Is_Valid());
	state.device.Get()->CopyDescriptorsSimple(view.count,
		state.gpu_resources.Cpu(PersistentDescriptorBase + view.index),
		state.CpuResource(view.index), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

template <typename Interface>
static Interface *Retain(Interface *object) noexcept
{
	if (object != nullptr)
		object->AddRef();
	return object;
}

static void Report_HResult(const char *operation, HRESULT result) noexcept
{
	if (SUCCEEDED(result))
		return;
	char buffer[128]{};
	std::snprintf(buffer, sizeof(buffer), "Graphics.DX12: %s failed (0x%08lx)\n", operation,
		static_cast<unsigned long>(result));
	OutputDebugStringA(buffer);
}

static void __stdcall DX12_Debug_Message_Callback(D3D12_MESSAGE_CATEGORY category,
	D3D12_MESSAGE_SEVERITY severity, D3D12_MESSAGE_ID id, LPCSTR description, void *context)
{
	(void)context;
	std::fprintf(stderr, "Graphics.DX12 debug [%u/%u/%u]: %s\n",
		static_cast<unsigned>(category), static_cast<unsigned>(severity), static_cast<unsigned>(id),
		description != nullptr ? description : "(no description)");
	std::fflush(stderr);
}

void DX12DeviceState::Unregister_Debug_Messages() noexcept
{
	if (!debug_message_callback_registered)
		return;
	if (debug_info_queue.Get() != nullptr)
		debug_info_queue.Get()->UnregisterMessageCallback(debug_message_callback_cookie);
	debug_message_callback_cookie = 0;
	debug_message_callback_registered = false;
	debug_info_queue.Reset();
}

DX12DebugCallbackRegistration::~DX12DebugCallbackRegistration() noexcept
{
	if (state != nullptr)
		state->Unregister_Debug_Messages();
}

static void Register_DX12_Debug_Messages(DX12DeviceState &state) noexcept
{
	if (!state.debug_layer || state.device.Get() == nullptr)
		return;
	if (FAILED(state.device.Get()->QueryInterface(IID_PPV_ARGS(state.debug_info_queue.Put()))))
		return;
	const HRESULT result = state.debug_info_queue.Get()->RegisterMessageCallback(
		DX12_Debug_Message_Callback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, &state,
		&state.debug_message_callback_cookie);
	if (FAILED(result)) {
		state.debug_info_queue.Reset();
		Report_HResult("ID3D12InfoQueue1::RegisterMessageCallback", result);
		return;
	}
	state.debug_message_callback_registered = true;
}

static bool Has_Texture_Usage(const RHITexture &description, RHITextureUsage usage) noexcept
{
	return (description.usage & static_cast<std::uint32_t>(usage)) != 0;
}

static DXGI_FORMAT To_DX12_Format(RHITextureFormat format) noexcept
{
	switch (format) {
	case RHITextureFormat::R8_UNorm: return DXGI_FORMAT_R8_UNORM;
	case RHITextureFormat::RG8_UNorm: return DXGI_FORMAT_R8G8_UNORM;
	case RHITextureFormat::RGBA8_UNorm: return DXGI_FORMAT_R8G8B8A8_UNORM;
	case RHITextureFormat::BGRA8_UNorm: return DXGI_FORMAT_B8G8R8A8_UNORM;
	case RHITextureFormat::RGBA16_Float: return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case RHITextureFormat::RGBA32_Float: return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case RHITextureFormat::R32_Float: return DXGI_FORMAT_R32_FLOAT;
	case RHITextureFormat::D24_UNorm_S8: return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case RHITextureFormat::D32_Float: return DXGI_FORMAT_D32_FLOAT;
	case RHITextureFormat::BGR565_UNorm: return DXGI_FORMAT_B5G6R5_UNORM;
	case RHITextureFormat::BGRA4444_UNorm: return DXGI_FORMAT_B4G4R4A4_UNORM;
	case RHITextureFormat::BGRX8_UNorm: return DXGI_FORMAT_B8G8R8X8_UNORM;
	case RHITextureFormat::BGRA5551_UNorm: return DXGI_FORMAT_B5G5R5A1_UNORM;
	case RHITextureFormat::A8_UNorm: return DXGI_FORMAT_A8_UNORM;
	case RHITextureFormat::RG8_SNorm: return DXGI_FORMAT_R8G8_SNORM;
	case RHITextureFormat::BC1_UNorm: return DXGI_FORMAT_BC1_UNORM;
	case RHITextureFormat::BC2_UNorm: return DXGI_FORMAT_BC2_UNORM;
	case RHITextureFormat::BC3_UNorm: return DXGI_FORMAT_BC3_UNORM;
	case RHITextureFormat::D16_UNorm: return DXGI_FORMAT_D16_UNORM;
	default: return DXGI_FORMAT_UNKNOWN;
	}
}

static bool Is_Block_Compressed(RHITextureFormat format) noexcept
{
	return format == RHITextureFormat::BC1_UNorm || format == RHITextureFormat::BC2_UNorm
		|| format == RHITextureFormat::BC3_UNorm;
}

static std::uint32_t Texture_Bytes_Per_Pixel(RHITextureFormat format) noexcept
{
	switch (format) {
	case RHITextureFormat::R8_UNorm:
	case RHITextureFormat::A8_UNorm: return 1;
	case RHITextureFormat::RG8_UNorm:
	case RHITextureFormat::RG8_SNorm:
	case RHITextureFormat::BGR565_UNorm:
	case RHITextureFormat::BGRA4444_UNorm:
	case RHITextureFormat::BGRA5551_UNorm:
	case RHITextureFormat::D16_UNorm: return 2;
	case RHITextureFormat::RGBA8_UNorm:
	case RHITextureFormat::BGRA8_UNorm:
	case RHITextureFormat::BGRX8_UNorm:
	case RHITextureFormat::R32_Float:
	case RHITextureFormat::D24_UNorm_S8:
	case RHITextureFormat::D32_Float: return 4;
	case RHITextureFormat::RGBA16_Float: return 8;
	case RHITextureFormat::RGBA32_Float: return 16;
	default: return 0;
	}
}

struct TextureTransferLayout final
{
	std::uint32_t width = 0;
	std::uint32_t height = 0;
	std::uint32_t depth = 0;
	std::uint32_t rows = 0;
	std::uint32_t row_bytes = 0;
	std::uint32_t row_pitch = 0;
	std::uint32_t slice_pitch = 0;
	std::uint32_t subresource = 0;
};

static bool Texture_Transfer_Layout(const RHITexture &description, std::uint32_t mip,
	std::uint32_t layer, std::uint32_t row_pitch, std::uint32_t slice_pitch,
	std::size_t capacity, TextureTransferLayout &output) noexcept
{
	if (mip >= description.mip_count || mip >= 32 || layer >= description.array_size)
		return false;
	const std::uint32_t width = (std::max)(1u, description.width >> mip);
	const std::uint32_t height = (std::max)(1u, description.height >> mip);
	const std::uint32_t depth = (std::max)(1u, description.depth >> mip);
	const bool compressed = Is_Block_Compressed(description.format);
	const std::uint32_t rows = compressed ? (height + 3u) / 4u : height;
	const std::uint64_t row_bytes = compressed
		? static_cast<std::uint64_t>((width + 3u) / 4u)
			* (description.format == RHITextureFormat::BC1_UNorm ? 8u : 16u)
		: static_cast<std::uint64_t>(width) * Texture_Bytes_Per_Pixel(description.format);
	if (row_bytes == 0 || row_bytes > UINT32_MAX)
		return false;
	if (row_pitch == 0)
		row_pitch = static_cast<std::uint32_t>(row_bytes);
	const std::uint64_t minimum_slice = static_cast<std::uint64_t>(row_pitch) * rows;
	if (row_pitch < row_bytes || minimum_slice > UINT32_MAX)
		return false;
	if (slice_pitch == 0)
		slice_pitch = static_cast<std::uint32_t>(minimum_slice);
	const auto required = static_cast<std::uint64_t>(slice_pitch) * (depth - 1u)
		+ static_cast<std::uint64_t>(row_pitch) * (rows - 1u) + row_bytes;
	if (slice_pitch < minimum_slice || capacity < required)
		return false;
	output = {width, height, depth, rows, static_cast<std::uint32_t>(row_bytes),
		row_pitch, slice_pitch, description.dimension == RHITextureDimension::Volume
			? mip : mip + layer * description.mip_count};
	return true;
}

static D3D12_RESOURCE_STATES Texture_Initial_State(const RHITexture &description) noexcept
{
	if (Has_Texture_Usage(description, RHITextureUsage::DepthStencil))
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	if (Has_Texture_Usage(description, RHITextureUsage::RenderTarget))
		return D3D12_RESOURCE_STATE_RENDER_TARGET;
	if (Has_Texture_Usage(description, RHITextureUsage::UnorderedAccess))
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	if (Has_Texture_Usage(description, RHITextureUsage::ShaderResource))
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	return D3D12_RESOURCE_STATE_COMMON;
}

static D3D12_RESOURCE_STATES Shader_Resource_State() noexcept
{
	return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
}

static D3D12_COMPARISON_FUNC To_DX12_Comparison(RHIComparison comparison) noexcept
{
	switch (comparison) {
	case RHIComparison::Never: return D3D12_COMPARISON_FUNC_NEVER;
	case RHIComparison::Less: return D3D12_COMPARISON_FUNC_LESS;
	case RHIComparison::Equal: return D3D12_COMPARISON_FUNC_EQUAL;
	case RHIComparison::LessEqual: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
	case RHIComparison::Greater: return D3D12_COMPARISON_FUNC_GREATER;
	case RHIComparison::NotEqual: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
	case RHIComparison::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	case RHIComparison::Always: return D3D12_COMPARISON_FUNC_ALWAYS;
	}
	return D3D12_COMPARISON_FUNC_ALWAYS;
}

static D3D12_STENCIL_OP To_DX12_Stencil(RHIStencilOperation operation) noexcept
{
	switch (operation) {
	case RHIStencilOperation::Keep: return D3D12_STENCIL_OP_KEEP;
	case RHIStencilOperation::Zero: return D3D12_STENCIL_OP_ZERO;
	case RHIStencilOperation::Replace: return D3D12_STENCIL_OP_REPLACE;
	case RHIStencilOperation::IncrementSaturate: return D3D12_STENCIL_OP_INCR_SAT;
	case RHIStencilOperation::DecrementSaturate: return D3D12_STENCIL_OP_DECR_SAT;
	case RHIStencilOperation::Invert: return D3D12_STENCIL_OP_INVERT;
	case RHIStencilOperation::Increment: return D3D12_STENCIL_OP_INCR;
	case RHIStencilOperation::Decrement: return D3D12_STENCIL_OP_DECR;
	}
	return D3D12_STENCIL_OP_KEEP;
}

static D3D12_BLEND To_DX12_Blend(RHIBlendFactor factor) noexcept
{
	switch (factor) {
	case RHIBlendFactor::Zero: return D3D12_BLEND_ZERO;
	case RHIBlendFactor::One: return D3D12_BLEND_ONE;
	case RHIBlendFactor::SourceColor: return D3D12_BLEND_SRC_COLOR;
	case RHIBlendFactor::InverseSourceColor: return D3D12_BLEND_INV_SRC_COLOR;
	case RHIBlendFactor::SourceAlpha: return D3D12_BLEND_SRC_ALPHA;
	case RHIBlendFactor::InverseSourceAlpha: return D3D12_BLEND_INV_SRC_ALPHA;
	case RHIBlendFactor::DestinationColor: return D3D12_BLEND_DEST_COLOR;
	case RHIBlendFactor::InverseDestinationColor: return D3D12_BLEND_INV_DEST_COLOR;
	case RHIBlendFactor::DestinationAlpha: return D3D12_BLEND_DEST_ALPHA;
	case RHIBlendFactor::InverseDestinationAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
	}
	return D3D12_BLEND_ONE;
}

static D3D12_BLEND_OP To_DX12_Blend_Operation(RHIBlendOperation operation) noexcept
{
	return operation == RHIBlendOperation::ReverseSubtract
		? D3D12_BLEND_OP_REV_SUBTRACT : D3D12_BLEND_OP_ADD;
}

static D3D12_PRIMITIVE_TOPOLOGY To_DX12_Topology(RHIPrimitiveTopology topology) noexcept
{
	switch (topology) {
	case RHIPrimitiveTopology::TriangleList: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case RHIPrimitiveTopology::TriangleStrip: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case RHIPrimitiveTopology::PointList: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	}
	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

static D3D12_PRIMITIVE_TOPOLOGY_TYPE To_DX12_Topology_Type(RHIPrimitiveTopology topology) noexcept
{
	return topology == RHIPrimitiveTopology::PointList ? D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT
		: D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
}

static D3D12_CULL_MODE To_DX12_Cull(RHICullMode mode) noexcept
{
	return mode == RHICullMode::None ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_BACK;
}

static D3D12_FILTER To_DX12_Filter(const RHISamplerDescription &sampler) noexcept
{
	if (sampler.anisotropy > 1)
		return D3D12_FILTER_ANISOTROPIC;
	const bool min_linear = sampler.minification == RHISamplerFilter::Linear;
	const bool mag_linear = sampler.magnification == RHISamplerFilter::Linear;
	const bool mip_linear = sampler.mipmap == RHISamplerFilter::Linear;
	if (min_linear && mag_linear && mip_linear) return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	if (min_linear && mag_linear && !mip_linear) return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	if (min_linear && !mag_linear && mip_linear) return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
	if (min_linear && !mag_linear && !mip_linear) return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
	if (!min_linear && mag_linear && mip_linear) return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	if (!min_linear && mag_linear && !mip_linear) return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	if (!min_linear && !mag_linear && mip_linear) return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	return D3D12_FILTER_MIN_MAG_MIP_POINT;
}

static DXGI_FORMAT To_DX12_Sampled_Format(RHITextureFormat format) noexcept
{
	switch (format) {
	case RHITextureFormat::D24_UNorm_S8: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	case RHITextureFormat::D32_Float: return DXGI_FORMAT_R32_FLOAT;
	case RHITextureFormat::D16_UNorm: return DXGI_FORMAT_R16_UNORM;
	default: return To_DX12_Format(format);
	}
}

static DXGI_FORMAT To_DX12_Storage_Format(const RHITexture &description) noexcept
{
	if (description.format == RHITextureFormat::D24_UNorm_S8)
		return DXGI_FORMAT_R24G8_TYPELESS;
	if (description.format == RHITextureFormat::D32_Float)
		return DXGI_FORMAT_R32_TYPELESS;
	if (description.format == RHITextureFormat::D16_UNorm
		&& Has_Texture_Usage(description, RHITextureUsage::ShaderResource))
		return DXGI_FORMAT_R16_TYPELESS;
	return To_DX12_Format(description.format);
}

static bool Load_Shader_Binary(const std::string &directory, const char *name,
	std::vector<std::byte> &data)
{
	if (name == nullptr)
		return false;
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

export struct DX12DeviceOptions final
{
	bool use_warp = false;
	void *window = nullptr;
	std::uint32_t width = 1280;
	std::uint32_t height = 720;
	const char *shader_directory = nullptr;
	const char *vertex_shader_name = "visual_basic.vso";
	const char *fragment_shader_name = "visual_basic.pso";
	RHITextureFormat backbuffer_format = RHITextureFormat::RGBA8_UNorm;
	bool enable_debug_layer = false;
};

export class DX12Device final : public Device
{
public:
	explicit DX12Device(DX12DeviceOptions options = {});
	~DX12Device() noexcept override = default;
	DX12Device(const DX12Device &) = delete;
	DX12Device &operator=(const DX12Device &) = delete;
	bool Is_Valid() const noexcept override;
	RHIDeviceStatus Get_Status() const noexcept override;
	bool Get_Adapter_Info(RHIAdapterInfo &info) const noexcept override;
	RHITextureLimits Texture_Limits() const noexcept override;
	RHIBufferHandle Create_Buffer(const RHIBuffer &description) override;
	RHIBufferHandle Create_Buffer_Initialized(const RHIBuffer &description, std::span<const std::byte> initial_data) override;
	RHITextureHandle Create_Texture(const RHITexture &description) override;
	RHITextureHandle Create_Texture_Initialized(const RHITexture &description, const RHITextureUpload &initial_data) override;
	RHIPipelineHandle Create_Pipeline(const RHIPipeline &description) override;
	RHIPipelineHandle Create_Pipeline(const RHIPipeline &description, RHIShaderBytecode vertex_shader, RHIShaderBytecode fragment_shader) override;
	bool Update_Buffer(RHIBufferHandle buffer, std::uint32_t offset, std::span<const std::byte> data) noexcept override;
	bool Update_Texture(RHITextureHandle texture, const RHITextureUpload &data) noexcept override;
	bool Readback_Texture(RHITextureHandle texture, std::span<std::byte> data, std::uint32_t row_pitch) noexcept override;
	bool Readback_Texture_Subresource(RHITextureHandle texture, const RHITextureReadback &data) noexcept override;
	bool Map_Texture(RHITextureHandle texture, std::uint32_t mip, std::uint32_t layer, bool read_only, RHITextureMapping &mapping) override;
	bool Unmap_Texture(RHITextureHandle texture, std::uint32_t mip, std::uint32_t layer) noexcept override;
	bool Generate_Texture_Mips(RHITextureHandle texture) noexcept override;
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
	std::unique_ptr<DX12DeviceState> m_state;
};

static bool Create_DX12_Root_Signatures(DX12DeviceState &state) noexcept;
static bool Create_DX12_Device(DX12DeviceState &state, const DX12DeviceOptions &options) noexcept;
static void Register_DX12_Debug_Messages(DX12DeviceState &state) noexcept;
static bool Create_DX12_Pipeline(DX12DeviceState &state, const RHIPipeline &description,
	std::span<const std::byte> vertex_bytecode, std::span<const std::byte> pixel_bytecode,
	DX12Pipeline &pipeline) noexcept;
static bool Allocate_CPU_Descriptors(DX12DescriptorHeap &heap, std::uint32_t &offset,
	std::uint32_t count, DX12DescriptorRange &range) noexcept;
static std::uint32_t Pipeline_Color_Variant(DXGI_FORMAT format) noexcept;
static std::uint32_t Pipeline_Depth_Variant(DXGI_FORMAT format) noexcept;

DX12DeviceState::~DX12DeviceState() noexcept
{
	if (recording)
		Submit_Current(true);
	if (last_submitted_fence != 0)
		Wait_For_Fence(last_submitted_fence);
	for (auto &frame : frames) {
		for (auto &upload : frame.transient_uploads)
			if (upload.Get() != nullptr)
				upload.Get()->Unmap(0, nullptr);
		if (frame.uploads.resource.Get() != nullptr && frame.uploads.mapped != nullptr)
			frame.uploads.resource.Get()->Unmap(0, nullptr);
	}
	if (fence_event != nullptr) {
		CloseHandle(fence_event);
		fence_event = nullptr;
	}
}

void DX12DeviceState::Collect_Deferred() noexcept
{
	if (fence.Get() != nullptr)
		completed_fence = (std::max)(completed_fence, fence.Get()->GetCompletedValue());
	for (auto it = deferred.begin(); it != deferred.end();) {
		if (it->fence <= completed_fence)
			it = deferred.erase(it);
		else
			++it;
	}
}

bool DX12DeviceState::Wait_For_Fence(std::uint64_t value) noexcept
{
	GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.WaitForFence");
	if (fence.Get() == nullptr || value == 0)
		return true;
	if (fence.Get()->GetCompletedValue() < value) {
		if (fence_event == nullptr)
			fence_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
		if (fence_event == nullptr)
			return false;
		const HRESULT event_result = fence.Get()->SetEventOnCompletion(value, fence_event);
		if (FAILED(event_result)) {
			Report_HResult("SetEventOnCompletion", event_result);
			return false;
		}
		WaitForSingleObject(fence_event, INFINITE);
	}
	completed_fence = (std::max)(completed_fence, value);
	Collect_Deferred();
	return true;
}

DX12SwapChain::~DX12SwapChain() noexcept
{
	if (m_state == nullptr)
		return;
	Release_Targets();
}

bool DX12SwapChain::Is_Valid() const noexcept
{
	if (m_state == nullptr || m_state->device.Get() == nullptr
		|| m_state->native_swap_chain.Get() == nullptr || m_width == 0 || m_height == 0)
		return false;
	for (std::uint32_t index = 0; index < SwapChainBufferCount; ++index)
		if (!m_backbuffers[index].Is_Valid() || !m_depth_targets[index].Is_Valid())
			return false;
	return true;
}

std::uint32_t DX12SwapChain::Current_Buffer_Index() const noexcept
{
	if (m_state == nullptr || m_state->native_swap_chain.Get() == nullptr)
		return 0;
	const UINT index = m_state->native_swap_chain.Get()->GetCurrentBackBufferIndex();
	return index < SwapChainBufferCount ? static_cast<std::uint32_t>(index) : 0;
}

RHIBackbuffer DX12SwapChain::Backbuffer() const noexcept
{
	return {m_backbuffers[Current_Buffer_Index()], m_width, m_height};
}

RHIDepthTarget DX12SwapChain::Depth_Target() const noexcept
{
	return {m_depth_targets[Current_Buffer_Index()], m_width, m_height};
}

void DX12SwapChain::Release_Targets() noexcept
{
	if (m_state == nullptr)
		return;
	for (auto &handle : m_backbuffers) {
		if (handle.Is_Valid())
			m_state->textures.Destroy(handle);
		handle = {};
	}
	for (auto &handle : m_depth_targets) {
		if (handle.Is_Valid())
			m_state->textures.Destroy(handle);
		handle = {};
	}
}

bool DX12SwapChain::Create_Targets(std::uint32_t width, std::uint32_t height)
{
	if (m_state == nullptr || m_state->device.Get() == nullptr
		|| m_state->native_swap_chain.Get() == nullptr || width == 0 || height == 0)
		return false;
	std::array<RHITextureHandle, SwapChainBufferCount> backbuffers{};
	std::array<RHITextureHandle, SwapChainBufferCount> depth_targets{};
	const auto destroy_created = [&]() noexcept {
		for (auto &handle : backbuffers) {
			if (handle.Is_Valid())
				m_state->textures.Destroy(handle);
			handle = {};
		}
		for (auto &handle : depth_targets) {
			if (handle.Is_Valid())
				m_state->textures.Destroy(handle);
			handle = {};
		}
	};
	for (std::uint32_t index = 0; index < SwapChainBufferCount; ++index) {
		DX12NativeObject<ID3D12Resource> native_backbuffer;
		if (FAILED(m_state->native_swap_chain.Get()->GetBuffer(index,
			IID_PPV_ARGS(native_backbuffer.Put())))) {
			destroy_created();
			return false;
		}
		DX12Texture backbuffer;
		backbuffer.object = std::move(native_backbuffer);
		backbuffer.width = width;
		backbuffer.height = height;
		const D3D12_RESOURCE_DESC native_description = backbuffer.object.Get()->GetDesc();
		backbuffer.format = native_description.Format == DXGI_FORMAT_B8G8R8A8_UNORM
			? RHITextureFormat::BGRA8_UNorm : RHITextureFormat::RGBA8_UNorm;
		backbuffer.description = {width, height, 1, backbuffer.format,
			static_cast<std::uint32_t>(RHITextureUsage::RenderTarget)};
		backbuffer.states = {D3D12_RESOURCE_STATE_PRESENT};
		if (!Allocate_CPU_Descriptors(m_state->cpu_render_targets, m_state->render_descriptor_offset,
			1, backbuffer.render_target_view)) {
			destroy_created();
			return false;
		}
		D3D12_RENDER_TARGET_VIEW_DESC render_view{};
		render_view.Format = native_description.Format;
		render_view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		m_state->device.Get()->CreateRenderTargetView(backbuffer.object.Get(), &render_view,
			m_state->CpuRTV(backbuffer.render_target_view.index));
		backbuffers[index] = m_state->textures.Create(std::move(backbuffer));
		if (!backbuffers[index].Is_Valid()) {
			destroy_created();
			return false;
		}

		D3D12_HEAP_PROPERTIES heap{};
		heap.Type = D3D12_HEAP_TYPE_DEFAULT;
		D3D12_RESOURCE_DESC depth_description{};
		depth_description.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depth_description.Width = width;
		depth_description.Height = height;
		depth_description.DepthOrArraySize = 1;
		depth_description.MipLevels = 1;
		depth_description.Format = DXGI_FORMAT_R24G8_TYPELESS;
		depth_description.SampleDesc.Count = 1;
		depth_description.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depth_description.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		D3D12_CLEAR_VALUE clear_value{};
		clear_value.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		clear_value.DepthStencil.Depth = 1.0f;
		clear_value.DepthStencil.Stencil = 0;
		DX12Texture depth;
		if (FAILED(m_state->device.Get()->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
			&depth_description, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear_value,
			IID_PPV_ARGS(depth.object.Put())))) {
			destroy_created();
			return false;
		}
		depth.width = width;
		depth.height = height;
		depth.format = RHITextureFormat::D24_UNorm_S8;
		depth.description = {width, height, 1, depth.format,
			static_cast<std::uint32_t>(RHITextureUsage::DepthStencil)};
		depth.states.assign(2, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		if (!Allocate_CPU_Descriptors(m_state->cpu_depth_targets, m_state->depth_descriptor_offset,
			1, depth.depth_stencil_view)) {
			destroy_created();
			return false;
		}
		D3D12_DEPTH_STENCIL_VIEW_DESC depth_view{};
		depth_view.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		m_state->device.Get()->CreateDepthStencilView(depth.object.Get(), &depth_view,
			m_state->CpuDSV(depth.depth_stencil_view.index));
		depth_targets[index] = m_state->textures.Create(std::move(depth));
		if (!depth_targets[index].Is_Valid()) {
			destroy_created();
			return false;
		}
	}
	m_backbuffers = backbuffers;
	m_depth_targets = depth_targets;
	m_width = width;
	m_height = height;
	return true;
}

bool DX12SwapChain::Resize(std::uint32_t width, std::uint32_t height)
{
	if (m_state == nullptr || m_state->native_swap_chain.Get() == nullptr
		|| m_state->frame_active || width == 0 || height == 0)
		return false;
	for (std::uint32_t index = 0; index < SwapChainBufferCount; ++index) {
		const auto *color = m_state->textures.Resolve(m_backbuffers[index]);
		const auto *depth = m_state->textures.Resolve(m_depth_targets[index]);
		if ((color != nullptr && color->references > 1)
			|| (depth != nullptr && depth->references > 1))
			return false;
	}
	const std::uint32_t old_width = m_width;
	const std::uint32_t old_height = m_height;
	m_state->command_list_facade.Reset_Frame_State();
	if (m_state->recording) {
		if (m_state->Submit_Current(true) == 0)
			return false;
	} else if (!m_state->Wait_For_Fence(m_state->last_submitted_fence)) {
		return false;
	}
	// All submitted references, including deferred releases, are complete before
	// releasing the native swap-chain resources required by ResizeBuffers.
	m_state->Collect_Deferred();
	Release_Targets();
	const HRESULT result = m_state->native_swap_chain.Get()->ResizeBuffers(0, width, height,
		DXGI_FORMAT_UNKNOWN, m_state->allow_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);
	if (FAILED(result)) {
		Report_HResult("ResizeBuffers", result);
		if (old_width != 0 && old_height != 0)
			Create_Targets(old_width, old_height);
		return false;
	}
	if (!Create_Targets(width, height))
		return false;
	m_state->ready_to_present = false;
	m_state->presented = false;
	return true;
}

bool DX12SwapChain::Present() noexcept
{
	if (!Is_Valid() || m_state->frame_active || !m_state->ready_to_present || m_state->presented)
		return false;
	GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.Present");
	const HRESULT result = m_state->native_swap_chain.Get()->Present(0,
		m_state->allow_tearing ? DXGI_PRESENT_ALLOW_TEARING : 0);
	if (FAILED(result)) {
		Report_HResult("Present", result);
		return false;
	}
	// Presentation queues work after the render submission's fence. Include it
	// in retirement so resize and shutdown cannot release a presented buffer
	// while the queue is still using it.
	const std::uint64_t signal = ++m_state->next_fence;
	const HRESULT signal_result = m_state->queue.Get()->Signal(m_state->fence.Get(), signal);
	if (FAILED(signal_result)) {
		Report_HResult("SignalAfterPresent", signal_result);
		return false;
	}
	m_state->last_submitted_fence = signal;
	m_state->frames[m_state->current_frame].fence = signal;
	m_state->ready_to_present = false;
	m_state->presented = true;
	// A subsequent frame may start recording while the frame runtime refreshes
	// its default attachments, before Device::Begin_Frame resets command state.
	// Do not let the closed list remember the buffer that was just presented.
	m_state->command_list_facade.Reset_Frame_State();
	return true;
}

bool DX12CommandList::Is_Ready() const noexcept
{
	return m_state != nullptr && m_state->device.Get() != nullptr
		&& m_state->command_list.Get() != nullptr && m_state->Ensure_Recording();
}

bool DX12CommandList::Is_Pipeline_Valid() const noexcept
{
	if (!Is_Ready() || !m_pipeline.Is_Valid())
		return false;
	const DX12Pipeline *pipeline = m_state->pipelines.Resolve(m_pipeline);
	return pipeline != nullptr && pipeline->pipeline_states[0].Get() != nullptr;
}

static std::uint64_t Hash_Bindless(std::span<const RHIBindlessResource> resources) noexcept
{
	std::uint64_t hash = 1469598103934665603ull;
	const auto mix = [&hash](std::uint64_t value) {
		hash ^= value;
		hash *= 1099511628211ull;
	};
	for (const auto &resource : resources) {
		mix(static_cast<std::uint64_t>(resource.type));
		mix(resource.index.Get_Index());
		mix(resource.index.Get_Generation());
		mix(resource.buffer.Get_Index());
		mix(resource.buffer.Get_Generation());
		mix(resource.texture.Get_Index());
		mix(resource.texture.Get_Generation());
		mix(static_cast<std::uint64_t>(resource.stage));
		mix(resource.constant_buffer_slot);
	}
	return hash ^ resources.size();
}

static bool Same_Bindless_Register(const RHIBindlessResource &left,
	const RHIBindlessResource &right) noexcept
{
	if (left.type != right.type)
		return false;
	if (left.type == RHIResourceType::Texture)
		return left.index.Get_Index() == right.index.Get_Index() && left.stage == right.stage;
	if (left.type == RHIResourceType::Material)
		return left.constant_buffer_slot == right.constant_buffer_slot;
	return false;
}

static bool Valid_Bindless_Resource(const DX12DeviceState &state,
	const RHIBindlessResource &resource) noexcept
{
	switch (resource.type) {
	case RHIResourceType::Texture:
		return resource.index.Is_Valid()
			&& resource.index.Get_Index() < BindlessSRVCount
			&& resource.texture.Is_Valid()
			&& state.textures.Resolve(resource.texture) != nullptr;
	case RHIResourceType::Buffer:
	{
		const DX12Buffer *buffer = state.buffers.Resolve(resource.buffer);
		return resource.buffer.Is_Valid() && buffer != nullptr
			&& buffer->shader_resource_view.Is_Valid();
	}
	case RHIResourceType::Material:
	{
		const DX12Buffer *buffer = state.buffers.Resolve(resource.buffer);
		return resource.constant_buffer_slot < BindlessCBVCount
			&& resource.buffer.Is_Valid() && buffer != nullptr
			&& buffer->usage == RHIBufferUsage::Constant && buffer->constants.page;
	}
	case RHIResourceType::Invalid:
	case RHIResourceType::Sampler:
		return true;
	}
	return false;
}

bool DX12CommandList::Bind_Pipeline(RHIPipelineHandle pipeline) noexcept
{
	GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.BindPipeline");
	if (!Is_Ready())
		return false;
	DX12Pipeline *resource = m_state->pipelines.Resolve(pipeline);
	if (resource == nullptr || resource->pipeline_states[0].Get() == nullptr)
		return false;
	if (m_pipeline == pipeline && !m_graphics_state_dirty)
		return true;
	ID3D12GraphicsCommandList *commands = m_state->command_list.Get();
	ID3D12DescriptorHeap *heaps[] = {m_state->gpu_resources.Get(), m_state->gpu_samplers.Get()};
	commands->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);
	commands->SetGraphicsRootSignature(m_state->graphics_root_signature.Get());
	commands->SetGraphicsRoot32BitConstants(3, static_cast<UINT>(resource->sampler_indices.size()),
		resource->sampler_indices.data(), 0);
	commands->IASetPrimitiveTopology(To_DX12_Topology(resource->topology));
	commands->OMSetStencilRef(resource->stencil_reference);
	m_pipeline = pipeline;
	m_topology = resource->topology;
	m_graphics_state_dirty = false;
	if (!Select_Pipeline_State())
		return false;
	Rebind_Input_Assembly();
	return true;
}

bool DX12CommandList::Bindless_Resources_Internal(std::span<const RHIBindlessResource> resources,
    bool cache) noexcept
{
    GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.BindlessResources");
    if (!Is_Ready())
        return false;
    if (cache) {
        std::size_t storage_count = 0;
        for (const auto &resource : resources) {
            storage_count += resource.type == RHIResourceType::Buffer;
            if (!Valid_Bindless_Resource(*m_state, resource))
                return false;
        }
        if (storage_count >= BindlessSRVCount)
            return false;
        try {
            if (storage_count != 0)
                std::erase_if(m_bindless_cache, [](const RHIBindlessResource &entry) {
                    return entry.type == RHIResourceType::Buffer;
                });
            for (const auto &resource : resources) {
                if (resource.type == RHIResourceType::Invalid || resource.type == RHIResourceType::Sampler)
                    continue;
                if (resource.type != RHIResourceType::Buffer)
                    std::erase_if(m_bindless_cache, [&resource](const RHIBindlessResource &entry) {
                        return Same_Bindless_Register(entry, resource);
                    });
                const bool target_conflict = resource.type == RHIResourceType::Texture
                    && (resource.texture == m_color_target || resource.texture == m_depth_target);
                if (!target_conflict)
                    m_bindless_cache.push_back(resource);
            }
        } catch (...) {
            return false;
        }
    }
    // Resource-release events remove cached identities. Internal replay operates
    // on this validated cache without repeating public-input validation.
    resources = m_bindless_cache;
    for (const auto &resource : resources)
        assert(Valid_Bindless_Resource(*m_state, resource));
    const std::uint64_t hash = Hash_Bindless(resources);
    if (!m_graphics_state_dirty && m_bindless_page != InvalidDescriptor && hash == m_bindless_hash)
        return true;
    if (m_graphics_state_dirty && m_pipeline.Is_Valid() && !Bind_Pipeline(m_pipeline))
        return false;

    std::array<std::uint32_t, BindlessSRVCount * 2> indices;
    indices.fill(PersistentDescriptorBase + m_state->null_srv_base);
    const std::uint64_t null_address = m_state->null_constant_buffer.Get()->GetGPUVirtualAddress();
    std::array<std::uint64_t, BindlessCBVCount> constants;
    constants.fill(null_address);
    if (m_draw_constant_gpu_address != 0)
        constants[1] = m_draw_constant_gpu_address;
    std::uint32_t storage_slot = BindlessSRVCount - 1;
    for (const auto &resource : resources) {
        if (resource.type == RHIResourceType::Texture) {
            DX12Texture *texture = m_state->textures.Resolve(resource.texture);
            assert(texture != nullptr && resource.index.Get_Index() < BindlessSRVCount);
            const bool target_conflict = resource.texture == m_color_target || resource.texture == m_depth_target;
            const bool sampleable = !target_conflict && texture->shader_resource_view.Is_Valid();
            if (sampleable && !m_state->Transition(*texture, Shader_Resource_State()))
                return false;
            const std::uint32_t stage = resource.stage == RHIShaderStage::Vertex ? 0 : 1;
            indices[stage * BindlessSRVCount + resource.index.Get_Index()] = PersistentDescriptorBase
                + (sampleable ? texture->shader_resource_view.index : m_state->null_srv_base);
        } else if (resource.type == RHIResourceType::Buffer) {
            const DX12Buffer *buffer = m_state->buffers.Resolve(resource.buffer);
            assert(buffer != nullptr && buffer->shader_resource_view.Is_Valid() && storage_slot != 0);
            indices[storage_slot] = indices[BindlessSRVCount + storage_slot]
                = PersistentDescriptorBase + buffer->shader_resource_view.index;
            --storage_slot;
        } else if (resource.type == RHIResourceType::Material) {
            const DX12Buffer *buffer = m_state->buffers.Resolve(resource.buffer);
            assert(buffer != nullptr && buffer->constants.page);
            constants[resource.constant_buffer_slot] = buffer->constants.GPU_Address();
        }
    }

    const bool extended_changed = std::memcmp(constants.data() + RootCBVCount,
        m_constant_addresses.data() + RootCBVCount,
        (BindlessCBVCount - RootCBVCount) * sizeof(std::uint64_t)) != 0;
    const bool has_extended = std::any_of(constants.begin() + RootCBVCount, constants.end(),
        [null_address](std::uint64_t address) { return address != null_address; });
    std::uint32_t extended_page = PersistentDescriptorBase + m_state->null_cbv_base + RootCBVCount;
    if (extended_changed && has_extended) {
        constexpr std::uint32_t count = BindlessCBVCount - RootCBVCount;
        if (m_state->frames[m_state->current_frame].descriptor_offset + count > FrameResourceDescriptors) {
            if (!m_state->Submit_Current(false) || !m_state->Ensure_Recording())
                return false;
        }
        auto &frame = m_state->frames[m_state->current_frame];
        extended_page = frame.descriptor_base + frame.descriptor_offset;
        frame.descriptor_offset += count;
        m_state->device.Get()->CopyDescriptorsSimple(count, m_state->gpu_resources.Cpu(extended_page),
            m_state->CpuResource(m_state->null_cbv_base + RootCBVCount), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        for (const auto &resource : resources) {
            if (resource.type != RHIResourceType::Material || resource.constant_buffer_slot < RootCBVCount)
                continue;
            const DX12Buffer *buffer = m_state->buffers.Resolve(resource.buffer);
            assert(buffer != nullptr);
            D3D12_CONSTANT_BUFFER_VIEW_DESC view{};
            view.BufferLocation = buffer->constants.GPU_Address();
            view.SizeInBytes = buffer->capacity;
            m_state->device.Get()->CreateConstantBufferView(&view,
                m_state->gpu_resources.Cpu(extended_page + resource.constant_buffer_slot - RootCBVCount));
        }
    }
    auto *commands = m_state->command_list.Get();
    ID3D12DescriptorHeap *heaps[] = {m_state->gpu_resources.Get(), m_state->gpu_samplers.Get()};
    commands->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);
    commands->SetGraphicsRootSignature(m_state->graphics_root_signature.Get());
    for (std::uint32_t stage = 0; stage < 2; ++stage) {
        const std::uint32_t first = stage * BindlessSRVCount;
        constexpr std::size_t bytes = BindlessSRVCount * sizeof(std::uint32_t);
        if (m_resource_index_addresses[stage] == 0
            || std::memcmp(indices.data() + first, m_resource_indices.data() + first, bytes) != 0) {
            const auto upload = m_state->Allocate_Upload(bytes, 256);
            if (upload.resource == nullptr)
                return false;
            std::memcpy(upload.cpu, indices.data() + first, bytes);
            std::memcpy(m_resource_indices.data() + first, indices.data() + first, bytes);
            m_resource_index_addresses[stage] = upload.gpu_address;
            commands->SetGraphicsRootConstantBufferView(stage, upload.gpu_address);
        }
    }
    for (std::uint32_t slot = 0; slot < RootCBVCount; ++slot)
        if (constants[slot] != m_constant_addresses[slot])
            commands->SetGraphicsRootConstantBufferView(Constant_Root_Parameter(slot), constants[slot]);
    if (extended_changed)
        commands->SetGraphicsRootDescriptorTable(2, m_state->gpu_resources.Gpu(extended_page));
    m_constant_addresses = constants;
    m_bindless_page = 0;
    m_bindless_hash = hash;
    m_constants_dirty = false;
    return true;
}

bool DX12CommandList::Set_Bindless_Resources(std::span<const RHIBindlessResource> resources) noexcept
{
	return Bindless_Resources_Internal(resources, true);
}

bool DX12CommandList::Select_Pipeline_State() noexcept
{
	GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.SelectPipelineState");
	if (!m_pipeline.Is_Valid())
		return true;
	DX12Pipeline *pipeline = m_state->pipelines.Resolve(m_pipeline);
	if (pipeline == nullptr)
		return false;
	DXGI_FORMAT color_format = DXGI_FORMAT_UNKNOWN;
	DXGI_FORMAT depth_format = DXGI_FORMAT_UNKNOWN;
	if (const DX12Texture *color = m_state->textures.Resolve(m_color_target))
		color_format = To_DX12_Format(color->format);
	if (const DX12Texture *depth = m_state->textures.Resolve(m_depth_target))
		depth_format = To_DX12_Format(depth->format);
	const std::uint32_t index = Pipeline_Depth_Variant(depth_format) * 6u
		+ Pipeline_Color_Variant(color_format);
	if (index >= pipeline->pipeline_states.size() || pipeline->pipeline_states[index].Get() == nullptr)
		return false;
	m_state->command_list.Get()->SetPipelineState(pipeline->pipeline_states[index].Get());
	return Apply_Scissor();
}

bool DX12CommandList::Apply_Scissor() noexcept
{
	if (m_state == nullptr || m_state->command_list.Get() == nullptr)
		return false;
	const DX12Pipeline *pipeline = m_state->pipelines.Resolve(m_pipeline);
	if (pipeline == nullptr)
		return true;
	RHIScissorRect scissor{};
	if (pipeline->scissor_test) {
		if (!m_has_scissor) {
			const D3D12_RECT native{};
			m_state->command_list.Get()->RSSetScissorRects(1, &native);
			return true;
		}
		scissor = m_scissor;
	} else {
		const DX12Texture *target = m_state->textures.Resolve(m_color_target);
		if (target == nullptr)
			target = m_state->textures.Resolve(m_depth_target);
		if (target == nullptr)
			return true;
		scissor.width = target->width;
		scissor.height = target->height;
	}
	const auto max_long = static_cast<std::uint32_t>((std::numeric_limits<LONG>::max)());
	if (scissor.width == 0 || scissor.height == 0
		|| scissor.x > max_long || scissor.y > max_long
		|| scissor.width > max_long || scissor.height > max_long
		|| scissor.x > max_long - scissor.width
		|| scissor.y > max_long - scissor.height)
		return false;
	D3D12_RECT native{};
	native.left = static_cast<LONG>(scissor.x);
	native.top = static_cast<LONG>(scissor.y);
	native.right = static_cast<LONG>(scissor.x + scissor.width);
	native.bottom = static_cast<LONG>(scissor.y + scissor.height);
	m_state->command_list.Get()->RSSetScissorRects(1, &native);
	return true;
}

void DX12CommandList::Rebind_Input_Assembly() noexcept
{
	ID3D12GraphicsCommandList *commands = m_state->command_list.Get();
	for (std::uint32_t slot = 0; slot < m_vertex_bindings.size(); ++slot) {
		const auto &binding = m_vertex_bindings[slot];
		if (!binding.buffer.Is_Valid())
			continue;
		const DX12Buffer *buffer = m_state->buffers.Resolve(binding.buffer);
		if (buffer == nullptr || buffer->object.Get() == nullptr)
			continue;
		D3D12_VERTEX_BUFFER_VIEW view{};
		view.BufferLocation = buffer->object.Get()->GetGPUVirtualAddress() + binding.offset;
		view.SizeInBytes = buffer->capacity > binding.offset ? buffer->capacity - binding.offset : 0;
		view.StrideInBytes = binding.stride;
		commands->IASetVertexBuffers(slot, 1, &view);
	}
	if (m_index_buffer.Is_Valid()) {
		const DX12Buffer *buffer = m_state->buffers.Resolve(m_index_buffer);
		if (buffer != nullptr && buffer->object.Get() != nullptr) {
			D3D12_INDEX_BUFFER_VIEW view{};
			view.BufferLocation = buffer->object.Get()->GetGPUVirtualAddress() + m_index_offset;
			view.SizeInBytes = buffer->capacity > m_index_offset ? buffer->capacity - m_index_offset : 0;
			view.Format = m_index_format == RHIIndexFormat::UInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
			commands->IASetIndexBuffer(&view);
		}
	}
	if (m_has_viewport) {
		D3D12_VIEWPORT viewport{};
		viewport.TopLeftX = static_cast<float>(m_viewport.x);
		viewport.TopLeftY = static_cast<float>(m_viewport.y);
		viewport.Width = static_cast<float>(m_viewport.width);
		viewport.Height = static_cast<float>(m_viewport.height);
		viewport.MinDepth = m_viewport.min_depth;
		viewport.MaxDepth = m_viewport.max_depth;
		commands->RSSetViewports(1, &viewport);
	}
}

bool DX12CommandList::Rebind_Targets() noexcept
{
	if (!m_color_target.Is_Valid() && !m_depth_target.Is_Valid())
		return true;
	D3D12_CPU_DESCRIPTOR_HANDLE color{};
	D3D12_CPU_DESCRIPTOR_HANDLE depth{};
	UINT color_count = 0;
	if (m_color_target.Is_Valid()) {
		const DX12Texture *target = m_state->textures.Resolve(m_color_target);
		if (target == nullptr || !target->render_target_view.Is_Valid()) {
			m_color_target = {};
			m_depth_target = {};
			return false;
		}
		color = m_state->CpuRTV(target->render_target_view.index);
		color_count = 1;
	}
	if (m_depth_target.Is_Valid()) {
		const DX12Texture *target = m_state->textures.Resolve(m_depth_target);
		if (target == nullptr || !target->depth_stencil_view.Is_Valid()) {
			m_color_target = {};
			m_depth_target = {};
			return false;
		}
		depth = m_state->CpuDSV(target->depth_stencil_view.index);
	}
	m_state->command_list.Get()->OMSetRenderTargets(color_count, color_count ? &color : nullptr,
		FALSE, depth.ptr != 0 ? &depth : nullptr);
	return true;
}

void DX12CommandList::On_New_Command_List() noexcept
{
	m_resource_index_addresses = {};
	m_constant_addresses = {};
	m_bindless_page = InvalidDescriptor;
	m_draw_constant_gpu_address = 0;
	m_graphics_state_dirty = true;
	if (m_pipeline.Is_Valid()) {
		const RHIPipelineHandle pipeline = m_pipeline;
		m_pipeline = {};
		// A retired pipeline must not prevent restoring the independent targets
		// and resource bindings before the next pipeline is selected.
		if (m_state->pipelines.Resolve(pipeline) != nullptr && !Bind_Pipeline(pipeline))
			return;
	}
	Rebind_Targets();
	if (!m_bindless_cache.empty())
		Bindless_Resources_Internal(m_bindless_cache, false);
	if (m_draw_constant_size != 0) {
		const auto slice = m_state->Allocate_Upload(m_draw_constant_size, 256);
		if (slice.resource != nullptr) {
			std::memcpy(slice.cpu, m_draw_constant_data.data(), m_draw_constant_size);
			m_draw_constant_gpu_address = slice.gpu_address;
			// Draw constants outlive pipeline bindings, including a retired pipeline.
			m_state->command_list.Get()->SetGraphicsRootSignature(m_state->graphics_root_signature.Get());
			m_state->command_list.Get()->SetGraphicsRootConstantBufferView(4, slice.gpu_address);
		}
	}
	Rebind_Input_Assembly();
}

static bool Remove_Bindless_Texture(std::vector<RHIBindlessResource> &resources,
	RHITextureHandle texture) noexcept
{
	resources.erase(std::remove_if(resources.begin(), resources.end(),
		[texture](const RHIBindlessResource &resource) {
			return resource.type == RHIResourceType::Texture && resource.texture == texture;
		}), resources.end());
	return true;
}

bool DX12CommandList::Set_Render_Targets(RHITextureHandle color_target,
	RHITextureHandle depth_target) noexcept
{
	if (!Is_Ready())
		return false;
	DX12Texture *color = m_state->textures.Resolve(color_target);
	DX12Texture *depth = m_state->textures.Resolve(depth_target);
	if (color == nullptr || !color->render_target_view.Is_Valid()
		|| depth == nullptr || !depth->depth_stencil_view.Is_Valid())
		return false;
	if (!m_state->Transition(*color, D3D12_RESOURCE_STATE_RENDER_TARGET)
		|| !m_state->Transition(*depth, D3D12_RESOURCE_STATE_DEPTH_WRITE))
		return false;
	const auto color_view = m_state->CpuRTV(color->render_target_view.index);
	const auto depth_view = m_state->CpuDSV(depth->depth_stencil_view.index);
	m_state->command_list.Get()->OMSetRenderTargets(1, &color_view, FALSE, &depth_view);
	m_color_target = color_target;
	m_depth_target = depth_target;
	Remove_Bindless_Texture(m_bindless_cache, color_target);
	Remove_Bindless_Texture(m_bindless_cache, depth_target);
	m_bindless_page = InvalidDescriptor;
	m_bindless_hash = 0;
	if (!m_bindless_cache.empty() && !Bindless_Resources_Internal(
		std::span<const RHIBindlessResource>(m_bindless_cache.data(), m_bindless_cache.size()), false))
		return false;
	return Select_Pipeline_State();
}

bool DX12CommandList::Set_Color_Target(RHITextureHandle color_target) noexcept
{
	if (!Is_Ready())
		return false;
	DX12Texture *color = m_state->textures.Resolve(color_target);
	if (color == nullptr || !color->render_target_view.Is_Valid()
		|| !m_state->Transition(*color, D3D12_RESOURCE_STATE_RENDER_TARGET))
		return false;
	const auto color_view = m_state->CpuRTV(color->render_target_view.index);
	m_state->command_list.Get()->OMSetRenderTargets(1, &color_view, FALSE, nullptr);
	m_color_target = color_target;
	m_depth_target = {};
	Remove_Bindless_Texture(m_bindless_cache, color_target);
	m_bindless_page = InvalidDescriptor;
	m_bindless_hash = 0;
	if (!m_bindless_cache.empty() && !Bindless_Resources_Internal(
		std::span<const RHIBindlessResource>(m_bindless_cache.data(), m_bindless_cache.size()), false))
		return false;
	return Select_Pipeline_State();
}

bool DX12CommandList::Set_Depth_Target(RHITextureHandle depth_target) noexcept
{
	if (!Is_Ready())
		return false;
	DX12Texture *depth = m_state->textures.Resolve(depth_target);
	if (depth == nullptr || !depth->depth_stencil_view.Is_Valid()
		|| !m_state->Transition(*depth, D3D12_RESOURCE_STATE_DEPTH_WRITE))
		return false;
	const auto depth_view = m_state->CpuDSV(depth->depth_stencil_view.index);
	m_state->command_list.Get()->OMSetRenderTargets(0, nullptr, FALSE, &depth_view);
	m_color_target = {};
	m_depth_target = depth_target;
	Remove_Bindless_Texture(m_bindless_cache, depth_target);
	m_bindless_page = InvalidDescriptor;
	m_bindless_hash = 0;
	if (!m_bindless_cache.empty() && !Bindless_Resources_Internal(
		std::span<const RHIBindlessResource>(m_bindless_cache.data(), m_bindless_cache.size()), false))
		return false;
	return Select_Pipeline_State();
}

bool DX12CommandList::Clear(const std::array<float, 4> &color, float depth) noexcept
{
	if (!Is_Ready() || !m_color_target.Is_Valid() || !m_depth_target.Is_Valid())
		return false;
	DX12Texture *color_target = m_state->textures.Resolve(m_color_target);
	DX12Texture *depth_target = m_state->textures.Resolve(m_depth_target);
	if (color_target == nullptr || !color_target->render_target_view.Is_Valid()
		|| depth_target == nullptr || !depth_target->depth_stencil_view.Is_Valid()
		|| !m_state->Transition(*color_target, D3D12_RESOURCE_STATE_RENDER_TARGET)
		|| !m_state->Transition(*depth_target, D3D12_RESOURCE_STATE_DEPTH_WRITE))
		return false;
	const auto color_view = m_state->CpuRTV(color_target->render_target_view.index);
	const auto depth_view = m_state->CpuDSV(depth_target->depth_stencil_view.index);
	m_state->command_list.Get()->ClearRenderTargetView(color_view, color.data(), 0, nullptr);
	m_state->command_list.Get()->ClearDepthStencilView(depth_view, D3D12_CLEAR_FLAG_DEPTH,
		depth, 0, 0, nullptr);
	return true;
}

bool DX12CommandList::Clear_Depth(float depth) noexcept
{
	if (!Is_Ready() || !m_depth_target.Is_Valid())
		return false;
	DX12Texture *target = m_state->textures.Resolve(m_depth_target);
	if (target == nullptr || !target->depth_stencil_view.Is_Valid()
		|| !m_state->Transition(*target, D3D12_RESOURCE_STATE_DEPTH_WRITE))
		return false;
	m_state->command_list.Get()->ClearDepthStencilView(m_state->CpuDSV(target->depth_stencil_view.index),
		D3D12_CLEAR_FLAG_DEPTH, depth, 0, 0, nullptr);
	return true;
}

bool DX12CommandList::Clear_Color_Target(RHITextureHandle texture,
	const std::array<float, 4> &color) noexcept
{
	if (!Is_Ready())
		return false;
	DX12Texture *target = m_state->textures.Resolve(texture);
	if (target == nullptr || !target->render_target_view.Is_Valid()
		|| !m_state->Transition(*target, D3D12_RESOURCE_STATE_RENDER_TARGET))
		return false;
	m_state->command_list.Get()->ClearRenderTargetView(m_state->CpuRTV(target->render_target_view.index),
		color.data(), 0, nullptr);
	return true;
}

bool DX12CommandList::Clear_Depth_Stencil_Target(RHITextureHandle texture, float depth,
	std::uint8_t stencil) noexcept
{
	if (!Is_Ready())
		return false;
	DX12Texture *target = m_state->textures.Resolve(texture);
	if (target == nullptr || !target->depth_stencil_view.Is_Valid()
		|| !m_state->Transition(*target, D3D12_RESOURCE_STATE_DEPTH_WRITE))
		return false;
	D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH;
	if (target->format == RHITextureFormat::D24_UNorm_S8)
		flags = static_cast<D3D12_CLEAR_FLAGS>(flags | D3D12_CLEAR_FLAG_STENCIL);
	m_state->command_list.Get()->ClearDepthStencilView(m_state->CpuDSV(target->depth_stencil_view.index),
		flags, depth, stencil, 0, nullptr);
	return true;
}

bool DX12CommandList::Copy_Texture(RHITextureHandle source,
	RHITextureHandle destination) noexcept
{
	if (!Is_Ready() || !source.Is_Valid() || !destination.Is_Valid())
		return false;
	DX12Texture *source_texture = m_state->textures.Resolve(source);
	DX12Texture *destination_texture = m_state->textures.Resolve(destination);
	if (source_texture == nullptr || destination_texture == nullptr
		|| source_texture->object.Get() == nullptr || destination_texture->object.Get() == nullptr
		|| source_texture->description.width != destination_texture->description.width
		|| source_texture->description.height != destination_texture->description.height
		|| source_texture->description.depth != destination_texture->description.depth
		|| source_texture->description.mip_count != destination_texture->description.mip_count
		|| source_texture->description.array_size != destination_texture->description.array_size
		|| source_texture->description.dimension != destination_texture->description.dimension
		|| source_texture->format != destination_texture->format)
		return false;
	const D3D12_RESOURCE_STATES source_state = source_texture->states.empty()
		? Texture_Initial_State(source_texture->description) : source_texture->states.front();
	const D3D12_RESOURCE_STATES destination_state = destination_texture->states.empty()
		? Texture_Initial_State(destination_texture->description) : destination_texture->states.front();
	if (!m_state->Transition(*source_texture, D3D12_RESOURCE_STATE_COPY_SOURCE)
		|| !m_state->Transition(*destination_texture, D3D12_RESOURCE_STATE_COPY_DEST))
		return false;
	m_state->command_list.Get()->CopyResource(destination_texture->object.Get(), source_texture->object.Get());
	m_state->Transition(*source_texture, source_state);
	m_state->Transition(*destination_texture, destination_state);
	return true;
}

bool DX12CommandList::Set_Viewport(RHIViewport viewport) noexcept
{
	if (!Is_Ready() || viewport.width == 0 || viewport.height == 0)
		return false;
	m_viewport = viewport;
	m_has_viewport = true;
	D3D12_VIEWPORT native{};
	native.TopLeftX = static_cast<float>(viewport.x);
	native.TopLeftY = static_cast<float>(viewport.y);
	native.Width = static_cast<float>(viewport.width);
	native.Height = static_cast<float>(viewport.height);
	native.MinDepth = viewport.min_depth;
	native.MaxDepth = viewport.max_depth;
	m_state->command_list.Get()->RSSetViewports(1, &native);
	return true;
}

bool DX12CommandList::Set_Scissor(RHIScissorRect scissor) noexcept
{
	if (!Is_Ready() || scissor.width == 0 || scissor.height == 0
		|| scissor.x > static_cast<std::uint32_t>(LONG_MAX)
		|| scissor.y > static_cast<std::uint32_t>(LONG_MAX)
		|| scissor.width > static_cast<std::uint32_t>(LONG_MAX)
		|| scissor.height > static_cast<std::uint32_t>(LONG_MAX)
		|| scissor.x > static_cast<std::uint32_t>(LONG_MAX) - scissor.width
		|| scissor.y > static_cast<std::uint32_t>(LONG_MAX) - scissor.height)
		return false;
	m_scissor = scissor;
	m_has_scissor = true;
	if (m_pipeline.Is_Valid())
		return Apply_Scissor();
	D3D12_RECT native{};
	native.left = static_cast<LONG>(scissor.x);
	native.top = static_cast<LONG>(scissor.y);
	native.right = static_cast<LONG>(scissor.x + scissor.width);
	native.bottom = static_cast<LONG>(scissor.y + scissor.height);
	m_state->command_list.Get()->RSSetScissorRects(1, &native);
	return true;
}

bool DX12CommandList::Set_Draw_Constants(std::span<const std::byte> data) noexcept
{
	if (!Is_Ready() || data.empty() || data.size() > m_draw_constant_data.size())
		return false;
	std::memset(m_draw_constant_data.data(), 0, m_draw_constant_data.size());
	std::memcpy(m_draw_constant_data.data(), data.data(), data.size());
	m_draw_constant_size = static_cast<std::uint32_t>((data.size() + 255u) & ~std::size_t(255u));
	const auto slice = m_state->Allocate_Upload(m_draw_constant_size, 256);
	if (slice.resource == nullptr)
		return false;
	std::memcpy(slice.cpu, m_draw_constant_data.data(), data.size());
	m_draw_constant_gpu_address = slice.gpu_address;
	m_state->command_list.Get()->SetGraphicsRootSignature(m_state->graphics_root_signature.Get());
	m_state->command_list.Get()->SetGraphicsRootConstantBufferView(4, slice.gpu_address);
	return true;
}

bool DX12CommandList::Set_Vertex_Buffer(std::uint32_t slot, RHIBufferHandle buffer,
	std::uint32_t stride, std::uint32_t offset) noexcept
{
	if (!Is_Ready() || slot >= m_vertex_bindings.size() || stride == 0)
		return false;
	DX12Buffer *resource = m_state->buffers.Resolve(buffer);
	if (resource == nullptr || resource->object.Get() == nullptr || offset >= resource->capacity)
		return false;
	D3D12_VERTEX_BUFFER_VIEW view{};
	view.BufferLocation = resource->object.Get()->GetGPUVirtualAddress() + offset;
	view.SizeInBytes = resource->capacity - offset;
	view.StrideInBytes = stride;
	m_state->command_list.Get()->IASetVertexBuffers(slot, 1, &view);
	m_vertex_bindings[slot] = {buffer, stride, offset};
	return true;
}

bool DX12CommandList::Set_Index_Buffer(RHIBufferHandle buffer, RHIIndexFormat format,
	std::uint32_t offset) noexcept
{
	if (!Is_Ready())
		return false;
	DX12Buffer *resource = m_state->buffers.Resolve(buffer);
	if (resource == nullptr || resource->object.Get() == nullptr || offset >= resource->capacity)
		return false;
	D3D12_INDEX_BUFFER_VIEW view{};
	view.BufferLocation = resource->object.Get()->GetGPUVirtualAddress() + offset;
	view.SizeInBytes = resource->capacity - offset;
	view.Format = format == RHIIndexFormat::UInt16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	m_state->command_list.Get()->IASetIndexBuffer(&view);
	m_index_buffer = buffer;
	m_index_format = format;
	m_index_offset = offset;
	return true;
}

bool DX12CommandList::Draw(std::uint32_t vertex_count, std::uint32_t first_vertex,
	std::uint32_t instance_count, std::uint32_t first_instance) noexcept
{
	if (m_constants_dirty && !Bindless_Resources_Internal(m_bindless_cache, false))
		return false;
	if (!Is_Pipeline_Valid() || vertex_count == 0 || instance_count == 0
		|| !Select_Pipeline_State())
		return false;
	m_state->command_list.Get()->DrawInstanced(vertex_count, instance_count, first_vertex, first_instance);
	Record_Draw(m_topology, vertex_count, instance_count);
	return true;
}

bool DX12CommandList::Draw_Indexed(std::uint32_t index_count, std::uint32_t first_index,
	std::int32_t base_vertex, std::uint32_t instance_count, std::uint32_t first_instance) noexcept
{
	if (m_constants_dirty && !Bindless_Resources_Internal(m_bindless_cache, false))
		return false;
	if (!Is_Pipeline_Valid() || index_count == 0 || instance_count == 0
		|| !Select_Pipeline_State())
		return false;
	m_state->command_list.Get()->DrawIndexedInstanced(index_count, instance_count, first_index,
		base_vertex, first_instance);
	Record_Draw(m_topology, index_count, instance_count);
	return true;
}

void DX12CommandList::Reset_Frame_State() noexcept
{
	m_resource_index_addresses = {};
	m_constant_addresses = {};
	m_pipeline = {};
	m_topology = RHIPrimitiveTopology::TriangleList;
	m_color_target = {};
	m_depth_target = {};
	m_bindless_cache.clear();
	m_bindless_page = InvalidDescriptor;
	m_bindless_hash = 0;
	m_draw_constant_size = 0;
	m_draw_constant_gpu_address = 0;
	m_vertex_bindings = {};
	m_index_buffer = {};
	m_has_viewport = false;
	m_graphics_state_dirty = true;
}

bool DX12CommandList::Reset_State() noexcept
{
	if (!Is_Ready())
		return false;
	Reset_Frame_State();
	return true;
}

static bool Create_Default_Buffer(ID3D12Device *device, std::uint64_t size,
	D3D12_RESOURCE_STATES initial_state, ID3D12Resource **output) noexcept
{
	if (device == nullptr || output == nullptr || size == 0)
		return false;
	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;
	D3D12_RESOURCE_DESC description{};
	description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	description.Width = size;
	description.Height = 1;
	description.DepthOrArraySize = 1;
	description.MipLevels = 1;
	description.SampleDesc.Count = 1;
	description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	const HRESULT result = device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
		&description, initial_state, nullptr, IID_PPV_ARGS(output));
	if (FAILED(result))
		Report_HResult("CreateCommittedResource(buffer)", result);
	return SUCCEEDED(result);
}

DX12Device::DX12Device(DX12DeviceOptions options)
	: m_state(std::make_unique<DX12DeviceState>())
{
	m_state->shader_directory = options.shader_directory != nullptr ? options.shader_directory : "";
	m_state->vertex_shader_name = options.vertex_shader_name != nullptr ? options.vertex_shader_name : "";
	m_state->fragment_shader_name = options.fragment_shader_name != nullptr ? options.fragment_shader_name : "";
	if (!Create_DX12_Device(*m_state, options))
		return;
	if (options.window != nullptr && options.width != 0 && options.height != 0) {
		DXGI_SWAP_CHAIN_DESC1 description{};
		description.Width = options.width;
		description.Height = options.height;
		description.Format = options.backbuffer_format == RHITextureFormat::BGRA8_UNorm
			? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
		description.SampleDesc.Count = 1;
		description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		description.BufferCount = SwapChainBufferCount;
		description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		description.Scaling = DXGI_SCALING_STRETCH;
		description.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		description.Flags = m_state->allow_tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		m_state->factory.Get()->MakeWindowAssociation(static_cast<HWND>(options.window), DXGI_MWA_NO_ALT_ENTER);
		DX12NativeObject<IDXGISwapChain1> native_swap_chain;
		const HRESULT result = m_state->factory.Get()->CreateSwapChainForHwnd(m_state->queue.Get(),
			static_cast<HWND>(options.window), &description, nullptr, nullptr,
			 native_swap_chain.Put());
		if (SUCCEEDED(result))
			native_swap_chain.Get()->QueryInterface(IID_PPV_ARGS(m_state->native_swap_chain.Put()));
		if (m_state->native_swap_chain.Get() == nullptr
			|| !m_state->swap_chain.Create_Targets(options.width, options.height)) {
			m_state->native_swap_chain.Reset();
		}
	}
}

bool DX12Device::Is_Valid() const noexcept
{
	return m_state != nullptr && m_state->device.Get() != nullptr
		&& m_state->queue.Get() != nullptr && m_state->command_list.Get() != nullptr
		&& !m_state->removed;
}

RHIDeviceStatus DX12Device::Get_Status() const noexcept
{
	if (m_state == nullptr || m_state->device.Get() == nullptr)
		return RHIDeviceStatus::Unavailable;
	if (m_state->removed || FAILED(m_state->device.Get()->GetDeviceRemovedReason()))
		return RHIDeviceStatus::Removed;
	return RHIDeviceStatus::Ready;
}

bool DX12Device::Get_Adapter_Info(RHIAdapterInfo &info) const noexcept
{
	info = {};
	if (!Is_Valid() || m_state->adapter.Get() == nullptr)
		return false;
	DXGI_ADAPTER_DESC1 description{};
	if (FAILED(m_state->adapter.Get()->GetDesc1(&description)))
		return false;
	info.vendor_id = description.VendorId;
	info.device_id = description.DeviceId;
	return true;
}

RHITextureLimits DX12Device::Texture_Limits() const noexcept
{
	return Is_Valid() ? RHITextureLimits{D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION,
		D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION} : RHITextureLimits{};
}

RHIBufferHandle DX12Device::Create_Buffer(const RHIBuffer &description)
{
	return Create_Buffer_Initialized(description, {});
}

RHIBufferHandle DX12Device::Create_Buffer_Initialized(const RHIBuffer &description,
	std::span<const std::byte> initial_data)
{
	if (!Is_Valid() || description.byte_size == 0
		|| (description.update_mode != RHIBufferUpdateMode::Preserve
			&& description.update_mode != RHIBufferUpdateMode::Discard)
		|| (description.update_mode == RHIBufferUpdateMode::Discard
			&& description.usage != RHIBufferUsage::Vertex
			&& description.usage != RHIBufferUsage::Index)
		|| initial_data.size() != 0 && initial_data.size() != description.byte_size
		|| initial_data.size() > UINT32_MAX)
		return {};
	if (description.usage == RHIBufferUsage::Storage
		&& (description.stride == 0 || description.byte_size % description.stride != 0))
		return {};
	if (description.usage == RHIBufferUsage::Constant && description.byte_size > UINT32_MAX - 255u)
		return {};
	const std::uint32_t capacity = description.usage == RHIBufferUsage::Constant
		? (description.byte_size + 255u) & ~255u : description.byte_size;
	const D3D12_RESOURCE_STATES initial_state = description.usage == RHIBufferUsage::Storage
		? Shader_Resource_State() : description.usage == RHIBufferUsage::Index
		? D3D12_RESOURCE_STATE_INDEX_BUFFER : D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	DX12Buffer resource;
	resource.usage = description.usage;
	resource.update_mode = description.update_mode;
	resource.byte_size = description.byte_size;
	resource.capacity = capacity;
	resource.stride = description.stride;
	resource.state = initial_state;
	if (description.usage == RHIBufferUsage::Constant) {
		resource.constant_data.resize(capacity);
		if (!initial_data.empty())
			std::memcpy(resource.constant_data.data(), initial_data.data(), initial_data.size());
		resource.constants = m_state->constant_memory.Allocate(m_state->device.Get(), capacity,
			m_state->completed_fence);
		if (!resource.constants.page)
			return {};
		std::memcpy(resource.constants.page->cpu + resource.constants.offset,
			resource.constant_data.data(), capacity);
	} else {
		resource.object = m_state->buffer_cache.Take(description.usage, capacity,
			description.update_mode, m_state->fence.Get() != nullptr
				? m_state->fence.Get()->GetCompletedValue() : 0);
		if (resource.object.Get() == nullptr
			&& !Create_Default_Buffer(m_state->device.Get(), capacity, initial_state, resource.object.Put()))
			return {};
	}
	if (description.usage == RHIBufferUsage::Storage) {
		if (!Allocate_CPU_Descriptors(m_state->cpu_resources, m_state->resource_descriptor_offset,
			1, resource.shader_resource_view))
			return {};
		D3D12_SHADER_RESOURCE_VIEW_DESC view{};
		view.Format = DXGI_FORMAT_UNKNOWN;
		view.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		view.Buffer.NumElements = description.byte_size / description.stride;
		view.Buffer.StructureByteStride = description.stride;
		m_state->device.Get()->CreateShaderResourceView(resource.object.Get(), &view,
			m_state->CpuResource(resource.shader_resource_view.index));
		Publish_Shader_Resource(*m_state, resource.shader_resource_view);
	}

	const RHIBufferHandle handle = m_state->buffers.Create(std::move(resource));
	if (!handle.Is_Valid())
		return {};
	if (description.usage != RHIBufferUsage::Constant
		&& !initial_data.empty() && !Update_Buffer(handle, 0, initial_data)) {
		Destroy_Buffer(handle);
		return {};
	}
	return handle;
}

static bool Create_Texture_Views(DX12DeviceState &state, DX12Texture &resource) noexcept
{
	const RHITexture &description = resource.description;
	const bool volume = description.dimension == RHITextureDimension::Volume;
	const bool cube = description.dimension == RHITextureDimension::Cube;
	const DXGI_FORMAT storage_format = To_DX12_Storage_Format(description);
	const DXGI_FORMAT sampled_format = To_DX12_Sampled_Format(description.format);
	const auto make_srv_dimension = [&]() {
		return volume ? D3D12_SRV_DIMENSION_TEXTURE3D : cube
			? D3D12_SRV_DIMENSION_TEXTURE2DARRAY : description.array_size > 1
			? D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION_TEXTURE2D;
	};
	if (Has_Texture_Usage(description, RHITextureUsage::ShaderResource)) {
		if (!Allocate_CPU_Descriptors(state.cpu_resources, state.resource_descriptor_offset,
			1, resource.shader_resource_view))
			return false;
		D3D12_SHADER_RESOURCE_VIEW_DESC view{};
		view.Format = sampled_format;
		view.ViewDimension = make_srv_dimension();
		view.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if (volume) {
			view.Texture3D.MipLevels = description.mip_count;
		} else {
			view.Texture2DArray.MipLevels = description.mip_count;
			view.Texture2DArray.ArraySize = description.array_size;
		}
		state.device.Get()->CreateShaderResourceView(resource.object.Get(), &view,
			state.CpuResource(resource.shader_resource_view.index));
		Publish_Shader_Resource(state, resource.shader_resource_view);
	}
	if (Has_Texture_Usage(description, RHITextureUsage::RenderTarget)) {
		if (!Allocate_CPU_Descriptors(state.cpu_render_targets, state.render_descriptor_offset,
			1, resource.render_target_view))
			return false;
		D3D12_RENDER_TARGET_VIEW_DESC view{};
		view.Format = To_DX12_Format(description.format);
		if (volume) {
			view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			view.Texture3D.MipSlice = 0;
			view.Texture3D.FirstWSlice = 0;
			view.Texture3D.WSize = description.depth;
		} else if (description.array_size > 1) {
			view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			view.Texture2DArray.ArraySize = 1;
			view.Texture2DArray.FirstArraySlice = description.output_layer;
		} else {
			view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		}
		state.device.Get()->CreateRenderTargetView(resource.object.Get(), &view,
			state.CpuRTV(resource.render_target_view.index));
	}
	if (Has_Texture_Usage(description, RHITextureUsage::DepthStencil)) {
		if (!Allocate_CPU_Descriptors(state.cpu_depth_targets, state.depth_descriptor_offset,
			1, resource.depth_stencil_view))
			return false;
		D3D12_DEPTH_STENCIL_VIEW_DESC view{};
		view.Format = To_DX12_Format(description.format);
		if (description.array_size > 1) {
			view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			view.Texture2DArray.ArraySize = 1;
			view.Texture2DArray.FirstArraySlice = description.output_layer;
		} else {
			view.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		}
		state.device.Get()->CreateDepthStencilView(resource.object.Get(), &view,
			state.CpuDSV(resource.depth_stencil_view.index));
	}
	if (Has_Texture_Usage(description, RHITextureUsage::UnorderedAccess)) {
		if (!Allocate_CPU_Descriptors(state.cpu_resources, state.resource_descriptor_offset,
			1, resource.unordered_access_view))
			return false;
		D3D12_UNORDERED_ACCESS_VIEW_DESC view{};
		view.Format = storage_format;
		if (volume) {
			view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
			view.Texture3D.WSize = description.depth;
		} else {
			view.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
			view.Texture2DArray.ArraySize = description.array_size;
		}
		state.device.Get()->CreateUnorderedAccessView(resource.object.Get(), nullptr, &view,
			state.CpuResource(resource.unordered_access_view.index));
	}
	if (description.generate_mips) {
		try {
			resource.mip_shader_resource_views.resize(description.mip_count);
			if (resource.compute_mips)
				resource.mip_unordered_access_views.resize(description.mip_count);
		} catch (...) {
			return false;
		}
		if (!resource.compute_mips) {
			std::uint32_t count = 0;
			for (std::uint32_t mip = 1; mip < description.mip_count; ++mip)
				count += volume ? (std::max)(1u, description.depth >> mip) : description.array_size;
			if (count != 0 && !Allocate_CPU_Descriptors(state.cpu_render_targets,
				state.render_descriptor_offset, count, resource.mip_render_targets))
				return false;
		}
		std::uint32_t render_offset = 0;
		for (std::uint32_t mip = 0; mip < description.mip_count; ++mip) {
			if (!Allocate_CPU_Descriptors(state.cpu_resources, state.resource_descriptor_offset,
				1, resource.mip_shader_resource_views[mip]))
				return false;
			D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
			srv.Format = sampled_format;
			srv.ViewDimension = volume ? D3D12_SRV_DIMENSION_TEXTURE3D
				: description.array_size > 1 ? D3D12_SRV_DIMENSION_TEXTURE2DARRAY
				: D3D12_SRV_DIMENSION_TEXTURE2D;
			srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			if (volume) {
				srv.Texture3D.MostDetailedMip = mip;
				srv.Texture3D.MipLevels = 1;
			} else if (description.array_size > 1) {
				srv.Texture2DArray.MostDetailedMip = mip;
				srv.Texture2DArray.MipLevels = 1;
				srv.Texture2DArray.ArraySize = description.array_size;
			} else {
				srv.Texture2D.MostDetailedMip = mip;
				srv.Texture2D.MipLevels = 1;
			}
			state.device.Get()->CreateShaderResourceView(resource.object.Get(), &srv,
				state.CpuResource(resource.mip_shader_resource_views[mip].index));
			if (!resource.compute_mips) {
				if (mip == 0)
					continue;
				const std::uint32_t slices = volume ? (std::max)(1u, description.depth >> mip)
					: description.array_size;
				for (std::uint32_t slice = 0; slice < slices; ++slice) {
					D3D12_RENDER_TARGET_VIEW_DESC rtv{};
					rtv.Format = To_DX12_Format(description.format);
					if (volume) {
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
						rtv.Texture3D = {mip, slice, 1};
					} else if (description.array_size > 1) {
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
						rtv.Texture2DArray = {mip, slice, 1, 0};
					} else {
						rtv.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
						rtv.Texture2D.MipSlice = mip;
					}
					state.device.Get()->CreateRenderTargetView(resource.object.Get(), &rtv,
						state.CpuRTV(resource.mip_render_targets.index + render_offset++));
				}
				continue;
			}
			if (!Allocate_CPU_Descriptors(state.cpu_resources, state.resource_descriptor_offset,
				1, resource.mip_unordered_access_views[mip]))
				return false;
			D3D12_UNORDERED_ACCESS_VIEW_DESC uav{};
			uav.Format = storage_format;
			uav.ViewDimension = volume ? D3D12_UAV_DIMENSION_TEXTURE3D
				: description.array_size > 1 ? D3D12_UAV_DIMENSION_TEXTURE2DARRAY
				: D3D12_UAV_DIMENSION_TEXTURE2D;
			if (volume) {
				uav.Texture3D.MipSlice = mip;
				uav.Texture3D.WSize = (std::max)(1u, description.depth >> mip);
			} else if (description.array_size > 1) {
				uav.Texture2DArray.MipSlice = mip;
				uav.Texture2DArray.ArraySize = description.array_size;
			} else {
				uav.Texture2D.MipSlice = mip;
			}
			state.device.Get()->CreateUnorderedAccessView(resource.object.Get(), nullptr, &uav,
				state.CpuResource(resource.mip_unordered_access_views[mip].index));
		}
	}
	return true;
}

RHITextureHandle DX12Device::Create_Texture(const RHITexture &description)
{
	return Create_Texture_Initialized(description, {});
}

RHITextureHandle DX12Device::Create_Texture_Initialized(const RHITexture &description,
	const RHITextureUpload &initial_data)
{
	if (!Is_Valid() || description.width == 0 || description.height == 0 || description.depth == 0
		|| description.array_size == 0 || description.mip_count == 0 || description.mip_count > 15
		|| description.output_layer >= description.array_size
		|| To_DX12_Format(description.format) == DXGI_FORMAT_UNKNOWN)
		return {};
	const bool volume = description.dimension == RHITextureDimension::Volume;
	const bool cube = description.dimension == RHITextureDimension::Cube;
	if (volume ? description.array_size != 1 : description.depth != 1)
		return {};
	if (cube && (description.width != description.height || description.array_size != 6))
		return {};
	if (Is_Block_Compressed(description.format)
		&& (volume || description.generate_mips || Has_Texture_Usage(description, RHITextureUsage::RenderTarget)
			|| Has_Texture_Usage(description, RHITextureUsage::UnorderedAccess)))
		return {};
	const bool depth = description.format == RHITextureFormat::D24_UNorm_S8
		|| description.format == RHITextureFormat::D32_Float || description.format == RHITextureFormat::D16_UNorm;
	if (depth && (volume || description.generate_mips || !initial_data.data.empty()))
		return {};
	if (description.generate_mips && (!Has_Texture_Usage(description, RHITextureUsage::ShaderResource)
		|| depth))
		return {};
	DX12Texture resource;
	resource.description = description;
	resource.width = description.width;
	resource.height = description.height;
	resource.format = description.format;
	if (description.generate_mips) {
		D3D12_FEATURE_DATA_FORMAT_SUPPORT support{To_DX12_Storage_Format(description)};
		if (FAILED(m_state->device.Get()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT,
			&support, sizeof(support))))
			return {};
		resource.compute_mips = (support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE) != 0;
		if (!resource.compute_mips && (support.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) == 0)
			return {};
	}
	D3D12_RESOURCE_DESC native{};
	native.Dimension = volume ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	native.Width = description.width;
	native.Height = description.height;
	native.DepthOrArraySize = static_cast<UINT16>(volume ? description.depth : description.array_size);
	native.MipLevels = static_cast<UINT16>(description.mip_count);
	native.Format = To_DX12_Storage_Format(description);
	native.SampleDesc.Count = 1;
	native.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	if (Has_Texture_Usage(description, RHITextureUsage::RenderTarget)
		|| (description.generate_mips && !resource.compute_mips))
		native.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	if (Has_Texture_Usage(description, RHITextureUsage::DepthStencil))
		native.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	if (Has_Texture_Usage(description, RHITextureUsage::UnorderedAccess) || resource.compute_mips)
		native.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	D3D12_CLEAR_VALUE clear_value{};
	const D3D12_CLEAR_VALUE *clear_pointer = nullptr;
	if (Has_Texture_Usage(description, RHITextureUsage::RenderTarget)) {
		clear_value.Format = To_DX12_Format(description.format);
		clear_value.Color[0] = clear_value.Color[1] = clear_value.Color[2] = clear_value.Color[3] = 0;
		clear_pointer = &clear_value;
	} else if (Has_Texture_Usage(description, RHITextureUsage::DepthStencil)) {
		clear_value.Format = To_DX12_Format(description.format);
		clear_value.DepthStencil.Depth = 1;
		clear_pointer = &clear_value;
	}
	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_DEFAULT;
	const HRESULT create_result = m_state->device.Get()->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
		&native, Texture_Initial_State(description), clear_pointer, IID_PPV_ARGS(resource.object.Put()));
	if (FAILED(create_result) || !Create_Texture_Views(*m_state, resource))
		return {};
	const std::uint32_t plane_count = description.format == RHITextureFormat::D24_UNorm_S8 ? 2u : 1u;
	const std::uint32_t subresources = (volume ? description.mip_count
		: description.mip_count * description.array_size) * plane_count;
	try {
		resource.states.assign(subresources, Texture_Initial_State(description));
	} catch (...) {
		return {};
	}
	const RHITextureHandle handle = m_state->textures.Create(std::move(resource));
	if (!handle.Is_Valid())
		return {};
	if (!initial_data.data.empty() && !Update_Texture(handle, initial_data)) {
		Destroy_Texture(handle);
		return {};
	}
	return handle;
}

static bool Transition_Buffer(DX12DeviceState &state, DX12Buffer &buffer,
	D3D12_RESOURCE_STATES desired) noexcept
{
	if (buffer.object.Get() == nullptr || !state.Ensure_Recording())
		return false;
	if (buffer.state == desired)
		return true;
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = buffer.object.Get();
	barrier.Transition.StateBefore = buffer.state;
	barrier.Transition.StateAfter = desired;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	state.command_list.Get()->ResourceBarrier(1, &barrier);
	buffer.state = desired;
	return true;
}

bool DX12Device::Update_Buffer(RHIBufferHandle buffer, std::uint32_t offset,
	std::span<const std::byte> data) noexcept
{
	GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.UpdateBuffer");
	if (!Is_Valid() || data.empty() || data.size() > UINT32_MAX)
		return false;
	DX12Buffer *resource = m_state->buffers.Resolve(buffer);
	if (resource == nullptr || offset > resource->byte_size
		|| data.size() > resource->byte_size - offset
		|| (resource->update_mode == RHIBufferUpdateMode::Discard && offset != 0))
		return false;
	if (resource->usage == RHIBufferUsage::Constant) {
		auto next = m_state->constant_memory.Allocate(m_state->device.Get(), resource->capacity,
			m_state->completed_fence);
		if (!next.page)
			return false;
		std::memcpy(resource->constant_data.data() + offset, data.data(), data.size());
		std::memcpy(next.page->cpu + next.offset, resource->constant_data.data(), resource->capacity);
		resource->constants.Retire(m_state->Retirement_Fence());
		resource->constants = std::move(next);

		m_state->command_list_facade.Invalidate_Constants();
		return true;
	}
	const auto upload = m_state->Allocate_Upload(data.size(), 256);
	if (upload.resource == nullptr)
		return false;
	std::memcpy(upload.cpu, data.data(), data.size());
	const D3D12_RESOURCE_STATES old_state = resource->state;
	if (!Transition_Buffer(*m_state, *resource, D3D12_RESOURCE_STATE_COPY_DEST))
		return false;
	m_state->command_list.Get()->CopyBufferRegion(resource->object.Get(), offset,
		upload.resource, upload.offset, data.size());
	Transition_Buffer(*m_state, *resource, old_state);
	return true;
}

bool DX12Device::Update_Texture(RHITextureHandle texture,
	const RHITextureUpload &data) noexcept
{
	if (!Is_Valid())
		return false;
	DX12Texture *resource = m_state->textures.Resolve(texture);
	TextureTransferLayout layout;
	if (resource == nullptr || resource->object.Get() == nullptr
		|| Has_Texture_Usage(resource->description, RHITextureUsage::DepthStencil)
		|| !Texture_Transfer_Layout(resource->description, data.mip_level, data.array_layer,
			data.row_pitch, data.slice_pitch, data.data.size(), layout))
		return false;
	D3D12_RESOURCE_DESC description = resource->object.Get()->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
	UINT rows = 0;
	UINT64 row_size = 0;
	UINT64 total_size = 0;
	m_state->device.Get()->GetCopyableFootprints(&description, layout.subresource, 1, 0,
		&footprint, &rows, &row_size, &total_size);
	const auto upload = m_state->Allocate_Upload(total_size, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
	if (upload.resource == nullptr || rows < layout.rows || footprint.Footprint.RowPitch < layout.row_bytes)
		return false;
	for (std::uint32_t slice = 0; slice < layout.depth; ++slice) {
		for (std::uint32_t row = 0; row < layout.rows; ++row) {
			std::byte *destination = upload.cpu + footprint.Offset
				+ static_cast<std::size_t>(slice) * footprint.Footprint.RowPitch * rows
				+ static_cast<std::size_t>(row) * footprint.Footprint.RowPitch;
			const std::byte *source = data.data.data()
				+ static_cast<std::size_t>(slice) * layout.slice_pitch
				+ static_cast<std::size_t>(row) * layout.row_pitch;
			std::memcpy(destination, source, layout.row_bytes);
		}
	}
	const D3D12_RESOURCE_STATES old_state = resource->states.empty()
		? Texture_Initial_State(resource->description) : resource->states[layout.subresource];
	if (!m_state->Transition(*resource, D3D12_RESOURCE_STATE_COPY_DEST, layout.subresource))
		return false;
	D3D12_TEXTURE_COPY_LOCATION source{};
	source.pResource = upload.resource;
	source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	source.PlacedFootprint = footprint;
	source.PlacedFootprint.Offset += upload.offset;
	D3D12_TEXTURE_COPY_LOCATION destination{};
	destination.pResource = resource->object.Get();
	destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	destination.SubresourceIndex = layout.subresource;
	m_state->command_list.Get()->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
	m_state->Transition(*resource, old_state, layout.subresource);
	return true;
}

bool DX12Device::Readback_Texture(RHITextureHandle texture, std::span<std::byte> data,
	std::uint32_t row_pitch) noexcept
{
	return Readback_Texture_Subresource(texture, {data, row_pitch, 0, 0, 0});
}

bool DX12Device::Map_Texture(RHITextureHandle texture, std::uint32_t mip,
	std::uint32_t layer, bool read_only, RHITextureMapping &output)
{
	output = {};
	if (!Is_Valid())
		return false;
	DX12Texture *resource = m_state->textures.Resolve(texture);
	TextureTransferLayout layout;
	if (resource == nullptr || resource->object.Get() == nullptr
		|| (!read_only && Has_Texture_Usage(resource->description, RHITextureUsage::DepthStencil))
		|| !Texture_Transfer_Layout(resource->description, mip, layer, 0, 0, SIZE_MAX, layout))
		return false;
	for (const auto &mapping : resource->mappings)
		if (mapping->subresource == layout.subresource)
			return false;
	D3D12_RESOURCE_DESC description = resource->object.Get()->GetDesc();
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
	UINT rows = 0;
	UINT64 row_size = 0;
	UINT64 total_size = 0;
	m_state->device.Get()->GetCopyableFootprints(&description, layout.subresource, 1, 0,
		&footprint, &rows, &row_size, &total_size);
	std::uint64_t mapped_size = footprint.Offset
		+ static_cast<std::uint64_t>(footprint.Footprint.RowPitch) * rows * layout.depth;
	const bool packed_depth_stencil = resource->format == RHITextureFormat::D24_UNorm_S8;
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT stencil_footprint{};
	std::uint32_t stencil_subresource = 0;
	D3D12_RESOURCE_STATES stencil_state = D3D12_RESOURCE_STATE_COMMON;
	if (packed_depth_stencil) {
		stencil_subresource = layout.subresource
			+ resource->description.mip_count * resource->description.array_size;
		if (stencil_subresource >= resource->states.size())
			return false;
		stencil_state = resource->states[stencil_subresource];
		UINT stencil_rows = 0;
		const std::uint64_t stencil_offset = (mapped_size + D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT - 1u)
			& ~(static_cast<std::uint64_t>(D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT) - 1u);
		m_state->device.Get()->GetCopyableFootprints(&description, stencil_subresource, 1,
			stencil_offset, &stencil_footprint, &stencil_rows, nullptr, nullptr);
		mapped_size = stencil_footprint.Offset
			+ static_cast<std::uint64_t>(stencil_footprint.Footprint.RowPitch) * stencil_rows * layout.depth;
	}
	D3D12_HEAP_PROPERTIES readback_heap{};
	readback_heap.Type = D3D12_HEAP_TYPE_READBACK;
	D3D12_RESOURCE_DESC buffer_description{};
	buffer_description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buffer_description.Width = mapped_size;
	buffer_description.Height = 1;
	buffer_description.DepthOrArraySize = 1;
	buffer_description.MipLevels = 1;
	buffer_description.SampleDesc.Count = 1;
	buffer_description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	auto mapping = std::make_unique<DX12TextureMapping>();
	if (FAILED(m_state->device.Get()->CreateCommittedResource(&readback_heap, D3D12_HEAP_FLAG_NONE,
		&buffer_description, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
		IID_PPV_ARGS(mapping->readback.Put()))))
		return false;
	const D3D12_RESOURCE_STATES old_state = resource->states[layout.subresource];
	if (!m_state->Transition(*resource, D3D12_RESOURCE_STATE_COPY_SOURCE, layout.subresource))
		return false;
	D3D12_TEXTURE_COPY_LOCATION source{};
	source.pResource = resource->object.Get();
	source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	source.SubresourceIndex = layout.subresource;
	D3D12_TEXTURE_COPY_LOCATION destination{};
	destination.pResource = mapping->readback.Get();
	destination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	destination.PlacedFootprint = footprint;
	m_state->command_list.Get()->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
	m_state->Transition(*resource, old_state, layout.subresource);
	if (packed_depth_stencil) {
		if (!m_state->Transition(*resource, D3D12_RESOURCE_STATE_COPY_SOURCE, stencil_subresource))
			return false;
		source.SubresourceIndex = stencil_subresource;
		destination.PlacedFootprint = stencil_footprint;
		m_state->command_list.Get()->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
		m_state->Transition(*resource, stencil_state, stencil_subresource);
	}
	if (!m_state->Submit_Current(true))
		return false;
	void *readback_bytes = nullptr;
	if (FAILED(mapping->readback.Get()->Map(0, nullptr, &readback_bytes)))
		return false;
	mapping->readback_mapped = true;
	std::byte *mapped_output = static_cast<std::byte *>(readback_bytes) + footprint.Offset;
	if (packed_depth_stencil) {
		// D3D12 copies stencil as an R8 plane. The public mapping retains the
		// packed D24S8 layout used by callers and the other backend.
		const auto *stencil_bytes = static_cast<const std::byte *>(readback_bytes) + stencil_footprint.Offset;
		for (std::uint32_t row = 0; row < layout.rows; ++row)
			for (std::uint32_t column = 0; column < layout.width; ++column)
				mapped_output[static_cast<std::size_t>(row) * footprint.Footprint.RowPitch + column * 4u + 3u]
					= stencil_bytes[static_cast<std::size_t>(row) * stencil_footprint.Footprint.RowPitch + column];
	}
	if (!read_only) {
		D3D12_HEAP_PROPERTIES upload_heap{};
		upload_heap.Type = D3D12_HEAP_TYPE_UPLOAD;
		if (FAILED(m_state->device.Get()->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE,
			&buffer_description, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
			IID_PPV_ARGS(mapping->upload.Put()))))
			return false;
		void *upload_bytes = nullptr;
		if (FAILED(mapping->upload.Get()->Map(0, nullptr, &upload_bytes)))
			return false;
		mapping->upload_mapped = true;
		std::byte *upload_output = static_cast<std::byte *>(upload_bytes) + footprint.Offset;
		for (std::uint32_t slice = 0; slice < layout.depth; ++slice)
			for (std::uint32_t row = 0; row < layout.rows; ++row)
				std::memcpy(upload_output + static_cast<std::size_t>(slice) * footprint.Footprint.RowPitch * rows
					+ static_cast<std::size_t>(row) * footprint.Footprint.RowPitch,
					mapped_output + static_cast<std::size_t>(slice) * footprint.Footprint.RowPitch * rows
					+ static_cast<std::size_t>(row) * footprint.Footprint.RowPitch,
					footprint.Footprint.RowPitch);
		mapping->readback.Get()->Unmap(0, nullptr);
		mapping->readback_mapped = false;
		mapped_output = upload_output;
	}
	mapping->subresource = layout.subresource;
	mapping->depth = layout.depth;
	mapping->rows = layout.rows;
	mapping->row_bytes = layout.row_bytes;
	mapping->row_pitch = footprint.Footprint.RowPitch;
	mapping->slice_pitch = footprint.Footprint.RowPitch * rows;
	mapping->footprint_offset = footprint.Offset;
	mapping->return_state = old_state;
	mapping->read_only = read_only;
	DX12TextureMapping *mapping_pointer = mapping.get();
	try {
		resource->mappings.push_back(std::move(mapping));
	} catch (...) {
		return false;
	}
	output = {std::span<std::byte>(mapped_output,
		static_cast<std::size_t>(mapping_pointer->slice_pitch) * (layout.depth - 1u)
			+ static_cast<std::size_t>(mapping_pointer->row_pitch) * (layout.rows - 1u)
			+ layout.row_bytes), mapping_pointer->row_pitch, mapping_pointer->slice_pitch};
	return true;
}

bool DX12Device::Unmap_Texture(RHITextureHandle texture, std::uint32_t mip,
	std::uint32_t layer) noexcept
{
	if (!Is_Valid())
		return false;
	DX12Texture *resource = m_state->textures.Resolve(texture);
	if (resource == nullptr || mip >= resource->description.mip_count
		|| layer >= resource->description.array_size)
		return false;
	const std::uint32_t subresource = resource->description.dimension == RHITextureDimension::Volume
		? mip : mip + layer * resource->description.mip_count;
	const auto found = std::find_if(resource->mappings.begin(), resource->mappings.end(),
		[subresource](const auto &mapping) { return mapping->subresource == subresource; });
	if (found == resource->mappings.end())
		return false;
	DX12TextureMapping &mapping = **found;
	if (mapping.readback_mapped && mapping.readback.Get() != nullptr) {
		mapping.readback.Get()->Unmap(0, nullptr);
		mapping.readback_mapped = false;
	}
	if (!mapping.read_only) {
		if (!m_state->Ensure_Recording())
			return false;
		const D3D12_RESOURCE_STATES old_state = resource->states[subresource];
		if (!m_state->Transition(*resource, D3D12_RESOURCE_STATE_COPY_DEST, subresource))
			return false;
		D3D12_TEXTURE_COPY_LOCATION source{};
		source.pResource = mapping.upload.Get();
		source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		source.PlacedFootprint.Offset = mapping.footprint_offset;
		source.PlacedFootprint.Footprint.Format = To_DX12_Storage_Format(resource->description);
		source.PlacedFootprint.Footprint.Width = (std::max)(1u, resource->description.width >> mip);
		source.PlacedFootprint.Footprint.Height = (std::max)(1u, resource->description.height >> mip);
		source.PlacedFootprint.Footprint.Depth = static_cast<UINT16>(mapping.depth);
		source.PlacedFootprint.Footprint.RowPitch = mapping.row_pitch;
		D3D12_TEXTURE_COPY_LOCATION destination{};
		destination.pResource = resource->object.Get();
		destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		destination.SubresourceIndex = subresource;
		m_state->command_list.Get()->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
		m_state->Transition(*resource, mapping.return_state, subresource);
		// Upload resources remain mapped and owned by the frame until its fence
		// retires. Releasing the mapping here would release a resource referenced
		// by the copy command before the GPU executes it.
		try {
			m_state->frames[m_state->current_frame].transient_uploads.push_back(
				std::move(mapping.upload));
			mapping.upload_mapped = false;
		} catch (...) {
			// The normal path is fence-scoped and does not wait per upload. If the
			// ownership transfer cannot allocate, drain this already-recorded copy
			// before releasing its upload resource.
			if (m_state->Submit_Current(true) == 0)
				return false;
			if (mapping.upload_mapped && mapping.upload.Get() != nullptr) {
				mapping.upload.Get()->Unmap(0, nullptr);
				mapping.upload_mapped = false;
			}
		}
	}
	resource->mappings.erase(found);
	return true;
}

bool DX12Device::Readback_Texture_Subresource(RHITextureHandle texture,
	const RHITextureReadback &data) noexcept
{
	if (!Is_Valid())
		return false;
	const DX12Texture *resource = m_state->textures.Resolve(texture);
	TextureTransferLayout layout;
	if (resource == nullptr || !Texture_Transfer_Layout(resource->description, data.mip_level,
		data.array_layer, data.row_pitch, data.slice_pitch, data.data.size(), layout))
		return false;
	RHITextureMapping mapping;
	if (!Map_Texture(texture, data.mip_level, data.array_layer, true, mapping))
		return false;
	for (std::uint32_t slice = 0; slice < layout.depth; ++slice)
		for (std::uint32_t row = 0; row < layout.rows; ++row)
			std::memcpy(data.data.data() + static_cast<std::size_t>(slice) * layout.slice_pitch
				+ static_cast<std::size_t>(row) * layout.row_pitch,
				mapping.bytes.data() + static_cast<std::size_t>(slice) * mapping.slice_pitch
				+ static_cast<std::size_t>(row) * mapping.row_pitch, layout.row_bytes);
	return Unmap_Texture(texture, data.mip_level, data.array_layer);
}

static int Mip_Component_Kind(RHITextureFormat format) noexcept
{
	switch (format) {
	case RHITextureFormat::R8_UNorm: return 1;
	case RHITextureFormat::RG8_UNorm: return 2;
	case RHITextureFormat::RGBA8_UNorm:
	case RHITextureFormat::BGRA8_UNorm:
	case RHITextureFormat::RGBA16_Float:
	case RHITextureFormat::RGBA32_Float: return 4;
	case RHITextureFormat::R32_Float: return 1;
	default: return 0;
	}
}

static bool Ensure_Mip_Pipeline(DX12DeviceState &state, RHITextureFormat format,
	RHITextureDimension dimension, bool array_view, ID3D12PipelineState **output) noexcept
{
	const int components = Mip_Component_Kind(format);
	if (components == 0 || output == nullptr)
		return false;
	const std::uint32_t dimension_index = dimension == RHITextureDimension::Volume ? 2u
		: array_view ? 1u : 0u;
	const std::uint32_t cache_index = static_cast<std::uint32_t>(format) * 3u + dimension_index;
	if (cache_index >= state.mip_pipeline_states.size())
		return false;
	if (state.mip_pipeline_states[cache_index].Get() != nullptr) {
		*output = state.mip_pipeline_states[cache_index].Get();
		return true;
	}
	const char *value_type = components == 1 ? "float" : components == 2 ? "float2" : "float4";
	const char *resource_type = dimension == RHITextureDimension::Volume ? "Texture3D"
		: array_view ? "Texture2DArray" : "Texture2D";
	const char *rw_resource_type = dimension == RHITextureDimension::Volume ? "RWTexture3D"
		: array_view ? "RWTexture2DArray" : "RWTexture2D";
	std::string source;
	source.reserve(1800);
	source += resource_type;
	source += "<";
	source += value_type;
	source += "> g_Source : register(t0);";
	source += rw_resource_type;
	source += "<";
	source += value_type;
	source += "> g_Destination : register(u0);";
	source += "SamplerState g_Sampler : register(s0);";
	source += "cbuffer MipConstants : register(b0) { uint g_Width; uint g_Height; uint g_Depth; uint g_Layers; };";
	source += "[numthreads(8,8,1)] void main(uint3 id : SV_DispatchThreadID) {";
	if (dimension == RHITextureDimension::Texture2D && !array_view) {
		source += "if (id.x >= g_Width || id.y >= g_Height) return;"
			"float2 uv=(float2(id.xy)+0.5)/float2(g_Width,g_Height);"
			"g_Destination[id.xy]=g_Source.SampleLevel(g_Sampler,uv,0); }";
	} else if (array_view) {
		source += "if (id.x >= g_Width || id.y >= g_Height || id.z >= g_Layers) return;"
			"float3 uv=float3((float2(id.xy)+0.5)/float2(g_Width,g_Height),id.z);"
			"g_Destination[id]=g_Source.SampleLevel(g_Sampler,uv,0); }";
	} else {
		source += "if (id.x >= g_Width || id.y >= g_Height || id.z >= g_Depth) return;"
			"float3 uv=(float3(id)+0.5)/float3(g_Width,g_Height,g_Depth);"
			"g_Destination[id]=g_Source.SampleLevel(g_Sampler,uv,0); }";
	}
	DX12NativeObject<ID3DBlob> shader;
	DX12NativeObject<ID3DBlob> errors;
	const HRESULT compile_result = D3DCompile(source.data(), source.size(), "Graphics.DX12.Mips",
		nullptr, nullptr, "main", "cs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0,
		shader.Put(), errors.Put());
	if (FAILED(compile_result)) {
		if (errors.Get() != nullptr)
			OutputDebugStringA(static_cast<const char *>(errors.Get()->GetBufferPointer()));
		return false;
	}
	D3D12_COMPUTE_PIPELINE_STATE_DESC description{};
	description.pRootSignature = state.mip_root_signature.Get();
	description.CS = {shader.Get()->GetBufferPointer(), shader.Get()->GetBufferSize()};
	const HRESULT create_result = state.device.Get()->CreateComputePipelineState(&description,
		IID_PPV_ARGS(state.mip_pipeline_states[cache_index].Put()));
	if (FAILED(create_result))
		return false;
	*output = state.mip_pipeline_states[cache_index].Get();
	return true;
}

static bool Generate_Raster_Mips(DX12DeviceState &state, DX12Texture &resource) noexcept
{
	const auto &texture = resource.description;
	const bool volume = texture.dimension == RHITextureDimension::Volume;
	const bool array = !volume && texture.array_size > 1;
	const std::uint32_t index = static_cast<std::uint32_t>(texture.format) * 3u
		+ (volume ? 2u : array ? 1u : 0u);
	if (index >= state.raster_mip_pipeline_states.size())
		return false;
	auto &pipeline = state.raster_mip_pipeline_states[index];
	if (pipeline.Get() == nullptr) {
		const char *vertex_source =
			"struct V { float4 p:SV_Position; float2 uv:TEXCOORD0; };"
			"V main(uint id:SV_VertexID) { V v; v.uv=float2((id<<1)&2,id&2);"
			"v.p=float4(v.uv*float2(2,-2)+float2(-1,1),0,1); return v; }";
		std::string pixel_source = volume ? "Texture3D" : array ? "Texture2DArray" : "Texture2D";
		pixel_source += "<float4> src:register(t0); SamplerState smp:register(s0);"
			"cbuffer C:register(b0) { uint slice; uint depth; uint unused0; uint unused1; };"
			"float4 main(float4 p:SV_Position,float2 uv:TEXCOORD0):SV_Target {"
			"return src.SampleLevel(smp,";
		pixel_source += volume ? "float3(uv,(slice+0.5)/depth)" : array ? "float3(uv,slice)" : "uv";
		pixel_source += ",0); }";
		DX12NativeObject<ID3DBlob> vertex, pixel, errors;
		if (FAILED(D3DCompile(vertex_source, std::strlen(vertex_source), "Graphics.DX12.MipVertex",
			nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, vertex.Put(), errors.Put()))
			|| FAILED(D3DCompile(pixel_source.data(), pixel_source.size(), "Graphics.DX12.MipPixel",
				nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, pixel.Put(), errors.Put())))
			return false;
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
		desc.pRootSignature = state.mip_root_signature.Get();
		desc.VS = {vertex.Get()->GetBufferPointer(), vertex.Get()->GetBufferSize()};
		desc.PS = {pixel.Get()->GetBufferPointer(), pixel.Get()->GetBufferSize()};
		desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		desc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		desc.RasterizerState.DepthClipEnable = TRUE;
		desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		desc.DepthStencilState.FrontFace = {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS};
		desc.DepthStencilState.BackFace = desc.DepthStencilState.FrontFace;
		auto &blend = desc.BlendState.RenderTarget[0];
		blend.SrcBlend = blend.SrcBlendAlpha = D3D12_BLEND_ONE;
		blend.DestBlend = blend.DestBlendAlpha = D3D12_BLEND_ZERO;
		blend.BlendOp = blend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		desc.SampleMask = UINT_MAX;
		desc.SampleDesc.Count = 1;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = To_DX12_Format(texture.format);
		if (FAILED(state.device.Get()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(pipeline.Put()))))
			return false;
	}
	if (!state.Ensure_Recording())
		return false;
	std::uint32_t render_offset = 0;
	for (std::uint32_t mip = 1; mip < texture.mip_count; ++mip) {
		if (state.frames[state.current_frame].descriptor_offset + 1 > FrameResourceDescriptors) {
			if (!state.Submit_Current(false) || !state.Ensure_Recording())
				return false;
		}
		const std::uint32_t layers = volume ? 1u : texture.array_size;
		for (std::uint32_t layer = 0; layer < layers; ++layer) {
			state.Transition(resource, Shader_Resource_State(), mip - 1 + layer * texture.mip_count);
			state.Transition(resource, D3D12_RESOURCE_STATE_RENDER_TARGET, mip + layer * texture.mip_count);
		}
		auto &frame = state.frames[state.current_frame];
		const std::uint32_t page = frame.descriptor_base + frame.descriptor_offset++;
		state.device.Get()->CopyDescriptorsSimple(1, state.gpu_resources.Cpu(page),
			state.CpuResource(resource.mip_shader_resource_views[mip - 1].index),
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		auto *commands = state.command_list.Get();
		ID3D12DescriptorHeap *heaps[] = {state.gpu_resources.Get(), state.gpu_samplers.Get()};
		commands->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);
		commands->SetGraphicsRootSignature(state.mip_root_signature.Get());
		commands->SetPipelineState(pipeline.Get());
		commands->SetGraphicsRootDescriptorTable(0, state.gpu_resources.Gpu(page));
		commands->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		const std::uint32_t width = (std::max)(1u, texture.width >> mip);
		const std::uint32_t height = (std::max)(1u, texture.height >> mip);
		const std::uint32_t slices = volume ? (std::max)(1u, texture.depth >> mip) : layers;
		const D3D12_VIEWPORT viewport{0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1};
		const D3D12_RECT scissor{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
		commands->RSSetViewports(1, &viewport);
		commands->RSSetScissorRects(1, &scissor);
		for (std::uint32_t slice = 0; slice < slices; ++slice) {
			const std::uint32_t constants[] = {slice, slices, 0, 0};
			commands->SetGraphicsRoot32BitConstants(2, 4, constants, 0);
			const auto target = state.CpuRTV(resource.mip_render_targets.index + render_offset++);
			commands->OMSetRenderTargets(1, &target, FALSE, nullptr);
			commands->DrawInstanced(3, 1, 0, 0);
		}
		for (std::uint32_t layer = 0; layer < layers; ++layer)
			state.Transition(resource, Shader_Resource_State(), mip + layer * texture.mip_count);
	}
	state.command_list_facade.On_New_Command_List();
	return true;
}

bool DX12Device::Generate_Texture_Mips(RHITextureHandle texture) noexcept
{
	if (!Is_Valid())
		return false;
	DX12Texture *resource = m_state->textures.Resolve(texture);
	if (resource == nullptr || !resource->description.generate_mips
		|| resource->description.mip_count < 2 || resource->mip_shader_resource_views.size()
		!= resource->description.mip_count)
		return false;
	if (!resource->compute_mips)
		return Generate_Raster_Mips(*m_state, *resource);
	ID3D12PipelineState *pipeline = nullptr;
	const bool array_view = resource->description.dimension != RHITextureDimension::Volume
		&& resource->description.array_size > 1;
	if (!Ensure_Mip_Pipeline(*m_state, resource->format, resource->description.dimension,
		array_view, &pipeline)
		|| !m_state->Ensure_Recording())
		return false;
	ID3D12GraphicsCommandList *commands = m_state->command_list.Get();
	ID3D12DescriptorHeap *heaps[] = {m_state->gpu_resources.Get(), m_state->gpu_samplers.Get()};
	commands->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);
	commands->SetComputeRootSignature(m_state->mip_root_signature.Get());
	commands->SetPipelineState(pipeline);
	const bool volume = resource->description.dimension == RHITextureDimension::Volume;
	const std::uint32_t layer_count = volume ? 1u : resource->description.array_size;
	for (std::uint32_t mip = 1; mip < resource->description.mip_count; ++mip) {
		const std::uint32_t source_mip = mip - 1;
		for (std::uint32_t layer = 0; layer < layer_count; ++layer) {
			const std::uint32_t source_subresource = volume ? source_mip
				: source_mip + layer * resource->description.mip_count;
			const std::uint32_t destination_subresource = volume ? mip
				: mip + layer * resource->description.mip_count;
			m_state->Transition(*resource, Shader_Resource_State(), source_subresource);
			m_state->Transition(*resource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, destination_subresource);
		}
		DX12FrameContext &frame = m_state->frames[m_state->current_frame];
		if (frame.descriptor_offset + 2 > FrameResourceDescriptors)
			return false;
		const std::uint32_t page = frame.descriptor_base + frame.descriptor_offset;
		frame.descriptor_offset += 2;
		m_state->device.Get()->CopyDescriptorsSimple(1, m_state->gpu_resources.Cpu(page),
			m_state->CpuResource(resource->mip_shader_resource_views[source_mip].index),
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_state->device.Get()->CopyDescriptorsSimple(1, m_state->gpu_resources.Cpu(page + 1),
			m_state->CpuResource(resource->mip_unordered_access_views[mip].index),
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		commands->SetComputeRootDescriptorTable(0, m_state->gpu_resources.Gpu(page));
		commands->SetComputeRootDescriptorTable(1, m_state->gpu_resources.Gpu(page + 1));
		const std::uint32_t width = (std::max)(1u, resource->description.width >> mip);
		const std::uint32_t height = (std::max)(1u, resource->description.height >> mip);
		const std::uint32_t depth = volume ? (std::max)(1u, resource->description.depth >> mip) : 1u;
		const std::uint32_t constants[] = {width, height, depth, layer_count};
		commands->SetComputeRoot32BitConstants(2, 4, constants, 0);
		commands->Dispatch((width + 7u) / 8u, (height + 7u) / 8u,
			resource->description.dimension == RHITextureDimension::Volume ? depth
				: array_view ? layer_count : 1u);
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = resource->object.Get();
		commands->ResourceBarrier(1, &barrier);
		for (std::uint32_t layer = 0; layer < layer_count; ++layer) {
			const std::uint32_t destination_subresource = volume ? mip
				: mip + layer * resource->description.mip_count;
			m_state->Transition(*resource, Shader_Resource_State(), destination_subresource);
		}
	}
	m_state->command_list_facade.On_New_Command_List();
	return true;
}

RHIPipelineHandle DX12Device::Create_Pipeline(const RHIPipeline &description)
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

RHIPipelineHandle DX12Device::Create_Pipeline(const RHIPipeline &description,
	RHIShaderBytecode vertex_shader, RHIShaderBytecode fragment_shader)
{
	if (!Is_Valid())
		return {};
	DX12Pipeline pipeline;
	struct SamplerReservation final
	{
		DX12DeviceState &state;
		DX12Pipeline &pipeline;
		bool transferred = false;
		~SamplerReservation() {
			if (!transferred) state.Release_Samplers(pipeline.sampler_indices, 0);
		}
	} reservation{*m_state, pipeline};
	if (!Create_DX12_Pipeline(*m_state, description, vertex_shader.data,
		fragment_shader.data, pipeline))
		return {};
	const auto handle = m_state->pipelines.Create(std::move(pipeline));
	reservation.transferred = true;
	return handle;
}

bool DX12Device::Retain_Texture(RHITextureHandle texture) noexcept
{
	if (!Is_Valid())
		return false;
	DX12Texture *resource = m_state->textures.Resolve(texture);
	if (resource == nullptr || resource->references == UINT32_MAX)
		return false;
	++resource->references;
	return true;
}

bool DX12Device::Destroy_Buffer(RHIBufferHandle buffer) noexcept
{
	GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.DestroyBuffer");
	if (m_state == nullptr)
		return false;
	DX12Buffer *resource = m_state->buffers.Resolve(buffer);
	if (resource == nullptr)
		return false;
	const std::uint64_t fence_value = m_state->Retirement_Fence();
	resource->constants.Retire(fence_value);
	if (resource->object.Get() != nullptr
		&& !m_state->Defer(static_cast<IUnknown *>(resource->object.Get()), fence_value))
		return false;
	if (DX12BufferCache::Eligible(resource->usage, resource->capacity))
		m_state->buffer_cache.Recycle(*resource, fence_value);
	m_state->command_list_facade.Release_Buffer_Bindings(buffer);
	return m_state->buffers.Destroy(buffer);
}

bool DX12Device::Destroy_Texture(RHITextureHandle texture) noexcept
{
	GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.DestroyTexture");
	if (m_state == nullptr)
		return false;
	DX12Texture *resource = m_state->textures.Resolve(texture);
	if (resource == nullptr)
		return false;
	if (resource->references > 1) {
		--resource->references;
		return true;
	}
	const std::uint64_t fence_value = m_state->Retirement_Fence();
	if (!m_state->Defer(static_cast<IUnknown *>(resource->object.Get()), fence_value))
		return false;
	for (const auto &mapping : resource->mappings) {
		if (!m_state->Defer(static_cast<IUnknown *>(mapping->readback.Get()), fence_value)
			|| !m_state->Defer(static_cast<IUnknown *>(mapping->upload.Get()), fence_value))
			return false;
	}
	m_state->command_list_facade.Release_Texture_Bindings(texture);
	return m_state->textures.Destroy(texture);
}

bool DX12Device::Destroy_Pipeline(RHIPipelineHandle pipeline) noexcept
{
	GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.DestroyPipeline");
	if (m_state == nullptr)
		return false;
	DX12Pipeline *resource = m_state->pipelines.Resolve(pipeline);
	if (resource == nullptr)
		return false;
	const std::uint64_t fence_value = m_state->Retirement_Fence();
	for (auto &native : resource->pipeline_states)
		if (!m_state->Defer(static_cast<IUnknown *>(native.Get()), fence_value))
			return false;
	m_state->command_list_facade.Release_Pipeline_Binding(pipeline);
	m_state->Release_Samplers(resource->sampler_indices, fence_value);
	return m_state->pipelines.Destroy(pipeline);
}

CommandList &DX12Device::Immediate_Command_List() noexcept
{
	return m_state->command_list_facade;
}

SwapChain &DX12Device::Get_Swap_Chain() noexcept
{
	return m_state->swap_chain;
}

bool DX12Device::Set_Exclusive_Fullscreen(bool fullscreen) noexcept
{
	return Is_Valid() && m_state->native_swap_chain.Get() != nullptr && !m_state->frame_active
		&& SUCCEEDED(m_state->native_swap_chain.Get()->SetFullscreenState(fullscreen ? TRUE : FALSE, nullptr));
}

bool DX12Device::Begin_Frame() noexcept
{
	if (!Is_Valid() || m_state->frame_active || !m_state->swap_chain.Is_Valid())
		return false;
	m_state->command_list_facade.Reset_Frame_State();
	if (!m_state->Ensure_Recording())
		return false;
	const RHIBackbuffer backbuffer = m_state->swap_chain.Backbuffer();
	const RHIDepthTarget depth = m_state->swap_chain.Depth_Target();
	if (!m_state->command_list_facade.Set_Render_Targets(backbuffer.texture, depth.texture))
		return false;
	m_state->frame_active = true;
	m_state->ready_to_present = false;
	m_state->presented = false;
	return true;
}

bool DX12Device::End_Frame() noexcept
{
	if (!Is_Valid() || !m_state->frame_active)
		return false;
	const RHIBackbuffer backbuffer = m_state->swap_chain.Backbuffer();
	DX12Texture *target = m_state->textures.Resolve(backbuffer.texture);
	if (target == nullptr || !m_state->Transition(*target, D3D12_RESOURCE_STATE_PRESENT))
		return false;
	if (m_state->Submit_Current(false) == 0)
		return false;
	m_state->frame_active = false;
	m_state->ready_to_present = true;
	m_state->presented = false;
	return true;
}

bool DX12DeviceState::Defer(IUnknown *object, std::uint64_t fence_value) noexcept
{
	if (object == nullptr)
		return true;
	try {
		if (deferred.empty() || deferred.back().fence != fence_value)
			deferred.push_back({fence_value, {}});
		deferred.back().objects.emplace_back(object);
		return true;
	} catch (...) {
		// The caller keeps ownership when this fails, so it must not release the
		// native object until it can be recorded for deferred release.  Waiting
		// here can block forever when fence_value belongs to an unsubmitted list.
		return false;
	}
}

bool DX12DeviceState::Defer_Buffer(DX12Buffer &resource, std::uint64_t fence_value) noexcept
{
	return Defer(static_cast<IUnknown *>(resource.object.Get()), fence_value);
}

std::uint32_t DX12DeviceState::Acquire_Sampler(const D3D12_SAMPLER_DESC &description)
{
	std::uint32_t available = InvalidDescriptor;
	for (std::uint32_t index = 0; index < sampler_slots.size(); ++index) {
		auto &slot = sampler_slots[index];
		if (std::memcmp(&slot.description, &description, sizeof(description)) == 0) {
			if (index == 0) return 0;
			if (slot.references == UINT32_MAX) return InvalidDescriptor;
			++slot.references;
			return index;
		}
		if (index != 0 && available == InvalidDescriptor && slot.references == 0
			&& slot.retirement_fence <= completed_fence) available = index;
	}
	if (available == InvalidDescriptor) {
		if (sampler_slots.size() == PersistentSamplerCount) return InvalidDescriptor;
		available = static_cast<std::uint32_t>(sampler_slots.size());
		sampler_slots.push_back({});
	}
	// Pending draws retain the old descriptor until their submission completes.
	sampler_slots[available] = {description, 1, 0};
	device.Get()->CreateSampler(&description, gpu_samplers.Cpu(available));
	return available;
}

void DX12DeviceState::Release_Samplers(std::span<const std::uint32_t> indices,
	std::uint64_t fence_value) noexcept
{
	for (const auto index : indices) {
		if (index == 0) continue; // The default sampler belongs to the device.
		auto &slot = sampler_slots[index];
		assert(slot.references != 0);
		--slot.references;
		slot.retirement_fence = (std::max)(slot.retirement_fence, fence_value);
	}
}

DX12UploadSlice DX12DeviceState::Allocate_Transient_Upload(std::uint64_t size,
	std::uint64_t alignment) noexcept
{
	if (device.Get() == nullptr || size == 0)
		return {};
	const std::uint64_t aligned_size = (size + alignment - 1u) & ~(alignment - 1u);
	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC description{};
	description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	description.Width = aligned_size;
	description.Height = 1;
	description.DepthOrArraySize = 1;
	description.MipLevels = 1;
	description.Format = DXGI_FORMAT_UNKNOWN;
	description.SampleDesc.Count = 1;
	description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	DX12NativeObject<ID3D12Resource> upload;
	const HRESULT result = device.Get()->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
		&description, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(upload.Put()));
	if (FAILED(result)) {
		Report_HResult("CreateCommittedResource(upload)", result);
		return {};
	}
	std::byte *mapped = nullptr;
	if (FAILED(upload.Get()->Map(0, nullptr, reinterpret_cast<void **>(&mapped))))
		return {};
	try {
		const auto resource = upload.Get();
		const auto gpu_address = resource->GetGPUVirtualAddress();
		const auto offset = static_cast<std::uint64_t>(0);
		const auto result_slice = DX12UploadSlice{resource, mapped, offset, gpu_address, size};
		if (current_frame >= frames.size()) {
			upload.Get()->Unmap(0, nullptr);
			return {};
		}
		frames[current_frame].transient_uploads.push_back(std::move(upload));
		return result_slice;
	} catch (...) {
		upload.Get()->Unmap(0, nullptr);
		return {};
	}
}

DX12UploadSlice DX12DeviceState::Allocate_Upload(std::uint64_t size,
	std::uint64_t alignment) noexcept
{
	if (!Ensure_Recording())
		return {};
	DX12FrameContext &frame = frames[current_frame];
	if (auto slice = frame.uploads.Allocate(size, alignment); slice.resource != nullptr)
		return slice;
	return Allocate_Transient_Upload(size, alignment);
}

bool DX12DeviceState::Ensure_Recording() noexcept
{
	if (recording)
		return true;
	if (device.Get() == nullptr || command_list.Get() == nullptr)
		return false;
	const std::uint32_t next_frame = (current_frame + 1u) % FrameCount;
	DX12FrameContext &frame = frames[next_frame];
	if (frame.fence != 0 && !Wait_For_Fence(frame.fence))
		return false;
	if (FAILED(frame.allocator.Get()->Reset()))
		return false;
	if (FAILED(command_list.Get()->Reset(frame.allocator.Get(), nullptr)))
		return false;
	current_frame = next_frame;
	frame.uploads.Reset();
	frame.descriptor_offset = 0;
	for (auto &upload : frame.transient_uploads)
		if (upload.Get() != nullptr)
			upload.Get()->Unmap(0, nullptr);
	frame.transient_uploads.clear();
	recording = true;
	command_list_facade.On_New_Command_List();
	Collect_Deferred();
	return true;
}

std::uint64_t DX12DeviceState::Submit_Current(bool wait) noexcept
{
	GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.Submit");
	if (!recording)
		return last_submitted_fence;
	const HRESULT close_result = command_list.Get()->Close();
	if (FAILED(close_result)) {
		Report_HResult("ID3D12GraphicsCommandList::Close", close_result);
		recording = false;
		return 0;
	}
	ID3D12CommandList *lists[] = {command_list.Get()};
	queue.Get()->ExecuteCommandLists(1, lists);
	const std::uint64_t signal = ++next_fence;
	const HRESULT signal_result = queue.Get()->Signal(fence.Get(), signal);
	if (FAILED(signal_result)) {
		Report_HResult("ID3D12CommandQueue::Signal", signal_result);
		recording = false;
		return 0;
	}
	frames[current_frame].fence = signal;
	last_submitted_fence = signal;
	recording = false;
	if (wait && !Wait_For_Fence(signal))
		return 0;
	Collect_Deferred();
	return signal;
}

bool DX12DeviceState::Transition(DX12Texture &texture, D3D12_RESOURCE_STATES desired,
	std::uint32_t subresource) noexcept
{
	if (texture.object.Get() == nullptr || !Ensure_Recording())
		return false;
	const auto apply = [&](std::uint32_t index) {
		if (index >= texture.states.size() || texture.states[index] == desired)
			return;
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = texture.object.Get();
		barrier.Transition.StateBefore = texture.states[index];
		barrier.Transition.StateAfter = desired;
		barrier.Transition.Subresource = index;
		command_list.Get()->ResourceBarrier(1, &barrier);
		texture.states[index] = desired;
	};
	if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
		bool different = false;
		for (const auto state : texture.states)
			different |= state != desired;
		if (!different)
			return true;
		const auto first_state = texture.states.empty() ? D3D12_RESOURCE_STATE_COMMON : texture.states.front();
		bool uniform = true;
		for (const auto state : texture.states)
			uniform &= state == first_state;
		if (!uniform) {
			for (std::uint32_t index = 0; index < texture.states.size(); ++index)
				apply(index);
			return true;
		}
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = texture.object.Get();
		barrier.Transition.StateBefore = first_state;
		barrier.Transition.StateAfter = desired;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		command_list.Get()->ResourceBarrier(1, &barrier);
		for (auto &state : texture.states)
			state = desired;
		return true;
	}
	apply(subresource);
	return subresource < texture.states.size();
}

static bool Allocate_CPU_Descriptors(DX12DescriptorHeap &heap, std::uint32_t &offset,
	std::uint32_t count, DX12DescriptorRange &range) noexcept
{
	if (count == 0 || offset > heap.Capacity() || count > heap.Capacity() - offset)
		return false;
	range = {offset, count};
	offset += count;
	return true;
}

static bool Create_Upload_Resource(ID3D12Device *device, std::uint64_t size,
	DX12UploadArena &arena) noexcept
{
	if (device == nullptr || size == 0)
		return false;
	D3D12_HEAP_PROPERTIES heap{};
	heap.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC description{};
	description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	description.Width = size;
	description.Height = 1;
	description.DepthOrArraySize = 1;
	description.MipLevels = 1;
	description.SampleDesc.Count = 1;
	description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	const HRESULT result = device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE,
		&description, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(arena.resource.Put()));
	if (FAILED(result)) {
		Report_HResult("CreateCommittedResource(frame upload)", result);
		return false;
	}
	if (FAILED(arena.resource.Get()->Map(0, nullptr, reinterpret_cast<void **>(&arena.mapped))))
		return false;
	arena.capacity = size;
	arena.offset = 0;
	return true;
}

static bool Create_DX12_Root_Signatures(DX12DeviceState &state) noexcept
{
	if (state.device.Get() == nullptr)
		return false;
	D3D12_DESCRIPTOR_RANGE cbv_range{};
	cbv_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	cbv_range.NumDescriptors = BindlessCBVCount - RootCBVCount;
	cbv_range.BaseShaderRegister = RootCBVCount;
	cbv_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	D3D12_ROOT_PARAMETER parameters[RootCBVCount + 4]{};
	for (std::uint32_t stage = 0; stage < 2; ++stage) {
		parameters[stage].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		parameters[stage].ShaderVisibility = stage == 0
			? D3D12_SHADER_VISIBILITY_VERTEX : D3D12_SHADER_VISIBILITY_PIXEL;
		parameters[stage].Descriptor.ShaderRegister = 0;
		parameters[stage].Descriptor.RegisterSpace = 1;
	}
	parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[2].DescriptorTable.NumDescriptorRanges = 1;
	parameters[2].DescriptorTable.pDescriptorRanges = &cbv_range;
	parameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	parameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	parameters[3].Constants.ShaderRegister = 1;
	parameters[3].Constants.RegisterSpace = 1;
	parameters[3].Constants.Num32BitValues = 16;
	for (std::uint32_t slot = 0; slot < RootCBVCount; ++slot) {
		auto &parameter = parameters[Constant_Root_Parameter(slot)];
		parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		parameter.Descriptor.ShaderRegister = slot;
	}
	D3D12_ROOT_SIGNATURE_DESC description{};
	description.NumParameters = static_cast<UINT>(std::size(parameters));
	description.pParameters = parameters;
	description.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		| D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED
		| D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
	DX12NativeObject<ID3DBlob> serialized;
	DX12NativeObject<ID3DBlob> errors;
	HRESULT result = D3D12SerializeRootSignature(&description, D3D_ROOT_SIGNATURE_VERSION_1,
		serialized.Put(), errors.Put());
	if (FAILED(result)) {
		if (errors.Get() != nullptr)
			OutputDebugStringA(static_cast<const char *>(errors.Get()->GetBufferPointer()));
		Report_HResult("D3D12SerializeRootSignature", result);
		return false;
	}
	result = state.device.Get()->CreateRootSignature(0, serialized.Get()->GetBufferPointer(),
		serialized.Get()->GetBufferSize(), IID_PPV_ARGS(state.graphics_root_signature.Put()));
	if (FAILED(result)) {
		Report_HResult("CreateRootSignature", result);
		return false;
	}
	return true;
}

static bool Supports_DX12_Shader_Model_6_6(ID3D12Device *device) noexcept
{
	if (device == nullptr)
		return false;
	D3D12_FEATURE_DATA_SHADER_MODEL shader_model{D3D_SHADER_MODEL_6_6};
	const HRESULT result = device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL,
		&shader_model, sizeof(shader_model));
	if (FAILED(result)) {
		Report_HResult("CheckFeatureSupport(shader model 6.6)", result);
		return false;
	}
	if (shader_model.HighestShaderModel < D3D_SHADER_MODEL_6_6) {
		OutputDebugStringA("Graphics.DX12: shader model 6.6 is required by the production DXIL shaders\n");
		return false;
	}
	return true;
}

static bool Create_DX12_Device(DX12DeviceState &state, const DX12DeviceOptions &options) noexcept
{
	if (options.backbuffer_format != RHITextureFormat::RGBA8_UNorm
		&& options.backbuffer_format != RHITextureFormat::BGRA8_UNorm)
		return false;
	UINT factory_flags = 0;
	char debug_environment[8]{};
	const DWORD debug_length = GetEnvironmentVariableA("GRAPHICS_DX12_DEBUG", debug_environment,
		static_cast<DWORD>(std::size(debug_environment)));
	state.debug_layer = options.enable_debug_layer || (debug_length != 0 && debug_environment[0] != '0');
	if (state.debug_layer) {
		DX12NativeObject<ID3D12Debug> debug;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debug.Put()))))
			debug.Get()->EnableDebugLayer();
		factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
	}
	HRESULT result = CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(state.factory.Put()));
	if (FAILED(result)) {
		Report_HResult("CreateDXGIFactory2", result);
		return false;
	}
	BOOL allow_tearing = FALSE;
	state.allow_tearing = SUCCEEDED(state.factory.Get()->CheckFeatureSupport(
		DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing, sizeof(allow_tearing))) && allow_tearing;
	if (options.use_warp) {
		result = state.factory.Get()->EnumWarpAdapter(IID_PPV_ARGS(state.adapter.Put()));
	} else {
		for (UINT index = 0; ; ++index) {
			DX12NativeObject<IDXGIAdapter1> candidate;
			result = state.factory.Get()->EnumAdapterByGpuPreference(index,
				DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(candidate.Put()));
			if (result == DXGI_ERROR_NOT_FOUND)
				break;
			if (FAILED(result) || candidate.Get() == nullptr) {
				Report_HResult("EnumAdapterByGpuPreference", result);
				break;
			}
			DXGI_ADAPTER_DESC1 adapter_description{};
			result = candidate.Get()->GetDesc1(&adapter_description);
			if (FAILED(result)) {
				Report_HResult("IDXGIAdapter1::GetDesc1", result);
				break;
			}
			if ((adapter_description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
				continue;
			DX12NativeObject<ID3D12Device> probe;
			if (SUCCEEDED(D3D12CreateDevice(candidate.Get(), D3D_FEATURE_LEVEL_11_0,
				IID_PPV_ARGS(probe.Put()))) && Supports_DX12_Shader_Model_6_6(probe.Get())) {
				state.adapter = std::move(candidate);
				break;
			}
		}
		result = state.adapter.Get() != nullptr ? S_OK : DXGI_ERROR_NOT_FOUND;
	}
	if (FAILED(result) || state.adapter.Get() == nullptr) {
		Report_HResult("SelectAdapter", result);
		return false;
	}
	result = D3D12CreateDevice(state.adapter.Get(), D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(state.device.Put()));
	if (FAILED(result)) {
		Report_HResult("D3D12CreateDevice", result);
		return false;
	}
	Register_DX12_Debug_Messages(state);
	state.constant_memory.Initialize(state.device.Get());
	if (!Supports_DX12_Shader_Model_6_6(state.device.Get()))
		return false;
	D3D12_FEATURE_DATA_D3D12_OPTIONS binding_options{};
	if (FAILED(state.device.Get()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS,
		&binding_options, sizeof(binding_options)))
		|| binding_options.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3)
		return false;
	D3D12_COMMAND_QUEUE_DESC queue_description{};
	queue_description.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = state.device.Get()->CreateCommandQueue(&queue_description, IID_PPV_ARGS(state.queue.Put()));
	if (FAILED(result)) {
		Report_HResult("CreateCommandQueue", result);
		return false;
	}
	result = state.device.Get()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(state.fence.Put()));
	if (FAILED(result)) {
		Report_HResult("CreateFence", result);
		return false;
	}
	state.fence_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
	if (state.fence_event == nullptr)
		return false;
	for (auto &frame : state.frames) {
		if (FAILED(state.device.Get()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(frame.allocator.Put()))))
			return false;
	}
	result = state.device.Get()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		state.frames[0].allocator.Get(), nullptr, IID_PPV_ARGS(state.command_list.Put()));
	if (FAILED(result)) {
		Report_HResult("CreateCommandList", result);
		return false;
	}
	const HRESULT close_result = state.command_list.Get()->Close();
	if (FAILED(close_result)) {
		Report_HResult("ID3D12GraphicsCommandList::Close", close_result);
		return false;
	}
	if (!Create_DX12_Root_Signatures(state))
		return false;
	{
		D3D12_DESCRIPTOR_RANGE srv{};
		srv.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srv.NumDescriptors = 1;
		srv.BaseShaderRegister = 0;
		srv.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		D3D12_DESCRIPTOR_RANGE uav{};
		uav.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		uav.NumDescriptors = 1;
		uav.BaseShaderRegister = 0;
		uav.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		D3D12_ROOT_PARAMETER parameters[3]{};
		parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		parameters[0].DescriptorTable.NumDescriptorRanges = 1;
		parameters[0].DescriptorTable.pDescriptorRanges = &srv;
		parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		parameters[1].DescriptorTable.NumDescriptorRanges = 1;
		parameters[1].DescriptorTable.pDescriptorRanges = &uav;
		parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		parameters[2].Constants.ShaderRegister = 0;
		parameters[2].Constants.Num32BitValues = 4;
		D3D12_STATIC_SAMPLER_DESC sampler{};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		D3D12_ROOT_SIGNATURE_DESC description{};
		description.NumParameters = 3;
		description.pParameters = parameters;
		description.NumStaticSamplers = 1;
		description.pStaticSamplers = &sampler;
		DX12NativeObject<ID3DBlob> serialized;
		DX12NativeObject<ID3DBlob> errors;
		result = D3D12SerializeRootSignature(&description, D3D_ROOT_SIGNATURE_VERSION_1,
			serialized.Put(), errors.Put());
		if (FAILED(result)) {
			if (errors.Get() != nullptr)
				OutputDebugStringA(static_cast<const char *>(errors.Get()->GetBufferPointer()));
			return false;
		}
		result = state.device.Get()->CreateRootSignature(0, serialized.Get()->GetBufferPointer(),
			serialized.Get()->GetBufferSize(), IID_PPV_ARGS(state.mip_root_signature.Put()));
		if (FAILED(result))
			return false;
	}
	if (!state.cpu_resources.Create(state.device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		262144, false)
		|| !state.cpu_render_targets.Create(state.device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		65536, false)
		|| !state.cpu_depth_targets.Create(state.device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
		65536, false)
		|| !state.gpu_resources.Create(state.device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		PersistentDescriptorBase + PersistentDescriptorCount, true)
		|| !state.gpu_samplers.Create(state.device.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		PersistentSamplerCount, true))
		return false;
	DX12DescriptorRange null_srvs;
	DX12DescriptorRange null_cbvs;
	if (!Allocate_CPU_Descriptors(state.cpu_resources, state.resource_descriptor_offset,
		BindlessSRVCount, null_srvs)
		|| !Allocate_CPU_Descriptors(state.cpu_resources, state.resource_descriptor_offset,
		BindlessCBVCount, null_cbvs))
		return false;
	state.null_srv_base = null_srvs.index;
	state.null_cbv_base = null_cbvs.index;
	D3D12_SHADER_RESOURCE_VIEW_DESC null_srv{};
	null_srv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	null_srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	null_srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	null_srv.Texture2D.MipLevels = 1;
	for (std::uint32_t index = 0; index < BindlessSRVCount; ++index)
		state.device.Get()->CreateShaderResourceView(nullptr, &null_srv, state.CpuResource(null_srvs.index + index));
	D3D12_HEAP_PROPERTIES null_heap{};
	null_heap.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC null_buffer_description{};
	null_buffer_description.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	null_buffer_description.Width = D3D12_REQ_CONSTANT_BUFFER_ELEMENT_COUNT * 16u;
	null_buffer_description.Height = 1;
	null_buffer_description.DepthOrArraySize = 1;
	null_buffer_description.MipLevels = 1;
	null_buffer_description.SampleDesc.Count = 1;
	null_buffer_description.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	if (FAILED(state.device.Get()->CreateCommittedResource(&null_heap, D3D12_HEAP_FLAG_NONE,
		&null_buffer_description, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(state.null_constant_buffer.Put()))))
		return false;
	void *null_bytes = nullptr;
	const D3D12_RANGE no_reads{0, 0};
	if (FAILED(state.null_constant_buffer.Get()->Map(0, &no_reads, &null_bytes)))
		return false;
	std::memset(null_bytes, 0, static_cast<std::size_t>(null_buffer_description.Width));
	state.null_constant_buffer.Get()->Unmap(0, nullptr);
	D3D12_CONSTANT_BUFFER_VIEW_DESC null_cbv{};
	null_cbv.BufferLocation = state.null_constant_buffer.Get()->GetGPUVirtualAddress();
	null_cbv.SizeInBytes = static_cast<UINT>(null_buffer_description.Width);
	for (std::uint32_t index = 0; index < BindlessCBVCount; ++index)
		state.device.Get()->CreateConstantBufferView(&null_cbv, state.CpuResource(null_cbvs.index + index));
	Publish_Shader_Resource(state, null_srvs);
	Publish_Shader_Resource(state, null_cbvs);
	D3D12_SAMPLER_DESC null_sampler{};
	null_sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	null_sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	null_sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	null_sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	null_sampler.MaxLOD = D3D12_FLOAT32_MAX;
	state.sampler_slots.reserve(PersistentSamplerCount);
	state.sampler_slots.push_back({null_sampler, 1, 0});
	state.device.Get()->CreateSampler(&null_sampler, state.gpu_samplers.Cpu(0));
	for (std::uint32_t index = 0; index < FrameCount; ++index) {
		DX12FrameContext &frame = state.frames[index];
		frame.descriptor_base = index * FrameResourceDescriptors;
		if (!Create_Upload_Resource(state.device.Get(), 64u * 1024u * 1024u, frame.uploads))
			return false;
	}
	return true;
}

static std::uint32_t Pipeline_Color_Variant(DXGI_FORMAT format) noexcept
{
	switch (format) {
	case DXGI_FORMAT_R8G8B8A8_UNORM: return 0;
	case DXGI_FORMAT_B8G8R8A8_UNORM: return 1;
	case DXGI_FORMAT_R16G16B16A16_FLOAT: return 2;
	case DXGI_FORMAT_R32_FLOAT: return 3;
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return 4;
	case DXGI_FORMAT_UNKNOWN: return 5;
	default: return 0;
	}
}

static std::uint32_t Pipeline_Depth_Variant(DXGI_FORMAT format) noexcept
{
	switch (format) {
	case DXGI_FORMAT_D24_UNORM_S8_UINT: return 1;
	case DXGI_FORMAT_D32_FLOAT: return 2;
	case DXGI_FORMAT_D16_UNORM: return 3;
	default: return 0;
	}
}

static D3D12_INPUT_ELEMENT_DESC Make_Input_Element(const char *semantic,
	UINT semantic_index, DXGI_FORMAT format, UINT offset) noexcept
{
	D3D12_INPUT_ELEMENT_DESC element{};
	element.SemanticName = semantic;
	element.SemanticIndex = semantic_index;
	element.Format = format;
	element.InputSlot = 0;
	element.AlignedByteOffset = offset;
	element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
	return element;
}

static bool Create_DX12_Pipeline(DX12DeviceState &state, const RHIPipeline &description,
	std::span<const std::byte> vertex_bytecode, std::span<const std::byte> pixel_bytecode,
	DX12Pipeline &pipeline) noexcept
{
	GRAPHICS_PROFILE_FOCUS_SCOPE("Graphics.DX12.CreatePipeline");
	if (state.device.Get() == nullptr || state.graphics_root_signature.Get() == nullptr
		|| vertex_bytecode.empty() || pixel_bytecode.empty()
		|| description.sampler_count == 0 || description.sampler_count > description.samplers.size()
		|| description.vertex_element_count > description.vertex_elements.size())
		return false;
	std::array<D3D12_INPUT_ELEMENT_DESC, 16> input_elements{};
	UINT input_count = 0;
	if (description.vertex_element_count != 0) {
		constexpr const char *semantics[] = {"POSITION", "COLOR", "NORMAL", "TEXCOORD"};
		constexpr DXGI_FORMAT formats[] = {DXGI_FORMAT_R32G32_FLOAT,
			DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,DXGI_FORMAT_R32_FLOAT};
		for (std::uint32_t index = 0; index < description.vertex_element_count; ++index) {
			const auto &source = description.vertex_elements[index];
			const auto semantic = static_cast<std::uint32_t>(source.semantic);
			const auto format = static_cast<std::uint32_t>(source.format);
			if (semantic >= std::size(semantics) || format >= std::size(formats))
				return false;
			input_elements[index] = Make_Input_Element(semantics[semantic], source.semantic_index,
				formats[format], source.offset);
		}
		input_count = description.vertex_element_count;
	} else {
		input_elements[0] = Make_Input_Element("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0);
		input_elements[1] = Make_Input_Element("COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 12);
		input_elements[2] = Make_Input_Element("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 28);
		if (description.vertex_format == RHIVertexFormat::Position3Color4UV2) {
			input_count = 3;
		} else if (description.vertex_format == RHIVertexFormat::Position3Color4UV2ResourceIndex) {
			input_elements[3] = Make_Input_Element("TEXCOORD", 1, DXGI_FORMAT_R32_UINT, 36);
			input_count = 4;
		} else if (description.vertex_format == RHIVertexFormat::Position3Color4UV2Skinned) {
			input_elements[2] = Make_Input_Element("TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 28);
			input_elements[3] = Make_Input_Element("BLENDINDICES", 0, DXGI_FORMAT_R16G16B16A16_UINT, 36);
			input_elements[4] = Make_Input_Element("BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 44);
			input_count = 5;
		} else if (description.vertex_format == RHIVertexFormat::Position3Color4UV2UV2Normal3) {
			input_elements[3] = Make_Input_Element("TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 36);
			input_elements[4] = Make_Input_Element("NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 44);
			input_count = 5;
		} else {
			return false;
		}
	}
	state.Collect_Deferred();
	for (std::uint32_t index = 0; index < description.sampler_count; ++index) {
		const auto &sampler = description.samplers[index];
		D3D12_SAMPLER_DESC native{};
		native.Filter = To_DX12_Filter(sampler);
		native.AddressU = sampler.address[0] == RHISamplerAddress::Clamp
			? D3D12_TEXTURE_ADDRESS_MODE_CLAMP : D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		native.AddressV = sampler.address[1] == RHISamplerAddress::Clamp
			? D3D12_TEXTURE_ADDRESS_MODE_CLAMP : D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		native.AddressW = sampler.address[2] == RHISamplerAddress::Clamp
			? D3D12_TEXTURE_ADDRESS_MODE_CLAMP : D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		native.MaxAnisotropy = (std::min)(static_cast<UINT>(sampler.anisotropy), 16u);
		native.MinLOD = sampler.min_lod;
		native.MaxLOD = sampler.max_lod;
		const auto sampler_index = state.Acquire_Sampler(native);
		if (sampler_index == InvalidDescriptor) return false;
		pipeline.sampler_indices[index] = sampler_index;
	}
	pipeline.stencil_reference = description.stencil.reference;
	pipeline.key = description.key;
	pipeline.topology = description.topology;
	pipeline.scissor_test = description.scissor_test;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC native{};
	native.pRootSignature = state.graphics_root_signature.Get();
	native.VS = {vertex_bytecode.data(), vertex_bytecode.size()};
	native.PS = {pixel_bytecode.data(), pixel_bytecode.size()};
	native.InputLayout = {input_elements.data(), input_count};
	native.PrimitiveTopologyType = To_DX12_Topology_Type(description.topology);
	native.SampleDesc.Count = 1;
	native.SampleMask = UINT_MAX;
	native.NumRenderTargets = 1;
	native.RasterizerState.FillMode = description.wireframe ? D3D12_FILL_MODE_WIREFRAME : D3D12_FILL_MODE_SOLID;
	native.RasterizerState.CullMode = To_DX12_Cull(description.cull_mode);
	native.RasterizerState.FrontCounterClockwise = description.front_counter_clockwise ? TRUE : FALSE;
	native.RasterizerState.DepthClipEnable = TRUE;
	native.RasterizerState.DepthBias = description.depth_bias;
	native.DepthStencilState.DepthEnable = description.depth_test ? TRUE : FALSE;
	native.DepthStencilState.DepthWriteMask = description.depth_write
		? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	native.DepthStencilState.DepthFunc = To_DX12_Comparison(description.depth_comparison);
	native.DepthStencilState.StencilEnable = description.stencil.enabled ? TRUE : FALSE;
	native.DepthStencilState.StencilReadMask = description.stencil.read_mask;
	native.DepthStencilState.StencilWriteMask = description.stencil.write_mask;
	const auto make_stencil = [](const RHIStencilFace &face) {
		D3D12_DEPTH_STENCILOP_DESC result{};
		result.StencilFunc = To_DX12_Comparison(face.comparison);
		result.StencilFailOp = To_DX12_Stencil(face.fail);
		result.StencilDepthFailOp = To_DX12_Stencil(face.depth_fail);
		result.StencilPassOp = To_DX12_Stencil(face.pass);
		return result;
	};
	native.DepthStencilState.FrontFace = make_stencil(description.stencil.front);
	native.DepthStencilState.BackFace = make_stencil(description.stencil.back);
	D3D12_RENDER_TARGET_BLEND_DESC &blend = native.BlendState.RenderTarget[0];
	blend.BlendEnable = description.blend_mode == RHIBlendMode::Disabled ? FALSE : TRUE;
	switch (description.blend_mode) {
	case RHIBlendMode::Disabled:
	case RHIBlendMode::Additive:
		blend.SrcBlend = D3D12_BLEND_ONE;
		blend.DestBlend = description.blend_mode == RHIBlendMode::Additive ? D3D12_BLEND_ONE : D3D12_BLEND_ZERO;
		break;
	case RHIBlendMode::Alpha:
		blend.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blend.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		break;
	case RHIBlendMode::Multiply:
		blend.SrcBlend = D3D12_BLEND_ZERO;
		blend.DestBlend = D3D12_BLEND_SRC_COLOR;
		break;
	case RHIBlendMode::ColorMultiply:
		blend.SrcBlend = D3D12_BLEND_DEST_COLOR;
		blend.DestBlend = D3D12_BLEND_SRC_COLOR;
		break;
	}
	if (description.custom_blend_factors) {
		blend.SrcBlend = To_DX12_Blend(description.source_blend);
		blend.DestBlend = To_DX12_Blend(description.destination_blend);
	}
	blend.BlendOp = To_DX12_Blend_Operation(description.blend_operation);
	blend.SrcBlendAlpha = D3D12_BLEND_ONE;
	blend.DestBlendAlpha = description.blend_mode == RHIBlendMode::Alpha
		? D3D12_BLEND_INV_SRC_ALPHA : D3D12_BLEND_ZERO;
	if (description.blend_alpha_like_color) {
		const auto alpha_factor = [](D3D12_BLEND factor) {
			if (factor == D3D12_BLEND_DEST_COLOR) return D3D12_BLEND_DEST_ALPHA;
			if (factor == D3D12_BLEND_SRC_COLOR) return D3D12_BLEND_SRC_ALPHA;
			if (factor == D3D12_BLEND_INV_SRC_COLOR) return D3D12_BLEND_INV_SRC_ALPHA;
			if (factor == D3D12_BLEND_INV_DEST_COLOR) return D3D12_BLEND_INV_DEST_ALPHA;
			return factor;
		};
		blend.SrcBlendAlpha = alpha_factor(blend.SrcBlend);
		blend.DestBlendAlpha = alpha_factor(blend.DestBlend);
	}
	blend.BlendOpAlpha = To_DX12_Blend_Operation(description.blend_operation);
	blend.RenderTargetWriteMask = description.color_write_mask & D3D12_COLOR_WRITE_ENABLE_ALL;
	constexpr std::array<DXGI_FORMAT, 6> colors = {
		DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM,
		DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_UNKNOWN};
	constexpr std::array<DXGI_FORMAT, 4> depths = {
		DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT,
		DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_D16_UNORM};
	for (std::uint32_t depth = 0; depth < depths.size(); ++depth) {
		for (std::uint32_t color = 0; color < colors.size(); ++color) {
			D3D12_GRAPHICS_PIPELINE_STATE_DESC variant = native;
			variant.NumRenderTargets = color == colors.size() - 1u ? 0u : 1u;
			variant.RTVFormats[0] = colors[color];
			variant.DSVFormat = depths[depth];
			if (depth == 0) {
				variant.DepthStencilState.DepthEnable = FALSE;
				variant.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
				variant.DepthStencilState.StencilEnable = FALSE;
			}
			const std::uint32_t index = depth * colors.size() + color;
			const HRESULT result = state.device.Get()->CreateGraphicsPipelineState(&variant,
				IID_PPV_ARGS(pipeline.pipeline_states[index].Put()));
			if (FAILED(result)) {
				Report_HResult("CreateGraphicsPipelineState", result);
				return false;
			}
		}
	}
	return true;
}

} // namespace Graphics
