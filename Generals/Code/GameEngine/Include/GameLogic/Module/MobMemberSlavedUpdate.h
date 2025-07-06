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

// MobMemberSlavedUpdate.h /////////////////////////////////////////////////////////////
// Will obey to spawner... or die trying... used by angry mob members
// Author: Mark Lorenzen, August 2002
 
#pragma once

#ifndef _MOBMEMBER_SLAVED_UPDATE_H_
#define _MOBMEMBER_SLAVED_UPDATE_H_

#define MM_SLAVED_UPDATE_RATE (LOGICFRAMES_PER_SECOND / 8) ///< This is a low priority module that only needs to be called every this many frames
#define MIN_SQUIRRELLINESS (0.01f)
#define MAX_SQUIRRELLINESS (1.0f)
#define CATCH_UP_RADIUS_MAX (50)
#define CATCH_UP_RADIUS_MIN (25)
#define DEFAULT_MUST_CATCH_UP_RADIUS (CATCH_UP_RADIUS_MAX)
#define DEFAULT_NO_NEED_TO_CATCH_UP_RADIUS (CATCH_UP_RADIUS_MIN)

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "GameLogic/Module/UpdateModule.h"
class DamageInfo;
enum ModelConditionFlagType CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
class MobMemberSlavedUpdateModuleData : public UpdateModuleData
{
public:
	//Example: Currently used by scout drones owned by rangers AND stinger soldiers owned by stinger sites.
	Int m_mustCatchUpRadius;		//Distance from master I'm allowed when he's idle. If I go too far away, I'll come back.
	Int m_noNeedToCatchUpRadius;	//Allowable wander distance from master while guarding master.
	Real m_squirrellinessRatio; 
	UnsignedInt m_catchUpCrisisBailTime; //after this many consecutive frames outside the catchup radius, I will teleport to the nexus

	MobMemberSlavedUpdateModuleData()
	{
		m_mustCatchUpRadius = DEFAULT_MUST_CATCH_UP_RADIUS;
		m_noNeedToCatchUpRadius = DEFAULT_NO_NEED_TO_CATCH_UP_RADIUS;
		m_squirrellinessRatio = 0;
		m_catchUpCrisisBailTime = 999999;//default to very large number
	}

	static void buildFieldParse(MultiIniFieldParse& p) 
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] = 
		{
			{ "MustCatchUpRadius",			INI::parseInt,	NULL, offsetof( MobMemberSlavedUpdateModuleData, m_mustCatchUpRadius ) },
			{ "CatchUpCrisisBailTime",			INI::parseUnsignedInt,	NULL, offsetof( MobMemberSlavedUpdateModuleData, m_catchUpCrisisBailTime ) },
			{ "NoNeedToCatchUpRadius",		INI::parseInt,	NULL, offsetof( MobMemberSlavedUpdateModuleData, m_noNeedToCatchUpRadius ) },
			{ "Squirrelliness",     INI::parseReal, NULL, offsetof( MobMemberSlavedUpdateModuleData, m_squirrellinessRatio ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};


enum MobStates CPP_11(: Int)
{
	MOB_STATE_NONE,
	MOB_STATE_IDLE,
	MOB_STATE_CATCHUP_NOW,
	MOB_STATE_CATCHING_UP,
	MOB_STATE_ATTACK,
};

//-------------------------------------------------------------------------------------------------
class MobMemberSlavedUpdate : public UpdateModule, public SlavedUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( MobMemberSlavedUpdate, "MobMemberSlavedUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( MobMemberSlavedUpdate, MobMemberSlavedUpdateModuleData )

public:

	MobMemberSlavedUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual SlavedUpdateInterface* getSlavedUpdateInterface() { return this; }// hee hee... behaves just like slavedupdate

	virtual ObjectID getSlaverID() const { return m_slaver; }
	virtual void onEnslave( const Object *slaver );
	virtual void onSlaverDie( const DamageInfo *info );
	virtual void onSlaverDamage( const DamageInfo *info );
	virtual void onObjectCreated();
	virtual Bool isSelfTasking() const { return m_isSelfTasking; };

	void doCatchUpLogic( Coord3D *pinnedPosition );

	void setMobState( MobStates state ) { m_mobState = state; };
	MobStates getMobState( void ) { return m_mobState; };


	virtual UpdateSleepTime update();	///< Deciding whether or not to make new guys

private:
	void startSlavedEffects( const Object *slaver );	///< We have been marked as Slaved, so we can't be selected or move too far or other stuff
	void stopSlavedEffects();		///< We are no longer slaved.

	ObjectID m_slaver;			///< To whom we are enslaved
	Int m_framesToWait;
	MobStates m_mobState;
	RGBColor m_personalColor;
	ObjectID m_primaryVictimID;
	Real m_squirrellinessRatio;
	Bool m_isSelfTasking;
	UnsignedInt m_catchUpCrisisTimer;// how many consecutive frames have I remained outside the catchup radius
	//This is a failsafe to make sure that an individual mobmember does not get isolated from his buddies
	// thus causing the mob to become invincible, since they will continue to bud around the nexus
	
};
#endif //_MOBMEMBER_AI_UPDATE_H_

