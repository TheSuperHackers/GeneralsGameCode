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

// FILE: ScriptEngine.cpp /////////////////////////////////////////////////////////////////////////
// The game scripting engine.  Interprets scripts.
// Author: John Ahlquist, Nov. 2001
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/DataChunk.h"
#include "Common/file.h"
#include "Common/FileSystem.h"
#include "Common/GameEngine.h"
#include "Common/GameState.h"
#include "Common/LatchRestore.h"
#include "Common/MessageStream.h"
#include "Common/PerfTimer.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Team.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameClient/MessageBox.h"
#include "GameClient/Shell.h"
#include "GameClient/View.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/ObjectTypes.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/ScriptActions.h"
#include "GameLogic/ScriptConditions.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/SidesList.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// These are for debugger window
static int st_LastCurrentFrame;
static int st_CurrentFrame;
static Bool st_CanAppCont;
static Bool st_AppIsFast = false;
static void _appendMessage(const AsciiString& str, Bool isTrueMessage = true, Bool shouldPause = false);
static void _adjustVariable(const AsciiString& str, Int value, Bool shouldPause = false);
static void _updateFrameNumber( void );
static HMODULE st_DebugDLL;
// That's it for debugger window

// These are for particle editor
#define DEFINE_PARTICLE_SYSTEM_NAMES 1
#include "GameClient/ParticleSys.h"
#include "Common/MapObject.h"
#include "../../GameEngineDevice/Include/W3DDevice/GameClient/W3DAssetManagerExposed.h"

static void _addUpdatedParticleSystem( AsciiString particleSystemName );
static void _appendAllParticleSystems( void );
static void _appendAllThingTemplates( void );
static int _getEditorBehavior( void );
static int _getNewCurrentParticleCap( void );
static AsciiString _getParticleSystemName( void );
static void _reloadParticleSystemFromINI( AsciiString particleSystemName );
static void _updateAndSetCurrentSystem( void );
extern void _updateAsciiStringParmsFromSystem( ParticleSystemTemplate *particleTemplate );
extern void _updateAsciiStringParmsToSystem( ParticleSystemTemplate *particleTemplate );
static void _updateCurrentParticleCap( void );
static void _updateCurrentParticleCount( void );
static void _updatePanelParameters( ParticleSystemTemplate *particleTemplate );
static void _writeOutINI( void );
extern void _writeSingleParticleSystem( File *out, ParticleSystemTemplate *particleTemplate );
static void _reloadTextures( void );

static HMODULE st_ParticleDLL;
ParticleSystem *st_particleSystem;
Bool st_particleSystemNeedsStopping = FALSE; ///< Set along with st_particleSystem if the particle system has infinite life
#define ARBITRARY_BUFF_SIZE	128
#define FORMAT_STRING "%.2f"
#define FORMAT_STRING_LEADING_STRING		"%s%.2f"
// That's it for particle editor

#if defined(RTS_INTERNAL)
	#define DO_VTUNE_STUFF
#endif

#ifdef DO_VTUNE_STUFF

//typedef __declspec(dllimport) void __cdecl (*VTProc)();
	typedef void (*VTProc)();
	
	static Bool						st_EnableVTune = false;
	static HMODULE				st_vTuneDLL = NULL;
	static VTProc VTPause = NULL;
	static VTProc VTResume = NULL;

	static void _initVTune( void );
	static void _updateVTune ( void );
	static void _cleanUpVTune( void );

#endif


enum { K_SCRIPTS_DATA_VERSION_1 = 1 };
enum { MAX_SPIN_COUNT = 20 };
#define NONE_STRING "<none>"

static const Int FRAMES_TO_SHOW_WIN_LOSE_MESSAGE = 120;

static const Int FRAMES_TO_FADE_IN_AT_START = 33;


//------------------------------------------------------------------------------ Performance Timers 
//#include "Common/PerfMetrics.h"
//#include "Common/PerfTimer.h"

// GLOBALS ////////////////////////////////////////////////////////////////////////////////////////
ScriptEngine *TheScriptEngine = NULL;

/// Local classes 
/// AttackPriorityInfo class

static const Int ATTACK_PRIORITY_DEFAULT = 1;
//-------------------------------------------------------------------------------------------------
/** Ctor */
//-------------------------------------------------------------------------------------------------
AttackPriorityInfo::AttackPriorityInfo() :m_defaultPriority(ATTACK_PRIORITY_DEFAULT), m_priorityMap(NULL) 
{
	m_name.clear();
}

//-------------------------------------------------------------------------------------------------
/** dtor */
//-------------------------------------------------------------------------------------------------
AttackPriorityInfo::~AttackPriorityInfo()
{
	if (m_priorityMap) delete m_priorityMap;
	m_priorityMap = NULL;
}

//-------------------------------------------------------------------------------------------------
/** set a priority for a thing template. */
//-------------------------------------------------------------------------------------------------
void AttackPriorityInfo::setPriority(const ThingTemplate *tThing, Int priority)
{
	if (tThing==NULL) return;
	if (m_priorityMap==NULL) {
		m_priorityMap = NEW AttackPriorityMap;	// STL type, so impractical to use memorypool
	}
	tThing = (const ThingTemplate *)tThing->getFinalOverride();
	Int &thePriority = (*m_priorityMap)[tThing];
	thePriority = priority;
}

/** set a priority for a thing template. */
//-------------------------------------------------------------------------------------------------
Int AttackPriorityInfo::getPriority(const ThingTemplate *tThing) const
{
	Int priority = m_defaultPriority;
	if (tThing==NULL) return priority;
	tThing = (const ThingTemplate *)tThing->getFinalOverride();
	if (m_priorityMap && !m_priorityMap->empty()) {
		AttackPriorityMap::const_iterator it = m_priorityMap->find(tThing);
		if (it != m_priorityMap->end()) 
		{
			priority = (*it).second;
		}
	}
	return priority;
}

#ifdef RTS_DEBUG
/** Dump the info. */
//-------------------------------------------------------------------------------------------------
void AttackPriorityInfo::dumpPriorityInfo(void)
{
	DEBUG_LOG(("Attack priority '%s', default %d\n", m_name.str(), m_defaultPriority));
	if (m_priorityMap==NULL) return;
	for (AttackPriorityMap::const_iterator it = m_priorityMap->begin(); it != m_priorityMap->end(); ++it) {
		const ThingTemplate *tThing = (*it).first;
		Int priority = (*it).second;
		(void)tThing; (void)priority;
		DEBUG_LOG(("  Thing '%s' priority %d\n",tThing->getName().str(), priority));
	}
}
#endif

// ------------------------------------------------------------------------------------------------
/** Reset to default state */
// ------------------------------------------------------------------------------------------------
void AttackPriorityInfo::reset( void )
{

	// clear name just to be clean
	m_name.clear();

	// go back to default priority
	m_defaultPriority = ATTACK_PRIORITY_DEFAULT;

	// delete the priority map if present
	if( m_priorityMap )
	{

		delete m_priorityMap;
		m_priorityMap = NULL;

	}  // end if

}  // end reset

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void AttackPriorityInfo::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void AttackPriorityInfo::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// name
	xfer->xferAsciiString( &m_name );

	xfer->xferInt( &m_defaultPriority );

	//
	// priority map count, note there is question to the following code that is
	// commented out in that we're not sure it returns the correct number of elements
	// in the map.  To be safe, we're getting the count by manually counting the
	// elements ourselves as we iterate through them
	//
	// Please KEEP the below code here for reference
	// UnsignedShort priorityMapCount = m_priorityMap ? m_priorityMap->size() : 0;
	//
	UnsignedShort priorityMapCount = 0;
	if( m_priorityMap )
	{
		AttackPriorityMap::const_iterator it;
		
		for( it = m_priorityMap->begin(); it != m_priorityMap->end(); ++it )
			++priorityMapCount;
	
	}  // end if
	xfer->xferUnsignedShort( &priorityMapCount );

	// priority map
	AsciiString thingTemplateName;
	const ThingTemplate *thingTemplate;
	Int priority;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		if( m_priorityMap )
		{

			// iterate all the entries
			AttackPriorityMap::const_iterator it;
			UnsignedShort count = 0;
			for( it = m_priorityMap->begin(); it != m_priorityMap->end(); ++it )
			{

				// keep a count for sanity
				++count;

				// write thing template name
				thingTemplate = (*it).first;
				thingTemplateName = thingTemplate->getName();
				DEBUG_ASSERTCRASH( thingTemplateName.isEmpty() == FALSE, 
													 ("AttackPriorityInfo::xfer - Writing an empty thing template name\n") );
				xfer->xferAsciiString( &thingTemplateName );

				// write priority
				priority = (*it).second;
				xfer->xferInt( &priority );

			}  // end for i

			// sanity
			DEBUG_ASSERTCRASH( count == priorityMapCount, 
												("AttackPriorityInfo::xfer - Mismatch in priority map size.  Size() method returned '%d' but actual iteration count was '%d'\n",
												 priorityMapCount, count) );

		}  // end if

	}  // end if, save
	else
	{

		// read all entries
		for( UnsignedShort i = 0; i < priorityMapCount; ++i )
		{

			// read thing template name, and get template
			xfer->xferAsciiString( &thingTemplateName );
			thingTemplate = TheThingFactory->findTemplate( thingTemplateName );
			if( thingTemplate == NULL )
			{

				DEBUG_CRASH(( "AttackPriorityInfo::xfer - Unable to find thing template '%s'\n",
											thingTemplateName.str() ));
				throw SC_INVALID_DATA;

			}  // end if

			// read priority
			xfer->xferInt( &priority );

			// set priority (this will allocate the map on the first call as well)
			setPriority( thingTemplate, priority );

		}  // end for

	}  // end else, load

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void AttackPriorityInfo::loadPostProcess( void )
{

}  // end loadPostProcess

// ScriptEngine class

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ScriptEngine::ScriptEngine():
m_numCounters(0),
m_numFlags(0),
m_callingTeam(NULL),
m_callingObject(NULL),
m_conditionTeam(NULL),
m_conditionObject(NULL),
m_currentPlayer(NULL),
m_skirmishHumanPlayer(NULL),
m_fade(FADE_NONE),
m_freezeByScript(FALSE),
m_frameObjectCountChanged(0),
//Added By Sadullah Nader
//Initializations inserted
m_closeWindowTimer(0),
m_curFadeFrame(0),
m_curFadeValue(0.0f),
m_endGameTimer(0),
m_fadeFramesDecrease(0),
m_fadeFramesHold(0),
m_fadeFramesIncrease(0),
m_firstUpdate(TRUE),
m_maxFade(0.0f),
m_minFade(0.0f),
m_numAttackInfo(0),
m_shownMPLocalDefeatWindow(FALSE),
m_objectsShouldReceiveDifficultyBonus(TRUE),
m_ChooseVictimAlwaysUsesNormal(false)
//
{
	st_CanAppCont = true;
	st_LastCurrentFrame = st_CurrentFrame = 0;
	// By default, difficulty should be normal.
	setGlobalDifficulty(DIFFICULTY_NORMAL);

}  // end ScriptEngine

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ScriptEngine::~ScriptEngine()
{
	if (st_DebugDLL) {
		FARPROC proc = GetProcAddress(st_DebugDLL, "DestroyDebugDialog");
		if (proc) {
			proc();
		}

		FreeLibrary(st_DebugDLL);
		st_DebugDLL = NULL;
	}

	if (st_ParticleDLL) {
		FARPROC proc = GetProcAddress(st_ParticleDLL, "DestroyParticleSystemDialog");
		if (proc) {
			proc();
		}

		FreeLibrary(st_ParticleDLL);
		st_ParticleDLL = NULL;
	}

#ifdef DO_VTUNE_STUFF
	_cleanUpVTune();
#endif

	reset(); // just in case.
}  // end ~ScriptEngine

