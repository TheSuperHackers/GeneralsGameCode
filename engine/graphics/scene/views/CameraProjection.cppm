module;

#include <array>
#include <cmath>
#include <cstdint>

export module Graphics.Scene.Views.CameraProjection;

export import Assets.Math;
export import Graphics.Scene.Views.View;

namespace Graphics
{

export struct CameraViewPlane final
{
	Assets::Vector2f min{};
	Assets::Vector2f max{};

	float Width() const noexcept { return max.x - min.x; }
	float Height() const noexcept { return max.y - min.y; }
};

// A camera viewport is expressed as a normalized rectangle. Device adapters
// turn it into an API viewport only when a target is bound.
export struct CameraViewport final
{
	Assets::Vector2f min{0.0f, 0.0f};
	Assets::Vector2f max{1.0f, 1.0f};

	float Width() const noexcept { return max.x - min.x; }
	float Height() const noexcept { return max.y - min.y; }
};

export enum class CameraProjectionType : std::uint8_t
{
	Perspective = 0,
	Ortho = 1
};

export enum class CameraProjectionResult : std::uint8_t
{
	InsideFrustum,
	OutsideFrustum,
	OutsideNearClip,
	OutsideFarClip
};

export inline constexpr float Camera_Projection_Epsilon = 0.0001f;

export class CameraProjection final
{
public:
	CameraProjection() noexcept
	{
		Set_View_Plane(0.87266463f);
	}

	void Set_Projection_Type(CameraProjectionType type) noexcept
	{
		m_type = type;
		Invalidate();
	}

	CameraProjectionType Get_Projection_Type() const noexcept
	{
		return m_type;
	}

	void Set_Clip_Planes(float near_clip, float far_clip) noexcept
	{
		m_near_clip = near_clip;
		m_far_clip = far_clip;
		Invalidate();
	}

	void Get_Clip_Planes(float &near_clip, float &far_clip) const noexcept
	{
		near_clip = m_near_clip;
		far_clip = m_far_clip;
	}

	float Get_Near_Clip() const noexcept { return m_near_clip; }
	float Get_Far_Clip() const noexcept { return m_far_clip; }

	void Set_View_Plane(Assets::Vector2f minimum, Assets::Vector2f maximum) noexcept
	{
		m_view_plane.min = minimum;
		m_view_plane.max = maximum;
		m_aspect_ratio = m_view_plane.Width() / m_view_plane.Height();
		Invalidate();
	}

	void Set_View_Plane(float horizontal_fov, float vertical_fov = -1.0f) noexcept
	{
		const float width_half = std::tan(horizontal_fov * 0.5f);
		const float height_half = vertical_fov == -1.0f
			? (1.0f / m_aspect_ratio) * width_half
			: std::tan(vertical_fov * 0.5f);

		if (vertical_fov != -1.0f)
			m_aspect_ratio = width_half / height_half;

		m_view_plane.min = {-width_half, -height_half};
		m_view_plane.max = {width_half, height_half};
		Invalidate();
	}

	void Set_Aspect_Ratio(float width_to_height) noexcept
	{
		m_aspect_ratio = width_to_height;
		m_view_plane.min.y = m_view_plane.min.x / m_aspect_ratio;
		m_view_plane.max.y = m_view_plane.max.x / m_aspect_ratio;
		Invalidate();
	}

	// Preserve the legacy camera assignment contract, where the view plane and
	// aspect-ratio field were copied independently.
	void Set_Aspect_Ratio_Value(float width_to_height) noexcept
	{
		m_aspect_ratio = width_to_height;
		Invalidate();
	}

	void Get_View_Plane(Assets::Vector2f &minimum, Assets::Vector2f &maximum) const noexcept
	{
		minimum = m_view_plane.min;
		maximum = m_view_plane.max;
	}

	const CameraViewPlane &Get_View_Plane() const noexcept
	{
		return m_view_plane;
	}

	float Get_Horizontal_FOV() const noexcept
	{
		return static_cast<float>(2.0 * std::atan2(m_view_plane.Width(), 2.0f));
	}

	float Get_Vertical_FOV() const noexcept
	{
		return static_cast<float>(2.0 * std::atan2(m_view_plane.Height(), 2.0f));
	}

	float Get_Aspect_Ratio() const noexcept
	{
		return m_aspect_ratio;
	}

	const Matrix4x4 &Get_Projection_Matrix() const noexcept
	{
		Ensure_Projection();
		return m_projection;
	}

	Matrix4x4 Build_Projection_Matrix() const noexcept
	{
		return Get_Projection_Matrix();
	}

	const Matrix4x4 &Get_Backend_Projection_Matrix() const noexcept
	{
		Ensure_Projection();
		m_backend_projection = m_projection;

		const float inverse_depth = static_cast<float>(1.0 / (m_far_clip - m_near_clip));
		if (m_type == CameraProjectionType::Perspective) {
			m_backend_projection.values[10] = -m_far_clip * inverse_depth;
			m_backend_projection.values[11] = -(m_far_clip * m_near_clip) * inverse_depth;
		} else {
			m_backend_projection.values[10] = -inverse_depth;
			m_backend_projection.values[11] = -m_near_clip * inverse_depth;
		}
		return m_backend_projection;
	}

