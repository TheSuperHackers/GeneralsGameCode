#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "W3DDevice/GameClient/W3DCamera.h"

import Graphics.Frame.AttachmentBindings;

namespace
{

Graphics::Vector3 To_Graphics(const Vector3 &value) noexcept
{
	return {value.X, value.Y, value.Z};
}

Vector3 To_Native(const Graphics::Vector3 &value) noexcept
{
	return {value.x, value.y, value.z};
}

Graphics::CameraProjectionType To_Graphics(W3DCamera::ProjectionType type) noexcept
{
	return type == W3DCamera::ORTHO
		? Graphics::CameraProjectionType::Ortho
		: Graphics::CameraProjectionType::Perspective;
}

W3DCamera::ProjectionType To_Native(Graphics::CameraProjectionType type) noexcept
{
	return type == Graphics::CameraProjectionType::Ortho
		? W3DCamera::ORTHO
		: W3DCamera::PERSPECTIVE;
}

W3DCamera::ProjectionResType To_Native(Graphics::CameraProjectionResult result) noexcept
{
	switch (result) {
	case Graphics::CameraProjectionResult::OutsideFrustum:
		return W3DCamera::OUTSIDE_FRUSTUM;
	case Graphics::CameraProjectionResult::OutsideNearClip:
		return W3DCamera::OUTSIDE_NEAR_CLIP;
	case Graphics::CameraProjectionResult::OutsideFarClip:
		return W3DCamera::OUTSIDE_FAR_CLIP;
	case Graphics::CameraProjectionResult::InsideFrustum:
	default:
		return W3DCamera::INSIDE_FRUSTUM;
	}
}

Graphics::CameraSphere To_Graphics(const SphereClass &sphere) noexcept
{
	return {To_Graphics(sphere.Center), sphere.Radius};
}

Graphics::CameraAxisAlignedBox To_Graphics(const AABoxClass &box) noexcept
{
	return {To_Graphics(box.Center), To_Graphics(box.Extent)};
}

Matrix4x4 To_Native(const Graphics::Matrix4x4 &source) noexcept
{
	Matrix4x4 result(true);
	for (int row = 0; row < 4; ++row)
		for (int column = 0; column < 4; ++column)
			result[row][column] = source.values[static_cast<std::size_t>(row * 4 + column)];
	return result;
}

Matrix3D To_Native(const Graphics::RenderTransform &source) noexcept
{
	return Graphics::Export_Affine_Transform<Matrix3D>(source);
}

void Set_Native_Frustum(FrustumClass &destination, const Graphics::CameraFrustum &source) noexcept
{
	for (std::size_t index = 0; index < source.corners.size(); ++index)
		destination.Corners[index] = To_Native(source.corners[index]);
	for (std::size_t index = 0; index < source.planes.size(); ++index) {
		const auto &plane = source.planes[index];
		destination.Planes[index].Set(plane.normal.x, plane.normal.y, plane.normal.z, plane.distance);
	}
	destination.BoundMin = To_Native(source.bound_min);
	destination.BoundMax = To_Native(source.bound_max);
}

void Set_Native_Near_Clip_Box(OBBoxClass &destination,
	const Graphics::CameraOrientedBox &source) noexcept
{
	destination.Center = To_Native(source.center);
	destination.Extent = To_Native(source.extent);
	for (int row = 0; row < 3; ++row)
		for (int column = 0; column < 3; ++column)
			destination.Basis[row][column] = source.basis[static_cast<std::size_t>(row * 3 + column)];
}

}

W3DCamera::W3DCamera()
{
	Set_Transform(Matrix3D(true));
}

W3DCamera::W3DCamera(const W3DCamera &source)
	: W3DRenderObject(source), m_state(source.m_state), m_viewport(source.m_viewport)
{
}

W3DCamera &W3DCamera::operator=(const W3DCamera &source)
{
	if (this != &source) {
		const Graphics::RenderTransform target_transform =
			Graphics::Import_Affine_Transform(Get_Transform());
		const float target_aspect_ratio = m_state.Get_Aspect_Ratio();
		const float target_depth_min = m_state.Get_Depth_Min();
		const float target_depth_max = m_state.Get_Depth_Max();
		W3DRenderObject::operator=(source);
		m_state = source.m_state;
		m_viewport = source.m_viewport;
		m_state.Set_Transform(target_transform);
		m_state.Set_Aspect_Ratio_Value(target_aspect_ratio);
		m_state.Set_Depth_Range(target_depth_min, target_depth_max);
		Invalidate_Native_Cache();
	}
	return *this;
}

