module;
#define BOOST_TEST_MODULE RibbonGeometryTests
#include <boost/test/included/unit_test.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

export module Graphics.Scene.Beams.RibbonGeometry.Tests;
import Graphics.Scene.Beams.RibbonGeometry;
import Graphics.Scene.Beams.RibbonIntersections;
import Graphics.Scene.Beams.RibbonSubdivision;
import Graphics.Scene.Beams.RibbonTextureCoordinates;
import Graphics.Scene.Props.Geometry;
using namespace Graphics;

namespace {

RibbonPoint Make_Point(float x, float y, float z, float v, std::array<float, 4> color)
{
    RibbonPoint point;
    point.position = {x, y, z};
    point.color = color;
    point.v = v;
    return point;
}

RibbonIntersection Make_Joint(std::size_t point_count, const RibbonPoint &point,
    std::array<float, 3> direction)
{
    RibbonIntersection joint;
    joint.point_count = point_count;
    joint.direction = direction;
    joint.point = point;
    return joint;
}

std::array<float, 3> Project(const RibbonPoint &point, std::array<float, 3> direction)
{
    const float distance = point.position[0] * direction[0]
        + point.position[1] * direction[1] + point.position[2] * direction[2];
    return {direction[0] * distance, direction[1] * distance, direction[2] * distance};
}

}

BOOST_AUTO_TEST_CASE(strip_topology_preserves_order_quantized_colors_and_uvs)
{
    const std::array<float, 3> top_direction{0.70710677f, 0.70710677f, 0};
    const std::array<float, 3> bottom_direction{0.70710677f, -0.70710677f, 0};
    std::array<RibbonPoint, 3> points{
        Make_Point(-0.75f, 0.25f, 0.5f, 0.25f, {0.1f, 0.5f, 0.9f, 0.3f}),
        Make_Point(0, 0.25f, 0.5f, 1.25f, {0.2f, 0.4f, 0.6f, 0.8f}),
        Make_Point(0.75f, 0.25f, 0.5f, 2.25f, {0.9f, 0.3f, 0.1f, 0.7f})};
    std::array<RibbonIntersection, 3> top{};
    std::array<RibbonIntersection, 3> bottom{};
    for (std::size_t i = 0; i < points.size(); ++i) {
        top[i] = Make_Joint(1, points[i], top_direction);
        bottom[i] = Make_Joint(1, points[i], bottom_direction);
    }

    RibbonGeometry geometry;
    BOOST_REQUIRE(geometry.Build(points, top, bottom, RibbonTextureMapping::Tiled, {0.25f, -0.5f}));
    BOOST_REQUIRE_EQUAL(geometry.Vertices().size(), 6u);
    const std::array<std::uint32_t, 12> expected_indices{
        0, 1, 2, 1, 3, 2, 2, 3, 4, 3, 5, 4};
    BOOST_CHECK(std::equal(geometry.Indices().begin(), geometry.Indices().end(), expected_indices.begin(),
        expected_indices.end()));

    for (std::size_t i = 0; i < points.size(); ++i) {
        const auto expected_top = Project(points[i], top_direction);
        const auto expected_bottom = Project(points[i], bottom_direction);
        BOOST_CHECK(geometry.Vertices()[i * 2].position == expected_top);
        BOOST_CHECK(geometry.Vertices()[i * 2 + 1].position == expected_bottom);
        BOOST_CHECK_EQUAL(geometry.Vertices()[i * 2].uv[0], 0.25f);
        BOOST_CHECK_EQUAL(geometry.Vertices()[i * 2 + 1].uv[0], 1.25f);
        BOOST_CHECK_EQUAL(geometry.Vertices()[i * 2].uv[1], points[i].v - 0.5f);
        BOOST_CHECK_EQUAL(geometry.Vertices()[i * 2 + 1].uv[1], points[i].v - 0.5f);

        const std::array<std::array<float, 4>, 3> expected_colors{{
            {26.0f / 255.0f, 128.0f / 255.0f, 230.0f / 255.0f, 77.0f / 255.0f},
            {51.0f / 255.0f, 102.0f / 255.0f, 153.0f / 255.0f, 204.0f / 255.0f},
            {230.0f / 255.0f, 77.0f / 255.0f, 26.0f / 255.0f, 179.0f / 255.0f}}};
        BOOST_CHECK(geometry.Vertices()[i * 2].color
            == expected_colors[i]);
        BOOST_CHECK(geometry.Vertices()[i * 2 + 1].color
            == expected_colors[i]);
    }

    const auto expected_vertices = std::vector<PropVertex>(geometry.Vertices().begin(), geometry.Vertices().end());
    const auto expected_topology = std::vector<std::uint32_t>(geometry.Indices().begin(), geometry.Indices().end());
    points = {};
    top = {};
    bottom = {};
    BOOST_CHECK_EQUAL(geometry.Vertices().size(), expected_vertices.size());
    BOOST_CHECK_EQUAL(geometry.Indices().size(), expected_topology.size());
    BOOST_CHECK(geometry.Vertices()[0].position == expected_vertices[0].position);
    BOOST_CHECK(geometry.Vertices()[5].color == expected_vertices[5].color);
    BOOST_CHECK(std::equal(geometry.Indices().begin(), geometry.Indices().end(), expected_topology.begin(),
        expected_topology.end()));
}

