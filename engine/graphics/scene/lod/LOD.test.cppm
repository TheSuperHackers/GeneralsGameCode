module;

#define BOOST_TEST_MODULE GraphicsLODTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cstdint>
#include <type_traits>

export module Graphics.Scene.LOD.Tests;

import Graphics.Scene.LOD;

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

static RenderInstance Make_Instance(float z, float radius, MeshHandle mesh) noexcept
{
	RenderInstance instance;
	instance.transform.matrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, z,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	instance.bounds.radius = radius;
	instance.mesh = mesh;
	return instance;
}

static MeshPool Make_Meshes(MeshHandle &base, MeshHandle &first_lod, MeshHandle &second_lod) noexcept
{
	MeshPool meshes;
	base = meshes.Create();
	first_lod = meshes.Create();
	second_lod = meshes.Create();

	Mesh *base_mesh = meshes.Resolve(base);
	base_mesh->lods[0] = {first_lod, 0.5f};
	base_mesh->lods[1] = {second_lod, 0.15f};
	base_mesh->lod_count = 2;
	return meshes;
}

static LODSelection Find_Selection(const LODSet &lod_set, InstanceHandle instance) noexcept
{
	for (const LODSelection selection : lod_set.Selections()) {
		if (selection.instance == instance)
			return selection;
	}
	return {};
}

static_assert(std::is_nothrow_move_constructible_v<LODSelection>);
static_assert(std::is_nothrow_move_assignable_v<LODSelection>);

BOOST_AUTO_TEST_CASE(lod_selects_by_projected_screen_size)
{
	MeshHandle base;
	MeshHandle first_lod;
	MeshHandle second_lod;
	MeshPool meshes = Make_Meshes(base, first_lod, second_lod);
	RenderScene scene;
	const InstanceHandle near_instance = scene.Create(Make_Instance(-2.0f, 1.0f, base));
	const InstanceHandle middle_instance = scene.Create(Make_Instance(-8.0f, 1.0f, base));
	const InstanceHandle far_instance = scene.Create(Make_Instance(-9.0f, 0.5f, base));

	std::array<InstanceHandle, 3> visible_storage{};
	VisibleSet visible_set(visible_storage);
	BOOST_REQUIRE(Build_Visible_Set(scene, Make_View(), visible_set));

	std::array<LODSelection, 3> lod_storage{};
	LODSet lod_set(lod_storage);
	BOOST_REQUIRE(Build_LOD_Set(scene, meshes, visible_set, Make_View(), lod_set));
	BOOST_REQUIRE(lod_set.Size() == 3);

	const LODSelection near_selection = Find_Selection(lod_set, near_instance);
	const LODSelection middle_selection = Find_Selection(lod_set, middle_instance);
	const LODSelection far_selection = Find_Selection(lod_set, far_instance);
	BOOST_CHECK(near_selection.mesh == base);
	BOOST_CHECK(near_selection.lod_index == 0);
	BOOST_CHECK(middle_selection.mesh == first_lod);
	BOOST_CHECK(middle_selection.lod_index == 1);
	BOOST_CHECK(far_selection.mesh == second_lod);
	BOOST_CHECK(far_selection.lod_index == 2);
}

BOOST_AUTO_TEST_CASE(lod_skips_stale_or_unresolved_instances)
{
	MeshHandle base;
	MeshHandle first_lod;
	MeshHandle second_lod;
	MeshPool meshes = Make_Meshes(base, first_lod, second_lod);
	RenderScene scene;
	const InstanceHandle instance = scene.Create(Make_Instance(-5.0f, 1.0f, base));

	std::array<InstanceHandle, 1> visible_storage{};
	VisibleSet visible_set(visible_storage);
	BOOST_REQUIRE(Build_Visible_Set(scene, Make_View(), visible_set));
	BOOST_REQUIRE(scene.Destroy(instance));

	std::array<LODSelection, 1> lod_storage{};
	LODSet lod_set(lod_storage);
	BOOST_CHECK(Build_LOD_Set(scene, meshes, visible_set, Make_View(), lod_set));
	BOOST_CHECK(lod_set.Size() == 0);
}

BOOST_AUTO_TEST_CASE(lod_falls_back_from_stale_mesh_lod)
{
	MeshHandle base;
	MeshHandle first_lod;
	MeshHandle second_lod;
	MeshPool meshes = Make_Meshes(base, first_lod, second_lod);
	RenderScene scene;
	const InstanceHandle instance = scene.Create(Make_Instance(-5.0f, 1.0f, base));
	BOOST_REQUIRE(meshes.Destroy(first_lod));

	std::array<InstanceHandle, 1> visible_storage{};
	VisibleSet visible_set(visible_storage);
	BOOST_REQUIRE(Build_Visible_Set(scene, Make_View(), visible_set));

	std::array<LODSelection, 1> lod_storage{};
	LODSet lod_set(lod_storage);
	BOOST_REQUIRE(Build_LOD_Set(scene, meshes, visible_set, Make_View(), lod_set));
	BOOST_REQUIRE(lod_set.Size() == 1);
	BOOST_CHECK(lod_set.Selections()[0].instance == instance);
	BOOST_CHECK(lod_set.Selections()[0].mesh == base);
	BOOST_CHECK(lod_set.Selections()[0].lod_index == 0);
}

