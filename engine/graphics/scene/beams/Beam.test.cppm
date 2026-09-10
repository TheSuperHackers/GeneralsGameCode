module;

#define BOOST_TEST_MODULE GraphicsBeamTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>

export module Graphics.Scene.Beams.Tests;

import Graphics.Scene.Beams;

using namespace Graphics;

static_assert(std::is_nothrow_move_constructible_v<BeamDescription>);
static_assert(std::is_nothrow_move_assignable_v<BeamDescription>);

static_assert(std::is_same_v<decltype(CreateBeam(std::declval<const BeamDesc &>())), BeamHandle>);

class TestCommandList final : public CommandList
{
public:
	bool Bind_Pipeline(RHIPipelineHandle pipeline) noexcept override
	{
		Record(4);
		return pipeline.Is_Valid();
	}

	bool Set_Bindless_Resources(std::span<const RHIBindlessResource>) noexcept override
	{
		Record(3);
		return true;
	}

	bool Set_Render_Targets(RHITextureHandle color_target, RHITextureHandle depth_target) noexcept override
	{
		Record(1);
		return color_target.Is_Valid() && depth_target.Is_Valid();
	}

	bool Set_Depth_Target(RHITextureHandle) noexcept override { return true; }
	bool Clear(const std::array<float, 4> &, float) noexcept override { return true; }
	bool Clear_Depth(float) noexcept override { return true; }

	bool Set_Viewport(RHIViewport viewport) noexcept override
	{
		Record(2);
		return viewport.width != 0 && viewport.height != 0;
	}

	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle buffer, std::uint32_t stride, std::uint32_t) noexcept override
	{
		Record(5);
		return buffer.Is_Valid() && stride != 0;
	}

	bool Set_Index_Buffer(RHIBufferHandle, RHIIndexFormat, std::uint32_t) noexcept override { return true; }

	bool Draw(std::uint32_t vertex_count, std::uint32_t, std::uint32_t instance_count, std::uint32_t) noexcept override
	{
		Record(6);
		return vertex_count != 0 && instance_count != 0;
	}

	bool Draw_Indexed(std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t, std::uint32_t) noexcept override { return true; }

	std::span<const std::uint32_t> Calls() const noexcept
	{
		return {m_calls.data(), m_count};
	}

private:
	void Record(std::uint32_t call) noexcept
	{
		if (m_count < m_calls.size())
			m_calls[m_count++] = call;
	}

	std::array<std::uint32_t, 6> m_calls{};
	std::size_t m_count = 0;
};

BOOST_AUTO_TEST_CASE(beam_storage_uses_typed_handles_and_rejects_stale_handles)
{
	BeamSet beams;
	beams.Reserve(2);
	const BeamHandle first = beams.Create();
	const BeamHandle second = beams.Create();
	BOOST_REQUIRE(first.Is_Valid());
	BOOST_REQUIRE(second.Is_Valid());
	BOOST_CHECK(first != second);

	BeamDescription updated;
	updated.start = {1.0f, 2.0f, 3.0f};
	updated.end = {4.0f, 5.0f, 6.0f};
	updated.width = 2.0f;
	BOOST_REQUIRE(beams.Update(first, updated));
	const BeamData data = beams.Data();
	BOOST_CHECK_EQUAL(data.start_x[beams.Dense_Index(first)], 1.0f);
	BOOST_CHECK_EQUAL(data.end_z[beams.Dense_Index(first)], 6.0f);
	BOOST_CHECK_EQUAL(data.widths[beams.Dense_Index(first)], 2.0f);

	BOOST_REQUIRE(beams.Destroy(first));
	BOOST_CHECK(!beams.Is_Valid(first));
	BOOST_CHECK(!beams.Update(first, updated));
	const BeamHandle reused = beams.Create(updated);
	BOOST_CHECK_EQUAL(reused.Get_Index(), first.Get_Index());
	BOOST_CHECK(reused.Get_Generation() != first.Get_Generation());
	BOOST_CHECK(!beams.Is_Valid(first));
	BOOST_CHECK(beams.Is_Valid(second));
}

BOOST_AUTO_TEST_CASE(typed_beam_api_rejects_uninitialized_renderer_without_raw_handles)
{
	const BeamHandle handle = CreateBeam({});
	BOOST_CHECK(!handle.Is_Valid());
	BOOST_CHECK(!UpdateBeam(handle, {}));
	DestroyBeam(handle);
}

BOOST_AUTO_TEST_CASE(beam_vertex_generation_is_batched_and_deterministic)
{
	BeamSet beams;
	BeamDescription first;
	first.start = {-0.75f, 0.0f, 0.0f};
	first.end = {0.75f, 0.0f, 0.0f};
	first.width = 0.10f;
	first.color = {1.0f, 0.25f, 0.5f, 1.0f};
	first.opacity = 0.5f;
	beams.Create(first);
	BeamDescription disabled = first;
	disabled.flags = BeamFlags::None;
	beams.Create(disabled);

	std::array<BeamVertex, 12> vertices{};
	const std::size_t count = Build_Beam_Vertices(beams.Data(), {}, vertices);
	BOOST_CHECK_EQUAL(count, 6);
	BOOST_CHECK_EQUAL(vertices[0].position[0], -0.75f);
	BOOST_CHECK_EQUAL(vertices[0].position[1], -0.05f);
	BOOST_CHECK_EQUAL(vertices[0].color[3], 0.5f);

	std::array<BeamVertex, 12> repeat{};
	BOOST_REQUIRE_EQUAL(Build_Beam_Vertices(beams.Data(), {}, repeat), count);
	for (std::size_t index = 0; index < count; ++index) {
		BOOST_CHECK_EQUAL(vertices[index].position[0], repeat[index].position[0]);
		BOOST_CHECK_EQUAL(vertices[index].position[1], repeat[index].position[1]);
		BOOST_CHECK_EQUAL(vertices[index].color[3], repeat[index].color[3]);
	}
}

