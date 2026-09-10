module;
#define BOOST_TEST_MODULE SpriteGeometryTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <cstdint>
#include <limits>
#include <vector>
export module Graphics.Scene.Particles.SpriteGeometry.Tests;
import Graphics.Scene.Particles.SpriteGeometry;
using namespace Graphics;
constexpr std::array<float, 16> identity{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

BOOST_AUTO_TEST_CASE(sprite_shapes_preserve_view_transform_rotation_uvs_and_quantized_color)
{
    SpriteGeometry geometry;
    SpritePoint point;
    point.position = {-2,0,.5f}; point.size = 2; point.angle = 1.5707963267948966f;
    point.color = {.5f,.25f,1,.5f}; point.texture_region = {.5f,0,1,.5f};
    auto view = identity; view[3] = 2; view[7] = 1;
    BOOST_REQUIRE(geometry.Build(1,SpriteShape::Quad,view,[&](auto) { return point; }));
    BOOST_REQUIRE_EQUAL(geometry.Vertices().size(),4);
    BOOST_CHECK_SMALL(geometry.Vertices()[0].position[0]+1,1e-6f);
    BOOST_CHECK_SMALL(geometry.Vertices()[0].position[1],1e-6f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].position[2],.5f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].uv[0],.5f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[1].uv[1],.5f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].color[0],128/255.0f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].color[1],64/255.0f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].color[3],128/255.0f);
    const std::array<unsigned,6> indices{0,1,2,2,3,0};
    BOOST_CHECK_EQUAL_COLLECTIONS(geometry.Indices().begin(),geometry.Indices().end(),indices.begin(),indices.end());
    point = {};
    BOOST_REQUIRE(geometry.Build(1,SpriteShape::Triangle,identity,[&](auto) { return point; }));
    BOOST_REQUIRE_EQUAL(geometry.Vertices().size(),3);
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].position[1],-2);
    BOOST_CHECK_EQUAL(geometry.Vertices()[1].position[0],-1.732f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[2].uv[1],.866f);
    // Both sprite shapes face the camera with the normal material culling mode.
    const auto &first = geometry.Vertices()[geometry.Indices()[0]].position;
    const auto &second = geometry.Vertices()[geometry.Indices()[1]].position;
    const auto &third = geometry.Vertices()[geometry.Indices()[2]].position;
    BOOST_CHECK_GT((second[0] - first[0]) * (third[1] - first[1])
        - (second[1] - first[1]) * (third[0] - first[0]), 0);
}

BOOST_AUTO_TEST_CASE(atlas_frames_wrap_without_static_tables)
{
    std::array<float,4> region{};
    BOOST_REQUIRE(Get_Sprite_Atlas_Region(5,2,2,region));
    const std::array<float,4> expected{.5f,0,1,.5f};
    BOOST_CHECK(region==expected);
    BOOST_CHECK(!Get_Sprite_Atlas_Region(0,0,2,region));
    BOOST_CHECK(region==expected);
    BOOST_REQUIRE(Get_Sprite_Atlas_Region(255,16,16,region));
    BOOST_CHECK_EQUAL(region[0],15/16.0f);
    BOOST_CHECK_EQUAL(region[1],15/16.0f);
    BOOST_CHECK_EQUAL(region[3],1);
}

BOOST_AUTO_TEST_CASE(geometry_owns_ordered_results_grows_and_rejects_partial_invalid_output)
{
    SpriteGeometry geometry;
    std::vector<SpritePoint> points(2);
    points[0].position[0]=10; points[1].position[0]=20;
    const std::array<unsigned,2> order{1,0};
    BOOST_REQUIRE(geometry.Build(2,SpriteShape::Quad,identity,[&](auto i) { return points[order[i]]; }));
    points.clear();
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].position[0],19.5f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[4].position[0],9.5f);
    BOOST_REQUIRE(geometry.Build(8193,SpriteShape::Quad,identity,[](auto i) {
        SpritePoint point; point.position[0]=static_cast<float>(i); return point;
    }));
    BOOST_CHECK_EQUAL(geometry.Vertices().size(),8193*4);
    BOOST_CHECK_EQUAL(geometry.Indices().back(),8192*4);
    BOOST_CHECK(!geometry.Build((std::numeric_limits<std::uint32_t>::max)(),SpriteShape::Quad,identity,[](auto) {
        BOOST_FAIL("Overflow must be rejected before reading points"); return SpritePoint{};
    }));
    BOOST_CHECK(geometry.Vertices().empty());
    BOOST_CHECK(!geometry.Build(2,SpriteShape::Quad,identity,[](auto i) {
        SpritePoint point; if(i)point.size=std::numeric_limits<float>::infinity();return point;
    }));
    BOOST_CHECK(geometry.Indices().empty());
    BOOST_REQUIRE(geometry.Build(0,SpriteShape::Triangle,identity,[](auto) { return SpritePoint{}; }));
    BOOST_CHECK(geometry.Vertices().empty());
}
