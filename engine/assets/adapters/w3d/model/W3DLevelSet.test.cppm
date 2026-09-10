module;
#define BOOST_TEST_MODULE W3DLevelSetTests
#include <boost/test/included/unit_test.hpp>
#include "W3DAssembly.test-data.h"
#include <span>
#include <string>
export module Assets.Adapters.W3D.LevelSet.Tests;
import Assets.Adapters.W3D.LevelSet;
using namespace Assets;
using namespace AssemblyTestData;
BOOST_AUTO_TEST_CASE(level_sets_own_authored_order_and_distance_metadata)
{
    ModelLevelSetDesc source;std::string error;
    auto bytes=LevelSet();
    BOOST_REQUIRE(W3D::W3DRead_Model_Level_Set(bytes,source,error));
    const auto instance=source;source={};bytes.clear();
    BOOST_CHECK_EQUAL(instance.name,"LEVELS");
    BOOST_REQUIRE_EQUAL(instance.levels.size(),2);
    BOOST_CHECK_EQUAL(instance.levels.front().name,"HIGH");
    BOOST_CHECK_EQUAL(instance.levels.back().name,"LOW");
    BOOST_CHECK_EQUAL(instance.levels.front().minimum_distance,0);
    BOOST_CHECK_EQUAL(instance.levels.back().maximum_distance,11);
}
BOOST_AUTO_TEST_CASE(level_sets_are_bounded_atomic_and_have_no_256_level_limit)
{
    ModelLevelSetDesc result;result.name="UNCHANGED";std::string error;
    const auto bytes=LevelSet();
    for(std::size_t count=0;count<bytes.size();++count) {
        BOOST_CHECK(!W3D::W3DRead_Model_Level_Set(std::span(bytes).first(count),result,error));
        BOOST_CHECK_EQUAL(result.name,"UNCHANGED");
    }
    BOOST_CHECK(!W3D::W3DRead_Model_Level_Set(LevelSet(0),result,error));
    BOOST_REQUIRE(W3D::W3DRead_Model_Level_Set(LevelSet(257),result,error));
    BOOST_CHECK_EQUAL(result.levels.size(),257);
    BOOST_CHECK_EQUAL(result.levels.back().minimum_distance,256);
}
