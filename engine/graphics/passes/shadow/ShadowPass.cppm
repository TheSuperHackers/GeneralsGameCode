module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.Passes.Shadow;

export import Graphics.Passes.Opaque;
export import Graphics.RenderGraph.Execution;
export import Graphics.Scene.GPUScene;
export import Graphics.Scene.Shadows;

namespace Graphics
{

export struct ShadowPassInput final
{
	std::span<const DrawData> draws{};
	std::span<const OpaqueMeshBinding> meshes{};
	std::span<const GPUInstanceData> instances{};
	PipelineHandle pipeline{};
	GraphResourceHandle depth_target{};
	RHIViewport viewport{};
	float clear_depth = 1.0f;
	std::span<const OpaqueSubmeshBinding> submeshes{};
	std::span<const RHIBindlessResource> bindless_resources{};
	PipelineHandle skinned_pipeline{};
};

export class ShadowPass final
{
public:
	static GraphPassHandle Add_To_Graph(RenderGraph &graph, GraphResourceHandle depth_target, std::uint32_t pass_key = 0)
	{
		if (!graph.Is_Resource_Valid(depth_target) || graph.Resource_Kind(depth_target) != GraphResourceKind::Texture)
			return {};

		const std::array<GraphResourceUse, 1> uses = {GraphResourceUse::Write(depth_target)};
		return graph.Add_Pass({pass_key}, uses);
	}

	static bool Add_Cascades_To_Graph(
		RenderGraph &graph,
		const ShadowMapResources &resources,
		std::span<GraphPassHandle> passes,
		std::uint32_t first_pass_key = 0)
	{
		if (!resources.Is_Valid() || passes.size() < resources.Count())
			return false;

		for (std::uint32_t cascade_index = 0; cascade_index < resources.Count(); ++cascade_index)
			passes[cascade_index] = {};

		GraphPassHandle previous_pass{};
		for (std::uint32_t cascade_index = 0; cascade_index < resources.Count(); ++cascade_index) {
			const GraphPassHandle pass = Add_To_Graph(graph, resources.Target(cascade_index), first_pass_key + cascade_index);
			if (!pass.Is_Valid())
				return false;

			if (previous_pass.Is_Valid() && !graph.Add_Dependency(pass, previous_pass))
				return false;

			passes[cascade_index] = pass;
			previous_pass = pass;
		}

		return true;
	}

	static bool Execute(CommandList &command_list, const PassResources &resources, const ShadowPassInput &input) noexcept
	{
		const RHITextureHandle depth_target = resources.Texture(input.depth_target);
		if (!depth_target.Is_Valid() || !input.pipeline.Is_Valid() || input.viewport.width == 0 || input.viewport.height == 0)
			return false;

		if (!command_list.Set_Depth_Target(depth_target)
			|| !command_list.Clear_Depth(input.clear_depth)
			|| !command_list.Set_Viewport(input.viewport)
			|| (!input.bindless_resources.empty() && !command_list.Set_Bindless_Resources(input.bindless_resources)))
			return false;

		PipelineHandle bound_pipeline{};
		bool has_bound_pipeline = false;
		OpaqueMeshBinding bound_mesh{};
		bool has_bound_mesh = false;
		for (const DrawData &draw : input.draws) {
			if (draw.mesh_index >= input.meshes.size() || draw.instance_index >= input.instances.size() || draw.instance_count == 0)
				return false;

			const GPUInstanceData &instance = input.instances[draw.instance_index];
			if ((instance.flags & static_cast<std::uint32_t>(RenderInstanceFlags::CastsShadow)) == 0
				|| (instance.flags & static_cast<std::uint32_t>(RenderInstanceFlags::Hidden)) != 0)
				continue;

			const OpaqueMeshBinding &mesh = input.meshes[draw.mesh_index];
			if (!mesh.vertex_buffer.Is_Valid() || !mesh.index_buffer.Is_Valid() || mesh.vertex_stride == 0 || mesh.index_count == 0)
				return false;

			std::uint32_t index_count = mesh.index_count;
			std::uint32_t first_index = mesh.first_index;
			std::int32_t base_vertex = mesh.base_vertex;
			if (mesh.submesh_count != 0) {
				if (draw.submesh_index >= mesh.submesh_count)
					return false;
				const std::uint64_t submesh_index = static_cast<std::uint64_t>(mesh.submesh_offset) + draw.submesh_index;
				if (submesh_index >= input.submeshes.size())
					return false;
				const OpaqueSubmeshBinding &submesh = input.submeshes[static_cast<std::size_t>(submesh_index)];
				if (submesh.index_count == 0)
					return false;
				index_count = submesh.index_count;
				first_index = submesh.first_index;
				base_vertex = submesh.base_vertex;
			}

			const PipelineHandle pipeline = mesh.vertex_format == RHIVertexFormat::Position3Color4UV2Skinned
				&& input.skinned_pipeline.Is_Valid() ? input.skinned_pipeline : input.pipeline;
			if (!pipeline.Is_Valid())
				return false;
			if (!has_bound_pipeline || pipeline != bound_pipeline) {
				if (!command_list.Bind_Pipeline(pipeline))
					return false;
				bound_pipeline = pipeline;
				has_bound_pipeline = true;
			}

			const bool mesh_state_changed = !has_bound_mesh
				|| mesh.vertex_buffer != bound_mesh.vertex_buffer
				|| mesh.vertex_stride != bound_mesh.vertex_stride
				|| mesh.index_buffer != bound_mesh.index_buffer
				|| mesh.index_format != bound_mesh.index_format;
			if (mesh_state_changed) {
				if (!command_list.Set_Vertex_Buffer(0, mesh.vertex_buffer, mesh.vertex_stride, 0)
					|| !command_list.Set_Index_Buffer(mesh.index_buffer, mesh.index_format, 0))
					return false;

				bound_mesh = mesh;
				has_bound_mesh = true;
			}

			if (!command_list.Draw_Indexed(index_count, first_index, base_vertex, draw.instance_count, draw.instance_index))
				return false;
		}

		return true;
	}
};

}
