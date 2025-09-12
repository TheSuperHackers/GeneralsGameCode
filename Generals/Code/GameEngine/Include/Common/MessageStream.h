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

// FILE: MessageStream.h //////////////////////////////////////////////////////
// The message stream propagates all messages thru a series of "translators"
// Author: Michael S. Booth, February 2001

#pragma once

#ifndef _MESSAGE_STREAM_H_
#define _MESSAGE_STREAM_H_

#include "Common/GameCommon.h"	// ensure we get DUMP_PERF_STATS, or not
#include "Common/SubsystemInterface.h"
#include "Lib/BaseType.h"
#include "Common/GameMemory.h"


enum { TRANSLATOR_ID_INVALID = -1 };

// how far the the cursor moves before a click becomes a drag


typedef UnsignedInt TranslatorID;								///< Unique identifiers for message stream translators

class Drawable;
class GameMessageList;
enum ObjectID CPP_11(: Int);
enum DrawableID CPP_11(: Int);

union GameMessageArgumentType														///< Union of possible data for given message type
{
	Int							integer;
	Real 						real;
	Bool						boolean;
	ObjectID				objectID;
	DrawableID			drawableID;
	UnsignedInt			teamID;
	UnsignedInt			squadID;
	Coord3D					location;
	ICoord2D				pixel;
	IRegion2D				pixelRegion;
	UnsignedInt			timestamp;
	WideChar				wChar;
};

enum GameMessageArgumentDataType CPP_11(: Int)
{
	ARGUMENTDATATYPE_INTEGER,
	ARGUMENTDATATYPE_REAL,
	ARGUMENTDATATYPE_BOOLEAN,
	ARGUMENTDATATYPE_OBJECTID,
	ARGUMENTDATATYPE_DRAWABLEID,
	ARGUMENTDATATYPE_TEAMID,
	ARGUMENTDATATYPE_LOCATION,
	ARGUMENTDATATYPE_PIXEL,
	ARGUMENTDATATYPE_PIXELREGION,
	ARGUMENTDATATYPE_TIMESTAMP,
	ARGUMENTDATATYPE_WIDECHAR,
	ARGUMENTDATATYPE_UNKNOWN
};

class GameMessageArgument : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(GameMessageArgument, "GameMessageArgument")
public:
	GameMessageArgument*				m_next;									///< The next argument
	GameMessageArgumentType			m_data;									///< The data storage of an argument
	GameMessageArgumentDataType	m_type;									///< The type of the argument.
};
EMPTY_DTOR(GameMessageArgument)

/**
 * A game message that either lives on TheMessageStream or TheCommandList.
 * Messages consist of a type, defining what the message is, and zero or more arguments
 * of various data types.  The user of a message must know how many and what type of
 * arguments are valid for a given message type.
 */
class GameMessage : public MemoryPoolObject
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(GameMessage, "GameMessage")

