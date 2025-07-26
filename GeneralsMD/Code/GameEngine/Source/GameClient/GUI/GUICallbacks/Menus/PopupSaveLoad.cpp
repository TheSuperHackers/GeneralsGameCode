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

// FILE: PopupSaveLoad.cpp /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: PopupSaveLoad.cpp
//
// Created:   Chris Brue, June 2002
//
// Desc:      the Save/Load window control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameEngine.h"
#include "Common/GameState.h"
#include "Common/MessageStream.h"
#include "GameClient/CampaignManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GameText.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/Shell.h"
#include "GameLogic/GameLogic.h"
#include "GameClient/GameWindowTransitions.h"

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static NameKeyType buttonBackKey					= NAMEKEY_INVALID;
static NameKeyType buttonSaveKey					= NAMEKEY_INVALID;
static NameKeyType buttonLoadKey					= NAMEKEY_INVALID;
static NameKeyType buttonDeleteKey				= NAMEKEY_INVALID;
static NameKeyType listboxGamesKey				= NAMEKEY_INVALID;
static NameKeyType buttonOverwriteCancel	= NAMEKEY_INVALID;
static NameKeyType buttonOverwriteConfirm = NAMEKEY_INVALID;
static NameKeyType buttonLoadCancel				= NAMEKEY_INVALID;
static NameKeyType buttonLoadConfirm			= NAMEKEY_INVALID;
static NameKeyType buttonSaveDescCancel		= NAMEKEY_INVALID;
static NameKeyType buttonSaveDescConfirm	= NAMEKEY_INVALID;
static NameKeyType buttonDeleteConfirm		= NAMEKEY_INVALID;
static NameKeyType buttonDeleteCancel			= NAMEKEY_INVALID;

static GameWindow *buttonFrame = NULL;
static GameWindow *overwriteConfirm = NULL;
static GameWindow *loadConfirm = NULL;
static GameWindow *saveDesc = NULL;
static GameWindow *listboxGames = NULL;
static GameWindow *editDesc = NULL;
static GameWindow *deleteConfirm = NULL;

static GameWindow *parent = NULL;
static SaveLoadLayoutType currentLayoutType = SLLT_INVALID;
static Bool isPopup = FALSE;
static Int	initialGadgetDelay = 2;
static Bool justEntered = FALSE;
static Bool isShuttingDown = false;

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
extern Bool DontShowMainMenu; //KRIS
extern Bool ReplayWasPressed;
// ------------------------------------------------------------------------------------------------
/** Given the current layout and selection in the game listbox, update the main save/load
	* menu buttons to be enabled or disabled */
// ------------------------------------------------------------------------------------------------
static void updateMenuActions( void )
{

	// for loading only, disable the save button, otherwise enable it
	GameWindow *saveButton = TheWindowManager->winGetWindowFromId( NULL, buttonSaveKey );
	DEBUG_ASSERTCRASH( saveButton, ("SaveLoadMenuInit: Unable to find save button") );
	if( currentLayoutType == SLLT_LOAD_ONLY )
		saveButton->winEnable( FALSE );
	else
		saveButton->winEnable( TRUE );

	// get the games listbox
	//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );

	// if something with a game file is selected we can use load and delete
	Int selected;
	GadgetListBoxGetSelected( listboxGames, &selected );
	AvailableGameInfo *selectedGameInfo;
	selectedGameInfo = (AvailableGameInfo *)GadgetListBoxGetItemData( listboxGames, selected );
	GameWindow *buttonLoad = TheWindowManager->winGetWindowFromId( NULL, buttonLoadKey );
	buttonLoad->winEnable( selectedGameInfo != NULL );
	GameWindow *buttonDelete = TheWindowManager->winGetWindowFromId( NULL, buttonDeleteKey );
	buttonDelete->winEnable( selectedGameInfo != NULL );
	
}  // end updateMenuActions