//-------------------------------------------------------------------------------------------------
/** Init */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::init( void )
{
	if (TheGlobalData->m_windowed)
		if (TheGlobalData->m_scriptDebug) {
			st_DebugDLL = LoadLibrary("DebugWindow.dll");
		} else {
			st_DebugDLL = NULL;
		}
		
		if (TheGlobalData->m_particleEdit) {
			st_ParticleDLL = LoadLibrary("ParticleEditor.dll");
		} else {
			st_ParticleDLL = NULL;
		}

		if (st_DebugDLL) {
			FARPROC proc = GetProcAddress(st_DebugDLL, "CreateDebugDialog");
			if (proc) {
				proc();
			}
		}

	if (st_ParticleDLL) {
		FARPROC proc = GetProcAddress(st_ParticleDLL, "CreateParticleSystemDialog");
		if (proc) {
			proc();
		}
	}

#ifdef DO_VTUNE_STUFF
	_initVTune();
#endif

#ifdef SPECIAL_SCRIPT_PROFILING
#ifdef DEBUG_LOGGING
	m_numFrames=0;
	m_totalUpdateTime=0;
	m_maxUpdateTime=0;
#endif
#endif
	
	if (TheScriptActions) {
		TheScriptActions->init();
	}
	if (TheScriptConditions) {
		TheScriptConditions->init();
	}
	/* Recipe for adding an action:
			1. In Scripts.h, add an enum element to enum ScriptActionType just before NUM_ITEMS.
			2. Go to the end of this section of templates, and create a template.
			3. Go to ScriptActions.h and add a protected method.
			4. Go to ScriptActions.cpp, and add your enum to the 
					switch in ScriptActions::executeAction to call your method in 3 above.
	*/

	// Set up the script action templates.
	Template *curTemplate = &m_actionTemplates[ScriptAction::DEBUG_MESSAGE_BOX];
	curTemplate->m_name = "[Scripting] Debug message and pause";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEXT_STRING;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Show debug string and pause: ";

	curTemplate = &m_actionTemplates[ScriptAction::DEBUG_STRING];
	curTemplate->m_name = "[Scripting] Debug string";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEXT_STRING;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Show debug string without pausing: ";

	curTemplate = &m_actionTemplates[ScriptAction::DEBUG_CRASH_BOX];
	curTemplate->m_name = "[Scripting] Display a crash box (debug/internal builds only).";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEXT_STRING;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Display a crash box with the text: ";

	curTemplate = &m_actionTemplates[ScriptAction::SET_FLAG];
	curTemplate->m_name = "[Scripting] Set flag to value";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::FLAG;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set ";
	curTemplate->m_uiStrings[1] = " to ";

	curTemplate = &m_actionTemplates[ScriptAction::SET_COUNTER];
	curTemplate->m_name = "[Scripting] Counter -- set to a value";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set ";
	curTemplate->m_uiStrings[1] = " to ";

	curTemplate = &m_actionTemplates[ScriptAction::SET_TREE_SWAY];
	curTemplate->m_name = "[Map] Set wind sway amount and direction.";
	curTemplate->m_numParameters = 5;
	curTemplate->m_parameters[0] = Parameter::ANGLE;
	curTemplate->m_parameters[1] = Parameter::ANGLE;
	curTemplate->m_parameters[2] = Parameter::ANGLE;
	curTemplate->m_parameters[3] = Parameter::INT;
	curTemplate->m_parameters[4] = Parameter::REAL;
	curTemplate->m_numUiStrings = 6;
	curTemplate->m_uiStrings[0] = "Set wind direction to ";
	curTemplate->m_uiStrings[1] = ", amount to sway ";
	curTemplate->m_uiStrings[2] = ", amount to lean with the wind ";
	curTemplate->m_uiStrings[3] = ", frames to take to sway once ";
	curTemplate->m_uiStrings[4] = ", randomness ";
	curTemplate->m_uiStrings[5] = "(0=lock step, 1=large random variation).";

	curTemplate = &m_actionTemplates[ScriptAction::SET_INFANTRY_LIGHTING_OVERRIDE];
	curTemplate->m_name = "[Map] Infantry Lighting - Set.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set lighting percent on infantry to ";
	curTemplate->m_uiStrings[1] = " (0.0==min, 1.0==normal day, 2.0==max (which is normal night).)";

	curTemplate = &m_actionTemplates[ScriptAction::RESET_INFANTRY_LIGHTING_OVERRIDE];
	curTemplate->m_name = "[Map] Infantry Lighting - Reset.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Reset infantry lighting to the normal setting. 1.0 for the two day states, 2.0 for the two night states. (Look in GamesData.ini)";

	curTemplate = &m_actionTemplates[ScriptAction::QUICKVICTORY];
	curTemplate->m_name = "[User] Announce quick win";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "End game in victory immediately.";

	curTemplate = &m_actionTemplates[ScriptAction::VICTORY];
	curTemplate->m_name = "[User] Announce win";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Announce win.";

	curTemplate = &m_actionTemplates[ScriptAction::DEFEAT];
	curTemplate->m_name = "[User] Announce lose";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Announce lose.";

	curTemplate = &m_actionTemplates[ScriptAction::NO_OP];
	curTemplate->m_name = "[Scripting] Null operation.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Null operation. (Does nothing.)";

	curTemplate = &m_actionTemplates[ScriptAction::SET_TIMER];
	curTemplate->m_name = "[Scripting] Frame countdown timer -- set.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Set timer ";
	curTemplate->m_uiStrings[1] = " to expire in ";
	curTemplate->m_uiStrings[2] = " frames.";

	curTemplate = &m_actionTemplates[ScriptAction::SET_RANDOM_TIMER];
	curTemplate->m_name = "[Scripting] Frame countdown timer -- set random.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Set timer ";
	curTemplate->m_uiStrings[1] = " to expire between ";
	curTemplate->m_uiStrings[2] = " and ";
	curTemplate->m_uiStrings[3] = " frames.";

	curTemplate = &m_actionTemplates[ScriptAction::STOP_TIMER];
	curTemplate->m_name = "[Scripting] Timer -- stop.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Stop timer ";

	curTemplate = &m_actionTemplates[ScriptAction::RESTART_TIMER];
	curTemplate->m_name = "[Scripting] Timer -- restart stopped.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Restart timer ";

	curTemplate = &m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT];
	curTemplate->m_name = "[Multimedia] Play sound effect.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SOUND;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Play ";
	curTemplate->m_uiStrings[1] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::ENABLE_SCRIPT];
	curTemplate->m_name = "[Scripting] Script -- enable.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SCRIPT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Enable ";
	curTemplate->m_uiStrings[1] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::DISABLE_SCRIPT];
	curTemplate->m_name = "[Scripting] Script -- disable.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SCRIPT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Disable ";
	curTemplate->m_uiStrings[1] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::CALL_SUBROUTINE];
	curTemplate->m_name = "[Scripting] Script -- run.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SCRIPT_SUBROUTINE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Run ";
	curTemplate->m_uiStrings[1] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT_AT];
	curTemplate->m_name = "[Multimedia] Play sound effect at waypoint.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SOUND;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Play ";
	curTemplate->m_uiStrings[1] = " at ";
	curTemplate->m_uiStrings[2] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::DAMAGE_MEMBERS_OF_TEAM];
	curTemplate->m_name = "[Team] Damage the members of a team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Damage ";
	curTemplate->m_uiStrings[1] = ", amount=";
	curTemplate->m_uiStrings[2] = " (-1==kill).";

 	curTemplate = &m_actionTemplates[ScriptAction::MOVE_TEAM_TO];
	curTemplate->m_name = "[Team] Set to move to a location.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Move ";
	curTemplate->m_uiStrings[1] = " to ";
	curTemplate->m_uiStrings[2] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS];
	curTemplate->m_name = "[Team] Set to follow a waypoint path.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT_PATH;
	curTemplate->m_parameters[2] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Have ";
	curTemplate->m_uiStrings[1] = " follow ";
	curTemplate->m_uiStrings[2] = " , as a team is ";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT];
	curTemplate->m_name = "[Team] Set to EXACTLY follow a waypoint path.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT_PATH;
	curTemplate->m_parameters[2] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Have ";
	curTemplate->m_uiStrings[1] = " EXACTLY follow ";
	curTemplate->m_uiStrings[2] = " , as a team is ";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_WANDER_IN_PLACE];
	curTemplate->m_name = "[Team] Set to wander around current location.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Have ";
	curTemplate->m_uiStrings[1] = " wander around it's current location.";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_INCREASE_PRIORITY];
	curTemplate->m_name = "[Team] AI - Increase priority by Success Priority Increase amount.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Increase the AI priority for";
	curTemplate->m_uiStrings[1] = "  by its Success Priority Increase amount.";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_DECREASE_PRIORITY];
	curTemplate->m_name = "[Team] AI - Reduce priority by Failure Priority Decrease amount.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Reduce the AI priority for";
	curTemplate->m_uiStrings[1] = "  by its Failure Priority Decrease amount.";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_WANDER];
	curTemplate->m_name = "[Team] Set to follow a waypoint path -- wander.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT_PATH;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Have ";
	curTemplate->m_uiStrings[1] = " wander along ";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_PANIC];
	curTemplate->m_name = "[Team] Set to follow a waypoint path -- panic.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT_PATH;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Have ";
	curTemplate->m_uiStrings[1] = " move in panic along ";

	curTemplate = &m_actionTemplates[ScriptAction::MOVE_NAMED_UNIT_TO];
	curTemplate->m_name = "[Unit] Move a specific unit to a location.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Move ";
	curTemplate->m_uiStrings[1] = " to ";
	curTemplate->m_uiStrings[2] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_SET_STATE];
	curTemplate->m_name = "[Team] Team custom state - set state.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TEAM_STATE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Set ";
	curTemplate->m_uiStrings[1] = " to ";
	curTemplate->m_uiStrings[2] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM];
	curTemplate->m_name = "[Team] Spawn a reinforcement team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Spawn an instance of ";
	curTemplate->m_uiStrings[1] = " at ";
	curTemplate->m_uiStrings[2] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM];
	curTemplate->m_name = "[Team] Spawn a reinforcement team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Spawn an instance of ";
	curTemplate->m_uiStrings[1] = " at ";
	curTemplate->m_uiStrings[2] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BUILDING];
	curTemplate->m_name = "[Skirmish Only] Build a building.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Build a building of type ";
	curTemplate->m_uiStrings[1] = ".";

  curTemplate = &m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER];
	curTemplate->m_name = "[Player] AI player build near a supply source.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Have AI ";
	curTemplate->m_uiStrings[1] = " build a ";
	curTemplate->m_uiStrings[2] = " near a supply src with at least ";
	curTemplate->m_uiStrings[3] = " available resources.";

  curTemplate = &m_actionTemplates[ScriptAction::TEAM_GUARD_SUPPLY_CENTER];
	curTemplate->m_name = "[Team] Set to guard - a supply source.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;									  
	curTemplate->m_uiStrings[0] = "Have Team ";
	curTemplate->m_uiStrings[1] = " guard attacked or closest supply src with at least ";
	curTemplate->m_uiStrings[2] = " available resources";

  curTemplate = &m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_UPGRADE];
	curTemplate->m_name = "[Player] AI player build an upgrade.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::UPGRADE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Have AI ";
	curTemplate->m_uiStrings[1] = " build this upgrade: ";

	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH	];
	curTemplate->m_name = "[Skirmish Only] Team follow approach path.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::SKIRMISH_WAYPOINT_PATH;
	curTemplate->m_parameters[2] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Have ";
	curTemplate->m_uiStrings[1] = " approach the enemy using path ";
	curTemplate->m_uiStrings[2] = ", as a team is ";

 	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_MOVE_TO_APPROACH_PATH	];
	curTemplate->m_name = "[Skirmish Only] Team move to approach path.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::SKIRMISH_WAYPOINT_PATH;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Have ";
	curTemplate->m_uiStrings[1] = " move to the start of enemy path ";
	curTemplate->m_uiStrings[2] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FRONT];
	curTemplate->m_name = "[Skirmish Only] Build base defense on front perimeter.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Build one additional perimeter base defenses, on the front.";

 	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FLANK];
	curTemplate->m_name = "[Skirmish Only] Build base defense on flank perimeter.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Build one additional perimeter base defenses, on the flank.";

 	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FRONT];
	curTemplate->m_name = "[Skirmish Only] Build structure on front perimeter.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Build one additional ";
	curTemplate->m_uiStrings[1] = ", on the front.";

 	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FLANK];
	curTemplate->m_name = "[Skirmish Only] Build structure on flank perimeter.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Build one additional ";
	curTemplate->m_uiStrings[1] = ", on the flank.";

 	curTemplate = &m_actionTemplates[ScriptAction::RECRUIT_TEAM];
	curTemplate->m_name = "[Team] Recruit a team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Recruit an instance of ";
	curTemplate->m_uiStrings[1] = ", maximum recruiting distance (feet):";
	curTemplate->m_uiStrings[2] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::MOVE_CAMERA_TO];
	curTemplate->m_name = "[Camera (M)] Move the camera to a location.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Move camera to ";
	curTemplate->m_uiStrings[1] = " in ";
	curTemplate->m_uiStrings[2] = " seconds, camera shutter ";
	curTemplate->m_uiStrings[3] = " seconds.";

 	curTemplate = &m_actionTemplates[ScriptAction::ZOOM_CAMERA];
	curTemplate->m_name = "[Camera] Change the camera zoom.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Change camera zoom to ";
	curTemplate->m_uiStrings[1] = " in ";
	curTemplate->m_uiStrings[2] = " seconds.";

 	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_FADE_ADD];
	curTemplate->m_name = "[Camera] Fade using an add blend to white.";
	curTemplate->m_numParameters = 5;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_parameters[3] = Parameter::INT;
	curTemplate->m_parameters[4] = Parameter::INT;
	curTemplate->m_numUiStrings = 6;
	curTemplate->m_uiStrings[0] = "Fade (0-1) from ";
	curTemplate->m_uiStrings[1] = " to ";
	curTemplate->m_uiStrings[2] = " adding toward white. Take ";
	curTemplate->m_uiStrings[3] = " frames to increase, hold for ";
	curTemplate->m_uiStrings[4] = " fames, and decrease ";
	curTemplate->m_uiStrings[5] = " frames.";

 	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT];
	curTemplate->m_name = "[Camera] Fade using a subtractive blend to black.";
	curTemplate->m_numParameters = 5;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_parameters[3] = Parameter::INT;
	curTemplate->m_parameters[4] = Parameter::INT;
	curTemplate->m_numUiStrings = 6;
	curTemplate->m_uiStrings[0] = "Fade (0-1) from ";
	curTemplate->m_uiStrings[1] = " to ";
	curTemplate->m_uiStrings[2] = " subtracting toward black. Take ";
	curTemplate->m_uiStrings[3] = " frames to increase, hold for ";
	curTemplate->m_uiStrings[4] = " fames, and decrease ";
	curTemplate->m_uiStrings[5] = " frames.";

 	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY];
	curTemplate->m_name = "[Camera] Fade using a multiply blend to black.";
	curTemplate->m_numParameters = 5;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_parameters[3] = Parameter::INT;
	curTemplate->m_parameters[4] = Parameter::INT;
	curTemplate->m_numUiStrings = 6;
	curTemplate->m_uiStrings[0] = "Fade (1-0) from ";
	curTemplate->m_uiStrings[1] = " to ";
	curTemplate->m_uiStrings[2] = " multiplying toward black. Take ";
	curTemplate->m_uiStrings[3] = " frames to increase, hold for ";
	curTemplate->m_uiStrings[4] = " fames, and decrease ";
	curTemplate->m_uiStrings[5] = " frames.";

 	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE];
	curTemplate->m_name = "[Camera] Fade using a saturate blend.";
	curTemplate->m_numParameters = 5;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_parameters[3] = Parameter::INT;
	curTemplate->m_parameters[4] = Parameter::INT;
	curTemplate->m_numUiStrings = 6;
	curTemplate->m_uiStrings[0] = "Fade (0.5-1) from ";
	curTemplate->m_uiStrings[1] = " to ";
	curTemplate->m_uiStrings[2] = " increasing saturation. Take ";
	curTemplate->m_uiStrings[3] = " frames to increase, hold for ";
	curTemplate->m_uiStrings[4] = " fames, and decrease ";
	curTemplate->m_uiStrings[5] = " frames.";

	curTemplate = &m_actionTemplates[ScriptAction::PITCH_CAMERA];
	curTemplate->m_name = "[Camera] Change the camera pitch.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Change camera pitch to ";
	curTemplate->m_uiStrings[1] = " in ";
	curTemplate->m_uiStrings[2] = " seconds.";

 	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_FOLLOW_NAMED];
	curTemplate->m_name = "[Camera] Follow a specific unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Have the camera follow ";
	curTemplate->m_uiStrings[1] = ".  Snap camera to object is ";
	curTemplate->m_uiStrings[2] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_STOP_FOLLOW];
	curTemplate->m_name = "[Camera] Stop following any units.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Stop following any units.";

	curTemplate = &m_actionTemplates[ScriptAction::SETUP_CAMERA];
	curTemplate->m_name = "[Camera] Set up the camera.";
	curTemplate->m_numParameters = 4;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_parameters[3] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 5;
	curTemplate->m_uiStrings[0] = "Position camera at ";
	curTemplate->m_uiStrings[1] = ", zoom = ";
	curTemplate->m_uiStrings[2] = "(0.0 to 1.0), pitch = ";
	curTemplate->m_uiStrings[3] = "(1.0==default), looking towards ";
	curTemplate->m_uiStrings[4] = ".";


 	curTemplate = &m_actionTemplates[ScriptAction::INCREMENT_COUNTER];
	curTemplate->m_name = "[Scripting] Counter -- increment.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::INT;
	curTemplate->m_parameters[1] = Parameter::COUNTER;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Add ";
	curTemplate->m_uiStrings[1] = " to counter ";

 	curTemplate = &m_actionTemplates[ScriptAction::DECREMENT_COUNTER];
	curTemplate->m_name = "[Scripting] Counter -- decrement.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::INT;
	curTemplate->m_parameters[1] = Parameter::COUNTER;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Subtract ";
	curTemplate->m_uiStrings[1] = " from counter ";

 	curTemplate = &m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH];
	curTemplate->m_name = "[Camera (M)] Move along a waypoint path.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Move along path starting with ";
	curTemplate->m_uiStrings[1] = " in ";
	curTemplate->m_uiStrings[2] = " seconds, camera shutter ";
	curTemplate->m_uiStrings[3] = " seconds.";

 	curTemplate = &m_actionTemplates[ScriptAction::ROTATE_CAMERA];
	curTemplate->m_name = "[Camera (R)] Rotate around the current viewpoint.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Rotate ";
	curTemplate->m_uiStrings[1] = " times, taking ";
	curTemplate->m_uiStrings[2] = " seconds total.";

	curTemplate = &m_actionTemplates[ScriptAction::RESET_CAMERA];
	curTemplate->m_name = "[Camera (M)] Reset to the default view.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Reset to ";
	curTemplate->m_uiStrings[1] = ", taking ";
	curTemplate->m_uiStrings[2] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::MOVE_CAMERA_TO_SELECTION];
	curTemplate->m_name = "[Camera mod(M)] End movement at selected unit.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "End movement at selected unit.";

	curTemplate = &m_actionTemplates[ScriptAction::SET_MILLISECOND_TIMER];
	curTemplate->m_name = "[Scripting] Seconds countdown timer -- set.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Set timer ";
	curTemplate->m_uiStrings[1] = " to expire in ";
	curTemplate->m_uiStrings[2] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER];
	curTemplate->m_name = "[Scripting] Seconds countdown timer -- set random.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Set timer ";
	curTemplate->m_uiStrings[1] = " to expire between ";
	curTemplate->m_uiStrings[2] = " and ";
	curTemplate->m_uiStrings[3] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::ADD_TO_MSEC_TIMER];
	curTemplate->m_name = "[Scripting] Seconds countdown timer -- add seconds.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::COUNTER;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Add ";
	curTemplate->m_uiStrings[1] = " seconds to timer ";
	curTemplate->m_uiStrings[2] = " .";

	curTemplate = &m_actionTemplates[ScriptAction::SUB_FROM_MSEC_TIMER];
	curTemplate->m_name = "[Scripting] Seconds countdown timer -- subtract seconds.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::COUNTER;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Subtract ";
	curTemplate->m_uiStrings[1] = " seconds from timer ";
	curTemplate->m_uiStrings[2] = " .";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_TIME];
	curTemplate->m_name = "[Camera mod(RM)] Freeze time during the camera movement.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Freeze time during the camera movement.";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_ANGLE];
	curTemplate->m_name = "[Camera mod(M)] Freeze camera angle during the camera movement.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Freeze camera angle during the camera movement.";

	curTemplate = &m_actionTemplates[ScriptAction::SUSPEND_BACKGROUND_SOUNDS];
	curTemplate->m_name = "[Multimedia] Suspend all sounds.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Suspend background sounds.";

	curTemplate = &m_actionTemplates[ScriptAction::RESUME_BACKGROUND_SOUNDS];
	curTemplate->m_name = "[Multimedia] Resume all sounds.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Resume background sounds.";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM];
	curTemplate->m_name = "[Camera mod(RM)] Final zoom for camera movement.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Adjust zoom to ";
	curTemplate->m_uiStrings[1] = " (1.0==max height, 0.0==in the ground.)";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH];
	curTemplate->m_name = "[Camera mod(RM)] Final pitch for camera movement.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Adjust pitch to ";
	curTemplate->m_uiStrings[1] = " (1.0==default, 0.0==toward horizon, >1 = toward ground.)";

	curTemplate = &m_actionTemplates[ScriptAction::SET_VISUAL_SPEED_MULTIPLIER];
	curTemplate->m_name = "[Multimedia] Modify visual game time.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Make visual time ";
	curTemplate->m_uiStrings[1] = " time normal (1=normal, 2 = twice as fast, ...).";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER];
	curTemplate->m_name = "[Camera mod(RM)] Final visual game time for camera movement.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Adjust game time to";
	curTemplate->m_uiStrings[1] = " times normal (1=normal, 2 = twice as fast, ...).";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOD_SET_ROLLING_AVERAGE];
	curTemplate->m_name = "[Camera mod(M)] Number of frames to average movements.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Average position and angle changes over";
	curTemplate->m_uiStrings[1] = " frames. (1=no smoothing, 5 = very smooth)";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOD_FINAL_LOOK_TOWARD];
	curTemplate->m_name = "[Camera mod(M)] Final camera look toward point.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Look toward";
	curTemplate->m_uiStrings[1] = " at the end of the camera movement.";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOD_LOOK_TOWARD];
	curTemplate->m_name = "[Camera mod(M)] Camera look toward point.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Look toward";
	curTemplate->m_uiStrings[1] = " during the camera movement.";

	curTemplate = &m_actionTemplates[ScriptAction::CREATE_OBJECT];
	curTemplate->m_name = "[Unit] Spawn -- object.";
	curTemplate->m_numParameters = 4;
	curTemplate->m_parameters[0] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_parameters[2] = Parameter::COORD3D;
	curTemplate->m_parameters[3] = Parameter::ANGLE;
	curTemplate->m_numUiStrings = 5;
	curTemplate->m_uiStrings[0] = "Spawn object ";
	curTemplate->m_uiStrings[1] = " in ";
	curTemplate->m_uiStrings[2] = " at position (";
	curTemplate->m_uiStrings[3] = "), rotated ";
	curTemplate->m_uiStrings[4] = " .";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ATTACK_TEAM];
	curTemplate->m_name = "[Team] Set to attack -- another team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begin attack on ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_ATTACK_NAMED];
	curTemplate->m_name = "[Unit] Set unit to attack another unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begin attack on ";

	curTemplate = &m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT];
	curTemplate->m_name = "[Unit] Spawn -- named unit on a team at a waypoint.";
	curTemplate->m_numParameters = 4;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[2] = Parameter::TEAM;
	curTemplate->m_parameters[3] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Spawn ";
	curTemplate->m_uiStrings[1] = " of type ";
	curTemplate->m_uiStrings[2] = " on ";
	curTemplate->m_uiStrings[3] = " at waypoint ";

	curTemplate = &m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT];
	curTemplate->m_name = "[Unit] Spawn -- unnamed unit on a team at a waypoint.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_parameters[2] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Spawn unit of type ";
	curTemplate->m_uiStrings[1] = " on ";
	curTemplate->m_uiStrings[2] = " at waypoint ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_APPLY_ATTACK_PRIORITY_SET];
	curTemplate->m_name = "[Unit] Apply unit's attack priority set.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::ATTACK_PRIORITY_SET;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Have ";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_APPLY_ATTACK_PRIORITY_SET];
	curTemplate->m_name = "[Team] Apply a team's attack priority set.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::ATTACK_PRIORITY_SET;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Have ";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING];
	curTemplate->m_name = "[Attack Priority Set] Modify priority for a single unit type.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::ATTACK_PRIORITY_SET;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "For ";
	curTemplate->m_uiStrings[1] = " set the priority of object type ";
	curTemplate->m_uiStrings[2] = " to ";

	curTemplate = &m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF];
	curTemplate->m_name = "[Attack Priority Set] Modify priorities for all of a kind.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::ATTACK_PRIORITY_SET;
	curTemplate->m_parameters[1] = Parameter::KIND_OF_PARAM;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "For ";
	curTemplate->m_uiStrings[1] = " set the priority of object type ";
	curTemplate->m_uiStrings[2] = " to ";

	curTemplate = &m_actionTemplates[ScriptAction::SET_DEFAULT_ATTACK_PRIORITY];
	curTemplate->m_name = "[Attack Priority Set] Set the default priority.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::ATTACK_PRIORITY_SET;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "For ";
	curTemplate->m_uiStrings[1] = " set the default priority to ";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_ADD_SKILLPOINTS];
	curTemplate->m_name = "[Player] Add/Subtract Skill Points.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is given ";
	curTemplate->m_uiStrings[2] = " Skill Points.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_ADD_RANKLEVEL];
	curTemplate->m_name = "[Player] Add/Subtract Rank Levels.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is given ";
	curTemplate->m_uiStrings[2] = " Rank Levels.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVEL];
	curTemplate->m_name = "[Player] Set Rank Level.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is given a Rank Level of ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVELLIMIT];
	curTemplate->m_name = "[Map] Set Rank Level Limit for current Map.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "The current map is given a Rank Level Limit of ";
	curTemplate->m_uiStrings[1] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_GRANT_SCIENCE];
	curTemplate->m_name = "[Player] Grant a Science to a given Player (ignoring prerequisites).";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SCIENCE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is granted ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_PURCHASE_SCIENCE];
	curTemplate->m_name = "[Player] Player attempts to purchase a Science.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SCIENCE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " attempts to purchase Science ";
	curTemplate->m_uiStrings[2] = ".";
	
	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY];
	curTemplate->m_name = "[Player] Set science availability.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SCIENCE;
	curTemplate->m_parameters[2] = Parameter::SCIENCE_AVAILABILITY;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " set ";
	curTemplate->m_uiStrings[2] = " availability to ";
	curTemplate->m_uiStrings[3] = ".";
	
	curTemplate = &m_actionTemplates[ScriptAction::SET_BASE_CONSTRUCTION_SPEED];
	curTemplate->m_name = "[Player] Set the delay between building teams.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " will delay ";
	curTemplate->m_uiStrings[1] = " seconds between building teams.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_SET_ATTITUDE];
	curTemplate->m_name = "[Unit] Set the general attitude of a specific unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::AI_MOOD;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " changes his attitude to ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_SET_ATTITUDE];
	curTemplate->m_name = "[Team] Set the general attitude of a team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::AI_MOOD;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " change their attitude to ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_SET_REPULSOR];
	curTemplate->m_name = "[Unit] Set the REPULSOR flag of a specific unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " REPULSOR flag is ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_SET_REPULSOR];
	curTemplate->m_name = "[Team] Set the REPULSOR flag of a team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " REPULSOR flag is ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_ATTACK_AREA];
	curTemplate->m_name = "[Unit] Set a specific unit to attack a specific trigger area.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " attacks anything in ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_ATTACK_TEAM];
	curTemplate->m_name = "[Unit] Set a specific unit to attack a team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " attacks ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ATTACK_AREA];
	curTemplate->m_name = "[Team] Set to attack -- trigger area.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " attack anything in ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ATTACK_NAMED];
	curTemplate->m_name = "[Team] Set to attack -- specific unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " attacks ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_LOAD_TRANSPORTS];
	curTemplate->m_name = "[Team] Transport -- automatically load.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " load into transports.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_ENTER_NAMED];
	curTemplate->m_name = "[Unit] Transport -- load unit into specific.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " loads into ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ENTER_NAMED];
	curTemplate->m_name = "[Team] Transport -- load team into specific.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " attempt to load into ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_EXIT_ALL];
	curTemplate->m_name = "[Unit] Transport -- unload units from specific.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " unloads.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_EXIT_ALL];
	curTemplate->m_name = "[Team] Transport -- unload team from all.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " unload.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS];
	curTemplate->m_name = "[Unit] Set a specific unit to follow a waypoint path.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT_PATH;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " follows waypoints, beginning at ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS_EXACT];
	curTemplate->m_name = "[Unit] Set a specific unit to EXACTLY follow a waypoint path.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT_PATH;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " EXACTLY follows waypoints, beginning at ";

		curTemplate = &m_actionTemplates[ScriptAction::NAMED_GUARD];
	curTemplate->m_name = "[Unit] Set to guard.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begins guarding.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_GUARD];
	curTemplate->m_name = "[Team] Set to guard -- current location.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begins guarding.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_GUARD_POSITION];
	curTemplate->m_name = "[Team] Set to guard -- location.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begins guarding at ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_GUARD_OBJECT];
	curTemplate->m_name = "[Team] Set to guard -- specific unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begins guarding ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_GUARD_AREA];
	curTemplate->m_name = "[Team] Set to guard -- area.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begins guarding ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_HUNT];
	curTemplate->m_name = "[Unit] Set a specific unit to hunt.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begins hunting.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_HUNT_WITH_COMMAND_BUTTON];
	curTemplate->m_name = "[Team] Set to hunt using commandbutton ability.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begins hunting using ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_HUNT];
	curTemplate->m_name = "[Team] Set to hunt.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begins hunting.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_HUNT];
	curTemplate->m_name = "[Player] Set all of a player's units to hunt.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begins hunting.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_SELL_EVERYTHING];
	curTemplate->m_name = "[Player] Set a player to sell everything.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " sells everything.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_DISABLE_BASE_CONSTRUCTION];
	curTemplate->m_name = "[Player] Set a player to be unable to build buildings.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is unable to build buildings.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_DISABLE_FACTORIES];
	curTemplate->m_name = "[Player] Set a player to be unable to build from a specific building.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is unable to build from ";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_DISABLE_UNIT_CONSTRUCTION];
	curTemplate->m_name = "[Player] Set a player to be unable to build units.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is unable to build units.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_ENABLE_BASE_CONSTRUCTION];
	curTemplate->m_name = "[Player] Set a player to be able to build buildings.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is able to build buildings.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_ENABLE_FACTORIES];
	curTemplate->m_name = "[Player] Set a player to be able to build from a specific building.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is able to build from ";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_ENABLE_UNIT_CONSTRUCTION];
	curTemplate->m_name = "[Player] Set a player to be able to build units.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is able to build units.";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOVE_HOME];
	curTemplate->m_name = "[Camera (M)] Move the camera to the home position.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "The camera moves to the home base.";

	curTemplate = &m_actionTemplates[ScriptAction::OVERSIZE_TERRAIN];
	curTemplate->m_name = "[Camera] Oversize the terrain.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Oversize the terrain ";
	curTemplate->m_uiStrings[1] = " tiles on each side [0 = reset to normal].";

	curTemplate = &m_actionTemplates[ScriptAction::BUILD_TEAM];
	curTemplate->m_name = "[Team] Build a team.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Build team ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_DAMAGE];
	curTemplate->m_name = "[Unit] Deal damage to a specific unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " takes ";
	curTemplate->m_uiStrings[2] = " points of damage.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_DELETE];
	curTemplate->m_name = "[Unit] Delete a specific unit.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is removed from the world.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_DELETE];
	curTemplate->m_name = "[Team] Delete a team.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is removed from the world.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_DELETE_LIVING];
	curTemplate->m_name = "[Team] Delete a team, but ignore dead guys.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Each living member of team ";
	curTemplate->m_uiStrings[1] = " is removed from the world.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_KILL];
	curTemplate->m_name = "[Unit] Kill a specific unit.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = "is dealt a lethal amount of damage.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_KILL];
	curTemplate->m_name = "[Team] Kill an entire team.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is dealt a lethal amount of damage.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_KILL];
	curTemplate->m_name = "[Player] Kill a player.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "All of ";
	curTemplate->m_uiStrings[1] = "'s buildings and units are dealt a lethal amount of damage.";

	curTemplate = &m_actionTemplates[ScriptAction::DISPLAY_TEXT];
	curTemplate->m_name = "[User] Display a string.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::LOCALIZED_TEXT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Displays ";
	curTemplate->m_uiStrings[1] = " in the text log and message area.";

	curTemplate = &m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT];
	curTemplate->m_name = "[User] Display a cinematic string.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::LOCALIZED_TEXT;
	curTemplate->m_parameters[1] = Parameter::FONT_NAME;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Displays ";
	curTemplate->m_uiStrings[1] = " with font type ";
	curTemplate->m_uiStrings[2] = " in the bottom letterbox for ";
	curTemplate->m_uiStrings[3] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::CAMEO_FLASH];
	curTemplate->m_name = "[User] Flash a cameo for a specified amount of time.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::COMMAND_BUTTON;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " flashes for ";
	curTemplate->m_uiStrings[2] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_FLASH];
	curTemplate->m_name = "[User] Flash a specific unit for a specified amount of time.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " flashes for ";
	curTemplate->m_uiStrings[2] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_FLASH];
	curTemplate->m_name = "[User] Flash a team for a specified amount of time.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " flashes for ";
	curTemplate->m_uiStrings[2] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_CUSTOM_COLOR];
	curTemplate->m_name = "[User] Set a specific unit to use a special indicator color.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::COLOR;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " uses the color ";
	curTemplate->m_uiStrings[2] = " .";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_FLASH_WHITE];
	curTemplate->m_name = "[User] Flash a specific unit white for a specified amount of time.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " flashes white for ";
	curTemplate->m_uiStrings[2] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_FLASH_WHITE];
	curTemplate->m_name = "[User] Flash a team white for a specified amount of time.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " flashes white for ";
	curTemplate->m_uiStrings[2] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE];
	curTemplate->m_name = "[User] Display Popup Message Box.";
	curTemplate->m_numParameters = 5;
	curTemplate->m_parameters[0] = Parameter::LOCALIZED_TEXT;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_parameters[3] = Parameter::INT;
	curTemplate->m_parameters[4] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 6;
	curTemplate->m_uiStrings[0] = "Displays ";
	curTemplate->m_uiStrings[1] = " at ";
	curTemplate->m_uiStrings[2] = " percent of the screen Width ";
	curTemplate->m_uiStrings[3] = " percent of the screen Height and a width of ";
	curTemplate->m_uiStrings[4] = " pixels and pauses the game (";
	curTemplate->m_uiStrings[5] = " )";

	curTemplate = &m_actionTemplates[ScriptAction::MOVIE_PLAY_FULLSCREEN];
	curTemplate->m_name = "[Multimedia] Play a movie in fullscreen mode.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::MOVIE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " plays fullscreen.";

	curTemplate = &m_actionTemplates[ScriptAction::MOVIE_PLAY_RADAR];
	curTemplate->m_name = "[Multimedia] Play a movie in the radar.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::MOVIE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " plays in the radar window.";

	curTemplate = &m_actionTemplates[ScriptAction::SOUND_PLAY_NAMED];
	curTemplate->m_name = "[Multimedia] Play a sound as though coming from a specific unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEXT_STRING;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " plays as though coming from ";

	curTemplate = &m_actionTemplates[ScriptAction::SPEECH_PLAY];
	curTemplate->m_name = "[Multimedia] Play a speech file.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::DIALOG;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " plays, allowing overlap ";
	curTemplate->m_uiStrings[2] = " (true to allow, false to disallow).";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_TRANSFER_OWNERSHIP_PLAYER];
	curTemplate->m_name = "[Player] Transfer assets from one player to another player.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "All assets of ";
	curTemplate->m_uiStrings[1] = " are transferred to ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_TRANSFER_OWNERSHIP_PLAYER];
	curTemplate->m_name = "[Player] Transfer a specific unit to the control of a player.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is transferred to the command of ";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_EXCLUDE_FROM_SCORE_SCREEN];
	curTemplate->m_name = "[Player] Exclude this player from the score screen.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Exclude ";
	curTemplate->m_uiStrings[1] = " from the score screen.";

	curTemplate = &m_actionTemplates[ScriptAction::ENABLE_SCORING];
	curTemplate->m_name = "[Scripting] Turn on scoring.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Turn on scoring.";

	curTemplate = &m_actionTemplates[ScriptAction::DISABLE_SCORING];
	curTemplate->m_name = "[Scripting] Turn off scoring.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Turn off scoring.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER];
	curTemplate->m_name = "[Player] Change how a player relates to another player.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_parameters[2] = Parameter::RELATION;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " considers ";
	curTemplate->m_uiStrings[2] = " to be ";

	curTemplate = &m_actionTemplates[ScriptAction::RADAR_CREATE_EVENT];
	curTemplate->m_name = "[Radar] Create a radar event at a specified location.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::COORD3D;
	curTemplate->m_parameters[1] = Parameter::RADAR_EVENT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "A radar event occurs at ";
	curTemplate->m_uiStrings[1] = " of type ";

	curTemplate = &m_actionTemplates[ScriptAction::OBJECT_CREATE_RADAR_EVENT];
	curTemplate->m_name = "[Radar] Create a radar event at a specific object.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::RADAR_EVENT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "A radar event occurs at ";
	curTemplate->m_uiStrings[1] = " of type ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_CREATE_RADAR_EVENT];
	curTemplate->m_name = "[Radar] Create a radar event at a specific team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::RADAR_EVENT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "A radar event occurs at ";
	curTemplate->m_uiStrings[1] = " of type ";
	
	curTemplate = &m_actionTemplates[ScriptAction::RADAR_DISABLE];
	curTemplate->m_name = "[Radar] Disable the radar.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "The radar is disabled.";

	curTemplate = &m_actionTemplates[ScriptAction::RADAR_ENABLE];
	curTemplate->m_name = "[Radar] Enable the radar.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "The radar is enabled.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_SET_STEALTH_ENABLED];
	curTemplate->m_name = "[Unit] Stealth set enabled or disabled.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Set ";
	curTemplate->m_uiStrings[1] = " stealth ability to ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_SET_STEALTH_ENABLED];
	curTemplate->m_name = "[Team] Stealth set enabled or disabled.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Set ";
	curTemplate->m_uiStrings[1] = " stealth ability to ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT];
	curTemplate->m_name = "[Map] Reveal map at waypoint -- fog.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "The map is revealed at ";
	curTemplate->m_uiStrings[1] = " with a radius of ";
	curTemplate->m_uiStrings[2] = " feet for ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT];
	curTemplate->m_name = "[Map] Reveal map at waypoint -- undo fog.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "The map is shrouded at ";
	curTemplate->m_uiStrings[1] = " with a radius of ";
	curTemplate->m_uiStrings[2] = " feet for ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::MAP_REVEAL_ALL];
	curTemplate->m_name = "[Map] Reveal the entire map for a player.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "The world is revealed for ";
	curTemplate->m_uiStrings[1] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_PERM];
	curTemplate->m_name = "[Map] Reveal the entire map permanently for a player.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "The world is revealed permanently for ";
	curTemplate->m_uiStrings[1] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_UNDO_PERM];
	curTemplate->m_name = "[Map] Reveal the entire map permanently is un-done for a player.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Undo the permanent reveal for ";
	curTemplate->m_uiStrings[1] = ".  This will mess things up badly if called when there has been no permanent reveal.";

	curTemplate = &m_actionTemplates[ScriptAction::MAP_SHROUD_ALL];
	curTemplate->m_name = "[Map] Shroud the entire map for a player.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "The world is shrouded for ";
	curTemplate->m_uiStrings[1] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::DISABLE_BORDER_SHROUD];
	curTemplate->m_name = "[Map] Border Shroud is turned off.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Shroud off the map edges is turned off.";

	curTemplate = &m_actionTemplates[ScriptAction::ENABLE_BORDER_SHROUD];
	curTemplate->m_name = "[Map] Border Shroud is turned on.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Shroud off the map edges is turned on.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_GARRISON_SPECIFIC_BUILDING];
	curTemplate->m_name = "[Team] Garrison a specific building with a team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " enters into building named ";

	curTemplate = &m_actionTemplates[ScriptAction::EXIT_SPECIFIC_BUILDING];
	curTemplate->m_name = "[Unit] Empty a specific building.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " empties.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_GARRISON_NEAREST_BUILDING];
	curTemplate->m_name = "[Team] Garrison a nearby building with a team.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " garrison a nearby building.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_EXIT_ALL_BUILDINGS];
	curTemplate->m_name = "[Team] Exit all buildings a team is in.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " exits all buildings.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_GARRISON_SPECIFIC_BUILDING];
	curTemplate->m_name = "[Unit] Garrison a specific building with a specific unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " garrison building ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_GARRISON_NEAREST_BUILDING];
	curTemplate->m_name = "[Unit] Garrison a nearby building with a specific unit.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " garrison a nearby building.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_EXIT_BUILDING];
	curTemplate->m_name = "[Unit] Exit the building the unit is in.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " leaves the building.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_GARRISON_ALL_BUILDINGS];
	curTemplate->m_name = "[Player] Garrison as many buildings as player has units for.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " garrison buildings.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_EXIT_ALL_BUILDINGS];
	curTemplate->m_name = "[Player] All units leave their garrisons.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " evacuate.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_AVAILABLE_FOR_RECRUITMENT];
	curTemplate->m_name = "[Team] Set whether members of a team can be recruited into another team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " sets their willingness to join teams to ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_COLLECT_NEARBY_FOR_TEAM];
	curTemplate->m_name = "[Team] Set to collect nearby units.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " attempts to collect nearby units for a team.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_MERGE_INTO_TEAM];
	curTemplate->m_name = "[Team] Merge a team into another team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " merges onto ";

	curTemplate = &m_actionTemplates[ScriptAction::IDLE_ALL_UNITS];
	curTemplate->m_name = "[Scripting] Idle all units for all players.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Idle all units for all players.";

	curTemplate = &m_actionTemplates[ScriptAction::RESUME_SUPPLY_TRUCKING];
	curTemplate->m_name = "[Scripting] All idle Supply Trucks attempt to resume supply routes.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "All idle Supply Trucks attempt to resume supply routes.";

	curTemplate = &m_actionTemplates[ScriptAction::DISABLE_INPUT];
	curTemplate->m_name = "[User] User input -- disable.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Disable mouse and keyboard input.";

	curTemplate = &m_actionTemplates[ScriptAction::ENABLE_INPUT];
	curTemplate->m_name = "[User] User input -- enable.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Enable mouse and keyboard input.";

	curTemplate = &m_actionTemplates[ScriptAction::SOUND_AMBIENT_PAUSE];
	curTemplate->m_name = "[Multimedia] Pause the ambient sounds.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Pause the ambient sounds.";

	curTemplate = &m_actionTemplates[ScriptAction::SOUND_AMBIENT_RESUME];
	curTemplate->m_name = "[Multimedia] Resume the ambient sounds.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Resume the ambient sounds.";

	curTemplate = &m_actionTemplates[ScriptAction::MUSIC_SET_TRACK];
	curTemplate->m_name = "[Multimedia] Play a music track.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::MUSIC;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_parameters[2] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Play ";
	curTemplate->m_uiStrings[1] = " using fadeout (";
	curTemplate->m_uiStrings[2] = ") and fadein (";
	curTemplate->m_uiStrings[3] = ").";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_BEGIN];
	curTemplate->m_name = "[Camera] Start letterbox mode.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Start letterbox mode (hide UI, add border).";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_END];
	curTemplate->m_name = "[Camera] End letterbox mode.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "End letterbox mode (show UI, remove border).";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_BW_MODE_BEGIN];
	curTemplate->m_name = "[Camera] Start black & white mode.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::INT;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Frames to fade into black & white mode = ";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_BW_MODE_END];
	curTemplate->m_name = "[Camera] End black & white mode.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Frames to fade into color mode = ";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR];
	curTemplate->m_name = "[Camera] Motion blur zoom.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::BOOLEAN;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Blur zoom, zoom in = ";
	curTemplate->m_uiStrings[1] = " (true=zoom in, false = zoom out), saturate colors = ";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_JUMP];
	curTemplate->m_name = "[Camera] Motion blur zoom with jump cut.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Blur zoom, zoom in at current location, zoom out at ";
	curTemplate->m_uiStrings[1] = ", saturate colors = ";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_FOLLOW];
	curTemplate->m_name = "[Camera] Start motion blur as the camera moves.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Start motion blur as the camera moves, amount= ";
	curTemplate->m_uiStrings[1] = " (start with 30 and adjust up or down). ";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_END_FOLLOW];
	curTemplate->m_name = "[Camera] End motion blur as the camera moves.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "End motion blur as the camera moves.";

	curTemplate = &m_actionTemplates[ScriptAction::DRAW_SKYBOX_BEGIN];
	curTemplate->m_name = "[Camera] Start skybox mode.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Start skybox mode (draw sky background).";

	curTemplate = &m_actionTemplates[ScriptAction::DRAW_SKYBOX_END];
	curTemplate->m_name = "[Camera] End skybox mode.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "End skybox mode (draw black background).";

	curTemplate = &m_actionTemplates[ScriptAction::FREEZE_TIME];
	curTemplate->m_name = "[Scripting] Time -- freeze .";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Freeze time.";

	curTemplate = &m_actionTemplates[ScriptAction::UNFREEZE_TIME];
	curTemplate->m_name = "[Scripting] Time -- unfreeze.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Unfreeze time.";

	curTemplate = &m_actionTemplates[ScriptAction::SHOW_MILITARY_CAPTION];
	curTemplate->m_name = "[Scripting] Show military briefing caption.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEXT_STRING;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Show military briefing ";
	curTemplate->m_uiStrings[1] = " for ";
	curTemplate->m_uiStrings[2] = " milliseconds.";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_SET_AUDIBLE_DISTANCE];
	curTemplate->m_name = "[Camera] Set the audible distance for camera-up shots.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set the audible range during camera-up shots to ";
	
	curTemplate = &m_actionTemplates[ScriptAction::NAMED_SET_HELD];
	curTemplate->m_name = "[Unit] Set unit to be held in place, ignoring Physics, Locomotors, etc.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Set Held status for ";
	curTemplate->m_uiStrings[1] = " to ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_SET_STOPPING_DISTANCE];
	curTemplate->m_name = "[Unit] Set stopping distance for current locomotor.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Set stopping distance for ";
	curTemplate->m_uiStrings[1] = " to ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::SET_STOPPING_DISTANCE];
	curTemplate->m_name = "[Team] Set stopping distance for each unit's current locomotor.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Set stopping distances for ";
	curTemplate->m_uiStrings[1] = " to ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::SET_FPS_LIMIT];
	curTemplate->m_name = "[Scripting] Set max frames per second.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set max FPS to ";
	curTemplate->m_uiStrings[1] = ".  (0 sets to default.)";

	curTemplate = &m_actionTemplates[ScriptAction::DISABLE_SPECIAL_POWER_DISPLAY];
	curTemplate->m_name = "[Scripting] Special power countdown display -- disable.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Disables special power countdown display.";

	curTemplate = &m_actionTemplates[ScriptAction::ENABLE_SPECIAL_POWER_DISPLAY];
	curTemplate->m_name = "[Scripting] Special power countdown display -- enable.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Enables special power countdown display.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_HIDE_SPECIAL_POWER_DISPLAY];
	curTemplate->m_name = "[Unit] Special power countdown timer -- hide.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Hides special power countdowns for ";
	curTemplate->m_uiStrings[1] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_SHOW_SPECIAL_POWER_DISPLAY];
	curTemplate->m_name = "[Unit] Special power countdown timer -- display.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Shows special power countdowns for ";
	curTemplate->m_uiStrings[1] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::MUSIC_SET_VOLUME];
	curTemplate->m_name = "[Multimedia] Set the current music volume.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set the desired music volume to ";
	curTemplate->m_uiStrings[1] = "%. (0-100)";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_TRANSFER_TO_PLAYER];
	curTemplate->m_name = "[Team] Transfer control of a team to a player.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Control of ";
	curTemplate->m_uiStrings[1] = " transfers to ";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_SET_MONEY];
	curTemplate->m_name = "[Player] Set player's money.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set ";
	curTemplate->m_uiStrings[1] = "'s money to $";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_GIVE_MONEY];
	curTemplate->m_name = "[Player] Gives/takes from player's money.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " gets $";

	curTemplate = &m_actionTemplates[ScriptAction::DISPLAY_COUNTER];
	curTemplate->m_name = "[Scripting] Counter -- display an individual counter to the user.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_parameters[1] = Parameter::LOCALIZED_TEXT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Show ";
	curTemplate->m_uiStrings[1] = " with text ";

	curTemplate = &m_actionTemplates[ScriptAction::HIDE_COUNTER];
	curTemplate->m_name = "[Scripting] Counter -- hides an individual counter from the user.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Hide ";

	curTemplate = &m_actionTemplates[ScriptAction::DISPLAY_COUNTDOWN_TIMER];
	curTemplate->m_name = "[Scripting] Timer -- display an individual timer to the user.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_parameters[1] = Parameter::LOCALIZED_TEXT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Show ";
	curTemplate->m_uiStrings[1] = " with text ";

	curTemplate = &m_actionTemplates[ScriptAction::HIDE_COUNTDOWN_TIMER];
	curTemplate->m_name = "[Scripting] Timer -- hides an individual timer from the user.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Hide ";

	curTemplate = &m_actionTemplates[ScriptAction::DISABLE_COUNTDOWN_TIMER_DISPLAY];
	curTemplate->m_name = "[Scripting] Timer -- hide all timers from the user.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Disables timer display.";

	curTemplate = &m_actionTemplates[ScriptAction::ENABLE_COUNTDOWN_TIMER_DISPLAY];
	curTemplate->m_name = "[Scripting] Timer -- display all timers to the user.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Enables timer display.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_STOP_SPECIAL_POWER_COUNTDOWN];
	curTemplate->m_name = "[Unit] Special power countdown timer -- pause.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Pause ";
	curTemplate->m_uiStrings[1] = "'s ";
	curTemplate->m_uiStrings[2] = " countdown.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_START_SPECIAL_POWER_COUNTDOWN];
	curTemplate->m_name = "[Unit] Special power countdown timer -- resume.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Resume ";
	curTemplate->m_uiStrings[1] = "'s ";
	curTemplate->m_uiStrings[2] = " countdown.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN];
	curTemplate->m_name = "[Unit] Special power countdown timer -- set.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Set ";
	curTemplate->m_uiStrings[1] = "'s ";
	curTemplate->m_uiStrings[2] = " to ";
	curTemplate->m_uiStrings[3] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN];
	curTemplate->m_name = "[Unit] Special power countdown timer -- add seconds.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = "'s ";
	curTemplate->m_uiStrings[2] = " has ";
	curTemplate->m_uiStrings[3] = " seconds added to it.";

	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST];
	curTemplate->m_name = "[Skirmish] Special power -- fire at enemy's highest cost area.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " fire ";
	curTemplate->m_uiStrings[2] = " at enemy's most costly area.";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_REPAIR_NAMED_STRUCTURE];
	curTemplate->m_name = "[Player] Repair named bridge or structure.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Have ";
	curTemplate->m_uiStrings[1] = " repair ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT];
	curTemplate->m_name = "[Unit] Special power -- fire at location.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_parameters[2] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " fires ";
	curTemplate->m_uiStrings[2] = " at ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED];
	curTemplate->m_name = "[Unit] Special power -- fire at unit.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_parameters[2] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " fires ";
	curTemplate->m_uiStrings[2] = " at ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::REFRESH_RADAR];
	curTemplate->m_name = "[Scripting] Refresh radar terrain.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Refresh radar terrain.";
	
	curTemplate = &m_actionTemplates[ScriptAction::NAMED_STOP];
	curTemplate->m_name = "[Unit] Set a specific unit to stop.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " stops.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_STOP];
	curTemplate->m_name = "[Team] Set to stop.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " stops.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_STOP_AND_DISBAND];
	curTemplate->m_name = "[Team] Set to stop, then disband.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " stops, then disbands.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM];
	curTemplate->m_name = "[Team] Override a team's relationship to another team.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_parameters[2] = Parameter::RELATION;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " considers ";
	curTemplate->m_uiStrings[2] = " to be ";
	curTemplate->m_uiStrings[3] = " (rather than using the the player relationship).";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM];
	curTemplate->m_name = "[Team] Remove an override to a team's relationship to another team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " uses the player relationship to ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_REMOVE_ALL_OVERRIDE_RELATIONS];
	curTemplate->m_name = "[Team] Remove all overrides to team's relationship to teams and/or players.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " uses the player relationship to all other teams and players.";

 	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED];
	curTemplate->m_name = "[Camera] Tether camera to a specific unit.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Have the camera tethered to ";
	curTemplate->m_uiStrings[1] = ".  Snap camera to object is ";
	curTemplate->m_uiStrings[2] = ".  Amount of play is ";
	curTemplate->m_uiStrings[3] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_STOP_TETHER_NAMED];
	curTemplate->m_name = "[Camera] Stop tether to any units.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Stop tether to any units.";

	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT];
	curTemplate->m_name = "[Camera] Set default camera.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Camera Pitch = ";
	curTemplate->m_uiStrings[1] = "(0.0==default), angle = ";
	curTemplate->m_uiStrings[2] = "(0.0 is N, 90.0 is W, etc), height = ";
	curTemplate->m_uiStrings[3] = "(1.0==default).";

 	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT];
	curTemplate->m_name = "[Camera (R)] Rotate toward unit.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Rotate toward ";
	curTemplate->m_uiStrings[1] = ", taking ";
	curTemplate->m_uiStrings[2] = " seconds and holding ";
	curTemplate->m_uiStrings[3] = " seconds.";

 	curTemplate = &m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT];
	curTemplate->m_name = "[Camera (R)] Rotate to look at a waypoint.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Rotate to look at ";
	curTemplate->m_uiStrings[1] = ", taking ";
	curTemplate->m_uiStrings[2] = " seconds.";

 	curTemplate = &m_actionTemplates[ScriptAction::UNIT_DESTROY_ALL_CONTAINED];
	curTemplate->m_name = "[Unit] Kill all units contained within a specific transport or structure.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
