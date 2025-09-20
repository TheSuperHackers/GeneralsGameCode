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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: W3DModelDraw.cpp ///////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:   Default w3d draw module
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

#define DEFINE_W3DANIMMODE_NAMES
#define DEFINE_WEAPONSLOTTYPE_NAMES

#define NO_DEBUG_CRC

#include <windows.h>

#include "Common/crc.h"
#include "Common/CRCDebug.h"
#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/PerfTimer.h"
#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "Common/GameLOD.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"
#include "GameClient/Shadow.h"
#include "GameLogic/GameLogic.h"		// for real-time frame
#include "GameLogic/Object.h"
#include "GameLogic/WeaponSet.h"
#include "GameLogic/FPUControl.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDisplay.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "WW3D2/hanim.h"
#include "WW3D2/hlod.h"
#include "WW3D2/rendobj.h"
#include "WW3D2/mesh.h"
#include "WW3D2/meshmdl.h"
#include "Common/BitFlagsIO.h"


//-------------------------------------------------------------------------------------------------
static inline Bool isValidTimeToCalcLogicStuff()
{
	return (TheGameLogic && TheGameLogic->isInGameLogicUpdate()) ||
		(TheGameState && TheGameState->isInLoadGame());
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

#if defined(DEBUG_CRC) && defined(RTS_DEBUG)
#include <cstdarg>
class LogClass
{
public:
	LogClass(const char *fname);
	~LogClass();
	void log(const char *fmt, ...);
	void dumpMatrix3D(const Matrix3D *m, AsciiString name, AsciiString fname, Int line);
	void dumpReal(Real r, AsciiString name, AsciiString fname, Int line);

protected:
	FILE *m_fp;
};

LogClass::LogClass(const char *fname)
{
	char buffer[ _MAX_PATH ];
	GetModuleFileName( NULL, buffer, sizeof( buffer ) );
	char *pEnd = buffer + strlen( buffer );
	while( pEnd != buffer )
	{
		if( *pEnd == '\\' )
		{
			*pEnd = 0;
			break;
		}
		pEnd--;
	}
	// TheSuperHackers @fix Caball009 03/06/2025 Don't use AsciiString here anymore because its memory allocator may not have been initialized yet.
	const std::string fullPath = std::string(buffer) + "\\" + fname;
	m_fp = fopen(fullPath.c_str(), "wt");
}

LogClass::~LogClass()
{
	if (m_fp)
	{
		fclose(m_fp);
	}
}

void LogClass::log(const char *fmt, ...)
{
	DEBUG_ASSERTCRASH(isValidTimeToCalcLogicStuff(), ("Calc'ing logic bone pos in client!!!"));
	if (!m_fp /*|| !isValidTimeToCalcLogicStuff()*/)
		return;
	static char buf[1024];
	static Int lastFrame = 0;
	static Int lastIndex = 0;
	if (lastFrame != TheGameLogic->getFrame())
	{
		lastFrame = TheGameLogic->getFrame();
		lastIndex = 0;
	}

	va_list va;
	va_start( va, fmt );
	vsnprintf(buf, 1024, fmt, va );
	va_end( va );

	char *tmp = buf;
	while (tmp && *tmp)
	{
		if (*tmp == '\r' || *tmp == '\n')
		{
			*tmp = ' ';
		}
		++tmp;
	}

	fprintf(m_fp, "%d:%d %s\n", lastFrame, lastIndex++, buf);
	fflush(m_fp);
}

void LogClass::dumpMatrix3D(const Matrix3D *m, AsciiString name, AsciiString fname, Int line)
{
	fname.toLower();
	fname = fname.reverseFind('\\') + 1;
	const Real *matrix = (const Real *)m;
	log("dumpMatrix3D() %s:%d %s",
		fname.str(), line, name.str());
	for (Int i=0; i<3; ++i)
		log("      0x%08X 0x%08X 0x%08X 0x%08X",
			AS_INT(matrix[(i<<2)+0]), AS_INT(matrix[(i<<2)+1]), AS_INT(matrix[(i<<2)+2]), AS_INT(matrix[(i<<2)+3]));
}

void LogClass::dumpReal(Real r, AsciiString name, AsciiString fname, Int line)
{
	if (!m_fp || !isValidTimeToCalcLogicStuff())
		return;
	fname.toLower();
	fname = fname.reverseFind('\\') + 1;
	log("dumpReal() %s:%d %s %8.8X (%f)",
		fname.str(), line, name.str(), AS_INT(r), r);
}

LogClass BonePosLog("bonePositions.txt");

#define BONEPOS_LOG(x) BonePosLog.log x
#define BONEPOS_DUMPMATRIX3D(x) BONEPOS_DUMPMATRIX3DNAMED(x, #x)
#define BONEPOS_DUMPMATRIX3DNAMED(x, y) BonePosLog.dumpMatrix3D(x, y, __FILE__, __LINE__)
#define BONEPOS_DUMPREAL(x) BONEPOS_DUMPREALNAMED(x, #x)
#define BONEPOS_DUMPREALNAMED(x, y) BonePosLog.dumpReal(x, y, __FILE__, __LINE__)

#else // DEBUG_CRC

#define BONEPOS_LOG(x) {}
#define BONEPOS_DUMPMATRIX3D(x) {}
#define BONEPOS_DUMPMATRIX3DNAMED(x, y) {}
#define BONEPOS_DUMPREAL(x) {}
#define BONEPOS_DUMPREALNAMED(x, y) {}

#endif // DEBUG_CRC

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

#if defined(RTS_DEBUG) || defined(DEBUG_CRASHING)
extern AsciiString TheThingTemplateBeingParsedName;
#endif

#if defined(RTS_DEBUG)
extern Real TheSkateDistOverride;
#endif

// flags that aren't read directly from INI, but set in response to various other situations
enum INIReadFlagsType CPP_11(: Int)
{
	ANIMS_COPIED_FROM_DEFAULT_STATE = 0,
	GOT_NONIDLE_ANIMS,
	GOT_IDLE_ANIMS,
};

enum ACBits CPP_11(: Int)
{
	RANDOMIZE_START_FRAME = 0,
	START_FRAME_FIRST,
	START_FRAME_LAST,
	ADJUST_HEIGHT_BY_CONSTRUCTION_PERCENT,
	PRISTINE_BONE_POS_IN_FINAL_FRAME,
	MAINTAIN_FRAME_ACROSS_STATES,
	RESTART_ANIM_WHEN_COMPLETE,
	MAINTAIN_FRAME_ACROSS_STATES2,
	MAINTAIN_FRAME_ACROSS_STATES3,
	MAINTAIN_FRAME_ACROSS_STATES4,

	AC_BITS_COUNT
};

static const char *const ACBitsNames[] =
{
	"RANDOMSTART",
	"START_FRAME_FIRST",
	"START_FRAME_LAST",
	"ADJUST_HEIGHT_BY_CONSTRUCTION_PERCENT",
	"PRISTINE_BONE_POS_IN_FINAL_FRAME",
	"MAINTAIN_FRAME_ACROSS_STATES",
	"RESTART_ANIM_WHEN_COMPLETE",
	"MAINTAIN_FRAME_ACROSS_STATES2",
	"MAINTAIN_FRAME_ACROSS_STATES3",
	"MAINTAIN_FRAME_ACROSS_STATES4",

	NULL
};
static_assert(ARRAY_SIZE(ACBitsNames) == AC_BITS_COUNT + 1, "Incorrect array size");

static const Int ALL_MAINTAIN_FRAME_FLAGS =
	(1<<MAINTAIN_FRAME_ACROSS_STATES) |
	(1<<MAINTAIN_FRAME_ACROSS_STATES2) |
	(1<<MAINTAIN_FRAME_ACROSS_STATES3) |
	(1<<MAINTAIN_FRAME_ACROSS_STATES4);

inline Bool isAnyMaintainFrameFlagSet(Int flags)
{
	return (flags & ALL_MAINTAIN_FRAME_FLAGS) != 0;
}

inline Bool isCommonMaintainFrameFlagSet(Int a, Int b)
{
	a &= ALL_MAINTAIN_FRAME_FLAGS;
	b &= ALL_MAINTAIN_FRAME_FLAGS;
	return (a & b) != 0;
}


/** @todo: Move this to some kind of INI file*/
//
// Note: these values are saved in save files, so you MUST NOT REMOVE OR CHANGE
// existing values!
//
static const char *TerrainDecalTextureName[TERRAIN_DECAL_MAX-1]=
{
#ifdef ALLOW_DEMORALIZE
	"DM_RING",//demoralized
#else
	"TERRAIN_DECAL_DEMORALIZED_OBSOLETE",
#endif
	"EXHorde",//enthusiastic
	"EXHorde_UP", //enthusiastic with nationalism
	"EXHordeB",//enthusiastic vehicle
	"EXHordeB_UP", //enthusiastic vehicle with nationalism
	"EXJunkCrate",//Marks a crate as special
};

const UnsignedInt NO_NEXT_DURATION = 0xffffffff;

//-------------------------------------------------------------------------------------------------
W3DAnimationInfo::W3DAnimationInfo(const AsciiString& name, Bool isIdle, Real distanceCovered) :
#ifdef RETAIN_ANIM_HANDLES
	m_handle(NULL),
	m_naturalDurationInMsec(0),
#else
	m_naturalDurationInMsec(-1),
#endif
	m_name(name),
	m_isIdleAnim(isIdle),
	m_distanceCovered(distanceCovered)
{
}

//-------------------------------------------------------------------------------------------------
W3DAnimationInfo::W3DAnimationInfo( const W3DAnimationInfo &r ) :
	m_name(r.m_name),
#ifdef RETAIN_ANIM_HANDLES
	m_handle(r.m_handle),
#endif
	m_distanceCovered(r.m_distanceCovered),
	m_isIdleAnim(r.m_isIdleAnim),
	m_naturalDurationInMsec(r.m_naturalDurationInMsec)
{
#ifdef RETAIN_ANIM_HANDLES
	if (m_handle)
		m_handle->Add_Ref();
#endif
}

//-------------------------------------------------------------------------------------------------
W3DAnimationInfo& W3DAnimationInfo::operator=(const W3DAnimationInfo &r)
{
	m_name = r.m_name;
	m_distanceCovered = r.m_distanceCovered;
	m_naturalDurationInMsec = r.m_naturalDurationInMsec;
	m_isIdleAnim = r.m_isIdleAnim;

#ifdef RETAIN_ANIM_HANDLES
	REF_PTR_RELEASE(m_handle);
	m_handle = r.m_handle;
	if (m_handle)
		m_handle->Add_Ref();
#endif

	return (*this);
}


//-------------------------------------------------------------------------------------------------
// note that this now returns an ADDREFED handle, which must be released by the caller!
HAnimClass* W3DAnimationInfo::getAnimHandle() const
{
#ifdef RETAIN_ANIM_HANDLES
	if (m_handle == NULL)
	{
		// Get_HAnim addrefs it, so we'll have to release it in our dtor.
		m_handle = W3DDisplay::m_assetManager->Get_HAnim(m_name.str());
		DEBUG_ASSERTCRASH(m_handle, ("*** ASSET ERROR: animation %s not found",m_name.str()));
		if (m_handle)
		{
			m_naturalDurationInMsec = m_handle->Get_Num_Frames() * 1000.0f / m_handle->Get_Frame_Rate();
		}
	}
	// since we have it locally, must addref.
	if (m_handle)
		m_handle->Add_Ref();
	return m_handle;
#else
	HAnimClass* handle = W3DDisplay::m_assetManager->Get_HAnim(m_name.str());
	DEBUG_ASSERTCRASH(handle, ("*** ASSET ERROR: animation %s not found",m_name.str()));
	if (handle != NULL && m_naturalDurationInMsec < 0)
	{
		m_naturalDurationInMsec = handle->Get_Num_Frames() * 1000.0f / handle->Get_Frame_Rate();
	}
	// since Get_HAnim() returns an addrefed handle, we must NOT addref here.
	//if (handle)
	//	handle->Add_Ref();
	return handle;
#endif
}

//-------------------------------------------------------------------------------------------------
W3DAnimationInfo::~W3DAnimationInfo()
{
#ifdef RETAIN_ANIM_HANDLES
	REF_PTR_RELEASE(m_handle);
	m_handle = NULL;
#endif
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::preloadAssets( TimeOfDay timeOfDay, Real scale )
{
	// load this asset
	if( m_modelName.isEmpty() == FALSE )
	{
		TheDisplay->preloadModelAssets( m_modelName );
	}

	// this can be called from the client, which is problematic
//	validateStuff(NULL, getDrawable()->getScale());
	//validateCachedBones(NULL, scale);
	//validateTurretInfo();
	//validateWeaponBarrelInfo();
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::addPublicBone(const AsciiString& boneName) const
{
	if (boneName.isEmpty() || boneName.isNone())
		return;

	AsciiString tmp = boneName;
	tmp.toLower();
	if (std::find(m_publicBones.begin(), m_publicBones.end(), tmp) == m_publicBones.end())
	{
		m_publicBones.push_back(tmp);
	}
}

//-------------------------------------------------------------------------------------------------
Bool ModelConditionInfo::matchesMode(Bool night, Bool snowy) const
{
	for (std::vector<ModelConditionFlags>::const_iterator it = m_conditionsYesVec.begin();
				it != m_conditionsYesVec.end();
				++it)
	{
		if (it->test(MODELCONDITION_NIGHT) == (night) &&
				it->test(MODELCONDITION_SNOW) == (snowy))
		{
			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
inline Bool testFlagBit(Int flags, Int bit)
{
	return (flags & (1<<bit)) != 0;
}

//-------------------------------------------------------------------------------------------------
static Bool findSingleBone(RenderObjClass* robj, const AsciiString& boneName, Matrix3D& mtx, Int& boneIndex)
{
	if (boneName.isNone() || boneName.isEmpty())
		return false;

	boneIndex = robj->Get_Bone_Index(boneName.str());
	if (boneIndex != 0)
	{
		mtx = robj->Get_Bone_Transform(boneIndex);
		return true;
	}
	else
	{
		return false;
	}
}

//-------------------------------------------------------------------------------------------------
static Bool findSingleSubObj(RenderObjClass* robj, const AsciiString& boneName, Matrix3D& mtx, Int& boneIndex)
{
	if (boneName.isNone() || boneName.isEmpty())
		return false;

	RenderObjClass* childObject = robj->Get_Sub_Object_By_Name(boneName.str());
	if (childObject)
	{
		mtx = childObject->Get_Transform();

		// you'd think this would work, but, it does not. do it the hard way.
		// boneIndex = childObject->Get_Sub_Object_Bone_Index(childObject);

		for (Int subObj = 0; subObj < robj->Get_Num_Sub_Objects(); subObj++)
		{
			RenderObjClass* test = robj->Get_Sub_Object(subObj);
			if (test == childObject)
			{
				boneIndex = robj->Get_Sub_Object_Bone_Index(0, subObj);
#if defined(RTS_DEBUG)
				test->Release_Ref();
				test = robj->Get_Sub_Object_On_Bone(0, boneIndex);
				DEBUG_ASSERTCRASH(test != NULL && test == childObject, ("*** ASSET ERROR: Hmm, bone problem"));
#endif
			}
			if (test) test->Release_Ref();
		}

		childObject->Release_Ref();

		return true;
	}
	else
	{
		return false;
	}
}

//-------------------------------------------------------------------------------------------------
static Bool doSingleBoneName(RenderObjClass* robj, const AsciiString& boneName, PristineBoneInfoMap& map)
{
	Bool foundAsBone = false;
	Bool foundAsSubObj = false;

	PristineBoneInfo info;
	AsciiString tmp;

	AsciiString boneNameTmp = boneName;
	boneNameTmp.toLower();	// convert to all-lowercase to avoid case sens issues later

	setFPMode();

	if (findSingleBone(robj, boneNameTmp, info.mtx, info.boneIndex))
	{
//DEBUG_LOG(("added bone %s",boneNameTmp.str()));
		BONEPOS_LOG(("Caching bone %s (index %d)", boneNameTmp.str(), info.boneIndex));
		BONEPOS_DUMPMATRIX3D(&(info.mtx));

		map[NAMEKEY(boneNameTmp)] = info;
		foundAsBone = true;
	}

	for (Int i = 1; i <= 99; ++i)
	{
		tmp.format("%s%02d", boneNameTmp.str(), i);
		if (findSingleBone(robj, tmp, info.mtx, info.boneIndex))
		{
//DEBUG_LOG(("added bone %s",tmp.str()));
			BONEPOS_LOG(("Caching bone %s (index %d)", tmp.str(), info.boneIndex));
			BONEPOS_DUMPMATRIX3D(&(info.mtx));
			map[NAMEKEY(tmp)] = info;
			foundAsBone = true;
		}
		else
		{
			break;
		}
	}

	if (!foundAsBone)
	{
		if (findSingleSubObj(robj, boneNameTmp, info.mtx, info.boneIndex))
		{
//DEBUG_LOG(("added subobj %s",boneNameTmp.str()));
			BONEPOS_LOG(("Caching bone from subobject %s (index %d)", boneNameTmp.str(), info.boneIndex));
			BONEPOS_DUMPMATRIX3D(&(info.mtx));
			map[NAMEKEY(boneNameTmp)] = info;
			foundAsSubObj = true;
		}

		for (Int i = 1; i <= 99; ++i)
		{
			tmp.format("%s%02d", boneNameTmp.str(), i);
			if (findSingleSubObj(robj, tmp, info.mtx, info.boneIndex))
			{
//DEBUG_LOG(("added subobj %s",tmp.str()));
				BONEPOS_LOG(("Caching bone from subobject %s (index %d)", tmp.str(), info.boneIndex));
				BONEPOS_DUMPMATRIX3D(&(info.mtx));
				map[NAMEKEY(tmp)] = info;
				foundAsSubObj = true;
			}
			else
			{
				break;
			}
		}
	}

	return foundAsBone || foundAsSubObj;
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::validateStuff(RenderObjClass* robj, Real scale, const std::vector<AsciiString>& extraPublicBones) const
{
// srj sez: hm, this doesn't make sense; I think we really do need to validate transition states.
//	if (m_transition != NO_TRANSITION)
//		return;

	loadAnimations();
	if (!(m_validStuff & PUBLIC_BONES_VALID) && isValidTimeToCalcLogicStuff())
	{
		for (std::vector<AsciiString>::const_iterator bone_it = extraPublicBones.begin(); bone_it != extraPublicBones.end(); ++bone_it)
		{
			addPublicBone(*bone_it);
		}
		m_validStuff |= PUBLIC_BONES_VALID;
	}
	validateCachedBones(robj, scale);
	validateTurretInfo();
	validateWeaponBarrelInfo();
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::validateCachedBones(RenderObjClass* robj, Real scale) const
{
	//DEBUG_ASSERTCRASH(isValidTimeToCalcLogicStuff(), ("calling validateCachedBones() from in GameClient!"));
	if (m_validStuff & PRISTINE_BONES_VALID)
		return;

	if (!isValidTimeToCalcLogicStuff())
	{
		m_pristineBones.clear();
		m_validStuff &= ~PRISTINE_BONES_VALID;
		return;
	}

	setFPMode();

	BONEPOS_LOG(("Validating bones for %s: %s", m_modelName.str(), getDescription().str()));
	//BONEPOS_LOG(("Passing in valid render obj: %d", (robj != 0)));
	BONEPOS_DUMPREAL(scale);

	m_pristineBones.clear();

	// go ahead and set this here, in case we return early.
	m_validStuff |= PRISTINE_BONES_VALID;

	Bool tossRobj = false;
	if (robj == NULL)
	{
		if (m_modelName.isEmpty())
		{
			//BONEPOS_LOG(("Bailing: model name is empty"));
			return;
		}

		robj = W3DDisplay::m_assetManager->Create_Render_Obj(m_modelName.str(), scale, 0);
		DEBUG_ASSERTCRASH(robj, ("*** ASSET ERROR: Model %s not found!",m_modelName.str()));
		if (!robj)
		{
			//BONEPOS_LOG(("Bailing: could not load render object"));
			return;
		}
		tossRobj = true;
	}

	Matrix3D			originalTransform = robj->Get_Transform();	// save the transform
	HLodClass*		hlod = NULL;
	HAnimClass*		curAnim = NULL;
	int						numFrames = 0;
	float					frame = 0.0f;
	int						mode = 0;
	float					mult = 1.0f;

	if (robj->Class_ID() == RenderObjClass::CLASSID_HLOD)
	{
		hlod = (HLodClass*)robj;
		curAnim = hlod->Peek_Animation_And_Info(frame, numFrames, mode, mult);
	}

	// if we have any animations in this state, always choose the first, since the animations
	// vary on a per-client basis.
	HAnimClass* animToUse;
	if (m_animations.size() > 0)
	{
		animToUse = m_animations.front().getAnimHandle();	// return an AddRef'ed handle
	}
	else
	{
		animToUse = curAnim;	// Peek_Animation_And_Info does not addref, so we must do so here
		if (animToUse)
			animToUse->Add_Ref();
	}
	if (animToUse != NULL)
	{
		// make sure we're in frame zero.
		Int whichFrame = testFlagBit(m_flags, PRISTINE_BONE_POS_IN_FINAL_FRAME) ? animToUse->Get_Num_Frames()-1 : 0;
		robj->Set_Animation(animToUse, whichFrame, RenderObjClass::ANIM_MODE_MANUAL);
		// must balance the addref, above
		REF_PTR_RELEASE(animToUse);
		animToUse = NULL;
	}

	Matrix3D tmp(true);
	tmp.Scale(scale);
	robj->Set_Transform(tmp);					// set to identity transform

	if (TheGlobalData)
	{
		for (std::vector<AsciiString>::const_iterator it = TheGlobalData->m_standardPublicBones.begin(); it != TheGlobalData->m_standardPublicBones.end(); ++it)
		{
			if (!doSingleBoneName(robj, *it, m_pristineBones))
			{
				// don't crash here, since these are catch-all global bones and won't be present in most models.
				//DEBUG_CRASH(("public bone %s (and variations thereof) not found in model %s!",it->str(),m_modelName.str()));
			}
			//else
			//{
			//	DEBUG_LOG(("global bone %s (or variations thereof) found in model %s",it->str(),m_modelName.str()));
			//}
		}
	}
	for (std::vector<AsciiString>::const_iterator it = m_publicBones.begin(); it != m_publicBones.end(); ++it)
	{
		if (!doSingleBoneName(robj, *it, m_pristineBones))
		{
			// DO crash here, since we specifically requested this bone for this model
			DEBUG_CRASH(("*** ASSET ERROR: public bone '%s' (and variations thereof) not found in model %s!",it->str(),m_modelName.str()));
		}
		//else
		//{
		//	DEBUG_LOG(("extra bone %s (or variations thereof) found in model %s",it->str(),m_modelName.str()));
		//}
	}

	robj->Set_Transform(originalTransform);			// restore previous transform
	if (curAnim != NULL)
	{
		robj->Set_Animation(curAnim, frame, mode);
		hlod->Set_Animation_Frame_Rate_Multiplier(mult);
	}

	if (tossRobj)
	{
		REF_PTR_RELEASE(robj);
	}
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::validateWeaponBarrelInfo() const
{
	//DEBUG_ASSERTCRASH(isValidTimeToCalcLogicStuff(), ("calling validateWeaponBarrelInfo() from in GameClient!"));
	if (m_validStuff & BARRELS_VALID)
		return;

	if (!isValidTimeToCalcLogicStuff())
	{
		return;
	}

	setFPMode();

	for (int wslot = 0; wslot < WEAPONSLOT_COUNT; ++wslot)
	{
		m_weaponBarrelInfoVec[wslot].clear();
		m_hasRecoilBonesOrMuzzleFlashes[wslot] = false;

		const AsciiString& fxBoneName = m_weaponFireFXBoneName[wslot];
		const AsciiString& recoilBoneName = m_weaponRecoilBoneName[wslot];
		const AsciiString& mfName = m_weaponMuzzleFlashName[wslot];
		const AsciiString& plbName = m_weaponProjectileLaunchBoneName[wslot];
		if (fxBoneName.isNotEmpty() || recoilBoneName.isNotEmpty() || mfName.isNotEmpty() || plbName.isNotEmpty())
		{
			Int prevFxBone = 0;
			char buffer[256];
			for (Int i = 1; i <= 99; ++i)
			{
				WeaponBarrelInfo info;
				info.m_projectileOffsetMtx.Make_Identity();

				if (!recoilBoneName.isEmpty())
				{
					sprintf(buffer, "%s%02d", recoilBoneName.str(), i);
					findPristineBone(NAMEKEY(buffer), &info.m_recoilBone);
				}
				if (!mfName.isEmpty())
				{
					sprintf(buffer, "%s%02d", mfName.str(), i);
					findPristineBone(NAMEKEY(buffer), &info.m_muzzleFlashBone);
#if defined(RTS_DEBUG) || defined(DEBUG_CRASHING)
					if (info.m_muzzleFlashBone)
						info.m_muzzleFlashBoneName = buffer;
#endif
				}
				if (!fxBoneName.isEmpty())
				{
					sprintf(buffer, "%s%02d", fxBoneName.str(), i);
					findPristineBone(NAMEKEY(buffer), &info.m_fxBone);
					// special case: if we have multiple muzzleflashes, but only one fxbone, use that fxbone for everything.
					if (info.m_fxBone == 0 && info.m_muzzleFlashBone != 0)
						info.m_fxBone = prevFxBone;
				}

				Int plbBoneIndex = 0;
				if (!plbName.isEmpty())
				{
					sprintf(buffer, "%s%02d", plbName.str(), i);
					const Matrix3D* mtx = findPristineBone(NAMEKEY(buffer), &plbBoneIndex);
					if (mtx != NULL)
						info.m_projectileOffsetMtx = *mtx;
				}

				if (info.m_fxBone == 0 && info.m_recoilBone == 0 && info.m_muzzleFlashBone == 0 && plbBoneIndex == 0)
					break;

				CRCDEBUG_LOG(("validateWeaponBarrelInfo() - model name %s wslot %d", m_modelName.str(), wslot));
				DUMPMATRIX3D(&(info.m_projectileOffsetMtx));
				BONEPOS_LOG(("validateWeaponBarrelInfo() - model name %s wslot %d", m_modelName.str(), wslot));
				BONEPOS_DUMPMATRIX3D(&(info.m_projectileOffsetMtx));
				m_weaponBarrelInfoVec[wslot].push_back(info);

				if (info.m_recoilBone != 0 || info.m_muzzleFlashBone != 0)
					m_hasRecoilBonesOrMuzzleFlashes[wslot] = true;

				prevFxBone = info.m_fxBone;
			}

			if (m_weaponBarrelInfoVec[wslot].empty())
			{
				// try the unadorned names
				WeaponBarrelInfo info;
				if (!recoilBoneName.isEmpty())
					findPristineBone(NAMEKEY(recoilBoneName), &info.m_recoilBone);

				if (!mfName.isEmpty())
					findPristineBone(NAMEKEY(mfName), &info.m_muzzleFlashBone);

#if defined(RTS_DEBUG) || defined(DEBUG_CRASHING)
				if (info.m_muzzleFlashBone)
					info.m_muzzleFlashBoneName = mfName;
#endif

				const Matrix3D* plbMtx = plbName.isEmpty() ? NULL : findPristineBone(NAMEKEY(plbName), NULL);
				if (plbMtx != NULL)
					info.m_projectileOffsetMtx = *plbMtx;
				else
					info.m_projectileOffsetMtx.Make_Identity();

				if (!fxBoneName.isEmpty())
					findPristineBone(NAMEKEY(fxBoneName), &info.m_fxBone);

				if (info.m_fxBone != 0 || info.m_recoilBone != 0 || info.m_muzzleFlashBone != 0 || plbMtx != NULL)
				{
					CRCDEBUG_LOG(("validateWeaponBarrelInfo() - model name %s (unadorned) wslot %d", m_modelName.str(), wslot));
					DUMPMATRIX3D(&(info.m_projectileOffsetMtx));
					BONEPOS_LOG(("validateWeaponBarrelInfo() - model name %s (unadorned) wslot %d", m_modelName.str(), wslot));
					BONEPOS_DUMPMATRIX3D(&(info.m_projectileOffsetMtx));
					m_weaponBarrelInfoVec[wslot].push_back(info);
					if (info.m_recoilBone != 0 || info.m_muzzleFlashBone != 0)
						m_hasRecoilBonesOrMuzzleFlashes[wslot] = true;
				}
				else
				{
					CRCDEBUG_LOG(("validateWeaponBarrelInfo() - model name %s (unadorned) found nothing", m_modelName.str()));
					BONEPOS_LOG(("validateWeaponBarrelInfo() - model name %s (unadorned) found nothing", m_modelName.str()));
				}
			}

			DEBUG_ASSERTCRASH(!(m_modelName.isNotEmpty() && m_weaponBarrelInfoVec[wslot].empty()), ("*** ASSET ERROR: No fx bone named '%s' found in model %s!",fxBoneName.str(),m_modelName.str()));
		}
	}
	m_validStuff |= BARRELS_VALID;
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::validateTurretInfo() const
{
	//DEBUG_ASSERTCRASH(isValidTimeToCalcLogicStuff(), ("calling validateTurretInfo() from in GameClient!"));
	if (m_validStuff & TURRETS_VALID)
		return;

	setFPMode();

	for (int tslot = 0; tslot < MAX_TURRETS; ++tslot)
	{
		TurretInfo& tur = m_turrets[tslot];
		if (!isValidTimeToCalcLogicStuff() || m_modelName.isEmpty())
		{
			tur.m_turretAngleBone = 0;
			tur.m_turretPitchBone = 0;
			continue;
		}

		if (tur.m_turretAngleNameKey != NAMEKEY_INVALID)
		{
			if (findPristineBone(tur.m_turretAngleNameKey, &tur.m_turretAngleBone) == NULL)
			{
				DEBUG_CRASH(("*** ASSET ERROR: TurretBone %s not found! (%s)",KEYNAME(tur.m_turretAngleNameKey).str(),m_modelName.str()));
				tur.m_turretAngleBone = 0;
			}
		}
		else
		{
			tur.m_turretAngleBone = 0;
		}

		if (tur.m_turretPitchNameKey != NAMEKEY_INVALID)
		{
			if (findPristineBone(tur.m_turretPitchNameKey, &tur.m_turretPitchBone) == NULL)
			{
				DEBUG_CRASH(("*** ASSET ERROR: TurretBone %s not found! (%s)",KEYNAME(tur.m_turretPitchNameKey).str(),m_modelName.str()));
				tur.m_turretPitchBone = 0;
			}
		}
		else
		{
			tur.m_turretPitchBone = 0;
		}
	}

	if (isValidTimeToCalcLogicStuff())
	{
		m_validStuff |= TURRETS_VALID;
	}
}

//-------------------------------------------------------------------------------------------------
const Matrix3D* ModelConditionInfo::findPristineBone(NameKeyType boneName, Int* boneIndex) const
{
	DEBUG_ASSERTCRASH((m_validStuff & PRISTINE_BONES_VALID), ("*** ASSET ERROR: bones are not valid"));
	if (!(m_validStuff & PRISTINE_BONES_VALID))
	{
		// set it to zero, some callers rely on this
		if (boneIndex)
			*boneIndex = 0;
		return NULL;
	}

	if (boneName == NAMEKEY_INVALID)
	{
		// set it to zero, some callers rely on this
		if (boneIndex)
			*boneIndex = 0;
		return NULL;
	}

	PristineBoneInfoMap::const_iterator it = m_pristineBones.find(boneName);
	if (it != m_pristineBones.end())
	{
		if (boneIndex)
			*boneIndex = it->second.boneIndex;
		return &it->second.mtx;
	}
	else
	{
		// set it to zero -- some callers rely on this!
		if (boneIndex)
			*boneIndex = 0;
		return NULL;
	}
}

//-------------------------------------------------------------------------------------------------
Bool ModelConditionInfo::findPristineBonePos(NameKeyType boneName, Coord3D& pos) const
{
	const Matrix3D* mtx = findPristineBone(boneName, NULL);
	if (mtx)
	{
		Vector3 v = mtx->Get_Translation();
		pos.x = v.X;
		pos.y = v.Y;
		pos.z = v.Z;
		return true;
	}
	else
	{
		pos.zero();
		return false;
	}
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::loadAnimations() const
{
#ifdef RETAIN_ANIM_HANDLES
	for (W3DAnimationVector::const_iterator it2 = m_animations.begin(); it2 != m_animations.end(); ++it2)
	{
		HAnimClass* h = it2->getAnimHandle();	// just force it to get loaded
		REF_PTR_RELEASE(h);
		h = NULL;
	}
#else
	// srj sez: I think there is no real reason to preload these all anymore. things that need the anims
	// (for bones) will force an implicit load anyway, but there's no point in forcibly loading the anims
	// that don't contain interesting logical bones.
#endif
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::clear()
{
	int i;
#if defined(RTS_DEBUG)
	m_description.clear();
#endif
	m_conditionsYesVec.clear();
	m_modelName.clear();
	for (i = 0; i < MAX_TURRETS; ++i)
	{
		m_turrets[i].clear();
	}
	m_hideShowVec.clear();
	for (i = 0; i < WEAPONSLOT_COUNT; ++i)
	{
		m_weaponFireFXBoneName[i].clear();
		m_weaponRecoilBoneName[i].clear();
		m_weaponMuzzleFlashName[i].clear();
		m_weaponProjectileLaunchBoneName[i].clear();
		m_weaponBarrelInfoVec[i].clear();
		m_hasRecoilBonesOrMuzzleFlashes[i] = false;
	}
	m_particleSysBones.clear();
	m_animations.clear();
	m_flags = 0;
	m_transitionKey = NAMEKEY_INVALID;
	m_allowToFinishKey = NAMEKEY_INVALID;
	m_iniReadFlags = 0;
	m_mode = RenderObjClass::ANIM_MODE_ONCE;
	m_transitionSig = NO_TRANSITION;
	m_animMinSpeedFactor = 1.0f;
	m_animMaxSpeedFactor = 1.0f;
	m_pristineBones.clear();
	m_validStuff = 0;
}

//-------------------------------------------------------------------------------------------------
W3DModelDrawModuleData::W3DModelDrawModuleData() :
	m_validated(0),
	m_okToChangeModelColor(false),
	m_animationsRequirePower(true),
#ifdef CACHE_ATTACH_BONE
	m_attachToDrawableBoneOffsetValid(false),
#endif
	m_minLODRequired(STATIC_GAME_LOD_LOW),
	m_defaultState(-1)
{
	const Real MAX_SHIFT = 3.0f;
	const Real INITIAL_RECOIL_RATE = 2.0f;
	const Real RECOIL_DAMPING = 0.4f;
	const Real SETTLE_RATE = 0.065f;

	m_projectileBoneFeedbackEnabledSlots = 0;
	m_initialRecoil = INITIAL_RECOIL_RATE;
	m_maxRecoil = MAX_SHIFT;
	m_recoilDamping = RECOIL_DAMPING;
	m_recoilSettle = SETTLE_RATE;
	// m_ignoreConditionStates defaults to all zero, which is what we want
}

//-------------------------------------------------------------------------------------------------
void W3DModelDrawModuleData::validateStuffForTimeAndWeather(const Drawable* draw, Bool night, Bool snowy) const
{
	if (!isValidTimeToCalcLogicStuff())
		return;

	enum
	{
		NORMAL			= 0x0001,
		NIGHT				= 0x0002,
		SNOWY				= 0x0004,
		NIGHT_SNOWY	= 0x0008
	};

	Int mode;
	if (night)
	{
		mode = (snowy) ? NIGHT_SNOWY : NIGHT;
	}
	else
	{
		mode = (snowy) ? SNOWY : NORMAL;
	}

	if (m_validated & mode)
		return;

	m_validated |= mode;

	ModelConditionVector::iterator c_it = m_conditionStates.begin();
	for (; c_it != m_conditionStates.end(); ++c_it)
	{
		if (!c_it->matchesMode(false, false) && !c_it->matchesMode(night, snowy))
			continue;

		c_it->validateStuff(NULL, draw->getScale(), m_extraPublicBones);
	}

	for (TransitionMap::iterator t_it = m_transitionMap.begin(); t_it != m_transitionMap.end(); ++t_it)
	{
		// here's the tricky part: only want to load the anims for this one if there is at least one
		// source AND at least one dest state that matches the current mode
		NameKeyType src = recoverSrcState(t_it->first);
		NameKeyType dst = recoverDstState(t_it->first);

		Bool a = false;
		Bool b = false;
		for (c_it = m_conditionStates.begin(); c_it != m_conditionStates.end(); ++c_it)
		{

			if (!a && c_it->m_transitionKey == src && c_it->matchesMode(night, snowy))
				a = true;

			if (!b && c_it->m_transitionKey == dst && c_it->matchesMode(night, snowy))
				b = true;

		}
		if (a && b)
		{
			t_it->second.loadAnimations();
			// nope -- transition states don't get public bones.
			//it->addPublicBone(m_extraPublicBones);

		// srj sez: hm, this doesn't make sense; I think we really do need to validate transition states.
			t_it->second.validateStuff(NULL, draw->getScale(), m_extraPublicBones);
		}
	}
}

//-------------------------------------------------------------------------------------------------
W3DModelDrawModuleData::~W3DModelDrawModuleData()
{
	m_conditionStateMap.clear();
}

//-------------------------------------------------------------------------------------------------
void W3DModelDrawModuleData::preloadAssets( TimeOfDay timeOfDay, Real scale ) const
{

	for( ModelConditionVector::iterator it = m_conditionStates.begin();
			 it != m_conditionStates.end();
			 ++it )
	{

		it->preloadAssets( timeOfDay, scale );

	}

}

//-------------------------------------------------------------------------------------------------
AsciiString W3DModelDrawModuleData::getBestModelNameForWB(const ModelConditionFlags& c) const
{
	const ModelConditionInfo* info = findBestInfo(c);
	if (info)
		return info->m_modelName;
	return AsciiString::TheEmptyString;
}

#ifdef CACHE_ATTACH_BONE
//-------------------------------------------------------------------------------------------------
const Vector3* W3DModelDrawModuleData::getAttachToDrawableBoneOffset(const Drawable* draw) const
{
	if (m_attachToDrawableBone.isEmpty())
	{
		return NULL;
	}
	else
	{
		if (!m_attachToDrawableBoneOffsetValid)
		{
		// must use pristine bone here since the result is used by logic
			Matrix3D boneMtx;
			if (draw->getPristineBonePositions(m_attachToDrawableBone.str(), 0, NULL, &boneMtx, 1) == 1)
			{
				m_attachToDrawableBoneOffset = boneMtx.Get_Translation();
			}
			else
			{
				m_attachToDrawableBoneOffset.X = 0;
				m_attachToDrawableBoneOffset.Y = 0;
				m_attachToDrawableBoneOffset.Z = 0;
			}
			m_attachToDrawableBoneOffsetValid = true;
		}
		return &m_attachToDrawableBoneOffset;
	}
}
#endif

//-------------------------------------------------------------------------------------------------
enum ParseCondStateType CPP_11(: Int)
{
	PARSE_NORMAL,
	PARSE_DEFAULT,
	PARSE_TRANSITION,
	PARSE_ALIAS
};

//-------------------------------------------------------------------------------------------------
static void parseAsciiStringLC( INI* ini, void * /*instance*/, void *store, const void* /*userData*/ )
{
	AsciiString* asciiString = (AsciiString *)store;
	*asciiString = ini->getNextAsciiString();
	asciiString->toLower();
}

//-------------------------------------------------------------------------------------------------
void W3DModelDrawModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "InitialRecoilSpeed",	INI::parseVelocityReal, NULL, offsetof(W3DModelDrawModuleData, m_initialRecoil) },
		{ "MaxRecoilDistance",	INI::parseReal, NULL, offsetof(W3DModelDrawModuleData, m_maxRecoil) },
		{ "RecoilDamping",	INI::parseReal, NULL, offsetof(W3DModelDrawModuleData, m_recoilDamping) },
		{ "RecoilSettleSpeed",	INI::parseVelocityReal, NULL, offsetof(W3DModelDrawModuleData, m_recoilSettle) },
		{ "OkToChangeModelColor",	INI::parseBool, NULL, offsetof(W3DModelDrawModuleData, m_okToChangeModelColor) },
		{ "AnimationsRequirePower",	INI::parseBool, NULL, offsetof(W3DModelDrawModuleData, m_animationsRequirePower) },
		{ "MinLODRequired",		INI::parseStaticGameLODLevel,	NULL,	offsetof(W3DModelDrawModuleData, m_minLODRequired) },
		{ "ProjectileBoneFeedbackEnabledSlots", INI::parseBitString32, TheWeaponSlotTypeNames, offsetof(W3DModelDrawModuleData, m_projectileBoneFeedbackEnabledSlots) },
		{ "DefaultConditionState", W3DModelDrawModuleData::parseConditionState, (void*)PARSE_DEFAULT, 0 },
		{ "ConditionState", W3DModelDrawModuleData::parseConditionState, (void*)PARSE_NORMAL, 0 },
		{ "AliasConditionState", W3DModelDrawModuleData::parseConditionState, (void*)PARSE_ALIAS, 0 },
		{ "TransitionState", W3DModelDrawModuleData::parseConditionState, (void*)PARSE_TRANSITION, 0 },
		{ "TrackMarks", parseAsciiStringLC, NULL, offsetof(W3DModelDrawModuleData, m_trackFile) },
		{ "ExtraPublicBone", INI::parseAsciiStringVectorAppend, NULL, offsetof(W3DModelDrawModuleData, m_extraPublicBones) },
		{ "AttachToBoneInAnotherModule", parseAsciiStringLC, NULL, offsetof(W3DModelDrawModuleData, m_attachToDrawableBone) },
		{ "IgnoreConditionStates", ModelConditionFlags::parseFromINI, NULL, offsetof(W3DModelDrawModuleData, m_ignoreConditionStates) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);

}

//-------------------------------------------------------------------------------------------------
enum AnimParseType CPP_11(: Int)
{
	ANIM_NORMAL,
	ANIM_IDLE
};

//-------------------------------------------------------------------------------------------------
static void parseAnimation(INI* ini, void *instance, void * /*store*/, const void* userData)
{
	AnimParseType animType = (AnimParseType)(UnsignedInt)userData;

	AsciiString animName = ini->getNextAsciiString();
	animName.toLower();

	const char* distanceCoveredToken = ini->getNextTokenOrNull();
	Real distanceCovered = distanceCoveredToken ? INI::scanReal(distanceCoveredToken) : 0;
	DEBUG_ASSERTCRASH(!(animType == ANIM_IDLE && distanceCovered != 0), ("You should not specify nonzero DistanceCovered values for Idle Anims"));

	const char* timesToRepeatToken = ini->getNextTokenOrNull();
	Int timesToRepeat = timesToRepeatToken ? INI::scanInt(timesToRepeatToken) : 1;
	if (timesToRepeat < 1) timesToRepeat = 1;

	W3DAnimationInfo animInfo(animName, (animType == ANIM_IDLE), distanceCovered);

	ModelConditionInfo* self = (ModelConditionInfo*)instance;
	if (self->m_iniReadFlags & (1<<ANIMS_COPIED_FROM_DEFAULT_STATE))
	{
		self->m_iniReadFlags &= ~((1<<ANIMS_COPIED_FROM_DEFAULT_STATE)|(1<<GOT_IDLE_ANIMS)|(1<<GOT_NONIDLE_ANIMS));
		self->m_animations.clear();
	}

	if (animInfo.isIdleAnim())
		self->m_iniReadFlags |= (1<<GOT_IDLE_ANIMS);
	else
		self->m_iniReadFlags |= (1<<GOT_NONIDLE_ANIMS);

	if (!animName.isEmpty() && !animName.isNone())
	{
		while (timesToRepeat--)
			self->m_animations.push_back(animInfo);
	}
}

//-------------------------------------------------------------------------------------------------
static void parseShowHideSubObject(INI* ini, void *instance, void *store, const void* userData)
{
	std::vector<ModelConditionInfo::HideShowSubObjInfo>* vec = (std::vector<ModelConditionInfo::HideShowSubObjInfo>*)store;
	AsciiString subObjName = ini->getNextAsciiString();
	subObjName.toLower();

	// handle "None"
	if (subObjName.isNone())
	{
		vec->clear();
		return;
	}

	while (subObjName.isNotEmpty())
	{
		Bool found = false;
		for (std::vector<ModelConditionInfo::HideShowSubObjInfo>::iterator it = vec->begin(); it != vec->end(); ++it)
		{
			if (stricmp(it->subObjName.str(), subObjName.str()) == 0)
			{
				it->hide = (userData != NULL);
				found = true;
			}
		}

		if (!found)
		{
			ModelConditionInfo::HideShowSubObjInfo info;
			info.subObjName = subObjName;
			info.hide = (userData != NULL);
			vec->push_back(info);
		}
		subObjName = ini->getNextAsciiString();
		subObjName.toLower();
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::showSubObject( const AsciiString& name, Bool show )
{
	if( name.isNotEmpty() )
	{
		Bool found = false;
		for( std::vector<ModelConditionInfo::HideShowSubObjInfo>::iterator it = m_subObjectVec.begin(); it != m_subObjectVec.end(); ++it )
		{
			if( stricmp( it->subObjName.str(), name.str() ) == 0 )
			{
				it->hide = !show;
				found = true;
			}
		}

		if( !found )
		{
			ModelConditionInfo::HideShowSubObjInfo info;
			info.subObjName = name;
			info.hide = !show;
			m_subObjectVec.push_back( info );
		}
	}
}



//-------------------------------------------------------------------------------------------------
static void parseWeaponBoneName(INI* ini, void *instance, void * store, const void* /*userData*/)
{
	ModelConditionInfo* self = (ModelConditionInfo*)instance;
	AsciiString* arr = (AsciiString*)store;

	WeaponSlotType wslot = (WeaponSlotType)INI::scanIndexList(ini->getNextToken(), TheWeaponSlotTypeNames);
	arr[wslot] = ini->getNextAsciiString();
	arr[wslot].toLower();
	if (arr[wslot].isNone())
		arr[wslot].clear();

	if (self)
		self->addPublicBone(arr[wslot]);
}

//-------------------------------------------------------------------------------------------------
static void parseParticleSysBone(INI* ini, void *instance, void * store, const void * /*userData*/)
{
	ParticleSysBoneInfo info;
	info.boneName = ini->getNextAsciiString();
	info.boneName.toLower();
	ini->parseParticleSystemTemplate(ini, instance, &(info.particleSystemTemplate), NULL);
	ModelConditionInfo *self = (ModelConditionInfo *)instance;
	self->m_particleSysBones.push_back(info);
}

//-------------------------------------------------------------------------------------------------
static void parseRealRange( INI *ini, void *instance, void *store, const void* /*userData*/ )
{
	ModelConditionInfo *self = (ModelConditionInfo *)instance;

	const char *token = ini->getNextToken();
	self->m_animMinSpeedFactor = ini->scanReal( token );
	token = ini->getNextToken();
	self->m_animMaxSpeedFactor = ini->scanReal( token );
}

//-------------------------------------------------------------------------------------------------
static void parseLowercaseNameKey(INI* ini, void *instance, void * store, const void * /*userData*/)
{
	NameKeyType* key = (NameKeyType*)store;

	AsciiString tmp = ini->getNextToken();
	tmp.toLower();

	*key = NAMEKEY(tmp.str());
}

//-------------------------------------------------------------------------------------------------
static void parseBoneNameKey(INI* ini, void *instance, void * store, const void * /*userData*/)
{
	ModelConditionInfo* self = (ModelConditionInfo*)instance;
	NameKeyType* key = (NameKeyType*)store;

	AsciiString tmp = ini->getNextToken();
	tmp.toLower();

	if (self)
		self->addPublicBone(tmp);

	if (tmp.isEmpty() || tmp.isNone())
		*key = NAMEKEY_INVALID;
	else
		*key = NAMEKEY(tmp.str());
}

//-------------------------------------------------------------------------------------------------
static Bool doesStateExist(const ModelConditionVector& v, const ModelConditionFlags& f)
{
	for (ModelConditionVector::const_iterator it = v.begin(); it != v.end(); ++it)
	{
		for (Int i = it->getConditionsYesCount()-1; i >= 0; --i)
		{
			if (f == it->getNthConditionsYes(i))
				return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
void W3DModelDrawModuleData::parseConditionState(INI* ini, void *instance, void * /*store*/, const void* userData)
{
	static const FieldParse myFieldParse[] =
	{
		{ "Model",	parseAsciiStringLC, NULL, offsetof(ModelConditionInfo, m_modelName) },
		{ "Turret",	parseBoneNameKey, NULL, offsetof(ModelConditionInfo, m_turrets[0].m_turretAngleNameKey) },
		{ "TurretArtAngle", INI::parseAngleReal, NULL, offsetof(ModelConditionInfo, m_turrets[0].m_turretArtAngle) },
		{ "TurretPitch",	parseBoneNameKey, NULL, offsetof(ModelConditionInfo, m_turrets[0].m_turretPitchNameKey) },
		{ "TurretArtPitch", INI::parseAngleReal, NULL, offsetof(ModelConditionInfo, m_turrets[0].m_turretArtPitch) },
		{ "AltTurret",	parseBoneNameKey, NULL, offsetof(ModelConditionInfo, m_turrets[1].m_turretAngleNameKey) },
		{ "AltTurretArtAngle", INI::parseAngleReal, NULL, offsetof(ModelConditionInfo, m_turrets[1].m_turretArtAngle) },
		{ "AltTurretPitch",	parseBoneNameKey, NULL, offsetof(ModelConditionInfo, m_turrets[1].m_turretPitchNameKey) },
		{ "AltTurretArtPitch", INI::parseAngleReal, NULL, offsetof(ModelConditionInfo, m_turrets[1].m_turretArtPitch) },
		{ "ShowSubObject", parseShowHideSubObject, (void*)0, offsetof(ModelConditionInfo, m_hideShowVec) },
		{ "HideSubObject", parseShowHideSubObject, (void*)1, offsetof(ModelConditionInfo, m_hideShowVec) },
		{ "WeaponFireFXBone", parseWeaponBoneName, NULL, offsetof(ModelConditionInfo, m_weaponFireFXBoneName[0]) },
		{ "WeaponRecoilBone", parseWeaponBoneName, NULL, offsetof(ModelConditionInfo, m_weaponRecoilBoneName[0]) },
		{ "WeaponMuzzleFlash", parseWeaponBoneName, NULL, offsetof(ModelConditionInfo, m_weaponMuzzleFlashName[0]) },
		{ "WeaponLaunchBone", parseWeaponBoneName, NULL, offsetof(ModelConditionInfo, m_weaponProjectileLaunchBoneName[0]) },
		{ "WeaponHideShowBone", parseWeaponBoneName, NULL, offsetof(ModelConditionInfo, m_weaponProjectileHideShowName[0]) },
		{ "Animation", parseAnimation, (void*)ANIM_NORMAL, offsetof(ModelConditionInfo, m_animations) },
		{ "IdleAnimation", parseAnimation, (void*)ANIM_IDLE, offsetof(ModelConditionInfo, m_animations) },
		{ "AnimationMode", INI::parseIndexList, TheAnimModeNames, offsetof(ModelConditionInfo, m_mode) },
		{ "TransitionKey", parseLowercaseNameKey, NULL, offsetof(ModelConditionInfo, m_transitionKey) },
		{ "WaitForStateToFinishIfPossible", parseLowercaseNameKey, NULL, offsetof(ModelConditionInfo, m_allowToFinishKey) },
		{ "Flags", INI::parseBitString32, ACBitsNames, offsetof(ModelConditionInfo, m_flags) },
		{ "ParticleSysBone", parseParticleSysBone, NULL, 0 },
		{ "AnimationSpeedFactorRange", parseRealRange, NULL, 0 },
		{ 0, 0, 0, 0 }
	};

	ModelConditionInfo info;
	W3DModelDrawModuleData* self = (W3DModelDrawModuleData*)instance;
	ParseCondStateType cst = (ParseCondStateType)(UnsignedInt)userData;
	switch (cst)
	{
		case PARSE_DEFAULT:
		{
			if (self->m_defaultState >= 0)
			{
				DEBUG_CRASH(("*** ASSET ERROR: you may have only one default state!"));
				throw INI_INVALID_DATA;
			}
			else if (ini->getNextTokenOrNull())
			{
				DEBUG_CRASH(("*** ASSET ERROR: unknown keyword"));
				throw INI_INVALID_DATA;
			}
			else
			{
				if (!self->m_conditionStates.empty())
				{
					DEBUG_CRASH(("*** ASSET ERROR: when using DefaultConditionState, it must be the first state listed (%s)",TheThingTemplateBeingParsedName.str()));
					throw INI_INVALID_DATA;
				}

				// note, this is size(), not size()-1, since we haven't actually modified the list yet
				self->m_defaultState = self->m_conditionStates.size();
				//DEBUG_LOG(("set default state to %d",self->m_defaultState));

				// add an empty conditionstateflag set
				ModelConditionFlags blankConditions;
				info.m_conditionsYesVec.clear();
				info.m_conditionsYesVec.push_back(blankConditions);

	#if defined(RTS_DEBUG)
				info.m_description.clear();
				info.m_description.concat(TheThingTemplateBeingParsedName);
				info.m_description.concat(" DEFAULT");
	#endif
			}
		}
		break;

		case PARSE_TRANSITION:
		{
		  AsciiString firstNm = ini->getNextToken(); firstNm.toLower();
		  AsciiString secondNm = ini->getNextToken(); secondNm.toLower();
			NameKeyType firstKey = NAMEKEY(firstNm);
			NameKeyType secondKey = NAMEKEY(secondNm);

			if (firstKey == secondKey)
			{
				DEBUG_CRASH(("*** ASSET ERROR: You may not declare a transition between two identical states"));
				throw INI_INVALID_DATA;
			}

			if (self->m_defaultState >= 0)
			{
				info = self->m_conditionStates.at(self->m_defaultState);
				info.m_iniReadFlags |= (1<<ANIMS_COPIED_FROM_DEFAULT_STATE);
				info.m_transitionKey = NAMEKEY_INVALID;
				info.m_allowToFinishKey = NAMEKEY_INVALID;
			}

			info.m_transitionSig = buildTransitionSig(firstKey, secondKey);
	#if defined(RTS_DEBUG)
			info.m_description.clear();
			info.m_description.concat(TheThingTemplateBeingParsedName);
			info.m_description.concat(" TRANSITION: ");
			info.m_description.concat(firstNm);
			info.m_description.concat(" ");
			info.m_description.concat(secondNm);
	#endif

		}
		break;

		case PARSE_ALIAS:
		{
			if (self->m_conditionStates.empty())
			{
				DEBUG_CRASH(("*** ASSET ERROR: AliasConditionState must refer to the previous state!"));
				throw INI_INVALID_DATA;
			}

			ModelConditionInfo& prevState = self->m_conditionStates.at(self->m_conditionStates.size()-1);

			ModelConditionFlags conditionsYes;

	#if defined(RTS_DEBUG)
			AsciiString description;
			conditionsYes.parse(ini, &description);

			prevState.m_description.concat("\nAKA: ");
			prevState.m_description.concat(description);
	#else
			conditionsYes.parse(ini, NULL);
	#endif

			if (conditionsYes.anyIntersectionWith(self->m_ignoreConditionStates))
			{
				DEBUG_CRASH(("You should not specify bits in a state once they are used in IgnoreConditionStates (%s)", TheThingTemplateBeingParsedName.str()));
				throw INI_INVALID_DATA;
			}

			if (doesStateExist(self->m_conditionStates, conditionsYes))
			{
				DEBUG_CRASH(("*** ASSET ERROR: duplicate condition states are not currently allowed"));
				throw INI_INVALID_DATA;
			}


			if (!conditionsYes.any() && self->m_defaultState >= 0)
			{
				DEBUG_CRASH(("*** ASSET ERROR: you may not specify both a Default state and a Conditions=None state"));
				throw INI_INVALID_DATA;
			}

			prevState.m_conditionsYesVec.push_back(conditionsYes);

			// yes, return, NOT break!
			return;
		}

		case PARSE_NORMAL:
		{
			if (self->m_defaultState >= 0 && cst != PARSE_ALIAS)
			{
				info = self->m_conditionStates.at(self->m_defaultState);
				info.m_iniReadFlags |= (1<<ANIMS_COPIED_FROM_DEFAULT_STATE);
				info.m_conditionsYesVec.clear();
			}
	// no, we do not currently require a default state, cuz it would break the exiting INI
	// files too badly. maybe someday.
	//	else
	//	{
	//		DEBUG_CRASH(("*** ASSET ERROR: you must specify a default state"));
	//		throw INI_INVALID_DATA;
	//	}

			ModelConditionFlags conditionsYes;
	#if defined(RTS_DEBUG) || defined(DEBUG_CRASHING)
			AsciiString description;
			conditionsYes.parse(ini, &description);

			info.m_description.clear();
			info.m_description.concat(TheThingTemplateBeingParsedName);
			info.m_description.concat("\n         ");
			info.m_description.concat(description);
	#else
			conditionsYes.parse(ini, NULL);
	#endif

			if (conditionsYes.anyIntersectionWith(self->m_ignoreConditionStates))
			{
				DEBUG_CRASH(("You should not specify bits in a state once they are used in IgnoreConditionStates (%s)", TheThingTemplateBeingParsedName.str()));
				throw INI_INVALID_DATA;
			}

			if (self->m_defaultState < 0 && self->m_conditionStates.empty() && conditionsYes.any())
			{
				// it doesn't actually NEED to be first, but it does need to be present, and this is the simplest way to enforce...
				DEBUG_CRASH(("*** ASSET ERROR: when not using DefaultConditionState, the first ConditionState must be for NONE (%s)",TheThingTemplateBeingParsedName.str()));
				throw INI_INVALID_DATA;
			}

			if (!conditionsYes.any() && self->m_defaultState >= 0)
			{
				DEBUG_CRASH(("*** ASSET ERROR: you may not specify both a Default state and a Conditions=None state"));
				throw INI_INVALID_DATA;
			}

			if (doesStateExist(self->m_conditionStates, conditionsYes))
			{
				DEBUG_CRASH(("*** ASSET ERROR: duplicate condition states are not currently allowed (%s)",info.m_description.str()));
				throw INI_INVALID_DATA;
			}

			DEBUG_ASSERTCRASH(info.m_conditionsYesVec.size() == 0, ("*** ASSET ERROR: nonempty m_conditionsYesVec.size(), see srj"));
			info.m_conditionsYesVec.clear();
			info.m_conditionsYesVec.push_back(conditionsYes);
		}
		break;
	}

	ini->initFromINI(&info, myFieldParse);

	if (info.m_modelName.isEmpty())
	{
		DEBUG_CRASH(("*** ASSET ERROR: you must specify a model name"));
		throw INI_INVALID_DATA;
	}
	else if (info.m_modelName.isNone())
	{
		info.m_modelName.clear();
	}

	if ((info.m_iniReadFlags & (1<<GOT_IDLE_ANIMS)) && (info.m_iniReadFlags & (1<<GOT_NONIDLE_ANIMS)))
	{
		DEBUG_CRASH(("*** ASSET ERROR: you should not specify both Animations and IdleAnimations for the same state"));
		throw INI_INVALID_DATA;
	}

	if ((info.m_iniReadFlags & (1<<GOT_IDLE_ANIMS)) && (info.m_mode != RenderObjClass::ANIM_MODE_ONCE && info.m_mode != RenderObjClass::ANIM_MODE_ONCE_BACKWARDS))
	{
		DEBUG_CRASH(("*** ASSET ERROR: Idle Anims should always use ONCE or ONCE_BACKWARDS (%s)",TheThingTemplateBeingParsedName.str()));
		throw INI_INVALID_DATA;
	}

	info.m_validStuff &= ~ModelConditionInfo::HAS_PROJECTILE_BONES;
	for (int wslot = 0; wslot < WEAPONSLOT_COUNT; ++wslot)
	{
		if (info.m_weaponProjectileLaunchBoneName[wslot].isNotEmpty())
		{
			info.m_validStuff |= ModelConditionInfo::HAS_PROJECTILE_BONES;
			break;
		}
	}

	if (cst == PARSE_TRANSITION)
	{
		if (info.m_iniReadFlags & (1<<GOT_IDLE_ANIMS))
		{
			DEBUG_CRASH(("*** ASSET ERROR: Transition States should not specify Idle anims"));
			throw INI_INVALID_DATA;
		}

		if (info.m_mode != RenderObjClass::ANIM_MODE_ONCE && info.m_mode != RenderObjClass::ANIM_MODE_ONCE_BACKWARDS)
		{
			DEBUG_CRASH(("*** ASSET ERROR: Transition States should always use ONCE or ONCE_BACKWARDS"));
			throw INI_INVALID_DATA;
		}

		if (info.m_transitionKey != NAMEKEY_INVALID || info.m_allowToFinishKey != NAMEKEY_INVALID)
		{
			DEBUG_CRASH(("*** ASSET ERROR: Transition States must not have transition keys or m_allowToFinishKey"));
			throw INI_INVALID_DATA;
		}

		self->m_transitionMap[info.m_transitionSig] = info;
	}
	else
	{
		self->m_conditionStates.push_back(info);
	}
}

//-------------------------------------------------------------------------------------------------
static Int countOnBits(UnsignedInt val)
{
	Int count = 0;
	for (Int i = 0; i < 32; ++i)
	{
		if (val & 1)
			++count;
		val >>= 1;
	}
	return count;
}

//-------------------------------------------------------------------------------------------------
const ModelConditionInfo* W3DModelDrawModuleData::findBestInfo(const ModelConditionFlags& c) const
{
	ModelConditionFlags bits = c;
	bits.clear(m_ignoreConditionStates);
	return m_conditionStateMap.findBestInfo(m_conditionStates, bits);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DModelDraw::W3DModelDraw(Thing *thing, const ModuleData* moduleData) : DrawModule(thing, moduleData)
{
	int i;
	m_animationMode = RenderObjClass::ANIM_MODE_LOOP;
	m_hideHeadlights = true;
	m_pauseAnimation = false;
	m_curState = NULL;
	m_hexColor = 0;
	m_renderObject = NULL;
	m_shadow = NULL;
	m_shadowEnabled = TRUE;
	m_terrainDecal = NULL;
	m_trackRenderObject = NULL;
	m_whichAnimInCurState = -1;
	m_nextState = NULL;
	m_nextStateAnimLoopDuration = NO_NEXT_DURATION;
	for (i = 0; i < WEAPONSLOT_COUNT; ++i)
	{
		m_weaponRecoilInfoVec[i].clear();
	}
	m_needRecalcBoneParticleSystems = false;
	m_fullyObscuredByShroud = false;

	// only validate the current time-of-day and weather conditions by default.
	getW3DModelDrawModuleData()->validateStuffForTimeAndWeather(getDrawable(),
											TheGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT,
											TheGlobalData->m_weather == WEATHER_SNOWY);

	ModelConditionFlags emptyFlags;
	const ModelConditionInfo* info = findBestInfo(emptyFlags);
	if (!info)
	{
		DEBUG_CRASH(("*** ASSET ERROR: all draw modules must have an IDLE state"));
		throw INI_INVALID_DATA;
	}

	Drawable* draw = getDrawable();
	Object* obj = draw ? draw->getObject() : NULL;
	if (obj)
	{
		if (TheGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT)
			m_hexColor = obj->getNightIndicatorColor();
		else
			m_hexColor = obj->getIndicatorColor();
	}

	setModelState(info);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DModelDraw::onDrawableBoundToObject(void)
{
	getW3DModelDrawModuleData()->validateStuffForTimeAndWeather(getDrawable(),
											TheGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT,
											TheGlobalData->m_weather == WEATHER_SNOWY);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DModelDraw::~W3DModelDraw(void)
{
	if (m_trackRenderObject && TheTerrainTracksRenderObjClassSystem)
	{
		TheTerrainTracksRenderObjClassSystem->unbindTrack(m_trackRenderObject);
		m_trackRenderObject = NULL;
	}

	nukeCurrentRender(NULL);
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::doStartOrStopParticleSys()
{
	Bool hidden = getDrawable()->isDrawableEffectivelyHidden() || m_fullyObscuredByShroud;

	for (std::vector<ParticleSysTrackerType>::const_iterator it = m_particleSystemIDs.begin(); it != m_particleSystemIDs.end(); ++it)
	//for (std::vector<ParticleSystemID>::const_iterator it = m_particleSystemIDs.begin(); it != m_particleSystemIDs.end(); ++it)
	{
		ParticleSystem *sys = TheParticleSystemManager->findParticleSystem((*it).id);
		if (sys != NULL) {
			// this can be NULL
			if (hidden) {
				sys->stop();
			} else {
				sys->start();
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::setHidden(Bool hidden)
{
	if (m_renderObject)
		m_renderObject->Set_Hidden(hidden);

	if (m_shadow)
		m_shadow->enableShadowRender(!hidden);

	m_shadowEnabled = hidden;

	if (m_terrainDecal)
		m_terrainDecal->enableShadowRender(!hidden);

	if (m_trackRenderObject && hidden)
	{	const Coord3D* pos = getDrawable()->getPosition();
		m_trackRenderObject->addCapEdgeToTrack(pos->x,pos->y);
	}

	doStartOrStopParticleSys();
}

/**Free all data used by this model's shadow.  This is used to dynamically enable/disable shadows by the options screen*/
void W3DModelDraw::releaseShadows(void)	///< frees all shadow resources used by this module - used by Options screen.
{
	if (m_shadow)
		m_shadow->release();
	m_shadow = NULL;
}

/** Create shadow resources if not already present. This is used to dynamically enable/disable shadows by the options screen*/
void W3DModelDraw::allocateShadows(void)
{
	const ThingTemplate *tmplate=getDrawable()->getTemplate();

	//Check if we don't already have a shadow but need one for this type of model.
	if (m_shadow == NULL && m_renderObject && TheW3DShadowManager && tmplate->getShadowType() != SHADOW_NONE)
	{
		Shadow::ShadowTypeInfo shadowInfo;
		strcpy(shadowInfo.m_ShadowName, tmplate->getShadowTextureName().str());
		DEBUG_ASSERTCRASH(shadowInfo.m_ShadowName[0] != '\0', ("this should be validated in ThingTemplate now"));
		shadowInfo.allowUpdates			= FALSE;		//shadow image will never update
		shadowInfo.allowWorldAlign	= TRUE;	//shadow image will wrap around world objects
		shadowInfo.m_type						= (ShadowType)tmplate->getShadowType();
		shadowInfo.m_sizeX					= tmplate->getShadowSizeX();
		shadowInfo.m_sizeY					= tmplate->getShadowSizeY();
		shadowInfo.m_offsetX				= tmplate->getShadowOffsetX();
		shadowInfo.m_offsetY				= tmplate->getShadowOffsetY();
  		m_shadow = TheW3DShadowManager->addShadow(m_renderObject, &shadowInfo);
		if (m_shadow)
		{	m_shadow->enableShadowInvisible(m_fullyObscuredByShroud);
			if (m_renderObject->Is_Hidden() || !m_shadowEnabled)
				m_shadow->enableShadowRender(FALSE);
		}
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::setShadowsEnabled(Bool enable)
{
	if (m_shadow)
		m_shadow->enableShadowRender(enable);
	m_shadowEnabled = enable;
}

/**collect some stats about the rendering cost of this draw module */
#if defined(RTS_DEBUG)
void W3DModelDraw::getRenderCost(RenderCost & rc) const
{
	getRenderCostRecursive(rc,m_renderObject);
	if (m_shadow)
		m_shadow->getRenderCost(rc);
}
#endif //RTS_DEBUG


/**recurse through sub-objs to collect stats about the rendering cost of this draw module */
#if defined(RTS_DEBUG)
void W3DModelDraw::getRenderCostRecursive(RenderCost & rc,RenderObjClass * robj) const
{
	if (robj == NULL) return;

	// recurse through sub-objects
	for (int i=0; i<robj->Get_Num_Sub_Objects(); i++) {
		RenderObjClass * sub_obj = robj->Get_Sub_Object(i);
		getRenderCostRecursive(rc,sub_obj);
		REF_PTR_RELEASE(sub_obj);
	}

	// Only consider visible sub-objects.  Some vehicles have hidden headlight cones for example
	// that are not normally rendered.
	if (robj->Is_Not_Hidden_At_All()) {

		// collect stats from meshes
		if (robj->Class_ID() == RenderObjClass::CLASSID_MESH) {
			MeshClass * mesh = (MeshClass*)robj;
			MeshModelClass * model = mesh->Peek_Model();
			if (model != NULL)
			{	if (model->Get_Flag(MeshGeometryClass::SORT))
					rc.addSortedMeshes(1);
				if (model->Get_Flag(MeshGeometryClass::SKIN))
					rc.addSkinMeshes(1);
			}

			rc.addDrawCalls(mesh->Get_Draw_Call_Count());
		}

		// collect bone stats.
		const HTreeClass * htree = robj->Get_HTree();
		if (htree != NULL) {
			rc.addBones(htree->Num_Pivots());
		}
	}
}
#endif //RTS_DEBUG

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::setFullyObscuredByShroud(Bool fullyObscured)
{
	if (m_fullyObscuredByShroud != fullyObscured)
	{
		m_fullyObscuredByShroud = fullyObscured;

		if (m_shadow)
			m_shadow->enableShadowInvisible(m_fullyObscuredByShroud);
		if (m_terrainDecal)
			m_terrainDecal->enableShadowInvisible(m_fullyObscuredByShroud);

		doStartOrStopParticleSys();
	}
}

//-------------------------------------------------------------------------------------------------
static Bool isAnimationComplete(RenderObjClass* r)
{
	if (r && r->Class_ID() == RenderObjClass::CLASSID_HLOD)
	{
		HLodClass *hlod = (HLodClass*)r;
		return hlod->Is_Animation_Complete();
	}

	return true;
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::adjustAnimSpeedToMovementSpeed()
{
	// if the cur animation has a "distance covered" number, try to throttle
	// the animation rate to match our actual speed.
	Real dist = getCurAnimDistanceCovered();
	if (dist > 0.0f)
	{
		const Object *obj = getDrawable()->getObject();
		if (obj)
		{
			const PhysicsBehavior* physics = obj->getPhysics();
			if (physics)
			{
				Real speed = physics->getVelocityMagnitude();	// in dist/frame
				if (speed > 0.0f)
				{
					// if distance-covered is specified for this anim, adjust the frame rate
					// so it appears to sync with our unit speed
					Real desiredDurationInMsec = dist / speed * MSEC_PER_LOGICFRAME_REAL;
					setCurAnimDurationInMsec(desiredDurationInMsec);
				}
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::adjustTransformMtx(Matrix3D& mtx) const
{
	const W3DModelDrawModuleData* d = getW3DModelDrawModuleData();

#ifdef CACHE_ATTACH_BONE
	const Vector3* offset = d->getAttachToDrawableBoneOffset(getDrawable());
	if (offset)
	{
		Vector3 tmp = mtx.Rotate_Vector(*offset);
		mtx.Adjust_X_Translation(tmp.X);
		mtx.Adjust_Y_Translation(tmp.Y);
		mtx.Adjust_Z_Translation(tmp.Z);
	}
#else
	if (d->m_attachToDrawableBone.isNotEmpty())
	{
		// override the mtx in this case. yes, call drawable, since the bone in question is
		// likely to be in another module!
		Matrix3D boneMtx;
		if (getDrawable()->getCurrentWorldspaceClientBonePositions(d->m_attachToDrawableBone.str(), boneMtx))
		{
			mtx = boneMtx;
		}
		else
		{
			DEBUG_LOG(("m_attachToDrawableBone %s not found",getW3DModelDrawModuleData()->m_attachToDrawableBone.str()));
		}
	}
#endif

	if (m_curState->m_flags & (1<<ADJUST_HEIGHT_BY_CONSTRUCTION_PERCENT))
	{
		const Object *obj = getDrawable()->getObject();
		if (obj)
		{
			Real pct = obj->getConstructionPercent();
			if (pct >= 0.0f)
			{
				Real height = obj->getGeometryInfo().getMaxHeightAbovePosition();
				mtx.Translate_Z(-height + (height * pct / 100.0f));
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::doDrawModule(const Matrix3D* transformMtx)
{
	// update whether or not we should be animating.
	setPauseAnimation( !getDrawable()->getShouldAnimate(getW3DModelDrawModuleData()->m_animationsRequirePower) );

	Matrix3D scaledTransform;
	if (getDrawable()->getInstanceScale() != 1.0f)
	{
		// do custom scaling of the W3D model.
		scaledTransform = *transformMtx;
		scaledTransform.Scale(getDrawable()->getInstanceScale());
		transformMtx = &scaledTransform;
		if (m_renderObject)
			m_renderObject->Set_ObjectScale(getDrawable()->getInstanceScale());
	}

	if (isAnimationComplete(m_renderObject))
	{
		if (m_curState != NULL && m_nextState != NULL)
		{
			//DEBUG_LOG(("transition %s is complete",m_curState->m_description.str()));
			const ModelConditionInfo* nextState = m_nextState;
			UnsignedInt nextDuration = m_nextStateAnimLoopDuration;
			m_nextState = NULL;
			m_nextStateAnimLoopDuration = NO_NEXT_DURATION;
			setModelState(nextState);
			if (nextDuration != NO_NEXT_DURATION)
			{
				//DEBUG_LOG(("restoring pending duration of %d frames",nextDuration));
				setAnimationLoopDuration(nextDuration);
			}
		}

		if (m_renderObject &&
					m_curState != NULL &&
					m_whichAnimInCurState != -1)
		{
			if (m_curState->m_animations[m_whichAnimInCurState].isIdleAnim())
			{
				//DEBUG_LOG(("randomly switching to new idle state!"));

				// state hasn't changed, if it's been awhile, switch the idle anim
				// (yes, that's right: pass curState for prevState)
				adjustAnimation(m_curState, -1.0);
			}
			else if (testFlagBit(m_curState->m_flags, RESTART_ANIM_WHEN_COMPLETE))
			{
				adjustAnimation(m_curState, -1.0);
			}
		}
	}

	adjustAnimSpeedToMovementSpeed();

	// set our position in the render object to our position of the drawable
	if (m_renderObject)
	{
		Matrix3D mtx = *transformMtx;
		adjustTransformMtx(mtx);
		m_renderObject->Set_Transform(mtx);
	}

	handleClientTurretPositioning();
	recalcBonesForClientParticleSystems();
	handleClientRecoil();

}

//-------------------------------------------------------------------------------------------------
const ModelConditionInfo* W3DModelDraw::findTransitionForSig(TransitionSig sig) const
{
	const TransitionMap& transitionMap = getW3DModelDrawModuleData()->m_transitionMap;
	TransitionMap::const_iterator it = transitionMap.find(sig);
	if (it != transitionMap.end())
	{
		return &(*it).second;
	}
	return NULL;
}

//-------------------------------------------------------------------------------------------------
Real W3DModelDraw::getCurrentAnimFraction() const
{
	if (m_curState != NULL
			&& isAnyMaintainFrameFlagSet(m_curState->m_flags)
			&& m_renderObject != NULL
			&& m_renderObject->Class_ID() == RenderObjClass::CLASSID_HLOD)
	{
		float framenum, dummy;
		int mode, numFrames;

		HLodClass* hlod = (HLodClass*)m_renderObject;
		/*HAnimClass* anim =*/ hlod->Peek_Animation_And_Info(framenum, numFrames, mode, dummy);
		if (framenum < 0.0)
			return 0.0;
		else if (framenum >= numFrames)
			return 1.0;
		else
			return (Real)framenum / ((Real)numFrames - 1);
	}
	else
	{
		return -1.0;
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::adjustAnimation(const ModelConditionInfo* prevState, Real prevAnimFraction)
{
	if (!m_curState)
		return;

	// if the current state has m_animations associated, do the right thing
	Int numAnims = m_curState->m_animations.size();
	if (numAnims > 0)
	{
		if (numAnims == 1)
		{
			m_whichAnimInCurState = 0;
		}
		else if (prevState == m_curState)
		{
			// select an idle anim different than the currently playing one
			Int animToAvoid = m_whichAnimInCurState;
			while (m_whichAnimInCurState == animToAvoid)
				m_whichAnimInCurState = GameClientRandomValue(0, numAnims-1);
		}
		else
		{
			m_whichAnimInCurState = GameClientRandomValue(0, numAnims-1);
		}

		const W3DAnimationInfo& animInfo = m_curState->m_animations[m_whichAnimInCurState];

		HAnimClass* animHandle = animInfo.getAnimHandle();	// note that this now returns an ADDREFED handle, which must be released by the caller!
		if (m_renderObject && animHandle)
		{
			Int startFrame = 0;
			if (m_curState->m_mode == RenderObjClass::ANIM_MODE_ONCE_BACKWARDS ||
					m_curState->m_mode == RenderObjClass::ANIM_MODE_LOOP_BACKWARDS)
			{
				startFrame = animHandle->Get_Num_Frames()-1;
			}

			if (testFlagBit(m_curState->m_flags, RANDOMIZE_START_FRAME))
			{
				startFrame = GameClientRandomValue(0, animHandle->Get_Num_Frames()-1);
			}
			else if (testFlagBit(m_curState->m_flags, START_FRAME_FIRST))
			{
				startFrame = 0;
			}
			else if (testFlagBit(m_curState->m_flags, START_FRAME_LAST))
			{
				startFrame = animHandle->Get_Num_Frames()-1;
			}
			// order is important here: MAINTAIN_FRAME_ACROSS_STATES is overridden by the other bits, above.
			else if (isAnyMaintainFrameFlagSet(m_curState->m_flags) &&
					prevState &&
					prevState != m_curState &&
					isAnyMaintainFrameFlagSet(prevState->m_flags) &&
					isCommonMaintainFrameFlagSet(m_curState->m_flags, prevState->m_flags) &&
					prevAnimFraction >= 0.0)
			{
				startFrame = REAL_TO_INT(prevAnimFraction * animHandle->Get_Num_Frames()-1);
			}

			m_renderObject->Set_Animation(animHandle, startFrame, m_curState->m_mode);
			REF_PTR_RELEASE(animHandle);
			animHandle = NULL;

			if (m_renderObject->Class_ID() == RenderObjClass::CLASSID_HLOD)
			{
				HLodClass *hlod = (HLodClass*)m_renderObject;
				Real factor = GameClientRandomValueReal( m_curState->m_animMinSpeedFactor, m_curState->m_animMaxSpeedFactor );
				hlod->Set_Animation_Frame_Rate_Multiplier( factor );
			}
		}

	}
	else
	{
		m_whichAnimInCurState = -1;
	}

}

//-------------------------------------------------------------------------------------------------
Bool W3DModelDraw::setCurAnimDurationInMsec(Real desiredDurationInMsec)
{
	if (m_renderObject && m_renderObject->Class_ID() == RenderObjClass::CLASSID_HLOD)
	{
		HLodClass* hlod = (HLodClass*)m_renderObject;
		HAnimClass* anim = hlod->Peek_Animation();
		if (anim)
		{
			Real naturalDurationInMsec = anim->Get_Num_Frames() * 1000.0f / anim->Get_Frame_Rate();
			if (naturalDurationInMsec > 0.0f && desiredDurationInMsec > 0.0f)
			{
				Real multiplier = naturalDurationInMsec / desiredDurationInMsec;
				hlod->Set_Animation_Frame_Rate_Multiplier(multiplier);
				return true;
			}
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
Real W3DModelDraw::getCurAnimDistanceCovered() const
{
	if (m_curState != NULL && m_whichAnimInCurState >= 0)
	{
		const W3DAnimationInfo& animInfo = m_curState->m_animations[m_whichAnimInCurState];
	#if defined(RTS_DEBUG)
		if (TheSkateDistOverride != 0.0f)
			return TheSkateDistOverride;
	#endif
		return animInfo.getDistanceCovered();
	}
	return 0.0f;
}

//-------------------------------------------------------------------------------------------------
/**
	Utility function to make it easier to recursively hide all objects connected to a certain bone.
	We will hide all objects connected to bones which are children of boneIdx
*/
static void doHideShowBoneSubObjs(Bool state, Int numSubObjects, Int boneIdx, RenderObjClass *fullObject, const HTreeClass *htree)
{
#if 1	//(gth) fixed and tested this version
	for (Int i=0; i < numSubObjects; i++)
	{
		bool is_child = false;
		Int parentBoneIndex = fullObject->Get_Sub_Object_Bone_Index(0, i);

		while (parentBoneIndex != 0)
		{
			parentBoneIndex = htree->Get_Parent_Index(parentBoneIndex);

			if (parentBoneIndex == boneIdx)
			{
				is_child = true;
				break;
			}
		}

		if (is_child)
		{
			RenderObjClass* childObject = fullObject->Get_Sub_Object(i);
			childObject->Set_Hidden(state);
			childObject->Release_Ref();
		}
	}
#endif
#if 0	//old slow version

  for (Int i=0; i < numSubObjects; i++)
  {
  	Int childBoneIndex = fullObject->Get_Sub_Object_Bone_Index(0, i);
  	Int parentIndex = htree->Get_Parent_Index(childBoneIndex);
  	if (childBoneIndex == parentIndex)
  		continue;

  	if (parentIndex == boneIdx)	// this object has our subobject as parent so copy hide state
  	{
  		RenderObjClass* childObject = fullObject->Get_Sub_Object(i);
  		// recurse down the hierarchy to hide all sub-children
  		doHideShowBoneSubObjs(state, numSubObjects, childBoneIndex, fullObject, htree);
		childObject->Set_Hidden(state);
  		childObject->Release_Ref();
  	}
  }
#endif
}

//-------------------------------------------------------------------------------------------------
void ModelConditionInfo::WeaponBarrelInfo::setMuzzleFlashHidden(RenderObjClass *fullObject, Bool hide) const
{
	if (fullObject)
	{
		RenderObjClass* childObject = fullObject->Get_Sub_Object_On_Bone(0, m_muzzleFlashBone);
		if (childObject)
		{
			childObject->Set_Hidden(hide);
			childObject->Release_Ref();
		}
		else
		{
			DEBUG_CRASH(("*** ASSET ERROR: childObject %s not found in setMuzzleFlashHidden()",m_muzzleFlashBoneName.str()));
		}
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::doHideShowSubObjs(const std::vector<ModelConditionInfo::HideShowSubObjInfo>* vec)
{
	if (!m_renderObject)
		return;

	if (!vec->empty())
	{
		for (std::vector<ModelConditionInfo::HideShowSubObjInfo>::const_iterator it = vec->begin(); it != vec->end(); ++it)
		{
			Int objIndex;
			RenderObjClass* subObj;

			if ((subObj = m_renderObject->Get_Sub_Object_By_Name(it->subObjName.str(), &objIndex)) != NULL)
			{
				subObj->Set_Hidden(it->hide);

				const HTreeClass *htree = m_renderObject->Get_HTree();
				if (htree)
				{
					//get the bone of this subobject so we can hide all other child objects that use this bone
					//as a parent.
					Int boneIdx = m_renderObject->Get_Sub_Object_Bone_Index(0, objIndex);
					doHideShowBoneSubObjs(it->hide, m_renderObject->Get_Num_Sub_Objects(), boneIdx, m_renderObject, htree);
				}
				subObj->Release_Ref();
			}
			else
			{
				DEBUG_CRASH(("*** ASSET ERROR: SubObject %s not found (%s)!",it->subObjName.str(),getDrawable()->getTemplate()->getName().str()));
			}
		}
	}

	//Kris (added Aug 2002)
	//This is really important as it allows a person to override modelcondition show/hide objects. Used by the A10 strike
	//September 2002 -- now used by the UpgradeSubObject system.
	if( !m_subObjectVec.empty() )
	{
		updateSubObjects();
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::stopClientParticleSystems()
{
	for (std::vector<ParticleSysTrackerType>::const_iterator it = m_particleSystemIDs.begin(); it != m_particleSystemIDs.end(); ++it)
	//for (std::vector<ParticleSystemID>::const_iterator it = m_particleSystemIDs.begin(); it != m_particleSystemIDs.end(); ++it)
	{
		ParticleSystem *sys = TheParticleSystemManager->findParticleSystem((*it).id);
		if (sys != NULL)
		{
			// this can be NULL
			sys->destroy();
		}
	}
	m_particleSystemIDs.clear();
}

//-------------------------------------------------------------------------------------------------
/*
	DANGER WARNING READ ME
	DANGER WARNING READ ME
	DANGER WARNING READ ME

	This function must not EVER do ANYTHING which can in any way, shape, or form
	affect GameLogic in any way; if it does, net desyncs will occur and we
	will all lose our jobs. This must remain pure-client-only, and ensure
	that nothing it does can be detected, even in read-only form, by GameLogic!

	DANGER WARNING READ ME
	DANGER WARNING READ ME
	DANGER WARNING READ ME
*/
void W3DModelDraw::handleClientTurretPositioning()
{
	if (!m_curState || !(m_curState->m_validStuff & ModelConditionInfo::TURRETS_VALID))
		return;

	for (int tslot = 0; tslot < MAX_TURRETS; ++tslot)
	{
		const ModelConditionInfo::TurretInfo& tur = m_curState->m_turrets[tslot];
		Real turretAngle = 0;
		Real turretPitch = 0;
		if (tur.m_turretAngleBone || tur.m_turretPitchBone)
		{
			const Object *obj = getDrawable()->getObject();
			if (obj)
			{
				const AIUpdateInterface* ai = obj->getAIUpdateInterface();
				if (ai)
					ai->getTurretRotAndPitch((WhichTurretType)tslot, &turretAngle, &turretPitch);
			}

			// do turret, if any
			if (tur.m_turretAngleBone != 0)
			{
				if (m_curState)
					turretAngle += tur.m_turretArtAngle;
				Matrix3D turretXfrm(1);
				turretXfrm.Rotate_Z(turretAngle);
				if (m_renderObject)
				{
					m_renderObject->Capture_Bone( tur.m_turretAngleBone );
					m_renderObject->Control_Bone( tur.m_turretAngleBone, turretXfrm );
				}
			}

			// do turret pitch, if any
			if (tur.m_turretPitchBone != 0)
			{
				if (m_curState)
					turretPitch += tur.m_turretArtPitch;
				Matrix3D turretPitchXfrm(1);
				turretPitchXfrm.Rotate_Y(-turretPitch);
				if (m_renderObject)
				{
					m_renderObject->Capture_Bone( tur.m_turretPitchBone );
					m_renderObject->Control_Bone( tur.m_turretPitchBone, turretPitchXfrm );
				}
			}
		}
	}
}


//void W3DModelDraw::handleClientFlagPositioning()
//{
//	const ModelConditionInfo::TurretInfo& tur = m_curState->m_turrets[tslot];
//	Real turretAngle = TheGlobalData->m_downwindAngle;
//	if (tur.m_flagAngleBone )
//	{
//		Matrix3D turretXfrm(1);
//		turretXfrm.Rotate_Z(turretAngle);
//		if (m_renderObject)
//		{
//			m_renderObject->Capture_Bone( tur.m_turretAngleBone );
//			m_renderObject->Control_Bone( tur.m_turretAngleBone, turretXfrm );
//		}
//	}
//}

//-------------------------------------------------------------------------------------------------
/*
	Note that, strictly speaking this code is WRONG, since it assumes it will get called every frame,
	but in fact, will only get called every frame that it is visible. In practice, this isn't a big problem,
	but...

	@todo fix me someday (srj)
*/
void W3DModelDraw::handleClientRecoil()
{
	const W3DModelDrawModuleData* d = getW3DModelDrawModuleData();
	if (!(m_curState->m_validStuff & ModelConditionInfo::BARRELS_VALID))
	{
		return;
	}

	// do recoil, if any
	for (int wslot = 0; wslot < WEAPONSLOT_COUNT; ++wslot)
	{
		if (!m_curState->m_hasRecoilBonesOrMuzzleFlashes[wslot])
			continue;

		const ModelConditionInfo::WeaponBarrelInfoVec& barrels = m_curState->m_weaponBarrelInfoVec[wslot];
		WeaponRecoilInfoVec& recoils = m_weaponRecoilInfoVec[wslot];
		Int count = barrels.size();
		Int recoilCount = recoils.size();
		DEBUG_ASSERTCRASH(count == recoilCount, ("Barrel count != recoil count!"));
		count = (count>recoilCount)?recoilCount:count;
		for (Int i = 0; i < count; ++i)
		{
			if (barrels[i].m_muzzleFlashBone != 0)
			{
				Bool hidden = recoils[i].m_state != WeaponRecoilInfo::RECOIL_START;
				//DEBUG_LOG(("adjust muzzleflash %08lx for Draw %08lx state %s to %d at frame %d",subObjToHide,this,m_curState->m_description.str(),hidden?1:0,TheGameLogic->getFrame()));
				barrels[i].setMuzzleFlashHidden(m_renderObject, hidden);
			}

			const Real TINY_RECOIL = 0.01f;
			if (barrels[i].m_recoilBone != 0)
			{
				switch (recoils[i].m_state )
				{
					case WeaponRecoilInfo::IDLE:
						// nothing
						break;

					case WeaponRecoilInfo::RECOIL_START:
					case WeaponRecoilInfo::RECOIL:
						recoils[i].m_shift += recoils[i].m_recoilRate;
						recoils[i].m_recoilRate *= d->m_recoilDamping;			// recoil decelerates
						if (recoils[i].m_shift >= d->m_maxRecoil)
						{
							recoils[i].m_shift = d->m_maxRecoil;
							recoils[i].m_state = WeaponRecoilInfo::SETTLE;
						}
						else if (fabs(recoils[i].m_recoilRate) < TINY_RECOIL)
						{
							recoils[i].m_state = WeaponRecoilInfo::SETTLE;
						}
						break;

					case WeaponRecoilInfo::SETTLE:
						recoils[i].m_shift -= d->m_recoilSettle;
						if (recoils[i].m_shift <= 0.0f)
						{
							recoils[i].m_shift = 0.0f;
							recoils[i].m_state = WeaponRecoilInfo::IDLE;
						}
						break;
				}

				Matrix3D gunXfrm;
				gunXfrm.Make_Identity();
				gunXfrm.Translate_X( -recoils[i].m_shift );
				//DEBUG_ASSERTLOG(recoils[i].m_shift==0.0f,("adjust bone %d by %f",recoils[i].m_recoilBone,recoils[i].m_shift));

				if (m_renderObject)
				{
					m_renderObject->Capture_Bone( barrels[i].m_recoilBone );
					m_renderObject->Control_Bone( barrels[i].m_recoilBone, gunXfrm );
				}
			}
			else
			{
				recoils[i].m_state = WeaponRecoilInfo::IDLE;
				//DEBUG_LOG(("reset Draw %08lx state %08lx",this,m_curState));
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
/*
	DANGER WARNING READ ME
	DANGER WARNING READ ME
	DANGER WARNING READ ME

	This function must not EVER do ANYTHING which can in any way, shape, or form
	affect GameLogic in any way; if it does, net desyncs will occur and we
	will all lose our jobs. This must remain pure-client-only, and ensure
	that nothing it does can be detected, even in read-only form, by GameLogic!

	DANGER WARNING READ ME
	DANGER WARNING READ ME
	DANGER WARNING READ ME
*/
void W3DModelDraw::recalcBonesForClientParticleSystems()
{
	if (m_needRecalcBoneParticleSystems)
	{
		const Drawable* drawable = getDrawable();
		if (drawable != NULL )
		{

			if( m_curState != NULL && drawable->testDrawableStatus( DRAWABLE_STATUS_NO_STATE_PARTICLES ) == FALSE )
			{
				for (std::vector<ParticleSysBoneInfo>::const_iterator it = m_curState->m_particleSysBones.begin(); it != m_curState->m_particleSysBones.end(); ++it)
				{
					ParticleSystem *sys = TheParticleSystemManager->createParticleSystem(it->particleSystemTemplate);
					if (sys != NULL)
					{
						Coord3D pos;
						pos.zero();
						Real rotation = 0.0f;

						Int boneIndex = m_renderObject ? m_renderObject->Get_Bone_Index(it->boneName.str()) : 0;
						if (boneIndex != 0)
						{
							// ugh... kill the mtx so we get it in modelspace, not world space
							Matrix3D originalTransform = m_renderObject->Get_Transform();	// save the transform
							Matrix3D tmp(true);
							tmp.Scale(getDrawable()->getScale());
							m_renderObject->Set_Transform(tmp);					// set to identity transform

							const Matrix3D boneTransform = m_renderObject->Get_Bone_Transform(boneIndex);
							Vector3 vpos = boneTransform.Get_Translation();
							rotation = boneTransform.Get_Z_Rotation();

							m_renderObject->Set_Transform(originalTransform);					// restore it

							pos.x = vpos.X;
							pos.y = vpos.Y;
							pos.z = vpos.Z;
						}

						// got the bone position...
						sys->setPosition(&pos);

						// and the direction, so that the system is oriented like the bone is
						sys->rotateLocalTransformZ(rotation);

						// Attatch it to the object...
						sys->attachToDrawable(drawable);

						// important: mark it as do-not-save, since we'll just re-create it when we reload.
						sys->setSaveable(FALSE);

						if (drawable->isDrawableEffectivelyHidden() || m_fullyObscuredByShroud)
						{
							sys->stop(); // don't start the systems for drawables that are hidden.
						}

						// store the particle system id so we can kill it later.

						ParticleSysTrackerType tracker;
						tracker.id = sys->getSystemID();
						tracker.boneIndex = boneIndex;

						m_particleSystemIDs.push_back( tracker );
					}
				}
			}
		}
		m_needRecalcBoneParticleSystems = false;
	}
}



//-------------------------------------------------------------------------------------------------
/*
	DANGER WARNING READ ME
	DANGER WARNING READ ME
	DANGER WARNING READ ME

	This function must not EVER do ANYTHING which can in any way, shape, or form
	affect GameLogic in any way; if it does, net desyncs will occur and we
	will all lose our jobs. This must remain pure-client-only, and ensure
	that nothing it does can be detected, even in read-only form, by GameLogic!

	DANGER WARNING READ ME
	DANGER WARNING READ ME
	DANGER WARNING READ ME
*/
Bool W3DModelDraw::updateBonesForClientParticleSystems()
{
	const Drawable* drawable = getDrawable();
	if (drawable != NULL && m_curState != NULL && m_renderObject != NULL )
	{
		for (std::vector<ParticleSysTrackerType>::const_iterator it = m_particleSystemIDs.begin(); it != m_particleSystemIDs.end(); ++it)
		{
			ParticleSystem *sys = TheParticleSystemManager->findParticleSystem((*it).id);
			Int boneIndex = (*it).boneIndex;
			if ( (sys != NULL) && (boneIndex != 0)  )
			{
				Matrix3D originalTransform = m_renderObject->Get_Transform();
				Matrix3D tmp(1);
				tmp.Scale(drawable->getScale());
				m_renderObject->Set_Transform(tmp);

				const Matrix3D boneTransform = m_renderObject->Get_Bone_Transform(boneIndex);// just a little worried about state changes
				Vector3 vpos = boneTransform.Get_Translation();
				Real rotation = boneTransform.Get_Z_Rotation();

				m_renderObject->Set_Transform(originalTransform);

				Coord3D pos;
				pos.x = vpos.X;
				pos.y = vpos.Y;
				pos.z = vpos.Z;

				sys->setPosition(&pos);
				sys->rotateLocalTransformZ(rotation);

			}
		}

	}

	return TRUE;

}




//-------------------------------------------------------------------------------------------------
void W3DModelDraw::setTerrainDecal(TerrainDecalType type)
{
	if (m_terrainDecal)
		m_terrainDecal->release();

	m_terrainDecal = NULL;

	if (type == TERRAIN_DECAL_NONE || type >= TERRAIN_DECAL_MAX)
		//turning off decals on this object. (or bad value.)
		return;

	const ThingTemplate *tmplate=getDrawable()->getTemplate();

	//create a new terrain decal
	Shadow::ShadowTypeInfo decalInfo;
	decalInfo.allowUpdates = FALSE;	//shadow image will never update
	decalInfo.allowWorldAlign = TRUE;	//shadow image will wrap around world objects
	decalInfo.m_type = SHADOW_ALPHA_DECAL;

	//decalInfo.m_type = SHADOW_ADDITIVE_DECAL;//temporary kluge to test graphics

	strcpy(decalInfo.m_ShadowName,TerrainDecalTextureName[type]);
	decalInfo.m_sizeX = tmplate->getShadowSizeX();
	decalInfo.m_sizeY = tmplate->getShadowSizeY();
	decalInfo.m_offsetX = tmplate->getShadowOffsetX();
	decalInfo.m_offsetY = tmplate->getShadowOffsetY();
	if (TheProjectedShadowManager)
		m_terrainDecal = TheProjectedShadowManager->addDecal(m_renderObject,&decalInfo);
	if (m_terrainDecal)
	{	m_terrainDecal->enableShadowInvisible(m_fullyObscuredByShroud);
		m_terrainDecal->enableShadowRender(m_shadowEnabled);
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::setTerrainDecalSize(Real x, Real y)
{
	if (m_terrainDecal)
	{
		m_terrainDecal->setSize(x,y);
	}
}
//-------------------------------------------------------------------------------------------------
void W3DModelDraw::setTerrainDecalOpacity(Real o)
{
	if (m_terrainDecal)
	{
		m_terrainDecal->setOpacity((Int)(255.0f * o));
	}
}


//-------------------------------------------------------------------------------------------------
void W3DModelDraw::nukeCurrentRender(Matrix3D* xform)
{
	// this needs to be "dirtied" so that the new pausing of the animation will be triggered.
	m_pauseAnimation = false;

	// changing geometry, so we need to remove shadow if present
	if (m_shadow)
		m_shadow->release();
	m_shadow = NULL;

	if(m_terrainDecal)
		m_terrainDecal->release();
	m_terrainDecal = NULL;

	// remove existing render object from the scene
	if (m_renderObject)
	{
		// save the transform for the new model
		if (xform)
			*xform = m_renderObject->Get_Transform();
		if (W3DDisplay::m_3DScene != NULL)
			W3DDisplay::m_3DScene->Remove_Render_Object(m_renderObject);
		REF_PTR_RELEASE(m_renderObject);
		m_renderObject = NULL;
	}
	else
	{
		if (xform)
		{
			*xform = *getDrawable()->getTransformMatrix();
		}
	}
}

//-------------------------------------------------------------------------------------------------
#if defined(RTS_DEBUG)	//art wants to see buildings without flags as a test.
void W3DModelDraw::hideGarrisonFlags(Bool hide)
{
	if (!m_renderObject)
		return;

	Int objIndex;
	RenderObjClass* subObj;

	if ((subObj = m_renderObject->Get_Sub_Object_By_Name("POLE", &objIndex)) != NULL)
	{
		subObj->Set_Hidden(hide);

		const HTreeClass *htree = m_renderObject->Get_HTree();
		if (htree)
		{
			//get the bone of this subobject so we can hide all other child objects that use this bone
			//as a parent.
			Int boneIdx = m_renderObject->Get_Sub_Object_Bone_Index(0, objIndex);
			doHideShowBoneSubObjs(hide, m_renderObject->Get_Num_Sub_Objects(), boneIdx, m_renderObject, htree);
		}
		subObj->Release_Ref();
	}
}
#endif

//-------------------------------------------------------------------------------------------------
/** Hides all subobjects which are headlights.  Used to disable lights on models during the day.*/
void W3DModelDraw::hideAllHeadlights(Bool hide)
{
	if (m_renderObject)
	{
		for (Int subObj = 0; subObj < m_renderObject->Get_Num_Sub_Objects(); subObj++)
		{
			RenderObjClass* test = m_renderObject->Get_Sub_Object(subObj);
			if (strstr(test->Get_Name(),"HEADLIGHT"))
			{
				test->Set_Hidden(hide);
			}
			test->Release_Ref();
		}
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::hideAllMuzzleFlashes(const ModelConditionInfo* state, RenderObjClass* renderObject)
{
	if (!state || !renderObject)
		return;

	if (!(state->m_validStuff & ModelConditionInfo::BARRELS_VALID))
	{
		return;
	}

	for (int wslot = 0; wslot < WEAPONSLOT_COUNT; ++wslot)
	{
		if (!state->m_hasRecoilBonesOrMuzzleFlashes[wslot])
			continue;

		const ModelConditionInfo::WeaponBarrelInfoVec& barrels = state->m_weaponBarrelInfoVec[wslot];
		for (ModelConditionInfo::WeaponBarrelInfoVec::const_iterator it = barrels.begin(); it != barrels.end(); ++it)
		{
			if (it->m_muzzleFlashBone != 0)
			{
				it->setMuzzleFlashHidden(renderObject, true);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
static Bool turretNamesDiffer(const ModelConditionInfo* a, const ModelConditionInfo* b)
{
	if (!(a->m_validStuff & ModelConditionInfo::TURRETS_VALID) || !(b->m_validStuff & ModelConditionInfo::TURRETS_VALID))
	{
		return true;
	}
	for (int i = 0; i < MAX_TURRETS; ++i)
		if (a->m_turrets[i].m_turretAngleNameKey != b->m_turrets[i].m_turretAngleNameKey ||
				a->m_turrets[i].m_turretPitchNameKey != b->m_turrets[i].m_turretPitchNameKey)
			return true;

	return false;
}

//-------------------------------------------------------------------------------------------------
/** Change the model for this drawable. If color is non-zero, it will also apply team color to the
	model */
//-------------------------------------------------------------------------------------------------
void W3DModelDraw::setModelState(const ModelConditionInfo* newState)
{
	DEBUG_ASSERTCRASH(newState, ("invalid state in W3DModelDraw::setModelState"));

#ifdef DEBUG_OBJECT_ID_EXISTS
	if (getDrawable() && getDrawable()->getObject() && getDrawable()->getObject()->getID() == TheObjectIDToDebug)
	{
		DEBUG_LOG(("REQUEST switching to state %s for obj %s %d",newState->m_description.str(),getDrawable()->getObject()->getTemplate()->getName().str(),getDrawable()->getObject()->getID()));
	}
#endif
	const ModelConditionInfo* nextState = NULL;
	if (m_curState != NULL && newState != NULL)
	{
		// if the requested state is the current state (and nothing is pending),
		// or if the requested state is pending, just punt.
		//
		// note that if the requested state matches the current state AND
		// we also have a pending state, then we nuke both of 'em. This sounds
		// weird, but is deliberate: nextState and newState will always be "real" states (or null),
		// leaving us these possibilities:
		//
		// -- curState is a transition state
		//		--in this case, new state should take precedence (unless there's a wait-to-finish marking,
		//		which would be handled by the else clause)
		//
		// -- curState is a real state, nextState is a real state with a wait-to-finish clause
		//		-- if curState == newState, presume that we should restart curState, thus punting thepending nextState
		//		-- if curState != newState, presume we should switch to it, thus punting curState and nextState
		//
		// -- curState is a real state, nextState is a real state without a wait-to-finish clause
		//		-- should be impossible!

		if ((m_curState == newState && m_nextState == NULL) ||
				(m_curState != NULL && m_nextState == newState))
		{
#ifdef DEBUG_OBJECT_ID_EXISTS
			if (getDrawable() && getDrawable()->getObject() && getDrawable()->getObject()->getID() == TheObjectIDToDebug)
			{
				DEBUG_LOG(("IGNORE duplicate state %s for obj %s %d",newState->m_description.str(),getDrawable()->getObject()->getTemplate()->getName().str(),getDrawable()->getObject()->getID()));
			}
#endif
			// I don't think he'll be interested...
			// he's already got one, you see
			return;
		}
		// check for allow-to-finish implicit transitions
		else if (newState != m_curState &&
							newState->m_allowToFinishKey != NAMEKEY_INVALID &&
							newState->m_allowToFinishKey == m_curState->m_transitionKey &&
							m_renderObject &&
							!isAnimationComplete(m_renderObject))
		{
#ifdef DEBUG_OBJECT_ID_EXISTS
			if (getDrawable() && getDrawable()->getObject() && getDrawable()->getObject()->getID() == TheObjectIDToDebug)
			{
				DEBUG_LOG(("ALLOW_TO_FINISH state %s for obj %s %d",newState->m_description.str(),getDrawable()->getObject()->getTemplate()->getName().str(),getDrawable()->getObject()->getID()));
			}
#endif
			m_nextState = newState;
			m_nextStateAnimLoopDuration = NO_NEXT_DURATION;
			return;
		}
		// check for a normal->normal state transition
		else if (newState != m_curState &&
					m_curState->m_transitionKey != NAMEKEY_INVALID &&
					newState->m_transitionKey != NAMEKEY_INVALID)
		{
			TransitionSig sig = buildTransitionSig(m_curState->m_transitionKey, newState->m_transitionKey);
			const ModelConditionInfo* transState = findTransitionForSig(sig);
			if (transState != NULL)
			{
#ifdef DEBUG_OBJECT_ID_EXISTS
				if (getDrawable() && getDrawable()->getObject() && getDrawable()->getObject()->getID() == TheObjectIDToDebug)
				{
					DEBUG_LOG(("using TRANSITION state %s before requested state %s for obj %s %d",transState->m_description.str(),newState->m_description.str(),getDrawable()->getObject()->getTemplate()->getName().str(),getDrawable()->getObject()->getID()));
				}
#endif
				nextState = newState;
				newState = transState;
			}
		}
	}

	// get this here, before we change anything... we'll need it to pass to adjustAnimation (srj)
	Real prevAnimFraction = getCurrentAnimFraction();

	//
	// Set up any particle effects for the new model.  Also stop the old ones, note that we
	// will never allow the placement of new ones if the status bit is set that tells us
	// we should not automatically create particle systems for the model condition states
	//
	if( getDrawable()->testDrawableStatus( DRAWABLE_STATUS_NO_STATE_PARTICLES ) == FALSE )
		m_needRecalcBoneParticleSystems = true;

	// always stop particle systems now
	stopClientParticleSystems();

	// and always hide muzzle flashes, in case modelstate and model have crossed at the same time
	hideAllMuzzleFlashes(newState, m_renderObject);

	// note that different states might use the same model; for these, don't go thru the
	// expense of creating a new render-object. (exception: if color is changing, or subobjs are changing,
	// or a few other things...)
	if (m_curState == NULL ||
			newState->m_modelName != m_curState->m_modelName ||
			turretNamesDiffer(newState, m_curState)
			// srj sez: I'm not sure why we want to do the "hard stuff" if we have projectile bones; I think
			// it is a holdover from days gone by when bones were handled quite differently, rather than being cached.
			// but doing this hard stuff is a lot more work, and I think it's unnecessary, so let's remove this.
			//|| (newState->m_validStuff & ModelConditionInfo::HAS_PROJECTILE_BONES)
		)
	{
		Matrix3D transform;
		nukeCurrentRender(&transform);
		Drawable* draw = getDrawable();

		// create a new render object and set into drawable
		if (newState->m_modelName.isEmpty())
		{
			m_renderObject = NULL;
		}
		else
		{
			m_renderObject = W3DDisplay::m_assetManager->Create_Render_Obj(newState->m_modelName.str(), draw->getScale(), m_hexColor);
			DEBUG_ASSERTCRASH(m_renderObject, ("*** ASSET ERROR: Model %s not found!",newState->m_modelName.str()));
		}

		//BONEPOS_LOG(("validateStuff() from within W3DModelDraw::setModelState()"));
		//BONEPOS_DUMPREAL(draw->getScale());

		newState->validateStuff(m_renderObject, draw->getScale(), getW3DModelDrawModuleData()->m_extraPublicBones);
		// ensure that any muzzle flashes from the *new* state, start out hidden...
//		hideAllMuzzleFlashes(newState, m_renderObject);//moved to above
		rebuildWeaponRecoilInfo(newState);
		doHideShowSubObjs(&newState->m_hideShowVec);

#if defined(RTS_DEBUG)	//art wants to see buildings without flags as a test.
		if (TheGlobalData->m_hideGarrisonFlags && draw->isKindOf(KINDOF_STRUCTURE))
			hideGarrisonFlags(TRUE);
#endif

		const ThingTemplate *tmplate = draw->getTemplate();

		// set up tracks, if not already set.
		if (m_renderObject &&
				TheGlobalData->m_makeTrackMarks &&
				!m_trackRenderObject &&
				TheTerrainTracksRenderObjClassSystem != NULL &&
				!getW3DModelDrawModuleData()->m_trackFile.isEmpty())
		{
			m_trackRenderObject = TheTerrainTracksRenderObjClassSystem->bindTrack(m_renderObject, 1.0f*MAP_XY_FACTOR, getW3DModelDrawModuleData()->m_trackFile.str());
			if (draw && m_trackRenderObject)
				m_trackRenderObject->setOwnerDrawable(draw);
		}

		// set up shadows
		if (m_renderObject && TheW3DShadowManager && tmplate->getShadowType() != SHADOW_NONE)
		{
			Shadow::ShadowTypeInfo shadowInfo;
			strcpy(shadowInfo.m_ShadowName, tmplate->getShadowTextureName().str());
			DEBUG_ASSERTCRASH(shadowInfo.m_ShadowName[0] != '\0', ("this should be validated in ThingTemplate now"));
			shadowInfo.allowUpdates			= FALSE;		//shadow image will never update
			shadowInfo.allowWorldAlign	= TRUE;	//shadow image will wrap around world objects
			shadowInfo.m_type						= (ShadowType)tmplate->getShadowType();
			shadowInfo.m_sizeX					= tmplate->getShadowSizeX();
			shadowInfo.m_sizeY					= tmplate->getShadowSizeY();
			shadowInfo.m_offsetX				= tmplate->getShadowOffsetX();
			shadowInfo.m_offsetY				= tmplate->getShadowOffsetY();

			DEBUG_ASSERTCRASH(m_shadow == NULL, ("m_shadow is not NULL"));
			m_shadow = TheW3DShadowManager->addShadow(m_renderObject, &shadowInfo, draw);
			if (m_shadow)
			{	m_shadow->enableShadowInvisible(m_fullyObscuredByShroud);
				m_shadow->enableShadowRender(m_shadowEnabled);
			}
		}

		if( m_renderObject )
		{
			// set collision type for render object.  Used by WW3D2 collision code.
			if (tmplate->isKindOf(KINDOF_SELECTABLE))
			{
				m_renderObject->Set_Collision_Type( PICK_TYPE_SELECTABLE );
			}

			if( tmplate->isKindOf( KINDOF_SHRUBBERY ))
			{
				m_renderObject->Set_Collision_Type( PICK_TYPE_SHRUBBERY );
			}
			if( tmplate->isKindOf( KINDOF_MINE ))
			{
				m_renderObject->Set_Collision_Type( PICK_TYPE_MINES );
			}
			if( tmplate->isKindOf( KINDOF_FORCEATTACKABLE ))
			{
				m_renderObject->Set_Collision_Type( PICK_TYPE_FORCEATTACKABLE );
			}
			if( tmplate->isKindOf( KINDOF_CLICK_THROUGH ))
			{
				m_renderObject->Set_Collision_Type( 0 );
			}

			Object *obj = draw->getObject();
 			if( obj )
   		{

				// for non bridge objects we adjust some collision types
				if( obj->isKindOf( KINDOF_BRIDGE ) == FALSE &&
						obj->isKindOf( KINDOF_BRIDGE_TOWER ) == FALSE )
				{

					if( obj->isKindOf( KINDOF_STRUCTURE ) && draw->getModelConditionFlags().test( MODELCONDITION_RUBBLE ) )
					{
	 					//A dead building, -- don't allow the user to click on rubble! Treat it as a location instead.
	   				m_renderObject->Set_Collision_Type( 0 );
					}
					else if( obj->isEffectivelyDead() )
					{
 						//A dead object, -- don't allow the user to click on rubble/hulks! Treat it as a location instead.
	   				m_renderObject->Set_Collision_Type( 0 );
					}
				}
   		}

			// add render object to our scene
			if (W3DDisplay::m_3DScene != NULL)
				W3DDisplay::m_3DScene->Add_Render_Object(m_renderObject);

			// tie in our drawable as the user data pointer in the render object
			m_renderObject->Set_User_Data(draw->getDrawableInfo());

			setTerrainDecal(draw->getTerrainDecalType());

			//We created a new render object so we need to preserve the visibility state
			//of the previous render object.
			if (draw->isDrawableEffectivelyHidden())
			{
				m_renderObject->Set_Hidden(TRUE);
				if (m_shadow)
					m_shadow->enableShadowRender(FALSE);
				m_shadowEnabled = FALSE;
			}

			//
			// set the transform for the new model to that we saved before, we do this so that the
			// model transition is smooth and will immediately appear at the same orientation and location
			// as the previous one
			//
			m_renderObject->Set_Transform(transform);
			onRenderObjRecreated();
		}
	}
	else
	{

		//BONEPOS_LOG(("validateStuff() from within W3DModelDraw::setModelState()"));
		//BONEPOS_DUMPREAL(getDrawable()->getScale());

		newState->validateStuff(m_renderObject, getDrawable()->getScale(), getW3DModelDrawModuleData()->m_extraPublicBones);
		rebuildWeaponRecoilInfo(newState);

		// ensure that any muzzle flashes from the *previous* state, are hidden...
//		hideAllMuzzleFlashes(m_curState, m_renderObject);// moved to above

		doHideShowSubObjs(&newState->m_hideShowVec);
	}
	hideAllHeadlights(m_hideHeadlights);

	const ModelConditionInfo* prevState = m_curState;	// save, to pass to adjustAnimation (could be null)
	m_curState = newState;
	m_nextState = nextState;
	m_nextStateAnimLoopDuration = NO_NEXT_DURATION;
	adjustAnimation(prevState, prevAnimFraction);
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::replaceModelConditionState(const ModelConditionFlags& c)
{
	m_hideHeadlights = c.test(MODELCONDITION_NIGHT) ? false : true;

	const ModelConditionInfo* info = findBestInfo(c);
	if (info)
		setModelState(info);

	hideAllHeadlights(m_hideHeadlights);
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::setSelectable(Bool selectable)
{
	// set collision type for render object.  Used by WW3D2 collision code.
	if( m_renderObject )
	{
		int current = m_renderObject->Get_Collision_Type();
		if (selectable) {
			current |= PICK_TYPE_SELECTABLE;
		} else {
			current &= ~PICK_TYPE_SELECTABLE;
		}
		m_renderObject->Set_Collision_Type(current);
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::replaceIndicatorColor(Color color)
{
	if (!getW3DModelDrawModuleData()->m_okToChangeModelColor)
		return;

	if (getRenderObject())
	{
		Int newColor = (color == 0) ? 0 : (color | 0xFF000000);
		if (newColor != m_hexColor)
		{
			m_hexColor = newColor;

			// set these to NULL to force a regen of everything in setModelState.
			const ModelConditionInfo* tmp = m_curState;
			m_curState = NULL;
			m_nextState = NULL;
			m_nextStateAnimLoopDuration = NO_NEXT_DURATION;
			setModelState(tmp);
		}
	}
}

//-------------------------------------------------------------------------------------------------
// this method must ONLY be called from the client, NEVER From the logic, not even indirectly.
Bool W3DModelDraw::clientOnly_getRenderObjInfo(Coord3D* pos, Real* boundingSphereRadius, Matrix3D* transform) const
{
	if (!m_renderObject)
		return false;

	Vector3 objPos = m_renderObject->Get_Position();	//get position of object
	pos->x = objPos.X;
	pos->y = objPos.Y;
	pos->z = objPos.Z;

	*transform = m_renderObject->Get_Transform();
	*boundingSphereRadius = m_renderObject->Get_Bounding_Sphere().Radius;

	return true;
}

//-------------------------------------------------------------------------------------------------
Bool W3DModelDraw::getProjectileLaunchOffset(
	const ModelConditionFlags& condition,
	WeaponSlotType wslot,
	Int specificBarrelToUse,
	Matrix3D* launchPos,
	WhichTurretType tur,
	Coord3D* turretRotPos,
	Coord3D* turretPitchPos
) const
{
	/*
		Note, this recalcs the state every time, rather than using m_curState,
		because m_curState could be a transition state, and we really need
		to get the pristine bone(s) for the state that logic believes to be current,
		not the one the client might currently be using...
	*/
	const ModelConditionInfo* stateToUse = findBestInfo(condition);
	if (!stateToUse)
	{
		CRCDEBUG_LOG(("can't find best info"));
		//BONEPOS_LOG(("can't find best info"));
		return false;
	}
#if defined(RTS_DEBUG)
	CRCDEBUG_LOG(("W3DModelDraw::getProjectileLaunchOffset() for %s",
		stateToUse->getDescription().str()));
#endif

#ifdef INTENSE_DEBUG
	AsciiString flags;
	for (Int i=0; i<condition.size(); ++i)
	{
		if (condition.test(i))
		{
			if (flags.isNotEmpty())
				flags.concat(',');
			flags.concat(condition.getNameFromSingleBit(i));
		}
	}
#endif // INTENSE_DEBUG

	const W3DModelDrawModuleData* d = getW3DModelDrawModuleData();
	//CRCDEBUG_LOG(("validateStuffs() from within W3DModelDraw::getProjectileLaunchOffset()"));
	//DUMPREAL(getDrawable()->getScale());
	//BONEPOS_LOG(("validateStuffs() from within W3DModelDraw::getProjectileLaunchOffset()"));
	//BONEPOS_DUMPREAL(getDrawable()->getScale());
	stateToUse->validateStuff(NULL, getDrawable()->getScale(), d->m_extraPublicBones);

	DEBUG_ASSERTCRASH(stateToUse->m_transitionSig == NO_TRANSITION,
		("It is never legal to getProjectileLaunchOffset from a Transition state (they vary on a per-client basis)... however, we can fix this (see srj)\n"));

	DEBUG_ASSERTCRASH(specificBarrelToUse >= 0, ("specificBarrelToUse should now always be explicit"));

#ifdef CACHE_ATTACH_BONE
#else
	Coord3D techOffset;
	techOffset.zero();
	// must use pristine bone here since the result is used by logic
	Matrix3D pivot;
	if (d->m_attachToDrawableBone.isNotEmpty() &&
			getDrawable()->getPristineBonePositions( d->m_attachToDrawableBone.str(), 0, NULL, &pivot, 1 ) == 1)
	{
		pivot.Pre_Rotate_Z(getDrawable()->getOrientation());
		techOffset.x = pivot.Get_X_Translation();
		techOffset.y = pivot.Get_Y_Translation();
		techOffset.z = pivot.Get_Z_Translation();
	}
#endif

	CRCDEBUG_LOG(("wslot = %d", wslot));
	const ModelConditionInfo::WeaponBarrelInfoVec& wbvec = stateToUse->m_weaponBarrelInfoVec[wslot];
	if( wbvec.empty() )
	{
		// Can't find the launch pos, but they might still want the other info they asked for
		CRCDEBUG_LOG(("empty wbvec"));
		//BONEPOS_LOG(("empty wbvec"));
		launchPos = NULL;
	}
	else
	{
		if (specificBarrelToUse < 0 || specificBarrelToUse >= wbvec.size())
			specificBarrelToUse = 0;

		if (launchPos)
		{
			CRCDEBUG_LOG(("specificBarrelToUse = %d", specificBarrelToUse));
			*launchPos = wbvec[specificBarrelToUse].m_projectileOffsetMtx;

			if (tur != TURRET_INVALID)
			{
				launchPos->Pre_Rotate_Z(stateToUse->m_turrets[tur].m_turretArtAngle);
				launchPos->Pre_Rotate_Y(-stateToUse->m_turrets[tur].m_turretArtPitch);
			}
#ifdef CACHE_ATTACH_BONE
#else
			launchPos->Adjust_Translation( Vector3( techOffset.x, techOffset.y, techOffset.z ) );
#endif

			DUMPMATRIX3D(launchPos);
		}
	}

	if (turretRotPos)
		turretRotPos->zero();
	if (turretPitchPos)
		turretPitchPos->zero();

	if (tur != TURRET_INVALID)
	{
#ifdef CACHE_ATTACH_BONE
		const Vector3* offset = d->getAttachToDrawableBoneOffset(getDrawable());
#endif
		const ModelConditionInfo::TurretInfo& turInfo = stateToUse->m_turrets[tur];
		if (turretRotPos)
		{
			if (turInfo.m_turretAngleNameKey != NAMEKEY_INVALID &&
					!stateToUse->findPristineBonePos(turInfo.m_turretAngleNameKey, *turretRotPos))
			{
				DEBUG_CRASH(("*** ASSET ERROR: TurretBone %s not found!",KEYNAME(turInfo.m_turretAngleNameKey).str()));
			}
#ifdef CACHE_ATTACH_BONE
			if (offset)
			{
				turretRotPos->x += offset->X;
				turretRotPos->y += offset->Y;
				turretRotPos->z += offset->Z;
			}
#endif
		}
		if (turretPitchPos)
		{
			if (turInfo.m_turretPitchNameKey != NAMEKEY_INVALID &&
					!stateToUse->findPristineBonePos(turInfo.m_turretPitchNameKey, *turretPitchPos))
			{
				DEBUG_CRASH(("*** ASSET ERROR: TurretBone %s not found!",KEYNAME(turInfo.m_turretPitchNameKey).str()));
			}
#ifdef CACHE_ATTACH_BONE
			if (offset)
			{
				turretPitchPos->x += offset->X;
				turretPitchPos->y += offset->Y;
				turretPitchPos->z += offset->Z;
			}
#endif
		}
	}
	return launchPos != NULL;// return if LaunchPos is valid or not
}

//-------------------------------------------------------------------------------------------------
Int W3DModelDraw::getPristineBonePositionsForConditionState(
	const ModelConditionFlags& condition,
	const char* boneNamePrefix,
	Int startIndex,
	Coord3D* positions,
	Matrix3D* transforms,
	Int maxBones
) const
{
	/*
		Note, this recalcs the state every time, rather than using m_curState,
		because m_curState could be a transition state, and we really need
		to get the pristine bone(s) for the state that logic believes to be current,
		not the one the client might currently be using...
	*/
	const ModelConditionInfo* stateToUse = findBestInfo(condition);
	if (!stateToUse)
		return 0;

//	if (isValidTimeToCalcLogicStuff())
//	{
//		CRCDEBUG_LOG(("W3DModelDraw::getPristineBonePositionsForConditionState() - state = '%s'",
//			stateToUse->getDescription().str()));
//		//CRCDEBUG_LOG(("renderObject == NULL: %d", (stateToUse==m_curState)?(m_renderObject == NULL):1));
//	}

	//BONEPOS_LOG(("validateStuff() from within W3DModelDraw::getPristineBonePositionsForConditionState()"));
	//BONEPOS_DUMPREAL(getDrawable()->getScale());
	// we must call this every time we set m_nextState, to ensure cached bones are happy
	stateToUse->validateStuff(
		// if the state is the current state, pass in the current render object
		// so that we don't have to re-create it!
		stateToUse == m_curState ? m_renderObject : NULL,
		getDrawable()->getScale(),
		getW3DModelDrawModuleData()->m_extraPublicBones);

	const int MAX_BONE_GET = 64;
	Matrix3D tmpMtx[MAX_BONE_GET];

	if (maxBones > MAX_BONE_GET)
		maxBones = MAX_BONE_GET;

	if (transforms == NULL)
		transforms = tmpMtx;

	Int posCount = 0;
	Int endIndex = (startIndex == 0) ? 0 : 99;
	char buffer[256];
	Int i = startIndex;
	for (; i <= endIndex; ++i)
	{
		if (i == 0)
			strcpy(buffer, boneNamePrefix);
		else
			sprintf(buffer, "%s%02d", boneNamePrefix, i);

		for (char *c = buffer; c && *c; ++c)
		{
			// convert to all-lowercase since that's how we filled in the map
			*c = tolower(*c);
		}

		const Matrix3D* mtx = stateToUse->findPristineBone(NAMEKEY(buffer), NULL);
		if (mtx)
		{
			transforms[posCount] = *mtx;
//			if (isValidTimeToCalcLogicStuff())
//			{
//				DUMPMATRIX3D(mtx);
//			}
		}
		else
		{
			//DEBUG_CRASH(("*** ASSET ERROR: Bone %s not found!",buffer));
			const Object *obj = getDrawable()->getObject();
			if (obj)
				transforms[posCount] = *obj->getTransformMatrix();
			else
				transforms[posCount].Make_Identity();
			break;
		}

		++posCount;
		if (posCount >= maxBones)
			break;
	}

	if (positions && transforms)
	{
		for (i = 0; i < posCount; ++i)
		{
			Vector3 pos = transforms[i].Get_Translation();
			positions[i].x = pos.X;
			positions[i].y = pos.Y;
			positions[i].z = pos.Z;
//			if (isValidTimeToCalcLogicStuff())
//			{
//				DUMPCOORD3D(&(positions[i]));
//			}
		}
	}

//	if (isValidTimeToCalcLogicStuff())
//	{
//		CRCDEBUG_LOG(("end of W3DModelDraw::getPristineBonePositionsForConditionState()"));
//	}

	return posCount;
}

//-------------------------------------------------------------------------------------------------
Bool W3DModelDraw::getCurrentWorldspaceClientBonePositions(const char* boneName, Matrix3D& transform) const
{
	if (!m_renderObject)
		return false;

	Int boneIndex = m_renderObject->Get_Bone_Index(boneName);
	if (boneIndex == 0)
		return false;

	transform = m_renderObject->Get_Bone_Transform(boneIndex);
	return true;
}

//-------------------------------------------------------------------------------------------------
Int W3DModelDraw::getCurrentBonePositions(
	const char* boneNamePrefix,
	Int startIndex,
	Coord3D* positions,
	Matrix3D* transforms,
	Int maxBones
) const
{
	const int MAX_BONE_GET = 64;
	Matrix3D tmpMtx[MAX_BONE_GET];

	if (maxBones > MAX_BONE_GET)
		maxBones = MAX_BONE_GET;

	if (transforms == NULL)
		transforms = tmpMtx;

	if( !m_renderObject )
		return 0;

	// ugh... kill the mtx so we get it in modelspace, not world space
	Matrix3D originalTransform = m_renderObject->Get_Transform();	// save the transform
// which is faster: slamming the xform (and invalidating the objects xforms)
// or inverting it ourself? gotta profile to see. (srj)
#define DO_INV
#ifdef DO_INV
	Matrix3D inverse;
	originalTransform.Get_Orthogonal_Inverse(inverse);
	inverse.Scale(getDrawable()->getScale());
#else
	Matrix3D tmp(true);
	tmp.Scale(getDrawable()->getScale());
	m_renderObject->Set_Transform(tmp);					// set to identity transform
#endif

	Int posCount = 0;
	Int endIndex = (startIndex == 0) ? 0 : 99;
	char buffer[256];
	Int i = startIndex;
	for (; i <= endIndex; ++i)
	{
		if (i == 0)
			strcpy(buffer, boneNamePrefix);
		else
			sprintf(buffer, "%s%02d", boneNamePrefix, i);

		Int boneIndex = m_renderObject->Get_Bone_Index(buffer);
		if (boneIndex == 0)
			break;

		transforms[posCount] = m_renderObject->Get_Bone_Transform(boneIndex);
#ifdef DO_INV
		transforms[posCount].preMul(inverse);
#endif
		//DEBUG_ASSERTCRASH(!m_renderObject->Is_Bone_Captured(boneIndex), ("bone is captured!"));

		++posCount;
		if (posCount >= maxBones)
			break;
	}

	if (positions && transforms)
	{
		for (i = 0; i < posCount; ++i)
		{
			Vector3 pos = transforms[i].Get_Translation();
			positions[i].x = pos.X;
			positions[i].y = pos.Y;
			positions[i].z = pos.Z;
		}
	}

#ifdef DO_INV
#else
	m_renderObject->Set_Transform(originalTransform);					// restore it
#endif

	return posCount;
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::reactToTransformChange( const Matrix3D* oldMtx,
																					 const Coord3D* oldPos,
																					 Real oldAngle )
{

	// set the position of our render object
	if( m_renderObject )
	{
		Matrix3D mtx = *getDrawable()->getTransformMatrix();
		adjustTransformMtx(mtx);
		m_renderObject->Set_Transform(mtx);
	}

	if (m_trackRenderObject)
	{
		Object *obj = getDrawable()->getObject();
		const Coord3D* pos = getDrawable()->getPosition();

		if (m_fullyObscuredByShroud)
		{
				m_trackRenderObject->addCapEdgeToTrack(pos->x, pos->y);
		}
		else
		{
			if (obj && obj->isSignificantlyAboveTerrain())
			{
				m_trackRenderObject->setAirborne();
			}
			m_trackRenderObject->addEdgeToTrack(pos->x, pos->y);
		}
	}
}

//-------------------------------------------------------------------------------------------------
const ModelConditionInfo* W3DModelDraw::findBestInfo(const ModelConditionFlags& c) const
{
	return getW3DModelDrawModuleData()->findBestInfo(c);
}

//-------------------------------------------------------------------------------------------------
Int W3DModelDraw::getBarrelCount(WeaponSlotType wslot) const
{
	return (m_curState && (m_curState->m_validStuff & ModelConditionInfo::BARRELS_VALID)) ?
		m_curState->m_weaponBarrelInfoVec[wslot].size() : 0;
}

//-------------------------------------------------------------------------------------------------
Bool W3DModelDraw::handleWeaponFireFX(WeaponSlotType wslot, Int specificBarrelToUse, const FXList* fxl, Real weaponSpeed, const Coord3D* victimPos, Real damageRadius)
{
	DEBUG_ASSERTCRASH(specificBarrelToUse >= 0, ("specificBarrelToUse should now always be explicit"));

	if (!m_curState || !(m_curState->m_validStuff & ModelConditionInfo::BARRELS_VALID))
		return false;

	const ModelConditionInfo::WeaponBarrelInfoVec& wbvec = m_curState->m_weaponBarrelInfoVec[wslot];
	if (wbvec.empty())
	{
		// don't do this... some other module of our drawable may have handled it.
		// just return false and let the caller sort it out.
		// FXList::doFXPos(fxl, getDrawable()->getPosition(), getDrawable()->getTransformMatrix(), weaponSpeed, victimPos);
		return false;
	}

	Bool handled = false;

	if (specificBarrelToUse < 0 || specificBarrelToUse > wbvec.size())
		specificBarrelToUse = 0;

	const ModelConditionInfo::WeaponBarrelInfo& info = wbvec[specificBarrelToUse];

	if (fxl)
	{
		if (info.m_fxBone && m_renderObject)
		{
			const Object *logicObject = getDrawable()->getObject();// This is slow, so store it
			if( ! m_renderObject->Is_Hidden() || (logicObject == NULL) )
			{
				// I can ask the drawable's bone position if I am not hidden (if I have no object I have no choice)
				Matrix3D mtx = m_renderObject->Get_Bone_Transform(info.m_fxBone);
				Coord3D pos;
				pos.x = mtx.Get_X_Translation();
				pos.y = mtx.Get_Y_Translation();
				pos.z = mtx.Get_Z_Translation();
				FXList::doFXPos(fxl, &pos, &mtx, weaponSpeed, victimPos, damageRadius);
			}
			else
			{
				// Else, I should just use my logic position for the effect placement.
				// Things in transports regularly fire from inside (hidden), so this is not weird.
				const Matrix3D *mtx;
				Coord3D pos;
				mtx = logicObject->getTransformMatrix();
				pos = *(logicObject->getPosition());
				/** @todo Once Firepoint bones are actually implemented, this matrix will become correct.
				// Unless of course they decide to not have the tracers come out of the windows, but rather go towards the target.
				// In that case, tracers will have to be rewritten to be "point towards secondary" instead of
				// "point straight ahead" which assumes we are facing the target.
				*/
				FXList::doFXPos(fxl, &pos, mtx, weaponSpeed, victimPos, damageRadius);
			}

			handled = true;
		}
		else
		{
			DEBUG_LOG(("*** no FXBone found for a non-null FXL"));
		}
	}

	if (info.m_recoilBone || info.m_muzzleFlashBone)
	{
		//DEBUG_LOG(("START muzzleflash %08lx for Draw %08lx state %s at frame %d",info.m_muzzleFlashBone,this,m_curState->m_description.str(),TheGameLogic->getFrame()));
		WeaponRecoilInfo& recoil = m_weaponRecoilInfoVec[wslot][specificBarrelToUse];
		recoil.m_state = WeaponRecoilInfo::RECOIL_START;
		recoil.m_recoilRate = getW3DModelDrawModuleData()->m_initialRecoil;
		if (info.m_muzzleFlashBone != 0)
			info.setMuzzleFlashHidden(m_renderObject, false);
	}

	return handled;
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::setAnimationLoopDuration(UnsignedInt numFrames)
{
// this is never defined -- srj
#ifdef NO_DURATIONS_ON_TRANSITIONS
	if (m_curState != NULL && m_curState->m_transition != NO_TRANSITION &&
			m_nextState != NULL && m_nextState->m_transition == NO_TRANSITION)
	{
		DEBUG_LOG(("deferring pending duration of %d frames",numFrames));
		m_nextStateAnimLoopDuration = numFrames;
		return;
	}

	m_nextStateAnimLoopDuration = NO_NEXT_DURATION;
	DEBUG_ASSERTCRASH(m_curState != NULL && m_curState->m_transition == NO_TRANSITION, ("Hmm, setAnimationLoopDuration on a transition state is probably not right... see srj"));
#else
	m_nextStateAnimLoopDuration = NO_NEXT_DURATION;
#endif
	Real desiredDurationInMsec = ceilf(numFrames * MSEC_PER_LOGICFRAME_REAL);
	setCurAnimDurationInMsec(desiredDurationInMsec);
}

//-------------------------------------------------------------------------------------------------
/**
	similar to the above, but assumes that the current state is a "ONCE",
	and is smart about transition states... if there is a transition state
	"inbetween", it is included in the completion time.
*/
void W3DModelDraw::setAnimationCompletionTime(UnsignedInt numFrames)
{
	if (m_curState != NULL && m_curState->m_transitionSig != NO_TRANSITION && m_curState->m_animations.size() > 0 &&
			m_nextState != NULL && m_nextState->m_transitionSig == NO_TRANSITION && m_nextState->m_animations.size() > 0)
	{
		// we have a transition; split up the time suitably.
		// note that this is just a guess, and assumes that the states
		// have only one anim each (or, if multiple, that they are
		// all pretty close in natural duration)
		const Real t1 = m_curState->m_animations.front().getNaturalDurationInMsec();
		const Real t2 = m_nextState->m_animations.front().getNaturalDurationInMsec();
		UnsignedInt transTime = REAL_TO_INT_FLOOR((numFrames * t1) / (t1 + t2));
		setAnimationLoopDuration(transTime);
		m_nextStateAnimLoopDuration = numFrames - transTime;
	}
	else
	{
		setAnimationLoopDuration(numFrames);
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::setPauseAnimation(Bool pauseAnim)
{
	if (m_pauseAnimation == pauseAnim)
	{
		return;
	}

	m_pauseAnimation = pauseAnim;

	if (m_renderObject && m_renderObject->Class_ID() == RenderObjClass::CLASSID_HLOD)
	{
		float framenum, dummy;
		int mode, numFrames;

		HLodClass* hlod = (HLodClass*)m_renderObject;
		HAnimClass* anim = hlod->Peek_Animation_And_Info(framenum, numFrames, mode, dummy);
		if (anim)
		{
			if (m_pauseAnimation)
			{
				m_animationMode = mode;
				hlod->Set_Animation(anim, framenum, RenderObjClass::ANIM_MODE_MANUAL);
			}
			else
			{
				hlod->Set_Animation(anim, framenum, m_animationMode);
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
#ifdef ALLOW_ANIM_INQUIRIES
// srj sez: not sure if this is a good idea, for net sync reasons...
Real W3DModelDraw::getAnimationScrubScalar( void ) const
{
	return getCurAnimDistanceCovered();
}
#endif

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::rebuildWeaponRecoilInfo(const ModelConditionInfo* state)
{
	Int wslot;

	if (state == NULL)
	{
		for (wslot = 0; wslot < WEAPONSLOT_COUNT; ++wslot)
		{
			m_weaponRecoilInfoVec[wslot].clear();
		}
	}
	else
	{
		for (wslot = 0; wslot < WEAPONSLOT_COUNT; ++wslot)
		{
			Int ncount = state->m_weaponBarrelInfoVec[wslot].size();
			if (m_weaponRecoilInfoVec[wslot].size() != ncount)
			{
				WeaponRecoilInfo tmp;
				m_weaponRecoilInfoVec[wslot].resize(ncount, tmp);
			}

			for (WeaponRecoilInfoVec::iterator it = m_weaponRecoilInfoVec[wslot].begin(); it != m_weaponRecoilInfoVec[wslot].end(); ++it)
			{
				it->clear();
			}
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Preload any assets for the time of day requested */
//-------------------------------------------------------------------------------------------------
void W3DModelDraw::preloadAssets( TimeOfDay timeOfDay )
{
	const W3DModelDrawModuleData *modData = getW3DModelDrawModuleData();

	if( modData )
		modData->preloadAssets( timeOfDay, getDrawable()->getScale() );
}

//-------------------------------------------------------------------------------------------------
Bool W3DModelDraw::isVisible() const
{
	return (m_renderObject && m_renderObject->Is_Really_Visible());
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::updateProjectileClipStatus( UnsignedInt shotsRemaining, UnsignedInt maxShots, WeaponSlotType slot )
{
	if ((getW3DModelDrawModuleData()->m_projectileBoneFeedbackEnabledSlots & (1 << slot)) == 0)
		return; // Only if specified.

	doHideShowProjectileObjects( shotsRemaining, maxShots, slot );// Means effectively, show m of n.
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::updateDrawModuleSupplyStatus( Int , Int currentSupply )
{
	// This level only cares if you have anything or not
	if( currentSupply > 0 )
	{
		getDrawable()->setModelConditionState( MODELCONDITION_CARRYING );
	}
	else
	{
		getDrawable()->clearModelConditionState( MODELCONDITION_CARRYING );
	}
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::doHideShowProjectileObjects( UnsignedInt showCount, UnsignedInt maxCount, WeaponSlotType slot )
{
	// Loop through the projectile bones, and start hiding off the front.  That is, 8,9 means to hide the first only.
	if (maxCount < showCount)
	{
		DEBUG_CRASH(("Someone is trying to show more projectiles than they have.") );
		return;
	}
	Int hideCount = maxCount - showCount;

	std::vector<ModelConditionInfo::HideShowSubObjInfo> showHideVector;
	ModelConditionInfo::HideShowSubObjInfo oneEntry;
	if (m_curState->m_weaponProjectileHideShowName[slot].isEmpty())
	{
		for( UnsignedInt projectileIndex = 0; projectileIndex < maxCount; projectileIndex++ )
		{
			oneEntry.subObjName.format("%s%02d", m_curState->m_weaponProjectileLaunchBoneName[slot].str(), (projectileIndex + 1));
			oneEntry.hide = (projectileIndex < hideCount);
			showHideVector.push_back( oneEntry );
		}
	}
	else
	{
		oneEntry.subObjName = m_curState->m_weaponProjectileHideShowName[slot];
		oneEntry.hide = (0 < hideCount);
		showHideVector.push_back( oneEntry );
	}

	// Rather than duplicate recursive utility stuff, I will build a vector like those used by the main ShowHide setting
	doHideShowSubObjs(&showHideVector);
}

//-------------------------------------------------------------------------------------------------
void W3DModelDraw::updateSubObjects()
{
	if (!m_renderObject)
		return;

	if (!m_subObjectVec.empty())
	{
		for (std::vector<ModelConditionInfo::HideShowSubObjInfo>::const_iterator it = m_subObjectVec.begin(); it != m_subObjectVec.end(); ++it)
		{
			Int objIndex;
			RenderObjClass* subObj;

			if ((subObj = m_renderObject->Get_Sub_Object_By_Name(it->subObjName.str(), &objIndex)) != NULL)
			{
				subObj->Set_Hidden(it->hide);

				const HTreeClass *htree = m_renderObject->Get_HTree();
				if (htree)
				{
					//get the bone of this subobject so we can hide all other child objects that use this bone
					//as a parent.
					Int boneIdx = m_renderObject->Get_Sub_Object_Bone_Index(0, objIndex);
					doHideShowBoneSubObjs( it->hide, m_renderObject->Get_Num_Sub_Objects(), boneIdx, m_renderObject, htree);
				}
				subObj->Release_Ref();
			}
			else
			{
				DEBUG_CRASH(("*** ASSET ERROR: SubObject %s not found (%s)!",it->subObjName.str(),getDrawable()->getTemplate()->getName().str()));
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DModelDraw::crc( Xfer *xfer )
{

	// extend base class
	DrawModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version
	* 2: Added animation frame (CBD)
	*/
// ------------------------------------------------------------------------------------------------
void W3DModelDraw::xfer( Xfer *xfer )
{

	// version
	const XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DrawModule::xfer( xfer );

	// weapon recoil info vectors
	UnsignedByte recoilInfoCount;
	WeaponRecoilInfo weaponRecoilInfo;
	for( Int i = 0; i < WEAPONSLOT_COUNT; ++i )
	{

		// count of data here
		recoilInfoCount = m_weaponRecoilInfoVec[ i ].size();
		xfer->xferUnsignedByte( &recoilInfoCount );
		if( xfer->getXferMode() == XFER_SAVE )
		{

			WeaponRecoilInfoVec::const_iterator it;
			for( it = m_weaponRecoilInfoVec[ i ].begin(); it != m_weaponRecoilInfoVec[ i ].end(); ++it )
			{

				// state
				weaponRecoilInfo.m_state = (*it).m_state;
				xfer->xferUser( &weaponRecoilInfo.m_state, sizeof( WeaponRecoilInfo::RecoilState ) );

				// shift
				weaponRecoilInfo.m_shift = (*it).m_shift;
				xfer->xferReal( &weaponRecoilInfo.m_shift );

				// recoil rate
				weaponRecoilInfo.m_recoilRate = (*it).m_recoilRate;
				xfer->xferReal( &weaponRecoilInfo.m_recoilRate );

			}

		}
		else
		{

			// clear this list before loading
			m_weaponRecoilInfoVec[ i ].clear();

			// read each data item
			for( Int j = 0; j < recoilInfoCount; ++j )
			{

				// read state
				xfer->xferUser( &weaponRecoilInfo.m_state, sizeof( WeaponRecoilInfo::RecoilState ) );

				// read shift
				xfer->xferReal( &weaponRecoilInfo.m_shift );

				// read recoil rate
				xfer->xferReal( &weaponRecoilInfo.m_recoilRate );

				// stuff it in the vector
				m_weaponRecoilInfoVec[ i ].push_back( weaponRecoilInfo );

			}

		}

	}

	// sub object vector
	UnsignedByte subObjectCount = m_subObjectVec.size();
	xfer->xferUnsignedByte( &subObjectCount );
	ModelConditionInfo::HideShowSubObjInfo hideShowSubObjInfo;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		std::vector<ModelConditionInfo::HideShowSubObjInfo>::const_iterator it;
		for( it = m_subObjectVec.begin(); it != m_subObjectVec.end(); ++it )
		{

			// sub object name
			hideShowSubObjInfo.subObjName = (*it).subObjName;
			xfer->xferAsciiString( &hideShowSubObjInfo.subObjName );

			// hide
			hideShowSubObjInfo.hide = (*it).hide;
			xfer->xferBool( &hideShowSubObjInfo.hide );

		}

	}
	else
	{

		// the vector must be emtpy
		m_subObjectVec.clear();

		// read each data item
		for( UnsignedByte i = 0; i < subObjectCount; ++i )
		{

			// name
			xfer->xferAsciiString( &hideShowSubObjInfo.subObjName );

			// hide
			xfer->xferBool( &hideShowSubObjInfo.hide );

			// stuff in vector
			m_subObjectVec.push_back( hideShowSubObjInfo );

		}

	}

	// animation
	if( version >= 2 )
	{

		if( xfer->getXferMode() == XFER_SAVE )
		{
				// srj sez: don't save info for transition states, since we can't really
				// restore them effectively.
			if ( m_renderObject
					&& m_renderObject->Class_ID() == RenderObjClass::CLASSID_HLOD
					&& m_curState
					&& m_curState->m_transitionSig == NO_TRANSITION )
			{

				// cast to HLod
				HLodClass *hlod = (HLodClass*)m_renderObject;

				// get animation info
				Int mode, numFrames;
				Real frame, dummy;
				HAnimClass *anim = hlod->Peek_Animation_And_Info( frame, numFrames, mode, dummy );

				// animation data is present
				Bool present = anim ? TRUE : FALSE;
				xfer->xferBool( &present );

				// if animation data is present
				if( anim )
				{

					// xfer mode
					xfer->xferInt( &mode );

					//
					// xfer frame as a fraction (this will allow the animations to change
					// in future patches but will still be mostly correct)
					//
					Real percent = frame / INT_TO_REAL( anim->Get_Num_Frames()-1 );
					xfer->xferReal( &percent );

				}

			}
			else
			{

				// animation data is *NOT* present
				Bool present = FALSE;
				xfer->xferBool( &present );

			}

		}
		else
		{

			// read animation present flag
			Bool present;
			xfer->xferBool( &present );

			if( present )
			{

				{
					// read mode
					Int mode;
					xfer->xferInt( &mode );		// note, this will be ignored
				}

				// read percent
				Real percent;
				xfer->xferReal( &percent );

				//
				// cast render object to HLod, if this is no longer possible we have read the
				// data already and can just ignore it
				//
				if( m_renderObject && m_renderObject->Class_ID() == RenderObjClass::CLASSID_HLOD )
				{
					HLodClass *hlod = (HLodClass *)m_renderObject;

					// get anim
					HAnimClass *anim = hlod->Peek_Animation();

					// set animation data
					if( anim )
					{

						// figure out frame number given percent written in file and total frames on anim
						Real frame = percent * INT_TO_REAL( anim->Get_Num_Frames()-1 );

						float dummy1, dummy2;
						int curMode, dummy3;
						hlod->Peek_Animation_And_Info(dummy1, dummy3, curMode, dummy2);

						// srj sez: do not change the animation mode. it's too risky, since if you change (say) a nonlooping
						// to a looping, something might break since it could rely on that anim terminating.

						// set animation data
						hlod->Set_Animation( anim, frame, curMode );

					}

				}

			}

		}

	}

	// when loading, update the sub objects if we have any
	if( xfer->getXferMode() == XFER_LOAD && m_subObjectVec.empty() == FALSE )
		updateSubObjects();

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DModelDraw::loadPostProcess( void )
{

	// extend base class
	DrawModule::loadPostProcess();

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
void W3DModelDrawModuleData::crc( Xfer *x )
{
	xfer(x);
}

// ------------------------------------------------------------------------------------------------
void W3DModelDrawModuleData::xfer( Xfer *x )
{

	// version
	const XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	x->xferVersion( &version, currentVersion );

	for (ModelConditionVector::iterator it = m_conditionStates.begin(); it != m_conditionStates.end(); ++it)
	{
		ModelConditionInfo *info = &(*it);
		x->xferByte(&(info->m_validStuff));
#if defined(RTS_DEBUG)
		x->xferAsciiString(&(info->m_description));
#endif
		if (info->m_validStuff)
		{
			for (PristineBoneInfoMap::iterator bit = info->m_pristineBones.begin(); bit != info->m_pristineBones.end(); ++bit)
			{
				PristineBoneInfo *bone = &(bit->second);
				x->xferInt(&(bone->boneIndex));
				x->xferUser(&(bone->mtx), sizeof(Matrix3D));
			}
			Int i=0;
			for (; i<MAX_TURRETS; ++i)
			{
				x->xferInt(&(info->m_turrets[i].m_turretAngleBone));
				x->xferInt(&(info->m_turrets[i].m_turretPitchBone));
			}
			for (i=0; i<WEAPONSLOT_COUNT; ++i)
			{
				for (ModelConditionInfo::WeaponBarrelInfoVec::iterator wit = info->m_weaponBarrelInfoVec[i].begin(); wit != info->m_weaponBarrelInfoVec[i].end(); ++wit)
				{
					x->xferUser(&(wit->m_projectileOffsetMtx), sizeof(Matrix3D));
				}
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
void W3DModelDrawModuleData::loadPostProcess( void )
{
}