public:

	/// The various messages which can be sent in a MessageStream
	/// @todo Replace this hardcoded enum with a generalized system that can be easily changed and updated
	/** @todo Because the Client will run faster than Logic, we'll need "superceding" messages for events
						such as mouse movements so we only send the latest one over the net */
	/**	@todo Create two classes of message: raw input messages, and command messages. Raw input messages
						will be destroyed when they reach the end of the stream, whereas command messages will be
						transferred to TheCommandList */
	enum Type
	{
		MSG_INVALID,																///< (none) this msg should never actually occur

		MSG_FRAME_TICK,															///< (timestamp) once each frame this message is sent thru the stream

		// Client to Server messages
		// Note: Please keep MSG_RAW_MOUSE between MSG_RAW_MOUSE_BEGIN and MSG_RAW_MOUSE_END
		MSG_RAW_MOUSE_BEGIN,
		MSG_RAW_MOUSE_POSITION,											///< (pixel) the cursor's position
		MSG_RAW_MOUSE_LEFT_BUTTON_DOWN,							///< (pixel, modifiers, time)
		MSG_RAW_MOUSE_LEFT_DOUBLE_CLICK,						///< (pixel, modifiers, time)
		MSG_RAW_MOUSE_LEFT_BUTTON_UP,								///< (pixel, modifiers, time)
		MSG_RAW_MOUSE_LEFT_CLICK,										///< (pixel, modifiers, time)
		MSG_RAW_MOUSE_LEFT_DRAG,										///< drag of the mouse with a button held down
		MSG_RAW_MOUSE_MIDDLE_BUTTON_DOWN,						///< (pixel, modifiers, time)
		MSG_RAW_MOUSE_MIDDLE_DOUBLE_CLICK,					///< (pixel, modifiers, time)
		MSG_RAW_MOUSE_MIDDLE_BUTTON_UP,							///< (pixel, modifiers, time)
		MSG_RAW_MOUSE_MIDDLE_DRAG,									///< drag of the mouse with a button held down
		MSG_RAW_MOUSE_RIGHT_BUTTON_DOWN,						///< (pixel, modifiers, time)
		MSG_RAW_MOUSE_RIGHT_DOUBLE_CLICK,						///< (pixel, modifiers, time)
		MSG_RAW_MOUSE_RIGHT_BUTTON_UP,							///< (pixel, modifiers, time)
		MSG_RAW_MOUSE_RIGHT_DRAG,										///< drag of the mouse with a button held down
		MSG_RAW_MOUSE_WHEEL,												///< (Int spin, + is away, - is toward user)
		MSG_RAW_MOUSE_END,

		MSG_RAW_KEY_DOWN,														///< (KeyDefType) the given key was pressed (uses Microsoft VK_ codes)
		MSG_RAW_KEY_UP,															///< (KeyDefType) the given key was released

		// Refined Mouse messages
		// NOTE: All processing should attempt to use these refined mouse messages, rather than the
		// RAW_* variants. (Please.) :-) jkmcd
		MSG_MOUSE_LEFT_CLICK,												///< (pixelRegion, 0 sized means its a point), (Int, modifier keys)
		MSG_MOUSE_LEFT_DOUBLE_CLICK,								///< (pixelRegion, 0 sized means its a point), (Int, modifier keys)
		MSG_MOUSE_MIDDLE_CLICK,											///< (pixelRegion, 0 sized means its a point), (Int, modifier keys)
		MSG_MOUSE_MIDDLE_DOUBLE_CLICK,							///< (pixelRegion, 0 sized means its a point), (Int, modifier keys)
		MSG_MOUSE_RIGHT_CLICK,											///< (pixelRegion, 0 sized means its a point), (Int, modifier keys)
		MSG_MOUSE_RIGHT_DOUBLE_CLICK,								///< (pixelRegion, 0 sized means its a point), (Int, modifier keys)
		// End Refined Mouse Messages

		MSG_CLEAR_GAME_DATA,												///< Clear all game data in memory
		MSG_NEW_GAME,																///< Start a new game

		// "meta" messages should be thought of as "virtual keystrokes" -- they exist
		// solely to provide an abstraction layer useful for keyboard/mouse remapping.
		// they should NEVER be sent over the network.
		MSG_BEGIN_META_MESSAGES,										///< Marker to delineate "meta" messages

		MSG_META_SAVE_VIEW1,												///< save current view as the given user-defined view
		MSG_META_SAVE_VIEW2,												///< save current view as the given user-defined view
		MSG_META_SAVE_VIEW3,												///< save current view as the given user-defined view
		MSG_META_SAVE_VIEW4,												///< save current view as the given user-defined view
		MSG_META_SAVE_VIEW5,												///< save current view as the given user-defined view
		MSG_META_SAVE_VIEW6,												///< save current view as the given user-defined view
		MSG_META_SAVE_VIEW7,												///< save current view as the given user-defined view
		MSG_META_SAVE_VIEW8,												///< save current view as the given user-defined view
		MSG_META_VIEW_VIEW1,												///< center view on the given user-defined view
		MSG_META_VIEW_VIEW2,												///< center view on the given user-defined view
		MSG_META_VIEW_VIEW3,												///< center view on the given user-defined view
		MSG_META_VIEW_VIEW4,												///< center view on the given user-defined view
		MSG_META_VIEW_VIEW5,												///< center view on the given user-defined view
		MSG_META_VIEW_VIEW6,												///< center view on the given user-defined view
		MSG_META_VIEW_VIEW7,												///< center view on the given user-defined view
		MSG_META_VIEW_VIEW8,												///< center view on the given user-defined view
		MSG_META_CREATE_TEAM0,											///< create user-defined team from the selected objects
		MSG_META_CREATE_TEAM1,											///< create user-defined team from the selected objects
		MSG_META_CREATE_TEAM2,											///< create user-defined team from the selected objects
		MSG_META_CREATE_TEAM3,											///< create user-defined team from the selected objects
		MSG_META_CREATE_TEAM4,											///< create user-defined team from the selected objects
		MSG_META_CREATE_TEAM5,											///< create user-defined team from the selected objects
		MSG_META_CREATE_TEAM6,											///< create user-defined team from the selected objects
		MSG_META_CREATE_TEAM7,											///< create user-defined team from the selected objects
		MSG_META_CREATE_TEAM8,											///< create user-defined team from the selected objects
		MSG_META_CREATE_TEAM9,											///< create user-defined team from the selected objects
		MSG_META_SELECT_TEAM0,											///< deselect all, then select all units in the given user-defined team
		MSG_META_SELECT_TEAM1,											///< deselect all, then select all units in the given user-defined team
		MSG_META_SELECT_TEAM2,											///< deselect all, then select all units in the given user-defined team
		MSG_META_SELECT_TEAM3,											///< deselect all, then select all units in the given user-defined team
		MSG_META_SELECT_TEAM4,											///< deselect all, then select all units in the given user-defined team
		MSG_META_SELECT_TEAM5,											///< deselect all, then select all units in the given user-defined team
		MSG_META_SELECT_TEAM6,											///< deselect all, then select all units in the given user-defined team
		MSG_META_SELECT_TEAM7,											///< deselect all, then select all units in the given user-defined team
		MSG_META_SELECT_TEAM8,											///< deselect all, then select all units in the given user-defined team
		MSG_META_SELECT_TEAM9,											///< deselect all, then select all units in the given user-defined team
		MSG_META_ADD_TEAM0,													///< add the user-defined team to the current selection
		MSG_META_ADD_TEAM1,													///< add the user-defined team to the current selection
		MSG_META_ADD_TEAM2,													///< add the user-defined team to the current selection
		MSG_META_ADD_TEAM3,													///< add the user-defined team to the current selection
		MSG_META_ADD_TEAM4,													///< add the user-defined team to the current selection
		MSG_META_ADD_TEAM5,													///< add the user-defined team to the current selection
		MSG_META_ADD_TEAM6,													///< add the user-defined team to the current selection
		MSG_META_ADD_TEAM7,													///< add the user-defined team to the current selection
		MSG_META_ADD_TEAM8,													///< add the user-defined team to the current selection
		MSG_META_ADD_TEAM9,													///< add the user-defined team to the current selection
		MSG_META_VIEW_TEAM0,												///< center view on given user-defined team (but do not affect selection)
		MSG_META_VIEW_TEAM1,												///< center view on given user-defined team (but do not affect selection)
		MSG_META_VIEW_TEAM2,												///< center view on given user-defined team (but do not affect selection)
		MSG_META_VIEW_TEAM3,												///< center view on given user-defined team (but do not affect selection)
		MSG_META_VIEW_TEAM4,												///< center view on given user-defined team (but do not affect selection)
		MSG_META_VIEW_TEAM5,												///< center view on given user-defined team (but do not affect selection)
		MSG_META_VIEW_TEAM6,												///< center view on given user-defined team (but do not affect selection)
		MSG_META_VIEW_TEAM7,												///< center view on given user-defined team (but do not affect selection)
		MSG_META_VIEW_TEAM8,												///< center view on given user-defined team (but do not affect selection)
		MSG_META_VIEW_TEAM9,												///< center view on given user-defined team (but do not affect selection)

		MSG_META_SELECT_MATCHING_UNITS,              ///< selects mathcing units, used for both on screen and across map
		MSG_META_SELECT_NEXT_UNIT,									///< select 'next' unit
		MSG_META_SELECT_PREV_UNIT,									///< select 'prev' unit
		MSG_META_SELECT_NEXT_WORKER,                ///< select 'next' worker
		MSG_META_SELECT_PREV_WORKER,                ///< select 'prev' worker
		MSG_META_SELECT_NEXT_IDLE_WORKER,                        ///< TheSuperHackers @feature L3-M 03/08/2025 select next idle worker
		MSG_META_VIEW_COMMAND_CENTER,								///< center view on command center
		MSG_META_VIEW_LAST_RADAR_EVENT,							///< center view on last radar event
		MSG_META_SELECT_HERO,                       ///< selects player's hero character, if exists...
		MSG_META_SELECT_ALL,                        ///< selects all units across screen
		MSG_META_SELECT_ALL_AIRCRAFT,								///< selects all air units just like select all
		MSG_META_SCATTER,														///< selected units scatter
		MSG_META_STOP,															///< selected units stop
		MSG_META_DEPLOY,														///< selected units 'deploy'
		MSG_META_CREATE_FORMATION,									///< selected units become a formation
		MSG_META_FOLLOW,														///< selected units 'follow'
		MSG_META_CHAT_PLAYERS,											///< send chat msg to all players
		MSG_META_CHAT_ALLIES,												///< send chat msg to allied players
		MSG_META_CHAT_EVERYONE,											///< send chat msg to everyone (incl. observers)
		MSG_META_DIPLOMACY,													///< bring up diplomacy screen
		MSG_META_OPTIONS,														///< bring up options screen
#if defined(RTS_DEBUG)
		MSG_META_HELP,															///< bring up help screen
#endif

		MSG_META_INCREASE_MAX_RENDER_FPS,						///< TheSuperHackers @feature Increase the max render fps
		MSG_META_DECREASE_MAX_RENDER_FPS,						///< TheSuperHackers @feature Decrease the max render fps
		MSG_META_INCREASE_LOGIC_TIME_SCALE,					///< TheSuperHackers @feature Increase the logic time scale
		MSG_META_DECREASE_LOGIC_TIME_SCALE,					///< TheSuperHackers @feature Decrease the logic time scale
		MSG_META_TOGGLE_LOWER_DETAILS,							///< toggles graphics options to crappy mode instantly
		MSG_META_TOGGLE_CONTROL_BAR,								///< show/hide controlbar

		MSG_META_BEGIN_PATH_BUILD,									///< enter path-building mode
		MSG_META_END_PATH_BUILD,										///< exit path-building mode
		MSG_META_BEGIN_FORCEATTACK,									///< enter force-attack mode
		MSG_META_END_FORCEATTACK,										///< exit force-attack mode
		MSG_META_BEGIN_FORCEMOVE,										///< enter force-move mode
		MSG_META_END_FORCEMOVE,											///< exit force-move mode
		MSG_META_BEGIN_WAYPOINTS,										///< enter waypoint mode
		MSG_META_END_WAYPOINTS,											///< exit waypoint mode
		MSG_META_BEGIN_PREFER_SELECTION,						///< The Shift key has been depressed alone
		MSG_META_END_PREFER_SELECTION,							///< The Shift key has been released.

		MSG_META_TAKE_SCREENSHOT,										///< take screenshot
		MSG_META_ALL_CHEER,													///< Yay! :)
		MSG_META_TOGGLE_ATTACKMOVE,									///< enter attack-move mode

		MSG_META_BEGIN_CAMERA_ROTATE_LEFT,
		MSG_META_END_CAMERA_ROTATE_LEFT,
		MSG_META_BEGIN_CAMERA_ROTATE_RIGHT,
		MSG_META_END_CAMERA_ROTATE_RIGHT,
		MSG_META_BEGIN_CAMERA_ZOOM_IN,
		MSG_META_END_CAMERA_ZOOM_IN,
		MSG_META_BEGIN_CAMERA_ZOOM_OUT,
		MSG_META_END_CAMERA_ZOOM_OUT,
		MSG_META_CAMERA_RESET,
		MSG_META_TOGGLE_FAST_FORWARD_REPLAY,				///< Toggle the fast forward feature
		MSG_META_TOGGLE_PAUSE,											///< TheSuperHackers @feature Toggle game pause
		MSG_META_TOGGLE_PAUSE_ALT,									///< TheSuperHackers @feature Toggle game pause (alternative mapping)
		MSG_META_STEP_FRAME,												///< TheSuperHackers @feature Step one frame
		MSG_META_STEP_FRAME_ALT,										///< TheSuperHackers @feature Step one frame (alternative mapping)


		// META items that are really for debug/demo/development use only...
		// They do not get built into RELEASE builds.
#if defined(RTS_DEBUG)
		MSG_META_DEMO_TOGGLE_BEHIND_BUILDINGS,			///< Toggles showing units behind buildings or not
		MSG_META_DEMO_TOGGLE_LETTERBOX,							///< enable/disable letterbox mode
		MSG_META_DEMO_TOGGLE_MESSAGE_TEXT,					///< toggle the text from the UI messages
		MSG_META_DEMO_LOD_DECREASE,                 ///< decrease LOD by 1
		MSG_META_DEMO_LOD_INCREASE,                 ///< increase LOD by 1
		MSG_META_DEMO_TOGGLE_ZOOM_LOCK,							///< Toggle the camera zoom lock on/off
		MSG_META_DEMO_PLAY_CAMEO_MOVIE,							///< Play a movie in the cameo spot
		MSG_META_DEMO_INSTANT_QUIT,									///< bail out of game immediately
		MSG_META_DEMO_TOGGLE_SPECIAL_POWER_DELAYS,	///< Toggle special power delays on/off
		MSG_META_DEMO_BATTLE_CRY,										///< battle cry
		MSG_META_DEMO_SWITCH_TEAMS,									///< switch local control to another team
		MSG_META_DEMO_SWITCH_TEAMS_BETWEEN_CHINA_USA, ///< switch the local player between china and usa
		MSG_META_DEMO_TOGGLE_PARTICLEDEBUG,					///< show/hide the particle system debug info
		MSG_META_DEMO_TOGGLE_SHADOW_VOLUMES,				///< show/hide shadow volumes
		MSG_META_DEMO_TOGGLE_FOGOFWAR,
		MSG_META_DEMO_KILL_ALL_ENEMIES,							///< kill ALL ENEMIES! (yeah!)
		MSG_META_DEMO_KILL_SELECTION,								///< kill the selected units (yeah!)
		MSG_META_DEMO_TOGGLE_HURT_ME_MODE,					///< do 10% damage to the selected units (yeah!)
		MSG_META_DEMO_TOGGLE_HAND_OF_GOD_MODE,			///< do 100% damage to the selected units (w00t!)
		MSG_META_DEMO_DEBUG_SELECTION,							///< select a given unit for state-machine debugging
		MSG_META_DEMO_LOCK_CAMERA_TO_SELECTION,			///< lock the view camera to the selected object
		MSG_META_DEMO_TOGGLE_SOUND,									///< toggle sound/video on/off
		MSG_META_DEMO_TOGGLE_TRACKMARKS,						///< toggle tank tread marks on/off
		MSG_META_DEMO_TOGGLE_WATERPLANE,						///< toggle waterplane on/off
		MSG_META_DEMO_TIME_OF_DAY,									///< change time-of-day lighting
		MSG_META_DEMO_TOGGLE_MUSIC,									///< turn background music on/off
		MSG_META_DEMO_MUSIC_NEXT_TRACK,							///< play next track
		MSG_META_DEMO_MUSIC_PREV_TRACK,							///< play prev track
		MSG_META_DEMO_NEXT_OBJECTIVE_MOVIE,					///< play next "Objective" movie
		MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE1,				///< play specific "Objective" movie
		MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE2,				///< play specific "Objective" movie
		MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE3,				///< play specific "Objective" movie
		MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE4,				///< play specific "Objective" movie
		MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE5,				///< play specific "Objective" movie
		MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE6,				///< play specific "Objective" movie
		MSG_META_DEMO_BEGIN_ADJUST_PITCH,						///< enter adjust-pitch mode
		MSG_META_DEMO_END_ADJUST_PITCH,							///< exit adjust-pitch mode
		MSG_META_DEMO_BEGIN_ADJUST_FOV,							///< enter adjust-FOV mode
		MSG_META_DEMO_END_ADJUST_FOV,								///< exit adjust-FOV mode
		MSG_META_DEMO_LOCK_CAMERA_TO_PLANES,				///< lock camera to airborne thingies
		MSG_META_DEMO_REMOVE_PREREQ,								///< Turn off Prerequisite checks in building legality, works in MULTIPLAYER for all humans
		MSG_META_DEMO_INSTANT_BUILD,								///< All building is with a timer of 1, works in MULTIPLAYER for all humans
		MSG_META_DEMO_FREE_BUILD,										///< All building is for 0 money, works in MULTIPLAYER for all humans
		MSG_META_DEMO_RUNSCRIPT1,										///< run script named "KEY_F1"
		MSG_META_DEMO_RUNSCRIPT2,										///< run script named "KEY_F2"
		MSG_META_DEMO_RUNSCRIPT3,										///< run script named "KEY_F3"
		MSG_META_DEMO_RUNSCRIPT4,										///< run script named "KEY_F4"
		MSG_META_DEMO_RUNSCRIPT5,										///< run script named "KEY_F5"
		MSG_META_DEMO_RUNSCRIPT6,										///< run script named "KEY_F6"
		MSG_META_DEMO_RUNSCRIPT7,										///< run script named "KEY_F7"
		MSG_META_DEMO_RUNSCRIPT8,										///< run script named "KEY_F8"
		MSG_META_DEMO_RUNSCRIPT9,										///< run script named "KEY_F9"
		MSG_META_DEMO_ENSHROUD,											///< re-shroud the world for the local player
		MSG_META_DEMO_DESHROUD,											///< de-shroud the world for the local player
		MSG_META_DEBUG_SHOW_EXTENTS,								///< show object extents
		MSG_META_DEBUG_SHOW_HEALTH,									///< show object health
		MSG_META_DEBUG_GIVE_VETERANCY,							///< give a veterancy level to selected objects
		MSG_META_DEBUG_TAKE_VETERANCY,							///< take a veterancy level from selected objects
		MSG_META_DEMO_TOGGLE_AI_DEBUG,							///< show/hide the ai debug stats
		MSG_META_DEMO_TOGGLE_CAMERA_DEBUG,					///< show/hide the camera debug stats
		MSG_META_DEMO_TOGGLE_AVI,										///< start capturing video
		MSG_META_DEMO_TOGGLE_BW_VIEW,								///< enable/disable black & white camera mode
		MSG_META_DEMO_TOGGLE_RED_VIEW,							///< enable/disable red tinted view
		MSG_META_DEMO_TOGGLE_GREEN_VIEW,						///< enable/disable green view
		MSG_META_DEMO_TOGGLE_MOTION_BLUR_ZOOM,			///< enable/disable green view
		MSG_META_DEMO_TOGGLE_MILITARY_SUBTITLES,		///< enable/disable military subtitles
		MSG_META_DEMO_ADD_CASH,											///< adds 10000 cash to the player
#ifdef ALLOW_SURRENDER
		MSG_META_DEMO_TEST_SURRENDER,							  ///< Test key to show surrender animation in game.
#endif
		MSG_META_DEMO_TOGGLE_RENDER,								///< toggle rendering on/off
		MSG_META_DEMO_KILL_AREA_SELECTION,					///< (teamID, objectID1, objectID2, ... objectIDN)
		MSG_META_DEMO_CYCLE_LOD_LEVEL,						///< cycles through dynamic game detail levels.

		MSG_META_DEBUG_INCR_ANIM_SKATE_SPEED,				///< for debugging anim skate speeds
		MSG_META_DEBUG_DECR_ANIM_SKATE_SPEED,				///< for debugging anim skate speeds
		MSG_META_DEBUG_CYCLE_EXTENT_TYPE,						///< change extent
		MSG_META_DEBUG_INCREASE_EXTENT_MAJOR,				///< change extent
		MSG_META_DEBUG_INCREASE_EXTENT_MAJOR_BIG,		///< change extent
		MSG_META_DEBUG_DECREASE_EXTENT_MAJOR,				///< change extent
		MSG_META_DEBUG_DECREASE_EXTENT_MAJOR_BIG,		///< change extent
		MSG_META_DEBUG_INCREASE_EXTENT_MINOR,				///< change extent
		MSG_META_DEBUG_INCREASE_EXTENT_MINOR_BIG,		///< change extent
		MSG_META_DEBUG_DECREASE_EXTENT_MINOR,				///< change extent
		MSG_META_DEBUG_DECREASE_EXTENT_MINOR_BIG,		///< change extent
		MSG_META_DEBUG_INCREASE_EXTENT_HEIGHT,			///< change extent
		MSG_META_DEBUG_INCREASE_EXTENT_HEIGHT_BIG,	///< change extent
		MSG_META_DEBUG_DECREASE_EXTENT_HEIGHT,			///< change extent
		MSG_META_DEBUG_DECREASE_EXTENT_HEIGHT_BIG,	///< change extent
		MSG_META_DEBUG_VTUNE_ON,										///< turn on/off Vtune
		MSG_META_DEBUG_VTUNE_OFF,										///< turn on/off Vtune
		MSG_META_DEBUG_TOGGLE_FEATHER_WATER, 			///< toggle lorenzen's feather water

		MSG_META_DEBUG_DUMP_ASSETS,						///< dumps currently used map assets to a file.

		MSG_NO_DRAW,																///< show/hide all objects to test Drawing code
		MSG_META_DEMO_TOGGLE_METRICS,								///< Toggle the metrics on/off
		MSG_META_DEMO_TOGGLE_PROJECTILEDEBUG,				///< Toggles bezier curves on projectiles on/off
		MSG_META_DEMO_TOGGLE_VISIONDEBUG,						///< Toggles vision debug circles on/off
		MSG_META_DEMO_TOGGLE_THREATDEBUG,						///< Toggle the threat debugger on/off
		MSG_META_DEMO_TOGGLE_CASHMAPDEBUG,					///< Toggle the cash map debugger on/off
		MSG_META_DEMO_TOGGLE_GRAPHICALFRAMERATEBAR,	///< Toggle the graphical framerate bar on/off
		MSG_META_DEMO_GIVE_ALL_SCIENCES,						///< grant all available sciences, works in MULTIPLAYER for all humans
		MSG_META_DEMO_GIVE_RANKLEVEL,								///< up one rank level
		MSG_META_DEMO_TAKE_RANKLEVEL,								///< up one rank level
		MSG_META_DEMO_GIVE_SCIENCEPURCHASEPOINTS,		///< give a science purchase point (but no rank change)
		MSG_META_DEBUG_TOGGLE_NETWORK,							///< toggle between having and not having network traffic.
		MSG_META_DEBUG_DUMP_PLAYER_OBJECTS,					///< Dump numbers of objects owned by each player to the script debug window
		MSG_META_DEBUG_DUMP_ALL_PLAYER_OBJECTS,			///< Dump numbers of objects owned by each player to the script debug window, and additional object info

		MSG_META_DEBUG_WIN,													///< Instant Win
		MSG_META_DEMO_TOGGLE_DEBUG_STATS,						///< show/hide the debug stats
		/// @todo END section to REMOVE (not disable) for release
#endif // defined(RTS_DEBUG)

#if defined(RTS_DEBUG)
		MSG_META_DEMO_TOGGLE_AUDIODEBUG,						///< show/hide the audio debug info
#endif//defined(RTS_DEBUG)
#ifdef DUMP_PERF_STATS
		MSG_META_DEMO_PERFORM_STATISTICAL_DUMP,			///< dump performance stats for this frame to StatisticsDump.txt
#endif//DUMP_PERF_STATS

		MSG_META_PLACE_BEACON,
		MSG_META_REMOVE_BEACON,

		MSG_END_META_MESSAGES,											///< Marker to delineate "meta" messages

		MSG_MOUSEOVER_DRAWABLE_HINT,								///< (drawableid) the given drawable is under the mouse, regardless of button states
		MSG_MOUSEOVER_LOCATION_HINT,								///< (location) The cursor is not over a drawable, but is here.
		MSG_VALID_GUICOMMAND_HINT,									///< posted when the gui command is valid if the user clicked to execute it.
		MSG_INVALID_GUICOMMAND_HINT,								///< posted when the gui command is not valid if the user were to click to attempt to execute it.
		MSG_AREA_SELECTION_HINT,										///< (pixelRegion) rectangular selection area under construction, not confirmed

		//Command hints
		MSG_DO_ATTACK_OBJECT_HINT,									///< (victim objectID) If clicked, an attack would be ordered, "Current Selection" is assumed
		MSG_IMPOSSIBLE_ATTACK_HINT,									///< we can't do anything, and target is out of range.
		MSG_DO_FORCE_ATTACK_OBJECT_HINT,						///< (victim objectID) If clicked, an attack would be ordered, "Current Selection" is assumed
		MSG_DO_FORCE_ATTACK_GROUND_HINT,						///< (victim objectID) If clicked, an attack would be ordered, "Current Selection" is assumed
		MSG_GET_REPAIRED_HINT,											///< If clicked, selected unit will go get repaired at clicked object
		MSG_GET_HEALED_HINT,												///< If clicked, selected unit will go get healed at clicked object
		MSG_DO_REPAIR_HINT,													///< if clicked, dozer will go repair the clicked target
		MSG_RESUME_CONSTRUCTION_HINT,								///< if clicked, dozer will go construct a partially constructed building
		MSG_ENTER_HINT,															///< if clicked, selected unit(s) will attempt to enter clicked object
		MSG_DOCK_HINT,															///< If clicked, selected unit(s) will dock
		MSG_DO_MOVETO_HINT,													///< (location) If clicked, a move would be ordered, "Current Selection" is assumed
		MSG_DO_ATTACKMOVETO_HINT,										///< (location) If clicked, a move would be ordered, "Current Selection" is assumed
		MSG_ADD_WAYPOINT_HINT,											///< (location) If clicked, a waypoint will be added for currently selected units.
		//Context command hints
		MSG_HIJACK_HINT,								///< if clicked, selected unit(s) will attempt to take over vehicle.
		MSG_FIREBOMB_HINT,								///< throw a molotov cocktail
		MSG_CONVERT_TO_CARBOMB_HINT,								///< if clicked, selected unit(s) will attempt to convert clicked object into a carbomb.
		MSG_CAPTUREBUILDING_HINT,
#ifdef ALLOW_SURRENDER
		MSG_PICK_UP_PRISONER_HINT,
#endif
		MSG_SNIPE_VEHICLE_HINT,
		MSG_DEFECTOR_HINT,
		MSG_SET_RALLY_POINT_HINT,										///< (location) if clicked, we will place a rally point here.
		MSG_DO_SPECIAL_POWER_OVERRIDE_DESTINATION_HINT,
		MSG_DO_SALVAGE_HINT,
		MSG_DO_INVALID_HINT,												///< Display invalid cursor because no real command can be done in this context.
		MSG_DO_ATTACK_OBJECT_AFTER_MOVING_HINT,
		MSG_HACK_HINT,

//*********************************************************************************************************
//*********************************************************************************************************
		/*
			Note that we start this at a fixed value, so that (regardless of ifdefs upstream from us) we have the same
			numeric values for all enums that are saved in replay files. This helps improve compatibility between
			replay files created in internal vs. release builds (though they will still generate sync errors in
			some situations due to other reasons, e.g., DEBUG_LOG using the FPU and thus changing its state).

			there should be NO IFDEFS OF ANY KIND FROM HERE UNTIL MSG_END_NETWORK_MESSAGES.

			there should be NO IFDEFS OF ANY KIND FROM HERE UNTIL MSG_END_NETWORK_MESSAGES.

			there should be NO IFDEFS OF ANY KIND FROM HERE UNTIL MSG_END_NETWORK_MESSAGES.

		*/
		MSG_BEGIN_NETWORK_MESSAGES = 1000,					///< MARKER TO DELINEATE MESSAGES THAT GO OVER THE NETWORK
//*********************************************************************************************************
		MSG_CREATE_SELECTED_GROUP,									/**< (Bool createNewGroup, objectID1, objectID2, ... objectIDN)
																									* The selected team is created/augmented with the given team members
																									*/
		MSG_CREATE_SELECTED_GROUP_NO_SOUND,					/**< (Bool createNewGroup, objectID1, objectID2, ... objectIDN)
																									* The selected team is created/augmented with the given team members.
																									* Do not play their selection sounds.
																									*/
		MSG_DESTROY_SELECTED_GROUP,									///< (teamID) the given team is no longer valid
		MSG_REMOVE_FROM_SELECTED_GROUP,							/**< (objectID1, objectID2, ... objectIDN)
																									* Remove these units from the selected group. (N should almost always be 1)
																									*/
		MSG_SELECTED_GROUP_COMMAND,									///< (teamID) the NEXT COMMAND acts upon all members of this team
		MSG_CREATE_TEAM0,														///< Creates a hotkey squad from the currently selected units.
		MSG_CREATE_TEAM1,
		MSG_CREATE_TEAM2,
		MSG_CREATE_TEAM3,
		MSG_CREATE_TEAM4,
		MSG_CREATE_TEAM5,
		MSG_CREATE_TEAM6,
		MSG_CREATE_TEAM7,
		MSG_CREATE_TEAM8,
		MSG_CREATE_TEAM9,
		MSG_SELECT_TEAM0,														///< Set a hotkey squad to be the currently selected units.
		MSG_SELECT_TEAM1,														///< Set a hotkey squad to be the currently selected units.
		MSG_SELECT_TEAM2,														///< Set a hotkey squad to be the currently selected units.
		MSG_SELECT_TEAM3,														///< Set a hotkey squad to be the currently selected units.
		MSG_SELECT_TEAM4,														///< Set a hotkey squad to be the currently selected units.
		MSG_SELECT_TEAM5,														///< Set a hotkey squad to be the currently selected units.
		MSG_SELECT_TEAM6,														///< Set a hotkey squad to be the currently selected units.
		MSG_SELECT_TEAM7,														///< Set a hotkey squad to be the currently selected units.
		MSG_SELECT_TEAM8,														///< Set a hotkey squad to be the currently selected units.
		MSG_SELECT_TEAM9,														///< Set a hotkey squad to be the currently selected units.
		MSG_ADD_TEAM0,															///< Add hotkey squad to the currently selected units.
		MSG_ADD_TEAM1,															///< Add hotkey squad to the currently selected units.
		MSG_ADD_TEAM2,															///< Add hotkey squad to the currently selected units.
		MSG_ADD_TEAM3,															///< Add hotkey squad to the currently selected units.
		MSG_ADD_TEAM4,															///< Add hotkey squad to the currently selected units.
		MSG_ADD_TEAM5,															///< Add hotkey squad to the currently selected units.
		MSG_ADD_TEAM6,															///< Add hotkey squad to the currently selected units.
		MSG_ADD_TEAM7,															///< Add hotkey squad to the currently selected units.
		MSG_ADD_TEAM8,															///< Add hotkey squad to the currently selected units.
		MSG_ADD_TEAM9,															///< Add hotkey squad to the currently selected units.
		MSG_DO_ATTACKSQUAD,													///< (numObjects) (numObjects * objectID)
		MSG_DO_WEAPON,															///< fire specific weapon
		MSG_DO_WEAPON_AT_LOCATION,									///< fire a specific weapon at location
		MSG_DO_WEAPON_AT_OBJECT,										///< fire a specific weapon at a target object
		MSG_DO_SPECIAL_POWER,												///< do special
		MSG_DO_SPECIAL_POWER_AT_LOCATION,						///< do special with target location
		MSG_DO_SPECIAL_POWER_AT_OBJECT,							///< do special at with target object
		MSG_SET_RALLY_POINT,												///< (objectID, location)
		MSG_PURCHASE_SCIENCE,												///< purchase a science
		MSG_QUEUE_UPGRADE,													///< queue the "research" of an upgrade
		MSG_CANCEL_UPGRADE,													///< cancel the "research" of an upgrade
		MSG_QUEUE_UNIT_CREATE,											///< clicked on a button to queue the production of a unit
		MSG_CANCEL_UNIT_CREATE,											///< clicked on UI button to cancel production of a unit
		MSG_DOZER_CONSTRUCT,												/**< building things requires clicking on a dozer
																										 selecting what to build, selecting where to
																										 build it ... this construct message will
																										 start the actual build process */
		MSG_DOZER_CONSTRUCT_LINE,										///< Like MSG_CONSTRUCT, but for build procesess that occur in a line (like walls)
		MSG_DOZER_CANCEL_CONSTRUCT,									///< cancel construction of a building
		MSG_SELL,																		///< sell a structure
		MSG_EXIT,																		///< WE want to exit from whatever WE are inside of
		MSG_EVACUATE,																///< Dump out all of OUR contained objects
		MSG_EXECUTE_RAILED_TRANSPORT,								///< Execute railed transport sequence
		MSG_COMBATDROP_AT_LOCATION,									///< dump out all rappellers
		MSG_COMBATDROP_AT_OBJECT,										///< dump out all rappellers
		MSG_AREA_SELECTION,													///< (pixelRegion) rectangular selection area
		MSG_DO_ATTACK_OBJECT,												///< (objectID, victim objectID)
		MSG_DO_FORCE_ATTACK_OBJECT,									///< force attack the given object if picked
		MSG_DO_FORCE_ATTACK_GROUND,									///< (locationID) bombard the given location if picked
		MSG_GET_REPAIRED,														///< selected unit will go get repaired at clicked object
		MSG_GET_HEALED,															///< selected unit will go get healed at clicked object
		MSG_DO_REPAIR,															///< dozer will go repair the clicked target
		MSG_RESUME_CONSTRUCTION,										///< resume construction on a structure
		MSG_ENTER,																	///< Enter object
		MSG_DOCK,																		///< Dock with this object
		MSG_DO_MOVETO,															///< location
		MSG_DO_ATTACKMOVETO,												///< location
		MSG_DO_FORCEMOVETO,													///< location
		MSG_ADD_WAYPOINT,														///< location
		MSG_DO_GUARD_POSITION,											///< Guard with the currently selected group
		MSG_DO_GUARD_OBJECT,												///< Guard with the currently selected group
		MSG_DO_STOP,																///< Stop with the currently selected group
		MSG_DO_SCATTER,															///< Scatter the currently selected group
		MSG_INTERNET_HACK,													///< Begin a persistent internet hack (free slow income)
		MSG_DO_CHEER,																///< Orders selected units to play cheer animation (if possible)
		MSG_TOGGLE_OVERCHARGE,											///< Toggle overcharge status of a power plant
		MSG_SWITCH_WEAPONS,													///< Switches which weapon slot to use for an object
		MSG_CONVERT_TO_CARBOMB,
		MSG_CAPTUREBUILDING,
		MSG_DISABLEVEHICLE_HACK,
		MSG_STEALCASH_HACK,
		MSG_DISABLEBUILDING_HACK,
		MSG_SNIPE_VEHICLE,
		MSG_DO_SPECIAL_POWER_OVERRIDE_DESTINATION,
		MSG_DO_SALVAGE,
		MSG_CLEAR_INGAME_POPUP_MESSAGE,							///< If we want a replay to work with the popup messages then we need it to be passed
		MSG_PLACE_BEACON,
		MSG_REMOVE_BEACON,
		MSG_SET_BEACON_TEXT,
		MSG_SET_REPLAY_CAMERA,											///< Track camera pos for replays
		MSG_SELF_DESTRUCT,													///< Destroys a player's units (for copy protection or to quit to observer)
		MSG_CREATE_FORMATION,												///< Creates a formation.
		MSG_LOGIC_CRC,															///< CRC from the logic passed around in a network game :)
		MSG_SET_MINE_CLEARING_DETAIL,								///< CRC from the logic passed around in a network game :)

		MSG_BEGIN_DEBUG_NETWORK_MESSAGES = 1900,		///< network messages that exist only in debug/internal builds. all grouped separately.

#if defined(RTS_DEBUG)
		// all debug/internal-only messages must go here.
		MSG_DEBUG_KILL_SELECTION,
		MSG_DEBUG_HURT_OBJECT,
		MSG_DEBUG_KILL_OBJECT,
#endif

//*********************************************************************************************************
		MSG_END_NETWORK_MESSAGES = 1999,						///< MARKER TO DELINEATE MESSAGES THAT GO OVER THE NETWORK
//*********************************************************************************************************
//*********************************************************************************************************


		// Server to Client messages
		MSG_TIMESTAMP,															///< The current frame number
		MSG_OBJECT_CREATED,													///< (objectID, Int type) Cause a drawable to be created and bound to this ID
		MSG_OBJECT_DESTROYED,												///< (objectID) Free bound drawable
		MSG_OBJECT_POSITION,												///< (objectID, location) New position of object
		MSG_OBJECT_ORIENTATION,											///< (objectID, angle) New orientation of object
		MSG_OBJECT_JOINED_TEAM,											///< (objectID) New team affiliation of object

		MSG_COUNT
	};

	GameMessage( Type type );

	GameMessage *next( void ) { return m_next; }		///< Return next message in the stream
	GameMessage *prev( void ) { return m_prev; }		///< Return prev message in the stream

	Type getType( void ) const { return m_type; }					///< Return the message type
	UnsignedByte getArgumentCount( void ) const { return m_argCount; }	///< Return the number of arguments for this msg

	const char *getCommandAsString( void ) const; ///< returns a string representation of the command type.
	static const char *getCommandTypeAsString(GameMessage::Type t);

	Int getPlayerIndex( void ) const { return m_playerIndex; }		///< Return the originating player

	// access methods for GameMessageArgumentType enum
	void appendIntegerArgument( Int arg );
	void appendRealArgument( Real arg );
	void appendBooleanArgument( Bool arg );
	void appendDrawableIDArgument( DrawableID arg );
	void appendObjectIDArgument( ObjectID arg );
	void appendTeamIDArgument( UnsignedInt arg );
	void appendLocationArgument( const Coord3D& arg );
	void appendPixelArgument( const ICoord2D& arg );
	void appendPixelRegionArgument( const IRegion2D& arg );
	void appendWideCharArgument( const WideChar& arg );

	void appendTimestampArgument( UnsignedInt arg );

	/**
	 * Return the given argument union.
	 * @todo This should be a more list-like interface.  Very inefficient.
	 */
	const GameMessageArgumentType *getArgument( Int argIndex ) const;
	GameMessageArgumentDataType getArgumentDataType( Int argIndex );

	void friend_setNext(GameMessage* m) { m_next = m; }
	void friend_setPrev(GameMessage* m) { m_prev = m; }
	void friend_setList(GameMessageList* m) { m_list = m; }
	void friend_setPlayerIndex(Int i) { m_playerIndex = i; }

