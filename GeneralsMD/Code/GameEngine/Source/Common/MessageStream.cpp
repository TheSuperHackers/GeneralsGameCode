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

// MessageStream.cpp
// Implementation of the message stream
// Author: Michael S. Booth, February 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/MessageStream.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Recorder.h"

#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"

/// The singleton message stream for messages going to TheGameLogic
MessageStream *TheMessageStream = NULL;
CommandList *TheCommandList = NULL;






//------------------------------------------------------------------------------------------------
// GameMessage
//

/**
 * Constructor
 */
GameMessage::GameMessage( GameMessage::Type type )
{
	m_playerIndex = ThePlayerList->getLocalPlayer()->getPlayerIndex();
	m_type = type;
	m_argList = NULL;
	m_argTail = NULL;
	m_argCount = 0;
	m_list = 0;
}


/**
 * Destructor
 */
GameMessage::~GameMessage( )
{
	// free all arguments
	GameMessageArgument *arg, *nextArg;

	for( arg = m_argList; arg; arg=nextArg )
	{
		nextArg = arg->m_next;
		deleteInstance(arg);
	}

	// detach message from list
	if (m_list)
		m_list->removeMessage( this );
}

/**
 * Return the given argument union.
 * @todo This should be a more list-like interface.  Very inefficient.
 */
const GameMessageArgumentType *GameMessage::getArgument( Int argIndex ) const
{
	static const GameMessageArgumentType junk = { 0 };

	int i=0;
	for( GameMessageArgument *a = m_argList; a; a=a->m_next, i++ )
		if (i == argIndex)
			return &a->m_data;

	DEBUG_CRASH(("argument not found"));
	return &junk;
}

/**
 * Return the given argument data type
 */
GameMessageArgumentDataType GameMessage::getArgumentDataType( Int argIndex )
{
	if (argIndex >= m_argCount) {
		return ARGUMENTDATATYPE_UNKNOWN;
	}
	int i=0;
	GameMessageArgument *a = m_argList;
	for (; a && (i < argIndex); a=a->m_next, ++i );

	if (a != NULL)
	{
		return a->m_type;
	}
	return ARGUMENTDATATYPE_UNKNOWN;
}

/**
 * Allocate a new argument, add it to the argument list, and increment the total arg count
 */
GameMessageArgument *GameMessage::allocArg( void )
{
	// allocate a new argument
	GameMessageArgument *arg = newInstance(GameMessageArgument);

	// add to end of argument list
	if (m_argTail)
		m_argTail->m_next = arg;
	else
	{
		m_argList = arg;
		m_argTail = arg;
	}

	arg->m_next = NULL;
	m_argTail = arg;

	m_argCount++;

	return arg;
}

/**
 * Append an integer argument
 */
void GameMessage::appendIntegerArgument( Int arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.integer = arg;
	a->m_type = ARGUMENTDATATYPE_INTEGER;
}

void GameMessage::appendRealArgument( Real arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.real = arg;
	a->m_type = ARGUMENTDATATYPE_REAL;
}

void GameMessage::appendBooleanArgument( Bool arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.boolean = arg;
	a->m_type = ARGUMENTDATATYPE_BOOLEAN;
}

void GameMessage::appendObjectIDArgument( ObjectID arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.objectID = arg;
	a->m_type = ARGUMENTDATATYPE_OBJECTID;
}

void GameMessage::appendDrawableIDArgument( DrawableID arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.drawableID = arg;
	a->m_type = ARGUMENTDATATYPE_DRAWABLEID;
}

void GameMessage::appendTeamIDArgument( UnsignedInt arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.teamID = arg;
	a->m_type = ARGUMENTDATATYPE_TEAMID;
}

void GameMessage::appendLocationArgument( const Coord3D& arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.location = arg;
	a->m_type = ARGUMENTDATATYPE_LOCATION;
}

void GameMessage::appendPixelArgument( const ICoord2D& arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.pixel = arg;
	a->m_type = ARGUMENTDATATYPE_PIXEL;
}

void GameMessage::appendPixelRegionArgument( const IRegion2D& arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.pixelRegion = arg;
	a->m_type = ARGUMENTDATATYPE_PIXELREGION;
}

void GameMessage::appendTimestampArgument( UnsignedInt arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.timestamp = arg;
	a->m_type = ARGUMENTDATATYPE_TIMESTAMP;
}

void GameMessage::appendWideCharArgument( const WideChar& arg )
{
	GameMessageArgument *a = allocArg();
	a->m_data.wChar = arg;
	a->m_type = ARGUMENTDATATYPE_WIDECHAR;
}

const char *GameMessage::getCommandAsString( void ) const
{
	return getCommandTypeAsString(m_type);
}