W3DCamera::~W3DCamera() = default;

W3DRenderObject *W3DCamera::Clone() const
{
	return NEW_REF(W3DCamera, (*this));
}

void W3DCamera::Set_Transform(const Matrix3D &transform)
{
	W3DRenderObject::Set_Transform(transform);
	m_state.Set_Transform(Graphics::Import_Affine_Transform(transform));
	Invalidate_Native_Cache();
}

void W3DCamera::Set_Position(const Vector3 &position)
{
	W3DRenderObject::Set_Position(position);
	m_state.Set_Transform(Graphics::Import_Affine_Transform(Get_Transform()));
	Invalidate_Native_Cache();
}

Vector3 W3DCamera::Get_Right_Dir() const
{
	return Get_Transform().Get_X_Vector();
}

Vector3 W3DCamera::Get_Forward_Dir() const
{
	return -Get_Transform().Get_Z_Vector();
}

Vector3 W3DCamera::Get_Up_Dir() const
{
	return Get_Transform().Get_Y_Vector();
}

void W3DCamera::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
	sphere.Center.Set(0.0f, 0.0f, 0.0f);
	sphere.Radius = Get_Depth();
}

void W3DCamera::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
	box.Center.Set(0.0f, 0.0f, 0.0f);
	box.Extent.Set(Get_Depth(), Get_Depth(), Get_Depth());
}

float W3DCamera::Get_Depth() const
{
	return m_state.Get_Depth();
}

void W3DCamera::Set_Projection_Type(ProjectionType type)
{
	m_state.Set_Projection_Type(To_Graphics(type));
	Invalidate_Native_Cache();
}

W3DCamera::ProjectionType W3DCamera::Get_Projection_Type() const
{
	return To_Native(m_state.Get_Projection_Type());
}

void W3DCamera::Set_Clip_Planes(float near_clip, float far_clip)
{
	m_state.Set_Clip_Planes(near_clip, far_clip);
	Invalidate_Native_Cache();
}

void W3DCamera::Get_Clip_Planes(float &near_clip, float &far_clip) const
{
	m_state.Get_Clip_Planes(near_clip, far_clip);
}

void W3DCamera::Set_Zbuffer_Range(float near_depth, float far_depth)
{
	m_state.Set_Zbuffer_Range(near_depth, far_depth);
}

void W3DCamera::Get_Zbuffer_Range(float &near_depth, float &far_depth) const
{
	m_state.Get_Zbuffer_Range(near_depth, far_depth);
}

void W3DCamera::Set_View_Plane(const Vector2 &minimum, const Vector2 &maximum)
{
	m_state.Set_View_Plane({minimum.X, minimum.Y}, {maximum.X, maximum.Y});
	Invalidate_Native_Cache();
}

void W3DCamera::Set_View_Plane(float horizontal_fov, float vertical_fov)
{
	m_state.Set_View_Plane(horizontal_fov, vertical_fov);
	Invalidate_Native_Cache();
}

void W3DCamera::Set_Aspect_Ratio(float width_to_height)
{
	m_state.Set_Aspect_Ratio(width_to_height);
	Invalidate_Native_Cache();
}

void W3DCamera::Get_View_Plane(Vector2 &minimum, Vector2 &maximum) const
{
	const auto &view_plane = m_state.Get_View_Plane();
	minimum.Set(view_plane.min.x, view_plane.min.y);
	maximum.Set(view_plane.max.x, view_plane.max.y);
}

float W3DCamera::Get_Horizontal_FOV() const
{
	return m_state.Get_Horizontal_FOV();
}

float W3DCamera::Get_Vertical_FOV() const
{
	return m_state.Get_Vertical_FOV();
}

float W3DCamera::Get_Aspect_Ratio() const
{
	return m_state.Get_Aspect_Ratio();
}

void W3DCamera::Get_Projection_Matrix(Matrix4x4 *matrix)
{
	if (matrix == nullptr)
		return;
	Update_Native_Cache();
	*matrix = m_projection_matrix;
}

