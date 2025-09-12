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

// FILE: PopupHostGame.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Jul 2002
//
//	Filename: 	PopupHostGame.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	Contains the Callbacks for the Host Game Popus
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GlobalData.h"
#include "Common/NameKeyGenerator.h"
#include "Common/version.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameText.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetListBox.h"
#include "GameNetwork/GameSpy/GSConfig.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpyOverlay.h"

#include "GameNetwork/GameSpy/LadderDefs.h"
#include "Common/CustomMatchPreferences.h"
#include "Common/LadderPreferences.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

static NameKeyType parentPopupID = NAMEKEY_INVALID;
static NameKeyType textEntryGameNameID = NAMEKEY_INVALID;
static NameKeyType buttonCreateGameID = NAMEKEY_INVALID;
static NameKeyType checkBoxAllowObserversID = NAMEKEY_INVALID;
static NameKeyType textEntryGameDescriptionID = NAMEKEY_INVALID;
static NameKeyType buttonCancelID = NAMEKEY_INVALID;
static NameKeyType textEntryLadderPasswordID = NAMEKEY_INVALID;
static NameKeyType comboBoxLadderNameID = NAMEKEY_INVALID;
static NameKeyType textEntryGamePasswordID = NAMEKEY_INVALID;
static NameKeyType checkBoxLimitArmiesID = NAMEKEY_INVALID;
static NameKeyType checkBoxUseStatsID = NAMEKEY_INVALID;

static GameWindow *parentPopup = NULL;
static GameWindow *textEntryGameName = NULL;
static GameWindow *buttonCreateGame = NULL;
static GameWindow *checkBoxAllowObservers = NULL;
static GameWindow *textEntryGameDescription = NULL;
static GameWindow *buttonCancel = NULL;
static GameWindow *comboBoxLadderName = NULL;
static GameWindow *textEntryLadderPassword = NULL;
static GameWindow *textEntryGamePassword = NULL;
static GameWindow *checkBoxLimitArmies = NULL;
static GameWindow *checkBoxUseStats = NULL;


void createGame( void );

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

// Ladders --------------------------------------------------------------------------------

static bool isPopulatingLadderBox = false;

void CustomMatchHideHostPopup(Bool hide)
{
	if (!parentPopup)
		return;

	parentPopup->winHide( hide );
}

void HandleCustomLadderSelection(Int ladderID)
{
	if (!parentPopup)
		return;

	CustomMatchPreferences pref;

	if (ladderID == 0)
	{
		pref.setLastLadder(AsciiString::TheEmptyString, 0);
		pref.write();
		return;
	}

	const LadderInfo *info = TheLadderList->findLadderByIndex(ladderID);
	if (!info)
	{
		pref.setLastLadder(AsciiString::TheEmptyString, 0);
	}
	else
	{
		pref.setLastLadder(info->address, info->port);
	}

	pref.write();
}

