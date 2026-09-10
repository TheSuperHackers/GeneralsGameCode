module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <utility>
#include <vector>

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif

export module Graphics.Scene.Particles;

export import Graphics.Resources.Handles.ResourceHandle;
export import Graphics.Shaders.Pipeline;
export import Graphics.Scene.Views.View;

import Graphics.Memory.AlignedAllocator;

namespace Graphics
{

export inline constexpr std::uint32_t Invalid_Particle_Index = std::numeric_limits<std::uint32_t>::max();
export inline constexpr std::uint32_t Invalid_Particle_Material_Index = std::numeric_limits<std::uint32_t>::max();

export enum class ParticleEmitterFlags : std::uint32_t
{
	None = 0,
	Enabled = 1u << 0,
	Billboard = 1u << 1,
	AlphaTest = 1u << 2,
	Additive = 1u << 3,
	Multiply = 1u << 4,
	PointSprite = 1u << 5
};

export constexpr ParticleEmitterFlags operator|(ParticleEmitterFlags left, ParticleEmitterFlags right) noexcept
{
	return static_cast<ParticleEmitterFlags>(static_cast<std::uint32_t>(left) | static_cast<std::uint32_t>(right));
}

export constexpr ParticleEmitterFlags operator&(ParticleEmitterFlags left, ParticleEmitterFlags right) noexcept
{
	return static_cast<ParticleEmitterFlags>(static_cast<std::uint32_t>(left) & static_cast<std::uint32_t>(right));
}

export constexpr bool Has_Particle_Emitter_Flag(ParticleEmitterFlags flags, ParticleEmitterFlags flag) noexcept
{
	return (flags & flag) == flag;
}

export struct ParticleEmitter final
{
	Vector3 position{};
	Vector3 velocity{};
	float particle_lifetime = 1.0f;
	float particle_size = 1.0f;
	std::array<float, 4> color{1.0f, 1.0f, 1.0f, 1.0f};
	MaterialHandle material{};
	ParticleEmitterFlags flags = ParticleEmitterFlags::Enabled;
	std::uint32_t max_particles = 1024;
	PipelineHandle pipeline{};
};

// Normalized texture rectangle: minimum U/V followed by maximum U/V.
export using ParticleTextureRegion = std::array<float, 4>;

export struct ParticleData final
{
	std::span<const float> position_x{};
	std::span<const float> position_y{};
	std::span<const float> position_z{};
	std::span<const float> velocity_x{};
	std::span<const float> velocity_y{};
	std::span<const float> velocity_z{};
	std::span<const float> lifetimes{};
	std::span<const float> sizes{};
	std::span<const float> color_r{};
	std::span<const float> color_g{};
	std::span<const float> color_b{};
	std::span<const float> color_a{};
	std::span<const float> angles{};
	std::span<const MaterialHandle> materials{};
	std::span<const ParticleEmitterFlags> emitter_flags{};
	std::span<const ParticleEmitterHandle> emitters{};
	std::span<const PipelineHandle> pipelines{};
	std::span<const ParticleTextureRegion> texture_regions{};

