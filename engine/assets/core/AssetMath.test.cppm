module;

#define BOOST_TEST_MODULE GeneralsAssetsMathTests

#include <boost/test/included/unit_test.hpp>
#include <array>
#include <limits>

export module Assets.Tests.AssetMath;

import Assets.Math;

BOOST_AUTO_TEST_CASE(bounds_validate_finite_ordered_extents)
{
	const Assets::Bounds3f valid{{0.0f, 1.0f, 2.0f}, {3.0f, 4.0f, 5.0f}};
	BOOST_CHECK(valid.Is_Valid());

	const Assets::Bounds3f reversed{{3.0f, 1.0f, 2.0f}, {0.0f, 4.0f, 5.0f}};
	BOOST_CHECK(!reversed.Is_Valid());
}

BOOST_AUTO_TEST_CASE(color_space_round_trips_each_hue_sector_and_monochrome)
{
	for (const Assets::Vector3f rgb : {Assets::Vector3f{1, 0, 0}, {1, 1, 0}, {0, 1, 0},
		{0, 1, 1}, {0, 0, 1}, {1, 0, 1}, {0.25f, 0.7f, 0.45f}, {0.4f, 0.4f, 0.4f}, {0, 0, 0}}) {
		const auto hsv = Assets::RGB_To_HSV(rgb);
		const auto result = Assets::HSV_To_RGB(hsv);
		BOOST_CHECK_SMALL(result.x - rgb.x, 0.00001f);
		BOOST_CHECK_SMALL(result.y - rgb.y, 0.00001f);
		BOOST_CHECK_SMALL(result.z - rgb.z, 0.00001f);
		if (rgb.x == rgb.y && rgb.y == rgb.z) { BOOST_CHECK_LT(hsv.x, 0); BOOST_CHECK_EQUAL(hsv.y, 0); }
	}
	for (const float hue : {-720.0f, -360.0f, 0.0f, 360.0f, 720.0f}) {
		const auto rgb = Assets::HSV_To_RGB({hue, 1, 1});
		BOOST_CHECK_EQUAL(rgb.x, 1); BOOST_CHECK_EQUAL(rgb.y, 0); BOOST_CHECK_EQUAL(rgb.z, 0);
	}
}

BOOST_AUTO_TEST_CASE(hue_shift_preserves_alpha_and_monochrome_and_clamps_saturation_value)
{
	const Assets::Vector3f shift{120, 0, 0};
	for (unsigned alpha : {0u, 1u, 127u, 255u}) {
		const unsigned source = (alpha << 24) | 0x00ff0000;
		BOOST_CHECK_EQUAL(Assets::Shift_Color_ARGB(source, shift), (alpha << 24) | 0x0000ff00);
	}
	const auto grey = Assets::Shift_Color_HSV(Assets::Vector3f{0.25f, 0.25f, 0.25f}, {170, 1, 0.25f});
	BOOST_CHECK_EQUAL(grey.x, 0.5f); BOOST_CHECK_EQUAL(grey.y, 0.5f); BOOST_CHECK_EQUAL(grey.z, 0.5f);
	const auto white = Assets::Shift_Color_HSV(Assets::Vector3f{1, 0, 0}, {0, -2, 2});
	BOOST_CHECK_EQUAL(white.x, 1); BOOST_CHECK_EQUAL(white.y, 1); BOOST_CHECK_EQUAL(white.z, 1);
	const auto black = Assets::Shift_Color_HSV(Assets::Vector3f{1, 0, 0}, {0, 0, -2});
	BOOST_CHECK_EQUAL(black.x, 0); BOOST_CHECK_EQUAL(black.y, 0); BOOST_CHECK_EQUAL(black.z, 0);
}

BOOST_AUTO_TEST_CASE(packed_color_conversion_retains_all_byte_values_and_rounding)
{
	for (unsigned value = 0; value < 256; ++value) {
		for (unsigned channel = 0; channel < 4; ++channel) {
			const unsigned source = value << (channel * 8);
			BOOST_CHECK_EQUAL(Assets::Color_To_ARGB(Assets::Color_From_ARGB(source)), source);
			BOOST_CHECK_EQUAL(Assets::Shift_Color_ARGB(source, {}), source);
		}
	}
	BOOST_CHECK_EQUAL(Assets::Color_To_ARGB({-1, 0.5f, 2, 0.25f}), 0x400080ffu);
	BOOST_CHECK_EQUAL(Assets::Color_To_ARGB({std::numeric_limits<float>::quiet_NaN(), 0, 0, 1}), 0xff000000u);
}
