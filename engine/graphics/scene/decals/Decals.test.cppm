module;

#define BOOST_TEST_MODULE GraphicsDecalsTests

#include <boost/test/included/unit_test.hpp>

#include <type_traits>

export module Graphics.Scene.Decals.Tests;

import Graphics.Scene.Decals;
import Graphics.Scene.RenderScene;

using namespace Graphics;

static_assert(!std::is_convertible_v<DecalHandle, MaterialHandle>);
static_assert(std::is_nothrow_move_constructible_v<RenderDecal>);
static_assert(std::is_nothrow_move_assignable_v<RenderDecal>);

BOOST_AUTO_TEST_CASE(decals_are_dense_typed_and_generation_checked)
{
	RenderScene scene;
	RenderDecal first;
	first.bounds.center = {1.0f, 2.0f, 3.0f};
	first.bounds.radius = 4.0f;
	first.material = MaterialHandle(8, 1);
	const DecalHandle old_handle = scene.Create_Decal(first);
	const DecalHandle remaining_handle = scene.Create_Decal();

	BOOST_CHECK(scene.Decal_Count() == 2);
	BOOST_CHECK(scene.Dense_Decal_Index(old_handle) == 0);
	BOOST_CHECK(scene.Dense_Decal_Index(remaining_handle) == 1);
	BOOST_REQUIRE(scene.Destroy_Decal(old_handle));
	BOOST_CHECK(scene.Dense_Decal_Index(old_handle) == Invalid_Render_Scene_Index);
	BOOST_CHECK(!scene.Destroy_Decal(old_handle));

	const DecalHandle reused_handle = scene.Create_Decal();
	BOOST_CHECK(reused_handle.Get_Index() == old_handle.Get_Index());
	BOOST_CHECK(reused_handle.Get_Generation() != old_handle.Get_Generation());
	BOOST_CHECK(scene.Dense_Decal_Index(reused_handle) == 1);

	RenderDecal updated;
	updated.transform.matrix.values[3] = 12.0f;
	updated.bounds.radius = 9.0f;
	updated.material = MaterialHandle(4, 1);
	updated.flags = RenderDecalFlags::None;
	BOOST_REQUIRE(scene.Update_Decal(reused_handle, updated));
	const RenderDecalData data = scene.Decals();
	BOOST_CHECK(data.bounds.center_x[1] == 0.0f);
	BOOST_CHECK(data.bounds.center_y[1] == 0.0f);
	BOOST_CHECK(data.bounds.center_z[1] == 0.0f);
	BOOST_CHECK(data.bounds.radii[1] == 9.0f);

	bool visited = false;
	BOOST_REQUIRE(scene.Visit_Decal(reused_handle, [&](DecalHandle handle, const RenderDecalView &view) noexcept {
		visited = handle == reused_handle
			&& view.transform.matrix.values[3] == 12.0f
			&& view.bounds.radius == 9.0f
			&& view.material == updated.material
			&& view.flags == RenderDecalFlags::None;
	}));
	BOOST_CHECK(visited);
	BOOST_CHECK(!scene.Visit_Decal(old_handle, [](DecalHandle, const RenderDecalView &) noexcept {}));
}

BOOST_AUTO_TEST_CASE(decal_pipeline_uses_depth_tested_alpha_projection_state)
{
	PipelineDesc description{};
	const PipelineDesc decal_description = Make_Decal_Pipeline(description);

	BOOST_CHECK(decal_description.depth_test);
	BOOST_CHECK(!decal_description.depth_write);
	BOOST_CHECK(decal_description.blend_mode == RHIBlendMode::Alpha);
}
