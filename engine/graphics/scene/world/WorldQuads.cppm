module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <span>
#include <vector>

export module Graphics.Scene.WorldQuads;

export import Graphics.Resources.Bindless.BindlessResourceTable;
export import Graphics.Resources.Materials.Material;
export import Graphics.Resources.Residency.GPUResourceResidency;
export import Graphics.RenderGraph.Execution;
export import Graphics.Scene.GPUScene;
export import Graphics.Scene.Views.View;
export import Graphics.Shaders.Library;

import Graphics.Memory.AlignedAllocator;

namespace Graphics
{

export inline constexpr std::uint32_t Invalid_World_Quad_Index = std::numeric_limits<std::uint32_t>::max();

export struct WorldQuadDescription final
{
	std::array<std::array<float, 3>, 4> corners{};
	std::array<float, 4> color{1.0f, 1.0f, 1.0f, 1.0f};
	MaterialHandle material{};
	PipelineHandle pipeline{};
	std::uint32_t sort_group = 0;
};

export struct WorldQuadData final
{
	std::span<const float> corner0_x{};
	std::span<const float> corner0_y{};
	std::span<const float> corner0_z{};
	std::span<const float> corner1_x{};
	std::span<const float> corner1_y{};
	std::span<const float> corner1_z{};
	std::span<const float> corner2_x{};
	std::span<const float> corner2_y{};
	std::span<const float> corner2_z{};
	std::span<const float> corner3_x{};
	std::span<const float> corner3_y{};
	std::span<const float> corner3_z{};
	std::span<const float> color_r{};
	std::span<const float> color_g{};
	std::span<const float> color_b{};
	std::span<const float> color_a{};
	std::span<const MaterialHandle> materials{};
	std::span<const PipelineHandle> pipelines{};
	std::span<const std::uint32_t> sort_groups{};

