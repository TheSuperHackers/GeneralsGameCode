module;
#define BOOST_TEST_MODULE ModelAssemblyTests
#include <boost/test/included/unit_test.hpp>
#include <limits>
#include <string>
export module Assets.ModelAssembly.Tests;
import Assets.ModelAssembly;
using namespace Assets;
BOOST_AUTO_TEST_CASE(assembly_keeps_level_attachments_and_application_metadata_separate) {
    ModelAssemblyDesc description;description.name="TANK";description.skeleton_name="RIG";
    description.levels={{100,{{"TANK.BODY",1}}},{200,{{"TANK.BODY_LOW",1}}}};
    description.aggregates={{"FLAG",2}};description.proxies={{"MUZZLE",3}};
    description.snap_points={{1,2,3}};std::string error;
    BOOST_REQUIRE(Validate_Model_Assembly(description,error));
    const auto copy=description;description.levels.clear();description.proxies.clear();
    BOOST_CHECK_EQUAL(copy.levels[1].children[0].object_name,"TANK.BODY_LOW");
    BOOST_CHECK_EQUAL(copy.aggregates[0].bone,2);
    BOOST_CHECK_EQUAL(copy.proxies[0].bone,3);
    BOOST_CHECK_EQUAL(copy.snap_points[0].z,3);
}
BOOST_AUTO_TEST_CASE(rejects_invalid_assembly_dimensions_and_attachment_data) {
    ModelAssemblyDesc description;std::string error;
    BOOST_CHECK(!Validate_Model_Assembly(description,error));
    description.name="ROOT_ONLY";description.levels.emplace_back();
    BOOST_CHECK(Validate_Model_Assembly(description,error));
    description.levels[0].maximum_screen_size=std::numeric_limits<float>::infinity();
    BOOST_CHECK(!Validate_Model_Assembly(description,error));
    description.levels[0].maximum_screen_size=100;
    description.proxies={{"",0}};BOOST_CHECK(!Validate_Model_Assembly(description,error));
    description.proxies={{"POINT",(std::numeric_limits<std::uint32_t>::max)()}};
    BOOST_CHECK(!Validate_Model_Assembly(description,error));
}
