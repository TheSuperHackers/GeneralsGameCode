module;

#include <cstddef>
#include <cstdint>
#include <array>
#include <span>

export module Graphics.Passes.Video;

export import Graphics.RenderGraph.Execution;
export import Graphics.RHI;

namespace Graphics
{

export struct VideoDraw final
{
	RHIViewport viewport{};
	RHIPipelineHandle pipeline{};
	RHIBufferHandle vertex_buffer{};
	RHIBufferHandle index_buffer{};
	std::uint32_t vertex_stride = 0;
	RHIIndexFormat index_format = RHIIndexFormat::UInt16;
	std::uint32_t index_count = 0;
	std::span<const RHIBindlessResource> resources{};
};

export struct VideoPassInput final
{
	std::span<const VideoDraw> draws{};
	GraphResourceHandle color_target{};
};

export class VideoPass final
{
public:
	static GraphPassHandle Add_To_Graph(RenderGraph &graph, GraphResourceHandle color_target, std::uint32_t pass_key = 0)
	{
		if (!graph.Is_Resource_Valid(color_target) || graph.Resource_Kind(color_target) != GraphResourceKind::Texture)
			return {};

		const std::array<GraphResourceUse, 1> uses = {
			GraphResourceUse::Write(color_target)
		};
		return graph.Add_Pass({pass_key}, uses);
	}

	static bool Execute(CommandList &command_list, const PassResources &resources, const VideoPassInput &input) noexcept
	{
		const RHITextureHandle color_target = resources.Texture(input.color_target);
		if (!color_target.Is_Valid() || !command_list.Set_Color_Target(color_target))
			return false;

		for (const VideoDraw &draw : input.draws) {
			if (!draw.pipeline.Is_Valid() || !draw.vertex_buffer.Is_Valid() || !draw.index_buffer.Is_Valid()
				|| draw.vertex_stride == 0 || draw.index_count == 0 || draw.viewport.width == 0 || draw.viewport.height == 0
				|| draw.resources.empty())
				return false;

			if (!command_list.Set_Viewport(draw.viewport)
				|| !command_list.Set_Bindless_Resources(draw.resources)
				|| !command_list.Bind_Pipeline(draw.pipeline)
				|| !command_list.Set_Vertex_Buffer(0, draw.vertex_buffer, draw.vertex_stride, 0)
				|| !command_list.Set_Index_Buffer(draw.index_buffer, draw.index_format, 0)
				|| !command_list.Draw_Indexed(draw.index_count))
				return false;
		}

		return true;
	}
};

}