	Matrix4x4 Build_Backend_Projection_Matrix() const noexcept
	{
		return Get_Backend_Projection_Matrix();
	}

	CameraProjectionResult Project_Camera_Space_Point(Vector3 &destination,
		const Vector3 &camera_point) const noexcept
	{
		return Project_Point(destination, camera_point, true);
	}

	// This spelling is useful to callers that already have camera-space data.
	CameraProjectionResult Project(Vector3 &destination, const Vector3 &camera_point) const noexcept
	{
		return Project_Point(destination, camera_point, false);
	}

	float Compute_Projected_Sphere_Radius(float distance, float radius) const noexcept
	{
		const auto &matrix = Get_Projection_Matrix().values;
		const float projected_x = matrix[0] * radius + matrix[2] * distance + matrix[3];
		const float projected_w = matrix[14] * distance + matrix[15];
		return projected_x / projected_w;
	}

private:
	void Invalidate() noexcept
	{
		m_projection_valid = false;
	}

	void Ensure_Projection() const noexcept
	{
		if (m_projection_valid)
			return;

		m_projection = Matrix4x4::Identity();
		const float left = m_type == CameraProjectionType::Perspective
			? m_view_plane.min.x * m_near_clip
			: m_view_plane.min.x;
		const float right = m_type == CameraProjectionType::Perspective
			? m_view_plane.max.x * m_near_clip
			: m_view_plane.max.x;
		const float bottom = m_type == CameraProjectionType::Perspective
			? m_view_plane.min.y * m_near_clip
			: m_view_plane.min.y;
		const float top = m_type == CameraProjectionType::Perspective
			? m_view_plane.max.y * m_near_clip
			: m_view_plane.max.y;

		if (m_type == CameraProjectionType::Perspective) {
			m_projection.values[0] = static_cast<float>(2.0 * m_near_clip / (right - left));
			m_projection.values[2] = (right + left) / (right - left);
			m_projection.values[5] = static_cast<float>(2.0 * m_near_clip / (top - bottom));
			m_projection.values[6] = (top + bottom) / (top - bottom);
			m_projection.values[10] = -(m_far_clip + m_near_clip) / (m_far_clip - m_near_clip);
			m_projection.values[11] = static_cast<float>(-(2.0 * m_far_clip * m_near_clip)
				/ (m_far_clip - m_near_clip));
			m_projection.values[14] = -1.0f;
			m_projection.values[15] = 0.0f;
		} else {
			m_projection.values[0] = 2.0f / (right - left);
			m_projection.values[3] = -(right + left) / (right - left);
			m_projection.values[5] = 2.0f / (top - bottom);
			m_projection.values[7] = -(top + bottom) / (top - bottom);
			m_projection.values[10] = -2.0f / (m_far_clip - m_near_clip);
			m_projection.values[11] = -(m_far_clip + m_near_clip) / (m_far_clip - m_near_clip);
		}

		m_projection_valid = true;
	}

	CameraProjectionResult Project_Point(Vector3 &destination,
		const Vector3 &camera_point, bool use_epsilon) const noexcept
	{
		if (camera_point.z > -m_near_clip + (use_epsilon ? Camera_Projection_Epsilon : 0.0f)) {
			destination = {};
			return CameraProjectionResult::OutsideNearClip;
		}

		const auto &matrix = Get_Projection_Matrix().values;
		const float projected_x = matrix[0] * camera_point.x
			+ matrix[1] * camera_point.y + matrix[2] * camera_point.z + matrix[3];
		const float projected_y = matrix[4] * camera_point.x
			+ matrix[5] * camera_point.y + matrix[6] * camera_point.z + matrix[7];
		const float projected_z = matrix[8] * camera_point.x
			+ matrix[9] * camera_point.y + matrix[10] * camera_point.z + matrix[11];
		const float projected_w = matrix[12] * camera_point.x
			+ matrix[13] * camera_point.y + matrix[14] * camera_point.z + matrix[15];
		const float inverse_w = 1.0f / projected_w;
		destination = {projected_x * inverse_w, projected_y * inverse_w, projected_z * inverse_w};

		if (destination.z > 1.0f)
			return CameraProjectionResult::OutsideFarClip;
		if (destination.x < -1.0f || destination.x > 1.0f
			|| destination.y < -1.0f || destination.y > 1.0f)
			return CameraProjectionResult::OutsideFrustum;
		return CameraProjectionResult::InsideFrustum;
	}

	CameraProjectionType m_type = CameraProjectionType::Perspective;
	CameraViewPlane m_view_plane{};
	float m_aspect_ratio = 4.0f / 3.0f;
	float m_near_clip = 1.0f;
	float m_far_clip = 1000.0f;
	mutable bool m_projection_valid = false;
	mutable Matrix4x4 m_projection{};
	mutable Matrix4x4 m_backend_projection{};
};

}