//-------------------------------------------------------------------------------------------------
/** Initialize the SaveLoad menu */
//-------------------------------------------------------------------------------------------------
void SaveLoadMenuInit( WindowLayout *layout, void *userData )
{

	// set default behavior for this menu
	currentLayoutType = SLLT_SAVE_AND_LOAD;
	isPopup = TRUE;
	// get layout type if present
	if( userData )
		currentLayoutType = *((SaveLoadLayoutType *)userData);

  // get ids for our children controls
	buttonBackKey					 = NAMEKEY( "PopupSaveLoad.wnd:ButtonBack" );
	buttonSaveKey					 = NAMEKEY( "PopupSaveLoad.wnd:ButtonSave" );
	buttonLoadKey					 = NAMEKEY( "PopupSaveLoad.wnd:ButtonLoad" );
	buttonDeleteKey        = NAMEKEY( "PopupSaveLoad.wnd:ButtonDelete" );
	listboxGamesKey				 = NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" );
	buttonOverwriteCancel	 = NAMEKEY( "PopupSaveLoad.wnd:ButtonOverwriteCancel" );
	buttonOverwriteConfirm = NAMEKEY( "PopupSaveLoad.wnd:ButtonOverwriteConfirm" );
	buttonLoadCancel			 = NAMEKEY( "PopupSaveLoad.wnd:ButtonLoadCancel" );
	buttonLoadConfirm      = NAMEKEY( "PopupSaveLoad.wnd:ButtonLoadConfirm" );
	buttonSaveDescCancel   = NAMEKEY( "PopupSaveLoad.wnd:ButtonSaveDescCancel" );
	buttonSaveDescConfirm  = NAMEKEY( "PopupSaveLoad.wnd:ButtonSaveDescConfirm" );
	buttonDeleteConfirm		 = NAMEKEY( "PopupSaveLoad.wnd:ButtonDeleteConfirm" );
	buttonDeleteCancel		 = NAMEKEY( "PopupSaveLoad.wnd:ButtonDeleteCancel" );

	//set keyboard focus to main parent and set modal
	NameKeyType parentID = TheNameKeyGenerator->nameToKey("PopupSaveLoad.wnd:SaveLoadMenu");
	parent = TheWindowManager->winGetWindowFromId( NULL, parentID );
	TheWindowManager->winSetFocus( parent );
	TheWindowManager->winSetModal( parent );

	// enable the menu action buttons
	buttonFrame = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );
	buttonFrame->winEnable( TRUE );

	// get confirmation windows and hide
	overwriteConfirm = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:OverwriteConfirmParent" ) );
	overwriteConfirm->winHide( TRUE );
	loadConfirm = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:LoadConfirmParent" ) );
	loadConfirm->winHide( TRUE );
	saveDesc = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:SaveDescParent" ) );
	saveDesc->winHide( TRUE );
	deleteConfirm = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:DeleteConfirmParent" ) );
	editDesc = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:EntryDesc" ) );
	// get the listbox that will have the save games in it
	listboxGames = TheWindowManager->winGetWindowFromId( NULL, listboxGamesKey );
	DEBUG_ASSERTCRASH( listboxGames != NULL, ("SaveLoadMenuInit - Unable to find games listbox") );

	// populate the listbox with the save games on disk
	TheGameState->populateSaveGameListbox( listboxGames, currentLayoutType );

	// update the availability of the menu buttons
	updateMenuActions();

}  // end SaveLoadMenuInit

