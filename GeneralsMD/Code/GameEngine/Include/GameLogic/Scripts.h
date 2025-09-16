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


// Scripts.h
// Script data structures.
// Author: John Ahlquist, November 2001

#pragma once

#ifndef SCRIPTS_H
#define SCRIPTS_H

#include "Common/Snapshot.h"
#include "GameNetwork/NetworkDefs.h"
#include "Common/ObjectStatusTypes.h"

#define THIS_TEAM "<This Team>"
#define ANY_TEAM "<Any Team>"

#define THIS_OBJECT "<This Object>"
#define ANY_OBJECT "<Any Object>"

#define THIS_PLAYER "<This Player>"
#define LOCAL_PLAYER "<Local Player>"

#define THE_PLAYER "ThePlayer"
#define TEAM_THE_PLAYER "teamThePlayer"

#define THIS_PLAYER_ENEMY "<This Player's Enemy>"

#define WATER_GRID "Water Grid"

// Skirmish waypoint names
#define SKIRMISH_CENTER "Center"
#define SKIRMISH_FLANK "Flank"
#define SKIRMISH_BACKDOOR "Backdoor"
#define SKIRMISH_SPECIAL "Special"

// Skirmish Player Areas
#define SKIRMISH_AREA_HOME_BASE				"Home Base"
#define SKIRMISH_AREA_ANY_SUPPLYDEPOT	"Any Supply Depot"

// Skirmish trigger names.
#define MY_INNER_PERIMETER		"[Skirmish]MyInnerPerimeter"
#define MY_OUTER_PERIMETER		"[Skirmish]MyOuterPerimeter"
#define ENEMY_OUTER_PERIMETER "[Skirmish]EnemyOuterPerimeter"
#define ENEMY_INNER_PERIMETER "[Skirmish]EnemyInnerPerimeter"

#define INNER_PERIMETER "InnerPerimeter"
#define OUTER_PERIMETER "OuterPerimeter"

class Parameter;
class Script;
class OrCondition;
class Condition;
class DataChunkInput;
struct DataChunkInfo;
class DataChunkOutput;

#define NO_MORE_COMPLEX_SKIRMISH_SCRIPTS
#ifndef NO_MORE_COMPLEX_SKIRMISH_SCRIPTS
// For now, we only actually allow the simple stuff to be chosen. However, as soon as I can get more
// time on the schedule, we need to extend these scripts to be more useful. To that end, I've
// created the parameters and such necessary for more complex behavior, but for now only the
// rudimentary support is enabled. jkmcd
	#define MORE_COMPLEX_SKIRMISH_SCRIPTS
#endif


// There is undoubtedly a more sophisticated way to do this, but for some reason my brain hurts.
// The corresponding UnsignedInt values that we pass around are:
// 1 means ground
// 2 means air
// 3 means air or ground
extern const char *Surfaces[];

// Shake intensities. Must be kept in sync with View::CameraShakeType
extern const char *ShakeIntensities[];

//-------------------------------------------------------------------------------------------------
// ******************************** class ScriptGroup ***********************************************
//-------------------------------------------------------------------------------------------------
class ScriptGroup : public MemoryPoolObject, public Snapshot
// This is a list of scripts that are grouped and named, with some
// additional properties.
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ScriptGroup, "ScriptGroup")
protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

protected:
	Script			*m_firstScript;
	AsciiString m_groupName;
	Bool				m_isGroupActive;
	Bool				m_isGroupSubroutine;
	ScriptGroup	*m_nextGroup;
	Bool				m_hasWarnings; ///< Runtime flag used by the editor only.

public:
	ScriptGroup();
	//~ScriptGroup();

public:

	ScriptGroup *duplicate(void) const;		// note, duplicates just this node, not the full list.
	ScriptGroup *duplicateAndQualify(const AsciiString& qualifier,
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const;		// note, duplicates just this node, not the full list.

	void setName(AsciiString name) { m_groupName = name;}
	void setActive(Bool active) { m_isGroupActive = active;}
	void setSubroutine(Bool subr) { m_isGroupSubroutine = subr;}
	void setWarnings(Bool warnings) { m_hasWarnings = warnings;}
	void setNextGroup(ScriptGroup *pGr) {m_nextGroup = pGr;}

	AsciiString getName(void) const { return m_groupName;}
	Bool isActive(void) const { return m_isGroupActive;}
	Bool isSubroutine(void) const { return m_isGroupSubroutine;}
	Bool hasWarnings(void) const { return m_hasWarnings;}
	ScriptGroup *getNext(void) const {return m_nextGroup;};
	Script *getScript(void) {return m_firstScript;};

	void addScript(Script *pScr, Int ndx);
	void deleteScript(Script *pScr);

	static void WriteGroupDataChunk(DataChunkOutput &chunkWriter, ScriptGroup *pGroup);
	static Bool ParseGroupDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);
};

//-------------------------------------------------------------------------------------------------
// ******************************** class OrCondition ***********************************************
//-------------------------------------------------------------------------------------------------
/// This is a linked list of or clauses, each containing a list of and clauses.
class OrCondition : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(OrCondition, "OrCondition")
protected:
	OrCondition *m_nextOr;   // Next or clause.
	Condition *m_firstAnd;	 // These are Anded.

public:
	OrCondition():m_nextOr(NULL),m_firstAnd(NULL){};
	//~OrCondition();
	/// Duplicate creates a "deep" copy.  If it is head of a linked list, duplicates the entire list.
	OrCondition *duplicate(void) const;
	OrCondition *duplicateAndQualify(const AsciiString& qualifier,
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const;

public:
	void setNextOrCondition(OrCondition *pOr) {m_nextOr = pOr;}
	void setFirstAndCondition(Condition *pAnd) {m_firstAnd = pAnd;}

	OrCondition *getNextOrCondition(void) {return m_nextOr;}
	Condition *getFirstAndCondition(void) {return m_firstAnd;}

	Condition *removeCondition(Condition *pCond);
	void deleteCondition(Condition *pCond);
	static void WriteOrConditionDataChunk(DataChunkOutput &chunkWriter, OrCondition *pCondition);
	static Bool ParseOrConditionDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);

	// Utility for moving scripts upwards
	Condition *findPreviousCondition( Condition *curCond );
};

//-------------------------------------------------------------------------------------------------
// ******************************** class ScriptAction ***********************************************
//-------------------------------------------------------------------------------------------------
/** This is an action that can be executed by a script.  The Action Types
are defined by ActionTemplates created in the ScriptEngine::init method. */
enum {MAX_PARMS=12};

