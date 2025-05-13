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

// FILE: SlavedUpdate.cpp /////////////////////////////////////////////////////////////////////////
// Author: Matt Campbell, March 2002
// Updated: Kris Morness, July 2002 -- Add support for advanced scout drone abilities
// Desc:  Slaved unit(s) remain close to their master. Used by scout drones, and used by stinger
//        soldiers that are close to a stinger site. It's important to note that any slaved units
//				can use any or all features, some of which are specialized.
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef _SLAVED_UPDATE_H_
#define _SLAVED_UPDATE_H_

const Int SLAVED_UPDATE_RATE = LOGICFRAMES_PER_SECOND/4; ///< This is a low priority module that only needs to be called every this many frames

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "GameLogic/Module/UpdateModule.h"
class DamageInfo;
enum ModelConditionFlagType CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
class SlavedUpdateModuleData : public UpdateModuleData
{
public:
	//Example: Currently used by scout drones owned by rangers AND stinger soldiers owned by stinger sites.
	Int m_guardMaxRange;		//Distance from master I'm allowed when he's idle. If I go too far away, I'll come back.
	Int m_guardWanderRange;	//Allowable wander distance from master while guarding master.

	//Example: Below are only currently used by various drones owned by rangers/vehicles
	Int m_attackRange;			//When master attacks a target, I'll go to the target -- how far am I allowed to go in this case.
	Int m_attackWanderRange;//If I'm at the target point, how far can I wander from it.
	Int m_scoutRange;				//If master is moving somewhere, I'll scout ahead -- how far am I allowed to go?
	Int m_scoutWanderRange;	//If I'm at the scout point, how far can I wander from it.
	Int m_distToTargetToGrantRangeBonus;	//How close I have to be to the master's target in order to grant master a range bonus.
	
	//Example: Below are used by battle drones
	Int m_repairRange;
	Real m_repairMinAltitude;
	Real m_repairMaxAltitude;

	Real m_repairRatePerSecond;	//How fast I can repair
	Int m_repairWhenHealthBelowPercentage; //When should I prioritize repairing my master.
	Int m_minReadyFrames;
	Int m_maxReadyFrames;
	Int m_minWeldFrames;
	Int m_maxWeldFrames; 
	AsciiString m_weldingSysName;
	AsciiString m_weldingFXBone;

	Bool m_stayOnSameLayerAsMaster;

	SlavedUpdateModuleData()
	{
		m_guardMaxRange = 0;
		m_guardWanderRange = 0;
		m_attackRange = 0;
		m_attackWanderRange = 0;
		m_scoutRange = 0;
		m_scoutWanderRange = 0;
		m_distToTargetToGrantRangeBonus = 0;
		m_repairRatePerSecond = 0.0f;
		m_repairWhenHealthBelowPercentage = 0;
		m_repairMinAltitude = 0.0f;
		m_repairMaxAltitude = 0.0f;
		m_minWeldFrames = 0;
		m_maxWeldFrames = 0;
		m_minReadyFrames = 0;
		m_maxReadyFrames = 0;
		m_stayOnSameLayerAsMaster = false;
	}