//	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "All units inside ";
	curTemplate->m_uiStrings[1] = " are killed.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH];
	curTemplate->m_name = "[Unit] Fire waypoint-weapon following waypoint path.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT_PATH;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " fire waypoint-weapon following waypoints starting at ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER];
	curTemplate->m_name = "[Team] Override a team's relationship to another player.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_parameters[2] = Parameter::RELATION;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " considers ";
	curTemplate->m_uiStrings[2] = " to be ";
	curTemplate->m_uiStrings[3] = " (rather than using the the player relationship).";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER];
	curTemplate->m_name = "[Team] Remove an override to a team's relationship to another player.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " uses the player relationship to ";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM];
	curTemplate->m_name = "[Player] Override a player's relationship to another team.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_parameters[2] = Parameter::RELATION;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " considers ";
	curTemplate->m_uiStrings[2] = " to be ";
	curTemplate->m_uiStrings[3] = " (rather than using the the player relationship).";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM];
	curTemplate->m_name = "[Player] Remove an override to a player's relationship to another team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " uses the player relationship to ";

	curTemplate = &m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT];
	curTemplate->m_name = "[Unit] Set a specific unit to execute a script sequentially.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SCRIPT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " executes ";
	curTemplate->m_uiStrings[2] = " sequentially.";

	curTemplate = &m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING];
	curTemplate->m_name = "[Unit] Set a specific unit to execute a looping sequential script.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SCRIPT;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " executes ";
	curTemplate->m_uiStrings[2] = " sequentially, ";
	curTemplate->m_uiStrings[3] = " times. (0=forever)";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT];
	curTemplate->m_name = "[Team] Execute script sequentially -- start.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::SCRIPT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " executes ";
	curTemplate->m_uiStrings[2] = " sequentially.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING];
	curTemplate->m_name = "[Team] Execute script sequentially -- looping.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::SCRIPT;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " executes ";
	curTemplate->m_uiStrings[2] = " sequentially, ";
	curTemplate->m_uiStrings[3] = " times. (0=forever)";

	curTemplate = &m_actionTemplates[ScriptAction::UNIT_STOP_SEQUENTIAL_SCRIPT];
	curTemplate->m_name = "[Unit] Set a specific unit to stop executing a sequential script.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " stops executing.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_STOP_SEQUENTIAL_SCRIPT];
	curTemplate->m_name = "[Team] Execute script sequentially -- stop.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " stops executing.";

	curTemplate = &m_actionTemplates[ScriptAction::UNIT_GUARD_FOR_FRAMECOUNT];
	curTemplate->m_name = "[Unit] Set to guard for some number of frames.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " guards for ";
	curTemplate->m_uiStrings[2] = " frames.";

	curTemplate = &m_actionTemplates[ScriptAction::UNIT_IDLE_FOR_FRAMECOUNT];
	curTemplate->m_name = "[Unit] Set to idle for some number of frames.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " idles for ";
	curTemplate->m_uiStrings[2] = " frames.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_GUARD_FOR_FRAMECOUNT];
	curTemplate->m_name = "[Team] Set to guard -- number of frames.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " guards for ";
	curTemplate->m_uiStrings[2] = " frames.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_IDLE_FOR_FRAMECOUNT];
	curTemplate->m_name = "[Team] Set to idle for some number of frames.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " idles for ";
	curTemplate->m_uiStrings[2] = " frames.";

	curTemplate = &m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT];
	curTemplate->m_name = "[Map] Adjust water height to a new level";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TRIGGER_AREA;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " changes altitude to ";

	curTemplate = &m_actionTemplates[ ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME ];
	curTemplate->m_name = "[Map] Adjust water height to a new level with damage over time";
	curTemplate->m_numParameters = 4;
	curTemplate->m_parameters[0] = Parameter::TRIGGER_AREA;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_parameters[3] = Parameter::REAL;
	curTemplate->m_numUiStrings = 5;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " changes altitude to ";
	curTemplate->m_uiStrings[2] = " in ";
	curTemplate->m_uiStrings[3] = " seconds doing ";
	curTemplate->m_uiStrings[4] = " dam/sec.";

	curTemplate = &m_actionTemplates[ ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY ];
	curTemplate->m_name = "[Unit] Use commandbutton ability."; 
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ABILITY;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED ];
	curTemplate->m_name = "[Unit] Use commandbutton ability on an object."; 
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ABILITY;
	curTemplate->m_parameters[2] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = " on ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_actionTemplates[ ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT ];
	curTemplate->m_name = "[Unit] Use commandbutton ability at a waypoint."; 
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ABILITY;
	curTemplate->m_parameters[2] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = " at ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_actionTemplates[ ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY ];
	curTemplate->m_name = "[Team] Use commandbutton ability."; 
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED ];
	curTemplate->m_name = "[Team] Use commandbutton ability on an object."; 
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_parameters[2] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = " on ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_actionTemplates[ ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT ];
	curTemplate->m_name = "[Team] Use commandbutton ability at a waypoint."; 
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_parameters[2] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = " at ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::MAP_SWITCH_BORDER];
	curTemplate->m_name = "[Map] Change the active boundary.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::BOUNDARY;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " becomes the active border.";

	curTemplate = &m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT];
	curTemplate->m_name = "[Scripting] Select the first object type on a team.";
	curTemplate->m_numParameters = 4;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[2] = Parameter::BOOLEAN;
	curTemplate->m_parameters[3] = Parameter::DIALOG;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " 's first ";
	curTemplate->m_uiStrings[2] = ", centers in view (";
	curTemplate->m_uiStrings[3] = ") while playing ";

	curTemplate = &m_actionTemplates[ScriptAction::RADAR_FORCE_ENABLE];
	curTemplate->m_name = "[Radar] Force enable the radar.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "The radar is now forced to be enabled.";


	curTemplate = &m_actionTemplates[ScriptAction::RADAR_REVERT_TO_NORMAL];
	curTemplate->m_name = "[Radar] Revert radar to normal behavior.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "The radar is now reverting to its normal behavior.";

	curTemplate = &m_actionTemplates[ScriptAction::SCREEN_SHAKE];
	curTemplate->m_name = "[Camera] Shake Screen.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SHAKE_INTENSITY;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "The screen will shake with ";

	curTemplate = &m_actionTemplates[ScriptAction::TECHTREE_MODIFY_BUILDABILITY_OBJECT];
	curTemplate->m_name = "[Map] Adjust the tech tree for a specific object type.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[1] = Parameter::BUILDABLE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " becomes ";

 	curTemplate = &m_actionTemplates[ScriptAction::SET_CAVE_INDEX];
	curTemplate->m_name = "[Unit] Set Cave connectivity index.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Cave named ";
	curTemplate->m_uiStrings[1] = " is set to being connected to all caves of index ";
	curTemplate->m_uiStrings[2] = ", but only if both Cave listings have no occupants. ";

 	curTemplate = &m_actionTemplates[ScriptAction::WAREHOUSE_SET_VALUE];
	curTemplate->m_name = "[Unit] Set cash value of Warehouse.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Warehouse named ";
	curTemplate->m_uiStrings[1] = " is set to having ";
	curTemplate->m_uiStrings[2] = " dollars worth of boxes. ";

 	curTemplate = &m_actionTemplates[ScriptAction::SOUND_DISABLE_TYPE];
	curTemplate->m_name = "[Multimedia] Sound Events -- disable type.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SOUND;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is disabled.";

 	curTemplate = &m_actionTemplates[ScriptAction::SOUND_ENABLE_TYPE];
	curTemplate->m_name = "[Multimedia] Sound Events -- enable type.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SOUND;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is enabled.";

 	curTemplate = &m_actionTemplates[ScriptAction::SOUND_REMOVE_TYPE];
	curTemplate->m_name = "[Multimedia] Sound Events -- remove type.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SOUND;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is removed.";

 	curTemplate = &m_actionTemplates[ScriptAction::SOUND_REMOVE_ALL_DISABLED];
	curTemplate->m_name = "[Multimedia] Sound Events -- remove all disabled.";
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Remove all disabled sound events.";

 	curTemplate = &m_actionTemplates[ScriptAction::SOUND_ENABLE_ALL];
	curTemplate->m_name = "[Multimedia] Sound Events -- enable all.";
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Enable all sound events.";

 	curTemplate = &m_actionTemplates[ScriptAction::AUDIO_OVERRIDE_VOLUME_TYPE];
	curTemplate->m_name = "[Multimedia] Sound Events -- override volume -- type.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SOUND;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " play at ";
	curTemplate->m_uiStrings[2] = "% of full volume.";

 	curTemplate = &m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_TYPE];
	curTemplate->m_name = "[Multimedia] Sound Events -- restore volume -- type.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SOUND;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " play at normal volume.";

 	curTemplate = &m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_ALL_TYPE];
	curTemplate->m_name = "[Multimedia] Sound Events -- restore volume -- all.";
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "All sound events play at normal volume.";

 	curTemplate = &m_actionTemplates[ScriptAction::NAMED_SET_TOPPLE_DIRECTION];
	curTemplate->m_name = "[Unit] Set topple direction.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::COORD3D;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " will topple towards ";
	curTemplate->m_uiStrings[2] = " if destroyed.";

 	curTemplate = &m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE];
	curTemplate->m_name = "[Unit] Move unit towards the nearest object of a specific type.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[2] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " will move towards the nearest ";
	curTemplate->m_uiStrings[2] = " within ";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE];
	curTemplate->m_name = "[Team] Move team towards the nearest object of a specific type.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[2] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " will move towards the nearest ";
	curTemplate->m_uiStrings[2] = " within ";
	
 	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE];
	curTemplate->m_name = "[Skirmish] Team attacks nearest group matching value comparison.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " attacks nearest group worth ";
	curTemplate->m_uiStrings[2] = " ";

 	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT];
	curTemplate->m_name = "[Skirmish] Team performs command ability on most valuable object.";
	curTemplate->m_numParameters = 4;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_parameters[3] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 5;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " performs ";
	curTemplate->m_uiStrings[2] = " on most expensive object within ";	
	curTemplate->m_uiStrings[3] = " ";
	curTemplate->m_uiStrings[4] = " (true = all valid sources, false = first valid source).";

 	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL];
	curTemplate->m_name = "[Skirmish] Delay a sequential script until the specified command ability is ready - all.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_parameters[2] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " 's ";
	curTemplate->m_uiStrings[2] = " all wait until ";	
	curTemplate->m_uiStrings[3] = " is ready.";
	
 	curTemplate = &m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL];
	curTemplate->m_name = "[Skirmish] Delay a sequential script until the specified command ability is ready - partial.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_parameters[2] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " 's ";
	curTemplate->m_uiStrings[2] = " wait until at least one member is ";	
	curTemplate->m_uiStrings[3] = " ready.";
	
 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_SPIN_FOR_FRAMECOUNT];
	curTemplate->m_name = "[Team] Set to continue current action for some number of frames.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " continue their current action for at least ";
	curTemplate->m_uiStrings[2] = " frames.";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED];
	curTemplate->m_name = "[Team] Use command ability -- all -- named enemy";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_parameters[2] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = "  on ";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT];
	curTemplate->m_name = "[Team] Use command ability -- all -- nearest enemy unit";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = "  on nearest enemy unit.";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING];
	curTemplate->m_name = "[Team] Use command ability -- all -- nearest enemy garrisoned building.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = "  on nearest enemy garrisoned building.";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF];
	curTemplate->m_name = "[Team] Use command ability -- all -- nearest enemy object with kind of.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_parameters[2] = Parameter::KIND_OF_PARAM;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = "  on nearest enemy with ";
	curTemplate->m_uiStrings[4] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING];
	curTemplate->m_name = "[Team] Use command ability -- all -- nearest enemy building.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = "  on nearest enemy building.";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS];
	curTemplate->m_name = "[Team] Use command ability -- all -- nearest enemy building kindof.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_parameters[2] = Parameter::KIND_OF_PARAM;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = "  on nearest enemy building with ";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE];
	curTemplate->m_name = "[Team] Use command ability -- all -- nearest object type.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_parameters[2] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = " on nearest object of type ";
	curTemplate->m_uiStrings[3] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON];
	curTemplate->m_name = "[Team] Use command ability -- partial -- self.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_parameters[2] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = "% of ";
	curTemplate->m_uiStrings[2] = " perform ";
	curTemplate->m_uiStrings[3] = ".";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT];
	curTemplate->m_name = "[Team] Capture unowned faction unit -- nearest.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " capture the nearest unowned faction unit.";

 	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS];
	curTemplate->m_name = "[Player] Create team from all captured units.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " creates a new ";
	curTemplate->m_uiStrings[2] = " from units it has captured. (There's nothing quite like being assaulted by your own captured units!)";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_ALL];
	curTemplate->m_name = "[Team] Delay a sequential script until the team is no longer contained - all";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " all delay until they are no longer contained.";

 	curTemplate = &m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL];
	curTemplate->m_name = "[Team] Delay a sequential script until the team is no longer contained - partial";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " delay until at least one of them is no longer contained.";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON];
	curTemplate->m_name = "[Team] Set emoticon for duration (-1.0 permanent, otherwise duration in sec).";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::EMOTICON;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = " emoticon for ";
	curTemplate->m_uiStrings[3] = " seconds.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON];
	curTemplate->m_name = "[Unit] Set emoticon for duration (-1.0 permanent, otherwise duration in sec).";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::EMOTICON;
	curTemplate->m_parameters[2] = Parameter::REAL;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " use ";
	curTemplate->m_uiStrings[2] = " emoticon for ";
	curTemplate->m_uiStrings[3] = " seconds.";

 	curTemplate = &m_actionTemplates[ScriptAction::OBJECTLIST_ADDOBJECTTYPE];
	curTemplate->m_name = "[Scripting] Object Type List -- Add Object Type.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::OBJECT_TYPE_LIST;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " : add ";

 	curTemplate = &m_actionTemplates[ScriptAction::OBJECTLIST_REMOVEOBJECTTYPE];
	curTemplate->m_name = "[Scripting] Object Type List -- Remove Object Type.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::OBJECT_TYPE_LIST;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " : remove ";

 	curTemplate = &m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT];
	curTemplate->m_name = "[Map] Reveal map at waypoint -- permanently.";
	curTemplate->m_numParameters = 4;
	curTemplate->m_parameters[0] = Parameter::WAYPOINT;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::SIDE;
	curTemplate->m_parameters[3] = Parameter::REVEALNAME;
	curTemplate->m_numUiStrings = 5;
	curTemplate->m_uiStrings[0] = "The map is permanently revealed at ";
	curTemplate->m_uiStrings[1] = " with a radius of ";
	curTemplate->m_uiStrings[2] = " for ";
	curTemplate->m_uiStrings[3] = ". (Afterwards referred to as ";
	curTemplate->m_uiStrings[4] = ").";

 	curTemplate = &m_actionTemplates[ScriptAction::MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT];
	curTemplate->m_name = "[Map] Reveal map at waypoint -- undo permanently.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::REVEALNAME;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is undone.";

 	curTemplate = &m_actionTemplates[ScriptAction::EVA_SET_ENABLED_DISABLED];
	curTemplate->m_name = "[Scripting] Enable or Disable EVA.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set EVA to be enabled ";
	curTemplate->m_uiStrings[1] = " (False to disable.)";

 	curTemplate = &m_actionTemplates[ScriptAction::OPTIONS_SET_OCCLUSION_MODE];
	curTemplate->m_name = "[Scripting] Enable or Disable Occlusion (Drawing Behind Buildings).";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set Occlusion to be enabled ";
	curTemplate->m_uiStrings[1] = " (False to disable.)";

	curTemplate = &m_actionTemplates[ScriptAction::OPTIONS_SET_DRAWICON_UI_MODE];
	curTemplate->m_name = "[Scripting] Enable or Disable Draw-icon UI.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set Draw-icon UI to be enabled ";
	curTemplate->m_uiStrings[1] = " (False to disable.)";

	curTemplate = &m_actionTemplates[ScriptAction::OPTIONS_SET_PARTICLE_CAP_MODE];
	curTemplate->m_name = "[Scripting] Enable or Disable Particle Cap.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set Particle Cap to be enabled ";
	curTemplate->m_uiStrings[1] = " (False to disable.)";

	curTemplate = &m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS];
	curTemplate->m_name = "[Unit] Affect flags set on object panel.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::OBJECT_PANEL_FLAG;
	curTemplate->m_parameters[2] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = " ";
	curTemplate->m_uiStrings[1] = " changes the value of flag ";
	curTemplate->m_uiStrings[2] = " to ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS];
	curTemplate->m_name = "[Team] Affect flags set on object panel - all.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::OBJECT_PANEL_FLAG;
	curTemplate->m_parameters[2] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = " ";
	curTemplate->m_uiStrings[1] = " change the value of flag ";
	curTemplate->m_uiStrings[2] = " to ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_SELECT_SKILLSET];
	curTemplate->m_name = "[Player] Set the skillset for a computer player.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = " ";
	curTemplate->m_uiStrings[1] = " uses skillset number ";
	curTemplate->m_uiStrings[2] = " (1-5).";

	curTemplate = &m_actionTemplates[ScriptAction::SCRIPTING_OVERRIDE_HULK_LIFETIME ];
	curTemplate->m_name = "[Scripting] Hulk set override lifetime.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Override hulk lifetime to ";
	curTemplate->m_uiStrings[1] = " seconds. Negative value reverts to normal behavior.";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_FACE_NAMED];
	curTemplate->m_name = "[Unit] Set unit to face another unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begin facing ";

	curTemplate = &m_actionTemplates[ScriptAction::NAMED_FACE_WAYPOINT];
	curTemplate->m_name = "[Unit] Set unit to face a waypoint.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begin facing ";
	
	curTemplate = &m_actionTemplates[ScriptAction::TEAM_FACE_NAMED];
	curTemplate->m_name = "[Team] Set team to face another unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begin facing ";

	curTemplate = &m_actionTemplates[ScriptAction::TEAM_FACE_WAYPOINT];
	curTemplate->m_name = "[Team] Set team to face a waypoint.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " begin facing ";
	
	curTemplate = &m_actionTemplates[ScriptAction::COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE];
	curTemplate->m_name = "[Scripting] Remove a command button from an object type.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::COMMAND_BUTTON;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = " ";
	curTemplate->m_uiStrings[1] = " is removed from all objects of type ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT];
	curTemplate->m_name = "[Scripting] Add a command button to an object type.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::COMMAND_BUTTON;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = " ";
	curTemplate->m_uiStrings[1] = " is added to all objects of type ";
	curTemplate->m_uiStrings[2] = " in slot number ";
	curTemplate->m_uiStrings[3] = " (1-12).";

	curTemplate = &m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION];
	curTemplate->m_name = "[Unit] Spawn -- named unit on a team at a position with an orientation.";
	curTemplate->m_numParameters = 5;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[2] = Parameter::TEAM;
	curTemplate->m_parameters[3] = Parameter::COORD3D;
	curTemplate->m_parameters[4] = Parameter::ANGLE;
	curTemplate->m_numUiStrings = 6;
	curTemplate->m_uiStrings[0] = "Spawn ";
	curTemplate->m_uiStrings[1] = " of type ";
	curTemplate->m_uiStrings[2] = " on team ";
	curTemplate->m_uiStrings[3] = " at position (";
	curTemplate->m_uiStrings[4] = "), rotated ";
	curTemplate->m_uiStrings[5] = " .";

	curTemplate = &m_actionTemplates[ScriptAction::PLAYER_AFFECT_RECEIVING_EXPERIENCE];
	curTemplate->m_name = "[Player] Change the modifier to generals experience that a player receives.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = " ";
	curTemplate->m_uiStrings[1] = " gains experience at ";
	curTemplate->m_uiStrings[2] = " times the usual rate (0.0 for no gain, 1.0 for normal rate)";

	curTemplate = &m_actionTemplates[ScriptAction::SOUND_SET_VOLUME];
	curTemplate->m_name = "[Multimedia] Set the current sound volume.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set the desired sound volume to ";
	curTemplate->m_uiStrings[1] = "%. (0-100)";

	curTemplate = &m_actionTemplates[ScriptAction::SPEECH_SET_VOLUME];
	curTemplate->m_name = "[Multimedia] Set the current speech volume.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Set the desired speech volume to ";
	curTemplate->m_uiStrings[1] = "%. (0-100)";

	curTemplate = &m_actionTemplates[ScriptAction::OBJECT_ALLOW_BONUSES];
	curTemplate->m_name = "[Map] Adjust Object Bonuses based on difficulty.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Enable Object Bonuses based on difficulty ";
	curTemplate->m_uiStrings[1] = " (true to enable, false to disable).";
	
	curTemplate = &m_actionTemplates[ScriptAction::TEAM_GUARD_IN_TUNNEL_NETWORK];
	curTemplate->m_name = "[Team] Set to guard - from inside tunnel network.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " Enter and guard from tunnel network.";

	curTemplate = &m_actionTemplates[ScriptAction::LOCALDEFEAT];
	curTemplate->m_name = "[Multiplayer] Announce local defeat.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Show 'Game Over' window";

	curTemplate = &m_actionTemplates[ScriptAction::VICTORY];
	curTemplate->m_name = "[Multiplayer] Announce victory.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Show 'Victorious' window and end game";

	curTemplate = &m_actionTemplates[ScriptAction::DEFEAT];
	curTemplate->m_name = "[Multiplayer] Announce defeat.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Show 'Defeated' window and end game";

	curTemplate = &m_actionTemplates[ScriptAction::RESIZE_VIEW_GUARDBAND];
	curTemplate->m_name = "[Map] Resize view guardband.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::REAL;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Allow bigger objects to be perceived as onscreen near the edge (" ;
	curTemplate->m_uiStrings[1] = ",";
	curTemplate->m_uiStrings[2] = ") Width then height, in world units.";

	curTemplate = &m_actionTemplates[ScriptAction::DELETE_ALL_UNMANNED];
	curTemplate->m_name = "[Scripting] Delete all unmanned (sniped) vehicles.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Delete all unmanned (sniped) vehicles." ;

	curTemplate = &m_actionTemplates[ScriptAction::CHOOSE_VICTIM_ALWAYS_USES_NORMAL];
	curTemplate->m_name = "[Map] Force ChooseVictim to ignore game difficulty and always use Normal setting.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Force ChooseVictim to ignore game difficulty and always use Normal setting ";
	curTemplate->m_uiStrings[1] = " (true to enable, false to disable).";

	///////////////////////////////////////////////////////////////////////////////////////////////////

	/* Recipe for adding a condition:
			1. In Scripts.h, add an enum element to enum ConditionType just before NUM_ITEMS.
			2. Go to the end of this section of templates, and create a template.
			3. Go to ScriptConditions.h and add a protected method.
			4. Go to ScriptConditions.cpp, and add your enum to the 
					switch in ScriptConditions::evaluateCondition to call your method in 3 above.
	*/

	// Set up condition templates.
	curTemplate = &m_conditionTemplates[Condition::CONDITION_FALSE];
	curTemplate->m_name = "[Scripting] False.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "False.";

	curTemplate = &m_conditionTemplates[Condition::COUNTER];
	curTemplate->m_name = "[Scripting] Counter compared to a value.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Counter ";
	curTemplate->m_uiStrings[1] = " IS ";
	curTemplate->m_uiStrings[2] = " ";

	curTemplate = &m_conditionTemplates[Condition::UNIT_HEALTH];
	curTemplate->m_name = "[Unit] Unit health % compared to a value.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " Health IS ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " percent.";

	curTemplate = &m_conditionTemplates[Condition::FLAG];
	curTemplate->m_name = "[Scripting] Flag compared to a value.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::FLAG;
	curTemplate->m_parameters[1] = Parameter::BOOLEAN;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " IS ";

	curTemplate = &m_conditionTemplates[Condition::TEAM_STATE_IS];
	curTemplate->m_name = "[Team] Team state is.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TEAM_STATE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " state IS ";

	curTemplate = &m_conditionTemplates[Condition::TEAM_STATE_IS_NOT];
	curTemplate->m_name = "[Team] Team state is not.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TEAM_STATE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " state IS NOT ";
																										 
	curTemplate = &m_conditionTemplates[Condition::CONDITION_TRUE];
	curTemplate->m_name = "[Scripting] True.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "True.";

	curTemplate = &m_conditionTemplates[Condition::TIMER_EXPIRED];
	curTemplate->m_name = "[Scripting] Timer expired.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::COUNTER;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Timer ";
	curTemplate->m_uiStrings[1] = " has expired.";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_ALL_DESTROYED];
	curTemplate->m_name = "[Player] All destroyed.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "Everything belonging to  ";
	curTemplate->m_uiStrings[1] = " has been destroyed.";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_ALL_BUILDFACILITIES_DESTROYED];
	curTemplate->m_name = "[Player] All factories destroyed.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "All factories belonging to  ";
	curTemplate->m_uiStrings[1] = " have been destroyed.";


	curTemplate = &m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY];
	curTemplate->m_name = "[Team] Team has units in an area.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_parameters[2] = Parameter::SURFACES_ALLOWED;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
 	curTemplate->m_uiStrings[1] = " has one or more units in ";
	curTemplate->m_uiStrings[2] = " (";
	curTemplate->m_uiStrings[3] = ").";

	curTemplate = &m_conditionTemplates[Condition::NAMED_INSIDE_AREA];
	curTemplate->m_name = "[Unit] Unit entered area.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
 	curTemplate->m_uiStrings[1] = " is in ";
	curTemplate->m_uiStrings[2] = " (";
	curTemplate->m_uiStrings[3] = ").";


	curTemplate = &m_conditionTemplates[Condition::TEAM_DESTROYED];
	curTemplate->m_name = "[Team] Team is destroyed.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been destroyed.";

	curTemplate = &m_conditionTemplates[Condition::NAMED_DESTROYED];
	curTemplate->m_name = "[Unit] Unit is destroyed.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been destroyed.";

	curTemplate = &m_conditionTemplates[Condition::NAMED_DYING];
	curTemplate->m_name = "[Unit] Unit is dying.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been killed, but still on screen.";

	curTemplate = &m_conditionTemplates[Condition::NAMED_TOTALLY_DEAD];
	curTemplate->m_name = "[Unit] Unit is finished dying.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been killed, and is finished dying.";

	curTemplate = &m_conditionTemplates[Condition::BRIDGE_BROKEN];
	curTemplate->m_name = "[Unit] Bridge is broken.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::BRIDGE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been broken.";

	curTemplate = &m_conditionTemplates[Condition::BRIDGE_REPAIRED];
	curTemplate->m_name = "[Unit] Bridge is repaired.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::BRIDGE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been repaired.";

	curTemplate = &m_conditionTemplates[Condition::NAMED_NOT_DESTROYED];
	curTemplate->m_name = "[Unit] Unit exists and is alive.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " exists and is alive.";

	curTemplate = &m_conditionTemplates[Condition::TEAM_HAS_UNITS];
	curTemplate->m_name = "[Team] Team has units.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has one or more units.";

	curTemplate = &m_conditionTemplates[Condition::CAMERA_MOVEMENT_FINISHED];
	curTemplate->m_name = "[Camera] Camera movement finished.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "The camera movement has finished.";

	curTemplate = &m_conditionTemplates[Condition::NAMED_INSIDE_AREA];
	curTemplate->m_name = "[Unit] Unit inside an area.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is inside ";

	curTemplate = &m_conditionTemplates[Condition::NAMED_OUTSIDE_AREA];
	curTemplate->m_name = "[Unit] Unit outside an area.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is outside ";	

	curTemplate = &m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY];
	curTemplate->m_name = "[Team] Team completely inside an area.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_parameters[2] = Parameter::SURFACES_ALLOWED;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is all inside ";
	curTemplate->m_uiStrings[2] = " (";
	curTemplate->m_uiStrings[3] = ").";

	curTemplate = &m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY];
	curTemplate->m_name = "[Team] Team is completely outside an area.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_parameters[2] = Parameter::SURFACES_ALLOWED;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is completely outside ";
	curTemplate->m_uiStrings[2] = " (";
	curTemplate->m_uiStrings[3] = ").";

	curTemplate = &m_conditionTemplates[Condition::NAMED_ATTACKED_BY_OBJECTTYPE];
	curTemplate->m_name = "[Unit] Unit is attacked by a specific unit type.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been attacked by a(n) ";

	curTemplate = &m_conditionTemplates[Condition::TEAM_ATTACKED_BY_OBJECTTYPE];
	curTemplate->m_name = "[Team] Team is attacked by a specific unit type.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been attacked by a(n) ";

	curTemplate = &m_conditionTemplates[Condition::NAMED_ATTACKED_BY_PLAYER];
	curTemplate->m_name = "[Unit] Unit has been attacked by a player.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been attacked by ";

	curTemplate = &m_conditionTemplates[Condition::TEAM_ATTACKED_BY_PLAYER];
	curTemplate->m_name = "[Team] Team has been attacked by a player.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been attacked by ";

	curTemplate = &m_conditionTemplates[Condition::BUILT_BY_PLAYER];
	curTemplate->m_name = "[Player] Player has built an object type.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been built by ";

	curTemplate = &m_conditionTemplates[Condition::NAMED_CREATED];
	curTemplate->m_name = "[Unit] Unit has been created.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been created.";

	curTemplate = &m_conditionTemplates[Condition::TEAM_CREATED];
	curTemplate->m_name = "[Team] Team has been created.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been created.";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_HAS_CREDITS];
	curTemplate->m_name = "[Player] Player has (comparison) to a number of credits.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::INT;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is ";
	curTemplate->m_uiStrings[2] = " the number of credits possessed by ";

	curTemplate = &m_conditionTemplates[Condition::NAMED_DISCOVERED];
	curTemplate->m_name = "[Player] Player has discovered a specific unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been discovered by ";

	curTemplate = &m_conditionTemplates[Condition::NAMED_BUILDING_IS_EMPTY];
	curTemplate->m_name = "[Unit] A specific building is empty.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
 	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is empty.";

	curTemplate = &m_conditionTemplates[Condition::BUILDING_ENTERED_BY_PLAYER];
	curTemplate->m_name = "[Player] Player has entered a specific building.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::UNIT;
 	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has entered building named ";

	curTemplate = &m_conditionTemplates[Condition::ENEMY_SIGHTED];
	curTemplate->m_name = "[Unit] Unit has sighted a(n) friendly/neutral/enemy unit belonging to a side.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::RELATION;
	curTemplate->m_parameters[2] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " sees a(n) ";
	curTemplate->m_uiStrings[2] = " unit belonging to ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_conditionTemplates[Condition::TYPE_SIGHTED];
	curTemplate->m_name = "[Unit] Unit has sighted a type of unit belonging to a side.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[2] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " sees a(n) ";
	curTemplate->m_uiStrings[2] = " belonging to ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_conditionTemplates[Condition::TEAM_DISCOVERED];
	curTemplate->m_name = "[Player] Player has discovered a team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been discovered by ";

	curTemplate = &m_conditionTemplates[Condition::MISSION_ATTEMPTS];
	curTemplate->m_name = "[Player] Player has attempted the mission a number of times.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has attempted the mission ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " times.";

	curTemplate = &m_conditionTemplates[Condition::NAMED_OWNED_BY_PLAYER];
	curTemplate->m_name = "[Player] Player owns the specific Unit.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is owned by ";

	curTemplate = &m_conditionTemplates[Condition::TEAM_OWNED_BY_PLAYER];
	curTemplate->m_name = "[Player] Player owns a specific team.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is owned by ";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS];
	curTemplate->m_name = "[Player] Player currently owns N or fewer buildings.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " currently owns ";
	curTemplate->m_uiStrings[2] = " or fewer buildings.";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS];
	curTemplate->m_name = "[Player] Player currently owns N or fewer faction buildings.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " currently owns ";
	curTemplate->m_uiStrings[2] = " or fewer faction buildings.";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_HAS_POWER];
	curTemplate->m_name = "[Player] Player's base currently has power.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " buildings are powered.";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_HAS_NO_POWER];
	curTemplate->m_name = "[Player] Player's base currently has no power.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " buildings are not powered.";

	curTemplate = &m_conditionTemplates[Condition::NAMED_REACHED_WAYPOINTS_END];
	curTemplate->m_name = "[Unit] Unit has reached the end of a specific waypoint path.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT_PATH;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has reached the end of ";

	curTemplate = &m_conditionTemplates[Condition::TEAM_REACHED_WAYPOINTS_END];
	curTemplate->m_name = "[Team] Team has reached the end of a specific waypoint path.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::WAYPOINT_PATH;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has reached the end of ";

	curTemplate = &m_conditionTemplates[Condition::NAMED_SELECTED];
	curTemplate->m_name = "[Unit] Unit currently selected.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is currently selected.";

	curTemplate = &m_conditionTemplates[Condition::NAMED_ENTERED_AREA];
	curTemplate->m_name = "[Unit] Unit enters an area.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " enters ";

	curTemplate = &m_conditionTemplates[Condition::NAMED_EXITED_AREA];
	curTemplate->m_name = "[Unit] Unit exits an area.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " exits ";

	curTemplate = &m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY];
	curTemplate->m_name = "[Team] Team entirely enters an area.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_parameters[2] = Parameter::SURFACES_ALLOWED;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " all enter ";
	curTemplate->m_uiStrings[2] = " (";
	curTemplate->m_uiStrings[3] = ").";



	curTemplate = &m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY];
	curTemplate->m_name = "[Team] One unit enters an area.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_parameters[2] = Parameter::SURFACES_ALLOWED;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "One unit from ";
	curTemplate->m_uiStrings[1] = " enters ";
	curTemplate->m_uiStrings[2] = " (";
	curTemplate->m_uiStrings[3] = ").";

	curTemplate = &m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY];
	curTemplate->m_name = "[Team] Team entirely exits an area.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_parameters[2] = Parameter::SURFACES_ALLOWED;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " all exit ";
	curTemplate->m_uiStrings[2] = " (";
	curTemplate->m_uiStrings[3] = ").";

	curTemplate = &m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY];
	curTemplate->m_name = "[Team] One unit exits an area.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_parameters[2] = Parameter::SURFACES_ALLOWED;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "One unit from ";
	curTemplate->m_uiStrings[1] = " exits ";
	curTemplate->m_uiStrings[2] = " (";
	curTemplate->m_uiStrings[3] = ").";

	curTemplate = &m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_VICTORY];
	curTemplate->m_name = "[Multiplayer] Multiplayer allied victory.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "The multiplayer game has ended in victory for the local player and his allies.";

	curTemplate = &m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_DEFEAT];
	curTemplate->m_name = "[Multiplayer] Multiplayer allied defeat.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "The multiplayer game has ended in defeat for the local player and his allies.";

	curTemplate = &m_conditionTemplates[Condition::MULTIPLAYER_PLAYER_DEFEAT];
	curTemplate->m_name = "[Multiplayer] Multiplayer local player defeat check.";
	curTemplate->m_numParameters = 0;
	curTemplate->m_numUiStrings = 1;
	curTemplate->m_uiStrings[0] = "Everything belonging to the local player has been destroyed, but his allies may or may not have been defeated.";

	curTemplate = &m_conditionTemplates[Condition::HAS_FINISHED_VIDEO];
	curTemplate->m_name = "[Multimedia] Video has completed playing.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::MOVIE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has completed playing.";

	curTemplate = &m_conditionTemplates[Condition::HAS_FINISHED_SPEECH];
	curTemplate->m_name = "[Multimedia] Speech has completed playing.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::DIALOG;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has completed playing.";

	curTemplate = &m_conditionTemplates[Condition::HAS_FINISHED_AUDIO];
	curTemplate->m_name = "[Multimedia] Sound has completed playing.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SOUND;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has completed playing.";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER];
	curTemplate->m_name = "[Player] Player starts using a special power.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " starts using ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED];
	curTemplate->m_name = "[Player] Player start using a special power from a named unit.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_parameters[2] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " starts using ";
	curTemplate->m_uiStrings[2] = " from ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER];
	curTemplate->m_name = "[Player] Player is midway through using a special power.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " is midway using ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED];
	curTemplate->m_name = "[Player] Player is midway through using a special power from a named unit.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_parameters[2] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " is midway using ";
	curTemplate->m_uiStrings[2] = " from ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER];
	curTemplate->m_name = "[Player] Player completed using a special power.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " completed using ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED];
	curTemplate->m_name = "[Player] Player completed using a special power from a named unit.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_parameters[2] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " completed using ";
	curTemplate->m_uiStrings[2] = " from ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_ACQUIRED_SCIENCE];
	curTemplate->m_name = "[Player] Player acquired a Science.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SCIENCE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " acquired ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_CAN_PURCHASE_SCIENCE];
	curTemplate->m_name = "[Player] Player can purchase a particular Science (has all prereqs & points).";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SCIENCE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " can purchase ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS];
	curTemplate->m_name = "[Player] Player has a certain number of Science Purchase Points available.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " has at least ";
	curTemplate->m_uiStrings[2] = " Science Purchase Points available.";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE];
	curTemplate->m_name = "[Player] Player built an upgrade.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::UPGRADE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " built ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED];
	curTemplate->m_name = "[Player] Player built an upgrade from a named unit.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::UPGRADE;
	curTemplate->m_parameters[2] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " built ";
	curTemplate->m_uiStrings[2] = " from ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER];
	curTemplate->m_name = "[Player] Player destroyed N or more of an opponent's buildings.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_parameters[2] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "Player ";
	curTemplate->m_uiStrings[1] = " destroyed ";
	curTemplate->m_uiStrings[2] = " or more buildings owned by ";
	curTemplate->m_uiStrings[3] = ".";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON];
	curTemplate->m_name = "[Player] Player has (comparison) unit type.";
	curTemplate->m_numParameters = 4;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_parameters[3] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " unit or structure of type ";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA];
	curTemplate->m_name = "[Player] Player has (comparison) unit type in an area.";
	curTemplate->m_numParameters = 5;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_parameters[3] = Parameter::OBJECT_TYPE;
	curTemplate->m_parameters[4] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 5;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " unit or structure of type ";
	curTemplate->m_uiStrings[4] = " in the ";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA];
	curTemplate->m_name = "[Player] Player has (comparison) kind of unit or structure in an area.";
	curTemplate->m_numParameters = 5;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_parameters[3] = Parameter::KIND_OF_PARAM;
	curTemplate->m_parameters[4] = Parameter::TRIGGER_AREA	;
	curTemplate->m_numUiStrings = 5;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " unit or structure with ";
	curTemplate->m_uiStrings[4] = " in the ";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT];
	curTemplate->m_name = "[Player] Player has (comparison) percent power supply to consumption.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " percent power supply ratio.";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE];
	curTemplate->m_name = "[Player] Player has (comparison) kilowatts excess power supply.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " excess kilowatts power supply.";

	curTemplate = &m_conditionTemplates[Condition::UNIT_EMPTIED];
	curTemplate->m_name = "[Unit] Unit has emptied its contents.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " emptied its contents.";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_SPECIAL_POWER_READY];
	curTemplate->m_name = "[Skirmish] Player's special power is ready to fire.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SPECIAL_POWER;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is ready to fire ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::UNIT_HAS_OBJECT_STATUS];
	curTemplate->m_name = "[Unit] Unit has object status.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::UNIT;
	curTemplate->m_parameters[1] = Parameter::OBJECT_STATUS;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has ";

	curTemplate = &m_conditionTemplates[Condition::TEAM_ALL_HAS_OBJECT_STATUS];
	curTemplate->m_name = "[Team] Team has object status - all.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::OBJECT_STATUS;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has ";

	curTemplate = &m_conditionTemplates[Condition::TEAM_SOME_HAVE_OBJECT_STATUS];
	curTemplate->m_name = "[Team] Team has object status - partial.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::TEAM;
	curTemplate->m_parameters[1] = Parameter::OBJECT_STATUS;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has ";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA];
	curTemplate->m_name = "[Skirmish Only] Player has total value in area.";
	curTemplate->m_numParameters = 4;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_parameters[3] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " within area ";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_PLAYER_FACTION];
	curTemplate->m_name = "[Skirmish] Player is faction. - untested";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::FACTION_NAME;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " is ";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE];
	curTemplate->m_name = "[Skirmish Only] Supplies are within specified distance.";
	curTemplate->m_numParameters = 4;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::TRIGGER_AREA;
	curTemplate->m_parameters[3] = Parameter::REAL;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has supplies within ";
	curTemplate->m_uiStrings[2] = " of ";
	curTemplate->m_uiStrings[3] = " worth at least ";


	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE];
	curTemplate->m_name = "[Skirmish Only] Tech building is within specified distance.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::REAL;
	curTemplate->m_parameters[2] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has a tech building within ";
	curTemplate->m_uiStrings[2] = " of ";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL];
	curTemplate->m_name = "[Skirmish] Command Ability is ready - all.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_parameters[2] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = "'s ";
	curTemplate->m_uiStrings[2] = " are ready to use ";
	curTemplate->m_uiStrings[3] = " (all applicable members).";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL];
	curTemplate->m_name = "[Skirmish] Command Ability is ready - partial";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::TEAM;
	curTemplate->m_parameters[2] = Parameter::COMMANDBUTTON_ALL_ABILITIES;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = "'s ";
	curTemplate->m_uiStrings[2] = " are ready to use ";
	curTemplate->m_uiStrings[3] = " (at least one member).";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS];
	curTemplate->m_name = "[Skirmish] Unowned faction unit -- comparison.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = ". There are ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " unowned faction units.";
	
	
	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD];
	curTemplate->m_name = "[Skirmish] Player has prerequisites to build an object type.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " can build ";
	curTemplate->m_uiStrings[2] = ".";
	
	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED];
	curTemplate->m_name = "[Skirmish] Player has garrisoned buildings -- comparison.";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " garrisoned buildings.";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS];
	curTemplate->m_name = "[Skirmish] Player has captured units -- comparison";
	curTemplate->m_numParameters = 3;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::COMPARISON;
	curTemplate->m_parameters[2] = Parameter::INT;
	curTemplate->m_numUiStrings = 4;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has captured ";
	curTemplate->m_uiStrings[2] = " ";
	curTemplate->m_uiStrings[3] = " units.";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_NAMED_AREA_EXIST];
	curTemplate->m_name = "[Skirmish] Area exists.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = ". ";
	curTemplate->m_uiStrings[2] = " exists.";
	
	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA];
	curTemplate->m_name = "[Skirmish] Player has units in an area";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has units in ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER];
	curTemplate->m_name = "[Skirmish] Player has been attacked by player.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has been attacked by ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA];
	curTemplate->m_name = "[Skirmish] Player doesn't have units in an area.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::TRIGGER_AREA;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has doesn't have units in ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER];
	curTemplate->m_name = "[Skirmish] Player has discovered another player.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has discovered ";
	curTemplate->m_uiStrings[2] = ".";

	curTemplate = &m_conditionTemplates[Condition::MUSIC_TRACK_HAS_COMPLETED];
	curTemplate->m_name = "[Multimedia] Music track has completed some number of times.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::MUSIC;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has completed at least ";
	curTemplate->m_uiStrings[2] = " times. (NOTE: This can only be used to "
		"start other music. USING THIS SCRIPT IN ANY OTHER WAY WILL CAUSE REPLAYS TO NOT WORK.)";

	curTemplate = &m_conditionTemplates[Condition::SUPPLY_SOURCE_SAFE];
	curTemplate->m_name = "[Skirmish] Supply source is safe.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " closest supply src with at least ";
	curTemplate->m_uiStrings[2] = " available resources is SAFE from enemy influence.";

	curTemplate = &m_conditionTemplates[Condition::SUPPLY_SOURCE_ATTACKED];
	curTemplate->m_name = "[Skirmish] Supply source is attacked.";
	curTemplate->m_numParameters = 1;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_numUiStrings = 2;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " supply source is under attack.";

	curTemplate = &m_conditionTemplates[Condition::START_POSITION_IS];
	curTemplate->m_name = "[Skirmish] Start position.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::INT;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " starting position is ";
	curTemplate->m_uiStrings[2] = " .";

	curTemplate = &m_conditionTemplates[Condition::PLAYER_LOST_OBJECT_TYPE];
	curTemplate->m_name = "[Player] Player has lost an object of type.";
	curTemplate->m_numParameters = 2;
	curTemplate->m_parameters[0] = Parameter::SIDE;
	curTemplate->m_parameters[1] = Parameter::OBJECT_TYPE;
	curTemplate->m_numUiStrings = 3;
	curTemplate->m_uiStrings[0] = "";
	curTemplate->m_uiStrings[1] = " has lost an object of type ";
	curTemplate->m_uiStrings[2] = " (can be an object type list).";
	

	reset();

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::reset( void )
{
	// setting FPS limit in case a script had changed it
	if (TheGameEngine && TheGlobalData)
		TheGameEngine->setFramesPerSecondLimit(TheGlobalData->m_framesPerSecondLimit);

	if (TheScriptActions) {
		TheScriptActions->reset();	 
	}
	if (TheScriptConditions) {
		TheScriptConditions->reset();
	}
	m_numCounters = 1;
	m_numAttackInfo = 1;
	m_numFlags = 1;
	m_endGameTimer = -1;
	m_closeWindowTimer = -1;

	m_callingTeam = NULL;
	m_callingObject = NULL;
	m_conditionTeam = NULL;
	m_conditionObject = NULL;
	m_currentPlayer = NULL;
	m_skirmishHumanPlayer = NULL;
	m_frameObjectCountChanged = 0;

	m_shownMPLocalDefeatWindow = FALSE;

	Int i;
	for (i=0; i<MAX_COUNTERS; i++) {
		m_counters[i].value = 0;
		m_counters[i].isCountdownTimer = false;
		m_counters[i].name.clear();
	}
	for (i=0; i<MAX_FLAGS; i++) {
		m_flags[i].value = false;
		m_flags[i].name.clear();
	}

	m_breezeInfo.m_direction = PI/3;
	m_breezeInfo.m_directionVec.x = Sin(m_breezeInfo.m_direction);
	m_breezeInfo.m_directionVec.y = Cos(m_breezeInfo.m_direction);
	m_breezeInfo.m_intensity = 0.07f*PI/4;
	m_breezeInfo.m_lean = 0.07f*PI/4;
	m_breezeInfo.m_breezePeriod = LOGICFRAMES_PER_SECOND * 5;
	m_breezeInfo.m_randomness = 0.2f;
	m_breezeInfo.m_breezeVersion = 0;

	m_freezeByScript = FALSE;
	m_objectsShouldReceiveDifficultyBonus = TRUE;
	m_ChooseVictimAlwaysUsesNormal = false;

#ifdef SPECIAL_SCRIPT_PROFILING
#ifdef DEBUG_LOGGING
	if (m_numFrames > 1) {
		DEBUG_LOG(("\n***SCRIPT ENGINE STATS %.0f frames:\n", m_numFrames));
		DEBUG_LOG(("Avg time to update %.3f milisec\n", 1000*m_totalUpdateTime/m_numFrames));
		DEBUG_LOG(("  Max time to update %.3f miliseconds.\n", m_maxUpdateTime*1000));
	}
	m_numFrames=0;
	m_totalUpdateTime=0;
	m_maxUpdateTime=0;

	Int numToDump;
	if (TheSidesList) {
		for (numToDump=0; numToDump<10; numToDump++) {
			Real maxTime = 0;
			Script *maxScript = NULL;
			/* Run through scripts & set condition team names. */
			for (i=0; i<TheSidesList->getNumSides(); i++) {
				ScriptList *pSL = TheSidesList->getSideInfo(i)->getScriptList();
				if (!pSL) continue;
				if (pSL == NULL) continue;
				Script *pScr;
				for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
					if (pScr->getConditionTime()>maxTime) {
						maxTime = pScr->getConditionTime();
						maxScript = pScr;
					}
				}
				ScriptGroup *pGroup;
				for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
					for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
						if (pScr->getConditionTime()>maxTime) {
							maxTime = pScr->getConditionTime();
							maxScript = pScr;
						}
					}
				}
			}
			if (maxScript) {
				DEBUG_LOG(("   SCRIPT %s total time %f seconds,\n        evaluated %d times, avg execution %2.3f msec (Goal less than 0.05)\n",
					maxScript->getName().str(),
					maxScript->getConditionTime(), maxScript->getConditionCount(), 1000*maxScript->getConditionTime()/maxScript->getConditionCount()) );
				maxScript->addToConditionTime(-2*maxTime); // reset to negative.
			}

		}
		DEBUG_LOG(("***\n"));
	}