class ScriptAction : public MemoryPoolObject // This is the action class.
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ScriptAction, "ScriptAction")
// friend bad for MPOs. (srj)
//friend class EditAction;
public:
	enum ScriptActionType
	{
		DEBUG_MESSAGE_BOX,							///< Show a message box.
		SET_FLAG,													///< Set a flag true of false.
		SET_COUNTER,											///< Set a counter to an integer.
		VICTORY,													///< Announce victory.
		DEFEAT,														///< Announce defeat.
		NO_OP, 														///< No op.
		SET_TIMER,												///< Start a frame timer.
		PLAY_SOUND_EFFECT,								///< Play sound.
		ENABLE_SCRIPT,										///< Enable script.
		DISABLE_SCRIPT,										///< Disable script.
		CALL_SUBROUTINE,									///< Call a subroutine.
		PLAY_SOUND_EFFECT_AT,							///< Play sound at location.
		DAMAGE_MEMBERS_OF_TEAM,						///< Damage team members (boom.)
		MOVE_TEAM_TO,											///< Move to.
		MOVE_CAMERA_TO,										///< Move camera to.
		INCREMENT_COUNTER,								///< Add to a counter.
		DECREMENT_COUNTER,								///< subtract from a counter.
		MOVE_CAMERA_ALONG_WAYPOINT_PATH,	///< Move camera along path.
		ROTATE_CAMERA,										///< Rotate camera.
		RESET_CAMERA,											///< Reset camera.
		SET_MILLISECOND_TIMER,						///< Start a time based timer.
		CAMERA_MOD_FREEZE_TIME,						///< Freeze time during a camera movement.
		SET_VISUAL_SPEED_MULTIPLIER,			///< Drop frames so that time "goes" faster..
		CREATE_OBJECT,										///< Create an object
		SUSPEND_BACKGROUND_SOUNDS,				///< Stop backgropund music & sounds.
		RESUME_BACKGROUND_SOUNDS,					///< Resume backgropund music & sounds.
		CAMERA_MOD_SET_FINAL_ZOOM,				///< Sets the final zoom for a camera movement
		CAMERA_MOD_SET_FINAL_PITCH,				///< Sets the final pitch for a camera movement
		CAMERA_MOD_FREEZE_ANGLE,					///< Freeze camera angle during a camera movement.
		CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER,///< Sets the final time multiplier for a camera movement.
		CAMERA_MOD_SET_ROLLING_AVERAGE,		///< Sets the number of frames to average changes (angle, positoin) to smooth out a camera movement.
		CAMERA_MOD_FINAL_LOOK_TOWARD,			///< Sets the look toward point for the end of a camera movement.
		CAMERA_MOD_LOOK_TOWARD,						///< Sets the look toward point during a camera movement.
		TEAM_ATTACK_TEAM,									///< Tell team to attack other team
		CREATE_REINFORCEMENT_TEAM,				///< Create a team of reinforcements.
		MOVE_CAMERA_TO_SELECTION,					///< Move the target to the selected item.
		TEAM_FOLLOW_WAYPOINTS,						///< Tell the team to begin following a path, starting at given waypoint
		TEAM_SET_STATE,										///< Set a team to a particular state.
		MOVE_NAMED_UNIT_TO,								///< Move a named unit to a position
		NAMED_ATTACK_NAMED,								///< Make a named unit attack another named unit
		CREATE_NAMED_ON_TEAM_AT_WAYPOINT,	///< Create a named unit on a team at a waypoint
		CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT,	///< Create a unnamed unit on a team at a waypoint
		NAMED_APPLY_ATTACK_PRIORITY_SET,	///< Applies an attack priority set to a named unit
		TEAM_APPLY_ATTACK_PRIORITY_SET,		///< Applies an attack priority set to a team
		SET_BASE_CONSTRUCTION_SPEED,			///< Impl JKMCD
		NAMED_SET_ATTITUDE,								///< Impl JKMCD
		TEAM_SET_ATTITUDE,								///< Impl JKMCD
		NAMED_ATTACK_AREA,								///< Set a unit to attack an area
		NAMED_ATTACK_TEAM,								///< Impl JKMCD
		TEAM_ATTACK_AREA,									///< Set a team to attack an area
		TEAM_ATTACK_NAMED,								///< Set a team to attack a unit
		TEAM_LOAD_TRANSPORTS,							///< Set a team to auto-load transports
		NAMED_ENTER_NAMED,								///< Set a unit to enter a transport
		TEAM_ENTER_NAMED,									///< Set a team to enter a transport
		NAMED_EXIT_ALL,										///< Set a unit to exit its transport
		TEAM_EXIT_ALL,										///< Set a team to exit all transports
		NAMED_FOLLOW_WAYPOINTS,						///< Set a unit to follow a waypoint path
		NAMED_GUARD,											///< Set a unit to guard it's current location.
		TEAM_GUARD,												///< Set a team to guard it's current location.
		NAMED_HUNT,												///< Set a unit to hunt
		TEAM_HUNT,												///< Set a team to hunt
		PLAYER_SELL_EVERYTHING,						///< Set a player to sell everything
		PLAYER_DISABLE_BASE_CONSTRUCTION,	///< Disallow a player from building anymore base buildings
		PLAYER_DISABLE_FACTORIES,					///< Disallow a player from building from factories
		PLAYER_DISABLE_UNIT_CONSTRUCTION,	///< Disallow a player from building units
		PLAYER_ENABLE_BASE_CONSTRUCTION,	///< Allow a player to build bases
		PLAYER_ENABLE_FACTORIES,					///< Allow a player to build from his factories
		PLAYER_ENABLE_UNIT_CONSTRUCTION,	///< Allow a player to build units
		CAMERA_MOVE_HOME,									///< Impl JKMCD
		BUILD_TEAM,												///< Build a team from a teamtemplate
		NAMED_DAMAGE,											///< Damage a unit
		NAMED_DELETE,											///< Delete a unit
		TEAM_DELETE,											///< Delete a team
		NAMED_KILL,												///< Kill a unit
		TEAM_KILL,												///< Kill a team
		PLAYER_KILL,											///< Kill a player
		DISPLAY_TEXT,											///< Display a localized string
		CAMEO_FLASH,											///< Flashes a command button
		NAMED_FLASH,											///< Flashes a specific unit
		TEAM_FLASH,												///< Flashes a specific team
		MOVIE_PLAY_FULLSCREEN,						///< Play a movie in fullscreen
		MOVIE_PLAY_RADAR,									///< Play a movie in the radar
		SOUND_PLAY_NAMED,									///< Play a sound as though it comes from a unit
		SPEECH_PLAY,											///< Play a speech
		PLAYER_TRANSFER_OWNERSHIP_PLAYER,	///< Transfer ownership of all units to another player
		NAMED_TRANSFER_OWNERSHIP_PLAYER,	///< Transfer ownership of a single unit to another player
		PLAYER_RELATES_PLAYER,						///< Update player-player relations
		RADAR_CREATE_EVENT,								///< Impl JKMCD
		RADAR_DISABLE,										///< Disable the radar
		RADAR_ENABLE,											///< Enable the radar
		MAP_REVEAL_AT_WAYPOINT,						///< Reveal the map within a radius of waypoint.
		TEAM_AVAILABLE_FOR_RECRUITMENT,		///< Impl JKMCD
		TEAM_COLLECT_NEARBY_FOR_TEAM,			///< Impl JKMCD
		TEAM_MERGE_INTO_TEAM,							///< Merge a team into another team
		DISABLE_INPUT,										///< Disable user input
		ENABLE_INPUT,											///< Re-enable user input
		PLAYER_HUNT,											///< Set an entire player's team to hunt
		SOUND_AMBIENT_PAUSE,							///< Pause all ambient sounds
		SOUND_AMBIENT_RESUME,							///< Resume all ambient sounds
		MUSIC_SET_TRACK,									///< Set the current music track, allowing for fade-in and fade-out
		SET_TREE_SWAY,										///< Set the trees' sway
		DEBUG_STRING,											///< Output a debug string without stopping.
		MAP_REVEAL_ALL,										///< Reveal the entire map
		TEAM_GARRISON_SPECIFIC_BUILDING,	///< Garrison a specific building
		EXIT_SPECIFIC_BUILDING,						///< Empty a specific building
		TEAM_GARRISON_NEAREST_BUILDING,		///< Garrison a nearby building
		TEAM_EXIT_ALL_BUILDINGS,					///< Empty all buildings of units
		NAMED_GARRISON_SPECIFIC_BUILDING,	///< Send a unit to garrison a specific building
		NAMED_GARRISON_NEAREST_BUILDING,		///< Send a unit to garrison the nearest building
		NAMED_EXIT_BUILDING,								///< Tell a unit to exit the building its currently in
		PLAYER_GARRISON_ALL_BUILDINGS,		///< Garrison as many buildings as possible (filling each one up)
		PLAYER_EXIT_ALL_BUILDINGS,				///< Tell all buildings to empty themselves for this player.
		TEAM_WANDER,											///< Tell a team to wander.
		TEAM_PANIC,												///< Tell a team to panic.
		SETUP_CAMERA,											///< Set up an initial camera position.
		CAMERA_LETTERBOX_BEGIN,						///< Set the camera into letterbox mode
		CAMERA_LETTERBOX_END,							///< End the Letterbox mode.
		ZOOM_CAMERA,											///< Set camera zoom.
		PITCH_CAMERA,											///< Set camera pitch.
		CAMERA_FOLLOW_NAMED,							///< Camera follow named.
		OVERSIZE_TERRAIN,									// Oversize the drawn terrain for effect.
		CAMERA_FADE_ADD,									///< Fade using an add blend
		CAMERA_FADE_SUBTRACT,							///< Fade using a subtract blend
		CAMERA_FADE_SATURATE,							///< Fade using a saturate blend
		CAMERA_FADE_MULTIPLY,							///< Fade using a multiply blend
		CAMERA_BW_MODE_BEGIN,							///< switch camera to black & white mode.
		CAMERA_BW_MODE_END,								///< switch camera back to color mode.
		DRAW_SKYBOX_BEGIN,								///< draw skybox
		DRAW_SKYBOX_END,									///< stop skybox
		SET_ATTACK_PRIORITY_THING,				///< Sets the attack priority for a single thing in a set.
		SET_ATTACK_PRIORITY_KIND_OF,			///< Sets the attack priority for all of a kind	in a set.
		SET_DEFAULT_ATTACK_PRIORITY,			///< Sets the default attack priority for a set.
		CAMERA_STOP_FOLLOW,								///< Ends a camera follow named.
		CAMERA_MOTION_BLUR,								///< Starts a camera motion blur zoom
		CAMERA_MOTION_BLUR_JUMP,					///< Starts a camera motion blur in & out with jump.
		CAMERA_MOTION_BLUR_FOLLOW,				///< Starts a motion blur follow scrolling effect.
		CAMERA_MOTION_BLUR_END_FOLLOW,		///< Ends a motion blur follow scrolling effect.
		FREEZE_TIME,											///< Freeze time.
		UNFREEZE_TIME,										///< Unfreeze time.
		SHOW_MILITARY_CAPTION,						///< Display military-style briefing text
		CAMERA_SET_AUDIBLE_DISTANCE,			///< Sets the range for the audible distance
		SET_STOPPING_DISTANCE,						///< Sets the stopping distance for a team
		NAMED_SET_STOPPING_DISTANCE,			///< Sets the stopping distance for a named object
		SET_FPS_LIMIT,										///< Sets the max frames-per-second
		MUSIC_SET_VOLUME,									///< Adjust the track volume
		MAP_SHROUD_AT_WAYPOINT,						///< Shroud the map at this waypoint
		MAP_SHROUD_ALL,										///< Shroud the entire map
		SET_RANDOM_TIMER,									///< Start a random frame based timer.
		SET_RANDOM_MSEC_TIMER,						///< Start a random time based timer.
		STOP_TIMER,												///< Stop a timer. Remembers time stopped.
		RESTART_TIMER,										///< Restarts a stopped timer. Remembers time stopped.
		ADD_TO_MSEC_TIMER,								///< Adds time to a timer.
		SUB_FROM_MSEC_TIMER,							///< Subtracts time from a timer.
		TEAM_TRANSFER_TO_PLAYER,					///< Transfers a team to a player, maintaining team-ness
		PLAYER_SET_MONEY,									///< Set a player's cash reserves to a specific value.
		PLAYER_GIVE_MONEY,								///< // Add/subtract cash from a player's reserves.
		DISABLE_SPECIAL_POWER_DISPLAY,		///< hides the special power countdown display
		ENABLE_SPECIAL_POWER_DISPLAY,			///< shows the special power countdown display
		NAMED_HIDE_SPECIAL_POWER_DISPLAY,	///< hides the special power countdown display for an object
		NAMED_SHOW_SPECIAL_POWER_DISPLAY,	///< shows the special power countdown display for an object
		DISPLAY_COUNTDOWN_TIMER,					///< shows a countdown timer
		HIDE_COUNTDOWN_TIMER,							///< hides a countdown timer
		ENABLE_COUNTDOWN_TIMER_DISPLAY,		///< enables countdown timer display
		DISABLE_COUNTDOWN_TIMER_DISPLAY,	///< disables countdown timer display
		NAMED_STOP_SPECIAL_POWER_COUNTDOWN,	///< pauses the countdown for a special power
		NAMED_START_SPECIAL_POWER_COUNTDOWN,///< resumes the countdown for a special power
		NAMED_SET_SPECIAL_POWER_COUNTDOWN,	///< set special power countdown for an object
		NAMED_ADD_SPECIAL_POWER_COUNTDOWN,	///< add/subtract time from a special power countdown
		NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT,	///< fire a special power
		NAMED_FIRE_SPECIAL_POWER_AT_NAMED,	///< fire a special power
		REFRESH_RADAR,										///< Force the radar to recalculate the terrain
		CAMERA_TETHER_NAMED,							///< Tether the camera location to a named unit
		CAMERA_STOP_TETHER_NAMED,					///< Break tether
		CAMERA_SET_DEFAULT,								///< Set default angle, pitch, zoom, etc for camera
		NAMED_STOP,												///< Make a named unit stop (go idle)
		TEAM_STOP,												///< Make a team stop (go idle)
		TEAM_STOP_AND_DISBAND,						///< Make a team stop and be merged into the default player team. Also, mark the whole team recruitable
		RECRUIT_TEAM,	  									///< Recruit a team?
		TEAM_SET_OVERRIDE_RELATION_TO_TEAM,					///< override a team's relation to another team
		TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM,			///< remove that override
		TEAM_REMOVE_ALL_OVERRIDE_RELATIONS,///< remove all overrides
		CAMERA_LOOK_TOWARD_OBJECT,				///< Rotate the camera to be pointing toward a unit
		NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH,    ///< Fires a specific weapon that follows waypoint path and detonates at the final waypoint.
		TEAM_SET_OVERRIDE_RELATION_TO_PLAYER,					///< override a team's relation to another player
		TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER,			///< remove that override
		PLAYER_SET_OVERRIDE_RELATION_TO_TEAM,					///< override a Player's relation to another team
		PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM,			///< remove that override
		UNIT_EXECUTE_SEQUENTIAL_SCRIPT,		///< Make a unit execute a script sequentially
		UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING,		///< Make a unit execute a script sequentially (forever)
		UNIT_STOP_SEQUENTIAL_SCRIPT,			///< Make a unit stop executing its sequential scripts
		TEAM_EXECUTE_SEQUENTIAL_SCRIPT,		///< Make a team execute a script sequentially
		TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING,		///< Make a team execute a script sequentially (forever)
		TEAM_STOP_SEQUENTIAL_SCRIPT,			///< Make a team stop executing its sequential scripts
		UNIT_GUARD_FOR_FRAMECOUNT,				///< Make a specific unit guard for a framecount
		UNIT_IDLE_FOR_FRAMECOUNT,					///< Make a specific unit stop idle for a framecount
		TEAM_GUARD_FOR_FRAMECOUNT,				///< Make a team guard for a framecount
		TEAM_IDLE_FOR_FRAMECOUNT,					///< Make a team stop idle for a framecount
		WATER_CHANGE_HEIGHT,							///< Adjust the water level
		NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED,				///< Make a unit use an ability on another unit.
		NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT,		///< Make a unit use an ability on a waypoint.
		WATER_CHANGE_HEIGHT_OVER_TIME,						///< Adjust the water level over a period of time.
		MAP_SWITCH_BORDER,												///< Switch to border color <Blah>
		TEAM_GUARD_POSITION,
		TEAM_GUARD_OBJECT,
		TEAM_GUARD_AREA,
		OBJECT_FORCE_SELECT,											///< Force selection of the first object type on the player's team
		CAMERA_LOOK_TOWARD_WAYPOINT,							///< Rotate the camera to be pointing toward a waypoint
		UNIT_DESTROY_ALL_CONTAINED,								///< Destroy all units contained by the named unit
		RADAR_FORCE_ENABLE,												///< Force the radar to be on regardless of player status
		RADAR_REVERT_TO_NORMAL,										///< Undo forcing the radar to be on
		SCREEN_SHAKE,															///< Shake the entire screen
		TECHTREE_MODIFY_BUILDABILITY_OBJECT,			///< Modify how "buildable" something is.
		WAREHOUSE_SET_VALUE,											///< Set a Warehouse to have this much money.
		OBJECT_CREATE_RADAR_EVENT,								///< Create a radar event at this object
		TEAM_CREATE_RADAR_EVENT,									///< Create a radar event at this team
		DISPLAY_CINEMATIC_TEXT,									  ///< Displays a string in the bottom letterbox
		DEBUG_CRASH_BOX,													///< Debug/Internal builds only: do a crash box
		SOUND_DISABLE_TYPE,
		SOUND_ENABLE_TYPE,
		SOUND_ENABLE_ALL,
		AUDIO_OVERRIDE_VOLUME_TYPE,
		AUDIO_RESTORE_VOLUME_TYPE,
		AUDIO_RESTORE_VOLUME_ALL_TYPE,
		INGAME_POPUP_MESSAGE,											///< Popup an ingame popup message box
		SET_CAVE_INDEX,														///< Set the index of which set of caves a cave is connected to.
		NAMED_SET_HELD,														///< Sets a named object to be "held" in place or not
		NAMED_SET_TOPPLE_DIRECTION,								///< Set the topple direction of the specified direction
		UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE,		///< Move a named unit towards the nearest object of type type
		TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE,		///< For each member of the team, move towards the nearest object of type
		MAP_REVEAL_ALL_PERM,
		MAP_REVEAL_ALL_UNDO_PERM,
		NAMED_SET_REPULSOR,												///< Set a unit as a repulsor (scares) civilians.
		TEAM_SET_REPULSOR,												///< Set a team as a repulsor (scares) civilians.
		TEAM_WANDER_IN_PLACE,											///< Set a team to wander around their current position.
		TEAM_INCREASE_PRIORITY,										///< Increment a team's ai priority by it's success amount.
		TEAM_DECREASE_PRIORITY,										///< Decrease a team's ai prioiryt by it's failure amount.
		DISPLAY_COUNTER,													///< shows a counter
		HIDE_COUNTER,															///< hides a counter
		TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED,	///< Make a team use an ability on another unit.
		TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT,			///< Make a team use an ability on a waypoint.
		NAMED_USE_COMMANDBUTTON_ABILITY,					///< Make a unit use an ability.
		TEAM_USE_COMMANDBUTTON_ABILITY,						///< Make a team use an ability.
		NAMED_FLASH_WHITE,												///< Flashes a specific unit
		TEAM_FLASH_WHITE,													///< Flashes a specific team
		SKIRMISH_BUILD_BUILDING,									///< Start building a building.
		SKIRMISH_FOLLOW_APPROACH_PATH,						///< Follow a particular route to the enemy.
		IDLE_ALL_UNITS,														///< cancel any current actions for one player (or all players)
		RESUME_SUPPLY_TRUCKING,										///< Re-enable supply trucking for *idle* trucks for one player (or all players)
		NAMED_CUSTOM_COLOR,												///< set a specific unit to use a special indicator color (not his controlling player's)
		SKIRMISH_MOVE_TO_APPROACH_PATH,						///< Follow a particular route to the enemy.
		SKIRMISH_BUILD_BASE_DEFENSE_FRONT,				///< Build some base defense on the front.
		SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST,	///< Fire a power or superweapon at the enemy's most costly concentration of structures & units..
		NAMED_RECEIVE_UPGRADE,										///< Unit receives an upgrade
		PLAYER_REPAIR_NAMED_STRUCTURE,						///< Send a dozer to repair a structure.
		SKIRMISH_BUILD_BASE_DEFENSE_FLANK,				///< Build some base defense on the flank.
		SKIRMISH_BUILD_STRUCTURE_FRONT,						///< Build some structure on the front.
		SKIRMISH_BUILD_STRUCTURE_FLANK,						///< Build some structure on the flank.
		SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE,	///< Send a team to attack the nearest group with value comparison
		SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT,	///< Send a single unit/ all available to perform a command button action on the most valuable target
		SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL,		///< In a sequential script, stay on this step until the specified command button is ready for all units.
		SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL,///< In a sequential script, stay on this step until the specified command button is ready for one unit on the team.
		TEAM_SPIN_FOR_FRAMECOUNT,									///< Tell the team to continue its current task without advancing for this number of frames
		TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED,			///< Tell the team to use the commandbutton on a named enemy
		TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT,	///< Tell the team to use the commandbutton on the nearest enemy unit
		TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING,	///< Tell the team to use the commandbutton on the nearest enemy garrisoned building
		TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF,	///< Tell the team to use the commandbutton on the nearest enemy that has kindof set
		TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING,	///< Tell the team to use the commandbutton on the nearest enemy building
		TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS,	///< Tell the team to use the commandbutton on the nearest enemy building class
		TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE,	///< Tell the team to use the commandbutton on the nearest enemy object of type
		TEAM_PARTIAL_USE_COMMANDBUTTON,						///< Tell some percentage of the team to use the commandbutton on
		TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT,///< Tell the team to capture the nearest unowned faction unit (unit whose pilot has been killed by Jarmen Kell)
		PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS,		///< Tell the player to create a new named team from all the units who have been captured.
		PLAYER_ADD_SKILLPOINTS,										///< add/subtract skill points to the player.
		PLAYER_ADD_RANKLEVEL,											///< add/subtract ranklevel for the player.
		PLAYER_SET_RANKLEVEL,											///< set the player's ranklevel to an explicit level.
		PLAYER_SET_RANKLEVELLIMIT,								///< set the RankLevelLimit for the current map.
		PLAYER_GRANT_SCIENCE,											///< grant a Science to the player, regardless of prereqs, etc
		PLAYER_PURCHASE_SCIENCE,									///< the Player purchases the specified Science (if he has enough points, prereqs, etc)
		TEAM_HUNT_WITH_COMMAND_BUTTON,						///< Hunt using a command button ability.
		TEAM_WAIT_FOR_NOT_CONTAINED_ALL,					///< In a sequential script, stay on this step until the entire team is no longer contained
		TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL,			///< In a sequential script, stay on this step until at least one member of the team is no longer contained
		TEAM_FOLLOW_WAYPOINTS_EXACT,							///< Follows waypoints exactly.
		NAMED_FOLLOW_WAYPOINTS_EXACT,							///< Follows waypoints exactly.
		TEAM_SET_EMOTICON,												///< Adds specified emoticon to each unit in team (-1 permanent, 0 get rid of, 1+ duration in ms)
		NAMED_SET_EMOTICON,												///< Adds specified emoticon to named unit (-1 permanent, 0 get rid of, 1+ duration in ms)
		AI_PLAYER_BUILD_SUPPLY_CENTER,						///< Tell the ai player to build a supply center.
		AI_PLAYER_BUILD_UPGRADE,									///< Tell the ai player to build an upgrade.
		OBJECTLIST_ADDOBJECTTYPE,									///< Add the associated object to the named list
		OBJECTLIST_REMOVEOBJECTTYPE,							///< Remove the associated object to the named list
		MAP_REVEAL_PERMANENTLY_AT_WAYPOINT,				///< Reveal an area at a waypoint
		MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT,	///< Undo a previous permanent reveal
		NAMED_SET_STEALTH_ENABLED,								///< Enables/disables stealth capability for a specific unit.
		TEAM_SET_STEALTH_ENABLED,									///< Enables/disables stealth capability for a team.
		EVA_SET_ENABLED_DISABLED,									///< Sets EVA to continue to evaluate messages or not.
		OPTIONS_SET_OCCLUSION_MODE,								///< Enables behind building functionality.
		LOCALDEFEAT,															///< Announce local player's defeat.
		OPTIONS_SET_DRAWICON_UI_MODE,							///< Enables/disables all emoticons.
		OPTIONS_SET_PARTICLE_CAP_MODE,						///< Enables/disables particle cap.
		PLAYER_SCIENCE_AVAILABILITY,							///< Determines the availability of a specific science.
		UNIT_AFFECT_OBJECT_PANEL_FLAGS,						///< Affects the various flags on the object panel for an individual unit
		TEAM_AFFECT_OBJECT_PANEL_FLAGS,						///< Affects the various flags on the object panel for all units on a team
		PLAYER_SELECT_SKILLSET,										///< Select the skillset for an AI player.
		SCRIPTING_OVERRIDE_HULK_LIFETIME,					///< Overrides the lifetime of hulks (for cinematic purposes).
		NAMED_FACE_NAMED,													///< Orders unit to face the position of the other named object.
		NAMED_FACE_WAYPOINT,											///< Orders unit to face a waypoint.
		TEAM_FACE_NAMED,													///< Orders all members of a team to face a specific unit.
		TEAM_FACE_WAYPOINT,												///< Orders all members of a team to face a waypoint.
		COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE,			///< Remove a button from a command bar for an objecttype
		COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT,		///< Add a button to the command bar for an objecttype, in a specific slot
		UNIT_SPAWN_NAMED_LOCATION_ORIENTATION,		///< Create a named unit at the specified location, altitude, and orientation
		PLAYER_AFFECT_RECEIVING_EXPERIENCE,				///< Adjust whether or not a player is receieving experience for kills
		PLAYER_EXCLUDE_FROM_SCORE_SCREEN,					///< This player should be listed in the score screen.  Should only be used in campaign games.
		TEAM_GUARD_SUPPLY_CENTER,									///< Have an ai team guard the nearest available supply center..
		ENABLE_SCORING,														///< Turn on scoring of kills, units destroyed, etc.
		DISABLE_SCORING,													///< Turn off scoring of kills, units destroyed, etc.
		SOUND_SET_VOLUME,													///< Adjust the volume of the sounds
		SPEECH_SET_VOLUME,												///< Adjust the volume of the speech
		DISABLE_BORDER_SHROUD,										///< Clear the shroud that draws off the map border
		ENABLE_BORDER_SHROUD,											///< Restore the shroud that draws off the map border (normally on)
		OBJECT_ALLOW_BONUSES,											///< Change whether or not the bonuses are applied to objects.
		SOUND_REMOVE_ALL_DISABLED,								///< Remove any and all disabled sounds from the sound channels.
		SOUND_REMOVE_TYPE,												///< Remove sound of type.
		TEAM_GUARD_IN_TUNNEL_NETWORK,							///< Team enters nearest tunnel network & guards.
		QUICKVICTORY,															///< End map in victory immediately.
		SET_INFANTRY_LIGHTING_OVERRIDE,						///< Overrides the natural lighting on infantry for closeups
		RESET_INFANTRY_LIGHTING_OVERRIDE,					///< And then goes back to the GlobalData defaults
		TEAM_DELETE_LIVING,												///< Delete only the living members of this team
		RESIZE_VIEW_GUARDBAND,										///< Allow bigger objects to be perceived as onscreen near the edge
		DELETE_ALL_UNMANNED,											///< Delete all unmanned (sniped) vehicles
		CHOOSE_VICTIM_ALWAYS_USES_NORMAL,					///< choose victim always uses normal AI behavior, ignoring game difficulty
		CAMERA_ENABLE_SLAVE_MODE,
		CAMERA_DISABLE_SLAVE_MODE,
		CAMERA_ADD_SHAKER_AT,			              	///< WST added 10.12.2002 (MBL)
		SET_TRAIN_HELD,				                    ///< LORENZEN -- Forbids trains from departing stations while true
    NAMED_SET_EVAC_LEFT_OR_RIGHT,	            ///< LORENZEN -- Which side of the garrisoned unit (LIKELY A TRAIN) do you want units to evacuate from?
    ENABLE_OBJECT_SOUND,                      ///< Enables the ambient sound on an object or fire the one-shot "ambient" sound on an object
    DISABLE_OBJECT_SOUND,                     ///< Disable the ambient sound on an object or kill the one-shot "ambient" sound on an object
		NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH, ///< Added for particle cannon to have beam follow waypoint path.
		NAMED_SET_UNMANNED_STATUS,								///< Make unit unmanned or manned.
		TEAM_SET_UNMANNED_STATUS,									///< Make all units on team unmanned or manned.
		NAMED_SET_BOOBYTRAPPED,										///< Add boobytrap to unit.
		TEAM_SET_BOOBYTRAPPED,										///< Add boobytrap to all units on team.
		SHOW_WEATHER,															///< show map defined weather.
		AI_PLAYER_BUILD_TYPE_NEAREST_TEAM,				///< Tell the ai player to build an object nearest team.
		// add new items here, please
		NUM_ITEMS
	};
	ScriptAction(ScriptActionType type);
	//~ScriptAction();

	ScriptAction *duplicate(void) const;
	ScriptAction *duplicateAndQualify(const AsciiString& qualifier,
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const;

protected:
		ScriptAction();  ///< Protected constructor for read.

protected:
	ScriptActionType	m_actionType;
	Int 							m_numParms;
	Parameter *				m_parms[MAX_PARMS];
	ScriptAction *		m_nextAction;
	Bool							m_hasWarnings; ///< Runtime flag used by the editor only.

public:
	void setActionType(ScriptActionType type);

public:
	void setNextAction(ScriptAction *pAct) {m_nextAction = pAct;}
	void setWarnings(Bool warnings) { m_hasWarnings = warnings;}
	ScriptActionType getActionType(void) {return m_actionType;}
	ScriptAction * getNext(void) {return m_nextAction;}
	AsciiString getUiText(void);
	Parameter *getParameter(Int ndx)
	{
		if (ndx>=0 && ndx<m_numParms)
			return m_parms[ndx];

		return NULL;
	}
	Bool hasWarnings(void) const { return m_hasWarnings;}
	Int getNumParameters(void) {return m_numParms;}
	Int getUiStrings(AsciiString strings[MAX_PARMS]);

	static void WriteActionDataChunk(DataChunkOutput &chunkWriter, ScriptAction *pAct);
	static Bool ParseActionDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);
	static void WriteActionFalseDataChunk(DataChunkOutput &chunkWriter, ScriptAction *pAct);
	static Bool ParseActionFalseDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);

