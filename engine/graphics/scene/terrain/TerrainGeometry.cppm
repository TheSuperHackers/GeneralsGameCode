module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <future>
#include <thread>
#include <span>
#include <vector>

export module Graphics.Scene.Terrain.Geometry;

namespace Graphics
{

// Corners run counterclockwise from the minimum X/Y corner. A cell keeps
// separate vertices because texture coordinates and blend coverage may be
// discontinuous at its edges.
export struct TerrainCell final
{
    std::array<float, 2> origin{};
    std::array<float, 2> spacing{1.0f, 1.0f};
    std::array<float, 4> heights{};
    std::array<std::array<float, 4>, 4> colors{};
    std::array<std::array<float, 2>, 4> base_uv{};
    std::array<std::array<float, 2>, 4> blend_uv{};
    std::array<std::array<float, 3>, 4> normals{{{0,0,1}, {0,0,1}, {0,0,1}, {0,0,1}}};
    bool alternate_diagonal = false;
    bool operator==(const TerrainCell &) const = default;
};

export struct TerrainVertex final
{
    std::array<float, 3> position{};
    std::array<float, 4> color{};
    std::array<float, 2> base_uv{};
    std::array<float, 2> blend_uv{};
    std::array<float, 3> normal{0, 0, 1};
};

static_assert(sizeof(TerrainVertex) == 56);

export std::array<float, 3> Terrain_Surface_Normal(float left, float right, float below, float above, float spacing) noexcept
{
    const float x = (left - right) / (2.0f * spacing);
    const float y = (below - above) / (2.0f * spacing);
    const float inverse_length = 1.0f / std::sqrt(x * x + y * y + 1.0f);
    return {x * inverse_length, y * inverse_length, inverse_length};
}

export struct TerrainGeometryBatch final
{
    std::array<float, 3> minimum{};
    std::array<float, 3> maximum{};
    std::uint32_t first_index = 0;
    std::uint32_t index_count = 0;
};

export class TerrainGeometry final
{
public:
    bool Build(std::span<const TerrainCell> cells, unsigned worker_count = 0)
    {
        constexpr std::size_t max_cells = std::numeric_limits<std::uint32_t>::max()
            / (4 * sizeof(TerrainVertex));
        if (cells.size() > max_cells)
            return false;
        // Validate before replacing the previous geometry. Failed edits must
        // not leave a partially updated surface visible.
        for (const TerrainCell &cell : cells) {
            if (!Is_Valid(cell))
                return false;
        }
        m_vertices.resize(cells.size() * 4);
        m_indices.resize(cells.size() * 6);
        constexpr std::size_t cells_per_batch = 256;
        m_batches.resize((cells.size() + cells_per_batch - 1) / cells_per_batch);
        // Each task owns complete batches and disjoint output ranges. Join
        // before publishing geometry; no GPU or game-state access occurs here.
        if (worker_count == 0) worker_count = std::thread::hardware_concurrency();
        worker_count = std::max(1u, std::min({worker_count, 8u,
            static_cast<unsigned>((m_batches.size() + 63) / 64)}));
        const auto build_range = [&](unsigned worker) {
            const std::size_t begin = m_batches.size() * worker / worker_count;
            const std::size_t end = m_batches.size() * (worker + 1) / worker_count;
            for (std::size_t index = begin; index < end; ++index)
                Build_Batch(cells, index, cells_per_batch);
        };
        std::array<std::future<void>, 7> workers;
        for (unsigned worker = 1; worker < worker_count; ++worker)
            workers[worker - 1] = std::async(std::launch::async, build_range, worker);
        build_range(0);
        for (unsigned worker = 1; worker < worker_count; ++worker)
            workers[worker - 1].get();
        return true;
    }

    std::span<const TerrainGeometryBatch> Batches() const noexcept { return m_batches; }
    std::span<const TerrainVertex> Vertices() const noexcept { return m_vertices; }
    std::span<const std::uint32_t> Indices() const noexcept { return m_indices; }

private:
    void Build_Batch(std::span<const TerrainCell> cells, std::size_t batch_index,
        std::size_t cells_per_batch) noexcept
    {
        constexpr std::array<std::array<float, 2>, 4> corners{{
            {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}};
        constexpr std::array<std::uint32_t, 6> regular{0, 2, 3, 0, 1, 2};
        constexpr std::array<std::uint32_t, 6> alternate{1, 3, 0, 1, 2, 3};
        TerrainGeometryBatch &batch = m_batches[batch_index];
        const std::size_t first_cell = batch_index * cells_per_batch;
        const std::size_t end_cell = std::min(first_cell + cells_per_batch, cells.size());
        batch.first_index = static_cast<std::uint32_t>(first_cell * 6);
        batch.index_count = static_cast<std::uint32_t>((end_cell - first_cell) * 6);
        for (std::size_t index = first_cell; index < end_cell; ++index) {
            const TerrainCell &cell = cells[index];
            for (std::size_t corner = 0; corner < 4; ++corner) {
                auto& vertex = m_vertices[index * 4 + corner];
                vertex = {
                    {cell.origin[0] + corners[corner][0] * cell.spacing[0],
                     cell.origin[1] + corners[corner][1] * cell.spacing[1], cell.heights[corner]},
                    cell.colors[corner], cell.base_uv[corner], cell.blend_uv[corner], cell.normals[corner]};
                if (index == first_cell && corner == 0)
                    batch.minimum = batch.maximum = vertex.position;
                else for (std::size_t axis = 0; axis < 3; ++axis) {
                    batch.minimum[axis] = std::min(batch.minimum[axis], vertex.position[axis]);
                    batch.maximum[axis] = std::max(batch.maximum[axis], vertex.position[axis]);
                }
            }
            const auto& indices = cell.alternate_diagonal ? alternate : regular;
            for (std::size_t triangle_index = 0; triangle_index < 6; ++triangle_index)
                m_indices[index * 6 + triangle_index] = static_cast<std::uint32_t>(index * 4) + indices[triangle_index];
        }
    }

    static bool Is_Valid(const TerrainCell &cell) noexcept
    {
        for (std::size_t axis = 0; axis < 2; ++axis) {
            if (!std::isfinite(cell.origin[axis]) || !std::isfinite(cell.spacing[axis])
                || cell.spacing[axis] <= 0.0f || !std::isfinite(cell.origin[axis] + cell.spacing[axis]))
                return false;
        }
        for (std::size_t corner = 0; corner < 4; ++corner) {
            if (!std::isfinite(cell.heights[corner]))
                return false;
            for (float normal : cell.normals[corner])
                if (!std::isfinite(normal)) return false;
            for (float color : cell.colors[corner])
                if (!std::isfinite(color))
                    return false;
            for (std::size_t axis = 0; axis < 2; ++axis)
                if (!std::isfinite(cell.base_uv[corner][axis]) || !std::isfinite(cell.blend_uv[corner][axis]))
                    return false;
        }
        return true;
    }

    std::vector<TerrainGeometryBatch> m_batches;
    std::vector<TerrainVertex> m_vertices;
    std::vector<std::uint32_t> m_indices;
};

}
