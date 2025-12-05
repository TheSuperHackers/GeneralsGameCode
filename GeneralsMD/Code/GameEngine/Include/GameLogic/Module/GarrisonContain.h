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

// FILE: GarrisonContain.h ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Contain module for structures that can be garrisoned
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/OpenContain.h"
#include "Common/ModelState.h"

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------


class GarrisonContainModuleData : public OpenContainModuleData
{
public:


	struct InitialRoster
	{
		AsciiString templateName;
		Int count;
	};


	Bool m_doIHealObjects;
	Real m_framesForFullHeal;
	Bool m_mobileGarrison;
	Bool m_immuneToClearBuildingAttacks;
  Bool m_isEnclosingContainer;

	InitialRoster		m_initialRoster;

	GarrisonContainModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    OpenContainModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "MobileGarrison", INI::parseBool, nullptr, offsetof( GarrisonContainModuleData, m_mobileGarrison ) },
			{ "HealObjects", INI::parseBool, nullptr, offsetof( GarrisonContainModuleData, m_doIHealObjects ) },
			{ "TimeForFullHeal", INI::parseDurationReal, nullptr, offsetof( GarrisonContainModuleData, m_framesForFullHeal ) },
			{ "InitialRoster", parseInitialRoster, nullptr, 0 },
			{ "ImmuneToClearBuildingAttacks", INI::parseBool, nullptr, offsetof( GarrisonContainModuleData, m_immuneToClearBuildingAttacks ) },
      { "IsEnclosingContainer", INI::parseBool, nullptr, offsetof( GarrisonContainModuleData, m_isEnclosingContainer ) },

			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	};

	static void parseInitialRoster( INI* ini, void *instance, void *store, const void* )
	{
		GarrisonContainModuleData* self = (GarrisonContainModuleData*)instance;
		const char* name = ini->getNextToken();
		const char* countStr = ini->getNextTokenOrNull();
		Int count = countStr ? INI::scanInt(countStr) : 1;

		self->m_initialRoster.templateName.set(name);
		self->m_initialRoster.count = count;
	};


};