BOOST_AUTO_TEST_CASE(lod_rejects_insufficient_output_capacity)
{
	MeshHandle base;
	MeshHandle first_lod;
	MeshHandle second_lod;
	MeshPool meshes = Make_Meshes(base, first_lod, second_lod);
	RenderScene scene;
	scene.Create(Make_Instance(-3.0f, 1.0f, base));
	scene.Create(Make_Instance(-4.0f, 1.0f, base));

	std::array<InstanceHandle, 2> visible_storage{};
	VisibleSet visible_set(visible_storage);
	BOOST_REQUIRE(Build_Visible_Set(scene, Make_View(), visible_set));

	std::array<LODSelection, 1> lod_storage{};
	LODSet lod_set(lod_storage);
	BOOST_CHECK(!Build_LOD_Set(scene, meshes, visible_set, Make_View(), lod_set));
	BOOST_CHECK(lod_set.Size() == 0);
}

BOOST_AUTO_TEST_CASE(lod_hysteresis_keeps_selection_inside_switch_band)
{
	MeshHandle base;
	MeshHandle first_lod;
	MeshHandle second_lod;
	MeshPool meshes = Make_Meshes(base, first_lod, second_lod);
	RenderScene scene;
	const InstanceHandle instance = scene.Create(Make_Instance(-7.2f, 0.5f, base));

	std::array<InstanceHandle, 1> visible_storage{};
	VisibleSet visible_set(visible_storage);
	BOOST_REQUIRE(Build_Visible_Set(scene, Make_View(), visible_set));
	std::array<LODSelection, 1> lod_storage{};
	LODSet lod_set(lod_storage);
	std::array<LODHistoryEntry, 1> history_storage{};
	LODHistory history(history_storage);
	history.Record(instance, 1);

	BOOST_REQUIRE(Build_LOD_Set(scene, meshes, visible_set, Make_View(), lod_set, history, {0.1f}));
	BOOST_CHECK(lod_set.Selections()[0].lod_index == 1);

	history.Record(instance, 2);
	BOOST_REQUIRE(Build_LOD_Set(scene, meshes, visible_set, Make_View(), lod_set, history, {0.1f}));
	BOOST_CHECK(lod_set.Selections()[0].lod_index == 2);
}

BOOST_AUTO_TEST_CASE(lod_history_is_generation_safe_and_selection_is_deterministic)
{
	MeshHandle base;
	MeshHandle first_lod;
	MeshHandle second_lod;
	MeshPool meshes = Make_Meshes(base, first_lod, second_lod);
	RenderScene scene;
	const InstanceHandle instance = scene.Create(Make_Instance(-5.0f, 1.0f, base));

	std::array<InstanceHandle, 1> visible_storage{};
	VisibleSet visible_set(visible_storage);
	BOOST_REQUIRE(Build_Visible_Set(scene, Make_View(), visible_set));
	std::array<LODSelection, 1> first_storage{};
	std::array<LODSelection, 1> second_storage{};
	LODSet first_set(first_storage);
	LODSet second_set(second_storage);
	std::array<LODHistoryEntry, 1> history_storage{};
	LODHistory history(history_storage);

	BOOST_REQUIRE(Build_LOD_Set(scene, meshes, visible_set, Make_View(), first_set, history, {0.1f}));
	BOOST_REQUIRE(Build_LOD_Set(scene, meshes, visible_set, Make_View(), second_set, history, {0.1f}));
	BOOST_REQUIRE(first_set.Size() == second_set.Size());
	BOOST_CHECK(first_set.Selections()[0].instance == second_set.Selections()[0].instance);
	BOOST_CHECK(first_set.Selections()[0].mesh == second_set.Selections()[0].mesh);
	BOOST_CHECK(first_set.Selections()[0].lod_index == second_set.Selections()[0].lod_index);

	BOOST_REQUIRE(scene.Destroy(instance));
	const InstanceHandle replacement = scene.Create(Make_Instance(-2.0f, 1.0f, base));
	BOOST_REQUIRE(replacement.Get_Generation() != instance.Get_Generation());
	std::uint32_t previous_lod = 99;
	BOOST_CHECK(!history.Previous(replacement, previous_lod));
}
