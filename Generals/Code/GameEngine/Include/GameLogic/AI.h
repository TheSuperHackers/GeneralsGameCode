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

// AI.h //
// AI header file
// Author: Michael S. Booth, November 2000

#pragma once

#ifndef _AI_H_
#define _AI_H_

#include "Common/Snapshot.h"
#include "Common/SubsystemInterface.h"
#include "Common/GameMemory.h"
#include "Common/GameType.h"
#include "GameLogic/Damage.h"
#include "Common/STLTypedefs.h"
#include "refcount.h"
#include "ref_ptr.h"

class AIGroup;
class AttackPriorityInfo;
class BuildListInfo;
class CommandButton;
class Object;
class PartitionFilter;
class Path;
class Pathfinder;
class Player;
class PolygonTrigger;
class UpgradeTemplate;
class WeaponTemplate;

enum GUICommandType CPP_11(: Int);
enum HackerAttackMode CPP_11(: Int);
enum WeaponSetType CPP_11(: Int);
enum WeaponLockType CPP_11(: Int);
enum SpecialPowerType CPP_11(: Int);

#if RETAIL_COMPATIBLE_AIGROUP
typedef AIGroup* AIGroupPtr;
#else
typedef RefCountPtr<AIGroup> AIGroupPtr;
#endif

typedef std::vector<ObjectID> VecObjectID;
typedef VecObjectID::iterator VecObjectIDIt;

typedef std::list<Object *> ListObjectPtr;
typedef ListObjectPtr::iterator ListObjectPtrIt;

enum AIDebugOptions CPP_11(: Int)
{
	AI_DEBUG_NONE = 0,
	AI_DEBUG_PATHS,
	AI_DEBUG_TERRAIN,
	AI_DEBUG_CELLS,
	AI_DEBUG_GROUND_PATHS,
	AI_DEBUG_END
};

enum
{
	// multiply by the owner type, either AI or human
	AI_VISIONFACTOR_OWNERTYPE = 0x01,

	// multiply by the "mood" of the unit
	AI_VISIONFACTOR_MOOD = 0x02,

	// If we want to consider the inner radius, we will specify guard inner. Otherwise, use guard outer.
	AI_VISIONFACTOR_GUARDINNER = 0x04,
};
enum {MAX_AI_UPGRADES = 20};

typedef struct {
	Int	m_numSkills;
	ScienceType m_skills[MAX_AI_UPGRADES];
} TSkillSet;

class AISideInfo : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AISideInfo, "AISideInfo")
public:
	AISideInfo( void ) : m_easy(0), m_normal(1), m_hard(2), m_next(NULL)
	{
		m_side.clear();
		m_baseDefenseStructure1.clear();
	}

	AsciiString m_side;						///< Name of the side
	Int					m_easy;						///< Number of gatherers to use in easy, normal & hard
	Int					m_normal;
	Int					m_hard;
	TSkillSet		m_skillSet1;
	TSkillSet		m_skillSet2;
	TSkillSet		m_skillSet3;
	TSkillSet		m_skillSet4;
	TSkillSet		m_skillSet5;
	AsciiString m_baseDefenseStructure1;
	AISideInfo *m_next;
};
EMPTY_DTOR(AISideInfo)

class AISideBuildList : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AISideBuildList, "AISideBuildList")
public:
	AISideBuildList( AsciiString side );
	//~AISideBuildList();

	void addInfo(BuildListInfo *info);

public:
	AsciiString				m_side;						///< Name of the faction.
	BuildListInfo*		m_buildList;				///< Build list for the faction.
	AISideBuildList*	m_next;
};



class TAiData : public Snapshot
{
public:
	TAiData();
	~TAiData();

	void addSideInfo(AISideInfo *info);
	void addFactionBuildList(AISideBuildList *buildList);

	// --------------- inherited from Snapshot interface --------------
	void crc( Xfer *xfer );
	void xfer( Xfer *xfer );
	void loadPostProcess( void );

	Real m_structureSeconds;		// Try to build a structure every N seconds.
	Real m_teamSeconds;					// Try to build a team every N seconds.
	Int m_resourcesWealthy;		// How many resources to be wealthy.
	Int m_resourcesPoor;			// How few resources to be poor.
	UnsignedInt m_forceIdleFramesCount;	// How many frames does a unit need to be Idle before it can begin looking for enemies?
	Real m_structuresWealthyMod; // Factor to multiply m_structurFrames by if we are wealthy.
	Real m_teamWealthyMod;		// Factor to multiply m_teamFrames by if we are wealthy.
	Real m_structuresPoorMod; // Factor to multiply m_structureFrames by if we are poor.
	Real m_teamPoorMod;				// Factor to multiply m_teamFrames if we are poor.
	Real m_teamResourcesToBuild; // Amount of the resources needed to build a team required before we start.
	Real m_guardInnerModifierAI;	// Multiply the AI unit's vision by this much == guard inner circle.
	Real m_guardOuterModifierAI;	// Multiply the AI unit's vision by this much == guard outer circle.
	Real m_guardInnerModifierHuman;	// Multiply the human unit's vision by this much == guard inner circle
	Real m_guardOuterModifierHuman;	// Multiply the human unit's vision by this much == guard outer circle
	UnsignedInt m_guardChaseUnitFrames;		// Number of frames for which a unit should
	UnsignedInt m_guardEnemyScanRate;		// rate to scan for enemies while guarding
	UnsignedInt m_guardEnemyReturnScanRate;		// rate to scan for enemies while guarding but returning

