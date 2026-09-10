module;

#include <array>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <span>

export module Graphics.Passes.PostProcess;

export import Graphics.RenderGraph.Execution;
export import Graphics.RHI;
export import Graphics.Scene.Environment;
export import Graphics.Shaders.Pipeline;

namespace Graphics
{

export enum class PostProcessToneMap : std::uint32_t
{
	Reinhard,
	ACES
};

export struct PostProcessParameters final
{
	float exposure = 0.0f;
	PostProcessToneMap tone_mapping = PostProcessToneMap::Reinhard;
	float output_gamma = 2.2f;
};

export struct alignas(16) PostProcessParameterData final
{
	float exposure = 0.0f;
	std::uint32_t tone_mapping = static_cast<std::uint32_t>(PostProcessToneMap::Reinhard);
	float output_gamma = 2.2f;
	std::uint32_t reserved = 0;

	bool Is_Valid() const noexcept
	{
		return std::isfinite(exposure)
			&& (tone_mapping == static_cast<std::uint32_t>(PostProcessToneMap::Reinhard)
				|| tone_mapping == static_cast<std::uint32_t>(PostProcessToneMap::ACES))
			&& std::isfinite(output_gamma)
			&& output_gamma > 0.0f;
	}
};

static_assert(sizeof(PostProcessParameterData) == 16);

export PostProcessParameterData Pack_Post_Process_Parameters(const PostProcessParameters &parameters) noexcept
{
	return {
		parameters.exposure,
		static_cast<std::uint32_t>(parameters.tone_mapping),
		parameters.output_gamma,
		0
	};
}

export PipelineDesc Make_Post_Process_Pipeline(PipelineDesc description) noexcept
{
	description.depth_test = false;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Disabled;
	return description;
}

export PostProcessParameters Apply_Environment_Exposure(PostProcessParameters parameters, const RenderEnvironment &environment) noexcept
{
	parameters.exposure += environment.exposure;
	return parameters;
}

export struct PostProcessPassInput final
{
	std::span<const RHIBindlessResource> bindless_resources{};
	ResourceIndex scene_color_index{};
	GraphResourceHandle scene_color{};
	GraphResourceHandle backbuffer{};
	RHIViewport viewport{};
	PipelineHandle pipeline{};
	PostProcessParameterData parameters{};
};

export class PostProcessPass final
{
public:
	static GraphPassHandle Add_To_Graph(RenderGraph &graph, GraphResourceHandle scene_color, GraphResourceHandle backbuffer, std::uint32_t pass_key = 0)
	{
		if (!graph.Is_Resource_Valid(scene_color) || !graph.Is_Resource_Valid(backbuffer) || scene_color == backbuffer)
			return {};
		if (graph.Resource_Kind(scene_color) != GraphResourceKind::Texture || graph.Resource_Kind(backbuffer) != GraphResourceKind::Texture)
			return {};

		const std::array<GraphResourceUse, 2> uses = {
			GraphResourceUse::Read(scene_color),
			GraphResourceUse::Write(backbuffer)
		};
		return graph.Add_Pass({pass_key}, uses);
	}

	static bool Execute(CommandList &command_list, const PassResources &resources, const PostProcessPassInput &input) noexcept
	{
		const RHITextureHandle scene_color = resources.Texture(input.scene_color);
		const RHITextureHandle backbuffer = resources.Texture(input.backbuffer);
		if (!scene_color.Is_Valid() || !backbuffer.Is_Valid() || scene_color == backbuffer || !input.pipeline.Is_Valid())
			return false;
		if (input.viewport.width == 0 || input.viewport.height == 0 || !input.parameters.Is_Valid())
			return false;
		if (!Has_Scene_Color_Binding(input.bindless_resources, input.scene_color_index, scene_color))
			return false;

		if (!command_list.Set_Color_Target(backbuffer)
			|| !command_list.Set_Viewport(input.viewport)
			|| !command_list.Set_Bindless_Resources(input.bindless_resources)
			|| !command_list.Bind_Pipeline(input.pipeline)
			|| !command_list.Draw(3, 0, 1, 0))
			return false;

		return true;
	}

private:
	static bool Has_Scene_Color_Binding(std::span<const RHIBindlessResource> resources, ResourceIndex index, RHITextureHandle texture) noexcept
	{
		if (!index.Is_Valid())
			return false;

		for (const RHIBindlessResource &resource : resources) {
			if (resource.index == index && resource.type == RHIResourceType::Texture && resource.texture == texture)
				return true;
		}
		return false;
	}
};

}