private:
	// friend classes are bad. don't use them. no, really.
	// if for no other reason than the fact that they subvert MemoryPoolObject. (srj)

	GameMessage *m_next, *m_prev;								///< List links for message list
	GameMessageList *m_list;										///< The list this message is on

	Type m_type;										///< The type of this message

	Int m_playerIndex;													///< The Player who issued the command

	/// @todo If a GameMessage needs more than 255 arguments, it needs to be split up into multiple GameMessage's.
	UnsignedByte m_argCount;										///< The number of arguments of this message

	GameMessageArgument *m_argList, *m_argTail;						///< This message's arguments

	/// allocate a new argument, add it to list, return pointer to its data
	GameMessageArgument *allocArg( void );

};


/**
 * The GameMessageList class encapsulates the manipulation of lists of GameMessages.
 * Both MessageStream and CommandList derive from this class.
 */
class GameMessageList : public SubsystemInterface
{

public:

	GameMessageList( void );
	virtual ~GameMessageList();

	virtual void init( void ) { };			///< Initialize system
	virtual void reset( void ) { };			///< Reset system
	virtual void update( void ) { };		///< Update system

	GameMessage *getFirstMessage( void ) { return m_firstMessage; }	///< Return the first message

	virtual void appendMessage( GameMessage *msg );			///< Add message to end of the list
	virtual void insertMessage( GameMessage *msg, GameMessage *messageToInsertAfter );	// Insert message after messageToInsertAfter.
	virtual void removeMessage( GameMessage *msg );			///< Remove message from the list
	virtual Bool containsMessageOfType( GameMessage::Type type );	///< Return true if a message of type is in the message stream



protected:
	GameMessage *m_firstMessage;								///< The first message on the list
	GameMessage *m_lastMessage;									///< The last message on the list
};

