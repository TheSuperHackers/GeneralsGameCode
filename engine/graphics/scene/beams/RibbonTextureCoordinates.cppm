module;
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>

export module Graphics.Scene.Beams.RibbonTextureCoordinates;

export namespace Graphics {

enum class RibbonTextureMapping { Across, Along, Tiled };

// Point indices are absolute within the ribbon, including across chunk boundaries.
inline float Ribbon_Texture_V(RibbonTextureMapping mapping, std::size_t point, float tiles) noexcept
{
    return mapping == RibbonTextureMapping::Across ? 0.0f : static_cast<float>(point) * tiles;
}

inline std::array<float, 2> Ribbon_Texture_U(RibbonTextureMapping mapping) noexcept
{
    return {0.0f, mapping == RibbonTextureMapping::Along ? 0.0f : 1.0f};
}

class RibbonTextureCoordinates {
public:
    explicit RibbonTextureCoordinates(std::uint32_t time = 0) noexcept : m_time(time) {}

    void SetOffset(std::array<float, 2> offset) noexcept { m_offset = offset; }
    void SetRate(std::array<float, 2> per_second) noexcept
    {
        m_rate = {per_second[0] * 0.001f, per_second[1] * 0.001f};
    }
    std::array<float, 2> Rate() const noexcept
    {
        return {m_rate[0] * 1000.0f, m_rate[1] * 1000.0f};
    }
    void Reset(std::uint32_t time) noexcept
    {
        m_time = time;
        m_offset = {};
    }
    std::array<float, 2> Advance(std::uint32_t time) noexcept
    {
        // Unsigned subtraction preserves elapsed time across the clock rollover.
        const auto elapsed = static_cast<float>(time - m_time);
        for (std::size_t axis = 0; axis < m_offset.size(); ++axis) {
            const float offset = m_offset[axis] + m_rate[axis] * elapsed;
            m_offset[axis] = offset - std::floor(offset);
        }
        m_time = time;
        return m_offset;
    }

private:
    std::uint32_t m_time;
    std::array<float, 2> m_offset{};
    std::array<float, 2> m_rate{};
};

}
