/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/camera.h                               $*
 *                                                                                             *
 *                    Org Author:: Greg_h                                                       *
 *                                                                                             *
 *                       $Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CameraClass::Get_Frustum -- returns the frustum of the camera                             *
 *   CameraClass::Get_Frustum_Planes -- returns pointer to the array of frustum planes         *
 *   CameraClass::Get_Frustum_Corners -- returns pointer to the array of frustum corners       *
 *   CameraClass::Get_View_Space_Frustum -- returns the view-space frustum for this camera     *
 *   CameraClass::Get_View_Space_Frustum_Planes -- returns the view space clip planes for this *
 *   CameraClass::Get_View_Space_Frustum_Corners -- returns the corners of the view space frus *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "rendobj.h"
#include "plane.h"
#include "frustum.h"
#include "obbox.h"
#include "vector2.h"
#include "matrix4.h"
#include "colmath.h"

class RenderInfoClass;

/**
** ViewportClass
** This class is used to define a "normalized" screen space rectangle for the
** camera to render into.  A viewport which filled the entire screen would be
** (0,0) - (1,1) with 0,0 being the upper left and 1,1 being the lower right.
*/
class ViewportClass
{
public:
	ViewportClass(void) : Min(0,0), Max(1,1)													{ }
	ViewportClass(const Vector2 & min,const Vector2 & max) : Min(min), Max(max)	{ }
	ViewportClass(const ViewportClass & vp) : Min(vp.Min), Max(vp.Max)				{ }

	float	Width(void)	const																			{ return Max.X - Min.X; }
	float Height(void) const																		{ return Max.Y - Min.Y; }

	Vector2 Min;
	Vector2 Max;
};


