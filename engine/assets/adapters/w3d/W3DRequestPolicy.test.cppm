module;

#define BOOST_TEST_MODULE W3DRequestPolicyTests
#include <boost/test/included/unit_test.hpp>

#include <optional>
#include <string>

export module Assets.Adapters.W3D.RequestPolicy.Tests;

import Assets.Adapters.W3D.RequestPolicy;

using Assets::W3D::W3DRequestKind;
using Assets::W3D::W3D_Make_Request_Paths;
using Assets::W3D::W3D_Request_Filename;
using Assets::W3D::W3D_Should_Attempt_Animation_Load;

BOOST_AUTO_TEST_CASE(model_requests_use_the_first_dot_and_preserve_spelling)
{
	const auto nested = W3D_Request_Filename(W3DRequestKind::Model, "Tank.Turret.Barrel");
	BOOST_REQUIRE(nested);
	BOOST_CHECK_EQUAL(*nested, "Tank.w3d");

	const auto plain = W3D_Request_Filename(W3DRequestKind::Model, "MiXeD_Model");
	BOOST_REQUIRE(plain);
	BOOST_CHECK_EQUAL(*plain, "MiXeD_Model.w3d");
}

BOOST_AUTO_TEST_CASE(animation_requests_use_the_complete_suffix)
{
	const auto animation = W3D_Request_Filename(W3DRequestKind::Animation, "Tank.Walk.Fast");
	BOOST_REQUIRE(animation);
	BOOST_CHECK_EQUAL(*animation, "Walk.Fast.w3d");

	BOOST_CHECK(!W3D_Request_Filename(W3DRequestKind::Animation, "Walk"));
	BOOST_CHECK(!W3D_Request_Filename(W3DRequestKind::Animation, ""));
}

BOOST_AUTO_TEST_CASE(skeleton_requests_use_the_complete_name)
{
	const auto skeleton = W3D_Request_Filename(W3DRequestKind::Skeleton, "TankRig");
	BOOST_REQUIRE(skeleton);
	BOOST_CHECK_EQUAL(*skeleton, "TankRig.w3d");
	BOOST_CHECK(!W3D_Request_Filename(W3DRequestKind::Skeleton, ""));
}

BOOST_AUTO_TEST_CASE(parent_directory_is_only_a_second_explicit_attempt)
{
	const auto paths = W3D_Make_Request_Paths(W3DRequestKind::Model, "Tank.Turret");
	BOOST_REQUIRE(paths);
	BOOST_CHECK_EQUAL(paths->primary, "Tank.w3d");
	BOOST_CHECK_EQUAL(paths->parent_directory, "..\\Tank.w3d");

	BOOST_CHECK(!W3D_Make_Request_Paths(W3DRequestKind::Animation, "Walk"));
}

BOOST_AUTO_TEST_CASE(animation_missing_cache_suppresses_repeated_requests)
{
	BOOST_CHECK(W3D_Should_Attempt_Animation_Load(true, false));
	BOOST_CHECK(!W3D_Should_Attempt_Animation_Load(true, true));
	BOOST_CHECK(!W3D_Should_Attempt_Animation_Load(false, false));
	BOOST_CHECK(!W3D_Should_Attempt_Animation_Load(false, true));
}
