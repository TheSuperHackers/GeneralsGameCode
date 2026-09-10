module;

#define BOOST_TEST_MODULE GraphicsDecalVisibilityTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.Scene.DecalVisibility.Tests;

import Graphics.Scene.DecalVisibility;

using namespace Graphics;

namespace
{
Matrix4x4 Make_Perspective(float near_clip, float far_clip) noexcept
{
	Matrix4x4 projection{};
	projection.values[0] = 1.0f;
	projection.values[5] = 1.0f;
	projection.values[10] = -(far_clip + near_clip) / (far_clip - near_clip);
	projection.values[11] = -2.0f * far_clip * near_clip / (far_clip - near_clip);
	projection.values[14] = -1.0f;
	return projection;
}

View Make_View() noexcept
{
	return {Matrix4x4::Identity(), Make_Perspective(1.0f, 100.0f), {}, {0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 1.0f}};
}
}

BOOST_AUTO_TEST_CASE(decals_are_culled_deterministically_against_the_view)
{
	RenderScene scene;
	RenderDecal visible;
	visible.bounds.center = {0.0f, 0.0f, -5.0f};
	const DecalHandle visible_handle = scene.Create_Decal(visible);

	RenderDecal outside;
	outside.bounds.center = {200.0f, 0.0f, -5.0f};
	scene.Create_Decal(outside);

	RenderDecal disabled;
	disabled.bounds.center = {0.0f, 0.0f, -5.0f};
	disabled.flags = RenderDecalFlags::None;
	scene.Create_Decal(disabled);

	std::array<DecalHandle, 3> storage{};
	VisibleDecalSet visible_decals(storage);
	const View view = Make_View();
	BOOST_REQUIRE(Build_Visible_Decals(scene, view, visible_decals));
	BOOST_REQUIRE(visible_decals.Size() == 1);
	BOOST_CHECK(visible_decals.Handles()[0] == visible_handle);

	const std::array<DecalHandle, 1> first_result = {visible_decals.Handles()[0]};
	BOOST_REQUIRE(Build_Visible_Decals(scene, view, visible_decals));
	BOOST_CHECK(visible_decals.Handles().size() == first_result.size());
	BOOST_CHECK(visible_decals.Handles()[0] == first_result[0]);
}

BOOST_AUTO_TEST_CASE(decal_visibility_rejects_insufficient_output_storage)
{
	RenderScene scene;
	RenderDecal decal;
	decal.bounds.center = {0.0f, 0.0f, -5.0f};
	scene.Create_Decal(decal);
	std::array<DecalHandle, 0> storage{};
	VisibleDecalSet visible_decals(storage);

	BOOST_CHECK(!Build_Visible_Decals(scene, Make_View(), visible_decals));
	BOOST_CHECK(visible_decals.Size() == 0);
}
