module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.Passes.Transparent;

export import Graphics.Passes.Opaque;
export import Graphics.RenderGraph.Execution;
export import Graphics.Scene.Transparency;

namespace Graphics
{

export struct TransparentPassInput final
{
	std::span<const DrawData> draws{};
	std::span<const OpaqueMeshBinding> meshes{};
	std::span<const RHIBindlessResource> bindless_resources{};
	GraphResourceHandle color_target{};
	GraphResourceHandle depth_target{};
	RHIViewport viewport{};
	std::span<const OpaqueSubmeshBinding> submeshes{};
	bool uses_gpu_draw_table = false;
};

export class TransparentPass final
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
			GraphResourceUse::Write(depth_target)
		};
		return graph.Add_Pass({pass_key}, uses);
	}

	static bool Execute(CommandList &command_list, const PassResources &resources, const TransparentPassInput &input) noexcept
	{
		const RHITextureHandle color_target = resources.Texture(input.color_target);
		const RHITextureHandle depth_target = resources.Texture(input.depth_target);
		if (!color_target.Is_Valid() || !depth_target.Is_Valid() || input.viewport.width == 0 || input.viewport.height == 0)
			return false;

		if (!command_list.Set_Render_Targets(color_target, depth_target)
			|| !command_list.Set_Viewport(input.viewport)
			|| !command_list.Set_Bindless_Resources(input.bindless_resources))
			return false;

		PipelineHandle bound_pipeline{};
		bool has_bound_pipeline = false;
		OpaqueMeshBinding bound_mesh{};
		bool has_bound_mesh = false;
		for (const DrawData &draw : input.draws) {
			if (draw.mesh_index >= input.meshes.size() || draw.material_index == Invalid_GPU_Index || draw.instance_index == Invalid_GPU_Index || draw.instance_count == 0 || !draw.pipeline.Is_Valid())
				return false;

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

			if (!has_bound_pipeline || draw.pipeline != bound_pipeline) {
				if (!command_list.Bind_Pipeline(draw.pipeline))
					return false;
				bound_pipeline = draw.pipeline;
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

			const std::uint32_t first_instance = input.uses_gpu_draw_table ? draw.gpu_draw_index : draw.instance_index;
			if (input.uses_gpu_draw_table && first_instance == Invalid_GPU_Index)
				return false;
			const std::array<std::uint32_t, 4> draw_constants = {first_instance, 0, 0, 0};
			if (!command_list.Set_Draw_Constants(std::as_bytes(std::span<const std::uint32_t>(draw_constants))))
				return false;
			if (!command_list.Draw_Indexed(index_count, first_index, base_vertex, draw.instance_count, first_instance))
				return false;
		}

		return true;
	}
};

}
