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
 *                     $Archive:: /Commando/Code/ww3d2/animobj.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 12/13/01 6:56p                                              $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "animobj.h"
#include "htree.h"
#include "assetmgr.h"
#include "hanim.h"
#include "hcanim.h"
#include "ww3d.h"
#include "WWDebug/wwmemlog.h"
#include "animatedsoundmgr.h"


Animatable3DObjClass::Animatable3DObjClass(const char * htree_name) :
	IsTreeValid(0),
	CurMotionMode(BASE_POSE)
{
	// Inline struct members can't be initialized in init list for some reason...
  ModeAnim.Motion=nullptr;
	ModeAnim.Frame=0.0f;
	ModeAnim.PrevFrame=0.0f;
	ModeAnim.LastSyncTime=WW3D::Get_Logic_Time_Milliseconds();
	ModeAnim.frameRateMultiplier=1.0;	// 020607 srj -- added
	ModeAnim.animDirection=1.0;	// 020607 srj -- added
	ModeInterp.Motion0=nullptr;
	ModeInterp.Motion1=nullptr;
	ModeInterp.Frame0=0.0f;
	ModeInterp.PrevFrame0=0.0f;
	ModeInterp.PrevFrame1=0.0f;
	ModeInterp.Frame1=0.0f;
	ModeInterp.Percentage=0.0f;
	ModeCombo.AnimCombo=nullptr;

	/*
	** Store a pointer to the htree
	*/
	if (htree_name == nullptr) {
		HTree = nullptr;
	} else if (htree_name[0] == 0) {
		HTree = W3DNEW HTreeClass;
		HTree->Init_Default ();
	} else {
		HTreeClass * source = WW3DAssetManager::Get_Instance()->Get_HTree(htree_name);
		if (source != nullptr) {
			HTree = W3DNEW HTreeClass(*source);
		} else {
			WWDEBUG_SAY(("Unable to find HTree: %s",htree_name));
			HTree = W3DNEW HTreeClass;
			HTree->Init_Default();
		}
	}
}


Animatable3DObjClass::Animatable3DObjClass(const Animatable3DObjClass & src) :
	CompositeRenderObjClass(src),
	IsTreeValid(0),
	CurMotionMode(BASE_POSE),
	HTree(nullptr)
{
   // Inline struct members can't be initialized in init list for some reason...
	ModeAnim.Motion=nullptr;
	ModeAnim.Frame=0.0f;
	ModeAnim.PrevFrame=0.0f;
	ModeAnim.LastSyncTime=WW3D::Get_Logic_Time_Milliseconds();
	ModeAnim.frameRateMultiplier=1.0;	// 020607 srj -- added
	ModeAnim.animDirection=1.0;	// 020607 srj -- added
	ModeInterp.Motion0=nullptr;
	ModeInterp.Motion1=nullptr;
	ModeInterp.Frame0=0.0f;
	ModeInterp.PrevFrame0=0.0f;
	ModeInterp.PrevFrame1=0.0f;
	ModeInterp.Frame1=0.0f;
	ModeInterp.Percentage=0.0f;
	ModeCombo.AnimCombo=nullptr;

	*this = src;
}


Animatable3DObjClass::~Animatable3DObjClass()
{
	Release();

	delete HTree;
}


Animatable3DObjClass & Animatable3DObjClass::operator = (const Animatable3DObjClass & that)
{
	if (&that != this) {
		Release();

		CompositeRenderObjClass::operator = (that);

		IsTreeValid = 0;
		CurMotionMode = BASE_POSE;
		ModeAnim.Motion = nullptr;
		ModeAnim.Frame = 0.0f;
		ModeAnim.PrevFrame = 0.0f;
		ModeAnim.LastSyncTime = WW3D::Get_Logic_Time_Milliseconds();
		ModeAnim.frameRateMultiplier=1.0;	// 020607 srj -- added
		ModeAnim.animDirection=1.0;	// 020607 srj -- added
		ModeInterp.Motion0 = nullptr;
		ModeInterp.Motion1 = nullptr;
		ModeInterp.Frame0 = 0.0f;
		ModeInterp.PrevFrame0 = 0.0f;
		ModeInterp.PrevFrame1 = 0.0f;
		ModeInterp.Frame1 = 0.0f;
		ModeInterp.Percentage = 0.0f;
		ModeCombo.AnimCombo = nullptr;

		delete HTree;
		HTree = W3DNEW HTreeClass(*that.HTree);
	}
	return *this;
}

