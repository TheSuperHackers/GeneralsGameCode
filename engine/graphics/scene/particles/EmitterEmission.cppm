module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numeric>

export module Graphics.Scene.Particles.EmitterEmission;

import Assets.Math;
import Assets.Particles;
import Graphics.Scene.Particles.EmitterKinematics;

namespace Graphics
{

export struct EmitterTransform final
{
    std::array<float, 4> orientation{0, 0, 0, 1};
    std::array<float, 3> origin{};
};

// Emission uses the cached interpolation contract, including its angle
// denominator and authored quaternion magnitude. Model animation has a
// different interpolation contract and must not be substituted here.
export class EmitterRotationInterval final
{
public:
    EmitterRotationInterval(const std::array<float, 4> &first,
        const std::array<float, 4> &second) noexcept
        : m_first(first), m_second(second)
    {
        float cosine = first[0] * second[0] + first[1] * second[1]
            + first[2] * second[2] + first[3] * second[3];
        m_flip = cosine < 0.0f;
        if (m_flip)
            cosine = -cosine;
        m_linear = 1.0f - cosine < 0.001;
        if (!m_linear)
            m_angle = std::acos(cosine);
    }

    std::array<float, 4> Sample(float fraction) const noexcept
    {
        float complement;
        if (m_linear)
            complement = 1.0f - fraction;
        else {
            const float inverse_angle = 1.0f / m_angle;
            complement = std::sin(m_angle - fraction * m_angle) * inverse_angle;
            fraction = std::sin(fraction * m_angle) * inverse_angle;
        }
        if (m_flip)
            fraction = -fraction;
        std::array<float, 4> result;
        for (unsigned axis = 0; axis < 4; ++axis)
            result[axis] = complement * m_first[axis] + fraction * m_second[axis];
        return result;
    }

private:
    std::array<float, 4> m_first, m_second;
    float m_angle = 0;
    bool m_flip = false, m_linear = true;
};

export std::array<float, 3> Rotate_Emitter_Vector(const std::array<float, 4> &q,
    const std::array<float, 3> &v) noexcept
{
    const float x = q[3] * v[0] + (q[1] * v[2] - v[1] * q[2]);
    const float y = q[3] * v[1] - (q[0] * v[2] - v[0] * q[2]);
    const float z = q[3] * v[2] + (q[0] * v[1] - v[0] * q[1]);
    const float w = -(q[0] * v[0] + q[1] * v[1] + q[2] * v[2]);
    return {w * (-q[0]) + q[3] * x + (y * (-q[2]) - (-q[1]) * z),
        w * (-q[1]) + q[3] * y - (x * (-q[2]) - (-q[0]) * z),
        w * (-q[2]) + q[3] * z + (x * (-q[1]) - (-q[0]) * y)};
}

export std::uint32_t Emitter_Buffer_Capacity(const Assets::EmitterAssetDesc &description,
    int maximum_buffer_size = 0) noexcept
{
    const float lifetime = description.lifetime > 0.0f ? description.lifetime : 1.0f;
    const std::uint32_t burst = description.burst_size != 0 ? description.burst_size : 1;
    int capacity = static_cast<int>(burst * description.emission_rate * (lifetime + 1));
    const int maximum_emissions = static_cast<int>(description.max_emissions);
    if (maximum_emissions > 0)
        capacity = (std::min)(capacity, maximum_emissions);
    if (maximum_buffer_size > 0)
        capacity = (std::min)(capacity, maximum_buffer_size);
    return static_cast<std::uint32_t>((std::max)(capacity, 2));
}

// The scene chooses when to emit and supplies transforms and random vectors.
// This owner handles burst timing, finite emission counts, and birth placement.
export class EmitterEmission final
{
public:
    explicit EmitterEmission(const Assets::EmitterAssetDesc &description)
        : m_interval(description.emission_rate > 0.0f
            ? static_cast<std::uint32_t>(1000.0f / description.emission_rate) : 1000u),
          m_burst(description.burst_size != 0 ? description.burst_size : 1),
          m_base_velocity{description.velocity.x * 0.001f,
              description.velocity.y * 0.001f, description.velocity.z * 0.001f},
          m_outward_velocity(description.outward_velocity * 0.001f),
          m_inheritance(description.velocity_inheritance),
          m_remaining(static_cast<int>(description.max_emissions)), m_maximum(m_remaining)
    {
    }

    std::uint32_t Interval() const noexcept { return m_interval; }
    std::uint32_t Remainder() const noexcept { return m_remainder; }
    int Remaining() const noexcept { return m_remaining; }
    bool Complete() const noexcept { return m_complete; }
    std::uint8_t Group() const noexcept { return m_group; }
    const EmitterTransform &Previous_Transform() const noexcept { return m_previous; }

