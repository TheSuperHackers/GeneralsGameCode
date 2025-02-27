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

// FILE: FlightDeckBehavior.h /////////////////////////////////////////////////////////////////////
// Author: Kris Morness, May 2003
// Desc: Handles aircraft movement and parking behavior for aircraft carriers.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Module/AIUpdate.h"


#define MAX_RUNWAYS			2 //***NOTE: If you change this, make sure you update the parsing section!
													//And also do a search for MAX_RUNWAYS and evaluate any special case comments!

enum
{
	RUNWAY_START_BONE,
	RUNWAY_END_BONE,
	NUM_RUNWAY_BONES,
};

struct RunwayDefinition
{
	RunwayDefinition()
	{
		m_catapultParticleSystem = NULL;
	}

	std::vector<AsciiString> m_spacesBoneNames;
	std::vector<AsciiString> m_taxiBoneNames;
	std::vector<AsciiString> m_creationBoneNames;
	AsciiString m_takeoffBoneNames[ NUM_RUNWAY_BONES ];
	AsciiString m_landingBoneNames[ NUM_RUNWAY_BONES ];
	const ParticleSystemTemplate *m_catapultParticleSystem;
};

//-------------------------------------------------------------------------------------------------
class FlightDeckBehaviorModuleData : public AIUpdateModuleData
{
public:
	RunwayDefinition m_runwayInfo[ MAX_RUNWAYS ];
	AsciiString   m_thingTemplateName;
	Real					m_healAmount;
	Real					m_approachHeight;
	Real					m_landingDeckHeightOffset;
	Int						m_numRows;
	Int						m_numCols;
	UnsignedInt		m_cleanupFrames;
	UnsignedInt		m_humanFollowFrames;
	UnsignedInt		m_replacementFrames;
	UnsignedInt		m_dockAnimationFrames;
	UnsignedInt		m_launchWaveFrames;
	UnsignedInt   m_launchRampFrames;
	UnsignedInt   m_lowerRampFrames;
	UnsignedInt		m_catapultFireFrames;

	FlightDeckBehaviorModuleData();
	static void buildFieldParse( MultiIniFieldParse& p );
	static void parseRunwayStrip( INI* ini, void *instance, void *store, const void* /*userData*/ );
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class FlightDeckBehavior : public AIUpdateInterface,
													 public ParkingPlaceBehaviorInterface,
													 public DieModuleInterface,
													 public ExitInterface

{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FlightDeckBehavior, "FlightDeckBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FlightDeckBehavior, FlightDeckBehaviorModuleData )

public:

	FlightDeckBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_DIE); }

	// BehaviorModule
	virtual ParkingPlaceBehaviorInterface* getParkingPlaceBehaviorInterface() { return this; }
	virtual ExitInterface* getUpdateExitInterface() { return this; }
	virtual DieModuleInterface* getDie() { return this; }

	// ExitInterface
	virtual Bool isExitBusy() const {return FALSE;}	///< Contain style exiters are getting the ability to space out exits, so ask this before reserveDoor as a kind of no-commitment check.
	virtual ExitDoorType reserveDoorForExit( const ThingTemplate* objType, Object *specificObject );
	virtual void exitObjectViaDoor( Object *newObj, ExitDoorType exitDoor );
	virtual void unreserveDoorForExit( ExitDoorType exitDoor );
	virtual void exitObjectByBudding( Object *newObj, Object *budHost ) { return; }
	virtual Bool getExitPosition( Coord3D& rallyPoint ) const { return FALSE; }
	virtual Bool getNaturalRallyPoint( Coord3D& rallyPoint, Bool offset = TRUE ) { return FALSE; }
	virtual void setRallyPoint( const Coord3D *pos ) {}
	virtual const Coord3D *getRallyPoint( void ) const { return NULL;}

	// UpdateModule
	virtual UpdateSleepTime update();

	// DieModule
	virtual void onDie( const DamageInfo *damageInfo );

	// ParkingPlaceBehaviorInterface
	virtual Bool shouldReserveDoorWhenQueued(const ThingTemplate* thing) const;
	virtual Bool hasAvailableSpaceFor(const ThingTemplate* thing) const;
	virtual Bool hasReservedSpace(ObjectID id) const;
	virtual Int  getSpaceIndex( ObjectID id ) const;
	virtual Bool reserveSpace(ObjectID id, Real parkingOffset, PPInfo* info);
	virtual void releaseSpace(ObjectID id);
	virtual Bool reserveRunway(ObjectID id, Bool forLanding);
	virtual void releaseRunway(ObjectID id);
	virtual void calcPPInfo( ObjectID id, PPInfo *info );
	virtual Int getRunwayIndex(ObjectID id);
	virtual Int getRunwayCount() const { return m_runways.size(); }
	virtual ObjectID getRunwayReservation( Int r, RunwayReservationType type );
	virtual void transferRunwayReservationToNextInLineForTakeoff(ObjectID id);
	virtual Real getApproachHeight() const { return getFlightDeckBehaviorModuleData()->m_approachHeight; }
	virtual Real getLandingDeckHeightOffset() const { return getFlightDeckBehaviorModuleData()->m_landingDeckHeightOffset; }
	virtual void setHealee(Object* healee, Bool add);
	virtual void killAllParkedUnits();
	virtual void defectAllParkedUnits(Team* newTeam, UnsignedInt detectionTime);
	virtual Bool calcBestParkingAssignment( ObjectID id, Coord3D *pos, Int *oldIndex = NULL, Int *newIndex = NULL );

	// AIUpdateInterface
	virtual void aiDoCommand(const AICommandParms* parms);

	virtual const std::vector<Coord3D>* getTaxiLocations( ObjectID id ) const;
	virtual const std::vector<Coord3D>* getCreationLocations( ObjectID id ) const;

