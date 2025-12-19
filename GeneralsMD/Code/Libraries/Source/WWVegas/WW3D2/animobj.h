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
 *                     $Archive:: /Commando/Code/ww3d2/animobj.h                              $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 12/10/01 11:18a                                             $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Animatable3DObjClass::Base_Update -- animation update function for the base pose          *
 *   Animatable3DObjClass::Anim_Update -- Update function for a single animation               *
 *   Animatable3DObjClass::Blend_Update -- update function for a blend of two animations       *
 *   Animatable3DObjClass::Combo_Update -- Animation update for a combination of anims         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "composite.h"
#include "htree.h"
#include "hanim.h"

class SkinClass;
class RenderInfoClass;



/**
** Animatable3DObjClass
** This class performs some of the work necessary to implement hierarchical animation.
** It implements much of the bone and animation interface of RenderObjClass.
*/
class Animatable3DObjClass : public CompositeRenderObjClass
{
public:

	Animatable3DObjClass(const char * htree_name);
	Animatable3DObjClass(const Animatable3DObjClass & src);
	Animatable3DObjClass & operator = (const Animatable3DObjClass &);
	virtual ~Animatable3DObjClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Rendering
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - "Scene Graph"
	/////////////////////////////////////////////////////////////////////////////
	virtual void 					Set_Transform(const Matrix3D &m);
	virtual void 					Set_Position(const Vector3 &v);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Hierarchical Animation
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Set_Animation(void);
	virtual void					Set_Animation( HAnimClass * motion,
															float frame, int anim_mode = ANIM_MODE_MANUAL);
	virtual void					Set_Animation( HAnimClass * motion0,
															float frame0,
															HAnimClass * motion1,
															float frame1,
															float percentage);
	virtual void					Set_Animation( HAnimComboClass * anim_combo);

	virtual void					Set_Animation_Frame_Rate_Multiplier(float multiplier);	// 020607 srj -- added

	virtual HAnimClass *	Peek_Animation_And_Info(float& frame, int& numFrames, int& mode, float& mult);	// 020710 srj -- added

	virtual HAnimClass *			Peek_Animation( void );
	virtual bool					Is_Animation_Complete( void ) const;
	virtual int						Get_Num_Bones(void);
	virtual const char *			Get_Bone_Name(int bone_index);
	virtual int						Get_Bone_Index(const char * bonename);
	virtual const Matrix3D &	Get_Bone_Transform(const char * bonename);
	virtual const Matrix3D &	Get_Bone_Transform(int boneindex);
	virtual void					Capture_Bone(int boneindex);
	virtual void					Release_Bone(int boneindex);
	virtual bool					Is_Bone_Captured(int boneindex) const;
	virtual void					Control_Bone(int bindex,const Matrix3D & objtm,bool world_space_translation = false);
	virtual const HTreeClass *	Get_HTree(void) const { return HTree; }

	//
	//	Simple bone evaluation methods for when the caller doesn't want
	// to update the hierarchy, but needs to know the transform of
	// a bone at a given frame.
	//
	virtual bool					Simple_Evaluate_Bone(int boneindex, Matrix3D *tm) const;
	virtual bool					Simple_Evaluate_Bone(int boneindex, float frame, Matrix3D *tm) const;

	// (gth) TESTING DYNAMICALLY SWAPPING SKELETONS!
	virtual void					Set_HTree(HTreeClass * htree);
	///Generals change so we can set sub-object transforms directly without having them revert to base pose
	///when marked dirty.  DON'T USE THIS UNLESS YOU HAVE A GOOD REASON! -MW
	void							Friend_Set_Hierarchy_Valid(bool onoff) const  	{ IsTreeValid = onoff; }

protected:

	// internally used to compute the current frame if the object is in ANIM_MODE_MANUAL
	float								Compute_Current_Frame(float *newDirection=nullptr) const;

	// Update the sub-object transforms according to the current anim state and root transform.
	virtual	void					Update_Sub_Object_Transforms(void);

	// Update the transforms using the base pose only
	void								Base_Update(const Matrix3D & root);

	// Update the transforms using a single frame of motion data
	void								Anim_Update(	const Matrix3D & root,
															HAnimClass * motion,
															float frame);

