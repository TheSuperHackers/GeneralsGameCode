module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

export module Graphics.Scene.WeatherParticles;

export import Graphics.Scene.Particles;
export import Graphics.Scene.Particles.Renderer;
export import Graphics.Scene.Views.View;

import Graphics.Memory.AlignedAllocator;

namespace Graphics
{

export struct WeatherParticleFieldDescription final
{
	std::span<const float> starting_heights{};
	std::uint32_t noise_width = 0;
	std::uint32_t noise_height = 0;
	float box_dimensions = 0.0f;
	float emitter_spacing = 0.0f;
	float fall_speed = 0.0f;
	float frequency_scale_x = 0.0f;
	float frequency_scale_y = 0.0f;
	float drift_amplitude = 0.0f;
	float particle_size = 0.0f;
	float culling_radius = 0.0f;
	std::array<float, 4> color{1.0f, 1.0f, 1.0f, 1.0f};
	MaterialHandle material{};
	ParticleEmitterFlags emitter_flags = ParticleEmitterFlags::Enabled | ParticleEmitterFlags::Billboard;
	PipelineHandle pipeline{};
};

export struct WeatherParticleCullingBounds final
{
	bool enabled = false;
	Vector3 center{};
	Vector3 extent{};
};

export bool Is_Valid_Weather_Particle_Field(const WeatherParticleFieldDescription &description) noexcept
{
	if (description.noise_width == 0 || description.noise_height == 0
		|| description.noise_width > std::numeric_limits<std::size_t>::max() / description.noise_height
		|| description.starting_heights.size() < static_cast<std::size_t>(description.noise_width) * description.noise_height
		|| !std::isfinite(description.box_dimensions) || description.box_dimensions <= 0.0f
		|| !std::isfinite(description.emitter_spacing) || description.emitter_spacing <= 0.0f
		|| !std::isfinite(description.fall_speed) || description.fall_speed <= 0.0f
		|| !std::isfinite(description.frequency_scale_x) || !std::isfinite(description.frequency_scale_y)
		|| !std::isfinite(description.drift_amplitude) || !std::isfinite(description.particle_size)
		|| !std::isfinite(description.culling_radius)
		|| description.particle_size < 0.0f || description.culling_radius < 0.0f)
		return false;

	for (const float component : description.color)
		if (!std::isfinite(component))
			return false;
	return true;
}

export class WeatherParticles final
{
public:
	WeatherParticles() noexcept = default;
	~WeatherParticles() noexcept
	{
		Shutdown();
	}

	WeatherParticles(const WeatherParticles &) = delete;
	WeatherParticles &operator=(const WeatherParticles &) = delete;
	WeatherParticles(WeatherParticles &&) = delete;
	WeatherParticles &operator=(WeatherParticles &&) = delete;

	bool Initialize(std::size_t capacity)
	{
		if (m_capacity != 0 || capacity == 0 || capacity > std::numeric_limits<std::uint32_t>::max())
			return false;

		m_position_x.resize(capacity);
		m_position_y.resize(capacity);
		m_position_z.resize(capacity);
		m_sizes.resize(capacity);
		m_color_r.resize(capacity);
		m_color_g.resize(capacity);
		m_color_b.resize(capacity);
		m_color_a.resize(capacity);
		m_zero.resize(capacity, 0.0f);
		m_one.resize(capacity, 1.0f);
		m_materials.resize(capacity);
		m_emitter_flags.resize(capacity);
		m_capacity = capacity;
		return true;
	}

	void Shutdown() noexcept
	{
		if (m_renderer != nullptr && m_emitter.Is_Valid())
			m_renderer->Destroy_Emitter(m_emitter);
		m_renderer = nullptr;
		m_emitter = {};
		m_emitter_state = {};
		m_position_x.clear();
		m_position_y.clear();
		m_position_z.clear();
		m_sizes.clear();
		m_color_r.clear();
		m_color_g.clear();
		m_color_b.clear();
		m_color_a.clear();
		m_zero.clear();
		m_one.clear();
		m_materials.clear();
		m_emitter_flags.clear();
		m_capacity = 0;
		m_count = 0;
		m_candidate_count = 0;
	}

	void Reset() noexcept
	{
		m_count = 0;
		m_candidate_count = 0;
	}

	bool Is_Initialized() const noexcept
	{
		return m_capacity != 0;
	}

	std::size_t Capacity() const noexcept
	{
		return m_capacity;
	}