// Releases any anims being held by this object.
void Animatable3DObjClass::Release()
{
	switch (CurMotionMode) {

		case BASE_POSE:
			break;

		case SINGLE_ANIM:
			if ( ModeAnim.Motion != nullptr ) {
				ModeAnim.Motion->Release_Ref();
				ModeAnim.Motion = nullptr;
			}
			break;

		case DOUBLE_ANIM:
			if ( ModeInterp.Motion0 != nullptr ) {
				ModeInterp.Motion0->Release_Ref();
				ModeInterp.Motion0 = nullptr;
			}

			if ( ModeInterp.Motion1 != nullptr ) {
				ModeInterp.Motion1->Release_Ref();
				ModeInterp.Motion1 = nullptr;
			}
			break;

		case MULTIPLE_ANIM:
			break;

		default:
			break;
	}
}

void Animatable3DObjClass::Render(RenderInfoClass & rinfo)
{
	if (HTree == nullptr) return;

	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	//
	// Force the hierarchy to be recalculated for single animations.
	//
	const bool isSingleAnim = CurMotionMode == SINGLE_ANIM && ModeAnim.AnimMode != ANIM_MODE_MANUAL;

	if (isSingleAnim || !Is_Hierarchy_Valid() || Are_Sub_Object_Transforms_Dirty()) {
		Update_Sub_Object_Transforms();
	}
}

void Animatable3DObjClass::Special_Render(SpecialRenderInfoClass & rinfo)
{
	if (HTree == nullptr) return;

	//
	// Force the hierarchy to be recalculated for single animations.
	//
	const bool isSingleAnim = CurMotionMode == SINGLE_ANIM && ModeAnim.AnimMode != ANIM_MODE_MANUAL;

	if (isSingleAnim || !Is_Hierarchy_Valid()) {
		Update_Sub_Object_Transforms();
	}
}


// Also marks the sub-objects as dirty.
void Animatable3DObjClass::Set_Transform(const Matrix3D &m)
{
	CompositeRenderObjClass::Set_Transform(m);
	Set_Hierarchy_Valid(false);
}


// Also marks the sub-objects as dirty.
void Animatable3DObjClass::Set_Position(const Vector3 &v)
{
	CompositeRenderObjClass::Set_Position(v);
	Set_Hierarchy_Valid(false);
}


int Animatable3DObjClass::Get_Num_Bones()
{
	if (HTree) {
		return HTree->Num_Pivots();
	} else {
		return 1;
	}
}


const char * Animatable3DObjClass::Get_Bone_Name(int bone_index)
{
	if (HTree) {
		return HTree->Get_Bone_Name(bone_index);
	} else {
		return "RootTransform";
	}
}


int Animatable3DObjClass::Get_Bone_Index(const char * bonename)
{
	if (HTree) {
		return HTree->Get_Bone_Index(bonename);
	} else {
		return 0;
	}
}



// Sets the animation state to the base pose.
void Animatable3DObjClass::Set_Animation()
{
	Release();
	CurMotionMode = BASE_POSE;
	Set_Hierarchy_Valid(false);
}


