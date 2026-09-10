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
 *                     $Archive:: /Commando/Code/ww3d2/AnimObj.h                              $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 12/10/01 11:18a                                             $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   W3DAnimatedModelRenderObject::Base_Update -- animation update function for the base pose          *
 *   W3DAnimatedModelRenderObject::Anim_Update -- Update function for a single animation               *
 *   W3DAnimatedModelRenderObject::Blend_Update -- update function for a blend of two animations       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
import Assets.Cache.Animations;


#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "WWLib/always.h"
#include "W3DDevice/GameClient/W3DModelGroupRenderObject.h"
import Graphics.Scene.Models.Hierarchy;
import Graphics.Scene.Models.Playback;

class SkinClass;
class W3DRenderContext;



/**
** W3DAnimatedModelRenderObject
** This class performs some of the work necessary to implement hierarchical animation.
** It implements much of the bone and animation interface of W3DRenderObject.
*/
class W3DAnimatedModelRenderObject : public W3DModelGroupRenderObject
{
public:

	W3DAnimatedModelRenderObject(const char * htree_name);
	W3DAnimatedModelRenderObject(const W3DAnimatedModelRenderObject & src);
	W3DAnimatedModelRenderObject & operator = (const W3DAnimatedModelRenderObject &);
	virtual ~W3DAnimatedModelRenderObject() override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Rendering
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(W3DRenderContext & rinfo) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - "Scene Graph"
	/////////////////////////////////////////////////////////////////////////////
	virtual void 					Set_Transform(const Matrix3D &m) override;
	virtual void 					Set_Position(const Vector3 &v) override;

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface - Hierarchical Animation
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Set_Animation() override;
	virtual void					Set_Animation( Assets::AnimationAssetHandle motion,
															float frame, int anim_mode = ANIM_MODE_MANUAL) override;
	virtual void					Set_Animation( Assets::AnimationAssetHandle motion0,
															float frame0,
															Assets::AnimationAssetHandle motion1,
															float frame1,
															float percentage) override;

	virtual void					Set_Animation_Frame_Rate_Multiplier(float multiplier);	// 020607 srj -- added

	virtual Assets::AnimationAssetHandle Peek_Animation_And_Info(float& frame, int& numFrames, int& mode, float& mult);	// 020710 srj -- added

	virtual Assets::AnimationAssetHandle Peek_Animation() override;
	virtual bool					Is_Animation_Complete() const;
	virtual int						Get_Num_Bones() override;
	virtual const char *			Get_Bone_Name(int bone_index) override;
	virtual int						Get_Bone_Index(const char * bonename) override;
	virtual Matrix3D 	Get_Bone_Transform(const char * bonename) override;
	virtual Matrix3D 	Get_Bone_Transform(int boneindex) override;
	virtual void					Capture_Bone(int boneindex) override;
	virtual void					Release_Bone(int boneindex) override;
	virtual bool					Is_Bone_Captured(int boneindex) const override;
	virtual void					Control_Bone(int bindex,const Matrix3D & objtm,bool world_space_translation = false) override;
	virtual const Graphics::ModelHierarchy *	Get_Model_Hierarchy() const override { return Hierarchy.get(); }

	//
	//	Simple bone evaluation methods for when the caller doesn't want
	// to update the hierarchy, but needs to know the transform of
	// a bone at a given frame.
	//
	virtual bool					Simple_Evaluate_Bone(int boneindex, Matrix3D *tm) const;
	virtual bool					Simple_Evaluate_Bone(int boneindex, float frame, Matrix3D *tm) const;

	// (gth) TESTING DYNAMICALLY SWAPPING SKELETONS!
	///Generals change so we can set sub-object transforms directly without having them revert to base pose
	///when marked dirty.  DON'T USE THIS UNLESS YOU HAVE A GOOD REASON! -MW
	void							Friend_Set_Hierarchy_Valid(bool onoff) const  	{ IsTreeValid = onoff; }

protected:

    virtual void Update_Sub_Object_Transforms() override;

	// flag to keep track of whether the hierarchy tree transforms are currently valid
	bool								Is_Hierarchy_Valid() const				{ return IsTreeValid; }
	void								Set_Hierarchy_Valid(bool onoff) const  	{ IsTreeValid = onoff; }

protected:

	// Is the hierarchy tree currently valid
	mutable bool  					IsTreeValid;

	// Hierarchy Tree
	std::unique_ptr<Graphics::ModelHierarchy> Hierarchy;

    Graphics::ModelPlayback Playback;
};
