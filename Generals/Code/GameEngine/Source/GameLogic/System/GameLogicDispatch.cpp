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

// FILE: GameLogicDispatch.cpp ////////////////////////////////////////////////////////////////////
// Author: Mike Booth, Colin Day
// Description: Message logic to drive the game play
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/CRCDebug.h"
#include "Common/FramePacer.h"
#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/GlobalData.h"
#include "Common/NameKeyGenerator.h"
#include "Common/ThingFactory.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/PlayerTemplate.h"
#include "Common/MessageStream.h"
#include "Common/MultiplayerSettings.h"
#include "Common/Recorder.h"
#include "Common/BuildAssistant.h"
#include "Common/SpecialPower.h"
#include "Common/ThingTemplate.h"
#include "Common/Upgrade.h"
#include "Common/StatsCollector.h"
#include "Common/Radar.h"

#include "GameLogic/AIPathfind.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/ObjectIter.h"
//#include "GameLogic/PartitionManager.h"
#include "GameLogic/AI.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/OpenContain.h"
#include "GameLogic/Module/ProductionUpdate.h"
#include "GameLogic/Module/SpecialPowerModule.h"
#include "GameLogic/ScriptActions.h"
#include "GameLogic/ScriptEngine.h"
#include "GameLogic/VictoryConditions.h"
#include "GameLogic/Weapon.h"

#include "GameClient/CommandXlat.h"
#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"
#include "GameClient/Eva.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/InGameUI.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/Mouse.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Shell.h"
#include "GameClient/Module/BeaconClientUpdate.h"
#include "GameClient/LookAtXlat.h"

#include "GameNetwork/NetworkInterface.h"




#define MAX_PATH_SUBJECTS 64
static Bool theBuildPlan = false;
static Object *thePlanSubject[ MAX_PATH_SUBJECTS ];
static int thePlanSubjectCount = 0;
//static WindowLayout *background = NULL;