const char *GameMessage::getCommandTypeAsString(GameMessage::Type t)
{
#define CASE_LABEL(x) case x: return #x;

	switch (t) {
	default: return (t >= GameMessage::MSG_COUNT) ? "Invalid command" : "UnknownMessage";

	CASE_LABEL(MSG_INVALID)
	CASE_LABEL(MSG_FRAME_TICK)
	CASE_LABEL(MSG_RAW_MOUSE_BEGIN)
	CASE_LABEL(MSG_RAW_MOUSE_POSITION)
	CASE_LABEL(MSG_RAW_MOUSE_LEFT_BUTTON_DOWN)
	CASE_LABEL(MSG_RAW_MOUSE_LEFT_DOUBLE_CLICK)
	CASE_LABEL(MSG_RAW_MOUSE_LEFT_BUTTON_UP)
	CASE_LABEL(MSG_RAW_MOUSE_LEFT_CLICK)
	CASE_LABEL(MSG_RAW_MOUSE_LEFT_DRAG)
	CASE_LABEL(MSG_RAW_MOUSE_MIDDLE_BUTTON_DOWN)
	CASE_LABEL(MSG_RAW_MOUSE_MIDDLE_DOUBLE_CLICK)
	CASE_LABEL(MSG_RAW_MOUSE_MIDDLE_BUTTON_UP)
	CASE_LABEL(MSG_RAW_MOUSE_MIDDLE_DRAG)
	CASE_LABEL(MSG_RAW_MOUSE_RIGHT_BUTTON_DOWN)
	CASE_LABEL(MSG_RAW_MOUSE_RIGHT_DOUBLE_CLICK)
	CASE_LABEL(MSG_RAW_MOUSE_RIGHT_BUTTON_UP)
	CASE_LABEL(MSG_RAW_MOUSE_RIGHT_DRAG)
	CASE_LABEL(MSG_RAW_MOUSE_WHEEL)
	CASE_LABEL(MSG_RAW_MOUSE_END)
	CASE_LABEL(MSG_RAW_KEY_DOWN)
	CASE_LABEL(MSG_RAW_KEY_UP)
	CASE_LABEL(MSG_MOUSE_LEFT_CLICK)
	CASE_LABEL(MSG_MOUSE_LEFT_DOUBLE_CLICK)
	CASE_LABEL(MSG_MOUSE_MIDDLE_CLICK)
	CASE_LABEL(MSG_MOUSE_MIDDLE_DOUBLE_CLICK)
	CASE_LABEL(MSG_MOUSE_RIGHT_CLICK)
	CASE_LABEL(MSG_MOUSE_RIGHT_DOUBLE_CLICK)
	CASE_LABEL(MSG_CLEAR_GAME_DATA)
	CASE_LABEL(MSG_NEW_GAME)
	CASE_LABEL(MSG_BEGIN_META_MESSAGES)
	CASE_LABEL(MSG_META_SAVE_VIEW1)
	CASE_LABEL(MSG_META_SAVE_VIEW2)
	CASE_LABEL(MSG_META_SAVE_VIEW3)
	CASE_LABEL(MSG_META_SAVE_VIEW4)
	CASE_LABEL(MSG_META_SAVE_VIEW5)
	CASE_LABEL(MSG_META_SAVE_VIEW6)
	CASE_LABEL(MSG_META_SAVE_VIEW7)
	CASE_LABEL(MSG_META_SAVE_VIEW8)
	CASE_LABEL(MSG_META_VIEW_VIEW1)
	CASE_LABEL(MSG_META_VIEW_VIEW2)
	CASE_LABEL(MSG_META_VIEW_VIEW3)
	CASE_LABEL(MSG_META_VIEW_VIEW4)
	CASE_LABEL(MSG_META_VIEW_VIEW5)
	CASE_LABEL(MSG_META_VIEW_VIEW6)
	CASE_LABEL(MSG_META_VIEW_VIEW7)
	CASE_LABEL(MSG_META_VIEW_VIEW8)
	CASE_LABEL(MSG_META_CREATE_TEAM0)
	CASE_LABEL(MSG_META_CREATE_TEAM1)
	CASE_LABEL(MSG_META_CREATE_TEAM2)
	CASE_LABEL(MSG_META_CREATE_TEAM3)
	CASE_LABEL(MSG_META_CREATE_TEAM4)
	CASE_LABEL(MSG_META_CREATE_TEAM5)
	CASE_LABEL(MSG_META_CREATE_TEAM6)
	CASE_LABEL(MSG_META_CREATE_TEAM7)
	CASE_LABEL(MSG_META_CREATE_TEAM8)
	CASE_LABEL(MSG_META_CREATE_TEAM9)
	CASE_LABEL(MSG_META_SELECT_TEAM0)
	CASE_LABEL(MSG_META_SELECT_TEAM1)
	CASE_LABEL(MSG_META_SELECT_TEAM2)
	CASE_LABEL(MSG_META_SELECT_TEAM3)
	CASE_LABEL(MSG_META_SELECT_TEAM4)
	CASE_LABEL(MSG_META_SELECT_TEAM5)
	CASE_LABEL(MSG_META_SELECT_TEAM6)
	CASE_LABEL(MSG_META_SELECT_TEAM7)
	CASE_LABEL(MSG_META_SELECT_TEAM8)
	CASE_LABEL(MSG_META_SELECT_TEAM9)
	CASE_LABEL(MSG_META_ADD_TEAM0)
	CASE_LABEL(MSG_META_ADD_TEAM1)
	CASE_LABEL(MSG_META_ADD_TEAM2)
	CASE_LABEL(MSG_META_ADD_TEAM3)
	CASE_LABEL(MSG_META_ADD_TEAM4)
	CASE_LABEL(MSG_META_ADD_TEAM5)
	CASE_LABEL(MSG_META_ADD_TEAM6)
	CASE_LABEL(MSG_META_ADD_TEAM7)
	CASE_LABEL(MSG_META_ADD_TEAM8)
	CASE_LABEL(MSG_META_ADD_TEAM9)
	CASE_LABEL(MSG_META_VIEW_TEAM0)
	CASE_LABEL(MSG_META_VIEW_TEAM1)
	CASE_LABEL(MSG_META_VIEW_TEAM2)
	CASE_LABEL(MSG_META_VIEW_TEAM3)
	CASE_LABEL(MSG_META_VIEW_TEAM4)
	CASE_LABEL(MSG_META_VIEW_TEAM5)
	CASE_LABEL(MSG_META_VIEW_TEAM6)
	CASE_LABEL(MSG_META_VIEW_TEAM7)
	CASE_LABEL(MSG_META_VIEW_TEAM8)
	CASE_LABEL(MSG_META_VIEW_TEAM9)
	CASE_LABEL(MSG_META_SELECT_MATCHING_UNITS)
	CASE_LABEL(MSG_META_SELECT_NEXT_UNIT)
	CASE_LABEL(MSG_META_SELECT_PREV_UNIT)
	CASE_LABEL(MSG_META_SELECT_NEXT_WORKER)
	CASE_LABEL(MSG_META_SELECT_PREV_WORKER)
	CASE_LABEL(MSG_META_VIEW_COMMAND_CENTER)
	CASE_LABEL(MSG_META_VIEW_LAST_RADAR_EVENT)
	CASE_LABEL(MSG_META_SELECT_HERO)
	CASE_LABEL(MSG_META_SELECT_ALL)
	CASE_LABEL(MSG_META_SELECT_ALL_AIRCRAFT)
	CASE_LABEL(MSG_META_SCATTER)
	CASE_LABEL(MSG_META_STOP)
	CASE_LABEL(MSG_META_DEPLOY)
	CASE_LABEL(MSG_META_CREATE_FORMATION)
	CASE_LABEL(MSG_META_FOLLOW)
	CASE_LABEL(MSG_META_CHAT_PLAYERS)
	CASE_LABEL(MSG_META_CHAT_ALLIES)
	CASE_LABEL(MSG_META_CHAT_EVERYONE)
	CASE_LABEL(MSG_META_DIPLOMACY)
	CASE_LABEL(MSG_META_OPTIONS)

#if defined(RTS_DEBUG)
	CASE_LABEL(MSG_META_HELP)
#endif

	CASE_LABEL(MSG_META_INCREASE_MAX_RENDER_FPS)
	CASE_LABEL(MSG_META_DECREASE_MAX_RENDER_FPS)
	CASE_LABEL(MSG_META_INCREASE_LOGIC_TIME_SCALE)
	CASE_LABEL(MSG_META_DECREASE_LOGIC_TIME_SCALE)
	CASE_LABEL(MSG_META_TOGGLE_LOWER_DETAILS)
	CASE_LABEL(MSG_META_TOGGLE_CONTROL_BAR)
	CASE_LABEL(MSG_META_BEGIN_PATH_BUILD)
	CASE_LABEL(MSG_META_END_PATH_BUILD)
	CASE_LABEL(MSG_META_BEGIN_FORCEATTACK)
	CASE_LABEL(MSG_META_END_FORCEATTACK)
	CASE_LABEL(MSG_META_BEGIN_FORCEMOVE)
	CASE_LABEL(MSG_META_END_FORCEMOVE)
	CASE_LABEL(MSG_META_BEGIN_WAYPOINTS)
	CASE_LABEL(MSG_META_END_WAYPOINTS)
	CASE_LABEL(MSG_META_BEGIN_PREFER_SELECTION)
	CASE_LABEL(MSG_META_END_PREFER_SELECTION)
	CASE_LABEL(MSG_META_TAKE_SCREENSHOT)
	CASE_LABEL(MSG_META_ALL_CHEER)
	CASE_LABEL(MSG_META_TOGGLE_ATTACKMOVE)
	CASE_LABEL(MSG_META_BEGIN_CAMERA_ROTATE_LEFT)
	CASE_LABEL(MSG_META_END_CAMERA_ROTATE_LEFT)
	CASE_LABEL(MSG_META_BEGIN_CAMERA_ROTATE_RIGHT)
	CASE_LABEL(MSG_META_END_CAMERA_ROTATE_RIGHT)
	CASE_LABEL(MSG_META_BEGIN_CAMERA_ZOOM_IN)
	CASE_LABEL(MSG_META_END_CAMERA_ZOOM_IN)
	CASE_LABEL(MSG_META_BEGIN_CAMERA_ZOOM_OUT)
	CASE_LABEL(MSG_META_END_CAMERA_ZOOM_OUT)
	CASE_LABEL(MSG_META_CAMERA_RESET)
	CASE_LABEL(MSG_META_TOGGLE_CAMERA_TRACKING_DRAWABLE)
	CASE_LABEL(MSG_META_DEMO_INSTANT_QUIT)

#if defined(_ALLOW_DEBUG_CHEATS_IN_RELEASE)//may be defined in GameCommon.h
	CASE_LABEL(MSG_CHEAT_RUNSCRIPT1)
	CASE_LABEL(MSG_CHEAT_RUNSCRIPT2)
	CASE_LABEL(MSG_CHEAT_RUNSCRIPT3)
	CASE_LABEL(MSG_CHEAT_RUNSCRIPT4)
	CASE_LABEL(MSG_CHEAT_RUNSCRIPT5)
	CASE_LABEL(MSG_CHEAT_RUNSCRIPT6)
	CASE_LABEL(MSG_CHEAT_RUNSCRIPT7)
	CASE_LABEL(MSG_CHEAT_RUNSCRIPT8)
	CASE_LABEL(MSG_CHEAT_RUNSCRIPT9)
	CASE_LABEL(MSG_CHEAT_TOGGLE_SPECIAL_POWER_DELAYS)
	CASE_LABEL(MSG_CHEAT_SWITCH_TEAMS)
	CASE_LABEL(MSG_CHEAT_KILL_SELECTION)
	CASE_LABEL(MSG_CHEAT_TOGGLE_HAND_OF_GOD_MODE)
	CASE_LABEL(MSG_CHEAT_INSTANT_BUILD)
	CASE_LABEL(MSG_CHEAT_DESHROUD)
	CASE_LABEL(MSG_CHEAT_ADD_CASH)
	CASE_LABEL(MSG_CHEAT_GIVE_ALL_SCIENCES)
	CASE_LABEL(MSG_CHEAT_GIVE_SCIENCEPURCHASEPOINTS)
	CASE_LABEL(MSG_CHEAT_SHOW_HEALTH)
	CASE_LABEL(MSG_CHEAT_TOGGLE_MESSAGE_TEXT)
#endif

	CASE_LABEL(MSG_META_TOGGLE_FAST_FORWARD_REPLAY)
	CASE_LABEL(MSG_META_TOGGLE_PAUSE)
	CASE_LABEL(MSG_META_STEP_FRAME)

#if defined(RTS_DEBUG)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_BEHIND_BUILDINGS)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_LETTERBOX)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_MESSAGE_TEXT)
	CASE_LABEL(MSG_META_DEMO_LOD_DECREASE)
	CASE_LABEL(MSG_META_DEMO_LOD_INCREASE)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_ZOOM_LOCK)
	CASE_LABEL(MSG_META_DEMO_PLAY_CAMEO_MOVIE)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_SPECIAL_POWER_DELAYS)
	CASE_LABEL(MSG_META_DEMO_BATTLE_CRY)
	CASE_LABEL(MSG_META_DEMO_SWITCH_TEAMS)
	CASE_LABEL(MSG_META_DEMO_SWITCH_TEAMS_BETWEEN_CHINA_USA)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_PARTICLEDEBUG)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_SHADOW_VOLUMES)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_FOGOFWAR)
	CASE_LABEL(MSG_META_DEMO_KILL_ALL_ENEMIES)
	CASE_LABEL(MSG_META_DEMO_KILL_SELECTION)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_HURT_ME_MODE)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_HAND_OF_GOD_MODE)
	CASE_LABEL(MSG_META_DEMO_DEBUG_SELECTION)
	CASE_LABEL(MSG_META_DEMO_LOCK_CAMERA_TO_SELECTION)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_SOUND)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_TRACKMARKS)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_WATERPLANE)
	CASE_LABEL(MSG_META_DEMO_TIME_OF_DAY)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_MUSIC)
	CASE_LABEL(MSG_META_DEMO_MUSIC_NEXT_TRACK)
	CASE_LABEL(MSG_META_DEMO_MUSIC_PREV_TRACK)
	CASE_LABEL(MSG_META_DEMO_NEXT_OBJECTIVE_MOVIE)
	CASE_LABEL(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE1)
	CASE_LABEL(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE2)
	CASE_LABEL(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE3)
	CASE_LABEL(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE4)
	CASE_LABEL(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE5)
	CASE_LABEL(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE6)
	CASE_LABEL(MSG_META_DEMO_BEGIN_ADJUST_PITCH)
	CASE_LABEL(MSG_META_DEMO_END_ADJUST_PITCH)
	CASE_LABEL(MSG_META_DEMO_BEGIN_ADJUST_FOV)
	CASE_LABEL(MSG_META_DEMO_END_ADJUST_FOV)
	CASE_LABEL(MSG_META_DEMO_LOCK_CAMERA_TO_PLANES)
	CASE_LABEL(MSG_META_DEMO_REMOVE_PREREQ)
	CASE_LABEL(MSG_META_DEMO_INSTANT_BUILD)
	CASE_LABEL(MSG_META_DEMO_FREE_BUILD)
	CASE_LABEL(MSG_META_DEMO_RUNSCRIPT1)
	CASE_LABEL(MSG_META_DEMO_RUNSCRIPT2)
	CASE_LABEL(MSG_META_DEMO_RUNSCRIPT3)
	CASE_LABEL(MSG_META_DEMO_RUNSCRIPT4)
	CASE_LABEL(MSG_META_DEMO_RUNSCRIPT5)
	CASE_LABEL(MSG_META_DEMO_RUNSCRIPT6)
	CASE_LABEL(MSG_META_DEMO_RUNSCRIPT7)
	CASE_LABEL(MSG_META_DEMO_RUNSCRIPT8)
	CASE_LABEL(MSG_META_DEMO_RUNSCRIPT9)
	CASE_LABEL(MSG_META_DEMO_ENSHROUD)
	CASE_LABEL(MSG_META_DEMO_DESHROUD)
	CASE_LABEL(MSG_META_DEBUG_SHOW_EXTENTS)
	CASE_LABEL(MSG_META_DEBUG_SHOW_AUDIO_LOCATIONS)
	CASE_LABEL(MSG_META_DEBUG_SHOW_HEALTH)
	CASE_LABEL(MSG_META_DEBUG_GIVE_VETERANCY)
	CASE_LABEL(MSG_META_DEBUG_TAKE_VETERANCY)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_AI_DEBUG)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_SUPPLY_CENTER_PLACEMENT)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_CAMERA_DEBUG)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_AVI)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_BW_VIEW)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_RED_VIEW)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_GREEN_VIEW)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_MOTION_BLUR_ZOOM)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_MILITARY_SUBTITLES)
	CASE_LABEL(MSG_META_DEMO_ADD_CASH)

