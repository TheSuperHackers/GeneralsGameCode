module;

#include <array>
#include <cstddef>

export module Graphics.Scene.Views.CameraState;

import Assets.Math;
export import Graphics.Scene.AffineTransform;
export import Graphics.Scene.Views.CameraFrustum;
export import Graphics.Scene.Views.CameraMatrices;

namespace Graphics
{

export class CameraState final
{
public:
	CameraState() noexcept = default;

	void Set_Transform(const RenderTransform &transform) noexcept
	{
		m_transform = transform;
		Invalidate();
	}

	const RenderTransform &Get_Transform() const noexcept
	{
		return m_transform;
	}

	const RenderTransform &Get_Inverse_Transform() const noexcept
	{
		Update_Cache();
		return m_inverse_transform;
	}

	void Set_Position(const Vector3 &position) noexcept
	{
		m_transform.matrix[3] = position.x;
		m_transform.matrix[7] = position.y;
		m_transform.matrix[11] = position.z;
		Invalidate();
	}

	Vector3 Get_Position() const noexcept
	{
		return {m_transform.matrix[3], m_transform.matrix[7], m_transform.matrix[11]};
	}

	Vector3 Get_Right_Dir() const noexcept
	{
		return {m_transform.matrix[0], m_transform.matrix[4], m_transform.matrix[8]};
	}

	Vector3 Get_Forward_Dir() const noexcept
	{
		return {-m_transform.matrix[2], -m_transform.matrix[6], -m_transform.matrix[10]};
	}

	Vector3 Get_Up_Dir() const noexcept
	{
		return {m_transform.matrix[1], m_transform.matrix[5], m_transform.matrix[9]};
	}

	void Set_Projection_Type(CameraProjectionType type) noexcept
	{
		m_projection.Set_Projection_Type(type);
		Invalidate();
	}

	CameraProjectionType Get_Projection_Type() const noexcept
	{
		return m_projection.Get_Projection_Type();
	}

	void Set_Clip_Planes(float near_clip, float far_clip) noexcept
	{
		m_projection.Set_Clip_Planes(near_clip, far_clip);
		Invalidate();
	}

	void Get_Clip_Planes(float &near_clip, float &far_clip) const noexcept
	{
		m_projection.Get_Clip_Planes(near_clip, far_clip);
	}

	float Get_Depth() const noexcept
	{
		return m_projection.Get_Far_Clip();
	}

	void Set_View_Plane(Assets::Vector2f minimum, Assets::Vector2f maximum) noexcept
	{
		m_projection.Set_View_Plane(minimum, maximum);
		Invalidate();
	}

	void Set_View_Plane(float horizontal_fov, float vertical_fov = -1.0f) noexcept
	{
		m_projection.Set_View_Plane(horizontal_fov, vertical_fov);
		Invalidate();
	}

	void Set_Aspect_Ratio(float width_to_height) noexcept
	{
		m_projection.Set_Aspect_Ratio(width_to_height);
		Invalidate();
	}

	void Set_Aspect_Ratio_Value(float width_to_height) noexcept
	{
		m_projection.Set_Aspect_Ratio_Value(width_to_height);
		Invalidate();
	}

	void Get_View_Plane(Assets::Vector2f &minimum, Assets::Vector2f &maximum) const noexcept
	{
		m_projection.Get_View_Plane(minimum, maximum);
	}

	const CameraViewPlane &Get_View_Plane() const noexcept
	{
		return m_projection.Get_View_Plane();
	}

	float Get_Horizontal_FOV() const noexcept
	{
		return m_projection.Get_Horizontal_FOV();
	}

	float Get_Vertical_FOV() const noexcept
	{
		return m_projection.Get_Vertical_FOV();
	}

	float Get_Aspect_Ratio() const noexcept
	{
		return m_projection.Get_Aspect_Ratio();
	}

	void Set_Viewport(Assets::Vector2f minimum, Assets::Vector2f maximum) noexcept
	{
		m_viewport.min = minimum;
		m_viewport.max = maximum;
		Invalidate();
	}

	void Get_Viewport(Assets::Vector2f &minimum, Assets::Vector2f &maximum) const noexcept
	{
		minimum = m_viewport.min;
		maximum = m_viewport.max;
	}

	const CameraViewport &Get_Viewport() const noexcept
	{
		return m_viewport;
	}

	void Set_Depth_Range(float minimum = 0.0f, float maximum = 1.0f) noexcept
	{
		m_depth_min = minimum;
		m_depth_max = maximum;
	}

