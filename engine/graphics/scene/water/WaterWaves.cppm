module;
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <numbers>
#include <span>
#include <vector>

export module Graphics.Scene.Water.Waves;
export import Graphics.Scene.Water.Displacement;

namespace Graphics
{
export struct WaterBodyMotion final
{
    std::uint32_t id = 0;
    std::array<float,2> position{};
    std::array<float,2> forward{1,0};
    float radius = 45;
};

export class WaterWaves final
{
public:
    void Clear() noexcept
    {
        m_rings.clear();
        m_bodies.clear();
        m_particles.clear();
        m_time = 0;
        m_prepared = false;
    }

    bool Emit_Circle(const std::array<float,2>& center, float radius,
        float speed, float amplitude, float time)
    {
        if (!std::isfinite(center[0]) || !std::isfinite(center[1]) || !std::isfinite(radius)
            || !std::isfinite(speed) || !std::isfinite(amplitude) || !std::isfinite(time)
            || radius <= 0 || speed <= 0 || amplitude < MinimumAmplitude) return false;
        const float count = std::floor(Tau*radius/ParticleSpacing);
        if (count > MaximumParticles) return false;
        m_rings.push_back({center,radius,speed,amplitude,time,
            static_cast<unsigned>((std::max)(count,1.0f))});
        m_prepared = false;
        return true;
    }

    void Update_Bodies(float time, std::span<const WaterBodyMotion> bodies)
    {
        if (!std::isfinite(time)) return;
        if (time < m_time) Clear();
        if (time == m_time && !m_bodies.empty()) return;
        m_time = time;
        for (auto& state : m_bodies) state.seen = false;
        for (const auto& body : bodies) {
            if (!std::isfinite(body.position[0]) || !std::isfinite(body.position[1])
                || !std::isfinite(body.radius) || body.radius <= 0
                || !std::isfinite(body.forward[0]) || !std::isfinite(body.forward[1])) continue;
            auto found = std::find_if(m_bodies.begin(),m_bodies.end(),
                [&](const auto& state) { return state.id == body.id; });
            if (found == m_bodies.end()) {
                m_bodies.push_back({body.id,body.position,time,true});
                continue;
            }
            const float elapsed = time-found->time;
            const float distance = std::hypot(body.position[0]-found->position[0],body.position[1]-found->position[1]);
            if (elapsed > 0 && elapsed <= 0.25f && distance < (std::max)(body.radius*4,100.0f)) {
                const bool moving = distance/elapsed > 1.0f;
                const float scale = std::clamp(body.radius/45.0f,0.2f,3.0f);
                const float radius = (moving ? 6.0f : 5.0f)*scale;
                const float speed = (moving ? 80.0f : 40.0f)*std::sqrt(scale);
                const float amplitude = moving ? 3.0f : 1.0f;
                std::array<float,2> center = body.position;
                if (moving) {
                    const float length = std::hypot(body.forward[0],body.forward[1]);
                    if (length > 0) for (unsigned axis = 0; axis < 2; ++axis)
                        center[axis] += body.forward[axis]*body.radius/length;
                }
                Emit_Circle(center,radius,speed,amplitude,time);
            }
            found->position = body.position;
            found->time = time;
            found->seen = true;
        }
        std::erase_if(m_bodies,[](const auto& state) { return !state.seen; });
    }

    std::span<const OceanWaveParticle> Prepare(float time)
    {
        if (m_prepared && m_prepared_time == time) return m_particles;
        m_particles.clear();
        if (!std::isfinite(time)) return m_particles;
        std::size_t retained = 0;
        std::size_t particle_count = 0;
        for (auto ring : m_rings) {
            const float age = time-ring.time;
            if (age < 0) continue;
            const float radius = ring.radius+age*ring.speed;
            const float spacing = Tau*radius/ring.count;
            float span = spacing;
            while (span > ParticleSpacing && ring.amplitude >= MinimumAmplitude) {
                if (ring.count > MaximumParticles/3) { ring.amplitude = 0; break; }
                ring.amplitude /= 3;
                ring.count *= 3;
                span /= 3;
            }
            if (ring.amplitude < MinimumAmplitude) continue;
            if (ring.count > MaximumParticles-particle_count) continue;
            m_rings[retained++] = ring;
            particle_count += ring.count;
        }
        m_rings.resize(retained);
        m_particles.resize(particle_count);
        std::size_t output = 0;
        for (const auto& ring : m_rings) {
            for (unsigned index = 0; index < ring.count; ++index) {
                const float angle = Tau*index/ring.count;
                m_particles[output++] = {ring.center,{std::cos(angle)*ring.speed,std::sin(angle)*ring.speed},
                    ring.amplitude,ring.time-ring.radius/ring.speed};
            }
        }
        m_prepared_time = time;
        m_prepared = true;
        return m_particles;
    }

    std::size_t Ring_Count() const noexcept { return m_rings.size(); }

private:
    static constexpr float Tau = 2*std::numbers::pi_v<float>;
    static constexpr float ParticleSpacing = 13;
    static constexpr float MinimumAmplitude = 0.1f;
    static constexpr unsigned MaximumParticles = OceanDisplacement::MaximumParticles;
    struct Ring {
        std::array<float,2> center;
        float radius, speed, amplitude, time;
        unsigned count;
    };
    struct BodyState {
        std::uint32_t id;
        std::array<float,2> position;
        float time;
        bool seen;
    };
    std::vector<Ring> m_rings;
    std::vector<BodyState> m_bodies;
    std::vector<OceanWaveParticle> m_particles;
    float m_time = 0;
    float m_prepared_time = 0;
    bool m_prepared = false;
};
}
