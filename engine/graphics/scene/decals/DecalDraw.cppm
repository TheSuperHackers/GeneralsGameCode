module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>

export module Graphics.Scene.DecalDraw;

export import Graphics.Scene.DecalVisibility;
export import Graphics.Scene.DrawGeneration;
export import Graphics.Scene.GPUScene;
export import Graphics.Shaders.Pipeline;

namespace Graphics
{

export class DecalDrawSet final
{
public:
	explicit DecalDrawSet(std::span<DecalDrawData> storage) noexcept
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

	std::span<const DecalDrawData> Records() const noexcept
	{
		return {m_storage.data(), m_count};
	}

private:
	friend bool Build_Decal_Draw_Data(const VisibleDecalSet &, const GPUScene &, DrawPass, DecalDrawSet &) noexcept;

	bool Try_Append(DecalDrawData data) noexcept
	{
		if (m_count >= m_storage.size())
			return false;

		m_storage[m_count++] = data;
		return true;
	}

	void Sort() noexcept
	{
		std::sort(m_storage.begin(), m_storage.begin() + m_count, [](const DecalDrawData &left, const DecalDrawData &right) noexcept {
			if (left.sort_key != right.sort_key)
				return left.sort_key < right.sort_key;
			if (left.pipeline.Get_Index() != right.pipeline.Get_Index())
				return left.pipeline.Get_Index() < right.pipeline.Get_Index();
			if (left.pipeline.Get_Generation() != right.pipeline.Get_Generation())
				return left.pipeline.Get_Generation() < right.pipeline.Get_Generation();
		if (left.material_index != right.material_index)
				return left.material_index < right.material_index;
			return left.decal_index < right.decal_index;
		});
	}

	std::span<DecalDrawData> m_storage;
	std::size_t m_count = 0;
};

export bool Build_Decal_Draw_Data(
	const VisibleDecalSet &visible_decals,
	const GPUScene &gpu_scene,
	DrawPass pass,
	DecalDrawSet &draw_set) noexcept;

export bool Build_Decal_Draw_Data(
	const VisibleDecalSet &visible_decals,
	const GPUScene &gpu_scene,
	DrawPass pass,
	DecalDrawSet &draw_set) noexcept
{
	draw_set.Clear();
	if (!pass.pipeline.Is_Valid())
		return false;

	const std::span<const GPUDecalData> decals = gpu_scene.Decals();
	for (const DecalHandle handle : visible_decals.Handles()) {
		const std::uint32_t decal_index = gpu_scene.Decal_Index(handle);
		if (decal_index == Invalid_GPU_Index || decal_index >= decals.size())
			continue;

		const GPUDecalData &decal = decals[decal_index];
		if ((decal.flags & static_cast<std::uint32_t>(RenderDecalFlags::Enabled)) == 0 || decal.material_index == Invalid_GPU_Index)
			continue;

		const DecalDrawData draw{
			decal_index,
			decal.material_index,
			pass.pipeline,
			(static_cast<std::uint64_t>(handle.Get_Index()) << 32) | handle.Get_Generation()
		};
		if (!draw_set.Try_Append(draw)) {
			draw_set.Clear();
			return false;
		}
	}

	draw_set.Sort();
	return true;
}

}
