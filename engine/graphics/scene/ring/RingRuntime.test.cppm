module;

#define BOOST_TEST_MODULE GraphicsAuthoredRingRuntimeTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

export module Graphics.Scene.Ring.Runtime.Tests;

import Assets.Math;
import Assets.Rings;
import Graphics.Scene.Primitives.Geometry;
import Graphics.Scene.Ring.Runtime;

namespace
{

Assets::RingAssetDesc Make_Description()
{
	Assets::RingAssetDesc description;
	description.name = "asymmetric-ring";
	description.center = {3.0f, -2.0f, 7.0f};
	description.extent = {9.0f, 8.0f, 7.0f};
	description.default_color = {0.2f, 0.4f, 0.6f, 1.0f};
	description.default_alpha = 0.7f;
	description.default_inner_scale = {1.5f, 0.5f};
	description.default_outer_scale = {1.5f, 0.5f};
	description.inner_extent = {0.25f, 0.75f};
	description.outer_extent = {2.0f, 1.25f};
	description.texture_name = "ring.tga";
	description.texture_tile_count = 7;
	return description;
}

void Check_Close(float actual, float expected, float tolerance = 0.0001f)
{
	BOOST_CHECK_SMALL(actual - expected, tolerance);
}

}

BOOST_AUTO_TEST_CASE(asymmetric_annulus_draw_data_retains_lod_uv_bounds_and_quantized_color)
{
	Graphics::AuthoredRingRuntime runtime(Make_Description());
	Graphics::AuthoredRingDrawData draw;
	BOOST_REQUIRE(runtime.Build_Draw_Data(draw));

	BOOST_CHECK(draw.geometry.segments == 48);
	BOOST_CHECK(draw.geometry.vertices.size() == 98);
	BOOST_CHECK(draw.geometry.indices.size() == 48u * 6u);
	BOOST_CHECK(draw.geometry.texture_tiles == 7.0f);
	BOOST_CHECK(draw.textured);
	BOOST_CHECK(draw.bounds.center.x == 3.0f);
	BOOST_CHECK(draw.bounds.center.y == -2.0f);
	BOOST_CHECK(draw.bounds.center.z == 7.0f);
	Check_Close(draw.bounds.extent.x, 9.0f);
	Check_Close(draw.bounds.extent.y, 8.0f);
	Check_Close(draw.bounds.extent.z, 7.0f);

	// The first pair is on the positive Y axis. The next pair proves that X
	// and Y use their own authored extents and scales.
	const auto vertices = draw.geometry.vertices;
	Check_Close(vertices[0].position[0], 0.0f);
	Check_Close(vertices[0].position[1], 0.375f);
	Check_Close(vertices[1].position[0], 0.0f);
	Check_Close(vertices[1].position[1], 0.625f);
	Check_Close(vertices[2].position[0], -0.0489473f);
	Check_Close(vertices[2].position[1], 0.3717918f);
	Check_Close(vertices[3].position[0], -0.3915786f);
	Check_Close(vertices[3].position[1], 0.6196530f);
	Check_Close(vertices[0].uv[0], 0.0f);
	Check_Close(vertices[2].uv[0], 7.0f / 48.0f);
	Check_Close(vertices[2].uv[1], 0.0f);
	Check_Close(vertices[3].uv[1], 1.0f);

	// These are independent wire-to-pixel expectations: 0.2, 0.4, 0.6,
	// and 0.7 round to 51, 102, 153, and 179 respectively.
	Check_Close(draw.vertex_color.r, 51.0f / 255.0f);
	Check_Close(draw.vertex_color.g, 102.0f / 255.0f);
	Check_Close(draw.vertex_color.b, 153.0f / 255.0f);
	Check_Close(draw.vertex_color.a, 179.0f / 255.0f);
	Check_Close(vertices[0].color[0], 51.0f / 255.0f);
	Check_Close(vertices[0].color[3], 179.0f / 255.0f);
	const auto *cached_vertices = draw.geometry.vertices.data();
	BOOST_REQUIRE(runtime.Build_Draw_Data(draw));
	BOOST_CHECK(draw.geometry.vertices.data() == cached_vertices);

	runtime.Set_LOD_Level(1);
	BOOST_REQUIRE(runtime.Build_Draw_Data(draw));
	BOOST_CHECK(draw.geometry.segments == 10);
	BOOST_CHECK(draw.geometry.vertices.size() == 22);
	BOOST_CHECK(draw.geometry.indices.size() == 60);
	BOOST_CHECK(runtime.Num_Polys() == 20);
	runtime.Set_LOD_Level(0);
	BOOST_CHECK(!runtime.Build_Draw_Data(draw));
	BOOST_CHECK(runtime.Value() == (std::numeric_limits<float>::max)());

	runtime.Set_LOD_Level(20);
	BOOST_CHECK(runtime.Post_Increment_Value() == -1.0f);
	runtime.Set_Outer_Scale({2.0f, 3.0f});
	const auto bounds = runtime.Bounds();
	Check_Close(bounds.extent.x, 9.0f);
	Check_Close(bounds.extent.y, 8.0f);
	Check_Close(bounds.extent.z, 7.0f);
	runtime.Set_Outer_Extent({2.0f, 1.25f});
	const auto reset_bounds = runtime.Bounds();
	Check_Close(reset_bounds.extent.x, 2.0f);
	Check_Close(reset_bounds.extent.y, 1.25f);
	Check_Close(reset_bounds.extent.z, 0.0f);
	runtime.Set_Visible(false);
	BOOST_CHECK(!runtime.Is_Visible());
	BOOST_CHECK(runtime.Is_Authored_Visible());
	BOOST_REQUIRE(runtime.Build_Draw_Data(draw));
	runtime.Set_Visible(true);
}