protected:
	static ScriptAction *ParseAction(DataChunkInput &file, DataChunkInfo *info, void *userData);

};

//-------------------------------------------------------------------------------------------------
// ******************************** class Script ***********************************************
//-------------------------------------------------------------------------------------------------
/** This is a script, in a linked list of scripts.  The m_condition list is evaluated, and if one of the
or clauses is true, the m_action list is executed.  . */
class Script : public MemoryPoolObject, public Snapshot
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(Script, "Script")
protected:	// Note - If you add any member vars, you must take them into account in duplicate() and updateFrom(), as well as file read/write.

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	AsciiString	m_scriptName;   ///<Short name.
	AsciiString m_comment;			///< Long comment.
	AsciiString m_conditionComment;			///< Long comment about conditions.
	AsciiString m_actionComment;			///< Long comment about actions.
	Int					m_delayEvaluationSeconds; ///< Number of seconds to delay evaluation.
	Bool				m_isActive;			 ///< If true, script is enabled.  If false, disabled.
	Bool				m_isOneShot;    ///< If true, script is made not active after executing the m_action actions.
	Bool				m_isSubroutine; ///< If true, script is a subroutine.
	Bool				m_easy;					///< Applies to easy campaign.
	Bool				m_normal;				///< Applies to normal campaign.
	Bool				m_hard;					///< Applies to Hard campaign.

	OrCondition *m_condition;		///< First in a list of ORed clauses.
	ScriptAction *m_action;			///< First in a list of actions executed if the conditions are true.
	ScriptAction *m_actionFalse;///< First in a list of actions executed if the conditions are false.
	Script			*m_nextScript;	///< Next in the list of scripts.

