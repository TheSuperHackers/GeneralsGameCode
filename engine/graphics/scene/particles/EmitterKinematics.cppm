module;

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

export module Graphics.Scene.Particles.EmitterKinematics;

import Assets.Math;

namespace Graphics
{

export struct EmitterBirth final
{
    std::array<float, 3> position{};
    std::array<float, 3> velocity{};
    std::uint32_t timestamp = 0;
    std::uint8_t group = 0;
};

export struct EmitterIndexRange final
{
    std::uint32_t begin = 0;
    std::uint32_t end = 0;
};

// Slot identity is retained across updates: visual random tables and LOD use
// the physical slot, while connected lines traverse particles in birth order.
export class EmitterKinematics final
{
public:
    EmitterKinematics(std::uint32_t capacity, std::uint32_t lifetime,
        std::array<float, 3> acceleration, bool ping_pong, std::uint32_t time)
        : m_capacity((std::max)(capacity, 2u)), m_lifetime(lifetime),
          m_last_update(time), m_ping_pong(ping_pong), m_pending(m_capacity),
          m_velocity(m_capacity), m_timestamp(m_capacity), m_group(m_capacity)
    {
        m_position[0].resize(m_capacity);
        if (m_ping_pong)
            m_position[1].resize(m_capacity);
        Set_Acceleration(acceleration);
    }

    EmitterKinematics(const EmitterKinematics &source, std::uint32_t time)
        : EmitterKinematics(source.m_capacity, source.m_lifetime,
            source.m_acceleration, source.m_ping_pong, time)
    {
        m_has_acceleration = source.m_has_acceleration;
    }

    EmitterKinematics(const EmitterKinematics &) = delete;
    EmitterKinematics &operator=(const EmitterKinematics &) = delete;

    std::uint32_t Capacity() const noexcept { return m_capacity; }
    std::uint32_t Count() const noexcept { return m_old_count + m_new_count; }
    std::uint32_t Pending_Count() const noexcept { return m_pending_count; }
    std::uint32_t Lifetime() const noexcept { return m_lifetime; }
    std::uint32_t Last_Update() const noexcept { return m_last_update; }
    const std::array<float, 3> &Acceleration() const noexcept { return m_acceleration; }

    void Set_Acceleration(std::array<float, 3> acceleration) noexcept
    {
        m_acceleration = acceleration;
        m_has_acceleration = acceleration[0] != 0 || acceleration[1] != 0 || acceleration[2] != 0;
    }

    void Scale_Acceleration(float scale) noexcept
    {
        for (float &axis : m_acceleration)
            axis *= scale;
    }

    EmitterBirth &Append_Birth() noexcept
    {
        EmitterBirth &birth = m_pending[m_pending_end];
        m_pending_end = Next(m_pending_end);
        if (m_pending_count == m_capacity)
            m_pending_start = Next(m_pending_start);
        else
            ++m_pending_count;
        return birth;
    }

    std::span<const std::array<float, 3>> Positions(std::uint32_t rendered_frame) const noexcept
    {
        return m_position[Position_Index(rendered_frame)];
    }
    std::span<const std::array<float, 3>> Velocities() const noexcept { return m_velocity; }
    std::span<const std::uint32_t> Timestamps() const noexcept { return m_timestamp; }
    std::span<const std::uint8_t> Groups() const noexcept { return m_group; }

    Assets::Bounds3f Bounds(float maximum_size, std::uint32_t rendered_frame) const noexcept
    {
        if (Count() == 0)
            return {};
        const auto positions = Positions(rendered_frame);
        auto minimum = positions[m_start];
        auto maximum = minimum;
        for (const auto range : Active_Ranges()) {
            for (std::uint32_t slot = range.begin; slot < range.end; ++slot) {
                for (unsigned axis = 0; axis < 3; ++axis) {
                    minimum[axis] = (std::min)(minimum[axis], positions[slot][axis]);
                    maximum[axis] = (std::max)(maximum[axis], positions[slot][axis]);
                }
            }
        }
        return {{minimum[0] - maximum_size, minimum[1] - maximum_size, minimum[2] - maximum_size},
            {maximum[0] + maximum_size, maximum[1] + maximum_size, maximum[2] + maximum_size}};
    }

    std::array<EmitterIndexRange, 2> Active_Ranges(bool storage_order = false) const noexcept
    {
        if (m_start < m_end || (m_start == m_end && m_old_count == 0))
            return {{{m_start, m_end}, {m_end, m_end}}};
        if (storage_order)
            return {{{0, m_end}, {m_start, m_capacity}}};
        return {{{m_start, m_capacity}, {0, m_end}}};
    }