BOOST_AUTO_TEST_CASE(animation_interpolates_extrapolates_and_holds_with_legacy_loop_boundaries)
{
	Assets::RingAssetDesc description;
	description.animation_duration = 2.0f;
	description.animation_loop = true;
	description.default_color = {0.1f, 0.1f, 0.1f, 1.0f};
	description.color_track.keys = {
		{0.0f, {0.1f, 0.2f, 0.3f, 1.0f}},
		{1.0f, {0.5f, 0.6f, 0.7f, 1.0f}}};
	description.alpha_track.keys = {{0.0f, 0.25f}, {1.0f, 0.75f}};
	description.inner_scale_track.keys = {{0.0f, {1.0f, 2.0f}}, {1.0f, {3.0f, 4.0f}}};
	description.outer_scale_track.keys = {{0.0f, {2.0f, 1.0f}}, {1.0f, {4.0f, 3.0f}}};

	Graphics::AuthoredRingRuntime runtime(description);
	runtime.Start_Animating();
	runtime.Advance(1.0f);
	const auto midpoint = runtime.State();
	Check_Close(midpoint.animation_time, 0.5f);
	Check_Close(midpoint.color.r, 0.3f);
	Check_Close(midpoint.color.g, 0.4f);
	Check_Close(midpoint.color.b, 0.5f);
	Check_Close(midpoint.alpha, 0.5f);
	Check_Close(midpoint.inner_scale.x, 2.0f);
	Check_Close(midpoint.inner_scale.y, 3.0f);
	Check_Close(midpoint.outer_scale.x, 3.0f);
	Check_Close(midpoint.outer_scale.y, 2.0f);
	Check_Close(runtime.Bounds().extent.x, 3.0f);
	Check_Close(runtime.Bounds().extent.y, 2.0f);

	// A time before the first authored key uses the first interval as an
	// extrapolation.  With keys at .25 and .75, time .1 gives fraction -.3.
	Assets::RingAssetDesc extrapolated_description;
	extrapolated_description.animation_duration = 1.0f;
	extrapolated_description.color_track.keys = {
		{0.25f, {0.2f, 0.4f, 0.6f, 1.0f}},
		{0.75f, {0.8f, 0.6f, 0.4f, 1.0f}}};
	Graphics::AuthoredRingRuntime extrapolated(extrapolated_description);
	extrapolated.Start_Animating();
	extrapolated.Advance(0.1f);
	Check_Close(extrapolated.State().animation_time, 0.1f);
	Check_Close(extrapolated.State().color.r, 0.02f);
	Check_Close(extrapolated.State().color.g, 0.34f);
	Check_Close(extrapolated.State().color.b, 0.66f);

	// Exactly one is not wrapped. A delta that produces 2.25 is reduced once,
	// leaving 1.25 and therefore still holding the final key.
	Graphics::AuthoredRingRuntime boundary(description);
	boundary.Start_Animating();
	boundary.Advance(2.0f);
	BOOST_CHECK(boundary.State().animation_time == 1.0f);
	Check_Close(boundary.State().color.r, 0.5f);
	boundary.Stop_Animating();
	boundary.Start_Animating();
	boundary.Advance(4.5f);
	Check_Close(boundary.State().animation_time, 1.25f);
	Check_Close(boundary.State().color.r, 0.5f);
}

