module;

#define BOOST_TEST_MODULE GraphicsWorldQuadsTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <limits>
#include <span>
#include <type_traits>

export module Graphics.Scene.WorldQuads.Tests;

import Graphics.Scene.WorldQuads;

using namespace Graphics;

static_assert(!std::is_convertible_v<RHIBufferHandle, RHITextureHandle>);
static_assert(!std::is_convertible_v<RHITextureHandle, MaterialHandle>);
static_assert(!std::is_convertible_v<MaterialHandle, PipelineHandle>);
static_assert(sizeof(GPUWorldQuadData) == 96);

class RecordingWorldQuadCommandList final : public CommandList
{
public:
	bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept override
	{
		++pipeline_bind_count;
		return pipeline.Is_Valid();
	}

	bool Set_Bindless_Resources(std::span<const RHIBindlessResource> resources) noexcept override
	{
		bindless_resource_count = resources.size();
		return true;
	}

	bool Set_Render_Targets(RHITextureHandle color, RHITextureHandle depth) noexcept override
	{
		++render_target_set_count;
		return color.Is_Valid() && depth.Is_Valid();
	}

	bool Set_Depth_Target(RHITextureHandle) noexcept override { return false; }
	bool Clear(const std::array<float, 4> &, float) noexcept override { return false; }
	bool Clear_Depth(float) noexcept override { return false; }

	bool Set_Viewport(RHIViewport viewport) noexcept override
	{
		++viewport_set_count;
		return viewport.width != 0 && viewport.height != 0;
	}

	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t) noexcept override
	{
		++vertex_buffer_set_count;
		return buffer.Is_Valid() && stride == sizeof(WorldQuadVertex);
	}

	bool Set_Index_Buffer(RHIBufferHandle, RHIIndexFormat, std::uint32_t) noexcept override { return false; }

	bool Draw(std::uint32_t vertex_count, std::uint32_t, std::uint32_t instance_count, std::uint32_t first_instance) noexcept override
	{
		last_vertex_count = vertex_count;
		last_instance_count = instance_count;
		last_first_instance = first_instance;
		++draw_count;
		return vertex_count == 6 && instance_count != 0;
	}

	bool Draw_Indexed(std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t, std::uint32_t) noexcept override { return false; }

	std::size_t bindless_resource_count = 0;
	std::uint32_t render_target_set_count = 0;
	std::uint32_t viewport_set_count = 0;
	std::uint32_t vertex_buffer_set_count = 0;
	std::uint32_t pipeline_bind_count = 0;
	std::uint32_t draw_count = 0;
	std::uint32_t last_vertex_count = 0;
	std::uint32_t last_instance_count = 0;
	std::uint32_t last_first_instance = 0;
};

BOOST_AUTO_TEST_CASE(world_quad_validation_preserves_corner_and_material_data)
{
	WorldQuadDescription description;
	description.material = MaterialHandle(4, 1);
	description.pipeline = PipelineHandle(7, 2);
	description.corners = {{
		{{-0.8f, -0.5f, 0.2f}},
		{{0.8f, -0.5f, 0.2f}},
		{{0.8f, 0.5f, 0.2f}},
		{{-0.8f, 0.5f, 0.2f}}
	}};
	description.color = {0.25f, 0.5f, 0.75f, 0.5f};

	BOOST_CHECK(Is_Valid_World_Quad(description));
	BOOST_CHECK_EQUAL(description.corners[0][0], -0.8f);
	BOOST_CHECK_EQUAL(description.corners[2][2], 0.2f);
	BOOST_CHECK_EQUAL(description.color[3], 0.5f);

	description.corners[1][1] = std::numeric_limits<float>::quiet_NaN();
	BOOST_CHECK(!Is_Valid_World_Quad(description));
}

BOOST_AUTO_TEST_CASE(world_quad_pass_batches_adjacent_compatible_draws)
{
	RenderGraph graph;
	const GraphResourceHandle color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle pass = WorldQuadPass::Add_To_Graph(graph, color, depth, 45);
	BOOST_REQUIRE(pass.Is_Valid());

	const std::array<GraphResourceBinding, 2> bindings = {
		GraphResourceBinding::Texture(color, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth, RHITextureHandle(2, 1))};
	ExecutionPlan plan;
	BOOST_REQUIRE(plan.Compile(graph, bindings));

	const PipelineHandle pipeline(3, 1);
	const std::array<WorldQuadDrawData, 3> draws = {{
		{0, 0, pipeline, 0, 0},
		{1, 0, pipeline, 0, 1},
		{2, 1, pipeline, 1, 2}}};
	const WorldQuadPassInput input{
		draws,
		RHIBufferHandle(5, 1),
		static_cast<std::uint32_t>(sizeof(WorldQuadVertex)),
		6,
		{},
		color,
		depth,
		{0, 0, 128, 72, 0.0f, 1.0f}};
	RecordingWorldQuadCommandList commands;
	BOOST_REQUIRE(plan.Execute(graph, commands, [&](GraphPassHandle current_pass, CommandList &command_list, const PassResources &resources) noexcept {
		return current_pass == pass && WorldQuadPass::Execute(command_list, resources, input);
	}));

	BOOST_CHECK_EQUAL(commands.render_target_set_count, 1);
	BOOST_CHECK_EQUAL(commands.viewport_set_count, 1);
	BOOST_CHECK_EQUAL(commands.vertex_buffer_set_count, 1);
	BOOST_CHECK_EQUAL(commands.pipeline_bind_count, 1);
	BOOST_CHECK_EQUAL(commands.draw_count, 1);
	BOOST_CHECK_EQUAL(commands.last_instance_count, 3);
	BOOST_CHECK_EQUAL(commands.last_first_instance, 0);
}

BOOST_AUTO_TEST_CASE(world_quad_pass_rejects_invalid_draw_records)
{
	RenderGraph graph;
	const GraphResourceHandle color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle pass = WorldQuadPass::Add_To_Graph(graph, color, depth);
	BOOST_REQUIRE(pass.Is_Valid());
 
	const std::array<GraphResourceBinding, 2> bindings = {
		GraphResourceBinding::Texture(color, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth, RHITextureHandle(2, 1))};
	ExecutionPlan plan;
	BOOST_REQUIRE(plan.Compile(graph, bindings));

	const std::array<WorldQuadDrawData, 1> draws = {{
		{Invalid_World_Quad_Index, 0, PipelineHandle(3, 1), 0, 0}}};
	const WorldQuadPassInput input{
		draws,
		RHIBufferHandle(5, 1),
		static_cast<std::uint32_t>(sizeof(WorldQuadVertex)),
		6,
		{},
		color,
		depth,
		{0, 0, 128, 72, 0.0f, 1.0f}};
	RecordingWorldQuadCommandList commands;
	BOOST_CHECK(!plan.Execute(graph, commands, [&](GraphPassHandle current_pass, CommandList &command_list, const PassResources &resources) noexcept {
		return current_pass == pass && WorldQuadPass::Execute(command_list, resources, input);
	}));
}