#ifdef ALLOW_SURRENDER
	CASE_LABEL(MSG_META_DEMO_TEST_SURRENDER)
#endif

	CASE_LABEL(MSG_META_DEMO_TOGGLE_RENDER)
	CASE_LABEL(MSG_META_DEMO_KILL_AREA_SELECTION)
	CASE_LABEL(MSG_META_DEMO_CYCLE_LOD_LEVEL)
	CASE_LABEL(MSG_META_DEBUG_INCR_ANIM_SKATE_SPEED)
	CASE_LABEL(MSG_META_DEBUG_DECR_ANIM_SKATE_SPEED)
	CASE_LABEL(MSG_META_DEBUG_CYCLE_EXTENT_TYPE)
	CASE_LABEL(MSG_META_DEBUG_INCREASE_EXTENT_MAJOR)
	CASE_LABEL(MSG_META_DEBUG_INCREASE_EXTENT_MAJOR_BIG)
	CASE_LABEL(MSG_META_DEBUG_DECREASE_EXTENT_MAJOR)
	CASE_LABEL(MSG_META_DEBUG_DECREASE_EXTENT_MAJOR_BIG)
	CASE_LABEL(MSG_META_DEBUG_INCREASE_EXTENT_MINOR)
	CASE_LABEL(MSG_META_DEBUG_INCREASE_EXTENT_MINOR_BIG)
	CASE_LABEL(MSG_META_DEBUG_DECREASE_EXTENT_MINOR)
	CASE_LABEL(MSG_META_DEBUG_DECREASE_EXTENT_MINOR_BIG)
	CASE_LABEL(MSG_META_DEBUG_INCREASE_EXTENT_HEIGHT)
	CASE_LABEL(MSG_META_DEBUG_INCREASE_EXTENT_HEIGHT_BIG)
	CASE_LABEL(MSG_META_DEBUG_DECREASE_EXTENT_HEIGHT)
	CASE_LABEL(MSG_META_DEBUG_DECREASE_EXTENT_HEIGHT_BIG)
	CASE_LABEL(MSG_META_DEBUG_VTUNE_ON)
	CASE_LABEL(MSG_META_DEBUG_VTUNE_OFF)
	CASE_LABEL(MSG_META_DEBUG_TOGGLE_FEATHER_WATER)
	CASE_LABEL(MSG_META_DEBUG_DUMP_ASSETS)
	CASE_LABEL(MSG_NO_DRAW)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_METRICS)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_PROJECTILEDEBUG)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_VISIONDEBUG)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_THREATDEBUG)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_CASHMAPDEBUG)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_GRAPHICALFRAMERATEBAR)
	CASE_LABEL(MSG_META_DEMO_GIVE_ALL_SCIENCES)
	CASE_LABEL(MSG_META_DEMO_GIVE_RANKLEVEL)
	CASE_LABEL(MSG_META_DEMO_TAKE_RANKLEVEL)
	CASE_LABEL(MSG_META_DEMO_GIVE_SCIENCEPURCHASEPOINTS)
	CASE_LABEL(MSG_META_DEBUG_TOGGLE_NETWORK)
	CASE_LABEL(MSG_META_DEBUG_DUMP_PLAYER_OBJECTS)
	CASE_LABEL(MSG_META_DEBUG_DUMP_ALL_PLAYER_OBJECTS)
	CASE_LABEL(MSG_META_DEBUG_OBJECT_ID_PERFORMANCE)
	CASE_LABEL(MSG_META_DEBUG_DRAWABLE_ID_PERFORMANCE)
	CASE_LABEL(MSG_META_DEBUG_SLEEPY_UPDATE_PERFORMANCE)
	CASE_LABEL(MSG_META_DEBUG_WIN)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_DEBUG_STATS)
