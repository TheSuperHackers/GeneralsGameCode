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
 *                     $Archive:: /Commando/Code/ww3d2/camera.cpp                             $*
 *                                                                                             *
 *                    Org Author:: Greg_h                                                       *
 *                                                                                             *
 *                       $Author:: Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CameraClass::CameraClass -- constructor                                                   *
 *   CameraClass::CameraClass -- copy constructor                                              *
 *   CameraClass::operator == -- assignment operator                                           *
 *   CameraClass::~CameraClass -- destructor                                                   *
 *   CameraClass::Clone -- virtual copy constructor                                            *
 *   CameraClass::Add -- add the camera to the world?                                          *
 *   CameraClass::Remove -- Remove the camera from the world?                                  *
 *   CameraClass::Get_Obj_Space_Bounding_Sphere -- returns the object space bounding sphere    *
 *   CameraClass::Get_Object_Space_Bounding_Box -- returns the object space bounding box       *
 *   CameraClass::Set_Transform -- set the transform of the camera                             *
 *   CameraClass::Set_Position -- Set the position of the camera                               *
 *   CameraClass::Set_Animation -- set the animation state of the camera                       *
 *   CameraClass::Set_Animation -- Set the animation state of the camera                       *
 *   CameraClass::Set_Animation -- set the animation state of the camera                       *
 *   CameraClass::Set_Animation -- set the animation state of the camera                       *
 *   CameraClass::Set_View_Plane -- control over the view plane                                *
 *   CameraClass::Set_View_Plane -- set the viewplane using fov angles                         *
 *   CameraClass::Set_Focal_Length -- set the view plane using focal length and aspect ratio   *
 *   CameraClass::Get_View_Plane -- get the corners of the current view plane                  *
 *   CameraClass::Project -- project a point from ws to the view plane                         *
 *   CameraClass::Project_Camera_Space_Point -- Project a point that is already in camera spac *
 *   CameraClass::Un_Project -- "unproject" a point from the view plane to world space         *
 *   CameraClass::Transform_To_View_Space -- transforms the given world space point to camera  *
 *   CameraClass::Rotate_To_View_Space -- rotates the given world space vector to camera space *
 *   CameraClass::Get_Near_Clip_Bounding_Box -- returns an obb that contains near clip plane   *
 *   CameraCLass::Update_Frustum -- updates the frustum parameters                             *
 *   CameraClass::Cull_Box -- tests whether the given box can be culled                        *
 *   CameraClass::Device_To_View_Space -- converts the given device coordinate to view space   *
 *   CameraClass::Device_To_World_Space -- converts given device coord to world space          *
 *   CameraClass::Camera_Push -- pushes the camera's parameters into the given GERD            *
 *   CameraClass::Camera_Pop -- pops the camera's parameters from the given GERD               *
 *   CameraClass::Set_Aspect_Ratio -- sets the aspect ratio of the camera                      *
 *   CameraClass::Apply_D3D_State -- sets the D3D states controlled by the camera              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "camera.h"
#include "ww3d.h"
#include "matrix4.h"
#include "dx8wrapper.h"


/***********************************************************************************************
 * CameraClass::CameraClass -- constructor                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
CameraClass::CameraClass(void) :
	Projection(PERSPECTIVE),
	Viewport(Vector2(0,0),Vector2(1,1)),		// pixel viewport to render into
	AspectRatio(4.0f/3.0f),
	ZNear(1.0f),										// near clip plane distance
	ZFar(1000.0f),										// far clip plane distance
	ZBufferMin(0.0f),									// smallest value we'll write into the z-buffer
	ZBufferMax(1.0f),									// largest value we'll write into the z-buffer
	FrustumValid(false)
{
	Set_Transform(Matrix3D(1));
	Set_View_Plane(DEG_TO_RADF(50.0f));
}


/***********************************************************************************************
 * CameraClass::CameraClass -- copy constructor                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *   4/13/2001  hy : added in copy code for new member functions                               *
 *=============================================================================================*/
CameraClass::CameraClass(const CameraClass & src) : 
	RenderObjClass(src),
	Projection(src.Projection),
	Viewport(src.Viewport),
	ViewPlane(src.ViewPlane),
	ZNear(src.ZNear),
	ZFar(src.ZFar),
	FrustumValid(src.FrustumValid),
	Frustum(src.Frustum),
	NearClipBBox(src.NearClipBBox),
	ProjectionTransform(src.ProjectionTransform),
	CameraInvTransform(src.CameraInvTransform),
	AspectRatio(src.AspectRatio),
	ZBufferMin(src.ZBufferMin),
	ZBufferMax(src.ZBufferMax)
{
	// just being paraniod in case any parent class doesn't completely copy the entire state...
	FrustumValid = false;
}