// Runtime fields - not saved or read.
	UnsignedInt m_frameToEvaluateAt; ///< When to evaluate the conditions next, if m_delayEvaluationSeconds>0.
	Bool				m_hasWarnings; ///< Runtime flag used by the editor only.
	AsciiString	m_conditionTeamName; ///< Runtime name used by ScriptEngine only.
	Real				m_conditionTime;		///< Amount of time (cum) to evaluate conditions.
	Real				m_curTime;		///< Amount of time (cum) to evaluate conditions.
	Int					m_conditionExecutedCount; ///< Number of times conditions evaluated.

public:
	Script();
	//~Script();
	Script *duplicate(void) const;	// note, duplicates just this node, not the full list.
	Script *duplicateAndQualify(const AsciiString& qualifier,
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const;

public:
	void setName(AsciiString name) { m_scriptName = name;}
	void setWarnings(Bool warnings) { m_hasWarnings = warnings;}
	void setComment(AsciiString comment) { m_comment = comment;}
	void setActionComment(AsciiString comment) { m_actionComment = comment;}
	void setConditionComment(AsciiString comment) { m_conditionComment = comment;}
	void setActive(Bool active) { m_isActive = active;}
	void setOneShot(Bool one) { m_isOneShot = one;}
	void setEasy(Bool easy) { m_easy = easy;}
	void setNormal(Bool normal) { m_normal = normal;}
	void setHard(Bool hard) { m_hard = hard;}
	void setSubroutine(Bool subr) { m_isSubroutine = subr;}
	void setNextScript(Script *pScr) {m_nextScript = pScr;}
	void setOrCondition(OrCondition *pCond) {m_condition = pCond;}
	void setAction(ScriptAction *pAction) {m_action = pAction;}
	void setFalseAction(ScriptAction *pAction) {m_actionFalse = pAction;}
	void updateFrom(Script *pSrc); ///< Updates this from pSrc.  pSrc IS MODIFIED - it's guts are removed.  jba.
	void setFrameToEvaluate(UnsignedInt frame) {m_frameToEvaluateAt=frame;}
	void incrementConditionCount(void) {m_conditionExecutedCount++;}
	void addToConditionTime(Real time) {m_conditionTime += time;}
	void setCurTime(Real time) {m_curTime	= time;}
	void setDelayEvalSeconds(Int delay) {m_delayEvaluationSeconds = delay;}

	UnsignedInt getFrameToEvaluate(void) {return m_frameToEvaluateAt;}
	Int getConditionCount(void) {return m_conditionExecutedCount;}
	Real getConditionTime(void) {return m_conditionTime;}
	Real getCurTime(void) {return m_curTime;}
	Int getDelayEvalSeconds(void) {return m_delayEvaluationSeconds;}

	AsciiString getName(void) const { return m_scriptName;}
	AsciiString getComment(void) const {return m_comment;}
	AsciiString getActionComment(void) const {return m_actionComment;}
	AsciiString getConditionComment(void) const {return m_conditionComment;}
	Bool hasWarnings(void) const { return m_hasWarnings;}
	Bool isActive(void) const { return m_isActive;}
	Bool isOneShot(void) const { return m_isOneShot;}
	Bool isEasy(void) const { return m_easy;}
	Bool isNormal(void) const { return m_normal;}
	Bool isHard(void) const { return m_hard;}
	Bool isSubroutine(void) const { return m_isSubroutine;}
	Script *getNext(void) const {return m_nextScript;}
	OrCondition *getOrCondition(void) const {return m_condition;}
	ScriptAction *getAction(void) const	{return m_action;}
	ScriptAction *getFalseAction(void) const {return m_actionFalse;}
	AsciiString getUiText(void);

	void deleteOrCondition(OrCondition *pCond);
	void deleteAction(ScriptAction *pAct);
	void deleteFalseAction(ScriptAction *pAct);

	static void WriteScriptDataChunk(DataChunkOutput &chunkWriter, Script *pScript);
	static Script *ParseScript(DataChunkInput &file, unsigned short version);
	static Bool ParseScriptFromListDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);
	static Bool ParseScriptFromGroupDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);

	// Support routines for moving conditions around
	OrCondition *findPreviousOrCondition( OrCondition *curOr );

	// Support routines for ScriptEngine -
	AsciiString getConditionTeamName(void) {return m_conditionTeamName;}
	void setConditionTeamName(AsciiString teamName) {m_conditionTeamName = teamName;}
};