//-------------------------------------------------------------------------------------------------
/** Initialize the SaveLoad menu */
//-------------------------------------------------------------------------------------------------
void SaveLoadMenuFullScreenInit( WindowLayout *layout, void *userData )
{

	TheShell->showShellMap(TRUE);

	isPopup = FALSE;
	// set default behavior for this menu
	currentLayoutType = SLLT_LOAD_ONLY;
	
	// get layout type if present
	if( userData )
		currentLayoutType = *((SaveLoadLayoutType *)userData);

  // get ids for our children controls
	buttonBackKey					 = NAMEKEY( "SaveLoad.wnd:ButtonBack" );
	buttonSaveKey					 = NAMEKEY( "SaveLoad.wnd:ButtonSave" );
	buttonLoadKey					 = NAMEKEY( "SaveLoad.wnd:ButtonLoad" );
	buttonDeleteKey        = NAMEKEY( "SaveLoad.wnd:ButtonDelete" );
	listboxGamesKey				 = NAMEKEY( "SaveLoad.wnd:ListboxGames" );
	buttonOverwriteCancel	 = NAMEKEY( "SaveLoad.wnd:ButtonOverwriteCancel" );
	buttonOverwriteConfirm = NAMEKEY( "SaveLoad.wnd:ButtonOverwriteConfirm" );
	buttonLoadCancel			 = NAMEKEY( "SaveLoad.wnd:ButtonLoadCancel" );
	buttonLoadConfirm      = NAMEKEY( "SaveLoad.wnd:ButtonLoadConfirm" );
	buttonSaveDescCancel   = NAMEKEY( "SaveLoad.wnd:ButtonSaveDescCancel" );
	buttonSaveDescConfirm  = NAMEKEY( "SaveLoad.wnd:ButtonSaveDescConfirm" );
	buttonDeleteConfirm		 = NAMEKEY( "SaveLoad.wnd:ButtonDeleteConfirm" );
	buttonDeleteCancel		 = NAMEKEY( "SaveLoad.wnd:ButtonDeleteCancel" );

	//set keyboard focus to main parent and set modal
	NameKeyType parentID = TheNameKeyGenerator->nameToKey("SaveLoad.wnd:SaveLoadMenu");
	parent = TheWindowManager->winGetWindowFromId( NULL, parentID );
	TheWindowManager->winSetFocus( parent );
//	TheWindowManager->winSetModal( parent );

	// enable the menu action buttons
	buttonFrame = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "SaveLoad.wnd:MenuButtonFrame" ) );
	buttonFrame->winEnable( TRUE );

	// get confirmation windows and hide
	overwriteConfirm = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "SaveLoad.wnd:OverwriteConfirmParent" ) );
	overwriteConfirm->winHide( TRUE );
	loadConfirm = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "SaveLoad.wnd:LoadConfirmParent" ) );
	loadConfirm->winHide( TRUE );
	saveDesc = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "SaveLoad.wnd:SaveDescParent" ) );
	saveDesc->winHide( TRUE );

	editDesc = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "SaveLoad.wnd:EntryDesc" ) );
	deleteConfirm = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "SaveLoad.wnd:DeleteConfirmParent" ) );
	// get the listbox that will have the save games in it
	listboxGames = TheWindowManager->winGetWindowFromId( NULL, listboxGamesKey );
	DEBUG_ASSERTCRASH( listboxGames != NULL, ("SaveLoadMenuInit - Unable to find games listbox") );

	// populate the listbox with the save games on disk
	TheGameState->populateSaveGameListbox( listboxGames, currentLayoutType );

	// update the availability of the menu buttons
	updateMenuActions();

	layout->hide(FALSE);
	justEntered = TRUE;
	initialGadgetDelay = 2;
	if(parent)
		parent->winHide(TRUE);
	isShuttingDown = false;
}  // end SaveLoadMenuInit

//-------------------------------------------------------------------------------------------------
/** SaveLoad menu shutdown method */
//-------------------------------------------------------------------------------------------------
void SaveLoadMenuShutdown( WindowLayout *layout, void *userData )
{
	
	Bool popImmediate = *(Bool *)userData;
	if( popImmediate )
	{

		layout->hide( TRUE );
		TheShell->shutdownComplete( layout );
		return;

	}  //end if

	// our shutdown is complete
	TheTransitionHandler->reverse("SaveLoadMenuFade");
	isShuttingDown = TRUE;
}  // end SaveLoadMenuShutdown

//-------------------------------------------------------------------------------------------------
/** SaveLoad menu update method */
//-------------------------------------------------------------------------------------------------
void SaveLoadMenuUpdate( WindowLayout *layout, void *userData )
{

	if(DontShowMainMenu && justEntered)
		justEntered = FALSE;
	if(ReplayWasPressed && justEntered)
	{
		justEntered = FALSE;
		ReplayWasPressed = FALSE;
	}
	if(justEntered)
	{
		if(initialGadgetDelay == 1)
		{
			TheTransitionHandler->remove("MainMenuDefaultMenuLogoFade");
			TheTransitionHandler->setGroup("SaveLoadMenuFade");
			initialGadgetDelay = 2;
			justEntered = FALSE;
		}
		else
			initialGadgetDelay--;
	}

	if(isShuttingDown && TheShell->isAnimFinished()&& TheTransitionHandler->isFinished())
		TheShell->shutdownComplete( layout );

}  // end SaveLoadMenuUpdate 

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
WindowMsgHandledType SaveLoadMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 )
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
						GameWindow *button = TheWindowManager->winGetWindowFromId( parent, buttonBackKey );
						
						//Kris: Patch 1.01 - November 12, 2003
						//If you are in the game, then bring up the popup save menu, select a save game, click delete,
						//hit ESC (brings you back to menu), then hit save/load again, the delete confirmation is still up
						//and clicking on yes causes it to crash. So whenever we hit esc to leave this interface, kill
						//the confirmation, and re-enable the listbox and buttonFrame.
						deleteConfirm->winHide( TRUE );
						listboxGames->winEnable( TRUE );
						buttonFrame->winEnable( TRUE );

						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED, 
																								(WindowMsgData)button, buttonBackKey );

					}  // end if

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}  // end escape

			}  // end switch( key )

		}  // end char

	}  // end switch( msg )

	return MSG_IGNORED;
}

