module;
#define BOOST_TEST_MODULE W3DCollectionTests
#include <boost/test/included/unit_test.hpp>
#include "W3DAssembly.test-data.h"
#include <array>
#include <limits>
#include <span>
#include <string>
export module Assets.Adapters.W3D.Collection.Tests;
import Assets.Adapters.W3D.Collection;
using namespace Assets;
using namespace AssemblyTestData;

BOOST_AUTO_TEST_CASE(collection_owns_names_points_and_transposed_proxy_matrix)
{
    ModelCollectionDesc source;std::string error;
    auto bytes=Collection();
    BOOST_REQUIRE_MESSAGE(W3D::W3DRead_Model_Collection(bytes,source,error),error);
    const auto instance=source;source={};bytes.clear();
    BOOST_CHECK_EQUAL(instance.name,"COLLECTION");
    BOOST_REQUIRE_EQUAL(instance.children.size(),2);
    BOOST_CHECK_EQUAL(instance.children[0],"Left");
    BOOST_CHECK_EQUAL(instance.children[1],"Right");
    BOOST_REQUIRE_EQUAL(instance.proxies.size(),1);
    BOOST_CHECK_EQUAL(instance.proxies[0].name,"LEFT");
    const std::array<float,12> expected{0,-1,0,-.5f, 1,0,0,0, 0,0,1,0};
    BOOST_CHECK_EQUAL_COLLECTIONS(instance.proxies[0].transform.begin(),instance.proxies[0].transform.end(),expected.begin(),expected.end());
    BOOST_REQUIRE_EQUAL(instance.snap_points.size(),1);
    BOOST_CHECK_EQUAL(instance.snap_points[0].x,1);
    BOOST_CHECK_EQUAL(instance.snap_points[0].y,2);
    BOOST_CHECK_EQUAL(instance.snap_points[0].z,3);
}
BOOST_AUTO_TEST_CASE(truncation_duplicate_points_and_invalid_counts_publish_nothing)
{
    const auto bytes=Collection();std::string error;
    ModelCollectionDesc result;result.name="UNCHANGED";
    for(std::size_t size=0;size<bytes.size();++size) {
        if(size==Collection(0).size() || size==Collection(1).size())continue;
        BOOST_TEST_CONTEXT("size="<<size) {
            BOOST_CHECK(!W3D::W3DRead_Model_Collection(std::span(bytes).first(size),result,error));
            BOOST_CHECK_EQUAL(result.name,"UNCHANGED");
        }
    }
    auto bad=bytes;bad[28]=std::byte(3);
    BOOST_CHECK(!W3D::W3DRead_Model_Collection(bad,result,error));
    bad=bytes;Chunk(bad,0x440,{});
    BOOST_CHECK(!W3D::W3DRead_Model_Collection(bad,result,error));
    bad=bytes;bad[Collection(0).size()+8+52]=std::byte(255);
    BOOST_CHECK(!W3D::W3DRead_Model_Collection(bad,result,error));
    BOOST_CHECK_EQUAL(result.name,"UNCHANGED");
}
BOOST_AUTO_TEST_CASE(collection_validation_rejects_nonfinite_transforms_and_empty_names)
{
    ModelCollectionDesc result;std::string error;
    BOOST_REQUIRE(W3D::W3DRead_Model_Collection(Collection(),result,error));
    result.proxies[0].transform[3]=std::numeric_limits<float>::infinity();
    BOOST_CHECK(!Validate_Model_Collection(result,error));
    result.proxies[0].transform[3]=0;result.children[0].clear();
    BOOST_CHECK(!Validate_Model_Collection(result,error));
    result.children[0]="Left";result.proxies[0].name.clear();
    BOOST_CHECK(!Validate_Model_Collection(result,error));
}
