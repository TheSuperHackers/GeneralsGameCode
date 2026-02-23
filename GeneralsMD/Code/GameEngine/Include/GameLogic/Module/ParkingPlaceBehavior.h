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

// FILE: ParkingPlaceBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, June 2002
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
class ParkingPlaceBehaviorModuleData : public UpdateModuleData
{
public:
	//UnsignedInt		m_framesForFullHeal;
	Real					m_healAmount;
//	Real					m_extraHealAmount4Helicopters;
	Int						m_numRows;
	Int						m_numCols;
	Real					m_approachHeight;
	Real					m_landingDeckHeightOffset;
	Bool					m_hasRunways;			// if true, each col has a runway in front of it
	Bool					m_parkInHangars;	// if true, park at the hangar production spot, not the "real" parking place

	ParkingPlaceBehaviorModuleData()
	{
		//m_framesForFullHeal = 0;
		m_healAmount = 0;
//    m_extraHealAmount4Helicopters = 0;
		m_numRows = 0;
		m_numCols = 0;
		m_approachHeight = 0.0f;
		m_landingDeckHeightOffset = 0.0f;
		m_hasRunways = false;
		m_parkInHangars = false;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
		UpdateModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "NumRows",						     INI::parseInt,	 nullptr, offsetof( ParkingPlaceBehaviorModuleData, m_numRows ) },
			{ "NumCols",						     INI::parseInt,	 nullptr, offsetof( ParkingPlaceBehaviorModuleData, m_numCols ) },
			{ "ApproachHeight",			     INI::parseReal, nullptr, offsetof( ParkingPlaceBehaviorModuleData, m_approachHeight ) },
			{ "LandingDeckHeightOffset", INI::parseReal, nullptr, offsetof( ParkingPlaceBehaviorModuleData, m_landingDeckHeightOffset ) },
			{ "HasRunways",					     INI::parseBool, nullptr, offsetof( ParkingPlaceBehaviorModuleData, m_hasRunways ) },
			{ "ParkInHangars",			     INI::parseBool, nullptr, offsetof( ParkingPlaceBehaviorModuleData, m_parkInHangars ) },
			{ "HealAmountPerSecond",     INI::parseReal, nullptr, offsetof( ParkingPlaceBehaviorModuleData, m_healAmount ) },
//			{ "ExtraHealAmount4Helicopters",  INI::parseReal, nullptr, offsetof( ParkingPlaceBehaviorModuleData, m_extraHealAmount4Helicopters ) },



			//{ "TimeForFullHeal",	INI::parseDurationUnsignedInt,	nullptr, offsetof( ParkingPlaceBehaviorModuleData, m_framesForFullHeal ) },
			{ 0, 0, 0, 0 }
		};
		p.add(dataFieldParse);
	}

private:

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class ParkingPlaceBehavior : public UpdateModule,
														 public DieModuleInterface,
														 public ParkingPlaceBehaviorInterface,
														 public ExitInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ParkingPlaceBehavior, "ParkingPlaceBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ParkingPlaceBehavior, ParkingPlaceBehaviorModuleData )

