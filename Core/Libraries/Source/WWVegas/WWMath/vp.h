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
 *                 Project Name : wwmath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/WWMath/vp.h                                  $*
 *                                                                                             *
 *                     Org Author:: Hector Yee                                                  *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 13                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * Transform - transforms a vector array given  Matrix3D                                       *
 * Copy - Copies data from source to destination                                                *
 * CopyIndexed-copies dst[]=src[index[]]                                                        *
 * Clear - clears array to zero                                                                 *
 * Normalize - normalize the array                                                              *
 * MinMax - Finds the min and max of the array                                                  *
 *                                                                                              *
 *----------------------------------------------------------------------------------------------*
 */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef VECTORPROCESSOR_H
#define VECTORPROCESSOR_H

class Vector2;
class Vector3;
class Vector4;
class Matrix3D;
class Matrix4x4;

class VectorProcessorClass
{
public:
	static void Transform(Vector3* dst,const Vector3 *src, const Matrix3D& matrix, const int count);
	static void Transform(Vector4* dst,const Vector3 *src, const Matrix4x4& matrix, const int count);
	static void Copy(unsigned *dst,const unsigned *src, const int count);
	static void Copy(Vector2 *dst,const Vector2 *src, const int count);
	static void Copy(Vector3 *dst,const Vector3 *src, const int count);
	static void Copy(Vector4 *dst,const Vector4 *src, const int count);
	static void Copy(Vector4 *dst,const Vector3 *src, const float * srca, const int count);
	static void Copy(Vector4 *dst,const Vector3 *src, const float srca, const int count);
	static void Copy(Vector4 *dst,const Vector3 &src, const float * srca, const int count);
	static void CopyIndexed(unsigned *dst,const unsigned *src, const unsigned int *index, const int count);
	static void CopyIndexed(Vector2 *dst,const Vector2 *src, const unsigned int *index, const int count);
	static void CopyIndexed(Vector3 *dst,const Vector3 *src, const unsigned int *index, const int count);
	static void CopyIndexed(Vector4 *dst,const Vector4 *src, const unsigned int *index, const int count);
	static void CopyIndexed(unsigned char* dst, const unsigned char* src, const unsigned int *index, int count);
	static void CopyIndexed(float* dst, float* src, const unsigned int *index, int count);
	static void Clamp(Vector4 *dst,const Vector4 *src, const float min, const float max, const int count);
	static void Clear (Vector3 *dst, const int count);
	static void Normalize(Vector3 *dst, const int count);
	static void MinMax(Vector3 *src, Vector3 &min, Vector3 &max, const int count);

	static void MulAdd(float * dest,float multiplier,float add,int count);

	static void Prefetch(void* address);

	static void DotProduct(float *dst, const Vector3 &a, const Vector3 *b,const int count);
	static void ClampMin(float *dst, float *src, const float min, const int count);
	static void Power(float *dst, float *src, const float pow, const int count);
};

#endif // VECTORPROCESSOR_H