	std::size_t Size() const noexcept
	{
		return corner0_x.size();
	}
};

export struct alignas(16) GPUWorldQuadData final
{
	std::array<std::array<float, 4>, 4> corners{};
	std::array<float, 4> color{1.0f, 1.0f, 1.0f, 1.0f};
	std::uint32_t material_index = Invalid_GPU_Index;
	std::uint32_t reserved0 = 0;
	std::uint32_t reserved1 = 0;
	std::uint32_t reserved2 = 0;
};

static_assert(sizeof(GPUWorldQuadData) == 96);

export struct WorldQuadVertex final
{
	float position[3]{};
	float color[4]{};
	float uv[2]{};
};

static_assert(sizeof(WorldQuadVertex) == 36);

export struct alignas(16) WorldQuadViewData final
{
	std::array<float, 16> view_projection{};
	std::array<float, 4> reserved0{};
	std::array<float, 4> reserved1{};
	std::array<float, 4> reserved2{};
};

static_assert(sizeof(WorldQuadViewData) == 112);

export struct WorldQuadDrawData final
{
	std::uint32_t quad_index = Invalid_World_Quad_Index;
	std::uint32_t material_index = Invalid_GPU_Index;
	PipelineHandle pipeline{};
	std::uint32_t sort_group = 0;
	std::uint64_t sort_key = 0;
};

static_assert(sizeof(WorldQuadDrawData) == 32);

export bool Is_Valid_World_Quad(const WorldQuadDescription &description) noexcept
{
	if (!description.material.Is_Valid() || !description.pipeline.Is_Valid())
		return false;
	for (const std::array<float, 3> &corner : description.corners)
		for (const float component : corner)
			if (!std::isfinite(component))
				return false;
	for (const float component : description.color)
		if (!std::isfinite(component))
			return false;
	return true;
}

std::array<float, 16> Make_View_Projection(const View &view) noexcept
{
	std::array<float, 16> result{};
	for (std::size_t row = 0; row < 4; ++row)
		for (std::size_t column = 0; column < 4; ++column)
			for (std::size_t element = 0; element < 4; ++element)
				result[row * 4 + column] += view.projection_matrix.values[row * 4 + element]
					* view.view_matrix.values[element * 4 + column];
	return result;
}

namespace
{

std::array<float, 3> Quad_Center(const WorldQuadData &data, std::size_t index) noexcept
{
	return {
		(data.corner0_x[index] + data.corner1_x[index] + data.corner2_x[index] + data.corner3_x[index]) * 0.25f,
		(data.corner0_y[index] + data.corner1_y[index] + data.corner2_y[index] + data.corner3_y[index]) * 0.25f,
		(data.corner0_z[index] + data.corner1_z[index] + data.corner2_z[index] + data.corner3_z[index]) * 0.25f
	};
}

float Distance_Squared(const std::array<float, 3> &left, const std::array<float, 3> &right) noexcept
{
	const float x = left[0] - right[0];
	const float y = left[1] - right[1];
	const float z = left[2] - right[2];
	return x * x + y * y + z * z;
}

float Quad_Radius(const WorldQuadData &data, std::size_t index, const std::array<float, 3> &center) noexcept
{
	const std::array<std::array<float, 3>, 4> corners = {{
		{{data.corner0_x[index], data.corner0_y[index], data.corner0_z[index]}},
		{{data.corner1_x[index], data.corner1_y[index], data.corner1_z[index]}},
		{{data.corner2_x[index], data.corner2_y[index], data.corner2_z[index]}},
		{{data.corner3_x[index], data.corner3_y[index], data.corner3_z[index]}}
	}};
	float radius_squared = 0.0f;
	for (const std::array<float, 3> &corner : corners)
		radius_squared = std::max(radius_squared, Distance_Squared(corner, center));
	return std::sqrt(radius_squared);
}

bool Passes_Plane(const FrustumPlane &plane, const std::array<float, 3> &center, float radius) noexcept
{
	const float signed_distance = plane.normal.x * center[0]
		+ plane.normal.y * center[1]
		+ plane.normal.z * center[2]
		+ plane.distance;
	return signed_distance >= -radius;
}

bool Is_Visible(const View &view, const WorldQuadData &data, std::size_t index) noexcept
{
	const std::array<float, 3> center = Quad_Center(data, index);
	const float radius = Quad_Radius(data, index, center);
	return Passes_Plane(view.frustum.left, center, radius)
		&& Passes_Plane(view.frustum.right, center, radius)
		&& Passes_Plane(view.frustum.bottom, center, radius)
		&& Passes_Plane(view.frustum.top, center, radius)
		&& Passes_Plane(view.frustum.near_plane, center, radius)
		&& Passes_Plane(view.frustum.far_plane, center, radius);
}

}

export struct WorldQuadPassInput final
{
	std::span<const WorldQuadDrawData> draws{};
	RHIBufferHandle quad_buffer{};
	std::uint32_t quad_stride = 0;
	std::uint32_t quad_vertex_count = 0;
	std::span<const RHIBindlessResource> bindless_resources{};
	GraphResourceHandle color_target{};
	GraphResourceHandle depth_target{};
	RHIViewport viewport{};
};

export class WorldQuadPass final
{
public:
	static GraphPassHandle Add_To_Graph(RenderGraph &graph, GraphResourceHandle color_target, GraphResourceHandle depth_target, std::uint32_t pass_key = 0)
	{
		if (!graph.Is_Resource_Valid(color_target) || !graph.Is_Resource_Valid(depth_target) || color_target == depth_target)
			return {};
		if (graph.Resource_Kind(color_target) != GraphResourceKind::Texture || graph.Resource_Kind(depth_target) != GraphResourceKind::Texture)
			return {};

		const std::array<GraphResourceUse, 2> uses = {
			GraphResourceUse::Write(color_target),
			GraphResourceUse::Read(depth_target)
		};
		return graph.Add_Pass({pass_key}, uses);
	}

