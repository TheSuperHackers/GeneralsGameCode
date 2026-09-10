module;

#define BOOST_TEST_MODULE GraphicsRingTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <span>
#include <type_traits>

export module Graphics.Scene.Ring.Tests;

import Graphics.RHI;
import Graphics.RenderGraph.Execution;
import Graphics.Scene.Ring;

using namespace Graphics;

static_assert(!std::is_convertible_v<RHIBufferHandle, RHITextureHandle>);
static_assert(!std::is_convertible_v<RHITextureHandle, PipelineHandle>);
static_assert(sizeof(RingVertex) == 36);

class RecordingRingCommandList final : public CommandList
{
public:
	bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept override
	{
		++pipeline_bind_count;
		return pipeline.Is_Valid();
	}

	bool Set_Bindless_Resources(std::span<const RHIBindlessResource>) noexcept override
	{
		++bindless_set_count;
		return true;
	}

	bool Set_Render_Targets(RHITextureHandle, RHITextureHandle) noexcept override { return false; }

	bool Set_Color_Target(RHITextureHandle target) noexcept override
	{
		++color_target_set_count;
		return target.Is_Valid();
	}

	bool Set_Depth_Target(RHITextureHandle) noexcept override { return false; }
	bool Clear(const std::array<float, 4> &, float) noexcept override { return false; }
	bool Clear_Depth(float) noexcept override { return false; }

	bool Set_Viewport(RHIViewport viewport) noexcept override
	{
		++viewport_set_count;
		return viewport.width != 0 && viewport.height != 0;
	}

	bool Set_Scissor(RHIScissorRect scissor) noexcept override
	{
		++scissor_set_count;
		return scissor.width != 0 && scissor.height != 0;
	}

	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t) noexcept override
	{
		++vertex_buffer_set_count;
		return buffer.Is_Valid() && stride == sizeof(RingVertex);
	}

	bool Set_Index_Buffer(RHIBufferHandle, RHIIndexFormat, std::uint32_t) noexcept override { return false; }

	bool Draw(std::uint32_t vertex_count, std::uint32_t, std::uint32_t, std::uint32_t) noexcept override
	{
		last_vertex_count = vertex_count;
		++draw_count;
		return vertex_count != 0;
	}

	bool Draw_Indexed(std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t, std::uint32_t) noexcept override { return false; }

	std::uint32_t color_target_set_count = 0;
	std::uint32_t viewport_set_count = 0;
	std::uint32_t scissor_set_count = 0;
	std::uint32_t bindless_set_count = 0;
	std::uint32_t pipeline_bind_count = 0;
	std::uint32_t vertex_buffer_set_count = 0;
	std::uint32_t draw_count = 0;
	std::uint32_t last_vertex_count = 0;
};

BOOST_AUTO_TEST_CASE(filled_ring_geometry_is_fixed_and_deterministic)
{
	RingDescription description;
	description.center_x = 2.0f;
	description.center_y = -1.0f;
	description.outer_radius = 0.5f;
	description.color = {0.25f, 0.5f, 0.75f, 0.5f};
	std::array<RingVertex, MaxRingVertexCount> vertices{};

	const std::size_t count = Build_Ring_Vertices(description, vertices);
	BOOST_REQUIRE_EQUAL(count, RingSegmentCount * 3);
	BOOST_CHECK_EQUAL(vertices[0].position[0], description.center_x);
	BOOST_CHECK_EQUAL(vertices[0].position[1], description.center_y);
	BOOST_CHECK_CLOSE(vertices[1].position[0], description.center_x + description.outer_radius, 0.001);
	BOOST_CHECK_EQUAL(vertices[1].color[2], description.color[2]);
	BOOST_CHECK_CLOSE(vertices[count - 1].position[0], description.center_x + description.outer_radius, 0.001);
	BOOST_CHECK_CLOSE(vertices[count - 1].position[1], description.center_y, 0.001);
}

BOOST_AUTO_TEST_CASE(annulus_geometry_contains_inner_and_outer_edges)
{
	RingDescription description;
	description.inner_radius = 0.2f;
	description.outer_radius = 0.6f;
	description.segments = 8;
	std::array<RingVertex, MaxRingVertexCount> vertices{};

	const std::size_t count = Build_Ring_Vertices(description, vertices);
	BOOST_REQUIRE_EQUAL(count, description.segments * 6);
	BOOST_CHECK_CLOSE(vertices[0].position[0], description.inner_radius, 0.001);
	BOOST_CHECK_CLOSE(vertices[1].position[0], description.outer_radius, 0.001);
	BOOST_CHECK_CLOSE(vertices[3].position[0], description.inner_radius, 0.001);
}

BOOST_AUTO_TEST_CASE(invalid_ring_descriptions_and_buffers_are_rejected)
{
	RingDescription invalid;
	invalid.outer_radius = -1.0f;
	std::array<RingVertex, MaxRingVertexCount> vertices{};
	BOOST_CHECK(!Is_Valid_Ring(invalid));
	BOOST_CHECK_EQUAL(Build_Ring_Vertices(invalid, vertices), 0);
	BOOST_CHECK_EQUAL(Build_Ring_Vertices(RingDescription{0, 0, 0, 0, 1, {}, 20}, std::span<RingVertex>(vertices.data(), 2)), 0);

	RenderGraph graph;
	const GraphResourceHandle buffer = graph.Create_Resource({GraphResourceKind::Buffer});
	const GraphResourceHandle texture = graph.Create_Resource({GraphResourceKind::Texture});
	BOOST_CHECK(!RingPass::Add_To_Graph(graph, buffer).Is_Valid());
	BOOST_CHECK(RingPass::Add_To_Graph(graph, texture).Is_Valid());
}

BOOST_AUTO_TEST_CASE(ring_pass_uses_only_generic_commands)
{
	RenderGraph graph;
	const GraphResourceHandle color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle pass = RingPass::Add_To_Graph(graph, color);
	BOOST_REQUIRE(pass.Is_Valid());
	const std::array<GraphResourceBinding, 1> bindings = {
		GraphResourceBinding::Texture(color, RHITextureHandle(1, 1))};
	ExecutionPlan plan;
	BOOST_REQUIRE(plan.Compile(graph, bindings));

	const RingPassInput input{
		color,
		{0, 0, 32, 32, 0.0f, 1.0f},
		PipelineHandle(2, 1),
		RHIBufferHandle(3, 1),
		{},
		6};
	RecordingRingCommandList commands;
	BOOST_REQUIRE(plan.Execute(graph, commands, [&](GraphPassHandle current_pass, CommandList &command_list, const PassResources &resources) noexcept {
		return current_pass == pass && RingPass::Execute(command_list, resources, input);
	}));
	BOOST_CHECK_EQUAL(commands.color_target_set_count, 1);
	BOOST_CHECK_EQUAL(commands.viewport_set_count, 1);
	BOOST_CHECK_EQUAL(commands.scissor_set_count, 1);
	BOOST_CHECK_EQUAL(commands.bindless_set_count, 1);
	BOOST_CHECK_EQUAL(commands.pipeline_bind_count, 1);
	BOOST_CHECK_EQUAL(commands.vertex_buffer_set_count, 1);
	BOOST_CHECK_EQUAL(commands.draw_count, 1);
	BOOST_CHECK_EQUAL(commands.last_vertex_count, 6);
}
