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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: GameClient/Eva.h /////////////////////////////////////////////////////////////////////////
// Author: John K. McDonald, Jr.
// DO NOT DISTRIBUTE

#pragma once
#ifndef __EVA_H__
#define __EVA_H__

#include "Common/SubsystemInterface.h"
#include "Common/AudioEventRTS.h"

class Player;
class INI;

//------------------------------------------------------------------------------------ Eva Messages
// Keep in sync with TheEvaMessageNames AND Eva::s_shouldPlayFuncs
enum EvaMessage CPP_11(: Int)
{
  EVA_Invalid = -1,
    
	EVA_FIRST = 0,
	EVA_LowPower = EVA_FIRST,
	EVA_InsufficientFunds,
	EVA_SuperweaponDetected_Own_ParticleCannon,
	EVA_SuperweaponDetected_Own_Nuke,
	EVA_SuperweaponDetected_Own_ScudStorm,
  EVA_SuperweaponDetected_Ally_ParticleCannon,
  EVA_SuperweaponDetected_Ally_Nuke,
  EVA_SuperweaponDetected_Ally_ScudStorm,
  EVA_SuperweaponDetected_Enemy_ParticleCannon,
  EVA_SuperweaponDetected_Enemy_Nuke,
  EVA_SuperweaponDetected_Enemy_ScudStorm,
	EVA_SuperweaponLaunched_Own_ParticleCannon,
	EVA_SuperweaponLaunched_Own_Nuke,
	EVA_SuperweaponLaunched_Own_ScudStorm,
  EVA_SuperweaponLaunched_Ally_ParticleCannon,
  EVA_SuperweaponLaunched_Ally_Nuke,
  EVA_SuperweaponLaunched_Ally_ScudStorm,
  EVA_SuperweaponLaunched_Enemy_ParticleCannon,
  EVA_SuperweaponLaunched_Enemy_Nuke,
  EVA_SuperweaponLaunched_Enemy_ScudStorm,
  EVA_SuperweaponReady_Own_ParticleCannon,
  EVA_SuperweaponReady_Own_Nuke,
  EVA_SuperweaponReady_Own_ScudStorm,
  EVA_SuperweaponReady_Ally_ParticleCannon,
  EVA_SuperweaponReady_Ally_Nuke,
  EVA_SuperweaponReady_Ally_ScudStorm,
  EVA_SuperweaponReady_Enemy_ParticleCannon,
  EVA_SuperweaponReady_Enemy_Nuke,
  EVA_SuperweaponReady_Enemy_ScudStorm,
	EVA_BuldingLost,
	EVA_BaseUnderAttack,
	EVA_AllyUnderAttack,
	EVA_BeaconDetected,
  EVA_EnemyBlackLotusDetected,
  EVA_EnemyJarmenKellDetected,
  EVA_EnemyColonelBurtonDetected,
  EVA_OwnBlackLotusDetected,
  EVA_OwnJarmenKellDetected,
  EVA_OwnColonelBurtonDetected,
	EVA_UnitLost,
	EVA_GeneralLevelUp,
	EVA_VehicleStolen,
	EVA_BuildingStolen,
	EVA_CashStolen,
	EVA_UpgradeComplete,
	EVA_BuildingBeingStolen,
	EVA_BuildingSabotaged,
	EVA_SuperweaponLaunched_Own_GPS_Scrambler,
  EVA_SuperweaponLaunched_Ally_GPS_Scrambler,
  EVA_SuperweaponLaunched_Enemy_GPS_Scrambler,
	EVA_SuperweaponLaunched_Own_Sneak_Attack,
  EVA_SuperweaponLaunched_Ally_Sneak_Attack,
  EVA_SuperweaponLaunched_Enemy_Sneak_Attack,

	EVA_COUNT,
};

extern const char *TheEvaMessageNames[];

//------------------------------------------------------------------------------------ EvaCheckInfo
struct EvaSideSounds
{
	AsciiString m_side;
	std::vector<AsciiString> m_soundNames;