#endif // defined(RTS_DEBUG)

#if defined(RTS_DEBUG)
	CASE_LABEL(MSG_META_DEMO_TOGGLE_AUDIODEBUG)
#endif//defined(RTS_DEBUG)

#ifdef DUMP_PERF_STATS
	CASE_LABEL(MSG_META_DEMO_PERFORM_STATISTICAL_DUMP)
#endif//DUMP_PERF_STATS

	CASE_LABEL(MSG_META_PLACE_BEACON)
	CASE_LABEL(MSG_META_REMOVE_BEACON)
	CASE_LABEL(MSG_END_META_MESSAGES)
	CASE_LABEL(MSG_MOUSEOVER_DRAWABLE_HINT)
	CASE_LABEL(MSG_MOUSEOVER_LOCATION_HINT)
	CASE_LABEL(MSG_VALID_GUICOMMAND_HINT)
	CASE_LABEL(MSG_INVALID_GUICOMMAND_HINT)
	CASE_LABEL(MSG_AREA_SELECTION_HINT)
	CASE_LABEL(MSG_DO_ATTACK_OBJECT_HINT)
	CASE_LABEL(MSG_DO_ATTACK_OBJECT_AFTER_MOVING_HINT)
	CASE_LABEL(MSG_DO_FORCE_ATTACK_OBJECT_HINT)
	CASE_LABEL(MSG_DO_FORCE_ATTACK_GROUND_HINT)
	CASE_LABEL(MSG_GET_REPAIRED_HINT)
	CASE_LABEL(MSG_GET_HEALED_HINT)
	CASE_LABEL(MSG_DO_REPAIR_HINT)
	CASE_LABEL(MSG_RESUME_CONSTRUCTION_HINT)
	CASE_LABEL(MSG_ENTER_HINT)
	CASE_LABEL(MSG_DOCK_HINT)
	CASE_LABEL(MSG_DO_MOVETO_HINT)
	CASE_LABEL(MSG_DO_ATTACKMOVETO_HINT)
	CASE_LABEL(MSG_ADD_WAYPOINT_HINT)
	CASE_LABEL(MSG_HIJACK_HINT)
	CASE_LABEL(MSG_SABOTAGE_HINT)
	CASE_LABEL(MSG_FIREBOMB_HINT)
	CASE_LABEL(MSG_CONVERT_TO_CARBOMB_HINT)
	CASE_LABEL(MSG_CAPTUREBUILDING_HINT)
	CASE_LABEL(MSG_HACK_HINT)

