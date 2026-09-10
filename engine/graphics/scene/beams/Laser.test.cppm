module;

#define BOOST_TEST_MODULE GraphicsLaserTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

export module Graphics.Scene.Beams.Laser.Tests;

import Graphics.Scene.Beams.Laser;
import Graphics.Scene.Beams;
import Graphics.RHI;

using namespace Graphics;

static_assert(std::is_nothrow_move_constructible_v<LaserDescription>);
static_assert(std::is_nothrow_move_assignable_v<LaserDescription>);
static_assert(std::is_nothrow_move_constructible_v<LaserView>);
static_assert(std::is_nothrow_move_assignable_v<LaserView>);
namespace
{

void Check_Position(const LaserVertex &vertex, Vec3 expected)
{
	BOOST_CHECK_EQUAL(vertex.position[0], expected.x);
	BOOST_CHECK_EQUAL(vertex.position[1], expected.y);
	BOOST_CHECK_EQUAL(vertex.position[2], expected.z);
}

void Check_Color(const LaserVertex &vertex, Color4 expected)
{
	BOOST_CHECK_EQUAL(vertex.color[0], expected.r);
	BOOST_CHECK_EQUAL(vertex.color[1], expected.g);
	BOOST_CHECK_EQUAL(vertex.color[2], expected.b);
	BOOST_CHECK_EQUAL(vertex.color[3], expected.a);
}

void Check_UV(const LaserVertex &vertex, float u, float v)
{
	BOOST_CHECK_EQUAL(vertex.uv[0], u);
	BOOST_CHECK_EQUAL(vertex.uv[1], v);
	BOOST_CHECK_EQUAL(vertex.detail_uv[0], u);
	BOOST_CHECK_EQUAL(vertex.detail_uv[1], v);
}

void Check_Normal(const LaserVertex &vertex, Vec3 expected)
{
	BOOST_CHECK_EQUAL(vertex.normal[0], expected.x);
	BOOST_CHECK_EQUAL(vertex.normal[1], expected.y);
	BOOST_CHECK_EQUAL(vertex.normal[2], expected.z);
}

LaserDescription Make_Authored_Laser()
{
	LaserDescription description;
	description.start = {-0.5f, 0.0f, 0.0f};
	description.end = {0.5f, 0.0f, 1.0f};
	description.width = 0.5f;
	description.color = {0.25f, 0.5f, 0.75f, 0.625f};
	description.uv_scale = 3.0f;
	description.uv_offset = -0.25f;
	description.scroll_rate = -0.75f;
	description.distortion = 0.4f;
	return description;
}

LaserView Make_View()
{
	LaserView view;
	view.camera_forward = {0.0f, 0.0f, -1.0f};
	view.camera_right = {1.0f, 0.0f, 0.0f};
	view.time_milliseconds = 1234;
	return view;
}

}

BOOST_AUTO_TEST_CASE(laser_vertices_keep_authored_width_color_order_and_uv_axes)
{
	const LaserDescription description = Make_Authored_Laser();
	const LaserView view = Make_View();
	std::array<LaserVertex, 6> vertices{};

	BOOST_REQUIRE_EQUAL(Build_Laser_Vertices(description, view, description.uv_offset, vertices), vertices.size());

	// For a direction of (1, 0, 1), camera-facing side is the independently
	// computed cross product (0, 1, 0).  This keeps the expected positions
	// separate from the production side-vector implementation.
	const Vec3 side{0.0f, 1.0f, 0.0f};
	const Vec3 half_side{0.0f, 0.25f, 0.0f};
	const Vec3 start_left{description.start.x - half_side.x,
		description.start.y - half_side.y, description.start.z - half_side.z};
	const Vec3 start_right{description.start.x + half_side.x,
		description.start.y + half_side.y, description.start.z + half_side.z};
	const Vec3 end_left{description.end.x - half_side.x,
		description.end.y - half_side.y, description.end.z - half_side.z};
	const Vec3 end_right{description.end.x + half_side.x,
		description.end.y + half_side.y, description.end.z + half_side.z};

	const std::array<Vec3, 6> expected_positions{
		start_left, start_right, end_left, end_left, start_right, end_right};
	const std::array<float, 6> expected_u{0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f};
	const std::array<float, 6> expected_v{-0.25f, -0.25f, 2.75f, 2.75f, -0.25f, 2.75f};
	for (std::size_t index = 0; index < vertices.size(); ++index) {
		Check_Position(vertices[index], expected_positions[index]);
		Check_Color(vertices[index], description.color);
		Check_UV(vertices[index], expected_u[index], expected_v[index]);
		Check_Normal(vertices[index], side);
		BOOST_CHECK_EQUAL(vertices[index].distortion[0], description.distortion);
		BOOST_CHECK_EQUAL(vertices[index].distortion[1], 0.0f);
	}
}