    void Set_Previous_Transform(const EmitterTransform &transform) noexcept { m_previous = transform; }
    void Reset() noexcept
    {
        m_remaining = m_maximum;
        m_remainder = 0;
        m_complete = false;
    }
    void Start() noexcept
    {
        if (m_complete) {
            m_remaining = m_maximum;
            m_complete = false;
        }
        ++m_group;
    }
    void Prepare_Clone() noexcept
    {
        m_complete = false;
        m_group = 0;
    }
    void Scale(float scale) noexcept
    {
        for (float &axis : m_base_velocity)
            axis *= scale;
        m_outward_velocity *= scale;
    }
    void Set_One_Time_Burst(std::uint32_t count) noexcept
    {
        m_one_time_count = count != 0 ? count : 1;
        m_one_time = true;
    }

    template<class PositionRandom, class VelocityRandom>
    bool Emit(EmitterKinematics &particles, std::uint32_t frame_time,
        std::uint32_t time, const EmitterTransform &current,
        PositionRandom position_random, VelocityRandom velocity_random) noexcept
    {
        if (m_interval == 0)
            return false;
        std::uint32_t interval = frame_time;
        if (interval > 100u * m_interval) {
            const std::uint32_t bursts = particles.Capacity() / std::gcd(particles.Capacity(), m_burst);
            const std::uint32_t cycle_time = m_interval * bursts;
            interval = cycle_time > 1 ? interval % cycle_time : 1;
        }
        m_remainder += interval;
        const float duration = static_cast<float>(interval);
        float fraction = 1 - static_cast<float>(m_remainder) / duration;
        const float fraction_step = static_cast<float>(m_interval) / duration;
        const EmitterRotationInterval rotation(m_previous.orientation, current.orientation);
        std::array<float, 3> inherited{};
        if (m_inheritance != 0) {
            const float scale = m_inheritance / duration;
            for (unsigned axis = 0; axis < 3; ++axis)
                inherited[axis] = (current.origin[axis] - m_previous.origin[axis]) * scale;
        }

        while (m_remainder > m_interval) {
            m_remainder -= m_interval;
            fraction += fraction_step;
            EmitterTransform at_birth;
            at_birth.orientation = rotation.Sample(fraction);
            for (unsigned axis = 0; axis < 3; ++axis)
                at_birth.origin[axis] = m_previous.origin[axis]
                    + (current.origin[axis] - m_previous.origin[axis]) * fraction;
            std::uint32_t count = m_one_time ? m_one_time_count : m_burst;
            m_one_time = false;
            if (m_remaining > 0) {
                if (count > static_cast<std::uint32_t>(m_remaining)) {
                    count = static_cast<std::uint32_t>(m_remaining);
                    m_remaining = 0;
                } else
                    m_remaining -= count;
                if (m_remaining <= 0)
                    m_complete = true;
            }
            const std::uint32_t timestamp = time - m_remainder;
            for (std::uint32_t particle = 0; particle < count; ++particle) {
                EmitterBirth &birth = particles.Append_Birth();
                birth.timestamp = timestamp;
                const auto local_position = position_random();
                birth.position = Rotate_Emitter_Vector(at_birth.orientation, local_position);
                for (unsigned axis = 0; axis < 3; ++axis)
                    birth.position[axis] += at_birth.origin[axis];
                auto local_velocity = velocity_random();
                if (m_outward_velocity != 0) {
                    const float length_squared = local_position[0] * local_position[0]
                        + local_position[1] * local_position[1] + local_position[2] * local_position[2];
                    if (length_squared != 0) {
                        const float scale = m_outward_velocity * (1.0f / std::sqrt(length_squared));
                        for (unsigned axis = 0; axis < 3; ++axis)
                            local_velocity[axis] += local_position[axis] * scale;
                    } else
                        local_velocity[0] += m_outward_velocity;
                }
                for (unsigned axis = 0; axis < 3; ++axis)
                    local_velocity[axis] += m_base_velocity[axis];
                const auto world_velocity = Rotate_Emitter_Vector(at_birth.orientation, local_velocity);
                for (unsigned axis = 0; axis < 3; ++axis)
                    birth.velocity[axis] = inherited[axis] + world_velocity[axis];
                birth.group = m_group;
            }
            if (m_complete)
                break;
        }
        m_previous = current;
        return true;
    }

private:
    std::uint32_t m_interval;
    std::uint32_t m_burst;
    std::uint32_t m_one_time_count = 1;
    bool m_one_time = false;
    std::array<float, 3> m_base_velocity;
    float m_outward_velocity;
    float m_inheritance;
    std::uint32_t m_remainder = 0;
    EmitterTransform m_previous;
    int m_remaining;
    int m_maximum;
    bool m_complete = false;
    std::uint8_t m_group = 0;
};

}
