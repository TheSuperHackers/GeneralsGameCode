module;

#define BOOST_TEST_MODULE GraphicsSkyPassTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <span>

export module Graphics.Passes.Sky.Tests;

import Graphics.Passes.Opaque;
import Graphics.Passes.Sky;

using namespace Graphics;

class RecordingSkyCommandList final : public CommandList
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
		last_bindless_count = resources.size();
		return true;
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
	std::size_t last_bindless_count = 0;
	std::uint32_t last_first_instance = 0;
	RHITextureHandle last_color_target{};
};

BOOST_AUTO_TEST_CASE(sky_pass_orders_before_opaque_and_declares_color_write)
{
	RenderGraph graph;
	const GraphResourceHandle scene_color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle sky_pass = SkyPass::Add_To_Graph(graph, scene_color, 10);
	const GraphPassHandle opaque_pass = OpaquePass::Add_To_Graph(graph, scene_color, depth, 20);
	BOOST_REQUIRE(sky_pass.Is_Valid());
	BOOST_REQUIRE(opaque_pass.Is_Valid());

	const std::span<const GraphResourceUse> declarations = graph.Pass_Resources(sky_pass);
	BOOST_REQUIRE(declarations.size() == 1);
	BOOST_CHECK(declarations[0].resource == scene_color);
	BOOST_CHECK(declarations[0].access == GraphResourceAccess::Write);

	ExecutionPlan plan;
	const std::array<GraphResourceBinding, 2> bindings = {
		GraphResourceBinding::Texture(scene_color, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth, RHITextureHandle(2, 1))
	};
	BOOST_REQUIRE(plan.Compile(graph, bindings));
	BOOST_REQUIRE(plan.Passes().size() == 2);
	BOOST_CHECK(plan.Passes()[0] == sky_pass);
	BOOST_CHECK(plan.Passes()[1] == opaque_pass);
}

BOOST_AUTO_TEST_CASE(sky_pass_propagates_view_environment_and_records_fullscreen_draw)
{
	RenderGraph graph;
	const GraphResourceHandle scene_color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle sky_pass = SkyPass::Add_To_Graph(graph, scene_color, 1);
	BOOST_REQUIRE(sky_pass.Is_Valid());

	RenderEnvironment environment;
	environment.sky.intensity = 1.5f;
	environment.sun_direction = {0.0f, 1.0f, -1.0f};
	environment.exposure = 0.75f;
	const View view(Matrix4x4::Identity(), Matrix4x4::Identity(), {10.0f, 20.0f, 30.0f}, {0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f});
	const SkyPassInput input = Make_Sky_Pass_Input(view, environment, scene_color, PipelineHandle(3, 1));
	BOOST_CHECK(input.environment.sky_zenith_intensity[3] == 1.5f);
	BOOST_CHECK(input.environment.sun_direction_intensity[1] == 1.0f);
	BOOST_CHECK(input.environment.fog_start_end_exposure[2] == 0.75f);
	BOOST_CHECK(input.view.camera_position[0] == 10.0f);
	BOOST_CHECK(input.view.camera_position[2] == 30.0f);
	BOOST_CHECK(input.viewport.width == 1280);
	BOOST_CHECK(input.viewport.height == 720);

	const std::array<GraphResourceBinding, 1> bindings = {
		GraphResourceBinding::Texture(scene_color, RHITextureHandle(1, 1))
	};
	ExecutionPlan plan;
	BOOST_REQUIRE(plan.Compile(graph, bindings));
	RecordingSkyCommandList command_list;
	BOOST_REQUIRE(plan.Execute(graph, command_list, [&](GraphPassHandle current_pass, CommandList &commands, const PassResources &resources) noexcept {
		return current_pass == sky_pass && SkyPass::Execute(commands, resources, input);
	}));
	BOOST_CHECK(command_list.color_target_set_count == 1);
	BOOST_CHECK(command_list.last_color_target == RHITextureHandle(1, 1));
	BOOST_CHECK(command_list.viewport_set_count == 1);
	BOOST_CHECK(command_list.bindless_set_count == 1);
	BOOST_CHECK(command_list.pipeline_bind_count == 1);
	BOOST_CHECK(command_list.draw_count == 1);
	BOOST_CHECK(command_list.last_first_instance == 0);
}

BOOST_AUTO_TEST_CASE(sky_pass_rejects_invalid_target)
{
	RenderGraph graph;
	const GraphResourceHandle buffer = graph.Create_Resource({GraphResourceKind::Buffer});
	BOOST_CHECK(!SkyPass::Add_To_Graph(graph, {}, 1).Is_Valid());
	BOOST_CHECK(!SkyPass::Add_To_Graph(graph, buffer, 1).Is_Valid());
}