#ifdef ALLOW_SURRENDER
	CASE_LABEL(MSG_PICK_UP_PRISONER_HINT)
#endif

	CASE_LABEL(MSG_SNIPE_VEHICLE_HINT)
	CASE_LABEL(MSG_DEFECTOR_HINT)
	CASE_LABEL(MSG_SET_RALLY_POINT_HINT)
	CASE_LABEL(MSG_DO_SALVAGE_HINT)
	CASE_LABEL(MSG_DO_INVALID_HINT)
	CASE_LABEL(MSG_BEGIN_NETWORK_MESSAGES)
	CASE_LABEL(MSG_CREATE_SELECTED_GROUP)
	CASE_LABEL(MSG_CREATE_SELECTED_GROUP_NO_SOUND)
	CASE_LABEL(MSG_DESTROY_SELECTED_GROUP)
	CASE_LABEL(MSG_REMOVE_FROM_SELECTED_GROUP)
	CASE_LABEL(MSG_SELECTED_GROUP_COMMAND)
	CASE_LABEL(MSG_CREATE_TEAM0)
	CASE_LABEL(MSG_CREATE_TEAM1)
	CASE_LABEL(MSG_CREATE_TEAM2)
	CASE_LABEL(MSG_CREATE_TEAM3)
	CASE_LABEL(MSG_CREATE_TEAM4)
	CASE_LABEL(MSG_CREATE_TEAM5)
	CASE_LABEL(MSG_CREATE_TEAM6)
	CASE_LABEL(MSG_CREATE_TEAM7)
	CASE_LABEL(MSG_CREATE_TEAM8)
	CASE_LABEL(MSG_CREATE_TEAM9)
	CASE_LABEL(MSG_SELECT_TEAM0)
	CASE_LABEL(MSG_SELECT_TEAM1)
	CASE_LABEL(MSG_SELECT_TEAM2)
	CASE_LABEL(MSG_SELECT_TEAM3)
	CASE_LABEL(MSG_SELECT_TEAM4)
	CASE_LABEL(MSG_SELECT_TEAM5)
	CASE_LABEL(MSG_SELECT_TEAM6)
	CASE_LABEL(MSG_SELECT_TEAM7)
	CASE_LABEL(MSG_SELECT_TEAM8)
	CASE_LABEL(MSG_SELECT_TEAM9)
	CASE_LABEL(MSG_ADD_TEAM0)
	CASE_LABEL(MSG_ADD_TEAM1)
	CASE_LABEL(MSG_ADD_TEAM2)
	CASE_LABEL(MSG_ADD_TEAM3)
	CASE_LABEL(MSG_ADD_TEAM4)
	CASE_LABEL(MSG_ADD_TEAM5)
	CASE_LABEL(MSG_ADD_TEAM6)
	CASE_LABEL(MSG_ADD_TEAM7)
	CASE_LABEL(MSG_ADD_TEAM8)
	CASE_LABEL(MSG_ADD_TEAM9)
	CASE_LABEL(MSG_DO_ATTACKSQUAD)
	CASE_LABEL(MSG_DO_WEAPON)
	CASE_LABEL(MSG_DO_WEAPON_AT_LOCATION)
	CASE_LABEL(MSG_DO_WEAPON_AT_OBJECT)
	CASE_LABEL(MSG_DO_SPECIAL_POWER)
	CASE_LABEL(MSG_DO_SPECIAL_POWER_AT_LOCATION)
	CASE_LABEL(MSG_DO_SPECIAL_POWER_AT_OBJECT)
	CASE_LABEL(MSG_SET_RALLY_POINT)
	CASE_LABEL(MSG_PURCHASE_SCIENCE)
	CASE_LABEL(MSG_QUEUE_UPGRADE)
	CASE_LABEL(MSG_CANCEL_UPGRADE)
	CASE_LABEL(MSG_QUEUE_UNIT_CREATE)
	CASE_LABEL(MSG_CANCEL_UNIT_CREATE)
	CASE_LABEL(MSG_DOZER_CONSTRUCT)
	CASE_LABEL(MSG_DOZER_CONSTRUCT_LINE)
	CASE_LABEL(MSG_DOZER_CANCEL_CONSTRUCT)
	CASE_LABEL(MSG_SELL)
	CASE_LABEL(MSG_EXIT)
	CASE_LABEL(MSG_EVACUATE)
	CASE_LABEL(MSG_EXECUTE_RAILED_TRANSPORT)
	CASE_LABEL(MSG_COMBATDROP_AT_LOCATION)
	CASE_LABEL(MSG_COMBATDROP_AT_OBJECT)
	CASE_LABEL(MSG_AREA_SELECTION)
	CASE_LABEL(MSG_DO_ATTACK_OBJECT)
	CASE_LABEL(MSG_DO_FORCE_ATTACK_OBJECT)
	CASE_LABEL(MSG_DO_FORCE_ATTACK_GROUND)
	CASE_LABEL(MSG_GET_REPAIRED)
	CASE_LABEL(MSG_GET_HEALED)
	CASE_LABEL(MSG_DO_REPAIR)
	CASE_LABEL(MSG_RESUME_CONSTRUCTION)
	CASE_LABEL(MSG_ENTER)
	CASE_LABEL(MSG_DOCK)
	CASE_LABEL(MSG_DO_MOVETO)
	CASE_LABEL(MSG_DO_ATTACKMOVETO)
	CASE_LABEL(MSG_DO_FORCEMOVETO)
	CASE_LABEL(MSG_ADD_WAYPOINT)
	CASE_LABEL(MSG_DO_GUARD_POSITION)
	CASE_LABEL(MSG_DO_GUARD_OBJECT)
	CASE_LABEL(MSG_DO_STOP)
	CASE_LABEL(MSG_DO_SCATTER)
	CASE_LABEL(MSG_INTERNET_HACK)
	CASE_LABEL(MSG_DO_CHEER)

