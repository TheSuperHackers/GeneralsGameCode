#pragma once

#include <array>
import Assets.Math;
import Geometry.QueryBounds;
#include "WWMath/aabox.h"
#include "WWMath/matrix3d.h"

namespace W3DQueryBounds
{
inline Assets::Vector3f Read(const Vector3 &value) noexcept { return {value.X, value.Y, value.Z}; }
inline Vector3 Write(Assets::Vector3f value) noexcept { return {value.x, value.y, value.z}; }
inline Assets::Bounds3f Read(const Vector3 &minimum, const Vector3 &maximum) noexcept
{
    return {Read(minimum), Read(maximum)};
}
inline Assets::Bounds3f Read(const AABoxClass &box) noexcept
{
    return Read(box.Center - box.Extent, box.Center + box.Extent);
}
inline void Write(const Assets::Bounds3f &bounds, Vector3 &minimum, Vector3 &maximum) noexcept
{
    minimum = Write(bounds.minimum);
    maximum = Write(bounds.maximum);
}
inline bool Disjoint(const AABoxClass &first, const AABoxClass &second) noexcept
{
    return Geometry::Boxes_Are_Disjoint(Read(first.Center), Read(first.Extent),
        Read(second.Center), Read(second.Extent));
}
template<unsigned Columns, class Matrix>
std::array<float, 3 * Columns> Read_Matrix(const Matrix &matrix) noexcept
{
    std::array<float, 3 * Columns> result;
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < Columns; ++column)
            result[row * Columns + column] = matrix[row][column];
    return result;
}
}
