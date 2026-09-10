module;

#define BOOST_TEST_MODULE GraphicsModelInstanceTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.Scene.Models.ModelInstance.Tests;

import Graphics.Scene.Models.ModelInstance;

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

BOOST_AUTO_TEST_CASE(model_instance_composes_model_transform_with_bone)
{
	std::array<SkeletonBone, 1> bones{
		SkeletonBone{Invalid_Bone_Index, Translation(1.0f, 2.0f, 3.0f)}
	};
	SkeletonPool skeletons;
	const SkeletonHandle skeleton = Create_Skeleton(skeletons, {bones, {}});
	BOOST_REQUIRE(skeleton.Is_Valid());

	ModelInstance instance;
	instance.skeleton = skeleton;
	instance.transform = Translation(10.0f, 20.0f, 30.0f);
	RenderTransform result;
	BOOST_REQUIRE(instance.Get_Bone_Transform(skeletons, BoneHandle(0, 1), result));
	BOOST_CHECK(result.matrix[3] == 11.0f);
	BOOST_CHECK(result.matrix[7] == 22.0f);
	BOOST_CHECK(result.matrix[11] == 33.0f);
}

BOOST_AUTO_TEST_CASE(model_instance_rejects_invalid_resources_and_attachment_handles)
{
	SkeletonPool skeletons;
	ModelInstance instance;
	RenderTransform result;
	BOOST_CHECK(!instance.Get_Bone_Transform(skeletons, BoneHandle(0, 1), result));

	std::array<SkeletonBone, 1> bones{
		SkeletonBone{Invalid_Bone_Index, Translation(0.0f, 0.0f, 0.0f)}
	};
	instance.skeleton = Create_Skeleton(skeletons, {bones, {}});
	BOOST_REQUIRE(instance.skeleton.Is_Valid());
	BOOST_CHECK(!instance.Get_Attachment_Transform(skeletons, AttachmentHandle(0, 1), result));
	BOOST_CHECK(!instance.Get_Bone_Transform(skeletons, BoneHandle(0, 2), result));
}

BOOST_AUTO_TEST_CASE(model_instance_updates_bones_and_attachments_from_animation)
{
	std::array<SkeletonBone, 2> bones{
		SkeletonBone{Invalid_Bone_Index, Translation(0.0f, 0.0f, 0.0f)},
		SkeletonBone{0, Translation(0.0f, 0.0f, 0.0f)}
	};
	std::array<SkeletonAttachment, 1> attachments{
		SkeletonAttachment{BoneHandle(1, 1), Translation(1.0f, 0.0f, 0.0f)}
	};
	SkeletonPool skeletons;
	const SkeletonHandle skeleton = Create_Skeleton(skeletons, {bones, attachments});
	BOOST_REQUIRE(skeleton.Is_Valid());

	std::array<RenderTransform, 4> frames{
		Translation(0.0f, 0.0f, 0.0f), Translation(0.0f, 0.0f, 0.0f),
		Translation(0.0f, 0.0f, 0.0f), Translation(4.0f, 0.0f, 0.0f)
	};
	AnimationClipPool animations;
	const AnimationClipHandle animation = Create_Animation_Clip(animations, {2, 2, 1.0f, frames});
	BOOST_REQUIRE(animation.Is_Valid());

	ModelInstance instance;
	instance.skeleton = skeleton;
	instance.transform = Translation(10.0f, 0.0f, 0.0f);
	BOOST_REQUIRE(instance.Set_Animation(skeletons, animations, animation, AnimationPlaybackMode::Once, 0.5f));

	RenderTransform bone_transform;
	BOOST_REQUIRE(instance.Get_Bone_Transform(skeletons, BoneHandle(1, 1), bone_transform));
	BOOST_CHECK_CLOSE(bone_transform.matrix[3], 12.0f, 0.001);

	RenderTransform attachment_transform;
	BOOST_REQUIRE(instance.Get_Attachment_Transform(skeletons, AttachmentHandle(0, 1), attachment_transform));
	BOOST_CHECK_CLOSE(attachment_transform.matrix[3], 13.0f, 0.001);

	BOOST_REQUIRE(instance.Advance_Animation(skeletons, animations, 1.0f));
	BOOST_REQUIRE(instance.Get_Bone_Transform(skeletons, BoneHandle(1, 1), bone_transform));
	BOOST_CHECK_CLOSE(bone_transform.matrix[3], 14.0f, 0.001);
}

BOOST_AUTO_TEST_CASE(model_instance_queries_the_blended_pose)
{
	const std::array<SkeletonBone, 2> bones{
		SkeletonBone{Invalid_Bone_Index, Translation(0.0f, 0.0f, 0.0f)},
		SkeletonBone{0, Translation(0.0f, 0.0f, 0.0f)}
	};
	const std::array<SkeletonAttachment, 1> attachments{
		SkeletonAttachment{BoneHandle(1, 1), Translation(1.0f, 0.0f, 0.0f)}
	};
	SkeletonPool skeletons;
	const SkeletonHandle skeleton = Create_Skeleton(skeletons, {bones, attachments});
	BOOST_REQUIRE(skeleton.Is_Valid());

	const std::array<RenderTransform, 4> first_frames{
		Translation(0.0f, 0.0f, 0.0f), Translation(1.0f, 0.0f, 0.0f),
		Translation(0.0f, 0.0f, 0.0f), Translation(2.0f, 0.0f, 0.0f)
	};
	const std::array<RenderTransform, 4> second_frames{
		Translation(4.0f, 0.0f, 0.0f), Translation(5.0f, 0.0f, 0.0f),
		Translation(4.0f, 0.0f, 0.0f), Translation(6.0f, 0.0f, 0.0f)
	};
	AnimationClipPool animations;
	const AnimationClipHandle first = Create_Animation_Clip(animations, {2, 2, 1.0f, first_frames});
	const AnimationClipHandle second = Create_Animation_Clip(animations, {2, 2, 1.0f, second_frames});
	BOOST_REQUIRE(first.Is_Valid());
	BOOST_REQUIRE(second.Is_Valid());

	ModelInstance instance;
	instance.skeleton = skeleton;
	instance.transform = Translation(10.0f, 0.0f, 0.0f);
	BOOST_REQUIRE(instance.Set_Animation_Blend(skeletons, animations, first,
		AnimationPlaybackMode::Once, 0.5f, second, AnimationPlaybackMode::Once, 0.5f, 0.5f));

	RenderTransform attachment_transform;
	BOOST_REQUIRE(instance.Get_Attachment_Transform(skeletons, AttachmentHandle(0, 1), attachment_transform));
	BOOST_CHECK_CLOSE(attachment_transform.matrix[3], 16.5f, 0.001);
	BOOST_CHECK_CLOSE(instance.pose.World_Transforms()[1].matrix[3], 5.5f, 0.001);
}