void PopulateCustomLadderListBox( GameWindow *win )
{
	if (!parentPopup || !win)
		return;

	isPopulatingLadderBox = true;

	CustomMatchPreferences pref;

	Color specialColor = GameSpyColor[GSCOLOR_MAP_SELECTED];
	Color normalColor = GameSpyColor[GSCOLOR_MAP_UNSELECTED];
	Color favoriteColor = GameSpyColor[GSCOLOR_MAP_UNSELECTED];
	Color localColor = GameSpyColor[GSCOLOR_MAP_UNSELECTED];
	Int index;
	GadgetListBoxReset( win );

	std::set<const LadderInfo *> usedLadders;

	// start with "No Ladder"
	index = GadgetListBoxAddEntryText( win, TheGameText->fetch("GUI:NoLadder"), normalColor, -1 );
	GadgetListBoxSetItemData( win, 0, index );

	// add the last ladder
	Int selectedPos = 0;
	AsciiString lastLadderAddr = pref.getLastLadderAddr();
	UnsignedShort lastLadderPort = pref.getLastLadderPort();
	const LadderInfo *info = TheLadderList->findLadder( lastLadderAddr, lastLadderPort );
	if (info && info->index > 0 && info->validCustom)
	{
		usedLadders.insert(info);
		index = GadgetListBoxAddEntryText( win, info->name, favoriteColor, -1 );
		GadgetListBoxSetItemData( win, (void *)(info->index), index );
		selectedPos = index;
	}

	// our recent ladders
	LadderPreferences ladPref;
	ladPref.loadProfile( TheGameSpyInfo->getLocalProfileID() );
	const LadderPrefMap recentLadders = ladPref.getRecentLadders();
	for (LadderPrefMap::const_iterator cit = recentLadders.begin(); cit != recentLadders.end(); ++cit)
	{
		AsciiString addr = cit->second.address;
		UnsignedShort port = cit->second.port;
		if (addr == lastLadderAddr && port == lastLadderPort)
			continue;
		const LadderInfo *info = TheLadderList->findLadder( addr, port );
		if (info && info->index > 0 && info->validCustom && usedLadders.find(info) == usedLadders.end())
		{
			usedLadders.insert(info);
			index = GadgetListBoxAddEntryText( win, info->name, favoriteColor, -1 );
			GadgetListBoxSetItemData( win, (void *)(info->index), index );
		}
	}

	// local ladders
	const LadderInfoList *lil = TheLadderList->getLocalLadders();
	LadderInfoList::const_iterator lit;
	for (lit = lil->begin(); lit != lil->end(); ++lit)
	{
		const LadderInfo *info = *lit;
		if (info && info->index < 0 && info->validCustom && usedLadders.find(info) == usedLadders.end())
		{
			usedLadders.insert(info);
			index = GadgetListBoxAddEntryText( win, info->name, localColor, -1 );
			GadgetListBoxSetItemData( win, (void *)(info->index), index );
		}
	}

	// special ladders
	lil = TheLadderList->getSpecialLadders();
	for (lit = lil->begin(); lit != lil->end(); ++lit)
	{
		const LadderInfo *info = *lit;
		if (info && info->index > 0 && info->validCustom && usedLadders.find(info) == usedLadders.end())
		{
			usedLadders.insert(info);
			index = GadgetListBoxAddEntryText( win, info->name, specialColor, -1 );
			GadgetListBoxSetItemData( win, (void *)(info->index), index );
		}
	}

	// standard ladders
	lil = TheLadderList->getStandardLadders();
	for (lit = lil->begin(); lit != lil->end(); ++lit)
	{
		const LadderInfo *info = *lit;
		if (info && info->index > 0 && info->validCustom && usedLadders.find(info) == usedLadders.end())
		{
			usedLadders.insert(info);
			index = GadgetListBoxAddEntryText( win, info->name, normalColor, -1 );
			GadgetListBoxSetItemData( win, (void *)(info->index), index );
		}
	}

	GadgetListBoxSetSelected( win, selectedPos );
	isPopulatingLadderBox = false;
}

void PopulateCustomLadderComboBox( void )
{
	if (!parentPopup || !comboBoxLadderName)
		return;

	isPopulatingLadderBox = true;

	CustomMatchPreferences pref;
	AsciiString userPrefFilename;
	Int localProfile = TheGameSpyInfo->getLocalProfileID();
	userPrefFilename.format("GeneralsOnline\\CustomPref%d.ini", localProfile);
	pref.load(userPrefFilename);

	std::set<const LadderInfo *> usedLadders;

	Color specialColor = GameSpyColor[GSCOLOR_MAP_SELECTED];
	Color normalColor = GameSpyColor[GSCOLOR_MAP_UNSELECTED];
	Int index;
	GadgetComboBoxReset( comboBoxLadderName );
	index = GadgetComboBoxAddEntry( comboBoxLadderName, TheGameText->fetch("GUI:NoLadder"), normalColor );
	GadgetComboBoxSetItemData( comboBoxLadderName, index, 0 );

	Int selectedPos = 0;
	AsciiString lastLadderAddr = pref.getLastLadderAddr();
	UnsignedShort lastLadderPort = pref.getLastLadderPort();
	const LadderInfo *info = TheLadderList->findLadder( lastLadderAddr, lastLadderPort );
	if (info && info->validCustom)
	{
		usedLadders.insert(info);
		index = GadgetComboBoxAddEntry( comboBoxLadderName, info->name, specialColor );
		GadgetComboBoxSetItemData( comboBoxLadderName, index, (void *)(info->index) );
		selectedPos = index;
	}

	LadderPreferences ladPref;
	ladPref.loadProfile( localProfile );
	const LadderPrefMap recentLadders = ladPref.getRecentLadders();
	for (LadderPrefMap::const_iterator cit = recentLadders.begin(); cit != recentLadders.end(); ++cit)
	{
		AsciiString addr = cit->second.address;
		UnsignedShort port = cit->second.port;
		if (addr == lastLadderAddr && port == lastLadderPort)
			continue;
		const LadderInfo *info = TheLadderList->findLadder( addr, port );
		if (info && info->validCustom && usedLadders.find(info) == usedLadders.end())
		{
			usedLadders.insert(info);
			index = GadgetComboBoxAddEntry( comboBoxLadderName, info->name, normalColor );
			GadgetComboBoxSetItemData( comboBoxLadderName, index, (void *)(info->index) );
		}
	}

	index = GadgetComboBoxAddEntry( comboBoxLadderName, TheGameText->fetch("GUI:ChooseLadder"), normalColor );
	GadgetComboBoxSetItemData( comboBoxLadderName, index, (void *)-1 );

	GadgetComboBoxSetSelectedPos( comboBoxLadderName, selectedPos );
	isPopulatingLadderBox = false;
}

