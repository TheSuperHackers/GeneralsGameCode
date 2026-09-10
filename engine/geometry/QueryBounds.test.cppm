module;

#define BOOST_TEST_MODULE QueryBoundsTests
#include <boost/test/included/unit_test.hpp>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>

export module Geometry.QueryBounds.Tests;
import Assets.Math;
import Geometry.QueryBounds;

BOOST_AUTO_TEST_CASE(swept_bounds_cover_both_endpoints_and_touching_boxes_are_not_rejected)
{
    const auto bounds = Geometry::Swept_Box_Bounds({2, 3, 4}, {1, 2, 3}, {-7, 8, 0});
    BOOST_CHECK_EQUAL(bounds.minimum.x, -6);
    BOOST_CHECK_EQUAL(bounds.maximum.x, 3);
    BOOST_CHECK_EQUAL(bounds.minimum.y, 1);
    BOOST_CHECK_EQUAL(bounds.maximum.y, 13);
    BOOST_CHECK_EQUAL(bounds.minimum.z, 1);
    BOOST_CHECK_EQUAL(bounds.maximum.z, 7);
    BOOST_CHECK(!Geometry::Bounds_Are_Disjoint(bounds, {{3, 1, 1}, {4, 2, 2}}));
    BOOST_CHECK(Geometry::Bounds_Are_Disjoint(bounds, {{3.01f, 1, 1}, {4, 2, 2}}));
    BOOST_CHECK(!Geometry::Boxes_Are_Disjoint({0, 0, 0}, {1, 2, 3}, {2, 0, 0}, {1, 2, 3}));
}

BOOST_AUTO_TEST_CASE(oriented_extents_preserve_conservative_padding_and_signed_basis_products)
{
    const std::array<float, 9> basis{0, -1, 0, 1, 0, 0, 0, 0, -1};
    const auto extent = Geometry::Oriented_Box_Extent(basis, {1, 2, 3}, .01f);
    BOOST_CHECK_EQUAL(extent.x, 2.01f);
    BOOST_CHECK_EQUAL(extent.y, 1.01f);
    BOOST_CHECK_EQUAL(extent.z, 3.01f);
}

BOOST_AUTO_TEST_CASE(quarter_turns_match_transformed_corners_and_four_turns_restore_bounds)
{
    const Assets::Bounds3f bounds{{-2, 3, -4}, {5, 7, 8}};
    const std::array<std::array<float, 12>, 4> matrices{{
        {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0},
        {0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0},
        {-1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0},
        {0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1, 0}}};
    for (unsigned turn = 0; turn < matrices.size(); ++turn) {
        const auto rotated = Geometry::Rotate_Bounds_Z(bounds, turn);
        const auto transformed = Geometry::Transform_Bounds_Corners(bounds, matrices[turn]);
        BOOST_CHECK_EQUAL(rotated.minimum.x, transformed.minimum.x);
        BOOST_CHECK_EQUAL(rotated.minimum.y, transformed.minimum.y);
        BOOST_CHECK_EQUAL(rotated.maximum.x, transformed.maximum.x);
        BOOST_CHECK_EQUAL(rotated.maximum.y, transformed.maximum.y);
        BOOST_CHECK_EQUAL(rotated.minimum.z, -4);
        BOOST_CHECK_EQUAL(rotated.maximum.z, 8);
    }
    auto rotated = bounds;
    for (unsigned turn = 0; turn < 4; ++turn)
        rotated = Geometry::Rotate_Bounds_Z(rotated, 1);
    BOOST_CHECK_EQUAL(rotated.minimum.x, bounds.minimum.x);
    BOOST_CHECK_EQUAL(rotated.maximum.y, bounds.maximum.y);
}

BOOST_AUTO_TEST_CASE(transformed_bounds_keep_the_authored_corner_order_and_literal_extents)
{
    // x' = -2y + 11, y' = 3x - 4, z' = -z + 6.
    // These values are the independently calculated extrema of the eight
    // transformed corners, rather than another call into the implementation.
    const Assets::Bounds3f bounds{{-2, 3, -4}, {5, 7, 8}};
    const std::array<float, 12> transform{
        0, -2, 0, 11,
        3, 0, 0, -4,
        0, 0, -1, 6};
    const auto result = Geometry::Transform_Bounds_Corners(bounds, transform);
    BOOST_CHECK_EQUAL(result.minimum.x, -3);
    BOOST_CHECK_EQUAL(result.maximum.x, 5);
    BOOST_CHECK_EQUAL(result.minimum.y, -10);
    BOOST_CHECK_EQUAL(result.maximum.y, 11);
    BOOST_CHECK_EQUAL(result.minimum.z, -2);
    BOOST_CHECK_EQUAL(result.maximum.z, 10);
}

BOOST_AUTO_TEST_CASE(equal_sweep_endpoints_retain_the_legacy_strict_comparison_choice)
{
    const auto bounds = Geometry::Swept_Box_Bounds({0, 0, 0}, {0, 0, 0}, {-0.0f, -0.0f, -0.0f});
    BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(bounds.minimum.x), std::uint32_t{0});
    BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(bounds.minimum.y), std::uint32_t{0});
    BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(bounds.minimum.z), std::uint32_t{0});
    BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(bounds.maximum.x), std::uint32_t{0});
    BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(bounds.maximum.y), std::uint32_t{0});
    BOOST_CHECK_EQUAL(std::bit_cast<std::uint32_t>(bounds.maximum.z), std::uint32_t{0});
}
