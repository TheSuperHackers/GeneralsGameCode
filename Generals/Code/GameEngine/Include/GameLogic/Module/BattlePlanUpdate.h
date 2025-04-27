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

// FILE: BattlePlanUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, September 2002
// Desc:   Update module to handle building states and battle plan execution & changes
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __BATTLE_PLAN_UPDATE_H_
#define __BATTLE_PLAN_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/KindOf.h"
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class SpecialPowerModule;
class ParticleSystem;
class FXList;
class AudioEventRTS;
enum  MaxHealthChangeType CPP_11(: Int);
enum  CommandOption CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class BattlePlanUpdateModuleData : public ModuleData
{
public:
	SpecialPowerTemplate *m_specialPowerTemplate;

  UnsignedInt m_bombardmentPlanAnimationFrames;
  UnsignedInt m_holdTheLinePlanAnimationFrames;
  UnsignedInt m_searchAndDestroyPlanAnimationFrames;
	UnsignedInt m_transitionIdleFrames;

	AsciiString	m_bombardmentUnpackName;
	AsciiString	m_bombardmentPackName;
	AsciiString m_bombardmentMessageLabel;
	AsciiString m_bombardmentAnnouncementName;
	AsciiString	m_searchAndDestroyUnpackName;
	AsciiString m_searchAndDestroyIdleName;
	AsciiString	m_searchAndDestroyPackName;
	AsciiString m_searchAndDestroyMessageLabel;
	AsciiString m_searchAndDestroyAnnouncementName;
	AsciiString m_holdTheLineUnpackName;
	AsciiString m_holdTheLinePackName;
	AsciiString m_holdTheLineMessageLabel;
	AsciiString m_holdTheLineAnnouncementName;

	UnsignedInt m_battlePlanParalyzeFrames;
	KindOfMaskType m_validMemberKindOf;
	KindOfMaskType m_invalidMemberKindOf;

	Real m_holdTheLineArmorDamageScalar;
	Real m_searchAndDestroySightRangeScalar;
	Real m_strategyCenterSearchAndDestroySightRangeScalar;
	Bool m_strategyCenterSearchAndDestroyDetectsStealth;
	Real m_strategyCenterHoldTheLineMaxHealthScalar;
	MaxHealthChangeType m_strategyCenterHoldTheLineMaxHealthChangeType;

	AsciiString m_visionObjectName;		///< name of object to create to reveal shroud to all players

	BattlePlanUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);

private: 

};

enum TransitionStatus CPP_11(: Int)
{
	TRANSITIONSTATUS_IDLE,
	TRANSITIONSTATUS_UNPACKING,
	TRANSITIONSTATUS_ACTIVE,
	TRANSITIONSTATUS_PACKING,
};

enum BattlePlanStatus CPP_11(: Int)
{
	PLANSTATUS_NONE,
	PLANSTATUS_BOMBARDMENT,
	PLANSTATUS_HOLDTHELINE,
	PLANSTATUS_SEARCHANDDESTROY,
};

class BattlePlanBonuses : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(BattlePlanBonuses, "BattlePlanBonuses")		
public:
	Real						m_armorScalar;
	Int							m_bombardment;				//Represents having weapon bonuses for bombardment plan
	Int							m_searchAndDestroy;		//Represents having weapon bonuses for searchAndDestroy plan
	Int							m_holdTheLine;				//Represents having weapon bonuses for holdTheLine plan
	Real						m_sightRangeScalar;
	KindOfMaskType	m_validKindOf;
	KindOfMaskType	m_invalidKindOf;
};
EMPTY_DTOR(BattlePlanBonuses)

#define ALL_PLANS	1000000		//Used when stacking or removing plans -- we only remove the bonuses when it's 0 or negative.

//-------------------------------------------------------------------------------------------------
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class BattlePlanUpdate : public UpdateModule, public SpecialPowerUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( BattlePlanUpdate, "BattlePlanUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( BattlePlanUpdate, BattlePlanUpdateModuleData );

public:

	BattlePlanUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// SpecialPowerUpdateInterface
	virtual void initiateIntentToDoSpecialPower(const SpecialPowerTemplate *specialPowerTemplate, const Object *targetObj, const Coord3D *targetPos, UnsignedInt commandOptions, Int locationCount );
	virtual Bool isSpecialAbility() const { return false; }
	virtual Bool isSpecialPower() const { return true; }
	virtual Bool isActive() const {return m_status != TRANSITIONSTATUS_IDLE;}
	virtual SpecialPowerUpdateInterface* getSpecialPowerUpdateInterface() { return this; }
	virtual Bool doesSpecialPowerHaveOverridableDestinationActive() const { return false; }
	virtual void setSpecialPowerOverridableDestination( const Coord3D *loc ) {}
	virtual Bool isPowerCurrentlyInUse( const CommandButton *command = NULL ) const;

	//Returns the currently active battle plan -- unpacked and ready... returns PLANSTATUS_NONE if in transition!
	BattlePlanStatus getActiveBattlePlan() const;

	virtual void onObjectCreated();
	virtual void onDelete();
	virtual UpdateSleepTime update();

	virtual CommandOption getCommandOption() const;
protected:

	void setStatus( TransitionStatus status );
	void enableTurret( Bool enable );
	void recenterTurret();
	Bool isTurretInNaturalPosition();
	void setBattlePlan( BattlePlanStatus plan );
	void createVisionObject();	

	BattlePlanStatus m_currentPlan;	//The current battle plan displayed by the building (includes packing & unpacking)
	BattlePlanStatus m_desiredPlan; //The user desired battle plan
	BattlePlanStatus m_planAffectingArmy; //The current battle plan that is affecting troops!
	TransitionStatus m_status;
	
	UnsignedInt m_nextReadyFrame;
	SpecialPowerModuleInterface *m_specialPowerModule;
	Bool				m_invalidSettings;
	Bool				m_centeringTurret;
	BattlePlanBonuses* m_bonuses;

	AudioEventRTS		m_bombardmentUnpack;
	AudioEventRTS		m_bombardmentPack;
	AudioEventRTS   m_bombardmentAnnouncement;
	AudioEventRTS		m_searchAndDestroyUnpack;
	AudioEventRTS   m_searchAndDestroyIdle;
	AudioEventRTS		m_searchAndDestroyPack;
	AudioEventRTS   m_searchAndDestroyAnnouncement;
	AudioEventRTS   m_holdTheLineUnpack;
	AudioEventRTS   m_holdTheLinePack;
	AudioEventRTS   m_holdTheLineAnnouncement;

	// vision object - hang on to this so we can delete it on destruction
	ObjectID m_visionObjectID;

};


#endif

