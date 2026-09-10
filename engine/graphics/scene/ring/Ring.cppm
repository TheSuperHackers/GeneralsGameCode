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
#include <string>
#include <utility>

export module Graphics.Scene.Ring;

import Assets.Math;
import Assets.Rings;
import Graphics.RHI;
import Graphics.Scene.Primitives.Geometry;
import Graphics.Scene.Props.Renderer;
export import Graphics.Resources.Bindless.BindlessResourceTable;
export import Graphics.Resources.Materials.Material;
export import Graphics.RenderGraph.Execution;
export import Graphics.Shaders.Library;

namespace Graphics
{

export inline constexpr std::uint32_t RingSegmentCount = 20;
export inline constexpr std::uint32_t MaxRingVertexCount = RingSegmentCount * 6;

export struct RingDescription final
{
	float center_x = 0.0f;
	float center_y = 0.0f;
	float center_z = 0.0f;
	float inner_radius = 0.0f;
	float outer_radius = 0.0f;
	std::array<float, 4> color{1.0f, 1.0f, 1.0f, 1.0f};
	std::uint32_t segments = RingSegmentCount;
};

export struct RingVertex final
{
	float position[3]{};
	float color[4]{};
	float uv[2]{};
};

static_assert(sizeof(RingVertex) == 36);

export inline constexpr std::uint32_t RingLODCount = 20;
export inline constexpr std::uint32_t RingLowestLOD = 10;
export inline constexpr std::uint32_t RingHighestLOD = 50;

export struct RingRuntimeState final
{
	Assets::Color4f color{0.75f, 0.75f, 0.75f, 1.0f};
	float alpha = 1.0f;
	Assets::Vector2f inner_scale{1.0f, 1.0f};
	Assets::Vector2f outer_scale{1.0f, 1.0f};
	float animation_time = 0.0f;
	bool visible = true;
	bool animating = false;
};

export struct RingGeometryDescription final
{
	Assets::Vector3f center{};
	Assets::Vector2f inner_extent{0.5f, 0.5f};
	Assets::Vector2f outer_extent{1.0f, 1.0f};
	std::array<float, 4> vertex_color{0.75f, 0.75f, 0.75f, 1.0f};
	float texture_tiles = 5.0f;
	std::uint32_t segments = RingHighestLOD - 2;
	bool textured = false;
};

export struct RingBounds final
{
	Assets::Vector3f center{};
	Assets::Vector3f extent{};
};

export struct RingDrawState final
{
	RingGeometryDescription geometry{};
	Assets::RingMaterialDesc material{};
	bool sort_required = false;
};

export bool Is_Valid_Ring(const RingDescription &description) noexcept
{
	if (description.segments < 3 || description.segments > RingSegmentCount)
		return false;
	if (!std::isfinite(description.center_x) || !std::isfinite(description.center_y) || !std::isfinite(description.center_z)
		|| !std::isfinite(description.inner_radius) || !std::isfinite(description.outer_radius)
		|| description.inner_radius < 0.0f || description.outer_radius <= 0.0f
		|| description.inner_radius > description.outer_radius)
		return false;
	for (const float component : description.color)
		if (!std::isfinite(component))
			return false;
	return true;
}

export std::size_t Build_Ring_Vertices(const RingDescription &description, std::span<RingVertex> output) noexcept
{
	if (!Is_Valid_Ring(description))
		return 0;

	const bool filled = description.inner_radius == 0.0f;
	const std::size_t vertices_per_segment = filled ? 3 : 6;
	const std::size_t required_vertices = static_cast<std::size_t>(description.segments) * vertices_per_segment;
	if (output.size() < required_vertices)
		return 0;

	constexpr float two_pi = 6.28318530717958647692f;
	const float delta = two_pi / static_cast<float>(description.segments);
	const auto write_vertex = [&description](RingVertex &vertex, float radius, float angle) noexcept {
		vertex.position[0] = description.center_x + radius * std::cos(angle);
		vertex.position[1] = description.center_y + radius * std::sin(angle);
		vertex.position[2] = description.center_z;
		for (std::size_t component = 0; component < description.color.size(); ++component)
			vertex.color[component] = description.color[component];
		vertex.uv[0] = 0.0f;
		vertex.uv[1] = 0.0f;
	};

	std::size_t output_index = 0;
	for (std::uint32_t segment = 0; segment < description.segments; ++segment) {
		const float current_angle = delta * static_cast<float>(segment);
		const float next_angle = segment + 1 == description.segments ? 0.0f : delta * static_cast<float>(segment + 1);
		if (filled) {
			write_vertex(output[output_index++], 0.0f, 0.0f);
			write_vertex(output[output_index++], description.outer_radius, current_angle);
			write_vertex(output[output_index++], description.outer_radius, next_angle);
		} else {
			write_vertex(output[output_index++], description.inner_radius, current_angle);
			write_vertex(output[output_index++], description.outer_radius, current_angle);
			write_vertex(output[output_index++], description.outer_radius, next_angle);
			write_vertex(output[output_index++], description.inner_radius, current_angle);
			write_vertex(output[output_index++], description.outer_radius, next_angle);
			write_vertex(output[output_index++], description.inner_radius, next_angle);
		}
	}
	return output_index;
}

export struct RingPassInput final
{
	GraphResourceHandle color_resource{};
	RHIViewport viewport{};
	PipelineHandle pipeline{};
	RHIBufferHandle vertex_buffer{};
	std::span<const RHIBindlessResource> bindless_resources{};
	std::uint32_t vertex_count = 0;
};

export class RingPass final
{
public:
	static GraphPassHandle Add_To_Graph(RenderGraph &graph, GraphResourceHandle color_resource, std::uint32_t pass_key = 0)
	{
		if (!graph.Is_Resource_Valid(color_resource) || graph.Resource_Kind(color_resource) != GraphResourceKind::Texture)
			return {};

		const std::array<GraphResourceUse, 1> uses = {GraphResourceUse::Write(color_resource)};
		return graph.Add_Pass({pass_key}, uses);
	}

