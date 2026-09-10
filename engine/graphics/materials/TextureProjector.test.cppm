module;
#define BOOST_TEST_MODULE TextureProjectorTests
#include <boost/test/included/unit_test.hpp>
#include <cmath>
#include <cstddef>

#if GRAPHICS_COMPARE_GAME_MATH
#include "Utility/CppMacros.h"
#include "WWMath/matrix3d.h"
#endif

export module Graphics.Materials.TextureProjector.Tests;
import Graphics.Materials.TextureProjector;
import Graphics.Scene.Views.View;

namespace
{
bool Nearly_Equal(float left, float right, float tolerance = 1.0e-4f) noexcept
{
	return std::fabs(left - right) <= tolerance;
}
}

BOOST_AUTO_TEST_CASE(perspective_fit_uses_the_game_look_at_basis)
{
	Graphics::TextureProjectorBounds bounds;
	bounds.center = {1.25f, -0.5f, 0.75f};
	bounds.extent = {2.0f, 5.0f, 3.0f};

	Graphics::Matrix4x4 object_transform = Graphics::Matrix4x4::Identity();
	object_transform.values = {
		0.0f, -1.0f, 0.0f, 4.0f,
		1.0f, 0.0f, 0.0f, -3.0f,
		0.0f, 0.0f, 1.0f, 2.0f,
		0.0f, 0.0f, 0.0f, 1.0f};
	const Graphics::Vector3 light_position{-11.0f, 17.0f, 23.0f};
	const Graphics::Vector3 world_center{
		object_transform.values[0] * bounds.center.x
			+ object_transform.values[1] * bounds.center.y
			+ object_transform.values[2] * bounds.center.z + object_transform.values[3],
		object_transform.values[4] * bounds.center.x
			+ object_transform.values[5] * bounds.center.y
			+ object_transform.values[6] * bounds.center.z + object_transform.values[7],
		object_transform.values[8] * bounds.center.x
			+ object_transform.values[9] * bounds.center.y
			+ object_transform.values[10] * bounds.center.z + object_transform.values[11]};

	const auto fit = Graphics::Fit_Perspective_Texture_Projector(bounds, object_transform, light_position);
	BOOST_REQUIRE(fit.valid);

#if GRAPHICS_COMPARE_GAME_MATH
	::Matrix3D expected;
	expected.Look_At(
		::Vector3(light_position.x, light_position.y, light_position.z),
		::Vector3(world_center.x, world_center.y, world_center.z), 0.0f);
	for (std::size_t row = 0; row < 3; ++row) {
		for (std::size_t column = 0; column < 4; ++column) {
			BOOST_CHECK(Nearly_Equal(
				fit.camera_transform.values[row * 4 + column],
				expected[row][column], 2.0e-5f));
		}
	}
#endif

	BOOST_CHECK(fit.horizontal_fov > 0.0f);
	BOOST_CHECK(fit.vertical_fov > 0.0f);
	BOOST_CHECK(fit.fitting_clip_start > 0.0f);
	BOOST_CHECK(fit.fitting_clip_end > fit.fitting_clip_start);
	BOOST_CHECK(Nearly_Equal(fit.projection.values[14], -1.0f));
	BOOST_CHECK(Nearly_Equal(fit.projection.values[15], 0.0f));
}

BOOST_AUTO_TEST_CASE(perspective_fit_retains_quick_extent_length_for_unequal_boxes)
{
	const Graphics::TextureProjectorBounds bounds{{0.0f, 0.0f, 0.0f}, {2.0f, 5.0f, 3.0f}};
	const Graphics::Matrix4x4 object_transform = Graphics::Matrix4x4::Identity();
	const Graphics::Vector3 light_position{-4.0f, -7.0f, 11.0f};
	const auto fit = Graphics::Fit_Perspective_Texture_Projector(bounds, object_transform, light_position);
	BOOST_REQUIRE(fit.valid);

	const float distance = std::sqrt(
		light_position.x * light_position.x + light_position.y * light_position.y
		+ light_position.z * light_position.z);
	const float quick_extent_length = 5.0f + (11.0f / 32.0f) * 3.0f + 0.25f * 2.0f;
	const float expected_clip_end = 2.0f * (distance + quick_extent_length);
	BOOST_CHECK_CLOSE(fit.fitting_clip_end, expected_clip_end, 0.01);

	const float expected_projection_x = static_cast<float>(
		1.0 / std::tan(static_cast<double>(fit.horizontal_fov) * 0.5));
	BOOST_CHECK(Nearly_Equal(fit.projection.values[0], expected_projection_x));
}

BOOST_AUTO_TEST_CASE(perspective_fit_rejects_a_light_inside_the_box)
{
	const Graphics::TextureProjectorBounds bounds{{0.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 2.0f}};
	const auto fit = Graphics::Fit_Perspective_Texture_Projector(
		bounds, Graphics::Matrix4x4::Identity(), {0.0f, 0.0f, 0.0f});
	BOOST_CHECK(!fit.valid);
}