	Real m_wallHeight;				// Height of special wall units can walk on top of.

	Real m_alertRangeModifier;			// When a unit is alert, its range will be modified by this value
	Real m_aggressiveRangeModifier;	// When a unit is aggressive, its range will be modified by this value

	/* The attack priority distance modifier changes relative values.  The relative priority
	   is reduced by the distance away, divided by the modifier.
		 Example: tanks are priority 10, powerplants priority 15, and modifier = 100.0
		 If a powerplant is 700 feet away from the attacker, and the tank is
		 100 feet, the effective priority for tank is 9 (10-(100/100), and the
		 effective priority for powerplant is 8 (15 -(700/100).  So the tanks
		 would be attacked first because their distance weighted priority is greater. */
	Real m_attackPriorityDistanceModifier; // Distance to reduce a relative AttackPriority by 1.


	/*
		How close to a waypoint does a group of units have to be to consider itself at the waypoint?
		m_skirmishGroupFudgeValue is multiplied by the number of the units in the group asking if its
		close enough to determine this.

		So for instance (if m_skirmishGroupFudgeValue was 5), if 2 units are walking along a path,
		they would check to see if they were less than 10 feet away from the waypoint in order
		to say they were close enough. A group of 10 units would consider themselves close enough
		if they were within 50 feet of the waypoint.
	*/
	Real m_skirmishGroupFudgeValue;

	Real m_maxRecruitDistance; // Maximum distance away that units can be recruited.
	Real m_repulsedDistance; // How far a repulsed unit will run past vision range before stopping.
	Bool m_enableRepulsors; // Is repulsion enabled?

	Bool m_forceSkirmishAI; // If true, forces skirmish ai instead of solo ai for development until the skirmish ui is done.  jba.
	Bool m_rotateSkirmishBases; // If true, rotates skirmish ai bases to face the center of the map. jba.

	Bool m_attackUsesLineOfSight; // If true, attack for units with KINDOF_ATTACK_NEEDS_LINE_OF_SIGHT  uses line of sight. jba.

	Bool m_attackIgnoreInsignificantBuildings; // If true, attack for ALL UNITS ignores buildings that are hostile that are not significant. jkmcd

	// Group pathfind info.
	Int	 m_minInfantryForGroup;		// We need at least this many to do it.
	Int	 m_minVehiclesForGroup;		// We need at least this many vehicles to do it.
	Real m_minDistanceForGroup;		// We need to move at least this far to do it.
	Real m_distanceRequiresGroup; // If we are moving this far or farther, force group moving.
	Real m_minClumpDensity;				// What density constitues a clump.  .5 means units occupying 1/2 of their bounding area.

	Int	 m_infantryPathfindDiameter; // Diameter of path in cells for infantry.
	Int  m_vehiclePathfindDiameter;  // Diameter of path in cells for vehicles.

	Int  m_rebuildDelaySeconds;  // Seconds to delay rebuilding after a base building is destroyed or captured.

	Real  m_supplyCenterSafeRadius;  // Radius to scan for enemies to determine safety.

	Real  m_aiDozerBoredRadiusModifier;  // Modifies ai dozers scan range so the move out farther than human ones.
	Bool	m_aiCrushesInfantry; // If true, AI vehicles will attempt to crush infantry.

	AISideInfo *m_sideInfo;

	AISideBuildList *m_sideBuildLists;

	TAiData *m_next;
} ;

//------------------------------------------------------------------------------------------------------------
/**
 * The AI subsystem is responsible for the implementation of
 * the Artificial Intelligence of the game.  This includes
 * the behaviors or all game entities, pathfinding, etc.
 */
class AI : public SubsystemInterface, public Snapshot
{
public:
	AI( void );
	~AI();

	virtual void init( void );						///< initialize AI to default values
	virtual void reset( void );						///< reset the AI system to prepare for a new map
	virtual void update( void );					///< do one frame of AI computation

	inline Pathfinder *pathfinder( void ) { return m_pathfinder; }	///< public access to the pathfind system
	enum
	{
		CAN_SEE														=	1 << 0,
		CAN_ATTACK												= 1 << 1,
		IGNORE_INSIGNIFICANT_BUILDINGS		= 1 << 2,
		ATTACK_BUILDINGS									= 1 << 3,
		WITHIN_ATTACK_RANGE								= 1 << 4,
		UNFOGGED													= 1 << 5
	};
	Object *findClosestEnemy( const Object *me, Real range, UnsignedInt qualifiers,
		const AttackPriorityInfo *info=NULL, PartitionFilter *optionalFilter=NULL);

	Object *findClosestRepulsor( const Object *me, Real range);