#endif
#endif

	_updateCurrentParticleCap();

	VecSequentialScriptPtrIt seqScriptIt;
	for (seqScriptIt = m_sequentialScripts.begin(); seqScriptIt != m_sequentialScripts.end(); ) {
		cleanupSequentialScript(seqScriptIt, TRUE);
	}

	// clear out all the lists of object types that were in the old map.
	for (AllObjectTypesIt it = m_allObjectTypeLists.begin(); it != m_allObjectTypeLists.end(); it = m_allObjectTypeLists.begin() ) {
		if (*it) {
			removeObjectTypes(*it);
		} else {
			m_allObjectTypeLists.erase(it);
		}
	}
	DEBUG_ASSERTCRASH( m_allObjectTypeLists.empty() == TRUE, ("ScriptEngine::reset - m_allObjectTypeLists should be empty but is not!\n") );

	// reset all the reveals that have taken place.
	m_namedReveals.clear();
	
	// Clear the named objects list.
 	m_namedObjects.clear();

	m_completedVideo.clear();
	m_testingSpeech.clear();
	m_testingAudio.clear();
	m_uiInteractions.clear();
	for (i=0; i<MAX_PLAYER_COUNT; ++i)
	{
		m_triggeredSpecialPowers[i].clear();
		m_midwaySpecialPowers[i].clear();
		m_finishedSpecialPowers[i].clear();
		m_acquiredSciences[i].clear();
		m_completedUpgrades[i].clear();
	}

	ScriptList::reset(); // Deletes scripts loaded when the map was loaded.

	// reset the attack priority data
	for( i = 0; i < MAX_ATTACK_PRIORITIES; ++i )
		m_attackPriorityInfo[ i ].reset();

	// clear out all of our object counts.
	for( i = 0; i < MAX_PLAYER_COUNT; ++i )
		m_objectCounts[i].clear();

	// clear topple directions
	m_toppleDirections.clear();
		
}  // end reset

//-------------------------------------------------------------------------------------------------
/** newMap */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::newMap( void )
{
	m_numCounters = 1;
	Int i;
	for (i=0; i<MAX_COUNTERS; i++) {
		m_counters[i].value = 0;
		m_counters[i].isCountdownTimer = false;
		m_counters[i].name.clear();
	}
	m_numFlags = 1;
	for (i=0; i<MAX_FLAGS; i++) {
		m_flags[i].value = false;
		m_flags[i].name.clear();
	}
	m_endGameTimer = -1;
	m_closeWindowTimer = -1;
#ifdef SPECIAL_SCRIPT_PROFILING
#ifdef DEBUG_LOGGING
	m_numFrames=0;
	m_totalUpdateTime=0;
	m_maxUpdateTime=0;
#endif
#endif

	m_completedVideo.clear();
	m_testingSpeech.clear();
	m_testingAudio.clear();
	m_uiInteractions.clear();
	for (i=0; i<MAX_PLAYER_COUNT; ++i)
	{
		m_triggeredSpecialPowers[i].clear();
		m_midwaySpecialPowers[i].clear();
		m_finishedSpecialPowers[i].clear();
		m_acquiredSciences[i].clear();
		m_completedUpgrades[i].clear();
	}

	/* Run through scripts & set condition team names. */
	for (i=0; i<TheSidesList->getNumSides(); i++) {
		ScriptList *pSL = TheSidesList->getSideInfo(i)->getScriptList();
		if (!pSL) continue;
		if (pSL == NULL) continue;
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			checkConditionsForTeamNames(pScr);
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				checkConditionsForTeamNames(pScr);
			}
		}
	}	
	m_firstUpdate = true;

	m_fade = FADE_MULTIPLY; //default to a fade in from black.
	m_curFadeFrame = 0;
	m_minFade = 1.0f;
	m_maxFade = 0.0f;
	m_fadeFramesIncrease = 0;
	m_fadeFramesHold = 0;
	m_fadeFramesDecrease = FRAMES_TO_FADE_IN_AT_START;
	m_curFadeValue = 0.0f;

}  // end newMap

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
DECLARE_PERF_TIMER(ScriptEngine)
void ScriptEngine::update( void )
{
	USE_PERF_TIMER(ScriptEngine)
#ifdef SPECIAL_SCRIPT_PROFILING
#ifdef DEBUG_LOGGING
	__int64 startTime64;
	double timeToUpdate=0.0f;
	__int64 endTime64,freq64;
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq64);//LORENZEN'S NOTE_TO_SELF: USE THIS
	QueryPerformanceCounter((LARGE_INTEGER *)&startTime64);//LORENZEN'S NOTE_TO_SELF: USE THIS
/* dump out the named objects table.  For extremely intense debug only.  jba. :P
	for (VecNamedRequestsIt it = m_namedObjects.begin(); it != m_namedObjects.end(); ++it) {
		AsciiString name = it->first;
		Object * obj = it->second;
		if (obj && obj->getAIUpdateInterface())
			DEBUG_LOG(("%s=%x('%s'), isDead%d\n", name.str(), obj, obj->getName().str(), obj->getAIUpdateInterface()->isDead()));
	}
	DEBUG_LOG(("\n\n"));
*/
#endif
#endif
	if (m_firstUpdate) {
		createNamedCache();
		particleEditorUpdate();
		m_firstUpdate = false;
	} else {
		particleEditorUpdate();
	}
	
	if (m_closeWindowTimer>0) {
		m_closeWindowTimer--;
		if (m_closeWindowTimer < 1) {
			TheScriptActions->closeWindows(FALSE); // Close victory or defeat windows.
		}
	}
	if (m_endGameTimer>0) {
		m_endGameTimer--;
		if (m_endGameTimer < 1) {
			// clear out all the game data
			/*GameMessage *msg =*/ TheMessageStream->appendMessage( GameMessage::MSG_CLEAR_GAME_DATA );
			//TheScriptActions->closeWindows(FALSE); // Close victory or defeat windows.
		}
	}
	_updateFrameNumber();
	if (isTimeFrozenDebug()) {
		st_LastCurrentFrame = st_CurrentFrame - 1;	// Force us to get clean result from CanAppContinue
		return;
	}

	if (m_fade!=FADE_NONE) {
		updateFades(); 
	}

	if (m_endGameTimer>=0) {
		return; // we are just timing down 
	}
	
	if (TheScriptActions) {
		TheScriptActions->update();
	}
	if (TheScriptConditions) {
		TheScriptConditions->update();
	}
	// Update any countdown timers.
	Int i;
	// Note - counters start at 1.  0 means not assigned.
	for (i=1; i<m_numCounters; i++) {
		if (m_counters[i].isCountdownTimer) {
			// If counter has any time left, decrement.  Counters go to -1 and stop.
			if (m_counters[i].value >= 0) {
				m_counters[i].value--;
			}
		}
	}

	// Evaluate the scripts.
	for (i=0; i<TheSidesList->getNumSides(); i++) {
		m_currentPlayer = ThePlayerList->getNthPlayer(i);
		ScriptList *pSL = TheSidesList->getSideInfo(i)->getScriptList();
		if (!pSL) continue;
		executeScripts(pSL->getScript());
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			if (!pGroup->isActive()) {
				continue; // Don't execute inactive groups.
			}
			if (pGroup->isSubroutine()) {
				continue; // Don't execute subroutine groups.
			}
			executeScripts(pGroup->getScript());
		}
		m_currentPlayer = NULL;
	}
	
	// Reset the entered/exited flag in teams, so the next update sets them 
	// correctly.  Also, execute any team created scripts.
	ThePlayerList->updateTeamStates();

	// Clear the UI Interaction flags.
	m_uiInteractions.clear();

	// update all sequential stuff.
	evaluateAndProgressAllSequentialScripts();

	// Script debugger stuff
	st_CurrentFrame++;
	if (st_DebugDLL) { 
		for (int j = 1; j < m_numCounters; ++j) {
			_adjustVariable(m_counters[j].name.str(), m_counters[j].value);
		}

		for (int k = 1; k < m_numFlags; ++k) {
			_adjustVariable(m_flags[k].name.str(), m_flags[k].value);
		}
	}
#ifdef RTS_DEBUG
	if (TheGameLogic->getFrame()==0) {
		for (i=0; i<m_numAttackInfo; i++) {
			m_attackPriorityInfo[i].dumpPriorityInfo();
		}
	}
#endif

#ifdef SPECIAL_SCRIPT_PROFILING
#ifdef DEBUG_LOGGING
	QueryPerformanceCounter((LARGE_INTEGER *)&endTime64);//LORENZEN'S NOTE_TO_SELF: USE THIS
	timeToUpdate = ((double)(endTime64-startTime64) / (double)(freq64));//LORENZEN'S NOTE_TO_SELF: USE THIS
	m_numFrames++;
	m_totalUpdateTime+=timeToUpdate;
	if (timeToUpdate > m_maxUpdateTime) m_maxUpdateTime = timeToUpdate;
	m_curUpdateTime = timeToUpdate;
#endif
#endif
	
#ifdef DO_VTUNE_STUFF
	_updateVTune();
#endif

}  // end update

//-------------------------------------------------------------------------------------------------
/** getStats */
//-------------------------------------------------------------------------------------------------
AsciiString ScriptEngine::getStats(Real *curTimePtr, Real *script1Time, Real *script2Time)
{
	*curTimePtr = 0;
	*script1Time = 0;
	*script2Time = 0;
	AsciiString msg = "Script Engine Profiling disabled.";
#ifdef SPECIAL_SCRIPT_PROFILING
#ifdef DEBUG_LOGGING
	msg = "#1-";
	*curTimePtr = (Real)m_curUpdateTime;
	Int numToDump;
	Int i;
	if (TheSidesList) {
		for (numToDump=0; numToDump<2; numToDump++) {
			Real maxTime = 0;
			Script *maxScript = NULL;
			/* Run through scripts & set condition team names. */
			for (i=0; i<TheSidesList->getNumSides(); i++) {
				ScriptList *pSL = TheSidesList->getSideInfo(i)->getScriptList();
				if (!pSL) continue;
				if (pSL == NULL) continue;
				Script *pScr;
				for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
					if (pScr->getCurTime()>maxTime) {
						maxTime = pScr->getCurTime();
						maxScript = pScr;
					}
				}
				ScriptGroup *pGroup;
				for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
					for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
						if (pScr->getCurTime()>maxTime) {
							maxTime = pScr->getCurTime();
							maxScript = pScr;
						}
					}
				}
			}
			if (maxScript) {
				if (numToDump == 0) {
					*script1Time = maxTime;
				}	else {
					*script2Time = maxTime;
					msg.concat(", #2-");
				}
				msg.concat(maxScript->getName());
				maxScript->setCurTime(0); // reset to 0.
			}
		}
	}
#endif
#endif
	return msg;
}  // end getStats

//-------------------------------------------------------------------------------------------------
/** startQuickEndGameTimer */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::startQuickEndGameTimer( void )
{
	m_endGameTimer = 1;
}  // end startQuickEndGameTimer

//-------------------------------------------------------------------------------------------------
/** startEndGameTimer */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::startEndGameTimer( void )
{
	m_endGameTimer = FRAMES_TO_SHOW_WIN_LOSE_MESSAGE;
}  // end startEndGameTimer

//-------------------------------------------------------------------------------------------------
/** startCloseWindowTimer */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::startCloseWindowTimer( void )
{
	m_closeWindowTimer = FRAMES_TO_SHOW_WIN_LOSE_MESSAGE;
}  // end startCloseWindowTimer

//-------------------------------------------------------------------------------------------------
/** updateFades */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::updateFades( void )
{
	m_curFadeFrame++;
	Int fade = m_curFadeFrame;
	Real factor;
	if (fade<=m_fadeFramesIncrease) {
		factor = (Real)m_curFadeFrame/m_fadeFramesIncrease;
		m_curFadeValue = m_minFade + factor*(m_maxFade-m_minFade);
		return;
	}	
	fade -= m_fadeFramesIncrease;
	if (fade<=m_fadeFramesHold) {
		m_curFadeValue = m_maxFade;
		return;
	}
	fade -= m_fadeFramesHold;
	if (fade<=m_fadeFramesDecrease) {
		Int divisor = m_fadeFramesDecrease+1;
		if (divisor==0) divisor = 1;
		factor = (Real)fade/divisor;
		m_curFadeValue = m_maxFade + factor*(m_minFade-m_maxFade);
		return;
	}	
	// time is up.
	m_fade = FADE_NONE;
}  // end updateFades

//-------------------------------------------------------------------------------------------------
/** getCurrentPlayer */
//-------------------------------------------------------------------------------------------------
Player *ScriptEngine::getCurrentPlayer(void)
{
	if (m_currentPlayer==NULL)
		AppendDebugMessage("***Unexpected NULL player:***", false);
	return m_currentPlayer;
}  // end getCurrentPlayer

//-------------------------------------------------------------------------------------------------
/** clearFlag */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::clearFlag(const AsciiString &name)
{
	Int j;
	for (j=0; j<MAX_PLAYER_COUNT; j++) {
		AsciiString modName;
		modName.format("%s%d", name.str(), j);
		// Note - flags start at 1.  0 means not assigned.
		Int i;
		for (i=1; i<m_numFlags; i++) {
			if ((modName==m_flags[i].name)) {
				m_flags[i].value = FALSE;
			}
		}
	}
}  // end clearFlag

//-------------------------------------------------------------------------------------------------
/** clearTeamFlags */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::clearTeamFlags(void)
{
	clearFlag("USA Team is Building");
	clearFlag("USA Air Team Is Building");
	clearFlag("USA Inf Team Is Building");
	clearFlag("China Team is Building");
	clearFlag("China Air Team Is Building");
	clearFlag("China Inf Team Is Building");
	clearFlag("GLA Team is Building");
	clearFlag("GLA Inf Team is Building");

}  // end clearTeamFlags

//-------------------------------------------------------------------------------------------------
/** getSkirmishEnemyPlayer */
//-------------------------------------------------------------------------------------------------
Player *ScriptEngine::getSkirmishEnemyPlayer(void)
{
	if (m_currentPlayer) {
		Player *enemy = m_currentPlayer->getCurrentEnemy();
		if (enemy==NULL) {
			// get the human player.
			Int i;
			for (i=0; i<ThePlayerList->getPlayerCount(); i++) {
				enemy = ThePlayerList->getNthPlayer(i);
				if (/*enemy->isLocalPlayer() &&*/ enemy->getPlayerType()==PLAYER_HUMAN) {
					return enemy;
				}
				enemy = NULL;
			}
		}
		return enemy;
	}
	DEBUG_CRASH(("No enemy found.  Unexpected but not fatal. jba."));
	return NULL;
}
#if 0
//-------------------------------------------------------------------------------------------------
/** getSkirmishPlayerFromParm */
//-------------------------------------------------------------------------------------------------
Player *ScriptEngine::getSkirmishPlayerFromParm(Parameter *pSkirmishPlayerParm)
{
	if (!pSkirmishPlayerParm)
		return NULL;

	return getSkirmishPlayerFromAsciiString(pSkirmishPlayerParm->getString());
}

//-------------------------------------------------------------------------------------------------
/** getSkirmishPlayerFromAsciiString */
//-------------------------------------------------------------------------------------------------
Player *ScriptEngine::getSkirmishPlayerFromAsciiString(const AsciiString& skirmishPlayerString)
{
	if (skirmishPlayerString == SKIRMISH_PLAYER_AI)
		return getSkirmishAIPlayer();
	else if (skirmishPlayerString == SKIRMISH_PLAYER_HUMAN)
		return getSkirmishHumanPlayer();
	
	AppendDebugMessage("***Invalid Skirmish Player name:***", false);

	return NULL;
}
#endif

//-------------------------------------------------------------------------------------------------
/** getPlayerFromAsciiString */
//-------------------------------------------------------------------------------------------------
Player *ScriptEngine::getPlayerFromAsciiString(const AsciiString& playerString)
{
	if (playerString == LOCAL_PLAYER)
		return ThePlayerList->getLocalPlayer();
	if (playerString == THIS_PLAYER)
		return getCurrentPlayer();
	else if (playerString == THIS_PLAYER_ENEMY)	{
		return getSkirmishEnemyPlayer();
	}
	else {
		NameKeyType key = NAMEKEY(playerString);
		Player *pPlayer = ThePlayerList->findPlayerWithNameKey(key);
		if (pPlayer!=NULL) {
			return pPlayer;
		}
	}
	
	AppendDebugMessage("***Invalid Player name:***", false);

	return NULL;
}

//-------------------------------------------------------------------------------------------------
/** getObjectTypeList */
//-------------------------------------------------------------------------------------------------
ObjectTypes *ScriptEngine::getObjectTypes(const AsciiString& objectTypeList)
{
	AllObjectTypesIt it;

	for (it = m_allObjectTypeLists.begin(); it != m_allObjectTypeLists.end(); ++it) {
		if ((*it) == NULL) {
			DEBUG_CRASH(("NULL object type list was unexpected. jkmcd"));
			continue;
		}

		if ((*it)->getListName() == objectTypeList) {
			return (*it);
		}
	}

	return NULL;
}

//-------------------------------------------------------------------------------------------------
/** doObjectTypeListMaintenance */
/** If addObject is false, remove the object. If it is true, add the object. */
/** If the object removed is the last object, then the list is removed as well. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::doObjectTypeListMaintenance(const AsciiString& objectTypeList, const AsciiString& objectType, Bool addObject)
{
	ObjectTypes *currentObjectTypeVec = getObjectTypes(objectTypeList);
	
	if (!currentObjectTypeVec) {
		ObjectTypes *newVec = newInstance(ObjectTypes)(objectTypeList);
		m_allObjectTypeLists.push_back(newVec);
		currentObjectTypeVec = newVec;
	}

	if (addObject) {
		currentObjectTypeVec->addObjectType(objectType);	
	} else {
		currentObjectTypeVec->removeObjectType(objectType);
	}

	// Remove it. Its dead Jim.
	if (currentObjectTypeVec->getListSize() == 0) {
		removeObjectTypes(currentObjectTypeVec);
		
		// Semantic emphasis
		currentObjectTypeVec = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
/** Given a name, return the associated trigger area, or NULL if one doesn't exist.
Handles skirmish name qualification.  */
//-------------------------------------------------------------------------------------------------
PolygonTrigger *ScriptEngine::getQualifiedTriggerAreaByName( AsciiString name )
{
	if (name == MY_INNER_PERIMETER || name == MY_OUTER_PERIMETER) {
		if (m_currentPlayer) {
			Int ndx = m_currentPlayer->getMpStartIndex()+1;
			if (name==MY_INNER_PERIMETER) {
				name.format("%s%d", INNER_PERIMETER, ndx);
			}	else {
				name.format("%s%d", OUTER_PERIMETER, ndx);
			}
		}	else {
			return NULL;
		}
	} else if (name == ENEMY_INNER_PERIMETER || name == ENEMY_OUTER_PERIMETER) {

		Int mpNdx;
		mpNdx = -1;
		if (m_currentPlayer) {
			Player *enemy = getCurrentPlayer()->getCurrentEnemy();
			if (enemy) {
				mpNdx = enemy->getMpStartIndex()+1;
			}
		}
		if (name==ENEMY_INNER_PERIMETER) {
			name.format("%s%d", INNER_PERIMETER, mpNdx);
		}	else {
			name.format("%s%d", OUTER_PERIMETER, mpNdx);
		}
	}
	PolygonTrigger *trig = TheTerrainLogic->getTriggerAreaByName(name);
	if (trig==NULL) {
		AsciiString msg = "!!!WARNING!!! Trigger area '";
		msg.concat(name);
		msg.concat("' not found.");
		AppendDebugMessage(msg, TRUE);
	}

	return trig;
}



//-------------------------------------------------------------------------------------------------
/** getTeamNamed */
//-------------------------------------------------------------------------------------------------
Team * ScriptEngine::getTeamNamed(const AsciiString& teamName)
{
	if (teamName == THIS_TEAM) {
		if (m_callingTeam) 
			return m_callingTeam;
		return m_conditionTeam;
	}
	if (m_callingTeam && m_callingTeam->getName() == teamName) {
		return m_callingTeam;
	}
	if (m_conditionTeam && m_conditionTeam->getName() == teamName) {
		return m_conditionTeam;
	}
	TeamPrototype *theTeamProto = TheTeamFactory->findTeamPrototype( teamName );
	if (theTeamProto == NULL) return NULL;
	if (theTeamProto->getIsSingleton()) {
		Team *theTeam = theTeamProto->getFirstItemIn_TeamInstanceList();
		if (theTeam && theTeam->isActive()) {
			return theTeam;
		}
		return NULL; // team wasn't active.
	}
	
	static int warnCount = 0;
	if (theTeamProto->countTeamInstances()>1) {
		if (warnCount<10) {
			warnCount++;
			AppendDebugMessage("***Referencing multiple team by unspecific instance:***", false);
			AppendDebugMessage(teamName, false);
		}
	}
	return theTeamProto->getFirstItemIn_TeamInstanceList();
}  // end getTeamNamed

//-------------------------------------------------------------------------------------------------
/** getUnitNamed */
//-------------------------------------------------------------------------------------------------
Object * ScriptEngine::getUnitNamed(const AsciiString& unitName)
{
	if (unitName == THIS_OBJECT) {
		if (m_callingObject) {
			return m_callingObject;
		}
		return m_conditionObject;
	}

	for (VecNamedRequestsIt it = m_namedObjects.begin(); it != m_namedObjects.end(); ++it) {
		if (unitName == (it->first)) {
			return it->second;
		}
	}
	return NULL;
}