// ------------------------------------------------------------------------------------------------
/** Get the file info of the selected savegame file in the listbox */
// ------------------------------------------------------------------------------------------------
static AvailableGameInfo *getSelectedSaveFileInfo( GameWindow *window )
{

	// get the listbox
	//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( window, listboxGamesKey );
	DEBUG_ASSERTCRASH( listboxGames != NULL, ("SaveLoadMenuInit - Unable to find games listbox") );

	// which item is selected
	Int selected;
	GadgetListBoxGetSelected( listboxGames, &selected );

	// get the item data of the selection
	AvailableGameInfo *selectedGameInfo;
	selectedGameInfo = (AvailableGameInfo *)GadgetListBoxGetItemData( listboxGames, selected );

	return selectedGameInfo;

}  // end getSelectedSaveFileInfo

// ---------------------------------------------------con------------------------------------------
// ------------------------------------------------------------------------------------------------				// close the save/load menu
static void doLoadGame( void )
{

	// get listbox of games
	//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );
	DEBUG_ASSERTCRASH( listboxGames, ("doLoadGame: Unable to find game listbox") );

	// get selected game info
	AvailableGameInfo *selectedGameInfo = getSelectedSaveFileInfo( listboxGames );
	DEBUG_ASSERTCRASH( selectedGameInfo, ("doLoadGame: No selected game info found") );

	// when loading a game we also close the quit/esc menu for the user when in-game
	if( TheShell->isShellActive() == FALSE )
	{
		destroyQuitMenu();		
//		ToggleQuitMenu();
//		TheTransitionHandler->remove("QuitNoSave");
//		TheTransitionHandler->remove("QuitFull");
	}
	else
	{
		TheTransitionHandler->remove("MainMenuLoadReplayMenu");
		TheTransitionHandler->remove("MainMenuLoadReplayMenuBack");
		TheGameLogic->prepareNewGame( GAME_SINGLE_PLAYER, DIFFICULTY_NORMAL, 0 );
	}

	//
	// load game, note the *copy* of the selected game info is passed here because we will
	// loose these allocated user data pointers attached as listbox item data when the
	// engine resets
	//
	if (TheGameState->loadGame( *selectedGameInfo ) != SC_OK)
	{
		if (TheGameLogic->isInGame())
			TheGameLogic->clearGameData( FALSE );
		TheGameEngine->reset();
		TheShell->showShell(TRUE);
	}

}  // end doLoadGame

// ------------------------------------------------------------------------------------------------
/** Close the save/load menu */
// ------------------------------------------------------------------------------------------------
static void closeSaveMenu( GameWindow *window )
{
	
	if(isPopup)
	{
		WindowLayout *saveLoadMenuLayout = window->winGetLayout();
		if( saveLoadMenuLayout )
			saveLoadMenuLayout->hide( TRUE );	
	}
	else
		TheShell->hideShell();

}  // end closeSaveMenu

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
static void setEditDescription( GameWindow *editControl )
{
	UnicodeString defaultDesc;
	Campaign *campaign = TheCampaignManager->getCurrentCampaign();

	//
	// if we have a campaign we will use a default description that describes the 
	// location and map in the campaign nicely, otherwise we will default to just
	// the map name (which is really only used in debug)
	//
	if( campaign )
		defaultDesc.format( L"%s %d", 
												TheGameText->fetch( campaign->m_campaignNameLabel ).str(),
												TheCampaignManager->getCurrentMissionNumber() + 1 );
	else
	{
		const char *mapName = TheGlobalData->m_mapName.reverseFind( '\\' );

		if( mapName )
			defaultDesc.format( L"%S", mapName + 1 );
		else
			defaultDesc.format( L"%S", TheGlobalData->m_mapName.str() );
		
		//Keep the extension out of the descriptive name.
		if( (defaultDesc.getLength() >= 4)  &&  (defaultDesc.getCharAt(defaultDesc.getLength()-4) == '.') )
		{
			defaultDesc.truncateBy(4);
		}

	}  // end else

	// set into edit control
	GadgetTextEntrySetText( editControl, defaultDesc );

}  // end setEditDescription

