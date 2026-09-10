module;

#define BOOST_TEST_MODULE GraphicsParticlePassTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <span>

export module Graphics.Passes.Particles.Tests;

import Graphics.Passes.Opaque;
import Graphics.Passes.Particles;

using namespace Graphics;

class RecordingParticleCommandList final : public CommandList
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

	bool Set_Index_Buffer(RHIBufferHandle, RHIIndexFormat, std::uint32_t) noexcept override
	{
		return false;
	}

	bool Set_Draw_Constants(std::span<const std::byte> data) noexcept override
	{
		return data.size() == sizeof(ParticleFrameParameters);
	}

	bool Draw(std::uint32_t vertex_count, std::uint32_t, std::uint32_t instance_count, std::uint32_t first_instance) noexcept override
	{
		++draw_count;
		last_vertex_count = vertex_count;
		last_first_instance = first_instance;
		return vertex_count != 0 && instance_count == 1;
	}

	bool Draw_Indexed(std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t, std::uint32_t) noexcept override
	{
		return false;
	}

	std::uint32_t target_set_count = 0;
	std::uint32_t viewport_set_count = 0;
	std::uint32_t bindless_set_count = 0;
	std::uint32_t pipeline_bind_count = 0;
	std::uint32_t vertex_buffer_set_count = 0;
	std::uint32_t draw_count = 0;
	std::uint32_t last_vertex_count = 0;
	std::uint32_t last_first_instance = 0;
};

BOOST_AUTO_TEST_CASE(particle_pass_tracks_depth_writes_and_draws_billboards_after_opaque)
{
	RenderGraph graph;
	const GraphResourceHandle color_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle opaque_pass = OpaquePass::Add_To_Graph(graph, color_target, depth_target, 10);
	const GraphPassHandle particle_pass = ParticlePass::Add_To_Graph(graph, color_target, depth_target, 20);
	BOOST_REQUIRE(opaque_pass.Is_Valid());
	BOOST_REQUIRE(particle_pass.Is_Valid());

	const std::span<const GraphResourceUse> declarations = graph.Pass_Resources(particle_pass);
	BOOST_REQUIRE(declarations.size() == 2);
	BOOST_CHECK(declarations[0].access == GraphResourceAccess::Write);
	BOOST_CHECK(declarations[1].resource == depth_target);
	BOOST_CHECK(declarations[1].access == GraphResourceAccess::Write);

	ExecutionPlan plan;
	const std::array<GraphResourceBinding, 2> bindings = {
		GraphResourceBinding::Texture(color_target, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth_target, RHITextureHandle(2, 1))
	};
	BOOST_REQUIRE(plan.Compile(graph, bindings));
	BOOST_REQUIRE(plan.Passes().size() == 2);
	BOOST_CHECK(plan.Passes()[0] == opaque_pass);
	BOOST_CHECK(plan.Passes()[1] == particle_pass);

	const std::array<ParticleDrawData, 1> draws = {
		ParticleDrawData{4, 6, PipelineHandle(7, 1), 0}
	};
	const std::array<RHIBindlessResource, 1> bindless_resources = {
		RHIBindlessResource{ResourceIndex(3, 1), RHIResourceType::Material, RHIBufferHandle(8, 1), {}}
	};
	const ParticlePassInput input{
		draws,
		ParticleBillboardBinding{RHIBufferHandle(9, 1), 32, 6},
		bindless_resources,
		color_target,
		depth_target,
		{0, 0, 1280, 720, 0.0f, 1.0f}
	};

	RecordingParticleCommandList command_list;
	BOOST_REQUIRE(plan.Execute(graph, command_list, [&](GraphPassHandle current_pass, CommandList &commands, const PassResources &resources) noexcept {
		if (current_pass == opaque_pass)
			return true;
		return current_pass == particle_pass && ParticlePass::Execute(commands, resources, input);
	}));
	BOOST_CHECK(command_list.target_set_count == 1);
	BOOST_CHECK(command_list.viewport_set_count == 1);
	BOOST_CHECK(command_list.bindless_set_count == 1);
	BOOST_CHECK(command_list.pipeline_bind_count == 1);
	BOOST_CHECK(command_list.vertex_buffer_set_count == 1);
	BOOST_CHECK(command_list.draw_count == 1);
	BOOST_CHECK(command_list.last_vertex_count == 6);
	// GPU records are packed in draw order, independently of CPU particle slots.
	BOOST_CHECK(command_list.last_first_instance == 0);
}

BOOST_AUTO_TEST_CASE(particle_pass_expands_point_sprites_to_pixel_sized_quads)
{
	RenderGraph graph;
	const GraphResourceHandle color_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth_target = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle particle_pass = ParticlePass::Add_To_Graph(graph, color_target, depth_target, 20);
	BOOST_REQUIRE(particle_pass.Is_Valid());

	ExecutionPlan plan;
	const std::array<GraphResourceBinding, 2> bindings = {
		GraphResourceBinding::Texture(color_target, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth_target, RHITextureHandle(2, 1))
	};
	BOOST_REQUIRE(plan.Compile(graph, bindings));

	const std::array<ParticleDrawData, 1> draws = {
		ParticleDrawData{4, 6, PipelineHandle(7, 1), 0, true}
	};
	const std::array<RHIBindlessResource, 1> bindless_resources = {
		RHIBindlessResource{ResourceIndex(3, 1), RHIResourceType::Material, RHIBufferHandle(8, 1), {}}
	};
	const ParticlePassInput input{
		draws,
		ParticleBillboardBinding{RHIBufferHandle(9, 1), 32, 6},
		bindless_resources,
		color_target,
		depth_target,
		{0, 0, 1280, 720, 0.0f, 1.0f}
	};

	RecordingParticleCommandList command_list;
	BOOST_REQUIRE(plan.Execute(graph, command_list, [&](GraphPassHandle current_pass, CommandList &commands, const PassResources &resources) noexcept {
		return current_pass == particle_pass && ParticlePass::Execute(commands, resources, input);
	}));
	BOOST_CHECK(command_list.last_vertex_count == 6);
}

BOOST_AUTO_TEST_CASE(particle_pass_rejects_invalid_graph_resources)
{
	RenderGraph graph;
	const GraphResourceHandle color_target = graph.Create_Resource({GraphResourceKind::Texture});
	BOOST_CHECK(!ParticlePass::Add_To_Graph(graph, color_target, {}, 1).Is_Valid());
	BOOST_CHECK(!ParticlePass::Add_To_Graph(graph, color_target, graph.Create_Resource({GraphResourceKind::Buffer}), 1).Is_Valid());
}
