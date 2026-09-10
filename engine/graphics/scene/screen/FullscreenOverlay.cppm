module;

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>

export module Graphics.Scene.Screen.FullscreenOverlay;

export import Graphics.Resources.Bindless.BindlessResourceTable;
export import Graphics.Resources.Materials.Material;
export import Graphics.RenderGraph.Execution;
export import Graphics.Shaders.Library;

namespace Graphics
{

export struct FullscreenOverlayDescription final
{
	std::array<float, 4> color{1.0f, 1.0f, 1.0f, 1.0f};
	RHIBlendMode blend_mode = RHIBlendMode::Additive;
	RHIBlendOperation blend_operation = RHIBlendOperation::Add;
	std::uint32_t draw_count = 1;
};

export struct FullscreenOverlayVertex final
{
	float position[3]{};
	float color[4]{};
	float uv[2]{};
};

static_assert(sizeof(FullscreenOverlayVertex) == 36);

export bool Is_Valid_Fullscreen_Overlay(const FullscreenOverlayDescription &description) noexcept
{
	for (const float component : description.color)
		if (!std::isfinite(component))
			return false;

	if (description.draw_count == 0 || description.draw_count > 2)
		return false;
	if (description.blend_mode != RHIBlendMode::Additive
		&& description.blend_mode != RHIBlendMode::Multiply
		&& description.blend_mode != RHIBlendMode::ColorMultiply)
		return false;
	return description.blend_mode == RHIBlendMode::Additive
		|| description.blend_operation == RHIBlendOperation::Add;
}

export struct FullscreenOverlayPassInput final
{
	GraphResourceHandle color_resource{};
	RHIViewport viewport{};
	PipelineHandle pipeline{};
	RHIBufferHandle vertex_buffer{};
	std::span<const RHIBindlessResource> bindless_resources{};
	std::uint32_t draw_count = 0;
};

export class FullscreenOverlayPass final
{
public:
	static GraphPassHandle Add_To_Graph(RenderGraph &graph, GraphResourceHandle color_resource, std::uint32_t pass_key = 0)
	{
		if (!graph.Is_Resource_Valid(color_resource) || graph.Resource_Kind(color_resource) != GraphResourceKind::Texture)
			return {};

		const std::array<GraphResourceUse, 1> uses = {GraphResourceUse::Write(color_resource)};
		return graph.Add_Pass({pass_key}, uses);
	}