	Object *findClosestAlly( const Object *me, Real range, UnsignedInt qualifiers);

	// --------------- inherited from Snapshot interface --------------
	void crc( Xfer *xfer );
	void xfer( Xfer *xfer );
	void loadPostProcess( void );

	// AI Groups -----------------------------------------------------------------------------------------------
	AIGroupPtr createGroup( void ); ///< instantiate a new AI Group
	void destroyGroup( AIGroup *group );	///< destroy the given AI Group
	AIGroup *findGroup( UnsignedInt id );	///< return the AI Group with the given ID

	// Formation info
	enum FormationID getNextFormationID(void);

	static void parseAiDataDefinition( INI* ini );
	const TAiData *getAiData() {return m_aiData;}

	// Note: Does not work for things that do not have AI. (This is in AI.h, after all)
	static Real getAdjustedVisionRangeForObject(const Object *object, Int factorsToConsider);

	static void parseSideInfo( INI* ini, void *instance, void *store, const void *userData );					///< Parse the image part of the INI file
	static void parseSkirmishBuildList( INI* ini, void *instance, void *store, const void *userData );					///< Parse the image part of the INI file
	static void parseStructure( INI* ini, void *instance, void *store, const void *userData );					///< Parse the image part of the INI file
	static void parseSkillSet( INI* ini, void *instance, void *store, const void *userData );					///< Parse the image part of the INI file
	static void parseScience( INI* ini, void *instance, void *store, const void *userData );					///< Parse the image part of the INI file

	inline UnsignedInt getNextGroupID( void ) { return ++m_nextGroupID; }

protected:
	Pathfinder *m_pathfinder;							///< the pathfinding system
	std::list<AIGroup *> m_groupList;			///< the list of AIGroups
	TAiData *m_aiData;
	void newOverride(void);
	void addSideInfo(AISideInfo *info);

	UnsignedInt m_nextGroupID;
	FormationID m_nextFormationID;
};

extern AI *TheAI;												///< the Artificial Intelligence singleton


class Waypoint;
class Team;
class Weapon;

// TheSuperHackers @build xezon 22/03/2025 Renames AI_PASSIVE to not conflict with macro in ws2def.h

// Note - written out in save/load xfer and .map files, don't change these numbers.
enum AttitudeType CPP_11(: Int) {
	ATTITUDE_SLEEP = -2,
	ATTITUDE_PASSIVE=-1,
	ATTITUDE_NORMAL=0,
	ATTITUDE_ALERT=1,
	ATTITUDE_AGGRESSIVE=2,
	ATTITUDE_INVALID=3
};

enum CommandSourceType CPP_11(: Int);

typedef UnsignedInt CommandSourceMask;

#ifdef DEFINE_COMMANDSOURCEMASK_NAMES
static const char *TheCommandSourceMaskNames[] =
{
	"FROM_PLAYER",
	"FROM_SCRIPT",
	"FROM_AI",

	NULL
};
#endif

//------------------------------------------------------------------------------------------------------------

enum AICommandType CPP_11(: Int)	// Stored in save file, do not reorder/renumber.  jba.
{
	AICMD_MOVE_TO_POSITION = 0,
	AICMD_MOVE_TO_OBJECT,
	AICMD_TIGHTEN_TO_POSITION,
	AICMD_MOVE_TO_POSITION_AND_EVACUATE,
	AICMD_MOVE_TO_POSITION_AND_EVACUATE_AND_EXIT,
	AICMD_IDLE,
	AICMD_FOLLOW_WAYPOINT_PATH,
	AICMD_FOLLOW_WAYPOINT_PATH_AS_TEAM,
	AICMD_FOLLOW_USER_PATH,	//Created by player in waypoint mode (this one will create little waypoint markers)
	AICMD_FOLLOW_PATH,
	AICMD_FOLLOW_EXITPRODUCTION_PATH,	// same as AICMD_FOLLOW_PATH, but only used when exiting your production facility.
	AICMD_ATTACK_OBJECT,
	AICMD_FORCE_ATTACK_OBJECT,
	AICMD_ATTACK_TEAM,
	AICMD_ATTACK_POSITION,
	AICMD_ATTACKMOVE_TO_POSITION,
	AICMD_ATTACKFOLLOW_WAYPOINT_PATH,
	AICMD_ATTACKFOLLOW_WAYPOINT_PATH_AS_TEAM,
	AICMD_HUNT,
	AICMD_REPAIR,
#ifdef ALLOW_SURRENDER
	AICMD_PICK_UP_PRISONER,
	AICMD_RETURN_PRISONERS,
#endif
	AICMD_RESUME_CONSTRUCTION,
	AICMD_GET_HEALED,
	AICMD_GET_REPAIRED,
	AICMD_ENTER,
	AICMD_DOCK,
	AICMD_EXIT,
	AICMD_EVACUATE,
	AICMD_EXECUTE_RAILED_TRANSPORT,
	AICMD_GO_PRONE,
	AICMD_GUARD_POSITION,
	AICMD_GUARD_OBJECT,
	AICMD_GUARD_AREA,
	AICMD_DEPLOY_ASSAULT_RETURN,
	AICMD_ATTACK_AREA,
	AICMD_HACK_INTERNET,
	AICMD_FACE_OBJECT,
	AICMD_FACE_POSITION,
	AICMD_RAPPEL_INTO,		// note that this applies to the rappeller.
	AICMD_COMBATDROP,			// note that this applies to the thing-being-rappelled-from.
	AICMD_COMMANDBUTTON_POS,
	AICMD_COMMANDBUTTON_OBJ,
	AICMD_COMMANDBUTTON,
	AICMD_WANDER,
	AICMD_WANDER_IN_PLACE,
	AICMD_PANIC,
	AICMD_BUSY,
	AICMD_FOLLOW_WAYPOINT_PATH_EXACT,
	AICMD_FOLLOW_WAYPOINT_PATH_AS_TEAM_EXACT,
	AICMD_MOVE_AWAY_FROM_UNIT,
	AICMD_FOLLOW_PATH_APPEND,
	AICMD_MOVE_TO_POSITION_EVEN_IF_SLEEPING,	// same as AICMD_MOVE_TO_POSITION, but even ATTITUDE_SLEEP units respond.
	AICMD_GUARD_TUNNEL_NETWORK,
};

