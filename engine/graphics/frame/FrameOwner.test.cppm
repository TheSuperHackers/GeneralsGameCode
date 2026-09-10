module;

#define BOOST_TEST_MODULE GraphicsFrameTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <span>

export module Graphics.FrameOwner.Tests;

import Graphics.FrameOwner;
import Graphics.RenderGraph.Frame;

using namespace Graphics;

class TestCommandList final : public CommandList
{
public:
	bool reset_succeeds = true;
	bool targets_succeed = true;
	bool Reset_State() noexcept override
	{
		++m_reset_count;
		return reset_succeeds;
	}

	bool Bind_Pipeline(RHIPipelineHandle) noexcept override { return true; }
	bool Set_Bindless_Resources(std::span<const RHIBindlessResource>) noexcept override { return true; }
	bool Set_Render_Targets(RHITextureHandle color_target, RHITextureHandle depth_target) noexcept override
	{
		m_color_target = color_target;
		m_depth_target = depth_target;
		return targets_succeed && color_target.Is_Valid() && depth_target.Is_Valid();
	}
	bool Set_Depth_Target(RHITextureHandle) noexcept override { return true; }
	bool Clear(const std::array<float, 4> &, float) noexcept override { return true; }
	bool Clear_Depth(float) noexcept override { return true; }
	bool Set_Viewport(RHIViewport) noexcept override { return true; }
	bool Set_Vertex_Buffer(std::uint32_t, RHIBufferHandle, std::uint32_t, std::uint32_t) noexcept override { return true; }
	bool Set_Index_Buffer(RHIBufferHandle, RHIIndexFormat, std::uint32_t) noexcept override { return true; }
	bool Draw(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept override { return true; }
	bool Draw_Indexed(std::uint32_t, std::uint32_t, std::int32_t, std::uint32_t, std::uint32_t) noexcept override { return true; }

	std::uint32_t Reset_Count() const noexcept { return m_reset_count; }
	RHITextureHandle Color_Target() const noexcept { return m_color_target; }
	RHITextureHandle Depth_Target() const noexcept { return m_depth_target; }

private:
	std::uint32_t m_reset_count = 0;
	RHITextureHandle m_color_target{};
	RHITextureHandle m_depth_target{};
};

class TestSwapChain final : public SwapChain
{
public:
	bool Is_Valid() const noexcept override { return m_valid; }
	RHIBackbuffer Backbuffer() const noexcept override { return m_backbuffer; }
	RHIDepthTarget Depth_Target() const noexcept override { return m_depth_target; }

	bool Resize(std::uint32_t width, std::uint32_t height) override
	{
		if (!m_valid || width == 0 || height == 0)
			return false;

		m_backbuffer.width = width;
		m_backbuffer.height = height;
		m_depth_target.width = width;
		m_depth_target.height = height;
		return true;
	}

	bool Present() noexcept override
	{
		++m_present_count;
		return m_valid;
	}

	std::uint32_t Present_Count() const noexcept { return m_present_count; }

private:
	bool m_valid = true;
	RHIBackbuffer m_backbuffer{{1, 1}, 1280, 720};
	RHIDepthTarget m_depth_target{{2, 1}, 1280, 720};
	std::uint32_t m_present_count = 0;
};

class TestDevice final : public Device
{
public:
	bool Is_Valid() const noexcept override { return true; }
	RHIBufferHandle Create_Buffer(const RHIBuffer &) override { return {}; }
	RHITextureHandle Create_Texture(const RHITexture &) override { return {}; }
	RHIPipelineHandle Create_Pipeline(const RHIPipeline &) override { return {}; }
	bool Destroy_Buffer(RHIBufferHandle) noexcept override { return true; }
	bool Destroy_Texture(RHITextureHandle) noexcept override { return true; }
	bool Destroy_Pipeline(RHIPipelineHandle) noexcept override { return true; }
	CommandList &Immediate_Command_List() noexcept override { return m_command_list; }
	SwapChain &Get_Swap_Chain() noexcept override { return m_swap_chain; }

	bool Begin_Frame() noexcept override
	{
		if (m_frame_active)
			return false;

		m_frame_active = true;
		return true;
	}

	bool End_Frame() noexcept override
	{
		if (!m_frame_active)
			return false;

		m_frame_active = false;
		return true;
	}

