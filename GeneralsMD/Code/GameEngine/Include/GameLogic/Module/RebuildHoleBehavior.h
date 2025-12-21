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

// FILE: RebuildHoleBehavior.h ////////////////////////////////////////////////////////////////////
// Author: Colin Day, June 2002
// Desc:   GLA Hole behavior that reconstructs a building after death
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class RebuildHoleBehaviorModuleData : public UpdateModuleData
{

public:

	RebuildHoleBehaviorModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

	Real m_workerRespawnDelay;							///< delay in frames from death of object till respawn of worker
	Real m_holeHealthRegenPercentPerSecond; ///< the hole recovers this % of the max hit points per second
	AsciiString m_workerTemplateName;				///< name of worker object

private:

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class RebuildHoleBehaviorInterface
{

public:

	virtual void startRebuildProcess( const ThingTemplate *rebuild, ObjectID spawnerID ) = 0;
	virtual ObjectID getSpawnerID( void ) = 0;
	virtual ObjectID getReconstructedBuildingID( void ) = 0;
	virtual const ThingTemplate* getRebuildTemplate() const = 0;

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class RebuildHoleBehavior : public UpdateModule,
														public DieModuleInterface,
														public RebuildHoleBehaviorInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( RebuildHoleBehavior, "RebuildHoleBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( RebuildHoleBehavior, RebuildHoleBehaviorModuleData )

public:

	RebuildHoleBehavior( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual RebuildHoleBehaviorInterface* getRebuildHoleBehaviorInterface() { return this; }

	static Int getInterfaceMask( void ) { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_DIE); }

	// BehaviorModule
	virtual DieModuleInterface* getDie( void ) { return this; }

	// UpdateModuleInterface
	virtual UpdateSleepTime update( void );

	// DieModuleInterface
	virtual void onDie( const DamageInfo *damageInfo );

	// RebuildHole specific methods
	virtual void startRebuildProcess( const ThingTemplate *rebuild, ObjectID spawnerID );
	virtual ObjectID getSpawnerID( void ) { return m_spawnerObjectID; }
	virtual ObjectID getReconstructedBuildingID( void ) { return m_reconstructingID; }
	virtual const ThingTemplate* getRebuildTemplate() const { return m_rebuildTemplate; }
	void transferBombs( Object *reconstruction );

	// interface acquisition
	static RebuildHoleBehaviorInterface* getRebuildHoleBehaviorInterfaceFromObject( Object *obj );

protected:

	void newWorkerRespawnProcess( Object *existingWorker );		///< start the worker respawn process (again if existingWorker is non nullptr)

	ObjectID m_workerID;										///< id of the worker that will rebuild us
	ObjectID m_reconstructingID;						///< ID of the object we're reconstructing
	ObjectID m_spawnerObjectID;							///< Object that was killed and therefore the object that created this hole
	UnsignedInt m_workerWaitCounter;				///< when this reaches zero we spawn a worker after death
	const ThingTemplate *m_workerTemplate;  ///< template of the worker to make
	const ThingTemplate *m_rebuildTemplate;	///< what we are rebuilding

};