private:

	Bool isAbleToGiveUpParkingSpace( Object *jet );
	Bool isAbleToMoveForward( const Object &jet ) const;
	Bool isInPositionToTakeoff( const Object &jet ) const;
	void validateAssignments();
	void propagateOrdersToPlanes();
	void propagateOrderToSpecificPlane( Object *jet );
	Bool hasTakeoffOrders();

	struct FlightDeckInfo
	{
		Coord3D				m_prep;
		Real					m_orientation;
		Int						m_runway;
		ObjectID			m_objectInSpace;

		FlightDeckInfo()
		{
			m_prep.zero();
			m_orientation = 0;
			m_runway = 0;
			m_objectInSpace = INVALID_ID;
		}
	};

	struct RunwayInfo
	{
		Coord3D		m_start;
		Matrix3D  m_startTransform;
		Coord3D		m_end;
		Coord3D		m_landingStart;
		Coord3D		m_landingEnd;
		std::vector<Coord3D> m_taxi;
		std::vector<Coord3D> m_creation;
		Real			m_startOrient;
		ObjectID	m_inUseByForTakeoff;
		ObjectID	m_inUseByForLanding;
	};

	struct HealingInfo
	{
		ObjectID		m_gettingHealedID;
		UnsignedInt	m_healStartFrame;
	};

	void buildInfo( Bool createUnits = TRUE);
	void purgeDead();
	void resetWakeFrame();
	FlightDeckInfo* findPPI(ObjectID id);
	FlightDeckInfo* findEmptyPPI();

  const ThingTemplate *m_thingTemplate;


	std::vector<FlightDeckInfo>		m_spaces;
	std::vector<RunwayInfo>				m_runways;
	std::list<HealingInfo>				m_healing;	// note, this list can vary in size, and be larger than the parking space count
	UnsignedInt										m_nextHealFrame;
	UnsignedInt										m_nextCleanupFrame;
	UnsignedInt										m_startedProductionFrame;
	UnsignedInt										m_nextAllowedProductionFrame;
	UnsignedInt										m_nextLaunchWaveFrame[ MAX_RUNWAYS ];
	UnsignedInt										m_rampUpFrame[ MAX_RUNWAYS ];		 //The frame the ramp has completed raising.
	UnsignedInt										m_catapultSystemFrame[ MAX_RUNWAYS ]; //The frame the catapult effect created.
	UnsignedInt										m_lowerRampFrame[ MAX_RUNWAYS ]; //The frame the ramp begins to lower.
	ObjectID											m_designatedTarget;
	AICommandType									m_designatedCommand;
	Coord3D												m_designatedPosition;
	Bool													m_gotInfo;
	Bool													m_rampUp[ MAX_RUNWAYS ];

};
