module;

#define BOOST_TEST_MODULE FontLoadTaskTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

export module Assets.Tests.FontLoadTask;

import Assets.Cache;
import Assets.Cache.FontLoadTask;
import Assets.Fonts;
import Assets.Handles;
import Assets.Identity;

namespace {

std::vector<std::byte> Read_Fixture()
{
    std::ifstream file(ASSETS_FONT_METRICS_FIXTURE, std::ios::binary | std::ios::ate);
    BOOST_REQUIRE(file.is_open());
    const auto size = file.tellg();
    BOOST_REQUIRE(size > 0);
    std::vector<std::byte> bytes(static_cast<std::size_t>(size));
    file.seekg(0);
    BOOST_REQUIRE(file.read(reinterpret_cast<char *>(bytes.data()), size));
    return bytes;
}

const Assets::FontGlyphAsset &Glyph(const Assets::FontAsset &font, char character)
{
    const auto found = std::find_if(font.Glyphs().begin(), font.Glyphs().end(),
        [character](const auto &glyph) { return glyph.character == character; });
    BOOST_REQUIRE(found != font.Glyphs().end());
    return *found;
}

Assets::FontLoadResult Load(const std::string &name)
{
    const auto bytes = Read_Fixture();
    return Assets::Load_Font_Asset({Assets::AssetType::Font, name},
        [&bytes](const Assets::AssetIdentity &) { return bytes; });
}

}

BOOST_AUTO_TEST_CASE(point_size_uses_em_units_and_excludes_external_line_gap)
{
    // The fixture has 1000 units/em, ascent 800, descent -200, line gap 100,
    // and an A advance of 600 units. Twelve points at 96 DPI is a 16-pixel em.
    const auto result = Load("font/metrics/12/0");
    BOOST_REQUIRE_MESSAGE(result.Succeeded(), result.error);
    BOOST_CHECK_EQUAL(result.asset->Height(), 16u);
    BOOST_CHECK_EQUAL(Glyph(*result.asset, 'A').spacing, 10);
    BOOST_CHECK_EQUAL(Glyph(*result.asset, ' ').spacing, 4);
    BOOST_CHECK(Glyph(*result.asset, ' ').alpha.empty());
    const auto &descender = Glyph(*result.asset, 'g');
    BOOST_REQUIRE_EQUAL(descender.alpha.size(), descender.width * 16u);
    BOOST_CHECK(std::any_of(descender.alpha.end() - descender.width,
        descender.alpha.end(), [](auto alpha) { return alpha != 0; }));
}

BOOST_AUTO_TEST_CASE(point_to_pixel_conversion_truncates_before_rasterization)
{
    const auto result = Load("font/metrics/11/0");
    BOOST_REQUIRE_MESSAGE(result.Succeeded(), result.error);
    BOOST_CHECK_EQUAL(result.asset->Height(), 14u);
    BOOST_CHECK_EQUAL(Glyph(*result.asset, 'A').spacing, 8);
}

BOOST_AUTO_TEST_CASE(requested_average_width_changes_glyphs_and_advances_only_horizontally)
{
    // OS/2 average width is 500 units: four pixels gives an x scale of .008.
    const auto result = Load("font/metrics/12/1/4");
    BOOST_REQUIRE_MESSAGE(result.Succeeded(), result.error);
    BOOST_CHECK(result.asset->Bold());
    BOOST_CHECK_EQUAL(result.asset->Height(), 16u);
    BOOST_CHECK_EQUAL(Glyph(*result.asset, 'A').spacing, 5);
    BOOST_CHECK_EQUAL(Glyph(*result.asset, 'A').width, 5u);
    BOOST_CHECK_EQUAL(Glyph(*result.asset, ' ').spacing, 2);
}

BOOST_AUTO_TEST_CASE(font_cache_distinguishes_width_and_reuses_identical_requests)
{
    const auto bytes = Read_Fixture();
    Assets::AssetCache cache([&bytes](const Assets::AssetIdentity &) { return bytes; });
    const auto regular = cache.Request_Font("Metrics", 12, false);
    const auto narrow = cache.Request_Font("Metrics", 12, false, 4);
    const auto larger = cache.Request_Font("Metrics", 24, false);
    BOOST_REQUIRE(regular.Is_Valid() && narrow.Is_Valid() && larger.Is_Valid());
    BOOST_CHECK(regular != narrow);
    BOOST_CHECK(cache.Request_Font("metrics", 12, false, 4) == narrow);
    BOOST_CHECK(cache.Request_Font("metrics", 12, false, 0) == regular);
    cache.Wait(regular);
    cache.Wait(narrow);
    cache.Wait(larger);
    BOOST_REQUIRE(cache.Try_Get_Font(regular) != nullptr);
    BOOST_REQUIRE(cache.Try_Get_Font(narrow) != nullptr);
    BOOST_REQUIRE(cache.Try_Get_Font(larger) != nullptr);
    BOOST_CHECK_EQUAL(Glyph(*cache.Try_Get_Font(regular), 'A').spacing, 10);
    BOOST_CHECK_EQUAL(Glyph(*cache.Try_Get_Font(narrow), 'A').spacing, 5);
    BOOST_CHECK_EQUAL(cache.Try_Get_Font(larger)->Height(), 32u);
    BOOST_CHECK_EQUAL(Glyph(*cache.Try_Get_Font(larger), 'A').spacing, 19);
}
