module;
#define BOOST_TEST_MODULE W3DRetentionTests
#include <boost/test/included/unit_test.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
export module Assets.Adapters.W3D.Retention.Tests;
import Assets.Adapters.W3D.Retention;
using namespace Assets::W3D;

BOOST_AUTO_TEST_CASE(model_retention_uses_root_identity_without_normalization)
{
    const std::unordered_set<std::string> retained{"Tank","A/B","","Rock"};
    const auto keep=[&](std::string_view name) {
        const auto key=W3D_Model_Retention_Key(name);
        return key && retained.contains(std::string(*key));
    };
    for(const auto* name:{"Tank","Tank.Turret","Tank.Turret.Barrel","Rock","A/B.Part",".Part",""})
        BOOST_CHECK(keep(name));
    for(const auto* name:{"tank","TANK.Turret","Tank#Red","Tank.Turret#Red","#Tank","A\\B.Part","Unknown"})
        BOOST_CHECK(!keep(name));
    const auto key=W3D_Model_Retention_Key("Tank.Turret");BOOST_REQUIRE(key);
    BOOST_CHECK_EQUAL(*key,"Tank");
}

BOOST_AUTO_TEST_CASE(animation_retention_uses_complete_suffix_and_requires_separator)
{
    const std::unordered_set<std::string> retained{"Walk","Walk.Fast","Walk#Variant",""};
    const auto keep=[&](std::string_view name) {
        const auto key=W3D_Animation_Retention_Key(name);
        return key && retained.contains(std::string(*key));
    };
    for(const auto* name:{"Tank.Walk","Other.Walk.Fast","Tank.Walk#Variant","Tank.",".Walk"})
        BOOST_CHECK(keep(name));
    for(const auto* name:{"Walk","","Tank.walk","Tank.Walk.Slow","Tank.Other.Walk"})
        BOOST_CHECK(!keep(name));
}
