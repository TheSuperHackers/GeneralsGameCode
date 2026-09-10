module;

#define BOOST_TEST_MODULE GraphicsAnimationTests

#include <boost/test/included/unit_test.hpp>

#include <array>

export module Graphics.Scene.Models.Animation.Tests;

import Graphics.Scene.Models.Animation;

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

BOOST_AUTO_TEST_CASE(animation_clip_samples_contiguous_local_transforms)
{
	std::array<RenderTransform, 4> frames{
		Translation(0.0f, 0.0f, 0.0f), Translation(1.0f, 0.0f, 0.0f),
		Translation(2.0f, 0.0f, 0.0f), Translation(3.0f, 0.0f, 0.0f)
	};
	const AnimationClip clip(2, 2, 1.0f, frames);
	std::array<RenderTransform, 2> output{};

	BOOST_REQUIRE(clip.Is_Valid());
	BOOST_REQUIRE(clip.Sample(0.5f, AnimationPlaybackMode::Once, output));
	BOOST_CHECK_CLOSE(output[0].matrix[3], 1.0f, 0.001);
	BOOST_CHECK_CLOSE(output[1].matrix[3], 2.0f, 0.001);

	std::array<RenderTransform, 2> repeat{};
	BOOST_REQUIRE(clip.Sample(0.5f, AnimationPlaybackMode::Once, repeat));
	for (std::size_t bone = 0; bone < output.size(); ++bone)
		for (std::size_t element = 0; element < output[bone].matrix.size(); ++element)
			BOOST_CHECK(output[bone].matrix[element] == repeat[bone].matrix[element]);
}

BOOST_AUTO_TEST_CASE(animation_clip_loops_and_supports_reverse_playback)
{
	std::array<RenderTransform, 2> frames{Translation(0.0f, 0.0f, 0.0f), Translation(4.0f, 0.0f, 0.0f)};
	const AnimationClip clip(1, 2, 1.0f, frames);
	std::array<RenderTransform, 1> output{};

	BOOST_REQUIRE(clip.Sample(2.0f, AnimationPlaybackMode::Loop, output));
	BOOST_CHECK_CLOSE(output[0].matrix[3], 0.0f, 0.001);
	BOOST_REQUIRE(clip.Sample(0.5f, AnimationPlaybackMode::Loop_Backwards, output));
	BOOST_CHECK_CLOSE(output[0].matrix[3], 2.0f, 0.001);
}

BOOST_AUTO_TEST_CASE(animation_clip_rejects_invalid_data)
{
	std::array<RenderTransform, 1> frame{Translation(0.0f, 0.0f, 0.0f)};
	BOOST_CHECK(!Validate_Animation_Clip_Description({0, 1, 1.0f, frame}));
	BOOST_CHECK(!Validate_Animation_Clip_Description({1, 1, 0.0f, frame}));
	BOOST_CHECK(!Validate_Animation_Clip_Description({1, 2, 1.0f, frame}));
	BOOST_CHECK(!AnimationClip(1, 2, 1.0f, frame).Is_Valid());
}

BOOST_AUTO_TEST_CASE(pose_evaluation_composes_hierarchy_deterministically)
{
	std::array<SkeletonBone, 2> bones{
		SkeletonBone{Invalid_Bone_Index, Translation(1.0f, 0.0f, 0.0f)},
		SkeletonBone{0, Translation(0.0f, 2.0f, 0.0f)}
	};
	const Skeleton skeleton(bones, {});
	std::array<RenderTransform, 2> local{
		Translation(3.0f, 0.0f, 0.0f), Translation(0.0f, 4.0f, 0.0f)
	};
	Pose pose;

	BOOST_REQUIRE(pose.Initialize(2));
	BOOST_REQUIRE(pose.Evaluate(skeleton, local));
	BOOST_CHECK_CLOSE(pose.World_Transforms()[0].matrix[3], 3.0f, 0.001);
	BOOST_CHECK_CLOSE(pose.World_Transforms()[0].matrix[7], 0.0f, 0.001);
	BOOST_CHECK_CLOSE(pose.World_Transforms()[1].matrix[3], 3.0f, 0.001);
	BOOST_CHECK_CLOSE(pose.World_Transforms()[1].matrix[7], 4.0f, 0.001);
}
