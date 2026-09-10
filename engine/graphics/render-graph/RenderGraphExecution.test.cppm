module;

#define BOOST_TEST_MODULE GraphicsRenderGraphExecutionTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <span>

export module Graphics.RenderGraph.Execution.Tests;

import Graphics.RenderGraph.Execution;

using namespace Graphics;

class RecordingCommandList final : public CommandList
{
public:
	bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept override
	{
		return pipeline.Is_Valid();
	}

	bool Set_Bindless_Resources(std::span<const RHIBindlessResource>) noexcept override
	{
		return true;
	}

	bool Set_Render_Targets(RHITextureHandle color_target, RHITextureHandle depth_target) noexcept override
	{
		return color_target.Is_Valid() && depth_target.Is_Valid();
	}

	bool Set_Depth_Target(RHITextureHandle depth_target) noexcept override
	{
		return depth_target.Is_Valid();
	}

	bool Clear(const std::array<float, 4> &, float) noexcept override
	{
		return true;
	}

	bool Clear_Depth(float) noexcept override
	{
		return true;
	}

	bool Set_Viewport(RHIViewport viewport) noexcept override
	{
		return viewport.width != 0 && viewport.height != 0;
	}

	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t) noexcept override
	{
		return buffer.Is_Valid() && stride != 0;
	}

	bool Set_Index_Buffer(RHIBufferHandle buffer, RHIIndexFormat, std::uint32_t) noexcept override
	{
		return buffer.Is_Valid();
	}

	bool Draw(std::uint32_t vertex_count, std::uint32_t, std::uint32_t instance_count, std::uint32_t) noexcept override
	{
		++draw_count;
		return vertex_count != 0 && instance_count != 0;
	}

	bool Draw_Indexed(std::uint32_t index_count, std::uint32_t, std::int32_t, std::uint32_t instance_count, std::uint32_t) noexcept override
	{
		++draw_count;
		return index_count != 0 && instance_count != 0;
	}

	std::uint32_t draw_count = 0;
};

static GraphResourceHandle buffer_resource;
static GraphResourceHandle texture_resource;
static RHIBufferHandle buffer_handle(7, 1);
static RHITextureHandle texture_handle(8, 1);
static std::array<std::uint32_t, 2> execution_order{};
static std::size_t execution_count = 0;

static bool Execute_Producer(CommandList &command_list, const PassResources &resources) noexcept
{
	if (resources.Declarations().size() != 1 || !resources.Is_Declared(buffer_resource))
		return false;
	if (resources.Buffer(buffer_resource) != buffer_handle || resources.Texture(buffer_resource).Is_Valid())
		return false;
	if (resources.Buffer(texture_resource).Is_Valid() || resources.Texture(texture_resource).Is_Valid())
		return false;

	execution_order[execution_count++] = 1;
	return command_list.Draw(3, 0, 1, 0);
}

static bool Execute_Consumer(CommandList &command_list, const PassResources &resources) noexcept
{
	if (resources.Declarations().size() != 2 || !resources.Is_Declared(buffer_resource) || !resources.Is_Declared(texture_resource))
		return false;
	if (resources.Buffer(buffer_resource) != buffer_handle || resources.Texture(texture_resource) != texture_handle)
		return false;
	if (resources.Buffer(texture_resource).Is_Valid() || resources.Texture(buffer_resource).Is_Valid())
		return false;

	execution_order[execution_count++] = 2;
	return command_list.Draw_Indexed(3, 0, 0, 1, 0);
}

BOOST_AUTO_TEST_CASE(render_graph_execution_follows_compiled_order_and_restricts_resources)
{
	RenderGraph graph;
	buffer_resource = graph.Create_Resource({GraphResourceKind::Buffer});
	texture_resource = graph.Create_Resource({GraphResourceKind::Texture});
	const std::array<GraphResourceUse, 1> producer_uses = {GraphResourceUse::Write(buffer_resource)};
	const std::array<GraphResourceUse, 2> consumer_uses = {
		GraphResourceUse::Read(buffer_resource),
		GraphResourceUse::Write(texture_resource)
	};
	const GraphPassHandle producer = graph.Add_Pass({1}, producer_uses);
	const GraphPassHandle consumer = graph.Add_Pass({2}, consumer_uses);
	const std::array<GraphResourceBinding, 2> resources = {
		GraphResourceBinding::Buffer(buffer_resource, buffer_handle),
		GraphResourceBinding::Texture(texture_resource, texture_handle)
	};
	const std::array<PassExecution, 2> executions = {
		PassExecution{producer, &Execute_Producer},
		PassExecution{consumer, &Execute_Consumer}
	};

	ExecutionPlan plan;
	BOOST_REQUIRE(plan.Compile(graph, resources, executions));
	BOOST_REQUIRE(plan.Is_Valid());
	BOOST_REQUIRE(plan.Passes().size() == 2);
	BOOST_CHECK(plan.Passes()[0] == producer);
	BOOST_CHECK(plan.Passes()[1] == consumer);

	execution_count = 0;
	RecordingCommandList command_list;
	BOOST_REQUIRE(plan.Execute(graph, command_list));
	BOOST_CHECK(execution_count == 2);
	BOOST_CHECK(execution_order[0] == 1);
	BOOST_CHECK(execution_order[1] == 2);
	BOOST_CHECK(command_list.draw_count == 2);
}

BOOST_AUTO_TEST_CASE(render_graph_execution_rejects_missing_and_invalid_resources)
{
	RenderGraph graph;
	const GraphResourceHandle resource = graph.Create_Resource({GraphResourceKind::Buffer});
	const std::array<GraphResourceUse, 1> uses = {GraphResourceUse::Read(resource)};
	const GraphPassHandle pass = graph.Add_Pass({1}, uses);
	const std::array<PassExecution, 1> executions = {PassExecution{pass, &Execute_Producer}};
	const std::array<GraphResourceBinding, 0> missing_resources{};

	ExecutionPlan plan;
	BOOST_CHECK(!plan.Compile(graph, missing_resources, executions));

	const std::array<GraphResourceBinding, 1> invalid_resources = {
		GraphResourceBinding::Buffer(resource, {})
	};
	BOOST_CHECK(!plan.Compile(graph, invalid_resources, executions));

	const std::array<GraphResourceUse, 1> invalid_use = {
		GraphResourceUse::Read(GraphResourceHandle(99, 1))
	};
	BOOST_CHECK(graph.Add_Pass({2}, invalid_use) == nullptr);
}
