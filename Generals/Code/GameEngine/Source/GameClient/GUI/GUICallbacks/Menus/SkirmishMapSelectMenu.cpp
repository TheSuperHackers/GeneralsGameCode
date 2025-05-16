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

// FILE: SkirmishMapSelectMenu.cpp ////////////////////////////////////////////////////////////////////////
// Author: Chris Brue, August 2002
// Description: MapSelect menu window callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameEngine.h"
#include "Common/MessageStream.h"
#include "Common/UserPreferences.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameText.h"
#include "GameClient/Mouse.h"
#include "GameClient/Shell.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetRadioButton.h"
#include "GameClient/GadgetStaticText.h"
#include "GameNetwork/LANAPICallbacks.h"
#include "GameClient/MapUtil.h"

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static NameKeyType buttonBack = NAMEKEY_INVALID;
static NameKeyType buttonOK = NAMEKEY_INVALID;
static NameKeyType listboxMap = NAMEKEY_INVALID;
static GameWindow *parent = NULL;
static GameWindow *mapList = NULL;

static NameKeyType radioButtonSystemMapsID = NAMEKEY_INVALID;
static NameKeyType radioButtonUserMapsID = NAMEKEY_INVALID;

static GameWindow *buttonMapStartPosition[MAX_SLOTS] = {NULL,NULL,NULL,NULL,
																								NULL,NULL,NULL,NULL };
static NameKeyType buttonMapStartPositionID[MAX_SLOTS] = { NAMEKEY_INVALID,NAMEKEY_INVALID,
																									NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID };

static GameWindow *winMapPreview = NULL;
static NameKeyType winMapPreviewID = NAMEKEY_INVALID;

static void NullifyControls()
{
	mapList = NULL;
	winMapPreview = NULL;
	parent = NULL;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		buttonMapStartPosition[i] = NULL;
	}
}

extern WindowLayout *skirmishMapSelectLayout;

// Tooltips -------------------------------------------------------------------------------