struct AICommandParms
{
	AICommandType						m_cmd;
  CommandSourceType				m_cmdSource;
  Coord3D									m_pos;
  Object*									m_obj;
  Object*									m_otherObj;
  const Team*							m_team;
	std::vector<Coord3D>		m_coords;
  const Waypoint*         m_waypoint;
  const PolygonTrigger*   m_polygon;
  Int											m_intValue;       /// misc usage
  DamageInfo							m_damage;
	const CommandButton*		m_commandButton;
	Path*										m_path;

	AICommandParms(AICommandType cmd, CommandSourceType cmdSource);
};

class AICommandParmsStorage
{
private:
	AICommandType						m_cmd;
  CommandSourceType				m_cmdSource;
  Coord3D									m_pos;
  ObjectID								m_obj;
  ObjectID								m_otherObj;
  AsciiString							m_teamName;
	std::vector<Coord3D>		m_coords;
  const Waypoint*         m_waypoint;
  const PolygonTrigger*   m_polygon;
  Int											m_intValue;       /// misc usage
  DamageInfo							m_damage;
	const CommandButton*  	m_commandButton;
	Path*										m_path;

public:
	void store(const AICommandParms& parms);
	void reconstitute(AICommandParms& parms) const;
	void doXfer(Xfer *xfer);
};

/**
	AI interface.  AIGroups, or Objects with an AIUpdate can be given these commands.

	NOTE NOTE NOTE: all of these may be overridden and possibly deferred by various AI classes,
	so they are NOT ALLOWED TO RETURN ANY VALUES, since the particular command issued might
	not be executed immediately...
*/
class AICommandInterface
{
public:

	virtual void aiDoCommand(const AICommandParms* parms) = 0;

