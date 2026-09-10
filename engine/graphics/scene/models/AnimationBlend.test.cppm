module;

#define BOOST_TEST_MODULE GraphicsAnimationBlendTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cmath>

export module Graphics.Scene.Models.AnimationBlend.Tests;

import Graphics.Scene.Models.AnimationBlend;

using namespace Graphics;

static RenderTransform Translation(float x, float y, float z) noexcept
{
	RenderTransform transform;
	transform.matrix = {
		1.0f, 0.0f, 0.0f, x,
		0.0f, 1.0f, 0.0f, y,
		0.0f, 0.0f, 1.0f, z,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return transform;
}

static RenderTransform Rotation_Z_Scale(float angle, float scale) noexcept
{
	const float cosine = std::cos(angle) * scale;
	const float sine = std::sin(angle) * scale;
	RenderTransform transform;
	transform.matrix = {
		cosine, -sine, 0.0f, 0.0f,
		sine, cosine, 0.0f, 0.0f,
		0.0f, 0.0f, scale, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	return transform;
}

static bool Same_Transform(const RenderTransform &left, const RenderTransform &right) noexcept
{
	for (std::size_t element = 0; element < left.matrix.size(); ++element)
		if (left.matrix[element] != right.matrix[element])
			return false;
	return true;
}

BOOST_AUTO_TEST_CASE(blend_weight_boundaries_preserve_source_poses)
{
	const RenderTransform first = Translation(1.0f, 2.0f, 3.0f);
	const RenderTransform second = Translation(4.0f, 5.0f, 6.0f);
	RenderTransform output;

	BOOST_REQUIRE(Blend_Transforms(first, second, 0.0f, output));
	BOOST_CHECK(Same_Transform(output, first));
	BOOST_REQUIRE(Blend_Transforms(first, second, 1.0f, output));
	BOOST_CHECK(Same_Transform(output, second));
}

BOOST_AUTO_TEST_CASE(blend_interpolates_translation_and_scale)
{
	const RenderTransform first = Translation(1.0f, 2.0f, 3.0f);
	RenderTransform second = Translation(5.0f, 6.0f, 7.0f);
	second.matrix[0] = 3.0f;
	second.matrix[5] = 4.0f;
	second.matrix[10] = 5.0f;
	RenderTransform output;

	BOOST_REQUIRE(Blend_Transforms(first, second, 0.5f, output));
	BOOST_CHECK_CLOSE(output.matrix[3], 3.0f, 0.001);
	BOOST_CHECK_CLOSE(output.matrix[7], 4.0f, 0.001);
	BOOST_CHECK_CLOSE(output.matrix[11], 5.0f, 0.001);
	BOOST_CHECK_CLOSE(output.matrix[0], 2.0f, 0.001);
	BOOST_CHECK_CLOSE(output.matrix[5], 2.5f, 0.001);
	BOOST_CHECK_CLOSE(output.matrix[10], 3.0f, 0.001);
}

BOOST_AUTO_TEST_CASE(blend_uses_shortest_rotation_interpolation)
{
	const RenderTransform first = Rotation_Z_Scale(0.0f, 1.0f);
	const RenderTransform second = Rotation_Z_Scale(1.57079632679489661923f, 1.0f);
	RenderTransform output;

	BOOST_REQUIRE(Blend_Transforms(first, second, 0.5f, output));
	BOOST_CHECK_CLOSE(output.matrix[0], 0.7071067f, 0.001);
	BOOST_CHECK_CLOSE(output.matrix[1], -0.7071067f, 0.001);
	BOOST_CHECK_CLOSE(output.matrix[4], 0.7071067f, 0.001);
	BOOST_CHECK_CLOSE(output.matrix[5], 0.7071067f, 0.001);
}

BOOST_AUTO_TEST_CASE(blend_evaluates_hierarchy_after_local_blend)
{
	const std::array<SkeletonBone, 2> bones = {{
		{Invalid_Bone_Index, Translation(0.0f, 0.0f, 0.0f)},
		{0, Translation(0.0f, 0.0f, 0.0f)}
	}};
	const Skeleton skeleton(bones, {});
	Pose output;
	BOOST_REQUIRE(output.Initialize(2));

	const std::array<RenderTransform, 2> first = {
		Translation(1.0f, 0.0f, 0.0f), Translation(2.0f, 0.0f, 0.0f)
	};
	const std::array<RenderTransform, 2> second = {
		Translation(3.0f, 0.0f, 0.0f), Translation(4.0f, 0.0f, 0.0f)
	};

	BOOST_REQUIRE(Blend_Poses(skeleton, first, second, 0.5f, output));
	BOOST_CHECK_CLOSE(output.Local_Transforms()[0].matrix[3], 2.0f, 0.001);
	BOOST_CHECK_CLOSE(output.Local_Transforms()[1].matrix[3], 3.0f, 0.001);
	BOOST_CHECK_CLOSE(output.World_Transforms()[1].matrix[3], 5.0f, 0.001);
}

BOOST_AUTO_TEST_CASE(blend_output_is_deterministic_and_rejects_invalid_input)
{
	const std::array<RenderTransform, 1> first = {Translation(0.0f, 0.0f, 0.0f)};
	const std::array<RenderTransform, 1> second = {Translation(2.0f, 4.0f, 6.0f)};
	std::array<RenderTransform, 1> output_a{};
	std::array<RenderTransform, 1> output_b{};

	BOOST_REQUIRE(Blend_Local_Poses(first, second, 0.5f, output_a));
	BOOST_REQUIRE(Blend_Local_Poses(first, second, 0.5f, output_b));
	BOOST_CHECK(Same_Transform(output_a[0], output_b[0]));
	BOOST_CHECK(!Blend_Local_Poses(first, {}, 0.5f, output_a));
	BOOST_CHECK(!Blend_Local_Poses(first, second, NAN, output_a));
}