//-------------------------------------------------------------------------------------------------
/** didUnitExist */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::didUnitExist(const AsciiString& unitName)
{
	for (VecNamedRequestsIt it = m_namedObjects.begin(); it != m_namedObjects.end(); ++it) {
		if (unitName == (it->first)) {
			return (it->second == NULL);
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** runScript - Executes a subroutine script, or script group - tests conditions, and executes actions or false actions.  */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::runScript(const AsciiString& scriptName, Team *pThisTeam)
{
	if (scriptName.isEmpty()) {
		return; // no script, just return.
	}
	if (scriptName==NONE_STRING) {
		return; // no script
	}
	

	Player *savPlayer = m_currentPlayer;
//	Team *pSavConditionTeam = m_conditionTeam;
	LatchRestore<Team *> latch(m_callingTeam, pThisTeam);

	m_conditionTeam = NULL;
	m_currentPlayer = NULL;
	if (m_callingTeam) {
		m_currentPlayer = m_callingTeam->getControllingPlayer();
	}
	Script  *pScript = NULL;
	ScriptGroup *pGroup = findGroup(scriptName);
	if (pGroup) {
		if (pGroup->isSubroutine()) {
			if (pGroup->isActive()) {
				executeScripts(pGroup->getScript());
			}
		}	else {
				AppendDebugMessage("***Attempting to call script that is not a subroutine:***", false);
				AppendDebugMessage(scriptName, false);
				DEBUG_LOG(("Attempting to call script '%s' that is not a subroutine.\n", scriptName.str()));
		}
	}	else {
		pScript = findScript(scriptName);
		if (pScript != NULL) {
			if (pScript->isSubroutine()) {
				executeScript(pScript);
			} else {
				AppendDebugMessage("***Attempting to call script that is not a subroutine:***", false);
				AppendDebugMessage(scriptName, false);
				DEBUG_LOG(("Attempting to call script '%s' that is not a subroutine.\n", scriptName.str()));
			}
		} else {
			AppendDebugMessage("***Script not defined:***", false);
			AppendDebugMessage(scriptName, false);
			DEBUG_LOG(("WARNING: Script '%s' not defined.\n", scriptName.str()));
		}
	}
	// m_callingTeam is restored automatically via LatchRestore
	m_conditionTeam = m_conditionTeam;
	m_currentPlayer = savPlayer;
}  // end runScript


//-------------------------------------------------------------------------------------------------
/** runScript - Executes a subroutine script, or script group - tests conditions, and executes actions or false actions.  */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::runObjectScript(const AsciiString& scriptName, Object *pThisObject)
{
	if (scriptName.isEmpty()) {
		return; // no script, just return.
	}
	if (scriptName==NONE_STRING) {
		return; // no script
	}
	Object *pSavCallingObject = m_callingObject;
	m_callingObject = pThisObject;
	Script  *pScript = NULL;
	ScriptGroup *pGroup = findGroup(scriptName);
	if (pGroup) {
		if (pGroup->isSubroutine()) {
			if (pGroup->isActive()) {
				executeScripts(pGroup->getScript());
			}
		}	else {
				AppendDebugMessage("***Attempting to call script that is not a subroutine:***", false);
				AppendDebugMessage(scriptName, false);
				DEBUG_LOG(("Attempting to call script '%s' that is not a subroutine.\n", scriptName.str()));
		}
	}	else {
		pScript = findScript(scriptName);
		if (pScript != NULL) {
			if (pScript->isSubroutine()) {
				executeScript(pScript);
			} else {
				AppendDebugMessage("***Attempting to call script that is not a subroutine:***", false);
				AppendDebugMessage(scriptName, false);
				DEBUG_LOG(("Attempting to call script '%s' that is not a subroutine.\n", scriptName.str()));
			}
		} else {
			AppendDebugMessage("***Script not defined:***", false);
			AppendDebugMessage(scriptName, false);
			DEBUG_LOG(("WARNING: Script '%s' not defined.\n", scriptName.str()));
		}
	}
	m_callingObject = pSavCallingObject;
}  // end runScript


//-------------------------------------------------------------------------------------------------
/** Allocates a counter, if this name doesn't exist. */
//-------------------------------------------------------------------------------------------------
Int ScriptEngine::allocateCounter( const AsciiString& name)
{
	Int i;
	// Note - counters start at 1.  0 means not assigned.
	for (i=1; i<m_numCounters; i++) {
		if (name==m_counters[i].name) {
			return i;
		}
	}
	DEBUG_ASSERTCRASH(m_numCounters<MAX_COUNTERS, ("Too many counters, failed to make '%s'.\n", name.str()));
	if (m_numCounters < MAX_COUNTERS) {
		m_counters[m_numCounters].name = name;
		i = m_numCounters;
		m_numCounters++;
		return(i);
	}
	return 0; // Shouldn't ever happen.
}

//-------------------------------------------------------------------------------------------------
/** Gets a counter */
//-------------------------------------------------------------------------------------------------
const TCounter *ScriptEngine::getCounter(const AsciiString& counterName)
{
	Int i;
	for (i=1; i<m_numCounters; i++)
	{
		if (counterName == m_counters[i].name)
		{
			return &(m_counters[i]);
		}
	}
	return NULL;
}

//-------------------------------------------------------------------------------------------------
void ScriptEngine::createNamedMapReveal(const AsciiString& revealName, const AsciiString& waypointName, Real radiusToReveal, const AsciiString& playerName)
{
	VecNamedRevealIt it;

	// Will fail if there's already one in existence of the same name.
	for (it = m_namedReveals.begin(); it != m_namedReveals.end(); ++it) {
		if (it->m_revealName == revealName) {
			DEBUG_CRASH(("ScriptEngine::createNamedMapReveal: Attempted to redefine named Reveal '%s', so I won't change it.\n", revealName.str()));
			return;
		}
	}

	NamedReveal reveal;
	reveal.m_playerName = playerName;
	reveal.m_radiusToReveal = radiusToReveal;
	reveal.m_revealName = revealName;
	reveal.m_waypointName = waypointName;

	m_namedReveals.push_back(reveal);
}
//-------------------------------------------------------------------------------------------------
void ScriptEngine::doNamedMapReveal(const AsciiString& revealName)
{
	VecNamedRevealIt it;

	NamedReveal *reveal = NULL;
	for (it = m_namedReveals.begin(); it != m_namedReveals.end(); ++it) {
		if (it->m_revealName == revealName) {
			reveal = &(*it);
			break;
		}
	}

	if (!reveal) {
		return;
	}

	Waypoint *way = TheTerrainLogic->getWaypointByName(reveal->m_waypointName);
	if (!way) {
		return;
	}

	Player *player = getPlayerFromAsciiString(reveal->m_playerName);
	if (!player) {
		return;
	}

	Coord3D pos;
	pos = *way->getLocation();

	ThePartitionManager->doShroudReveal(pos.x, pos.y, reveal->m_radiusToReveal, player->getPlayerMask());
}

//-------------------------------------------------------------------------------------------------
void ScriptEngine::undoNamedMapReveal(const AsciiString& revealName)
{
	VecNamedRevealIt it;

	NamedReveal *reveal = NULL;
	for (it = m_namedReveals.begin(); it != m_namedReveals.end(); ++it) {
		if (it->m_revealName == revealName) {
			reveal = &(*it);
			break;
		}
	}

	if (!reveal) {
		return;
	}

	Waypoint *way = TheTerrainLogic->getWaypointByName(reveal->m_waypointName);
	if (!way) {
		return;
	}

	Player *player = getPlayerFromAsciiString(reveal->m_playerName);
	if (!player) {
		return;
	}

	Coord3D pos;
	pos = *way->getLocation();

	ThePartitionManager->undoShroudReveal(pos.x, pos.y, reveal->m_radiusToReveal, player->getPlayerMask());
}

//-------------------------------------------------------------------------------------------------
void ScriptEngine::removeNamedMapReveal(const AsciiString& revealName)
{
	VecNamedRevealIt it;
	
	for (it = m_namedReveals.begin(); it != m_namedReveals.end(); ++it) {
		if (it->m_revealName == revealName) {
			m_namedReveals.erase(it);
			return;
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Allocates a flag, if this name doesn't exist. */
//-------------------------------------------------------------------------------------------------
Int ScriptEngine::allocateFlag( const AsciiString& name)
{
	Int i;
	// Note - flags start at 1.  0 means not assigned.
	for (i=1; i<m_numFlags; i++) {
		if ((name==m_flags[i].name)) {
			return i;
		}
	}
	DEBUG_ASSERTCRASH(m_numFlags < MAX_FLAGS, ("Too many flags, failed to make '%s'..\n", name.str()));
	if (m_numFlags < MAX_FLAGS) {
		m_flags[m_numFlags].name = name;
		i = m_numFlags;
		m_numFlags++;
		return(i);
	}
	return 0; // Shouldn't ever happen.
}

//-------------------------------------------------------------------------------------------------
/** Locates a group by name. */
//-------------------------------------------------------------------------------------------------
ScriptGroup  *ScriptEngine::findGroup(const AsciiString& name)
{
	Int i;
	for (i=0; i<TheSidesList->getNumSides(); i++) {
		ScriptList *pSL = TheSidesList->getSideInfo(i)->getScriptList();
		if (pSL==NULL) continue;
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			if (pGroup->getName() == name) {
				return pGroup;
			}
		}
	}
	return 0; // Shouldn't ever happen.
}

//-------------------------------------------------------------------------------------------------
/** Locates a script by name. */
//-------------------------------------------------------------------------------------------------
Script  *ScriptEngine::findScript(const AsciiString& name)
{
	Int i;
	for (i=0; i<TheSidesList->getNumSides(); i++) {
		ScriptList *pSL = TheSidesList->getSideInfo(i)->getScriptList();
		if (pSL==NULL) continue;
		Script *pScr;
		for (pScr = pSL->getScript(); pScr; pScr=pScr->getNext()) {
			if ((name==pScr->getName())) {
				return pScr;
			}
		}
		ScriptGroup *pGroup;
		for (pGroup = pSL->getScriptGroup(); pGroup; pGroup=pGroup->getNext()) {
			for (pScr = pGroup->getScript(); pScr; pScr=pScr->getNext()) {
				if ((name==pScr->getName())) {
					return pScr;
				}
			}
		}
	}
	return 0; // Shouldn't ever happen.
}

//-------------------------------------------------------------------------------------------------
/** Evaluates a counter condition */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::evaluateCounter( Condition *pCondition )
{
	DEBUG_ASSERTCRASH(pCondition->getNumParameters() >= 3, ("Not enough parameters.\n"));
	DEBUG_ASSERTCRASH(pCondition->getConditionType() == Condition::COUNTER, ("Wrong condition.\n"));
	Int counterNdx = pCondition->getParameter(0)->getInt();
	if (counterNdx == 0) {
		counterNdx = allocateCounter(pCondition->getParameter(0)->getString());
		pCondition->getParameter(0)->friend_setInt(counterNdx);
	}
	Int value = pCondition->getParameter(2)->getInt();
	switch (pCondition->getParameter(1)->getInt()) {
		case Parameter::LESS_THAN: return m_counters[counterNdx].value < value;
		case Parameter::LESS_EQUAL: return m_counters[counterNdx].value <= value;
		case Parameter::EQUAL: return m_counters[counterNdx].value == value;
		case Parameter::GREATER_EQUAL: return m_counters[counterNdx].value >= value;
		case Parameter::GREATER: return m_counters[counterNdx].value > value;
		case Parameter::NOT_EQUAL: return m_counters[counterNdx].value != value;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** Sets a counter. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::setCounter( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 2, ("Not enough parameters.\n"));
	Int counterNdx = pAction->getParameter(0)->getInt();
	if (counterNdx == 0) {
		counterNdx = allocateCounter(pAction->getParameter(0)->getString());
		pAction->getParameter(0)->friend_setInt(counterNdx);
	}
	Int value = pAction->getParameter(1)->getInt();
	m_counters[counterNdx].value = value;
}

//-------------------------------------------------------------------------------------------------
/** Sets a fade. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::setFade( ScriptAction *pAction )
{
#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	if (TheGlobalData->m_disableCameraFade)
	{
		m_fade = FADE_NONE;
		return;
	}
#endif

	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 5, ("Not enough parameters.\n"));
	switch (pAction->getActionType()) {
		default:	m_fade = FADE_NONE; return;
		case ScriptAction::CAMERA_FADE_ADD: m_fade = FADE_ADD; break;
		case ScriptAction::CAMERA_FADE_SUBTRACT: m_fade = FADE_SUBTRACT; break;
		case ScriptAction::CAMERA_FADE_SATURATE: m_fade = FADE_SATURATE; break;
		case ScriptAction::CAMERA_FADE_MULTIPLY: m_fade = FADE_MULTIPLY; break;
	}
	m_curFadeFrame = 0;
	m_minFade = pAction->getParameter(0)->getReal();
	m_maxFade = pAction->getParameter(1)->getReal();
	m_fadeFramesIncrease = pAction->getParameter(2)->getInt();
	m_fadeFramesHold = pAction->getParameter(3)->getInt();
	m_fadeFramesDecrease = pAction->getParameter(4)->getInt();
	m_curFadeValue = m_minFade;
	if( m_fadeFramesIncrease == 0 )
	{
		updateFades();
	}
}

//-------------------------------------------------------------------------------------------------
/** Sets a counter. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::setSway( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 5, ("Not enough parameters.\n"));
	++m_breezeInfo.m_breezeVersion;
	m_breezeInfo.m_direction = pAction->getParameter(0)->getReal();
	m_breezeInfo.m_directionVec.x = Sin(m_breezeInfo.m_direction);
	m_breezeInfo.m_directionVec.y = Cos(m_breezeInfo.m_direction);
	m_breezeInfo.m_intensity = pAction->getParameter(1)->getReal();
	m_breezeInfo.m_lean = pAction->getParameter(2)->getReal();
	m_breezeInfo.m_breezePeriod = pAction->getParameter(3)->getInt();
	if (m_breezeInfo.m_breezePeriod<1) 
		m_breezeInfo.m_breezePeriod = 1;
	m_breezeInfo.m_randomness = pAction->getParameter(4)->getReal();

}

//-------------------------------------------------------------------------------------------------
/** Adds to a counter. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::addCounter( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 2, ("Not enough parameters.\n"));
	Int value = pAction->getParameter(0)->getInt();
	Int counterNdx = pAction->getParameter(1)->getInt();
	if (counterNdx == 0) {
		counterNdx = allocateCounter(pAction->getParameter(1)->getString());
		pAction->getParameter(1)->friend_setInt(counterNdx);
	}
	m_counters[counterNdx].value += value;
}

//-------------------------------------------------------------------------------------------------
/** Subtracts from a counter. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::subCounter( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 2, ("Not enough parameters.\n"));
	Int value = pAction->getParameter(0)->getInt();
	Int counterNdx = pAction->getParameter(1)->getInt();
	if (counterNdx == 0) {
		counterNdx = allocateCounter(pAction->getParameter(1)->getString());
		pAction->getParameter(1)->friend_setInt(counterNdx);
	}
	m_counters[counterNdx].value -= value;
}

//-------------------------------------------------------------------------------------------------
/** Evaluates a flag */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::evaluateFlag( Condition *pCondition )
{
	DEBUG_ASSERTCRASH(pCondition->getNumParameters() >= 2, ("Not enough parameters.\n"));
	DEBUG_ASSERTCRASH(pCondition->getConditionType() == Condition::FLAG, ("Wrong condition.\n"));
	Int flagNdx = pCondition->getParameter(0)->getInt();
	if (flagNdx == 0) {
		flagNdx = allocateFlag(pCondition->getParameter(0)->getString());
		pCondition->getParameter(0)->friend_setInt(flagNdx);
	}
	Int value = pCondition->getParameter(1)->getInt();
	Bool boolVal = (value!=0);
	Bool boolFlag = (m_flags[flagNdx].value != 0);
	
	if (boolVal == boolFlag) {
		return true;
	}

	for (ListAsciiStringIt it = m_uiInteractions.begin(); it != m_uiInteractions.end(); ++it) {
		if (it->compare(pCondition->getParameter(0)->getString()) == 0) {
			// just return. This flag will be cleared up at the end of the ScriptEngine::update() call
			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** Sets a flag */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::setFlag( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 2, ("Not enough parameters.\n"));
	Int flagNdx = pAction->getParameter(0)->getInt();
	if (flagNdx == 0) {
		flagNdx = allocateFlag(pAction->getParameter(0)->getString());
		pAction->getParameter(0)->friend_setInt(flagNdx);
	}
	Bool value = pAction->getParameter(1)->getInt();
	m_flags[flagNdx].value = value;
}



//-------------------------------------------------------------------------------------------------
/** Finds a named attack info.  Note - may return null. */
//-------------------------------------------------------------------------------------------------
AttackPriorityInfo * ScriptEngine::findAttackInfo(const AsciiString& name, Bool addIfNotFound)
{
	// Note - m_attackPriorityInfo[0] is the default info, with an empty name.
	Int i;
	for (i=1; i<m_numAttackInfo; i++) {
		if (m_attackPriorityInfo[i].getName() == name) {
			return &m_attackPriorityInfo[i];
		}
	}
	if (addIfNotFound && m_numAttackInfo<MAX_ATTACK_PRIORITIES) {
		m_attackPriorityInfo[m_numAttackInfo].friend_setName(name);
		m_numAttackInfo++;
		return &m_attackPriorityInfo[m_numAttackInfo-1];
	}
	return NULL;
}

/// Attack priority stuff.
//-------------------------------------------------------------------------------------------------
/** Returns the default attack priority info.  Never returns null. */
//-------------------------------------------------------------------------------------------------
const AttackPriorityInfo *ScriptEngine::getDefaultAttackInfo(void)
{
	// Note - m_attackPriorityInfo[0] is the default info, with an empty name.
	return &m_attackPriorityInfo[0];
}

//-------------------------------------------------------------------------------------------------
/** Returns the named attack info, if non-existent returns default attack priority info.  
		Never returns null. */
//-------------------------------------------------------------------------------------------------
const AttackPriorityInfo *ScriptEngine::getAttackInfo(const AsciiString& name)
{
	Int i;
	for (i=1; i<m_numAttackInfo; i++) {
		if (m_attackPriorityInfo[i].getName() == name) {
			return &m_attackPriorityInfo[i];
		}
	}
	// Note - m_attackPriorityInfo[0] is the default info, with an empty name.
	return &m_attackPriorityInfo[0];
}

//-------------------------------------------------------------------------------------------------
/** Sets an Attack Priority Set value for a thing */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::setPriorityThing( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 3, ("Not enough parameters.\n"));
	
	AsciiString typeArgument = pAction->getParameter(1)->getString();

	// Our argument could be an individual type, or a list name.
	const ObjectTypes *types = TheScriptEngine->getObjectTypes(typeArgument);
	if( !types ) 
	{
		// Lookup failed, so it is just a single type

		const ThingTemplate *thingTemplate;
		// get thing template based from map object name
		thingTemplate = TheThingFactory->findTemplate(typeArgument);
		if (thingTemplate==NULL) {
			AppendDebugMessage("***Attempting to set attack priority on an invalid thing:***", false);
			AppendDebugMessage(pAction->getParameter(0)->getString(), false);
			return;
		}
		AttackPriorityInfo *info = findAttackInfo(pAction->getParameter(0)->getString(), true);
		if (info==NULL) {
			AppendDebugMessage("***Error allocating attack priority set - fix or raise limit. ***", false);
			return;
		}
		// DEBUG LOGGING
//		AsciiString debug = "Set the priority on ";
//		debug.concat(thingTemplate->getName().str());
//		debug.concat(" to ");
//		char intBuffer[16];
//		itoa(pAction->getParameter(2)->getInt(), intBuffer, 10);
//		debug.concat(intBuffer);
//		AppendDebugMessage(debug, false);
		// END DEBUGGING LOGGING
		info->setPriority(thingTemplate, pAction->getParameter(2)->getInt());

		return;
	}
	else 
	{
		// Found a list by this name, so we have a bunch of things

		for( Int typeIndex = 0; typeIndex < types->getListSize(); typeIndex ++ )
		{
			AsciiString thisTypeName = types->getNthInList(typeIndex);
			const ThingTemplate *thisType = TheThingFactory->findTemplate(thisTypeName);
			if (thisType==NULL) {
				AppendDebugMessage("***Attempting to set attack priority on an invalid thing:***", false);
				AppendDebugMessage(pAction->getParameter(0)->getString(), false);
				return;
			}
			AttackPriorityInfo *info = findAttackInfo(pAction->getParameter(0)->getString(), true);
			if (info==NULL) {
				AppendDebugMessage("***Error allocating attack priority set - fix or raise limit. ***", false);
				return;
			}
			// DEBUG LOGGING
//			AsciiString debug = "Set the priority on ";
//			debug.concat(thisType->getName().str());
//			debug.concat(" to ");
//			char intBuffer[16];
//			itoa(pAction->getParameter(2)->getInt(), intBuffer, 10);
//			debug.concat(intBuffer);
//			AppendDebugMessage(debug, false);
			// END DEBUGGING LOGGING
			info->setPriority(thisType, pAction->getParameter(2)->getInt());
		}

		return;
	}

}

//-------------------------------------------------------------------------------------------------
/** Sets an Attack Priority Set value for all things of a particular kind. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::setPriorityKind( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 3, ("Not enough parameters.\n"));
	AttackPriorityInfo *info = findAttackInfo(pAction->getParameter(0)->getString(), true);
	if (info==NULL) {
		AppendDebugMessage("***Error allocating attack priority set - fix or raise limit. ***", false);
		return;
	}
	KindOfType kind = (KindOfType)pAction->getParameter(1)->getInt();
	Int priority = pAction->getParameter(2)->getInt();
	const ThingTemplate *tTemplate;
	for( tTemplate = TheThingFactory->firstTemplate();
			 tTemplate;
			 tTemplate = tTemplate->friend_getNextTemplate() )
	{
				 if (tTemplate->isKindOf(kind)) {
					 info->setPriority(tTemplate, priority);
				 }
	}
}

//-------------------------------------------------------------------------------------------------
/** Sets an Attack Priority Set default value. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::setPriorityDefault( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 2, ("Not enough parameters.\n"));
	AttackPriorityInfo *info = findAttackInfo(pAction->getParameter(0)->getString(), true);
	if (info==NULL) {
		AppendDebugMessage("***Error allocating attack priority set - fix or raise limit. ***", false);
		return;
	}
	info->friend_setDefaultPriority(pAction->getParameter(1)->getInt());
}

//-------------------------------------------------------------------------------------------------
Int ScriptEngine::getObjectCount(Int playerIndex, const AsciiString& objectTypeName) const
{
	if (!ThePlayerList->getNthPlayer(playerIndex)->isPlayerActive()) {
		// Inactive players should always pretend they have 0 objects.
		return 0;
	}

	const ObjectTypeCount &ocm = m_objectCounts[playerIndex];

	ObjectTypeCount::const_iterator it = ocm.find(objectTypeName);
	if (it == ocm.end()) {
		return 0;
	}

	return it->second;
}

//-------------------------------------------------------------------------------------------------
void ScriptEngine::setObjectCount(Int playerIndex, const AsciiString& objectTypeName, Int newCount)
{
	// Don't really need inactive player checks here.
	
	ObjectTypeCount &ocm = m_objectCounts[playerIndex];
	ocm[objectTypeName] = newCount;
}

//-------------------------------------------------------------------------------------------------
/** Removes an object types list from the list owned by the script engine, and then deletes the */
/**	associated item. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::removeObjectTypes(ObjectTypes *typesToRemove)
{
	if (typesToRemove == NULL) {
		return;
	}

	AllObjectTypesIt it = std::find(m_allObjectTypeLists.begin(), m_allObjectTypeLists.end(), typesToRemove);

	if (it == m_allObjectTypeLists.end()) {
		// Debug crash maybe?
		return;
	}

	// delete it.
	typesToRemove->deleteInstance();

	// remove it from the main array of stuff
	m_allObjectTypeLists.erase(it);
}

//-------------------------------------------------------------------------------------------------
/** Evaluates a timer */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::evaluateTimer( Condition *pCondition )
{
	DEBUG_ASSERTCRASH(pCondition->getNumParameters() >= 1, ("Not enough parameters.\n"));
	DEBUG_ASSERTCRASH(pCondition->getConditionType() == Condition::TIMER_EXPIRED, ("Wrong condition.\n"));
	Int counterNdx = pCondition->getParameter(0)->getInt();
	if (counterNdx == 0) {
		counterNdx = allocateCounter(pCondition->getParameter(0)->getString());
		pCondition->getParameter(0)->friend_setInt(counterNdx);
	}
	if (!m_counters[counterNdx].isCountdownTimer) {
		return false; // Timer hasn't been started yet.
	}
	Int value = m_counters[counterNdx].value;
	// Timers decrement down to -1.
	return (value < 1);
}

//-------------------------------------------------------------------------------------------------
/** Starts a timer. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::setTimer( ScriptAction *pAction, Bool millisecondTimer, Bool random )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 2, ("Not enough parameters.\n"));
	Int counterNdx = pAction->getParameter(0)->getInt();
	if (counterNdx == 0) {
		counterNdx = allocateCounter(pAction->getParameter(0)->getString());
		pAction->getParameter(0)->friend_setInt(counterNdx);
	}
	if (millisecondTimer) {
		Real value = pAction->getParameter(1)->getReal();
		if (random) {
			Real randomValue = pAction->getParameter(2)->getReal();
			value = GameLogicRandomValue(value, randomValue);
		}
		m_counters[counterNdx].value = REAL_TO_INT_CEIL(ConvertDurationFromMsecsToFrames(value*1000));
	} else {
		Int value = pAction->getParameter(1)->getInt();
		if (random) {
			Int randomValue = pAction->getParameter(2)->getInt();
			value = GameLogicRandomValue(value, randomValue);
		}
		m_counters[counterNdx].value = value;
	}
	m_counters[counterNdx].isCountdownTimer = true;
}

//-------------------------------------------------------------------------------------------------
/** Stops a timer. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::pauseTimer( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 1, ("Not enough parameters.\n"));
	Int counterNdx = pAction->getParameter(0)->getInt();
	if (counterNdx == 0) {
		counterNdx = allocateCounter(pAction->getParameter(0)->getString());
		pAction->getParameter(0)->friend_setInt(counterNdx);
	}
	m_counters[counterNdx].isCountdownTimer = false;
}

//-------------------------------------------------------------------------------------------------
/** Restarts a timer. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::restartTimer( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 1, ("Not enough parameters.\n"));
	Int counterNdx = pAction->getParameter(0)->getInt();
	if (counterNdx == 0) {
		counterNdx = allocateCounter(pAction->getParameter(0)->getString());
		pAction->getParameter(0)->friend_setInt(counterNdx);
	}
	if (m_counters[counterNdx].value > 0) {
		m_counters[counterNdx].isCountdownTimer = true;
	}
}

//-------------------------------------------------------------------------------------------------
/** adjusts a timer. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::adjustTimer( ScriptAction *pAction, Bool millisecondTimer, Bool add)
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 2, ("Not enough parameters.\n"));
	Int counterNdx = pAction->getParameter(1)->getInt();
	if (counterNdx == 0) {
		counterNdx = allocateCounter(pAction->getParameter(1)->getString());
		pAction->getParameter(1)->friend_setInt(counterNdx);
	}
	if (millisecondTimer) {
		Real value = pAction->getParameter(0)->getReal();
		if (!add)
			value = -value;
		m_counters[counterNdx].value += REAL_TO_INT_CEIL(ConvertDurationFromMsecsToFrames(value*1000));
	} else {
		Int value = pAction->getParameter(0)->getInt();
		if (!add)
			value = -value;
		m_counters[counterNdx].value += value;
	}
}

//-------------------------------------------------------------------------------------------------
/** Enables a script or group. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::enableScript( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 1, ("Not enough parameters.\n"));
	ScriptGroup *pGroup = findGroup(pAction->getParameter(0)->getString());
	if (pGroup) {
		pGroup->setActive(true);
	}
	Script *pScript = findScript(pAction->getParameter(0)->getString());
	if (pScript) {
		pScript->setActive(true);
	}
}

//-------------------------------------------------------------------------------------------------
/** Enables a script or group. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::disableScript( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 1, ("Not enough parameters.\n"));
	Script *pScript = findScript(pAction->getParameter(0)->getString());
	if (pScript) {
		pScript->setActive(false);
	}
	ScriptGroup *pGroup = findGroup(pAction->getParameter(0)->getString());
	if (pGroup) {
		pGroup->setActive(false);
	}
}

//-------------------------------------------------------------------------------------------------
/** Executes a script subroutine. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::callSubroutine( ScriptAction *pAction )
{
	DEBUG_ASSERTCRASH(pAction->getNumParameters() >= 1, ("Not enough parameters.\n"));
	AsciiString scriptName = pAction->getParameter(0)->getString();
	Script  *pScript;
	ScriptGroup *pGroup = findGroup(scriptName);
	if (pGroup) {
		if (pGroup->isSubroutine()) {
			if (pGroup->isActive()) {
				executeScripts(pGroup->getScript());
			}
		}	else {
				AppendDebugMessage("***Attempting to call script that is not a subroutine:***", false);
				AppendDebugMessage(scriptName, false);
				DEBUG_LOG(("Attempting to call script '%s' that is not a subroutine.\n", scriptName.str()));
		}
	}	else {
		pScript = findScript(scriptName);
		if (pScript != NULL) {
			if (pScript->isSubroutine()) {
				executeScript(pScript);
			} else {
				AppendDebugMessage("***Attempting to call script that is not a subroutine:***", false);
				AppendDebugMessage(scriptName, false);
				DEBUG_LOG(("Attempting to call script '%s' that is not a subroutine.\n", scriptName.str()));
			}
		} else {
			AppendDebugMessage("***Script not defined:***", false);
			AppendDebugMessage(scriptName, false);
			DEBUG_LOG(("WARNING: Script '%s' not defined.\n", scriptName.str()));
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Checks to see if any teams are referenced in the conditions, so we can properly
iterate over multiple teams. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::checkConditionsForTeamNames(Script *pScript)
{
	AsciiString singletonTeamName;
	AsciiString multiTeamName;

	if (pScript->getDelayEvalSeconds()>0) {
		// Offset by a random number of frames
		pScript->setFrameToEvaluate(GameLogicRandomValue(0,2*LOGICFRAMES_PER_SECOND));
	} else {
		pScript->setFrameToEvaluate(0);
	}

	AsciiString scriptName = pScript->getName();
	OrCondition *pOr;
	for (pOr = pScript->getOrCondition(); pOr; pOr = pOr->getNextOrCondition()) {
		Condition *pCondition;
		for (pCondition = pOr->getFirstAndCondition(); pCondition; pCondition = pCondition->getNext()) {
			Int i;
			for (i=0; i<pCondition->getNumParameters(); i++) {
				if (Parameter::TEAM == pCondition->getParameter(i)->getParameterType()) {
					AsciiString teamName = pCondition->getParameter(i)->getString();
					TeamPrototype *proto = TheTeamFactory->findTeamPrototype(teamName);
					if (proto==NULL) continue; // Undefined team - don't bother.
					Bool singleton = proto->getIsSingleton();
					if (proto->getTemplateInfo()->m_maxInstances < 2) {
						singleton = true;
					}
					if (singleton) {
						singletonTeamName = teamName;		// Singleton team - use if it is the only one, but can have multiple of these.
					} else {
						if (multiTeamName.isEmpty()) {
							multiTeamName = teamName;		// Use one multiply defined team.  Good.
						} else if (multiTeamName!=teamName) {
							// More than one multiply defined team - bad.
							AppendDebugMessage("***WARNING: Script contains multiple non-singleton team conditions::***", false);
							AppendDebugMessage(scriptName, false);
							AppendDebugMessage(multiTeamName, false);
							AppendDebugMessage(teamName, false);
							DEBUG_LOG(("WARNING: Script '%s' contains multiple non-singleton team conditions: %s & %s.\n", scriptName.str(), 
								multiTeamName.str(), teamName.str()));
						}
					}
				}
			}
		}
	}
	if (multiTeamName.isEmpty()) {
		if (!singletonTeamName.isEmpty()) {
			pScript->setConditionTeamName(singletonTeamName);
		}
	} else {
		pScript->setConditionTeamName(multiTeamName);
	}

}

//-------------------------------------------------------------------------------------------------
/** Executes a script. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::executeScript( Script *pScript )
{

	pScript->setCurTime(0);
	// If script is not active, return.
	if (!pScript->isActive()) {
		return;
	}
	enum GameDifficulty difficulty = getGlobalDifficulty();
	if (m_currentPlayer) {
		difficulty = m_currentPlayer->getPlayerDifficulty();
	}
	// If script doesn't match difficulty level, return.
	switch (difficulty) {
		case DIFFICULTY_EASY : if (!pScript->isEasy()) return;  break;
		case DIFFICULTY_NORMAL : if (!pScript->isNormal()) return;  break;
		case DIFFICULTY_HARD : if (!pScript->isHard()) return;  break;
	}
	// If we are doing peridic evaluation, check the frame.
	if (TheGameLogic->getFrame()<pScript->getFrameToEvaluate()) {
		return;
	}
	Int delaySeconds = pScript->getDelayEvalSeconds();

	if (delaySeconds>0) {
		pScript->setFrameToEvaluate(TheGameLogic->getFrame()+delaySeconds*LOGICFRAMES_PER_SECOND);
	}
#ifdef DEBUG_LOGGING
#ifdef SPECIAL_SCRIPT_PROFILING
	__int64 startTime64;
	Real timeToEvaluate=0.0f;
	__int64 endTime64,freq64;
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq64);
	QueryPerformanceCounter((LARGE_INTEGER *)&startTime64);
#endif
#endif

	Team *pSavConditionTeam = m_conditionTeam;
	TeamPrototype *pProto = NULL;

	if (!pScript->getConditionTeamName().isEmpty()) {
		pProto = TheTeamFactory->findTeamPrototype(pScript->getConditionTeamName());
	}

	if (pProto && pProto->countTeamInstances() > 0) {
		// We have a team referred to in the conditions.  Iterate over the instances of the team, 
		// applying the script conditions (and possibly actions) to each instance of the team.
		for (DLINK_ITERATOR<Team> iter = pProto->iterate_TeamInstanceList(); !iter.done(); iter.advance()) {
			m_conditionTeam = iter.cur();
			// If conditions evaluate to true, execute actions.
			if (evaluateConditions(pScript)) {
				// Script Debug window
				if (pScript->getAction()) {
					_appendMessage(pScript->getName());
					executeActions(pScript->getAction());
				}
				
				if (pScript->isOneShot()) {
					pScript->setActive(false);
				}
			}	else if (pScript->getFalseAction()) {

				// Script Debug window
				_appendMessage(pScript->getName(), false);

				// Only do this is there are actually false actions.
				executeActions(pScript->getFalseAction());
			}
		}

	} else {
		m_conditionTeam = NULL;
		// If conditions evaluate to true, execute actions.
		if (evaluateConditions(pScript)) {
			if (pScript->getAction()) {
				// Script Debug window
				_appendMessage(pScript->getName());
				executeActions(pScript->getAction());
			}

			if (pScript->isOneShot()) {
				pScript->setActive(false);
			}
		}	else if (pScript->getFalseAction()) {

			// Script Debug window
			_appendMessage(pScript->getName(), false);

			// Only do this is there are actually false actions.
			executeActions(pScript->getFalseAction());
			if (pScript->isOneShot()) {
				pScript->setActive(false);
			}
		}
	}
#ifdef DEBUG_LOGGING
#ifdef SPECIAL_SCRIPT_PROFILING
	QueryPerformanceCounter((LARGE_INTEGER *)&endTime64);
	timeToEvaluate = ((Real)(endTime64-startTime64) / (Real)(freq64));
	pScript->setCurTime(timeToEvaluate);
#endif
#endif

	m_conditionTeam = pSavConditionTeam;
}

//-------------------------------------------------------------------------------------------------
/** Evaluates a condition */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::evaluateCondition( Condition *pCondition )
{
	switch (pCondition->getConditionType()) {
		default: 
			return TheScriptConditions->evaluateCondition(pCondition);
		case Condition::CONDITION_FALSE: return false;
		case Condition::CONDITION_TRUE: return true;
		case Condition::COUNTER: return evaluateCounter(pCondition);
		case Condition::FLAG: return evaluateFlag(pCondition);
		case Condition::TIMER_EXPIRED: return evaluateTimer(pCondition);
	}
}

//-------------------------------------------------------------------------------------------------
/** Execute an action specified by pActionHead */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::friend_executeAction( ScriptAction *pActionHead, Team *pThisTeam )
{
	Team *pSavCallingTeam = m_callingTeam;
	Player *pSavPlayer = m_currentPlayer;
	m_callingTeam = pThisTeam;
	m_currentPlayer = NULL;
	if (pThisTeam) {
		m_currentPlayer = pThisTeam->getControllingPlayer();
	}
	executeActions(pActionHead);
	m_callingTeam = pSavCallingTeam;
	m_currentPlayer = pSavPlayer;
}

//-------------------------------------------------------------------------------------------------
/** adds an object to the cache, allowing it to be looked up by name */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::addObjectToCache(Object* pNewObject)
{
	if (!pNewObject) {
		return;
	}

	AsciiString objName = pNewObject->getName();

	if (objName == AsciiString::TheEmptyString) {
		return;
	}

	for (VecNamedRequestsIt it = m_namedObjects.begin(); it != m_namedObjects.end(); ++it) {
		if (it->first == objName) {
			if (it->second == NULL) {
				AsciiString newNameForDead;
				newNameForDead.format("Reassigning dead object's name '%s' to object (%d) of type '%s'\n", objName.str(), pNewObject->getID(), pNewObject->getTemplate()->getName().str());
				TheScriptEngine->AppendDebugMessage(newNameForDead, FALSE);
				DEBUG_LOG((newNameForDead.str()));
				it->second = pNewObject;
				return;
			} else {
				DEBUG_CRASH(("Attempting to assign the name '%s' to object (%d) of type '%s'," 
										 " but object (%d) of type '%s' already has that name\n",
										 objName.str(), pNewObject->getID(), pNewObject->getTemplate()->getName().str(), 
										 it->second->getID(), it->second->getTemplate()->getName().str()));
				return;
			}
		}

		if (pNewObject == (it->second)) {
			it->first = objName;
			return;
		}
	}

	NamedRequest req;
	req.first = objName;
	req.second = pNewObject;

	m_namedObjects.push_back(req);
}

//-------------------------------------------------------------------------------------------------
/** removes a dead object from the cache, to prevent "Bad Stuff"(r) */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::removeObjectFromCache( Object* pDeadObject )
{
	for (VecNamedRequestsIt it = m_namedObjects.begin(); it != m_namedObjects.end(); ++it) {
		if (pDeadObject == (it->second)) {
			it->second = NULL;	// Don't remove it, cause we want to check whether we ever knew a name later
			break;
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Kris:
		Looks for existing cached object with same name and replaces that object point with the supplied one. 
		This is an important feature for units that change into something else. Good examples include terrorists
		entering a vehicle to convert it to a carbomb, pilots adding veterancy to vehicles, hijackers stealing
		vehicles, and infantry taking over disabled vehicles.
*/
//-------------------------------------------------------------------------------------------------
void ScriptEngine::transferObjectName( const AsciiString& unitName, Object *pNewObject )
{
	//Sanity checks
	if( !pNewObject || !unitName.getLength() )
	{
		return;
	}

	//John Ahlquist: When transferring an object name, make sure the new object isn't already in 
	//							 the vector. If so, remove it, or it'll end up there twice and cause a crash.
	if( pNewObject->getName().isNotEmpty() ) 
	{
		removeObjectFromCache(pNewObject);
	}

	pNewObject->setName(unitName); // make sure it's named the name.

	//Loop through the cached list and find the string entry. If found, change the object
	//so it's pointing to the new one.
	for( VecNamedRequestsIt it = m_namedObjects.begin(); it != m_namedObjects.end(); ++it )
	{
		if( !unitName.compare( it->first ) )
		{
			Object* pOldObj = it->second;
			if( pOldObj )
			{
				// if you are transferring your name, you should also transfer any custom indicator color you have.
				if (pOldObj->hasCustomIndicatorColor())
					pNewObject->setCustomIndicatorColor(pOldObj->getIndicatorColor());
				else
					pNewObject->removeCustomIndicatorColor();
			}

			it->second = pNewObject;

			return;
		}
	}

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ScriptEngine::notifyOfObjectDestruction( Object *pDeadObject )
{
	if (!pDeadObject->getName().isEmpty()) 
	{
		removeObjectFromCache(pDeadObject);
	}

	if (m_conditionObject == pDeadObject) {
		m_conditionObject = NULL;
	}

	if (m_callingObject == pDeadObject) {
		m_callingObject = NULL;
	}
}

//-------------------------------------------------------------------------------------------------
/** Notify the script engine that a video has completed */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::notifyOfCompletedVideo( const AsciiString& completedVideo ) 
{
	m_completedVideo.push_back(completedVideo);
}

//-------------------------------------------------------------------------------------------------
/** Notify the script engine that a special power fired */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::notifyOfTriggeredSpecialPower( Int playerIndex, const AsciiString& completedPower, ObjectID sourceObj )
{
	m_triggeredSpecialPowers[playerIndex].push_back(AsciiStringObjectIDPair(completedPower, sourceObj));
}

//-------------------------------------------------------------------------------------------------
/** Notify the script engine that a special power fired */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::notifyOfMidwaySpecialPower( Int playerIndex, const AsciiString& completedPower, ObjectID sourceObj )
{
	m_midwaySpecialPowers[playerIndex].push_back(AsciiStringObjectIDPair(completedPower, sourceObj));
}

//-------------------------------------------------------------------------------------------------
/** Notify the script engine that a special power fired */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::notifyOfCompletedSpecialPower( Int playerIndex, const AsciiString& completedPower, ObjectID sourceObj )
{
	m_finishedSpecialPowers[playerIndex].push_back(AsciiStringObjectIDPair(completedPower, sourceObj));
}

//-------------------------------------------------------------------------------------------------
/** Notify the script engine that an upgrade finished */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::notifyOfCompletedUpgrade( Int playerIndex, const AsciiString& upgrade, ObjectID sourceObj )
{
	m_completedUpgrades[playerIndex].push_back(AsciiStringObjectIDPair(upgrade, sourceObj));
}

//-------------------------------------------------------------------------------------------------
/** Notify the script engine that a general was chosen fired */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::notifyOfAcquiredScience( Int playerIndex, ScienceType science )
{
	m_acquiredSciences[playerIndex].push_back(science);
}

//-------------------------------------------------------------------------------------------------
/** Notify that a UI button was pressed and some flag should go true, for one frame only. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::signalUIInteract(const AsciiString& hookName)
{
	m_uiInteractions.push_front(hookName);
#ifdef DEBUG_LOGGING
	AppendDebugMessage(hookName, false); // don't bother in Release
#endif
}

//-------------------------------------------------------------------------------------------------
/** Determine whether a video has completed */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isVideoComplete( const AsciiString& completedVideo, Bool removeFromList )
{
	ListAsciiStringIt findIt = std::find(m_completedVideo.begin(), m_completedVideo.end(), completedVideo);
	if (findIt != m_completedVideo.end()) {
		if (removeFromList) {
			m_completedVideo.erase(findIt);
		}
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** Determine whether a speech has completed */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isSpeechComplete( const AsciiString& testSpeech, Bool removeFromList )
{
	ListAsciiStringUINTIt findIt;
	for (findIt = m_testingSpeech.begin(); findIt != m_testingSpeech.end(); ++findIt) {
		if (findIt->first == testSpeech) {
			break;
		}
	}

	if (findIt == m_testingSpeech.end()) {
		PairAsciiStringUINT newPair;
		AudioEventRTS event(testSpeech);
		Real audioLength = TheAudio->getAudioLengthMS(&event);
		UnsignedInt frameCount = REAL_TO_UNSIGNEDINT(audioLength / MSEC_PER_LOGICFRAME_REAL);

		newPair.first = testSpeech;
		newPair.second = frameCount + TheGameLogic->getFrame();
		m_testingSpeech.push_front(newPair);
		findIt = m_testingSpeech.begin();
	}

	if (TheGameLogic->getFrame() >= findIt->second) {
		// This audio is completed
		if (removeFromList) {
			m_testingSpeech.erase(findIt);
		}
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/** Determine whether a sound has completed */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isAudioComplete( const AsciiString& testAudio, Bool removeFromList )
{
	ListAsciiStringUINTIt findIt;
	for (findIt = m_testingAudio.begin(); findIt != m_testingAudio.end(); ++findIt) {
		if (findIt->first == testAudio) {
			break;
		}
	}

	if (findIt == m_testingAudio.end()) {
		PairAsciiStringUINT newPair;
		AudioEventRTS event(testAudio);
		Real audioLength = TheAudio->getAudioLengthMS(&event);
		UnsignedInt frameCount = REAL_TO_UNSIGNEDINT(audioLength / MSEC_PER_LOGICFRAME_REAL);

		newPair.first = testAudio;
		newPair.second = frameCount + TheGameLogic->getFrame();
		m_testingAudio.push_front(newPair);
		findIt = m_testingAudio.begin();
	}

	if (TheGameLogic->getFrame() >= findIt->second) {
		// This audio is completed
		if (removeFromList) {
			m_testingAudio.erase(findIt);
		}
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------------------
/** Determine whether a special power has been started */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isSpecialPowerTriggered( Int playerIndex, const AsciiString& completedPower, Bool removeFromList, ObjectID sourceObj )
{
	if (playerIndex < 0 || playerIndex >= MAX_PLAYER_COUNT)
		return FALSE;

	ListAsciiStringObjectID *specialList = &(m_triggeredSpecialPowers[playerIndex]);

	for (ListAsciiStringObjectIDIt findIt = specialList->begin(); findIt != specialList->end(); ++findIt)
	{
		AsciiStringObjectIDPair pair = *findIt;
		if (pair.first == completedPower && (sourceObj == INVALID_ID || sourceObj == pair.second))
		{
			if (removeFromList) {
				specialList->erase(findIt);
			}
			return TRUE;
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** Determine whether a special power has reached a midpoint (not required for all special powers!) */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isSpecialPowerMidway( Int playerIndex, const AsciiString& completedPower, Bool removeFromList, ObjectID sourceObj )
{
	if (playerIndex < 0 || playerIndex >= MAX_PLAYER_COUNT)
		return FALSE;

	ListAsciiStringObjectID *specialList = &(m_midwaySpecialPowers[playerIndex]);

	for (ListAsciiStringObjectIDIt findIt = specialList->begin(); findIt != specialList->end(); ++findIt)
	{
		AsciiStringObjectIDPair pair = *findIt;
		if (pair.first == completedPower && (sourceObj == INVALID_ID || sourceObj == pair.second))
		{
			if (removeFromList) {
				specialList->erase(findIt);
			}
			return TRUE;
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** Determine whether a special power has been finished */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isSpecialPowerComplete( Int playerIndex, const AsciiString& completedPower, Bool removeFromList, ObjectID sourceObj )
{
	if (playerIndex < 0 || playerIndex >= MAX_PLAYER_COUNT)
		return FALSE;

	ListAsciiStringObjectID *specialList = &(m_finishedSpecialPowers[playerIndex]);

	for (ListAsciiStringObjectIDIt findIt = specialList->begin(); findIt != specialList->end(); ++findIt)
	{
		AsciiStringObjectIDPair pair = *findIt;
		if (pair.first == completedPower && (sourceObj == INVALID_ID || sourceObj == pair.second))
		{
			if (removeFromList) {
				specialList->erase(findIt);
			}
			return TRUE;
		}
	}

	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** Determine whether an upgrade has been completed */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isUpgradeComplete( Int playerIndex, const AsciiString& upgrade, Bool removeFromList, ObjectID sourceObj )
{
	if (playerIndex < 0 || playerIndex >= MAX_PLAYER_COUNT)
		return FALSE;

	ListAsciiStringObjectID *specialList = &(m_completedUpgrades[playerIndex]);

	for (ListAsciiStringObjectIDIt findIt = specialList->begin(); findIt != specialList->end(); ++findIt)
	{
		AsciiStringObjectIDPair pair = *findIt;
		if (pair.first == upgrade && (sourceObj == INVALID_ID || sourceObj == pair.second))
		{
			if (removeFromList) {
				specialList->erase(findIt);
			}
			return TRUE;
		}
	}

	return FALSE;
}

#ifdef OLD_GENERALS_SPECIALIZATION
//-------------------------------------------------------------------------------------------------
/** Determine whether a general has been chosen */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isGeneralChosen( Int playerIndex, const AsciiString& generalName, Bool removeFromList, ObjectID sourceObj )
{
	if (playerIndex < 0 || playerIndex >= MAX_PLAYER_COUNT)
		return FALSE;

	ListAsciiStringObjectID *specialList = &(m_chosenGenerals[playerIndex]);

	for (ListAsciiStringObjectIDIt findIt = specialList->begin(); findIt != specialList->end(); ++findIt)
	{
		AsciiStringObjectIDPair pair = *findIt;
		if (pair.first == generalName && (sourceObj == INVALID_ID || sourceObj == pair.second))
		{
			if (removeFromList) {
				specialList->erase(findIt);
			}
			return TRUE;
		}
	}

	return FALSE;
}
#else
//-------------------------------------------------------------------------------------------------
/** Determine whether a science has been chosen */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isScienceAcquired( Int playerIndex, ScienceType science, Bool removeFromList )
{
	if (playerIndex < 0 || playerIndex >= MAX_PLAYER_COUNT)
		return FALSE;

	ScienceVec* specialList = &(m_acquiredSciences[playerIndex]);

	for (ScienceVec::iterator it = specialList->begin(); it != specialList->end(); ++it)
	{
		if (*it == science)
		{
			if (removeFromList) 
			{
				specialList->erase(it);
			}
			return TRUE;
		}
	}

	return FALSE;
}
#endif

//-------------------------------------------------------------------------------------------------
/** if the object has a specified topple direction, change it to direction. Otherwise add it to the
/** list. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::setToppleDirection( const AsciiString& objectName, const Coord3D *direction )
{
	if (objectName.isEmpty()) {
		return;
	}

	ListAsciiStringCoord3DIt it;
	for (it = m_toppleDirections.begin(); it != m_toppleDirections.end(); ++it) {
		if (it->first == objectName) {
			if (direction) {
				it->second = *direction;
			} else {
				m_toppleDirections.erase(it);
			}
			return;
		}
	}

	AsciiStringCoord3DPair newPair;
	newPair.first = objectName;
	newPair.second = (*direction);
	m_toppleDirections.push_front(newPair);
}

//-------------------------------------------------------------------------------------------------
/** if the object is named and has a specified topple direction, topple adjust direction to reflect
/** it. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::adjustToppleDirection( Object *object, Coord2D *direction)
{
	if (!(object && direction)) {
		return;
	}

	Coord3D dir;
	dir.x = (*direction).x;
	dir.y = (*direction).y;
	adjustToppleDirection(object, &dir);
	(*direction).x = dir.x;
	(*direction).y = dir.y;
}

//-------------------------------------------------------------------------------------------------
/** if the object is named and has a specified topple direction, topple adjust direction to reflect
/** it. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::adjustToppleDirection( Object *object, Coord3D *direction)
{
	AsciiString objName = object->getName();
	if (objName.isEmpty() || !direction) {
		return;
	}

	// this one *MAY* have a specified topple direction
	ListAsciiStringCoord3DIt it;
	for (it = m_toppleDirections.begin(); it != m_toppleDirections.end(); ++it) {
		if (it->first == objName) {
			(*direction) = it->second;
			(*direction).normalize();
			return;
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Evaluates a list of conditions */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::evaluateConditions( Script *pScript, Team *thisTeam, Player *player )
{
	LatchRestore<Team*> latch(m_callingTeam, thisTeam);
	if (thisTeam) player = thisTeam->getControllingPlayer();
	if (player==NULL) player=m_currentPlayer;
	LatchRestore<Player*> latch2(m_currentPlayer, player);
	OrCondition *pConditionHead = pScript->getOrCondition();
	Bool testValue = false;

#ifdef DEBUG_LOGGING
#define COLLECT_CONDITION_EVAL_TIMES
#endif
#ifdef COLLECT_CONDITION_EVAL_TIMES
	__int64 startTime64;
	Real timeToEvaluate=0.0f;
	__int64 endTime64,freq64;
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq64);
	QueryPerformanceCounter((LARGE_INTEGER *)&startTime64);
#endif
	OrCondition *pCurCondition;
	for (pCurCondition = pConditionHead; pCurCondition; pCurCondition = pCurCondition->getNextOrCondition()) {
		Condition *pCondition = pCurCondition->getFirstAndCondition();
		if (!pCondition) continue; // No conditions, so go to the next or.
		Bool andTerm = true; 
		while (pCondition && andTerm) {
			if (!evaluateCondition(pCondition)) {
				andTerm = false;
				break; // Short circuit the and evauation - after the first false, we can quit.
			}
			pCondition = pCondition->getNext();
		}
		if (andTerm) { // The outer list is OR'ed - so any true inner means we are true.
			testValue = true;
			break;
		}
	}
#ifdef COLLECT_CONDITION_EVAL_TIMES
	QueryPerformanceCounter((LARGE_INTEGER *)&endTime64);
	timeToEvaluate = ((Real)(endTime64-startTime64) / (Real)(freq64));
	pScript->incrementConditionCount();
	pScript->addToConditionTime(timeToEvaluate);
#endif

	return testValue; // If none of the or's fired, then it is false.
}



//-------------------------------------------------------------------------------------------------
/** Execute a linked list of actions */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::executeActions( ScriptAction *pActionHead )
{
	ScriptAction *pCurAction;
	UnicodeString uStr1;
	for (pCurAction = pActionHead; pCurAction; pCurAction = pCurAction->getNext()) {
		switch (pCurAction->getActionType()) {
			default: if (TheScriptActions) TheScriptActions->executeAction(pCurAction); break;
			case ScriptAction::SET_COUNTER: setCounter(pCurAction);	break;
			case ScriptAction::SET_TREE_SWAY: setSway(pCurAction); break;
			case ScriptAction::INCREMENT_COUNTER: addCounter(pCurAction);	break;
			case ScriptAction::DECREMENT_COUNTER: subCounter(pCurAction);	break;
			case ScriptAction::SET_FLAG: setFlag(pCurAction);break;
			case ScriptAction::STOP_TIMER: pauseTimer(pCurAction);break;
			case ScriptAction::RESTART_TIMER: restartTimer(pCurAction);break;
			case ScriptAction::SET_TIMER: setTimer(pCurAction, false, false);break;
			case ScriptAction::SET_MILLISECOND_TIMER: setTimer(pCurAction, true, false);break;
			case ScriptAction::SET_RANDOM_TIMER: setTimer(pCurAction, false, true);break;
			case ScriptAction::SET_RANDOM_MSEC_TIMER: setTimer(pCurAction, true, true);break;
			case ScriptAction::ADD_TO_MSEC_TIMER: adjustTimer(pCurAction, true, true);break;
			case ScriptAction::SUB_FROM_MSEC_TIMER: adjustTimer(pCurAction, true, false);break;
			case ScriptAction::ENABLE_SCRIPT: enableScript(pCurAction);break;
			case ScriptAction::DISABLE_SCRIPT: disableScript(pCurAction);break;
			case ScriptAction::CALL_SUBROUTINE: callSubroutine(pCurAction);break;

			// Fade operations.
			case ScriptAction::CAMERA_FADE_ADD : 
			case ScriptAction::CAMERA_FADE_SUBTRACT : 
			case ScriptAction::CAMERA_FADE_SATURATE : 
			case ScriptAction::CAMERA_FADE_MULTIPLY : 
				setFade(pCurAction); break;

			// Attack priority set operations.
			case ScriptAction::SET_ATTACK_PRIORITY_THING : setPriorityThing(pCurAction); break;
			case ScriptAction::SET_ATTACK_PRIORITY_KIND_OF : setPriorityKind(pCurAction); break;
			case ScriptAction::SET_DEFAULT_ATTACK_PRIORITY : setPriorityDefault(pCurAction); break;

			case ScriptAction::NO_OP: /* just break. */; break;
		}
	}
}
																		
//-------------------------------------------------------------------------------------------------
/** Execute a linked list of scripts */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::executeScripts( Script *pScriptHead )
{

	// Evaluate the scripts.
	Script *pCurScript;
	for (pCurScript = pScriptHead; pCurScript; pCurScript=pCurScript->getNext()) {
		if (pCurScript->isSubroutine()) {
			continue; // Don't execute subroutines, except when called by other scripts.
		}
		executeScript(pCurScript);
	}
}  // end update


//-------------------------------------------------------------------------------------------------
/** Gets the ui and parameter template for a script action */
//-------------------------------------------------------------------------------------------------
const ActionTemplate * ScriptEngine::getActionTemplate( Int ndx )
{
	DEBUG_ASSERTCRASH(ndx >= 0 && ndx < ScriptAction::NUM_ITEMS, ("Out of range.\n"));
	if (ndx <0 || ndx >= ScriptAction::NUM_ITEMS) ndx = 0;
	DEBUG_ASSERTCRASH (!m_actionTemplates[ndx].getName().isEmpty(), ("Need to initialize action enum=%d.\n", ndx));
	
	return &m_actionTemplates[ndx];
}  // end getActionTemplate

//-------------------------------------------------------------------------------------------------
/** Gets the ui and parameter template for a script condition */
//-------------------------------------------------------------------------------------------------
const ConditionTemplate * ScriptEngine::getConditionTemplate( Int ndx )
{
	DEBUG_ASSERTCRASH(ndx >= 0 && ndx < ScriptAction::NUM_ITEMS, ("Out of range.\n"));
	if (ndx <0 || ndx >= Condition::NUM_ITEMS) ndx = 0;
	DEBUG_ASSERTCRASH (!m_conditionTemplates[ndx].getName().isEmpty(), ("Need to initialize Condition enum=%d.\n", ndx));
	return &m_conditionTemplates[ndx];
}  // end getConditionTemplate

//-------------------------------------------------------------------------------------------------
/** Fills the named object cache initally. */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::createNamedCache( void )
{
	m_namedObjects.clear();

	if( !TheGameLogic )
	{
		return;
	}
	Object* pObj = TheGameLogic->getFirstObject();

	while (pObj) {
		if (!pObj->getName().isEmpty()) {
			NamedRequest req;
			req.first = pObj->getName();
			req.second = pObj;
			m_namedObjects.push_back(req);
		}
		pObj = pObj->getNextObject();
	}
}

void ScriptEngine::appendSequentialScript(const SequentialScript *scriptToSequence)
{
	SequentialScript *newSequentialScript = newInstance( SequentialScript );	
	(*newSequentialScript) = (*scriptToSequence);

	// Must set this to NULL, as we don't want an infinite loop.
	newSequentialScript->m_nextScriptInSequence = NULL;

	// reset the instruction pointer
	newSequentialScript->m_currentInstruction = -1;
	
	VecSequentialScriptPtrIt it;
	Bool found = false;
	for (it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); ++it) {
		SequentialScript *seqScript = (*it);
		if (!seqScript) {
			continue;
		}

		if ((scriptToSequence->m_objectID && scriptToSequence->m_objectID == seqScript->m_objectID) ||
				 (scriptToSequence->m_teamToExecOn && scriptToSequence->m_teamToExecOn == seqScript->m_teamToExecOn)) {
			found = true;
			while (seqScript->m_nextScriptInSequence) { 
				seqScript = seqScript->m_nextScriptInSequence;
			}

			seqScript->m_nextScriptInSequence = newSequentialScript;
			break;
		}
	}

	if (!found) {
		m_sequentialScripts.push_back(newSequentialScript);
	}

	// do not delete either of these here. 
}

void ScriptEngine::removeSequentialScript(SequentialScript *scriptToRemove)
{
	
}

void ScriptEngine::removeAllSequentialScripts(Object *obj)
{
	if (!obj) {
		return;
	}

	ObjectID id = obj->getID();
	VecSequentialScriptPtrIt it;
	for (it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); /* empty */) {
		SequentialScript *seqScript = (*it);
		if (!seqScript) {
			continue;
		}
		if (seqScript->m_objectID == id) {
			cleanupSequentialScript(it, TRUE);
		}
		++it;
	}
}

void ScriptEngine::removeAllSequentialScripts(Team *team)
{
	// this function will remove all pending scripts for this team, so just call it.
	notifyOfTeamDestruction(team);
}

void ScriptEngine::notifyOfObjectCreationOrDestruction(void)
{
	m_frameObjectCountChanged = TheGameLogic->getFrame();
}

void ScriptEngine::notifyOfTeamDestruction(Team *teamDestroyed)
{
	if (!teamDestroyed) {
		return;		
	}

	VecSequentialScriptPtrIt it;
	for (it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); /* empty */) {
		SequentialScript *seqScript = (*it);
		if (!seqScript) {
			continue;
		}
		
		if (seqScript->m_teamToExecOn == teamDestroyed) {
			it = cleanupSequentialScript(it, TRUE);
			continue;
		}
		++it;
	}

	if (m_callingTeam == teamDestroyed)
		m_callingTeam = NULL;
	if (m_conditionTeam == teamDestroyed)
		m_conditionTeam = NULL;
}

void ScriptEngine::setSequentialTimer(Object *obj, Int frameCount)
{
	if (!obj) {
		return;
	}

	ObjectID id = obj->getID();
	VecSequentialScriptPtrIt it;
	for (it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); ++it) {
		SequentialScript *seqScript = (*it);
		if (!seqScript) {
			continue;
		}

		if (seqScript->m_objectID == id) {
			// this is it.
			seqScript->m_framesToWait = frameCount;
			return;
		}
	}
}

void ScriptEngine::setSequentialTimer(Team *team, Int frameCount)
{
	if (!team) {
		return;
	}

	VecSequentialScriptPtrIt it;
	for (it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); ++it) {
		SequentialScript *seqScript = (*it);
		if (!seqScript) {
			continue;
		}

		if (seqScript->m_teamToExecOn == team) {
			// this is it.
			seqScript->m_framesToWait = frameCount;
			return;
		}
	}
}


void ScriptEngine::evaluateAndProgressAllSequentialScripts( void )
{
	VecSequentialScriptPtrIt it;
	SequentialScript* lastScript = NULL;
	Bool itAdvanced = false;

	Int spinCount = 0;
	for (it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); /* empty */) {
		if ((*it) == lastScript) {
			++spinCount;
		} else {
			spinCount = 0;
		}

		if (spinCount > MAX_SPIN_COUNT) {
			SequentialScript *seqScript = (*it);
			if (seqScript) {
				DEBUG_LOG(("Sequential script %s appears to be in an infinite loop.\n", 
					seqScript->m_scriptToExecuteSequentially->getName().str()));
			}
			++it;
			continue;
		}

		lastScript = (*it);
		
		itAdvanced = false;

		SequentialScript *seqScript = (*it);
		if (seqScript == NULL) {
			it = cleanupSequentialScript(it, false);
			continue;
		}

		Team *team = seqScript->m_teamToExecOn;
		Object *obj = TheGameLogic->findObjectByID(seqScript->m_objectID);
		if (!(obj || team)) {
			it = cleanupSequentialScript(it, false);
			itAdvanced = true;
			continue;
		}
		m_currentPlayer = NULL;
		if (obj) {
			m_currentPlayer = obj->getControllingPlayer();
		} else if (team) {
			m_currentPlayer = team->getControllingPlayer();
		}
		if (m_currentPlayer && !m_currentPlayer->isSkirmishAIPlayer()) {
			m_currentPlayer = NULL;
		}

		AIUpdateInterface *ai = obj ? obj->getAIUpdateInterface() : NULL;
		AIGroup *aigroup = (team ? TheAI->createGroup() : NULL);
		if (aigroup) {
			team->getTeamAsAIGroup(aigroup);
		}

		if( ai || aigroup ) {
			if (((ai && (ai->isIdle()) || (aigroup && aigroup->isIdle())) && 
				seqScript->m_framesToWait < 1) || (seqScript->m_framesToWait == 0)) {
				
				// We want to supress messages if we're repeatedly waiting for an event to occur, cause 
				// it KILLS our debug framerate.
				Bool displayMessage = TRUE;

				// Time to progress to the next task.
				if (seqScript->m_dontAdvanceInstruction) {
					seqScript->m_dontAdvanceInstruction = FALSE;
					displayMessage = FALSE;
				} else {
					++seqScript->m_currentInstruction;
				}

				AsciiString msg = "Advancing SeqScript '";
				msg.concat(seqScript->m_scriptToExecuteSequentially->getName());
				msg.concat("' on ");
				AsciiString name;
				if (team) name = team->getName();
				if (obj) name = obj->getName();
				msg.concat(name);
				msg.concat(" -- ");

				int instruction = seqScript->m_currentInstruction;
				ScriptAction *action = seqScript->m_scriptToExecuteSequentially->getAction();
				while (action && instruction) {
					--instruction;
					action = action->getNext();
				}

				if (action) {
					m_conditionTeam = team;
					m_conditionObject = obj;
					seqScript->m_framesToWait = -1;

					// Save off the next action
					ScriptAction *nextAction = action->getNext();
					action->setNextAction(NULL);
					if (action->getActionType() == ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL) {
						if (!TheScriptConditions->evaluateSkirmishCommandButtonIsReady(NULL, action->getParameter(1), action->getParameter(2), true)) {
							seqScript->m_dontAdvanceInstruction = TRUE;
						}
					} else if (action->getActionType() == ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL) {
						if (!TheScriptConditions->evaluateSkirmishCommandButtonIsReady(NULL, action->getParameter(1), action->getParameter(2), false)) {
							seqScript->m_dontAdvanceInstruction = TRUE;
						}
					} else if (action->getActionType() == ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_ALL) {
						if (TheScriptConditions->evaluateTeamIsContained(action->getParameter(0), true)) {
							seqScript->m_dontAdvanceInstruction = TRUE;
						}
					} else if (action->getActionType() == ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL) {
						if (TheScriptConditions->evaluateTeamIsContained(action->getParameter(0), false)) {
							seqScript->m_dontAdvanceInstruction = TRUE;
						}
					} else {
						executeActions(action);
					}

					if (displayMessage) {
						msg.concat(action->getUiText());
						AppendDebugMessage(msg, false);
					} else {
						msg.clear();
					}

					action->setNextAction(nextAction);
					
					// Check to see if executing our action told us to wait. If so, skip to the next Sequential script
					if (seqScript->m_dontAdvanceInstruction) {
						++it;
						itAdvanced = true;
						continue;
					}

					if (ai && ai->isIdle()) {
						// pretend like we've already advanced to allow multiple checks on this object this frame.
						itAdvanced = true;
					} else if (team) {
						// attempt to rebuild the aigroup, as it probably expired during the action execution
						aigroup = (team ? TheAI->createGroup() : NULL);
						team->getTeamAsAIGroup(aigroup);
					}

					if (aigroup && aigroup->isIdle()) {
						// pretend like we've already advanced to allow multiple checks on this object this frame.
						itAdvanced = true;
					}

					if (itAdvanced) {	// check to make sure they aren't dead.
						if (obj && obj->isEffectivelyDead()) {
							it = cleanupSequentialScript(it, true);
							continue;
						}

						if (aigroup && aigroup->isGroupAiDead()) {
							it = cleanupSequentialScript(it, true);
							continue;
						}
					}
				} else {
					if (seqScript->m_timesToLoop != 0) {
						if (seqScript->m_timesToLoop != -1) {
							--seqScript->m_timesToLoop;
						}

						seqScript->m_framesToWait = -1;
						appendSequentialScript(seqScript);
					}

					it = cleanupSequentialScript(it, false);
					itAdvanced = true;
				}
			} else if (seqScript->m_framesToWait > 0) {
				--seqScript->m_framesToWait;
			}
		}

		if (!itAdvanced) {
			++it;
		}
	}
	m_currentPlayer = NULL;
}

ScriptEngine::VecSequentialScriptPtrIt ScriptEngine::cleanupSequentialScript(VecSequentialScriptPtrIt it, Bool cleanDanglers)
{
	SequentialScript *seqScript;
	seqScript = (*it);
	if (!seqScript) {
		return it;				
	}

	SequentialScript *scriptToDelete = seqScript;
	if (cleanDanglers) {
		while (seqScript) {
			scriptToDelete = seqScript;
			seqScript = seqScript->m_nextScriptInSequence;
			scriptToDelete->deleteInstance();
			scriptToDelete = NULL;
		}
		(*it) = NULL;
	} else {
		// we want to make sure to not delete any dangling scripts.
		(*it) = scriptToDelete->m_nextScriptInSequence;
		scriptToDelete->deleteInstance();
		scriptToDelete = NULL;
	}


	if ((*it) == NULL) {
		return m_sequentialScripts.erase(it);
	}

	return it;
}

Bool ScriptEngine::hasUnitCompletedSequentialScript( Object *object, const AsciiString& sequentialScriptName )
{

	return FALSE;
}

Bool ScriptEngine::hasTeamCompletedSequentialScript( Team *team, const AsciiString& sequentialScriptName )
{

	return FALSE;
}

Bool ScriptEngine::getEnableVTune() const
{
#ifdef DO_VTUNE_STUFF
	return st_EnableVTune;
#else
	return false;
#endif
}

void ScriptEngine::setEnableVTune(Bool value)
{
#ifdef DO_VTUNE_STUFF
	st_EnableVTune = value;
#endif
}

//----SequentialScript-----------------------------------------------------------------------------
SequentialScript::SequentialScript() : m_teamToExecOn(NULL), 
																			 m_objectID(INVALID_ID), 
																			 m_scriptToExecuteSequentially(NULL), 
																			 m_currentInstruction(START_INSTRUCTION),
																			 m_timesToLoop(0),
																			 m_framesToWait(-1),
																			 m_dontAdvanceInstruction(FALSE),
																			 m_nextScriptInSequence(NULL)
{
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SequentialScript::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SequentialScript::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// team
	TeamID teamID = m_teamToExecOn ? m_teamToExecOn->getID() : TEAM_ID_INVALID;
	xfer->xferUser( &teamID, sizeof( TeamID ) );
	if( xfer->getXferMode() == XFER_LOAD )
	{

		// tie up pointer
		m_teamToExecOn = TheTeamFactory->findTeamByID( teamID );

		// sanity
		if( teamID != TEAM_ID_INVALID && m_teamToExecOn == NULL )
		{

			DEBUG_CRASH(( "SequentialScript::xfer - Unable to find team by ID (#%d) for m_teamToExecOn\n", 
										teamID ));
			throw SC_INVALID_DATA;

		}  // end if

	}  // end if

	// object id
	xfer->xferObjectID( &m_objectID );

	// saving
	AsciiString scriptName;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// write script name
		scriptName = m_scriptToExecuteSequentially->getName();
		xfer->xferAsciiString( &scriptName );

	}  // end if, save
	else
	{

		// read script name
		xfer->xferAsciiString( &scriptName );

		// script pointer
		DEBUG_ASSERTCRASH( m_scriptToExecuteSequentially == NULL, ("SequentialScript::xfer - m_scripttoExecuteSequentially\n") );
			
		// find script
		m_scriptToExecuteSequentially = const_cast<Script*>(TheScriptEngine->findScriptByName(scriptName));

		// sanity	
		DEBUG_ASSERTCRASH( m_scriptToExecuteSequentially != NULL,
											 ("SequentialScript::xfer - m_scriptToExecuteSequentially is NULL but should not be\n") );

	}  // end else, load

	// current instruction
	xfer->xferInt( &m_currentInstruction );

	// times to loop
	xfer->xferInt( &m_timesToLoop );

	// frames to wait
	xfer->xferInt( &m_framesToWait );

	// dont advance instruction
	xfer->xferBool( &m_dontAdvanceInstruction );
	
}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SequentialScript::loadPostProcess( void )
{

}  // end loadPostProcess

#ifdef NOT_IN_USE
//----SequentialScriptStatus Stuff ----------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SequentialScriptStatus::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SequentialScriptStatus::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// object id
	xfer->xferObjectID( &m_objectID );

	// sequential script completed
	xfer->xferAsciiString( &m_sequentialScriptCompleted );

	// is executing sequentially
	xfer->xferBool( &m_isExecutingSequentially );

}  // end xfer

// ------------------------------------------------------------------------------------------------
// Load post process */
// ------------------------------------------------------------------------------------------------
void SequentialScriptStatus::loadPostProcess( void )
{

}  // end loadPostProcess
#endif

//----Particle Editor Stuff------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/** Updates the particle editor if its present */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::particleEditorUpdate( void )
{
	if (!st_ParticleDLL) {
		return;
	}

	_updateCurrentParticleCount();
	
	Bool busyWait = false;
	do {
		if (m_firstUpdate) {
			_appendAllParticleSystems();
			_appendAllThingTemplates();
		} else {
			switch (_getEditorBehavior())
			{			
				case 0x00:
				{
					busyWait = false;
					return;
				}

				case 0x01:
				case 0x02:
				{
					_updateAndSetCurrentSystem();
					busyWait = false;
					break;
				}
				
				case 0x03:
				{
					AsciiString particleSystemName = _getParticleSystemName();
					_addUpdatedParticleSystem(particleSystemName);
					ParticleSystemTemplate *pTemp = const_cast<ParticleSystemTemplate*>(TheParticleSystemManager->findTemplate(particleSystemName));
					if (pTemp) {
						// make sure that this system is fully up to date.
						_updateAsciiStringParmsToSystem(pTemp);
					}
					_writeOutINI();
					busyWait = false;
					break;
				}

				case 0x04:
				{
					AsciiString particleSystemName = _getParticleSystemName();
					_reloadParticleSystemFromINI(particleSystemName);
					busyWait = false;
					return;
				}
				
				case 0x05:
				{
					int newCap = _getNewCurrentParticleCap();
					if (newCap >= 0) {
						TheWritableGlobalData->m_maxParticleCount = newCap;
					}
					busyWait = false;
				}

				case 0x06:
				{
					_reloadTextures();
					busyWait = false;
					break;
				}

				// destroy all particle systems
				case 0x07:
				{

					TheParticleSystemManager->reset();

/*
					//iterate through particle system list and remove each particle system
					ParticleSystemManager::ParticleSystemList particleSysList;
					ParticleSystemManager::ParticleSystemListIt it;
					while (true)
					{
						// reassign values into variables
						particleSysList = TheParticleSystemManager->getAllParticleSystems();
						it = particleSysList.begin();
						if (it == particleSysList.end())
							break;

						// check to make sure the particle system is valid
						ParticleSystem *sys = (*it);
						if (!sys)
							continue;

						//before removing the system, make sure to remove all of its particles individually
						while (sys->getParticleCount() > 0) {
							TheParticleSystemManager->removeParticle(sys->getFirstParticle());
							sys->removeParticle(sys->getFirstParticle());
						}

						TheParticleSystemManager->removeParticleSystem(sys);
						++it;
					}
*/

					//Int particleNum = TheParticleSystemManager->getParticleCount();
					_updateCurrentParticleCount(); // probably don't need this...
				}

				case 0xFE:
				{
					busyWait = true;
					break;
				}

				case 0xFF:
				{
					busyWait = false;
					break;
				}
			}
		}
	} while (busyWait);
}

//-------------------------------------------------------------------------------------------------
/** Is time frozen by a script? */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isTimeFrozenScript( void )
{
	return m_freezeByScript;
}

//-------------------------------------------------------------------------------------------------
/** Freeze time */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::doFreezeTime( void )
{
	m_freezeByScript = TRUE;
}

//-------------------------------------------------------------------------------------------------
/** Unfreeze time */
//-------------------------------------------------------------------------------------------------
void ScriptEngine::doUnfreezeTime( void )
{
	m_freezeByScript = FALSE;
}

//-------------------------------------------------------------------------------------------------
/** For Debug and Internal builds, returns whether to continue (!pause), for release, returns false */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isTimeFrozenDebug(void)
{
	typedef Bool (*funcptr)(void);

	if (st_DebugDLL) {
		if (st_LastCurrentFrame != st_CurrentFrame) {
			st_LastCurrentFrame = st_CurrentFrame;

			FARPROC proc = GetProcAddress(st_DebugDLL, "CanAppContinue");
			if (proc) {
				st_CanAppCont = ((funcptr)proc)();

				if (st_CanAppCont) {
				}
			}
		}
		return !st_CanAppCont;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
/** For Debug and Internal builds, returns whether we are running fast (skipping draw) */
//-------------------------------------------------------------------------------------------------
Bool ScriptEngine::isTimeFast(void)
{
	typedef Bool (*funcptr)(void);

	if (st_DebugDLL) {
		FARPROC proc = GetProcAddress(st_DebugDLL, "CanAppContinue");
 		proc = GetProcAddress(st_DebugDLL, "RunAppFast");
		if (proc && ((funcptr)proc)()) {
			st_AppIsFast = true;
		} else {
			if (st_AppIsFast) {
				st_AppIsFast = false;
			} 
		}
		if (st_AppIsFast) {
			if ((TheGameLogic->getFrame()%10) == 0) {
				return false;
			}	 
			return true;
		} else {
			return false;
		}
	}
	return false;
}

void ScriptEngine::forceUnfreezeTime(void)
{
	typedef void (*funcptr)(void);

	if (st_DebugDLL) {
		FARPROC proc = GetProcAddress(st_DebugDLL, "ForceAppContinue");
		if (proc) {
			((funcptr)proc)();
		}
	}
}

void ScriptEngine::AppendDebugMessage(const AsciiString& strToAdd, Bool forcePause)
{
#ifdef INTENSE_DEBUG
	DEBUG_LOG(("-SCRIPT- %d %s\n", TheGameLogic->getFrame(), strToAdd.str()));
#endif
	typedef void (*funcptr)(const char*);
	if (!st_DebugDLL) {
		return;
	}

	FARPROC proc;
	if (forcePause) {
		proc = GetProcAddress(st_DebugDLL, "AppendMessageAndPause");
	} else {
		proc = GetProcAddress(st_DebugDLL, "AppendMessage");
	}

	if (!proc) {
		return;
	}
	AsciiString msg;
	msg.format("%d ", TheGameLogic->getFrame());
	msg.concat(strToAdd);
	((funcptr)proc)(msg.str());
}

void ScriptEngine::AdjustDebugVariableData(const AsciiString& variableName, Int value, Bool forcePause)
{
	_adjustVariable(variableName, value, (forcePause != 0));
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ScriptEngine::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
static void xferListAsciiString( Xfer *xfer, ListAsciiString *list )
{

	// sanity
	DEBUG_ASSERTCRASH( list != NULL, ("xferListAsciiString - Invalid parameters\n") );

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// size of list
	UnsignedShort count = list->size();
	xfer->xferUnsignedShort( &count );

	// list data
	AsciiString string;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// write each string
		ListAsciiStringIt it;
		for( it = list->begin(); it != list->end(); ++it )
		{

			string = *it;
			xfer->xferAsciiString( &string );

		}  // end for, it

	}  // end if, save
	else
	{

		// this list should be empty upon loading
		if( list->empty() == FALSE )
		{

			DEBUG_CRASH(( "xferListAsciiString - list should be empty upon loading but is not\n" ));
			throw SC_INVALID_DATA;

		}  // end if

		// read each string
		for( UnsignedShort i = 0; i < count; ++i )
		{

			// read string
			xfer->xferAsciiString( &string );

			// put on list
			list->push_back( string );

		}  // end for, i

	}  // end else, load

}  // end xferListAsciiString

// ------------------------------------------------------------------------------------------------
/** Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
static void xferListAsciiStringUINT( Xfer *xfer, ListAsciiStringUINT *list )
{

	// sanity
	DEBUG_ASSERTCRASH( list != NULL, ("xferListAsciiStringUINT - Invalid parameters\n") );

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// size of list
	UnsignedShort count = list->size();
	xfer->xferUnsignedShort( &count );

	// list data
	AsciiString string;
	UnsignedInt unsignedIntData;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// write each string
		ListAsciiStringUINTIt it;
		for( it = list->begin(); it != list->end(); ++it )
		{

			// string
			string = it->first;
			xfer->xferAsciiString( &string );

			// unsigned int data
			unsignedIntData = it->second;
			xfer->xferUnsignedInt( &unsignedIntData );

		}  // end for, it

	}  // end if, save
	else
	{
		PairAsciiStringUINT newPair;

		// this list should be empty upon loading
		if( list->empty() == FALSE )
		{

			DEBUG_CRASH(( "xferListAsciiStringUINT - list should be empty upon loading but is not\n" ));
			throw SC_INVALID_DATA;

		}  // end if

		// read each string
		for( UnsignedShort i = 0; i < count; ++i )
		{

			// read string
			xfer->xferAsciiString( &string );

			// read unsigned int data
			xfer->xferUnsignedInt( &unsignedIntData );

			// put on list
			newPair.first = string;
			newPair.second = unsignedIntData;
			list->push_back( newPair );

		}  // end for, i

	}  // end else, load

}  // end xferListAsciiStringUINT

// ------------------------------------------------------------------------------------------------
/** Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
static void xferListAsciiStringObjectID( Xfer *xfer, ListAsciiStringObjectID *list )
{

	// sanity
	DEBUG_ASSERTCRASH( list != NULL, ("xferListAsciiStringObjectID - Invalid parameters\n") );

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// size of list
	UnsignedShort count = list->size();
	xfer->xferUnsignedShort( &count );

	// list data
	AsciiString string;
	ObjectID objectID;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// write each string
		ListAsciiStringObjectIDIt it;
		for( it = list->begin(); it != list->end(); ++it )
		{

			// string
			string = it->first;
			xfer->xferAsciiString( &string );

			// object id
			objectID = it->second;
			xfer->xferObjectID( &objectID );

		}  // end for, it

	}  // end if, save
	else
	{
		AsciiStringObjectIDPair newPair;

		// this list should be empty upon loading
		if( list->empty() == FALSE )
		{

			DEBUG_CRASH(( "xferListAsciiStringObjectID - list should be empty upon loading but is not\n" ));
			throw SC_INVALID_DATA;

		}  // end if

		// read each string
		for( UnsignedShort i = 0; i < count; ++i )
		{

			// read string
			xfer->xferAsciiString( &string );

			// read object id data
			xfer->xferObjectID( &objectID );

			// put on list
			newPair.first = string;
			newPair.second = objectID;
			list->push_back( newPair );

		}  // end for, i

	}  // end else, load

}  // end xferListAsciiStringObjectID

// ------------------------------------------------------------------------------------------------
/** Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
static void xferListAsciiStringCoord3D( Xfer *xfer, ListAsciiStringCoord3D *list )
{

	// sanity
	DEBUG_ASSERTCRASH( list != NULL, ("xferListAsciiStringCoord3D - Invalid parameters\n") );

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// size of list
	UnsignedShort count = list->size();
	xfer->xferUnsignedShort( &count );

	// list data
	AsciiString string;
	Coord3D coord;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// write each string
		ListAsciiStringCoord3DIt it;
		for( it = list->begin(); it != list->end(); ++it )
		{

			// string
			string = it->first;
			xfer->xferAsciiString( &string );

			// coord
			coord = it->second;
			xfer->xferCoord3D( &coord );

		}  // end for, it

	}  // end if, save
	else
	{
		AsciiStringCoord3DPair newPair;

		// this list should be empty upon loading
		if( list->empty() == FALSE )
		{

			DEBUG_CRASH(( "xferListAsciiStringCoord3D - list should be empty upon loading but is not\n" ));
			throw SC_INVALID_DATA;

		}  // end if

		// read each string
		for( UnsignedShort i = 0; i < count; ++i )
		{

			// read string
			xfer->xferAsciiString( &string );

			// read coord 
			xfer->xferCoord3D( &coord );

			// put on list
			newPair.first = string;
			newPair.second = coord;
			list->push_back( newPair );

		}  // end for, i

	}  // end else, load

}  // ene xferListAsciiStringCoord3D

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ScriptEngine::setGlobalDifficulty( GameDifficulty difficulty )
{
	DEBUG_LOG(("ScriptEngine::setGlobalDifficulty(%d)\n", ((Int)difficulty)));
	m_gameDifficulty = difficulty;
}

// ------------------------------------------------------------------------------------------------
/** Xfer method 
	* Version Info:
	* 1: Initial version
	* 2: Added m_namedReveals and m_allObjectTypeLists (CBD)
	* 3: Added m_objectsShouldReceiveDifficultyBonus (JKMCD)
	* 4: current music track info
	* 5: add ChooseVictimAlwaysUsesNormal
	*/
// ------------------------------------------------------------------------------------------------
void ScriptEngine::xfer( Xfer *xfer )
{
	Int i;

	// version
	const XferVersion currentVersion = 5;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// sequential script count and data
	UnsignedShort sequentialScriptCount = m_sequentialScripts.size();
	xfer->xferUnsignedShort( &sequentialScriptCount );
	SequentialScript *sequentialScript;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// save each element
		VecSequentialScriptPtrIt it;
		for( it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); ++it )
		{

			// get data
			sequentialScript = *it;

			// xfer data
			xfer->xferSnapshot( sequentialScript );

		}  // end for, it

	}  // end if, save
	else
	{

		// this list should be empty on loading
		if( m_sequentialScripts.size() != 0 )
		{

			DEBUG_CRASH(( "ScriptEngine::xfer - m_sequentialScripts should be empty but is not\n" ));
			throw SC_INVALID_DATA;

		}  // end if

		// read each entry
		for( UnsignedShort i = 0; i < sequentialScriptCount; ++i )
		{

			// allocate new sequential script and put on our list
			sequentialScript = newInstance( SequentialScript );

			// tie to our list
			m_sequentialScripts.push_back( sequentialScript );

			// xfer data
			xfer->xferSnapshot( sequentialScript );

		}  // end for i

	}  // end else, load

	// counters
	UnsignedShort countersSize = m_numCounters;
	xfer->xferUnsignedShort( &countersSize );
	if( countersSize > MAX_COUNTERS )
	{

		DEBUG_CRASH(( "ScriptEngine::xfer - MAX_COUNTERS has changed size, need to version this\n" ));
		throw SC_INVALID_DATA;

	}  // end if
	for( i = 0; i < countersSize; ++i )
	{

		// value
		xfer->xferInt( &m_counters[ i ].value );

		// name
		xfer->xferAsciiString( &m_counters[ i ].name );

		// countdown timer
		xfer->xferBool( &m_counters[ i ].isCountdownTimer );
		
	}  // end for, i

	// num counters
	xfer->xferInt( &m_numCounters );

	// flags
	UnsignedShort flagsSize = m_numFlags;
	xfer->xferUnsignedShort( &flagsSize );
	if( flagsSize > MAX_FLAGS )
	{
	
		DEBUG_CRASH(( "ScriptEngine::xfer - MAX_FLAGS has changed size, need to version this\n" ));
		throw SC_INVALID_DATA;

	}  // end if
	for( i = 0; i < flagsSize; ++i )
	{

		// value
		xfer->xferBool( &m_flags[ i ].value );
		
		// name
		xfer->xferAsciiString( &m_flags[ i ].name );

	}  // end for i
		
	// num flags
	xfer->xferInt( &m_numFlags );

	// attack priority info
	UnsignedShort attackPriorityInfoSize = m_numAttackInfo;
	xfer->xferUnsignedShort( &attackPriorityInfoSize );
	if( attackPriorityInfoSize > MAX_ATTACK_PRIORITIES )
	{

		DEBUG_CRASH(( "ScriptEngine::xfer - MAX_ATTACK_PRIORITIES size has changed, need to version this\n" ));
		throw SC_INVALID_DATA;

	}  // end if
	for( i = 0; i < attackPriorityInfoSize; ++i )
	{

		// xfer each data
		xfer->xferSnapshot( &m_attackPriorityInfo[ i ] );

	}  // end for i

	// num attack info
	xfer->xferInt( &m_numAttackInfo );

	// end game timers
	xfer->xferInt( &m_endGameTimer );
	xfer->xferInt( &m_closeWindowTimer );

	// named objects
	UnsignedShort namedObjectsCount = m_namedObjects.size();
	xfer->xferUnsignedShort( &namedObjectsCount );
	AsciiString namedObjectName;
	Object *obj;
	ObjectID objectID;
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// iterate elemnts
		VecNamedRequestsIt it;
		for( it = m_namedObjects.begin(); it != m_namedObjects.end(); ++it )
		{

			// write name
			namedObjectName = it->first;
			xfer->xferAsciiString( &namedObjectName );

			// write object id (note that object may be NULL)
			obj = it->second;
			objectID = obj ? obj->getID() : INVALID_ID;
			xfer->xferObjectID( &objectID );

		}  // end for i

	}  // end if, save
	else
	{
		NamedRequest req;

		//
		// list should be empty, it is legal for it to not be empty at this point
		// according to John M., so we're clearing it now
		//
		m_namedObjects.clear();

		// read each element
		for( UnsignedShort i = 0; i < namedObjectsCount; ++i )
		{

			// read name
			xfer->xferAsciiString( &namedObjectName );

			// read object id and turn into object pointer
			xfer->xferObjectID( &objectID );
			obj = TheGameLogic->findObjectByID( objectID );
			if( obj == NULL && objectID != INVALID_ID )
			{

				DEBUG_CRASH(( "ScriptEngine::xfer - Unable to find object by ID for m_namedObjects\n" ));
				throw SC_INVALID_DATA;

			}  // end if

			// assign
			req.first = namedObjectName;
			req.second = obj;
			m_namedObjects.push_back( req );

		}  // end for, i

	}  // end else, load

	// first update
	xfer->xferBool( &m_firstUpdate );

	// trade (this needs a better descriptive name (CBD)
	xfer->xferUser( &m_fade, sizeof( TFade ) );

	// min fade
	xfer->xferReal( &m_minFade );

	// max fade
	xfer->xferReal( &m_maxFade );

	// curr fade value
	xfer->xferReal( &m_curFadeValue );

	// current fade frame
	xfer->xferInt( &m_curFadeFrame );

	// fade frames increase
	xfer->xferInt( &m_fadeFramesIncrease );

	// fade frames hold
	xfer->xferInt( &m_fadeFramesHold );

	// fade frames decrease
	xfer->xferInt( &m_fadeFramesDecrease );

	// complete video
	xferListAsciiString( xfer, &m_completedVideo );

	// testing speech
	xferListAsciiStringUINT( xfer, &m_testingSpeech );

	// testing audio
	xferListAsciiStringUINT( xfer, &m_testingAudio );

	// ui interactions
	xferListAsciiString( xfer, &m_uiInteractions );

	// triggered special powers
	UnsignedShort triggeredSpecialPowersSize = MAX_PLAYER_COUNT;
	xfer->xferUnsignedShort( &triggeredSpecialPowersSize );
	if( triggeredSpecialPowersSize != MAX_PLAYER_COUNT )
	{
	
		DEBUG_CRASH(( "ScriptEngine::xfer - MAX_PLAYER_COUNT has changed, m_triggeredSpecialPowers size is now different and we must version this\n" ));
		throw SC_INVALID_DATA;

	}  // end if
	for( i = 0; i < triggeredSpecialPowersSize; ++i )
		xferListAsciiStringObjectID( xfer, &m_triggeredSpecialPowers[ i ] );

	// midway special powers
	UnsignedShort midwaySpecialPowersSize = MAX_PLAYER_COUNT;
	xfer->xferUnsignedShort( &midwaySpecialPowersSize );
	if( midwaySpecialPowersSize != MAX_PLAYER_COUNT )
	{
	
		DEBUG_CRASH(( "ScriptEngine::xfer - MAX_PLAYER_COUNT has changed, m_midwaySpecialPowers size is now different and we must version this\n" ));
		throw SC_INVALID_DATA;

	}  // end if
	for( i = 0; i < midwaySpecialPowersSize; ++i )
		xferListAsciiStringObjectID( xfer, &m_midwaySpecialPowers[ i ] );

	// finished special powers
	UnsignedShort finishedSpecialPowersSize = MAX_PLAYER_COUNT;
	xfer->xferUnsignedShort( &finishedSpecialPowersSize );
	if( finishedSpecialPowersSize != MAX_PLAYER_COUNT )
	{
	
		DEBUG_CRASH(( "ScriptEngine::xfer - MAX_PLAYER_COUNT has changed, m_finishedSpecialPowers size is now different and we must version this\n" ));
		throw SC_INVALID_DATA;

	}  // end if
	for( i = 0; i < finishedSpecialPowersSize; ++i )
		xferListAsciiStringObjectID( xfer, &m_finishedSpecialPowers[ i ] );

	// completed upgrades
	UnsignedShort completedUpgradesSize = MAX_PLAYER_COUNT;
	xfer->xferUnsignedShort( &completedUpgradesSize );
	if( completedUpgradesSize != MAX_PLAYER_COUNT )
	{
	
		DEBUG_CRASH(( "ScriptEngine::xfer - MAX_PLAYER_COUNT has changed, m_completedUpgrades size is now different and we must version this\n" ));
		throw SC_INVALID_DATA;

	}  // end if
	for( i = 0; i < completedUpgradesSize; ++i )
		xferListAsciiStringObjectID( xfer, &m_completedUpgrades[ i ] );

	// acquired sciences
	UnsignedShort acquiredSciencesSize = MAX_PLAYER_COUNT;
	xfer->xferUnsignedShort( &acquiredSciencesSize );
	if( acquiredSciencesSize != MAX_PLAYER_COUNT )
	{
	
		DEBUG_CRASH(( "ScriptEngine::xfer - MAX_PLAYER_COUNT has changed, m_acquiredSciences size is now different and we must version this\n" ));
		throw SC_INVALID_DATA;

	}  // end if
	for( i = 0; i < acquiredSciencesSize; ++i )
		xfer->xferScienceVec( &m_acquiredSciences[ i ] );

	// topple directions
	xferListAsciiStringCoord3D( xfer, &m_toppleDirections );

	// breeze info
	xfer->xferReal(	&m_breezeInfo.m_direction );
	xfer->xferReal(	&m_breezeInfo.m_directionVec.x );
	xfer->xferReal(	&m_breezeInfo.m_directionVec.y );
	xfer->xferReal( &m_breezeInfo.m_intensity );
	xfer->xferReal( &m_breezeInfo.m_lean );
	xfer->xferReal( &m_breezeInfo.m_randomness );
	xfer->xferShort( &m_breezeInfo.m_breezePeriod );
	xfer->xferShort( &m_breezeInfo.m_breezeVersion );

	// game difficulty
	xfer->xferUser( &m_gameDifficulty, sizeof( GameDifficulty ) );

	// freeze by script
	xfer->xferBool( &m_freezeByScript );
	
	// version 2
	if( version >= 2 )
	{

		// number of entries in named reveals
		UnsignedShort namedRevealCount = m_namedReveals.size();
		xfer->xferUnsignedShort( &namedRevealCount );

		// named reveal data
		if( xfer->getXferMode() == XFER_SAVE )
		{

			// iterate vector
			VecNamedRevealIt it;
			for( it = m_namedReveals.begin(); it != m_namedReveals.end(); ++it )
			{

				// name
				xfer->xferAsciiString( &it->m_revealName );

				// waypoint name
				xfer->xferAsciiString( &it->m_waypointName );

				// radius
				xfer->xferReal( &it->m_radiusToReveal );

				// player name
				xfer->xferAsciiString( &it->m_playerName );
				
			}  // end for, it

		}  // end if, save
		else
		{

			// the vector should be emtpy now
			if( m_namedReveals.empty() == FALSE )
			{

				DEBUG_CRASH(( "ScriptEngine::xfer - m_namedReveals should be empty but is not!\n" ));
				throw SC_INVALID_DATA;

			}  // end if

			// read all entries
			NamedReveal reveal;
			for( UnsignedShort i = 0; i < namedRevealCount; ++i )
			{
				
				// read name
				xfer->xferAsciiString( &reveal.m_revealName );

				// read waypoint name
				xfer->xferAsciiString( &reveal.m_waypointName );

				// read radius
				xfer->xferReal( &reveal.m_radiusToReveal );
				
				// read player name
				xfer->xferAsciiString( &reveal.m_playerName );

				// put on list
				m_namedReveals.push_back( reveal );

			}  // end for, i

		}  // end else, load

		// all object type lists size
		UnsignedShort allObjectTypesCount = m_allObjectTypeLists.size();
		xfer->xferUnsignedShort( &allObjectTypesCount );

		// all object type lists data
		if( xfer->getXferMode() == XFER_SAVE )
		{
			
			// iterate list
			AllObjectTypesIt it;
			ObjectTypes *objectTypes;
			for( it = m_allObjectTypeLists.begin(); it != m_allObjectTypeLists.end(); ++it )
			{

				// get object types from iterator
				objectTypes = *it;

				// save object types
				xfer->xferSnapshot( objectTypes );

			}  // end for, it

		}  // end if, save
		else
		{

			// sanity, the list should be empty now
			if( m_allObjectTypeLists.empty() == FALSE )
			{

				DEBUG_CRASH(( "ScriptEngine::xfer - m_allObjectTypeLists should be empty but is not!\n" ));
				throw SC_INVALID_DATA;

			}  // end if

			// read all data
			ObjectTypes *objectTypes;
			for( UnsignedShort i = 0; i < allObjectTypesCount; ++i )
			{

				// allocate a new object types
				objectTypes = newInstance( ObjectTypes );

				// xfer object types data
				xfer->xferSnapshot( objectTypes );

				// put on list
				m_allObjectTypeLists.push_back( objectTypes );

			}  // end for, i

		}  //  end else, load

	}  // end if, version 2

	if (version >= 3) {
		xfer->xferBool(&m_objectsShouldReceiveDifficultyBonus);
	} else {
		m_objectsShouldReceiveDifficultyBonus = TRUE;
	}

	if (version >= 4)
	{
		xfer->xferAsciiString(&m_currentTrackName);
	}

	if (version >= 5)
	{
		xfer->xferBool(&m_ChooseVictimAlwaysUsesNormal);
	}
	else
	{
		m_ChooseVictimAlwaysUsesNormal = false;
	}

	if( xfer->getXferMode() == XFER_LOAD ) {
		// We are doing a load.  If there is no fade active, do a black fade in to start.
		if (m_fade == FADE_NONE) {
			m_fade = FADE_MULTIPLY; //default to a fade in from black.
			m_curFadeFrame = 0;
			m_minFade = 1.0f;
			m_maxFade = 0.0f;	
			m_fadeFramesIncrease = 0;
			m_fadeFramesHold = 0;
			m_fadeFramesDecrease = FRAMES_TO_FADE_IN_AT_START;
			m_curFadeValue = 0.0f;
		}
	}
}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ScriptEngine::loadPostProcess( void )
{

	// Now that we've loaded everything, go through and set them all back in sync with what we
	// currently think they should be.
	TheScriptActions->doEnableOrDisableObjectDifficultyBonuses(m_objectsShouldReceiveDifficultyBonus);

	if (m_currentTrackName.isNotEmpty())
	{
		AudioEventRTS event(m_currentTrackName);
		event.setPlayerIndex(ThePlayerList->getLocalPlayer()->getPlayerIndex());
		TheAudio->addAudioEvent(&event);
	}

}  // end loadPostProcess

//#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
void ScriptEngine::debugVictory( void )
{
	ScriptAction *action = newInstance(ScriptAction)(ScriptAction::VICTORY);
	TheScriptActions->executeAction(action);
}
//#endif

Bool ScriptEngine::hasShownMPLocalDefeatWindow(void)
{
	return m_shownMPLocalDefeatWindow;
}

void ScriptEngine::markMPLocalDefeatWindowShown(void)
{
	m_shownMPLocalDefeatWindow = TRUE;
}

// ------------------------------------------------------------------------------------------------
/** Misc helper functions (ParticleEdit, etc) */
// ------------------------------------------------------------------------------------------------

void _appendMessage(const AsciiString& str, Bool isTrueMessage, Bool shouldPause)
{
	typedef void (*funcptr)(const char*);

	AsciiString msg;
	msg.format("%d ", TheGameLogic->getFrame());
	if (isTrueMessage) {
		msg.concat("Run script - ");
	} else {
		msg.concat("Run script false -");
	}
	msg.concat(str);

#ifdef INTENSE_DEBUG
	DEBUG_LOG(("-SCRIPT- %s\n", msg.str()));
#endif
	if (!st_DebugDLL) {
		return;
	}

	FARPROC proc;
	if (shouldPause) {
		proc = GetProcAddress(st_DebugDLL, "AppendMessageAndPause");
	} else {
		proc = GetProcAddress(st_DebugDLL, "AppendMessage");
	}
	if (!proc) {
		return;
	}

	((funcptr)proc)(msg.str());
}

void _adjustVariable(const AsciiString& str, Int value, Bool shouldPause)
{
	typedef void (*funcptr)(const char*, const char*);
	if (!st_DebugDLL) {
		return;
	}

	FARPROC proc;
	if (shouldPause) {
		proc = GetProcAddress(st_DebugDLL, "AdjustVariableAndPause");
	} else {
		proc = GetProcAddress(st_DebugDLL, "AdjustVariable");
	}

	if (!proc) {
		return;
	}

	char buff[12];	// for sprintf
	sprintf(buff, "%d", value);

	((funcptr)proc)(str.str(), buff);
}

void _updateFrameNumber( void )
{
	if (TheScriptEngine->isTimeFast()) return;
	typedef void (*funcptr)(int);
	if (!st_DebugDLL) {
		return;
	}

	FARPROC proc;
	proc = GetProcAddress(st_DebugDLL, "SetFrameNumber");
	if (!proc) {
		return;
	}

	UnsignedInt frameNum = TheGameLogic->getFrame();

	((funcptr)proc)(frameNum);
}

void _appendAllParticleSystems( void )
{
	typedef void (*funcptr)(const char*);
	if (!st_ParticleDLL) {
		return;
	}
	FARPROC proc;

	proc = GetProcAddress(st_ParticleDLL, "RemoveAllParticleSystems");
	if (proc) {
		proc();
	} else {
		return;
	}

	proc = GetProcAddress(st_ParticleDLL, "AppendParticleSystem");
	if (!proc) {
		return;
	}

	// Copy just the names for the list of particle system templates
	ParticleSystemManager::TemplateMap::iterator begin(TheParticleSystemManager->beginParticleSystemTemplate());
	ParticleSystemManager::TemplateMap::iterator end(TheParticleSystemManager->endParticleSystemTemplate());
	for (; begin != end; ++begin) {
		((funcptr)proc)((*begin).first.str());
	}
}

// all ThingTemplates can be thrown with a particle system, so...
void _appendAllThingTemplates( void )
{
	typedef void (*funcptr)(const char*);
	if (!st_ParticleDLL) {
		return;
	}
	FARPROC proc;

	proc = GetProcAddress(st_ParticleDLL, "RemoveAllThingTemplates");
	if (proc) {
		proc();
	} else {
		return;
	}

	proc = GetProcAddress(st_ParticleDLL, "AppendThingTemplate");
	if (!proc) {
		return;
	}

	const ThingTemplate *pTemplate = TheThingFactory->firstTemplate();
	while (pTemplate) {
		((funcptr)proc)(pTemplate->getName().str());
		pTemplate = pTemplate->friend_getNextTemplate();
	}

}


void _addUpdatedParticleSystem( AsciiString particleSystemName )
{
	typedef void (*funcptr)(const char*);
	typedef void (*funcptr2)(ParticleSystemTemplate*);
	if (!st_ParticleDLL) {
		return;
	}

	if (TheParticleSystemManager->findTemplate(particleSystemName)) {
		return;
	}
	
	FARPROC proc, proc2;
	proc = GetProcAddress(st_ParticleDLL, "AppendParticleSystem");
	if (!proc) {
		return;
	}

	proc2 = GetProcAddress(st_ParticleDLL, "UpdateSystemUseParameters");
	if (!proc2) {
		return;
	}


	ParticleSystemTemplate *pTemplate = TheParticleSystemManager->newTemplate(particleSystemName);
	if (!pTemplate) {
		return;
	}

	((funcptr)proc)(pTemplate->getName().str());
	((funcptr2)proc2)(pTemplate);
}

AsciiString _getParticleSystemName( void )
{
	typedef void (*funcptr)(char*);

	if (!st_ParticleDLL) {
		return AsciiString::TheEmptyString;
	}

	FARPROC proc;
	proc = GetProcAddress(st_ParticleDLL, "GetSelectedParticleSystemName");
	if (!proc) {
		return AsciiString::TheEmptyString;
	}

	static char buff[1024];

	((funcptr) proc)(buff);

	return AsciiString(buff);
}

void _updatePanelParameters( ParticleSystemTemplate *particleTemplate )
{
	typedef void (*funcptr)(ParticleSystemTemplate*);
	
	if (!st_ParticleDLL) {
		return;
	}

	FARPROC proc;	
	proc = GetProcAddress(st_ParticleDLL, "UpdateCurrentParticleSystem");
	if (!proc) {
		return;
	}

	((funcptr) proc)(particleTemplate);
}

void _updateAsciiStringParmsToSystem( ParticleSystemTemplate *particleTemplate )
{
	typedef void (*funcptr)(int, char*, ParticleSystemTemplate **);

	if (!st_ParticleDLL || !particleTemplate) {
		return;
	}

	FARPROC proc;	
	proc = GetProcAddress(st_ParticleDLL, "GetSelectedParticleAsciiStringParm");

	if (!proc) {
		return;
	}
	
	char buff[ARBITRARY_BUFF_SIZE];
	ParticleSystemTemplate* otherTemp;

	((funcptr) proc)(0, buff, &otherTemp); // PARM_ParticleTypeName
	if (otherTemp == particleTemplate) {
		particleTemplate->m_particleTypeName.set(buff);	
	}

	
	((funcptr) proc)(1, buff, &otherTemp); // PARM_SlaveSystemName
	if (otherTemp == particleTemplate) {
		particleTemplate->m_slaveSystemName.set(buff);
	}

	((funcptr) proc)(2, buff, &otherTemp); // PARM_AttachedSystemName
	if (otherTemp == particleTemplate) {
		particleTemplate->m_attachedSystemName.set(buff);	
	}
}

extern void _updateAsciiStringParmsFromSystem( ParticleSystemTemplate *particleTemplate )
{
	typedef void (*funcptr)(int, const char*, ParticleSystemTemplate**);

	if (!st_ParticleDLL || !particleTemplate) {
		return;
	}

	FARPROC proc;	
	proc = GetProcAddress(st_ParticleDLL, "UpdateParticleAsciiStringParm");

	if (!proc) {
		return;
	}

	((funcptr) proc)(0, particleTemplate->m_particleTypeName.str(), NULL);	// PARM_ParticleTypeName
	((funcptr) proc)(1, particleTemplate->m_slaveSystemName.str(), NULL);	// PARM_SlaveSystemName
	((funcptr) proc)(2, particleTemplate->m_attachedSystemName.str(), NULL);	// PARM_AttachedSystemName

}

#define BACKUP_FILE_NAME	"Data\\INI\\ParticleSystem"
#define BACKUP_EXT				"BAK"
static void _writeOutINI( void )
{
	// currently, this uses NO intelligence. It blindly iterates through all of the 
	// particle system templates and writes out every field that it thinks it should.
	const int maxFileLength = 128;
	char buff[maxFileLength];
	
	File *saveFile = NULL;
	
	int i = 0;
	do {
		if (saveFile) {
				saveFile->close();
				saveFile = NULL;
		}
		sprintf(buff, "%s%d.%s", BACKUP_FILE_NAME, i, BACKUP_EXT);
		saveFile = TheFileSystem->openFile(buff, File::READ | File::TEXT);
		++i;
	} while (saveFile);

	saveFile = TheFileSystem->openFile(buff, File::WRITE | File::TEXT);
	if (!saveFile) {
		return;
	}
	
	// save the old file
	File *oldINI = TheFileSystem->openFile("Data\\INI\\ParticleSystem.ini", File::READ | File::TEXT);
	
	if (oldINI) {
		char singleChar;
		while (oldINI->position() != oldINI->size()) {
			oldINI->read(&singleChar, 1);
			saveFile->write(&singleChar, 1);
		}
		oldINI->close();
		oldINI = NULL;
		saveFile->close();
		saveFile = NULL;

	}


	// open the .ini file for writing, truncate.
	File *newINI = TheFileSystem->openFile("Data\\INI\\ParticleSystem.ini", File::WRITE | File::TEXT);

	if (!newINI) {
		DEBUG_CRASH(("Unable to open ParticleSystem.ini. Is it write protected?"));
		return;
	}

	ParticleSystemManager::TemplateMap::iterator begin(TheParticleSystemManager->beginParticleSystemTemplate());
	ParticleSystemManager::TemplateMap::iterator end(TheParticleSystemManager->endParticleSystemTemplate());

	for (; begin != end; ++begin) {
		_writeSingleParticleSystem(newINI, (*begin).second);
	}

	newINI->close();
	newINI = NULL;
}


static const std::string HEADER =					"ParticleSystem";
static const std::string SEP_SPACE =			" ";
static const std::string SEP_HEAD	=				"  ";
static const std::string SEP_EOL =				"\n";
static const std::string SEP_TAB =				"\t";
static const std::string STR_TRUE	=				"Yes";
static const std::string STR_FALSE =			"No";
static const std::string EQ_WITH_SPACES	=	" = ";
static const std::string STR_R = 					"R:";
static const std::string STR_G =					"G:";
static const std::string STR_B =					"B:";
static const std::string STR_X = 					"X:";
static const std::string STR_Y =					"Y:";
static const std::string STR_Z =					"Z:";

static const std::string STR_END =				"End";

static const std::string F_PRIORITY =			"Priority";

static const std::string F_ISONESHOT =		"IsOneShot";
static const std::string F_SHADER =				"Shader";
static const std::string F_TYPE =					"Type";
static const std::string F_PARTICLENAME =	"ParticleName";
static const std::string F_ANGLEX =				"AngleX";
static const std::string F_ANGLEY =				"AngleY";
static const std::string F_ANGLEZ	=				"AngleZ";
static const std::string F_ANGLERATEX	=		"AngularRateX";
static const std::string F_ANGLERATEY	=		"AngularRateY";
static const std::string F_ANGLERATEZ	=		"AngularRateZ";
static const std::string F_ANGLEDAMP =		"AngularDamping";
static const std::string F_VELOCITYDAMP	=	"VelocityDamping";
static const std::string F_GRAVITY =			"Gravity";
static const std::string F_SLAVESYSTEM =	"SlaveSystem";
static const std::string F_SLAVEPOS =			"SlavePosOffset";
static const std::string F_ATTACHED =			"PerParticleAttachedSystem";
static const std::string F_LIFETIME =			"Lifetime";
static const std::string F_SYSLIFETIME =	"SystemLifetime";
static const std::string F_SIZE =					"Size";
static const std::string F_STARTSIZERATE ="StartSizeRate";
static const std::string F_SIZERATE =			"SizeRate";
static const std::string F_SIZERATEDAMP =	"SizeRateDamping";

static const std::string F_ALPHA1 =				"Alpha1";
static const std::string F_ALPHA2 =				"Alpha2";
static const std::string F_ALPHA3 =				"Alpha3";
static const std::string F_ALPHA4 =				"Alpha4";
static const std::string F_ALPHA5 =				"Alpha5";
static const std::string F_ALPHA6 =				"Alpha6";
static const std::string F_ALPHA7 =				"Alpha7";
static const std::string F_ALPHA8 =				"Alpha8";

static const std::string F_COLOR1 =				"Color1";
static const std::string F_COLOR2 =				"Color2";
static const std::string F_COLOR3 =				"Color3";
static const std::string F_COLOR4 =				"Color4";
static const std::string F_COLOR5 =				"Color5";
static const std::string F_COLOR6 =				"Color6";
static const std::string F_COLOR7 =				"Color7";
static const std::string F_COLOR8 =				"Color8";
static const std::string F_COLORSCALE =		"ColorScale";

static const std::string F_BURSTDELAY =		"BurstDelay";
static const std::string F_BURSTCOUNT =		"BurstCount";
static const std::string F_INITIALDELAY =	"InitialDelay";
static const std::string F_DRIFTVELOCITY ="DriftVelocity";

static const std::string F_VELOCITYTYPE =	"VelocityType";

static const std::string F_VELORTHOX =		"VelOrthoX";
static const std::string F_VELORTHOY =		"VelOrthoY";
static const std::string F_VELORTHOZ =		"VelOrthoZ";

static const std::string F_VELSPHERE =		"VelSpherical";
static const std::string F_HEMISPHERE	= 	"VelHemispherical";

static const std::string F_VELCYLRAD =		"VelCylindricalRadial";
static const std::string F_VELCYLNOR =		"VelCylindricalNormal";

static const std::string F_VELOUTWARD =		"VelOutward";
static const std::string F_VELOUTOTHER =	"VelOutwardOther";

static const std::string F_VOLUMETYPE = 	"VolumeType";

static const std::string F_VOLLINESTART =	"VolLineStart";
static const std::string F_VOLLINEEND =		"VolLineEnd";

static const std::string F_VOLBOXHALF	=		"VolBoxHalfSize";
static const std::string F_VOLSPHERERAD	=	"VolSphereRadius";
static const std::string F_VOLCYLRAD =		"VolCylinderRadius";
static const std::string F_VOLCYLLEN =		"VolCylinderLength";
static const std::string F_ISHOLLOW =			"IsHollow";
static const std::string F_ISXYPLANAR =		"IsGroundAligned";
static const std::string F_ISEMITABOVEGROUNDONLY 
																			=		"IsEmitAboveGroundOnly";
static const std::string F_ISPARTICLEUPTOWARDSEMITTER 
																			=		"IsParticleUpTowardsEmitter";

static const std::string F_WINDMOTION = "WindMotion";
static const std::string F_WINDANGLECHANGEMIN = "WindAngleChangeMin";
static const std::string F_WINDANGLECHANGEMAX = "WindAngleChangeMax";
static const std::string F_WINDPINGPONGSTARTANGLEMIN = "WindPingPongStartAngleMin";
static const std::string F_WINDPINGPONGSTARTANGLEMAX = "WindPingPongStartAngleMax";
static const std::string F_WINDPINGPONGENDANGLEMIN = "WindPingPongEndAngleMin";
static const std::string F_WINDPINGPONGENDANGLEMAX = "WindPingPongEndAngleMax";

void _writeSingleParticleSystem( File *out, ParticleSystemTemplate *templ )
{
	if (!templ || !out || templ->getName().isEmpty()) {
		// sanity
		return;
	}

	static char buff1[ARBITRARY_BUFF_SIZE];
	static char buff2[ARBITRARY_BUFF_SIZE];
	static char buff3[ARBITRARY_BUFF_SIZE];
	static char buff4[ARBITRARY_BUFF_SIZE];
	

	// the .append looks REALLY ugly, but this code was written with streams in mind, and so 
	// these were all originally << (feed-operator for streams)
	// I might come back and re-write this later, if there are enough complaints. ;-) jkmcd
	// in the meantime, move along...
	std::string thisEntry = "";
	thisEntry.append(HEADER).append(SEP_SPACE).append(templ->getName().str()).append(SEP_EOL);
	thisEntry.append(SEP_HEAD).append(F_PRIORITY).append(EQ_WITH_SPACES).append(ParticlePriorityNames[templ->m_priority]).append(SEP_EOL);
	thisEntry.append(SEP_HEAD).append(F_ISONESHOT).append(EQ_WITH_SPACES).append((templ->m_isOneShot ? STR_TRUE : STR_FALSE)).append(SEP_EOL);
	thisEntry.append(SEP_HEAD).append(F_SHADER).append(EQ_WITH_SPACES).append(ParticleShaderTypeNames[templ->m_shaderType]).append(SEP_EOL);
	thisEntry.append(SEP_HEAD).append(F_TYPE).append(EQ_WITH_SPACES).append(ParticleTypeNames[templ->m_particleType]).append(SEP_EOL);
	thisEntry.append(SEP_HEAD).append(F_PARTICLENAME).append(EQ_WITH_SPACES).append(templ->m_particleTypeName.str()).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_angleX.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_angleX.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_ANGLEX).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_angleY.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_angleY.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_ANGLEY).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_angleZ.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_angleZ.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_ANGLEZ).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);

	sprintf(buff1, FORMAT_STRING, templ->m_angularRateX.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_angularRateX.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_ANGLERATEX).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_angularRateY.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_angularRateY.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_ANGLERATEY).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_angularRateZ.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_angularRateZ.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_ANGLERATEZ).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);

	sprintf(buff1, FORMAT_STRING, templ->m_angularDamping.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_angularDamping.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_ANGLEDAMP).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);

	sprintf(buff1, FORMAT_STRING, templ->m_velDamping.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_velDamping.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_VELOCITYDAMP).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_gravity);
	thisEntry.append(SEP_HEAD).append(F_GRAVITY).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);
	if (!templ->m_slaveSystemName.isEmpty()) {
		thisEntry.append(SEP_HEAD).append(F_SLAVESYSTEM).append(EQ_WITH_SPACES).append(templ->m_slaveSystemName.str()).append(SEP_EOL);
		sprintf(buff1, FORMAT_STRING_LEADING_STRING, STR_X.c_str(), templ->m_slavePosOffset.x);
		sprintf(buff2, FORMAT_STRING_LEADING_STRING, STR_Y.c_str(), templ->m_slavePosOffset.y);
		sprintf(buff3, FORMAT_STRING_LEADING_STRING, STR_Z.c_str(), templ->m_slavePosOffset.z);
		thisEntry.append(SEP_HEAD).append(F_SLAVEPOS).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);
	}

	if (!templ->m_attachedSystemName.isEmpty()) {
		thisEntry.append(SEP_HEAD).append(F_ATTACHED).append(EQ_WITH_SPACES).append(templ->m_attachedSystemName.str()).append(SEP_EOL);
	}

	sprintf(buff1, FORMAT_STRING, templ->m_lifetime.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_lifetime.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_LIFETIME).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);

	sprintf(buff1, "%d", templ->m_systemLifetime);
	thisEntry.append(SEP_HEAD).append(F_SYSLIFETIME).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);

	sprintf(buff1, FORMAT_STRING, templ->m_startSize.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_startSize.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_SIZE).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_startSizeRate.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_startSizeRate.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_STARTSIZERATE).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_sizeRate.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_sizeRate.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_SIZERATE).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_sizeRateDamping.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_sizeRateDamping.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_SIZERATEDAMP).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);

	sprintf(buff1, FORMAT_STRING, templ->m_alphaKey[0].var.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_alphaKey[0].var.getMaximumValue());
	sprintf(buff3, "%d", templ->m_alphaKey[0].frame);
	thisEntry.append(SEP_HEAD).append(F_ALPHA1).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);

	sprintf(buff1, FORMAT_STRING, templ->m_alphaKey[1].var.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_alphaKey[1].var.getMaximumValue());
	sprintf(buff3, "%d", templ->m_alphaKey[1].frame);
	thisEntry.append(SEP_HEAD).append(F_ALPHA2).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);

	sprintf(buff1, FORMAT_STRING, templ->m_alphaKey[2].var.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_alphaKey[2].var.getMaximumValue());
	sprintf(buff3, "%d", templ->m_alphaKey[2].frame);
	thisEntry.append(SEP_HEAD).append(F_ALPHA3).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_alphaKey[3].var.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_alphaKey[3].var.getMaximumValue());
	sprintf(buff3, "%d", templ->m_alphaKey[3].frame);
	thisEntry.append(SEP_HEAD).append(F_ALPHA4).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_alphaKey[4].var.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_alphaKey[4].var.getMaximumValue());
	sprintf(buff3, "%d", templ->m_alphaKey[4].frame);
	thisEntry.append(SEP_HEAD).append(F_ALPHA5).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_alphaKey[5].var.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_alphaKey[5].var.getMaximumValue());
	sprintf(buff3, "%d", templ->m_alphaKey[5].frame);
	thisEntry.append(SEP_HEAD).append(F_ALPHA6).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_alphaKey[6].var.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_alphaKey[6].var.getMaximumValue());
	sprintf(buff3, "%d", templ->m_alphaKey[6].frame);
	thisEntry.append(SEP_HEAD).append(F_ALPHA7).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_alphaKey[7].var.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_alphaKey[7].var.getMaximumValue());
	sprintf(buff3, "%d", templ->m_alphaKey[7].frame);
	thisEntry.append(SEP_HEAD).append(F_ALPHA8).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);

	sprintf(buff1, "%s%d", STR_R.c_str(), REAL_TO_INT(templ->m_colorKey[0].color.red * 255 + 0.5));
	sprintf(buff2, "%s%d", STR_G.c_str(), REAL_TO_INT(templ->m_colorKey[0].color.green * 255 + 0.5));
	sprintf(buff3, "%s%d", STR_B.c_str(), REAL_TO_INT(templ->m_colorKey[0].color.blue * 255 + 0.5));
	sprintf(buff4, "%d", templ->m_colorKey[0].frame);
	thisEntry.append(SEP_HEAD).append(F_COLOR1).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_SPACE).append(buff4).append(SEP_EOL);

	sprintf(buff1, "%s%d", STR_R.c_str(), REAL_TO_INT(templ->m_colorKey[1].color.red * 255 + 0.5));
	sprintf(buff2, "%s%d", STR_G.c_str(), REAL_TO_INT(templ->m_colorKey[1].color.green * 255 + 0.5));
	sprintf(buff3, "%s%d", STR_B.c_str(), REAL_TO_INT(templ->m_colorKey[1].color.blue * 255 + 0.5));
	sprintf(buff4, "%d", templ->m_colorKey[1].frame);
	thisEntry.append(SEP_HEAD).append(F_COLOR2).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_SPACE).append(buff4).append(SEP_EOL);

	sprintf(buff1, "%s%d", STR_R.c_str(), REAL_TO_INT(templ->m_colorKey[2].color.red * 255 + 0.5));
	sprintf(buff2, "%s%d", STR_G.c_str(), REAL_TO_INT(templ->m_colorKey[2].color.green * 255 + 0.5));
	sprintf(buff3, "%s%d", STR_B.c_str(), REAL_TO_INT(templ->m_colorKey[2].color.blue * 255 + 0.5));
	sprintf(buff4, "%d", templ->m_colorKey[2].frame);
	thisEntry.append(SEP_HEAD).append(F_COLOR3).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_SPACE).append(buff4).append(SEP_EOL);
	
	sprintf(buff1, "%s%d", STR_R.c_str(), REAL_TO_INT(templ->m_colorKey[3].color.red * 255 + 0.5));
	sprintf(buff2, "%s%d", STR_G.c_str(), REAL_TO_INT(templ->m_colorKey[3].color.green * 255 + 0.5));
	sprintf(buff3, "%s%d", STR_B.c_str(), REAL_TO_INT(templ->m_colorKey[3].color.blue * 255 + 0.5));
	sprintf(buff4, "%d", templ->m_colorKey[3].frame);
	thisEntry.append(SEP_HEAD).append(F_COLOR4).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_SPACE).append(buff4).append(SEP_EOL);
	
	sprintf(buff1, "%s%d", STR_R.c_str(), REAL_TO_INT(templ->m_colorKey[4].color.red * 255 + 0.5));
	sprintf(buff2, "%s%d", STR_G.c_str(), REAL_TO_INT(templ->m_colorKey[4].color.green * 255 + 0.5));
	sprintf(buff3, "%s%d", STR_B.c_str(), REAL_TO_INT(templ->m_colorKey[4].color.blue * 255 + 0.5));
	sprintf(buff4, "%d", templ->m_colorKey[4].frame);
	thisEntry.append(SEP_HEAD).append(F_COLOR5).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_SPACE).append(buff4).append(SEP_EOL);
	
	sprintf(buff1, "%s%d", STR_R.c_str(), REAL_TO_INT(templ->m_colorKey[5].color.red * 255 + 0.5));
	sprintf(buff2, "%s%d", STR_G.c_str(), REAL_TO_INT(templ->m_colorKey[5].color.green * 255 + 0.5));
	sprintf(buff3, "%s%d", STR_B.c_str(), REAL_TO_INT(templ->m_colorKey[5].color.blue * 255 + 0.5));
	sprintf(buff4, "%d", templ->m_colorKey[5].frame);
	thisEntry.append(SEP_HEAD).append(F_COLOR6).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_SPACE).append(buff4).append(SEP_EOL);
	
	sprintf(buff1, "%s%d", STR_R.c_str(), REAL_TO_INT(templ->m_colorKey[6].color.red * 255 + 0.5));
	sprintf(buff2, "%s%d", STR_G.c_str(), REAL_TO_INT(templ->m_colorKey[6].color.green * 255 + 0.5));
	sprintf(buff3, "%s%d", STR_B.c_str(), REAL_TO_INT(templ->m_colorKey[6].color.blue * 255 + 0.5));
	sprintf(buff4, "%d", templ->m_colorKey[6].frame);
	thisEntry.append(SEP_HEAD).append(F_COLOR7).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_SPACE).append(buff4).append(SEP_EOL);
	
	sprintf(buff1, "%s%d", STR_R.c_str(), REAL_TO_INT(templ->m_colorKey[7].color.red * 255 + 0.5));
	sprintf(buff2, "%s%d", STR_G.c_str(), REAL_TO_INT(templ->m_colorKey[7].color.green * 255 + 0.5));
	sprintf(buff3, "%s%d", STR_B.c_str(), REAL_TO_INT(templ->m_colorKey[7].color.blue * 255 + 0.5));
	sprintf(buff4, "%d", templ->m_colorKey[7].frame);
	thisEntry.append(SEP_HEAD).append(F_COLOR8).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_SPACE).append(buff4).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_colorScale.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_colorScale.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_COLORSCALE).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_burstDelay.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_burstDelay.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_BURSTDELAY).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_burstCount.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_burstCount.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_BURSTCOUNT).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING, templ->m_initialDelay.getMinimumValue());
	sprintf(buff2, FORMAT_STRING, templ->m_initialDelay.getMaximumValue());
	thisEntry.append(SEP_HEAD).append(F_INITIALDELAY).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	
	sprintf(buff1, FORMAT_STRING_LEADING_STRING, STR_X.c_str(), templ->m_driftVelocity.x);
	sprintf(buff2, FORMAT_STRING_LEADING_STRING, STR_Y.c_str(), templ->m_driftVelocity.y);
	sprintf(buff3, FORMAT_STRING_LEADING_STRING, STR_Z.c_str(), templ->m_driftVelocity.z);
	thisEntry.append(SEP_HEAD).append(F_DRIFTVELOCITY).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);

	thisEntry.append(SEP_HEAD).append(F_VELOCITYTYPE).append(EQ_WITH_SPACES).append(EmissionVelocityTypeNames[templ->m_emissionVelocityType]).append(SEP_EOL);

	if (templ->m_emissionVelocityType == ParticleSystemInfo::ORTHO) {

		sprintf(buff1, FORMAT_STRING, templ->m_emissionVelocity.ortho.x.getMinimumValue());
		sprintf(buff2, FORMAT_STRING, templ->m_emissionVelocity.ortho.x.getMaximumValue());
		thisEntry.append(SEP_HEAD).append(F_VELORTHOX).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
		
		sprintf(buff1, FORMAT_STRING, templ->m_emissionVelocity.ortho.y.getMinimumValue());
		sprintf(buff2, FORMAT_STRING, templ->m_emissionVelocity.ortho.y.getMaximumValue());
		thisEntry.append(SEP_HEAD).append(F_VELORTHOY).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
		
		sprintf(buff1, FORMAT_STRING, templ->m_emissionVelocity.ortho.z.getMinimumValue());
		sprintf(buff2, FORMAT_STRING, templ->m_emissionVelocity.ortho.z.getMaximumValue());
		thisEntry.append(SEP_HEAD).append(F_VELORTHOZ).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	} else if (templ->m_emissionVelocityType == ParticleSystemInfo::SPHERICAL) {
		sprintf(buff1, FORMAT_STRING, templ->m_emissionVelocity.spherical.speed.getMinimumValue());
		sprintf(buff2, FORMAT_STRING, templ->m_emissionVelocity.spherical.speed.getMaximumValue());
		thisEntry.append(SEP_HEAD).append(F_VELSPHERE).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);

	} else if (templ->m_emissionVelocityType == ParticleSystemInfo::HEMISPHERICAL) {
		sprintf(buff1, FORMAT_STRING, templ->m_emissionVelocity.hemispherical.speed.getMinimumValue());
		sprintf(buff2, FORMAT_STRING, templ->m_emissionVelocity.hemispherical.speed.getMaximumValue());
		thisEntry.append(SEP_HEAD).append(F_HEMISPHERE).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);

	} else if (templ->m_emissionVelocityType == ParticleSystemInfo::CYLINDRICAL) {
		sprintf(buff1, FORMAT_STRING, templ->m_emissionVelocity.cylindrical.radial.getMinimumValue());
		sprintf(buff2, FORMAT_STRING, templ->m_emissionVelocity.cylindrical.radial.getMaximumValue());
		thisEntry.append(SEP_HEAD).append(F_VELCYLRAD).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);

		sprintf(buff1, FORMAT_STRING, templ->m_emissionVelocity.cylindrical.normal.getMinimumValue());
		sprintf(buff2, FORMAT_STRING, templ->m_emissionVelocity.cylindrical.normal.getMaximumValue());
		thisEntry.append(SEP_HEAD).append(F_VELCYLNOR).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);

	} else if (templ->m_emissionVelocityType == ParticleSystemInfo::OUTWARD) {
		sprintf(buff1, FORMAT_STRING, templ->m_emissionVelocity.outward.speed.getMinimumValue());
		sprintf(buff2, FORMAT_STRING, templ->m_emissionVelocity.outward.speed.getMaximumValue());
		thisEntry.append(SEP_HEAD).append(F_VELOUTWARD).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);

		sprintf(buff1, FORMAT_STRING, templ->m_emissionVelocity.outward.otherSpeed.getMinimumValue());
		sprintf(buff2, FORMAT_STRING, templ->m_emissionVelocity.outward.otherSpeed.getMaximumValue());
		thisEntry.append(SEP_HEAD).append(F_VELOUTOTHER).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_EOL);
	}	

	thisEntry.append(SEP_HEAD).append(F_VOLUMETYPE).append(EQ_WITH_SPACES).append(EmissionVolumeTypeNames[templ->m_emissionVolumeType]).append(SEP_EOL);

	if (templ->m_emissionVolumeType == ParticleSystemInfo::POINT) {
		// nothing to output here for lines
	} else if (templ->m_emissionVolumeType == ParticleSystemInfo::LINE) {
		sprintf(buff1, FORMAT_STRING_LEADING_STRING, STR_X.c_str(), templ->m_emissionVolume.line.start.x);
		sprintf(buff2, FORMAT_STRING_LEADING_STRING, STR_Y.c_str(), templ->m_emissionVolume.line.start.y);
		sprintf(buff3, FORMAT_STRING_LEADING_STRING, STR_Z.c_str(), templ->m_emissionVolume.line.start.z);
		thisEntry.append(SEP_HEAD).append(F_VOLLINESTART).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);

		sprintf(buff1, FORMAT_STRING_LEADING_STRING, STR_X.c_str(), templ->m_emissionVolume.line.end.x);
		sprintf(buff2, FORMAT_STRING_LEADING_STRING, STR_Y.c_str(), templ->m_emissionVolume.line.end.y);
		sprintf(buff3, FORMAT_STRING_LEADING_STRING, STR_Z.c_str(), templ->m_emissionVolume.line.end.z);
		thisEntry.append(SEP_HEAD).append(F_VOLLINEEND).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);

	} else if (templ->m_emissionVolumeType == ParticleSystemInfo::BOX) {
		sprintf(buff1, FORMAT_STRING_LEADING_STRING, STR_X.c_str(), templ->m_emissionVolume.box.halfSize.x);
		sprintf(buff2, FORMAT_STRING_LEADING_STRING, STR_Y.c_str(), templ->m_emissionVolume.box.halfSize.y);
		sprintf(buff3, FORMAT_STRING_LEADING_STRING, STR_Z.c_str(), templ->m_emissionVolume.box.halfSize.z);
		thisEntry.append(SEP_HEAD).append(F_VOLBOXHALF).append(EQ_WITH_SPACES).append(buff1).append(SEP_SPACE).append(buff2).append(SEP_SPACE).append(buff3).append(SEP_EOL);

	} else if (templ->m_emissionVolumeType == ParticleSystemInfo::SPHERE) {
		sprintf(buff1, FORMAT_STRING, templ->m_emissionVolume.sphere.radius);
		thisEntry.append(SEP_HEAD).append(F_VOLSPHERERAD).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);

	} else if (templ->m_emissionVolumeType == ParticleSystemInfo::CYLINDER) {
		sprintf(buff1, FORMAT_STRING, templ->m_emissionVolume.cylinder.radius);
		thisEntry.append(SEP_HEAD).append(F_VOLCYLRAD).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);

		sprintf(buff1, FORMAT_STRING, templ->m_emissionVolume.cylinder.length);
		thisEntry.append(SEP_HEAD).append(F_VOLCYLLEN).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);
	}

	thisEntry.append(SEP_HEAD).append(F_ISHOLLOW).append(EQ_WITH_SPACES).append((templ->m_isEmissionVolumeHollow ? STR_TRUE : STR_FALSE)).append(SEP_EOL);
	thisEntry.append(SEP_HEAD).append(F_ISXYPLANAR).append(EQ_WITH_SPACES).append((templ->m_isGroundAligned ? STR_TRUE : STR_FALSE)).append(SEP_EOL);
	thisEntry.append(SEP_HEAD).append(F_ISEMITABOVEGROUNDONLY).append(EQ_WITH_SPACES).append((templ->m_isEmitAboveGroundOnly ? STR_TRUE : STR_FALSE)).append(SEP_EOL);
	thisEntry.append(SEP_HEAD).append(F_ISPARTICLEUPTOWARDSEMITTER).append(EQ_WITH_SPACES).append((templ->m_isParticleUpTowardsEmitter ? STR_TRUE : STR_FALSE)).append(SEP_EOL);

	// wind angle and stuff
	thisEntry.append(SEP_HEAD).append(F_WINDMOTION).append(EQ_WITH_SPACES).append(WindMotionNames[templ->m_windMotion]).append(SEP_EOL);

	sprintf( buff1, "%f", templ->m_windAngleChangeMin );
	thisEntry.append(SEP_HEAD).append(F_WINDANGLECHANGEMIN).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);
	sprintf( buff1, "%f", templ->m_windAngleChangeMax );
	thisEntry.append(SEP_HEAD).append(F_WINDANGLECHANGEMAX).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);

	sprintf( buff1, "%f", templ->m_windMotionStartAngleMin );
	thisEntry.append(SEP_HEAD).append(F_WINDPINGPONGSTARTANGLEMIN).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);
	sprintf( buff1, "%f", templ->m_windMotionStartAngleMax );
	thisEntry.append(SEP_HEAD).append(F_WINDPINGPONGSTARTANGLEMAX).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);

	sprintf( buff1, "%f", templ->m_windMotionEndAngleMin );
	thisEntry.append(SEP_HEAD).append(F_WINDPINGPONGENDANGLEMIN).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);
	sprintf( buff1, "%f", templ->m_windMotionEndAngleMax );
	thisEntry.append(SEP_HEAD).append(F_WINDPINGPONGENDANGLEMAX).append(EQ_WITH_SPACES).append(buff1).append(SEP_EOL);

	thisEntry.append(STR_END).append(SEP_EOL).append(SEP_EOL);

