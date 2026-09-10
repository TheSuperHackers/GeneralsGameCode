module;

#include <cstddef>
#include <cstdint>
#include <span>

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif

export module Graphics.Scene.ParticleVisibility;

export import Graphics.Scene.Particles;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export class VisibleParticleSet;

export bool Build_Visible_Particles(const ParticleSystem &particles, const View &view, VisibleParticleSet &visible_particles) noexcept;

export class VisibleParticleSet final
{
public:
	explicit VisibleParticleSet(std::span<std::uint32_t> storage) noexcept
		: m_storage(storage)
	{
	}

	void Clear() noexcept
	{
		m_count = 0;
	}

	std::size_t Size() const noexcept
	{
		return m_count;
	}

	std::span<const std::uint32_t> Indices() const noexcept
	{
		return {m_storage.data(), m_count};
	}

private:
	friend bool Build_Visible_Particles(const ParticleSystem &, const View &, VisibleParticleSet &) noexcept;

	bool Try_Append(std::uint32_t particle_index) noexcept
	{
		if (m_count >= m_storage.size())
			return false;

		m_storage[m_count++] = particle_index;
		return true;
	}

	std::span<std::uint32_t> m_storage;
	std::size_t m_count = 0;
};

namespace
{
bool Passes_Plane(const FrustumPlane &plane, float x, float y, float z, float radius) noexcept
{
	const float signed_distance = plane.normal.x * x
		+ plane.normal.y * y
		+ plane.normal.z * z
		+ plane.distance;
	return signed_distance >= -radius;
}

bool Is_Visible(const Frustum &frustum, float x, float y, float z, float radius) noexcept
{
	const bool left = Passes_Plane(frustum.left, x, y, z, radius);
	const bool right = Passes_Plane(frustum.right, x, y, z, radius);
	const bool bottom = Passes_Plane(frustum.bottom, x, y, z, radius);
	const bool top = Passes_Plane(frustum.top, x, y, z, radius);
	const bool near_plane = Passes_Plane(frustum.near_plane, x, y, z, radius);
	const bool far_plane = Passes_Plane(frustum.far_plane, x, y, z, radius);
	return left & right & bottom & top & near_plane & far_plane;
}
}

export bool Build_Visible_Particles(const ParticleSystem &particles, const View &view, VisibleParticleSet &visible_particles) noexcept
{
	GRAPHICS_PROFILE_SCOPE("Graphics::Build_Visible_Particles");
	visible_particles.Clear();
	const ParticleData data = particles.Particles();
	for (std::size_t particle_index = 0; particle_index < data.Size(); ++particle_index) {
		if (!Has_Particle_Emitter_Flag(data.emitter_flags[particle_index], ParticleEmitterFlags::Enabled))
			continue;

		const float size = data.sizes[particle_index];
		// A rotated quad reaches beyond its half-width at the corners.
		const float radius = size > 0.0f ? size * 1.41421356237f : 0.0f;
		if (!Is_Visible(
			view.frustum,
			data.position_x[particle_index],
			data.position_y[particle_index],
			data.position_z[particle_index],
			radius))
			continue;

		if (!visible_particles.Try_Append(static_cast<std::uint32_t>(particle_index))) {
			visible_particles.Clear();
			return false;
		}
	}

	return true;
}

}