	void Set_Zbuffer_Range(float minimum = 0.0f, float maximum = 1.0f) noexcept
	{
		Set_Depth_Range(minimum, maximum);
	}

	void Get_Depth_Range(float *minimum, float *maximum) const noexcept
	{
		if (minimum != nullptr)
			*minimum = m_depth_min;
		if (maximum != nullptr)
			*maximum = m_depth_max;
	}

	void Get_Zbuffer_Range(float &minimum, float &maximum) const noexcept
	{
		minimum = m_depth_min;
		maximum = m_depth_max;
	}

	float Get_Depth_Min() const noexcept { return m_depth_min; }
	float Get_Depth_Max() const noexcept { return m_depth_max; }

	const Matrix4x4 &Get_Projection_Matrix() const noexcept
	{
		Update_Cache();
		return m_projection_matrix;
	}

	const Matrix4x4 &Get_Backend_Projection_Matrix() const noexcept
	{
		Update_Cache();
		return m_backend_projection_matrix;
	}

	const Matrix4x4 &Get_View_Matrix() const noexcept
	{
		Update_Cache();
		return m_view_matrix;
	}

	CameraProjectionResult Project(Vector3 &destination, const Vector3 &world_point) const noexcept
	{
		Update_Cache();
		return m_projection.Project(destination, Transform_Point(m_inverse_transform, world_point));
	}

	CameraProjectionResult Project_Camera_Space_Point(Vector3 &destination,
		const Vector3 &camera_point) const noexcept
	{
		Update_Cache();
		return m_projection.Project_Camera_Space_Point(destination, camera_point);
	}

	void Un_Project(Vector3 &destination, const Assets::Vector2f &view_point) const noexcept
	{
		const CameraViewPlane &view_plane = m_projection.Get_View_Plane();
		const float view_width = view_plane.Width();
		const float view_height = view_plane.Height();
		const Vector3 camera_point{
			view_plane.min.x + view_width * (view_point.x + 1.0f) * 0.5f,
			view_plane.min.y + view_height * (view_point.y + 1.0f) * 0.5f,
			-1.0f};
		destination = Transform_Point(m_transform, camera_point);
	}

	void Transform_To_View_Space(Vector3 &destination, const Vector3 &world_point) const noexcept
	{
		Update_Cache();
		destination = Transform_Point(m_inverse_transform, world_point);
	}

	void Rotate_To_View_Space(Vector3 &destination, const Vector3 &world_vector) const noexcept
	{
		Update_Cache();
		destination = Rotate_Vector(m_inverse_transform, world_vector);
	}

	void Device_To_View_Space(const Assets::Vector2f &device_coordinate,
		float target_width, float target_height, Vector3 &view_coordinate) const noexcept
	{
		const CameraViewPlane &view_plane = m_projection.Get_View_Plane();
		const float normalized_x = device_coordinate.x / target_width;
		const float normalized_y = device_coordinate.y / target_height;
		view_coordinate.x = view_plane.min.x
			+ (normalized_x - m_viewport.min.x) * view_plane.Width() / m_viewport.Width();
		view_coordinate.y = view_plane.max.y
			- (normalized_y - m_viewport.min.y) * view_plane.Height() / m_viewport.Height();
		view_coordinate.z = -1.0f;
	}

	void Device_To_World_Space(const Assets::Vector2f &device_coordinate,
		float target_width, float target_height, Vector3 &world_coordinate) const noexcept
	{
		Vector3 view_coordinate{};
		Device_To_View_Space(device_coordinate, target_width, target_height, view_coordinate);
		world_coordinate = Transform_Point(m_transform, view_coordinate);
	}

	float Compute_Projected_Sphere_Radius(float distance, float radius) const noexcept
	{
		return m_projection.Compute_Projected_Sphere_Radius(distance, radius);
	}

	const CameraFrustum &Get_Frustum() const noexcept
	{
		Update_Cache();
		return m_frustum;
	}

	const CameraFrustum &Get_View_Space_Frustum() const noexcept
	{
		Update_Cache();
		return m_view_space_frustum;
	}

	const CameraOrientedBox &Get_Near_Clip_Bounding_Box() const noexcept
	{
		Update_Cache();
		return m_near_clip_box;
	}

	bool Cull_Sphere(const CameraSphere &sphere) const noexcept
	{
		return Get_Frustum().Cull_Sphere(sphere);
	}

	bool Cull_Sphere_On_Frustum_Sides(const CameraSphere &sphere) const noexcept
	{
		return Get_Frustum().Cull_Sphere_On_Frustum_Sides(sphere);
	}

