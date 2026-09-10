module;
#include <cassert>
#include <cstddef>
#include <limits>
#include <span>
#include <vector>
export module Graphics.Scene.Models.DetailLevels;

namespace Graphics {

export class ModelDetailLevels final {
    struct Level {
        float maximum_screen_area = (std::numeric_limits<float>::max)();
        float non_pixel_cost = 0;
        float pixel_cost_per_area = 0;
        float benefit_factor = 0;
    };
public:
    void Initialize(std::size_t count) {
        m_levels.assign(count, {});
        m_costs.assign(count, 0);
        m_values.assign(count + 1, 0);
        m_bias = 1;
    }
    void Set_Maximum_Area(std::size_t level, float area) { m_levels[level].maximum_screen_area = area; }
    float Maximum_Area(std::size_t level) const { return m_levels[level].maximum_screen_area; }
    void Set_Bias(float bias) { m_bias = bias; }
    float Bias() const noexcept { return m_bias; }
    void Set_Polygon_Count(std::size_t level, int count) {
        auto& target = m_levels[level];
        target.pixel_cost_per_area = 0;
        target.non_pixel_cost = count != 0 ? static_cast<float>(count) : .000001f;
        target.benefit_factor = count != 0 ? 1 - .5f / (count * count) : 0;
    }
    int Calculate(float area, std::span<float> values, std::span<float> costs) const {
        assert(costs.size() >= m_levels.size() && values.size() > m_levels.size());
        for (std::size_t level = 0; level < m_levels.size(); ++level)
            costs[level] = m_levels[level].non_pixel_cost + m_levels[level].pixel_cost_per_area * area;
        std::size_t level = 0;
        for (; level < m_levels.size() && m_levels[level].maximum_screen_area < area; ++level)
            values[level] = (std::numeric_limits<float>::max)();
        if (m_levels.empty()) {
            values[0] = -1;
            return 0;
        }
        if (level >= m_levels.size()) level = m_levels.size() - 1;
        else values[level] = (std::numeric_limits<float>::max)();
        const int minimum = static_cast<int>(level);
        for (++level; level < m_levels.size(); ++level)
            values[level] = (m_levels[level].benefit_factor * area * m_bias) / costs[level];
        values[m_levels.size()] = -1;
        return minimum;
    }
    int Update(float area) { return Calculate(area, m_values, m_costs); }
    float Cost(std::size_t level) const { return m_costs[level]; }
    float Value(std::size_t level) const { return m_values[level]; }

private:
    std::vector<Level> m_levels;
    std::vector<float> m_costs;
    std::vector<float> m_values;
    float m_bias = 1;
};
}
