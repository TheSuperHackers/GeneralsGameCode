module;

#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif

export module Graphics.Scene.Beams;

export import Graphics.Resources.Bindless.BindlessResourceTable;
export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Resources.Materials.Material;
export import Graphics.Resources.Residency.GPUResourceResidency;
export import Graphics.RenderGraph.Execution;
export import Graphics.Shaders.Library;

import Graphics.Memory.AlignedAllocator;

namespace Graphics
{

export inline constexpr std::uint32_t Invalid_Beam_Index = std::numeric_limits<std::uint32_t>::max();

export enum class BeamFlags : std::uint32_t
{
	None = 0,
	Enabled = 1u << 0,
	AlphaTest = 1u << 1,
	Additive = 1u << 2,
	Multiply = 1u << 3
};

export constexpr BeamFlags operator|(BeamFlags left, BeamFlags right) noexcept
{
	return static_cast<BeamFlags>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

export constexpr BeamFlags operator&(BeamFlags left, BeamFlags right) noexcept
{
	return static_cast<BeamFlags>(static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

export constexpr bool Has_Beam_Flag(BeamFlags flags, BeamFlags flag) noexcept
{
	return (flags & flag) == flag;
}

export struct Vec3 final
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};

export struct Color4 final
{
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;
	float a = 1.0f;
};

export struct BeamDescription final
{
	Vec3 start{};
	Vec3 end{};
	float width = 0.5f;
	Color4 color{};
	float opacity = 1.0f;
	float uv_scale = 1.0f;
	float uv_offset = 0.0f;
	MaterialHandle material{};
	BeamFlags flags = BeamFlags::Enabled;
	PipelineHandle pipeline{};
	bool color_gradient = false;
	Color4 start_color{};
	Color4 end_color{};
};

export using BeamDesc = BeamDescription;

static_assert(std::is_nothrow_move_constructible_v<BeamDescription>);
static_assert(std::is_nothrow_move_assignable_v<BeamDescription>);

export struct BeamData final
{
	std::span<const float> start_x{};
	std::span<const float> start_y{};
	std::span<const float> start_z{};
	std::span<const float> end_x{};
	std::span<const float> end_y{};
	std::span<const float> end_z{};
	std::span<const float> widths{};
	std::span<const float> color_r{};
	std::span<const float> color_g{};
	std::span<const float> color_b{};
	std::span<const float> color_a{};
	std::span<const float> opacities{};
	std::span<const float> uv_scales{};
	std::span<const float> uv_offsets{};
	std::span<const MaterialHandle> materials{};
	std::span<const BeamFlags> flags{};
	std::span<const PipelineHandle> pipelines{};
	std::span<const std::uint8_t> color_gradient{};
	std::span<const float> start_color_r{};
	std::span<const float> start_color_g{};
	std::span<const float> start_color_b{};
	std::span<const float> start_color_a{};
	std::span<const float> end_color_r{};
	std::span<const float> end_color_g{};
	std::span<const float> end_color_b{};
	std::span<const float> end_color_a{};
	std::span<const BeamHandle> handles{};

	std::size_t Size() const noexcept
	{
		return start_x.size();
	}
};

export struct BeamView final
{
	std::array<float, 16> view_projection{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	std::array<float, 3> camera_right{1.0f, 0.0f, 0.0f};
	std::array<float, 3> camera_up{0.0f, 1.0f, 0.0f};
	std::array<float, 3> camera_forward{0.0f, 0.0f, -1.0f};
};

export struct BeamVertex final
{
	float position[3]{};
	float color[4]{};
	float uv[2]{};
	std::uint32_t resource_index = std::numeric_limits<std::uint32_t>::max();
};

static_assert(sizeof(BeamVertex) == 40);

export struct BeamDrawRange final
{
	PipelineHandle pipeline{};
	std::uint32_t first_vertex = 0;
	std::uint32_t vertex_count = 0;
};

export class BeamSet final
{
public:
	void Reserve(std::size_t capacity)
	{
		m_start_x.reserve(capacity);
		m_start_y.reserve(capacity);
		m_start_z.reserve(capacity);
		m_end_x.reserve(capacity);
		m_end_y.reserve(capacity);
		m_end_z.reserve(capacity);
		m_widths.reserve(capacity);
		m_color_r.reserve(capacity);
		m_color_g.reserve(capacity);
		m_color_b.reserve(capacity);
		m_color_a.reserve(capacity);
		m_opacities.reserve(capacity);
		m_uv_scales.reserve(capacity);
		m_uv_offsets.reserve(capacity);
		m_materials.reserve(capacity);
		m_flags.reserve(capacity);
		m_pipelines.reserve(capacity);
		m_color_gradient.reserve(capacity);
		m_start_color_r.reserve(capacity);
		m_start_color_g.reserve(capacity);
		m_start_color_b.reserve(capacity);
		m_start_color_a.reserve(capacity);
		m_end_color_r.reserve(capacity);
		m_end_color_g.reserve(capacity);
		m_end_color_b.reserve(capacity);
		m_end_color_a.reserve(capacity);
		m_dense_handles.reserve(capacity);
		m_slots.reserve(capacity);
	}

	BeamHandle Create(const BeamDescription &description = {})
	{
		Ensure_Capacity();
		if (m_slots.size() >= std::numeric_limits<std::uint32_t>::max() && m_free_head == Invalid_Beam_Index)
			return {};

		const bool reuses_slot = m_free_head != Invalid_Beam_Index;
		const std::uint32_t slot_index = reuses_slot ? m_free_head : static_cast<std::uint32_t>(m_slots.size());
		const std::uint32_t dense_index = static_cast<std::uint32_t>(Size());

		Append(description);
		if (reuses_slot) {
			Slot &slot = m_slots[slot_index];
			m_free_head = slot.next_free;
			slot.next_free = Invalid_Beam_Index;
			slot.dense_index = dense_index;
		} else {
			m_slots.push_back({dense_index, Invalid_Beam_Index, 1});
		}

		m_dense_handles.emplace_back(slot_index, m_slots[slot_index].generation);
		return BeamHandle(slot_index, m_slots[slot_index].generation);
	}

	bool Destroy(BeamHandle handle) noexcept
	{
		if (!Is_Valid(handle))
			return false;

		const std::uint32_t slot_index = handle.Get_Index();
		const std::uint32_t dense_index = m_slots[slot_index].dense_index;
		const std::uint32_t last_dense_index = static_cast<std::uint32_t>(Size() - 1);
		if (dense_index != last_dense_index) {
			Move_Dense(dense_index, last_dense_index);
			const std::uint32_t moved_slot_index = m_dense_handles[last_dense_index].Get_Index();
			m_dense_handles[dense_index] = m_dense_handles[last_dense_index];
			m_slots[moved_slot_index].dense_index = dense_index;
		}

		Pop_Dense();
		Slot &slot = m_slots[slot_index];
		slot.dense_index = Invalid_Beam_Index;
		slot.generation = Next_Generation(slot.generation);
		slot.next_free = m_free_head;
		m_free_head = slot_index;
		return true;
	}

	bool Update(BeamHandle handle, const BeamDescription &description) noexcept
	{
		if (!Is_Valid(handle))
			return false;

		const std::uint32_t dense_index = m_slots[handle.Get_Index()].dense_index;
		m_start_x[dense_index] = description.start.x;
		m_start_y[dense_index] = description.start.y;
		m_start_z[dense_index] = description.start.z;
		m_end_x[dense_index] = description.end.x;
		m_end_y[dense_index] = description.end.y;
		m_end_z[dense_index] = description.end.z;
		m_widths[dense_index] = description.width;
		m_color_r[dense_index] = description.color.r;
		m_color_g[dense_index] = description.color.g;
		m_color_b[dense_index] = description.color.b;
		m_color_a[dense_index] = description.color.a;
		m_opacities[dense_index] = description.opacity;
		m_uv_scales[dense_index] = description.uv_scale;
		m_uv_offsets[dense_index] = description.uv_offset;
		m_materials[dense_index] = description.material;
		m_flags[dense_index] = description.flags;
		m_pipelines[dense_index] = description.pipeline;
		m_color_gradient[dense_index] = description.color_gradient;
		m_start_color_r[dense_index] = description.start_color.r;
		m_start_color_g[dense_index] = description.start_color.g;
		m_start_color_b[dense_index] = description.start_color.b;
		m_start_color_a[dense_index] = description.start_color.a;
		m_end_color_r[dense_index] = description.end_color.r;
		m_end_color_g[dense_index] = description.end_color.g;
		m_end_color_b[dense_index] = description.end_color.b;
		m_end_color_a[dense_index] = description.end_color.a;
		return true;
	}

	bool Is_Valid(BeamHandle handle) const noexcept
	{
		if (!handle.Is_Valid() || handle.Get_Index() >= m_slots.size())
			return false;

		const Slot &slot = m_slots[handle.Get_Index()];
		return slot.dense_index != Invalid_Beam_Index && slot.generation == handle.Get_Generation();
	}

	std::uint32_t Dense_Index(BeamHandle handle) const noexcept
	{
		return Is_Valid(handle) ? m_slots[handle.Get_Index()].dense_index : Invalid_Beam_Index;
	}

	std::size_t Size() const noexcept
	{
		return m_start_x.size();
	}

	BeamData Data() const noexcept
	{
		return {
			m_start_x,
			m_start_y,
			m_start_z,
			m_end_x,
			m_end_y,
			m_end_z,
			m_widths,
			m_color_r,
			m_color_g,
			m_color_b,
			m_color_a,
			m_opacities,
			m_uv_scales,
			m_uv_offsets,
			m_materials,
			m_flags,
			m_pipelines,
			m_color_gradient,
			m_start_color_r,
			m_start_color_g,
			m_start_color_b,
			m_start_color_a,
			m_end_color_r,
			m_end_color_g,
			m_end_color_b,
			m_end_color_a,
			m_dense_handles
		};
	}

	void Clear() noexcept
	{
		m_start_x.clear();
		m_start_y.clear();
		m_start_z.clear();
		m_end_x.clear();
		m_end_y.clear();
		m_end_z.clear();
		m_widths.clear();
		m_color_r.clear();
		m_color_g.clear();
		m_color_b.clear();
		m_color_a.clear();
		m_opacities.clear();
		m_uv_scales.clear();
		m_uv_offsets.clear();
		m_materials.clear();
		m_flags.clear();
		m_pipelines.clear();
		m_color_gradient.clear();
		m_start_color_r.clear();
		m_start_color_g.clear();
		m_start_color_b.clear();
		m_start_color_a.clear();
		m_end_color_r.clear();
		m_end_color_g.clear();
		m_end_color_b.clear();
		m_end_color_a.clear();
		m_dense_handles.clear();
		m_slots.clear();
		m_free_head = Invalid_Beam_Index;
	}

private:
	struct Slot final
	{
		std::uint32_t dense_index = Invalid_Beam_Index;
		std::uint32_t next_free = Invalid_Beam_Index;
		std::uint32_t generation = 0;
	};

	static std::size_t Next_Capacity(std::size_t current) noexcept
	{
		return current == 0
			? 1
			: current > std::numeric_limits<std::size_t>::max() / 2
				? std::numeric_limits<std::size_t>::max()
				: current * 2;
	}

	static std::uint32_t Next_Generation(std::uint32_t generation) noexcept
	{
		const std::uint32_t next = generation + 1;
		return next == 0 ? 1 : next;
	}

	void Ensure_Capacity()
	{
		if (m_start_x.size() < m_start_x.capacity())
			return;

		Reserve(Next_Capacity(m_start_x.size()));
	}

	void Append(const BeamDescription &description)
	{
		m_start_x.push_back(description.start.x);
		m_start_y.push_back(description.start.y);
		m_start_z.push_back(description.start.z);
		m_end_x.push_back(description.end.x);
		m_end_y.push_back(description.end.y);
		m_end_z.push_back(description.end.z);
		m_widths.push_back(description.width);
		m_color_r.push_back(description.color.r);
		m_color_g.push_back(description.color.g);
		m_color_b.push_back(description.color.b);
		m_color_a.push_back(description.color.a);
		m_opacities.push_back(description.opacity);
		m_uv_scales.push_back(description.uv_scale);
		m_uv_offsets.push_back(description.uv_offset);
		m_materials.push_back(description.material);
		m_flags.push_back(description.flags);
		m_pipelines.push_back(description.pipeline);
		m_color_gradient.push_back(description.color_gradient);
		m_start_color_r.push_back(description.start_color.r);
		m_start_color_g.push_back(description.start_color.g);
		m_start_color_b.push_back(description.start_color.b);
		m_start_color_a.push_back(description.start_color.a);
		m_end_color_r.push_back(description.end_color.r);
		m_end_color_g.push_back(description.end_color.g);
		m_end_color_b.push_back(description.end_color.b);
		m_end_color_a.push_back(description.end_color.a);
	}

	void Move_Dense(std::uint32_t destination, std::uint32_t source) noexcept
	{
		m_start_x[destination] = m_start_x[source];
		m_start_y[destination] = m_start_y[source];
		m_start_z[destination] = m_start_z[source];
		m_end_x[destination] = m_end_x[source];
		m_end_y[destination] = m_end_y[source];
		m_end_z[destination] = m_end_z[source];
		m_widths[destination] = m_widths[source];
		m_color_r[destination] = m_color_r[source];
		m_color_g[destination] = m_color_g[source];
		m_color_b[destination] = m_color_b[source];
		m_color_a[destination] = m_color_a[source];
		m_opacities[destination] = m_opacities[source];
		m_uv_scales[destination] = m_uv_scales[source];
		m_uv_offsets[destination] = m_uv_offsets[source];
		m_materials[destination] = m_materials[source];
		m_flags[destination] = m_flags[source];
		m_pipelines[destination] = m_pipelines[source];
		m_color_gradient[destination] = m_color_gradient[source];
		m_start_color_r[destination] = m_start_color_r[source];
		m_start_color_g[destination] = m_start_color_g[source];
		m_start_color_b[destination] = m_start_color_b[source];
		m_start_color_a[destination] = m_start_color_a[source];
		m_end_color_r[destination] = m_end_color_r[source];
		m_end_color_g[destination] = m_end_color_g[source];
		m_end_color_b[destination] = m_end_color_b[source];
		m_end_color_a[destination] = m_end_color_a[source];
	}

	void Pop_Dense() noexcept
	{
		m_start_x.pop_back();
		m_start_y.pop_back();
		m_start_z.pop_back();
		m_end_x.pop_back();
		m_end_y.pop_back();
		m_end_z.pop_back();
		m_widths.pop_back();
		m_color_r.pop_back();
		m_color_g.pop_back();
		m_color_b.pop_back();
		m_color_a.pop_back();
		m_opacities.pop_back();
		m_uv_scales.pop_back();
		m_uv_offsets.pop_back();
		m_materials.pop_back();
		m_flags.pop_back();
		m_pipelines.pop_back();
		m_color_gradient.pop_back();
		m_start_color_r.pop_back();
		m_start_color_g.pop_back();
		m_start_color_b.pop_back();
		m_start_color_a.pop_back();
		m_end_color_r.pop_back();
		m_end_color_g.pop_back();
		m_end_color_b.pop_back();
		m_end_color_a.pop_back();
		m_dense_handles.pop_back();
	}

	AlignedVector<float> m_start_x;
	AlignedVector<float> m_start_y;
	AlignedVector<float> m_start_z;
	AlignedVector<float> m_end_x;
	AlignedVector<float> m_end_y;
	AlignedVector<float> m_end_z;
	AlignedVector<float> m_widths;
	AlignedVector<float> m_color_r;
	AlignedVector<float> m_color_g;
	AlignedVector<float> m_color_b;
	AlignedVector<float> m_color_a;
	AlignedVector<float> m_opacities;
	AlignedVector<float> m_uv_scales;
	AlignedVector<float> m_uv_offsets;
	AlignedVector<MaterialHandle> m_materials;
	AlignedVector<BeamFlags> m_flags;
	AlignedVector<PipelineHandle> m_pipelines;
	AlignedVector<std::uint8_t> m_color_gradient;
	AlignedVector<float> m_start_color_r;
	AlignedVector<float> m_start_color_g;
	AlignedVector<float> m_start_color_b;
	AlignedVector<float> m_start_color_a;
	AlignedVector<float> m_end_color_r;
	AlignedVector<float> m_end_color_g;
	AlignedVector<float> m_end_color_b;
	AlignedVector<float> m_end_color_a;
	std::vector<BeamHandle> m_dense_handles;
	std::vector<Slot> m_slots;
	std::uint32_t m_free_head = Invalid_Beam_Index;
};

export std::size_t Build_Beam_Vertices(
	const BeamData &beams,
	const BeamView &view,
	std::span<BeamVertex> output,
	std::span<const std::uint32_t> order = {},
	std::span<const std::uint32_t> resource_indices = {},
	std::span<const PipelineHandle> pipelines = {},
	std::span<BeamDrawRange> ranges = {}) noexcept;

export struct BeamPassInput final
{
	RHITextureHandle color_target{};
	RHITextureHandle depth_target{};
	RHIBufferHandle vertex_buffer{};
	PipelineHandle pipeline{};
	std::span<const RHIBindlessResource> bindless_resources{};
	GraphResourceHandle color_resource{};
	GraphResourceHandle depth_resource{};
	RHIViewport viewport{};
	std::uint32_t vertex_count = 0;
	std::span<const BeamDrawRange> draw_ranges{};
};

export class BeamPass final
{
public:
	static GraphPassHandle Add_To_Graph(RenderGraph &graph, GraphResourceHandle color_resource, GraphResourceHandle depth_resource, std::uint32_t pass_key = 0)
	{
		if (!graph.Is_Resource_Valid(color_resource) || !graph.Is_Resource_Valid(depth_resource) || color_resource == depth_resource)
			return {};
		if (graph.Resource_Kind(color_resource) != GraphResourceKind::Texture || graph.Resource_Kind(depth_resource) != GraphResourceKind::Texture)
			return {};

		const std::array<GraphResourceUse, 2> uses = {
			GraphResourceUse::Write(color_resource),
			GraphResourceUse::Write(depth_resource)
		};
		return graph.Add_Pass({pass_key}, uses);
	}

	static bool Execute(CommandList &commands, const PassResources &resources, const BeamPassInput &input) noexcept
	{
		const RHITextureHandle color_target = resources.Texture(input.color_resource);
		const RHITextureHandle depth_target = resources.Texture(input.depth_resource);
		if (!color_target.Is_Valid() || !depth_target.Is_Valid() || !input.vertex_buffer.Is_Valid() || !input.pipeline.Is_Valid() || input.vertex_count == 0 || input.viewport.width == 0 || input.viewport.height == 0)
			return false;

		if (!commands.Set_Render_Targets(color_target, depth_target)
			|| !commands.Set_Viewport(input.viewport)
			|| !commands.Set_Bindless_Resources(input.bindless_resources))
			return false;

		if (input.draw_ranges.empty()) {
			return commands.Bind_Pipeline(input.pipeline)
				&& commands.Set_Vertex_Buffer(0, input.vertex_buffer, sizeof(BeamVertex), 0)
				&& commands.Draw(input.vertex_count);
		}

		for (const BeamDrawRange &range : input.draw_ranges) {
			if (!range.pipeline.Is_Valid() || range.vertex_count == 0
				|| !commands.Bind_Pipeline(range.pipeline)
				|| !commands.Set_Vertex_Buffer(0, input.vertex_buffer, sizeof(BeamVertex), 0)
				|| !commands.Draw(range.vertex_count, range.first_vertex))
				return false;
		}
		return true;
	}
};

export class BeamRenderer final
{
public:
	bool Initialize(Device &device, const std::filesystem::path &shader_directory, std::size_t max_beams = 4096)
	{
		if (m_device != nullptr || max_beams == 0 || max_beams > std::numeric_limits<std::uint32_t>::max() / 6u)
			return false;

		const std::size_t vertex_count = max_beams * 6;
		if (vertex_count > std::numeric_limits<std::size_t>::max() / sizeof(BeamVertex) || vertex_count * sizeof(BeamVertex) > std::numeric_limits<std::uint32_t>::max())
			return false;

		m_device = &device;
		m_beams.Reserve(max_beams);
		m_vertices.resize(vertex_count);
		m_order.reserve(max_beams);
		m_resource_indices.resize(max_beams, std::numeric_limits<std::uint32_t>::max());
		m_resolved_pipelines.resize(max_beams);
		m_draw_ranges.reserve(max_beams);
		m_graph = std::make_unique<RenderGraph>();
		m_graph->Reserve(2, 1, 2);
		m_color_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_depth_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_pass = BeamPass::Add_To_Graph(*m_graph, m_color_resource, m_depth_resource, 30);
		if (!m_color_resource.Is_Valid() || !m_depth_resource.Is_Valid() || !m_pass.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_shader = m_shaders.Load_Beam(shader_directory);
		if (!m_shader.Is_Valid()) {
			Shutdown();
			return false;
		}

		Material material;
		material.shader = m_shader;
		material.parameters.values[0] = 1.0f;
		material.parameters.values[1] = 1.0f;
		material.parameters.values[2] = 1.0f;
		material.parameters.values[3] = 1.0f;
		m_material = m_materials.Create(material);
		if (!m_material.Is_Valid()) {
			Shutdown();
			return false;
		}

		const PipelineDesc pipeline_description = m_shaders.Make_Pipeline_Description(m_shader, Make_Beam_Pipeline());
		PipelineDesc alpha_test_description = pipeline_description;
		alpha_test_description.depth_write = true;
		alpha_test_description.blend_mode = RHIBlendMode::Disabled;
		PipelineDesc additive_description = pipeline_description;
		additive_description.blend_mode = RHIBlendMode::Additive;
		PipelineDesc multiply_description = pipeline_description;
		multiply_description.blend_mode = RHIBlendMode::Multiply;
		m_pipelines[0] = m_shaders.Create_Pipeline(device, m_shader, pipeline_description);
		m_pipelines[1] = m_shaders.Create_Pipeline(device, m_shader, additive_description);
		m_pipelines[2] = m_shaders.Create_Pipeline(device, m_shader, multiply_description);
		m_pipelines[3] = m_shaders.Create_Pipeline(device, m_shader, alpha_test_description);
		m_pipeline = m_pipelines[0];
		if (!m_pipelines[0].Is_Valid() || !m_pipelines[1].Is_Valid() || !m_pipelines[2].Is_Valid() || !m_pipelines[3].Is_Valid()) {
			Shutdown();
			return false;
		}

		m_material_constants = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(material.parameters.Bytes().size()), RHIBufferUsage::Constant, 16},
			material.parameters.Bytes());
		if (!m_material_constants.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_vertex_buffer = device.Create_Buffer({static_cast<std::uint32_t>(vertex_count * sizeof(BeamVertex)), RHIBufferUsage::Vertex, sizeof(BeamVertex)});
		if (!m_vertex_buffer.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_bindless.Reserve(256, 0, 128, 0, 128);
		if (!m_bindless.Register_Material(m_material, m_material_constants).Is_Valid()) {
			Shutdown();
			return false;
		}
		m_residency = std::make_unique<GPUResourceResidency>(device);

		return true;
	}

	void Shutdown() noexcept
	{
		if (m_device != nullptr) {
			if (m_vertex_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_vertex_buffer);
			if (m_material_constants.Is_Valid())
				m_device->Destroy_Buffer(m_material_constants);
			for (const PipelineHandle pipeline : m_pipelines)
				if (pipeline.Is_Valid())
					m_device->Destroy_Pipeline(pipeline);
		}

		m_vertex_buffer = {};
		m_material_constants = {};
		m_residency.reset();
		m_bindless.Clear();
		if (m_material.Is_Valid())
			m_materials.Destroy(m_material);
		m_material = {};
		m_textures = {};
		m_materials = {};
		m_samplers = {};
		m_pipeline = {};
		m_pipelines.fill({});
		if (m_shader.Is_Valid())
			m_shaders.Destroy(m_shader);
		m_shader = {};
		m_order.clear();
		m_resource_indices.clear();
		m_resolved_pipelines.clear();
		m_draw_ranges.clear();
		m_plan = {};
		m_graph.reset();
		m_pass = {};
		m_color_resource = {};
		m_depth_resource = {};
		m_beams.Clear();
		m_vertex_count = 0;
		m_device = nullptr;
	}

	bool Is_Initialized() const noexcept
	{
		return m_device != nullptr && m_pipeline.Is_Valid() && m_vertex_buffer.Is_Valid();
	}

	BeamHandle Create(const BeamDescription &description = {})
	{
		return Is_Initialized() ? m_beams.Create(description) : BeamHandle{};
	}

	bool Update(BeamHandle handle, const BeamDescription &description) noexcept
	{
		return Is_Initialized() && m_beams.Update(handle, description);
	}

	bool Destroy(BeamHandle handle) noexcept
	{
		return Is_Initialized() && m_beams.Destroy(handle);
	}

	BeamData Data() const noexcept
	{
		return m_beams.Data();
	}

	std::size_t Size() const noexcept
	{
		return m_beams.Size();
	}

	MaterialHandle Default_Material() const noexcept
	{
		return m_material;
	}

	PipelineHandle Pipeline_For_Flags(BeamFlags flags) const noexcept
	{
		if (Has_Beam_Flag(flags, BeamFlags::AlphaTest))
			return m_pipelines[3];
		if (Has_Beam_Flag(flags, BeamFlags::Additive))
			return m_pipelines[1];
		if (Has_Beam_Flag(flags, BeamFlags::Multiply))
			return m_pipelines[2];
		return m_pipelines[0];
	}

	ShaderHandle Beam_Shader() const noexcept
	{
		return m_shader;
	}

	TextureHandle Create_Texture(const Texture &description, std::span<const std::byte> initial_data)
	{
		if (!Is_Initialized() || m_residency == nullptr || initial_data.empty() || !Has_Texture_Usage(description.usage, TextureUsage::Sampled))
			return {};

		Texture stored = description;
		stored.pixel_data = {};
		const TextureHandle handle = m_textures.Create(stored);
		if (!handle.Is_Valid())
			return {};

		Texture upload = description;
		upload.pixel_data = initial_data;
		if (!m_residency->Upload_Texture(handle, upload)) {
			m_textures.Destroy(handle);
			return {};
		}

		const GPUResidentTexture resident = m_residency->Texture_Info(handle);
		if (!resident.texture.Is_Valid() || !m_bindless.Register_Texture(handle, resident.texture).Is_Valid()) {
			m_residency->Destroy_Texture(handle);
			m_textures.Destroy(handle);
			return {};
		}
		return handle;
	}

	MaterialHandle Create_Material(const Material &description)
	{
		if (!Is_Initialized() || m_residency == nullptr)
			return {};

		Material stored = description;
		if (!stored.shader.Is_Valid())
			stored.shader = m_shader;
		const MaterialHandle handle = m_materials.Create(stored);
		if (!handle.Is_Valid() || !m_residency->Upload_Material(handle, stored)) {
			m_materials.Destroy(handle);
			return {};
		}

		const GPUResidentMaterial resident = m_residency->Material_Info(handle);
		if (!resident.constants.Is_Valid() || !m_bindless.Register_Material(handle, resident.constants).Is_Valid()) {
			m_residency->Destroy_Material(handle);
			m_materials.Destroy(handle);
			return {};
		}
		return handle;
	}

	bool Destroy_Texture(TextureHandle handle) noexcept
	{
		if (m_residency == nullptr || m_textures.Resolve(handle) == nullptr)
			return false;
		m_bindless.Destroy_Texture(handle);
		m_residency->Destroy_Texture(handle);
		return m_textures.Destroy(handle);
	}

	bool Destroy_Material(MaterialHandle handle) noexcept
	{
		if (handle == m_material || m_residency == nullptr || m_materials.Resolve(handle) == nullptr)
			return false;
		m_bindless.Destroy_Material(handle);
		m_residency->Destroy_Material(handle);
		return m_materials.Destroy(handle);
	}

	bool Set_View(const BeamView &view) noexcept
	{
		if (!Is_Initialized())
			return false;
		m_view = view;
		return true;
	}

	const BeamView &View() const noexcept
	{
		return m_view;
	}

	std::span<const BeamVertex> Vertices() const noexcept
	{
		return {m_vertices.data(), m_vertex_count};
	}

	bool Render(CommandList &commands, RHITextureHandle color_target, RHITextureHandle depth_target, RHIViewport viewport) noexcept
	{
		GRAPHICS_PROFILE_SCOPE("Graphics::BeamRenderer::Render");
		if (!Is_Initialized() || !color_target.Is_Valid() || !depth_target.Is_Valid() || viewport.width == 0 || viewport.height == 0)
			return false;

		const BeamData data = m_beams.Data();
		m_order.clear();
		for (std::size_t index = 0; index < data.Size(); ++index) {
			m_resolved_pipelines[index] = data.pipelines.empty() || !data.pipelines[index].Is_Valid()
				? Pipeline_For_Flags(data.flags[index])
				: data.pipelines[index];
			m_resource_indices[index] = std::numeric_limits<std::uint32_t>::max();
			if (!data.materials.empty()) {
				if (const Material *material = m_materials.Resolve(data.materials[index]); material != nullptr && material->textures[0].Is_Valid()) {
					const ResourceIndex resource_index = m_bindless.Texture_Index(material->textures[0]);
					if (resource_index.Is_Valid())
						m_resource_indices[index] = resource_index.Get_Index();
				}
			}
			m_order.push_back(static_cast<std::uint32_t>(index));
		}
		std::sort(m_order.begin(), m_order.end(), [this, &data](std::uint32_t left, std::uint32_t right) noexcept {
			const PipelineHandle left_pipeline = m_resolved_pipelines[left];
			const PipelineHandle right_pipeline = m_resolved_pipelines[right];
			if (left_pipeline.Get_Index() != right_pipeline.Get_Index())
				return left_pipeline.Get_Index() < right_pipeline.Get_Index();
			if (left_pipeline.Get_Generation() != right_pipeline.Get_Generation())
				return left_pipeline.Get_Generation() < right_pipeline.Get_Generation();
			if (data.materials[left] != data.materials[right])
				return data.materials[left].Get_Index() < data.materials[right].Get_Index();
			return left < right;
		});
		m_draw_ranges.clear();
		m_vertex_count = static_cast<std::uint32_t>(Build_Beam_Vertices(
			data,
			m_view,
			std::span<BeamVertex>(m_vertices),
			std::span<const std::uint32_t>(m_order),
			std::span<const std::uint32_t>(m_resource_indices.data(), data.Size()),
			std::span<const PipelineHandle>(m_resolved_pipelines.data(), data.Size()),
			std::span<BeamDrawRange>(m_draw_ranges)));
		if (m_vertex_count == 0)
			return true;
		if (!m_device->Update_Buffer(m_vertex_buffer, 0, std::as_bytes(std::span<const BeamVertex>(m_vertices.data(), m_vertex_count))))
			return false;

		m_bindings[0] = GraphResourceBinding::Texture(m_color_resource, color_target);
		m_bindings[1] = GraphResourceBinding::Texture(m_depth_resource, depth_target);
		if (!m_plan.Is_Valid() && !m_plan.Compile(*m_graph, m_bindings))
			return false;

		const BeamPassInput input{
			color_target,
			depth_target,
			m_vertex_buffer,
			m_pipeline,
			m_bindless.Resources(),
			m_color_resource,
			m_depth_resource,
			viewport,
			m_vertex_count,
			std::span<const BeamDrawRange>(m_draw_ranges)
		};
		return m_plan.Execute(*m_graph, commands, [&](GraphPassHandle pass, CommandList &command_list, const PassResources &resources) noexcept {
			return pass == m_pass && BeamPass::Execute(command_list, resources, input);
		});
	}

	bool Render(RHITextureHandle color_target, RHITextureHandle depth_target, RHIViewport viewport) noexcept
	{
		return m_device != nullptr && Render(m_device->Immediate_Command_List(), color_target, depth_target, viewport);
	}

private:
	Device *m_device = nullptr;
	BeamSet m_beams;
	BeamView m_view{};
	AlignedVector<BeamVertex> m_vertices;
	std::vector<std::uint32_t> m_order;
	std::vector<std::uint32_t> m_resource_indices;
	std::vector<PipelineHandle> m_resolved_pipelines;
	std::vector<BeamDrawRange> m_draw_ranges;
	std::uint32_t m_vertex_count = 0;
	ShaderLibrary m_shaders;
	ShaderHandle m_shader{};
	TexturePool m_textures;
	SamplerPool m_samplers;
	MaterialPool m_materials;
	std::unique_ptr<GPUResourceResidency> m_residency;
	MaterialHandle m_material{};
	std::array<PipelineHandle, 4> m_pipelines{};
	PipelineHandle m_pipeline{};
	RHIBufferHandle m_material_constants{};
	RHIBufferHandle m_vertex_buffer{};
	BindlessResourceTable m_bindless;
	std::unique_ptr<RenderGraph> m_graph;
	ExecutionPlan m_plan;
	std::array<GraphResourceBinding, 2> m_bindings{};
	GraphResourceHandle m_color_resource{};
	GraphResourceHandle m_depth_resource{};
	GraphPassHandle m_pass{};
};

namespace
{
Vec3 operator-(Vec3 left, Vec3 right) noexcept
{
	return {left.x - right.x, left.y - right.y, left.z - right.z};
}

Vec3 operator+(Vec3 left, Vec3 right) noexcept
{
	return {left.x + right.x, left.y + right.y, left.z + right.z};
}

Vec3 operator*(Vec3 value, float scalar) noexcept
{
	return {value.x * scalar, value.y * scalar, value.z * scalar};
}

float Dot(Vec3 left, Vec3 right) noexcept
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

Vec3 Cross(Vec3 left, Vec3 right) noexcept
{
	return {
		left.y * right.z - left.z * right.y,
		left.z * right.x - left.x * right.z,
		left.x * right.y - left.y * right.x
	};
}

Vec3 Normalize(Vec3 value) noexcept
{
	const float length_squared = Dot(value, value);
	if (!(length_squared > 1.0e-12f) || !std::isfinite(length_squared))
		return {};

	const float inverse_length = 1.0f / std::sqrt(length_squared);
	return value * inverse_length;
}

std::array<float, 4> Transform(const std::array<float, 16> &matrix, Vec3 point) noexcept
{
	return {
		matrix[0] * point.x + matrix[1] * point.y + matrix[2] * point.z + matrix[3],
		matrix[4] * point.x + matrix[5] * point.y + matrix[6] * point.z + matrix[7],
		matrix[8] * point.x + matrix[9] * point.y + matrix[10] * point.z + matrix[11],
		matrix[12] * point.x + matrix[13] * point.y + matrix[14] * point.z + matrix[15]
	};
}

void Write_Vertex(BeamVertex &vertex, const std::array<float, 4> &position, const std::array<float, 4> &color, float u, float v, std::uint32_t resource_index) noexcept
{
	vertex.position[0] = position[0];
	vertex.position[1] = position[1];
	vertex.position[2] = position[2];
	vertex.color[0] = color[0];
	vertex.color[1] = color[1];
	vertex.color[2] = color[2];
	vertex.color[3] = color[3];
	vertex.uv[0] = u;
	vertex.uv[1] = v;
	vertex.resource_index = resource_index;
}
}

export std::size_t Build_Beam_Vertices(
	const BeamData &beams,
	const BeamView &view,
	std::span<BeamVertex> output,
	std::span<const std::uint32_t> order,
	std::span<const std::uint32_t> resource_indices,
	std::span<const PipelineHandle> pipelines,
	std::span<BeamDrawRange> ranges) noexcept
{
	if (beams.Size() > output.size() / 6)
		return 0;
	if ((!order.empty() && order.size() != beams.Size())
		|| (!resource_indices.empty() && resource_indices.size() != beams.Size())
		|| (!pipelines.empty() && pipelines.size() != beams.Size()))
		return 0;

	std::size_t output_count = 0;
	std::size_t range_count = 0;
	for (std::size_t ordered_index = 0; ordered_index < beams.Size(); ++ordered_index) {
		const std::size_t index = order.empty() ? ordered_index : order[ordered_index];
		if (index >= beams.Size())
			return 0;
		if (!Has_Beam_Flag(beams.flags[index], BeamFlags::Enabled) || !(beams.widths[index] > 0.0f))
			continue;

		const Vec3 start{beams.start_x[index], beams.start_y[index], beams.start_z[index]};
		const Vec3 end{beams.end_x[index], beams.end_y[index], beams.end_z[index]};
		const Vec3 direction = Normalize(end - start);
		if (Dot(direction, direction) == 0.0f)
			continue;

		Vec3 side = Normalize(Cross(direction, {view.camera_forward[0], view.camera_forward[1], view.camera_forward[2]}));
		if (Dot(side, side) == 0.0f)
			side = Normalize({view.camera_right[0], view.camera_right[1], view.camera_right[2]});
		if (Dot(side, side) == 0.0f)
			continue;

		const float half_width = beams.widths[index] * 0.5f;
		const Vec3 offset = side * half_width;
		const Vec3 start_left = start - offset;
		const Vec3 start_right = start + offset;
		const Vec3 end_left = end - offset;
		const Vec3 end_right = end + offset;
		const std::array<float, 4> start_clip = Transform(view.view_projection, start_left);
		const std::array<float, 4> start_right_clip = Transform(view.view_projection, start_right);
		const std::array<float, 4> end_clip = Transform(view.view_projection, end_left);
		const std::array<float, 4> end_right_clip = Transform(view.view_projection, end_right);
		if (start_clip[3] == 0.0f || start_right_clip[3] == 0.0f || end_clip[3] == 0.0f || end_right_clip[3] == 0.0f)
			continue;

		const std::array<float, 4> color = {
			beams.color_r[index],
			beams.color_g[index],
			beams.color_b[index],
			beams.color_a[index] * beams.opacities[index]
		};
		const std::array<float, 4> start_color = beams.color_gradient.empty() || beams.color_gradient[index] == 0
			? color
			: std::array<float, 4>{
				beams.start_color_r[index],
				beams.start_color_g[index],
				beams.start_color_b[index],
				beams.start_color_a[index] * beams.opacities[index]
			};
		const std::array<float, 4> end_color = beams.color_gradient.empty() || beams.color_gradient[index] == 0
			? color
			: std::array<float, 4>{
				beams.end_color_r[index],
				beams.end_color_g[index],
				beams.end_color_b[index],
				beams.end_color_a[index] * beams.opacities[index]
			};
		const std::uint32_t resource_index = resource_indices.empty() ? std::numeric_limits<std::uint32_t>::max() : resource_indices[index];
		const PipelineHandle pipeline = pipelines.empty() ? (beams.pipelines.empty() ? PipelineHandle{} : beams.pipelines[index]) : pipelines[index];
		if (!ranges.empty()) {
			if (range_count != 0 && ranges[range_count - 1].pipeline == pipeline
				&& ranges[range_count - 1].first_vertex + ranges[range_count - 1].vertex_count == output_count) {
				ranges[range_count - 1].vertex_count += 6;
			} else {
				if (range_count >= ranges.size())
					return 0;
				ranges[range_count++] = {pipeline, static_cast<std::uint32_t>(output_count), 6};
			}
		}
		const float start_v = beams.uv_offsets[index];
		const float end_v = start_v + beams.uv_scales[index];
		Write_Vertex(output[output_count++], start_clip, start_color, 0.0f, start_v, resource_index);
		Write_Vertex(output[output_count++], start_right_clip, start_color, 1.0f, start_v, resource_index);
		Write_Vertex(output[output_count++], end_clip, end_color, 0.0f, end_v, resource_index);
		Write_Vertex(output[output_count++], end_clip, end_color, 0.0f, end_v, resource_index);
		Write_Vertex(output[output_count++], start_right_clip, start_color, 1.0f, start_v, resource_index);
		Write_Vertex(output[output_count++], end_right_clip, end_color, 1.0f, end_v, resource_index);
	}

	return output_count;
}

namespace
{
BeamRenderer g_beam_renderer;
}

export BeamHandle CreateBeam(const BeamDesc &description)
{
	return g_beam_renderer.Create(description);
}

export bool UpdateBeam(BeamHandle handle, const BeamDesc &description) noexcept
{
	return g_beam_renderer.Update(handle, description);
}

export void DestroyBeam(BeamHandle handle) noexcept
{
	g_beam_renderer.Destroy(handle);
}

export bool InitializeBeams(Device &device, const std::filesystem::path &shader_directory, std::size_t max_beams)
{
	return g_beam_renderer.Initialize(device, shader_directory, max_beams);
}

export void ShutdownBeams() noexcept
{
	g_beam_renderer.Shutdown();
}

export bool SetBeamView(const BeamView &view) noexcept
{
	return g_beam_renderer.Set_View(view);
}

export BeamRenderer &GetBeamRenderer() noexcept
{
	return g_beam_renderer;
}

}
