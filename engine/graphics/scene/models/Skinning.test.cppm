#define BOOST_TEST_MODULE GraphicsSkinningTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.Scene.Models.Skinning.Tests;

import Graphics.Scene.Models.Skinning;

using namespace Graphics;

static RenderTransform Translation(float x, float y, float z) noexcept
{
	RenderTransform transform;
	transform.matrix[0] = 1.0f;
	transform.matrix[5] = 1.0f;
	transform.matrix[10] = 1.0f;
	transform.matrix[15] = 1.0f;
	transform.matrix[3] = x;
	transform.matrix[7] = y;
	transform.matrix[11] = z;
	return transform;
}

BOOST_AUTO_TEST_CASE(bone_matrix_ranges_map_poses_to_contiguous_gpu_data)
{
	BoneMatrixTable table;
	table.Reserve(2, 4);
	const std::array<RenderTransform, 2> first_pose = {
		Translation(1.0f, 0.0f, 0.0f), Translation(2.0f, 0.0f, 0.0f)
	};
	const std::array<RenderTransform, 1> second_pose = {Translation(3.0f, 0.0f, 0.0f)};
	const PoseHandle first = table.Create(first_pose);
	const PoseHandle second = table.Create(second_pose);

	BOOST_REQUIRE(first.Is_Valid());
	BOOST_REQUIRE(second.Is_Valid());
	BOOST_CHECK(table.Range(first).first_matrix == 0u);
	BOOST_CHECK(table.Range(first).count == 2u);
	BOOST_CHECK(table.Range(second).first_matrix == 2u);
	BOOST_CHECK(table.Matrices()[2].matrix[3] == 3.0f);
}

BOOST_AUTO_TEST_CASE(bone_matrix_updates_are_dirty_and_stale_poses_are_rejected)
{
	BoneMatrixTable table;
	table.Reserve(1, 2);
	const std::array<RenderTransform, 2> initial = {
		Translation(0.0f, 0.0f, 0.0f), Translation(1.0f, 0.0f, 0.0f)
	};
	const PoseHandle pose = table.Create(initial);
	BOOST_REQUIRE(pose.Is_Valid());
	table.Clear_Dirty();

	const std::array<RenderTransform, 2> updated = {
		Translation(4.0f, 0.0f, 0.0f), Translation(5.0f, 0.0f, 0.0f)
	};
	BOOST_REQUIRE(table.Update(pose, updated));
	BOOST_REQUIRE_EQUAL(table.Dirty_Ranges().size(), 1u);
	BOOST_CHECK(table.Dirty_Ranges()[0].first_matrix == 0u);
	BOOST_CHECK(table.Dirty_Ranges()[0].count == 2u);
	BOOST_CHECK(table.Matrices()[0].matrix[3] == 4.0f);

	BOOST_REQUIRE(table.Destroy(pose));
	BOOST_CHECK(!table.Is_Valid(pose));
	BOOST_CHECK(!table.Range(pose).Is_Valid());
	const PoseHandle replacement = table.Create(updated);
	BOOST_REQUIRE(replacement.Is_Valid());
	BOOST_CHECK(replacement.Get_Index() == pose.Get_Index());
	BOOST_CHECK(replacement.Get_Generation() != pose.Get_Generation());
	BOOST_CHECK(!table.Update(pose, updated));
	BOOST_CHECK(table.Update(replacement, updated));
}

BOOST_AUTO_TEST_CASE(invalid_pose_handles_cannot_resolve_or_update)
{
	BoneMatrixTable table;
	table.Reserve(1, 1);
	const std::array<RenderTransform, 1> matrices = {Translation(0.0f, 0.0f, 0.0f)};
	BOOST_CHECK(!table.Is_Valid(PoseHandle{}));
	BOOST_CHECK(!table.Range(PoseHandle{}).Is_Valid());
	BOOST_CHECK(!table.Update(PoseHandle{}, matrices));
	BOOST_CHECK(!table.Destroy(PoseHandle{}));
}