//-------------------------------------------------------------------------------------------------
// ******************************** class Parameter ***********************************************
//-------------------------------------------------------------------------------------------------
/** This is a parameter, in either a condition or an action. */
class Parameter : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(Parameter, "Parameter")
// friend bad for MPOs. (srj)
//friend class EditParameter;
//friend class EditCoordParameter;
//friend class EditObjectParameter;
//friend class ScriptEngine;
//friend class ScriptConditions;

public:

	enum ParameterType
	{
		INT=0,							// Int.
		REAL,								// Real.
		SCRIPT,							// String.	Any script.
		TEAM,								// String.
		COUNTER,						// String, int index into counters array at runtime.
		FLAG,								// String, int index into flags array at runtime.
		COMPARISON,					// Int, see comparison types below.
		WAYPOINT,						// String.
		BOOLEAN,						// Int 0 or 1 for flags.
		TRIGGER_AREA,				// String.
		TEXT_STRING,				// String.
		SIDE,								// String.
		SOUND,							// String.
		SCRIPT_SUBROUTINE,	// String.  Script that is a subroutine.
		UNIT,								// String.  Name of a unit.
		OBJECT_TYPE,				// String.  Name of a thing template.
		COORD3D,						// String.  (x,y,z), ie (2.0,7,5).	Use getCoord()
		ANGLE,							// Real.  Radians.
		TEAM_STATE,					// String.  Name of a state that a team is in.
		RELATION,						// Int. Specifies the relation towards a team
		AI_MOOD,						// Int. Specifies the aggressiveness towards a team.
		DIALOG,							// String, specifies string usable with getSpeech(...)
		MUSIC,							// String, specifies string usable with getTrack(...)
		MOVIE,							// String, specifies string usable with open(...) or load(...)
		WAYPOINT_PATH,			// String, waypoint path label.
		LOCALIZED_TEXT,			// String, must be looked up from the localized text file.
		BRIDGE,							// String, Named bridge.
		KIND_OF_PARAM,			// Integer, KindOfType enum.
		ATTACK_PRIORITY_SET,// String, name of an attack priority set.
		RADAR_EVENT_TYPE,		// Int. Specifies the radar Event type
		SPECIAL_POWER,			// String, specifies superweapon types
		SCIENCE,						// String, specifies a science/general/faction/buzzword-of-the-week
		UPGRADE,						// String, specifies a type of upgrade
		COMMANDBUTTON_ABILITY, // Int, refers to the appropriate 1-12 commandbutton that is used for an object.
		BOUNDARY,						// Int, specifies a boundary index.
		BUILDABLE,					// Int, specifies buildability of a specific object.
		SURFACES_ALLOWED,		// Int, specifies whether the check should consider Air, Ground, or Air and Ground
		SHAKE_INTENSITY,		// Int, specifies which Intensity to use (Intensity in 10 cities!)
		COMMAND_BUTTON,  		// String
		FONT_NAME,       		// String, the name of the desired font
		OBJECT_STATUS,			// String, specifies ObjectStatusTypes name. However, translated to an ObjectStatusMaskType on read
		COMMANDBUTTON_ALL_ABILITIES, // String, refers to all command buttons
		SKIRMISH_WAYPOINT_PATH, // String, name of a predefined skirmish waypoint path.
		COLOR,							// color (as int) in ARGB format.
		EMOTICON,						// List of all possible emoticons.
		OBJECT_PANEL_FLAG,	// String, specifies a flag name available on the options panel.
		FACTION_NAME,				// The name of the faction (GLA, America, Munkee, China, etc)
		OBJECT_TYPE_LIST,		// String, Special case of Object Type.
		REVEALNAME,					// String, the name of the look taking place.
		SCIENCE_AVAILABILITY, // String, the name of the different science availabilities.
    LEFT_OR_RIGHT,        // 1=left, 2=right, okay?
		PERCENT,						// Real.  A percentage.
		NUM_ITEMS
	};

	enum { // Comparison types.  Stored in Int value.
		LESS_THAN = 0,
		LESS_EQUAL,
		EQUAL,
		GREATER_EQUAL,
		GREATER,
		NOT_EQUAL
	};

	enum { // Relation types. Stored in Int value. (Values from GameCommon.h)
		REL_ENEMY			= ENEMIES,
		REL_NEUTRAL		= NEUTRAL,
		REL_FRIEND		= ALLIES
	};

	Parameter(ParameterType type, int val = 0) :
		m_initialized(false),
		m_paramType(type),
		m_int(val),
		m_real(0)
	{
		m_coord.x=0;m_coord.y=0;m_coord.z=0;
	}

