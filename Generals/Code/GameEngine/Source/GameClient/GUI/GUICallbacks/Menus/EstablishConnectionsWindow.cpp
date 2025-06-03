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

///// EstablishConnectionsWindow.cpp /////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine


#include "GameClient/GameWindowManager.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/EstablishConnectionsMenu.h"
#include "GameNetwork/GUIUtil.h"
#include "GameNetwork/GameSpy/StagingRoomGameInfo.h"

static WindowLayout *establishConnectionsLayout;

static NameKeyType buttonQuitID = NAMEKEY_INVALID;
static NameKeyType staticPlayer1NameID = NAMEKEY_INVALID;
static NameKeyType staticPlayer2NameID = NAMEKEY_INVALID;
static NameKeyType staticPlayer3NameID = NAMEKEY_INVALID;
static NameKeyType staticPlayer4NameID = NAMEKEY_INVALID;
static NameKeyType staticPlayer5NameID = NAMEKEY_INVALID;
static NameKeyType staticPlayer6NameID = NAMEKEY_INVALID;
static NameKeyType staticPlayer7NameID = NAMEKEY_INVALID;

static NameKeyType staticPlayer1StatusID = NAMEKEY_INVALID;
static NameKeyType staticPlayer2StatusID = NAMEKEY_INVALID;
static NameKeyType staticPlayer3StatusID = NAMEKEY_INVALID;
static NameKeyType staticPlayer4StatusID = NAMEKEY_INVALID;
static NameKeyType staticPlayer5StatusID = NAMEKEY_INVALID;
static NameKeyType staticPlayer6StatusID = NAMEKEY_INVALID;
static NameKeyType staticPlayer7StatusID = NAMEKEY_INVALID;

static GameWindow *buttonQuitWindow = NULL;
static GameWindow *staticPlayer1Name = NULL;
static GameWindow *staticPlayer2Name = NULL;
static GameWindow *staticPlayer3Name = NULL;
static GameWindow *staticPlayer4Name = NULL;
static GameWindow *staticPlayer5Name = NULL;
static GameWindow *staticPlayer6Name = NULL;
static GameWindow *staticPlayer7Name = NULL;

static GameWindow *staticPlayer1Status = NULL;
static GameWindow *staticPlayer2Status = NULL;
static GameWindow *staticPlayer3Status = NULL;
static GameWindow *staticPlayer4Status = NULL;
static GameWindow *staticPlayer5Status = NULL;
static GameWindow *staticPlayer6Status = NULL;
static GameWindow *staticPlayer7Status = NULL;

static const char *layoutFilename = "GameSpyGameOptionsMenu.wnd";
static const char *parentName = "GameSpyGameOptionsMenuParent";
static const char *gadgetsToHide[] =
{
	"MapWindow",
	"StaticTextGameName",
	"StaticTextTeam",
	"StaticTextFaction",
	"StaticTextColor",
	"StaticTextPlayers",
	"TextEntryMapDisplay",
	"ButtonSelectMap",
	"ButtonStart",
	"StaticTextMapPreview",
	NULL // keep this last
};
static const char *perPlayerGadgetsToHide[] =
{
	"ComboBoxTeam",
	"ComboBoxColor",
	"ComboBoxPlayerTemplate",
	"ComboBoxPlayer",
	"ButtonAccept",
	"GenericPing",
	//"ButtonStartPosition",
	NULL // keep this last
};

static const char *qmlayoutFilename = "WOLQuickMatchMenu.wnd";
static const char *qmparentName = "WOLQuickMatchMenuParent";
static const char *qmgadgetsToHide[] =
{
	"StaticTextTitle",
	"ButtonBack",
	"ButtonOptions",
	"ButtonBuddies",
	"ButtonWiden",
	"ButtonStop",
	"ButtonStart",
	NULL // keep this last
};
static const char *qmperPlayerGadgetsToHide[] =
{
	//"ButtonStartPosition",
	NULL // keep this last
};

static void showGameSpyGameOptionsUnderlyingGUIElements( Bool show )
{
	ShowUnderlyingGUIElements( show, layoutFilename, parentName, gadgetsToHide, perPlayerGadgetsToHide );
	
}
static void showGameSpyQMUnderlyingGUIElements( Bool show )
{
	ShowUnderlyingGUIElements( show, qmlayoutFilename, qmparentName, qmgadgetsToHide, qmperPlayerGadgetsToHide );
}

static void InitEstablishConnectionsDialog( void ) {
	buttonQuitID = TheNameKeyGenerator->nameToKey( AsciiString("EstablishConnectionsScreen.wnd:ButtonQuit") );
	buttonQuitWindow = TheWindowManager->winGetWindowFromId(NULL, buttonQuitID);
}

void ShowEstablishConnectionsWindow( void ) {
	if (establishConnectionsLayout == NULL) {
		establishConnectionsLayout = TheWindowManager->winCreateLayout( "Menus/EstablishConnectionsScreen.wnd" );
		InitEstablishConnectionsDialog();
	}
	establishConnectionsLayout->hide(FALSE);
	TheWindowManager->winSetFocus(establishConnectionsLayout->getFirstWindow());
	if (!TheGameSpyGame->isQMGame())
	{
		showGameSpyGameOptionsUnderlyingGUIElements(FALSE);
	}
	else
	{
		showGameSpyQMUnderlyingGUIElements(FALSE);
	}
}

void HideEstablishConnectionsWindow( void ) {
	if (establishConnectionsLayout == NULL) {
//		establishConnectionsLayout = TheWindowManager->winCreateLayout( "Menus/EstablishConnectionsScreen.wnd" );
//		InitEstablishConnectionsDialog();
		return;
	}
//	establishConnectionsLayout->hide(TRUE);
//	establishConnectionsLayout->hide(TRUE);
//	TheWindowManager->winDestroy(establishConnectionsLayout);
	establishConnectionsLayout->destroyWindows();
	deleteInstance(establishConnectionsLayout);
	establishConnectionsLayout = NULL;
	if (!TheGameSpyGame->isQMGame())
	{
		showGameSpyGameOptionsUnderlyingGUIElements(TRUE);
	}
	else
	{
		showGameSpyQMUnderlyingGUIElements(TRUE);
	}
}

WindowMsgHandledType EstablishConnectionsControlInput(GameWindow *window, UnsignedInt msg,
																											WindowMsgData mData1, WindowMsgData mData2) {

	return MSG_IGNORED;
}

WindowMsgHandledType EstablishConnectionsControlSystem(GameWindow *window, UnsignedInt msg,
																											 WindowMsgData mData1, WindowMsgData mData2) {

	switch (msg) {
		case GBM_SELECTED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if (controlID == buttonQuitID) {
					TheEstablishConnectionsMenu->abortGame();
				}
				break;
			}
	} // end switch
	return MSG_HANDLED;
}
