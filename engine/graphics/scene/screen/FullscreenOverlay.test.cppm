module;

#define BOOST_TEST_MODULE GraphicsFullscreenOverlayTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <span>
#include <type_traits>

export module Graphics.Scene.Screen.FullscreenOverlay.Tests;

import Graphics.Scene.Screen.FullscreenOverlay;

using namespace Graphics;

static_assert(!std::is_convertible_v<RHIBufferHandle, RHITextureHandle>);
static_assert(!std::is_convertible_v<RHITextureHandle, PipelineHandle>);
static_assert(sizeof(FullscreenOverlayVertex) == 36);

class RecordingOverlayCommandList final : public CommandList
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
		return buffer.Is_Valid() && stride == sizeof(FullscreenOverlayVertex);
	}

	bool Set_Index_Buffer(RHIBufferHandle, RHIIndexFormat, std::uint32_t) noexcept override { return false; }

	bool Draw(std::uint32_t vertex_count, std::uint32_t, std::uint32_t, std::uint32_t) noexcept override
	{
		++draw_count;
		return vertex_count == 3;
	}

	bool Draw_Indexed(std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t, std::uint32_t) noexcept override { return false; }

	std::uint32_t color_target_set_count = 0;
	std::uint32_t viewport_set_count = 0;
	std::uint32_t scissor_set_count = 0;
	std::uint32_t bindless_set_count = 0;
	std::uint32_t pipeline_bind_count = 0;
	std::uint32_t vertex_buffer_set_count = 0;
	std::uint32_t draw_count = 0;
};

BOOST_AUTO_TEST_CASE(overlay_descriptions_preserve_generic_blend_modes)
{
	FullscreenOverlayDescription add;
	BOOST_CHECK(Is_Valid_Fullscreen_Overlay(add));
	BOOST_CHECK_EQUAL(static_cast<int>(Make_Fullscreen_Overlay_Pipeline().blend_mode), static_cast<int>(RHIBlendMode::Additive));

	FullscreenOverlayDescription subtract;
	subtract.blend_operation = RHIBlendOperation::ReverseSubtract;
	BOOST_CHECK(Is_Valid_Fullscreen_Overlay(subtract));

	FullscreenOverlayDescription saturate;
	saturate.blend_mode = RHIBlendMode::ColorMultiply;
	saturate.draw_count = 2;
	BOOST_CHECK(Is_Valid_Fullscreen_Overlay(saturate));
	saturate.blend_operation = RHIBlendOperation::ReverseSubtract;
	BOOST_CHECK(!Is_Valid_Fullscreen_Overlay(saturate));

	FullscreenOverlayDescription invalid;
	invalid.blend_mode = RHIBlendMode::Disabled;
	BOOST_CHECK(!Is_Valid_Fullscreen_Overlay(invalid));
}

BOOST_AUTO_TEST_CASE(overlay_pass_draw_count_is_deterministic)
{
	RenderGraph graph;
	const GraphResourceHandle color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphPassHandle pass = FullscreenOverlayPass::Add_To_Graph(graph, color, 50);
	BOOST_REQUIRE(pass.Is_Valid());
	BOOST_REQUIRE_EQUAL(graph.Pass_Resources(pass).size(), 1);
	BOOST_CHECK(graph.Pass_Resources(pass)[0].resource == color);
	BOOST_CHECK(graph.Pass_Resources(pass)[0].access == GraphResourceAccess::Write);

	const std::array<GraphResourceBinding, 1> bindings = {
		GraphResourceBinding::Texture(color, RHITextureHandle(1, 1))};
	ExecutionPlan plan;
	BOOST_REQUIRE(plan.Compile(graph, bindings));
	const FullscreenOverlayPassInput input{
		color,
		{0, 0, 32, 32, 0.0f, 1.0f},
		PipelineHandle(2, 1),
		RHIBufferHandle(3, 1),
		{},
		2};
	RecordingOverlayCommandList commands;
	BOOST_REQUIRE(plan.Execute(graph, commands, [&](GraphPassHandle current_pass, CommandList &command_list, const PassResources &resources) noexcept {
		return current_pass == pass && FullscreenOverlayPass::Execute(command_list, resources, input);
	}));
	BOOST_CHECK_EQUAL(commands.color_target_set_count, 1);
	BOOST_CHECK_EQUAL(commands.viewport_set_count, 1);
	BOOST_CHECK_EQUAL(commands.scissor_set_count, 1);
	BOOST_CHECK_EQUAL(commands.bindless_set_count, 1);
	BOOST_CHECK_EQUAL(commands.pipeline_bind_count, 1);
	BOOST_CHECK_EQUAL(commands.vertex_buffer_set_count, 1);
	BOOST_CHECK_EQUAL(commands.draw_count, 2);
}

BOOST_AUTO_TEST_CASE(overlay_pass_rejects_non_texture_graph_resources)
{
	RenderGraph graph;
	const GraphResourceHandle buffer = graph.Create_Resource({GraphResourceKind::Buffer});
	BOOST_CHECK(!FullscreenOverlayPass::Add_To_Graph(graph, buffer).Is_Valid());
}
