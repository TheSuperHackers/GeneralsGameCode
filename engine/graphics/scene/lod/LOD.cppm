module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif

export module Graphics.Scene.LOD;

export import Graphics.Resources.Meshes.Mesh;
export import Graphics.Scene.RenderScene;
export import Graphics.Scene.Visibility;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export struct LODSelection final
{
	InstanceHandle instance{};
	MeshHandle mesh{};
	std::uint32_t lod_index = 0;
};

export struct LODSelectionOptions final
{
	float hysteresis = 0.0f;
};

export struct LODHistoryEntry final
{
	InstanceHandle instance{};
	std::uint32_t lod_index = 0;
};

export class LODHistory final
{
public:
	explicit LODHistory(std::span<LODHistoryEntry> storage) noexcept
		: m_storage(storage)
	{
	}

	void Clear() noexcept
	{
		for (LODHistoryEntry &entry : m_storage)
			entry = {};
	}

	bool Previous(InstanceHandle instance, std::uint32_t &lod_index) const noexcept
	{
		if (!instance.Is_Valid() || instance.Get_Index() >= m_storage.size())
			return false;

		const LODHistoryEntry &entry = m_storage[instance.Get_Index()];
		if (entry.instance != instance)
			return false;

		lod_index = entry.lod_index;
		return true;
	}

	void Record(InstanceHandle instance, std::uint32_t lod_index) noexcept
	{
		if (instance.Is_Valid() && instance.Get_Index() < m_storage.size())
			m_storage[instance.Get_Index()] = {instance, lod_index};
	}

private:
	std::span<LODHistoryEntry> m_storage;
};

export class LODSet;

export bool Build_LOD_Set(const RenderScene &scene, const MeshPool &meshes, const VisibleSet &visible_set, const View &view, LODSet &lod_set) noexcept;
export bool Build_LOD_Set(const RenderScene &scene, const MeshPool &meshes, const VisibleSet &visible_set, const View &view,
	LODSet &lod_set, LODHistory &history, LODSelectionOptions options = {}) noexcept;

export class LODSet final
{
public:
	explicit LODSet(std::span<LODSelection> storage) noexcept
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

	std::span<const LODSelection> Selections() const noexcept
	{
		return {m_storage.data(), m_count};
	}

private:
	friend bool Build_LOD_Set(const RenderScene &scene, const MeshPool &meshes, const VisibleSet &visible_set, const View &view, LODSet &lod_set) noexcept;
	friend bool Build_LOD_Set(const RenderScene &scene, const MeshPool &meshes, const VisibleSet &visible_set, const View &view,
		LODSet &lod_set, LODHistory &history, LODSelectionOptions options) noexcept;

	bool Try_Append(LODSelection selection) noexcept
	{
		if (m_count >= m_storage.size())
			return false;

		m_storage[m_count++] = selection;
		return true;
	}

	std::span<LODSelection> m_storage;
	std::size_t m_count = 0;
};