#ifdef ALLOW_SURRENDER
	CASE_LABEL(MSG_DO_SURRENDER)
#endif

	CASE_LABEL(MSG_TOGGLE_OVERCHARGE)

#ifdef ALLOW_SURRENDER
	CASE_LABEL(MSG_RETURN_TO_PRISON)
#endif

	CASE_LABEL(MSG_SWITCH_WEAPONS)
	CASE_LABEL(MSG_CONVERT_TO_CARBOMB)
	CASE_LABEL(MSG_CAPTUREBUILDING)
	CASE_LABEL(MSG_DISABLEVEHICLE_HACK)
	CASE_LABEL(MSG_STEALCASH_HACK)
	CASE_LABEL(MSG_DISABLEBUILDING_HACK)
	CASE_LABEL(MSG_SNIPE_VEHICLE)

#ifdef ALLOW_SURRENDER
	CASE_LABEL(MSG_PICK_UP_PRISONER)
#endif

	CASE_LABEL(MSG_DO_SALVAGE)
	CASE_LABEL(MSG_CLEAR_INGAME_POPUP_MESSAGE)
	CASE_LABEL(MSG_PLACE_BEACON)
	CASE_LABEL(MSG_REMOVE_BEACON)
	CASE_LABEL(MSG_SET_BEACON_TEXT)
	CASE_LABEL(MSG_SET_REPLAY_CAMERA)
	CASE_LABEL(MSG_SELF_DESTRUCT)
	CASE_LABEL(MSG_CREATE_FORMATION)
	CASE_LABEL(MSG_LOGIC_CRC)

#if defined(RTS_DEBUG)
	CASE_LABEL(MSG_DEBUG_KILL_SELECTION)
	CASE_LABEL(MSG_DEBUG_HURT_OBJECT)
	CASE_LABEL(MSG_DEBUG_KILL_OBJECT)
#endif

	CASE_LABEL(MSG_END_NETWORK_MESSAGES)
	CASE_LABEL(MSG_TIMESTAMP)
	CASE_LABEL(MSG_OBJECT_CREATED)
	CASE_LABEL(MSG_OBJECT_DESTROYED)
	CASE_LABEL(MSG_OBJECT_POSITION)
	CASE_LABEL(MSG_OBJECT_ORIENTATION)
	CASE_LABEL(MSG_OBJECT_JOINED_TEAM)
	CASE_LABEL(MSG_SET_MINE_CLEARING_DETAIL)
	CASE_LABEL(MSG_ENABLE_RETALIATION_MODE)
	}

#undef CASE_LABEL
}


//------------------------------------------------------------------------------------------------
// GameMessageList
//

/**
 * Constructor
 */
GameMessageList::GameMessageList( void )
{
	m_firstMessage = 0;
	m_lastMessage = 0;
}

/**
 * Destructor
 */
GameMessageList::~GameMessageList()
{
	// destroy all messages currently on the list
	GameMessage *msg, *nextMsg;
	for( msg = m_firstMessage; msg; msg = nextMsg )
	{
		nextMsg = msg->next();
		// set list ptr to null to avoid it trying to remove itself from the list
		// that we are in the process of nuking...
		msg->friend_setList(NULL);
		deleteInstance(msg);
	}
}

/**
 * Append message to end of message list
 */
void GameMessageList::appendMessage( GameMessage *msg )
{
	msg->friend_setNext(NULL);

	if (m_lastMessage)
	{
		m_lastMessage->friend_setNext(msg);
		msg->friend_setPrev(m_lastMessage);
		m_lastMessage = msg;
	}
	else
	{
		// first message
		m_firstMessage = msg;
		m_lastMessage = msg;
		msg->friend_setPrev(NULL);
	}

	// note containment within message itself
	msg->friend_setList(this);
}

/**
 * Inserts the msg after messageToInsertAfter.
 */
void GameMessageList::insertMessage( GameMessage *msg, GameMessage *messageToInsertAfter )
{
	// First, set msg's next to be messageToInsertAfter's next.
	msg->friend_setNext(messageToInsertAfter->next());

	// Next, set msg's prev to be messageToInsertAfter
	msg->friend_setPrev(messageToInsertAfter);

	// Now update the next message's prev to be msg
	if (msg->next())
		msg->next()->friend_setPrev(msg);
	else	// if the friend wasn't there, then messageToInsertAfter is the last message. Update it to be msg.
		m_lastMessage = msg;

	// Finally, update the messageToInsertAfter's next to be msg
	messageToInsertAfter->friend_setNext(msg);

	// note containment within the message itself
	msg->friend_setList(this);
}

