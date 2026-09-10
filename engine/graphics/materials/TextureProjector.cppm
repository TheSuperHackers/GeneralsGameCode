module;
#include <cmath>
#include <cstddef>
#include <utility>

export module Graphics.Materials.TextureProjector;
import Graphics.Scene.Views.View;

namespace Graphics
{
namespace
{
float Euclidean_Length(const Vector3 &value) noexcept
{
	return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

float Quick_Length(const Vector3 &value) noexcept
{
	float max = std::fabs(value.x);
	float mid = std::fabs(value.y);
	float min = std::fabs(value.z);
	if (max < mid) std::swap(max, mid);
	if (max < min) std::swap(max, min);
	if (mid < min) std::swap(mid, min);
	return max + (11.0f / 32.0f) * mid + 0.25f * min;
}

Vector3 Add(const Vector3 &left, const Vector3 &right) noexcept
{
	return {left.x + right.x, left.y + right.y, left.z + right.z};
}

Vector3 Subtract(const Vector3 &left, const Vector3 &right) noexcept
{
	return {left.x - right.x, left.y - right.y, left.z - right.z};
}

Vector3 Scale(const Vector3 &value, float scale) noexcept
{
	return {value.x * scale, value.y * scale, value.z * scale};
}

Vector3 Transform_Point(const Matrix4x4 &matrix, const Vector3 &point) noexcept
{
	return {
		matrix.values[0] * point.x + matrix.values[1] * point.y + matrix.values[2] * point.z + matrix.values[3],
		matrix.values[4] * point.x + matrix.values[5] * point.y + matrix.values[6] * point.z + matrix.values[7],
		matrix.values[8] * point.x + matrix.values[9] * point.y + matrix.values[10] * point.z + matrix.values[11]};
}

Vector3 Transform_Extent(const Matrix4x4 &matrix, const Vector3 &extent) noexcept
{
	return {
		std::fabs(matrix.values[0]) * extent.x + std::fabs(matrix.values[1]) * extent.y
			+ std::fabs(matrix.values[2]) * extent.z,
		std::fabs(matrix.values[4]) * extent.x + std::fabs(matrix.values[5]) * extent.y
			+ std::fabs(matrix.values[6]) * extent.z,
		std::fabs(matrix.values[8]) * extent.x + std::fabs(matrix.values[9]) * extent.y
			+ std::fabs(matrix.values[10]) * extent.z};
}

Matrix4x4 Orthogonal_Inverse(const Matrix4x4 &matrix) noexcept
{
	Matrix4x4 result = Matrix4x4::Identity();
	for (std::size_t row = 0; row < 3; ++row)
		for (std::size_t column = 0; column < 3; ++column)
			result.values[row * 4 + column] = matrix.values[column * 4 + row];
	const Vector3 translation{matrix.values[3], matrix.values[7], matrix.values[11]};
	const Vector3 rotated = Transform_Point(result, translation);
	result.values[3] = -rotated.x;
	result.values[7] = -rotated.y;
	result.values[11] = -rotated.z;
	return result;
}

Matrix4x4 Make_Camera_Transform(const Vector3 &position, const Vector3 &direction) noexcept
{
	// This is the camera-to-world convention used by the game math library:
	// the camera looks down -Z and keeps world Z as its up reference.
	const float xy_length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
	const float sine_yaw = xy_length != 0.0f ? direction.y / xy_length : 0.0f;
	const float cosine_yaw = xy_length != 0.0f ? direction.x / xy_length : 1.0f;
	const float sine_pitch = direction.z;
	const float cosine_pitch = xy_length;
	Matrix4x4 result = Matrix4x4::Identity();
	result.values = {
		sine_yaw, -sine_pitch * cosine_yaw, -cosine_pitch * cosine_yaw, position.x,
		-cosine_yaw, -sine_pitch * sine_yaw, -cosine_pitch * sine_yaw, position.y,
		0.0f, cosine_pitch, -sine_pitch, position.z,
		0.0f, 0.0f, 0.0f, 1.0f};
	return result;
}

Matrix4x4 Make_Perspective(float horizontal_fov, float vertical_fov,
	float clip_start, float clip_end) noexcept
{
	Matrix4x4 result = Matrix4x4::Identity();
	result.values[0] = static_cast<float>(1.0 / std::tan(horizontal_fov * 0.5));
	result.values[5] = static_cast<float>(1.0 / std::tan(vertical_fov * 0.5));
	result.values[10] = -(clip_end + clip_start) / (clip_end - clip_start);
	result.values[11] = static_cast<float>(-(2.0 * clip_end * clip_start)
		/ (clip_end - clip_start));
	result.values[14] = -1.0f;
	result.values[15] = 0.0f;
	return result;
}

bool Finite(const Vector3 &value) noexcept
{
	return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}
}

export struct TextureProjectorBounds final
{
	Vector3 center{};
	Vector3 extent{};
};

export struct TextureProjectorFit final
{
	bool valid = false;
	Matrix4x4 camera_transform = Matrix4x4::Identity();
	Matrix4x4 projection = Matrix4x4::Identity();
	float horizontal_fov = 0.0f;
	float vertical_fov = 0.0f;
	float fitting_clip_start = 0.0f;
	float fitting_clip_end = 0.0f;
};

// Fit a perspective projector around an object-space axis-aligned box. The
// returned transform is camera-to-world, matching the game camera adapter.
// The projection matrix intentionally uses a fixed 0.1 clip start. The fit
// clip values remain available for the camera's culling volume independently.
export TextureProjectorFit Fit_Perspective_Texture_Projector(
	const TextureProjectorBounds &bounds, const Matrix4x4 &object_transform,
	const Vector3 &light_position) noexcept
{
	TextureProjectorFit result;
	const Vector3 world_center = Transform_Point(object_transform, bounds.center);
	Vector3 direction = Subtract(world_center, light_position);
	const float direction_length = Euclidean_Length(direction);
	if (!(direction_length > 0.0f) || !std::isfinite(direction_length)) return result;
	direction = Scale(direction, 1.0f / direction_length);

	result.camera_transform = Make_Camera_Transform(light_position, direction);
	const Matrix4x4 world_to_camera = Orthogonal_Inverse(result.camera_transform);
	const Matrix4x4 object_to_camera = Compose_Matrices(world_to_camera, object_transform);
	const Vector3 camera_center = Transform_Point(object_to_camera, bounds.center);
	const Vector3 camera_extent = Transform_Extent(object_to_camera, bounds.extent);
	if (!Finite(camera_center) || !Finite(camera_extent)
		|| camera_center.z > 0.0f || camera_extent.z > std::fabs(camera_center.z)) return result;

	const float object_extent_length = Quick_Length(bounds.extent);
	const float fitting_clip_start = -camera_center.z;
	const float fitting_clip_end = -(camera_center.z - object_extent_length) * 2.0f;
	const float horizontal_denominator = camera_center.z + camera_extent.z;
	const float vertical_denominator = camera_center.z + camera_extent.z;
	if (!(fitting_clip_start > 0.0f) || !(fitting_clip_end > 0.1f)
		|| horizontal_denominator == 0.0f || vertical_denominator == 0.0f) return result;

	result.horizontal_fov = 2.0f * std::atan(std::fabs(camera_extent.x / horizontal_denominator));
	result.vertical_fov = 2.0f * std::atan(std::fabs(camera_extent.y / vertical_denominator));
	if (!(result.horizontal_fov > 0.0f) || !(result.vertical_fov > 0.0f)
		|| !std::isfinite(result.horizontal_fov) || !std::isfinite(result.vertical_fov)) return result;

	result.fitting_clip_start = fitting_clip_start;
	result.fitting_clip_end = fitting_clip_end;
	result.projection = Make_Perspective(result.horizontal_fov, result.vertical_fov,
		0.1f, fitting_clip_end);
	result.valid = true;
	return result;
}

// Compose the projector's clip-space matrix with a camera-to-world view
// transform for use by TextureProjectionState.
export Matrix4x4 Make_Texture_Projector_View_Transform(
	const TextureProjectorFit &fit, const Matrix4x4 &camera_transform) noexcept
{
	if (!fit.valid) return Matrix4x4::Identity();
	const Matrix4x4 world_to_projector = Orthogonal_Inverse(fit.camera_transform);
	return Compose_Matrices(fit.projection,
		Compose_Matrices(world_to_projector, camera_transform));
}
}