void Animatable3DObjClass::Set_Animation(HAnimClass * motion, float frame, int mode)
{

	if ( motion ) {
		// Add_Ref before we remove, in case it is the same one.
		motion->Add_Ref();
		Release();
		CurMotionMode = SINGLE_ANIM;
		ModeAnim.Motion = motion;
		ModeAnim.PrevFrame = ModeAnim.Frame;
		ModeAnim.Frame = frame;
		ModeAnim.LastSyncTime = WW3D::Get_Logic_Time_Milliseconds();
		ModeAnim.frameRateMultiplier=1.0;	// 020607 srj -- added
		ModeAnim.animDirection=1.0;	// 020607 srj -- added

		ModeAnim.AnimMode = mode;

		if (mode < ANIM_MODE_LOOP_BACKWARDS)
			ModeAnim.animDirection = 1.0f;	//assume playing forwards
		else
			ModeAnim.animDirection = -1.0f;	//reverse animation playback

		const char* sound_name = AnimatedSoundMgrClass::Get_Embedded_Sound_Name(motion);
		if (sound_name) {
			int bone_index = Get_Bone_Index(sound_name);
			motion->Set_Embedded_Sound_Bone_Index(bone_index);
		}
	} else {
		CurMotionMode = BASE_POSE;
		Release();
	}

	Set_Hierarchy_Valid(false);
}

void Animatable3DObjClass::Set_Animation
(
	HAnimClass * motion0,
	float frame0,
	HAnimClass * motion1,
	float frame1,
	float percentage
)
{
	Release();

	CurMotionMode = DOUBLE_ANIM;
	ModeInterp.Motion0 = motion0;
	ModeInterp.Motion1 = motion1;
	ModeInterp.PrevFrame0 = ModeInterp.Frame0;
	ModeInterp.PrevFrame1 = ModeInterp.Frame1;
	ModeInterp.Frame0 = frame0;
	ModeInterp.Frame1 = frame1;
	ModeInterp.Percentage = percentage;
	Set_Hierarchy_Valid(false);

	if ( ModeInterp.Motion0 != nullptr ) {
		ModeInterp.Motion0->Add_Ref();
		const char* sound_name = AnimatedSoundMgrClass::Get_Embedded_Sound_Name(motion0);
		if (sound_name) {
			int bone_index = Get_Bone_Index(sound_name);
			motion0->Set_Embedded_Sound_Bone_Index(bone_index);
		}
	}

	if ( ModeInterp.Motion1 != nullptr ) {
		ModeInterp.Motion1->Add_Ref();
		const char* sound_name = AnimatedSoundMgrClass::Get_Embedded_Sound_Name(motion1);
		if (sound_name) {
			int bone_index = Get_Bone_Index(sound_name);
			motion1->Set_Embedded_Sound_Bone_Index(bone_index);
		}
	}
}


void Animatable3DObjClass::Set_Animation
(
	HAnimComboClass * anim_combo
)
{
	Release();

	CurMotionMode = MULTIPLE_ANIM;
	ModeCombo.AnimCombo = anim_combo;
	Set_Hierarchy_Valid(false);

	if (anim_combo) {
		int count = anim_combo->Get_Num_Anims();
		for (int index = 0; index < count; index ++) {
			HAnimClass *motion = anim_combo->Peek_Motion(index);

			const char* sound_name = AnimatedSoundMgrClass::Get_Embedded_Sound_Name(motion);
			if (sound_name) {
				int bone_index = Get_Bone_Index(sound_name);
				motion->Set_Embedded_Sound_Bone_Index(bone_index);
			}
		}
	}
}


HAnimClass *	Animatable3DObjClass::Peek_Animation()
{
	if ( CurMotionMode == SINGLE_ANIM ) {
		return ModeAnim.Motion;
	} else {
		return nullptr;
	}
}


const Matrix3D &	Animatable3DObjClass::Get_Bone_Transform(const char * bonename)
{
	if (HTree) {
		WWASSERT(HTree);
		WWASSERT(bonename);

		int idx = HTree->Get_Bone_Index(bonename);
		return Get_Bone_Transform(idx);
	} else {
		return Get_Transform();
	}
}