// ------------------------------------------------------------------------------------------------
/** Issue the movement command to the object */
// ------------------------------------------------------------------------------------------------
static void doMoveTo( Object *obj, const Coord3D *pos )
{
	AIUpdateInterface *ai = obj->getAIUpdateInterface();
	DEBUG_ASSERTCRASH(ai, ("Attemped doMoveTo() on an Object with no AI"));
	if (ai)
	{
		if (theBuildPlan)
		{
			int i;

			// if this object isn't in the buildPlan set, add it
			for( i=0; i<thePlanSubjectCount; i++ )
				if (thePlanSubject[i] == obj)
					break;

			if (i == thePlanSubjectCount)
				thePlanSubject[ thePlanSubjectCount++ ] = obj;

			ai->queueWaypoint( pos );
		}
		else
		{
			ai->clearWaypointQueue();
			obj->leaveGroup();
			obj->releaseWeaponLock(LOCKED_TEMPORARILY);	// release any temporary locks.
			ai->aiMoveToPosition( pos, CMD_FROM_PLAYER );
		}
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
static void doSetRallyPoint( Object *obj, const Coord3D& pos )
{
	Bool isLocalPlayer = obj->isLocallyControlled();

	//
	// we must be able to find a path from the object to the point they have chosen, cause setting
	// a rally point at a invalid location would suck.  To be super nice, we have to make sure
	// that every type of object that can be created from the thing setting the rally point
	// can actually find a path from the thing to the point
	//

	// to see the never-finished code to check all locomotor sets, see past revs of GUICommandTranslator.cpp -MDC

	//
	// for now, just use the basic human locomotor ... and enable the above code when Steven
	// tells me how to get the locomotor sets based on a thing template (CBD)
	//
	NameKeyType key = NAMEKEY( "BasicHumanLocomotor" );
	LocomotorSet locomotorSet;
	locomotorSet.addLocomotor( TheLocomotorStore->findLocomotorTemplate( key ) );
	if( TheAI->pathfinder()->quickDoesPathExist( locomotorSet, obj->getPosition(), &pos ) == FALSE )
	{

		// user feedback
		if( isLocalPlayer )
		{

			// display error message to user
			TheInGameUI->message( TheGameText->fetch( "GUI:RallyPointNoPath" ) );

			// play the no can do sound
			static AudioEventRTS rallyNotSet("UnableToSetRallyPoint");
			rallyNotSet.setPosition(&pos);
			rallyNotSet.setPlayerIndex(obj->getControllingPlayer()->getPlayerIndex());
			TheAudio->addAudioEvent(&rallyNotSet);

		}

		return;

	}

	// feedback to the player
	if( isLocalPlayer )
	{

		// print a message to the user
		UnicodeString info;
		info.format( TheGameText->fetch( "GUI:RallyPointSet" ),
								 obj->getTemplate()->getDisplayName().str() );
		TheInGameUI->message( info );

		// play a sound for setting the rally point
		static AudioEventRTS rallyPointSet("RallyPointSet");
		rallyPointSet.setPosition(&pos);
		rallyPointSet.setPlayerIndex(obj->getControllingPlayer()->getPlayerIndex());
		TheAudio->addAudioEvent(&rallyPointSet);

		// mark the UI as dirty so that we re-evaluate the selection and show the rally point
		Drawable *draw = obj->getDrawable();
		if( draw && draw->isSelected() )
			TheControlBar->markUIDirty();

	}

	// if this object has a ProductionExitUpdate interface, we are setting a rally point
	ExitInterface *exitInterface = obj->getObjectExitInterface();
	if( exitInterface )
	{
		// set the rally point
		exitInterface->setRallyPoint( &pos );

	}

}

static Object * getSingleObjectFromSelection(const AIGroup *currentlySelectedGroup)
{
	if( currentlySelectedGroup )
	{
		const VecObjectID& selectedObjects = currentlySelectedGroup->getAllIDs();
		DEBUG_ASSERTCRASH(selectedObjects.size() == 1, ("Trying to get single object from multiple selection!"));
		VecObjectID::const_iterator it = selectedObjects.begin();
		return TheGameLogic->findObjectByID(*it);
	}
	return NULL;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void GameLogic::closeWindows( void )
{
	HideDiplomacy();
	ResetDiplomacy();
	HideInGameChat();
	ResetInGameChat();
	TheControlBar->hidePurchaseScience();
	TheControlBar->hideSpecialPowerShortcut();
	HideQuitMenu();

	// hide the options menu
	NameKeyType buttonID = TheNameKeyGenerator->nameToKey( "OptionsMenu.wnd:ButtonBack" );
	GameWindow *button = TheWindowManager->winGetWindowFromId( NULL, buttonID );
	GameWindow *window = TheWindowManager->winGetWindowFromId( NULL, TheNameKeyGenerator->nameToKey("OptionsMenu.wnd:OptionsMenuParent") );
	if(window)
		TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
																			(WindowMsgData)button, buttonID );
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void GameLogic::clearGameData( Bool showScoreScreen )
{
	if( !isInGame() )
	{
		DEBUG_CRASH(("We tried to clear the game data when we weren't in a game"));
		return;
	}

//	m_background = TheWindowManager->winCreateLayout("Menus/BlankWindow.wnd");
//	DEBUG_ASSERTCRASH(m_background,("We Couldn't Load Menus/BlankWindow.wnd"));
//	m_background->hide(FALSE);
//	m_background->bringForward();
	// reset the game engine to accept data for a new game
	if(TheStatsCollector)
		TheStatsCollector->writeFileEnd();

	TheScriptActions->closeWindows(FALSE); // Close victory or defeat windows.

	Bool shellGame = FALSE;
	if ((!isInShellGame() || !isInGame()) && showScoreScreen && !TheGlobalData->m_headless)
	{
		shellGame = TRUE;
		TheShell->push("Menus/ScoreScreen.wnd");
		TheShell->showShell(FALSE); // by passing in false, we don't want to run the Init on the shell screen we just pushed on

		void FixupScoreScreenMovieWindow( void );
		FixupScoreScreenMovieWindow();
	}

	TheGameEngine->reset();
	setGameMode(GAME_NONE);
//	m_background->bringForward();
//	if(shellGame)


	if (TheGlobalData->m_initialFile.isEmpty() == FALSE)
	{
		TheGameEngine->setQuitting(TRUE);
	}

	HideControlBar();
	closeWindows();

	TheMouse->setVisibility(TRUE);

	if(m_background)
	{
		m_background->destroyWindows();
		deleteInstance(m_background);
		m_background = NULL;
	}

}

// ------------------------------------------------------------------------------------------------
/** Prepare for a new game */
// ------------------------------------------------------------------------------------------------
void GameLogic::prepareNewGame( GameMode gameMode, GameDifficulty diff, Int rankPoints )
{
	//Added By Sadullah Nader
	//Fix for loading game scene
	setGameLoading(TRUE);
	//

	TheScriptEngine->setGlobalDifficulty(diff);

	if(!m_background)
	{
		m_background = TheWindowManager->winCreateLayout("Menus/BlankWindow.wnd");
		DEBUG_ASSERTCRASH(m_background,("We Couldn't Load Menus/BlankWindow.wnd"));
		m_background->hide(FALSE);
		m_background->bringForward();
	}
	m_background->getFirstWindow()->winClearStatus(WIN_STATUS_IMAGE);
	TheGameLogic->setGameMode( gameMode );
	if (!TheGlobalData->m_pendingFile.isEmpty())
	{
		TheWritableGlobalData->m_mapName = TheGlobalData->m_pendingFile;
		TheWritableGlobalData->m_pendingFile.clear();
	}

	m_rankPointsToAddAtGameStart = rankPoints;
	DEBUG_LOG(("GameLogic::prepareNewGame() - m_rankPointsToAddAtGameStart = %d", m_rankPointsToAddAtGameStart));

	// If we're about to start a game, hide the shell.
	if(!TheGameLogic->isInShellGame())
		TheShell->hideShell();

	m_startNewGame = FALSE;

}

//-------------------------------------------------------------------------------------------------
/** This message handles dispatches object command messages to the
  * appropriate objects.
	* @todo Rename this to "CommandProcessor", or similar. */
//-------------------------------------------------------------------------------------------------
void GameLogic::logicMessageDispatcher( GameMessage *msg, void *userData )
{
#ifdef RTS_DEBUG
	DEBUG_ASSERTCRASH(msg != NULL && msg != (GameMessage*)0xdeadbeef, ("bad msg"));
#endif

	Player *thisPlayer = ThePlayerList->getNthPlayer( msg->getPlayerIndex() );
	DEBUG_ASSERTCRASH( thisPlayer, ("logicMessageDispatcher: Processing message from unknown player (player index '%d')",
																	msg->getPlayerIndex()) );

	AIGroupPtr currentlySelectedGroup = NULL;

	if (isInGame())
	{
		if (msg->getType() >= GameMessage::MSG_BEGIN_NETWORK_MESSAGES && msg->getType() <= GameMessage::MSG_END_NETWORK_MESSAGES)
		{
			if (msg->getType() != GameMessage::MSG_LOGIC_CRC && msg->getType() != GameMessage::MSG_SET_REPLAY_CAMERA)
			{
				currentlySelectedGroup = TheAI->createGroup(); // can't do this outside a game - it'll cause sync errors galore.
				CRCGEN_LOG(( "Creating AIGroup %d in GameLogic::logicMessageDispatcher()", currentlySelectedGroup?currentlySelectedGroup->getID():0 ));
#if RETAIL_COMPATIBLE_AIGROUP
				thisPlayer->getCurrentSelectionAsAIGroup(currentlySelectedGroup);
#else
				thisPlayer->getCurrentSelectionAsAIGroup(currentlySelectedGroup.Peek());
#endif

				// We can't issue commands to groups that contain units that don't belong to the issuing player, so pretend like
				// there's nothing selected. Also, if currentlySelectedGroup is empty, go ahead and delete it, so that we can skip
				// any processing on it.
				if (currentlySelectedGroup->isEmpty())
				{
#if RETAIL_COMPATIBLE_AIGROUP
					TheAI->destroyGroup(currentlySelectedGroup);
#endif
					currentlySelectedGroup = NULL;
				}

				// If there are any units that the player doesn't own, then remove them from the "currentlySelectedGroup"
				if (currentlySelectedGroup)
					if (currentlySelectedGroup->removeAnyObjectsNotOwnedByPlayer(thisPlayer))
						currentlySelectedGroup = NULL;

				if(TheStatsCollector)
					TheStatsCollector->collectMsgStats(msg);
			}
		}
	}

#ifdef DEBUG_LOGGING
	AsciiString commandName;

	commandName = msg->getCommandAsString();
	if (msg->getType() < GameMessage::MSG_BEGIN_NETWORK_MESSAGES || msg->getType() > GameMessage::MSG_END_NETWORK_MESSAGES)
	{
		commandName.concat(" (NON-LOGIC-MESSAGE!!!)");
	}
	else if (msg->getType() == GameMessage::MSG_BEGIN_NETWORK_MESSAGES)
	{
		commandName = " (CRC message!)";
	}
#if 0
	if (commandName.isNotEmpty() /*&& msg->getType() != GameMessage::MSG_FRAME_TICK*/)
	{
		DEBUG_LOG(("Frame %d: GameLogic::logicMessageDispatcher() saw a %s from player %d (%ls)", getFrame(), commandName.str(),
			msg->getPlayerIndex(), thisPlayer->getPlayerDisplayName().str()));
	}
#endif
#endif // DEBUG_LOGGING

	// process the message
	GameMessage::Type msgType = msg->getType();
	switch( msgType )
	{
		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_NEW_GAME:
		{
			//DEBUG_ASSERTCRASH(msg->getArgumentCount() == 1 || msg->getArgumentCount() == 2, ("%d arguments to MSG_NEW_GAME", msg->getArgumentCount()));
			GameMode gameMode = (GameMode)msg->getArgument( 0 )->integer;
			Int rankPoints = 0;
			GameDifficulty diff = DIFFICULTY_NORMAL;
			if ( msg->getArgumentCount() >= 2 )
				diff = (GameDifficulty)msg->getArgument( 1 )->integer;
			if ( msg->getArgumentCount() >= 3 )
				rankPoints = msg->getArgument( 2 )->integer;

			if ( msg->getArgumentCount() >= 4 )
			{
				Int maxFPS = msg->getArgument( 3 )->integer;
				if (maxFPS < 1 || maxFPS > 1000)
					maxFPS = TheGlobalData->m_framesPerSecondLimit;
				DEBUG_LOG(("Setting max FPS limit to %d FPS", maxFPS));
				TheFramePacer->setFramesPerSecondLimit(maxFPS);
				TheWritableGlobalData->m_useFpsLimit = true;
			}

			// prepare for new game
			prepareNewGame( gameMode, diff, rankPoints );

			// start new game
			startNewGame( FALSE );

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_CLEAR_GAME_DATA:
		{

#if defined(RTS_DEBUG)
			if (TheDisplay && TheGlobalData->m_dumpAssetUsage)
				TheDisplay->dumpAssetUsage(TheGlobalData->m_mapName.str());
#endif

			if (currentlySelectedGroup)
			{
#if RETAIL_COMPATIBLE_AIGROUP
				TheAI->destroyGroup(currentlySelectedGroup);
#else
				currentlySelectedGroup->removeAll();
#endif
			}
			currentlySelectedGroup = NULL;
			TheGameLogic->clearGameData();
			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_META_BEGIN_PATH_BUILD:
		{
			DEBUG_LOG(("META: begin path build"));
			DEBUG_ASSERTCRASH(!theBuildPlan, ("mismatched theBuildPlan"));

			if (theBuildPlan == false)
			{
				theBuildPlan = true;
				thePlanSubjectCount = 0;
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_META_END_PATH_BUILD:
		{
			DEBUG_LOG(("META: end path build"));
			DEBUG_ASSERTCRASH(theBuildPlan, ("mismatched theBuildPlan"));

			// tell everyone who participated in the plan to move
			for( int i=0; i<thePlanSubjectCount; i++ )
			{
				AIUpdateInterface *ai = thePlanSubject[i]->getAIUpdateInterface();
				if (ai)
					ai->executeWaypointQueue();
			}

			theBuildPlan = false;
			thePlanSubjectCount = 0;

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_SET_RALLY_POINT:
		{
			Object *obj = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );
			Coord3D dest = msg->getArgument( 1 )->location;
			if (obj)
			{
				doSetRallyPoint( obj, dest );
			}

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_WEAPON:
		{
			WeaponSlotType weaponSlot = (WeaponSlotType)msg->getArgument( 0 )->integer;
			Int maxShotsToFire = msg->getArgument( 1 )->integer;

			// lock it just till the weapon is empty or the attack is "done"
			if( currentlySelectedGroup && currentlySelectedGroup->setWeaponLockForGroup( weaponSlot, LOCKED_TEMPORARILY ))
			{
				currentlySelectedGroup->groupAttackPosition( NULL, maxShotsToFire, CMD_FROM_PLAYER );
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_COMBATDROP_AT_OBJECT:
		{
			Object *targetObject = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );

			// issue command for either single object or for selected group
			if( currentlySelectedGroup )
				currentlySelectedGroup->groupCombatDrop( targetObject,
																								 *targetObject->getPosition(),
																								 CMD_FROM_PLAYER );

/*
			if( sourceObject && targetObject )
			{
				AIUpdateInterface* sourceAI = sourceObject->getAIUpdateInterface();
				if (sourceAI)
				{
					sourceAI->aiCombatDrop( targetObject, *targetObject->getPosition(), CMD_FROM_PLAYER );
				}
			}
*/

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_COMBATDROP_AT_LOCATION:
		{
			Coord3D targetLoc = msg->getArgument( 0 )->location;

			if( currentlySelectedGroup )
				currentlySelectedGroup->groupCombatDrop( NULL, targetLoc, CMD_FROM_PLAYER );

/*
			if( sourceObject )
			{
				AIUpdateInterface* sourceAI = sourceObject->getAIUpdateInterface();
				if (sourceAI)
				{
					sourceAI->aiCombatDrop( NULL, targetLoc, CMD_FROM_PLAYER );
				}
			}
*/

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_WEAPON_AT_OBJECT:
		{
			// Lock the weapon choice to the right weapon, then give an attack command

			WeaponSlotType weaponSlot = (WeaponSlotType)msg->getArgument( 0 )->integer;
			Object *targetObject = TheGameLogic->findObjectByID( msg->getArgument( 1 )->objectID );
			Int maxShotsToFire = msg->getArgument( 2 )->integer;

			// sanity
			if( targetObject == NULL )
				break;


			// issue command for either single object or for selected group
			if( currentlySelectedGroup )
			{
					// lock it just till the weapon is empty or the attack is "done"
				if (currentlySelectedGroup->setWeaponLockForGroup( weaponSlot, LOCKED_TEMPORARILY ))
					currentlySelectedGroup->groupAttackObject( targetObject, maxShotsToFire, CMD_FROM_PLAYER );
			}
			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_SWITCH_WEAPONS:
		{
			// use the selected group
			WeaponSlotType weaponSlot = (WeaponSlotType)msg->getArgument( 0 )->integer;
			// lock until un-switched, or switched to something else.
 			if( currentlySelectedGroup )
				currentlySelectedGroup->setWeaponLockForGroup( weaponSlot, LOCKED_PERMANENTLY );
			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_SET_MINE_CLEARING_DETAIL:
		{
			if( currentlySelectedGroup )
			{
				currentlySelectedGroup->setMineClearingDetail(true);
			}
			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_WEAPON_AT_LOCATION:
		{
			WeaponSlotType weaponSlot = (WeaponSlotType)msg->getArgument( 0 )->integer;
			Coord3D targetLoc = msg->getArgument( 1 )->location;
			Int maxShotsToFire = msg->getArgument( 2 )->integer;

			// issue command for either single object or for selected group
			if( currentlySelectedGroup )
			{
					// lock it just till the weapon is empty or the attack is "done"
				if (currentlySelectedGroup->setWeaponLockForGroup( weaponSlot, LOCKED_TEMPORARILY ))
 					currentlySelectedGroup->groupAttackPosition( &targetLoc, maxShotsToFire, CMD_FROM_PLAYER );


			}

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_SPECIAL_POWER:
		{

			// first argument is the special power ID
			UnsignedInt specialPowerID = msg->getArgument( 0 )->integer;

			// Command button options -- special power may care about variance options
			UnsignedInt options = msg->getArgument( 1 )->integer;

			// check for possible specific source, ignoring selection.
			ObjectID sourceID = msg->getArgument(2)->objectID;
			Object* source = TheGameLogic->findObjectByID(sourceID);
			if (source != NULL)
			{
				AIGroupPtr theGroup = TheAI->createGroup();
				theGroup->add(source);
				theGroup->groupDoSpecialPower( specialPowerID, options );
#if RETAIL_COMPATIBLE_AIGROUP
				TheAI->destroyGroup(theGroup);
#else
				theGroup->removeAll();
#endif
			}
			else
			{
				//Use the selected group!
				if( currentlySelectedGroup )
				{
					currentlySelectedGroup->groupDoSpecialPower( specialPowerID, options );
				}
			}
			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_SPECIAL_POWER_AT_LOCATION:
		{
			// first argument is the special power ID
			UnsignedInt specialPowerID = msg->getArgument( 0 )->integer;

			// Location argument 2 is destination
			Coord3D targetCoord = msg->getArgument(1)->location;

			// Object in way -- if applicable (some specials care, others don't)
			ObjectID objectID = msg->getArgument( 2 )->objectID;
			Object *objectInWay = TheGameLogic->findObjectByID( objectID );

			// Command button options -- special power may care about variance options
			UnsignedInt options = msg->getArgument( 3 )->integer;

			// check for possible specific source, ignoring selection.
			ObjectID sourceID = msg->getArgument(4)->objectID;
			Object* source = TheGameLogic->findObjectByID(sourceID);
			if (source != NULL)
			{
				AIGroupPtr theGroup = TheAI->createGroup();
				theGroup->add(source);
				theGroup->groupDoSpecialPowerAtLocation( specialPowerID, &targetCoord, INVALID_ANGLE, objectInWay, options );
#if RETAIL_COMPATIBLE_AIGROUP
				TheAI->destroyGroup(theGroup);
#else
				theGroup->removeAll();
#endif
			}
			else
			{
				//Use the selected group!
				if( currentlySelectedGroup )
				{
					currentlySelectedGroup->groupDoSpecialPowerAtLocation( specialPowerID, &targetCoord, INVALID_ANGLE, objectInWay, options );
				}
			}
			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_SPECIAL_POWER_AT_OBJECT:
		{
			// first argument is the special power ID
			UnsignedInt specialPowerID = msg->getArgument( 0 )->integer;

			// argument 2 is target object
			ObjectID targetID = msg->getArgument(1)->objectID;
			Object *target = TheGameLogic->findObjectByID( targetID );
			if( !target )
			{
				break;
			}

			// Command button options -- special power may care about variance options
			UnsignedInt options = msg->getArgument( 2 )->integer;

			// check for possible specific source, ignoring selection.
			ObjectID sourceID = msg->getArgument(3)->objectID;
			Object* source = TheGameLogic->findObjectByID(sourceID);
			if (source != NULL)
			{
				AIGroupPtr theGroup = TheAI->createGroup();
				theGroup->add(source);
				theGroup->groupDoSpecialPowerAtObject( specialPowerID, target, options );
#if RETAIL_COMPATIBLE_AIGROUP
				TheAI->destroyGroup(theGroup);
#else
				theGroup->removeAll();
#endif
			}
			else
			{
				if( currentlySelectedGroup )
				{
					currentlySelectedGroup->groupDoSpecialPowerAtObject( specialPowerID, target, options );
				}
			}
			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_ATTACKMOVETO:
		{
			Coord3D dest = msg->getArgument( 0 )->location;

			if (currentlySelectedGroup)
			{
				currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);	// release any temporary locks.
				currentlySelectedGroup->groupAttackMoveToPosition( &dest, NO_MAX_SHOTS_LIMIT, CMD_FROM_PLAYER );
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_FORCEMOVETO:
		{
			Coord3D dest = msg->getArgument( 0 )->location;

			if (currentlySelectedGroup)
			{
				currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);	// release any temporary locks.
				currentlySelectedGroup->groupMoveToPosition( &dest, false, CMD_FROM_PLAYER );
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		// MSG_DO_SALVAGE is intentionally set up to mimic the moveto.
		case GameMessage::MSG_DO_SALVAGE:
		case GameMessage::MSG_DO_MOVETO:
		{
			Coord3D dest = msg->getArgument( 0 )->location;

			if( currentlySelectedGroup )
			{
				//DEBUG_LOG(("GameLogicDispatch - got a MSG_DO_MOVETO command"));
				currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);	// release any temporary locks.
				currentlySelectedGroup->groupMoveToPosition( &dest, false, CMD_FROM_PLAYER );
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_ADD_WAYPOINT:
		{
			Coord3D dest = msg->getArgument( 0 )->location;

			if( currentlySelectedGroup )
			{
				//DEBUG_LOG(("GameLogicDispatch - got a MSG_DO_MOVETO command"));
				currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);	// release any temporary locks.
				currentlySelectedGroup->groupMoveToPosition( &dest, true, CMD_FROM_PLAYER );
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_GUARD_POSITION:
		{
			Coord3D loc = msg->getArgument( 0 )->location;
			GuardMode gm = (GuardMode)msg->getArgument( 1 )->integer;
			if (currentlySelectedGroup)
			{
				currentlySelectedGroup->groupGuardPosition(&loc, gm, CMD_FROM_PLAYER);
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_GUARD_OBJECT:
		{
			Object* obj = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );
			if (!obj)
				break;

			GuardMode gm = (GuardMode)msg->getArgument( 1 )->integer;
			if (currentlySelectedGroup)
			{
				currentlySelectedGroup->groupGuardObject(obj, gm, CMD_FROM_PLAYER);
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_STOP:
		{
			if (currentlySelectedGroup)
			{
				currentlySelectedGroup->groupIdle(CMD_FROM_PLAYER);
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_SCATTER:
		{
			if (currentlySelectedGroup)
			{
				currentlySelectedGroup->groupScatter(CMD_FROM_PLAYER);
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_CREATE_FORMATION:
		{
			if (currentlySelectedGroup)
			{
				currentlySelectedGroup->groupCreateFormation(CMD_FROM_PLAYER);
			}

			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_CLEAR_INGAME_POPUP_MESSAGE:
		{

			if( TheInGameUI )
			{
				TheInGameUI->clearPopupMessageData();
			}
			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_CHEER:
		{
			//All selected units play cheer animation.
			if( currentlySelectedGroup )
			{
				currentlySelectedGroup->groupCheer( CMD_FROM_PLAYER );
			}
			break;
		}

#if defined(RTS_DEBUG)
		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DEBUG_KILL_SELECTION:
		{
			//All selected units die
			if( currentlySelectedGroup )
			{
				const VecObjectID& selectedObjects = currentlySelectedGroup->getAllIDs();
				for (VecObjectID::const_iterator it = selectedObjects.begin(); it != selectedObjects.end(); ++it)
				{
					Object *obj = findObjectByID(*it);
					if (obj)
					{
						obj->kill();
					}
				}
			}
			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DEBUG_HURT_OBJECT:
		{
			Object* objToHurt = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );
			if (objToHurt)
			{
				DamageInfo damageInfo;
				damageInfo.in.m_damageType = DAMAGE_UNRESISTABLE;
				damageInfo.in.m_deathType = DEATH_NORMAL;
				damageInfo.in.m_sourceID = INVALID_ID;
				damageInfo.in.m_amount = objToHurt->getBodyModule()->getMaxHealth() / 10.0f;
				objToHurt->attemptDamage( &damageInfo );
			}
			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DEBUG_KILL_OBJECT:
		{
			Object* objToHurt = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );
			if (objToHurt)
			{
				objToHurt->kill();
			}
			break;
		}
#endif

#ifdef ALLOW_SURRENDER
		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_SURRENDER:
		{
			//All selected units surrender
			if( currentlySelectedGroup )
			{
				Object* objWeSurrenderedTo = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );
				Bool surrender = msg->getArgument( 1 )->boolean;
				currentlySelectedGroup->groupSurrender( objWeSurrenderedTo, surrender, CMD_FROM_PLAYER );
			}
			break;
		}
#endif

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_ENTER:
		{
			Object *enter = TheGameLogic->findObjectByID( msg->getArgument( 1 )->objectID );

			// sanity
			if( enter == NULL )
				break;

			if( currentlySelectedGroup )
			{
				currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);	// release any temporary locks.
				currentlySelectedGroup->groupEnter( enter, CMD_FROM_PLAYER );
			}

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_EXIT:
		{
			Object *objectWantingToExit = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );
#if RETAIL_COMPATIBLE_AIGROUP
			Object *objectContainingExiter = getSingleObjectFromSelection(currentlySelectedGroup);
#else
			Object *objectContainingExiter = getSingleObjectFromSelection(currentlySelectedGroup.Peek());
#endif

			// sanity
			if( objectWantingToExit == NULL )
				break;

			if( objectContainingExiter == NULL )
				break;

			// sanity, the player must actually control this object
			if( objectWantingToExit->getControllingPlayer() != thisPlayer )
				break;

			objectWantingToExit->releaseWeaponLock(LOCKED_TEMPORARILY);	// release any temporary locks.

			// exit whatever objectWantingToExit is INSIDE of
			AIUpdateInterface *ai = objectWantingToExit->getAIUpdateInterface();
			if( ai )
				ai->aiExit( objectContainingExiter, CMD_FROM_PLAYER );
			// Just like Enter, Exit needs to know the thing to exit.  This can no longer be assumed because of the Tunnel system.
			// If you do not specify the thing to Exit, it will Exit the thing it thinks it is in.  For a tunnel network,
			// that will be the specific Tunnel it entered.  (Scripts can talk directly to the guy to say Get Out Regardless)

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_EVACUATE:
		{
			// issue command for either single object or for selected group
//			AIGroup *group = TheAI->findGroup( *selectedGroupID );
			if( currentlySelectedGroup )
			{
				//Coord3D pos;
				//Bool hasArgs = FALSE;
				//hasArgs = (msg->getArgumentCount() > 0);

				//if (hasArgs)
				//	pos = msg->getArgument(0)->location;

				currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);	// release any temporary locks.

				// evacuate message is for the selected group
				//if (hasArgs)
				//	currentlySelectedGroup->groupMoveToAndEvacuate( &pos, CMD_FROM_PLAYER );
				//else
				currentlySelectedGroup->groupEvacuate( CMD_FROM_PLAYER );

// no, this is bad, don't do here, do when POSTING message
//			pickAndPlayUnitVoiceResponse( TheInGameUI->getAllSelectedDrawables(), GameMessage::MSG_EVACUATE );

			}

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_EXECUTE_RAILED_TRANSPORT:
		{

			// issue command to currently selected objects
			if( currentlySelectedGroup )
				currentlySelectedGroup->groupExecuteRailedTransport( CMD_FROM_PLAYER );

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_INTERNET_HACK:
		{
//			ObjectID sourceID = msg->getArgument( 0 )->objectID;
			if( currentlySelectedGroup )
			{
				currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);	// release any temporary locks.
				currentlySelectedGroup->groupHackInternet( CMD_FROM_PLAYER );
			}
			break;
		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_GET_REPAIRED:
		{
			Object *repairDepot = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );

			// sanity
			if( repairDepot == NULL )
				break;

			// tell the currently selected group to go get repaired
			if( currentlySelectedGroup )
				currentlySelectedGroup->groupGetRepaired( repairDepot, CMD_FROM_PLAYER );

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_DOCK:
		{
			Object *dockBuilding = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );

			// sanity
			if( dockBuilding == NULL )
				break;

			// tell the currently selected group to go get repaired
			if( currentlySelectedGroup )
				currentlySelectedGroup->groupDock( dockBuilding, CMD_FROM_PLAYER );

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_GET_HEALED:
		{
			Object *healDest = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );

			// sanity
			if( healDest == NULL )
				break;

			// tell the currently selected group to enter the building for healing
			if( currentlySelectedGroup )
				currentlySelectedGroup->groupGetHealed( healDest, CMD_FROM_PLAYER );

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_REPAIR:
		{
			Object *repairTarget = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );

			// sanity
			if( repairTarget == NULL )
				break;

			//
			// tell the currently selected group of objects to go repair the target object, note
			// that only one of them will actually go ahead and do the repair
			//
			if( currentlySelectedGroup )
				currentlySelectedGroup->groupRepair( repairTarget, CMD_FROM_PLAYER );

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_RESUME_CONSTRUCTION:
		{
			Object *constructTarget = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );

			// sanity
			if( constructTarget == NULL )
				break;

			//
			// tell the currently selected group of objects to resume construction on
			// the target object, note that only one of them will go off and resume construction
			// on the target
			//
			if( currentlySelectedGroup )
				currentlySelectedGroup->groupResumeConstruction( constructTarget, CMD_FROM_PLAYER );

// no, this is bad, don't do here, do when POSTING message
//		pickAndPlayUnitVoiceResponse( TheInGameUI->getAllSelectedDrawables(), msg->getType() );

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_SPECIAL_POWER_OVERRIDE_DESTINATION:
		{
			const Coord3D *loc = &msg->getArgument( 0 )->location;
			SpecialPowerType spType = (SpecialPowerType)msg->getArgument( 1 )->integer;

			ObjectID sourceID = msg->getArgument(2)->objectID;
			Object* source = TheGameLogic->findObjectByID(sourceID);
			if (source != NULL)
			{
				AIGroupPtr theGroup = TheAI->createGroup();
				theGroup->add(source);
				theGroup->groupOverrideSpecialPowerDestination( spType, loc, CMD_FROM_PLAYER );
#if RETAIL_COMPATIBLE_AIGROUP
				TheAI->destroyGroup(theGroup);
#else
				theGroup->removeAll();
#endif
			}
			else
			{
				if( currentlySelectedGroup )
				{
					currentlySelectedGroup->groupOverrideSpecialPowerDestination( spType, loc, CMD_FROM_PLAYER );
				}
			}

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_ATTACK_OBJECT:
		{
			Object *enemy = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );

			// Check enemy, as it is possible that he died this frame.
			if (enemy)
			{
				if (currentlySelectedGroup)
				{

					currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);	// release any temporary locks.
					currentlySelectedGroup->groupAttackObject( enemy, NO_MAX_SHOTS_LIMIT, CMD_FROM_PLAYER );

				}

			}

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_FORCE_ATTACK_OBJECT:
		{
			Object *enemy = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );

			// Check enemy, as it is possible that he died this frame.
			if (enemy)
			{
				if (currentlySelectedGroup)
				{
					currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);	// release any temporary locks.
					currentlySelectedGroup->groupForceAttackObject( enemy, NO_MAX_SHOTS_LIMIT, CMD_FROM_PLAYER );
				}

			}

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DO_FORCE_ATTACK_GROUND:
		{
			const Coord3D *pos = &msg->getArgument( 0 )->location;

			if (currentlySelectedGroup)
			{

				/////////////////////////////////////////////////////////////////////
				//Lorenzen sez: unclear, yet how to solve this for all cases
				//Kris: This code was added to allow the toxin tractor to force attack
				//      while contaminating. When this change was made, it was causing
				//      rangers and scud launchers to reset to primary weapon mode whenever
				//      force attacking while not idle. I fixed this by enforcing the
				//      temporary and permanent modes that are already set when attempting
				//      the new lock. In this case, the temp lock attempt will fail whenever
				//      a permanent lock is in effect, thus fixing the ranger and scud and
				//      allowing the tox tractor to work as well.
				Bool forceAttackRequiresPrimaryWeapon = !currentlySelectedGroup->isIdle();
				if ( forceAttackRequiresPrimaryWeapon )
				{
					currentlySelectedGroup->setWeaponLockForGroup( PRIMARY_WEAPON, LOCKED_TEMPORARILY );
					currentlySelectedGroup->groupAttackPosition( pos, NO_MAX_SHOTS_LIMIT, CMD_FROM_PLAYER );
					currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);
				}
				else
				///////////////////////////////////////////////////////////////////
				{
					currentlySelectedGroup->releaseWeaponLockForGroup(LOCKED_TEMPORARILY);
					currentlySelectedGroup->groupAttackPosition( pos, NO_MAX_SHOTS_LIMIT, CMD_FROM_PLAYER );
				}


			}

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_QUEUE_UPGRADE:
		{
			const UpgradeTemplate *upgradeT = TheUpgradeCenter->findUpgradeByKey( (NameKeyType)(msg->getArgument( 1 )->integer) );
			if (!upgradeT)	// sanity
				break;

			if (currentlySelectedGroup)
				currentlySelectedGroup->queueUpgrade( upgradeT );

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_CANCEL_UPGRADE:
		{
#if RETAIL_COMPATIBLE_AIGROUP
			Object *producer = getSingleObjectFromSelection(currentlySelectedGroup);
#else
			Object *producer = getSingleObjectFromSelection(currentlySelectedGroup.Peek());
#endif
			const UpgradeTemplate *upgradeT = TheUpgradeCenter->findUpgradeByKey( (NameKeyType)(msg->getArgument( 0 )->integer) );

			// sanity
			if( producer == NULL || upgradeT == NULL )
				break;

			// the player must actually control the producer object
			if( producer->getControllingPlayer() != thisPlayer )
				break;

			// producer must have a production update
				ProductionUpdateInterface *pu = producer->getProductionUpdateInterface();
				if( pu == NULL )
				break;

			// cancel the upgrade
			pu->cancelUpgrade( upgradeT );

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_QUEUE_UNIT_CREATE:
		{
#if RETAIL_COMPATIBLE_AIGROUP
			Object *producer = getSingleObjectFromSelection(currentlySelectedGroup);
#else
			Object *producer = getSingleObjectFromSelection(currentlySelectedGroup.Peek());
#endif
			const ThingTemplate *whatToCreate;
			ProductionID productionID;

			// get data from the message
			whatToCreate = TheThingFactory->findByTemplateID( msg->getArgument( 0 )->integer );
			productionID = (ProductionID)msg->getArgument( 1 )->integer;

			// sanity
			if ( producer == NULL || whatToCreate == NULL )
				break;

			// get the production interface for the producer
			ProductionUpdateInterface *pu = producer->getProductionUpdateInterface();
			if( pu == NULL )
			{

				DEBUG_ASSERTCRASH( 0, ("MSG_QUEUE_UNIT_CREATE: Producer '%s' doesn't have a unit production interface",
															producer->getTemplate()->getName().str()) );
				break;

			}

			// queue the build
			pu->queueCreateUnit( whatToCreate, productionID );

			break;

		}

		//-------------------------------------------------------------------------------------------------
		case GameMessage::MSG_CANCEL_UNIT_CREATE:
		{
#if RETAIL_COMPATIBLE_AIGROUP
			Object *producer = getSingleObjectFromSelection(currentlySelectedGroup);
#else
			Object *producer = getSingleObjectFromSelection(currentlySelectedGroup.Peek());
#endif
			ProductionID productionID = (ProductionID)msg->getArgument( 0 )->integer;

			// sanity
			if( producer == NULL )
				break;

			// sanity, the player must control the producer
			if( producer->getControllingPlayer() != thisPlayer )
				break;

			// get the unit production interface
			ProductionUpdateInterface *pu = producer->getProductionUpdateInterface();
			if( pu == NULL )
				return;

			// cancel the production
			pu->cancelUnitCreate( productionID );

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DOZER_CONSTRUCT:
		case GameMessage::MSG_DOZER_CONSTRUCT_LINE:
		{
			const ThingTemplate *place;
			Coord3D loc;
			Real angle;

			// get player, what to place, and location
#if RETAIL_COMPATIBLE_AIGROUP
			Object *constructorObject = getSingleObjectFromSelection(currentlySelectedGroup);
#else
			Object *constructorObject = getSingleObjectFromSelection(currentlySelectedGroup.Peek());
#endif
			place = TheThingFactory->findByTemplateID( msg->getArgument( 0 )->integer );
			loc = msg->getArgument( 1 )->location;
			angle = msg->getArgument( 2 )->real;

			if( place == NULL || constructorObject == NULL )
				break;  //These are not crashes, as the object may have died before this message came in

			if( msg->getType() == GameMessage::MSG_DOZER_CONSTRUCT )
			{

				TheBuildAssistant->buildObjectNow( constructorObject, place, &loc, angle,
																					 constructorObject->getControllingPlayer() );

			}
			else
			{
				Coord3D locEnd;

				// get the end of the line location in the world
				locEnd = msg->getArgument( 3 )->location;

				// place the line of structures, the end location being present will make it happen
				TheBuildAssistant->buildObjectLineNow( constructorObject, place, &loc, &locEnd, angle,
																							 constructorObject->getControllingPlayer() );

			}

			// place the sound for putting a building down

			static AudioEventRTS placeBuilding(AsciiString("PlaceBuilding"));
			placeBuilding.setObjectID(constructorObject->getID());
			TheAudio->addAudioEvent( &placeBuilding );


// no, this is bad, don't do here, do when POSTING message
//		pickAndPlayUnitVoiceResponse( TheInGameUI->getAllSelectedDrawables(), msg->getType() );

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DOZER_CANCEL_CONSTRUCT:
		{

			// get the building to cancel construction on
#if RETAIL_COMPATIBLE_AIGROUP
			Object *building = getSingleObjectFromSelection(currentlySelectedGroup);
#else
			Object *building = getSingleObjectFromSelection(currentlySelectedGroup.Peek());
#endif
			if( building == NULL )
				break;

			// the player sending this message must actually control this building
			if( building->getControllingPlayer() != thisPlayer )
				break;

			// Check to make sure it is actually under construction
			if( !building->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) )
				break;

			// OK, refund the money to the player, unless it is a rebuilding Hole.
			if( !building->testStatus(OBJECT_STATUS_RECONSTRUCTING))
			{
				Money *money = thisPlayer->getMoney();
				UnsignedInt amount = building->getTemplate()->calcCostToBuild( thisPlayer );
				money->deposit( amount, TRUE, FALSE );
			}

			//
			// Destroy the building ... killing the
			// building will automatically cause the dozer also cancel its own building
			// behavior and it will go on its merry way doing other assigned tasks
			//
			building->kill();

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_SELL:
		{

			// use the selected group
			if( currentlySelectedGroup )
				currentlySelectedGroup->groupSell( CMD_FROM_PLAYER );

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_TOGGLE_OVERCHARGE:
		{

			// use the selected group
			if( currentlySelectedGroup )
				currentlySelectedGroup->groupToggleOvercharge( CMD_FROM_PLAYER );

			break;

		}

#ifdef ALLOW_SURRENDER
		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_PICK_UP_PRISONER:
		{
			Object *prisoner = TheGameLogic->findObjectByID( msg->getArgument( 0 )->objectID );

			if( prisoner )
			{

				// use selected group
				if( currentlySelectedGroup )
					currentlySelectedGroup->groupPickUpPrisoner( prisoner, CMD_FROM_PLAYER );

			}

			break;

		}
#endif

#ifdef ALLOW_SURRENDER
		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_RETURN_TO_PRISON:
		{

			// use selected group
			if( currentlySelectedGroup )
				currentlySelectedGroup->groupReturnToPrison( NULL, CMD_FROM_PLAYER );

			break;

		}
#endif

		//---------------------------------------------------------------------------------------------
		// No sound does exactly the same logical processing as the usual message. Just double them up.
		case GameMessage::MSG_CREATE_SELECTED_GROUP_NO_SOUND:
		case GameMessage::MSG_CREATE_SELECTED_GROUP:
		{
			Bool createNewGroup = msg->getArgument( 0 )->boolean;
			Player *player = ThePlayerList->getNthPlayer(msg->getPlayerIndex());

			if (player == NULL) {
				DEBUG_CRASH(("GameLogicDispatch - MSG_CREATE_SELECTED_GROUP had an invalid player nubmer"));
				break;
			}

			Bool firstObject = TRUE;

			for (Int i = 1; i < msg->getArgumentCount(); ++i) {
				Object *obj = TheGameLogic->findObjectByID( msg->getArgument( i )->objectID );
				if (!obj) {
					continue;
				}

				TheGameLogic->selectObject(obj, createNewGroup && firstObject, player->getPlayerMask());
				firstObject = FALSE;
			}

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_REMOVE_FROM_SELECTED_GROUP:
		{
			Player *player = ThePlayerList->getNthPlayer(msg->getPlayerIndex());

			if (player == NULL) {
				DEBUG_CRASH(("GameLogicDispatch - MSG_CREATE_SELECTED_GROUP had an invalid player nubmer"));
				break;
			}

			for (Int i = 0; i < msg->getArgumentCount(); ++i) {
				ObjectID objID = msg->getArgument(i)->objectID;
				Object *objToRemove = TheGameLogic->findObjectByID(objID);
				if (!objToRemove) {
					continue;
				}

				TheGameLogic->deselectObject(objToRemove, player->getPlayerMask());
			}

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_DESTROY_SELECTED_GROUP:
		{
			Player *player = ThePlayerList->getNthPlayer(msg->getPlayerIndex());
			if (player != NULL)
			{
				player->setCurrentlySelectedAIGroup(NULL);
			}

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_SELECTED_GROUP_COMMAND:
		{

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_PLACE_BEACON:
		{
			if (thisPlayer->getPlayerTemplate() == NULL)
				break;
			Coord3D pos = msg->getArgument( 0 )->location;
			Region3D r;
			TheTerrainLogic->getExtent(&r);
			if (!r.isInRegionNoZ(&pos))
				pos = TheTerrainLogic->findClosestEdgePoint(&pos);
			const ThingTemplate *thing = TheThingFactory->findTemplate( thisPlayer->getPlayerTemplate()->getBeaconTemplate() );
			if (thing && !TheVictoryConditions->hasSinglePlayerBeenDefeated(thisPlayer))
			{
				// how many does this player have active?
				Int count;
				thisPlayer->countObjectsByThingTemplate( 1, &thing, false, &count );
				DEBUG_LOG(("Player already has %d beacons active", count));
				if (count >= TheMultiplayerSettings->getMaxBeaconsPerPlayer())
				{
					if (thisPlayer == ThePlayerList->getLocalPlayer())
					{
						// tell the user
						TheInGameUI->message( TheGameText->fetch("GUI:TooManyBeacons") );

						// play a sound
						static AudioEventRTS aSound("BeaconPlacementFailed");
						aSound.setPosition(&pos);
						aSound.setPlayerIndex(thisPlayer->getPlayerIndex());
						TheAudio->addAudioEvent(&aSound);
					}

					break;
				}
				Object *object = TheThingFactory->newObject( thing, thisPlayer->getDefaultTeam() );
				object->setPosition( &pos );
				object->setProducer(NULL);

				if (thisPlayer->getRelationship( ThePlayerList->getLocalPlayer()->getDefaultTeam() ) == ALLIES || ThePlayerList->getLocalPlayer()->isPlayerObserver())
				{
					// tell the user
					UnicodeString s;
					s.format(TheGameText->fetch("GUI:BeaconPlaced"), thisPlayer->getPlayerDisplayName().str());
					TheInGameUI->message( s );

					// play a sound
					static AudioEventRTS aSound("BeaconPlaced");
					aSound.setPlayerIndex(thisPlayer->getPlayerIndex());
					aSound.setPosition(&pos);
					TheAudio->addAudioEvent(&aSound);

					// beacons are a rare event; play a nifty radar event thingie
					TheRadar->createEvent( object->getPosition(), RADAR_EVENT_INFORMATION );

					if (ThePlayerList->getLocalPlayer()->getRelationship(thisPlayer->getDefaultTeam()) == ALLIES)
						TheEva->setShouldPlay(EVA_BeaconDetected);

					TheControlBar->markUIDirty(); // check if we should grey out the button
				}
				else
				{

					Int updateCount = 0;
					static NameKeyType nameKeyClientUpdate = NAMEKEY("BeaconClientUpdate");
					ClientUpdateModule ** clientModules = object->getDrawable()->getClientUpdateModules();
					if (clientModules)
					{
						while (*clientModules)
						{
							if ((*clientModules)->getModuleNameKey() == nameKeyClientUpdate)
							{
								(*(BeaconClientUpdate **)clientModules)->hideBeacon();
								++updateCount;
							}

							++clientModules;
						}
					}
					DEBUG_ASSERTCRASH(updateCount == 1, ("Saw %d update modules for the beacon!", updateCount));

				}
			}
			else
			{
				// tell the user
				TheInGameUI->message( TheGameText->fetch("GUI:BeaconPlacementFailed") );

				// play a sound
				static AudioEventRTS aSound("BeaconPlacementFailed");
				aSound.setPosition(&pos);
				aSound.setPlayerIndex(thisPlayer->getPlayerIndex());
				TheAudio->addAudioEvent(&aSound);
			}
			break;
		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_REMOVE_BEACON:
		{
			AIGroupPtr allSelectedObjects = TheAI->createGroup();
#if RETAIL_COMPATIBLE_AIGROUP
			thisPlayer->getCurrentSelectionAsAIGroup(allSelectedObjects); // need to act on all objects, so we can hide teammates' beacons.
#else
			thisPlayer->getCurrentSelectionAsAIGroup(allSelectedObjects.Peek()); // need to act on all objects, so we can hide teammates' beacons.
#endif
			if( allSelectedObjects )
			{
				const VecObjectID& selectedObjects = allSelectedObjects->getAllIDs();
				for (VecObjectID::const_iterator it = selectedObjects.begin(); it != selectedObjects.end(); ++it)
				{
					Object *beacon = findObjectByID(*it);
					if (beacon)
					{
						// TheSuperHackers @bugfix Prevent runtime crashing when a beacon is no longer associated with an initialized player.
						const PlayerTemplate *playerTemplate = beacon->getControllingPlayer()->getPlayerTemplate();
						if (!playerTemplate)
							continue;

						const ThingTemplate *thing = TheThingFactory->findTemplate( playerTemplate->getBeaconTemplate() );
						if (thing && thing->isEquivalentTo(beacon->getTemplate()))
						{
							if (beacon->getControllingPlayer() == thisPlayer)
							{
								TheGameLogic->destroyObject(beacon); // the owner is telling it to go away.  such is life.

								TheControlBar->markUIDirty(); // check if we should un-grey out the button
							}
							else if (thisPlayer == ThePlayerList->getLocalPlayer())
							{
								Drawable *beaconDrawable = beacon->getDrawable();
								if (beaconDrawable)
								{

									static NameKeyType nameKeyClientUpdate = NAMEKEY("BeaconClientUpdate");
									ClientUpdateModule ** clientModules = beaconDrawable->getClientUpdateModules();
									if (clientModules)
									{
										while (*clientModules)
										{
											if ((*clientModules)->getModuleNameKey() == nameKeyClientUpdate)
												(*(BeaconClientUpdate **)clientModules)->hideBeacon();

											++clientModules;
										}
									}
								}
							}
						}
					}
				}
#if RETAIL_COMPATIBLE_AIGROUP
				if (allSelectedObjects->isEmpty())
				{
					TheAI->destroyGroup(allSelectedObjects);
					allSelectedObjects = NULL;
				}
#endif
			}
			break;
		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_SET_BEACON_TEXT:
		{
			if( currentlySelectedGroup )
			{
				const VecObjectID& selectedObjects = currentlySelectedGroup->getAllIDs();
				for (VecObjectID::const_iterator it = selectedObjects.begin(); it != selectedObjects.end(); ++it)
				{
					Object *beacon = findObjectByID(*it);
					if (beacon)
					{
						Drawable *beaconDrawable = beacon->getDrawable();
						if (beaconDrawable)
						{
							UnicodeString s;
							for( int i=0; i<msg->getArgumentCount(); i++ )
							{
								s.concat( msg->getArgument(i)->wChar );
							}

							if (s.isEmpty())
								beaconDrawable->clearCaptionText();
							else
								beaconDrawable->setCaptionText(s);
						}
					}
				}
			}
			break;
		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_SELF_DESTRUCT:
		{
			if (msg->getArgument(0)->boolean)
			{
				// transfer control to any living ally
				Int i=0;
				for (; i<ThePlayerList->getPlayerCount(); ++i)
				{
					if (i != msg->getPlayerIndex())
					{
						Player *otherPlayer = ThePlayerList->getNthPlayer(i);
						if (thisPlayer->getRelationship(otherPlayer->getDefaultTeam()) == ALLIES &&
							otherPlayer->getRelationship(thisPlayer->getDefaultTeam()) == ALLIES)
						{
							if (TheVictoryConditions->hasSinglePlayerBeenDefeated(otherPlayer))
								continue;

							// a living ally!  hooray!
							otherPlayer->transferAssetsFromThat(thisPlayer);
							thisPlayer->killPlayer(); // just to be safe (and to kill beacons etc that don't transfer)
							break;
						}
					}
				}
				if (i == ThePlayerList->getPlayerCount())
				{
					// didn't find any allies.  die, loner!
					thisPlayer->killPlayer();
				}
			}
			else
			{
				thisPlayer->killPlayer();
			}
			// There is no reason to do any notification here, it now takes place in the victory conditions.
			// bonehead.
			break;
		}

		// --------------------------------------------------------------------------------------------
		case GameMessage::MSG_SET_REPLAY_CAMERA:
		{
			if (TheRecorder->isPlaybackMode() && TheGlobalData->m_useCameraInReplay && TheControlBar->getObserverLookAtPlayer() == thisPlayer)
			{
				if (TheTacticalView->isCameraMovementFinished())
				{
					ViewLocation loc;
					Coord3D pos;
					Real pitch, angle, zoom;
					pos = msg->getArgument( 0 )->location;
					angle = msg->getArgument( 1 )->real;
					pitch = msg->getArgument( 2 )->real;
					zoom = msg->getArgument( 3 )->real;
					loc.init(pos.x, pos.y, pos.z, angle, pitch, zoom);
					TheTacticalView->setLocation( &loc );

					// TheSuperHackers @fix xezon 18/09/2025 Lock the new location to avoid user input from changing the camera in this frame.
					TheTacticalView->lockViewUntilFrame( getFrame() + 1 );

					if (!TheLookAtTranslator->hasMouseMovedRecently())
					{
						TheMouse->setCursor( (Mouse::MouseCursor)(msg->getArgument( 4 )->integer) );
						ICoord2D mousePos = msg->getArgument( 5 )->pixel;
						TheMouse->setPosition( mousePos.x, mousePos.y );
						TheLookAtTranslator->setCurrentPos( mousePos );
					}
				}
			}
			break;
		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_CREATE_TEAM0:
		case GameMessage::MSG_CREATE_TEAM1:
		case GameMessage::MSG_CREATE_TEAM2:
		case GameMessage::MSG_CREATE_TEAM3:
		case GameMessage::MSG_CREATE_TEAM4:
		case GameMessage::MSG_CREATE_TEAM5:
		case GameMessage::MSG_CREATE_TEAM6:
		case GameMessage::MSG_CREATE_TEAM7:
		case GameMessage::MSG_CREATE_TEAM8:
		case GameMessage::MSG_CREATE_TEAM9:
		{
			Int playerIndex = msg->getPlayerIndex();
			Player *player = ThePlayerList->getNthPlayer(playerIndex);
			DEBUG_ASSERTCRASH(player != NULL, ("Could not find player for create team message"));

			// TheSuperHackers @tweak Stubbjax 17/08/2025 The local player processes this message in CommandXlat for immediate assignment.
			if (player && !player->isLocalPlayer())
				player->processCreateTeamGameMessage(msg->getType() - GameMessage::MSG_CREATE_TEAM0, msg);

			break;
		}

		case GameMessage::MSG_SELECT_TEAM0:
		case GameMessage::MSG_SELECT_TEAM1:
		case GameMessage::MSG_SELECT_TEAM2:
		case GameMessage::MSG_SELECT_TEAM3:
		case GameMessage::MSG_SELECT_TEAM4:
		case GameMessage::MSG_SELECT_TEAM5:
		case GameMessage::MSG_SELECT_TEAM6:
		case GameMessage::MSG_SELECT_TEAM7:
		case GameMessage::MSG_SELECT_TEAM8:
		case GameMessage::MSG_SELECT_TEAM9:
		{
			Int playerIndex = msg->getPlayerIndex();
			Player *player = ThePlayerList->getNthPlayer(playerIndex);
			DEBUG_ASSERTCRASH(player != NULL, ("Could not find player for select team message"));

			if (player == NULL)
			{
				break;
			}

			player->processSelectTeamGameMessage(msg->getType() - GameMessage::MSG_SELECT_TEAM0, msg);
			break;
		}

		case GameMessage::MSG_ADD_TEAM0:
		case GameMessage::MSG_ADD_TEAM1:
		case GameMessage::MSG_ADD_TEAM2:
		case GameMessage::MSG_ADD_TEAM3:
		case GameMessage::MSG_ADD_TEAM4:
		case GameMessage::MSG_ADD_TEAM5:
		case GameMessage::MSG_ADD_TEAM6:
		case GameMessage::MSG_ADD_TEAM7:
		case GameMessage::MSG_ADD_TEAM8:
		case GameMessage::MSG_ADD_TEAM9:
		{
			Int playerIndex = msg->getPlayerIndex();
			Player *player = ThePlayerList->getNthPlayer(playerIndex);
			DEBUG_ASSERTCRASH(player != NULL, ("Could not find player for add team message"));

			if (player == NULL)
			{
				break;
			}

			player->processAddTeamGameMessage(msg->getType() - GameMessage::MSG_ADD_TEAM0, msg);
			break;
		}


		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_LOGIC_CRC:
		{
			if (TheNetwork)
			{
				Int slotIndex = -1;
				for (Int i=0; i<MAX_SLOTS; ++i)
				{
					if (thisPlayer->getPlayerType() == PLAYER_HUMAN && TheNetwork->getPlayerName(i) == thisPlayer->getPlayerDisplayName())
					{
						slotIndex = i;
						break;
					}
				}

				if (slotIndex < 0 || !TheNetwork->isPlayerConnected(slotIndex))
					break;

				if (thisPlayer->isLocalPlayer())
				{
#if defined(RTS_DEBUG)
					// don't even put this in release, cause someone might hack it.
					if (!TheDebugIgnoreSyncErrors)
					{
#endif
						m_shouldValidateCRCs = TRUE;
#if defined(RTS_DEBUG)
					}
#endif
				}

				//UnsignedInt oldCRC = m_cachedCRCs[msg->getPlayerIndex()];
				UnsignedInt newCRC = msg->getArgument(0)->integer;
				//DEBUG_LOG(("Recieved CRC of %8.8X from %ls on frame %d", newCRC,
					//thisPlayer->getPlayerDisplayName().str(), m_frame));
				m_cachedCRCs[msg->getPlayerIndex()] = newCRC; // to mask problem: = (oldCRC < newCRC)?newCRC:oldCRC;
			}
			else if (TheRecorder && TheRecorder->isPlaybackMode())
			{
				UnsignedInt newCRC = msg->getArgument(0)->integer;
				//DEBUG_LOG(("Saw CRC of %X from player %d.  Our CRC is %X.  Arg count is %d",
					//newCRC, thisPlayer->getPlayerIndex(), getCRC(), msg->getArgumentCount()));

				TheRecorder->handleCRCMessage(newCRC, thisPlayer->getPlayerIndex(), (msg->getArgument(1)->boolean));
			}
			break;

		}

		//---------------------------------------------------------------------------------------------
		case GameMessage::MSG_PURCHASE_SCIENCE:
		{
			ScienceType science = (ScienceType)msg->getArgument( 0 )->integer;

			// sanity
			if( science == SCIENCE_INVALID || thisPlayer == NULL )
				break;

			thisPlayer->attemptToPurchaseScience(science);

			break;

		}

	}

#if RETAIL_COMPATIBLE_AIGROUP
	// TheSuperHackers @bugfix xezon 28/06/2025 This hack avoids crashing when players are selected during Replay playback.
	// It can read data from an already deleted AIGroup and return this function when its member size is 0, signifying that
	// it is indeed deleted.
	if (currentlySelectedGroup && currentlySelectedGroup->getCount() == 0)
		return;
#endif

	/**/ /// @todo: multiplayer semantics
	if (currentlySelectedGroup && TheRecorder->isPlaybackMode() && TheGlobalData->m_useCameraInReplay && TheControlBar->getObserverLookAtPlayer() == thisPlayer /*&& !TheRecorder->isMultiplayer()*/)
	{
		const VecObjectID& selectedObjects = currentlySelectedGroup->getAllIDs();
		TheInGameUI->deselectAllDrawables();
		for (VecObjectID::const_iterator it = selectedObjects.begin(); it != selectedObjects.end(); ++it)
		{
			const Object *obj = findObjectByID(*it);
			if (obj)
			{
				Drawable *draw = obj->getDrawable();
				if (draw)
					TheInGameUI->selectDrawable(draw);
			}
		}
	}
	/**/

	if( currentlySelectedGroup != NULL )
	{
#if RETAIL_COMPATIBLE_AIGROUP
		TheAI->destroyGroup(currentlySelectedGroup);
#else
		currentlySelectedGroup->removeAll();
#endif
	}

}