/**
** CameraClass
** This object controls how vertices are transformed from world-space to view
** space, the parameters of the perspective projection, and the viewport
** on screen that the result is mapped into.
**
** Cameras are not "rendered" and do not need to be "added" to a scene.  A
** CameraClass is passed into the WW3D::Render(...) function.  The reason
** they are render objects is so that they can be inserted onto the bone of
** some animation and move with the animation...
**
** For all of the projection functions (Matrix4x4, ProjectorClass (used by
** decals and texture projections), and CameraClass) I followed the OpenGL
** convention of passing positive distances for your clip planes even though
** in a right-handed coordinate system your z values are negative after
** transformation to camera space.  So Set_Clip_Planes expects positive distances
** to your near and far clip planes.
**
** (gth) We should probably separate CameraClass from RenderObjClass... In the
** case where a camera is using a transform from an animation; the programmer
** is usually requesting the transform and plugging it into the camera anyway.
*/
class CameraClass : public RenderObjClass
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
		OUTSIDE_FAR_CLIP,
	};

	CameraClass(void);
	CameraClass(const CameraClass & src);
	CameraClass & operator = (const CameraClass &);
	virtual ~CameraClass(void);
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const { return CLASSID_CAMERA; }

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface -  Rendering, cameras don't "render"
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(RenderInfoClass & rinfo) { }

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - "Scene Graph"
	// Cameras cache their frustum description, this is invalidated whenever
	// the transform/position is changed
	/////////////////////////////////////////////////////////////////////////////
	virtual void 					Set_Transform(const Matrix3D &m);
	virtual void 					Set_Position(const Vector3 &v);

  	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Bounding Volumes
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
   virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & box) const;

	///////////////////////////////////////////////////////////////////////////
	// Camera parameter control
	///////////////////////////////////////////////////////////////////////////
	// Depth of the scene.
	float								Get_Depth(void) const;

	// Setting the projection type
	void								Set_Projection_Type(ProjectionType ptype);
	ProjectionType					Get_Projection_Type(void);

	// Setting the clipping ranges in world space distances
	void								Set_Clip_Planes(float znear,float zfar);
	void								Get_Clip_Planes(float & znear,float & zfar) const;

	// Setting the zbuffer range used during rendering. (Added to allow subdividing the z-buffer. -MW).
	void								Set_Zbuffer_Range(float znear,float zfar) {ZBufferMin = znear;ZBufferMax=zfar;}
	void								Get_Zbuffer_Range(float & znear,float & zfar) const {znear=ZBufferMin;zfar=ZBufferMax;}

	// Methods for setting the View Plane.
	// NOTE: View plane is always at a distance of 1.0 from the eye.
	void								Set_View_Plane(const Vector2 & min,const Vector2 & max);
	void								Set_View_Plane(float hfov,float vfov = -1);
	void								Set_Aspect_Ratio(float width_to_height);

	// Methods for querying the View Plane settings.
	void								Get_View_Plane(Vector2 & set_min,Vector2 & set_max) const;
	float								Get_Horizontal_FOV(void) const;
	float								Get_Vertical_FOV(void) const;
	float								Get_Aspect_Ratio(void) const;

	// Access to the projection matrices for this camera
	void								Get_Projection_Matrix(Matrix4x4 * set_tm);
	void								Get_D3D_Projection_Matrix(Matrix4x4 * set_tm);
	void								Get_View_Matrix(Matrix3D * set_tm);
	const Matrix4x4 &				Get_Projection_Matrix(void);
	const Matrix3D &				Get_View_Matrix(void);

	// Projecting and Un-Projecting a point
	ProjectionResType				Project(Vector3 & dest,const Vector3 & ws_point) const;
	ProjectionResType				Project_Camera_Space_Point(Vector3 & dest,const Vector3 & cam_point) const;
	void								Un_Project(Vector3 & dest,const Vector2 & view_point) const;
	void								Transform_To_View_Space(Vector3 & dest,const Vector3 & ws_point) const;
	void								Rotate_To_View_Space(Vector3 & dest,const Vector3 & ws_vector) const;

	// Viewport control
	void								Set_Viewport(const Vector2 & min,const Vector2 & max);
	void								Get_Viewport(Vector2 & set_min,Vector2 & set_max) const;
	const ViewportClass &		Get_Viewport(void) const;

	void								Set_Depth_Range(float zstart = 0.0f,float zend = 1.0f);
	void								Get_Depth_Range(float * set_zstart,float * set_zend) const;

	// Culling for various bounding volumes.  These functions will return true if the
	// given primitive is culled (i.e. it is *outside* the view frustum)
	bool								Cull_Sphere(const SphereClass & sphere) const;
	bool								Cull_Sphere_On_Frustum_Sides(const SphereClass & sphere) const;
	bool								Cull_Box(const AABoxClass & box) const;

	// Various properties of the camera's frustum:  These funcitons return a
	// pointer to the internal storage of the descriptions.  there will be
	// 6 frustum planes, 8 corner points, see the implementations of these
	// functions for definitions on which points/planes are associated with
	// each index.  Better yet, just use the Frustum object.
	const FrustumClass &			Get_Frustum(void) const;
	const PlaneClass *			Get_Frustum_Planes(void) const;
	const Vector3 *				Get_Frustum_Corners(void) const;
	const FrustumClass &			Get_View_Space_Frustum(void) const;
	const PlaneClass *			Get_View_Space_Frustum_Planes(void) const;
	const Vector3 *				Get_View_Space_Frustum_Corners(void) const;
	const OBBoxClass &			Get_Near_Clip_Bounding_Box(void) const;

	// Methods for transforming/projecting points between various coordinate systems
	// associated with this camera.
	// "Device Space" - pixel coordinate
	// "View Space" - 3D space where the view point is at 0,0,0 and the view plane is at z=-1.0
	// "World Space" - 3D world coordinate system.
	void								Device_To_View_Space(const Vector2 & device_coord,Vector3 * view_coord);
	void								Device_To_World_Space(const Vector2 & device_coord,Vector3 * world_coord);
	float								Compute_Projected_Sphere_Radius(float dist,float radius);

	// apply this camera's settings into d3d.
	void								Apply(void);

	// utility class to convert to old space of 0..1
	static void	Convert_Old(Vector3 &pos);

protected:

	void								Update_Frustum(void) const;

	ProjectionType					Projection;		// projection type, orthographic or perspective
	ViewportClass					Viewport;		// pixel viewport to render into
	ViewportClass					ViewPlane;		// corners of a slice through the frustum at z=-1.0
	float								AspectRatio;	// aspect ratio of the camera, width / height
	float								ZNear;			// near clip plane distance
	float								ZFar;				// far clip plane distance
	float								ZBufferMin;		// smallest value we'll write into the z-buffer (usually 0.0)
	float								ZBufferMax;		// largest value we'll write into the z-buffer (usually 1.0)

	mutable bool					FrustumValid;
	mutable FrustumClass			Frustum;							// world-space frustum and clip planes
	mutable FrustumClass			ViewSpaceFrustum;				// view-space frustum and clip planes
	mutable OBBoxClass			NearClipBBox;					// obbox which bounds the near clip plane
	mutable Matrix4x4				ProjectionTransform;
	mutable Matrix3D				CameraInvTransform;
};


inline float CameraClass::Get_Depth(void) const
{
	return ZFar;
}

inline void CameraClass::Set_Projection_Type(ProjectionType ptype)
{
	FrustumValid = false;
	Projection = ptype;
}

inline CameraClass::ProjectionType CameraClass::Get_Projection_Type(void)
{
	return Projection;
}

