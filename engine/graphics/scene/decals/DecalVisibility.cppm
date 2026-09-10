module;

#include <cstddef>
#include <span>

export module Graphics.Scene.DecalVisibility;

export import Graphics.Scene.Decals;
export import Graphics.Scene.RenderScene;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export class VisibleDecalSet;

export bool Build_Visible_Decals(const RenderScene &scene, const View &view, VisibleDecalSet &visible_decals) noexcept;

export class VisibleDecalSet final
{
public:
	explicit VisibleDecalSet(std::span<DecalHandle> storage) noexcept
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

	std::span<const DecalHandle> Handles() const noexcept
	{
		return {m_storage.data(), m_count};
	}

private:
	friend bool Build_Visible_Decals(const RenderScene &, const View &, VisibleDecalSet &) noexcept;

	bool Try_Append(DecalHandle handle) noexcept
	{
		if (m_count >= m_storage.size())
			return false;

		m_storage[m_count++] = handle;
		return true;
	}

	std::span<DecalHandle> m_storage;
	std::size_t m_count = 0;
};

namespace
{
bool Passes_Plane(const FrustumPlane &plane, float center_x, float center_y, float center_z, float radius) noexcept
{
	const float signed_distance = plane.normal.x * center_x
		+ plane.normal.y * center_y
		+ plane.normal.z * center_z
		+ plane.distance;
	return signed_distance >= -radius;
}

bool Is_Visible(const Frustum &frustum, float center_x, float center_y, float center_z, float radius) noexcept
{
	const bool left = Passes_Plane(frustum.left, center_x, center_y, center_z, radius);
	const bool right = Passes_Plane(frustum.right, center_x, center_y, center_z, radius);
	const bool bottom = Passes_Plane(frustum.bottom, center_x, center_y, center_z, radius);
	const bool top = Passes_Plane(frustum.top, center_x, center_y, center_z, radius);
	const bool near_plane = Passes_Plane(frustum.near_plane, center_x, center_y, center_z, radius);
	const bool far_plane = Passes_Plane(frustum.far_plane, center_x, center_y, center_z, radius);
	return left & right & bottom & top & near_plane & far_plane;
}
}

export bool Build_Visible_Decals(const RenderScene &scene, const View &view, VisibleDecalSet &visible_decals) noexcept
{
	visible_decals.Clear();
	const RenderDecalData decals = scene.Decals();
	for (std::size_t dense_index = 0; dense_index < decals.Size(); ++dense_index) {
		if (!Has_Render_Decal_Flag(decals.flags[dense_index], RenderDecalFlags::Enabled))
			continue;
		if (!Is_Visible(
			view.frustum,
			decals.bounds.center_x[dense_index],
			decals.bounds.center_y[dense_index],
			decals.bounds.center_z[dense_index],
			decals.bounds.radii[dense_index]))
			continue;
		if (!visible_decals.Try_Append(decals.handles[dense_index])) {
			visible_decals.Clear();
			return false;
		}
	}

	return true;
}

}