const Matrix3D &	Animatable3DObjClass::Get_Bone_Transform(int boneindex)
{
	Validate_Transform();

	if (HTree) {
		/*
		** If our hierarchy isn't valid, we just need to evaluate our animation
		** state.
		*/
		if (!Is_Hierarchy_Valid()) {
			Update_Sub_Object_Transforms();
		}

		return HTree->Get_Transform(boneindex);
	} else {
		return Transform;
	}
}


// The captured bone overrides the animation.
void Animatable3DObjClass::Capture_Bone(int boneindex)
{
	if (HTree) {
		HTree->Capture_Bone(boneindex);
	}
}


// The released bone follows the animation again.
void Animatable3DObjClass::Release_Bone(int boneindex)
{
	if (HTree) {
		HTree->Release_Bone(boneindex);
	}
}


bool Animatable3DObjClass::Is_Bone_Captured(int boneindex) const
{
	if (HTree) {
		return HTree->Is_Bone_Captured(boneindex);
	} else {
		return false;
	}
}


void Animatable3DObjClass::Control_Bone(int bindex,const Matrix3D & objtm,bool world_space_translation)
{
#ifdef DEBUG_CRASHING
	for (int j=0; j<3; j++) {
		for (int i=0; i<4; i++) {
			WWASSERT(WWMath::Is_Valid_Float(objtm[j][i]));
		}
	}
#endif

	if (HTree) {
		HTree->Control_Bone(bindex,objtm,world_space_translation);
		Set_Hierarchy_Valid(false);
	}
}

void Animatable3DObjClass::Update_Sub_Object_Transforms()
{
	/*
	** The RenderObj implementation will cause our 'container'
	** to update if we are not valid yet
	*/
	CompositeRenderObjClass::Update_Sub_Object_Transforms();

	/*
	** Update the transforms
	*/
	switch (CurMotionMode) {

		case BASE_POSE:
			Base_Update(Transform);
			break;

		case SINGLE_ANIM:

			if ( ModeAnim.AnimMode != ANIM_MODE_MANUAL ) {
				Single_Anim_Progress();
			}
			Anim_Update(Transform,ModeAnim.Motion,ModeAnim.Frame);

			/*
			**	Play any sounds that are triggered by this frame of animation
			*/
			if ( ModeAnim.Motion->Has_Embedded_Sounds() ) {
				ModeAnim.PrevFrame = AnimatedSoundMgrClass::Trigger_Sound(ModeAnim.Motion, ModeAnim.PrevFrame, ModeAnim.Frame, HTree->Get_Transform(ModeAnim.Motion->Get_Embedded_Sound_Bone_Index()));
			}
			break;

		case DOUBLE_ANIM:
			Blend_Update(Transform,ModeInterp.Motion0,ModeInterp.Frame0,
				ModeInterp.Motion1,ModeInterp.Frame1,ModeInterp.Percentage);

			/*
			**	Play any sounds that are triggered by this frame of animation
			*/
			if ( ModeInterp.Motion0->Has_Embedded_Sounds() ) {
				ModeInterp.PrevFrame0 = AnimatedSoundMgrClass::Trigger_Sound(ModeInterp.Motion0, ModeInterp.PrevFrame0, ModeInterp.Frame0, HTree->Get_Transform(ModeInterp.Motion0->Get_Embedded_Sound_Bone_Index()));
			}

			if ( ModeInterp.Motion1->Has_Embedded_Sounds() ) {
				ModeInterp.PrevFrame1 = AnimatedSoundMgrClass::Trigger_Sound(ModeInterp.Motion1, ModeInterp.PrevFrame1, ModeInterp.Frame1, HTree->Get_Transform(ModeInterp.Motion1->Get_Embedded_Sound_Bone_Index()));
			}

			break;

		case MULTIPLE_ANIM:
		{
			Combo_Update(Transform,ModeCombo.AnimCombo);

			/*
			**	Play any sounds that are triggered by this frame of animation
			*/
			int count = ModeCombo.AnimCombo->Get_Num_Anims();
			for (int index = 0; index < count; index ++) {
				HAnimClass *motion = ModeCombo.AnimCombo->Peek_Motion(index);

				if ( motion != nullptr && motion->Has_Embedded_Sounds() ) {
					float prev_frame = AnimatedSoundMgrClass::Trigger_Sound(motion, ModeCombo.AnimCombo->Get_Prev_Frame(index),
																				ModeCombo.AnimCombo->Get_Frame(index), HTree->Get_Transform(motion->Get_Embedded_Sound_Bone_Index()));
					ModeCombo.AnimCombo->Set_Prev_Frame(index, prev_frame);
				}

			}
			break;
		}

		default:
			break;
	}
	Set_Hierarchy_Valid(true);
}