/**
	What to do with a GameMessage after a translator has handled it.
	Use a custom enum (rather than a Bool) to make the code more obvious.
*/
enum GameMessageDisposition CPP_11(: Int)
{
	KEEP_MESSAGE,			///< continue processing this message thru other translators
	DESTROY_MESSAGE		///< destroy this message immediately and don't hand it to any other translators
};

class GameMessageTranslator
{
public:
	virtual GameMessageDisposition translateGameMessage(const GameMessage *msg) = 0;
	virtual ~GameMessageTranslator() { }
};

/**
 * A MessageStream contains an ordered list of messages which can have one or more
 * prioritized message handler functions ("translators") attached to it.
 */
class MessageStream : public GameMessageList
{

public:

	MessageStream( void );
	virtual ~MessageStream();

	// Inherited Methods ----------------------------------------------------------------------------
	virtual void init( void );
	virtual void reset( void );
	virtual void update( void );

	virtual GameMessage *appendMessage( GameMessage::Type type );		///< Append a message to the end of the stream
	virtual GameMessage *insertMessage( GameMessage::Type type, GameMessage *messageToInsertAfter );	// Insert message after messageToInsertAfter.

	// Methods NOT Inherited ------------------------------------------------------------------------
	void propagateMessages( void );													///< Propagate messages through attached translators

