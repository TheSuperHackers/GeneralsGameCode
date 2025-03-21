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

/* $Header: /Commando/Code/ww3d/sr_util.h 16    5/09/00 1:10p Jani_p $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando / G 3D Library                                      * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/ww3d/sr_util.h                               $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 5/09/00 10:23a                                              $* 
 *                                                                                             * 
 *                    $Revision:: 16                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef SR_UTIL_H
#define SR_UTIL_H

#include "always.h"
#include "matrix3d.h"
#include "Vector3i.h"

#include <srVector3i.hpp>
#include <srVector3.hpp>
#include <srVector2.hpp>
#include <srMatrix4x3.hpp>

class srNode;
class srMeshModel;
class srCamera;
class srGERD;
class CameraClass;


/*
** Macros for setting and clearing a pointer to a surrender object.  There are 
** similar macros for our ref-counting system in refcount.h...
*/
#define SR_REF_PTR_SET(dst,src)	{ if (dst) dst->release(); dst = src; if(dst) dst->addReference(); }
#define SR_REF_PTR_RELEASE(x)		{ if (x) x->release(); x = NULL; }


/*
** PUSH_TRANSFORM, POP_TRANSFORM macros.  These are just some macros that push
** and pop a render object's transform in/out of the GERD.  I made them macros
** so that they could be inline without greatly increasing dependencies.
*/
#define PUSH_TRANSFORM(renderinfo,tm)	\
	srMatrix4x3 srtm;\
	Convert_Westwood_Matrix(tm,&srtm); \
	renderinfo.Gerd.matrixMode(srGERD::MODELVIEW); \
	renderinfo.Gerd.pushMultMatrix(srtm); 

#define POP_TRANSFORM(renderinfo) \
	renderinfo.Gerd.matrixMode(srGERD::MODELVIEW); \
	renderinfo.Gerd.popMatrix(); 


/*
** These functions convert between "westwood" (we use right-handed and column vectors)
** and "surrender" (left-handed, column vector) matrices.  Object and cameras are
** treated separately.
*/
void		Set_SR_Transform(srNode * obj,const Matrix3D & tm);
Matrix3D	Get_SR_Transform(srNode * obj);

void		Set_SR_Camera_Transform(srCamera * obj,const Matrix3D & transform);
Matrix3D	Get_SR_Camera_Transform(srCamera * obj);

/*
** This function is used to "pushmultiply" a W3D matrix into the given GERD
*/
void		Push_Multiply_Westwood_Matrix(srGERD * gerd,const Matrix3D & tm);

/*
** Functions for converting between Matrix3D's and srMatrix4's
** Only does the "type" conversion, no coordinate system changes...
*/
void Convert_Westwood_Matrix(const Matrix3D & wtm,srMatrix4 * set_sr_tm);
void Convert_Westwood_Matrix(const Matrix3D & wtm,srMatrix4d * set_sr_tm);
void Convert_Westwood_Matrix(const Matrix3D & wtm,srMatrix4x3 * set_sr_tm);
void Convert_Westwood_Matrix(const Matrix3D & wtm,srMatrix3 * set_sr_tm,srVector3 * set_sr_translation);
void Convert_Westwood_Matrix(const Matrix4 & wtm,srMatrix4 * set_sr_tm);

void Convert_Surrender_Matrix(const srMatrix4 & srtm,Matrix3D * set_w3d_tm);
void Convert_Surrender_Matrix(const srMatrix4x3 & srtm,Matrix3D * set_w3d_tm);

void Multiply_Westwood_And_Surrender_Matrix(const Matrix3D& w3d_tm,const srMatrix4& srtm_s,srMatrix4 & srtm_d);

/*
** This function will "convert" a pointer to an srVector3 into a pointer to a Vector3.
** Yes, this sucks.  In places where we are dealing with surrender vectors a lot,
** we should probably just use the surrender math functions. 
*/


