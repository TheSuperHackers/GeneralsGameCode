module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.Passes.Particles;

export import Graphics.RenderGraph.Execution;
export import Graphics.Scene.ParticleDraw;
export import Graphics.RHI;

namespace Graphics
{

export struct ParticleBillboardBinding final
{
	RHIBufferHandle vertex_buffer{};
	std::uint32_t vertex_stride = 0;
	std::uint32_t vertex_count = 0;
};

export struct ParticleFrameParameters final
{
	std::array<float, 16> view{};
	std::array<float, 16> projection{};
	std::array<std::uint32_t, 4> particle_offset{};
};
static_assert(sizeof(ParticleFrameParameters) == 144);

export struct ParticlePassInput final
{
	std::span<const ParticleDrawData> draws{};
	ParticleBillboardBinding billboard{};
	std::span<const RHIBindlessResource> bindless_resources{};
	GraphResourceHandle color_target{};
	GraphResourceHandle depth_target{};
	RHIViewport viewport{};
	ParticleFrameParameters frame{};
};

export class ParticlePass final
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

	static bool Execute(CommandList &command_list, const PassResources &resources, const ParticlePassInput &input) noexcept
	{
		const RHITextureHandle color_target = resources.Texture(input.color_target);
		const RHITextureHandle depth_target = resources.Texture(input.depth_target);
		if (!color_target.Is_Valid() || !depth_target.Is_Valid() || input.viewport.width == 0 || input.viewport.height == 0)
			return false;
		if (!input.billboard.vertex_buffer.Is_Valid() || input.billboard.vertex_stride == 0 || input.billboard.vertex_count == 0)
			return false;
		if (!command_list.Set_Render_Targets(color_target, depth_target)
			|| !command_list.Set_Viewport(input.viewport)
			|| !command_list.Set_Bindless_Resources(input.bindless_resources)
			|| !command_list.Set_Vertex_Buffer(0, input.billboard.vertex_buffer, input.billboard.vertex_stride, 0))
			return false;

		PipelineHandle bound_pipeline{};
		bool has_bound_pipeline = false;
		for (std::size_t draw_index = 0; draw_index < input.draws.size();) {
			const ParticleDrawData &draw = input.draws[draw_index];
			if (draw.particle_index == Invalid_Particle_Index || draw.material_index == Invalid_Particle_Material_Index || !draw.pipeline.Is_Valid())
				return false;

			if (!has_bound_pipeline || draw.pipeline != bound_pipeline) {
				if (!command_list.Bind_Pipeline(draw.pipeline))
					return false;
				bound_pipeline = draw.pipeline;
				has_bound_pipeline = true;
			}

			const std::uint32_t vertex_count = input.billboard.vertex_count;
			std::size_t batch_count = 1;
			while (draw_index + batch_count < input.draws.size()) {
				const ParticleDrawData &next = input.draws[draw_index + batch_count];
				if (next.pipeline != draw.pipeline || next.point_sprite != draw.point_sprite)
					break;
				++batch_count;
			}
			// The shader's instance ID is local to this draw. Address the packed
			// particle records explicitly when a blend change starts a new batch.
			ParticleFrameParameters frame = input.frame;
			frame.particle_offset[0] = static_cast<std::uint32_t>(draw_index);
			if (!command_list.Set_Draw_Constants(std::as_bytes(std::span(&frame, 1)))
				|| !command_list.Draw(vertex_count, 0, static_cast<std::uint32_t>(batch_count), 0))
				return false;
			draw_index += batch_count;
		}

		return true;
	}
};

}
