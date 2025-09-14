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

/* $Header: /Commando/Code/ww3d2/htree.h 6     10/01/01 5:55p Patrick $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/htree.h                                $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 9/28/01 3:05p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef HTREE_H
#define HTREE_H

#include "always.h"
#include "pivot.h"
#include "quat.h"
#include "matrix3d.h"
#include "vector3.h"
#include "w3d_file.h"
#include "wwdebug.h"

class HAnimClass;
class HAnimComboClass;
class MeshClass;
class ChunkLoadClass;
class ChunkSaveClass;
class HRawAnimClass;

/*

	HTreeClass

	A hierarchy of coordinate systems in an initial
	configuration.  All motion data is applied to one
	of these objects.  Motion is stored as deltas from
	the hierarchy tree's initial configuration.

	Normally, the user will probably not deal with
	HTreeClasses; they are used internally
	by the HierarchyModelClass.

*/
class HTreeClass : public W3DMPO
{
	W3DMPO_GLUE(HTreeClass)
public:

	enum
	{
		OK,
		LOAD_ERROR
	};

	HTreeClass(void);
	HTreeClass(const HTreeClass & src);
	~HTreeClass(void);

	int					Load_W3D(ChunkLoadClass & cload);
	void					Init_Default(void);

	WWINLINE const char *		Get_Name(void)								const { return Name; }
	WWINLINE int					Num_Pivots(void)							const { return NumPivots; }
	int					Get_Bone_Index(const char * name)	const;
	const char *		Get_Bone_Name(int boneid)				const;
	int					Get_Parent_Index(int bone_indx)		const;

	void					Base_Update(const Matrix3D & root);

	void					Anim_Update(		const Matrix3D &		root,
													HAnimClass *			motion,
													float						frame);
	void					Anim_Update_Without_Interpolation(const Matrix3D & root,HRawAnimClass * motion,float frame);

	void					Blend_Update(		const Matrix3D &		root,
													HAnimClass *			motion0,
													float						frame0,
													HAnimClass *			motion1,
													float						frame1,
													float						percentage);

	void					Combo_Update(		const Matrix3D &		root,
													HAnimComboClass *		anim);

	WWINLINE const Matrix3D	&	Get_Transform(int pivot) const;
	WWINLINE bool					Get_Visibility(int pivot) const;

	WWINLINE const Matrix3D &	Get_Root_Transform(void) const;

	// User control over a bone.  While a bone is captured, you can over-ride the
	// animation transform used by the bone.
	void					Capture_Bone(int boneindex);
	void					Release_Bone(int boneindex);
	bool					Is_Bone_Captured(int boneindex) const;
	void					Control_Bone(int boneindex,const Matrix3D & relative_tm,bool world_space_translation = false);
	void					Get_Bone_Control(int boneindex, Matrix3D & relative_tm) const;



	//
	//	Simple pivot evaluation methods for when the caller doesn't want
	// to update the whole animation, but needs to know the transform of
	// a pivot at a given frame.
	//
	bool					Simple_Evaluate_Pivot (HAnimClass *motion, int pivot_index, float frame, const Matrix3D &obj_tm, Matrix3D *end_tm) const;
	bool					Simple_Evaluate_Pivot (int pivot_index, const Matrix3D &obj_tm, Matrix3D *end_tm) const;

	// Scale this HTree by a constant factor:
	void					Scale(float factor);

	// Morph the bones on the HTree based on the scale passed in
	static HTreeClass *	Alter_Avatar_HTree( const HTreeClass *tree, Vector3 &scale );

	// Morph the bones on the HTree using weights from a number of other HTrees
	static HTreeClass *	Create_Morphed( int num_morph_sources,
													 const float morph_weights[],
													 const HTreeClass *tree_array[] );

	// Create an HTree by Interpolating between others
	static HTreeClass	*	Create_Interpolated( const HTreeClass * tree_a0_b0,
														   const HTreeClass * tree_a0_b1,
														   const HTreeClass * tree_a1_b0,
														   const HTreeClass * tree_a1_b1,
														   float lerp_a, float lerp_b );

	// Create an HTree by Interpolating between others
	static HTreeClass	*	Create_Interpolated( const HTreeClass * tree_base,
														   const HTreeClass * tree_a,
														   const HTreeClass * tree_b,
														   float a_scale, float b_scale );

private:

	char					Name[W3D_NAME_LEN];
	int					NumPivots;
	PivotClass *		Pivot;
	float					ScaleFactor;

	void					Free(void);
	bool					read_pivots(ChunkLoadClass & cload,bool pre30);

	friend class MeshClass;



};

WWINLINE const Matrix3D &	HTreeClass::Get_Root_Transform(void) const
{
	return Pivot[0].Transform;
}

WWINLINE bool HTreeClass::Get_Visibility(int pivot) const
{
	WWASSERT(pivot >= 0);
	WWASSERT(pivot < NumPivots);
	return Pivot[pivot].IsVisible;
}

/***********************************************************************************************
 * HTreeClass::Get_Transform -- returns the transformation for the desired pivot               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WWINLINE const Matrix3D & HTreeClass::Get_Transform(int pivot) const
{
	assert(pivot >= 0);
	assert(pivot < NumPivots);

	return Pivot[pivot].Transform;
}



#endif