BOOST_AUTO_TEST_CASE(fan_topology_reuses_merged_side_and_advances_both_sides_at_corner)
{
    const std::array<float, 3> top_direction{0.70710677f, 0.70710677f, 0};
    const std::array<float, 3> bottom_direction{0.70710677f, -0.70710677f, 0};
    std::array<RibbonPoint, 4> points{
        Make_Point(-0.8f, 0.5f, 0, 0, {1, 0, 0, 1}),
        Make_Point(-0.3f, 0.5f, 0, 1, {1, 0, 0, 1}),
        Make_Point(0.3f, 0.5f, 0, 2, {1, 0, 0, 1}),
        Make_Point(0.8f, 0.5f, 0, 3, {0, 0, 1, 1})};
    const std::array<RibbonIntersection, 2> top{
        Make_Joint(3, points[0], top_direction), Make_Joint(1, points[3], top_direction)};
    const std::array<RibbonIntersection, 4> bottom{
        Make_Joint(1, points[0], bottom_direction), Make_Joint(1, points[1], bottom_direction),
        Make_Joint(1, points[2], bottom_direction), Make_Joint(1, points[3], bottom_direction)};

    RibbonGeometry geometry;
    BOOST_REQUIRE(geometry.Build(points, top, bottom, RibbonTextureMapping::Along, {}));
    BOOST_REQUIRE_EQUAL(geometry.Vertices().size(), 6u);
    BOOST_REQUIRE_EQUAL(geometry.Indices().size(), 12u);
    const std::array<std::uint32_t, 12> expected_indices{
        0, 1, 2, 0, 2, 3, 0, 3, 4, 3, 5, 4};
    BOOST_CHECK(std::equal(geometry.Indices().begin(), geometry.Indices().end(), expected_indices.begin(),
        expected_indices.end()));
    BOOST_CHECK_EQUAL(geometry.Vertices()[0].uv[0], 0.0f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[1].uv[0], 0.0f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[4].color[2], 1.0f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[5].color[2], 1.0f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[2].uv[1], 1.0f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[3].uv[1], 2.0f);
    BOOST_CHECK_EQUAL(geometry.Vertices()[4].uv[1], 3.0f);

    const std::array<RibbonIntersection, 4> top_unmerged{
        Make_Joint(1, points[0], top_direction), Make_Joint(1, points[1], top_direction),
        Make_Joint(1, points[2], top_direction), Make_Joint(1, points[3], top_direction)};
    const std::array<RibbonIntersection, 2> bottom_merged{
        Make_Joint(3, points[0], bottom_direction), Make_Joint(1, points[3], bottom_direction)};
    RibbonGeometry reciprocal;
    BOOST_REQUIRE(reciprocal.Build(points, top_unmerged, bottom_merged, RibbonTextureMapping::Along, {}));
    BOOST_REQUIRE_EQUAL(reciprocal.Vertices().size(), 6u);
    BOOST_REQUIRE_EQUAL(reciprocal.Indices().size(), 12u);
    const std::array<std::uint32_t, 12> expected_reciprocal_indices{
        0, 1, 2, 2, 1, 3, 3, 1, 4, 1, 5, 4};
    BOOST_CHECK(std::equal(reciprocal.Indices().begin(), reciprocal.Indices().end(),
        expected_reciprocal_indices.begin(), expected_reciprocal_indices.end()));
    BOOST_CHECK_EQUAL(reciprocal.Vertices()[4].uv[1], 3.0f);
}

