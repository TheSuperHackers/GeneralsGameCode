module;

#define BOOST_TEST_MODULE GraphicsVisibilityTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.Scene.Visibility.Tests;

import Graphics.Scene.Visibility;

using namespace Graphics;

static Matrix4x4 Make_Projection() noexcept
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
	return projection;
}

static View Make_View() noexcept
{
	return {Matrix4x4::Identity(), Make_Projection(), {}, {0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}};
}

static RenderInstance Make_Instance(float x, float y, float z, float radius) noexcept
{
	RenderInstance instance;
	instance.transform.matrix = {
		1.0f, 0.0f, 0.0f, x,
		0.0f, 1.0f, 0.0f, y,
		0.0f, 0.0f, 1.0f, z,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	instance.bounds.radius = radius;
	return instance;
}

BOOST_AUTO_TEST_CASE(visibility_outputs_compact_visible_handles)
{
	RenderScene scene;
	const InstanceHandle visible_handle = scene.Create(Make_Instance(0.0f, 0.0f, -5.0f, 1.0f));
	RenderInstance hidden = Make_Instance(0.0f, 0.0f, -5.0f, 1.0f);
	hidden.flags = RenderInstanceFlags::Hidden;
	scene.Create(hidden);
	scene.Create(Make_Instance(50.0f, 0.0f, -5.0f, 1.0f));
	scene.Create(Make_Instance(0.0f, 0.0f, 5.0f, 1.0f));

	std::array<InstanceHandle, 3> storage{};
	VisibleSet visible_set(storage);

	BOOST_REQUIRE(Build_Visible_Set(scene, Make_View(), visible_set));
	BOOST_REQUIRE(visible_set.Size() == 1);
	BOOST_CHECK(visible_set.Handles()[0] == visible_handle);
}

BOOST_AUTO_TEST_CASE(visibility_uses_world_transform_and_negative_z_forward)
{
	RenderScene scene;
	RenderInstance instance = Make_Instance(0.0f, 0.0f, -6.0f, 0.5f);
	instance.bounds.center = {0.0f, 0.0f, 1.0f};
	const InstanceHandle visible_handle = scene.Create(instance);

	std::array<InstanceHandle, 1> storage{};
	VisibleSet visible_set(storage);

	BOOST_REQUIRE(Build_Visible_Set(scene, Make_View(), visible_set));
	BOOST_REQUIRE(visible_set.Size() == 1);
	BOOST_CHECK(visible_set.Handles()[0] == visible_handle);
}

BOOST_AUTO_TEST_CASE(visibility_rejects_insufficient_output_capacity)
{
	RenderScene scene;
	scene.Create(Make_Instance(-2.0f, 0.0f, -5.0f, 0.5f));
	scene.Create(Make_Instance(2.0f, 0.0f, -5.0f, 0.5f));

	std::array<InstanceHandle, 1> storage{};
	VisibleSet visible_set(storage);

	BOOST_CHECK(!Build_Visible_Set(scene, Make_View(), visible_set));
	BOOST_CHECK(visible_set.Size() == 0);
}
