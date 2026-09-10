module;

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cmath>
#include <limits>
#include <span>

export module Graphics.Scene.ParticleDraw;

export import Graphics.Scene.DrawGeneration;
export import Graphics.Scene.ParticleVisibility;
export import Graphics.Scene.Particles;

namespace Graphics
{

export struct alignas(16) ParticleDrawData final
{
	std::uint32_t particle_index = Invalid_Particle_Index;
	std::uint32_t material_index = Invalid_Particle_Material_Index;
	PipelineHandle pipeline{};
	std::uint64_t sort_key = 0;
	bool point_sprite = false;
};

static_assert(sizeof(ParticleDrawData) == 32);

export class ParticleDrawSet final
{
public:
	explicit ParticleDrawSet(std::span<ParticleDrawData> storage) noexcept
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

	std::span<const ParticleDrawData> Records() const noexcept
	{
		return {m_storage.data(), m_count};
	}

private:
	friend bool Build_Particle_Draw_Data(const VisibleParticleSet &, const ParticleSystem &, const View &, const GPUScene &, DrawPass, ParticleDrawSet &) noexcept;

	bool Try_Append(ParticleDrawData data) noexcept
	{
		if (m_count >= m_storage.size())
			return false;

		m_storage[m_count++] = data;
		return true;
	}

	void Sort() noexcept
	{
		std::sort(m_storage.begin(), m_storage.begin() + m_count, [](const ParticleDrawData &left, const ParticleDrawData &right) noexcept {
			if (left.sort_key != right.sort_key)
				return left.sort_key < right.sort_key;
			if (left.pipeline.Get_Index() != right.pipeline.Get_Index())
				return left.pipeline.Get_Index() < right.pipeline.Get_Index();
			if (left.pipeline.Get_Generation() != right.pipeline.Get_Generation())
				return left.pipeline.Get_Generation() < right.pipeline.Get_Generation();
			if (left.material_index != right.material_index)
				return left.material_index < right.material_index;
			return left.particle_index < right.particle_index;
		});
	}

	std::span<ParticleDrawData> m_storage;
	std::size_t m_count = 0;
};

export bool Build_Particle_Draw_Data(
	const VisibleParticleSet &visible_particles,
	const ParticleSystem &particles,
	const View &view,
	const GPUScene &gpu_scene,
	DrawPass pass,
	ParticleDrawSet &draw_set) noexcept;

namespace
{
std::uint32_t Depth_Sort_Key(const View &view, const ParticleData &particles, std::size_t particle_index) noexcept
{
	const float view_z = view.view_matrix(2, 0) * particles.position_x[particle_index]
		+ view.view_matrix(2, 1) * particles.position_y[particle_index]
		+ view.view_matrix(2, 2) * particles.position_z[particle_index]
		+ view.view_matrix(2, 3);
	const float depth = -view_z;
	if (!std::isfinite(depth) || depth <= 0.0f)
		return std::numeric_limits<std::uint32_t>::max();

	return std::numeric_limits<std::uint32_t>::max() - std::bit_cast<std::uint32_t>(depth);
}

std::uint64_t Make_Sort_Key(const View &view, const ParticleData &particles, std::size_t particle_index, std::uint64_t pass_sort_key) noexcept
{
	return (static_cast<std::uint64_t>(Depth_Sort_Key(view, particles, particle_index)) << 32)
		| static_cast<std::uint32_t>(pass_sort_key);
}
}

export bool Build_Particle_Draw_Data(
	const VisibleParticleSet &visible_particles,
	const ParticleSystem &particles,
	const View &view,
	const GPUScene &gpu_scene,
	DrawPass pass,
	ParticleDrawSet &draw_set) noexcept
{
	draw_set.Clear();
	if (!pass.pipeline.Is_Valid())
		return false;

	const ParticleData data = particles.Particles();
	const std::span<const GPUMaterialData> materials = gpu_scene.Materials();
	for (const std::uint32_t particle_index : visible_particles.Indices()) {
		if (particle_index >= data.Size() || !Has_Particle_Emitter_Flag(data.emitter_flags[particle_index], ParticleEmitterFlags::Enabled))
			continue;
		const PipelineHandle particle_pipeline = data.pipelines.empty() || !data.pipelines[particle_index].Is_Valid()
			? pass.pipeline
			: data.pipelines[particle_index];
		if (!particle_pipeline.Is_Valid())
			continue;

		const std::uint32_t material_index = gpu_scene.Material_Index(data.materials[particle_index]);
		if (material_index == Invalid_GPU_Index || material_index >= materials.size())
			continue;

		if (!draw_set.Try_Append({
			particle_index,
			material_index,
			particle_pipeline,
			Make_Sort_Key(view, data, particle_index, pass.sort_key),
			Has_Particle_Emitter_Flag(data.emitter_flags[particle_index], ParticleEmitterFlags::PointSprite)
		})) {
			draw_set.Clear();
			return false;
		}
	}

	draw_set.Sort();
	return true;
}

}
