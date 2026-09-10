module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

export module Graphics.Scene.Particles.EmitterVisualState;

import Assets.Math;
import Assets.Particles;
import Graphics.Scene.Particles.EmitterKinematics;

namespace Graphics
{

namespace EmitterVisualDetail
{
inline std::uint32_t Random_Table_Size(std::uint32_t capacity) noexcept
{
    std::uint32_t size = 1;
    while (size < capacity && size < 32)
        size *= 2;
    return size;
}

template<std::size_t Channels> struct Key final
{
    std::uint32_t time = 0;
    std::array<float, Channels> value{};
    std::array<float, Channels> delta{};
};

// RGB uses reciprocal multiplication, while scalar tracks use division.
// Retaining that distinction also retains authored interpolation rounding.
template<std::size_t Channels> class Track final
{
public:
    template<class Source, class Convert, class Sample>
    void Prepare(const Source &source, Convert convert, Sample sample,
        std::uint32_t capacity, std::uint32_t lifetime, float epsilon,
        float value_scale = 1.0f, float delta_scale = 1.0f, bool force_varying = false)
    {
        const auto random = convert(source.random);
        bool random_zero = true;
        for (const float channel : random)
            random_zero = random_zero && std::fabs(channel) < epsilon;
        m_varying = force_varying || !random_zero || !source.keys.empty();

        std::size_t retained = 0;
        if (m_varying) {
            while (retained < source.keys.size()
                && static_cast<std::uint32_t>(source.keys[retained].time * 1000.0f) < lifetime)
                ++retained;
        }
        m_keys.resize(retained + 1);
        m_keys[0].time = 0;
        m_keys[0].value = convert(source.start);
        for (float &channel : m_keys[0].value)
            channel *= value_scale;
        for (std::size_t key = 1; key < m_keys.size(); ++key) {
            m_keys[key].time = static_cast<std::uint32_t>(source.keys[key - 1].time * 1000.0f);
            m_keys[key].value = convert(source.keys[key - 1].value);
            for (float &channel : m_keys[key].value)
                channel *= value_scale;
        }
        for (std::size_t key = 0; key + 1 < m_keys.size(); ++key) {
            const float interval = static_cast<float>(m_keys[key + 1].time - m_keys[key].time);
            Set_Delta(m_keys[key], m_keys[key + 1].value, interval, delta_scale, false);
        }
        auto &last = m_keys.back();
        last.delta = {};
        if (m_varying && retained < source.keys.size()) {
            auto next = convert(source.keys[retained].value);
            for (float &channel : next)
                channel *= value_scale;
            const float interval = source.keys[retained].time * 1000.0f - static_cast<float>(last.time);
            Set_Delta(last, next, interval, delta_scale, true);
        }

        m_random.clear();
        if (m_varying) {
            m_random.resize(random_zero ? 1 : Random_Table_Size(capacity));
            if (!random_zero) {
                std::array<float, Channels> scale;
                const float inverse_max = 1.0f / static_cast<float>((std::numeric_limits<std::int32_t>::max)());
                for (std::size_t channel = 0; channel < Channels; ++channel)
                    scale[channel] = random[channel] * value_scale * inverse_max;
                for (auto &entry : m_random)
                    entry = sample(scale);
            }
        }
    }

    bool Varying() const noexcept { return m_varying; }
    std::size_t Last_Key() const noexcept { return m_keys.size() - 1; }
    const std::array<float, Channels> &Start() const noexcept { return m_keys.front().value; }
    const std::vector<Key<Channels>> &Keys() const noexcept { return m_keys; }

    std::array<float, Channels> Sample(std::uint32_t age, std::uint32_t slot, std::size_t &key) const noexcept
    {
        if (!m_varying)
            return Start();
        while (age < m_keys[key].time)
            --key;
        std::array<float, Channels> result;
        const auto &value = m_keys[key];
        const auto &random = m_random[slot & (m_random.size() - 1)];
        const float interval = static_cast<float>(age - value.time);
        for (std::size_t channel = 0; channel < Channels; ++channel)
            result[channel] = value.value[channel] + value.delta[channel] * interval + random[channel];
        return result;
    }

    float Random(std::uint32_t slot) const noexcept requires (Channels == 1)
    {
        return m_random[slot & (m_random.size() - 1)][0];
    }

    void Scale(float scale) noexcept
    {
        for (auto &key : m_keys) {
            for (float &value : key.value)
                value *= scale;
            for (float &delta : key.delta)
                delta *= scale;
        }
        for (auto &entry : m_random)
            for (float &value : entry)
                value *= scale;
    }

private:
    static void Set_Delta(Key<Channels> &key, const std::array<float, Channels> &next,
        float interval, float scale, bool last) noexcept
    {
        if constexpr (Channels == 3) {
            const float inverse = 1.0f / interval;
            for (std::size_t channel = 0; channel < Channels; ++channel)
                key.delta[channel] = (next[channel] - key.value[channel]) * inverse;
        } else {
            if (last)
                key.delta[0] = scale * (next[0] - key.value[0]) / interval;
            else
                key.delta[0] = scale * ((next[0] - key.value[0]) / interval);
        }
    }

    bool m_varying = false;
    std::vector<Key<Channels>> m_keys;
    std::vector<std::array<float, Channels>> m_random;
};

class OrientationTrack final
{
public:
    template<class Sample>
    void Prepare(const Assets::EmitterFloatTrack &source, float orientation_random,
        Sample sample, std::uint32_t capacity, std::uint32_t lifetime)
    {
        constexpr float epsilon = 2.77777778e-4f;
        const bool orientation_zero = std::fabs(orientation_random) < epsilon;
        const bool enabled = !orientation_zero || std::fabs(source.random) >= epsilon
            || !source.keys.empty() || std::fabs(source.start) >= epsilon;
        m_rotation.Prepare(source, [](float value) { return std::array{value}; },
            sample, capacity, lifetime, epsilon, 0.001f, 0.5f, enabled);
        if (!enabled)
            return;
        const auto &keys = m_rotation.Keys();
        m_integrated.resize(keys.size());
        for (std::size_t key = 1; key < keys.size(); ++key) {
            const float interval = static_cast<float>(keys[key].time - keys[key - 1].time);
            m_integrated[key] = m_integrated[key - 1] + interval
                * (keys[key - 1].value[0] + keys[key - 1].delta[0] * interval);
        }
        m_random.resize(orientation_zero ? 1 : Random_Table_Size(capacity));
        if (!orientation_zero) {
            const float inverse_max = 1.0f / static_cast<float>((std::numeric_limits<std::int32_t>::max)());
            const std::array scale{orientation_random * inverse_max};
            for (float &entry : m_random)
                entry = sample(scale)[0];
        }
    }

