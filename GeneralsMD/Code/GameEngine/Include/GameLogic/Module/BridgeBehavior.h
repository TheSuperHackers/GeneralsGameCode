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

// FILE: BridgeBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, July 2002
// Desc:   Behavior module for bridges
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __BRIDGE_BEHAVIOR_H_
#define __BRIDGE_BEHAVIOR_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/AudioEventRTS.h"
#include "GameClient/TerrainRoads.h"
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/DamageModule.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
enum BridgeTowerType CPP_11(: Int);
class FXList;
class ObjectCreationList;
class Bridge;
class BridgeInfo;

// TYPES //////////////////////////////////////////////////////////////////////////////////////////
// ------------------------------------------------------------------------------------------------
struct TimeAndLocationInfo
{
	UnsignedInt delay;			///< how long to wait to execute this
	AsciiString boneName;		///< which bone to execute at
};
// ------------------------------------------------------------------------------------------------
struct BridgeFXInfo
{
	const FXList *fx;
	TimeAndLocationInfo timeAndLocationInfo;
};

// ------------------------------------------------------------------------------------------------
struct BridgeOCLInfo
{
	const ObjectCreationList *ocl;
	TimeAndLocationInfo timeAndLocationInfo;
};

// ------------------------------------------------------------------------------------------------
typedef std::list< BridgeFXInfo > BridgeFXList;
typedef std::list< BridgeOCLInfo > BridgeOCLList;
typedef std::list< ObjectID > ObjectIDList;
typedef ObjectIDList::iterator ObjectIDListIterator;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class BridgeBehaviorInterface
{

public:

	virtual void setTower( BridgeTowerType towerType, Object *tower ) = 0;
	virtual ObjectID getTowerID( BridgeTowerType towerType ) = 0;
	virtual void createScaffolding( void ) = 0;
	virtual void removeScaffolding( void ) = 0;
	virtual Bool isScaffoldInMotion( void ) = 0;
	virtual Bool isScaffoldPresent( void ) = 0;

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class BridgeBehaviorModuleData : public BehaviorModuleData
{

public:

	BridgeBehaviorModuleData( void );
	~BridgeBehaviorModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

	Real m_lateralScaffoldSpeed;
	Real m_verticalScaffoldSpeed;
	BridgeFXList m_fx;							///< list of FX lists to execute
	BridgeOCLList m_ocl;						///< list of OCL to execute

	static void parseFX( INI *ini, void *instance, void *store, const void* userData );
	static void parseOCL( INI *ini, void *instance, void *store, const void* userData );

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class BridgeBehavior : public UpdateModule,
											 public BridgeBehaviorInterface,
											 public DamageModuleInterface,
											 public DieModuleInterface
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( BridgeBehavior, BridgeBehaviorModuleData );
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( BridgeBehavior, "BridgeBehavior" )

public:

	BridgeBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// module methods
	static Int getInterfaceMask( void ) { return (MODULEINTERFACE_DAMAGE) |
																							 (MODULEINTERFACE_DIE) |
																							 (MODULEINTERFACE_UPDATE); }
	virtual BridgeBehaviorInterface* getBridgeBehaviorInterface( void ) { return this; }
	virtual void onDelete( void );

	// Damage methods
	virtual DamageModuleInterface* getDamage( void ) { return this; }
	virtual void onDamage( DamageInfo *damageInfo );
	virtual void onHealing( DamageInfo *damageInfo );
	virtual void onBodyDamageStateChange( const DamageInfo* damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState );

	// Die methods
	virtual DieModuleInterface* getDie( void ) { return this; }
	virtual void onDie( const DamageInfo *damageInfo );

	// Update methods
	virtual UpdateModuleInterface *getUpdate( void ) { return this; }
	virtual UpdateSleepTime update( void );

	// our own methods
	static BridgeBehaviorInterface *getBridgeBehaviorInterfaceFromObject( Object *obj );
	virtual void setTower( BridgeTowerType towerType, Object *tower );	///< connect tower to us
	virtual ObjectID getTowerID( BridgeTowerType towerType );						///< retrive one of our towers
	virtual void createScaffolding( void );		///< create scaffolding around bridge
	virtual void removeScaffolding( void );		///< remove scaffolding around bridge
	virtual Bool isScaffoldInMotion( void );	///< is scaffold in motion
	virtual Bool isScaffoldPresent( void ) { return m_scaffoldPresent; }

protected:

	void resolveFX( void );
	void handleObjectsOnBridgeOnDie( void );
	void doAreaEffects( TerrainRoadType *bridgeTemplate, Bridge *bridge,
											const ObjectCreationList *ocl, const FXList *fx );
	void setScaffoldData( Object *obj,
												Real *angle,
												Real *sunkenHeight,
												const Coord3D *riseToPos,
												const Coord3D *buildPos,
												const Coord3D *bridgeCenter );

	void getRandomSurfacePosition( TerrainRoadType *bridgeTemplate,
																 const BridgeInfo *bridgeInfo,
																 Coord3D *pos );

	ObjectID m_towerID[ BRIDGE_MAX_TOWERS ];		///< the towers that are a part of us

	// got damaged fx stuff
	const ObjectCreationList *m_damageToOCL[ BODYDAMAGETYPE_COUNT ][ MAX_BRIDGE_BODY_FX ];
	const FXList *m_damageToFX[ BODYDAMAGETYPE_COUNT ][ MAX_BRIDGE_BODY_FX ];
	AudioEventRTS m_damageToSound[ BODYDAMAGETYPE_COUNT ];

	// got repaired fx stuff
	const ObjectCreationList *m_repairToOCL[ BODYDAMAGETYPE_COUNT ][ MAX_BRIDGE_BODY_FX ];
	const FXList *m_repairToFX[ BODYDAMAGETYPE_COUNT ][ MAX_BRIDGE_BODY_FX ];
	AudioEventRTS m_repairToSound[ BODYDAMAGETYPE_COUNT ];

	Bool m_fxResolved;		///< TRUE until we've loaded our fx pointers and sounds

	Bool m_scaffoldPresent;									///< TRUE when we have repair scaffolding visible
	ObjectIDList m_scaffoldObjectIDList;		///< list of scaffold object IDs

	UnsignedInt m_deathFrame;								///< frame we died on

};

#endif  // end __BRIDGE_DAMAGE_H_