// If the animation is 'single', evaluate the given pivot and return its transform.
bool Animatable3DObjClass::Simple_Evaluate_Bone(int boneindex, Matrix3D *tm) const
{
	bool retval = false;

	//
	//	Only do this for simple animations
	//
	if (	CurMotionMode == NONE ||
			CurMotionMode == BASE_POSE ||
			CurMotionMode == SINGLE_ANIM)
	{
		//
		//	Determine which frame we should be on, then use this
		// information to determine the bone's transform.
		//
		float curr_frame = Compute_Current_Frame ();
		retval = Simple_Evaluate_Bone (boneindex, curr_frame, tm);

	} else {

		const_cast <Animatable3DObjClass *>(this)->Update_Sub_Object_Transforms();
		*tm = HTree->Get_Transform(boneindex);

	}

	return retval;
}


// If the animation is 'single', evaluate the given pivot and return its transform.
bool Animatable3DObjClass::Simple_Evaluate_Bone(int boneindex, float frame, Matrix3D *tm) const
{
	bool retval = false;

	//
	//	Only do this for simple animations
	//
	if (HTree != nullptr) {

		if (CurMotionMode == SINGLE_ANIM) {
			retval = HTree->Simple_Evaluate_Pivot (ModeAnim.Motion, boneindex, frame, Get_Transform (), tm);
		} else if (CurMotionMode == NONE || CurMotionMode == BASE_POSE) {
			retval = HTree->Simple_Evaluate_Pivot (boneindex, Get_Transform (), tm);
		} else {
			*tm = Transform;
		}

	} else {
		*tm = Transform;
	}

	return retval;
}


// Returns the animation frame for the next render.
// WARNING: Only works for Single and CSingle!
float Animatable3DObjClass::Compute_Current_Frame(float *newDirection) const
{
	float frame = 0;
	float direction = ModeAnim.animDirection;

	switch (CurMotionMode)
	{
		case SINGLE_ANIM:
		{
			frame = ModeAnim.Frame;

			if (ModeAnim.AnimMode != ANIM_MODE_MANUAL) {
				//
				//	Compute the current frame based on elapsed time.
				//	TheSuperHackers @info Is using elapsed time because frame computation is not guaranteed to be called every render frame!
				//
				// TheSuperHackers @tweak The animation render update is now decoupled from the logic step.
				const float syncMilliseconds = WW3D::Get_Logic_Time_Milliseconds() - ModeAnim.LastSyncTime;
				const float animMilliseconds = ModeAnim.Motion->Get_Frame_Rate() * ModeAnim.frameRateMultiplier * ModeAnim.animDirection * syncMilliseconds;
				const float animSeconds = animMilliseconds * 0.001f;
				frame += animSeconds;

				//
				//	Wrap the frame
				//
				const int numFrames = ModeAnim.Motion->Get_Num_Frames() - 1;

				switch (ModeAnim.AnimMode)
				{
					case ANIM_MODE_ONCE:
						if (frame >= numFrames) {
							frame = numFrames;
						}
						break;
					case ANIM_MODE_LOOP:
						if ( frame >= numFrames ) {
							frame -= numFrames;
							// If it is still too far out, reset
							if ( frame >= numFrames ) {
								frame = 0;
							}
						}
						break;
					case ANIM_MODE_ONCE_BACKWARDS:	//play animation one time but backwards
						if (frame < 0) {
							frame = 0;
						}
						break;
					case ANIM_MODE_LOOP_BACKWARDS:	//play animation backwards in a loop
						if ( frame < 0 ) {
							frame += numFrames;
							// If it is still too far out, reset
							if ( frame < 0 ) {
								frame = numFrames;
							}
						}
						break;
					case ANIM_MODE_LOOP_PINGPONG:
						if (ModeAnim.animDirection >= 1.0f)
						{
							//playing forwards, reverse direction
							if (frame >= numFrames)
							{
								//step backwards in animation by excess time
								frame = numFrames * 2 - frame;
								// If it is still too far out, reset
								if ( frame >= numFrames - 1 )
									frame = numFrames;
								direction = ModeAnim.animDirection * -1.0f;
							}
						}
						else
						{
							//playing backwards, reverse direction
							if (frame < 0)
							{
								//step forwards in animation by excess time
								frame = -frame;
								// If it is still too far out, reset
								if ( frame >= numFrames )
										frame = 0;
								direction = ModeAnim.animDirection * -1.0f;
							}
						}
						break;
				}
			}
		}
		break;
	}

	if (newDirection)
		*newDirection = direction;
	return frame;
}

