module;
#define BOOST_TEST_MODULE RibbonEdgesTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cmath>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Beams.RibbonEdges.Tests;
import Graphics.Scene.Beams.RibbonEdges;
import Graphics.Scene.Beams.RibbonSubdivision;
using namespace Graphics;

BOOST_AUTO_TEST_CASE(straight_ribbon_planes_have_authored_width_and_inward_orientation)
{
    std::array<RibbonPoint,2> source;
    source[0].position={-1,0,-5};
    source[1].position={1,0,-5};
    RibbonEdges result;
    BOOST_REQUIRE(result.Build(source,2));
    source={};
    BOOST_REQUIRE_EQUAL(result.Edges().size(),1);
    const auto& edge=result.Edges()[0];
    BOOST_CHECK(edge.direction == (std::array<float,3>{1,0,0}));
    const float length=std::sqrt(26.0f);
    BOOST_CHECK_CLOSE(edge.top[1],-5/length,0.0001f);
    BOOST_CHECK_CLOSE(edge.top[2],-1/length,0.0001f);
    BOOST_CHECK_CLOSE(edge.bottom[1],5/length,0.0001f);
    BOOST_CHECK_CLOSE(edge.bottom[2],-1/length,0.0001f);
    BOOST_CHECK(!edge.fold);
    BOOST_CHECK_EQUAL(result.Points()[0].position[0],-1);
}

BOOST_AUTO_TEST_CASE(turns_and_folds_keep_continuous_edge_identity)
{
    std::array<RibbonPoint,4> source;
    source[0].position={-1,0,-5}; source[1].position={1,0,-5};
    source[2].position={0,0,-5}; source[3].position={2,0,-5};
    RibbonEdges result;
    BOOST_REQUIRE(result.Build(source,2));
    BOOST_CHECK(!result.Edges()[0].fold);
    BOOST_CHECK(result.Edges()[1].fold);
    BOOST_CHECK(result.Edges()[2].fold);
    for (unsigned i=1;i<3;++i) {
        // A reversed segment swaps the geometric edges. The fold operation
        // additionally negates their normals; a second fold restores them.
        const float sign = i==1 ? -1.0f : 1.0f;
        for (unsigned axis=0;axis<3;++axis) {
            BOOST_CHECK_EQUAL(result.Edges()[i].top[axis], sign*result.Edges()[0].top[axis]);
            BOOST_CHECK_EQUAL(result.Edges()[i].bottom[axis], sign*result.Edges()[0].bottom[axis]);
        }
    }
    source[2].position={1,1,-5};
    BOOST_REQUIRE(result.Build(std::span(source).first(3),2));
    BOOST_CHECK(!result.Edges()[1].fold);
    BOOST_CHECK(result.Edges()[1].direction == (std::array<float,3>{0,1,0}));
}

BOOST_AUTO_TEST_CASE(duplicate_points_are_adjusted_in_owned_storage_and_large_inputs_are_not_truncated)
{
    std::vector<RibbonPoint> source(4097);
    for (unsigned i=0;i<source.size();++i) source[i].position={float(i),0,-5};
    source[1]=source[0];
    RibbonEdges result;
    BOOST_REQUIRE(result.Build(source,1));
    BOOST_CHECK_EQUAL(result.Edges().size(),4096);
    BOOST_CHECK_EQUAL(result.Points()[1].position[0],0.001f);
    BOOST_CHECK_EQUAL(source[1].position[0],0);
    source[2].position[0]=std::numeric_limits<float>::infinity();
    BOOST_CHECK(!result.Build(source,1));
    BOOST_CHECK(result.Edges().empty());
    BOOST_CHECK(result.Points().empty());
    BOOST_REQUIRE(result.Build({},1));
    BOOST_CHECK(result.Edges().empty());
}
