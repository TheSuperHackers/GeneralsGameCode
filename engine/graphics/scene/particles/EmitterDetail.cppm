module;

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <span>

export module Graphics.Scene.Particles.EmitterDetail;

import Assets.Particles;

namespace Graphics
{

// Retains the emitter's decimation levels and screen-area estimate. The game
// does not configure the retired editor's global screen-size limit table.
export class EmitterDetail final
{
public:
    EmitterDetail(std::uint32_t capacity, Assets::EmitterGeometryMode mode)
        : m_capacity(capacity), m_mode(mode)
    {
        Calculate(1.0f, m_values, m_costs);
    }
    EmitterDetail(const EmitterDetail &source)
        : m_capacity(source.m_capacity), m_mode(source.m_mode),
          m_count((std::min)(source.m_capacity, 17u)),
          m_decimation(source.m_decimation), m_bias(source.m_bias)
    {
        m_decimation = (std::min)(m_decimation, m_count - 1);
        Calculate(1.0f, m_values, m_costs);
    }

    std::uint32_t Decimation() const noexcept { return m_decimation; }
    unsigned Count() const noexcept { return m_count; }
    int Level() const noexcept { return static_cast<int>(m_count - 1 - m_decimation); }
    void Set_Level(int level) noexcept
    {
        m_decimation = m_count - 1 - static_cast<unsigned>((std::clamp)(level, 0, static_cast<int>(m_count - 1)));
    }
    void Increment() noexcept { if (m_decimation != 0) --m_decimation; }
    void Decrement() noexcept { if (m_decimation + 1 < m_count) ++m_decimation; }
    void Set_Bias(float bias) noexcept { m_bias = (std::max)(bias, 0.0f); }
    float Cost() const noexcept { return m_costs[Level()]; }
    float Value() const noexcept { return m_values[Level()]; }
    float Next_Value() const noexcept { return m_values[Level() + 1]; }

    void Prepare(float distance, float radius, float maximum_size,
        float width_factor, float height_factor) noexcept
    {
        float sphere_radius = 0.0f, particle_radius = 0.0f;
        if (distance != 0.0f) {
            const float inverse = 1.0f / distance;
            sphere_radius = radius * inverse;
            particle_radius = maximum_size * inverse;
        }
        const float sphere_area = sphere_radius * sphere_radius;
        const float particle_area = particle_radius * particle_radius * m_capacity;
        const float area = 3.141592654f * (std::min)(sphere_area, particle_area) * width_factor * height_factor;
        m_projected_area = 0.9f * m_projected_area + 0.1f * area;
        Calculate(m_projected_area, m_values, m_costs);
    }

    int Calculate(float area, std::span<float> values, std::span<float> costs) const noexcept
    {
        if (values.size() < m_count + 1 || costs.size() < m_count)
            return 0;
        float factor = 0;
        switch (m_mode) {
        case Assets::EmitterGeometryMode::SpriteTriangles: factor = m_capacity * 0.0625f; break;
        case Assets::EmitterGeometryMode::SpriteQuads: factor = m_capacity * 2.0f * 0.0625f; break;
        case Assets::EmitterGeometryMode::Line: factor = static_cast<float>(2 * m_capacity - 1) * 0.0625f; break;
        case Assets::EmitterGeometryMode::LineGroupTetra: factor = m_capacity * 4.0f * 0.0625f; break;
        case Assets::EmitterGeometryMode::LineGroupPrism: factor = m_capacity * 8.0f * 0.0625f; break;
        default: break;
        }
        for (unsigned level = 0; level < m_count; ++level) {
            const float cost = factor * static_cast<float>(level);
            costs[level] = cost != 0.0f ? cost : 0.000001f;
        }
        values[0] = (std::numeric_limits<float>::max)();
        for (unsigned level = 1; level < m_count; ++level) {
            const float polygons = costs[level];
            const float benefit = polygons > 0.0001f ? 1 - (0.5f / (polygons * polygons)) : 0.0f;
            values[level] = (benefit * area * m_bias) / costs[level];
        }
        values[m_count] = -1.0f;
        return 0;
    }

private:
    std::uint32_t m_capacity;
    Assets::EmitterGeometryMode m_mode;
    unsigned m_count = 17;
    unsigned m_decimation = 0;
    float m_bias = 1.0f;
    float m_projected_area = 0.0f;
    std::array<float, 17> m_costs{};
    std::array<float, 18> m_values{};
};

}
