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

/* $Header: /Commando/Code/ww3d2/pivot.cpp 1     1/22/01 3:36p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/pivot.cpp                              $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   PivotClass::PivotClass -- Constructor for PivotClass                                      *
 *   PivotClass::Compute_Transform -- Update the pivot's transformation matrix                 *
 *   PivotClass::Compute_Transform -- Update the pivot's transformation matrix                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "pivot.h"
#include "wwmath.h"
#include <string.h>


/***********************************************************************************************
 * PivotClass::PivotClass -- Constructor for PivotClass                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
PivotClass::PivotClass(void) :
	Parent(NULL),
	BaseTransform(1),
	Transform(1),
#ifdef LAZY_CAP_MTX_ALLOC
	CapTransformPtr(NULL),
	Index(0),
	IsVisible(true),
	WorldSpaceTranslation(false)
#else
	CapTransform(1),
	Index(0),
	IsVisible(true),
	WorldSpaceTranslation(false),
	IsCaptured(false),
	Unused(false)
#endif
{
	Name[0] = 0;
}

PivotClass::PivotClass(const PivotClass& that) :
	Parent(that.Parent),
	BaseTransform(that.BaseTransform),
	Transform(that.Transform),
#ifdef LAZY_CAP_MTX_ALLOC
	CapTransformPtr(NULL),
	Index(that.Index),
	IsVisible(that.IsVisible),
	WorldSpaceTranslation(that.WorldSpaceTranslation)
#else
	CapTransform(that.CapTransform),
	Index(that.Index),
	IsVisible(that.IsVisible),
	WorldSpaceTranslation(that.WorldSpaceTranslation),
	IsCaptured(that.IsCaptured),
	Unused(that.Unused)
#endif
{
	memcpy(Name, that.Name, sizeof(Name));
#ifdef LAZY_CAP_MTX_ALLOC
	if (that.CapTransformPtr != NULL)
	{
		CapTransformPtr = MSGW3DNEW("PivotClassCaptureBoneMtx") DynamicMatrix3D;
		CapTransformPtr->Mat = that.CapTransformPtr->Mat;
	}
#endif
}

PivotClass& PivotClass::operator=(const PivotClass& that)
{
	if (this != &that)
	{
		memcpy(Name, that.Name, sizeof(Name));
		Parent = that.Parent;
		BaseTransform = that.BaseTransform;
		Transform = that.Transform;
	#ifdef LAZY_CAP_MTX_ALLOC
		CapTransformPtr = NULL;
		Index = that.Index;
		IsVisible = that.IsVisible;
		WorldSpaceTranslation = that.WorldSpaceTranslation;
		if (that.CapTransformPtr != NULL)
		{
			CapTransformPtr = MSGW3DNEW("PivotClassCaptureBoneMtx") DynamicMatrix3D;
			CapTransformPtr->Mat = that.CapTransformPtr->Mat;
		}
	#else
		CapTransform = that.CapTransform;
		Index = that.Index;
		IsVisible = that.IsVisible;
		WorldSpaceTranslation = that.WorldSpaceTranslation;
		IsCaptured = that.IsCaptured;
		Unused = that.Unused;
	#endif
		}
	return *this;
}

void PivotClass::Capture_Update(void)
{
#ifdef LAZY_CAP_MTX_ALLOC
	if (!CapTransformPtr)
		return;

	const Matrix3D* ct = &CapTransformPtr->Mat;
#else
	const Matrix3D* ct = &CapTransform;
#endif

	if ( WorldSpaceTranslation )
	{
		// The Translation of CapTransform is meant to be in world space,
		// so remove before applying orientation
		Matrix3D CapOrientation = *ct;
		CapOrientation.Set_Translation( Vector3( 0,0,0 ) );
#ifdef ALLOW_TEMPORARIES
		Matrix3D::Multiply(Transform,CapOrientation,&(Transform));
#else
		Transform.postMul(CapOrientation);
#endif
		// Now apply translation in world space
		Transform.Adjust_Translation( ct->Get_Translation() );
	}
	else
	{
#ifdef ALLOW_TEMPORARIES
		Matrix3D::Multiply(Transform, *ct, &(Transform));
#else
		Transform.postMul(*ct);
#endif
	}
}