	static bool Execute(CommandList &commands, const PassResources &resources, const RingPassInput &input) noexcept
	{
		const RHITextureHandle color_target = resources.Texture(input.color_resource);
		if (!color_target.Is_Valid() || !input.pipeline.Is_Valid() || !input.vertex_buffer.Is_Valid()
			|| input.vertex_count == 0 || input.vertex_count > MaxRingVertexCount
			|| input.viewport.width == 0 || input.viewport.height == 0)
			return false;

		return commands.Set_Color_Target(color_target)
			&& commands.Set_Viewport(input.viewport)
			&& commands.Set_Scissor({input.viewport.x, input.viewport.y, input.viewport.width, input.viewport.height})
			&& commands.Set_Bindless_Resources(input.bindless_resources)
			&& commands.Bind_Pipeline(input.pipeline)
			&& commands.Set_Vertex_Buffer(0, input.vertex_buffer, sizeof(RingVertex), 0)
			&& commands.Draw(input.vertex_count);
	}
};

export class RingRenderer final
{
public:
	bool Initialize(Device &device, const std::filesystem::path &shader_directory, std::uint32_t segments = RingSegmentCount)
	{
		if (m_device != nullptr || segments < 3 || segments > RingSegmentCount)
			return false;

		m_device = &device;
		m_segments = segments;
		m_graph = std::make_unique<RenderGraph>();
		m_graph->Reserve(1, 1, 1);
		m_color_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_pass = RingPass::Add_To_Graph(*m_graph, m_color_resource, 40);
		if (!m_color_resource.Is_Valid() || !m_pass.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_shader = m_shaders.Load_Ring(shader_directory);
		if (!m_shader.Is_Valid()) {
			Shutdown();
			return false;
		}
		m_pipeline = m_shaders.Create_Pipeline(device, m_shader, Make_Ring_Pipeline());
		if (!m_pipeline.Is_Valid()) {
			Shutdown();
			return false;
		}

		Material material;
		material.shader = m_shader;
		material.flags = MaterialFlags::Unlit | MaterialFlags::VertexColor;
		for (std::size_t index = 0; index < 4; ++index)
			material.parameters.values[index] = 1.0f;
		m_material = m_materials.Create(material);
		if (!m_material.Is_Valid()) {
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
		m_bindless.Reserve(1, 0, 0, 0, 1);
		if (!m_bindless.Register_Material(m_material, m_material_constants).Is_Valid()) {
			Shutdown();
			return false;
		}

		m_vertex_buffer = device.Create_Buffer({
			static_cast<std::uint32_t>(m_vertices.size() * sizeof(RingVertex)),
			RHIBufferUsage::Vertex,
			static_cast<std::uint32_t>(sizeof(RingVertex))});
		if (!m_vertex_buffer.Is_Valid()) {
			Shutdown();
			return false;
		}
		return true;
	}

	void Shutdown() noexcept
	{
		if (m_device != nullptr) {
			if (m_vertex_buffer.Is_Valid())
				m_device->Destroy_Buffer(m_vertex_buffer);
			if (m_material_constants.Is_Valid())
				m_device->Destroy_Buffer(m_material_constants);
			if (m_pipeline.Is_Valid())
				m_device->Destroy_Pipeline(m_pipeline);
		}
		m_bindless.Clear();
		if (m_material.Is_Valid())
			m_materials.Destroy(m_material);
		m_material = {};
		m_materials = {};
		if (m_shader.Is_Valid())
			m_shaders.Destroy(m_shader);
		m_shader = {};
		m_pipeline = {};
		m_vertex_buffer = {};
		m_material_constants = {};
		m_graph.reset();
		m_plan = {};
		m_bindings = {};
		m_color_resource = {};
		m_pass = {};
		m_bound_target = {};
		m_active = false;
		m_vertex_count = 0;
		m_segments = RingSegmentCount;
		m_device = nullptr;
	}

	bool Is_Initialized() const noexcept
	{
		return m_device != nullptr && m_pipeline.Is_Valid() && m_vertex_buffer.Is_Valid();
	}

	bool Set_Ring(const RingDescription &description) noexcept
	{
		if (!Is_Initialized() || description.segments != m_segments || !Is_Valid_Ring(description))
			return false;
		m_description = description;
		m_active = true;
		return true;
	}

	void Clear() noexcept
	{
		m_active = false;
	}

	bool Render(CommandList &commands, RHITextureHandle color_target, RHIViewport viewport) noexcept
	{
		if (!Is_Initialized() || !color_target.Is_Valid() || viewport.width == 0 || viewport.height == 0)
			return false;
		if (!m_active)
			return true;

		m_vertex_count = static_cast<std::uint32_t>(Build_Ring_Vertices(m_description, std::span<RingVertex>(m_vertices)));
		if (m_vertex_count == 0
			|| !m_device->Update_Buffer(m_vertex_buffer, 0, std::as_bytes(std::span<const RingVertex>(m_vertices.data(), m_vertex_count))))
			return false;

		m_bindings[0] = GraphResourceBinding::Texture(m_color_resource, color_target);
		if (!m_plan.Is_Valid() || m_bound_target != color_target) {
			if (!m_plan.Compile(*m_graph, m_bindings))
				return false;
			m_bound_target = color_target;
		}

		const RingPassInput input{
			m_color_resource,
			viewport,
			m_pipeline,
			m_vertex_buffer,
			m_bindless.Resources(),
			m_vertex_count};
		const bool rendered = m_plan.Execute(*m_graph, commands, [&](GraphPassHandle pass, CommandList &command_list, const PassResources &resources) noexcept {
			return pass == m_pass && RingPass::Execute(command_list, resources, input);
		});
		m_active = false;
		return rendered;
	}

	bool Render(RHITextureHandle color_target, RHIViewport viewport) noexcept
	{
		return m_device != nullptr && Render(m_device->Immediate_Command_List(), color_target, viewport);
	}

private:
	Device *m_device = nullptr;
	std::uint32_t m_segments = RingSegmentCount;
	RingDescription m_description{};
	std::array<RingVertex, MaxRingVertexCount> m_vertices{};
	std::uint32_t m_vertex_count = 0;
	bool m_active = false;
	ShaderLibrary m_shaders;
	ShaderHandle m_shader{};
	PipelineHandle m_pipeline{};
	MaterialPool m_materials;
	MaterialHandle m_material{};
	RHIBufferHandle m_material_constants{};
	RHIBufferHandle m_vertex_buffer{};
	BindlessResourceTable m_bindless;
	std::unique_ptr<RenderGraph> m_graph;
	ExecutionPlan m_plan;
	std::array<GraphResourceBinding, 1> m_bindings{};
	GraphResourceHandle m_color_resource{};
	GraphPassHandle m_pass{};
	RHITextureHandle m_bound_target{};
};

namespace
{
RingRenderer g_ring_renderer;
}

export RingRenderer &GetRingRenderer() noexcept
{
	return g_ring_renderer;
}

}
