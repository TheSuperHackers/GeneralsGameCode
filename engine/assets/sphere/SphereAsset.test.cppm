module;

#define BOOST_TEST_MODULE AssetsSphereTests

#include <boost/test/included/unit_test.hpp>

#include <array>
#include <cmath>
#include <limits>

export module Assets.Tests.Spheres;

import Assets.Math;
import Assets.Spheres;

BOOST_AUTO_TEST_CASE(default_sphere_description_is_finite_and_named)
{
	Assets::SphereAssetDesc asset;
	asset.name = "test.sphere";
	BOOST_CHECK(Assets::Is_Valid_Sphere_Asset(asset));
	BOOST_CHECK((asset.attributes & Assets::SphereAttributeUseAlphaVector) != 0);
	const std::array<float, 4> expected_rotation{0, 0, 0, 1};
	BOOST_CHECK(asset.default_vector_rotation == expected_rotation);
}

BOOST_AUTO_TEST_CASE(track_order_and_numeric_values_are_validated)
{
	Assets::SphereAssetDesc asset;
	asset.name = "test.sphere";
	asset.color_track.keys = {{0.0f, {0, 0, 0}}, {1.0f, {1, 1, 1}}};
	BOOST_CHECK(Assets::Is_Valid_Sphere_Asset(asset));
	asset.color_track.keys[1].time = 0.0f;
	BOOST_CHECK(!Assets::Is_Valid_Sphere_Asset(asset));
	asset.color_track.keys[1].time = 1.0f;
	asset.default_alpha = std::numeric_limits<float>::infinity();
	BOOST_CHECK(!Assets::Is_Valid_Sphere_Asset(asset));
}
