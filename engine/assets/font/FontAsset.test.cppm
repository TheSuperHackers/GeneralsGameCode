module;

#define BOOST_TEST_MODULE GeneralsAssetsFontTests

#include <boost/test/included/unit_test.hpp>

#include <cstdint>
#include <utility>
#include <vector>

export module Assets.Tests.FontAsset;

import Assets.Fonts;

BOOST_AUTO_TEST_CASE(font_payload_keeps_metrics_and_sorted_glyphs)
{
	Assets::FontGlyphAsset zulu;
	zulu.character = static_cast<std::uint16_t>('Z');
	zulu.width = 4;
	zulu.spacing = 5;
	zulu.alpha.assign(20, 0xff);

	Assets::FontGlyphAsset alpha;
	alpha.character = static_cast<std::uint16_t>('A');
	alpha.width = 3;
	alpha.spacing = 4;
	alpha.alpha.assign(15, 0x7f);

	const Assets::FontAsset asset("Arial", 12, true, 5, 1, {std::move(zulu), std::move(alpha)});
	BOOST_CHECK(asset.Family() == "Arial");
	BOOST_CHECK_EQUAL(asset.Point_Size(), 12);
	BOOST_CHECK(asset.Bold());
	BOOST_CHECK_EQUAL(asset.Height(), 5);
	BOOST_CHECK_EQUAL(asset.Extra_Overlap(), 1);
	BOOST_REQUIRE_EQUAL(asset.Glyphs().size(), 2);
	BOOST_CHECK(asset.Glyphs()[0].character == static_cast<std::uint16_t>('A'));
	BOOST_CHECK(asset.Glyphs()[1].character == static_cast<std::uint16_t>('Z'));
}