/**
 * Remove given message from the list.
 */
void GameMessageList::removeMessage( GameMessage *msg )
{
	if (msg->next())
		msg->next()->friend_setPrev(msg->prev());
	else
		m_lastMessage = msg->prev();

	if (msg->prev())
		msg->prev()->friend_setNext(msg->next());
	else
		m_firstMessage = msg->next();

	msg->friend_setList(NULL);
}

/**
 * Return whether or not a message of the given type is in the message list
 */
Bool GameMessageList::containsMessageOfType( GameMessage::Type type )
{
	GameMessage *msg = getFirstMessage();
	while (msg) {
		if (msg->getType() == type) {
			return true;
		}
		msg = msg->next();
	}
	return false;
}

//------------------------------------------------------------------------------------------------
// MessageStream
//


/**
 * Constructor
 */
MessageStream::MessageStream( void )
{
	m_firstTranslator = 0;
	m_nextTranslatorID = 1;
}

/**
 * Destructor
 */
MessageStream::~MessageStream()
{
	// destroy all translators
	TranslatorData *trans, *nextTrans;
	for( trans=m_firstTranslator; trans; trans=nextTrans )
	{
		nextTrans = trans->m_next;
		delete trans;
	}
}

/**
	* Init
	*/
void MessageStream::init( void )
{
	// extend
	GameMessageList::init();
}

/**
	* Reset
	*/
void MessageStream::reset( void )
{

	/// @todo Reset the MessageStream

	// extend
	GameMessageList::reset();

}

/**
	* Update
	*/
void MessageStream::update( void )
{
	// extend
	GameMessageList::update();

}

/**
 * Create a new message of the given message type and append it
 * to this message stream.  Return the message such that any data
 * associated with this message can be attached to it.
 */
GameMessage *MessageStream::appendMessage( GameMessage::Type type )
{
	GameMessage *msg = newInstance(GameMessage)( type );

	// add message to list
	GameMessageList::appendMessage( msg );

	return msg;
}

/**
 * Create a new message of the given message type and insert it
 * in the stream after messageToInsertAfter, which must not be NULL.
 */
GameMessage *MessageStream::insertMessage( GameMessage::Type type, GameMessage *messageToInsertAfter )
{
	GameMessage *msg = newInstance(GameMessage)(type);

	GameMessageList::insertMessage(msg, messageToInsertAfter);

	return msg;
}

/**
 * Attach the given Translator to the message stream, and return a
 * unique TranslatorID identifying it.
 * Translators are placed on a list, sorted by priority order.  If two
 * Translators share a priority, they are kept in the same order they
 * were attached.
 */
TranslatorID MessageStream::attachTranslator( GameMessageTranslator *translator,
																							UnsignedInt priority)
{
	MessageStream::TranslatorData *newSS = NEW MessageStream::TranslatorData;
	MessageStream::TranslatorData *ss;

	newSS->m_translator = translator;
	newSS->m_priority = priority;
	newSS->m_id = m_nextTranslatorID++;

	if (m_firstTranslator == NULL)
	{
		// first Translator to be attached
		newSS->m_prev = NULL;
		newSS->m_next = NULL;
		m_firstTranslator = newSS;
		m_lastTranslator = newSS;
		return newSS->m_id;
	}

	// seach the Translator list for our priority location
	for( ss=m_firstTranslator; ss; ss=ss->m_next )
		if (ss->m_priority > newSS->m_priority)
			break;

	if (ss)
	{
		// insert new Translator just BEFORE this one,
		// therefore, m_lastTranslator cannot be affected
		if (ss->m_prev)
		{
			ss->m_prev->m_next = newSS;
			newSS->m_prev = ss->m_prev;
			newSS->m_next = ss;
			ss->m_prev = newSS;
		}
		else
		{
			// insert at head of list
			newSS->m_prev = NULL;
			newSS->m_next = m_firstTranslator;
			m_firstTranslator->m_prev = newSS;
			m_firstTranslator = newSS;
		}
	}
	else
	{
		// append Translator to end of list
		m_lastTranslator->m_next = newSS;
		newSS->m_prev = m_lastTranslator;
		newSS->m_next = NULL;
		m_lastTranslator = newSS;
	}

	return newSS->m_id;
}

/**
	* Find a translator attached to this message stream given the ID
	*/
GameMessageTranslator* MessageStream::findTranslator( TranslatorID id )
{
	MessageStream::TranslatorData *translatorData;

	for( translatorData = m_firstTranslator; translatorData; translatorData = translatorData->m_next )
	{

		if( translatorData->m_id == id )
			return translatorData->m_translator;

	}

	return NULL;

}

/**
 * Remove a previously attached translator.
 */