public:

	ParkingPlaceBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_DIE); }

	// BehaviorModule
	virtual DieModuleInterface *getDie() override { return this; }
	virtual ParkingPlaceBehaviorInterface* getParkingPlaceBehaviorInterface() override { return this; }
	virtual ExitInterface* getUpdateExitInterface() override { return this; }

	// ExitInterface
	virtual Bool isExitBusy() const override {return FALSE;}	///< Contain style exiters are getting the ability to space out exits, so ask this before reserveDoor as a kind of no-commitment check.
	virtual ExitDoorType reserveDoorForExit( const ThingTemplate* objType, Object *specificObject ) override;
	virtual void exitObjectViaDoor( Object *newObj, ExitDoorType exitDoor ) override;
	virtual void unreserveDoorForExit( ExitDoorType exitDoor ) override;
	virtual void exitObjectByBudding( Object *newObj, Object *budHost ) override { return; }

	virtual Bool getExitPosition( Coord3D& rallyPoint ) const override;
	virtual Bool getNaturalRallyPoint( Coord3D& rallyPoint, Bool offset = TRUE ) const override;
	virtual void setRallyPoint( const Coord3D *pos ) override;			///< define a "rally point" for units to move towards
	virtual const Coord3D *getRallyPoint() const override;			///< define a "rally point" for units to move towards

	// UpdateModule
	virtual UpdateSleepTime update() override;

	// DieModule
	virtual void onDie( const DamageInfo *damageInfo ) override;

	// ParkingPlaceBehaviorInterface
	virtual Bool shouldReserveDoorWhenQueued(const ThingTemplate* thing) const override;
	virtual Bool hasAvailableSpaceFor(const ThingTemplate* thing) const override;
	virtual Bool hasReservedSpace(ObjectID id) const override;
	virtual Int  getSpaceIndex( ObjectID id ) const override;
	virtual Bool reserveSpace(ObjectID id, Real parkingOffset, PPInfo* info) override;
	virtual void releaseSpace(ObjectID id) override;
	virtual Bool reserveRunway(ObjectID id, Bool forLanding) override;
	virtual void releaseRunway(ObjectID id) override;
	virtual void calcPPInfo( ObjectID id, PPInfo *info ) override;
	virtual Int getRunwayIndex(ObjectID id) override;
	virtual Int getRunwayCount() const override { return m_runways.size(); }
	virtual ObjectID getRunwayReservation( Int r, RunwayReservationType type ) override;
	virtual void transferRunwayReservationToNextInLineForTakeoff(ObjectID id) override;
	virtual Real getApproachHeight() const override { return getParkingPlaceBehaviorModuleData()->m_approachHeight; }
	virtual Real getLandingDeckHeightOffset() const override { return getParkingPlaceBehaviorModuleData()->m_landingDeckHeightOffset; }
	virtual void setHealee(Object* healee, Bool add) override;
	virtual void killAllParkedUnits() override;
	virtual void defectAllParkedUnits(Team* newTeam, UnsignedInt detectionTime) override;
	virtual Bool calcBestParkingAssignment( ObjectID id, Coord3D *pos, Int *oldIndex = nullptr, Int *newIndex = nullptr ) override { return FALSE; }
	virtual const std::vector<Coord3D>* getTaxiLocations( ObjectID id ) const { return nullptr; }
	virtual const std::vector<Coord3D>* getCreationLocations( ObjectID id ) const { return nullptr; }

private:

	struct ParkingPlaceInfo
	{
		Coord3D      m_hangarStart;
		Real         m_hangarStartOrient;
		Coord3D      m_location;
		Coord3D      m_prep;
		Real         m_orientation;
		Int          m_runway;
		ExitDoorType m_door;
		ObjectID     m_objectInSpace;
		Bool         m_reservedForExit;
		Bool         m_postponedRunwayReservationForTakeoff;

		ParkingPlaceInfo()
		{
			m_hangarStart.zero();
			m_hangarStartOrient = 0;
			m_location.zero();
			m_prep.zero();
			m_orientation = 0;
			m_runway = 0;
			m_door = DOOR_NONE_AVAILABLE;
			m_objectInSpace = INVALID_ID;
			m_reservedForExit = false;
			m_postponedRunwayReservationForTakeoff = false;
		}
	};

	struct RunwayInfo
	{
		Coord3D		m_start;
		Coord3D		m_end;
		ObjectID	m_inUseBy;
		ObjectID	m_nextInLineForTakeoff;
		Bool			m_wasInLine;
	};

	struct HealingInfo
	{
		ObjectID		m_gettingHealedID;
		UnsignedInt	m_healStartFrame;
	};

	std::vector<ParkingPlaceInfo>	m_spaces;
	std::vector<RunwayInfo>				m_runways;
	std::list<HealingInfo>				m_healing;	// note, this list can vary in size, and be larger than the parking space count
	UnsignedInt										m_nextHealFrame;
	Bool													m_gotInfo;

	Bool postponeRunwayReservation(UnsignedInt spaceIndex, Bool forLanding);
	void buildInfo();
	void purgeDead();
	void resetWakeFrame();

	ParkingPlaceInfo* findPPI(ObjectID id);
	ParkingPlaceInfo* findEmptyPPI();

	Coord3D m_heliRallyPoint;
	Bool m_heliRallyPointExists;				///< Only move to the rally point if this is true
};