// Window Functions -----------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/** Initialize the PopupHostGameInit menu */
//-------------------------------------------------------------------------------------------------
void PopupHostGameInit( WindowLayout *layout, void *userData )
{
	parentPopupID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:ParentHostPopUp"));
	parentPopup = TheWindowManager->winGetWindowFromId(NULL, parentPopupID);

	textEntryGameNameID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:TextEntryGameName"));
	textEntryGameName = TheWindowManager->winGetWindowFromId(parentPopup, textEntryGameNameID);
	UnicodeString name;
	name.translate(TheGameSpyInfo->getLocalName());
	GadgetTextEntrySetText(textEntryGameName, name);

	textEntryGameDescriptionID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:TextEntryGameDescription"));
	textEntryGameDescription = TheWindowManager->winGetWindowFromId(parentPopup, textEntryGameDescriptionID);
	GadgetTextEntrySetText(textEntryGameDescription, UnicodeString::TheEmptyString);

	textEntryLadderPasswordID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:TextEntryLadderPassword"));
	textEntryLadderPassword = TheWindowManager->winGetWindowFromId(parentPopup, textEntryLadderPasswordID);
	GadgetTextEntrySetText(textEntryLadderPassword, UnicodeString::TheEmptyString);

	textEntryGamePasswordID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:TextEntryGamePassword"));
	textEntryGamePassword = TheWindowManager->winGetWindowFromId(parentPopup, textEntryGamePasswordID);
	GadgetTextEntrySetText(textEntryGamePassword, UnicodeString::TheEmptyString);

	buttonCreateGameID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:ButtonCreateGame"));
	buttonCreateGame = TheWindowManager->winGetWindowFromId(parentPopup, buttonCreateGameID);

	buttonCancelID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:ButtonCancel"));
	buttonCancel = TheWindowManager->winGetWindowFromId(parentPopup, buttonCancelID);

	checkBoxAllowObserversID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:CheckBoxAllowObservers"));
	checkBoxAllowObservers = TheWindowManager->winGetWindowFromId(parentPopup, checkBoxAllowObserversID);
	CustomMatchPreferences customPref;
	GadgetCheckBoxSetChecked(checkBoxAllowObservers, customPref.allowsObservers());

	comboBoxLadderNameID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:ComboBoxLadderName"));
	comboBoxLadderName = TheWindowManager->winGetWindowFromId(parentPopup, comboBoxLadderNameID);
	if (comboBoxLadderName)
		GadgetComboBoxReset(comboBoxLadderName);
	PopulateCustomLadderComboBox();

  checkBoxUseStatsID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:CheckBoxUseStats"));
  checkBoxUseStats = TheWindowManager->winGetWindowFromId(parentPopup, checkBoxUseStatsID);
	Bool usingStats = customPref.getUseStats();
  GadgetCheckBoxSetChecked( checkBoxUseStats, usingStats );

	// limit armies is disallowed in "use stats" games
  checkBoxLimitArmiesID = TheNameKeyGenerator->nameToKey(AsciiString("PopupHostGame.wnd:CheckBoxLimitArmies"));
  checkBoxLimitArmies = TheWindowManager->winGetWindowFromId(parentPopup, checkBoxLimitArmiesID);
	checkBoxLimitArmies->winEnable(! usingStats );
  GadgetCheckBoxSetChecked( checkBoxLimitArmies, usingStats? FALSE : customPref.getFactionsLimited() );

	TheWindowManager->winSetFocus( parentPopup );
	TheWindowManager->winSetModal( parentPopup );

}


//-------------------------------------------------------------------------------------------------
/** PopupHostGameUpdate callback */
//-------------------------------------------------------------------------------------------------
void PopupHostGameUpdate( WindowLayout * layout, void *userData)
{
	if (GadgetCheckBoxIsChecked( checkBoxUseStats ))
	{
		checkBoxLimitArmies->winEnable( FALSE );
		GadgetCheckBoxSetChecked( checkBoxLimitArmies, FALSE );
	}
	else
	{
		checkBoxLimitArmies->winEnable( TRUE );
	}
}