	bool Configure(const WeatherParticleFieldDescription &description) noexcept
	{
		if (!Is_Initialized() || !Is_Valid_Weather_Particle_Field(description))
			return false;

		ParticleEmitter emitter;
		emitter.particle_size = description.particle_size;
		emitter.color = description.color;
		emitter.material = description.material;
		emitter.flags = description.emitter_flags;
		emitter.max_particles = static_cast<std::uint32_t>(m_capacity);
		emitter.pipeline = description.pipeline;
		if (m_emitter_state.material != emitter.material
			|| m_emitter_state.flags != emitter.flags
			|| m_emitter_state.max_particles != emitter.max_particles
			|| m_emitter_state.pipeline != emitter.pipeline) {
			if (m_renderer != nullptr && m_emitter.Is_Valid() && !m_renderer->Update_Emitter(m_emitter, emitter))
				return false;
			m_emitter_state = emitter;
		}
		return true;
	}

	bool Bind(ParticleRenderer &renderer) noexcept
	{
		if (!Is_Initialized() || !renderer.Is_Initialized())
			return false;
		if (m_renderer != nullptr && m_renderer != &renderer)
			return false;
		m_renderer = &renderer;
		if (!m_emitter.Is_Valid())
			m_emitter = renderer.Create_Emitter(m_emitter_state);
		return m_emitter.Is_Valid();
	}

	bool Unbind(ParticleRenderer &renderer) noexcept
	{
		if (m_renderer != &renderer)
			return false;
		const bool destroyed = !m_emitter.Is_Valid() || renderer.Destroy_Emitter(m_emitter);
		m_renderer = nullptr;
		m_emitter = {};
		return destroyed;
	}

	bool Update(const WeatherParticleFieldDescription &description, Vector3 camera_position, float time_seconds,
		const View &view, const WeatherParticleCullingBounds &bounds) noexcept
	{
		if (!Is_Initialized() || !Is_Valid_Weather_Particle_Field(description)
			|| !std::isfinite(camera_position.x) || !std::isfinite(camera_position.y)
			|| !std::isfinite(camera_position.z) || !std::isfinite(time_seconds))
			return false;
		if (bounds.enabled && (!std::isfinite(bounds.center.x) || !std::isfinite(bounds.center.y)
			|| !std::isfinite(bounds.center.z) || !std::isfinite(bounds.extent.x)
			|| !std::isfinite(bounds.extent.y) || !std::isfinite(bounds.extent.z)
			|| bounds.extent.x < 0.0f || bounds.extent.y < 0.0f || bounds.extent.z < 0.0f))
			return false;

		m_count = 0;
		m_candidate_count = 0;
		const std::int32_t emitters_in_half = static_cast<std::int32_t>(std::floor(description.box_dimensions / description.emitter_spacing * 0.5f));
		const std::int32_t center_x = static_cast<std::int32_t>(std::floor(camera_position.x / description.emitter_spacing));
		const std::int32_t center_y = static_cast<std::int32_t>(std::floor(camera_position.y / description.emitter_spacing));
		const std::int32_t origin_x = center_x - emitters_in_half;
		const std::int32_t origin_y = center_y - emitters_in_half;
		const std::int32_t end_x = center_x + emitters_in_half;
		const std::int32_t end_y = center_y + emitters_in_half;
		const float snow_ceiling = camera_position.z + description.box_dimensions * 0.5f;
		const float height_traveled = time_seconds * description.fall_speed
			+ std::fmod(camera_position.z, description.box_dimensions);
		const float culling_radius = std::max(description.culling_radius, description.particle_size);

		for (std::int32_t y = origin_y; y < end_y && m_candidate_count < m_capacity; ++y) {
			for (std::int32_t x = origin_x; x < end_x && m_candidate_count < m_capacity; ++x) {
				++m_candidate_count;
				const std::uint32_t noise_x = Wrap_Index(x, description.noise_width);
				const std::uint32_t noise_y = Wrap_Index(y, description.noise_height);
				const std::size_t noise_offset = static_cast<std::size_t>(noise_x)
					+ static_cast<std::size_t>(noise_y) * description.noise_width;
				const float height = snow_ceiling - std::fmod(
					height_traveled + description.starting_heights[noise_offset], description.box_dimensions);
				const float position_x = x * description.emitter_spacing
					+ description.drift_amplitude * std::sin(height * description.frequency_scale_x + static_cast<float>(x));
				const float position_y = y * description.emitter_spacing
					+ description.drift_amplitude * std::sin(height * description.frequency_scale_y + static_cast<float>(y));
				if (!Passes_Culling(position_x, position_y, height, culling_radius, view, bounds))
					continue;

				m_position_x[m_count] = position_x;
				m_position_y[m_count] = position_y;
				m_position_z[m_count] = height;
				m_sizes[m_count] = description.particle_size;
				m_color_r[m_count] = description.color[0];
				m_color_g[m_count] = description.color[1];
				m_color_b[m_count] = description.color[2];
				m_color_a[m_count] = description.color[3];
				m_materials[m_count] = description.material;
				m_emitter_flags[m_count] = description.emitter_flags;
				++m_count;
			}
		}
		return true;
	}