	inline void aiMoveToPosition( const Coord3D *pos, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_MOVE_TO_POSITION, cmdSource);
		parms.m_pos = *pos;
		aiDoCommand(&parms);
	}

	inline void aiMoveToPositionEvenIfSleeping( const Coord3D *pos, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_MOVE_TO_POSITION_EVEN_IF_SLEEPING, cmdSource);
		parms.m_pos = *pos;
		aiDoCommand(&parms);
	}

	inline void aiMoveToObject( Object *obj, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_MOVE_TO_OBJECT, cmdSource);
		parms.m_obj = obj;
		aiDoCommand(&parms);
	}

	inline void aiTightenToPosition( const Coord3D *pos, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_TIGHTEN_TO_POSITION, cmdSource);
		parms.m_pos = *pos;
		aiDoCommand(&parms);
	}

	inline void aiMoveToAndEvacuate( const Coord3D *pos, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_MOVE_TO_POSITION_AND_EVACUATE, cmdSource);
		parms.m_pos = *pos;
		aiDoCommand(&parms);
	}

	inline void aiMoveToAndEvacuateAndExit( const Coord3D *pos, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_MOVE_TO_POSITION_AND_EVACUATE_AND_EXIT, cmdSource);
		parms.m_pos = *pos;
		aiDoCommand(&parms);
	}

	inline void aiIdle(CommandSourceType cmdSource)
	{
		AICommandParms parms(AICMD_IDLE, cmdSource);
		aiDoCommand(&parms);
	}

	inline void aiBusy(CommandSourceType cmdSource)
	{
		AICommandParms parms(AICMD_BUSY, cmdSource);
		aiDoCommand(&parms);
	}

	inline void aiFollowWaypointPath( const Waypoint *way, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_FOLLOW_WAYPOINT_PATH, cmdSource);
		parms.m_waypoint = way;
		aiDoCommand(&parms);
	}

	inline void aiFollowWaypointPathExact( const Waypoint *way, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_FOLLOW_WAYPOINT_PATH_EXACT, cmdSource);
		parms.m_waypoint = way;
		aiDoCommand(&parms);
	}

	inline void aiFollowWaypointPathAsTeam( const Waypoint *way, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_FOLLOW_WAYPOINT_PATH_AS_TEAM, cmdSource);
		parms.m_waypoint = way;
		aiDoCommand(&parms);
	}

	inline void aiFollowWaypointPathExactAsTeam( const Waypoint *way, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_FOLLOW_WAYPOINT_PATH_AS_TEAM_EXACT, cmdSource);
		parms.m_waypoint = way;
		aiDoCommand(&parms);
	}

	inline void aiFollowExitProductionPath( const std::vector<Coord3D>* path, Object *ignoreObject, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_FOLLOW_EXITPRODUCTION_PATH, cmdSource);
		parms.m_coords = *path;
		parms.m_obj = ignoreObject;
		aiDoCommand(&parms);
	}

	inline void aiFollowPath( const std::vector<Coord3D>* path, Object *ignoreObject, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_FOLLOW_PATH, cmdSource);
		parms.m_coords = *path;
		parms.m_obj = ignoreObject;
		aiDoCommand(&parms);
	}

	inline void aiFollowPathAppend( const Coord3D* pos, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_FOLLOW_PATH_APPEND, cmdSource);
		parms.m_pos = *pos;
		aiDoCommand(&parms);
	}

	inline void aiAttackObject( Object *victim, Int maxShotsToFire, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_ATTACK_OBJECT, cmdSource);
		parms.m_obj = victim;
		parms.m_intValue = maxShotsToFire;
		aiDoCommand(&parms);
	}

	inline void aiForceAttackObject( Object *victim, Int maxShotsToFire, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_FORCE_ATTACK_OBJECT, cmdSource);
		parms.m_obj = victim;
		parms.m_intValue = maxShotsToFire;
		aiDoCommand(&parms);
	}

	inline void aiAttackTeam( const Team *team, Int maxShotsToFire, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_ATTACK_TEAM, cmdSource);
		parms.m_team = team;
		parms.m_intValue = maxShotsToFire;
		aiDoCommand(&parms);
	}

	inline void aiAttackPosition( const Coord3D *pos, Int maxShotsToFire, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_ATTACK_POSITION, cmdSource);
		parms.m_pos = *pos;
		parms.m_intValue = maxShotsToFire;
		aiDoCommand(&parms);
	}

	inline void aiAttackMoveToPosition( const Coord3D *pos, Int maxShotsToFire, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_ATTACKMOVE_TO_POSITION, cmdSource);
		parms.m_pos = *pos;
		parms.m_intValue = maxShotsToFire;
		aiDoCommand(&parms);
	}

	inline void aiAttackFollowWaypointPath( const Waypoint *way, Int maxShotsToFire, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_ATTACKFOLLOW_WAYPOINT_PATH, cmdSource);
		parms.m_waypoint = way;
		parms.m_intValue = maxShotsToFire;
		aiDoCommand(&parms);
	}

	inline void aiAttackFollowWaypointPathAsTeam( const Waypoint *way, Int maxShotsToFire, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_ATTACKFOLLOW_WAYPOINT_PATH_AS_TEAM, cmdSource);
		parms.m_waypoint = way;
		parms.m_intValue = maxShotsToFire;
		aiDoCommand(&parms);
	}

	inline void aiHunt( CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_HUNT, cmdSource);
		aiDoCommand(&parms);
	}

	inline void aiAttackArea( const PolygonTrigger *areaToGuard, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_ATTACK_AREA, cmdSource);
		parms.m_polygon = areaToGuard;
		aiDoCommand(&parms);
	}

	inline void aiRepair( Object *obj, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_REPAIR, cmdSource);
		parms.m_obj = obj;
		aiDoCommand(&parms);
	}

#ifdef ALLOW_SURRENDER
	inline void aiPickUpPrisoner( Object *obj, CommandSourceType cmdSource )
	{
		AICommandParms parms( AICMD_PICK_UP_PRISONER, cmdSource );
		parms.m_obj = obj;
		aiDoCommand( &parms );
	}
#endif

#ifdef ALLOW_SURRENDER
	inline void aiReturnPrisoners( Object *prison, CommandSourceType cmdSource )
	{
		AICommandParms parms( AICMD_RETURN_PRISONERS, cmdSource );
		parms.m_obj = prison;
		aiDoCommand( &parms );
	}