	// Update the transforms blending two frames of motion data
	void								Blend_Update(	const Matrix3D & root,
															HAnimClass * motion0,
															float frame0,
															HAnimClass * motion1,
															float frame1,
															float percentage);

	// Update the transforms with an AnimationCombination
	void								Combo_Update(	const Matrix3D & root,
															HAnimComboClass *anim);

	// flag to keep track of whether the hierarchy tree transforms are currently valid
	bool								Is_Hierarchy_Valid(void) const				{ return IsTreeValid; }
	void								Set_Hierarchy_Valid(bool onoff) const  	{ IsTreeValid = onoff; }

	// Progress animations for single anim (loop and once)
	void								Single_Anim_Progress( void );

	// Release any animations
	void								Release( void );

protected:

	// Is the hierarchy tree currently valid
	mutable bool  					IsTreeValid;

	// Hierarchy Tree
	HTreeClass *					HTree;

	// Animation state for the next frame.  When we add more flexible motion
	// compositing, add a new state and its associated data to the union below
	enum {
		NONE = 0,
		BASE_POSE,
		SINGLE_ANIM,
		DOUBLE_ANIM,
		MULTIPLE_ANIM,
	};

	int								CurMotionMode;

	union {
		// CurMotionMode == SINGLE_ANIM
    struct {
			HAnimClass *			Motion;
			float		  				Frame;
			float						PrevFrame;
			int						AnimMode;
			int								LastSyncTime;
			float							animDirection;
			float							frameRateMultiplier;	// 020607 srj -- added
		} ModeAnim;

		// CurMotionMode == DOUBLE_ANIM
		struct {

			HAnimClass *			Motion0;
			HAnimClass *			Motion1;
			float		  				Frame0;
			float		  				Frame1;
			float						PrevFrame0;
			float						PrevFrame1;
			float		  				Percentage;
		} ModeInterp;

		// CurMotionMode == MULTIPLE_ANIM
		struct {
			HAnimComboClass *		AnimCombo;
		} ModeCombo;

	};

	friend class SkinClass;
};




/***********************************************************************************************
 * Animatable3DObjClass::Base_Update -- animation update function for the base pose            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/2/99     GTH : Created.                                                                 *
 *=============================================================================================*/
inline void Animatable3DObjClass::Base_Update(const Matrix3D & root)
{
	/*
	** This method simply puts the meshes in the base pose's configuration
	*/
	if (HTree) {
		HTree->Base_Update(root);
	}
	Set_Hierarchy_Valid(true);
}


/***********************************************************************************************
 * Animatable3DObjClass::Anim_Update -- Update function for a single animation                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/2/99     GTH : Created.                                                                 *
 *=============================================================================================*/
inline void Animatable3DObjClass::Anim_Update(const Matrix3D & root,HAnimClass * motion,float frame)
{
	/*
	** Apply motion to the base pose
	*/
	if ((motion) && (HTree)) {
#if !WW3D_ENABLE_RAW_ANIM_INTERPOLATION
		if (motion->Class_ID() == HAnimClass::CLASSID_HRAWANIM)
			HTree->Anim_Update_Without_Interpolation(root,(HRawAnimClass*)motion,frame);
		else
#endif
			HTree->Anim_Update(root,motion,frame);
	}
	Set_Hierarchy_Valid(true);
}


/***********************************************************************************************
 * Animatable3DObjClass::Blend_Update -- update function for a blend of two animations         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/2/99     GTH : Created.                                                                 *
 *=============================================================================================*/
inline void Animatable3DObjClass::Blend_Update
(
	const Matrix3D &	root,
	HAnimClass *		motion0,
	float					frame0,
	HAnimClass *		motion1,
	float					frame1,
	float					percentage
)
{
	/*
	** Apply motion to the base pose
	*/
	if (HTree) {
		HTree->Blend_Update(root,motion0,frame0,motion1,frame1,percentage);
	}
	Set_Hierarchy_Valid(true);
}


/***********************************************************************************************
 * Animatable3DObjClass::Combo_Update -- Animation update for a combination of anims           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/2/99     GTH : Created.                                                                 *
 *=============================================================================================*/
inline void Animatable3DObjClass::Combo_Update( const Matrix3D & root, HAnimComboClass *anim )
{
	if (HTree) {
		HTree->Combo_Update(root, anim);
	}
	Set_Hierarchy_Valid(true);
}