	bool Cull_Box(const CameraAxisAlignedBox &box) const noexcept
	{
		return Get_Frustum().Cull_Box(box);
	}

	bool Cull_Oriented_Box(const CameraOrientedBox &box) const noexcept
	{
		return Get_Frustum().Cull_Oriented_Box(box);
	}

	void Publish_Camera_Matrices() const noexcept
	{
		Update_Cache();
		CameraMatrices &matrices = Get_Camera_Matrices();
		matrices.view = m_view_matrix;
		matrices.projection = m_backend_projection_matrix;
	}

	static void Convert_Old(Vector3 &position) noexcept
	{
		position.x = (position.x + 1.0f) * 0.5f;
		position.y = (position.y + 1.0f) * 0.5f;
	}

private:
	static Vector3 Transform_Point(const RenderTransform &transform, const Vector3 &point) noexcept
	{
		const auto &matrix = transform.matrix;
		return {
			matrix[0] * point.x + matrix[1] * point.y + matrix[2] * point.z + matrix[3],
			matrix[4] * point.x + matrix[5] * point.y + matrix[6] * point.z + matrix[7],
			matrix[8] * point.x + matrix[9] * point.y + matrix[10] * point.z + matrix[11]};
	}

	static Vector3 Rotate_Vector(const RenderTransform &transform, const Vector3 &vector) noexcept
	{
		const auto &matrix = transform.matrix;
		return {
			matrix[0] * vector.x + matrix[1] * vector.y + matrix[2] * vector.z,
			matrix[4] * vector.x + matrix[5] * vector.y + matrix[6] * vector.z,
			matrix[8] * vector.x + matrix[9] * vector.y + matrix[10] * vector.z};
	}

	static Matrix4x4 To_Matrix4x4(const RenderTransform &transform) noexcept
	{
		Matrix4x4 result = Matrix4x4::Identity();
		for (std::size_t row = 0; row < 3; ++row)
			for (std::size_t column = 0; column < 4; ++column)
				result.values[row * 4 + column] = transform.matrix[row * 4 + column];
		return result;
	}

	static CameraOrientedBox Make_Near_Clip_Box(const RenderTransform &transform,
		const CameraViewPlane &view_plane, float near_clip) noexcept
	{
		CameraOrientedBox result{};
		result.center = Transform_Point(transform, {0.0f, 0.0f, -near_clip});
		result.extent = {
			view_plane.Width() * near_clip * 0.5f,
			view_plane.Height() * near_clip * 0.5f,
			0.01f};
		result.basis = {
			transform.matrix[0], transform.matrix[1], transform.matrix[2],
			transform.matrix[4], transform.matrix[5], transform.matrix[6],
			transform.matrix[8], transform.matrix[9], transform.matrix[10]};
		return result;
	}

	void Invalidate() noexcept
	{
		m_cache_valid = false;
	}

	void Update_Cache() const noexcept
	{
		if (m_cache_valid)
			return;

		Try_Invert_Affine(m_transform, m_inverse_transform);
		m_view_matrix = To_Matrix4x4(m_inverse_transform);
		m_projection_matrix = m_projection.Get_Projection_Matrix();
		m_backend_projection_matrix = m_projection.Get_Backend_Projection_Matrix();
		const CameraViewPlane &view_plane = m_projection.Get_View_Plane();
		const float near_clip = m_projection.Get_Near_Clip();
		const float far_clip = m_projection.Get_Far_Clip();
		m_frustum = Build_Camera_Frustum(m_transform, view_plane, -near_clip, -far_clip);
		m_view_space_frustum = Build_Camera_Frustum(Affine_Identity(), view_plane, -near_clip, -far_clip);
		m_near_clip_box = Make_Near_Clip_Box(m_transform, view_plane, near_clip);
		m_cache_valid = true;
	}

	RenderTransform m_transform = Affine_Identity();
	CameraProjection m_projection{};
	CameraViewport m_viewport{};
	float m_depth_min = 0.0f;
	float m_depth_max = 1.0f;

	mutable bool m_cache_valid = false;
	mutable RenderTransform m_inverse_transform = Affine_Identity();
	mutable Matrix4x4 m_view_matrix = Matrix4x4::Identity();
	mutable Matrix4x4 m_projection_matrix = Matrix4x4::Identity();
	mutable Matrix4x4 m_backend_projection_matrix = Matrix4x4::Identity();
	mutable CameraFrustum m_frustum{};
	mutable CameraFrustum m_view_space_frustum{};
	mutable CameraOrientedBox m_near_clip_box{};
};

}
