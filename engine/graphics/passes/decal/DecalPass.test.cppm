module;

#define BOOST_TEST_MODULE GraphicsDecalPassTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <span>

export module Graphics.Passes.Decal.Tests;

import Graphics.Passes.Decal;
import Graphics.Passes.Opaque;

using namespace Graphics;

class RecordingDecalCommandList final : public CommandList
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

	bool Draw_Indexed(std::uint32_t index_count, std::uint32_t, std::int32_t, std::uint32_t instance_count, std::uint32_t first_instance) noexcept override
	{
		++draw_count;
		last_first_instance = first_instance;
		return index_count != 0 && instance_count == 1;
	}

	std::uint32_t target_set_count = 0;
	std::uint32_t viewport_set_count = 0;
	std::uint32_t bindless_set_count = 0;
	std::uint32_t pipeline_bind_count = 0;
	std::uint32_t vertex_buffer_set_count = 0;
	std::uint32_t index_buffer_set_count = 0;
	std::uint32_t draw_count = 0;
	std::uint32_t last_first_instance = 0;
};

BOOST_AUTO_TEST_CASE(decal_pass_reads_opaque_depth_and_writes_color)
{
	RenderGraph graph;
	const GraphResourceHandle color_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle opaque_pass = OpaquePass::Add_To_Graph(graph, color_target, depth_target, 10);
	const GraphPassHandle pass = DecalPass::Add_To_Graph(graph, color_target, depth_target, 20);
	BOOST_REQUIRE(opaque_pass.Is_Valid());
	BOOST_REQUIRE(pass.Is_Valid());

	const std::span<const GraphResourceUse> declarations = graph.Pass_Resources(pass);
	BOOST_REQUIRE(declarations.size() == 2);
	BOOST_CHECK(declarations[0].resource == color_target);
	BOOST_CHECK(declarations[0].access == GraphResourceAccess::Write);
	BOOST_CHECK(declarations[1].resource == depth_target);
	BOOST_CHECK(declarations[1].access == GraphResourceAccess::Read);

	ExecutionPlan plan;
	const std::array<GraphResourceBinding, 2> bindings = {
		GraphResourceBinding::Texture(color_target, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth_target, RHITextureHandle(2, 1))
	};
	BOOST_REQUIRE(plan.Compile(graph, bindings));
	BOOST_REQUIRE(plan.Passes().size() == 2);
	BOOST_CHECK(plan.Passes()[0] == opaque_pass);
	BOOST_CHECK(plan.Passes()[1] == pass);

	const std::array<DecalDrawData, 1> draws = {
		DecalDrawData{4, 6, PipelineHandle(7, 1), 0}
	};
	const std::array<RHIBindlessResource, 1> bindless_resources = {
		RHIBindlessResource{ResourceIndex(3, 1), RHIResourceType::Material, RHIBufferHandle(8, 1), {}}
	};
	const DecalPassInput input{
		draws,
		DecalVolumeBinding{RHIBufferHandle(9, 1), RHIBufferHandle(10, 1), RHIIndexFormat::UInt16, 32, 36, 0, 0},
		bindless_resources,
		color_target,
		depth_target,
		{0, 0, 1280, 720, 0.0f, 1.0f}
	};

	RecordingDecalCommandList command_list;
	BOOST_REQUIRE(plan.Execute(graph, command_list, [&](GraphPassHandle current_pass, CommandList &commands, const PassResources &resources) noexcept {
		if (current_pass == opaque_pass)
			return true;
		return current_pass == pass && DecalPass::Execute(commands, resources, input);
	}));
	BOOST_CHECK(command_list.target_set_count == 1);
	BOOST_CHECK(command_list.viewport_set_count == 1);
	BOOST_CHECK(command_list.bindless_set_count == 1);
	BOOST_CHECK(command_list.pipeline_bind_count == 1);
	BOOST_CHECK(command_list.vertex_buffer_set_count == 1);
	BOOST_CHECK(command_list.index_buffer_set_count == 1);
	BOOST_CHECK(command_list.draw_count == 1);
	BOOST_CHECK(command_list.last_first_instance == 4);
}

BOOST_AUTO_TEST_CASE(decal_pass_rejects_invalid_graph_resources)
{
	RenderGraph graph;
	const GraphResourceHandle color_target = graph.Create_Resource({GraphResourceKind::Texture});
	BOOST_CHECK(!DecalPass::Add_To_Graph(graph, color_target, {}, 1).Is_Valid());
	BOOST_CHECK(!DecalPass::Add_To_Graph(graph, color_target, graph.Create_Resource({GraphResourceKind::Buffer}), 1).Is_Valid());
}