void W3DCamera::Get_Backend_Projection_Matrix(Matrix4x4 *matrix)
{
	if (matrix == nullptr)
		return;
	Update_Native_Cache();
	*matrix = m_backend_projection_matrix;
}

void W3DCamera::Get_View_Matrix(Matrix3D *matrix)
{
	if (matrix == nullptr)
		return;
	Update_Native_Cache();
	*matrix = m_view_matrix;
}

const Matrix4x4 &W3DCamera::Get_Projection_Matrix()
{
	Update_Native_Cache();
	return m_projection_matrix;
}

const Matrix3D &W3DCamera::Get_View_Matrix()
{
	Update_Native_Cache();
	return m_view_matrix;
}

W3DCamera::ProjectionResType W3DCamera::Project(Vector3 &destination,
	const Vector3 &world_point) const
{
	Update_Native_Cache();
	Graphics::Vector3 projected{};
	const auto result = m_state.Project(projected, To_Graphics(world_point));
	destination = To_Native(projected);
	return To_Native(result);
}

W3DCamera::ProjectionResType W3DCamera::Project_Camera_Space_Point(Vector3 &destination,
	const Vector3 &camera_point) const
{
	Update_Native_Cache();
	Graphics::Vector3 projected{};
	const auto result = m_state.Project_Camera_Space_Point(projected, To_Graphics(camera_point));
	destination = To_Native(projected);
	return To_Native(result);
}

void W3DCamera::Un_Project(Vector3 &destination, const Vector2 &view_point) const
{
	Update_Native_Cache();
	Graphics::Vector3 world{};
	m_state.Un_Project(world, {view_point.X, view_point.Y});
	destination = To_Native(world);
}

void W3DCamera::Transform_To_View_Space(Vector3 &destination, const Vector3 &world_point) const
{
	Update_Native_Cache();
	Graphics::Vector3 view{};
	m_state.Transform_To_View_Space(view, To_Graphics(world_point));
	destination = To_Native(view);
}

void W3DCamera::Rotate_To_View_Space(Vector3 &destination, const Vector3 &world_vector) const
{
	Update_Native_Cache();
	Graphics::Vector3 view{};
	m_state.Rotate_To_View_Space(view, To_Graphics(world_vector));
	destination = To_Native(view);
}

void W3DCamera::Set_Viewport(const Vector2 &minimum, const Vector2 &maximum)
{
	m_viewport.Min = minimum;
	m_viewport.Max = maximum;
	m_state.Set_Viewport({minimum.X, minimum.Y}, {maximum.X, maximum.Y});
	Invalidate_Native_Cache();
}

void W3DCamera::Get_Viewport(Vector2 &minimum, Vector2 &maximum) const
{
	minimum = m_viewport.Min;
	maximum = m_viewport.Max;
}

const W3DViewport &W3DCamera::Get_Viewport() const
{
	return m_viewport;
}

void W3DCamera::Set_Depth_Range(float start, float end)
{
	m_state.Set_Depth_Range(start, end);
}

void W3DCamera::Get_Depth_Range(float *start, float *end) const
{
	m_state.Get_Depth_Range(start, end);
}

bool W3DCamera::Cull_Sphere(const SphereClass &sphere) const
{
	Update_Native_Cache();
	return m_state.Cull_Sphere(To_Graphics(sphere));
}

bool W3DCamera::Cull_Sphere_On_Frustum_Sides(const SphereClass &sphere) const
{
	Update_Native_Cache();
	return m_state.Cull_Sphere_On_Frustum_Sides(To_Graphics(sphere));
}

bool W3DCamera::Cull_Box(const AABoxClass &box) const
{
	Update_Native_Cache();
	return m_state.Cull_Box(To_Graphics(box));
}

const FrustumClass &W3DCamera::Get_Frustum() const
{
	Update_Native_Cache();
	return m_frustum;
}

const PlaneClass *W3DCamera::Get_Frustum_Planes() const
{
	return Get_Frustum().Planes;
}

const Vector3 *W3DCamera::Get_Frustum_Corners() const
{
	return Get_Frustum().Corners;
}

const FrustumClass &W3DCamera::Get_View_Space_Frustum() const
{
	Update_Native_Cache();
	return m_view_space_frustum;
}

