module;

#include <array>
#include <cstdint>
#include <span>

export module Graphics.Passes.Sky;

export import Graphics.RenderGraph.Execution;
export import Graphics.RHI;
export import Graphics.Scene.Environment;
export import Graphics.Shaders.Pipeline;

namespace Graphics
{

export struct SkyPassInput final
{
	std::span<const RHIBindlessResource> bindless_resources{};
	GraphResourceHandle color_target{};
	RHIViewport viewport{};
	PipelineHandle pipeline{};
	GPUEnvironmentData environment{};
	EnvironmentViewData view{};
};

export PipelineDesc Make_Sky_Pipeline(PipelineDesc description) noexcept
{
	description.depth_test = false;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Disabled;
	return description;
}

export RHIViewport Make_Sky_Viewport(const View &view) noexcept
{
	return {
		static_cast<std::uint32_t>(view.viewport.x),
		static_cast<std::uint32_t>(view.viewport.y),
		static_cast<std::uint32_t>(view.viewport.width),
		static_cast<std::uint32_t>(view.viewport.height),
		view.viewport.min_depth,
		view.viewport.max_depth
	};
}

export SkyPassInput Make_Sky_Pass_Input(
	const View &view,
	const RenderEnvironment &environment,
	GraphResourceHandle color_target,
	PipelineHandle pipeline,
	std::span<const RHIBindlessResource> bindless_resources = {}) noexcept
{
	return {
		bindless_resources,
		color_target,
		Make_Sky_Viewport(view),
		pipeline,
		Pack_Environment_Data(environment),
		Pack_Environment_View(view)
	};
}

export class SkyPass final
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

	static bool Execute(CommandList &command_list, const PassResources &resources, const SkyPassInput &input) noexcept
	{
		const RHITextureHandle color_target = resources.Texture(input.color_target);
		if (!color_target.Is_Valid() || !input.pipeline.Is_Valid() || input.viewport.width == 0 || input.viewport.height == 0)
			return false;

		if (!command_list.Set_Color_Target(color_target)
			|| !command_list.Set_Viewport(input.viewport)
			|| !command_list.Set_Bindless_Resources(input.bindless_resources)
			|| !command_list.Bind_Pipeline(input.pipeline)
			|| !command_list.Draw(3, 0, 1, 0))
			return false;

		return true;
	}
};

}