	std::size_t Size() const noexcept
	{
		return position_x.size();
	}
};

export struct alignas(16) GPUParticleData final
{
	std::array<float, 4> position_lifetime{};
	std::array<float, 4> velocity_size{};
	std::array<float, 4> color{};
	std::uint32_t material_index = Invalid_Particle_Material_Index;
	std::uint32_t emitter_flags = 0;
	float angle = 0.0f;
	std::uint32_t texture_index = Invalid_Particle_Material_Index;
	ParticleTextureRegion texture_region{0, 0, 1, 1};
};

static_assert(sizeof(GPUParticleData) == 80);

export struct ParticleVertex final
{
	float position[3]{};
	float color[4]{};
	float uv[2]{};
};

static_assert(sizeof(ParticleVertex) == 36);

export GPUParticleData Pack_GPU_Particle(const ParticleData &particles, std::size_t particle_index, std::uint32_t material_index) noexcept
{
	GPUParticleData data;
	data.position_lifetime = {
		particles.position_x[particle_index],
		particles.position_y[particle_index],
		particles.position_z[particle_index],
		particles.lifetimes[particle_index]
	};
	data.velocity_size = {
		particles.velocity_x[particle_index],
		particles.velocity_y[particle_index],
		particles.velocity_z[particle_index],
		particles.sizes[particle_index]
	};
	data.color = {
		particles.color_r[particle_index],
		particles.color_g[particle_index],
		particles.color_b[particle_index],
		particles.color_a[particle_index]
	};
	data.angle = particles.angles.empty() ? 0.0f : particles.angles[particle_index];
	if (!particles.texture_regions.empty())
		data.texture_region = particles.texture_regions[particle_index];
	data.material_index = material_index;
	data.emitter_flags = static_cast<std::uint32_t>(particles.emitter_flags[particle_index]);
	return data;
}

export PipelineDesc Make_Particle_Pipeline(PipelineDesc description) noexcept
{
	description.depth_test = true;
	description.depth_write = false;
	description.blend_mode = RHIBlendMode::Alpha;
	return description;
}

export class ParticleSystem final
{
public:
	void Reserve(std::size_t emitter_capacity, std::size_t particle_capacity)
	{
		m_emitters.reserve(emitter_capacity);
		m_emitter_particle_counts.reserve(emitter_capacity);
		m_emitter_dense_handles.reserve(emitter_capacity);
		m_emitter_slots.reserve(emitter_capacity);
		m_position_x.reserve(particle_capacity);
		m_position_y.reserve(particle_capacity);
		m_position_z.reserve(particle_capacity);
		m_velocity_x.reserve(particle_capacity);
		m_velocity_y.reserve(particle_capacity);
		m_velocity_z.reserve(particle_capacity);
		m_lifetimes.reserve(particle_capacity);
		m_sizes.reserve(particle_capacity);
		m_color_r.reserve(particle_capacity);
		m_color_g.reserve(particle_capacity);
		m_color_b.reserve(particle_capacity);
		m_color_a.reserve(particle_capacity);
		m_angles.reserve(particle_capacity);
		m_materials.reserve(particle_capacity);
		m_particle_emitter_flags.reserve(particle_capacity);
		m_particle_emitters.reserve(particle_capacity);
		m_particle_pipelines.reserve(particle_capacity);
		m_texture_regions.reserve(particle_capacity);
	}

	ParticleEmitterHandle Create_Emitter(const ParticleEmitter &emitter = {})
	{
		Ensure_Emitter_Capacity();
		const bool reuses_slot = m_emitter_free_head != Invalid_Particle_Index;
		const std::uint32_t slot_index = reuses_slot
			? m_emitter_free_head
			: static_cast<std::uint32_t>(m_emitter_slots.size());
		const std::uint32_t dense_index = static_cast<std::uint32_t>(m_emitters.size());

		m_emitters.push_back(emitter);
		m_emitter_particle_counts.push_back(0);
		if (reuses_slot) {
			EmitterSlot &slot = m_emitter_slots[slot_index];
			m_emitter_free_head = slot.next_free;
			slot.next_free = Invalid_Particle_Index;
			slot.dense_index = dense_index;
		} else {
			m_emitter_slots.push_back({dense_index, Invalid_Particle_Index, 1});
		}
		m_emitter_dense_handles.emplace_back(slot_index, m_emitter_slots[slot_index].generation);
		return ParticleEmitterHandle(slot_index, m_emitter_slots[slot_index].generation);
	}

	bool Destroy_Emitter(ParticleEmitterHandle handle) noexcept
	{
		if (!Is_Valid_Emitter_Handle(handle))
			return false;

		for (std::size_t particle_index = 0; particle_index < m_particle_emitters.size();) {
			if (m_particle_emitters[particle_index] == handle)
				Remove_Particle(particle_index);
			else
				++particle_index;
		}

		const std::uint32_t slot_index = handle.Get_Index();
		const std::uint32_t dense_index = m_emitter_slots[slot_index].dense_index;
		const std::uint32_t last_dense_index = static_cast<std::uint32_t>(m_emitters.size() - 1);
		if (dense_index != last_dense_index) {
			m_emitters[dense_index] = std::move(m_emitters[last_dense_index]);
			m_emitter_particle_counts[dense_index] = m_emitter_particle_counts[last_dense_index];
			m_emitter_dense_handles[dense_index] = m_emitter_dense_handles[last_dense_index];
			const std::uint32_t moved_slot_index = m_emitter_dense_handles[last_dense_index].Get_Index();
			m_emitter_slots[moved_slot_index].dense_index = dense_index;
		}

		m_emitters.pop_back();
		m_emitter_particle_counts.pop_back();
		m_emitter_dense_handles.pop_back();

		EmitterSlot &slot = m_emitter_slots[slot_index];
		slot.dense_index = Invalid_Particle_Index;
		slot.generation = Next_Generation(slot.generation);
		slot.next_free = m_emitter_free_head;
		m_emitter_free_head = slot_index;
		return true;
	}