namespace
{
bool Project_Y(const Matrix4x4 &projection_matrix, float view_x, float view_y, float view_z, float &projected_y) noexcept
{
	const float clip_y = projection_matrix(1, 0) * view_x
		+ projection_matrix(1, 1) * view_y
		+ projection_matrix(1, 2) * view_z
		+ projection_matrix(1, 3);
	const float clip_w = projection_matrix(3, 0) * view_x
		+ projection_matrix(3, 1) * view_y
		+ projection_matrix(3, 2) * view_z
		+ projection_matrix(3, 3);
	if (clip_w == 0.0f)
		return false;

	projected_y = clip_y / clip_w;
	return true;
}

float Projected_Screen_Size(const View &view, const RenderWorldBoundsData &bounds, std::size_t dense_index) noexcept
{
	const float world_x = bounds.center_x[dense_index];
	const float world_y = bounds.center_y[dense_index];
	const float world_z = bounds.center_z[dense_index];
	const float world_radius = bounds.radii[dense_index];
	const float view_x = view.view_matrix(0, 0) * world_x
		+ view.view_matrix(0, 1) * world_y
		+ view.view_matrix(0, 2) * world_z
		+ view.view_matrix(0, 3);
	const float view_y = view.view_matrix(1, 0) * world_x
		+ view.view_matrix(1, 1) * world_y
		+ view.view_matrix(1, 2) * world_z
		+ view.view_matrix(1, 3);
	const float view_z = view.view_matrix(2, 0) * world_x
		+ view.view_matrix(2, 1) * world_y
		+ view.view_matrix(2, 2) * world_z
		+ view.view_matrix(2, 3);

	float projected_top = 0.0f;
	float projected_bottom = 0.0f;
	if (!Project_Y(view.projection_matrix, view_x, view_y + world_radius, view_z, projected_top)
		|| !Project_Y(view.projection_matrix, view_x, view_y - world_radius, view_z, projected_bottom))
		return std::numeric_limits<float>::max();

	return std::abs(projected_top - projected_bottom);
}

std::uint32_t Nominal_LOD_Index(const Mesh &mesh, const MeshPool &meshes, float screen_size) noexcept
{
	const std::uint32_t lod_count = mesh.lod_count < Mesh::MaxLodCount ? mesh.lod_count : Mesh::MaxLodCount;
	std::uint32_t selected_lod = 0;
	for (std::uint32_t lod_index = 0; lod_index < lod_count; ++lod_index) {
		const MeshLod &lod = mesh.lods[lod_index];
		if (std::isnan(lod.max_screen_size) || lod.max_screen_size < 0.0f)
			break;
		if (screen_size > lod.max_screen_size)
			break;
		if (lod.mesh.Is_Valid() && meshes.Resolve(lod.mesh) != nullptr) {
			selected_lod = lod_index + 1;
		}
	}
	return selected_lod;
}

std::uint32_t Select_LOD_Index(const Mesh &mesh, const MeshPool &meshes, float screen_size,
	std::uint32_t previous_lod, bool has_previous, LODSelectionOptions options) noexcept
{
	const std::uint32_t lod_count = mesh.lod_count < Mesh::MaxLodCount ? mesh.lod_count : Mesh::MaxLodCount;
	const std::uint32_t total_lod_count = lod_count + 1;
	const std::uint32_t selected_lod = Nominal_LOD_Index(mesh, meshes, screen_size);
	if (!has_previous || previous_lod >= total_lod_count || previous_lod == selected_lod)
		return selected_lod;

	float hysteresis = options.hysteresis;
	if (!std::isfinite(hysteresis) || hysteresis < 0.0f)
		hysteresis = 0.0f;
	hysteresis = std::min(hysteresis, 0.99f);

	if (selected_lod > previous_lod) {
		const float threshold = mesh.lods[previous_lod].max_screen_size;
		if (std::isfinite(threshold) && screen_size > threshold * (1.0f - hysteresis))
			return previous_lod;
	} else {
		const float threshold = mesh.lods[selected_lod].max_screen_size;
		if (std::isfinite(threshold) && screen_size <= threshold * (1.0f + hysteresis))
			return previous_lod;
	}

	return selected_lod;
}

LODSelection Make_Selection(InstanceHandle instance_handle, MeshHandle instance_mesh, const MeshPool &meshes,
	const Mesh &mesh, float screen_size, LODHistory *history, LODSelectionOptions options) noexcept
{
	std::uint32_t previous_lod = 0;
	const bool has_previous = history != nullptr && history->Previous(instance_handle, previous_lod);
	const std::uint32_t lod_index = Select_LOD_Index(mesh, meshes, screen_size, previous_lod, has_previous, options);
	LODSelection selection{instance_handle, instance_mesh, 0};
	if (lod_index != 0) {
		selection.mesh = mesh.lods[lod_index - 1].mesh;
		selection.lod_index = lod_index;
	}

	return selection;
}
}

export bool Build_LOD_Set(const RenderScene &scene, const MeshPool &meshes, const VisibleSet &visible_set, const View &view, LODSet &lod_set) noexcept
{
	LODHistory history(std::span<LODHistoryEntry>{});
	return Build_LOD_Set(scene, meshes, visible_set, view, lod_set, history, {});
}

export bool Build_LOD_Set(const RenderScene &scene, const MeshPool &meshes, const VisibleSet &visible_set, const View &view,
	LODSet &lod_set, LODHistory &history, LODSelectionOptions options) noexcept
{
	GRAPHICS_PROFILE_SCOPE("Graphics::Build_LOD_Set");
	lod_set.Clear();
	const RenderSceneData scene_data = scene.Data();

	for (const InstanceHandle instance_handle : visible_set.Handles()) {
		const std::uint32_t dense_index = scene.Dense_Index(instance_handle);
		if (dense_index == Invalid_Render_Scene_Index || dense_index >= scene_data.Size())
			continue;

		const MeshHandle instance_mesh = scene_data.meshes[dense_index];
		const Mesh *mesh = meshes.Resolve(instance_mesh);
		if (mesh == nullptr)
			continue;

		const float screen_size = Projected_Screen_Size(view, scene_data.world_bounds, dense_index);
		if (!lod_set.Try_Append(Make_Selection(instance_handle, instance_mesh, meshes, *mesh, screen_size, &history, options))) {
			lod_set.Clear();
			return false;
		}
	}

	for (const LODSelection &selection : lod_set.Selections())
		history.Record(selection.instance, selection.lod_index);

	return true;
}

}
