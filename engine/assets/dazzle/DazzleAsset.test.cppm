module;
#define BOOST_TEST_MODULE DazzleAssetTests
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <utility>
export module Assets.Dazzles.Tests;
import Assets.Dazzles;

BOOST_AUTO_TEST_CASE(definitions_own_names_and_sprite_arrays_across_source_release_and_copy) {
    Assets::DazzleDefinitions definitions;
    {
        Assets::LensFlareDefinition flare;
        flare.name = "Flare";
        flare.texture = "flare.tga";
        flare.sprites.resize(17000);
        flare.sprites.back().location = -.75f;
        flare.sprites.back().uv = {.2f,.3f,.4f,.5f};
        definitions.lens_flares.push_back(std::move(flare));
        Assets::DazzleDefinition dazzle;
        dazzle.name = "Lamp";
        dazzle.lens_flare = "Flare";
        definitions.dazzles.push_back(std::move(dazzle));
    }
    auto copy = definitions;
    definitions.lens_flares.back().sprites.back().location = 9;
    definitions = {};
    BOOST_CHECK_EQUAL(copy.dazzles.front().lens_flare, "Flare");
    BOOST_CHECK_EQUAL(copy.lens_flares.front().texture, "flare.tga");
    BOOST_REQUIRE_EQUAL(copy.lens_flares.front().sprites.size(), 17000);
    BOOST_CHECK_EQUAL(copy.lens_flares.front().sprites.back().location, -.75f);
    BOOST_CHECK_EQUAL(copy.lens_flares.front().sprites.back().uv[3], .5f);
}