/***********************************************************************************************
 * CameraClass::operator == -- assignment operator                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
CameraClass & CameraClass::operator = (const CameraClass & that)
{
	if (this != &that) {
		RenderObjClass::operator = (that);
	
		Projection = that.Projection;
		Viewport = that.Viewport;
		ViewPlane = that.ViewPlane;
		ZNear = that.ZNear;
		ZFar = that.ZFar;
		FrustumValid = that.FrustumValid;
		Frustum = that.Frustum;
		NearClipBBox = that.NearClipBBox;
		ProjectionTransform = that.ProjectionTransform;
		CameraInvTransform = that.CameraInvTransform;
		
		// just being paraniod in case any parent class doesn't completely copy the entire state...
		FrustumValid = false;
	}

	return * this;
}  	


/***********************************************************************************************
 * CameraClass::~CameraClass -- destructor                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
CameraClass::~CameraClass(void)
{
}


/***********************************************************************************************
 * CameraClass::Clone -- virtual copy constructor                                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
RenderObjClass * CameraClass::Clone(void) const
{
	return NEW_REF( CameraClass, (*this) );
}


/***********************************************************************************************
 * CameraClass::Get_Obj_Space_Bounding_Sphere -- returns the object space bounding sphere      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/29/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	sphere.Center.Set(0,0,0);
	sphere.Radius = ZFar;		// could optimize this but its not really used.
}


/***********************************************************************************************
 * CameraClass::Get_Object_Space_Bounding_Box -- returns the object space bounding box         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/29/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box.Center.Set(0,0,0);
	box.Extent.Set(ZFar,ZFar,ZFar);	// could optimize this but its not really used.
}


/***********************************************************************************************
 * CameraClass::Set_Transform -- set the transform of the camera                               *
 *                                                                                             *
 *    This is over-ridden to invalidate the cached frustum parameters                          *
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
void CameraClass::Set_Transform(const Matrix3D &m)	
{ 
	RenderObjClass::Set_Transform(m);
	FrustumValid = false; 
}


/***********************************************************************************************
 * CameraClass::Set_Position -- Set the position of the camera                                 *
 *                                                                                             *
 *    This is overriden to invalidate the cached frustum parameters                            *
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
void CameraClass::Set_Position(const Vector3 &v)
{ 
	RenderObjClass::Set_Position(v); 
	FrustumValid = false; 
}


/***********************************************************************************************
 * CameraClass::Set_View_Plane -- control over the view plane                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 * min - x,y of the upper left corner of the view rectangle (dist is assumed to be 1.0)        *
 * max - x,y of the lower right corner of the view rectangle                                   *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Set_View_Plane(const Vector2 & vmin,const Vector2 & vmax)
{
	ViewPlane.Min = vmin;
	ViewPlane.Max = vmax;
	AspectRatio = (vmax.X - vmin.X) / (vmax.Y - vmin.Y);
	FrustumValid = false;
}


/***********************************************************************************************
 * CameraClass::Set_View_Plane -- set the viewplane using fov angles                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Set_View_Plane(float hfov,float vfov)
{

	float width_half = tan(hfov/2.0);
	float height_half = 0.0f;
	
	if (vfov == -1) {									
		height_half = (1.0f / AspectRatio) * width_half;		// use the aspect ratio
	} else {
		height_half = tan(vfov/2.0);
		AspectRatio = width_half / height_half;					// or, initialize the aspect ratio
	}
	
	ViewPlane.Min.Set(-width_half,-height_half);
	ViewPlane.Max.Set(width_half,height_half);
	
	FrustumValid = false;
}


/***********************************************************************************************
 * CameraClass::Set_Aspect_Ratio -- sets the aspect ratio of the camera                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/29/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Set_Aspect_Ratio(float width_to_height)
{
	AspectRatio = width_to_height;
	ViewPlane.Min.Y = ViewPlane.Min.X / AspectRatio;
	ViewPlane.Max.Y = ViewPlane.Max.X / AspectRatio;
	FrustumValid = false;
}


/***********************************************************************************************
 * CameraClass::Get_View_Plane -- get the corners of the current view plane                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Get_View_Plane(Vector2 & set_min,Vector2 & set_max) const
{
	set_min = ViewPlane.Min;
	set_max = ViewPlane.Max;
}


/***********************************************************************************************
 * CameraClass::Project -- project a point from ws to the view plane                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 * dest - will be set to a point on the normalized view plane.  x,y, and z range from -1 to 1  *
 * ws_point - input point in world space                                                       *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * ProjectionResType indicating whether the point was in the frustum                           *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * When the input is behind the near clip plane, 0,0,0 is returned.                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
CameraClass::ProjectionResType CameraClass::Project(Vector3 & dest,const Vector3 & ws_point) const
{
	Update_Frustum();

	Vector3 cam_point;
	Matrix3D::Transform_Vector(CameraInvTransform,ws_point,&cam_point);

	if (cam_point.Z > -ZNear) {
		dest.Set(0,0,0);
		return OUTSIDE_NEAR_CLIP;
	}

	Vector4 view_point = ProjectionTransform * cam_point;
	float oow = 1.0f / view_point.W;
	dest.X = view_point.X * oow;
	dest.Y = view_point.Y * oow;
	dest.Z = view_point.Z * oow;

	if (dest.Z > 1.0f) {
		return OUTSIDE_FAR_CLIP;
	}

	if ((dest.X < -1.0f) || (dest.X > 1.0f) || (dest.Y < -1.0f) || (dest.Y > 1.0f)) {
		return OUTSIDE_FRUSTUM;
	}	

	return INSIDE_FRUSTUM;
}

/***********************************************************************************************
 * CameraClass::Project_Camera_Space_Point -- Project a point that is already in camera space  *
 *                                                                                             *
 * INPUT:                                                                                      *
 * dest - will be set to a point on the normalized view plane.  x,y, and z range from -1 to 1  *
 * cam_point - input point in camera space                                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * ProjectionResType indicating whether the point was in the frustum                           *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * When the input is behind the near clip plane, 0,0,0 is returned.                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/17/2000 gth : Created.                                                                 *
 *=============================================================================================*/