	static void buildFieldParse(MultiIniFieldParse& p) 
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] = 
		{
			{ "GuardMaxRange",			INI::parseInt,	NULL, offsetof( SlavedUpdateModuleData, m_guardMaxRange ) },
			{ "GuardWanderRange",		INI::parseInt,	NULL, offsetof( SlavedUpdateModuleData, m_guardWanderRange ) },
			{ "AttackRange",				INI::parseInt,	NULL, offsetof( SlavedUpdateModuleData, m_attackRange ) },
			{ "AttackWanderRange",	INI::parseInt,	NULL, offsetof( SlavedUpdateModuleData, m_attackWanderRange ) },
			{ "ScoutRange",					INI::parseInt,	NULL, offsetof( SlavedUpdateModuleData, m_scoutRange ) },
			{ "ScoutWanderRange",		INI::parseInt,	NULL, offsetof( SlavedUpdateModuleData, m_scoutWanderRange ) },
			{ "RepairRange",				INI::parseInt,	NULL, offsetof( SlavedUpdateModuleData, m_repairRange ) },
			{ "RepairMinAltitude",		  INI::parseReal, NULL, offsetof( SlavedUpdateModuleData, m_repairMinAltitude ) },
			{ "RepairMaxAltitude",		  INI::parseReal, NULL, offsetof( SlavedUpdateModuleData, m_repairMaxAltitude ) },
			{ "DistToTargetToGrantRangeBonus", INI::parseInt, NULL, offsetof( SlavedUpdateModuleData, m_distToTargetToGrantRangeBonus ) },
			{ "RepairRatePerSecond", INI::parseReal, NULL, offsetof( SlavedUpdateModuleData, m_repairRatePerSecond ) },
			{ "RepairWhenBelowHealth%", INI::parseInt, NULL, offsetof( SlavedUpdateModuleData, m_repairWhenHealthBelowPercentage ) },
			{ "RepairMinReadyTime", INI::parseDurationUnsignedInt, NULL, offsetof( SlavedUpdateModuleData, m_minReadyFrames ) },
			{ "RepairMaxReadyTime", INI::parseDurationUnsignedInt, NULL, offsetof( SlavedUpdateModuleData, m_maxReadyFrames ) },
			{ "RepairMinWeldTime",  INI::parseDurationUnsignedInt, NULL, offsetof( SlavedUpdateModuleData, m_minWeldFrames ) },
			{ "RepairMaxWeldTime",  INI::parseDurationUnsignedInt, NULL, offsetof( SlavedUpdateModuleData, m_maxWeldFrames ) },
			{ "RepairWeldingSys",		INI::parseAsciiString,	NULL, offsetof( SlavedUpdateModuleData, m_weldingSysName ) },
			{ "RepairWeldingFXBone", INI::parseAsciiString, NULL, offsetof( SlavedUpdateModuleData, m_weldingFXBone ) },
			{ "StayOnSameLayerAsMaster", INI::parseBool, NULL, offsetof( SlavedUpdateModuleData, m_stayOnSameLayerAsMaster ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

enum RepairStates CPP_11(: Int)
{
	REPAIRSTATE_NONE,
	REPAIRSTATE_UNPACKING,
	REPAIRSTATE_PACKING,
	REPAIRSTATE_READY,
	REPAIRSTATE_EXTENDING,
	REPAIRSTATE_RETRACTING,
	REPAIRSTATE_WELDING,
};

//-------------------------------------------------------------------------------------------------
class SlavedUpdate : public UpdateModule, public SlavedUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SlavedUpdate, "SlavedUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SlavedUpdate, SlavedUpdateModuleData )

public:

	SlavedUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual SlavedUpdateInterface* getSlavedUpdateInterface() { return this; }

	virtual ObjectID getSlaverID() const { return m_slaver; }
	virtual void onEnslave( const Object *slaver );
	virtual void onSlaverDie( const DamageInfo *info );
	virtual void onSlaverDamage( const DamageInfo *info );
	virtual void onObjectCreated();
	virtual Bool isSelfTasking() const { return FALSE; };


	void doScoutLogic( const Coord3D *mastersDestination );
	void doAttackLogic( const Object *target );
	void doGuardLogic( Coord3D *pinnedPosition );
	void doRepairLogic();
	void endRepair();
	void setRepairState( RepairStates repairState );
	void setRepairModelConditionStates( ModelConditionFlagType flag );
	void moveToNewRepairSpot();

	virtual UpdateSleepTime update();	///< Deciding whether or not to make new guys

private:
	void startSlavedEffects( const Object *slaver );	///< We have been marked as Slaved, so we can't be selected or move too far or other stuff
	void stopSlavedEffects();		///< We are no longer slaved.

	ObjectID m_slaver;			///< To whom we are enslaved
	Coord3D m_guardPointOffset;	///< Where we should go when not busy and still enslaved
	Int m_framesToWait;
	RepairStates m_repairState;
	Bool m_repairing;
};

#endif
