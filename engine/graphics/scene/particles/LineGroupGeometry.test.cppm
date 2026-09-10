module;
#define BOOST_TEST_MODULE LineGroupGeometryTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstdint>
#include <limits>
export module Graphics.Scene.Particles.LineGroupGeometry.Tests;
import Graphics.Scene.Particles.LineGroupGeometry;
using namespace Graphics;
constexpr std::array<float,9> identity{1,0,0,0,1,0,0,0,1};

BOOST_AUTO_TEST_CASE(tetrahedron_preserves_topology_head_tail_colors_and_uvs)
{
    LineGroupPoint point;
    point.head={1,2,3};point.tail={4,5,6};point.size=2;point.u=.25f;
    point.head_color={.5f,.25f,1,1};point.tail_color={0,1,0,.5f};
    LineGroupGeometry geometry;
    BOOST_REQUIRE(geometry.Build(1,LineGroupShape::Tetrahedron,identity,[&](auto){return point;}));
    point={};
    BOOST_REQUIRE_EQUAL(geometry.Vertices().size(),4);
    const auto vertices=geometry.Vertices();
    BOOST_CHECK_EQUAL(vertices[0].position[0],4);
    BOOST_CHECK_EQUAL(vertices[0].position[2],6);
    BOOST_CHECK_SMALL(vertices[1].position[0]-1,1e-6f);
    BOOST_CHECK_SMALL(vertices[1].position[1]-4,1e-6f);
    BOOST_CHECK_CLOSE(vertices[2].position[0],1-1.7320508f,.001f);
    BOOST_CHECK_EQUAL(vertices[0].uv[1],1);
    BOOST_CHECK_EQUAL(vertices[1].uv[1],0);
    BOOST_CHECK_EQUAL(vertices[2].uv[0],.25f);
    BOOST_CHECK_EQUAL(vertices[1].color[0],128/255.0f);
    BOOST_CHECK_EQUAL(vertices[0].color[3],128/255.0f);
    const std::array<unsigned,12> expected{0,2,1,0,3,2,0,1,3,1,2,3};
    BOOST_CHECK_EQUAL_COLLECTIONS(geometry.Indices().begin(),geometry.Indices().end(),expected.begin(),expected.end());
}

BOOST_AUTO_TEST_CASE(prism_preserves_both_caps_rotated_cross_section_and_signed_size)
{
    LineGroupPoint point;point.tail={0,0,4};point.size=-2;
    const std::array<float,9> rotation{0,-1,0,1,0,0,0,0,1};
    LineGroupGeometry geometry;
    BOOST_REQUIRE(geometry.Build(1,LineGroupShape::Prism,rotation,[&](auto){return point;}));
    BOOST_REQUIRE_EQUAL(geometry.Vertices().size(),6);
    BOOST_CHECK_SMALL(geometry.Vertices()[0].position[0]-2,1e-6f);
    for(unsigned i=0;i<3;++i) {
        BOOST_CHECK_EQUAL(geometry.Vertices()[i].position[0],geometry.Vertices()[i+3].position[0]);
        BOOST_CHECK_EQUAL(geometry.Vertices()[i+3].position[2],4);
        BOOST_CHECK_EQUAL(geometry.Vertices()[i+3].uv[1],1);
    }
    const std::array<unsigned,24> expected{0,1,2,0,3,1,1,3,4,1,4,5,1,5,2,0,2,5,0,5,3,3,5,4};
    BOOST_CHECK_EQUAL_COLLECTIONS(geometry.Indices().begin(),geometry.Indices().end(),expected.begin(),expected.end());
}

BOOST_AUTO_TEST_CASE(dynamic_geometry_preserves_order_and_degenerate_lines_and_rejects_partial_output)
{
    LineGroupGeometry geometry;
    BOOST_REQUIRE(geometry.Build(8193,LineGroupShape::Prism,identity,[](auto i){
        LineGroupPoint point;point.head[0]=point.tail[0]=static_cast<float>(8192-i);return point;
    }));
    BOOST_CHECK_EQUAL(geometry.Vertices().size(),8193*6);
    BOOST_CHECK_EQUAL(geometry.Vertices().front().position[0],8192);
    BOOST_CHECK_EQUAL(geometry.Vertices().back().position[0],0);
    BOOST_CHECK(!geometry.Build((std::numeric_limits<std::uint32_t>::max)(),LineGroupShape::Prism,identity,[](auto){
        BOOST_FAIL("overflow must reject before source access");return LineGroupPoint{};
    }));
    BOOST_CHECK(geometry.Vertices().empty());
    BOOST_CHECK(!geometry.Build(2,LineGroupShape::Prism,identity,[](auto i){
        LineGroupPoint point;if(i)point.size=std::numeric_limits<float>::infinity();return point;
    }));
    BOOST_CHECK(geometry.Indices().empty());
}