inline Vector3 * As_Vector3(srVector3 * v) { return (Vector3 *)v; }
inline Vector3 & As_Vector3(srVector3 & v) { return (Vector3 &) v; }
inline srVector3 * As_srVector3(Vector3 * v) { return (srVector3 *)v; }
inline srVector3 & As_srVector3(Vector3 & v) { return (srVector3 &) v; }
inline const srVector3 * As_srVector3(const Vector3 * v) { return (const srVector3 *)v; }
inline const srVector3 & As_srVector3(const Vector3 & v) { return (const srVector3 &) v; }

/*
** Here's a set of equally sucky functions for dealing with srVector4 and Vector4's.
*/

inline Vector4 * As_Vector4(srVector4 * v) { return (Vector4 *)v; }
inline Vector4 & As_Vector4(srVector4 & v) { return (Vector4 &) v; }
inline srVector4 * As_srVector4(Vector4 * v) { return (srVector4 *)v; }
inline srVector4 & As_srVector4(Vector4 & v) { return (srVector4 &) v; }
inline const srVector4 * As_srVector4(const Vector4 * v) { return (const srVector4 *)v; }
inline const srVector4 & As_srVector4(const Vector4 & v) { return (const srVector4 &) v; }

/*
** More suckage, but now with 2's!
*/

inline Vector2 * As_Vector2(srVector2 * v) { return (Vector2 *)v; }
inline Vector2 & As_Vector2(srVector2 & v) { return (Vector2 &) v; }
inline srVector2 * As_srVector2(Vector2 * v) { return (srVector2 *)v; }
inline srVector2 & As_srVector2(Vector2 & v) { return (srVector2 &) v; }
inline const srVector2 * As_srVector2(const Vector2 * v) { return (const srVector2 *)v; }
inline const srVector2 & As_srVector2(const Vector2 & v) { return (const srVector2 &) v; }

/*
** Yahoo, here is some more.  If they break this we're gonna really be hurting!!!!
*/

inline Vector3i * As_Vector3i(srVector3i * v) { return (Vector3i *)v; }
inline Vector3i & As_Vector3i(srVector3i & v) { return (Vector3i &) v; }
inline srVector3i * As_srVector3i(Vector3i * v) { return (srVector3i *)v; }
inline srVector3i & As_srVector3i(Vector3i & v) { return (srVector3i &) v; }
inline const srVector3i * As_srVector3i(const Vector3i * v) { return (const srVector3i *)v; }
inline const srVector3i & As_srVector3i(const Vector3i & v) { return (const srVector3i &)v; }

/*
** This function returns the worldspace coordinates of the eight frustum
** corners of the given camera.
*/
void Get_Camera_Frustum_Corners(const CameraClass * camera, Vector3 points[8]);

/*
** This function returns the worldspace coordinates of the eight frustum
** corners of the given camera, with the depth (z distance) clamped to two
** given values.
*/
bool Get_ZClamped_Camera_Frustum_Corners(const CameraClass * camera,
	Vector3 points[8], float minz, float maxz);


// the SRMeshClass is used to store all the information neccessary to perform intersection 
// testing on a particular mesh. It is used by the Intersection class within WW3D, and
// elsewhere within G's planet mode.
// feel free to extend this to store other srTriMesh data members as needed 
class RenderObjClass;
typedef unsigned short POLYGONINDEX;

class SRMeshClass {
public:
	RenderObjClass *RenderObject; // what initialized this structure

	POLYGONINDEX PolygonCount;

	srVector3i *PolygonVertices;
	srVector4 *PolygonEquations;
	srVector3 *VertexLocations;
	srVector3 *VertexNormals;

	void Copy(SRMeshClass &info) {
		RenderObject = info.RenderObject;
		PolygonCount = info.PolygonCount;
		PolygonVertices = info.PolygonVertices;
		PolygonEquations = info.PolygonEquations;
		VertexLocations = info.VertexLocations;
		VertexNormals = info.VertexNormals;
	}

	void Initialize(RenderObjClass *renderObject, srMeshModel *meshmodel);
};

#endif
