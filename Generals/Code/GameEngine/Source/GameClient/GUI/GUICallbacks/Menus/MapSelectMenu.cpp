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

// FILE: MapSelectMenu.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, October 2001
// Description: MapSelect menu window callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameEngine.h"
#include "Common/MessageStream.h"
#include "Common/RandomValue.h"
#include "Common/UserPreferences.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/ScriptEngine.h"
#include "GameClient/AnimateWindowManager.h"
#include "GameClient/CampaignManager.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetRadioButton.h"
#include "GameClient/MapUtil.h"
#include "GameClient/Mouse.h"

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
static NameKeyType radioButtonSystemMapsID = NAMEKEY_INVALID;
static NameKeyType radioButtonUserMapsID = NAMEKEY_INVALID;
static GameWindow *mapList = nullptr;

static Bool showSoloMaps = true;
static Bool isShuttingDown = false;
static Bool startGame = false;
static Bool buttonPushed = false;
static GameDifficulty s_AIDiff = DIFFICULTY_NORMAL;
static void setupGameStart(AsciiString mapName)
{
	startGame = true;
	TheWritableGlobalData->m_pendingFile = mapName;
	TheShell->reverseAnimatewindow();
}

static void doGameStart( void )
{
	startGame = false;

	if (TheGameLogic->isInGame())
		TheGameLogic->clearGameData();
	//TheScriptEngine->setGlobalDifficulty(s_AIDiff); // CANNOT DO THIS! REPLAYS WILL BREAK!!!
	// send a message to the logic for a new game
	GameMessage *msg = TheMessageStream->appendMessage( GameMessage::MSG_NEW_GAME );
	msg->appendIntegerArgument(GAME_SINGLE_PLAYER);
	msg->appendIntegerArgument(s_AIDiff);
	msg->appendIntegerArgument(0);

	/// @todo: when Campaign & skirmish are separated, make campaign have fixed seed and skirmish random.
	InitRandom(0);
	/*
	if (TheGlobalData->m_fixedSeed >= 0)
		InitGameLogicRandom(TheGlobalData->m_fixedSeed);
	else
		InitGameLogicRandom(GameClientRandomValue(0, INT_MAX - 1));
	*/

	isShuttingDown = true;
}


//-------------------------------------------------------------------------------------------------
/** This is called when a shutdown is complete for this menu */
//-------------------------------------------------------------------------------------------------
static void shutdownComplete( WindowLayout *layout )
{

	isShuttingDown = false;

	// hide the layout
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout );

}