BOOST_AUTO_TEST_CASE(no_channels_duration_zero_and_visibility_control_match_lifecycle)
{
	Assets::RingAssetDesc empty;
	empty.animation_duration = 2.0f;
	Graphics::AuthoredRingRuntime no_channels(empty);
	no_channels.Start_Animating();
	no_channels.Advance(100.0f);
	BOOST_CHECK(no_channels.State().animation_time == 0.0f);

	Assets::RingAssetDesc instant;
	instant.animation_duration = 0.0f;
	instant.color_track.keys = {{0.0f, {0.25f, 0.5f, 0.75f, 1.0f}},
		{1.0f, {0.75f, 0.5f, 0.25f, 1.0f}}};
	Graphics::AuthoredRingRuntime duration_zero(instant);
	duration_zero.Start_Animating();
	duration_zero.Advance(0.0f);
	BOOST_CHECK(duration_zero.State().animation_time == 1.0f);
	Check_Close(duration_zero.State().color.r, 0.75f);

	Graphics::AuthoredRingRuntime lifecycle(instant);
	lifecycle.Start_Animating();
	lifecycle.Advance(0.25f);
	lifecycle.Set_Hidden(true);
	BOOST_CHECK(lifecycle.Is_Visible());
	BOOST_CHECK(!lifecycle.Is_Authored_Visible());
	BOOST_CHECK(!lifecycle.Is_Animating());
	BOOST_CHECK(lifecycle.State().animation_time == 0.0f);
	Graphics::AuthoredRingDrawData hidden_draw;
	BOOST_CHECK(!lifecycle.Build_Draw_Data(hidden_draw));
	lifecycle.Set_Hidden(false);
	BOOST_CHECK(lifecycle.Is_Visible());
	BOOST_CHECK(lifecycle.Is_Authored_Visible());
	BOOST_CHECK(lifecycle.Is_Animating());
	BOOST_CHECK(lifecycle.State().animation_time == 0.0f);
	lifecycle.Set_Animation_Hidden(true);
	BOOST_CHECK(lifecycle.Is_Visible());
	BOOST_CHECK(!lifecycle.Is_Authored_Visible());
	BOOST_CHECK(!lifecycle.Is_Animating());
	lifecycle.Set_Animation_Hidden(false);
	BOOST_CHECK(lifecycle.Is_Visible());
	BOOST_CHECK(lifecycle.Is_Authored_Visible());
	BOOST_CHECK(lifecycle.Is_Animating());
	lifecycle.Advance(0.25f);
	BOOST_CHECK(lifecycle.State().animation_time == 1.0f);
	lifecycle.Set_Visible(false);
	BOOST_CHECK(!lifecycle.Is_Visible());
	BOOST_CHECK(lifecycle.Is_Authored_Visible());
	BOOST_CHECK(lifecycle.Is_Animating());
	BOOST_CHECK(lifecycle.State().animation_time == 1.0f);
	Graphics::AuthoredRingDrawData hidden_from_camera_draw;
	BOOST_REQUIRE(lifecycle.Build_Draw_Data(hidden_from_camera_draw));
	lifecycle.Set_Visible(true);
	BOOST_CHECK(lifecycle.Is_Visible());
	BOOST_CHECK(lifecycle.Is_Authored_Visible());
	BOOST_CHECK(lifecycle.Is_Animating());
	BOOST_CHECK(lifecycle.State().animation_time == 1.0f);
}