// Progress anims for loop and once.
// WARNING: Only works for Single and CSingle.
void Animatable3DObjClass::Single_Anim_Progress ()
{
	//
	//	Update the current frame (only works in "SINGLE_ANIM" mode!)
	//
	WWASSERT(CurMotionMode == SINGLE_ANIM);

	//
	// Update the frame number and sync time
	//
	ModeAnim.PrevFrame		= ModeAnim.Frame;
	ModeAnim.Frame				= Compute_Current_Frame(&ModeAnim.animDirection);
	ModeAnim.LastSyncTime	= WW3D::Get_Logic_Time_Milliseconds();
}


// Is the current animation on the last frame?
// WARNING: Only works for Single, ONCE anims.
bool	Animatable3DObjClass::Is_Animation_Complete() const
{
	if (CurMotionMode == SINGLE_ANIM) {

		if ( ModeAnim.AnimMode == ANIM_MODE_ONCE ) {
			return ( ModeAnim.Frame == ModeAnim.Motion->Get_Num_Frames() - 1 );
		}
		else
		if ( ModeAnim.AnimMode == ANIM_MODE_ONCE_BACKWARDS)
		{	return ( ModeAnim.Frame == 0);
		}
	}
	return false;
}

HAnimClass * Animatable3DObjClass::Peek_Animation_And_Info(float& frame, int& numFrames, int& mode, float& mult)
{
	if ( CurMotionMode == SINGLE_ANIM ) {
		frame = ModeAnim.Frame;
		numFrames = ModeAnim.Motion ? ModeAnim.Motion->Get_Num_Frames() : 0;
		mode = ModeAnim.AnimMode;
		mult = ModeAnim.frameRateMultiplier;
		return ModeAnim.Motion;
	} else {
		return nullptr;
	}
}

void Animatable3DObjClass::Set_Animation_Frame_Rate_Multiplier(float multiplier)
{
	// 020607 srj -- added
	ModeAnim.frameRateMultiplier = multiplier;
}

// (gth) TESTING DYNAMICALLY SWAPPING SKELETONS!

void Animatable3DObjClass::Set_HTree(HTreeClass * new_htree)
{
	WWMEMLOG(MEM_ANIMATION);
	// try to ensure that the htree we're using has the same structure...
	WWASSERT(new_htree->Num_Pivots() == HTree->Num_Pivots());

	// just assign it...
	delete HTree;
	HTree = W3DNEW HTreeClass(*new_htree);
}


// EOF - animobj.cpp