CameraClass::ProjectionResType 
CameraClass::Project_Camera_Space_Point(Vector3 & dest,const Vector3 & cam_point) const
{
	Update_Frustum();

	if ( cam_point.Z > -ZNear + WWMATH_EPSILON) {
		dest.Set(0,0,0);
		return OUTSIDE_NEAR_CLIP;
	}

	Vector4 view_point = ProjectionTransform * cam_point;
	float oow = 1.0f / view_point.W;
	dest.X = view_point.X * oow;
	dest.Y = view_point.Y * oow;
	dest.Z = view_point.Z * oow;

	if (dest.Z > 1.0f) {
		return OUTSIDE_FAR_CLIP;
	}

	if ((dest.X < -1.0f) || (dest.X > 1.0f) || (dest.Y < -1.0f) || (dest.Y > 1.0f)) {
		return OUTSIDE_FRUSTUM;
	}	

	return INSIDE_FRUSTUM;
}

/***********************************************************************************************
 * CameraClass::Un_Project -- "unproject" a point from the view plane to world space           *
 *                                                                                             *
 * The given point is assumed to be on the view_plane at z=-1.  The 3D world space point that  *
 * this represents will be returned in dest.                                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 * dest - will be filled in with the 3D world space point that the given point represents      *
 * view_point - point on the view_plane to be un-projected                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Un_Project(Vector3 & dest,const Vector2 & view_point) const
{
	/*
	** map view_point.X from -1..1 to ViewPlaneMin.X..ViewPlaneMax.X
	** map view_point.Y from -1..1 to ViewPlaneMin.X..ViewPlaneMax.X
	*/
	float vpdx = ViewPlane.Max.X - ViewPlane.Min.X;
	float vpdy = ViewPlane.Max.Y - ViewPlane.Min.Y;
	
	Vector3 point;
	point.X = ViewPlane.Min.X + vpdx * (view_point.X + 1.0f) * 0.5f;
	point.Y = ViewPlane.Min.Y + vpdy * (view_point.Y + 1.0f) * 0.5f;
	point.Z = -1.0f;

	Matrix3D::Transform_Vector(Transform,point,&dest);
}
	