    bool Varying() const noexcept { return m_rotation.Varying(); }
    std::size_t Last_Key() const noexcept { return m_rotation.Last_Key(); }
    std::uint8_t Sample(std::uint32_t age, std::uint32_t slot, std::size_t &key) const noexcept
    {
        const auto &keys = m_rotation.Keys();
        while (age < keys[key].time)
            --key;
        const float interval = static_cast<float>(age - keys[key].time);
        const float orientation = m_integrated[key]
            + (keys[key].value[0] + keys[key].delta[0] * interval) * interval
            + m_rotation.Random(slot) * static_cast<float>(age)
            + m_random[slot & (m_random.size() - 1)];
        return static_cast<std::uint8_t>(static_cast<int>(orientation * 256.0f) & 255);
    }

private:
    Track<1> m_rotation;
    std::vector<float> m_integrated;
    std::vector<float> m_random;
};
}

// Prepared tracks and visual arrays are independent of simulation storage and
// texture ownership. Rendering reads values without advancing the simulation.
export class EmitterVisualState final
{
public:
    template<class SampleScalar, class SampleColor>
    EmitterVisualState(const Assets::EmitterAssetDesc &description,
        std::uint32_t capacity, std::uint32_t lifetime,
        SampleScalar scalar_random, SampleColor color_random)
        : m_line_group(description.geometry_mode == Assets::EmitterGeometryMode::LineGroupTetra
            || description.geometry_mode == Assets::EmitterGeometryMode::LineGroupPrism)
    {
        const auto scalar = [](float value) { return std::array{value}; };
        const auto color = [](Assets::Color4f value) { return std::array{value.r, value.g, value.b}; };
        // Preparation order is part of the random-stream contract.
        m_color_track.Prepare(description.color, color, color_random, capacity, lifetime, 0.0038f);
        m_alpha_track.Prepare(description.opacity, scalar, scalar_random, capacity, lifetime, 0.0038f);
        m_size_track.Prepare(description.size, scalar, scalar_random, capacity, lifetime, 1.0e-12f);
        m_orientation_track.Prepare(description.rotation, description.initial_orientation_random,
            scalar_random, capacity, lifetime);
        m_frame_track.Prepare(description.frame, scalar, scalar_random, capacity, lifetime, 0.1f);
        m_blur_track.Prepare(description.blur_time, scalar, scalar_random, capacity, lifetime, 1.0e-5f);

        m_max_size = m_size_track.Start()[0];
        if (m_size_track.Varying()) {
            for (const auto &key : m_size_track.Keys())
                m_max_size = (std::max)(m_max_size, key.value[0]);
            const auto &last = m_size_track.Keys().back();
            const float last_size = last.value[0] + last.delta[0] * static_cast<float>(lifetime - last.time);
            m_max_size = (std::max)(m_max_size, last_size) + std::fabs(description.size.random);
        }
        Allocate(capacity);
    }

    EmitterVisualState(const EmitterVisualState &source)
        : m_color_track(source.m_color_track), m_alpha_track(source.m_alpha_track),
          m_size_track(source.m_size_track), m_orientation_track(source.m_orientation_track),
          m_frame_track(source.m_frame_track), m_blur_track(source.m_blur_track),
          m_line_group(source.m_line_group), m_max_size(source.m_max_size)
    {
        Allocate(source.m_capacity);
    }
    EmitterVisualState &operator=(const EmitterVisualState &) = delete;

