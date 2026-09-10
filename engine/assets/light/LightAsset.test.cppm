module;

#define BOOST_TEST_MODULE AssetsLightTests

#include <boost/test/included/unit_test.hpp>

#include <limits>

export module Assets.Tests.Lights;

import Assets.Lights;

BOOST_AUTO_TEST_CASE(default_light_description_matches_light_class_defaults)
{
	Assets::LightAssetDesc light;
	BOOST_CHECK(light.type == Assets::LightType::Point);
	BOOST_CHECK(!light.cast_shadows);
	BOOST_CHECK_EQUAL(light.intensity, 1.0f);
	BOOST_CHECK_EQUAL(light.ambient.x, 1.0f);
	BOOST_CHECK_EQUAL(light.diffuse.y, 1.0f);
	BOOST_CHECK_EQUAL(light.specular.z, 1.0f);
	BOOST_CHECK(!light.near_attenuation_enabled);
	BOOST_CHECK_EQUAL(light.near_attenuation_start, 0.0f);
	BOOST_CHECK_EQUAL(light.near_attenuation_end, 0.0f);
	BOOST_CHECK(!light.far_attenuation_enabled);
	BOOST_CHECK_EQUAL(light.far_attenuation_start, 50.0f);
	BOOST_CHECK_EQUAL(light.far_attenuation_end, 100.0f);
	BOOST_CHECK_EQUAL(light.spot_direction.z, 1.0f);
	BOOST_CHECK_CLOSE(light.spot_angle, 0.7853981633974483f, 0.001);
	BOOST_CHECK_EQUAL(light.spot_exponent, 1.0f);
	BOOST_CHECK(Assets::Is_Valid_Light_Asset(light));
}

BOOST_AUTO_TEST_CASE(light_description_validation_rejects_nonfinite_values)
{
	Assets::LightAssetDesc light;
	light.intensity = std::numeric_limits<float>::infinity();
	BOOST_CHECK(!Assets::Is_Valid_Light_Asset(light));
	light.intensity = 1.0f;
	light.spot_direction.x = std::numeric_limits<float>::quiet_NaN();
	BOOST_CHECK(!Assets::Is_Valid_Light_Asset(light));
}