    bool Advance(std::uint32_t time, std::uint32_t rendered_frame) noexcept
    {
        const std::uint32_t elapsed = time - m_last_update;
        if (elapsed == 0)
            return false;

        Receive_Births(time, rendered_frame);
        Expire_Particles(time);
        if (m_old_count != 0)
            Move_Particles(elapsed, rendered_frame);
        m_end = m_new_end;
        m_old_count += m_new_count;
        m_new_count = 0;
        m_last_update = time;
        return true;
    }

private:
    std::uint32_t Next(std::uint32_t index) const noexcept
    {
        return index + 1 == m_capacity ? 0 : index + 1;
    }
    unsigned Position_Index(std::uint32_t frame) const noexcept
    {
        return m_ping_pong ? frame & 1u : 0;
    }

    void Receive_Births(std::uint32_t time, std::uint32_t rendered_frame) noexcept
    {
        const unsigned buffer = Position_Index(rendered_frame);
        auto &position = m_position[buffer];
        while (m_pending_count != 0) {
            const EmitterBirth &birth = m_pending[m_pending_start];
            m_pending_start = Next(m_pending_start);
            --m_pending_count;
            m_timestamp[m_new_end] = birth.timestamp;
            const std::uint32_t age = time - birth.timestamp;
            if (age >= m_lifetime)
                continue;
            const float interval = static_cast<float>(age);
            for (unsigned axis = 0; axis < 3; ++axis) {
                if (m_has_acceleration) {
                    position[m_new_end][axis] = birth.position[axis]
                        + (birth.velocity[axis] + 0.5f * m_acceleration[axis] * interval) * interval;
                    m_velocity[m_new_end][axis] = birth.velocity[axis] + m_acceleration[axis] * interval;
                } else {
                    position[m_new_end][axis] = birth.position[axis] + birth.velocity[axis] * interval;
                    m_velocity[m_new_end][axis] = birth.velocity[axis];
                }
            }
            if (m_ping_pong)
                m_position[buffer ^ 1u][m_new_end] = birth.position;
            m_group[m_new_end] = birth.group;
            m_new_end = Next(m_new_end);
            ++m_new_count;
            if (m_new_count + m_old_count > m_capacity) {
                m_start = Next(m_start);
                if (m_old_count != 0)
                    --m_old_count;
                else {
                    m_end = Next(m_end);
                    --m_new_count;
                }
            }
        }
    }

    void Expire_Particles(std::uint32_t time) noexcept
    {
        const auto ranges = Active_Ranges();
        for (const auto range : ranges) {
            for (std::uint32_t slot = range.begin; slot < range.end; ++slot) {
                if (time - m_timestamp[slot] < m_lifetime) {
                    m_start = slot;
                    return;
                }
                --m_old_count;
            }
        }
        m_start = m_end;
    }

    void Move_Particles(std::uint32_t elapsed, std::uint32_t rendered_frame) noexcept
    {
        const unsigned buffer = Position_Index(rendered_frame);
        auto &position = m_position[buffer];
        const auto &previous = m_position[buffer ^ 1u];
        const float interval = static_cast<float>(elapsed);
        std::array<float, 3> velocity_delta{}, acceleration_delta{};
        for (unsigned axis = 0; axis < 3; ++axis) {
            velocity_delta[axis] = m_acceleration[axis] * interval;
            acceleration_delta[axis] = m_acceleration[axis] * (0.5f * interval * interval);
        }
        for (const auto range : Active_Ranges()) {
            for (std::uint32_t slot = range.begin; slot < range.end; ++slot) {
                for (unsigned axis = 0; axis < 3; ++axis) {
                    if (m_has_acceleration) {
                        if (m_ping_pong)
                            position[slot][axis] = previous[slot][axis]
                                + m_velocity[slot][axis] * interval + acceleration_delta[axis];
                        else
                            position[slot][axis] += m_velocity[slot][axis] * interval + acceleration_delta[axis];
                        m_velocity[slot][axis] += velocity_delta[axis];
                    } else {
                        // With no acceleration, each ping-pong buffer advances
                        // its own positions, matching the retained update path.
                        position[slot][axis] += m_velocity[slot][axis] * interval;
                    }
                }
            }
        }
    }

    std::uint32_t m_capacity;
    std::uint32_t m_lifetime;
    std::uint32_t m_last_update;
    std::array<float, 3> m_acceleration{};
    bool m_has_acceleration = false;
    bool m_ping_pong;
    std::vector<EmitterBirth> m_pending;
    std::uint32_t m_pending_start = 0, m_pending_end = 0, m_pending_count = 0;
    std::array<std::vector<std::array<float, 3>>, 2> m_position;
    std::vector<std::array<float, 3>> m_velocity;
    std::vector<std::uint32_t> m_timestamp;
    std::vector<std::uint8_t> m_group;
    std::uint32_t m_start = 0, m_end = 0, m_new_end = 0;
    std::uint32_t m_old_count = 0, m_new_count = 0;
};

}