#endif

	inline void aiResumeConstruction( Object *obj, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_RESUME_CONSTRUCTION, cmdSource);
		parms.m_obj = obj;
		aiDoCommand(&parms);
	}

	inline void aiGetHealed( Object *healDepot, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_GET_HEALED, cmdSource);
		parms.m_obj = healDepot;
		aiDoCommand(&parms);
	}

	inline void aiGetRepaired( Object *repairDepot, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_GET_REPAIRED, cmdSource);
		parms.m_obj = repairDepot;
		aiDoCommand(&parms);
	}

	inline void aiEnter( Object *obj, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_ENTER, cmdSource);
		parms.m_obj = obj;
		aiDoCommand(&parms);
	}

	inline void aiDock( Object *obj, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_DOCK, cmdSource);
		parms.m_obj = obj;
		aiDoCommand(&parms);
	}

	inline void aiExit( Object *objectToExit, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_EXIT, cmdSource);
		parms.m_obj = objectToExit;
		aiDoCommand(&parms);
	}

	inline void aiEvacuate( Bool exposeStealthUnits, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_EVACUATE, cmdSource);
		if( exposeStealthUnits )
			parms.m_intValue = 1;
		else
			parms.m_intValue = 0;
		aiDoCommand(&parms);
	}

	inline void aiExecuteRailedTransport( CommandSourceType cmdSource )
	{
		AICommandParms parms( AICMD_EXECUTE_RAILED_TRANSPORT, cmdSource );
		aiDoCommand( &parms );
	}

	inline void aiGoProne( const DamageInfo *damageInfo, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_GO_PRONE, cmdSource);
		parms.m_damage = *damageInfo;
		aiDoCommand(&parms);
	}

	inline void aiGuardPosition( const Coord3D *pos, GuardMode guardMode, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_GUARD_POSITION, cmdSource);
		parms.m_pos = *pos;
		parms.m_intValue = guardMode;
		aiDoCommand(&parms);
	}

	inline void aiGuardObject( Object *objToGuard, GuardMode guardMode, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_GUARD_OBJECT, cmdSource);
		parms.m_obj = objToGuard;
		parms.m_intValue = guardMode;
		aiDoCommand(&parms);
	}

	inline void aiGuardArea( const PolygonTrigger *areaToGuard, GuardMode guardMode, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_GUARD_AREA, cmdSource);
		parms.m_polygon = areaToGuard;
		parms.m_intValue = guardMode;
		aiDoCommand(&parms);
	}

	inline void aiGuardTunnelNetwork( GuardMode guardMode, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_GUARD_TUNNEL_NETWORK, cmdSource);
		parms.m_intValue = guardMode;
		aiDoCommand(&parms);
	}

	inline void aiHackInternet( CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_HACK_INTERNET, cmdSource);
		aiDoCommand(&parms);
	}

	inline void aiFaceObject( Object *target, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_FACE_OBJECT, cmdSource);
		parms.m_obj = target;
		aiDoCommand(&parms);
	}

	inline void aiFacePosition( const Coord3D *pos, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_FACE_POSITION, cmdSource);
		parms.m_pos = *pos;
		aiDoCommand(&parms);
	}

	inline void aiRappelInto( Object *target, const Coord3D& pos, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_RAPPEL_INTO, cmdSource);
		parms.m_obj = target;
		parms.m_pos = pos;
		aiDoCommand(&parms);
	}

	inline void aiCombatDrop( Object *target, const Coord3D& pos, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_COMBATDROP, cmdSource);
		parms.m_obj = target;
		parms.m_pos = pos;
		aiDoCommand(&parms);
	}

	inline void aiDoCommandButton( const CommandButton *commandButton, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_COMMANDBUTTON, cmdSource);
		parms.m_commandButton = commandButton;
		aiDoCommand(&parms);
	}

	inline void aiDoCommandButtonAtPosition( const CommandButton *commandButton, const Coord3D *pos, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_COMMANDBUTTON_POS, cmdSource);
		parms.m_pos = *pos;
		parms.m_commandButton = commandButton;
		aiDoCommand(&parms);
	}

	inline void aiDoCommandButtonAtObject( const CommandButton *commandButton, Object *obj, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_COMMANDBUTTON_OBJ, cmdSource);
		parms.m_obj = obj;
		parms.m_commandButton = commandButton;
		aiDoCommand(&parms);
	}

	inline void aiMoveAwayFromUnit( Object *obj, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_MOVE_AWAY_FROM_UNIT, cmdSource);
		parms.m_obj = obj;
		aiDoCommand(&parms);
	}

	inline void aiWander( const Waypoint *way, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_WANDER, cmdSource);
		parms.m_waypoint = way;
		aiDoCommand(&parms);
	}

	inline void aiWanderInPlace(CommandSourceType cmdSource)
	{
		AICommandParms parms(AICMD_WANDER_IN_PLACE, cmdSource);
		aiDoCommand(&parms);
	}

	inline void aiPanic( const Waypoint *way, CommandSourceType cmdSource )
	{
		AICommandParms parms(AICMD_PANIC, cmdSource);
		parms.m_waypoint = way;
		aiDoCommand(&parms);
	}

};


//------------------------------------------------------------------------------------------------------------
/**
 * An "AIGroup" is a simple collection of AI objects, used by the AI
 * for such things as Group Pathfinding.
 */
class AIGroup : public MemoryPoolObject, public Snapshot
{
private:
	void groupAttackObjectPrivate( Bool forced, Object *victim, Int maxShotsToFire, CommandSourceType cmdSource );					///< attack given object

public:

	// --------------- inherited from Snapshot interface --------------
	void crc( Xfer *xfer );
	void xfer( Xfer *xfer );
	void loadPostProcess( void );

#if !RETAIL_COMPATIBLE_AIGROUP
	void Add_Ref() const { m_refCount.Add_Ref(); }
	void Release_Ref() const { m_refCount.Release_Ref(destroy, this); }
	UnsignedShort Num_Refs() const { return m_refCount.Num_Refs(); }
#endif

	void groupMoveToPosition( const Coord3D *pos, Bool addWaypoint, CommandSourceType cmdSource );
	void groupMoveToAndEvacuate( const Coord3D *pos, CommandSourceType cmdSource );			///< move to given position(s)
	void groupMoveToAndEvacuateAndExit( const Coord3D *pos, CommandSourceType cmdSource );			///< move to given position & unload transport.
	void groupIdle(CommandSourceType cmdSource);						///< Enter idle state.
	void groupScatter(CommandSourceType cmdSource);						///< Enter idle state.
	void groupCreateFormation(CommandSourceType cmdSource); ///< Make the current selection a user formation.
	void groupTightenToPosition( const Coord3D *pos, Bool addWaypoint, CommandSourceType cmdSource );			///< move to given position(s)
	void groupFollowWaypointPath( const Waypoint *way, CommandSourceType cmdSource );///< start following the path from the given point
	void groupFollowWaypointPathAsTeam( const Waypoint *way, CommandSourceType cmdSource );///< start following the path from the given point
	void groupFollowWaypointPathExact( const Waypoint *way, CommandSourceType cmdSource );///< start following the path from the given point
	void groupFollowWaypointPathAsTeamExact( const Waypoint *way, CommandSourceType cmdSource );///< start following the path from the given point
	void groupFollowPath( const std::vector<Coord3D>* path, Object *ignoreObject, CommandSourceType cmdSource );///< follow the path defined by the given array of points
	void groupAttackObject( Object *victim, Int maxShotsToFire, CommandSourceType cmdSource )
	{
		groupAttackObjectPrivate(false, victim, maxShotsToFire, cmdSource);
	}
	void groupForceAttackObject( Object *victim, Int maxShotsToFire, CommandSourceType cmdSource )
	{
		groupAttackObjectPrivate(true, victim, maxShotsToFire, cmdSource);
	}
	void groupAttackTeam( const Team *team, Int maxShotsToFire, CommandSourceType cmdSource );							///< attack the given team
	void groupAttackPosition( const Coord3D *pos, Int maxShotsToFire, CommandSourceType cmdSource );						///< attack given spot
	void groupAttackMoveToPosition( const Coord3D *pos, Int maxShotsToFire, CommandSourceType cmdSource );	///< Attack move to the location
	void groupHunt( CommandSourceType cmdSource );														///< begin "seek and destroy"
	void groupRepair( Object *obj, CommandSourceType cmdSource );						///< repair the given object
	void groupResumeConstruction( Object *obj, CommandSourceType cmdSource );	///< resume construction on the object
	void groupGetHealed( Object *healDepot, CommandSourceType cmdSource );		///< go get healed at the heal depot
	void groupGetRepaired( Object *repairDepot, CommandSourceType cmdSource );///< go get repaired at the repair depot
	void groupEnter( Object *obj, CommandSourceType cmdSource );							///< enter the given object
	void groupDock( Object *obj, CommandSourceType cmdSource );							///< get near given object and wait for enter clearance
	void groupExit( Object *objectToExit, CommandSourceType cmdSource );			///< get out of this Object
	void groupEvacuate( CommandSourceType cmdSource );												///< empty its contents
	void groupExecuteRailedTransport( CommandSourceType cmdSource );					///< execute railed transport events
	void groupGoProne( const DamageInfo *damageInfo, CommandSourceType cmdSource );												///< life altering state change, if this AI can do it
	void groupGuardPosition( const Coord3D *pos, GuardMode guardMode, CommandSourceType cmdSource );						///< guard the given spot
	void groupGuardObject( Object *objToGuard, GuardMode guardMode, CommandSourceType cmdSource );			///< guard an object
	void groupGuardArea( const PolygonTrigger *areaToGuard, GuardMode guardMode, CommandSourceType cmdSource ); ///< guard an area
	void groupAttackArea( const PolygonTrigger *areaToGuard, CommandSourceType cmdSource ); ///< guard an area
	void groupHackInternet( CommandSourceType cmdSource );				///< Begin hacking the internet for free cash from the heavens.
	void groupDoSpecialPower( UnsignedInt specialPowerID, UnsignedInt commandOptions );
	void groupDoSpecialPowerAtObject( UnsignedInt specialPowerID, Object *object, UnsignedInt commandOptions );
	void groupDoSpecialPowerAtLocation( UnsignedInt specialPowerID, const Coord3D *location, Real angle, const Object *object, UnsignedInt commandOptions );
#ifdef ALLOW_SURRENDER
	void groupSurrender( const Object *objWeSurrenderedTo, Bool surrender, CommandSourceType cmdSource );
#endif
	void groupCheer( CommandSourceType cmdSource );
	void groupSell( CommandSourceType cmdSource );
	void groupToggleOvercharge( CommandSourceType cmdSource );
#ifdef ALLOW_SURRENDER
	void groupPickUpPrisoner( Object *prisoner, CommandSourceType cmdSource );	///< pick up prisoner
	void groupReturnToPrison( Object *prison, CommandSourceType cmdSource );		///< return to prison
#endif
	void groupCombatDrop( Object *target, const Coord3D& pos, CommandSourceType cmdSource );
	void groupDoCommandButton( const CommandButton *commandButton, CommandSourceType cmdSource );
	void groupDoCommandButtonAtPosition( const CommandButton *commandButton, const Coord3D *pos, CommandSourceType cmdSource );
	void groupDoCommandButtonAtObject( const CommandButton *commandButton, Object *obj, CommandSourceType cmdSource );
	void groupSetEmoticon( const AsciiString &name, Int duration );
	void groupOverrideSpecialPowerDestination( SpecialPowerType spType, const Coord3D *loc, CommandSourceType cmdSource );

