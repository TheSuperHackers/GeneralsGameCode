module;

#define BOOST_TEST_MODULE GraphicsRenderGraphTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.RenderGraph.Tests;

import Graphics.RenderGraph;

using namespace Graphics;

BOOST_AUTO_TEST_CASE(render_graph_orders_resource_dependencies)
{
	RenderGraph graph;
	const GraphResourceHandle color = graph.Create_Resource({GraphResourceKind::Texture});
	const std::array<GraphResourceUse, 1> write_color = {GraphResourceUse::Write(color)};
	const std::array<GraphResourceUse, 1> read_color = {GraphResourceUse::Read(color)};
	const GraphPassHandle producer = graph.Add_Pass({1}, write_color);
	const GraphPassHandle consumer = graph.Add_Pass({2}, read_color);

	BOOST_REQUIRE(producer != nullptr);
	BOOST_REQUIRE(consumer != nullptr);
	BOOST_REQUIRE(graph.Compile());
	BOOST_REQUIRE(graph.Is_Compiled());
	const auto order = graph.Execution_Order();
	BOOST_REQUIRE(order.size() == 2);
	BOOST_CHECK(order[0] == producer);
	BOOST_CHECK(order[1] == consumer);
}

BOOST_AUTO_TEST_CASE(render_graph_preserves_deterministic_order_for_independent_passes)
{
	RenderGraph graph;
	const GraphPassHandle first = graph.Add_Pass({1}, {});
	const GraphPassHandle second = graph.Add_Pass({2}, {});
	const GraphPassHandle third = graph.Add_Pass({3}, {});

	BOOST_REQUIRE(first != nullptr);
	BOOST_REQUIRE(second != nullptr);
	BOOST_REQUIRE(third != nullptr);
	BOOST_REQUIRE(graph.Compile());
	const auto order = graph.Execution_Order();
	BOOST_REQUIRE(order.size() == 3);
	BOOST_CHECK(order[0] == first);
	BOOST_CHECK(order[1] == second);
	BOOST_CHECK(order[2] == third);
}

BOOST_AUTO_TEST_CASE(render_graph_detects_dependency_cycles)
{
	RenderGraph graph;
	const GraphPassHandle first = graph.Add_Pass({1}, {});
	const GraphPassHandle second = graph.Add_Pass({2}, {});

	BOOST_REQUIRE(graph.Add_Dependency(first, second));
	BOOST_REQUIRE(graph.Add_Dependency(second, first));
	BOOST_CHECK(!graph.Compile());
	BOOST_CHECK(!graph.Is_Compiled());
	BOOST_CHECK(graph.Execution_Order().empty());
}