//----------------------------------------------------------------------------------------------
static void processLoadButtonPress(GameWindow *window) 
{
	// get the filename of the selected savegame in the listbox
	AvailableGameInfo *selectedGameInfo = getSelectedSaveFileInfo( window );
	if( selectedGameInfo )
	{

		//
		// if we're in the shell we do not need a confirmation dialog that states we will
		// lose the current loaded game data cause we're not in a game
		//
		if( TheShell->isShellActive() == TRUE )
		{

			// just close the menu and do the load game logic
			closeSaveMenu( window );
			doLoadGame();

		}  // end if
		else
		{

			//GameWindow *buttonFrame  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );
			//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );
			//GameWindow *loadConfirm  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:LoadConfirmParent" ) );

			// disable listbox and buttons
			listboxGames->winEnable( FALSE );
			buttonFrame->winEnable( FALSE );

			// show the load confirm dialog
			loadConfirm->winHide( FALSE );

		}  // end else

	}  // end if
}

//-------------------------------------------------------------------------------------------------
/** SaveLoad menu system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType SaveLoadMenuSystem( GameWindow *window, UnsignedInt msg, 
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

			break;

		}  // end case

    //----------------------------------------------------------------------------------------------
    case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			break;

		}  // end input

    //----------------------------------------------------------------------------------------------
		case GLM_DOUBLE_CLICKED:
			{
				GameWindow *control = (GameWindow *)mData1;
				GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( window, listboxGamesKey );
				DEBUG_ASSERTCRASH( listboxGames != NULL, ("SaveLoadMenuInit - Unable to find games listbox") );

				if (listboxGames != NULL) {
					int rowSelected = mData2;
					GadgetListBoxSetSelected(listboxGames, rowSelected);

					if (control == listboxGames)
					{
						processLoadButtonPress(window);
					}
				}
				break;
			}

		// --------------------------------------------------------------------------------------------
		case GLM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;

			GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( window, listboxGamesKey );
			DEBUG_ASSERTCRASH( listboxGames != NULL, ("SaveLoadMenuInit - Unable to find games listbox") );

			//
			// handle games listbox, when certain items are selected in the listbox only some
			// commands are available
			//
			if( control == listboxGames )
				updateMenuActions();

			break;

		}  // end selected

    //---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();
      
      if( controlID == buttonLoadKey )
      {
				processLoadButtonPress(window);
      }  // end if
      else if( controlID == buttonSaveKey )
      {

				// sanity
				DEBUG_ASSERTCRASH( currentLayoutType == SLLT_SAVE_AND_LOAD ||
													 currentLayoutType == SLLT_SAVE_ONLY,
													 ("SaveLoadMenuSystem - layout type '%d' does not allow saving",
													 currentLayoutType) );
													 	
				// get save file info
				AvailableGameInfo *selectedGameInfo = getSelectedSaveFileInfo( window );

				// if there is no file info, this is a new game
				if( selectedGameInfo == NULL )
				{

					// show the save description window
					//GameWindow *saveDesc = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:SaveDescParent" ) );
					saveDesc->winHide( FALSE );

					// set the description text entry field to default value
					//GameWindow *editDesc = TheWindowManager->winGetWindowFromId( saveDesc, NAMEKEY( "PopupSaveLoad.wnd:EntryDesc" ) );
					setEditDescription( editDesc );

					// disable the listbox of games
					//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );
					listboxGames->winEnable( FALSE );

					// disable the frame window of buttons for the main save/load menu
					//GameWindow *buttonFrame  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );
					//buttonFrame->winEnable( FALSE );

					TheWindowManager->winSetFocus(editDesc);

				}  // end if
				else
				{

					// disable listbox of games
					//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );
					listboxGames->winEnable( FALSE );

					// disable and therefore lock out main save/load menu buttons
					//GameWindow *buttonFrame  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );
					buttonFrame->winEnable( FALSE );

					// show the save save confirm
					//GameWindow *overwriteConfirm  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:OverwriteConfirmParent" ) );
					overwriteConfirm->winHide( FALSE );

				}  // end else

      }
			else if( controlID == buttonDeleteKey )
			{

				// which item is selected in the game listbox
				//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );
				Int selected;
				GadgetListBoxGetSelected( listboxGames, &selected );
				AvailableGameInfo *selectedGameInfo;
				selectedGameInfo = (AvailableGameInfo *)GadgetListBoxGetItemData( listboxGames, selected );

				// delete file
				if( selectedGameInfo )
				{

					// disable games listbox
					listboxGames->winEnable( FALSE );

					// disable menu buttons
					//GameWindow *buttonFrame = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );
					buttonFrame->winEnable( FALSE );

					// unhide confirmation dialog
					//GameWindow *deleteConfirm = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:DeleteConfirmParent" ) );
					deleteConfirm->winHide( FALSE );

				}  // end if
								
			}  // end else if
			else if( controlID == buttonBackKey )
			{
				if(isPopup)
				{				
					// close the save/load menu
					closeSaveMenu( window );
				}
				else
				{
					TheShell->pop();
				}

			}  // end if
			else if( controlID == buttonDeleteConfirm || controlID == buttonDeleteCancel )
			{
				//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );

				// delete if confirm
				if( controlID == buttonDeleteConfirm )
				{

					// which item is selected in the game listbox
					Int selected;
					GadgetListBoxGetSelected( listboxGames, &selected );
					AvailableGameInfo *selectedGameInfo;
					selectedGameInfo = (AvailableGameInfo *)GadgetListBoxGetItemData( listboxGames, selected );

					// construct path to filename
					AsciiString filepath = TheGameState->getFilePathInSaveDirectory(selectedGameInfo->filename);

					// delete the file
					DeleteFile( filepath.str() );
					
					// repopulate the listbox
					TheGameState->populateSaveGameListbox( listboxGames, currentLayoutType );

				}  // end if

				// hide the confirm dialog
				//GameWindow *deleteConfirm = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:DeleteConfirmParent" ) );
				deleteConfirm->winHide( TRUE );

				// enable listbox of games
				listboxGames->winEnable( TRUE );

				// enable menu actions pane
				//GameWindow *buttonFrame = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );
				buttonFrame->winEnable( TRUE );
				updateMenuActions();

			}  // end if
			else if( controlID == buttonOverwriteCancel || controlID == buttonOverwriteConfirm )
			{
				//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );

				// hide save confirm dialog
				//GameWindow *overwriteConfirm  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:OverwriteConfirmParent" ) );
				overwriteConfirm->winHide( TRUE );

				// if saving, do the save for the selected listbox item
				if( controlID == buttonOverwriteConfirm )
				{

					// show the save description window
//					GameWindow *saveDesc = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:SaveDescParent" ) );
//					saveDesc->winHide( FALSE );

					// get save game info for the currently selected game in the listbox
					Int selected;
					GadgetListBoxGetSelected( listboxGames, &selected );

					// get the item data of the selection
					AvailableGameInfo *selectedGameInfo;
					selectedGameInfo = (AvailableGameInfo *)GadgetListBoxGetItemData( listboxGames, selected );
					DEBUG_ASSERTCRASH( selectedGameInfo, ("SaveLoadMenuSystem: Internal error, listbox entry to overwrite game has no item data set into listbox element") );

					// enable the listbox of games
					//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );
					listboxGames->winEnable( TRUE );

					// enable the frame window of buttons for the main save/load menu
					//GameWindow *buttonFrame  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );
					buttonFrame->winEnable( TRUE );
					updateMenuActions();
					
					//Added By Sadullah Nader
					//Fix for bug
					// close save menuu
					closeSaveMenu( window );
					
					//
					// given the context of this menu figure out which type of save game we're acutally
					// saving right now.  As it turns out, when this menu is used in the save only
					// mode it means that the save is a mission save between maps because you can only
					// save the game between maps and can of course not load one
					//
					SaveFileType fileType;
					if( currentLayoutType == SLLT_SAVE_AND_LOAD )
						fileType = SAVE_FILE_TYPE_NORMAL;
					else
						fileType = SAVE_FILE_TYPE_MISSION;

					// save the game
					AsciiString filename;
					filename = selectedGameInfo->filename;
					TheGameState->saveGame( filename, selectedGameInfo->saveGameInfo.description, fileType );

/*
					// set the description text entry field to default value
					GameWindow *editDesc = TheWindowManager->winGetWindowFromId( saveDesc, NAMEKEY( "PopupSaveLoad.wnd:EntryDesc" ) );
					setEditDescription( editDesc );
*/
				}  // end if
				else if( controlID == buttonOverwriteCancel )
				{
					//GameWindow *buttonFrame  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );

					// enable buttons and list box on main parent
					buttonFrame->winEnable( TRUE );
					updateMenuActions();
					listboxGames->winEnable( TRUE );

				}  // end else if

			}  // end else if
			else if( controlID == buttonSaveDescConfirm )
			{

				// get description window
				//GameWindow *saveDesc = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:SaveDescParent" ) );

				// get description text
				//GameWindow *entryDesc = TheWindowManager->winGetWindowFromId( saveDesc, NAMEKEY( "PopupSaveLoad.wnd:EntryDesc" ) );
				UnicodeString desc = GadgetTextEntryGetText( editDesc );

				// hide desc window
				saveDesc->winHide( TRUE );

				// enable the listbox of games
				//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );
				listboxGames->winEnable( TRUE );

				// enable the frame window of buttons for the main save/load menu
				//GameWindow *buttonFrame  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );
				buttonFrame->winEnable( TRUE );
				updateMenuActions();

				// close save menuu
				closeSaveMenu( window );

				// get save filename
				AvailableGameInfo *selectedGameInfo = getSelectedSaveFileInfo( listboxGames );

				//
				// given the context of this menu figure out which type of save game we're acutally
				// saving right now.  As it turns out, when this menu is used in the save only
				// mode it means that the save is a mission save between maps because you can only
				// save the game between maps and can of course not load one
				//
				SaveFileType fileType;
				if( currentLayoutType == SLLT_SAVE_AND_LOAD )
					fileType = SAVE_FILE_TYPE_NORMAL;
				else
					fileType = SAVE_FILE_TYPE_MISSION;

				// save the game
				AsciiString filename;
				if( selectedGameInfo )
					filename = selectedGameInfo->filename;
				TheGameState->saveGame( filename, desc, fileType );

			}  // end else if
			else if( controlID == buttonSaveDescCancel )
			{

				// hide the desc window
				//GameWindow *saveDesc = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:SaveDescParent" ) );
				saveDesc->winHide( TRUE );

				// enable the listbox of games
				//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );
				listboxGames->winEnable( TRUE );

				// enable the frame window of buttons for the main save/load menu
				//GameWindow *buttonFrame  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );
				buttonFrame->winEnable( TRUE );
				updateMenuActions();

			}  // end else if
			else if( controlID == buttonLoadConfirm || controlID == buttonLoadCancel )
			{

				// hide confirm dialog
				//GameWindow *loadConfirm  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:LoadConfirmParent" ) );
				loadConfirm->winHide( TRUE );

				// enable the listbox of games again
				//GameWindow *listboxGames = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:ListboxGames" ) );
				listboxGames->winEnable( TRUE );

				// enable the main save/load menu button controls
				//GameWindow *buttonFrame  = TheWindowManager->winGetWindowFromId( parent, NAMEKEY( "PopupSaveLoad.wnd:MenuButtonFrame" ) );
				buttonFrame->winEnable( TRUE );
				updateMenuActions();

				
				

				// do the load game
				if( controlID == buttonLoadConfirm )
				{
					//Moved by Sadullah Nader
					//moved to fix the 
					// close save/load layout menu
					closeSaveMenu( window );
					doLoadGame();
				}

			}  // end else if
	
			break;

		}  // end selected

		default:
			return MSG_IGNORED;

	}  // end switch

	return MSG_HANDLED;

}