static void mapListTooltipFunc(GameWindow *window,
													WinInstanceData *instData,
													UnsignedInt mouse)
{
	Int x, y, row, col;
	x = LOLONGTOSHORT(mouse);
	y = HILONGTOSHORT(mouse);

	GadgetListBoxGetEntryBasedOnXY(window, x, y, row, col);

	if (row == -1 || col == -1)
	{
		TheMouse->setCursorTooltip( UnicodeString::TheEmptyString);
		return;
	}

	Int imageItemData = (Int)GadgetListBoxGetItemData(window, row, 1);
	UnicodeString tooltip;
	switch (imageItemData)
	{
		case 0:
			tooltip = TheGameText->fetch("TOOLTIP:MapNoSuccess");
			break;
		case 1:
			tooltip = TheGameText->fetch("TOOLTIP:MapEasySuccess");
			break;
		case 2:
			tooltip = TheGameText->fetch("TOOLTIP:MapMediumSuccess");
			break;
		case 3:
			tooltip = TheGameText->fetch("TOOLTIP:MapHardSuccess");
			break;
	}

	TheMouse->setCursorTooltip( tooltip );
}

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
void positionStartSpots( AsciiString mapName, GameWindow *buttonMapStartPositions[], GameWindow *mapWindow);
void skirmishPositionStartSpots( void );
void skirmishUpdateSlotList( void );
void showSkirmishGameOptionsUnderlyingGUIElements( Bool show )
{                          
	AsciiString parentName( "SkirmishGameOptionsMenu.wnd:SkirmishGameOptionsMenuParent" );
	NameKeyType parentID = TheNameKeyGenerator->nameToKey( parentName );
	GameWindow *parent = TheWindowManager->winGetWindowFromId( NULL, parentID );
	if (!parent)
		return;

	// hide some GUI elements of the screen underneath
	GameWindow *win;

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:MapWindow") );
	win->winHide( !show );

	//win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:StaticTextTitle") );
	//win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:StaticTextTeam") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:StaticTextFaction") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:StaticTextColor") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxTeam0") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxTeam1") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxTeam2") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxTeam3") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxTeam4") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxTeam5") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxTeam6") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxTeam7") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxColor0") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxColor1") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxColor2") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxColor3") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxColor4") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxColor5") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxColor6") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxColor7") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxPlayerTemplate0") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxPlayerTemplate1") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxPlayerTemplate2") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxPlayerTemplate3") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxPlayerTemplate4") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxPlayerTemplate5") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxPlayerTemplate6") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ComboBoxPlayerTemplate7") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:TextEntryMapDisplay") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ButtonSelectMap") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ButtonStart") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:StaticTextMapPreview") );
	win->winHide( !show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ButtonReset") );
	win->winEnable( show );

	win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:ButtonBack") );
	win->winEnable( show );
}

//-------------------------------------------------------------------------------------------------
/** Initialize the MapSelect menu */
//-------------------------------------------------------------------------------------------------
void SkirmishMapSelectMenuInit( WindowLayout *layout, void *userData )
{

	// set keyboard focus to main parent
	AsciiString parentName( "SkirmishMapSelectMenu.wnd:SkrimishMapSelectMenuParent" );
	NameKeyType parentID = TheNameKeyGenerator->nameToKey( parentName );
	parent = TheWindowManager->winGetWindowFromId( NULL, parentID );

	TheWindowManager->winSetFocus( parent );

	LANPreferences pref;
	Bool usesSystemMapDir = pref.usesSystemMapDir();

	const MapMetaData *mmd = TheMapCache->findMap(TheSkirmishGameInfo->getMap());
	if (mmd)
	{
		usesSystemMapDir = mmd->m_isOfficial;
	}

	winMapPreviewID = TheNameKeyGenerator->nameToKey( AsciiString("SkirmishMapSelectMenu.wnd:WinMapPreview") );
	winMapPreview = TheWindowManager->winGetWindowFromId(parent, winMapPreviewID);


	buttonBack = TheNameKeyGenerator->nameToKey( AsciiString("SkirmishMapSelectMenu.wnd:ButtonBack") );
	buttonOK = TheNameKeyGenerator->nameToKey( AsciiString("SkirmishMapSelectMenu.wnd:ButtonOK") );
	listboxMap = TheNameKeyGenerator->nameToKey( AsciiString("SkirmishMapSelectMenu.wnd:ListboxMap") );

	radioButtonSystemMapsID = TheNameKeyGenerator->nameToKey( "SkirmishMapSelectMenu.wnd:RadioButtonSystemMaps" );
	radioButtonUserMapsID = TheNameKeyGenerator->nameToKey( "SkirmishMapSelectMenu.wnd:RadioButtonUserMaps" );
	GameWindow *radioButtonSystemMaps = TheWindowManager->winGetWindowFromId( parent, radioButtonSystemMapsID );
	GameWindow *radioButtonUserMaps = TheWindowManager->winGetWindowFromId( parent, radioButtonUserMapsID );
	if (usesSystemMapDir)
		GadgetRadioSetSelection( radioButtonSystemMaps, FALSE );
	else
		GadgetRadioSetSelection( radioButtonUserMaps, FALSE );

	AsciiString tmpString;
	for (Int i = 0; i < MAX_SLOTS; i++)
	{
		tmpString.format("SkirmishMapSelectMenu.wnd:ButtonMapStartPosition%d", i);
		buttonMapStartPositionID[i] = TheNameKeyGenerator->nameToKey( tmpString );
		buttonMapStartPosition[i] = TheWindowManager->winGetWindowFromId( winMapPreview, buttonMapStartPositionID[i] );
		DEBUG_ASSERTCRASH(buttonMapStartPosition[i], ("Could not find the ButtonMapStartPosition[%d]",i ));
		buttonMapStartPosition[i]->winHide(TRUE);
		buttonMapStartPosition[i]->winEnable(FALSE);
	}

	showSkirmishGameOptionsUnderlyingGUIElements(FALSE);

	// get the listbox window
	AsciiString listString( "SkirmishMapSelectMenu.wnd:ListboxMap" );
	NameKeyType mapListID = TheNameKeyGenerator->nameToKey( listString );
	mapList = TheWindowManager->winGetWindowFromId( parent, mapListID );
	if( mapList )
	{
		if (TheMapCache)
			TheMapCache->updateCache();
		if (usesSystemMapDir)
		{
			populateMapListbox( mapList, TRUE, TRUE, TheSkirmishGameInfo->getMap() );
		}
		else
		{
			populateMapListbox( mapList, FALSE, FALSE, TheSkirmishGameInfo->getMap() );
			populateMapListboxNoReset( mapList, FALSE, TRUE, TheSkirmishGameInfo->getMap() );
		}
		mapList->winSetTooltipFunc(mapListTooltipFunc);
	}

}  // end SkirmishMapSelectMenuInit

//-------------------------------------------------------------------------------------------------
/** MapSelect menu shutdown method */
//-------------------------------------------------------------------------------------------------
void SkirmishMapSelectMenuShutdown( WindowLayout *layout, void *userData )
{

	// hide menu
	layout->hide( TRUE );
	
	NullifyControls();
	
	// our shutdown is complete
	TheShell->shutdownComplete( layout );

}  // end LanMapSelectMenuShutdown

//-------------------------------------------------------------------------------------------------
/** MapSelect menu update method */
//-------------------------------------------------------------------------------------------------
void SkirmishMapSelectMenuUpdate( WindowLayout *layout, void *userData )
{

}  // end SkirmishMapSelectMenuUpdate

//-------------------------------------------------------------------------------------------------
/** Map select menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType SkirmishMapSelectMenuInput( GameWindow *window, UnsignedInt msg,
																				 WindowMsgData mData1, WindowMsgData mData2 )
{

	switch( msg ) 
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
			UnsignedByte state = mData2;

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
						AsciiString buttonName( "SkirmishMapSelectMenu.wnd:ButtonBack" );
						NameKeyType buttonID = TheNameKeyGenerator->nameToKey( buttonName );
						GameWindow *button = TheWindowManager->winGetWindowFromId( window, buttonID );

						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED, 
																								(WindowMsgData)button, buttonID );

					}  // end if

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}  // end escape

			}  // end switch( key )

		}  // end char

	}  // end switch( msg )

	return MSG_IGNORED;

}  // end SkirmishMapSelectMenuInput

//-------------------------------------------------------------------------------------------------
/** MapSelect menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType SkirmishMapSelectMenuSystem( GameWindow *window, UnsignedInt msg, 
																				  WindowMsgData mData1, WindowMsgData mData2 )
{
	
	switch( msg ) 
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{
			break;

		}  // end create

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			NullifyControls();
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
		case GLM_DOUBLE_CLICKED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				if( controlID == listboxMap ) 
				{
					int rowSelected = mData2;
				
					if (rowSelected >= 0)
					{
						GadgetListBoxSetSelected( control, rowSelected );
						GameWindow *button = TheWindowManager->winGetWindowFromId( window, buttonOK );

						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED, 
																								(WindowMsgData)button, buttonOK );
					}
				}
				break;
			}
		//---------------------------------------------------------------------------------------------
			case GLM_SELECTED:
			{
				GameWindow *mapWindow = TheWindowManager->winGetWindowFromId( parent, listboxMap );

				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				if( controlID == listboxMap ) 
				{
					int rowSelected = mData2;
					if( rowSelected < 0 )
					{
						positionStartSpots( AsciiString::TheEmptyString, buttonMapStartPosition, winMapPreview);
						//winMapPreview->winClearStatus(WIN_STATUS_IMAGE);
						break;
					}
					winMapPreview->winSetStatus(WIN_STATUS_IMAGE);
					UnicodeString map;
					// get text of the map to load
					map = GadgetListBoxGetText( mapWindow, rowSelected, 0 );

					// set the map name in the global data map name
					AsciiString asciiMap;
					const char *mapFname = (const char *)GadgetListBoxGetItemData( mapWindow, rowSelected );
					DEBUG_ASSERTCRASH(mapFname, ("No map item data"));
					if (mapFname)
						asciiMap = mapFname;
					else
						asciiMap.translate( map );
					asciiMap.toLower();
					Image *image = getMapPreviewImage(asciiMap);
					winMapPreview->winSetUserData((void *)TheMapCache->findMap(asciiMap));
					if(image)
					{
						winMapPreview->winSetEnabledImage(0, image);
					}
					else
					{
						winMapPreview->winClearStatus(WIN_STATUS_IMAGE);
					}
					positionStartSpots( asciiMap, buttonMapStartPosition, winMapPreview);
				}
				break;
			}

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			// this isn't fixed yet
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

			if ( controlID == radioButtonSystemMapsID )
			{
				if (TheMapCache)
					TheMapCache->updateCache();
				populateMapListbox( mapList, TRUE, TRUE, TheSkirmishGameInfo->getMap() );
				//LANPreferences pref;
				//pref["UseSystemMapDir"] = "yes";
				//pref.write();
			}
			else if ( controlID == radioButtonUserMapsID )
			{
				if (TheMapCache)
					TheMapCache->updateCache();
				populateMapListbox( mapList, FALSE, FALSE, TheSkirmishGameInfo->getMap() );
				populateMapListboxNoReset( mapList, FALSE, TRUE, TheSkirmishGameInfo->getMap() );
				//LANPreferences pref;
				//pref["UseSystemMapDir"] = "no";
				//pref.write();
			}
			else if ( controlID == buttonBack )
			{
				showSkirmishGameOptionsUnderlyingGUIElements(TRUE);

				skirmishMapSelectLayout->destroyWindows();
				deleteInstance(skirmishMapSelectLayout);
				skirmishMapSelectLayout = NULL;
				skirmishPositionStartSpots();
				//TheShell->pop();
				//do you need this ??
				//PostToLanGameOptions( MAP_BACK );
			}  // end if
			else if ( controlID == buttonOK )
			{

				Int selected;
				UnicodeString map;
				GameWindow *mapWindow = TheWindowManager->winGetWindowFromId( parent, listboxMap );

				// get the selected index
				GadgetListBoxGetSelected( mapWindow, &selected );

				if( selected != -1 )
				{
					//buttonPushed = true;
					// set the map name in the global data map name
					AsciiString asciiMap;
					const char *mapFname = (const char *)GadgetListBoxGetItemData( mapWindow, selected );
					DEBUG_ASSERTCRASH(mapFname, ("No map item data"));
					if (mapFname)
						asciiMap = mapFname;
					else
						asciiMap.translate( map );
					TheSkirmishGameInfo->setMap( asciiMap );

					const MapMetaData *md = TheMapCache->findMap(asciiMap);
					if (!md)
					{
						TheSkirmishGameInfo->setMapCRC(0);
						TheSkirmishGameInfo->setMapSize(0);
					}
					else
					{
						TheSkirmishGameInfo->setMapCRC(md->m_CRC);
						TheSkirmishGameInfo->setMapSize(md->m_filesize);
					}

					// reset the start positions
					for(Int i = 0; i < MAX_SLOTS; ++i)
						TheSkirmishGameInfo->getSlot(i)->setStartPos(-1);
					GameWindow *win;
					win	= TheWindowManager->winGetWindowFromId( parent, TheNameKeyGenerator->nameToKey("SkirmishGameOptionsMenu.wnd:TextEntryMapDisplay") );
					if(win)
					{
				    if (md)
				    {
  						GadgetStaticTextSetText(win, md->m_displayName);
            }
					}
					//if (mapFname)
						//setupGameStart(mapFname);

					showSkirmishGameOptionsUnderlyingGUIElements(TRUE);
					skirmishPositionStartSpots();
					skirmishUpdateSlotList();

					skirmishMapSelectLayout->destroyWindows();
					deleteInstance(skirmishMapSelectLayout);
					skirmishMapSelectLayout = NULL;
					//TheShell->pop();

				}  // end if
			}  // end else if

			break;

		}  // end selected

		default:
			return MSG_IGNORED;

	}  // end switch

	return MSG_HANDLED;

}  // end SkirmishMapSelectMenuSystem*/
