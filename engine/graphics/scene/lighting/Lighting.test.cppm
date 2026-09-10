module;

#define BOOST_TEST_MODULE GraphicsLightingTests

#include <boost/test/included/unit_test.hpp>

#include <type_traits>

export module Graphics.Scene.Lighting.Tests;

import Graphics.Scene.Lighting;

using namespace Graphics;

static_assert(std::is_nothrow_move_constructible_v<RenderLight>);
static_assert(std::is_nothrow_move_assignable_v<RenderLight>);
static_assert(std::is_nothrow_move_constructible_v<GPULightData>);
static_assert(std::is_nothrow_move_assignable_v<GPULightData>);
static_assert(!std::is_convertible_v<LightHandle, MeshHandle>);

BOOST_AUTO_TEST_CASE(light_types_and_defaults_are_backend_agnostic)
{
	const RenderLight light;

	BOOST_CHECK(light.type == RenderLightType::Point);
	BOOST_CHECK(Has_Render_Light_Flag(light.flags, RenderLightFlags::Enabled));
	BOOST_CHECK(light.direction.z == -1.0f);
	BOOST_CHECK(light.color.x == 1.0f);
	BOOST_CHECK(light.intensity == 1.0f);
}

BOOST_AUTO_TEST_CASE(gpu_light_data_is_compact_and_aligned)
{
	GPULightData light;
	light.position_range = {1.0f, 2.0f, 3.0f, 40.0f};
	light.direction_intensity = {0.0f, 0.0f, -1.0f, 5.0f};
	light.color_inner_angle = {0.5f, 0.25f, 0.75f, 0.2f};
	light.outer_angle = 0.6f;
	light.type = static_cast<std::uint32_t>(RenderLightType::Spot);
	light.flags = static_cast<std::uint32_t>(RenderLightFlags::Enabled);

	BOOST_CHECK(light.position_range[3] == 40.0f);
	BOOST_CHECK(light.direction_intensity[3] == 5.0f);
	BOOST_CHECK(light.color_inner_angle[3] == 0.2f);
	BOOST_CHECK(light.outer_angle == 0.6f);
	BOOST_CHECK(light.type == static_cast<std::uint32_t>(RenderLightType::Spot));
}