	static const FieldParse s_evaSideSounds[];		///< the parse table for INI definition
	const FieldParse *getFieldParse( void ) const { return s_evaSideSounds; }
};

//------------------------------------------------------------------------------------ EvaCheckInfo
class EvaCheckInfo : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(EvaCheckInfo, "EvaCheckInfo")		

public:
	EvaMessage									m_message;
	UnsignedInt									m_framesBetweenChecks;
	UnsignedInt									m_framesToExpire;
	UnsignedInt									m_priority;	// higher priority is more important, and will be played in preference to lower preference
	std::vector<EvaSideSounds>	m_evaSideSounds;
	
	EvaCheckInfo();

	static const FieldParse s_evaEventInfo[];		///< the parse table for INI definition
	const FieldParse *getFieldParse( void ) const { return s_evaEventInfo; }
};
EMPTY_DTOR(EvaCheckInfo)

//---------------------------------------------------------------------------------------- EvaCheck
struct EvaCheck
{
	enum { TRIGGEREDON_NOT = (UnsignedInt)-1 };
	enum { NEXT_CHECK_NOW = 0 };

	const EvaCheckInfo *m_evaInfo;
	UnsignedInt m_triggeredOnFrame;
	UnsignedInt m_timeForNextCheck;
	Bool m_alreadyPlayed;	

		EvaCheck();
};

//-------------------------------------------------------------------------------- ShouldPlayStruct
typedef Bool (*ShouldPlayFunc)( Player *localPlayer );

//--------------------------------------------------------------------------------------------- Eva
class Eva : public SubsystemInterface
{
	private:
		static const ShouldPlayFunc s_shouldPlayFuncs[];

		typedef std::vector<EvaCheckInfo *> EvaCheckInfoPtrVec;
		typedef EvaCheckInfoPtrVec::iterator EvaCheckInfoPtrVecIt;
		EvaCheckInfoPtrVec m_allCheckInfos;

		typedef std::vector<EvaCheck> EvaCheckVec;
		typedef EvaCheckVec::iterator EvaCheckVecIt;

		// This list contains things that either want to play, 
		// or have played and are waiting till they are allowed to check again to play.
		EvaCheckVec m_checks;

		// This is the one speech we're allowed to be playing. If its playing, it always has priority
		AudioEventRTS m_evaSpeech;

		Player *m_localPlayer;

		// Variables for condition checks go here. 
		Int m_previousBuildingCount;
		Int m_previousUnitCount;
		mutable EvaMessage m_messageBeingTested;	// Used by the generic hooks so they can figure out which flag to test.
		Bool m_shouldPlay[EVA_COUNT];	// These aren't all used, but some of them are.
		
		Bool m_enabled;

	public:
		Eva();
		virtual ~Eva();

	public:		// From SubsystemInterface
		virtual void init();
		virtual void reset();
		virtual void update();

		static EvaMessage nameToMessage(const AsciiString& name);
		static AsciiString messageToName(EvaMessage message);

		EvaCheckInfo *newEvaCheckInfo(AsciiString name);
		const EvaCheckInfo *getEvaCheckInfo(AsciiString name);

		void setShouldPlay(EvaMessage messageToPlay);

		void setEvaEnabled(Bool enabled);

    // Parse EvaMessage enum name in INI
    static void parseEvaMessageFromIni( INI * ini, void *instance, void *store, const void* userData );

	protected: 	// Note: These are all protected. They should *NEVER* be made public. They are for internal use only
		Bool isTimeForCheck(EvaMessage messageToTest, UnsignedInt currentFrame) const;
		Bool messageShouldPlay(EvaMessage messageToTest, UnsignedInt currentFrame) const;

		// As soon as you want the logic of these to be public, make a logical representation and have these call that instead.
		static Bool shouldPlayLowPower( Player *localPlayer );
		static Bool shouldPlayGenericHandler( Player * );

		void playMessage(EvaMessage messageToTest, UnsignedInt currentFrame);
		void processPlayingMessages(UnsignedInt currentFrame);


};

extern Eva *TheEva;

#endif /* __EVA_H__ */
