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

// FILE: GameInfoWindow.cpp ////////////////////////////////////////////////////////////////////////
// Author: Chris Huybregts, Feb 2002
// Description: Game Info window callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/WindowLayout.h"
#include "GameClient/MapUtil.h"
#include "GameClient/Shell.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetStaticText.h"

#include "GameClient/GameText.h"
#include "GameClient/GameInfoWindow.h"
#include "Common/MultiplayerSettings.h"
#include "Common/PlayerTemplate.h"
#include "GameNetwork/GameInfo.h"
#include "GameNetwork/LANAPI.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

static GameWindow *parent = NULL;
static GameWindow *staticTextGameName = NULL;
static GameWindow *staticTextMapName = NULL;
static GameWindow *listBoxPlayers = NULL;
static GameWindow *winCrates = NULL;
static GameWindow *winSuperWeapons = NULL;
static GameWindow *winFreeForAll = NULL;

static NameKeyType parentID = NAMEKEY_INVALID;
static NameKeyType staticTextGameNameID = NAMEKEY_INVALID;
static NameKeyType staticTextMapNameID = NAMEKEY_INVALID;
static NameKeyType listBoxPlayersID = NAMEKEY_INVALID;
static NameKeyType winCratesID = NAMEKEY_INVALID;
static NameKeyType winSuperWeaponsID = NAMEKEY_INVALID;
static NameKeyType winFreeForAllID = NAMEKEY_INVALID;

static WindowLayout *gameInfoWindowLayout = NULL;
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

void CreateLANGameInfoWindow( GameWindow *sizeAndPosWin )
{
	if( !gameInfoWindowLayout )
		gameInfoWindowLayout = TheWindowManager->winCreateLayout( AsciiString( "Menus/GameInfoWindow.wnd" ) );
	
	gameInfoWindowLayout->runInit();
	gameInfoWindowLayout->bringForward();
	gameInfoWindowLayout->hide( TRUE );

	if( !parent || !sizeAndPosWin )
		return;
	Int x, y, width, height;
	sizeAndPosWin->winGetScreenPosition(&x,&y);
	parent->winSetPosition(x,y);

	sizeAndPosWin->winGetSize( &width, &height );
	parent->winSetSize(width, height);

}

void DestroyGameInfoWindow(void)
{
	if (gameInfoWindowLayout)
	{
		gameInfoWindowLayout->destroyWindows();
		gameInfoWindowLayout->deleteInstance();
		gameInfoWindowLayout = NULL;		
	}
}

void RefreshGameInfoWindow(GameInfo *gameInfo, UnicodeString gameName)
{
	static const Image *randomIcon = TheMappedImageCollection->findImageByName("GameinfoRANDOM");
	static const Image *observerIcon = TheMappedImageCollection->findImageByName("GameinfoOBSRVR");
	if(!gameInfoWindowLayout || !gameInfo )
		return;

	parent->winHide( FALSE );
	parent->winBringToTop();

	// Set the game name
	GadgetStaticTextSetText(staticTextGameName, ((LANGameInfo *)gameInfo)->getPlayerName(0));
	// set the map name
	UnicodeString map;
	AsciiString asciiMap = gameInfo->getMap();
	asciiMap.toLower();
	std::map<AsciiString, MapMetaData>::iterator it = TheMapCache->find(asciiMap);
	if (it != TheMapCache->end())
	{
		map = it->second.m_displayName;
	}
	else
	{
		// can happen if the map will have to be transferred... so use the leaf name (srj)
		const char *noPath = gameInfo->getMap().reverseFind('\\');
		if (noPath) 
		{
			++noPath;
		}
		else
		{
			noPath = gameInfo->getMap().str();
		}
		map.translate(noPath);
	}
	GadgetStaticTextSetText(staticTextMapName,map);

	// fill in the player list

	GadgetListBoxReset(listBoxPlayers);

	Int numColors = TheMultiplayerSettings->getNumColors();
	Color white = GameMakeColor(255,255,255,255);
//	Color grey =  GameMakeColor(188,188,188,255);
	for (Int i = 0; i < MAX_SLOTS; i ++)
	{
		Color playerColor = white;
		Int color = -1;
		Int addedRow;
		GameSlot *slot = gameInfo->getSlot(i);
		if(!slot || (slot->isOccupied() == FALSE))
			continue;
		color = slot->getColor();
		if(color > -1 && color < numColors)
		{
			MultiplayerColorDefinition *def = TheMultiplayerSettings->getColor(color);
			playerColor = def->getColor();
		}
		if(slot->isAI())
		{
			switch(slot->getState())
			{
				case SLOT_EASY_AI:
				{
					addedRow = GadgetListBoxAddEntryText(listBoxPlayers,TheGameText->fetch("GUI:EasyAI"),playerColor,-1, 1);
					break;
				}
				case SLOT_MED_AI:
				{
					addedRow = GadgetListBoxAddEntryText(listBoxPlayers,TheGameText->fetch("GUI:MediumAI"),playerColor,-1, 1);
					break;
				}
				case SLOT_BRUTAL_AI:
				{
					addedRow = GadgetListBoxAddEntryText(listBoxPlayers,TheGameText->fetch("GUI:HardAI"),playerColor,-1, 1);
					break;
				}
				default:
					break;
			}
		}
		else if(slot->isHuman())
		{
			addedRow = GadgetListBoxAddEntryText(listBoxPlayers, slot->getName(),playerColor,-1,1);
		}
		Int playerTemplate = slot->getPlayerTemplate();
		if(playerTemplate == PLAYERTEMPLATE_OBSERVER)
		{
			GadgetListBoxAddEntryImage(listBoxPlayers, observerIcon,addedRow, 0, 22,25);
		}
		else if(playerTemplate < 0 || playerTemplate >= ThePlayerTemplateStore->getPlayerTemplateCount())
		{
			///< @todo: When we get art that shows player's side, then we'll actually draw the art instead of putting in text
			GadgetListBoxAddEntryImage(listBoxPlayers, randomIcon,addedRow, 0, 22,25);
			//GadgetListBoxAddEntryText(listBoxPlayers,TheGameText->fetch("GUI:???"),playerColor,addedRow, 0);
		}
		else
		{
			const PlayerTemplate *fact = ThePlayerTemplateStore->getNthPlayerTemplate(playerTemplate);
			GadgetListBoxAddEntryImage(listBoxPlayers, fact->getSideIconImage(),addedRow, 0, 22,25);
			//GadgetListBoxAddEntryText(listBoxPlayers,fact->getDisplayName(),playerColor,addedRow, 0);
		}
	
	}
}