private:
	ParameterType	m_paramType;
	Bool					m_initialized;
	Int						m_int;
	Real					m_real;
	AsciiString		m_string;
	Coord3D				m_coord;
	ObjectStatusMaskType m_objectStatus;

protected:
	void setInt(Int i) {m_int = i;}
	void setReal(Real r) {m_real = r;}
	void setCoord3D(const Coord3D *pLoc);
	void setString(AsciiString s) {m_string = s;}
	void setStatus( ObjectStatusMaskType objectStatus ) { m_objectStatus.set( objectStatus ); }

public:
	Int getInt(void) const {return m_int;}
	Real getReal(void) const {return m_real;}
	void getCoord3D(Coord3D *pLoc) const;
	ParameterType getParameterType(void) const {return m_paramType;}
	ObjectStatusMaskType getStatus() const { return m_objectStatus; }

	void friend_setInt(Int i) {m_int = i;}
	void friend_setReal(Real r) {m_real = r;}
	void friend_setCoord3D(const Coord3D *pLoc) { setCoord3D(pLoc); }
	void friend_setString(AsciiString s) {m_string = s;}

	void qualify(const AsciiString& qualifier,const AsciiString& playerTemplateName,const AsciiString& newPlayerName);

	const AsciiString& getString(void) const {return m_string;}
	AsciiString getUiText(void) const;

	void WriteParameter(DataChunkOutput &chunkWriter);
	static Parameter *ReadParameter(DataChunkInput &file);

};
EMPTY_DTOR(Parameter)

