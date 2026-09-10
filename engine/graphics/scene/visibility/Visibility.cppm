module;

#include <cstddef>
#include <span>

#if defined(RTS_PROFILE_TRACY)
#include <tracy/Tracy.hpp>
#define GRAPHICS_PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define GRAPHICS_PROFILE_SCOPE(name) ((void)0)
#endif

export module Graphics.Scene.Visibility;

export import Graphics.Scene.RenderScene;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export class VisibleSet;

export bool Build_Visible_Set(const RenderScene &scene, const View &view, VisibleSet &visible_set) noexcept;

export class VisibleSet final
{
public:
	explicit VisibleSet(std::span<InstanceHandle> storage) noexcept
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

	std::span<const InstanceHandle> Handles() const noexcept
	{
		return {m_storage.data(), m_count};
	}

private:
	friend bool Build_Visible_Set(const RenderScene &scene, const View &view, VisibleSet &visible_set) noexcept;

	bool Try_Append(InstanceHandle handle) noexcept
	{
		if (m_count >= m_storage.size())
			return false;

		m_storage[m_count++] = handle;
		return true;
	}

	std::span<InstanceHandle> m_storage;
	std::size_t m_count = 0;
};

namespace
{
bool Passes_Plane(const FrustumPlane &plane, float center_x, float center_y, float center_z, float radius) noexcept
{
	const float signed_distance = plane.normal.x * center_x + plane.normal.y * center_y + plane.normal.z * center_z + plane.distance;
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

export bool Build_Visible_Set(const RenderScene &scene, const View &view, VisibleSet &visible_set) noexcept
{
	GRAPHICS_PROFILE_SCOPE("Graphics::Build_Visible_Set");
	visible_set.Clear();
	const RenderSceneData scene_data = scene.Data();
	for (std::size_t dense_index = 0; dense_index < scene_data.Size(); ++dense_index) {
		if (Has_Render_Instance_Flag(scene_data.flags[dense_index], RenderInstanceFlags::Hidden))
			continue;

		if (!Is_Visible(
			view.frustum,
			scene_data.world_bounds.center_x[dense_index],
			scene_data.world_bounds.center_y[dense_index],
			scene_data.world_bounds.center_z[dense_index],
			scene_data.world_bounds.radii[dense_index]))
			continue;

		if (!visible_set.Try_Append(scene_data.handles[dense_index])) {
			visible_set.Clear();
			return false;
		}
	}

	return true;
}

}
