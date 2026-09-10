module;

#include <array>
#include <cmath>
#include <cstddef>

export module Graphics.Scene.Views.CameraFrustum;

import Assets.Math;
export import Graphics.Scene.AffineTransform;
export import Graphics.Scene.Views.CameraProjection;

namespace Graphics
{

export struct CameraFrustumPlane final
{
	Vector3 normal{};
	float distance = 0.0f;
};

export struct CameraSphere final
{
	Vector3 center{};
	float radius = 0.0f;
};

export struct CameraAxisAlignedBox final
{
	Vector3 center{};
	Vector3 extent{};
};

// The basis is row-major and its columns are the box's local axes, matching
// RenderTransform and the native matrix types used by the game adapter.
export struct CameraOrientedBox final
{
	Vector3 center{};
	Vector3 extent{};
	std::array<float, 9> basis{
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f};
};

export struct CameraFrustum final
{
	// Plane 0: near, 1: bottom, 2: right, 3: top, 4: left, 5: far.
	std::array<CameraFrustumPlane, 6> planes{};
	// Corner 0: near upper left, 1: near upper right, 2: near lower left,
	// 3: near lower right. Corners 4..7 are the corresponding far corners.
	std::array<Vector3, 8> corners{};
	Vector3 bound_min{};
	Vector3 bound_max{};

	const CameraFrustumPlane &Plane(std::size_t index) const noexcept
	{
		return planes[index];
	}

	const Vector3 &Corner(std::size_t index) const noexcept
	{
		return corners[index];
	}

	bool Cull_Sphere(const CameraSphere &sphere) const noexcept
	{
		for (const CameraFrustumPlane &plane : planes) {
			if (Dot(plane.normal, sphere.center) - plane.distance > sphere.radius)
				return true;
		}
		return false;
	}

	bool Cull_Sphere_On_Frustum_Sides(const CameraSphere &sphere) const noexcept
	{
		for (std::size_t index = 1; index < 5; ++index) {
			const CameraFrustumPlane &plane = planes[index];
			if (Dot(plane.normal, sphere.center) - plane.distance > sphere.radius)
				return true;
		}
		return false;
	}

	bool Cull_Box(const CameraAxisAlignedBox &box) const noexcept
	{
		for (const CameraFrustumPlane &plane : planes) {
			const float projected_extent = std::fabs(plane.normal.x) * box.extent.x
				+ std::fabs(plane.normal.y) * box.extent.y
				+ std::fabs(plane.normal.z) * box.extent.z;
			if (Dot(plane.normal, box.center) - plane.distance - projected_extent
				> Camera_Projection_Epsilon)
				return true;
		}
		return false;
	}

	bool Cull_Oriented_Box(const CameraOrientedBox &box) const noexcept
	{
		for (const CameraFrustumPlane &plane : planes) {
			const float projected_extent = Oriented_Extent(plane.normal, box);
			if (Dot(plane.normal, box.center) - plane.distance - projected_extent
				> Camera_Projection_Epsilon)
				return true;
		}
		return false;
	}

private:
	static float Dot(const Vector3 &left, const Vector3 &right) noexcept
	{
		return left.x * right.x + left.y * right.y + left.z * right.z;
	}

