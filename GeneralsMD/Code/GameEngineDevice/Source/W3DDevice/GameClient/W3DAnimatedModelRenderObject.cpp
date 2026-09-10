import Graphics.Frame.RenderClock;
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

#include "W3DDevice/GameClient/W3DAnimatedModelRenderObject.h"
import Graphics.Scene.Models.Hierarchy;
#include "W3DDevice/GameClient/W3DAssetCatalog.h"

#include "WWDebug/wwmemlog.h"
import Assets.Cache.Animations;

static_assert(static_cast<int>(Graphics::ModelPlaybackMode::Manual) == W3DRenderObject::ANIM_MODE_MANUAL);
static_assert(static_cast<int>(Graphics::ModelPlaybackMode::Loop) == W3DRenderObject::ANIM_MODE_LOOP);
static_assert(static_cast<int>(Graphics::ModelPlaybackMode::Once) == W3DRenderObject::ANIM_MODE_ONCE);
static_assert(static_cast<int>(Graphics::ModelPlaybackMode::PingPong) == W3DRenderObject::ANIM_MODE_LOOP_PINGPONG);
static_assert(static_cast<int>(Graphics::ModelPlaybackMode::LoopBackwards) == W3DRenderObject::ANIM_MODE_LOOP_BACKWARDS);
static_assert(static_cast<int>(Graphics::ModelPlaybackMode::OnceBackwards) == W3DRenderObject::ANIM_MODE_ONCE_BACKWARDS);

W3DAnimatedModelRenderObject::W3DAnimatedModelRenderObject(const char * htree_name) :
    IsTreeValid(false), Hierarchy(nullptr), Playback(Assets::Get_Animation_Cache())
{
	
    if (htree_name == nullptr) {
		Hierarchy = nullptr;
	} else if (htree_name[0] == 0) {
		Hierarchy.reset(W3DNEW Graphics::ModelHierarchy);
		Hierarchy->Initialize_Default ();
	} else {
		auto* assets = W3DAssetCatalog::Get_Instance();
		const auto skeleton = assets->Get_Skeleton(htree_name);
		const auto* source = assets->Resolve_Skeleton(skeleton);
		if (source != nullptr) {
			Hierarchy.reset(W3DNEW Graphics::ModelHierarchy(*source));
		} else {
			WWDEBUG_SAY(("Unable to find Hierarchy: %s",htree_name));
			Hierarchy.reset(W3DNEW Graphics::ModelHierarchy);
			Hierarchy->Initialize_Default();
		}
	}
}

W3DAnimatedModelRenderObject::W3DAnimatedModelRenderObject(const W3DAnimatedModelRenderObject & src) :
    W3DModelGroupRenderObject(src), IsTreeValid(false),
    Hierarchy(src.Hierarchy ? W3DNEW Graphics::ModelHierarchy(*src.Hierarchy) : nullptr),
    Playback(Assets::Get_Animation_Cache())
{
}

W3DAnimatedModelRenderObject::~W3DAnimatedModelRenderObject() = default;

W3DAnimatedModelRenderObject & W3DAnimatedModelRenderObject::operator = (const W3DAnimatedModelRenderObject & that)
{
    if (&that != this) {
        W3DModelGroupRenderObject::operator=(that);
        Playback.Reset();
        IsTreeValid = false;
        Hierarchy.reset(that.Hierarchy ? W3DNEW Graphics::ModelHierarchy(*that.Hierarchy) : nullptr);
    }
    return *this;
}

void W3DAnimatedModelRenderObject::Render(W3DRenderContext & rinfo)
{
	if (Hierarchy == nullptr) return;

	if (Is_Not_Hidden_At_All() == false) {
		return;
	}

	//
	// Force the hierarchy to be recalculated for single animations.
	//
	const bool isSingleAnim = Playback.Is_Advancing();

	if (isSingleAnim || !Is_Hierarchy_Valid() || Are_Sub_Object_Transforms_Dirty()) {
		Update_Sub_Object_Transforms();
	}
}

void W3DAnimatedModelRenderObject::Set_Transform(const Matrix3D &m)
{
	W3DModelGroupRenderObject::Set_Transform(m);
	Set_Hierarchy_Valid(false);
}

void W3DAnimatedModelRenderObject::Set_Position(const Vector3 &v)
{
	W3DModelGroupRenderObject::Set_Position(v);
	Set_Hierarchy_Valid(false);
}

int W3DAnimatedModelRenderObject::Get_Num_Bones()
{
	if (Hierarchy) {
		return Hierarchy->Bone_Count();
	} else {
		return 1;
	}
}

const char * W3DAnimatedModelRenderObject::Get_Bone_Name(int bone_index)
{
	if (Hierarchy) {
		return Hierarchy->Bone_Name(bone_index);
	} else {
		return "RootTransform";
	}
}

int W3DAnimatedModelRenderObject::Get_Bone_Index(const char * bonename)
{
	if (Hierarchy) {
		return Hierarchy->Bone_Index(bonename);
	} else {
		return 0;
	}
}

void W3DAnimatedModelRenderObject::Set_Animation()
{
    Playback.Reset();
    Set_Hierarchy_Valid(false);
}