	static bool Execute(CommandList &commands, const PassResources &resources, const FullscreenOverlayPassInput &input) noexcept
	{
		const RHITextureHandle color_target = resources.Texture(input.color_resource);
		if (!color_target.Is_Valid() || !input.pipeline.Is_Valid() || !input.vertex_buffer.Is_Valid()
			|| input.draw_count == 0 || input.draw_count > 2
			|| input.viewport.width == 0 || input.viewport.height == 0)
			return false;

		if (!commands.Set_Color_Target(color_target)
			|| !commands.Set_Viewport(input.viewport)
			|| !commands.Set_Scissor({input.viewport.x, input.viewport.y, input.viewport.width, input.viewport.height})
			|| !commands.Set_Bindless_Resources(input.bindless_resources)
			|| !commands.Bind_Pipeline(input.pipeline)
			|| !commands.Set_Vertex_Buffer(0, input.vertex_buffer, sizeof(FullscreenOverlayVertex), 0))
			return false;

		for (std::uint32_t draw = 0; draw < input.draw_count; ++draw)
			if (!commands.Draw(3))
				return false;
		return true;
	}
};

export class FullscreenOverlayRenderer final
{
public:
	bool Initialize(Device &device, const std::filesystem::path &shader_directory)
	{
		if (m_device != nullptr)
			return false;

		m_device = &device;
		m_graph = std::make_unique<RenderGraph>();
		m_graph->Reserve(1, 1, 1);
		m_color_resource = m_graph->Create_Resource({GraphResourceKind::Texture});
		m_pass = FullscreenOverlayPass::Add_To_Graph(*m_graph, m_color_resource, 50);
		if (!m_color_resource.Is_Valid() || !m_pass.Is_Valid()) {
			Shutdown();
			return false;
		}

		m_shader = m_shaders.Load_Fullscreen_Overlay(shader_directory);
		if (!m_shader.Is_Valid()) {
			Shutdown();
			return false;
		}
		const PipelineDesc base_pipeline = m_shaders.Make_Pipeline_Description(m_shader, Make_Fullscreen_Overlay_Pipeline());
		PipelineDesc reverse_subtract_pipeline = base_pipeline;
		reverse_subtract_pipeline.blend_operation = RHIBlendOperation::ReverseSubtract;
		PipelineDesc color_multiply_pipeline = base_pipeline;
		color_multiply_pipeline.blend_mode = RHIBlendMode::ColorMultiply;
		PipelineDesc multiply_pipeline = base_pipeline;
		multiply_pipeline.blend_mode = RHIBlendMode::Multiply;
		m_pipelines[0] = m_shaders.Create_Pipeline(device, m_shader, base_pipeline);
		m_pipelines[1] = m_shaders.Create_Pipeline(device, m_shader, reverse_subtract_pipeline);
		m_pipelines[2] = m_shaders.Create_Pipeline(device, m_shader, color_multiply_pipeline);
		m_pipelines[3] = m_shaders.Create_Pipeline(device, m_shader, multiply_pipeline);
		if (!m_pipelines[0].Is_Valid() || !m_pipelines[1].Is_Valid() || !m_pipelines[2].Is_Valid() || !m_pipelines[3].Is_Valid()) {
			Shutdown();
			return false;
		}

		Material material;
		material.shader = m_shader;
		material.flags = MaterialFlags::Unlit | MaterialFlags::VertexColor | MaterialFlags::Transparent;
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

		m_vertices = {{
			{{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
			{{-1.0f, 3.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
			{{3.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
		}};
		m_vertex_buffer = device.Create_Buffer_Initialized(
			{static_cast<std::uint32_t>(sizeof(m_vertices)), RHIBufferUsage::Vertex, sizeof(FullscreenOverlayVertex)},
			std::as_bytes(std::span<const FullscreenOverlayVertex>(m_vertices)));
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
			for (const PipelineHandle pipeline : m_pipelines)
				if (pipeline.Is_Valid())
					m_device->Destroy_Pipeline(pipeline);
		}
		m_bindless.Clear();
		if (m_material.Is_Valid())
			m_materials.Destroy(m_material);
		m_material = {};
		m_materials = {};
		if (m_shader.Is_Valid())
			m_shaders.Destroy(m_shader);
		m_shader = {};
		m_pipelines.fill({});
		m_vertex_buffer = {};
		m_material_constants = {};
		m_graph.reset();
		m_plan = {};
		m_bindings = {};
		m_color_resource = {};
		m_pass = {};
		m_bound_target = {};
		m_active = false;
		m_device = nullptr;
	}

	bool Is_Initialized() const noexcept
	{
		return m_device != nullptr && m_pipelines[0].Is_Valid() && m_vertex_buffer.Is_Valid();
	}

	bool Set_Overlay(const FullscreenOverlayDescription &description) noexcept
	{
		if (!Is_Initialized() || !Is_Valid_Fullscreen_Overlay(description))
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

		Material *material = m_materials.Resolve(m_material);
		if (material == nullptr)
			return false;
		for (std::size_t index = 0; index < m_description.color.size(); ++index)
			material->parameters.values[index] = m_description.color[index];
		material->Mark_Dirty();
		if (!m_device->Update_Buffer(
			m_material_constants,
			0,
			material->parameters.Bytes()))
			return false;

		m_bindings[0] = GraphResourceBinding::Texture(m_color_resource, color_target);
		if (!m_plan.Is_Valid() || m_bound_target != color_target) {
			if (!m_plan.Compile(*m_graph, m_bindings))
				return false;
			m_bound_target = color_target;
		}

		const FullscreenOverlayPassInput input{
			m_color_resource,
			viewport,
			Pipeline_For(m_description),
			m_vertex_buffer,
			m_bindless.Resources(),
			m_description.draw_count};
		if (!input.pipeline.Is_Valid())
			return false;
		const bool rendered = m_plan.Execute(*m_graph, commands, [&](GraphPassHandle pass, CommandList &command_list, const PassResources &resources) noexcept {
			return pass == m_pass && FullscreenOverlayPass::Execute(command_list, resources, input);
		});
		m_active = false;
		return rendered;
	}

	bool Render(RHITextureHandle color_target, RHIViewport viewport) noexcept
	{
		return m_device != nullptr && Render(m_device->Immediate_Command_List(), color_target, viewport);
	}

private:
	PipelineHandle Pipeline_For(const FullscreenOverlayDescription &description) const noexcept
	{
		if (description.blend_mode == RHIBlendMode::Multiply)
			return m_pipelines[3];
		if (description.blend_mode == RHIBlendMode::ColorMultiply)
			return m_pipelines[2];
		return description.blend_operation == RHIBlendOperation::ReverseSubtract ? m_pipelines[1] : m_pipelines[0];
	}

	Device *m_device = nullptr;
	FullscreenOverlayDescription m_description{};
	std::array<FullscreenOverlayVertex, 3> m_vertices{};
	bool m_active = false;
	ShaderLibrary m_shaders;
	ShaderHandle m_shader{};
	std::array<PipelineHandle, 4> m_pipelines{};
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
FullscreenOverlayRenderer g_fullscreen_overlay_renderer;
}

export FullscreenOverlayRenderer &GetFullscreenOverlayRenderer() noexcept
{
	return g_fullscreen_overlay_renderer;
}

}