	static float Oriented_Extent(const Vector3 &normal, const CameraOrientedBox &box) noexcept
	{
		const auto &basis = box.basis;
		return box.extent.x * std::fabs(normal.x * basis[0] + normal.y * basis[3] + normal.z * basis[6])
			+ box.extent.y * std::fabs(normal.x * basis[1] + normal.y * basis[4] + normal.z * basis[7])
			+ box.extent.z * std::fabs(normal.x * basis[2] + normal.y * basis[5] + normal.z * basis[8]);
	}
};

namespace
{

Vector3 Add(const Vector3 &left, const Vector3 &right) noexcept
{
	return {left.x + right.x, left.y + right.y, left.z + right.z};
}

Vector3 Subtract(const Vector3 &left, const Vector3 &right) noexcept
{
	return {left.x - right.x, left.y - right.y, left.z - right.z};
}

Vector3 Cross(const Vector3 &left, const Vector3 &right) noexcept
{
	return {
		left.y * right.z - left.z * right.y,
		left.z * right.x - left.x * right.z,
		left.x * right.y - left.y * right.x};
}

float Dot(const Vector3 &left, const Vector3 &right) noexcept
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

Vector3 Transform_Point(const RenderTransform &transform, const Vector3 &point) noexcept
{
	const auto &matrix = transform.matrix;
	return {
		matrix[0] * point.x + matrix[1] * point.y + matrix[2] * point.z + matrix[3],
		matrix[4] * point.x + matrix[5] * point.y + matrix[6] * point.z + matrix[7],
		matrix[8] * point.x + matrix[9] * point.y + matrix[10] * point.z + matrix[11]};
}

CameraFrustumPlane Make_Plane(const Vector3 &first, const Vector3 &second,
	const Vector3 &third) noexcept
{
	const Vector3 cross = Cross(Subtract(second, first), Subtract(third, first));
	if (cross.x == 0.0f && cross.y == 0.0f && cross.z == 0.0f)
		return {{0.0f, 0.0f, 1.0f}, 0.0f};
	const float length_squared = Dot(cross, cross);
	const float inverse_length = 1.0f / std::sqrt(length_squared);
	const Vector3 normal{
		cross.x * inverse_length,
		cross.y * inverse_length,
		cross.z * inverse_length};
	return {normal, Dot(normal, first)};
}

}

// znear and zfar are positive distances in the camera API. Signed values are
// also accepted so this remains directly compatible with FrustumClass::Init.
export CameraFrustum Build_Camera_Frustum(const RenderTransform &camera_transform,
	const CameraViewPlane &view_plane, float znear, float zfar) noexcept
{
	if (znear > 0.0f && zfar > 0.0f) {
		znear = -znear;
		zfar = -zfar;
	}

	std::array<Vector3, 8> local_corners{};
	const Vector3 x_axis{
		camera_transform.matrix[0], camera_transform.matrix[4], camera_transform.matrix[8]};
	const Vector3 y_axis{
		camera_transform.matrix[1], camera_transform.matrix[5], camera_transform.matrix[9]};
	const Vector3 z_axis{
		camera_transform.matrix[2], camera_transform.matrix[6], camera_transform.matrix[10]};
	const bool reflected = Dot(z_axis, Cross(x_axis, y_axis)) < 0.0f;

	const Assets::Vector2f upper_left = reflected
		? Assets::Vector2f{view_plane.max.x, view_plane.max.y}
		: Assets::Vector2f{view_plane.min.x, view_plane.max.y};
	const Assets::Vector2f upper_right = reflected
		? Assets::Vector2f{view_plane.min.x, view_plane.max.y}
		: Assets::Vector2f{view_plane.max.x, view_plane.max.y};
	const Assets::Vector2f lower_left = reflected
		? Assets::Vector2f{view_plane.max.x, view_plane.min.y}
		: Assets::Vector2f{view_plane.min.x, view_plane.min.y};
	const Assets::Vector2f lower_right = reflected
		? Assets::Vector2f{view_plane.min.x, view_plane.min.y}
		: Assets::Vector2f{view_plane.max.x, view_plane.min.y};

	local_corners[0] = {upper_left.x * znear, upper_left.y * znear, znear};
	local_corners[1] = {upper_right.x * znear, upper_right.y * znear, znear};
	local_corners[2] = {lower_left.x * znear, lower_left.y * znear, znear};
	local_corners[3] = {lower_right.x * znear, lower_right.y * znear, znear};
	local_corners[4] = {upper_left.x * zfar, upper_left.y * zfar, zfar};
	local_corners[5] = {upper_right.x * zfar, upper_right.y * zfar, zfar};
	local_corners[6] = {lower_left.x * zfar, lower_left.y * zfar, zfar};
	local_corners[7] = {lower_right.x * zfar, lower_right.y * zfar, zfar};

	CameraFrustum result{};
	for (std::size_t index = 0; index < local_corners.size(); ++index)
		result.corners[index] = Transform_Point(camera_transform, local_corners[index]);

	result.planes[0] = Make_Plane(result.corners[0], result.corners[3], result.corners[1]);
	result.planes[1] = Make_Plane(result.corners[0], result.corners[5], result.corners[4]);
	result.planes[2] = Make_Plane(result.corners[0], result.corners[6], result.corners[2]);
	result.planes[3] = Make_Plane(result.corners[2], result.corners[7], result.corners[3]);
	result.planes[4] = Make_Plane(result.corners[1], result.corners[7], result.corners[5]);
	result.planes[5] = Make_Plane(result.corners[4], result.corners[7], result.corners[6]);

	result.bound_min = result.bound_max = result.corners[0];
	for (std::size_t index = 1; index < result.corners.size(); ++index) {
		const Vector3 &corner = result.corners[index];
		if (corner.x < result.bound_min.x) result.bound_min.x = corner.x;
		if (corner.x > result.bound_max.x) result.bound_max.x = corner.x;
		if (corner.y < result.bound_min.y) result.bound_min.y = corner.y;
		if (corner.y > result.bound_max.y) result.bound_max.y = corner.y;
		if (corner.z < result.bound_min.z) result.bound_min.z = corner.z;
		if (corner.z > result.bound_max.z) result.bound_max.z = corner.z;
	}
	return result;
}

}
