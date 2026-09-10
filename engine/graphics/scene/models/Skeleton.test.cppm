module;

#define BOOST_TEST_MODULE GraphicsSkeletonTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.Scene.Models.Skeleton.Tests;

import Graphics.Scene.Models.Skeleton;

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

BOOST_AUTO_TEST_CASE(skeleton_preserves_hierarchy_and_composes_rest_transforms)
{
	std::array<SkeletonBone, 2> bones{
		SkeletonBone{Invalid_Bone_Index, Translation(1.0f, 2.0f, 3.0f)},
		SkeletonBone{0, Translation(4.0f, 5.0f, 6.0f)}
	};
	const Skeleton skeleton(bones, {});

	BOOST_REQUIRE(skeleton.Is_Valid());
	const BoneHandle child = skeleton.Bone(1);
	BOOST_CHECK(skeleton.Parent(child) == 0);
	RenderTransform transform;
	BOOST_REQUIRE(skeleton.Rest_Transform(child, transform));
	BOOST_CHECK(transform.matrix[3] == 5.0f);
	BOOST_CHECK(transform.matrix[7] == 7.0f);
	BOOST_CHECK(transform.matrix[11] == 9.0f);
}

BOOST_AUTO_TEST_CASE(skeleton_rejects_invalid_parent_order_and_bones)
{
	std::array<SkeletonBone, 2> bones{
		SkeletonBone{1, Translation(0.0f, 0.0f, 0.0f)},
		SkeletonBone{Invalid_Bone_Index, Translation(0.0f, 0.0f, 0.0f)}
	};
	BOOST_CHECK(!Validate_Skeleton_Description({bones, {}}));

	const Skeleton skeleton({}, {});
	BOOST_CHECK(!skeleton.Is_Valid());
	BOOST_CHECK(!skeleton.Is_Valid_Bone(BoneHandle(0, 1)));
}

BOOST_AUTO_TEST_CASE(skeleton_attachment_queries_compose_bone_and_local_transforms)
{
	std::array<SkeletonBone, 1> bones{
		SkeletonBone{Invalid_Bone_Index, Translation(2.0f, 3.0f, 4.0f)}
	};
	const BoneHandle bone(0, 1);
	std::array<SkeletonAttachment, 1> attachments{
		SkeletonAttachment{bone, Translation(5.0f, 6.0f, 7.0f)}
	};
	const Skeleton skeleton(bones, attachments);

	BOOST_REQUIRE(skeleton.Is_Valid());
	RenderTransform transform;
	BOOST_REQUIRE(skeleton.Attachment_Transform(skeleton.Attachment(0), transform));
	BOOST_CHECK(transform.matrix[3] == 7.0f);
	BOOST_CHECK(transform.matrix[7] == 9.0f);
	BOOST_CHECK(transform.matrix[11] == 11.0f);
	BOOST_CHECK(!skeleton.Attachment_Transform(AttachmentHandle(0, 2), transform));
}
