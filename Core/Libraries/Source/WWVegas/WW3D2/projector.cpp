/*
**	Command & Conquer Generals Zero Hour(tm)
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
 *                     $Archive:: /Commando/Code/ww3d2/projector.cpp                          $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   ProjectorClass::ProjectorClass -- Constructor                                             *
 *   ProjectorClass::~ProjectorClass -- Destructor                                             *
 *   ProjectorClass::Set_Transform -- Set the transform for the projector                      *
 *   ProjectorClass::Get_Transform -- Returns the current transform                            *
 *   ProjectorClass::Set_Perspective_Projection -- Set up a perspective projection             *
 *   ProjectorClass::Set_Ortho_Projection -- Set up an orthographic projection                 *
 *   ProjectorClass::Compute_Texture_Coordinate -- computes texcoord for given world-space poi *
 *   ProjectorClass::Update_WS_Bounding_Volume -- Recalculate the world-space bounding box     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "projector.h"
#include "refcount.h"
#include "matrixmapper.h"


/***********************************************************************************************
 * ProjectorClass::ProjectorClass -- Constructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
ProjectorClass::ProjectorClass(void) :
	Transform(1),
	Projection(1),
	LocalBoundingVolume(Vector3(0,0,0),Vector3(1,1,1)),
	WorldBoundingVolume(Vector3(0,0,0),Vector3(1,1,1),Matrix3x3(1))
{
	Mapper=NEW_REF(MatrixMapperClass,(0));
}


/***********************************************************************************************
 * ProjectorClass::~ProjectorClass -- Destructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
ProjectorClass::~ProjectorClass(void)
{
	REF_PTR_RELEASE(Mapper);
}

/***********************************************************************************************
 * ProjectorClass::Set_Transform -- Set the transform for the projector                        *
 *                                                                                             *
 *    Projectors can be positioned in world space just like cameras!  Just point the -Z axis   *
 *    at the target.                                                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *   1/27/00    gth : Created.                                                                 *
 *=============================================================================================*/
void ProjectorClass::Set_Transform(const Matrix3D & tm)
{
	Transform = tm;
	Update_WS_Bounding_Volume();
}


/***********************************************************************************************
 * ProjectorClass::Get_Transform -- Returns the current transform                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
const Matrix3D & ProjectorClass::Get_Transform(void) const
{
	return Transform;
}


/***********************************************************************************************
 * ProjectorClass::Set_Perspective_Projection -- Set up a perspective projection               *
 *                                                                                             *
 * INPUT:                                                                                      *
 * hfov - radians, horizontal field of view of the projection                                  *
 * vfov - radians, vertical field of view of the projection                                    *
 * znear - distance to near clipping plane                                                     *
 * zfar - distance to far clipping plane                                                       *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Remember that znear and zfar are *distances*.  They are positive numbers as in the OpenGL   *
 * convention even though the coordinates will be negative.                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
void ProjectorClass::Set_Perspective_Projection(float hfov,float vfov,float znear,float zfar)
{
	Mapper->Set_Type(MatrixMapperClass::PERSPECTIVE_PROJECTION);
	Projection.Init_Perspective(hfov,vfov,0.1f,zfar);					// don't use znear for the projection matrix

	float tan_hfov2 = tan(hfov) * 0.5f;
	float tan_vfov2 = tan(vfov) * 0.5f;

	LocalBoundingVolume.Center.Set(0.0f,0.0f,-(zfar+znear)*0.5f);	// note, zcenter is negative
	LocalBoundingVolume.Extent.X = zfar * tan_hfov2;
	LocalBoundingVolume.Extent.Y = zfar * tan_vfov2;
	LocalBoundingVolume.Extent.Z = (zfar-znear)*0.5f;

	Update_WS_Bounding_Volume();
}


/***********************************************************************************************
 * ProjectorClass::Set_Ortho_Projection -- Set up an orthographic projection                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 * xmin - "left" x coordinate in texture (camera) space                                        *
 * xmax - "right" x coordinate                                                                 *
 * ymin - "bottom" y coordinate                                                                *
 * ymax - "top" y coordinate                                                                   *
 * znear - distance to near clipping plane                                                     *
 * zfar - distance to far clipping plane                                                       *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Remember that znear and zfar are *distances*.  They are positive numbers as in the OpenGL   *
 * convention even though the coordinates will be negative.                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
void ProjectorClass::Set_Ortho_Projection(float xmin,float xmax,float ymin,float ymax,float znear,float zfar)
{
	Mapper->Set_Type(MatrixMapperClass::ORTHO_PROJECTION);
	Projection.Init_Ortho(xmin,xmax,ymin,ymax,0.1f,zfar);			// don't use znear for the projection matrix

	LocalBoundingVolume.Center.Set((xmax+xmin)*0.5f, (ymax+ymin)*0.5f, -(zfar+znear)*0.5f);
	LocalBoundingVolume.Extent.Set((xmax-xmin)*0.5f, (ymax-ymin)*0.5f, (zfar-znear)*0.5f);

	Update_WS_Bounding_Volume();
}


/***********************************************************************************************
 * ProjectorClass::Compute_Texture_Coordinate -- computes texcoord for given world-space point *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/27/00    gth : Created.                                                                 *
 *=============================================================================================*/
void ProjectorClass::Compute_Texture_Coordinate(const Vector3 & point,Vector3 * set_stq)
{
	Mapper->Compute_Texture_Coordinate(point,set_stq);
}


/***********************************************************************************************
 * ProjectorClass::Update_WS_Bounding_Volume -- Recalculate the world-space bounding box       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/11/00    gth : Created.                                                                 *
 *=============================================================================================*/
void ProjectorClass::Update_WS_Bounding_Volume(void)
{
	/*
	** Recompute our world-space bounding volume
	*/
	OBBoxClass localbox(LocalBoundingVolume.Center,LocalBoundingVolume.Extent,Matrix3x3(1));
	OBBoxClass::Transform(Transform,localbox,&WorldBoundingVolume);
}