inline void CameraClass::Set_Viewport(const Vector2 & min,const Vector2 & max)
{
	Viewport.Min = min; Viewport.Max = max;
	FrustumValid = false;
}

inline void	CameraClass::Get_Viewport(Vector2 & set_min,Vector2 & set_max) const
{
	set_min = Viewport.Min;
	set_max = Viewport.Max;
}

inline void	CameraClass::Set_Depth_Range(float zmin,float zmax)
{
	ZBufferMin = zmin;
	ZBufferMax = zmax;
}

inline void	CameraClass::Get_Depth_Range(float * set_zmin,float * set_zmax) const
{
	if (set_zmin != nullptr) {
		*set_zmin = ZBufferMin;
	}
	if (set_zmax != nullptr) {
		*set_zmax = ZBufferMax;
	}
}

inline const ViewportClass & CameraClass::Get_Viewport(void) const
{
	return Viewport;
}

inline bool CameraClass::Cull_Sphere(const SphereClass & sphere) const
{
	const FrustumClass & frustum = Get_Frustum();
	return CollisionMath::Overlap_Test(frustum,sphere) == CollisionMath::OUTSIDE;
}

inline bool CameraClass::Cull_Sphere_On_Frustum_Sides(const SphereClass & sphere) const
{
	const FrustumClass & frustum = Get_Frustum();
	const PlaneClass * planes = frustum.Planes;
	bool is_visible = true;
	for (int i = 1; i < 5; i++) {
		is_visible = is_visible && (CollisionMath::Overlap_Test(planes[i],sphere) & (CollisionMath::INSIDE|CollisionMath::BOTH));
	}
	return !is_visible;
}


/***********************************************************************************************
 * CameraClass::Get_Frustum -- returns the frustum of the camera                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/24/99    GTH : Created.                                                                 *
 *=============================================================================================*/
inline const FrustumClass &
CameraClass::Get_Frustum(void) const
{
	Update_Frustum();
	return Frustum;
}

/***********************************************************************************************
 * CameraClass::Get_Frustum_Planes -- returns pointer to the array of frustum planes           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/29/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline const PlaneClass *
CameraClass::Get_Frustum_Planes(void) const
{
	const FrustumClass & frustum = Get_Frustum();
	return frustum.Planes;
}


/***********************************************************************************************
 * CameraClass::Get_Frustum_Corners -- returns pointer to the array of frustum corners         *
 *                                                                                             *
 * The camera frustum corner FrustumCorners are defined in the following order                 *
 * The first four points lie on the near clipping plane:                                       *
 *		upper left 0, upper right 1, lower left 2, lower right 3.                                *
 * The last four points lie on the far clipping plane, numbered analogous fashion.             *
 *		upper left 4, upper right 5, lower left 6, lower right 7.                                *
 * (remember: the camera space has x going to the right, y up and z backwards).                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/29/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline const Vector3 *
CameraClass::Get_Frustum_Corners(void) const
{
	const FrustumClass & frustum = Get_Frustum();
	return frustum.Corners;
}


/***********************************************************************************************
 * CameraClass::Get_View_Space_Frustum -- returns the view-space frustum for this camera       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/16/2001  gth : Created.                                                                 *
 *=============================================================================================*/
inline const FrustumClass & CameraClass::Get_View_Space_Frustum(void) const
{
	Update_Frustum();
	return ViewSpaceFrustum;
}


/***********************************************************************************************
 * CameraClass::Get_View_Space_Frustum_Planes -- returns the view space clip planes for this c *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/16/2001  gth : Created.                                                                 *
 *=============================================================================================*/
inline const PlaneClass * CameraClass::Get_View_Space_Frustum_Planes(void) const
{
	const FrustumClass & frustum = Get_View_Space_Frustum();
	return frustum.Planes;
}


/***********************************************************************************************
 * CameraClass::Get_View_Space_Frustum_Corners -- returns the corners of the view space frustu *
 *                                                                                             *
 * The camera frustum corner FrustumCorners are defined in the following order                 *
 * The first four points lie on the near clipping plane:                                       *
 *		upper left 0, upper right 1, lower left 2, lower right 3.                                *
 * The last four points lie on the far clipping plane, numbered analogous fashion.             *
 *		upper left 4, upper right 5, lower left 6, lower right 7.                                *
 * (remember: camera space has x going to the right, y up and z backwards).                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/16/2001  gth : Created.                                                                 *
 *=============================================================================================*/
inline const Vector3 * CameraClass::Get_View_Space_Frustum_Corners(void) const
{
	const FrustumClass & frustum = Get_View_Space_Frustum();
	return frustum.Corners;
}