void SetDifficultyRadioButton( void )
{
	NameKeyType parentID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:MapSelectMenuParent" );
	GameWindow *parent = TheWindowManager->winGetWindowFromId( nullptr, parentID );

	if (!TheScriptEngine)
	{
		s_AIDiff = DIFFICULTY_EASY;
	}
	else
	{
		switch (TheScriptEngine->getGlobalDifficulty())
		{
			case DIFFICULTY_EASY:
			{
				NameKeyType radioButtonEasyAIID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:RadioButtonEasyAI" );
				GameWindow *radioButtonEasyAI = TheWindowManager->winGetWindowFromId( parent, radioButtonEasyAIID );
				GadgetRadioSetSelection(radioButtonEasyAI, FALSE);
				s_AIDiff = DIFFICULTY_EASY;
				break;
			}
			case DIFFICULTY_NORMAL:
			{
				NameKeyType radioButtonMediumAIID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:RadioButtonMediumAI" );
				GameWindow *radioButtonMediumAI = TheWindowManager->winGetWindowFromId( parent, radioButtonMediumAIID );
				GadgetRadioSetSelection(radioButtonMediumAI, FALSE);
				s_AIDiff = DIFFICULTY_NORMAL;
				break;
			}
			case DIFFICULTY_HARD:
			{
				NameKeyType radioButtonHardAIID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:RadioButtonHardAI" );
				GameWindow *radioButtonHardAI = TheWindowManager->winGetWindowFromId( parent, radioButtonHardAIID );
				GadgetRadioSetSelection(radioButtonHardAI, FALSE);
				s_AIDiff = DIFFICULTY_HARD;
				break;
			}

		default:
			{
				DEBUG_CRASH(("unrecognized difficulty level in the script engine"));
			}

		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Initialize the MapSelect menu */
//-------------------------------------------------------------------------------------------------
void MapSelectMenuInit( WindowLayout *layout, void *userData )
{
	showSoloMaps = true;
	buttonPushed = false;
	isShuttingDown = false;
	startGame = false;
	TheShell->showShellMap(TRUE);
	// show menu
	layout->hide( FALSE );

	OptionPreferences pref;
	Bool usesSystemMapDir = pref.usesSystemMapDir();

	// get the listbox window
	NameKeyType mapListID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:ListboxMap" );
	mapList = TheWindowManager->winGetWindowFromId( nullptr, mapListID );
	if( mapList )
	{
		if (TheMapCache)
			TheMapCache->updateCache();
		populateMapListbox( mapList, usesSystemMapDir, !showSoloMaps );
	}


	// set keyboard focus to main parent
	NameKeyType parentID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:MapSelectMenuParent" );
	GameWindow *parent = TheWindowManager->winGetWindowFromId( nullptr, parentID );
	TheWindowManager->winSetFocus( parent );

	NameKeyType buttonBackID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:ButtonBack" );
	GameWindow *buttonBack = TheWindowManager->winGetWindowFromId( nullptr, buttonBackID );

	NameKeyType buttonOKID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:ButtonOK" );
	GameWindow *buttonOK = TheWindowManager->winGetWindowFromId( nullptr, buttonOKID );


	TheShell->registerWithAnimateManager(buttonBack, WIN_ANIMATION_SLIDE_RIGHT, TRUE,0);
	TheShell->registerWithAnimateManager(buttonOK, WIN_ANIMATION_SLIDE_LEFT, TRUE, 0);

	SetDifficultyRadioButton();

	radioButtonSystemMapsID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:RadioButtonSystemMaps" );
	radioButtonUserMapsID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:RadioButtonUserMaps" );
	GameWindow *radioButtonSystemMaps = TheWindowManager->winGetWindowFromId( parent, radioButtonSystemMapsID );
	GameWindow *radioButtonUserMaps = TheWindowManager->winGetWindowFromId( parent, radioButtonUserMapsID );
	if (usesSystemMapDir)
		GadgetRadioSetSelection( radioButtonSystemMaps, FALSE );
	else
		GadgetRadioSetSelection( radioButtonUserMaps, FALSE );
}

//-------------------------------------------------------------------------------------------------
/** MapSelect menu shutdown method */
//-------------------------------------------------------------------------------------------------
void MapSelectMenuShutdown( WindowLayout *layout, void *userData )
{
	if (!startGame)
		isShuttingDown = true;

	// if we are shutting down for an immediate pop, skip the animations
	Bool popImmediate = *(Bool *)userData;
	if( popImmediate )
	{

		shutdownComplete( layout );
		return;

	}

	if (!startGame)
		TheShell->reverseAnimatewindow();

}

//-------------------------------------------------------------------------------------------------
/** MapSelect menu update method */
//-------------------------------------------------------------------------------------------------
void MapSelectMenuUpdate( WindowLayout *layout, void *userData )
{

	if (startGame && TheShell->isAnimFinished())
		doGameStart();

	// We'll only be successful if we've requested to
	if(isShuttingDown && TheShell->isAnimFinished())
		shutdownComplete(layout);


}

//-------------------------------------------------------------------------------------------------
/** Map select menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType MapSelectMenuInput( GameWindow *window, UnsignedInt msg,
																				 WindowMsgData mData1, WindowMsgData mData2 )
{

	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
			UnsignedByte state = mData2;
			if (buttonPushed)
				break;

			switch( key )
			{

				// ----------------------------------------------------------------------------------------
				case KEY_ESC:
				{

					//
					// send a simulated selected event to the parent window of the
					// back/exit button
					//
					if( BitIsSet( state, KEY_STATE_UP ) )
					{
						NameKeyType buttonID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:ButtonBack" );
						GameWindow *button = TheWindowManager->winGetWindowFromId( window, buttonID );

						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
																								(WindowMsgData)button, buttonID );

					}

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}

			}

		}

	}

	return MSG_IGNORED;

}

//-------------------------------------------------------------------------------------------------
/** MapSelect menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType MapSelectMenuSystem( GameWindow *window, UnsignedInt msg,
																				  WindowMsgData mData1, WindowMsgData mData2 )
{
	static NameKeyType buttonBack = NAMEKEY_INVALID;
	static NameKeyType buttonOK = NAMEKEY_INVALID;
	static NameKeyType listboxMap = NAMEKEY_INVALID;
	static NameKeyType radioButtonEasyAI = NAMEKEY_INVALID;
	static NameKeyType radioButtonMediumAI = NAMEKEY_INVALID;
	static NameKeyType radioButtonHardAI = NAMEKEY_INVALID;
	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{

			// get ids for our children controls
			buttonBack = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:ButtonBack" );
			buttonOK = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:ButtonOK" );
			listboxMap = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:ListboxMap" );
			radioButtonEasyAI = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:RadioButtonEasyAI" );
			radioButtonMediumAI = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:RadioButtonMediumAI" );
			radioButtonHardAI = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:RadioButtonHardAI" );
			break;

		}

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			return MSG_HANDLED;

		}

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			if (buttonPushed)
				break;

			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

			static NameKeyType singlePlayerID = NAMEKEY("MapSelectMenu.wnd:ButtonSinglePlayer");
			static NameKeyType multiplayerID = NAMEKEY("MapSelectMenu.wnd:ButtonMultiplayer");
			if ( controlID == singlePlayerID )
			{
				showSoloMaps = true;
				OptionPreferences pref;
				populateMapListbox( mapList, pref.usesSystemMapDir(), !showSoloMaps );
			}
			else if ( controlID == multiplayerID )
			{
				showSoloMaps = false;
				OptionPreferences pref;
				populateMapListbox( mapList, pref.usesSystemMapDir(), !showSoloMaps );
			}
			else if ( controlID == radioButtonSystemMapsID )
			{
				if (TheMapCache)
					TheMapCache->updateCache();
				populateMapListbox( mapList, TRUE, !showSoloMaps );
				OptionPreferences pref;
				pref["UseSystemMapDir"] = "yes";
				pref.write();
			}
			else if ( controlID == radioButtonUserMapsID )
			{
				if (TheMapCache)
					TheMapCache->updateCache();
				populateMapListbox( mapList, FALSE, !showSoloMaps );
				OptionPreferences pref;
				pref["UseSystemMapDir"] = "no";
				pref.write();
			}
			else if( controlID == buttonBack )
			{

				// go back one screen
				TheShell->pop();
				buttonPushed = true;

			}
			else if( controlID == buttonOK )
			{

				Int selected;
				UnicodeString map;
				GameWindow *mapWindow = TheWindowManager->winGetWindowFromId( nullptr, listboxMap );

				// get the selected index
				GadgetListBoxGetSelected( mapWindow, &selected );

				if( selected != -1 )
				{
					buttonPushed = true;
					// reset the campaign manager to empty
					if( TheCampaignManager )
					  TheCampaignManager->setCampaign( "" );
					// get text of the map to load
					const char *mapFname = (const char *)GadgetListBoxGetItemData( mapWindow, selected );
					DEBUG_ASSERTCRASH(mapFname, ("No map item data"));
					if (mapFname)
						setupGameStart(mapFname);
				}

			}
			else if( controlID == radioButtonEasyAI)
			{
				s_AIDiff = DIFFICULTY_EASY;
			}
			else if( controlID == radioButtonMediumAI)
			{
				s_AIDiff = DIFFICULTY_NORMAL;
			}
			else if( controlID == radioButtonHardAI)
			{
				s_AIDiff = DIFFICULTY_HARD;
			}
			break;

		}
		case GLM_DOUBLE_CLICKED:
			{
				if (buttonPushed)
					break;

				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				if( controlID == listboxMap )
				{
					int rowSelected = mData2;

					if (rowSelected >= 0)
					{
						//buttonPushed = true;
						GadgetListBoxSetSelected( control, rowSelected );
						NameKeyType buttonOKID = TheNameKeyGenerator->nameToKey( "MapSelectMenu.wnd:ButtonOK" );
						GameWindow *buttonOK = TheWindowManager->winGetWindowFromId( nullptr, buttonOKID );

						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
																								(WindowMsgData)buttonOK, buttonOKID );
					}
				}
				break;
			}
		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}
