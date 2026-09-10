module;
#include <array>
#include <cstddef>
export module Graphics.Scene.Terrain.Visibility;
export import Graphics.Scene.Terrain.Geometry;
namespace Graphics
{
export bool Is_Terrain_Batch_Visible(const TerrainGeometryBatch &batch, const std::array<float, 16> &matrix) noexcept
{
    // Test the support point against the six clip planes. Depth is [0,w],
    // matching the projection contract used by the graphics views.
    for (std::size_t plane = 0; plane < 6; ++plane) {
        std::array<float, 4> coefficients{};
        const std::size_t row = plane / 2;
        const float sign = (plane % 2) == 0 ? 1.0f : -1.0f;
        for (std::size_t component = 0; component < 4; ++component)
            coefficients[component] = plane == 4 ? matrix[8 + component]
                : matrix[12 + component] + sign * matrix[row * 4 + component];
        float distance = coefficients[3];
        for (std::size_t axis = 0; axis < 3; ++axis)
            distance += coefficients[axis] * (coefficients[axis] >= 0 ? batch.maximum[axis] : batch.minimum[axis]);
        if (distance < 0) return false;
    }
    return true;
}
}
