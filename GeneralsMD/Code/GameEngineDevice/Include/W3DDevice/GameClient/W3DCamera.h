#pragma once

#include "W3DDevice/GameClient/W3DRenderObject.h"
#include "WWMath/aabox.h"
#include "WWMath/frustum.h"
#include "WWMath/matrix4.h"
#include "WWMath/obbox.h"
#include "WWMath/sphere.h"
#include "WWMath/vector2.h"

import Graphics.Scene.Views.CameraState;

class W3DRenderContext;

class W3DViewport final
{
public:
	W3DViewport() : Min(0.0f, 0.0f), Max(1.0f, 1.0f) {}
	W3DViewport(const Vector2 &minimum, const Vector2 &maximum) : Min(minimum), Max(maximum) {}

	float Width() const { return Max.X - Min.X; }
	float Height() const { return Max.Y - Min.Y; }

	Vector2 Min;
	Vector2 Max;
};

// Game-facing camera adapter. Projection, frustum construction, and point
// conversion live in Graphics::CameraState; this class retains the native
// value types required by existing render-object callers.
class W3DCamera : public W3DRenderObject
{
public:
	enum ProjectionType
	{
		PERSPECTIVE = 0,
		ORTHO
	};

	enum ProjectionResType
	{
		INSIDE_FRUSTUM,
		OUTSIDE_FRUSTUM,
		OUTSIDE_NEAR_CLIP,
		OUTSIDE_FAR_CLIP
	};

	W3DCamera();
	W3DCamera(const W3DCamera &source);
	W3DCamera &operator=(const W3DCamera &source);
	~W3DCamera() override;

	W3DRenderObject *Clone() const override;
	int Class_ID() const override { return W3DRenderObject::CLASSID_CAMERA; }

	void Render(W3DRenderContext &) override {}

	void Set_Transform(const Matrix3D &transform) override;
	void Set_Position(const Vector3 &position) override;

	Vector3 Get_Right_Dir() const;
	Vector3 Get_Forward_Dir() const;
	Vector3 Get_Up_Dir() const;

	void Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const override;
	void Get_Obj_Space_Bounding_Box(AABoxClass &box) const override;

	float Get_Depth() const;

	void Set_Projection_Type(ProjectionType type);
	ProjectionType Get_Projection_Type() const;

	void Set_Clip_Planes(float near_clip, float far_clip);
	void Get_Clip_Planes(float &near_clip, float &far_clip) const;

	void Set_Zbuffer_Range(float near_depth, float far_depth);
	void Get_Zbuffer_Range(float &near_depth, float &far_depth) const;

	void Set_View_Plane(const Vector2 &minimum, const Vector2 &maximum);
	void Set_View_Plane(float horizontal_fov, float vertical_fov = -1.0f);
	void Set_Aspect_Ratio(float width_to_height);

	void Get_View_Plane(Vector2 &minimum, Vector2 &maximum) const;
	float Get_Horizontal_FOV() const;
	float Get_Vertical_FOV() const;
	float Get_Aspect_Ratio() const;

	void Get_Projection_Matrix(Matrix4x4 *matrix);
	void Get_Backend_Projection_Matrix(Matrix4x4 *matrix);
	void Get_View_Matrix(Matrix3D *matrix);
	const Matrix4x4 &Get_Projection_Matrix();
	const Matrix3D &Get_View_Matrix();

	ProjectionResType Project(Vector3 &destination, const Vector3 &world_point) const;
	ProjectionResType Project_Camera_Space_Point(Vector3 &destination,
		const Vector3 &camera_point) const;
	void Un_Project(Vector3 &destination, const Vector2 &view_point) const;
	void Transform_To_View_Space(Vector3 &destination, const Vector3 &world_point) const;
	void Rotate_To_View_Space(Vector3 &destination, const Vector3 &world_vector) const;

	void Set_Viewport(const Vector2 &minimum, const Vector2 &maximum);
	void Get_Viewport(Vector2 &minimum, Vector2 &maximum) const;
	const W3DViewport &Get_Viewport() const;

	void Set_Depth_Range(float start = 0.0f, float end = 1.0f);
	void Get_Depth_Range(float *start, float *end) const;

	bool Cull_Sphere(const SphereClass &sphere) const;
	bool Cull_Sphere_On_Frustum_Sides(const SphereClass &sphere) const;
	bool Cull_Box(const AABoxClass &box) const;

	const FrustumClass &Get_Frustum() const;
	const PlaneClass *Get_Frustum_Planes() const;
	const Vector3 *Get_Frustum_Corners() const;
	const FrustumClass &Get_View_Space_Frustum() const;
	const PlaneClass *Get_View_Space_Frustum_Planes() const;
	const Vector3 *Get_View_Space_Frustum_Corners() const;
	const OBBoxClass &Get_Near_Clip_Bounding_Box() const;

	void Device_To_View_Space(const Vector2 &device_coordinate, Vector3 *view_coordinate);
	void Device_To_World_Space(const Vector2 &device_coordinate, Vector3 *world_coordinate);
	float Compute_Projected_Sphere_Radius(float distance, float radius);

	void Apply();

	static void Convert_Old(Vector3 &position);

private:
	void Invalidate_Native_Cache() noexcept;
	void Synchronize_State_Transform() const noexcept;
	void Update_Native_Cache() const;

	mutable Graphics::CameraState m_state;
	W3DViewport m_viewport;
	mutable bool m_native_cache_valid = false;
	mutable FrustumClass m_frustum;
	mutable FrustumClass m_view_space_frustum;
	mutable OBBoxClass m_near_clip_box;
	mutable Matrix4x4 m_projection_matrix;
	mutable Matrix4x4 m_backend_projection_matrix;
	mutable Matrix3D m_view_matrix;
};
