module;

#define BOOST_TEST_MODULE GraphicsTransparentPassTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <span>

export module Graphics.Passes.Transparent.Tests;

import Graphics.Passes.Transparent;

using namespace Graphics;

class RecordingCommandList final : public CommandList
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
		bindless_was_set = !resources.empty();
		return true;
	}

	bool Set_Render_Targets(RHITextureHandle color_target, RHITextureHandle depth_target) noexcept override
	{
		++target_set_count;
		return color_target.Is_Valid() && depth_target.Is_Valid();
	}

	bool Set_Depth_Target(RHITextureHandle) noexcept override
	{
		return false;
	}

	bool Clear(const std::array<float, 4> &, float) noexcept override
	{
		++clear_count;
		return true;
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

	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t) noexcept override
	{
		++vertex_buffer_set_count;
		return buffer.Is_Valid() && stride != 0;
	}

	bool Set_Index_Buffer(RHIBufferHandle buffer, RHIIndexFormat, std::uint32_t) noexcept override
	{
		++index_buffer_set_count;
		return buffer.Is_Valid();
	}

	bool Draw(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept override
	{
		return false;
	}

	bool Draw_Indexed(std::uint32_t index_count, std::uint32_t, std::int32_t, std::uint32_t instance_count, std::uint32_t) noexcept override
	{
		++draw_count;
		return index_count != 0 && instance_count != 0;
	}

	std::uint32_t target_set_count = 0;
	std::uint32_t clear_count = 0;
	std::uint32_t viewport_set_count = 0;
	std::uint32_t bindless_set_count = 0;
	std::uint32_t pipeline_bind_count = 0;
	std::uint32_t vertex_buffer_set_count = 0;
	std::uint32_t index_buffer_set_count = 0;
	std::uint32_t draw_count = 0;
	bool bindless_was_set = false;
};

BOOST_AUTO_TEST_CASE(transparent_pass_is_ordered_after_opaque)
{
	RenderGraph graph;
	const GraphResourceHandle color_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle opaque_pass = OpaquePass::Add_To_Graph(graph, color_target, depth_target, 10);
	const GraphPassHandle transparent_pass = TransparentPass::Add_To_Graph(graph, color_target, depth_target, 20);
	BOOST_REQUIRE(opaque_pass.Is_Valid());
	BOOST_REQUIRE(transparent_pass.Is_Valid());

	ExecutionPlan plan;
	const std::array<GraphResourceBinding, 2> bindings = {
		GraphResourceBinding::Texture(color_target, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth_target, RHITextureHandle(2, 1))
	};
	BOOST_REQUIRE(plan.Compile(graph, bindings));
	BOOST_REQUIRE(plan.Passes().size() == 2);
	BOOST_CHECK(plan.Passes()[0] == opaque_pass);
	BOOST_CHECK(plan.Passes()[1] == transparent_pass);
}

BOOST_AUTO_TEST_CASE(transparent_pass_reuses_bindless_resources_without_clearing)
{
	RenderGraph graph;
	const GraphResourceHandle color_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle pass = TransparentPass::Add_To_Graph(graph, color_target, depth_target);
	const std::array<GraphResourceBinding, 2> bindings = {
		GraphResourceBinding::Texture(color_target, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth_target, RHITextureHandle(2, 1))
	};
	ExecutionPlan plan;
	BOOST_REQUIRE(plan.Compile(graph, bindings));

	const std::array<DrawData, 1> draws = {
		DrawData{0, 3, 0, 1, PipelineHandle(4, 1), 0}
	};
	const std::array<OpaqueMeshBinding, 1> meshes = {
		OpaqueMeshBinding{RHIBufferHandle(5, 1), RHIBufferHandle(6, 1), RHIIndexFormat::UInt32, 32, 3, 0, 0}
	};
	const std::array<RHIBindlessResource, 1> bindless_resources = {
		RHIBindlessResource{ResourceIndex(2, 1), RHIResourceType::Material, RHIBufferHandle(7, 1), {}}
	};
	const TransparentPassInput input{
		draws,
		meshes,
		bindless_resources,
		color_target,
		depth_target,
		{0, 0, 1280, 720, 0.0f, 1.0f}
	};

	RecordingCommandList command_list;
	BOOST_REQUIRE(plan.Execute(graph, command_list, [&](GraphPassHandle current_pass, CommandList &commands, const PassResources &resources) noexcept {
		return current_pass == pass && TransparentPass::Execute(commands, resources, input);
	}));
	BOOST_CHECK(command_list.target_set_count == 1);
	BOOST_CHECK(command_list.clear_count == 0);
	BOOST_CHECK(command_list.viewport_set_count == 1);
	BOOST_CHECK(command_list.bindless_set_count == 1);
	BOOST_CHECK(command_list.bindless_was_set);
	BOOST_CHECK(command_list.pipeline_bind_count == 1);
	BOOST_CHECK(command_list.vertex_buffer_set_count == 1);
	BOOST_CHECK(command_list.index_buffer_set_count == 1);
	BOOST_CHECK(command_list.draw_count == 1);
}