	static bool Execute(CommandList &commands, const PassResources &resources, const WorldQuadPassInput &input) noexcept
	{
		const RHITextureHandle color_target = resources.Texture(input.color_target);
		const RHITextureHandle depth_target = resources.Texture(input.depth_target);
		if (!color_target.Is_Valid() || !depth_target.Is_Valid() || !input.quad_buffer.Is_Valid()
			|| input.quad_stride == 0 || input.quad_vertex_count == 0 || input.viewport.width == 0 || input.viewport.height == 0)
			return false;
		if (!commands.Set_Render_Targets(color_target, depth_target)
			|| !commands.Set_Viewport(input.viewport)
			|| !commands.Set_Bindless_Resources(input.bindless_resources)
			|| !commands.Set_Vertex_Buffer(0, input.quad_buffer, input.quad_stride, 0))
			return false;

		PipelineHandle bound_pipeline{};
		bool has_bound_pipeline = false;
		for (std::size_t draw_index = 0; draw_index < input.draws.size();) {
			const WorldQuadDrawData &draw = input.draws[draw_index];
			if (draw.quad_index == Invalid_World_Quad_Index || draw.material_index == Invalid_GPU_Index || !draw.pipeline.Is_Valid())
				return false;

			if (!has_bound_pipeline || draw.pipeline != bound_pipeline) {
				if (!commands.Bind_Pipeline(draw.pipeline))
					return false;
				bound_pipeline = draw.pipeline;
				has_bound_pipeline = true;
			}

			std::size_t batch_count = 1;
			while (draw_index + batch_count < input.draws.size()) {
				const WorldQuadDrawData &next = input.draws[draw_index + batch_count];
				if (next.pipeline != draw.pipeline)
					break;
				++batch_count;
			}
			if (!commands.Draw(input.quad_vertex_count, 0, static_cast<std::uint32_t>(batch_count), static_cast<std::uint32_t>(draw_index)))
				return false;
			draw_index += batch_count;
		}

		return true;
	}
};

export class WorldQuadRenderer final
{
public:
	bool Initialize(Device &device, const std::filesystem::path &shader_directory, std::size_t max_quads = 1000, std::size_t max_materials = 8)
	{
		if (m_device != nullptr || !device.Is_Valid() || max_quads == 0 || max_materials == 0
			|| max_quads > std::numeric_limits<std::uint32_t>::max() / sizeof(GPUWorldQuadData)
			|| max_materials > std::numeric_limits<std::uint32_t>::max() / sizeof(GPUMaterialData))
			return false;

		m_device = &device;
		m_capacity = max_quads;
		m_quads.Reserve(max_quads);
		m_draws.reserve(max_quads);
		m_gpu_quads.resize(max_quads);
		m_materials.Reserve(max_materials);
		m_gpu_scene.Reserve(0, 0, max_materials, max_materials + 4);
		m_graph = std::make_unique<RenderGraph>();
		m_graph->Reserve(2, 1, 2);
		m_color_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_depth_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_pass = WorldQuadPass::Add_To_Graph(*m_graph, m_color_resource, m_depth_resource, 45);
		if (!m_color_resource.Is_Valid() || !m_depth_resource.Is_Valid() || !m_pass.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_shader = m_shaders.Load_World_Quad(shader_directory);
		if (!m_shader.Is_Valid()) {
			Shutdown();
			return false;
		}
		m_pipeline = m_shaders.Create_Pipeline(device, m_shader, Make_World_Quad_Pipeline());
		if (!m_pipeline.Is_Valid()) {
			Shutdown();
			return false;
		}
		m_residency = std::make_unique<GPUResourceResidency>(device);

		constexpr std::array<WorldQuadVertex, 6> quad = {{
			{{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
			{{-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
			{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
			{{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
			{{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
			{{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
		}};
		m_quad_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(quad)), RHIBufferUsage::Vertex, sizeof(WorldQuadVertex)},
			std::as_bytes(std::span<const WorldQuadVertex>(quad)));
		m_gpu_quad_buffer = device.Create_Buffer({
			static_cast<std::uint32_t>(max_quads * sizeof(GPUWorldQuadData)),
			RHIBufferUsage::Storage,
			static_cast<std::uint32_t>(sizeof(GPUWorldQuadData))
		});
		m_material_buffer = device.Create_Buffer({
			static_cast<std::uint32_t>(max_materials * sizeof(GPUMaterialData)),
			RHIBufferUsage::Storage,
			static_cast<std::uint32_t>(sizeof(GPUMaterialData))
		});
		m_view_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(WorldQuadViewData)), RHIBufferUsage::Storage, static_cast<std::uint32_t>(sizeof(WorldQuadViewData))},
			std::as_bytes(std::span<const WorldQuadViewData>(&m_gpu_view, 1)));
		if (!m_quad_buffer.Is_Valid() || !m_gpu_quad_buffer.Is_Valid() || !m_material_buffer.Is_Valid() || !m_view_buffer.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_bindless.Reserve(3, 3, 125, 0, max_materials);
		if (!m_bindless.Register_Buffer(m_gpu_quad_buffer).Is_Valid()
			|| !m_bindless.Register_Buffer(m_material_buffer).Is_Valid()
			|| !m_bindless.Register_Buffer(m_view_buffer).Is_Valid()) {
			Shutdown();
			return false;
		}
		m_materials_dirty = true;
		return true;
	}

	void Shutdown() noexcept
	{
		if (m_device != nullptr) {
			if (m_quad_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_quad_buffer);
			if (m_gpu_quad_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_gpu_quad_buffer);
			if (m_material_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_material_buffer);
			if (m_view_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_view_buffer);
			if (m_pipeline.Is_Valid())
				m_device->Destroy_Pipeline(m_pipeline);
		}

		m_bindless.Clear();
		m_residency.reset();
		m_textures = {};
		m_samplers = {};
		m_materials = {};
		m_gpu_scene = {};
		m_shaders.Destroy(m_shader);
		m_shader = {};
		m_pipeline = {};
		m_quad_buffer = {};
		m_gpu_quad_buffer = {};
		m_material_buffer = {};
		m_view_buffer = {};
		m_quads.Clear();
		m_draws.clear();
		m_gpu_quads.clear();
		m_graph.reset();
		m_plan = {};
		m_bindings = {};
		m_color_resource = {};
		m_depth_resource = {};
		m_pass = {};
		m_bound_color_target = {};
		m_bound_depth_target = {};
		m_view = {};
		m_gpu_view = {};
		m_capacity = 0;
		m_materials_dirty = false;
		m_view_dirty = false;
		m_device = nullptr;
	}

	bool Is_Initialized() const noexcept
	{
		return m_device != nullptr && m_pipeline.Is_Valid() && m_gpu_quad_buffer.Is_Valid() && m_material_buffer.Is_Valid();
	}

	ShaderHandle World_Quad_Shader() const noexcept
	{
		return m_shader;
	}

	PipelineHandle Pipeline() const noexcept
	{
		return m_pipeline;
	}

	TextureHandle Create_Texture(const Texture &description, std::span<const std::byte> initial_data)
	{
		if (!Is_Initialized() || initial_data.empty() || !Has_Texture_Usage(description.usage, TextureUsage::Sampled))
			return {};

		Texture stored = description;
		stored.pixel_data = {};
		const TextureHandle handle = m_textures.Create(stored);
		if (!handle.Is_Valid())
			return {};

		Texture upload = description;
		upload.pixel_data = initial_data;
		if (m_residency == nullptr || !m_residency->Upload_Texture(handle, upload)) {
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
		if (!Is_Initialized() || m_residency == nullptr || description.shader != m_shader)
			return {};

		const MaterialHandle handle = m_materials.Create(description);
		if (!handle.Is_Valid() || !m_residency->Upload_Material(handle, description)) {
			m_materials.Destroy(handle);
			return {};
		}
		const GPUResidentMaterial resident = m_residency->Material_Info(handle);
		if (!resident.constants.Is_Valid() || !m_bindless.Register_Material(handle, resident.constants).Is_Valid() || !Rebuild_Materials()) {
			m_bindless.Destroy_Material(handle);
			m_residency->Destroy_Material(handle);
			m_materials.Destroy(handle);
			return {};
		}
		return handle;
	}

	bool Destroy_Texture(TextureHandle handle) noexcept
	{
		if (m_textures.Resolve(handle) == nullptr || m_residency == nullptr)
			return false;
		m_bindless.Destroy_Texture(handle);
		m_residency->Destroy_Texture(handle);
		return m_textures.Destroy(handle);
	}

	bool Destroy_Material(MaterialHandle handle) noexcept
	{
		if (m_materials.Resolve(handle) == nullptr || m_residency == nullptr)
			return false;
		m_bindless.Destroy_Material(handle);
		m_residency->Destroy_Material(handle);
		return m_materials.Destroy(handle) && Rebuild_Materials();
	}

	bool Set_View(const View &view) noexcept
	{
		if (!Is_Initialized())
			return false;
		m_view = view;
		m_gpu_view.view_projection = Make_View_Projection(view);
		m_view_dirty = true;
		return true;
	}

	void Clear() noexcept
	{
		m_quads.Clear();
	}

	bool Submit(const WorldQuadDescription &description) noexcept
	{
		if (!Is_Initialized() || !Is_Valid_World_Quad(description) || description.pipeline != m_pipeline || m_quads.Size() >= m_capacity)
			return false;
		return m_quads.Append(description);
	}

	std::size_t Quad_Count() const noexcept
	{
		return m_quads.Size();
	}

	std::size_t Visible_Quad_Count() const noexcept
	{
		return m_draws.size();
	}

	std::size_t Draw_Count() const noexcept
	{
		return m_last_draw_count;
	}

	bool Render(CommandList &commands, RHITextureHandle color_target, RHITextureHandle depth_target, RHIViewport viewport) noexcept
	{
		if (!Is_Initialized() || !color_target.Is_Valid() || !depth_target.Is_Valid() || viewport.width == 0 || viewport.height == 0)
			return false;

		m_draws.clear();
		const WorldQuadData data = m_quads.Data();
		for (std::size_t quad_index = 0; quad_index < data.Size(); ++quad_index) {
			if (!Is_Visible(m_view, data, quad_index))
				continue;
			const std::uint32_t material_index = m_gpu_scene.Material_Index(data.materials[quad_index]);
			if (material_index == Invalid_GPU_Index || !data.pipelines[quad_index].Is_Valid())
				continue;
			m_draws.push_back({
				static_cast<std::uint32_t>(quad_index),
				material_index,
				data.pipelines[quad_index],
				data.sort_groups[quad_index],
				(static_cast<std::uint64_t>(data.sort_groups[quad_index]) << 32) | static_cast<std::uint32_t>(quad_index)
			});
		}
		std::sort(m_draws.begin(), m_draws.end(), [](const WorldQuadDrawData &left, const WorldQuadDrawData &right) noexcept {
			if (left.sort_group != right.sort_group)
				return left.sort_group < right.sort_group;
			if (left.pipeline.Get_Index() != right.pipeline.Get_Index())
				return left.pipeline.Get_Index() < right.pipeline.Get_Index();
			if (left.pipeline.Get_Generation() != right.pipeline.Get_Generation())
				return left.pipeline.Get_Generation() < right.pipeline.Get_Generation();
			if (left.material_index != right.material_index)
				return left.material_index < right.material_index;
			return left.quad_index < right.quad_index;
		});
		m_last_draw_count = 0;
		for (std::size_t draw_index = 0; draw_index < m_draws.size();) {
			++m_last_draw_count;
			const PipelineHandle pipeline = m_draws[draw_index].pipeline;
			++draw_index;
			while (draw_index < m_draws.size() && m_draws[draw_index].pipeline == pipeline)
				++draw_index;
		}

		for (std::size_t draw_index = 0; draw_index < m_draws.size(); ++draw_index) {
			const std::size_t source_index = m_draws[draw_index].quad_index;
			GPUWorldQuadData &gpu_quad = m_gpu_quads[draw_index];
			gpu_quad.corners[0] = {data.corner0_x[source_index], data.corner0_y[source_index], data.corner0_z[source_index], 1.0f};
			gpu_quad.corners[1] = {data.corner1_x[source_index], data.corner1_y[source_index], data.corner1_z[source_index], 1.0f};
			gpu_quad.corners[2] = {data.corner2_x[source_index], data.corner2_y[source_index], data.corner2_z[source_index], 1.0f};
			gpu_quad.corners[3] = {data.corner3_x[source_index], data.corner3_y[source_index], data.corner3_z[source_index], 1.0f};
			gpu_quad.color = {data.color_r[source_index], data.color_g[source_index], data.color_b[source_index], data.color_a[source_index]};
			gpu_quad.material_index = m_draws[draw_index].material_index;
		}
		if (!m_draws.empty() && !m_device->Update_Buffer(m_gpu_quad_buffer, 0,
			std::as_bytes(std::span<const GPUWorldQuadData>(m_gpu_quads.data(), m_draws.size()))))
			return false;
		if (m_materials_dirty) {
			if (!m_gpu_scene.Materials().empty() && !m_device->Update_Buffer(m_material_buffer, 0, std::as_bytes(m_gpu_scene.Materials())))
				return false;
			m_materials_dirty = false;
		}
		if (m_view_dirty && !m_device->Update_Buffer(m_view_buffer, 0, std::as_bytes(std::span<const WorldQuadViewData>(&m_gpu_view, 1))))
			return false;
		m_view_dirty = false;

		m_bindings[0] = GraphResourceBinding::Texture(m_color_resource, color_target);
		m_bindings[1] = GraphResourceBinding::Texture(m_depth_resource, depth_target);
		if (!m_plan.Is_Valid() || m_bound_color_target != color_target || m_bound_depth_target != depth_target) {
			if (!m_plan.Compile(*m_graph, m_bindings))
				return false;
			m_bound_color_target = color_target;
			m_bound_depth_target = depth_target;
		}

		const WorldQuadPassInput input{
			m_draws,
			m_quad_buffer,
			static_cast<std::uint32_t>(sizeof(WorldQuadVertex)),
			6,
			m_bindless.Resources(),
			m_color_resource,
			m_depth_resource,
			viewport
		};
		return m_plan.Execute(*m_graph, commands, [&](GraphPassHandle pass, CommandList &command_list, const PassResources &resources) noexcept {
			return pass == m_pass && WorldQuadPass::Execute(command_list, resources, input);
		});
	}

	bool Render(RHITextureHandle color_target, RHITextureHandle depth_target, RHIViewport viewport) noexcept
	{
		return m_device != nullptr && Render(m_device->Immediate_Command_List(), color_target, depth_target, viewport);
	}

private:
	class QuadStorage final
	{
	public:
		void Reserve(std::size_t capacity)
		{
			for (AlignedVector<float> &column : m_corners)
				column.reserve(capacity);
			for (AlignedVector<float> &column : m_colors)
				column.reserve(capacity);
			m_materials.reserve(capacity);
			m_pipelines.reserve(capacity);
			m_sort_groups.reserve(capacity);
		}

		void Clear() noexcept
		{
			for (AlignedVector<float> &column : m_corners)
				column.clear();
			for (AlignedVector<float> &column : m_colors)
				column.clear();
			m_materials.clear();
			m_pipelines.clear();
			m_sort_groups.clear();
		}

		std::size_t Size() const noexcept
		{
			return m_materials.size();
		}

		bool Append(const WorldQuadDescription &description) noexcept
		{
			for (std::size_t corner = 0; corner < 4; ++corner)
				for (std::size_t component = 0; component < 3; ++component)
					m_corners[corner * 3 + component].push_back(description.corners[corner][component]);
			for (std::size_t component = 0; component < 4; ++component)
				m_colors[component].push_back(description.color[component]);
			m_materials.push_back(description.material);
			m_pipelines.push_back(description.pipeline);
			m_sort_groups.push_back(description.sort_group);
			return true;
		}

		WorldQuadData Data() const noexcept
		{
			return {
				m_corners[0], m_corners[1], m_corners[2],
				m_corners[3], m_corners[4], m_corners[5],
				m_corners[6], m_corners[7], m_corners[8],
				m_corners[9], m_corners[10], m_corners[11],
				m_colors[0], m_colors[1], m_colors[2], m_colors[3],
				m_materials, m_pipelines, m_sort_groups
			};
		}

	private:
		std::array<AlignedVector<float>, 12> m_corners;
		std::array<AlignedVector<float>, 4> m_colors;
		AlignedVector<MaterialHandle> m_materials;
		AlignedVector<PipelineHandle> m_pipelines;
		AlignedVector<std::uint32_t> m_sort_groups;
	};

	bool Rebuild_Materials() noexcept
	{
		if (!m_gpu_scene.Build(m_empty_scene, m_meshes, m_textures, m_samplers, m_materials, nullptr, &m_bindless))
			return false;
		m_materials_dirty = true;
		return true;
	}

	Device *m_device = nullptr;
	std::size_t m_capacity = 0;
	QuadStorage m_quads;
	std::vector<WorldQuadDrawData> m_draws;
	std::vector<GPUWorldQuadData> m_gpu_quads;
	std::size_t m_last_draw_count = 0;
	MeshPool m_meshes;
	TexturePool m_textures;
	SamplerPool m_samplers;
	MaterialPool m_materials;
	std::unique_ptr<GPUResourceResidency> m_residency;
	RenderScene m_empty_scene;
	GPUScene m_gpu_scene;
	ShaderLibrary m_shaders;
	ShaderHandle m_shader{};
	PipelineHandle m_pipeline{};
	RHIBufferHandle m_quad_buffer{};
	RHIBufferHandle m_gpu_quad_buffer{};
	RHIBufferHandle m_material_buffer{};
	RHIBufferHandle m_view_buffer{};
	BindlessResourceTable m_bindless;
	View m_view{};
	WorldQuadViewData m_gpu_view{};
	bool m_materials_dirty = false;
	bool m_view_dirty = false;
	std::unique_ptr<RenderGraph> m_graph;
	ExecutionPlan m_plan;
	std::array<GraphResourceBinding, 2> m_bindings{};
	GraphResourceHandle m_color_resource{};
	GraphResourceHandle m_depth_resource{};
	GraphPassHandle m_pass{};
	RHITextureHandle m_bound_color_target{};
	RHITextureHandle m_bound_depth_target{};
};

namespace
{
WorldQuadRenderer g_world_quad_renderer;
}

export WorldQuadRenderer &GetWorldQuadRenderer() noexcept
{
	return g_world_quad_renderer;
}

}