	void setAttitude( AttitudeType tude );	///< set the behavior modifier for this agent
	AttitudeType getAttitude( void ) const;				///< get the current behavior modifier state

	Bool isIdle() const;
	//Definition of busy -- when explicitly in the busy state. Moving or attacking is not considered busy!
	Bool isBusy() const;
	Bool isGroupAiDead() const;

	// Returns an object that can perform the special power. Useful for making queries on the Action Manager
	Object *getSpecialPowerSourceObject( UnsignedInt specialPowerID );

	// Returns an object that has a command button for the GUI command type.
	Object *getCommandButtonSourceObject( GUICommandType type );

	// Group methods --------------------------------------------------------------------------------
	Bool isMember( Object *obj );						///< return true if object is in this group

	Real getSpeed( void );									///< return the speed of the group's slowest member
	Bool getCenter( Coord3D *center );				///< compute centroid of group
	Bool getMinMaxAndCenter( Coord2D *min, Coord2D *max, Coord3D *center );
	void computeIndividualDestination( Coord3D *dest, const Coord3D *groupDest,
		Object *obj, const Coord3D *center, Bool isFormation ); ///< compute destination of individual object, based on group destination
	Int getCount( void );										///< return the number of objects in the group
	Bool isEmpty( void );										///< returns true if the group has no members
	void queueUpgrade( const UpgradeTemplate *upgrade );	///< queue an upgrade

	void add( Object *obj );								///< add object to group

	// Returns true if the group was emptied.
	Bool remove( Object *obj);

	void removeAll( void );

	// If the group contains any objects not owned by ownerPlayer, return TRUE.
	Bool containsAnyObjectsNotOwnedByPlayer( const Player *ownerPlayer );

	// Removes any objects that aren't owned by the player, and returns true if the group was emptied.
	Bool removeAnyObjectsNotOwnedByPlayer( const Player *ownerPlayer );

	UnsignedInt getID( void );

	///< get IDs for every object in this group
	const VecObjectID& getAllIDs ( void ) const;

	void recomputeGroupSpeed() { m_dirty = true; }

	void setMineClearingDetail( Bool set );
	Bool setWeaponLockForGroup( WeaponSlotType weaponSlot, WeaponLockType lockType ); ///< Set the groups' weapon choice.
	void releaseWeaponLockForGroup(WeaponLockType lockType);///< Clear each guys weapon choice
	void setWeaponSetFlag( WeaponSetType wst );

protected:
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AIGroup, "AIGroupPool" );		///< @todo Set real numbers for mem alloc

	ListObjectPtrIt internalRemove(ListObjectPtrIt iterToRemove);

	Bool friend_moveInfantryToPos( const Coord3D *pos, CommandSourceType cmdSource );
	Bool friend_moveVehicleToPos( const Coord3D *pos, CommandSourceType cmdSource );
	void friend_moveFormationToPos( const Coord3D *pos, CommandSourceType cmdSource );
	Bool friend_computeGroundPath( const Coord3D *pos, CommandSourceType cmdSource );

private:
	// AIGroups must be created through TheAI->createGroup()
	friend class AI;
	AIGroup( void );

#if !RETAIL_COMPATIBLE_AIGROUP
	static void destroy(AIGroup* self)
	{
		TheAI->destroyGroup(self);
	}
#endif

	void recompute( void );									///< recompute various group info, such as speed, leader, etc

	ListObjectPtr m_memberList;							///< the list of member Objects
	UnsignedInt	m_memberListSize;	 					///< the size of the list of member Objects

	Real m_speed;														///< maximum speed of group (slowest member)
	Bool m_dirty;														///< "dirty bit" - if true then group speed, leader, needs recompute

#if !RETAIL_COMPATIBLE_AIGROUP
	RefCountValue<UnsignedShort> m_refCount; ///< the reference counter
#endif

	UnsignedInt m_id;												///< the unique ID of this group
	Path *m_groundPath;											///< Group ground path.

	mutable VecObjectID	m_lastRequestedIDList;			///< this is used so we can return by reference, saving a copy
};


#endif // _AI_H_
