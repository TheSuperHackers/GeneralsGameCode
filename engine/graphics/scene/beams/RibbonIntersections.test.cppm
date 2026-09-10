module;
#define BOOST_TEST_MODULE RibbonIntersectionsTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cmath>
#include <vector>
export module Graphics.Scene.Beams.RibbonIntersections.Tests;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonEdges;
import Graphics.Scene.Beams.RibbonIntersections;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(straight_endpoints_and_parallel_midpoints_preserve_attributes)
{
    std::array<RibbonPoint,3> points;
    for(unsigned i=0;i<3;++i) {
        points[i].position={float(i)-1,0,-5};
        points[i].v=float(i)*2;
        points[i].color={float(i)*.25f,0,0,.5f};
    }
    RibbonEdges edges;
    RibbonIntersections result;
    BOOST_REQUIRE(edges.Build(points,2));
    BOOST_REQUIRE(result.Build(edges,false,2,1.5f));
    edges={}; points={};
    for(auto side:{result.Top(),result.Bottom()}) {
        BOOST_REQUIRE_EQUAL(side.size(),3);
        for(unsigned i=0;i<3;++i) {
            BOOST_CHECK_EQUAL(side[i].point_count,1);
            BOOST_CHECK_EQUAL(side[i].point.v,float(i)*2);
            BOOST_CHECK_EQUAL(side[i].point.color[0],float(i)*.25f);
            BOOST_CHECK_EQUAL(side[i].point.color[3],.5f);
            BOOST_CHECK_EQUAL(side[i].parallel,i==1);
            BOOST_CHECK_EQUAL(side[i].fold,i!=1);
            const auto& d=side[i].direction;
            BOOST_CHECK_CLOSE(d[0]*d[0]+d[1]*d[1]+d[2]*d[2],1.0f,.001f);
            BOOST_CHECK(d[2]<0);
        }
    }
    BOOST_CHECK_CLOSE(result.Top()[1].direction[1],1/std::sqrt(26.0f),.001f);
    BOOST_CHECK_CLOSE(result.Bottom()[1].direction[1],-1/std::sqrt(26.0f),.001f);
}

BOOST_AUTO_TEST_CASE(merging_preserves_point_coverage_and_reused_storage_is_deterministic)
{
    RibbonEdges edges;
    RibbonIntersections result;
    bool observed_merge=false;
    for(unsigned sample=0;sample<20;++sample) {
        std::vector<RibbonPoint> points(24);
        for(unsigned i=0;i<points.size();++i) {
            const float angle=float(i)*(.02f+float(sample)*.025f);
            points[i].position={std::cos(angle),std::sin(angle),-5.0f};
            points[i].v=float(i);
            points[i].color={.2f,.4f,.6f,.5f};
        }
        BOOST_REQUIRE(edges.Build(points,2));
        BOOST_REQUIRE(result.Build(edges,true,2,0));
        const std::vector<RibbonIntersection> expected_top(result.Top().begin(),result.Top().end());
        const std::vector<RibbonIntersection> expected_bottom(result.Bottom().begin(),result.Bottom().end());
        observed_merge |= result.Top().size()<points.size() || result.Bottom().size()<points.size();
        for(auto side:{result.Top(),result.Bottom()}) {
            std::size_t coverage=0;
            for(const auto& joint:side) {
                coverage+=joint.point_count;
                for(float axis:joint.direction) BOOST_CHECK(std::isfinite(axis));
            }
            BOOST_CHECK_EQUAL(coverage,points.size());
        }
        // Perturb all workspace fields before rebuilding the identical input.
        BOOST_REQUIRE(result.Build(edges,false,2,1.5f));
        BOOST_REQUIRE(result.Build(edges,true,2,0));
        for(unsigned side=0;side<2;++side) {
            const auto actual=side ? result.Bottom() : result.Top();
            const auto& expected=side ? expected_bottom : expected_top;
            BOOST_REQUIRE_EQUAL(actual.size(),expected.size());
            for(unsigned i=0;i<actual.size();++i) {
                BOOST_CHECK(actual[i].direction==expected[i].direction);
                BOOST_CHECK(actual[i].point.color==expected[i].point.color);
                BOOST_CHECK_EQUAL(actual[i].point_count,expected[i].point_count);
                BOOST_CHECK_EQUAL(actual[i].parallel,expected[i].parallel);
                BOOST_CHECK_EQUAL(actual[i].fold,expected[i].fold);
            }
        }
    }
    BOOST_CHECK(observed_merge);
}

BOOST_AUTO_TEST_CASE(empty_and_large_inputs_are_not_limited_by_retired_chunk_storage)
{
    std::vector<RibbonPoint> points(4097);
    for(unsigned i=0;i<points.size();++i) points[i].position={float(i)*.01f,0,-5};
    RibbonEdges edges;
    RibbonIntersections result;
    BOOST_REQUIRE(edges.Build(points,1));
    BOOST_REQUIRE(result.Build(edges,false,1,1.5f));
    BOOST_CHECK_EQUAL(result.Top().size(),4097);
    BOOST_CHECK_EQUAL(result.Bottom().size(),4097);
    BOOST_REQUIRE(edges.Build({},1));
    BOOST_REQUIRE(result.Build(edges,true,1,1.5f));
    BOOST_CHECK(result.Top().empty());
    BOOST_CHECK(result.Bottom().empty());
}
