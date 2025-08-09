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

// FILE: SpawnBehavior.h //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, January 2002
//				 Colin Day, October 2002
// Desc:	 Behavior will create and monitor a group of spawned units and replace as needed
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _SPAWN_BEHAVIOR_H_
#define _SPAWN_BEHAVIOR_H_

const Int SPAWN_UPDATE_RATE = LOGICFRAMES_PER_SECOND/2; ///< This is a low priority module that only needs to be called every this many frames

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Module/DamageModule.h"

//-------------------------------------------------------------------------------------------------
class ThingTemplate;
enum CanAttackResult CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
class SpawnBehaviorModuleData : public BehaviorModuleData
{
public:
	Int m_spawnNumberData;								///< How many spawn I maintain
	Int m_spawnStartNumberData;				  	///< How many spawn I start with
	Int m_spawnReplaceDelayData;					///< After this many frames, I can replace one
	Int m_initialBurst;						        ///< How many should I make immediately, ignoring the delay?
	Bool m_isOneShotData;									///< Do I just spawn once and go dormant?
	Bool m_canReclaimOrphans;							///< Can I reclaim orphans for purposes of spawning
	Bool m_aggregateHealth;								///< should I calc an offset for the healthbox, averaging all my spawn
	Bool m_exitByBudding;									///< do I create each new spawn atop an existing one?
	Bool m_spawnedRequireSpawner;					///< Spawned objects can only exist while the spawner (us) is alive and present
	Bool m_slavesHaveFreeWill;						///< Slaves with free will don't attack when parent attacks.
	DamageTypeFlags m_damageTypesToPropagateToSlaves;
	std::vector<AsciiString> m_spawnTemplateNameData;
	DieMuxData m_dieMuxData;

	SpawnBehaviorModuleData()
	{
		m_spawnNumberData = 0;
		m_spawnReplaceDelayData = 0;
		//Added By Sadullah Nader
		//Initialization(s) inserted
		m_spawnStartNumberData = 0;
		//
		m_initialBurst = 0;
		m_isOneShotData = FALSE;
		m_canReclaimOrphans = FALSE;
		m_aggregateHealth = FALSE;
		m_exitByBudding = FALSE;
		m_spawnTemplateNameData.clear();
		m_spawnedRequireSpawner = FALSE;
		m_slavesHaveFreeWill = FALSE;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    BehaviorModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "SpawnNumber",							INI::parseInt,										NULL, offsetof( SpawnBehaviorModuleData, m_spawnNumberData ) },
			{ "SpawnReplaceDelay",				INI::parseDurationUnsignedInt,		NULL, offsetof( SpawnBehaviorModuleData, m_spawnReplaceDelayData ) },
			{ "OneShot",									INI::parseBool,										NULL, offsetof( SpawnBehaviorModuleData, m_isOneShotData ) },
			{ "CanReclaimOrphans",				INI::parseBool,										NULL,	offsetof( SpawnBehaviorModuleData, m_canReclaimOrphans ) },
			{ "AggregateHealth",  				INI::parseBool,										NULL, offsetof( SpawnBehaviorModuleData, m_aggregateHealth ) },
			{ "ExitByBudding",    				INI::parseBool,										NULL, offsetof( SpawnBehaviorModuleData, m_exitByBudding ) },
			{ "SpawnTemplateName",				INI::parseAsciiStringVectorAppend,NULL, offsetof( SpawnBehaviorModuleData, m_spawnTemplateNameData ) },
			{ "SpawnedRequireSpawner",		INI::parseBool,										NULL,	offsetof( SpawnBehaviorModuleData, m_spawnedRequireSpawner ) },
			{ "PropagateDamageTypesToSlavesWhenExisting",   INI::parseDamageTypeFlags, NULL, offsetof( SpawnBehaviorModuleData, m_damageTypesToPropagateToSlaves ) },
			{ "InitialBurst",				      INI::parseInt,						        NULL, offsetof( SpawnBehaviorModuleData, m_initialBurst ) },
			{ "SlavesHaveFreeWill",				INI::parseBool,										NULL, offsetof( SpawnBehaviorModuleData, m_slavesHaveFreeWill ) },
			{ 0, 0, 0, 0 }

		};
    p.add(dataFieldParse);
		p.add(DieMuxData::getFieldParse(), offsetof( SpawnBehaviorModuleData, m_dieMuxData ));
	}
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class SpawnBehaviorInterface
{

public:

	virtual Bool maySpawnSelfTaskAI( Real maxSelfTaskersRatio ) = 0;
	virtual void onSpawnDeath( ObjectID deadSpawn, DamageInfo *damageInfo ) = 0;
	virtual Object* getClosestSlave( const Coord3D *pos ) = 0;
	virtual void orderSlavesToAttackTarget( Object *target, Int maxShotsToFire, CommandSourceType cmdSource ) = 0;
	virtual void orderSlavesToAttackPosition( const Coord3D *pos, Int maxShotsToFire, CommandSourceType cmdSource ) = 0;
	virtual CanAttackResult getCanAnySlavesAttackSpecificTarget( AbleToAttackType attackType, const Object *target, CommandSourceType cmdSource ) = 0;
	virtual CanAttackResult getCanAnySlavesUseWeaponAgainstTarget( AbleToAttackType attackType, const Object *victim, const Coord3D *pos, CommandSourceType cmdSource ) = 0;
	virtual Bool canAnySlavesAttack() = 0;
	virtual void orderSlavesToGoIdle( CommandSourceType cmdSource ) = 0;
	virtual void orderSlavesDisabledUntil( DisabledType type, UnsignedInt frame ) = 0;
	virtual void orderSlavesToClearDisabled( DisabledType type ) = 0;
	virtual void giveSlavesStealthUpgrade( Bool grantStealth ) = 0;
	virtual Bool areAllSlavesStealthed() const = 0;
	virtual void revealSlaves() = 0;
	virtual Bool doSlavesHaveFreedom() const = 0;
};