void W3DAnimatedModelRenderObject::Set_Animation(Assets::AnimationAssetHandle motion, float frame, int mode)
{
    Playback.Set(motion, frame, static_cast<Graphics::ModelPlaybackMode>(mode),
        Graphics::Get_Render_Clock().Logic_Time_Milliseconds());
    Set_Hierarchy_Valid(false);
}

void W3DAnimatedModelRenderObject::Set_Animation
(
	Assets::AnimationAssetHandle motion0,
	float frame0,
	Assets::AnimationAssetHandle motion1,
	float frame1,
	float percentage
)
{
    Playback.Blend(motion0, frame0, motion1, frame1, percentage);
    Set_Hierarchy_Valid(false);
}

Assets::AnimationAssetHandle W3DAnimatedModelRenderObject::Peek_Animation()
{
    return Playback.Clip();
}

Matrix3D 	W3DAnimatedModelRenderObject::Get_Bone_Transform(const char * bonename)
{
	if (Hierarchy) {
		WWASSERT(Hierarchy);
		WWASSERT(bonename);

		int idx = Hierarchy->Bone_Index(bonename);
		return Get_Bone_Transform(idx);
	} else {
		return Get_Transform();
	}
}

Matrix3D 	W3DAnimatedModelRenderObject::Get_Bone_Transform(int boneindex)
{
	Validate_Transform();

	if (Hierarchy) {
		
		if (!Is_Hierarchy_Valid()) {
			Update_Sub_Object_Transforms();
		}

		return Graphics::Export_Affine_Transform<Matrix3D>(Hierarchy->World_Transform(boneindex));
	} else {
		return Get_Transform_No_Validity_Check();
	}
}

void W3DAnimatedModelRenderObject::Capture_Bone(int boneindex)
{
	if (Hierarchy) {
		Hierarchy->Capture(boneindex);
	}
}

void W3DAnimatedModelRenderObject::Release_Bone(int boneindex)
{
	if (Hierarchy) {
		Hierarchy->Release(boneindex);
	}
}

bool W3DAnimatedModelRenderObject::Is_Bone_Captured(int boneindex) const
{
	if (Hierarchy) {
		return Hierarchy->Is_Captured(boneindex);
	} else {
		return false;
	}
}

void W3DAnimatedModelRenderObject::Control_Bone(int bindex,const Matrix3D & objtm,bool world_space_translation)
{
#ifdef WWDEBUG
	for (int j=0; j<3; j++) {
		for (int i=0; i<4; i++) {
			WWASSERT(WWMath::Is_Valid_Float(objtm[j][i]));
		}
	}
#endif

	if (Hierarchy) {
		Hierarchy->Control(bindex,Graphics::Import_Affine_Transform(objtm),world_space_translation);
		Set_Hierarchy_Valid(false);
	}
}

void W3DAnimatedModelRenderObject::Update_Sub_Object_Transforms()
{
    W3DModelGroupRenderObject::Update_Sub_Object_Transforms();
    if (Hierarchy) Playback.Evaluate(*Hierarchy, Graphics::Import_Affine_Transform(Get_Transform_No_Validity_Check()),
        Graphics::Get_Render_Clock().Logic_Time_Milliseconds());
    Set_Hierarchy_Valid(true);
}

bool W3DAnimatedModelRenderObject::Simple_Evaluate_Bone(int boneindex, Matrix3D *tm) const
{
    if (!tm) return false;
    if (!Hierarchy) {
        *tm = Get_Transform_No_Validity_Check();
        return false;
    }
    if (!Playback.Is_Blended()) {
        const float frame = Playback.Current_Frame(Graphics::Get_Render_Clock().Logic_Time_Milliseconds());
        return Simple_Evaluate_Bone(boneindex,frame,tm);
    }
    const_cast<W3DAnimatedModelRenderObject*>(this)->Update_Sub_Object_Transforms();
    *tm = Graphics::Export_Affine_Transform<Matrix3D>(Hierarchy->World_Transform(boneindex));
    return false;
}

bool	W3DAnimatedModelRenderObject::Is_Animation_Complete() const
{
    return Playback.Is_Complete();
}

Assets::AnimationAssetHandle W3DAnimatedModelRenderObject::Peek_Animation_And_Info(float& frame, int& numFrames, int& mode, float& mult)
{
    const auto clip = Playback.Clip();
    if (clip) {
        frame = Playback.Frame();
        numFrames = static_cast<int>(Assets::Get_Animation_Cache().Resolve(clip)->frame_count);
        mode = static_cast<int>(Playback.Mode());
        mult = Playback.Multiplier();
    }
    return clip;
}

void W3DAnimatedModelRenderObject::Set_Animation_Frame_Rate_Multiplier(float multiplier)
{
    Playback.Set_Multiplier(multiplier);
}

bool W3DAnimatedModelRenderObject::Simple_Evaluate_Bone(int boneindex,float frame,Matrix3D * tm) const
{
    if (!tm) return false;
    if (!Hierarchy || Playback.Is_Blended()) {
        *tm = Get_Transform_No_Validity_Check();
        return false;
    }
    Graphics::RenderTransform result;
    const bool valid = Playback.Evaluate_Bone(*Hierarchy, boneindex, frame,
        Graphics::Import_Affine_Transform(Get_Transform()), result);
    *tm = Graphics::Export_Affine_Transform<Matrix3D>(result);
    return valid;
}