//	fwrite(thisEntry.c_str(), thisEntry.size(), 1, out);
	out->write(thisEntry.c_str(), thisEntry.size());
}

static int _getEditorBehavior( void )
{
	typedef int (*funcptr)( void );

	if (!st_ParticleDLL) {
		return 0x00;
	}

	FARPROC proc;	
	proc = GetProcAddress(st_ParticleDLL, "NextParticleEditorBehavior");

	if (!proc) {
		return 0x00;
	}

	return ((funcptr)proc)();
}

static void _updateAndSetCurrentSystem( void )
{
	AsciiString particleSystemName = _getParticleSystemName();
	_addUpdatedParticleSystem(particleSystemName);
	ParticleSystemTemplate *pTemp = const_cast<ParticleSystemTemplate*>(TheParticleSystemManager->findTemplate(particleSystemName));
	if (pTemp) {
		_updateAsciiStringParmsToSystem(pTemp);
		_updateAsciiStringParmsFromSystem(pTemp);
		_updatePanelParameters(pTemp);

		if( st_particleSystemNeedsStopping ) 
		{
			st_particleSystem->stop();
			st_particleSystem->destroy();
			st_particleSystemNeedsStopping = FALSE;
		}
		st_particleSystem = TheParticleSystemManager->createParticleSystem(pTemp);
		if( st_particleSystem ) 
		{
			if( st_particleSystem->isSystemForever() )
				st_particleSystemNeedsStopping = TRUE;// Only infinite lifetime systems need to be stopped.
			// You can't stop others, because you can't know if they have deleted themselves.  That used
			// to be a tiny memory overwrite, now it is a crash since destroy() now has a function call.

			ParticleSystemTemplate *parentTemp = TheParticleSystemManager->findParentTemplate(pTemp->getName(), 0);
			if (parentTemp) {
				ParticleSystem *parentSystem = NULL;
				parentSystem = TheParticleSystemManager->createParticleSystem(parentTemp);

				if (parentSystem) {
					ParticleSystem::mergeRelatedParticleSystems(parentSystem, st_particleSystem, true);
					parentSystem->stop();
					parentSystem->destroy();
				}
			}

			Coord3D pos;
			pos.x = pos.y = 50 * MAP_XY_FACTOR;
			pos.z = TheTerrainLogic->getGroundHeight(pos.x, pos.y) + 5 * MAP_XY_FACTOR;
			st_particleSystem->setPosition(&pos);
		}
	}
}

