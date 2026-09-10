module;
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>
export module Graphics.Scene.Models.GeometryMath;
import Graphics.Scene.Models.Hierarchy;

namespace Graphics {
namespace ModelGeometryMathDetail {
template<class Vector>
void Normalize(Vector& value) {
    const float squared = value[0] * value[0] + value[1] * value[1] + value[2] * value[2];
    if (squared != 0.0f) {
        const float inverse = 1.0f / static_cast<float>(std::sqrt(squared));
        value[0] *= inverse;
        value[1] *= inverse;
        value[2] *= inverse;
    }
}
}

export template<class Position, class Triangle, class Plane>
void Compute_Model_Planes(std::span<const Position> positions, std::span<const Triangle> triangles,
    std::span<Plane> planes)
{
    assert(planes.size() == triangles.size());
    for (std::size_t polygon = 0; polygon < triangles.size(); ++polygon) {
        const auto& triangle = triangles[polygon];
        assert(triangle[0] < positions.size() && triangle[1] < positions.size() && triangle[2] < positions.size());
        const auto& p0 = positions[triangle[0]];
        const auto& p1 = positions[triangle[1]];
        const auto& p2 = positions[triangle[2]];
        const std::array<float, 3> a{p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};
        const std::array<float, 3> b{p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]};
        std::array<float, 3> normal{a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
        ModelGeometryMathDetail::Normalize(normal);
        auto& plane = planes[polygon];
        plane[0] = normal[0]; plane[1] = normal[1]; plane[2] = normal[2];
        plane[3] = -(p0[0] * normal[0] + p0[1] * normal[1] + p0[2] * normal[2]);
    }
}

export template<class Position, class Triangle, class Plane, class ShadeIndex>
void Compute_Model_Normals(std::span<const Triangle> triangles, std::span<const Plane> planes,
    std::span<const ShadeIndex> shade_indices, std::span<Position> normals)
{
    assert(planes.size() == triangles.size());
    assert(shade_indices.empty() || shade_indices.size() == normals.size());
    for (auto& normal : normals) normal[0] = normal[1] = normal[2] = 0;
    const auto accumulate = [&](const auto& target_index) {
        for (std::size_t polygon = 0; polygon < triangles.size(); ++polygon)
            for (unsigned corner = 0; corner < 3; ++corner) {
                const auto source = triangles[polygon][corner];
                assert(source < normals.size());
                const auto target = target_index(source);
                assert(target < normals.size());
                for (unsigned axis = 0; axis < 3; ++axis) normals[target][axis] += planes[polygon][axis];
            }
    };
    if (shade_indices.empty()) accumulate([](auto index) { return index; });
    else {
        accumulate([&](auto index) { return shade_indices[index]; });
        // The authored smoothing contract visits masters before aliases and
        // normalizes masters here, then normalizes every result once more.
        for (std::size_t vertex = 0; vertex < normals.size(); ++vertex) {
            assert(shade_indices[vertex] < normals.size());
            if (shade_indices[vertex] == vertex) ModelGeometryMathDetail::Normalize(normals[vertex]);
            else normals[vertex] = normals[shade_indices[vertex]];
        }
    }
    for (auto& normal : normals) ModelGeometryMathDetail::Normalize(normal);
}

export template<class Position>
bool Compute_Model_Bounds(std::span<const Position> positions, Position& minimum, Position& maximum,
    Position& center, float& radius)
{
    minimum = maximum = center = Position{0, 0, 0};
    radius = 0;
    if (positions.empty()) return false;
    minimum = maximum = positions[0];
    for (std::size_t vertex = 1; vertex < positions.size(); ++vertex)
        for (unsigned axis = 0; axis < 3; ++axis) {
            minimum[axis] = minimum[axis] < positions[vertex][axis] ? minimum[axis] : positions[vertex][axis];
            maximum[axis] = maximum[axis] > positions[vertex][axis] ? maximum[axis] : positions[vertex][axis];
        }
    std::array<float, 3> extent;
    for (unsigned axis = 0; axis < 3; ++axis) {
        center[axis] = (minimum[axis] + maximum[axis]) / 2.0f;
        extent[axis] = maximum[axis] - center[axis];
    }
    radius = static_cast<float>(std::sqrt(extent[0] * extent[0] + extent[1] * extent[1] + extent[2] * extent[2])) * 1.00001f;
    return true;
}

export template<class Position>
bool Scale_Model_Geometry(std::span<Position> positions, Position& minimum, Position& maximum,
    Position& center, float& radius, const Position& scale)
{
    for (auto& position : positions)
        for (unsigned axis = 0; axis < 3; ++axis) position[axis] *= scale[axis];
    for (unsigned axis = 0; axis < 3; ++axis) {
        minimum[axis] *= scale[axis]; maximum[axis] *= scale[axis]; center[axis] *= scale[axis];
    }
    float largest = scale[0] > scale[1] ? scale[0] : scale[1];
    largest = largest > scale[2] ? largest : scale[2];
    radius *= largest;
    return scale[0] != scale[1] || scale[1] != scale[2];
}

export template<class Position>
void Deform_Model_Geometry(std::span<const Position> positions, std::span<const Position> normals,
    std::span<const std::uint16_t> bones, const ModelHierarchy& hierarchy,
    std::span<Position> output_positions, std::span<Position> output_normals = {})
{
    assert(bones.size() == positions.size() && output_positions.size() == positions.size());
    assert(output_normals.empty() || (normals.size() == positions.size() && output_normals.size() == positions.size()));
    for (std::size_t first = 0; first < positions.size();) {
        const auto bone = bones[first];
        assert(bone < hierarchy.Bone_Count());
        const auto& transform = hierarchy.World_Transform(bone).matrix;
        std::size_t end = first + 1;
        while (end < positions.size() && bones[end] == bone) ++end;
        for (std::size_t vertex = first; vertex < end; ++vertex) {
            const auto position = positions[vertex];
            for (unsigned axis = 0; axis < 3; ++axis) {
                const auto row = axis * 4;
                output_positions[vertex][axis] = transform[row] * position[0] + transform[row + 1] * position[1]
                    + transform[row + 2] * position[2] + transform[row + 3];
            }
        }
        if (!output_normals.empty()) {
            for (std::size_t vertex = first; vertex < end; ++vertex) {
                const auto normal = normals[vertex];
                for (unsigned axis = 0; axis < 3; ++axis) {
                    const auto row = axis * 4;
                    // The retained normal transform includes zero translation
                    // and does not normalize nonuniformly scaled results.
                    output_normals[vertex][axis] = transform[row] * normal[0] + transform[row + 1] * normal[1]
                        + transform[row + 2] * normal[2] + 0.0f;
                }
            }
        }
        first = end;
    }
}
}
