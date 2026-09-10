module;
#define BOOST_TEST_MODULE W3DAggregateTests
#include <boost/test/included/unit_test.hpp>
#include "W3DAssembly.test-data.h"
#include <span>
#include <string>
export module Assets.Adapters.W3D.Aggregate.Tests;
import Assets.Adapters.W3D.Aggregate;
using namespace Assets;
using namespace AssemblyTestData;
BOOST_AUTO_TEST_CASE(aggregate_retains_named_attachments_and_optional_classification)
{
    for(bool metadata:{false,true}) {
        W3D::W3DAggregateDescription source;std::string error;
        auto bytes=Aggregate(metadata);
        BOOST_REQUIRE(W3D::W3DRead_Model_Aggregate(bytes,source,error));
        const auto instance=source;source={};bytes.clear();
        BOOST_CHECK_EQUAL(instance.model.name,"AGGREGATE");
        BOOST_CHECK_EQUAL(instance.model.base_model,"BASE");
        BOOST_REQUIRE_EQUAL(instance.model.attachments.size(),2);
        BOOST_CHECK_EQUAL(instance.model.attachments[0].model_name,"Left");
        BOOST_CHECK_EQUAL(instance.model.attachments[0].bone_name,"LEFT");
        BOOST_CHECK_EQUAL(instance.model.attachments[1].model_name,"Right");
        BOOST_CHECK_EQUAL(instance.model.attachments[1].bone_name,"RIGHT");
        BOOST_CHECK_EQUAL(instance.model.match_detail_levels,metadata);
        BOOST_CHECK_EQUAL(instance.original_class_id,metadata?23:25);
    }
}
BOOST_AUTO_TEST_CASE(aggregate_truncation_and_counts_cannot_partially_publish)
{
    const auto bytes=Aggregate();std::string error;
    W3D::W3DAggregateDescription result;result.model.name="UNCHANGED";result.original_class_id=99;
    for(std::size_t count=0;count<bytes.size();++count) {
        if(count==Aggregate(false).size())continue;
        BOOST_CHECK(!W3D::W3DRead_Model_Aggregate(std::span(bytes).first(count),result,error));
        BOOST_CHECK_EQUAL(result.model.name,"UNCHANGED");
        BOOST_CHECK_EQUAL(result.original_class_id,99);
    }
    auto bad=bytes;bad[28+8+32]=std::byte(255);
    BOOST_CHECK(!W3D::W3DRead_Model_Aggregate(bad,result,error));
    bad=bytes;Chunk(bad,0x604,Bytes(20));
    BOOST_CHECK(!W3D::W3DRead_Model_Aggregate(bad,result,error));
    BOOST_CHECK_EQUAL(result.model.name,"UNCHANGED");
}