//-------------------------------------------------------------------------------------------------
/** A GarrisonContain is used for objects that can be garrisoned, heh, go figure */
//-------------------------------------------------------------------------------------------------
class GarrisonContain : public OpenContain
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( GarrisonContain, "GarrisonContain" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( GarrisonContain, GarrisonContainModuleData )

public:

	GarrisonContain( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update( void );						///< called once per frame

	virtual Bool isValidContainerFor( const Object* obj, Bool checkCapacity) const; // Garrison has an extra check forbidding any containment if ReallyDamaged
	virtual Bool isGarrisonable() const { return true; }	///< can this unit be Garrisoned? (ick)
	virtual Bool isBustable() const { return TRUE; }	///< can this container get busted by a bunkerbuster
	virtual Bool isImmuneToClearBuildingAttacks() const { return getGarrisonContainModuleData()->m_immuneToClearBuildingAttacks; }
	virtual Bool isHealContain() const { return false; } ///< true when container only contains units while healing (not a transport!)
	virtual Bool isTunnelContain() const { return FALSE; }
	virtual Bool isPassengerAllowedToFire(  ObjectID id = INVALID_ID  ) const;	///< Hey, can I shoot out of this container?
  virtual Bool isEnclosingContainerFor( const Object *obj ) const { return getGarrisonContainModuleData()->m_isEnclosingContainer; }
  virtual Bool isSpecialOverlordStyleContainer() const {return FALSE;}

	virtual void removeAllContained( Bool exposeStealthUnits );	///< remove all contents of this open container

	virtual void exitObjectViaDoor( Object *exitObj, ExitDoorType exitDoor );	///< exit one of our content items from us
	virtual void exitObjectByBudding( Object *newObj, Object *budHost ) { return; };
	virtual void onContaining( Object *obj, Bool wasSelected );				///< object now contains 'obj'
	virtual void onRemoving( Object *obj );					///< object no longer contains 'obj'
  virtual void onSelling( void );


	// A Garrison Contain must eject all passengers when it crosses the ReallyDamaged threshold.
	virtual void onBodyDamageStateChange( const DamageInfo* damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState);  ///< Die Interface state change callback

	/**
		return the player that *appears* to control this unit, given an observing player.
		if null, use getObject()->getControllingPlayer() instead.
	*/
	virtual const Player* getApparentControllingPlayer( const Player* observingPlayer ) const;
	virtual void recalcApparentControllingPlayer( void );
	virtual Bool isDisplayedOnControlBar() const {return TRUE;}///< Does this container display its contents on the ControlBar?

  virtual void onDamage( DamageInfo *info );

  virtual void setEvacDisposition( EvacDisposition disp ) { m_evacDisposition = disp; };

protected:

	virtual void redeployOccupants( void );				///< redeploy the occupants of us at all available garrison points
	virtual void onObjectCreated();

	void validateRallyPoint( void );							///< validate (if necessary) and pick (if possible) an exit rally point

	virtual Bool calcBestGarrisonPosition( Coord3D *sourcePos, const Coord3D *targetPos );
	virtual Bool attemptBestFirePointPosition( Object *source, Weapon *weapon, Object *victim );
	virtual Bool attemptBestFirePointPosition( Object *source, Weapon *weapon, const Coord3D *targetPos );

	void updateEffects( void );										///< do any effects needed per frame
	void loadGarrisonPoints( void );							///< load garrison point position data and save for later
	void putObjectAtBestGarrisonPoint( Object *obj, Object *target, const Coord3D *targetPos );	///< place object at position of the best garrison point to use for its target
	void putObjectAtGarrisonPoint( Object *obj, ObjectID targetID, Int conditionIndex, Int index );					///< place object at the specified garrison point index
	enum { SEARCH_FOR_REMOVE = -1 };
	void removeObjectFromGarrisonPoint( Object *obj, Int index = SEARCH_FOR_REMOVE );///< remove object from the garrison point placement
	void addValidObjectsToGarrisonPoints( void );			///< add any objects with targets to a garrison point
	void removeInvalidObjectsFromGarrisonPoints( void );	///< remove objects with invalid targets from valid points
	void trackTargets( void );										///< keep attackers at the closest garrison point to their active target

  void matchObjectsToGarrisonPoints( void );                ///< Every frame, and whenever anyone enters or leaves
  void positionObjectsAtStationGarrisonPoints( void );  ///< enforce that everybody stays at their pre-assigned space
  void loadStationGarrisonPoints( void );
  Bool pickAStationForMe( const Object *pbj );
  void removeObjectFromStationPoint( const Object *obj );

	enum { GARRISON_INDEX_INVALID = -1 };
	Int findConditionIndex( void );										///< find the condition index to use given the current object body state
	Int getObjectGarrisonPointIndex( Object *obj );		///< get the garrison point index object is at (if present)
	Int findClosestFreeGarrisonPointIndex( Int conditionIndex,
																				 const Coord3D *targetPos );  ///< find closest free garrison point to the target location

	void healObjects( void );	///< heal all the objects within me
	void healSingleObject( Object *obj, Real frames );	///< heal just one of the objects within me
	void moveObjectsWithMe( void );				///< translates all the garrisoned object to this->getObject()->getPosition()

private:

	enum { MAX_GARRISON_POINTS = 40 };


	//
	// The max units inside any garrisoned structure is 10.  Since the units will "move around"
	// the inside of the structure to be close to their targets, we need a max of 10 garrison points
	// on each side of the building to accomodate everybody inside
	//
	// ----------------------------------------------------------------------------------------------
	struct GarrisonPointData
	{
		union
		{
			Object *		object;							///< object at this garrison point
			ObjectID		objectID;						///< for loading
		};
		ObjectID			targetID;						///< object ID that is our current target
		UnsignedInt		placeFrame;					///< frame we were placed at this garrison point
		UnsignedInt		lastEffectFrame;		///< last frame we fired our effects on
		union
		{
			Drawable *	effect;							///< effect object for showing gun barrels and muzzle flash fire
			DrawableID	effectID;						///< for loading
		};
	};


  struct StationPointData
  {
    ObjectID  occupantID;
    Coord3D   position;
  };

	// ----------------------------------------------------------------------------------------------
	enum
	{
		GARRISON_POINT_PRISTINE,
		GARRISON_POINT_DAMAGED,
		GARRISON_POINT_REALLY_DAMAGED,

		MAX_GARRISON_POINT_CONDITIONS
	};

	Team *							m_originalTeam;																///< our original team before we were garrisoned
	GarrisonPointData		m_garrisonPointData[ MAX_GARRISON_POINTS ];		///< the garrison point placement data
	Int									m_garrisonPointsInUse;
	Coord3D							m_garrisonPoint[ MAX_GARRISON_POINT_CONDITIONS ][ MAX_GARRISON_POINTS ];		///< the garrison point positions (in world coords) for pristine, damaged, and really damaged
	Coord3D							m_exitRallyPoint;												///< Point to rally at when exiting structure (if possible)

  std::vector<StationPointData> m_stationPointList;

	Bool		m_stationGarrisonPointsInitialized;	///< DO NOT XFER THIS!!!   TRUE once we have loaded the pre-assigned garrison point positions from the art
	Bool		m_garrisonPointsInitialized;							///< TRUE once we have loaded the garrison point positions from the art
	Bool		m_hideGarrisonedStateFromNonallies;								///< if T, don't appear to be garrisoned (all stealthy)
	Bool		m_rallyValid;															///< TRUE when m_exitRallyPoint is valid

  EvacDisposition m_evacDisposition;

};