BOOST_AUTO_TEST_CASE(scale_scales_all_authored_keys_and_cost_arrays_require_capacity)
{
	Assets::RingAssetDesc description;
	description.default_inner_scale = {2.0f, 3.0f};
	description.default_outer_scale = {4.0f, 5.0f};
	description.extent = {11.0f, 12.0f, 13.0f};
	description.inner_scale_track.keys = {{0.0f, {1.0f, 1.5f}}, {1.0f, {2.0f, 2.5f}}};
	description.outer_scale_track.keys = {{0.0f, {3.0f, 3.5f}}, {1.0f, {4.0f, 4.5f}}};
	Graphics::AuthoredRingRuntime runtime(description);
	runtime.Scale(2.0f, 3.0f, 99.0f);
	Check_Close(runtime.State().inner_scale.x, 4.0f);
	Check_Close(runtime.State().inner_scale.y, 9.0f);
	Check_Close(runtime.State().outer_scale.x, 8.0f);
	Check_Close(runtime.State().outer_scale.y, 15.0f);
	Check_Close(runtime.Get_Default_Inner_Scale().x, 2.0f);
	Check_Close(runtime.Get_Default_Inner_Scale().y, 4.5f);
	Check_Close(runtime.Get_Default_Outer_Scale().x, 6.0f);
	Check_Close(runtime.Get_Default_Outer_Scale().y, 10.5f);
	Check_Close(runtime.Bounds().extent.x, 11.0f);
	Check_Close(runtime.Bounds().extent.y, 12.0f);
	Check_Close(runtime.Bounds().extent.z, 13.0f);

	std::array<float, Graphics::AuthoredRingValueCount> values{};
	std::array<float, Graphics::AuthoredRingCostCount> costs{};
	BOOST_CHECK(runtime.Calculate_Cost_Value_Arrays(2.0f, values, costs));
	BOOST_CHECK(costs[0] == 0.000001f);
	BOOST_CHECK(costs[1] == 20.0f);
	BOOST_CHECK(costs[20] == 96.0f);
	std::array<float, Graphics::AuthoredRingValueCount - 1> short_values{};
	BOOST_CHECK(!runtime.Calculate_Cost_Value_Arrays(2.0f, short_values, costs));
}