/***********************************************************************************************
 * CameraClass::Transform_To_View_Space -- transforms the given world space point to camera sp *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/22/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Transform_To_View_Space(Vector3 & dest,const Vector3 & ws_point) const
{
	Update_Frustum();
	Matrix3D::Transform_Vector(CameraInvTransform,ws_point,&dest);
}
	

/***********************************************************************************************
 * CameraClass::Rotate_To_View_Space -- rotates the given world space vector to camera space   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/22/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Rotate_To_View_Space(Vector3 & dest,const Vector3 & ws_vector) const
{
	Update_Frustum();
	Matrix3D::Rotate_Vector(CameraInvTransform,ws_vector,&dest);
}



/***********************************************************************************************
 * CameraClass::Get_Near_Clip_Bounding_Box -- returns an obb that contains near clip plane     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/25/99    GTH : Created.                                                                 *
 *=============================================================================================*/
const OBBoxClass & CameraClass::Get_Near_Clip_Bounding_Box(void) const
{
	Update_Frustum();
	return NearClipBBox;	
}	


/***********************************************************************************************
 * CameraClass::Cull_Box -- tests whether the given box can be culled                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
bool CameraClass::Cull_Box(const AABoxClass & box) const
{
	const FrustumClass & frustum = Get_Frustum();
	return CollisionMath::Overlap_Test(frustum,box) == CollisionMath::OUTSIDE;
}


/***********************************************************************************************
 * CameraClass::Update_Frustum -- updates the frustum parameters                               *
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
void CameraClass::Update_Frustum(void) const
{
	if (FrustumValid) return;

   Vector2 vpmin,vpmax;
   float znear,zfar;
	float znear_dist,zfar_dist;

   Matrix3D cam_mat = Get_Transform();
   Get_View_Plane(vpmin, vpmax); // Normalized view plane at a depth of 1.0
   Get_Clip_Planes(znear_dist, zfar_dist);

   // Forward is negative Z in our viewspace coordinate system.
   znear = -znear_dist;
   zfar = -zfar_dist;

	// Update the frustum
	FrustumValid = true;
	Frustum.Init(cam_mat,vpmin,vpmax,znear,zfar);
	ViewSpaceFrustum.Init(Matrix3D(1),vpmin,vpmax,znear,zfar);

	// Update the OBB around the near clip rectangle
#ifdef ALLOW_TEMPORARIES
	NearClipBBox.Center = cam_mat * Vector3(0,0,znear);
#else
	cam_mat.mulVector3(Vector3(0,0,znear), NearClipBBox.Center);
#endif
	NearClipBBox.Extent.X = (vpmax.X - vpmin.X) * (-znear) * 0.5f;	// (near_clip_x / |znear|) == (vpmin.X / 1.0f)...
	NearClipBBox.Extent.Y = (vpmax.Y - vpmin.Y) * (-znear) * 0.5f;
	NearClipBBox.Extent.Z = 0.01f;
	NearClipBBox.Basis.Set(cam_mat);

	// Update the inverse camera matrix
	Transform.Get_Inverse(CameraInvTransform);

	// Update the projection matrix
	if (Projection == PERSPECTIVE) {

		ProjectionTransform.Init_Perspective(	vpmin.X*znear_dist, vpmax.X*znear_dist,
															vpmin.Y*znear_dist, vpmax.Y*znear_dist,
															znear_dist, zfar_dist );

	} else {
		
		ProjectionTransform.Init_Ortho( vpmin.X,vpmax.X,vpmin.Y,vpmax.Y,znear_dist,zfar_dist);

	}
}


/***********************************************************************************************
 * CameraClass::Device_To_View_Space -- converts the given device coordinate to view space     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Device_To_View_Space(const Vector2 & device_coord,Vector3 * set_view)
{
	int res_width;
	int res_height;
	int res_bits;
	bool windowed;

	WW3D::Get_Render_Target_Resolution(res_width,res_height,res_bits,windowed);

	// convert the device coordinates into normalized device coordinates:
	Vector2 ndev;
	ndev.X = device_coord.X / (float)res_width;
	ndev.Y = device_coord.Y / (float)res_height;

	// view space rectangle which corresponds to the viewport
	Vector2 vs_min;
	Vector2 vs_max;
	Get_View_Plane(vs_min,vs_max);
	
	// mapping from the viewport coordinates to view space coordinates
	set_view->X = vs_min.X + (ndev.X - Viewport.Min.X) * (vs_max.X - vs_min.X) / (Viewport.Width());
	set_view->Y = vs_max.Y - (ndev.Y - Viewport.Min.Y) * (vs_max.Y - vs_min.Y) / (Viewport.Height());
	set_view->Z = -1.0f;
}


/***********************************************************************************************
 * CameraClass::Device_To_World_Space -- converts given device coord to world space            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/8/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Device_To_World_Space(const Vector2 & device_coord,Vector3 * world_coord)
{
	Vector3 vs;
	Device_To_View_Space(device_coord,&vs);
	Matrix3D::Transform_Vector(Transform,vs,world_coord);
}


/***********************************************************************************************
 * CameraClass::Apply -- sets the D3D states controlled by the camera                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/29/2001  gth : Created.                                                                 *
 *=============================================================================================*/
