module;

#define BOOST_TEST_MODULE GraphicsViewTests

#include <boost/test/included/unit_test.hpp>

#include <cmath>
#include <type_traits>

export module Graphics.Scene.Views.View.Tests;

import Graphics.Scene.Views.View;

using namespace Graphics;

static_assert(std::is_nothrow_move_constructible_v<View>);
static_assert(std::is_nothrow_move_assignable_v<View>);

BOOST_AUTO_TEST_CASE(view_construction_preserves_camera_data)
{
	const Vector3 position{10.0f, 20.0f, 30.0f};
	const Viewport viewport{4.0f, 8.0f, 1280.0f, 720.0f, 0.0f, 1.0f};
	const View view(Matrix4x4::Identity(), Matrix4x4::Identity(), position, viewport);

	BOOST_CHECK(view.position.x == 10.0f);
	BOOST_CHECK(view.position.y == 20.0f);
	BOOST_CHECK(view.position.z == 30.0f);
	BOOST_CHECK(view.viewport.x == 4.0f);
	BOOST_CHECK(view.viewport.y == 8.0f);
	BOOST_CHECK(view.viewport.width == 1280.0f);
	BOOST_CHECK(view.viewport.height == 720.0f);
}

BOOST_AUTO_TEST_CASE(view_derives_normalized_frustum_planes)
{
	constexpr float near_clip = 1.0f;
	constexpr float far_clip = 10.0f;
	constexpr float inverse_depth = 1.0f / (far_clip - near_clip);

	Matrix4x4 projection{};
	projection.values[0] = 1.0f;
	projection.values[5] = 1.0f;
	projection.values[10] = -(far_clip + near_clip) * inverse_depth;
	projection.values[11] = -2.0f * far_clip * near_clip * inverse_depth;
	projection.values[14] = -1.0f;

	View view(Matrix4x4::Identity(), projection, {}, {});
	view.Derive_Frustum();

	const float side_component = 1.0f / std::sqrt(2.0f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.left.normal.x, side_component, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.left.normal.z, -side_component, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.right.normal.x, -side_component, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.right.normal.z, -side_component, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.bottom.normal.y, side_component, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.bottom.normal.z, -side_component, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.top.normal.y, -side_component, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.top.normal.z, -side_component, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.near_plane.normal.z, -1.0f, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.near_plane.distance, -near_clip, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.far_plane.normal.z, 1.0f, 0.00001f);
	BOOST_CHECK_CLOSE_FRACTION(view.frustum.far_plane.distance, far_clip, 0.00001f);
}
