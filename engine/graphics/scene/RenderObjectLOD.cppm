module;

#include <array>
#include <cmath>
#include <limits>
#include <span>

export module Graphics.Scene.RenderObjectLOD;

namespace Graphics
{

export inline constexpr float Render_Object_Min_LOD_Value =
	(std::numeric_limits<float>::max)();
export inline constexpr float Render_Object_Max_LOD_Value = -1.0f;

export constexpr float Base_Render_Object_Cost(float cost) noexcept
{
	return cost != 0.0f ? cost : 0.000001f;
}

// Project the object's bounding sphere using the view-plane convention used
// by the game-facing camera. The adapter supplies only camera and viewport
// values; the projection arithmetic stays in the graphics layer.
export float Project_Render_Object_Screen_Size(
	const std::array<float, 3> &camera_position,
	const std::array<float, 3> &bounds_center,
	float bounds_radius,
	float viewport_width,
	float viewport_height,
	float view_plane_width,
	float view_plane_height) noexcept
{
	const float offset_x = bounds_center[0] - camera_position[0];
	const float offset_y = bounds_center[1] - camera_position[1];
	const float offset_z = bounds_center[2] - camera_position[2];
	const float distance = std::sqrt(
		offset_x * offset_x + offset_y * offset_y + offset_z * offset_z);
	const float radius = distance != 0.0f ? bounds_radius / distance : 0.0f;
	const float width_factor = viewport_width / view_plane_width;
	const float height_factor = viewport_height / view_plane_height;
	return 3.141592654f * radius * radius * width_factor * height_factor;
}

// The base object has one LOD. Values has one sentinel slot in addition to
// the cost slot, matching the predictive LOD caller contract.
export int Calculate_Base_Render_Object_LOD(
	float cost,
	std::span<float> values,
	std::span<float> costs) noexcept
{
	if (values.size() < 2 || costs.empty())
		return 0;
	values[0] = Render_Object_Min_LOD_Value;
	values[1] = Render_Object_Max_LOD_Value;
	costs[0] = Base_Render_Object_Cost(cost);
	return 0;
}

}