void MessageStream::removeTranslator( TranslatorID id )
{
	MessageStream::TranslatorData *ss;

	for( ss=m_firstTranslator; ss; ss=ss->m_next )
		if (ss->m_id == id)
		{
			// found the translator - remove it
			if (ss->m_prev)
				ss->m_prev->m_next = ss->m_next;
			else
				m_firstTranslator = ss->m_next;

			if (ss->m_next)
				ss->m_next->m_prev = ss->m_prev;
			else
				m_lastTranslator = ss->m_prev;

			// delete the translator data
			delete ss;

			break;
		}
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
#if defined(RTS_DEBUG)

Bool isInvalidDebugCommand( GameMessage::Type t )
{
	// see if this is something that should be prevented in multiplayer games
	// Don't reject this stuff in skirmish games.
	if (TheGameLogic && !TheGameLogic->isInSkirmishGame() &&
			(TheRecorder && TheRecorder->isMultiplayer() && TheRecorder->getMode() == RECORDERMODETYPE_RECORD))
	{
		switch (t)
		{
		case GameMessage::MSG_META_DEMO_REMOVE_PREREQ:
		case GameMessage::MSG_META_DEMO_INSTANT_BUILD:
		case GameMessage::MSG_META_DEMO_FREE_BUILD:
		case GameMessage::MSG_META_DEMO_GIVE_ALL_SCIENCES:
			// TheSuperHackers @tweak Debug cheats are now multiplayer compatible. Happy cheating Munkees :)
			return false;

		case GameMessage::MSG_META_DEMO_SWITCH_TEAMS:
		case GameMessage::MSG_META_DEMO_SWITCH_TEAMS_BETWEEN_CHINA_USA:
		case GameMessage::MSG_META_DEMO_KILL_ALL_ENEMIES:
		case GameMessage::MSG_META_DEMO_KILL_SELECTION:
		case GameMessage::MSG_META_DEMO_TOGGLE_HURT_ME_MODE:
		case GameMessage::MSG_META_DEMO_TOGGLE_HAND_OF_GOD_MODE:
		case GameMessage::MSG_META_DEMO_TOGGLE_SPECIAL_POWER_DELAYS:
		case GameMessage::MSG_META_DEMO_TIME_OF_DAY:
		case GameMessage::MSG_META_DEMO_LOCK_CAMERA_TO_PLANES:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT1:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT2:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT3:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT4:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT5:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT6:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT7:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT8:
		case GameMessage::MSG_META_DEMO_RUNSCRIPT9:
		case GameMessage::MSG_META_DEMO_ENSHROUD:
		case GameMessage::MSG_META_DEMO_DESHROUD:
		case GameMessage::MSG_META_DEBUG_GIVE_VETERANCY:
		case GameMessage::MSG_META_DEBUG_TAKE_VETERANCY:
		case GameMessage::MSG_META_DEMO_ADD_CASH:
		case GameMessage::MSG_META_DEBUG_INCR_ANIM_SKATE_SPEED:
		case GameMessage::MSG_META_DEBUG_DECR_ANIM_SKATE_SPEED:
		case GameMessage::MSG_META_DEBUG_CYCLE_EXTENT_TYPE:
		case GameMessage::MSG_META_DEMO_TOGGLE_RENDER:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_MAJOR:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_MAJOR_BIG:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_MAJOR:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_MAJOR_BIG:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_MINOR:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_MINOR_BIG:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_MINOR:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_MINOR_BIG:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_HEIGHT:
		case GameMessage::MSG_META_DEBUG_INCREASE_EXTENT_HEIGHT_BIG:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_HEIGHT:
		case GameMessage::MSG_META_DEBUG_DECREASE_EXTENT_HEIGHT_BIG:
		case GameMessage::MSG_META_DEMO_KILL_AREA_SELECTION:
		case GameMessage::MSG_DEBUG_KILL_SELECTION:
		case GameMessage::MSG_DEBUG_HURT_OBJECT:
		case GameMessage::MSG_DEBUG_KILL_OBJECT:
		case GameMessage::MSG_META_DEMO_GIVE_SCIENCEPURCHASEPOINTS:
		case GameMessage::MSG_META_DEMO_GIVE_RANKLEVEL:
		case GameMessage::MSG_META_DEMO_TAKE_RANKLEVEL:
		case GameMessage::MSG_META_DEBUG_WIN:

			return true;
		}
	}
	return false;
}
#endif

/**
 * Propagate messages thru attached Translators, invoking each Translator's
 * callback for each message in the stream.
 * Once all Translators have evaluated the message stream, all messages
 * in the stream are destroyed.
 */
void MessageStream::propagateMessages( void )
{
	MessageStream::TranslatorData *ss;
	GameMessage *msg, *next;

	// process each Translator
	for( ss=m_firstTranslator; ss; ss=ss->m_next )
	{
		for( msg=m_firstMessage; msg; msg=next )
		{
			if (ss->m_translator
#if defined(RTS_DEBUG)
				&& !isInvalidDebugCommand(msg->getType())
#endif
				)
			{
				GameMessageDisposition disp = ss->m_translator->translateGameMessage(msg);
				next = msg->next();
				if (disp == DESTROY_MESSAGE)
				{
					deleteInstance(msg);
				}
			}
			else
			{
				next = msg->next();
			}
		}
	}


	// transfer all messages that reached the end of the stream to TheCommandList
	TheCommandList->appendMessageList( m_firstMessage );

	// clear the stream
	m_firstMessage = NULL;
	m_lastMessage = NULL;

}


//------------------------------------------------------------------------------------------------
// CommandList
//

/**
 * Constructor
 */
CommandList::CommandList( void )
{
}

/**
 * Destructor
 */
CommandList::~CommandList()
{
	destroyAllMessages();
}

/**
	* Init
	*/
void CommandList::init( void )
{

	// extend
	GameMessageList::init();

}

/**
	* Destroy all messages on the list, and reset list to empty
	*/
void CommandList::reset( void )
{

	// extend
	GameMessageList::reset();

	// destroy all messages
	destroyAllMessages();

}

/**
	* Update
	*/
void CommandList::update( void )
{

	// extend
	GameMessageList::update();

}

/**
	* Destroy all messages on the command list, this will get called from the
	* destructor and reset methods, DO NOT throw exceptions
	*/
void CommandList::destroyAllMessages( void )
{
	GameMessage *msg, *next;

	for( msg=m_firstMessage; msg; msg=next )
	{
		next = msg->next();
		deleteInstance(msg);
	}

	m_firstMessage = NULL;
	m_lastMessage = NULL;

}

/**
 * Adds messages to the end of TheCommandList.
 * Primarily used by TheMessageStream to put the final messages that reach the end of the
 * stream on TheCommandList. Since TheGameClient will update faster than TheNetwork
 * and TheGameLogic, messages will accumulate on this list.
 */
void CommandList::appendMessageList( GameMessage *list )
{
	GameMessage *msg, *next;

	for( msg = list; msg; msg = next )
	{
		next = msg->next();
		appendMessage( msg );
	}
}

//-----------------------------------------------------------------------------
/**
 * Given an "anchor" point and the current mouse position (dest),
 * construct a valid 2D bounding region.
 */
void buildRegion( const ICoord2D *anchor, const ICoord2D *dest, IRegion2D *region )
{
	// build rectangular region defined by the drag selection
	if (anchor->x < dest->x)
	{
		region->lo.x = anchor->x;
		region->hi.x = dest->x;
	}
	else
	{
		region->lo.x = dest->x;
		region->hi.x = anchor->x;
	}

	if (anchor->y < dest->y)
	{
		region->lo.y = anchor->y;
		region->hi.y = dest->y;
	}
	else
	{
		region->lo.y = dest->y;
		region->hi.y = anchor->y;
	}
}
