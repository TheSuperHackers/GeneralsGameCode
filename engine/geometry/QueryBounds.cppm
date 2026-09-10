module;

#include <algorithm>
#include <array>
#include <cmath>

export module Geometry.QueryBounds;

import Assets.Math;

namespace Geometry
{

export constexpr Assets::Bounds3f Swept_Box_Bounds(Assets::Vector3f center,
    Assets::Vector3f extent, Assets::Vector3f movement) noexcept
{
    // Keep the initial box as the first candidate. The legacy implementation
    // updated its bounds only for strict comparisons, which determines which
    // signed zero survives equal endpoint values.
    Assets::Bounds3f result{
        {center.x - extent.x, center.y - extent.y, center.z - extent.z},
        {center.x + extent.x, center.y + extent.y, center.z + extent.z}};
    const Assets::Vector3f end_min{
        center.x + movement.x - extent.x,
        center.y + movement.y - extent.y,
        center.z + movement.z - extent.z};
    const Assets::Vector3f end_max{
        center.x + movement.x + extent.x,
        center.y + movement.y + extent.y,
        center.z + movement.z + extent.z};
    if (end_max.x > result.maximum.x) result.maximum.x = end_max.x;
    if (end_max.y > result.maximum.y) result.maximum.y = end_max.y;
    if (end_max.z > result.maximum.z) result.maximum.z = end_max.z;
    if (end_min.x < result.minimum.x) result.minimum.x = end_min.x;
    if (end_min.y < result.minimum.y) result.minimum.y = end_min.y;
    if (end_min.z < result.minimum.z) result.minimum.z = end_min.z;
    return result;
}

export inline Assets::Vector3f Oriented_Box_Extent(const std::array<float, 9> &basis,
    Assets::Vector3f extent, float padding = 0) noexcept
{
    return {std::fabs(basis[0] * extent.x) + std::fabs(basis[1] * extent.y) + std::fabs(basis[2] * extent.z) + padding,
        std::fabs(basis[3] * extent.x) + std::fabs(basis[4] * extent.y) + std::fabs(basis[5] * extent.z) + padding,
        std::fabs(basis[6] * extent.x) + std::fabs(basis[7] * extent.y) + std::fabs(basis[8] * extent.z) + padding};
}

export constexpr bool Bounds_Are_Disjoint(const Assets::Bounds3f &first,
    const Assets::Bounds3f &second) noexcept
{
    return first.minimum.x > second.maximum.x || first.maximum.x < second.minimum.x
        || first.minimum.y > second.maximum.y || first.maximum.y < second.minimum.y
        || first.minimum.z > second.maximum.z || first.maximum.z < second.minimum.z;
}

export inline bool Boxes_Are_Disjoint(Assets::Vector3f first_center, Assets::Vector3f first_extent,
    Assets::Vector3f second_center, Assets::Vector3f second_extent) noexcept
{
    return std::fabs(second_center.x - first_center.x) > second_extent.x + first_extent.x
        || std::fabs(second_center.y - first_center.y) > second_extent.y + first_extent.y
        || std::fabs(second_center.z - first_center.z) > second_extent.z + first_extent.z;
}

// Right-angle rotations keep axis alignment and do not introduce trig error.
export constexpr Assets::Vector3f Rotate_Z(Assets::Vector3f vector, unsigned quarter_turns) noexcept
{
    switch (quarter_turns & 3u) {
    case 1: return {-vector.y, vector.x, vector.z};
    case 2: return {-vector.x, -vector.y, vector.z};
    case 3: return {vector.y, -vector.x, vector.z};
    default: return vector;
    }
}

export constexpr Assets::Bounds3f Rotate_Bounds_Z(const Assets::Bounds3f &bounds,
    unsigned quarter_turns) noexcept
{
    const auto &minimum = bounds.minimum;
    const auto &maximum = bounds.maximum;
    switch (quarter_turns & 3u) {
    case 1: return {{-maximum.y, minimum.x, minimum.z}, {-minimum.y, maximum.x, maximum.z}};
    case 2: return {{-maximum.x, -maximum.y, minimum.z}, {-minimum.x, -minimum.y, maximum.z}};
    case 3: return {{minimum.y, -maximum.x, minimum.z}, {maximum.y, -minimum.x, maximum.z}};
    default: return bounds;
    }
}

export inline Assets::Bounds3f Transform_Bounds_Corners(const Assets::Bounds3f &bounds,
    const std::array<float, 12> &transform) noexcept
{
    const auto &low = bounds.minimum;
    const auto &high = bounds.maximum;
    // Retain corner order for the full-corner transform path.
    const std::array<Assets::Vector3f, 8> corners{{
        {low.x, low.y, low.z}, {low.x, high.y, low.z},
        {high.x, high.y, low.z}, {high.x, low.y, low.z},
        {low.x, low.y, high.z}, {low.x, high.y, high.z},
        {high.x, high.y, high.z}, {high.x, low.y, high.z}}};
    Assets::Bounds3f result;
    for (unsigned index = 0; index < corners.size(); ++index) {
        const auto &point = corners[index];
        const Assets::Vector3f transformed{
            transform[0] * point.x + transform[1] * point.y + transform[2] * point.z + transform[3],
            transform[4] * point.x + transform[5] * point.y + transform[6] * point.z + transform[7],
            transform[8] * point.x + transform[9] * point.y + transform[10] * point.z + transform[11]};
        if (index == 0) {
            result = {transformed, transformed};
            continue;
        }
        if (result.minimum.x >= transformed.x) result.minimum.x = transformed.x;
        if (result.minimum.y >= transformed.y) result.minimum.y = transformed.y;
        if (result.minimum.z >= transformed.z) result.minimum.z = transformed.z;
        if (result.maximum.x <= transformed.x) result.maximum.x = transformed.x;
        if (result.maximum.y <= transformed.y) result.maximum.y = transformed.y;
        if (result.maximum.z <= transformed.z) result.maximum.z = transformed.z;
    }
    return result;
}

}