	bool Append() noexcept
	{
		return m_renderer != nullptr && m_emitter.Is_Valid() && m_renderer->Append_Particles(m_emitter, Data());
	}

	ParticleData Data() const noexcept
	{
		return {
			{m_position_x.data(), m_count},
			{m_position_y.data(), m_count},
			{m_position_z.data(), m_count},
			{m_zero.data(), m_count},
			{m_zero.data(), m_count},
			{m_zero.data(), m_count},
			{m_one.data(), m_count},
			{m_sizes.data(), m_count},
			{m_color_r.data(), m_count},
			{m_color_g.data(), m_count},
			{m_color_b.data(), m_count},
			{m_color_a.data(), m_count},
			{},
			{m_materials.data(), m_count},
			{m_emitter_flags.data(), m_count},
			{},
			{}
		};
	}

	std::size_t Particle_Count() const noexcept
	{
		return m_count;
	}

	std::size_t Candidate_Count() const noexcept
	{
		return m_candidate_count;
	}

	ParticleEmitterHandle Emitter() const noexcept
	{
		return m_emitter;
	}

private:
	static constexpr std::int32_t Hash_Offset = 100000;

	static std::uint32_t Wrap_Index(std::int32_t coordinate, std::uint32_t dimension) noexcept
	{
		const std::int32_t shifted = coordinate + Hash_Offset;
		if ((dimension & (dimension - 1)) == 0)
			return static_cast<std::uint32_t>(shifted) & (dimension - 1);
		const std::int32_t remainder = shifted % static_cast<std::int32_t>(dimension);
		return static_cast<std::uint32_t>(remainder >= 0 ? remainder : remainder + static_cast<std::int32_t>(dimension));
	}

	static bool Passes_Plane(const FrustumPlane &plane, float x, float y, float z, float radius) noexcept
	{
		return plane.normal.x * x + plane.normal.y * y + plane.normal.z * z + plane.distance >= -radius;
	}

	static bool Passes_Frustum(float x, float y, float z, float radius, const Frustum &frustum) noexcept
	{
		return Passes_Plane(frustum.left, x, y, z, radius)
			&& Passes_Plane(frustum.right, x, y, z, radius)
			&& Passes_Plane(frustum.bottom, x, y, z, radius)
			&& Passes_Plane(frustum.top, x, y, z, radius)
			&& Passes_Plane(frustum.near_plane, x, y, z, radius)
			&& Passes_Plane(frustum.far_plane, x, y, z, radius);
	}

	static bool Passes_Culling(float x, float y, float z, float radius, const View &view,
		const WeatherParticleCullingBounds &bounds) noexcept
	{
		if (bounds.enabled && (std::fabs(x - bounds.center.x) > bounds.extent.x + radius
			|| std::fabs(y - bounds.center.y) > bounds.extent.y + radius
			|| std::fabs(z - bounds.center.z) > bounds.extent.z + radius))
			return false;
		return Passes_Frustum(x, y, z, radius, view.frustum);
	}

	ParticleRenderer *m_renderer = nullptr;
	ParticleEmitterHandle m_emitter{};
	ParticleEmitter m_emitter_state{};
	std::size_t m_capacity = 0;
	std::size_t m_count = 0;
	std::size_t m_candidate_count = 0;
	AlignedVector<float> m_position_x;
	AlignedVector<float> m_position_y;
	AlignedVector<float> m_position_z;
	AlignedVector<float> m_sizes;
	AlignedVector<float> m_color_r;
	AlignedVector<float> m_color_g;
	AlignedVector<float> m_color_b;
	AlignedVector<float> m_color_a;
	AlignedVector<float> m_zero;
	AlignedVector<float> m_one;
	std::vector<MaterialHandle> m_materials;
	std::vector<ParticleEmitterFlags> m_emitter_flags;
};

}