	bool Is_Frame_Active() const noexcept { return m_frame_active; }
	TestSwapChain &Test_Swap_Chain() noexcept { return m_swap_chain; }

private:
	TestCommandList m_command_list;
	TestSwapChain m_swap_chain;
	bool m_frame_active = false;
};

static std::uint32_t g_graphics_executor_calls = 0;

static bool Execute_Test_Graphics_Phase(Device &device, CommandList &commands, const FrameTargets &targets) noexcept
{
	++g_graphics_executor_calls;
	return &device.Immediate_Command_List() == &commands
		&& targets.backbuffer.texture.Is_Valid()
		&& targets.depth.texture.Is_Valid();
}

BOOST_AUTO_TEST_CASE(frame_binds_current_targets_and_controls_lifecycle)
{
	RenderGraph graph;
	const GraphResourceHandle color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle depth = graph.Create_Resource({GraphResourceKind::Texture});
	std::array<GraphResourceBinding, 2> bindings{};
	TestDevice device;
	Frame frame;

	BOOST_CHECK(!frame.Is_Active());
	BOOST_CHECK(!frame.End(device));
	BOOST_REQUIRE(frame.Begin(graph, device, color, depth, bindings));
	BOOST_CHECK(frame.Is_Active());
	BOOST_CHECK(device.Is_Frame_Active());
	BOOST_CHECK(bindings[0].resource == color);
	BOOST_CHECK(bindings[1].resource == depth);
	BOOST_CHECK(bindings[0].texture == device.Get_Swap_Chain().Backbuffer().texture);
	BOOST_CHECK(bindings[1].texture == device.Get_Swap_Chain().Depth_Target().texture);
	BOOST_CHECK(!frame.Present(device));
	BOOST_REQUIRE(frame.End(device));
	BOOST_CHECK(!frame.Is_Active());
	BOOST_CHECK(!device.Is_Frame_Active());
	BOOST_REQUIRE(frame.Present(device));
	BOOST_CHECK_EQUAL(device.Test_Swap_Chain().Present_Count(), 1);
	BOOST_CHECK(!frame.Present(device));
}

BOOST_AUTO_TEST_CASE(frame_rejects_invalid_graph_targets)
{
	RenderGraph graph;
	const GraphResourceHandle color = graph.Create_Resource({GraphResourceKind::Texture});
	const GraphResourceHandle buffer = graph.Create_Resource({GraphResourceKind::Buffer});
	std::array<GraphResourceBinding, 2> bindings{};
	TestDevice device;
	Frame frame;

	BOOST_CHECK(!frame.Begin(graph, device, color, buffer, bindings));
	BOOST_CHECK(!frame.Begin(graph, device, color, {}, bindings));
}

BOOST_AUTO_TEST_CASE(frame_owner_owns_drawing_submission_and_presentation)
{
	TestDevice device;
	FrameOwner owner;

	BOOST_REQUIRE(owner.Begin_Frame(device));
	BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Drawing);
	BOOST_CHECK(owner.Targets().backbuffer.texture == device.Get_Swap_Chain().Backbuffer().texture);
	BOOST_CHECK(owner.Targets().depth.texture == device.Get_Swap_Chain().Depth_Target().texture);
	BOOST_REQUIRE(owner.Execute_Queued_Draws(device));
	BOOST_CHECK_EQUAL(static_cast<TestCommandList &>(device.Immediate_Command_List()).Reset_Count(), 1);
	BOOST_CHECK(static_cast<TestCommandList &>(device.Immediate_Command_List()).Color_Target() == owner.Targets().backbuffer.texture);
	BOOST_CHECK(static_cast<TestCommandList &>(device.Immediate_Command_List()).Depth_Target() == owner.Targets().depth.texture);
	BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Submitted);
	BOOST_REQUIRE(owner.End_Frame(device));
	BOOST_CHECK(owner.Phase() == FrameOwnerPhase::ReadyToPresent);
	BOOST_REQUIRE(owner.Present(device));
	BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Idle);
	BOOST_CHECK_EQUAL(device.Test_Swap_Chain().Present_Count(), 1);
	BOOST_CHECK_EQUAL(owner.Invalid_Operation_Count(), 0);
}

BOOST_AUTO_TEST_CASE(frame_owner_rejects_double_present_and_invalid_ownership)
{
	TestDevice device;
	FrameOwner owner;

	BOOST_CHECK(!owner.Present(device));
	BOOST_REQUIRE(owner.Begin_Frame(device));
	BOOST_CHECK(!owner.Begin_Frame(device));
	owner.Abort(device);
	BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Idle);
	BOOST_REQUIRE(owner.Begin_Frame(device));
	BOOST_REQUIRE(owner.Execute_Queued_Draws(device));
	BOOST_REQUIRE(owner.End_Frame(device));
	BOOST_REQUIRE(owner.Present(device));
	BOOST_CHECK(!owner.Present(device));
	BOOST_CHECK_EQUAL(device.Test_Swap_Chain().Present_Count(), 1);
	BOOST_CHECK_EQUAL(owner.Invalid_Operation_Count(), 3);
}