    float Max_Size() const noexcept { return m_max_size; }
    float Start_Size() const noexcept { return m_size_track.Start()[0]; }
    bool Has_Diffuse() const noexcept { return !m_diffuse.empty(); }
    bool Is_Line_Group() const noexcept { return m_line_group; }

    std::array<float, 4> Default_Color() const noexcept
    {
        const auto &color = m_color_track.Start();
        return {color[0], color[1], color[2], m_alpha_track.Start()[0]};
    }
    std::array<float, 4> Color(std::uint32_t slot) const noexcept
    {
        return Has_Diffuse() ? m_diffuse[slot] : Default_Color();
    }
    float Size(std::uint32_t slot) const noexcept
    {
        return m_size.empty() ? Start_Size() : m_size[slot];
    }
    std::uint8_t Orientation(std::uint32_t slot) const noexcept
    {
        return m_orientation.empty() ? 0 : m_orientation[slot];
    }
    float Frame(std::uint32_t slot) const noexcept
    {
        return m_frame.empty() ? m_frame_track.Start()[0] : m_frame[slot];
    }
    const std::array<float, 3> &Tail(std::uint32_t slot) const noexcept { return m_tail[slot]; }

    void Scale(float scale) noexcept
    {
        m_size_track.Scale(scale);
        m_max_size *= scale;
    }

    void Evaluate(const EmitterKinematics &particles, std::uint32_t time, std::uint32_t rendered_frame) noexcept
    {
        if (m_diffuse.empty() && m_size.empty() && m_orientation.empty() && m_frame.empty() && m_tail.empty())
            return;
        auto color_key = m_color_track.Last_Key();
        auto alpha_key = m_alpha_track.Last_Key();
        auto size_key = m_size_track.Last_Key();
        auto orientation_key = m_orientation_track.Last_Key();
        auto frame_key = m_frame_track.Last_Key();
        auto blur_key = m_blur_track.Last_Key();
        const auto positions = particles.Positions(rendered_frame);
        const auto velocities = particles.Velocities();
        const auto timestamps = particles.Timestamps();
        for (const auto range : particles.Active_Ranges()) {
            for (std::uint32_t slot = range.begin; slot < range.end; ++slot) {
                const std::uint32_t age = time - timestamps[slot];
                if (!m_diffuse.empty()) {
                    // A missing varying RGB/alpha array supplies white/one in
                    // the combined stream; constant-only draws use Default_Color.
                    const auto color = m_color_track.Varying()
                        ? m_color_track.Sample(age, slot, color_key) : std::array{1.0f, 1.0f, 1.0f};
                    const float alpha = m_alpha_track.Varying()
                        ? m_alpha_track.Sample(age, slot, alpha_key)[0] : 1.0f;
                    m_diffuse[slot] = {color[0], color[1], color[2], alpha};
                    for (float &channel : m_diffuse[slot])
                        channel = (std::clamp)(channel, 0.0f, 1.0f);
                }
                if (!m_size.empty()) {
                    const float size = m_size_track.Sample(age, slot, size_key)[0];
                    m_size[slot] = size >= 0.0f ? size : 0.0f;
                }
                if (!m_orientation.empty())
                    m_orientation[slot] = m_orientation_track.Sample(age, slot, orientation_key);
                if (!m_frame.empty()) {
                    const float frame = m_frame_track.Sample(age, slot, frame_key)[0];
                    m_frame[slot] = m_line_group ? frame : static_cast<float>(static_cast<int>(frame) & 255);
                }
                if (!m_tail.empty()) {
                    const float blur = m_blur_track.Sample(age, slot, blur_key)[0];
                    for (unsigned axis = 0; axis < 3; ++axis)
                        m_tail[slot][axis] = positions[slot][axis] - velocities[slot][axis] * blur * 1000;
                }
            }
        }
    }

private:
    void Allocate(std::uint32_t capacity)
    {
        m_capacity = capacity;
        if (m_color_track.Varying() || m_alpha_track.Varying())
            m_diffuse.resize(capacity);
        if (m_size_track.Varying())
            m_size.resize(capacity);
        if (m_orientation_track.Varying())
            m_orientation.resize(capacity);
        if (m_frame_track.Varying())
            m_frame.resize(capacity);
        if (m_line_group)
            m_tail.resize(capacity);
    }

    EmitterVisualDetail::Track<3> m_color_track;
    EmitterVisualDetail::Track<1> m_alpha_track;
    EmitterVisualDetail::Track<1> m_size_track;
    EmitterVisualDetail::OrientationTrack m_orientation_track;
    EmitterVisualDetail::Track<1> m_frame_track;
    EmitterVisualDetail::Track<1> m_blur_track;
    bool m_line_group;
    float m_max_size;
    std::uint32_t m_capacity = 0;
    std::vector<std::array<float, 4>> m_diffuse;
    std::vector<float> m_size;
    std::vector<std::uint8_t> m_orientation;
    std::vector<float> m_frame;
    std::vector<std::array<float, 3>> m_tail;
};

}
