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

// FILE: RailroadBehavior.h ////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen, September 2002
// Desc: Railroad Train AI
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Waypoint;

typedef std::vector<AsciiString> TemplateNameList;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class RailroadBehaviorModuleData : public PhysicsBehaviorModuleData
{

public:

	RailroadBehaviorModuleData( void );
	static void buildFieldParse( MultiIniFieldParse &p )
	{
		PhysicsBehaviorModuleData::buildFieldParse( p );

		static const FieldParse dataFieldParse[] =
		{
			{ "PathPrefixName",		INI::parseAsciiString, nullptr,	offsetof( RailroadBehaviorModuleData, m_pathPrefixName ) },
			{ "CrashFXTemplateName",		INI::parseAsciiString, nullptr,	offsetof( RailroadBehaviorModuleData, m_CrashFXTemplateName ) },
			{ "IsLocomotive",		INI::parseBool, nullptr,	offsetof( RailroadBehaviorModuleData, m_isLocomotive ) },
			{ "CarriageTemplateName",  INI::parseAsciiStringVectorAppend, nullptr, offsetof(RailroadBehaviorModuleData, m_carriageTemplateNameData) },
			{ "BigMetalBounceSound",			INI::parseAudioEventRTS,	nullptr,	offsetof( RailroadBehaviorModuleData, m_bigMetalImpactDefaultSound) },
			{ "SmallMetalBounceSound",			INI::parseAudioEventRTS,	nullptr,	offsetof( RailroadBehaviorModuleData, m_smallMetalImpactDefaultSound) },
			{ "MeatyBounceSound",			INI::parseAudioEventRTS,	nullptr,	offsetof( RailroadBehaviorModuleData, m_meatyImpactDefaultSound) },
			{ "RunningGarrisonSpeedMax",			INI::parseReal,	nullptr,	offsetof( RailroadBehaviorModuleData, m_runningGarrisonSpeedMax) },
			{ "KillSpeedMin",			INI::parseReal,	nullptr,	offsetof( RailroadBehaviorModuleData, m_killSpeedMin) },
			{ "SpeedMax",			INI::parseReal,	nullptr,	offsetof( RailroadBehaviorModuleData, m_speedMax) },
			{ "Acceleration",			INI::parseReal,	nullptr,	offsetof( RailroadBehaviorModuleData, m_acceleration) },
			{ "Braking",			INI::parseReal,	nullptr,	offsetof( RailroadBehaviorModuleData, m_braking) },
			{ "WaitAtStationTime",			INI::parseDurationUnsignedInt,	nullptr,	offsetof( RailroadBehaviorModuleData, m_waitAtStationTime) },
			{ "RunningSound",			INI::parseAudioEventRTS,	nullptr,	offsetof( RailroadBehaviorModuleData, m_runningSound) },
			{ "ClicketyClackSound",			INI::parseAudioEventRTS,	nullptr,	offsetof( RailroadBehaviorModuleData, m_clicketyClackSound) },
			{ "WhistleSound",			INI::parseAudioEventRTS,	nullptr,	offsetof( RailroadBehaviorModuleData, m_whistleSound) },
			{ "Friction",			INI::parseReal,	nullptr,	offsetof( RailroadBehaviorModuleData, m_friction) },
			{ 0, 0, 0, 0 }
		};

		p.add( dataFieldParse );

	}

	TemplateNameList m_carriageTemplateNameData;
	AsciiString m_pathPrefixName;		///< prefix to use for waypont start and end points we'll look for
	AsciiString m_CrashFXTemplateName;

	Bool m_isLocomotive;
	Real m_runningGarrisonSpeedMax;
	Real m_killSpeedMin;
	Real m_speedMax;
	Real m_acceleration;
	Real m_braking;
	Real m_friction;
	UnsignedInt m_waitAtStationTime;
	AudioEventRTS			m_runningSound;
	AudioEventRTS			m_clicketyClackSound;
	AudioEventRTS				m_bigMetalImpactDefaultSound;
	AudioEventRTS				m_smallMetalImpactDefaultSound;
	AudioEventRTS				m_meatyImpactDefaultSound;
	AudioEventRTS				m_whistleSound;
};
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------






struct TrackPoint
{
	TrackPoint( void )
	{
		clear();
	};

	void clear( void )
	{
		m_position.set(0,0,0);
		m_distanceFromPrev = 0;
		m_distanceFromFirst = 0;
		m_handle = 0xfacade;
		m_isFirstPoint = FALSE;
		m_isLastPoint = FALSE;
		m_isTunnelOrBridge = FALSE;
		m_isStation = FALSE;
		m_isDisembark = FALSE;
		m_isPingPong = FALSE;
	};


	Int getHandle( void )
	{
		return m_handle;
	};

	Coord3D m_position;
	Real m_distanceFromPrev;
	Real m_distanceFromFirst;
	Int  m_handle;
	Bool m_isFirstPoint;
	Bool m_isLastPoint;
	Bool m_isTunnelOrBridge;
	Bool m_isStation;
	Bool m_isDisembark;
	Bool m_isPingPong;

};
typedef std::list<TrackPoint> TrackPointList;