	/**
		Attach a translator function to the stream at a priority value. Lower priorities are executed first.
		Note that MessageStream assumes ownership of the translator, and is responsible for freeing it!
	*/
	TranslatorID attachTranslator( GameMessageTranslator *translator, UnsignedInt priority);
	GameMessageTranslator* findTranslator( TranslatorID id );
	void removeTranslator( TranslatorID );				///< Remove a previously attached translator

protected:

	struct TranslatorData
	{
		TranslatorData *m_next, *m_prev;						///< List links for list of translators
		TranslatorID m_id;													///< The unique ID of this translator
		GameMessageTranslator *m_translator;					///< The translor's interface function
		UnsignedInt m_priority;											///< The priority level of this translator

		TranslatorData() : m_next(0), m_prev(0), m_id(0), m_translator(0), m_priority(0)
		{
		}

		~TranslatorData()
		{
			delete m_translator;
		}
	};

	TranslatorData *m_firstTranslator;						///< List of registered translators, in order of priority
	TranslatorData *m_lastTranslator;
	TranslatorID m_nextTranslatorID;							///< For issuing unique translator ID's

};


/**
 * The CommandList is the final set of messages that have made their way through
 * all of the Translators of the MessageStream, and reached the end.
 * This set of commands will be executed by the GameLogic on its next iteration.
 */
class CommandList : public GameMessageList
{
public:
	CommandList( void );
	virtual ~CommandList();

	virtual void init( void );			///< Init command list
	virtual void reset( void );			///< Destroy all messages and reset list to empty
	virtual void update( void );		///< Update hook

	void appendMessageList( GameMessage *list );			///< Adds messages to the end of the command list

protected:

	void destroyAllMessages( void );		///< The meat of a reset and a shutdown

};

//
// The message stream that filters client input into game commands
//
extern MessageStream *TheMessageStream;

//
// The list of commands awaiting execution by the GameLogic
//
extern CommandList *TheCommandList;


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Functions used in multiple translators should go here.
//

/**
 * Given an "anchor" point and the current mouse position (dest),
 * construct a valid 2D bounding region.
 */
extern void buildRegion( const ICoord2D *anchor, const ICoord2D *dest, IRegion2D *region );

#endif // _MESSAGE_STREAM_H_