BOOST_AUTO_TEST_CASE(frame_owner_executes_registered_generic_graphics_phase)
{
	TestDevice device;
	FrameOwner owner;
	g_graphics_executor_calls = 0;

	BOOST_REQUIRE(owner.Set_Draw_Executor(&Execute_Test_Graphics_Phase));
	BOOST_REQUIRE(owner.Begin_Frame(device));
	BOOST_REQUIRE(owner.Execute_Queued_Draws(device));
	BOOST_CHECK_EQUAL(g_graphics_executor_calls, 1);
	BOOST_REQUIRE(owner.End_Frame(device));
	BOOST_REQUIRE(owner.Present(device));
}

BOOST_AUTO_TEST_CASE(frame_owner_rejects_foreign_device_phase_access)
{
	TestDevice owner_device;
	TestDevice foreign_device;
	FrameOwner owner;

	BOOST_REQUIRE(owner.Begin_Frame(owner_device));
	BOOST_CHECK(!owner.Execute_Queued_Draws(foreign_device));
	owner.Abort(owner_device);
	BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Idle);
	BOOST_CHECK_EQUAL(owner.Invalid_Operation_Count(), 1);
}

BOOST_AUTO_TEST_CASE(frame_owner_foreign_abort_preserves_active_frame)
{
	TestDevice owner_device;
	TestDevice foreign_device;
	FrameOwner owner;

	BOOST_REQUIRE(owner.Begin_Frame(owner_device));
	const FrameTargets targets = owner.Targets();
	owner.Abort(foreign_device);
	BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Drawing);
	BOOST_CHECK(owner_device.Is_Frame_Active());
	BOOST_CHECK(owner.Targets().backbuffer.texture == targets.backbuffer.texture);
	BOOST_CHECK_EQUAL(owner.Invalid_Operation_Count(), 1);
	BOOST_REQUIRE(owner.Execute_Queued_Draws(owner_device));
	BOOST_REQUIRE(owner.End_Frame(owner_device));
	owner.Abort(foreign_device);
	BOOST_CHECK(owner.Phase() == FrameOwnerPhase::ReadyToPresent);
	BOOST_CHECK_EQUAL(owner.Invalid_Operation_Count(), 2);
	BOOST_REQUIRE(owner.Present(owner_device));
}

BOOST_AUTO_TEST_CASE(offscreen_frame_can_finish_without_presenting_and_resume_window_drawing)
{
	TestDevice device;
	FrameOwner owner;
	BOOST_REQUIRE(owner.Begin_Frame(device));
	owner.Abort(device);
	BOOST_CHECK(!device.Is_Frame_Active());
	BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Idle);
	BOOST_CHECK_EQUAL(device.Test_Swap_Chain().Present_Count(), 0);
	BOOST_REQUIRE(owner.Begin_Frame(device));
	BOOST_REQUIRE(owner.Execute_Queued_Draws(device));
	BOOST_REQUIRE(owner.End_Frame(device));
	BOOST_REQUIRE(owner.Present(device));
	BOOST_CHECK_EQUAL(device.Test_Swap_Chain().Present_Count(), 1);
	BOOST_CHECK_EQUAL(owner.Invalid_Operation_Count(), 0);
}

BOOST_AUTO_TEST_CASE(failed_submission_requires_abort_and_cannot_present_partial_drawing)
{
	for (unsigned failure = 0; failure < 3; ++failure) {
		TestDevice device;
		TestDevice foreign;
		FrameOwner owner;
		auto& commands = static_cast<TestCommandList&>(device.Immediate_Command_List());
		commands.reset_succeeds = failure != 0;
		commands.targets_succeed = failure != 1;
		if (failure == 2)
			BOOST_REQUIRE(owner.Set_Draw_Executor(+[](Device&, CommandList&, const FrameTargets&) noexcept { return false; }));
		BOOST_REQUIRE(owner.Begin_Frame(device));
		BOOST_CHECK(!owner.Execute_Queued_Draws(device));
		BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Failed);
		BOOST_CHECK(!owner.End_Frame(device));
		BOOST_CHECK(!owner.Present(device));
		BOOST_CHECK_EQUAL(device.Test_Swap_Chain().Present_Count(), 0);
		owner.Abort(foreign);
		BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Failed);
		BOOST_CHECK(device.Is_Frame_Active());
		owner.Abort(device);
		BOOST_CHECK(!device.Is_Frame_Active());
		BOOST_CHECK(owner.Phase() == FrameOwnerPhase::Idle);
		commands.reset_succeeds = commands.targets_succeed = true;
		BOOST_REQUIRE(owner.Set_Draw_Executor(nullptr));
		BOOST_REQUIRE(owner.Begin_Frame(device));
		BOOST_REQUIRE(owner.Execute_Queued_Draws(device));
		BOOST_REQUIRE(owner.End_Frame(device));
		BOOST_REQUIRE(owner.Present(device));
		BOOST_CHECK_EQUAL(device.Test_Swap_Chain().Present_Count(), 1);
	}
}