extern const char* const TheObjectFlagsNames[];

//-------------------------------------------------------------------------------------------------
// ******************************** class Condition ***********************************************
//-------------------------------------------------------------------------------------------------
/** This is a condition.  The parameters for a ConditionType are set up in
ConditionTemplates created in ScriptEngine::init.

// SPECIAL NOTE ABOUT Skirmish Scripts: Please note that ALL Skirmish conditions should first pass a pSkirmishPlayerParm to
// prevet the necessity of having to write additional scripts for other players / skirmish types later.
*/

class Condition : public MemoryPoolObject  // This is the conditional class.
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(Condition, "Condition")
// friend bad for MPOs. (srj)
//friend class EditCondition;
public:
	enum ConditionType
	{
		CONDITION_FALSE,										// Always evaluates to false.
		COUNTER,															// COUNTER, COMPARISON, INT
		FLAG,																	// FLAG BOOLEAN compares flag to value.
		CONDITION_TRUE,												// Always evaluates to true.
		TIMER_EXPIRED,												// If a timer expired.
		PLAYER_ALL_DESTROYED,									// If a side (player) is totally destroyed.
		PLAYER_ALL_BUILDFACILITIES_DESTROYED, // If a side (player) has all build facilities destroyed.
		TEAM_INSIDE_AREA_PARTIALLY,						// If a player has units from a team entering an area.
		TEAM_DESTROYED,												// If a team has been destroyed.
		CAMERA_MOVEMENT_FINISHED,							// If the camera has completed it's current movement.
		TEAM_HAS_UNITS,												// If a team has units.
		TEAM_STATE_IS,												// If a team is in a particular state.
		TEAM_STATE_IS_NOT,										// If a team is not in a particular state.
		NAMED_INSIDE_AREA,										// If a player has units inside an area.
		NAMED_OUTSIDE_AREA,										// If a player has units outside an area.
		NAMED_DESTROYED,											// If a unit is destroyed
		NAMED_NOT_DESTROYED,									// If a unit is destroyed
		TEAM_INSIDE_AREA_ENTIRELY,						// If a team is entirely inside an area.
		TEAM_OUTSIDE_AREA_ENTIRELY,						// If a team is entirely outside an area.
		NAMED_ATTACKED_BY_OBJECTTYPE,					// If a unit has been attacked by an object of type objecttype
		TEAM_ATTACKED_BY_OBJECTTYPE,					// If a team has been attacked by an object of type objecttype
		NAMED_ATTACKED_BY_PLAYER,							// If a unit has been attacked by a unit controlled by player
		TEAM_ATTACKED_BY_PLAYER,							// If a team has been attacked by a unit controlled by player
		BUILT_BY_PLAYER,											// If a unit of type objecttype has been built by player
		NAMED_CREATED,												// If a unit is currently in existence
		TEAM_CREATED,													// If a team is currently in existence
		PLAYER_HAS_CREDITS,										// If X [Less Than | Equal | More than] Players credits
		NAMED_DISCOVERED,											// If a named unit is currently "seen" by a player.
		TEAM_DISCOVERED,											// If a unit in a team is currently "seen" by a player.
		MISSION_ATTEMPTS,							// Impl JKMCD
		NAMED_OWNED_BY_PLAYER,								// If a unit is currently owned by a player
		TEAM_OWNED_BY_PLAYER,									// If a team is currently owned by a player
		PLAYER_HAS_N_OR_FEWER_BUILDINGS,			// If a player has count or fewer buildings
		PLAYER_HAS_POWER,											// If a player currently has power to his facilities
		NAMED_REACHED_WAYPOINTS_END,					// If a unit has reached the end of the path that contains Waypoint
		TEAM_REACHED_WAYPOINTS_END,						// If a team has reached the end of the path that contains Waypoint
		NAMED_SELECTED,												// If a unit is selected
		NAMED_ENTERED_AREA,										// If a unit enters an area
		NAMED_EXITED_AREA,										// If a unit exits an area
		TEAM_ENTERED_AREA_ENTIRELY,						// If a team has entirely entered an area
		TEAM_ENTERED_AREA_PARTIALLY,					// If a team has exactly one unit entering an area
		TEAM_EXITED_AREA_ENTIRELY,						// If a team has entirely exited an area
		TEAM_EXITED_AREA_PARTIALLY,						// If a team has exactly one unit exiting an area
		MULTIPLAYER_ALLIED_VICTORY,						// If a single group of allies has vanquished everyone else
		MULTIPLAYER_ALLIED_DEFEAT,						// If a single group of allies has been vanquished
		MULTIPLAYER_PLAYER_DEFEAT,						// If a player (but not necessarily his allies) has been defeated
		PLAYER_HAS_NO_POWER,									// If a player doesn't have power
		HAS_FINISHED_VIDEO,										// If a video has completed playing
		HAS_FINISHED_SPEECH,									// If a speech has completed playing
		HAS_FINISHED_AUDIO,										// If an audio piece has completed playing
		BUILDING_ENTERED_BY_PLAYER,						// If a player has garrisoned a building.
		ENEMY_SIGHTED,												// If a unit sees any type of unit/building belonging to a player.
		UNIT_HEALTH,													// Compares the unit health
		BRIDGE_REPAIRED,											// If a bridge has been repaired.
		BRIDGE_BROKEN,												// If a bridge has been broken.
		NAMED_DYING,													// A unit has been killed, but is in the process of dying (using slow death state)
		NAMED_TOTALLY_DEAD,										// A unit is done dying.
		PLAYER_HAS_OBJECT_COMPARISON,					// Does a player have more/less than X number of units of type object
		OBSOLETE_SCRIPT_1,										// Obsolete!
		OBSOLETE_SCRIPT_2,										// Obsolete!
		PLAYER_TRIGGERED_SPECIAL_POWER,				// has a player triggered a specific special power type
		PLAYER_COMPLETED_SPECIAL_POWER,				// has a player's special power completed?
		PLAYER_MIDWAY_SPECIAL_POWER,					// If the special power is midway through activation
		PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED,	// has a player triggered a specific special power type
		PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED,	// has a player's special power completed?
		PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED,			// If the special power is midway through activation
		DEFUNCT_PLAYER_SELECTED_GENERAL,							// no longer in use!
		DEFUNCT_PLAYER_SELECTED_GENERAL_FROM_NAMED,		// no longer in use!
		PLAYER_BUILT_UPGRADE,									// If a player built a specific upgrade
		PLAYER_BUILT_UPGRADE_FROM_NAMED,			// If a player built a specific upgrade from a specific building
		PLAYER_DESTROYED_N_BUILDINGS_PLAYER,	// If a player has destroyed N or more buildings of a specific player
		UNIT_COMPLETED_SEQUENTIAL_EXECUTION,	// If a unit has completed sequential execution
		TEAM_COMPLETED_SEQUENTIAL_EXECUTION,	// If a team has completed sequential execution
		PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA, // If player has a specific unit or building type...
		PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA, // If player has a kindof in a trigger area.
		UNIT_EMPTIED,													// True if a unit has just emptied its cargo
		TYPE_SIGHTED,														// True if a specific unit has just seen a type of unit belonging to a specific player
		NAMED_BUILDING_IS_EMPTY,								// True if a specific building is empty
		PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS,			// If a player has count or fewer faction buildings
		UNIT_HAS_OBJECT_STATUS,									// True if an object has that object status bit set to true
		TEAM_ALL_HAS_OBJECT_STATUS,							// True if all objects on a team have that object status bit set to true
		TEAM_SOME_HAVE_OBJECT_STATUS,						// True if an object on the team has that object status bit set to true
		PLAYER_POWER_COMPARE_PERCENT,						// True if a player's power ratio compares to a percentage.
		PLAYER_EXCESS_POWER_COMPARE_VALUE,			// True if a player has comparison  power in kilowatts.
		SKIRMISH_SPECIAL_POWER_READY,						// True if the special power is ready to fire.
		SKIRMISH_VALUE_IN_AREA,									// True if the units in an area are worth comparison to a value
		SKIRMISH_PLAYER_FACTION,								// True if player is a specific faction
		SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE,// True if there are supplies within distance of the perimeter
		SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE,	// True if there is a tech building within distance of the perimeter
		SKIRMISH_COMMAND_BUTTON_READY_ALL,			// True when ???
		SKIRMISH_COMMAND_BUTTON_READY_PARTIAL,	// True when ???
		SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS,		// True if there are comparison to unowned faction unit on the map.
		SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD,	// True if we can build this thing (we have prerequisites)
		SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED,	// True if there are comparison to garrisoned buildings (by player) on the map
		SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS,	// True if there are comparison to captured units
		SKIRMISH_NAMED_AREA_EXIST,							// True if a named area exists
		SKIRMISH_PLAYER_HAS_UNITS_IN_AREA,			// True if a player has units in an area
		SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER,	// True if the player has been attacked by a player
		SKIRMISH_PLAYER_IS_OUTSIDE_AREA,				// True if player doesn't have any units in an area
		SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER,	// True if a player has discovered another player

		PLAYER_ACQUIRED_SCIENCE,								// True if the player has acquired the specified Science.
		PLAYER_HAS_SCIENCEPURCHASEPOINTS,				// True if the player has at least N SciencePurchasePoints.
		PLAYER_CAN_PURCHASE_SCIENCE,						// True is player can purchase the given science (has all prereqs & points needed)
		MUSIC_TRACK_HAS_COMPLETED,							// True if the specified track has completed at least N times
		PLAYER_LOST_OBJECT_TYPE,								// True if a player has lost a specific unit type

		SUPPLY_SOURCE_SAFE,											// True if the nearest available supply source is not under enemy influence.
		SUPPLY_SOURCE_ATTACKED,									// True if our supply depot or dozer near depot was attacked.
		START_POSITION_IS,											// True if our start position matches.
		NAMED_HAS_FREE_CONTAINER_SLOTS,					///< Kris -- Checks if any given container has any free slots.

		NUM_ITEMS		 // Always the last condition.
	};

	Condition(enum ConditionType type);
	//~Condition();

	Condition *duplicate(void) const;
	Condition *duplicateAndQualify(const AsciiString& qualifier,
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName) const;

