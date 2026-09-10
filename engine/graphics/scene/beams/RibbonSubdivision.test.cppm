module;
#define BOOST_TEST_MODULE RibbonSubdivisionTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <limits>
export module Graphics.Scene.Beams.RibbonSubdivision.Tests;
import Graphics.Scene.Beams.RibbonSubdivision;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(depth_first_noise_and_uvs_preserve_segment_start_colors)
{
    std::array<RibbonPoint,2> source;
    source[0].color={1,0,0,.25f};source[1].position={1,0,0};source[1].v=1;
    source[1].color={0,1,0,1};
    unsigned calls=0;
    RibbonSubdivision result;
    BOOST_REQUIRE(result.Build(2,2,1,[&](auto i){return source[i];},[&]{return std::array{0.0f,float(++calls),0.0f};}));
    source={};
    BOOST_CHECK_EQUAL(calls,3);
    BOOST_REQUIRE_EQUAL(result.Points().size(),5);
    const std::array<float,5> heights{0,1.5f,1,2,0};
    for(unsigned i=0;i<5;++i) {
        BOOST_CHECK_EQUAL(result.Points()[i].position[0],i*.25f);
        BOOST_CHECK_EQUAL(result.Points()[i].position[1],heights[i]);
        BOOST_CHECK_EQUAL(result.Points()[i].v,i*.25f);
        BOOST_CHECK_EQUAL(result.Points()[i].color[0],i<4?1:0);
    }
    BOOST_CHECK_EQUAL(result.Points().back().color[3],1);
}

BOOST_AUTO_TEST_CASE(noise_is_consumed_at_zero_amplitude_and_original_endpoints_are_not_duplicated)
{
    RibbonSubdivision result;
    unsigned calls=0;
    BOOST_REQUIRE(result.Build(3,1,0,[](auto i){RibbonPoint p;p.position[0]=float(i);p.v=float(i);return p;},
        [&]{++calls;return std::array<float,3>{1,1,1};}));
    BOOST_CHECK_EQUAL(calls,2);
    BOOST_REQUIRE_EQUAL(result.Points().size(),5);
    for(unsigned i=0;i<5;++i) BOOST_CHECK_EQUAL(result.Points()[i].position[0],i*.5f);
    BOOST_REQUIRE(result.Build(2,0,1,[](auto i){RibbonPoint p;p.v=float(i);return p;},
        [&]{BOOST_FAIL("zero subdivisions must not consume randomness");return std::array<float,3>{};}));
    BOOST_CHECK_EQUAL(result.Points().size(),2);
}

BOOST_AUTO_TEST_CASE(storage_grows_and_invalid_input_does_not_publish_partial_output)
{
    RibbonSubdivision result;
    BOOST_REQUIRE(result.Build(2,14,0,[](auto i){RibbonPoint p;p.position[0]=float(i);return p;},[]{return std::array<float,3>{};}));
    BOOST_CHECK_EQUAL(result.Points().size(),16385);
    BOOST_CHECK(!result.Build(2,std::numeric_limits<std::size_t>::digits,0,[](auto){
        BOOST_FAIL("invalid level must reject before source access");return RibbonPoint{};
    },[]{return std::array<float,3>{};}));
    BOOST_CHECK(result.Points().empty());
    BOOST_CHECK(!result.Build(2,1,1,[](auto){return RibbonPoint{};},[]{
        return std::array<float,3>{std::numeric_limits<float>::infinity(),0,0};
    }));
    BOOST_CHECK(result.Points().empty());
}
