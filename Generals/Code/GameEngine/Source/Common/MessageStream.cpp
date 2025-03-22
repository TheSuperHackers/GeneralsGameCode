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
		arg->deleteInstance();
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

AsciiString GameMessage::getCommandAsAsciiString( void )
{
	return getCommandTypeAsAsciiString(m_type);
}

AsciiString GameMessage::getCommandTypeAsAsciiString(GameMessage::Type t)
{
#define CHECK_IF(x) if (t == x) { return #x; }
	AsciiString commandName = "UnknownMessage";
	if (t >= GameMessage::MSG_COUNT)
	{
		commandName = "Invalid command";
	}
	CHECK_IF(MSG_INVALID)
	CHECK_IF(MSG_FRAME_TICK)
	CHECK_IF(MSG_RAW_MOUSE_BEGIN)
	CHECK_IF(MSG_RAW_MOUSE_POSITION)
	CHECK_IF(MSG_RAW_MOUSE_LEFT_BUTTON_DOWN)
	CHECK_IF(MSG_RAW_MOUSE_LEFT_DOUBLE_CLICK)
	CHECK_IF(MSG_RAW_MOUSE_LEFT_BUTTON_UP)
	CHECK_IF(MSG_RAW_MOUSE_LEFT_CLICK)
	CHECK_IF(MSG_RAW_MOUSE_LEFT_DRAG)
	CHECK_IF(MSG_RAW_MOUSE_MIDDLE_BUTTON_DOWN)
	CHECK_IF(MSG_RAW_MOUSE_MIDDLE_DOUBLE_CLICK)
	CHECK_IF(MSG_RAW_MOUSE_MIDDLE_BUTTON_UP)
	CHECK_IF(MSG_RAW_MOUSE_MIDDLE_DRAG)
	CHECK_IF(MSG_RAW_MOUSE_RIGHT_BUTTON_DOWN)
	CHECK_IF(MSG_RAW_MOUSE_RIGHT_DOUBLE_CLICK)
	CHECK_IF(MSG_RAW_MOUSE_RIGHT_BUTTON_UP)
	CHECK_IF(MSG_RAW_MOUSE_RIGHT_DRAG)
	CHECK_IF(MSG_RAW_MOUSE_WHEEL)
	CHECK_IF(MSG_RAW_MOUSE_END)
	CHECK_IF(MSG_RAW_KEY_DOWN)
	CHECK_IF(MSG_RAW_KEY_UP)
	CHECK_IF(MSG_MOUSE_LEFT_CLICK)
	CHECK_IF(MSG_MOUSE_LEFT_DOUBLE_CLICK)
	CHECK_IF(MSG_MOUSE_MIDDLE_CLICK)
	CHECK_IF(MSG_MOUSE_MIDDLE_DOUBLE_CLICK)
	CHECK_IF(MSG_MOUSE_RIGHT_CLICK)
	CHECK_IF(MSG_MOUSE_RIGHT_DOUBLE_CLICK)
	CHECK_IF(MSG_CLEAR_GAME_DATA)
	CHECK_IF(MSG_NEW_GAME)
	CHECK_IF(MSG_BEGIN_META_MESSAGES)
	CHECK_IF(MSG_META_SAVE_VIEW1)
	CHECK_IF(MSG_META_SAVE_VIEW2)
	CHECK_IF(MSG_META_SAVE_VIEW3)
	CHECK_IF(MSG_META_SAVE_VIEW4)
	CHECK_IF(MSG_META_SAVE_VIEW5)
	CHECK_IF(MSG_META_SAVE_VIEW6)
	CHECK_IF(MSG_META_SAVE_VIEW7)
	CHECK_IF(MSG_META_SAVE_VIEW8)
	CHECK_IF(MSG_META_VIEW_VIEW1)
	CHECK_IF(MSG_META_VIEW_VIEW2)
	CHECK_IF(MSG_META_VIEW_VIEW3)
	CHECK_IF(MSG_META_VIEW_VIEW4)
	CHECK_IF(MSG_META_VIEW_VIEW5)
	CHECK_IF(MSG_META_VIEW_VIEW6)
	CHECK_IF(MSG_META_VIEW_VIEW7)
	CHECK_IF(MSG_META_VIEW_VIEW8)
	CHECK_IF(MSG_META_CREATE_TEAM0)
	CHECK_IF(MSG_META_CREATE_TEAM1)
	CHECK_IF(MSG_META_CREATE_TEAM2)
	CHECK_IF(MSG_META_CREATE_TEAM3)
	CHECK_IF(MSG_META_CREATE_TEAM4)
	CHECK_IF(MSG_META_CREATE_TEAM5)
	CHECK_IF(MSG_META_CREATE_TEAM6)
	CHECK_IF(MSG_META_CREATE_TEAM7)
	CHECK_IF(MSG_META_CREATE_TEAM8)
	CHECK_IF(MSG_META_CREATE_TEAM9)
	CHECK_IF(MSG_META_SELECT_TEAM0)
	CHECK_IF(MSG_META_SELECT_TEAM1)
	CHECK_IF(MSG_META_SELECT_TEAM2)
	CHECK_IF(MSG_META_SELECT_TEAM3)
	CHECK_IF(MSG_META_SELECT_TEAM4)
	CHECK_IF(MSG_META_SELECT_TEAM5)
	CHECK_IF(MSG_META_SELECT_TEAM6)
	CHECK_IF(MSG_META_SELECT_TEAM7)
	CHECK_IF(MSG_META_SELECT_TEAM8)
	CHECK_IF(MSG_META_SELECT_TEAM9)
	CHECK_IF(MSG_META_ADD_TEAM0)
	CHECK_IF(MSG_META_ADD_TEAM1)
	CHECK_IF(MSG_META_ADD_TEAM2)
	CHECK_IF(MSG_META_ADD_TEAM3)
	CHECK_IF(MSG_META_ADD_TEAM4)
	CHECK_IF(MSG_META_ADD_TEAM5)
	CHECK_IF(MSG_META_ADD_TEAM6)
	CHECK_IF(MSG_META_ADD_TEAM7)
	CHECK_IF(MSG_META_ADD_TEAM8)
	CHECK_IF(MSG_META_ADD_TEAM9)
	CHECK_IF(MSG_META_VIEW_TEAM0)
	CHECK_IF(MSG_META_VIEW_TEAM1)
	CHECK_IF(MSG_META_VIEW_TEAM2)
	CHECK_IF(MSG_META_VIEW_TEAM3)
	CHECK_IF(MSG_META_VIEW_TEAM4)
	CHECK_IF(MSG_META_VIEW_TEAM5)
	CHECK_IF(MSG_META_VIEW_TEAM6)
	CHECK_IF(MSG_META_VIEW_TEAM7)
	CHECK_IF(MSG_META_VIEW_TEAM8)
	CHECK_IF(MSG_META_VIEW_TEAM9)
	CHECK_IF(MSG_META_SELECT_MATCHING_UNITS)
	CHECK_IF(MSG_META_SELECT_NEXT_UNIT)
	CHECK_IF(MSG_META_SELECT_PREV_UNIT)
	CHECK_IF(MSG_META_SELECT_NEXT_WORKER)
	CHECK_IF(MSG_META_SELECT_PREV_WORKER)
	CHECK_IF(MSG_META_VIEW_COMMAND_CENTER)
	CHECK_IF(MSG_META_VIEW_LAST_RADAR_EVENT)
	CHECK_IF(MSG_META_SELECT_HERO)
	CHECK_IF(MSG_META_SELECT_ALL)
	CHECK_IF(MSG_META_SCATTER)
	CHECK_IF(MSG_META_STOP)
	CHECK_IF(MSG_META_DEPLOY)
	CHECK_IF(MSG_META_CREATE_FORMATION)
	CHECK_IF(MSG_META_FOLLOW)
	CHECK_IF(MSG_META_CHAT_PLAYERS)
	CHECK_IF(MSG_META_CHAT_ALLIES)
	CHECK_IF(MSG_META_CHAT_EVERYONE)
	CHECK_IF(MSG_META_DIPLOMACY)
	CHECK_IF(MSG_META_OPTIONS)
#if defined(_DEBUG) || defined(_INTERNAL)
	CHECK_IF(MSG_META_HELP)
#endif
	CHECK_IF(MSG_META_TOGGLE_LOWER_DETAILS)
	CHECK_IF(MSG_META_TOGGLE_CONTROL_BAR)
	CHECK_IF(MSG_META_BEGIN_PATH_BUILD)
	CHECK_IF(MSG_META_END_PATH_BUILD)
	CHECK_IF(MSG_META_BEGIN_FORCEATTACK)
	CHECK_IF(MSG_META_END_FORCEATTACK)
	CHECK_IF(MSG_META_BEGIN_FORCEMOVE)
	CHECK_IF(MSG_META_END_FORCEMOVE)
	CHECK_IF(MSG_META_BEGIN_WAYPOINTS)
	CHECK_IF(MSG_META_END_WAYPOINTS)
	CHECK_IF(MSG_META_BEGIN_PREFER_SELECTION)
	CHECK_IF(MSG_META_END_PREFER_SELECTION)
	CHECK_IF(MSG_META_TAKE_SCREENSHOT)
	CHECK_IF(MSG_META_ALL_CHEER)
	CHECK_IF(MSG_META_TOGGLE_ATTACKMOVE)
	CHECK_IF(MSG_META_BEGIN_CAMERA_ROTATE_LEFT)
	CHECK_IF(MSG_META_END_CAMERA_ROTATE_LEFT)
	CHECK_IF(MSG_META_BEGIN_CAMERA_ROTATE_RIGHT)
	CHECK_IF(MSG_META_END_CAMERA_ROTATE_RIGHT)
	CHECK_IF(MSG_META_BEGIN_CAMERA_ZOOM_IN)
	CHECK_IF(MSG_META_END_CAMERA_ZOOM_IN)
	CHECK_IF(MSG_META_BEGIN_CAMERA_ZOOM_OUT)
	CHECK_IF(MSG_META_END_CAMERA_ZOOM_OUT)
	CHECK_IF(MSG_META_CAMERA_RESET)
#if defined(_DEBUG) || defined(_INTERNAL)
	CHECK_IF(MSG_META_DEMO_TOGGLE_BEHIND_BUILDINGS)
	CHECK_IF(MSG_META_DEMO_TOGGLE_LETTERBOX)
	CHECK_IF(MSG_META_DEMO_TOGGLE_MESSAGE_TEXT)
	CHECK_IF(MSG_META_DEMO_LOD_DECREASE)
	CHECK_IF(MSG_META_DEMO_LOD_INCREASE)
	CHECK_IF(MSG_META_DEMO_TOGGLE_ZOOM_LOCK)
	CHECK_IF(MSG_META_DEMO_PLAY_CAMEO_MOVIE)
	CHECK_IF(MSG_META_DEMO_INSTANT_QUIT)
	CHECK_IF(MSG_META_DEMO_TOGGLE_SPECIAL_POWER_DELAYS)
	CHECK_IF(MSG_META_DEMO_BATTLE_CRY)
	CHECK_IF(MSG_META_DEMO_SWITCH_TEAMS)
	CHECK_IF(MSG_META_DEMO_SWITCH_TEAMS_BETWEEN_CHINA_USA)
	CHECK_IF(MSG_META_DEMO_TOGGLE_PARTICLEDEBUG)
	CHECK_IF(MSG_META_DEMO_TOGGLE_SHADOW_VOLUMES)
	CHECK_IF(MSG_META_DEMO_TOGGLE_FOGOFWAR)
	CHECK_IF(MSG_META_DEMO_KILL_ALL_ENEMIES)
	CHECK_IF(MSG_META_DEMO_KILL_SELECTION)
	CHECK_IF(MSG_META_DEMO_TOGGLE_HURT_ME_MODE)
	CHECK_IF(MSG_META_DEMO_TOGGLE_HAND_OF_GOD_MODE)
	CHECK_IF(MSG_META_DEMO_DEBUG_SELECTION)
	CHECK_IF(MSG_META_DEMO_LOCK_CAMERA_TO_SELECTION)
	CHECK_IF(MSG_META_DEMO_TOGGLE_SOUND)
	CHECK_IF(MSG_META_DEMO_TOGGLE_TRACKMARKS)
	CHECK_IF(MSG_META_DEMO_TOGGLE_WATERPLANE)
	CHECK_IF(MSG_META_DEMO_TIME_OF_DAY)
	CHECK_IF(MSG_META_DEMO_TOGGLE_MUSIC)
	CHECK_IF(MSG_META_DEMO_MUSIC_NEXT_TRACK)
	CHECK_IF(MSG_META_DEMO_MUSIC_PREV_TRACK)
	CHECK_IF(MSG_META_DEMO_NEXT_OBJECTIVE_MOVIE)
	CHECK_IF(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE1)
	CHECK_IF(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE2)
	CHECK_IF(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE3)
	CHECK_IF(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE4)
	CHECK_IF(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE5)
	CHECK_IF(MSG_META_DEMO_PLAY_OBJECTIVE_MOVIE6)
	CHECK_IF(MSG_META_DEMO_BEGIN_ADJUST_PITCH)
	CHECK_IF(MSG_META_DEMO_END_ADJUST_PITCH)
	CHECK_IF(MSG_META_DEMO_BEGIN_ADJUST_FOV)
	CHECK_IF(MSG_META_DEMO_END_ADJUST_FOV)
	CHECK_IF(MSG_META_DEMO_LOCK_CAMERA_TO_PLANES)
	CHECK_IF(MSG_META_DEMO_REMOVE_PREREQ)
	CHECK_IF(MSG_META_DEMO_INSTANT_BUILD)
	CHECK_IF(MSG_META_DEMO_FREE_BUILD)
	CHECK_IF(MSG_META_DEMO_RUNSCRIPT1)
	CHECK_IF(MSG_META_DEMO_RUNSCRIPT2)
	CHECK_IF(MSG_META_DEMO_RUNSCRIPT3)
	CHECK_IF(MSG_META_DEMO_RUNSCRIPT4)
	CHECK_IF(MSG_META_DEMO_RUNSCRIPT5)
	CHECK_IF(MSG_META_DEMO_RUNSCRIPT6)
	CHECK_IF(MSG_META_DEMO_RUNSCRIPT7)
	CHECK_IF(MSG_META_DEMO_RUNSCRIPT8)
	CHECK_IF(MSG_META_DEMO_RUNSCRIPT9)
	CHECK_IF(MSG_META_DEMO_ENSHROUD)
	CHECK_IF(MSG_META_DEMO_DESHROUD)
	CHECK_IF(MSG_META_DEBUG_SHOW_EXTENTS)
	CHECK_IF(MSG_META_DEBUG_SHOW_HEALTH)
	CHECK_IF(MSG_META_DEBUG_GIVE_VETERANCY)
	CHECK_IF(MSG_META_DEBUG_TAKE_VETERANCY)
	CHECK_IF(MSG_META_DEMO_TOGGLE_AI_DEBUG)
	CHECK_IF(MSG_META_DEMO_TOGGLE_CAMERA_DEBUG)
	CHECK_IF(MSG_META_DEMO_TOGGLE_AVI)
	CHECK_IF(MSG_META_DEMO_TOGGLE_BW_VIEW)
	CHECK_IF(MSG_META_DEMO_TOGGLE_RED_VIEW)
	CHECK_IF(MSG_META_DEMO_TOGGLE_GREEN_VIEW)
	CHECK_IF(MSG_META_DEMO_TOGGLE_MOTION_BLUR_ZOOM)
	CHECK_IF(MSG_META_DEMO_TOGGLE_MILITARY_SUBTITLES)
	CHECK_IF(MSG_META_DEMO_ADD_CASH)
#ifdef ALLOW_SURRENDER
	CHECK_IF(MSG_META_DEMO_TEST_SURRENDER)
#endif
	CHECK_IF(MSG_META_DEMO_TOGGLE_RENDER)
	CHECK_IF(MSG_META_DEMO_KILL_AREA_SELECTION)
	CHECK_IF(MSG_META_DEMO_CYCLE_LOD_LEVEL)
	CHECK_IF(MSG_META_DEBUG_INCR_ANIM_SKATE_SPEED)
	CHECK_IF(MSG_META_DEBUG_DECR_ANIM_SKATE_SPEED)
	CHECK_IF(MSG_META_DEBUG_CYCLE_EXTENT_TYPE)
	CHECK_IF(MSG_META_DEBUG_INCREASE_EXTENT_MAJOR)
	CHECK_IF(MSG_META_DEBUG_INCREASE_EXTENT_MAJOR_BIG)
	CHECK_IF(MSG_META_DEBUG_DECREASE_EXTENT_MAJOR)
	CHECK_IF(MSG_META_DEBUG_DECREASE_EXTENT_MAJOR_BIG)
	CHECK_IF(MSG_META_DEBUG_INCREASE_EXTENT_MINOR)
	CHECK_IF(MSG_META_DEBUG_INCREASE_EXTENT_MINOR_BIG)
	CHECK_IF(MSG_META_DEBUG_DECREASE_EXTENT_MINOR)
	CHECK_IF(MSG_META_DEBUG_DECREASE_EXTENT_MINOR_BIG)
	CHECK_IF(MSG_META_DEBUG_INCREASE_EXTENT_HEIGHT)
	CHECK_IF(MSG_META_DEBUG_INCREASE_EXTENT_HEIGHT_BIG)
	CHECK_IF(MSG_META_DEBUG_DECREASE_EXTENT_HEIGHT)
	CHECK_IF(MSG_META_DEBUG_DECREASE_EXTENT_HEIGHT_BIG)
	CHECK_IF(MSG_META_DEBUG_VTUNE_ON)
	CHECK_IF(MSG_META_DEBUG_VTUNE_OFF)
	CHECK_IF(MSG_META_DEBUG_TOGGLE_FEATHER_WATER)
	CHECK_IF(MSG_META_DEBUG_DUMP_ASSETS)
	CHECK_IF(MSG_NO_DRAW)
	CHECK_IF(MSG_META_DEMO_TOGGLE_METRICS)
	CHECK_IF(MSG_META_DEMO_TOGGLE_PROJECTILEDEBUG)
	CHECK_IF(MSG_META_DEMO_TOGGLE_VISIONDEBUG)
	CHECK_IF(MSG_META_DEMO_TOGGLE_THREATDEBUG)
	CHECK_IF(MSG_META_DEMO_TOGGLE_CASHMAPDEBUG)
	CHECK_IF(MSG_META_DEMO_TOGGLE_GRAPHICALFRAMERATEBAR)
	CHECK_IF(MSG_META_DEMO_GIVE_ALL_SCIENCES)
	CHECK_IF(MSG_META_DEMO_GIVE_RANKLEVEL)
	CHECK_IF(MSG_META_DEMO_TAKE_RANKLEVEL)
	CHECK_IF(MSG_META_DEMO_GIVE_SCIENCEPURCHASEPOINTS)
	CHECK_IF(MSG_META_DEBUG_TOGGLE_NETWORK)
	CHECK_IF(MSG_META_DEBUG_DUMP_PLAYER_OBJECTS)
	CHECK_IF(MSG_META_DEBUG_DUMP_ALL_PLAYER_OBJECTS)
	CHECK_IF(MSG_META_DEBUG_WIN)
	CHECK_IF(MSG_META_DEMO_TOGGLE_DEBUG_STATS)
#endif // defined(_DEBUG) || defined(_INTERNAL)


#if defined(_INTERNAL) || defined(_DEBUG) || defined(_PLAYTEST)
	CHECK_IF(MSG_META_DEMO_TOGGLE_AUDIODEBUG)
#endif//defined(_INTERNAL) || defined(_DEBUG) || defined(_PLAYTEST)
#ifdef DUMP_PERF_STATS
	CHECK_IF(MSG_META_DEMO_PERFORM_STATISTICAL_DUMP)
#endif//DUMP_PERF_STATS
	CHECK_IF(MSG_META_PLACE_BEACON)
	CHECK_IF(MSG_META_REMOVE_BEACON)
	CHECK_IF(MSG_END_META_MESSAGES)
	CHECK_IF(MSG_MOUSEOVER_DRAWABLE_HINT)
	CHECK_IF(MSG_MOUSEOVER_LOCATION_HINT)
	CHECK_IF(MSG_VALID_GUICOMMAND_HINT)
	CHECK_IF(MSG_INVALID_GUICOMMAND_HINT)
	CHECK_IF(MSG_AREA_SELECTION_HINT)
	CHECK_IF(MSG_DO_ATTACK_OBJECT_HINT)
	CHECK_IF(MSG_DO_ATTACK_OBJECT_AFTER_MOVING_HINT)
	CHECK_IF(MSG_DO_FORCE_ATTACK_OBJECT_HINT)
	CHECK_IF(MSG_DO_FORCE_ATTACK_GROUND_HINT)
	CHECK_IF(MSG_GET_REPAIRED_HINT)
	CHECK_IF(MSG_GET_HEALED_HINT)
	CHECK_IF(MSG_DO_REPAIR_HINT)
	CHECK_IF(MSG_RESUME_CONSTRUCTION_HINT)
	CHECK_IF(MSG_ENTER_HINT)
	CHECK_IF(MSG_DOCK_HINT)
	CHECK_IF(MSG_DO_MOVETO_HINT)
	CHECK_IF(MSG_DO_ATTACKMOVETO_HINT)
	CHECK_IF(MSG_ADD_WAYPOINT_HINT)
	CHECK_IF(MSG_HIJACK_HINT)
	CHECK_IF(MSG_FIREBOMB_HINT)
	CHECK_IF(MSG_CONVERT_TO_CARBOMB_HINT)
	CHECK_IF(MSG_CAPTUREBUILDING_HINT)
	CHECK_IF(MSG_HACK_HINT)
#ifdef ALLOW_SURRENDER
	CHECK_IF(MSG_PICK_UP_PRISONER_HINT)
#endif
	CHECK_IF(MSG_SNIPE_VEHICLE_HINT)
	CHECK_IF(MSG_DEFECTOR_HINT)
	CHECK_IF(MSG_SET_RALLY_POINT_HINT)
	CHECK_IF(MSG_DO_SALVAGE_HINT)
	CHECK_IF(MSG_DO_INVALID_HINT)
	CHECK_IF(MSG_BEGIN_NETWORK_MESSAGES)
	CHECK_IF(MSG_CREATE_SELECTED_GROUP)
	CHECK_IF(MSG_CREATE_SELECTED_GROUP_NO_SOUND)
	CHECK_IF(MSG_DESTROY_SELECTED_GROUP)
	CHECK_IF(MSG_REMOVE_FROM_SELECTED_GROUP)
	CHECK_IF(MSG_SELECTED_GROUP_COMMAND)
	CHECK_IF(MSG_CREATE_TEAM0)
	CHECK_IF(MSG_CREATE_TEAM1)
	CHECK_IF(MSG_CREATE_TEAM2)
	CHECK_IF(MSG_CREATE_TEAM3)
	CHECK_IF(MSG_CREATE_TEAM4)
	CHECK_IF(MSG_CREATE_TEAM5)
	CHECK_IF(MSG_CREATE_TEAM6)
	CHECK_IF(MSG_CREATE_TEAM7)
	CHECK_IF(MSG_CREATE_TEAM8)
	CHECK_IF(MSG_CREATE_TEAM9)
	CHECK_IF(MSG_SELECT_TEAM0)
	CHECK_IF(MSG_SELECT_TEAM1)
	CHECK_IF(MSG_SELECT_TEAM2)
	CHECK_IF(MSG_SELECT_TEAM3)
	CHECK_IF(MSG_SELECT_TEAM4)
	CHECK_IF(MSG_SELECT_TEAM5)
	CHECK_IF(MSG_SELECT_TEAM6)
	CHECK_IF(MSG_SELECT_TEAM7)
	CHECK_IF(MSG_SELECT_TEAM8)
	CHECK_IF(MSG_SELECT_TEAM9)
	CHECK_IF(MSG_ADD_TEAM0)
	CHECK_IF(MSG_ADD_TEAM1)
	CHECK_IF(MSG_ADD_TEAM2)
	CHECK_IF(MSG_ADD_TEAM3)
	CHECK_IF(MSG_ADD_TEAM4)
	CHECK_IF(MSG_ADD_TEAM5)
	CHECK_IF(MSG_ADD_TEAM6)
	CHECK_IF(MSG_ADD_TEAM7)
	CHECK_IF(MSG_ADD_TEAM8)
	CHECK_IF(MSG_ADD_TEAM9)
	CHECK_IF(MSG_DO_ATTACKSQUAD)
	CHECK_IF(MSG_DO_WEAPON)
	CHECK_IF(MSG_DO_WEAPON_AT_LOCATION)
	CHECK_IF(MSG_DO_WEAPON_AT_OBJECT)
	CHECK_IF(MSG_DO_SPECIAL_POWER)
	CHECK_IF(MSG_DO_SPECIAL_POWER_AT_LOCATION)
	CHECK_IF(MSG_DO_SPECIAL_POWER_AT_OBJECT)
	CHECK_IF(MSG_SET_RALLY_POINT)
	CHECK_IF(MSG_PURCHASE_SCIENCE)
	CHECK_IF(MSG_QUEUE_UPGRADE)
	CHECK_IF(MSG_CANCEL_UPGRADE)
	CHECK_IF(MSG_QUEUE_UNIT_CREATE)
	CHECK_IF(MSG_CANCEL_UNIT_CREATE)
	CHECK_IF(MSG_DOZER_CONSTRUCT)
	CHECK_IF(MSG_DOZER_CONSTRUCT_LINE)
	CHECK_IF(MSG_DOZER_CANCEL_CONSTRUCT)
	CHECK_IF(MSG_SELL)
	CHECK_IF(MSG_EXIT)
	CHECK_IF(MSG_EVACUATE)
	CHECK_IF(MSG_EXECUTE_RAILED_TRANSPORT)
	CHECK_IF(MSG_COMBATDROP_AT_LOCATION)
	CHECK_IF(MSG_COMBATDROP_AT_OBJECT)
	CHECK_IF(MSG_AREA_SELECTION)
	CHECK_IF(MSG_DO_ATTACK_OBJECT)
	CHECK_IF(MSG_DO_FORCE_ATTACK_OBJECT)
	CHECK_IF(MSG_DO_FORCE_ATTACK_GROUND)
	CHECK_IF(MSG_GET_REPAIRED)
	CHECK_IF(MSG_GET_HEALED)
	CHECK_IF(MSG_DO_REPAIR)
	CHECK_IF(MSG_RESUME_CONSTRUCTION)
	CHECK_IF(MSG_ENTER)
	CHECK_IF(MSG_DOCK)
	CHECK_IF(MSG_DO_MOVETO)
	CHECK_IF(MSG_DO_ATTACKMOVETO)
	CHECK_IF(MSG_DO_FORCEMOVETO)
	CHECK_IF(MSG_ADD_WAYPOINT)
	CHECK_IF(MSG_DO_GUARD_POSITION)
	CHECK_IF(MSG_DO_GUARD_OBJECT)
	CHECK_IF(MSG_DO_STOP)
	CHECK_IF(MSG_DO_SCATTER)
	CHECK_IF(MSG_INTERNET_HACK)
	CHECK_IF(MSG_DO_CHEER)
#ifdef ALLOW_SURRENDER
	CHECK_IF(MSG_DO_SURRENDER)
#endif
	CHECK_IF(MSG_TOGGLE_OVERCHARGE)
#ifdef ALLOW_SURRENDER
	CHECK_IF(MSG_RETURN_TO_PRISON)
#endif
	CHECK_IF(MSG_SWITCH_WEAPONS)
	CHECK_IF(MSG_CONVERT_TO_CARBOMB)
	CHECK_IF(MSG_CAPTUREBUILDING)
	CHECK_IF(MSG_DISABLEVEHICLE_HACK)
	CHECK_IF(MSG_STEALCASH_HACK)
	CHECK_IF(MSG_DISABLEBUILDING_HACK)
	CHECK_IF(MSG_SNIPE_VEHICLE)
#ifdef ALLOW_SURRENDER
	CHECK_IF(MSG_PICK_UP_PRISONER)
#endif
	CHECK_IF(MSG_DO_SALVAGE)
	CHECK_IF(MSG_CLEAR_INGAME_POPUP_MESSAGE)
	CHECK_IF(MSG_PLACE_BEACON)
	CHECK_IF(MSG_REMOVE_BEACON)
	CHECK_IF(MSG_SET_BEACON_TEXT)
	CHECK_IF(MSG_SET_REPLAY_CAMERA)
	CHECK_IF(MSG_SELF_DESTRUCT)
	CHECK_IF(MSG_CREATE_FORMATION)
	CHECK_IF(MSG_LOGIC_CRC)
#if defined(_DEBUG) || defined(_INTERNAL)
	CHECK_IF(MSG_DEBUG_KILL_SELECTION)
	CHECK_IF(MSG_DEBUG_HURT_OBJECT)
	CHECK_IF(MSG_DEBUG_KILL_OBJECT)
#endif
	CHECK_IF(MSG_END_NETWORK_MESSAGES)
	CHECK_IF(MSG_TIMESTAMP)
	CHECK_IF(MSG_OBJECT_CREATED)
	CHECK_IF(MSG_OBJECT_DESTROYED)
	CHECK_IF(MSG_OBJECT_POSITION)
	CHECK_IF(MSG_OBJECT_ORIENTATION)
	CHECK_IF(MSG_OBJECT_JOINED_TEAM)
	CHECK_IF(MSG_SET_MINE_CLEARING_DETAIL)
	return commandName;
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
		msg->deleteInstance();
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
#if defined(_DEBUG) || defined(_INTERNAL)

Bool isInvalidDebugCommand( GameMessage::Type t )
{
	// see if this is something that should be prevented in multiplayer games
	// Don't reject this stuff in skirmish games.
	if (TheGameLogic && !TheGameLogic->isInSkirmishGame() && 
			(TheRecorder && TheRecorder->isMultiplayer() && TheRecorder->getMode() == RECORDERMODETYPE_RECORD))
	{
		switch (t)
		{
		case GameMessage::MSG_META_DEMO_INSTANT_QUIT:
		case GameMessage::MSG_META_DEMO_SWITCH_TEAMS:
		case GameMessage::MSG_META_DEMO_SWITCH_TEAMS_BETWEEN_CHINA_USA:
		case GameMessage::MSG_META_DEMO_KILL_ALL_ENEMIES:
		case GameMessage::MSG_META_DEMO_KILL_SELECTION:
		case GameMessage::MSG_META_DEMO_TOGGLE_HURT_ME_MODE:
		case GameMessage::MSG_META_DEMO_TOGGLE_HAND_OF_GOD_MODE:
		case GameMessage::MSG_META_DEMO_TOGGLE_SPECIAL_POWER_DELAYS:
		case GameMessage::MSG_META_DEMO_TIME_OF_DAY:
		case GameMessage::MSG_META_DEMO_LOCK_CAMERA_TO_PLANES:
		case GameMessage::MSG_META_DEMO_REMOVE_PREREQ:
		case GameMessage::MSG_META_DEMO_INSTANT_BUILD:
		case GameMessage::MSG_META_DEMO_FREE_BUILD:
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
//#pragma MESSAGE ("WARNING - DEBUG key in multiplayer!")
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
		case GameMessage::MSG_META_DEMO_GIVE_ALL_SCIENCES:
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
#if defined(_DEBUG) || defined(_INTERNAL)
				&& !isInvalidDebugCommand(msg->getType())
#endif
				)
			{
				GameMessageDisposition disp = ss->m_translator->translateGameMessage(msg);
				next = msg->next();
				if (disp == DESTROY_MESSAGE)
				{
					msg->deleteInstance();
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
		msg->deleteInstance();
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