BOOST_AUTO_TEST_CASE(ribbon_geometry_storage_grows_beyond_retired_chunk_capacity)
{
    constexpr std::size_t count = 4097;
    const std::array<float, 3> top_direction{0.70710677f, 0.70710677f, 0};
    const std::array<float, 3> bottom_direction{0.70710677f, -0.70710677f, 0};
    std::vector<RibbonPoint> points(count);
    std::vector<RibbonIntersection> top(count);
    std::vector<RibbonIntersection> bottom(count);
    for (std::size_t i = 0; i < count; ++i) {
        points[i] = Make_Point(-0.75f + static_cast<float>(i) * 0.01f, 0.5f, 0, static_cast<float>(i),
            {1, 1, 1, 1});
        top[i] = Make_Joint(1, points[i], top_direction);
        bottom[i] = Make_Joint(1, points[i], bottom_direction);
    }

    RibbonGeometry geometry;
    BOOST_REQUIRE(geometry.Build(points, top, bottom, RibbonTextureMapping::Tiled, {}));
    BOOST_CHECK_EQUAL(geometry.Vertices().size(), count * 2);
    BOOST_CHECK_EQUAL(geometry.Indices().size(), (count - 1) * 6);
    BOOST_CHECK_EQUAL(geometry.Indices().back(), static_cast<std::uint32_t>(count * 2 - 2));
}

BOOST_AUTO_TEST_CASE(invalid_ribbon_geometry_does_not_publish_partial_output)
{
    const std::array<RibbonPoint, 2> points{
        Make_Point(-0.5f, 0.25f, 0.5f, 0, {1, 1, 1, 1}),
        Make_Point(0.5f, 0.25f, 0.5f, 1, {1, 1, 1, 1})};
    const std::array<float, 3> direction{0.70710677f, 0.70710677f, 0};
    const std::array<RibbonIntersection, 2> valid{
        Make_Joint(1, points[0], direction), Make_Joint(1, points[1], direction)};
    RibbonGeometry geometry;
    BOOST_REQUIRE(geometry.Build(points, valid, valid, RibbonTextureMapping::Across, {}));
    BOOST_CHECK(!geometry.Vertices().empty());

    auto zero_count = valid;
    zero_count[0].point_count = 0;
    BOOST_CHECK(!geometry.Build(points, zero_count, valid, RibbonTextureMapping::Across, {}));
    BOOST_CHECK(geometry.Vertices().empty());
    BOOST_CHECK(geometry.Indices().empty());

    auto nonfinite = valid;
    nonfinite[1].direction[0] = std::numeric_limits<float>::quiet_NaN();
    BOOST_CHECK(!geometry.Build(points, nonfinite, valid, RibbonTextureMapping::Across, {}));
    BOOST_CHECK(geometry.Vertices().empty());
    BOOST_CHECK(geometry.Indices().empty());
}