	bool Update_Emitter(ParticleEmitterHandle handle, const ParticleEmitter &emitter) noexcept
	{
		if (!Is_Valid_Emitter_Handle(handle))
			return false;

		const std::uint32_t dense_index = m_emitter_slots[handle.Get_Index()].dense_index;
		m_emitters[dense_index] = emitter;
		for (std::size_t particle_index = 0; particle_index < m_particle_emitters.size(); ++particle_index) {
			if (m_particle_emitters[particle_index] != handle)
				continue;
			m_materials[particle_index] = emitter.material;
			m_particle_emitter_flags[particle_index] = emitter.flags;
			m_particle_pipelines[particle_index] = emitter.pipeline;
		}
		return true;
	}

	void Clear() noexcept
	{
		Clear_Particles();
		while (!m_emitter_dense_handles.empty())
			Destroy_Emitter(m_emitter_dense_handles.back());
	}

	void Clear_Particles() noexcept
	{
		m_position_x.clear();
		m_position_y.clear();
		m_position_z.clear();
		m_velocity_x.clear();
		m_velocity_y.clear();
		m_velocity_z.clear();
		m_lifetimes.clear();
		m_sizes.clear();
		m_color_r.clear();
		m_color_g.clear();
		m_color_b.clear();
		m_color_a.clear();
		m_angles.clear();
		m_materials.clear();
		m_particle_emitter_flags.clear();
		m_particle_emitters.clear();
		m_particle_pipelines.clear();
		m_texture_regions.clear();
		for (std::uint32_t &count : m_emitter_particle_counts)
			count = 0;
	}

	bool Append_Particles(ParticleEmitterHandle handle, const ParticleData &source) noexcept
	{
		if (!Is_Valid_Emitter_Handle(handle) || !Has_Consistent_Source_Size(source))
			return false;

		const std::size_t count = source.Size();
		if (!Can_Append(count))
			return false;

		const std::uint32_t dense_emitter_index = m_emitter_slots[handle.Get_Index()].dense_index;
		const std::size_t first_particle = m_position_x.size();
		const std::size_t new_size = first_particle + count;
		m_position_x.resize(new_size);
		m_position_y.resize(new_size);
		m_position_z.resize(new_size);
		m_velocity_x.resize(new_size);
		m_velocity_y.resize(new_size);
		m_velocity_z.resize(new_size);
		m_lifetimes.resize(new_size);
		m_sizes.resize(new_size);
		m_color_r.resize(new_size);
		m_color_g.resize(new_size);
		m_color_b.resize(new_size);
		m_color_a.resize(new_size);
		m_angles.resize(new_size);
		m_materials.resize(new_size);
		m_particle_emitter_flags.resize(new_size);
		m_particle_emitters.resize(new_size);
		m_particle_pipelines.resize(new_size);
		m_texture_regions.resize(new_size);

		for (std::size_t index = 0; index < count; ++index) {
			const std::size_t particle_index = first_particle + index;
			m_position_x[particle_index] = source.position_x[index];
			m_position_y[particle_index] = source.position_y[index];
			m_position_z[particle_index] = source.position_z[index];
			m_velocity_x[particle_index] = source.velocity_x[index];
			m_velocity_y[particle_index] = source.velocity_y[index];
			m_velocity_z[particle_index] = source.velocity_z[index];
			m_lifetimes[particle_index] = source.lifetimes[index];
			m_sizes[particle_index] = source.sizes[index];
			m_color_r[particle_index] = source.color_r[index];
			m_color_g[particle_index] = source.color_g[index];
			m_color_b[particle_index] = source.color_b[index];
			m_color_a[particle_index] = source.color_a[index];
			m_angles[particle_index] = source.angles.empty() ? 0.0f : source.angles[index];
			m_materials[particle_index] = source.materials[index];
			m_particle_emitter_flags[particle_index] = source.emitter_flags[index];
			m_particle_emitters[particle_index] = handle;
			m_particle_pipelines[particle_index] = source.pipelines.empty() ? m_emitters[dense_emitter_index].pipeline : source.pipelines[index];
			m_texture_regions[particle_index] = source.texture_regions.empty() ? ParticleTextureRegion{0, 0, 1, 1} : source.texture_regions[index];
		}
		m_emitter_particle_counts[dense_emitter_index] += static_cast<std::uint32_t>(count);
		return true;
	}