static void _reloadParticleSystemFromINI( AsciiString particleSystemName )
{
	if (!st_ParticleDLL || particleSystemName.isEmpty()) {
		return;
	}

	// Here's what we're doing
	// Find the entry in Data\\INI\\ParticleSystem.ini
	// put that whole entry into a temp file
	// force the INI to reload with INI_LOAD_OVERWRITE
	// force the particle system editor to recognize that the system has changed without its consent.

	static char linebuff[INI_MAX_CHARS_PER_LINE + 1];
	linebuff[0] = 0;

	// save the old file
	File *iniFile = TheFileSystem->openFile("Data\\INI\\ParticleSystem.ini", File::READ | File::TEXT);
	File *outTempINI = NULL;

	if (!iniFile) {
		return;
	}
	
	try {
		// find the entry
		while (!((iniFile->eof()) || INI::isDeclarationOfType("ParticleSystem", particleSystemName, linebuff))) {
			iniFile->nextLine(linebuff, INI_MAX_CHARS_PER_LINE);
		}

		{	// copy it to a temp file
			if (iniFile->eof()) {
				throw 0;
			}
			
			outTempINI = TheFileSystem->openFile("temporary.ini", File::WRITE | File::TEXT);
			if (!outTempINI) {	
				throw 0;
			}


			while (!(iniFile->eof() || INI::isEndOfBlock(linebuff)) ) {
				outTempINI->write(linebuff, strlen(linebuff));
				iniFile->nextLine(linebuff, INI_MAX_CHARS_PER_LINE);
			}

			if (iniFile->eof()) {
				throw 1;
			}
			

			// write out the closing "END"
			outTempINI->write(linebuff, strlen(linebuff));
			outTempINI->close();
			outTempINI = NULL;
		}

		// force the current system to stop.
		if (st_particleSystemNeedsStopping) 
		{
			st_particleSystem->stop();
			st_particleSystem->destroy();
			st_particleSystemNeedsStopping = FALSE;
		}
		// reload that entry
		INI ini;
		ini.load("temporary.ini", INI_LOAD_OVERWRITE, NULL);
		
		// delete the file
//		unlink("temporary.ini");

		// force the particle system to update itself
		ParticleSystemTemplate *pTemp = const_cast<ParticleSystemTemplate*>(TheParticleSystemManager->findTemplate(particleSystemName));
		_updateAsciiStringParmsFromSystem(pTemp);
		_updatePanelParameters(pTemp);

	} catch (int why) {
		switch(why) 
		{
			case 2:	
			case 1: if (outTempINI) { outTempINI->close(); }
			case 0: if (iniFile) { iniFile->close(); }
		}
	}

}