// ------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpawnBehavior : public UpdateModule,
											public SpawnBehaviorInterface,
											public DieModuleInterface,
											public DamageModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpawnBehavior, "SpawnBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpawnBehavior, SpawnBehaviorModuleData )

public:

	SpawnBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// module methods
	static Int getInterfaceMask( void ) { return (MODULEINTERFACE_UPDATE) | (MODULEINTERFACE_DIE) | (MODULEINTERFACE_DAMAGE); }
	virtual void onDelete( void );
	virtual UpdateModuleInterface *getUpdate() { return this; }
	virtual DieModuleInterface *getDie() { return this; }
	virtual DamageModuleInterface *getDamage() { return this; }
	virtual SpawnBehaviorInterface* getSpawnBehaviorInterface() { return this; }

	// update methods
	virtual UpdateSleepTime update();

	// die methods
	virtual void onDie( const DamageInfo *damageInfo );

	// damage methods
	virtual void onDamage( DamageInfo *damageInfo );
	virtual void onHealing( DamageInfo *damageInfo ) { }
	virtual void onBodyDamageStateChange( const DamageInfo* damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState) { }

	// SpawnBehaviorInterface methods
	virtual Bool maySpawnSelfTaskAI( Real maxSelfTaskersRatio );
	virtual void onSpawnDeath( ObjectID deadSpawn, DamageInfo *damageInfo );	///< Something we spawned and set up to tell us it died just died.
	virtual Object* getClosestSlave( const Coord3D *pos );
	virtual void orderSlavesToAttackTarget( Object *target, Int maxShotsToFire, CommandSourceType cmdSource );
	virtual void orderSlavesToAttackPosition( const Coord3D *pos, Int maxShotsToFire, CommandSourceType cmdSource );
	virtual CanAttackResult getCanAnySlavesAttackSpecificTarget( AbleToAttackType attackType, const Object *target, CommandSourceType cmdSource );
	virtual CanAttackResult getCanAnySlavesUseWeaponAgainstTarget( AbleToAttackType attackType, const Object *victim, const Coord3D *pos, CommandSourceType cmdSource );
	virtual Bool canAnySlavesAttack();
	virtual void orderSlavesToGoIdle( CommandSourceType cmdSource );
	virtual void orderSlavesDisabledUntil( DisabledType type, UnsignedInt frame );
	virtual void orderSlavesToClearDisabled( DisabledType type );
	virtual void giveSlavesStealthUpgrade( Bool grantStealth );
	virtual Bool areAllSlavesStealthed() const;
	virtual void revealSlaves();
	virtual Bool doSlavesHaveFreedom() const { return getSpawnBehaviorModuleData()->m_slavesHaveFreeWill; }

	// **********************************************************************************************
	// our own methods
	void stopSpawning();	///< Whoever owns this module may want to turn it off
	void startSpawning();	///< Whoever owns this module may want to turn it on

	void computeAggregateStates(void);
//	void notifySelfTasking( Bool isSelfTasking );

private:

	Bool shouldTryToSpawn(); ///< For my own use, should I even think of spawning
	Bool createSpawn();										///< Actual work of creating a guy

	const ThingTemplate* m_spawnTemplate;	///< What it is I spawn
	Int m_oneShotCountdown;						///< and if so, this is what "once" entails
	Int m_framesToWait;
	Int m_firstBatchCount;   ///<how many to start off with on the first Update();

	/// @todo Make sure the allocator for std::list<> is a good one.  Otherwise override it.
	typedef std::list<Int> intList;
	typedef std::list<Int>::iterator intListIterator;
	typedef std::list<Int>::reverse_iterator intListReverseIterator;
	typedef std::list<ObjectID> objectIDList;
	typedef std::list<ObjectID>::iterator objectIDListIterator;
	typedef std::list<ObjectID>::reverse_iterator objectIDListReverseIterator;

	intList m_replacementTimes;			///< A list of frame times that I need to create new spawns
	objectIDList m_spawnIDs;				///< My darling little spawns.  I need to keep track of them explicitly for the Slave type stuff
	Bool m_active;									///< Am I currently turned on


	Object *reclaimOrphanSpawn( void );		///< find existing orphaned spawn object if present

	Bool m_aggregateHealth;			///< should I calc an offset for the healthbox, averaging all my spawn
	Bool m_initialBurstTimesInited;
	Int m_spawnCount;						///< so I can track for zero = kill; (aggregate)
	UnsignedInt m_selfTaskingSpawnCount;		///< How many of my spawn have I authorized to do their own thing?

	UnsignedInt m_initialBurstCountdown;

	std::vector<AsciiString>::const_iterator m_templateNameIterator;

};

#endif