BOOST_AUTO_TEST_CASE(laser_vertices_fall_back_to_camera_right_for_parallel_view)
{
	LaserDescription description;
	description.start = {-0.5f, 0.0f, 0.0f};
	description.end = {-0.5f, 0.0f, -1.0f};
	description.width = 0.2f;
	LaserView view = Make_View();
	view.camera_forward = {0.0f, 0.0f, -1.0f};
	view.camera_right = {0.0f, 1.0f, 0.0f};
	std::array<LaserVertex, 6> vertices{};

	BOOST_REQUIRE_EQUAL(Build_Laser_Vertices(description, view, 0.0f, vertices), vertices.size());
	Check_Position(vertices[0], {-0.5f, -0.1f, 0.0f});
	Check_Position(vertices[1], {-0.5f, 0.1f, 0.0f});
	Check_Position(vertices[2], {-0.5f, -0.1f, -1.0f});
	Check_Position(vertices[5], {-0.5f, 0.1f, -1.0f});
	for (const LaserVertex &vertex : vertices)
		Check_Normal(vertex, {0.0f, 1.0f, 0.0f});
}

BOOST_AUTO_TEST_CASE(laser_vertices_reject_disabled_degenerate_and_insufficient_inputs)
{
	const LaserView view = Make_View();
	LaserDescription description = Make_Authored_Laser();
	std::array<LaserVertex, 6> vertices{};

	description.enabled = false;
	BOOST_CHECK_EQUAL(Build_Laser_Vertices(description, view, 0.0f, vertices), 0u);
	description.enabled = true;
	description.width = 0.0f;
	BOOST_CHECK_EQUAL(Build_Laser_Vertices(description, view, 0.0f, vertices), 0u);
	description.width = -1.0f;
	BOOST_CHECK_EQUAL(Build_Laser_Vertices(description, view, 0.0f, vertices), 0u);
	description.width = 0.5f;
	description.end = description.start;
	BOOST_CHECK_EQUAL(Build_Laser_Vertices(description, view, 0.0f, vertices), 0u);

	description.end = {0.5f, 0.0f, 1.0f};
	std::array<LaserVertex, 5> short_output{};
	BOOST_CHECK_EQUAL(Build_Laser_Vertices(description, view, 0.0f, short_output), 0u);
}

BOOST_AUTO_TEST_CASE(laser_handles_update_destroy_and_reject_stale_generations)
{
	LaserRenderer renderer;
	LaserDescription first_description = Make_Authored_Laser();
	const LaserDescription second_description = Make_Authored_Laser();
	const LaserHandle first = renderer.Create(first_description);
	const LaserHandle second = renderer.Create(second_description);
	BOOST_REQUIRE(first.Is_Valid());
	BOOST_REQUIRE(second.Is_Valid());
	BOOST_CHECK(first != second);
	BOOST_CHECK_EQUAL(renderer.Size(), 2u);

	first_description.width = 1.25f;
	first_description.color = {0.9f, 0.2f, 0.1f, 0.8f};
	BOOST_REQUIRE(renderer.Update(first, first_description));
	BOOST_REQUIRE(renderer.Destroy(first));
	BOOST_CHECK(!renderer.Update(first, second_description));
	BOOST_CHECK(!renderer.Destroy(first));
	BOOST_CHECK_EQUAL(renderer.Size(), 1u);

	const LaserHandle reused = renderer.Create(first_description);
	BOOST_REQUIRE(reused.Is_Valid());
	BOOST_CHECK_EQUAL(reused.Get_Index(), first.Get_Index());
	BOOST_CHECK(reused.Get_Generation() != first.Get_Generation());
	BOOST_CHECK(!renderer.Update(first, second_description));
	BOOST_CHECK(renderer.Update(reused, first_description));
	BOOST_REQUIRE(renderer.Destroy(second));
	BOOST_REQUIRE(renderer.Destroy(reused));
	BOOST_CHECK_EQUAL(renderer.Size(), 0u);
}

BOOST_AUTO_TEST_CASE(laser_view_is_retained_only_after_initialization)
{
	LaserRenderer renderer;
	const LaserView original = Make_View();
	BOOST_CHECK(!renderer.Set_View(original));
	BOOST_CHECK_EQUAL(renderer.View().time_milliseconds, 0u);
	BOOST_CHECK_EQUAL(renderer.View().camera_forward[2], -1.0f);
}