	bool Is_Emitter_Valid(ParticleEmitterHandle handle) const noexcept
	{
		return Is_Valid_Emitter_Handle(handle);
	}

	bool Spawn(ParticleEmitterHandle handle, std::uint32_t count) noexcept
	{
		if (!Is_Valid_Emitter_Handle(handle))
			return false;
		if (count == 0)
			return true;

		const std::uint32_t dense_emitter_index = m_emitter_slots[handle.Get_Index()].dense_index;
		const ParticleEmitter &emitter = m_emitters[dense_emitter_index];
		const std::size_t current_count = m_emitter_particle_counts[dense_emitter_index];
		if (current_count > emitter.max_particles || count > emitter.max_particles - current_count)
			return false;
		if (count > m_position_x.capacity() - m_position_x.size()
			|| count > m_position_y.capacity() - m_position_y.size()
			|| count > m_position_z.capacity() - m_position_z.size()
			|| count > m_velocity_x.capacity() - m_velocity_x.size()
			|| count > m_velocity_y.capacity() - m_velocity_y.size()
			|| count > m_velocity_z.capacity() - m_velocity_z.size()
			|| count > m_lifetimes.capacity() - m_lifetimes.size()
			|| count > m_sizes.capacity() - m_sizes.size()
			|| count > m_color_r.capacity() - m_color_r.size()
			|| count > m_color_g.capacity() - m_color_g.size()
			|| count > m_color_b.capacity() - m_color_b.size()
			|| count > m_color_a.capacity() - m_color_a.size()
			|| count > m_materials.capacity() - m_materials.size()
			|| count > m_particle_emitter_flags.capacity() - m_particle_emitter_flags.size()
			|| count > m_particle_emitters.capacity() - m_particle_emitters.size()
			|| count > m_particle_pipelines.capacity() - m_particle_pipelines.size()
			|| count > m_texture_regions.capacity() - m_texture_regions.size())
			return false;

		const std::size_t first_particle = m_position_x.size();
		const std::size_t new_size = first_particle + count;
		m_position_x.resize(new_size);
		m_position_y.resize(new_size);
		m_position_z.resize(new_size);
		m_velocity_x.resize(new_size);
		m_velocity_y.resize(new_size);
		m_velocity_z.resize(new_size);
		m_lifetimes.resize(new_size);
		m_sizes.resize(new_size);
		m_color_r.resize(new_size);
		m_color_g.resize(new_size);
		m_color_b.resize(new_size);
		m_color_a.resize(new_size);
		m_angles.resize(new_size);
		m_materials.resize(new_size);
		m_particle_emitter_flags.resize(new_size);
		m_particle_emitters.resize(new_size);
		m_particle_pipelines.resize(new_size);
		m_texture_regions.resize(new_size);

		for (std::uint32_t index = 0; index < count; ++index) {
			const std::size_t particle_index = first_particle + index;
			m_position_x[particle_index] = emitter.position.x;
			m_position_y[particle_index] = emitter.position.y;
			m_position_z[particle_index] = emitter.position.z;
			m_velocity_x[particle_index] = emitter.velocity.x;
			m_velocity_y[particle_index] = emitter.velocity.y;
			m_velocity_z[particle_index] = emitter.velocity.z;
			m_lifetimes[particle_index] = emitter.particle_lifetime;
			m_sizes[particle_index] = emitter.particle_size;
			m_color_r[particle_index] = emitter.color[0];
			m_color_g[particle_index] = emitter.color[1];
			m_color_b[particle_index] = emitter.color[2];
			m_color_a[particle_index] = emitter.color[3];
			m_angles[particle_index] = 0.0f;
			m_texture_regions[particle_index] = {0, 0, 1, 1};
			m_materials[particle_index] = emitter.material;
			m_particle_emitter_flags[particle_index] = emitter.flags;
			m_particle_emitters[particle_index] = handle;
			m_particle_pipelines[particle_index] = emitter.pipeline;
		}
		m_emitter_particle_counts[dense_emitter_index] += count;
		return true;
	}