void CameraClass::Apply(void)
{
	Update_Frustum();

	int width,height,bits;
	bool windowed;
	WW3D::Get_Render_Target_Resolution(width,height,bits,windowed);
	
	D3DVIEWPORT8 vp;
	vp.X = (DWORD)(Viewport.Min.X * (float)width);
	vp.Y = (DWORD)(Viewport.Min.Y * (float)height);
	vp.Width = (DWORD)((Viewport.Max.X - Viewport.Min.X) * (float)width);
	vp.Height = (DWORD)((Viewport.Max.Y - Viewport.Min.Y) * (float)height);
	vp.MinZ = ZBufferMin;
	vp.MaxZ = ZBufferMax;
	DX8Wrapper::Set_Viewport(&vp);

	Matrix4x4 d3dprojection;
	Get_D3D_Projection_Matrix(&d3dprojection);
	DX8Wrapper::Set_Transform(D3DTS_PROJECTION,d3dprojection);
	DX8Wrapper::Set_Transform(D3DTS_VIEW,CameraInvTransform);
}

void CameraClass::Set_Clip_Planes(float znear,float zfar)						
{ 
	FrustumValid = false;
	ZNear = znear;
	ZFar = zfar;
}

void CameraClass::Get_Clip_Planes(float & znear,float & zfar) const		
{ 
	znear = ZNear;
	zfar = ZFar;
}

float CameraClass::Get_Horizontal_FOV(void) const 
{ 
	float width = ViewPlane.Max.X - ViewPlane.Min.X;
	return 2*WWMath::Atan2(width,2.0);
}

float CameraClass::Get_Vertical_FOV(void) const 
{ 
	float height = ViewPlane.Max.Y - ViewPlane.Min.Y;
	return 2*WWMath::Atan2(height,2.0);
}

float CameraClass::Get_Aspect_Ratio(void) const 
{ 
	return AspectRatio;
}

void CameraClass::Get_Projection_Matrix(Matrix4x4 * set_tm)
{
	WWASSERT(set_tm != NULL);
	
	Update_Frustum();
	*set_tm = ProjectionTransform;
}

void CameraClass::Get_D3D_Projection_Matrix(Matrix4x4 * set_tm)
{
	WWASSERT(set_tm != NULL);
	Update_Frustum();
	*set_tm = ProjectionTransform;

	/*
	** We need to flip the handed-ness of the projection matrix and
	** move the z-range to 0<z<1 rather than -1<z<1
	*/
	float oozdiff = 1.0 / (ZFar - ZNear);
	if (Projection == PERSPECTIVE) {
		(*set_tm)[2][2] = -(ZFar) * oozdiff;
		(*set_tm)[2][3] = -(ZFar*ZNear) * oozdiff;
	} else {
		(*set_tm)[2][2] = -oozdiff;
		(*set_tm)[2][3] = -ZNear * oozdiff;
	}
}

void CameraClass::Get_View_Matrix(Matrix3D * set_tm)
{
	WWASSERT(set_tm != NULL);
	Update_Frustum();
	*set_tm = CameraInvTransform;
}

const Matrix4x4 & CameraClass::Get_Projection_Matrix(void)
{
	Update_Frustum();
	return ProjectionTransform;
}

const Matrix3D & CameraClass::Get_View_Matrix(void)
{
	Update_Frustum();
	return CameraInvTransform;
}

void CameraClass::Convert_Old(Vector3 &pos)
{
	pos.X=(pos.X+1)/2;
	pos.Y=(pos.Y+1)/2;
}

float CameraClass::Compute_Projected_Sphere_Radius(float dist,float radius)
{
	Vector4 result = ProjectionTransform * Vector4(radius,0.0f,dist,1.0f);
	return result.X / result.W;
}