struct TrainTrack
{

	TrainTrack( void ) // a constructor 4 u
	{
		clear();
		incReference();
	};


	void clear( void )
	{
		m_pointList.clear();
		m_isLooping = FALSE;
		m_isValid = FALSE;
		m_refCount = 0;
		m_length = 0.0f;
	};

	Bool m_isLooping;
	Bool m_isValid;
	Real m_length;

	void incReference();
	Bool releaseReference();

	// To protect the track form ever going out of sync between cars on the same train...
	// I restrict write access to the first referencer, before a second one is added (the locomotive)
	TrackPointList* getWritablePointList( void ) { return m_refCount == 1 ? &m_pointList : nullptr; };
	const TrackPointList* getPointList( void ) { return &m_pointList; };

private:
	TrackPointList m_pointList;
	UnsignedInt m_refCount;

};

//-------------------------------------------------------------------------------------------------
class RailroadBehavior	: public PhysicsBehavior
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( RailroadBehavior, "RailroadBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( RailroadBehavior, RailroadBehaviorModuleData )

public:

	RailroadBehavior( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	static Int getInterfaceMask() { return PhysicsBehavior::getInterfaceMask(); }

	struct PullInfo
	{
		Real m_direction; ///< 1 = forward, -1 = backward
		Real speed;
		Real trackDistance;
		Coord3D towHitchPosition;
		Int m_mostRecentSpecialPointHandle;

		UnsignedInt previousWaypoint;
		UnsignedInt currentWaypoint;
		void xferPullInfo( Xfer *xfer );

	};

	typedef std::vector<AsciiString> TemplateNameVector;
	typedef TemplateNameVector::const_iterator TemplateNameIterator;



	//UpdateModule methods
//	virtual SleepyUpdatePhase getUpdatePhase() const { return PHASE_FINAL; }

	// PhysicsBehavior methods
	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal );
	virtual Bool isRailroad() const ;
	virtual UpdateSleepTime update( void );


	// TRAINY METHODS
	void getPulled( PullInfo *info );
	void destroyTheWholeTrainNow( void );
	void hitchNewCarriagebyTemplate( ObjectID parentID, const TemplateNameVector& list, TemplateNameIterator& iter, TrainTrack *trackPointList );
	void hitchNewCarriagebyProximity( ObjectID parentID, TrainTrack *trackPointList );
	void disembark( void );
	Bool hasEverBeenHitched( void ) { return m_hasEverBeenHitched; };

  void setHeld( Bool held ) {m_held = held;};

  void makeAWallOutOfThisTrain( Bool on );


protected:

	//definitions
	enum ConductorState
	{
		APPLY_BRAKES,
		WAIT_AT_STATION,
		ACCELERATE,
		COAST
	};

	enum StationTask
	{
		DO_NOTHING,
		DISEMBARK
	};


	// our methods
	void updatePositionTrackDistance( PullInfo *pullerInfo, PullInfo *myInfo );
	void loadTrackData( void );
	void createCarriages( void );
	void FindPosByPathDistance( Coord3D *pos, const Real dist, const Real length, Bool setState = FALSE );
	void playImpactSound(Object *victim, const Coord3D *impactPosition);

	StationTask m_nextStationTask;
	ObjectID m_trailerID; ///< the ID of the object I am directly pulling
	PullInfo conductorPullInfo;
	PullInfo m_pullInfo;

	AudioEventRTS			m_whistleSound;
	AudioEventRTS			m_clicketyClackSound;
	AudioEventRTS			m_runningSound;
	AudioHandle				m_runningSoundHandle;

	TrainTrack *m_track;

	Int m_currentPointHandle;
	Int m_waitAtStationTimer;

	//Flags
	Bool m_carriagesCreated; ///< TRUE once we have made all the cars in the train
	Bool m_hasEverBeenHitched; /// has somebody ever hitched me? Remains true, even after puller dies.
	Bool m_trackDataLoaded; ///< have I TRIED to load track data, yet? I only try once!
	Bool m_waitingInWings; /// I have not entered the real track yet, so leave me alone
	Bool m_endOfLine;				/// I have reached the end of a non looping track
	Bool m_isLocomotive; ///< Am I a locomotive,
	Bool m_isLeadCarriage; ///< Am the carraige in front,
	Int m_wantsToBeLeadCarriage; ///< Am the carraige in front,
	Bool m_disembark; ///< If I wait at a station, I should also evacuate everybody when I get there
	Bool m_inTunnel; ///< Am I in a tunnel, so I wil not snap to ground height, until the next waypoint,
												//  i.e. do I provide the movement and scheduling AI for m_trailerID
												//  And therefore for his and his and his..........

  Bool m_held;   ///< This will prevent a loco from departing a station

	ConductorState m_conductorState;

	WaypointID m_anchorWaypointID;


};
