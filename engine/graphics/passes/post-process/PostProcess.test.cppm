module;

#define BOOST_TEST_MODULE GraphicsPostProcessTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <span>
#include <type_traits>

export module Graphics.Passes.PostProcess.Tests;

import Graphics.Passes.Opaque;
import Graphics.Passes.PostProcess;

using namespace Graphics;

static_assert(std::is_trivially_copyable_v<PostProcessParameterData>);
static_assert(std::is_nothrow_move_constructible_v<PostProcessParameterData>);

class RecordingPostProcessCommandList final : public CommandList
{
public:
	bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept override
	{
		++pipeline_bind_count;
		return pipeline.Is_Valid();
	}

	bool Set_Bindless_Resources(std::span<const RHIBindlessResource> resources) noexcept override
	{
		++bindless_set_count;
		return !resources.empty();
	}

	bool Set_Render_Targets(RHITextureHandle, RHITextureHandle) noexcept override
	{
		return false;
	}

	bool Set_Color_Target(RHITextureHandle color_target) noexcept override
	{
		++color_target_set_count;
		last_color_target = color_target;
		return color_target.Is_Valid();
	}

	bool Set_Depth_Target(RHITextureHandle) noexcept override
	{
		return false;
	}

	bool Clear(const std::array<float, 4> &, float) noexcept override
	{
		return false;
	}

	bool Clear_Depth(float) noexcept override
	{
		return false;
	}

	bool Set_Viewport(RHIViewport viewport) noexcept override
	{
		++viewport_set_count;
		return viewport.width != 0 && viewport.height != 0;
	}

	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle, std::uint32_t, std::uint32_t) noexcept override
	{
		return false;
	}

	bool Set_Index_Buffer(RHIBufferHandle, RHIIndexFormat, std::uint32_t) noexcept override
	{
		return false;
	}

	bool Draw(std::uint32_t vertex_count, std::uint32_t, std::uint32_t instance_count, std::uint32_t first_instance) noexcept override
	{
		++draw_count;
		last_first_instance = first_instance;
		return vertex_count == 3 && instance_count == 1;
	}

	bool Draw_Indexed(std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t, std::uint32_t) noexcept override
	{
		return false;
	}

	std::uint32_t color_target_set_count = 0;
	std::uint32_t viewport_set_count = 0;
	std::uint32_t bindless_set_count = 0;
	std::uint32_t pipeline_bind_count = 0;
	std::uint32_t draw_count = 0;
	std::uint32_t last_first_instance = 0;
	RHITextureHandle last_color_target{};
};

BOOST_AUTO_TEST_CASE(post_process_parameters_pack_exposure_tone_map_and_gamma)
{
	const PostProcessParameters parameters{1.5f, PostProcessToneMap::ACES, 2.4f};
	const PostProcessParameterData data = Pack_Post_Process_Parameters(parameters);

	BOOST_CHECK(data.Is_Valid());
	BOOST_CHECK(data.exposure == 1.5f);
	BOOST_CHECK(data.tone_mapping == static_cast<std::uint32_t>(PostProcessToneMap::ACES));
	BOOST_CHECK(data.output_gamma == 2.4f);

	const PostProcessParameterData invalid{-1.0f, 9, 0.0f, 0};
	BOOST_CHECK(!invalid.Is_Valid());
}

BOOST_AUTO_TEST_CASE(post_process_applies_environment_exposure)
{
	RenderEnvironment environment;
	environment.exposure = 1.25f;
	const PostProcessParameters parameters = Apply_Environment_Exposure(
		{0.5f, PostProcessToneMap::ACES, 2.2f},
		environment);

	BOOST_CHECK(parameters.exposure == 1.75f);
	BOOST_CHECK(parameters.tone_mapping == PostProcessToneMap::ACES);
	BOOST_CHECK(parameters.output_gamma == 2.2f);
}

BOOST_AUTO_TEST_CASE(post_process_reads_scene_color_and_writes_backbuffer_in_order)
{
	RenderGraph graph;
	const GraphResourceHandle scene_color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle backbuffer = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle opaque_pass = OpaquePass::Add_To_Graph(graph, scene_color, depth, 10);
	const GraphPassHandle post_process_pass = PostProcessPass::Add_To_Graph(graph, scene_color, backbuffer, 20);
	BOOST_REQUIRE(opaque_pass.Is_Valid());
	BOOST_REQUIRE(post_process_pass.Is_Valid());

	const std::span<const GraphResourceUse> declarations = graph.Pass_Resources(post_process_pass);
	BOOST_REQUIRE(declarations.size() == 2);
	BOOST_CHECK(declarations[0].resource == scene_color);
	BOOST_CHECK(declarations[0].access == GraphResourceAccess::Read);
	BOOST_CHECK(declarations[1].resource == backbuffer);
	BOOST_CHECK(declarations[1].access == GraphResourceAccess::Write);

	ExecutionPlan plan;
	const std::array<GraphResourceBinding, 3> bindings = {
		GraphResourceBinding::Texture(scene_color, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth, RHITextureHandle(2, 1)),
		GraphResourceBinding::Texture(backbuffer, RHITextureHandle(3, 1))
	};
	BOOST_REQUIRE(plan.Compile(graph, bindings));
	BOOST_REQUIRE(plan.Passes().size() == 2);
	BOOST_CHECK(plan.Passes()[0] == opaque_pass);
	BOOST_CHECK(plan.Passes()[1] == post_process_pass);

	const std::array<RHIBindlessResource, 1> bindless_resources = {
		RHIBindlessResource{ResourceIndex(8, 1), RHIResourceType::Texture, {}, RHITextureHandle(1, 1)}
	};
	const PostProcessPassInput input{
		bindless_resources,
		ResourceIndex(8, 1),
		scene_color,
		backbuffer,
		{0, 0, 1280, 720, 0.0f, 1.0f},
		PipelineHandle(4, 1),
		Pack_Post_Process_Parameters({1.0f, PostProcessToneMap::Reinhard, 2.2f})
	};

	RecordingPostProcessCommandList command_list;
	BOOST_REQUIRE(plan.Execute(graph, command_list, [&](GraphPassHandle current_pass, CommandList &commands, const PassResources &resources) noexcept {
		if (current_pass == opaque_pass)
			return true;
		return current_pass == post_process_pass && PostProcessPass::Execute(commands, resources, input);
	}));
	BOOST_CHECK(command_list.color_target_set_count == 1);
	BOOST_CHECK(command_list.last_color_target == RHITextureHandle(3, 1));
	BOOST_CHECK(command_list.viewport_set_count == 1);
	BOOST_CHECK(command_list.bindless_set_count == 1);
	BOOST_CHECK(command_list.pipeline_bind_count == 1);
	BOOST_CHECK(command_list.draw_count == 1);
	BOOST_CHECK(command_list.last_first_instance == 0);
}

BOOST_AUTO_TEST_CASE(post_process_rejects_invalid_or_aliased_graph_resources)
{
	RenderGraph graph;
	const GraphResourceHandle scene_color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle buffer = graph.Create_Resource({GraphResourceKind::Buffer});

	BOOST_CHECK(!PostProcessPass::Add_To_Graph(graph, scene_color, {}, 1).Is_Valid());
	BOOST_CHECK(!PostProcessPass::Add_To_Graph(graph, scene_color, buffer, 1).Is_Valid());
	BOOST_CHECK(!PostProcessPass::Add_To_Graph(graph, scene_color, scene_color, 1).Is_Valid());
}