//-------------------------------------------------------------------------------------------------
/** PopupHostGameInput callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType PopupHostGameInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
			UnsignedByte state = mData2;
//			if (buttonPushed)
//				break;

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
						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
																							(WindowMsgData)buttonCancel, buttonCancelID );

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
/** PopupHostGameSystem callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType PopupHostGameSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 )
{
  switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{

			break;

		}
    //---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			parentPopup = NULL;

			break;

		}

    //----------------------------------------------------------------------------------------------
    case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			break;

		}

    //----------------------------------------------------------------------------------------------
		case GEM_UPDATE_TEXT:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if ( controlID == textEntryGameNameID )
				{
					UnicodeString txtInput;

					// grab the game's name
					txtInput.set(GadgetTextEntryGetText( textEntryGameName ));

					// Clean up the text (remove leading/trailing chars, etc)
					const WideChar *c = txtInput.str();
					while (c && (iswspace(*c)))
						c++;

					if (c)
						txtInput = UnicodeString(c);
					else
						txtInput = UnicodeString::TheEmptyString;

					// Put the whitespace-free version in the box
					GadgetTextEntrySetText( textEntryGameName, txtInput );

				}
				break;
			}
    //---------------------------------------------------------------------------------------------
		case GCM_SELECTED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				Int pos = -1;
				GadgetComboBoxGetSelectedPos(control, &pos);

				if (controlID == comboBoxLadderNameID && !isPopulatingLadderBox)
				{
					if (pos >= 0)
					{
						Int ladderID = (Int)GadgetComboBoxGetItemData(control, pos);
						if (ladderID < 0)
						{
							// "Choose a ladder" selected - open overlay
							PopulateCustomLadderComboBox(); // this restores the non-"Choose a ladder" selection
							GameSpyOpenOverlay( GSOVERLAY_LADDERSELECT );
						}
					}
				}
				break;
			}

    //---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

      if( controlID == buttonCancelID )
			{
				parentPopup = NULL;
				GameSpyCloseOverlay(GSOVERLAY_GAMEOPTIONS);
				SetLobbyAttemptHostJoin( FALSE );
			}
			else if( controlID == buttonCreateGameID)
			{
				UnicodeString name;
				name = GadgetTextEntryGetText(textEntryGameName);
				name.trim();
				if(name.getLength() <= 0)
				{
					name.translate(TheGameSpyInfo->getLocalName());
					GadgetTextEntrySetText(textEntryGameName, name);
				}
				createGame();
				parentPopup = NULL;
				GameSpyCloseOverlay(GSOVERLAY_GAMEOPTIONS);
			}
			break;
		}
		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}


//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

void createGame( void )
{
	TheGameSpyInfo->setCurrentGroupRoom(0);
	PeerRequest req;
	UnicodeString gameName = GadgetTextEntryGetText(textEntryGameName);
	req.peerRequestType = PeerRequest::PEERREQUEST_CREATESTAGINGROOM;
	req.text = gameName.str();
	TheGameSpyGame->setGameName(gameName);
	AsciiString passwd;
	passwd.translate(GadgetTextEntryGetText(textEntryGamePassword));
	req.password = passwd.str();
	CustomMatchPreferences customPref;
	Bool aO = GadgetCheckBoxIsChecked(checkBoxAllowObservers);
  Bool limitArmies = GadgetCheckBoxIsChecked( checkBoxLimitArmies );
  Bool useStats = GadgetCheckBoxIsChecked( checkBoxUseStats );
	customPref.setAllowsObserver(aO);
  customPref.setFactionsLimited( limitArmies );
  customPref.setUseStats( useStats );
	customPref.write();
	req.stagingRoomCreation.allowObservers = aO;
  req.stagingRoomCreation.useStats = useStats;
	TheGameSpyGame->setAllowObservers(aO);
  TheGameSpyGame->setOldFactionsOnly( limitArmies );
  TheGameSpyGame->setUseStats( useStats );
	req.stagingRoomCreation.exeCRC = TheGlobalData->m_exeCRC;
	req.stagingRoomCreation.iniCRC = TheGlobalData->m_iniCRC;
	req.stagingRoomCreation.gameVersion = TheGameSpyInfo->getInternalIP();
	req.stagingRoomCreation.restrictGameList = TheGameSpyConfig->restrictGamesToLobby();

	Int ladderSelectPos = -1, ladderID = -1;
	GadgetComboBoxGetSelectedPos(comboBoxLadderName, &ladderSelectPos);
	req.ladderIP = "localhost";
	req.stagingRoomCreation.ladPort = 0;
	if (ladderSelectPos >= 0)
	{
		ladderID = (Int)GadgetComboBoxGetItemData(comboBoxLadderName, ladderSelectPos);
		if (ladderID != 0)
		{
			// actual ladder
			const LadderInfo *info = TheLadderList->findLadderByIndex(ladderID);
			if (info)
			{
				req.ladderIP = info->address.str();
				req.stagingRoomCreation.ladPort = info->port;
			}
		}
	}
	TheGameSpyGame->setLadderIP(req.ladderIP.c_str());
	TheGameSpyGame->setLadderPort(req.stagingRoomCreation.ladPort);
	req.hostPingStr = TheGameSpyInfo->getPingString().str();

	TheGameSpyPeerMessageQueue->addRequest(req);
}