	bool Update(float delta_seconds) noexcept
	{
		GRAPHICS_PROFILE_SCOPE("Graphics::ParticleSystem::Update");
		if (!(delta_seconds >= 0.0f) || !std::isfinite(delta_seconds))
			return false;

		for (std::size_t particle_index = 0; particle_index < m_lifetimes.size();) {
			m_lifetimes[particle_index] -= delta_seconds;
			if (m_lifetimes[particle_index] <= 0.0f) {
				Remove_Particle(particle_index);
				continue;
			}

			m_position_x[particle_index] += m_velocity_x[particle_index] * delta_seconds;
			m_position_y[particle_index] += m_velocity_y[particle_index] * delta_seconds;
			m_position_z[particle_index] += m_velocity_z[particle_index] * delta_seconds;
			++particle_index;
		}
		return true;
	}

	std::size_t Emitter_Count() const noexcept
	{
		return m_emitters.size();
	}

	std::size_t Particle_Count() const noexcept
	{
		return m_lifetimes.size();
	}

	std::size_t Particle_Count(ParticleEmitterHandle handle) const noexcept
	{
		if (!Is_Valid_Emitter_Handle(handle))
			return 0;
		return m_emitter_particle_counts[m_emitter_slots[handle.Get_Index()].dense_index];
	}

	ParticleData Particles() const noexcept
	{
		return {
			m_position_x,
			m_position_y,
			m_position_z,
			m_velocity_x,
			m_velocity_y,
			m_velocity_z,
			m_lifetimes,
			m_sizes,
			m_color_r,
			m_color_g,
			m_color_b,
			m_color_a,
			m_angles,
			m_materials,
			m_particle_emitter_flags,
			m_particle_emitters,
			m_particle_pipelines,
			m_texture_regions
		};
	}

private:
	struct EmitterSlot final
	{
		std::uint32_t dense_index = Invalid_Particle_Index;
		std::uint32_t next_free = Invalid_Particle_Index;
		std::uint32_t generation = 0;
	};

	static constexpr std::uint32_t Next_Generation(std::uint32_t generation) noexcept
	{
		const std::uint32_t next_generation = generation + 1;
		return next_generation == 0 ? 1 : next_generation;
	}

	static std::size_t Next_Capacity(std::size_t current_capacity) noexcept
	{
		return current_capacity == 0
			? 1
			: current_capacity > std::numeric_limits<std::size_t>::max() / 2
				? std::numeric_limits<std::size_t>::max()
				: current_capacity * 2;
	}

	void Ensure_Emitter_Capacity()
	{
		if (m_emitters.size() < m_emitters.capacity())
			return;

		Reserve(Next_Capacity(m_emitters.size()), m_position_x.capacity());
	}

	bool Is_Valid_Emitter_Handle(ParticleEmitterHandle handle) const noexcept
	{
		if (!handle.Is_Valid() || handle.Get_Index() >= m_emitter_slots.size())
			return false;

		const EmitterSlot &slot = m_emitter_slots[handle.Get_Index()];
		return slot.dense_index != Invalid_Particle_Index && slot.generation == handle.Get_Generation();
	}

	static bool Has_Consistent_Source_Size(const ParticleData &source) noexcept
	{
		const std::size_t count = source.position_x.size();
		return source.position_y.size() == count
			&& source.position_z.size() == count
			&& source.velocity_x.size() == count
			&& source.velocity_y.size() == count
			&& source.velocity_z.size() == count
			&& source.lifetimes.size() == count
			&& source.sizes.size() == count
			&& source.color_r.size() == count
			&& source.color_g.size() == count
			&& source.color_b.size() == count
			&& source.color_a.size() == count
			&& (source.angles.empty() || source.angles.size() == count)
			&& source.materials.size() == count
			&& source.emitter_flags.size() == count
			&& (source.pipelines.empty() || source.pipelines.size() == count)
			&& (source.texture_regions.empty() || source.texture_regions.size() == count);
	}

	bool Can_Append(std::size_t count) const noexcept
	{
		return count <= m_position_x.capacity() - m_position_x.size()
			&& count <= m_position_y.capacity() - m_position_y.size()
			&& count <= m_position_z.capacity() - m_position_z.size()
			&& count <= m_velocity_x.capacity() - m_velocity_x.size()
			&& count <= m_velocity_y.capacity() - m_velocity_y.size()
			&& count <= m_velocity_z.capacity() - m_velocity_z.size()
			&& count <= m_lifetimes.capacity() - m_lifetimes.size()
			&& count <= m_sizes.capacity() - m_sizes.size()
			&& count <= m_color_r.capacity() - m_color_r.size()
			&& count <= m_color_g.capacity() - m_color_g.size()
			&& count <= m_color_b.capacity() - m_color_b.size()
			&& count <= m_color_a.capacity() - m_color_a.size()
			&& count <= m_angles.capacity() - m_angles.size()
			&& count <= m_materials.capacity() - m_materials.size()
			&& count <= m_particle_emitter_flags.capacity() - m_particle_emitter_flags.size()
			&& count <= m_particle_emitters.capacity() - m_particle_emitters.size()
			&& count <= m_particle_pipelines.capacity() - m_particle_pipelines.size()
			&& count <= m_texture_regions.capacity() - m_texture_regions.size();
	}