BOOST_AUTO_TEST_CASE(segmented_line_data_remains_contiguous_and_updateable)
{
	BeamSet beams;
	beams.Reserve(4);
	std::array<BeamHandle, 4> handles{};
	for (std::size_t index = 0; index < handles.size(); ++index) {
		BeamDescription segment;
		segment.start = {0.0f, 0.0f, static_cast<float>(index)};
		segment.end = {0.1f * static_cast<float>(index), 0.0f, static_cast<float>(index + 1)};
		segment.width = 0.25f;
		handles[index] = beams.Create(segment);
		BOOST_REQUIRE(handles[index].Is_Valid());
	}

	const BeamData initial = beams.Data();
	BOOST_CHECK_EQUAL(initial.Size(), handles.size());
	for (std::size_t index = 0; index < handles.size(); ++index)
		BOOST_CHECK_EQUAL(initial.start_z[beams.Dense_Index(handles[index])], static_cast<float>(index));

	BeamDescription updated;
	updated.start = {-1.0f, 2.0f, 3.0f};
	updated.end = {4.0f, 5.0f, 6.0f};
	updated.width = 0.5f;
	BOOST_REQUIRE(beams.Update(handles[2], updated));

	const BeamData result = beams.Data();
	const std::uint32_t dense_index = beams.Dense_Index(handles[2]);
	BOOST_CHECK_EQUAL(result.start_x[dense_index], -1.0f);
	BOOST_CHECK_EQUAL(result.end_z[dense_index], 6.0f);
	BOOST_CHECK_EQUAL(result.widths[dense_index], 0.5f);
}

BOOST_AUTO_TEST_CASE(beam_uv_metadata_preserves_tiling_and_scroll)
{
	BeamSet beams;
	BeamDescription description;
	description.start = {-0.5f, 0.0f, 0.0f};
	description.end = {0.5f, 0.0f, 0.0f};
	description.uv_scale = 3.0f;
	description.uv_offset = -0.25f;
	BOOST_REQUIRE(beams.Create(description).Is_Valid());

	std::array<BeamVertex, 6> vertices{};
	BOOST_REQUIRE_EQUAL(Build_Beam_Vertices(beams.Data(), {}, vertices), vertices.size());
	BOOST_CHECK_EQUAL(vertices[0].uv[1], -0.25f);
	BOOST_CHECK_EQUAL(vertices[2].uv[1], 2.75f);
	BOOST_CHECK_EQUAL(vertices[5].uv[1], 2.75f);
}

BOOST_AUTO_TEST_CASE(beam_presentation_payload_preserves_material_and_flags)
{
	BeamSet beams;
	BeamDescription description;
	description.material = MaterialHandle(7, 3);
	description.flags = BeamFlags::Enabled;
	const BeamHandle handle = beams.Create(description);
	BOOST_REQUIRE(handle.Is_Valid());

	const BeamData data = beams.Data();
	const std::uint32_t dense_index = beams.Dense_Index(handle);
	BOOST_REQUIRE_NE(dense_index, Invalid_Beam_Index);
	BOOST_CHECK(data.materials[dense_index] == description.material);
	BOOST_CHECK(data.flags[dense_index] == BeamFlags::Enabled);
}

BOOST_AUTO_TEST_CASE(beam_pass_declares_color_and_depth_writes)
{
	RenderGraph graph;
	const GraphResourceHandle color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle pass = BeamPass::Add_To_Graph(graph, color, depth, 30);
	BOOST_REQUIRE(pass.Is_Valid());
	BOOST_REQUIRE(graph.Compile());
	BOOST_REQUIRE_EQUAL(graph.Execution_Order().size(), 1);
	BOOST_CHECK(graph.Pass_Resources(pass)[0].resource == color);
	BOOST_CHECK(graph.Pass_Resources(pass)[1].resource == depth);
}

BOOST_AUTO_TEST_CASE(beam_pass_records_only_generic_rhi_commands)
{
	RenderGraph graph;
	const GraphResourceHandle color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle pass = BeamPass::Add_To_Graph(graph, color, depth);
	BOOST_REQUIRE(pass.Is_Valid());

	std::array<GraphResourceBinding, 2> bindings = {
		GraphResourceBinding::Texture(color, RHITextureHandle(1, 1)),
		GraphResourceBinding::Texture(depth, RHITextureHandle(2, 1))
	};
	ExecutionPlan plan;
	BOOST_REQUIRE(plan.Compile(graph, bindings));

	const BeamPassInput input{
		RHITextureHandle(1, 1),
		RHITextureHandle(2, 1),
		RHIBufferHandle(3, 1),
		PipelineHandle(4, 1),
		{},
		color,
		depth,
		{0, 0, 16, 16, 0.0f, 1.0f},
		3
	};
	TestCommandList commands;
	BOOST_REQUIRE(plan.Execute(graph, commands, [&](GraphPassHandle current_pass, CommandList &command_list, const PassResources &resources) noexcept {
		return current_pass == pass && BeamPass::Execute(command_list, resources, input);
	}));

	const std::array<std::uint32_t, 6> expected = {1, 2, 3, 4, 5, 6};
	BOOST_CHECK_EQUAL_COLLECTIONS(commands.Calls().begin(), commands.Calls().end(), expected.begin(), expected.end());
}
