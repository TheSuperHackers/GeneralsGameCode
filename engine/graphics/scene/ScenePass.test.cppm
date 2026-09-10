module;

#define BOOST_TEST_MODULE ScenePassTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstdint>
#include <vector>

export module Graphics.Scene.Pass.Tests;

import Graphics.Scene.Pass;

using namespace Graphics;

namespace
{

struct Invocation final
{
	ScenePassInvocation pass;
	SceneDrawParameters parameters;
};

}

BOOST_AUTO_TEST_CASE(disabled_extra_pass_applies_fog_and_restores_scene_parameters)
{
	ScenePass pass;
	pass.Set_Fog_Enable(true);
	pass.Set_Fog_Color({0.2f, 0.4f, 0.6f});
	pass.Set_Fog_Range(12.0f, 240.0f);
	pass.Set_Polygon_Mode(ScenePolygonMode::Line);

	SceneDrawParameters parameters;
	parameters.depth_bias = 19;
	parameters.wireframe = false;
	parameters.fog.start = -3.0f;
	std::vector<Invocation> calls;
	{
		SceneDrawScope scope(parameters);
		pass.Apply_Base_State(parameters);
		const bool pre_saw_polygon_mode = parameters.wireframe;
		parameters.wireframe = false;
		const bool result = pass.Execute(parameters, false,
			[&](ScenePassInvocation invocation, SceneDrawParameters &current) {
				calls.push_back({invocation, current});
				return true;
			});

		BOOST_CHECK(result);
		BOOST_CHECK(pre_saw_polygon_mode);
		BOOST_CHECK(!parameters.wireframe);
		BOOST_CHECK_EQUAL(parameters.fog.start, 12.0f);
	}

	BOOST_REQUIRE_EQUAL(calls.size(), 1u);
	BOOST_CHECK(calls[0].pass.stage == ScenePassStage::Base);
	BOOST_CHECK(!calls[0].pass.texturing_enabled);
	BOOST_CHECK(calls[0].parameters.fog.enabled);
	BOOST_CHECK_EQUAL(calls[0].parameters.fog.start, 12.0f);
	BOOST_CHECK_EQUAL(calls[0].parameters.fog.end, 240.0f);
	const std::array<float, 4> expected_fog_color{0.2f, 0.4f, 0.6f, 1.0f};
	BOOST_CHECK(calls[0].parameters.fog.color == expected_fog_color);
	// Pre-processing can override the polygon state, and Execute leaves that
	// state available to the post-processing callback.
	BOOST_CHECK(!calls[0].parameters.wireframe);
	BOOST_CHECK_EQUAL(parameters.depth_bias, 19);
	BOOST_CHECK(!parameters.wireframe);
	BOOST_CHECK_EQUAL(parameters.fog.start, -3.0f);
}

BOOST_AUTO_TEST_CASE(line_extra_pass_disables_texturing_uses_bias_and_runs_after_base_failure)
{
	ScenePass pass;
	pass.Set_Extra_Pass_Mode(SceneExtraPassMode::Line);
	SceneDrawParameters parameters;
	parameters.depth_bias = 13;
	parameters.wireframe = true;
	std::vector<Invocation> calls;
	{
		SceneDrawScope scope(parameters);
		pass.Apply_Base_State(parameters);
		const bool result = pass.Execute(parameters, false,
			[&](ScenePassInvocation invocation, SceneDrawParameters &current) {
				calls.push_back({invocation, current});
				return invocation.stage == ScenePassStage::Extra;
			});

		BOOST_CHECK(!result);
		// The post-processing callback observes the extra-stage state before
		// the outer scene scope restores the caller's values.
		BOOST_CHECK_EQUAL(parameters.depth_bias, 7);
		BOOST_CHECK(parameters.wireframe);
	}

	BOOST_REQUIRE_EQUAL(calls.size(), 2u);
	BOOST_CHECK(calls[0].pass.stage == ScenePassStage::Base);
	BOOST_CHECK(!calls[0].pass.texturing_enabled);
	BOOST_CHECK_EQUAL(calls[0].parameters.depth_bias, 0);
	BOOST_CHECK(calls[0].parameters.wireframe == false);
	BOOST_CHECK(calls[1].pass.stage == ScenePassStage::Extra);
	BOOST_CHECK(!calls[1].pass.texturing_enabled);
	BOOST_CHECK_EQUAL(calls[1].parameters.depth_bias, 7);
	BOOST_CHECK(calls[1].parameters.wireframe);
	BOOST_CHECK_EQUAL(parameters.depth_bias, 13);
	BOOST_CHECK(parameters.wireframe);
}

BOOST_AUTO_TEST_CASE(clear_line_extra_pass_clears_before_untextured_wireframe_stage)
{
	ScenePass pass;
	pass.Set_Extra_Pass_Mode(SceneExtraPassMode::ClearLine);
	SceneDrawParameters parameters;
	std::vector<int> order;
	std::array<float, 4> clear_color{1, 1, 1, 1};
	pass.Apply_Base_State(parameters);
	BOOST_REQUIRE(pass.Execute(parameters, false,
		[&](ScenePassInvocation invocation, SceneDrawParameters &) {
			order.push_back(invocation.stage == ScenePassStage::Base ? 0 : 2);
			return true;
		},
		[&](std::array<float, 4> color) {
			order.push_back(1);
			clear_color = color;
		}));

	BOOST_REQUIRE_EQUAL(order.size(), 3u);
	const std::vector<int> expected_order{0, 1, 2};
	BOOST_CHECK(order == expected_order);
	const std::array<float, 4> expected_clear_color{0, 0, 0, 0};
	BOOST_CHECK(clear_color == expected_clear_color);
}

BOOST_AUTO_TEST_CASE(extra_pass_mode_is_refreshed_after_base_stage)
{
	ScenePass pass;
	pass.Set_Extra_Pass_Mode(SceneExtraPassMode::Line);
	SceneDrawParameters parameters;
	std::vector<ScenePassStage> calls;

	BOOST_REQUIRE(pass.Execute(parameters, true,
		[&](ScenePassInvocation invocation, SceneDrawParameters &) {
			calls.push_back(invocation.stage);
			if (invocation.stage == ScenePassStage::Base)
				pass.Set_Extra_Pass_Mode(SceneExtraPassMode::Disabled);
			return true;
		}));

	const std::vector<ScenePassStage> expected_calls{ScenePassStage::Base};
	BOOST_CHECK(calls == expected_calls);
}

BOOST_AUTO_TEST_CASE(base_stage_can_select_clear_line_extra_pass)
{
	ScenePass pass;
	pass.Set_Extra_Pass_Mode(SceneExtraPassMode::Line);
	SceneDrawParameters parameters;
	std::vector<ScenePassStage> calls;
	int clear_count = 0;

	BOOST_REQUIRE(pass.Execute(parameters, true,
		[&](ScenePassInvocation invocation, SceneDrawParameters &) {
			calls.push_back(invocation.stage);
			if (invocation.stage == ScenePassStage::Base)
				pass.Set_Extra_Pass_Mode(SceneExtraPassMode::ClearLine);
			return true;
		},
		[&](std::array<float, 4>) {
			++clear_count;
		}));

	const std::vector<ScenePassStage> expected_calls{
		ScenePassStage::Base, ScenePassStage::Extra};
	BOOST_CHECK(calls == expected_calls);
	BOOST_CHECK_EQUAL(clear_count, 1);
}