void HideGameInfoWindow(Bool hide)
{
	if(!parent)
		return;
	parent->winHide(hide);

}

//-------------------------------------------------------------------------------------------------
/** Initialize the GameInfoWindow */
//-------------------------------------------------------------------------------------------------
void GameInfoWindowInit( WindowLayout *layout, void *userData )
{

	parentID = TheNameKeyGenerator->nameToKey( "GameInfoWindow.wnd:ParentGameInfo" );
	staticTextGameNameID = TheNameKeyGenerator->nameToKey( "GameInfoWindow.wnd:StaticTextGameName" );
	staticTextMapNameID = TheNameKeyGenerator->nameToKey( "GameInfoWindow.wnd:StaticTextMapName" );
	listBoxPlayersID = TheNameKeyGenerator->nameToKey( "GameInfoWindow.wnd:ListBoxPlayers" );
	winCratesID = TheNameKeyGenerator->nameToKey( "GameInfoWindow.wnd:WinCrates" );
	winSuperWeaponsID = TheNameKeyGenerator->nameToKey( "GameInfoWindow.wnd:WinSuperWeapons" );
	winFreeForAllID = TheNameKeyGenerator->nameToKey( "GameInfoWindow.wnd:WinFreeForAll" );
	
	parent = TheWindowManager->winGetWindowFromId( NULL, parentID );
	staticTextGameName = TheWindowManager->winGetWindowFromId( parent, staticTextGameNameID );
	staticTextMapName = TheWindowManager->winGetWindowFromId( parent, staticTextMapNameID );
	listBoxPlayers = TheWindowManager->winGetWindowFromId( parent, listBoxPlayersID );
	winCrates = TheWindowManager->winGetWindowFromId( parent, winCratesID );
	winSuperWeapons = TheWindowManager->winGetWindowFromId( parent, winSuperWeaponsID );
	winFreeForAll = TheWindowManager->winGetWindowFromId( parent, winFreeForAllID );

	GadgetStaticTextSetText(staticTextGameName,UnicodeString::TheEmptyString);
	GadgetStaticTextSetText(staticTextMapName,UnicodeString::TheEmptyString);
	GadgetListBoxReset(listBoxPlayers);
	
}  // end MapSelectMenuInit


//-------------------------------------------------------------------------------------------------
/** GameInfo window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType GameInfoWindowSystem( GameWindow *window, UnsignedInt msg, 
																				  WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg ) 
	{
// might use these later
//			GameWindow *control = (GameWindow *)mData1;
//			Int controlID = control->winGetWindowId();


		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{

			break;

		}  // end create

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{

			break;

		}  // end case

		// --------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			return MSG_HANDLED;

		}  // end input

		//---------------------------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch

	return MSG_HANDLED;

}  // end MapSelectMenuSystem