BOOST_AUTO_TEST_CASE(material_sort_layers_and_destination_one_color_equation_are_semantic)
{
	Assets::RingMaterialDesc material;
	material.destination_blend = Assets::RingBlendFactor::Zero;
	BOOST_CHECK(Graphics::AuthoredRingRuntime::Ordered_Layer(material) == 0);
	material.destination_blend = Assets::RingBlendFactor::One;
	material.source_blend = Assets::RingBlendFactor::One;
	BOOST_CHECK(Graphics::AuthoredRingRuntime::Ordered_Layer(material) == 10);
	material.destination_blend = Assets::RingBlendFactor::InverseSourceColor;
	BOOST_CHECK(Graphics::AuthoredRingRuntime::Ordered_Layer(material) == 15);
	material.destination_blend = Assets::RingBlendFactor::InverseSourceAlpha;
	BOOST_CHECK(Graphics::AuthoredRingRuntime::Ordered_Layer(material) == 20);
	material.alpha_test = true;
	material.destination_blend = Assets::RingBlendFactor::Zero;
	BOOST_CHECK(Graphics::AuthoredRingRuntime::Ordered_Layer(material) == 0);
	material.source_blend = Assets::RingBlendFactor::SourceAlpha;
	material.destination_blend = Assets::RingBlendFactor::InverseSourceAlpha;
	BOOST_CHECK(Graphics::AuthoredRingRuntime::Ordered_Layer(material) == 0);
	material.source_blend = Assets::RingBlendFactor::One;
	material.destination_blend = Assets::RingBlendFactor::One;
	BOOST_CHECK(Graphics::AuthoredRingRuntime::Ordered_Layer(material) == 10);
	material.destination_blend = Assets::RingBlendFactor::InverseSourceColor;
	BOOST_CHECK(Graphics::AuthoredRingRuntime::Ordered_Layer(material) == 15);
	material.destination_blend = Assets::RingBlendFactor::InverseSourceAlpha;
	BOOST_CHECK(Graphics::AuthoredRingRuntime::Ordered_Layer(material) == 20);

	Assets::RingAssetDesc draw_description;
	draw_description.material.source_blend = Assets::RingBlendFactor::One;
	draw_description.material.destination_blend = Assets::RingBlendFactor::One;
	draw_description.material.alpha_test = true;
	Graphics::AuthoredRingRuntime additive(draw_description);
	Graphics::AuthoredRingDrawData additive_draw;
	BOOST_REQUIRE(additive.Build_Draw_Data(additive_draw));
	BOOST_CHECK(additive_draw.ordered_layer == 10);
	BOOST_CHECK(!additive_draw.sort_required);
	draw_description.material.destination_blend = Assets::RingBlendFactor::InverseSourceColor;
	Graphics::AuthoredRingRuntime screen(draw_description);
	Graphics::AuthoredRingDrawData screen_draw;
	BOOST_REQUIRE(screen.Build_Draw_Data(screen_draw));
	BOOST_CHECK(screen_draw.ordered_layer == 15);
	BOOST_CHECK(!screen_draw.sort_required);
	draw_description.material.alpha_test = false;
	Graphics::AuthoredRingRuntime translucent(draw_description);
	Graphics::AuthoredRingDrawData translucent_draw;
	BOOST_REQUIRE(translucent.Build_Draw_Data(translucent_draw));
	BOOST_CHECK(translucent_draw.ordered_layer == 15);
	BOOST_CHECK(translucent_draw.sort_required);

	const Graphics::AuthoredRingRuntimeState state{
		{0.25f, 0.5f, 0.75f, 1.0f}, 0.5f, {1.0f, 1.0f}, {1.0f, 1.0f}, 0.0f, true, false};
	const auto premultiplied = Graphics::AuthoredRingRuntime::Quantize_Color(
		state, Assets::RingBlendFactor::One);
	Check_Close(premultiplied.r, 32.0f / 255.0f);
	Check_Close(premultiplied.g, 64.0f / 255.0f);
	Check_Close(premultiplied.b, 96.0f / 255.0f);
	Check_Close(premultiplied.a, 1.0f);
	const auto straight = Graphics::AuthoredRingRuntime::Quantize_Color(
		state, Assets::RingBlendFactor::InverseSourceAlpha);
	Check_Close(straight.r, 64.0f / 255.0f);
	Check_Close(straight.g, 128.0f / 255.0f);
	Check_Close(straight.b, 191.0f / 255.0f);
	Check_Close(straight.a, 128.0f / 255.0f);
	const Graphics::AuthoredRingRuntimeState rounding_state{
		{0.0f, 0.5f / 255.0f, 1.5f / 255.0f, 1.0f}, 127.5f / 255.0f,
		{1.0f, 1.0f}, {1.0f, 1.0f}, 0.0f, true, false};
	const auto rounded = Graphics::AuthoredRingRuntime::Quantize_Color(
		rounding_state, Assets::RingBlendFactor::InverseSourceAlpha);
	Check_Close(rounded.r, 0.0f);
	Check_Close(rounded.g, 1.0f / 255.0f);
	Check_Close(rounded.b, 2.0f / 255.0f);
	Check_Close(rounded.a, 128.0f / 255.0f);
}