static int _getNewCurrentParticleCap( void )
{
	typedef int (*funcptr)( void );

	if (!st_ParticleDLL) {
		return -1;
	}

	FARPROC proc;	
	proc = GetProcAddress(st_ParticleDLL, "GetNewParticleCap");

	if (!proc) {
		return -1;
	}

	return ((funcptr)proc)();
}

static void _updateCurrentParticleCap( void )
{
	typedef void (*funcptr)( int );

	if (!st_ParticleDLL) {
		return;
	}

	FARPROC proc;	
	proc = GetProcAddress(st_ParticleDLL, "UpdateCurrentParticleCap");

	if (!proc) {
		return;
	}

	((funcptr)proc)(TheGlobalData->m_maxParticleCount);
}

static void _updateCurrentParticleCount( void )
{
	typedef void (*funcptr)( int );

	if (!st_ParticleDLL) {
		return;
	}

	FARPROC proc;	
	proc = GetProcAddress(st_ParticleDLL, "UpdateCurrentNumParticles");

	if (!proc) {
		return;
	}

	((funcptr)proc)(TheParticleSystemManager->getParticleCount());
}

static void _reloadTextures( void )
{
	// Need no interaction with the particle editor now.
	ReloadAllTextures();
}

#ifdef DO_VTUNE_STUFF
static void _initVTune()
{
	// always try loading it, even if -vtune wasn't specified.
	st_vTuneDLL = ::LoadLibrary("vtuneapi.dll");
// nope, not here...
//DEBUG_ASSERTCRASH(st_vTuneDLL != NULL, "VTuneAPI DLL not found!"));
	
	if (st_vTuneDLL)
	{
		VTPause = (VTProc)::GetProcAddress(st_vTuneDLL, "VTPause");
		VTResume = (VTProc)::GetProcAddress(st_vTuneDLL, "VTResume");
		DEBUG_ASSERTCRASH(VTPause != NULL && VTResume != NULL, ("VTuneAPI procs not found!\n"));
	}
	else
	{
		VTPause = NULL;
		VTResume = NULL;
	}

	if (TheGlobalData->m_vTune) 
	{
		// if -vtune was specified, start it paused.
		st_EnableVTune = false;
		if (VTPause)
			VTPause();		
		// only complain about it being missing if they were expecting it to be present
		DEBUG_ASSERTCRASH(st_vTuneDLL != NULL, ("VTuneAPI DLL not found!\n"));
	}
	else
	{
		// otherwise enable it.
		st_EnableVTune = true;
		if (VTResume)
			VTResume();
	}
}

static void _updateVTune()
{
	if (!st_vTuneDLL) 
		return;

	if (st_EnableVTune)
	{
		if (VTResume)
			VTResume();
	}
	else
	{
		if (VTPause)
			VTPause();		
	}
}

static void _cleanUpVTune()
{
	if (st_vTuneDLL) 
	{
		FreeLibrary(st_vTuneDLL);
	}
	st_vTuneDLL = NULL;
	VTPause = NULL;
	VTResume = NULL;
}
#endif	// VTUNE