const PlaneClass *W3DCamera::Get_View_Space_Frustum_Planes() const
{
	return Get_View_Space_Frustum().Planes;
}

const Vector3 *W3DCamera::Get_View_Space_Frustum_Corners() const
{
	return Get_View_Space_Frustum().Corners;
}

const OBBoxClass &W3DCamera::Get_Near_Clip_Bounding_Box() const
{
	Update_Native_Cache();
	return m_near_clip_box;
}

void W3DCamera::Device_To_View_Space(const Vector2 &device_coordinate, Vector3 *view_coordinate)
{
	if (view_coordinate == nullptr)
		return;
	Update_Native_Cache();
	const auto &viewport = Graphics::Get_Attachment_Bindings().Default().viewport;
	Graphics::Vector3 view{};
	m_state.Device_To_View_Space({device_coordinate.X, device_coordinate.Y},
		static_cast<float>(viewport.width), static_cast<float>(viewport.height), view);
	*view_coordinate = To_Native(view);
}

void W3DCamera::Device_To_World_Space(const Vector2 &device_coordinate, Vector3 *world_coordinate)
{
	if (world_coordinate == nullptr)
		return;
	Update_Native_Cache();
	const auto &viewport = Graphics::Get_Attachment_Bindings().Default().viewport;
	Graphics::Vector3 world{};
	m_state.Device_To_World_Space({device_coordinate.X, device_coordinate.Y},
		static_cast<float>(viewport.width), static_cast<float>(viewport.height), world);
	*world_coordinate = To_Native(world);
}

float W3DCamera::Compute_Projected_Sphere_Radius(float distance, float radius)
{
	return m_state.Compute_Projected_Sphere_Radius(distance, radius);
}

void W3DCamera::Apply()
{
	Update_Native_Cache();
	const auto &screen = Graphics::Get_Attachment_Bindings().Default().viewport;
	const float width = static_cast<float>(screen.width);
	const float height = static_cast<float>(screen.height);
	Graphics::RHIViewport viewport{};
	viewport.x = static_cast<std::uint32_t>(m_viewport.Min.X * width);
	viewport.y = static_cast<std::uint32_t>(m_viewport.Min.Y * height);
	viewport.width = static_cast<std::uint32_t>(m_viewport.Width() * width);
	viewport.height = static_cast<std::uint32_t>(m_viewport.Height() * height);
	m_state.Get_Depth_Range(&viewport.min_depth, &viewport.max_depth);
	Graphics::Get_Attachment_Bindings().Set_Viewport(viewport);
	m_state.Publish_Camera_Matrices();
}

void W3DCamera::Convert_Old(Vector3 &position)
{
	position.X = (position.X + 1.0f) * 0.5f;
	position.Y = (position.Y + 1.0f) * 0.5f;
}

void W3DCamera::Invalidate_Native_Cache() noexcept
{
	m_native_cache_valid = false;
}

void W3DCamera::Update_Native_Cache() const
{
	if (m_native_cache_valid)
		return;
	Synchronize_State_Transform();

	const Matrix3D &native_transform = Get_Transform();
	const auto &projection = m_state.Get_Projection_Matrix();
	const auto &backend_projection = m_state.Get_Backend_Projection_Matrix();
	const auto &view = m_state.Get_Inverse_Transform();
	m_projection_matrix = To_Native(projection);
	m_backend_projection_matrix = To_Native(backend_projection);
	m_view_matrix = To_Native(view);
	Set_Native_Frustum(m_frustum, m_state.Get_Frustum());
	Set_Native_Frustum(m_view_space_frustum, m_state.Get_View_Space_Frustum());
	Set_Native_Near_Clip_Box(m_near_clip_box, m_state.Get_Near_Clip_Bounding_Box());
	m_frustum.CameraTransform = native_transform;
	m_view_space_frustum.CameraTransform = Matrix3D(true);
	m_native_cache_valid = true;
}

void W3DCamera::Synchronize_State_Transform() const noexcept
{
	const Matrix3D &native_transform = Get_Transform();
	const Graphics::RenderTransform transform = Graphics::Import_Affine_Transform(native_transform);
	if (transform.matrix != m_state.Get_Transform().matrix) {
		m_state.Set_Transform(transform);
		m_native_cache_valid = false;
	}
}