protected:
	Condition();  ///< Protected constructor for read.

private:
	enum ConditionType	m_conditionType;
	Int 		m_numParms;
	Parameter *m_parms[MAX_PARMS];
	Condition *m_nextAndCondition;

	Int				m_hasWarnings; ///< Runtime flag used by the editor only.
	Int				m_customData;  ///< Custom data for cacheing.
	UnsignedInt m_customFrame; ///< Custom frame count for cacheing.

public:
	void setConditionType(enum ConditionType type);

public:
	void setNextCondition(Condition *pScr) {m_nextAndCondition = pScr;}
	void setWarnings(Bool warnings) { m_hasWarnings = warnings;}
	enum ConditionType getConditionType(void) {return m_conditionType;}
	Condition * getNext(void) {return m_nextAndCondition;}
	AsciiString getUiText(void);
	Parameter *getParameter(Int ndx)
	{
		if (ndx>=0 && ndx<m_numParms)
			return m_parms[ndx];

		return NULL;
	}

	Int getNumParameters(void) {return m_numParms;}
	Int getUiStrings(AsciiString strings[MAX_PARMS]);
	Bool hasWarnings(void) const { return m_hasWarnings;}
	Int getCustomData(void) const {return m_customData;}
	void setCustomData(Int val) { m_customData = val;}

	Int getCustomFrame(void) const {return m_customFrame;}
	void setCustomFrame(Int val) { m_customFrame = val;}

	static void WriteConditionDataChunk(DataChunkOutput &chunkWriter, Condition *pCond);
	static Bool ParseConditionDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);

};

#define dontCOUNT_SCRIPT_USAGE

//-------------------------------------------------------------------------------------------------
// ******************************** class Template ***********************************************
//-------------------------------------------------------------------------------------------------
/** The template defines the parameters and ui strings used to display a
condition or action. */
class Template : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(Template, "Template")
// friend bad for MPOs. (srj)
//friend class ScriptEngine;
public:
	AsciiString m_uiName;
	AsciiString m_uiName2;
	AsciiString m_internalName;
	NameKeyType m_internalNameKey; // matches internal name.jba [3/20/2003]
	Int					m_numUiStrings;
	AsciiString m_uiStrings[MAX_PARMS];
	Int					m_numParameters;
	enum Parameter::ParameterType m_parameters[MAX_PARMS];
	AsciiString m_helpText;
#ifdef COUNT_SCRIPT_USAGE
	mutable Int					m_numTimesUsed;
	mutable AsciiString m_firstMapUsed;
#endif

public:
	Template();

public:
	AsciiString getName(void) const {return m_uiName;}
	AsciiString getName2(void) const {return m_uiName2;}
	Int getUiStrings(AsciiString strings[MAX_PARMS]) const;
	Int getNumParameters(void) const {return m_numParameters;}
	enum Parameter::ParameterType getParameterType(Int ndx) const;
};
EMPTY_DTOR(Template)

//-------------------------------------------------------------------------------------------------
// ******************************** class ConditionTemplate ***********************************************
//-------------------------------------------------------------------------------------------------
/// Template for condition.
class ConditionTemplate : public Template{};

//-------------------------------------------------------------------------------------------------
// ******************************** class ActionTemplate ***********************************************
//-------------------------------------------------------------------------------------------------
/// Template for action.
class ActionTemplate : public Template{};

//-------------------------------------------------------------------------------------------------
// ******************************** class ScriptList ***********************************************
//-------------------------------------------------------------------------------------------------
/// List of scripts.  May have one level of hierarchy in the ScriptGroup list.
class ScriptList : public MemoryPoolObject, public Snapshot
// A script list belongs to a Player.  It contains the heads of the
// the top level scripts, and the script groups.
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ScriptList, "ScriptList")
protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	ScriptGroup		*m_firstGroup;
	Script				*m_firstScript;
	static Int		m_curId;

	static ScriptList *s_readLists[MAX_PLAYER_COUNT];
	static Int				s_numInReadList;

public:
	ScriptList();
	//~ScriptList();

public:
	static void updateDefaults(void);
	static void reset(void);
	static Int getNextID(void) {m_curId++; return m_curId;};

public:
	ScriptGroup *getScriptGroup(void) {return m_firstGroup;};
	Script *getScript(void) {return m_firstScript;};
	void WriteScriptListDataChunk(DataChunkOutput &chunkWriter);
	static Bool ParseScriptListDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);

	void addGroup(ScriptGroup *pGrp, Int ndx);
	void addScript(Script *pScr, Int ndx);
	void deleteScript(Script *pScr);
	void deleteGroup(ScriptGroup *pGrp);

	void discard(void);

	ScriptList *duplicate(void) const;
	ScriptList *duplicateAndQualify(const AsciiString& qualifier,
			const AsciiString& playerTemplateName, const AsciiString& newPlayerName ) const;
	/// Reads a set of scripts into m_readScripts.  Use getReadScripts to access.
	static Bool ParseScriptsDataChunk(DataChunkInput &file, DataChunkInfo *info, void *userData);
	/// Writes sides (including build list info.)
	static void WriteScriptsDataChunk(DataChunkOutput &chunkWriter, ScriptList *scriptLists[], Int numLists);

	/// Returns array of script list pointers.  This can only be called once after scripts
	/// are read, and the caller is responsible for deleting the scripts.
	static Int getReadScripts(ScriptList *scriptLists[MAX_PLAYER_COUNT]);
};

#endif