	void Decrement_Emitter_Count(ParticleEmitterHandle handle) noexcept
	{
		if (!Is_Valid_Emitter_Handle(handle))
			return;

		const std::uint32_t dense_index = m_emitter_slots[handle.Get_Index()].dense_index;
		if (m_emitter_particle_counts[dense_index] != 0)
			--m_emitter_particle_counts[dense_index];
	}

	void Remove_Particle(std::size_t particle_index) noexcept
	{
		const ParticleEmitterHandle removed_emitter = m_particle_emitters[particle_index];
		Decrement_Emitter_Count(removed_emitter);
		const std::size_t last_particle_index = m_lifetimes.size() - 1;
		if (particle_index != last_particle_index) {
			m_position_x[particle_index] = m_position_x[last_particle_index];
			m_position_y[particle_index] = m_position_y[last_particle_index];
			m_position_z[particle_index] = m_position_z[last_particle_index];
			m_velocity_x[particle_index] = m_velocity_x[last_particle_index];
			m_velocity_y[particle_index] = m_velocity_y[last_particle_index];
			m_velocity_z[particle_index] = m_velocity_z[last_particle_index];
			m_lifetimes[particle_index] = m_lifetimes[last_particle_index];
			m_sizes[particle_index] = m_sizes[last_particle_index];
			m_color_r[particle_index] = m_color_r[last_particle_index];
			m_color_g[particle_index] = m_color_g[last_particle_index];
			m_color_b[particle_index] = m_color_b[last_particle_index];
			m_color_a[particle_index] = m_color_a[last_particle_index];
			m_angles[particle_index] = m_angles[last_particle_index];
			m_materials[particle_index] = m_materials[last_particle_index];
			m_particle_emitter_flags[particle_index] = m_particle_emitter_flags[last_particle_index];
			m_particle_emitters[particle_index] = m_particle_emitters[last_particle_index];
			m_particle_pipelines[particle_index] = m_particle_pipelines[last_particle_index];
			m_texture_regions[particle_index] = m_texture_regions[last_particle_index];
		}

		m_position_x.pop_back();
		m_position_y.pop_back();
		m_position_z.pop_back();
		m_velocity_x.pop_back();
		m_velocity_y.pop_back();
		m_velocity_z.pop_back();
		m_lifetimes.pop_back();
		m_sizes.pop_back();
		m_color_r.pop_back();
		m_color_g.pop_back();
		m_color_b.pop_back();
		m_color_a.pop_back();
		m_angles.pop_back();
		m_materials.pop_back();
		m_particle_emitter_flags.pop_back();
		m_particle_emitters.pop_back();
		m_particle_pipelines.pop_back();
		m_texture_regions.pop_back();
	}

	std::vector<ParticleEmitter> m_emitters;
	std::vector<std::uint32_t> m_emitter_particle_counts;
	std::vector<ParticleEmitterHandle> m_emitter_dense_handles;
	std::vector<EmitterSlot> m_emitter_slots;
	std::uint32_t m_emitter_free_head = Invalid_Particle_Index;

	AlignedVector<float> m_position_x;
	AlignedVector<float> m_position_y;
	AlignedVector<float> m_position_z;
	AlignedVector<float> m_velocity_x;
	AlignedVector<float> m_velocity_y;
	AlignedVector<float> m_velocity_z;
	AlignedVector<float> m_lifetimes;
	AlignedVector<float> m_sizes;
	AlignedVector<float> m_color_r;
	AlignedVector<float> m_color_g;
	AlignedVector<float> m_color_b;
	AlignedVector<float> m_color_a;
	AlignedVector<float> m_angles;
	AlignedVector<MaterialHandle> m_materials;
	AlignedVector<ParticleEmitterFlags> m_particle_emitter_flags;
	AlignedVector<ParticleEmitterHandle> m_particle_emitters;
	AlignedVector<PipelineHandle> m_particle_pipelines;
	AlignedVector<ParticleTextureRegion> m_texture_regions;
};

}
