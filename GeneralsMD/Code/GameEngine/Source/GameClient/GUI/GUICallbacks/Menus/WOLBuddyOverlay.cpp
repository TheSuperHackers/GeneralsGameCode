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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: WOLBuddyOverlay.cpp
// Author: Chris Huybregts, November 2001
// Description: Lan Lobby Menu
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/AudioEventRTS.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "GameClient/GameText.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetRadioButton.h"
#include "GameClient/Display.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/BuddyDefs.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/LobbyUtils.h"
#include "GameNetwork/GameSpy/PersistentStorageDefs.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/GameSpy/ThreadUtils.h"

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////


// window ids ------------------------------------------------------------------------------
static NameKeyType parentID = NAMEKEY_INVALID;
static NameKeyType buttonHideID = NAMEKEY_INVALID;
static NameKeyType buttonAddBuddyID = NAMEKEY_INVALID;
static NameKeyType buttonDeleteBuddyID = NAMEKEY_INVALID;
static NameKeyType textEntryID = NAMEKEY_INVALID;
static NameKeyType listboxBuddyID = NAMEKEY_INVALID;
static NameKeyType listboxChatID = NAMEKEY_INVALID;
static NameKeyType buttonAcceptBuddyID = NAMEKEY_INVALID;
static NameKeyType buttonDenyBuddyID = NAMEKEY_INVALID;
static NameKeyType radioButtonBuddiesID = NAMEKEY_INVALID;
static NameKeyType radioButtonIgnoreID = NAMEKEY_INVALID;
static NameKeyType parentBuddiesID = NAMEKEY_INVALID;
static NameKeyType parentIgnoreID = NAMEKEY_INVALID;
static NameKeyType listboxIgnoreID = NAMEKEY_INVALID;
static NameKeyType buttonNotificationID = NAMEKEY_INVALID;


// Window Pointers ------------------------------------------------------------------------
static GameWindow *parent = NULL;
static GameWindow *buttonHide = NULL;
static GameWindow *buttonAddBuddy = NULL;
static GameWindow *buttonDeleteBuddy = NULL;
static GameWindow *textEntry = NULL;
static GameWindow *listboxBuddy = NULL;
static GameWindow *listboxChat = NULL;
static GameWindow *buttonAcceptBuddy = NULL;
static GameWindow *buttonDenyBuddy = NULL;
static GameWindow *radioButtonBuddies = NULL;
static GameWindow *radioButtonIgnore = NULL;
static GameWindow *parentBuddies = NULL;
static GameWindow *parentIgnore = NULL;
static GameWindow *listboxIgnore = NULL;

static Bool isOverlayActive = false;
void insertChat( BuddyMessage msg );
// RightClick pointers ---------------------------------------------------------------------
static GameWindow *rcMenu = NULL;
static WindowLayout *noticeLayout = NULL;
static UnsignedInt noticeExpires = 0;
enum { NOTIFICATION_EXPIRES = 3000 };

void setUnignoreText( WindowLayout *layout, AsciiString nick, GPProfile id);
void refreshIgnoreList( void );
void showNotificationBox( AsciiString nick, UnicodeString message);
void deleteNotificationBox( void );
static Bool lastNotificationWasStatus = FALSE;
static Int numOnlineInNotification = 0;

class BuddyControls
{
public:
	BuddyControls(void );
	GameWindow *listboxChat;
	NameKeyType listboxChatID;

	GameWindow *listboxBuddies;
	NameKeyType listboxBuddiesID;

	GameWindow *textEntryEdit;
	NameKeyType textEntryEditID;
	Bool isInit;
};

static BuddyControls buddyControls;
BuddyControls::BuddyControls(	void )
{
	listboxChat = NULL;
	listboxChatID = NAMEKEY_INVALID;
	listboxBuddies = NULL;
	listboxBuddiesID = NAMEKEY_INVALID;
	textEntryEdit = NULL;
	textEntryEditID = NAMEKEY_INVALID;
	isInit = FALSE;
}
// At this point I don't give a damn about how good this way is.  I'm doing it anyway.
enum
{
	BUDDY_RESETALL_CRAP = -1,
	BUDDY_WINDOW_BUDDIES = 0,
	BUDDY_WINDOW_DIPLOMACY,
	BUDDY_WINDOW_WELCOME_SCREEN,
};

void InitBuddyControls(Int type)
{
	if(!TheGameSpyInfo)
	{
		buddyControls.textEntryEditID = NAMEKEY_INVALID;
		buddyControls.textEntryEdit = NULL;
		buddyControls.listboxBuddiesID = NAMEKEY_INVALID;
		buddyControls.listboxChatID = NAMEKEY_INVALID;
		buddyControls.listboxBuddies = NULL;
		buddyControls.listboxChat = NULL;
		buddyControls.isInit = FALSE;
		return;
	}
	switch (type) {
	case BUDDY_RESETALL_CRAP:
		buddyControls.textEntryEditID = NAMEKEY_INVALID;
		buddyControls.textEntryEdit = NULL;
		buddyControls.listboxBuddiesID = NAMEKEY_INVALID;
		buddyControls.listboxChatID = NAMEKEY_INVALID;
		buddyControls.listboxBuddies = NULL;
		buddyControls.listboxChat = NULL;
		buddyControls.isInit = FALSE;
	break;
	case BUDDY_WINDOW_BUDDIES:
		buddyControls.textEntryEditID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:TextEntryChat" ) );
		buddyControls.textEntryEdit = TheWindowManager->winGetWindowFromId(NULL,  buddyControls.textEntryEditID);
		buddyControls.listboxBuddiesID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:ListboxBuddies" ) );
		buddyControls.listboxChatID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:ListboxBuddyChat" ) );
		buddyControls.listboxBuddies = TheWindowManager->winGetWindowFromId( NULL,  buddyControls.listboxBuddiesID );
		buddyControls.listboxChat = TheWindowManager->winGetWindowFromId( NULL,  buddyControls.listboxChatID);
		GadgetTextEntrySetText(buddyControls.textEntryEdit, UnicodeString::TheEmptyString);
		buddyControls.isInit = TRUE;
		break;
	case BUDDY_WINDOW_DIPLOMACY:
		buddyControls.textEntryEditID = TheNameKeyGenerator->nameToKey( AsciiString( "Diplomacy.wnd:TextEntryChat" ) );
		buddyControls.textEntryEdit = TheWindowManager->winGetWindowFromId(NULL,  buddyControls.textEntryEditID);
		buddyControls.listboxBuddiesID = TheNameKeyGenerator->nameToKey( AsciiString( "Diplomacy.wnd:ListboxBuddies" ) );
		buddyControls.listboxChatID = TheNameKeyGenerator->nameToKey( AsciiString( "Diplomacy.wnd:ListboxBuddyChat" ) );
		buddyControls.listboxBuddies = TheWindowManager->winGetWindowFromId( NULL,  buddyControls.listboxBuddiesID );
		buddyControls.listboxChat = TheWindowManager->winGetWindowFromId( NULL,  buddyControls.listboxChatID);
		GadgetTextEntrySetText(buddyControls.textEntryEdit, UnicodeString::TheEmptyString);
		buddyControls.isInit = TRUE;
		break;
	case BUDDY_WINDOW_WELCOME_SCREEN:
		break;
	default:
		DEBUG_ASSERTCRASH(FALSE, ("Well, you really shouldn't have gotten here, if you really care about GUI Bugs, search for this string, you you don't care, call chris (who probably doesn't care either"));
	}

}

WindowMsgHandledType BuddyControlSystem( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2)
{
	if(!TheGameSpyInfo || TheGameSpyInfo->getLocalProfileID() == 0 || !buddyControls.isInit)
	{
		return MSG_IGNORED;
	}

	switch( msg )
	{
		case GLM_RIGHT_CLICKED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if( controlID == buddyControls.listboxBuddiesID )
				{
					RightClickStruct *rc = (RightClickStruct *)mData2;
					WindowLayout *rcLayout;
					if(rc->pos < 0)
						break;

					GPProfile profileID = (GPProfile)GadgetListBoxGetItemData(control, rc->pos, 0);
					RCItemType itemType = (RCItemType)(Int)GadgetListBoxGetItemData(control, rc->pos, 1);
					UnicodeString nick = GadgetListBoxGetText(control, rc->pos);

					GadgetListBoxSetSelected(control, rc->pos);
					if (itemType == ITEM_BUDDY)
						rcLayout = TheWindowManager->winCreateLayout(AsciiString("Menus/RCBuddiesMenu.wnd"));
					else if (itemType == ITEM_REQUEST)
						rcLayout = TheWindowManager->winCreateLayout(AsciiString("Menus/RCBuddyRequestMenu.wnd"));
					else
						rcLayout = TheWindowManager->winCreateLayout(AsciiString("Menus/RCNonBuddiesMenu.wnd"));
					rcMenu = rcLayout->getFirstWindow();
					rcMenu->winGetLayout()->runInit();
					rcMenu->winBringToTop();
					rcMenu->winHide(FALSE);


					ICoord2D rcSize, rcPos;
					rcMenu->winGetSize(&rcSize.x, &rcSize.y);
					rcPos.x = rc->mouseX;
					rcPos.y = rc->mouseY;
					if(rc->mouseX + rcSize.x > TheDisplay->getWidth())
						rcPos.x = TheDisplay->getWidth() - rcSize.x;
					if(rc->mouseY + rcSize.y > TheDisplay->getHeight())
						rcPos.y = TheDisplay->getHeight() - rcSize.y;
					rcMenu->winSetPosition(rcPos.x, rcPos.y);


					GameSpyRCMenuData *rcData = NEW GameSpyRCMenuData;
					rcData->m_id = profileID;
					rcData->m_nick.translate(nick);
					rcData->m_itemType = itemType;
					setUnignoreText(rcLayout, rcData->m_nick, rcData->m_id);
					rcMenu->winSetUserData((void *)rcData);
					TheWindowManager->winSetLoneWindow(rcMenu);
				}
				else
					return MSG_IGNORED;
				break;
			}
			case GEM_EDIT_DONE:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				if(controlID != buddyControls.textEntryEditID)
					return MSG_IGNORED;

				// see if someone's selected
				Int selected = -1;
				GadgetListBoxGetSelected(buddyControls.listboxBuddies, &selected);
				if (selected >= 0)
				{
					GPProfile selectedProfile = (GPProfile)GadgetListBoxGetItemData(buddyControls.listboxBuddies, selected);
					BuddyInfoMap *m = TheGameSpyInfo->getBuddyMap();
					BuddyInfoMap::iterator recipIt = m->find(selectedProfile);
					if (recipIt == m->end())
						break;

					DEBUG_LOG(("Trying to send a buddy message to %d.", selectedProfile));
					if (TheGameSpyGame && TheGameSpyGame->isInGame() && TheGameSpyGame->isGameInProgress() &&
						!ThePlayerList->getLocalPlayer()->isPlayerActive())
					{
						DEBUG_LOG(("I'm dead - gotta look for cheats."));
						for (Int i=0; i<MAX_SLOTS; ++i)
						{
							DEBUG_LOG(("Slot[%d] profile is %d", i, TheGameSpyGame->getGameSpySlot(i)->getProfileID()));
							if (TheGameSpyGame->getGameSpySlot(i)->getProfileID() == selectedProfile)
							{
								// can't send to someone in our game if we're dead/observing.  security breach and all that.  no seances for you.
								if (buddyControls.listboxChat)
								{
									GadgetListBoxAddEntryText( buddyControls.listboxChat, TheGameText->fetch("Buddy:CantTalkToIngameBuddy"),
										GameSpyColor[GSCOLOR_DEFAULT], -1, -1 );
								}
								return MSG_HANDLED;
							}
						}
					}

					// read the user's input and clear the entry box
					UnicodeString txtInput;
					txtInput.set(GadgetTextEntryGetText( buddyControls.textEntryEdit ));
					GadgetTextEntrySetText(buddyControls.textEntryEdit, UnicodeString::TheEmptyString);
					txtInput.trim();
					if (!txtInput.isEmpty())
					{
						// Send the message
						BuddyRequest req;
						req.buddyRequestType = BuddyRequest::BUDDYREQUEST_MESSAGE;
						wcslcpy(req.arg.message.text, txtInput.str(), MAX_BUDDY_CHAT_LEN);
						req.arg.message.recipient = selectedProfile;
						TheGameSpyBuddyMessageQueue->addRequest(req);

						// save message for future incarnations of the buddy window
						BuddyMessageList *messages = TheGameSpyInfo->getBuddyMessages();
						BuddyMessage message;
						message.m_timestamp = time(NULL);
						message.m_senderID = TheGameSpyInfo->getLocalProfileID();
						message.m_senderNick = TheGameSpyInfo->getLocalBaseName();
						message.m_recipientID = selectedProfile;
						message.m_recipientNick = recipIt->second.m_name;
						message.m_message = UnicodeString(req.arg.message.text);
						messages->push_back(message);

						// put message on screen
						insertChat(message);
					}
				}
				else
				{
					// nobody selected.  Prompt the user.
					if (buddyControls.listboxChat)
					{
						GadgetListBoxAddEntryText( buddyControls.listboxChat, TheGameText->fetch("Buddy:SelectBuddyToChat"),
							GameSpyColor[GSCOLOR_DEFAULT], -1, -1 );
					}
				}
				break;
			}
		default:
			return MSG_IGNORED;
	}
	return MSG_HANDLED;
}


static void insertChat( BuddyMessage msg )
{
	if (buddyControls.listboxChat)
	{
		BuddyInfoMap *m = TheGameSpyInfo->getBuddyMap();
		BuddyInfoMap::iterator senderIt = m->find(msg.m_senderID);
		BuddyInfoMap::iterator recipientIt = m->find(msg.m_recipientID);
		Bool localSender = (msg.m_senderID == TheGameSpyInfo->getLocalProfileID());
		UnicodeString s;
		//UnicodeString timeStr = UnicodeString(_wctime( (const time_t *)&msg.m_timestamp ));
		UnicodeString timeStr;
		if (localSender /*&& recipientIt != m->end()*/)
		{
			s.format(L"[%hs -> %hs] %s", TheGameSpyInfo->getLocalBaseName().str(), msg.m_recipientNick.str(), msg.m_message.str());
			Int index = GadgetListBoxAddEntryText( buddyControls.listboxChat, s, GameSpyColor[GSCOLOR_PLAYER_SELF], -1, -1 );
			GadgetListBoxAddEntryText( buddyControls.listboxChat, timeStr, GameSpyColor[GSCOLOR_PLAYER_SELF], index, 1);
		}
		else if (!localSender /*&& senderIt != m->end()*/)
		{
			if (!msg.m_senderID)
			{
				s = msg.m_message;
				Int index = GadgetListBoxAddEntryText( buddyControls.listboxChat, s, GameSpyColor[GSCOLOR_DEFAULT], -1, -1 );
				GadgetListBoxAddEntryText( buddyControls.listboxChat, timeStr, GameSpyColor[GSCOLOR_DEFAULT], index, 1);
			}
			else
			{
				s.format(L"[%hs] %s", msg.m_senderNick.str(), msg.m_message.str());
				Int index = GadgetListBoxAddEntryText( buddyControls.listboxChat, s, GameSpyColor[GSCOLOR_PLAYER_BUDDY], -1, -1 );
				GadgetListBoxAddEntryText( buddyControls.listboxChat, timeStr, GameSpyColor[GSCOLOR_PLAYER_BUDDY], index, 1);
			}
		}
	}
}

void updateBuddyInfo( void )
{
	if (!TheGameSpyBuddyMessageQueue->isConnected())
	{
		GadgetListBoxReset(buddyControls.listboxBuddies);
		return;
	}

	if (!buddyControls.isInit)
		return;

	int selected;
	GPProfile selectedProfile = 0;
	int visiblePos = GadgetListBoxGetTopVisibleEntry(buddyControls.listboxBuddies);

	GadgetListBoxGetSelected(buddyControls.listboxBuddies, &selected);
	if (selected >= 0)
		selectedProfile = (GPProfile)GadgetListBoxGetItemData(buddyControls.listboxBuddies, selected);

	selected = -1;
	GadgetListBoxReset(buddyControls.listboxBuddies);

	// Add buddies
	BuddyInfoMap *buddies = TheGameSpyInfo->getBuddyMap();
	BuddyInfoMap::iterator bIt;
	for (bIt = buddies->begin(); bIt != buddies->end(); ++bIt)
	{
		BuddyInfo info = bIt->second;
		GPProfile profileID = bIt->first;

		// insert name into box
		UnicodeString formatStr;
		formatStr.translate(info.m_name.str());//, info.m_status, info.m_statusString.str(), info.m_locationString.str());
		Color nameColor = (TheGameSpyInfo->isSavedIgnored(profileID)) ?
			GameSpyColor[GSCOLOR_PLAYER_IGNORED] : GameSpyColor[GSCOLOR_PLAYER_BUDDY];
		int index = GadgetListBoxAddEntryText(buddyControls.listboxBuddies, formatStr, nameColor, -1, -1);

		// insert status into box
		AsciiString marker;
		marker.format("Buddy:%ls", info.m_statusString.str());
		if (!info.m_statusString.compareNoCase(L"Offline") ||
			!info.m_statusString.compareNoCase(L"Online") ||
			!info.m_statusString.compareNoCase(L"Matching"))
		{
			formatStr = TheGameText->fetch(marker);
		}
		else if (!info.m_statusString.compareNoCase(L"Staging") ||
			!info.m_statusString.compareNoCase(L"Loading") ||
			!info.m_statusString.compareNoCase(L"Playing"))
		{
			formatStr.format(TheGameText->fetch(marker), info.m_locationString.str());
		}
		else if (!info.m_statusString.compareNoCase(L"Chatting"))
		{
			UnicodeString roomName;
			GroupRoomMap::iterator gIt = TheGameSpyInfo->getGroupRoomList()->find( _wtoi(info.m_locationString.str()) );
			if (gIt != TheGameSpyInfo->getGroupRoomList()->end())
			{
				AsciiString s;
				s.format("GUI:%s", gIt->second.m_name.str());
				roomName = TheGameText->fetch(s);
			}
			formatStr.format(TheGameText->fetch(marker), roomName.str());
		}
		else
		{
			formatStr = info.m_statusString;
		}
		GadgetListBoxAddEntryText(buddyControls.listboxBuddies, formatStr, GameSpyColor[GSCOLOR_DEFAULT], index, 1);
		GadgetListBoxSetItemData(buddyControls.listboxBuddies, (void *)(profileID), index, 0 );
		GadgetListBoxSetItemData(buddyControls.listboxBuddies, (void *)(ITEM_BUDDY), index, 1 );

		if (profileID == selectedProfile)
			selected = index;
	}

	// add requests
	buddies = TheGameSpyInfo->getBuddyRequestMap();
	for (bIt = buddies->begin(); bIt != buddies->end(); ++bIt)
	{
		BuddyInfo info = bIt->second;
		GPProfile profileID = bIt->first;

		// insert name into box
		UnicodeString formatStr;
		formatStr.translate(info.m_name.str());
		int index = GadgetListBoxAddEntryText(buddyControls.listboxBuddies, formatStr, GameSpyColor[GSCOLOR_DEFAULT], -1, -1);
		GadgetListBoxSetItemData(buddyControls.listboxBuddies, (void *)(profileID), index, 0 );

		// insert status into box
		formatStr = TheGameText->fetch("GUI:BuddyAddReq");
		GadgetListBoxAddEntryText(buddyControls.listboxBuddies, formatStr, GameSpyColor[GSCOLOR_DEFAULT], index, 1);
		GadgetListBoxSetItemData(buddyControls.listboxBuddies, (void *)(ITEM_REQUEST), index, 1 );

		if (profileID == selectedProfile)
			selected = index;
	}


	// select the same guy
	if (selected >= 0)
	{
		GadgetListBoxSetSelected(buddyControls.listboxBuddies, selected);
	}

	// view the same spot
	GadgetListBoxSetTopVisibleEntry(buddyControls.listboxBuddies, visiblePos);
}

void HandleBuddyResponses( void )
{
	if (TheGameSpyBuddyMessageQueue)
	{
		BuddyResponse resp;
		if (TheGameSpyBuddyMessageQueue->getResponse( resp ))
		{
			switch (resp.buddyResponseType)
			{
			case BuddyResponse::BUDDYRESPONSE_LOGIN:
				{
					deleteNotificationBox();
				}
				break;
			case BuddyResponse::BUDDYRESPONSE_DISCONNECT:
				{
					lastNotificationWasStatus = FALSE;
					numOnlineInNotification = 0;
					showNotificationBox(AsciiString::TheEmptyString, TheGameText->fetch("Buddy:MessageDisconnected"));
				}
				break;
			case BuddyResponse::BUDDYRESPONSE_MESSAGE:
				{
					if ( !wcscmp(resp.arg.message.text, L"I have authorized your request to add me to your list") )
						break;

					if (TheGameSpyInfo->isSavedIgnored(resp.profile))
					{
						//DEBUG_CRASH(("Player is ignored!"));
						break; // no buddy messages from ignored people
					}

					// save message for future incarnations of the buddy window
					BuddyMessageList *messages = TheGameSpyInfo->getBuddyMessages();
					BuddyMessage message;
					message.m_timestamp = resp.arg.message.date;
					message.m_senderID = resp.profile;
					message.m_recipientID = TheGameSpyInfo->getLocalProfileID();
					message.m_recipientNick = TheGameSpyInfo->getLocalBaseName();
					message.m_message = resp.arg.message.text;
					// insert status into box
					BuddyInfoMap *m = TheGameSpyInfo->getBuddyMap();
					BuddyInfoMap::iterator senderIt = m->find(message.m_senderID);
					AsciiString nick;
					if (senderIt != m->end())
						nick = senderIt->second.m_name.str();
					else
						nick = resp.arg.message.nick;
					message.m_senderNick = nick;
					messages->push_back(message);

					DEBUG_LOG(("Inserting buddy chat from '%s'/'%s'", nick.str(), resp.arg.message.nick));

					// put message on screen
					insertChat(message);

					// play audio notification
					AudioEventRTS buddyMsgAudio("GUIMessageReceived");
					if( TheAudio )
					{
						TheAudio->addAudioEvent( &buddyMsgAudio );
					}

					UnicodeString snippet = message.m_message;
					snippet.truncateTo(11);

					UnicodeString s;
					s.format(TheGameText->fetch("Buddy:MessageNotification"), nick.str(), snippet.str());
					lastNotificationWasStatus = FALSE;
					numOnlineInNotification = 0;
					showNotificationBox(AsciiString::TheEmptyString, s);
				}
				break;
			case BuddyResponse::BUDDYRESPONSE_REQUEST:
				{
					// save request for future incarnations of the buddy window
					BuddyInfoMap *m = TheGameSpyInfo->getBuddyRequestMap();
					BuddyInfo info;
					info.m_countryCode = resp.arg.request.countrycode;
					info.m_email = resp.arg.request.email;
					info.m_name = resp.arg.request.nick;
					info.m_id = resp.profile;
					info.m_status = (GPEnum)0;
					info.m_statusString = resp.arg.request.text;
					(*m)[resp.profile] = info;

					// TODO: put request on screen
					updateBuddyInfo();
					// insert status into box
					lastNotificationWasStatus = FALSE;
					numOnlineInNotification = 0;
					showNotificationBox(info.m_name, TheGameText->fetch("Buddy:AddNotification"));
				}
				break;
			case BuddyResponse::BUDDYRESPONSE_STATUS:
				{
					BuddyInfoMap *m = TheGameSpyInfo->getBuddyMap();
					BuddyInfoMap::const_iterator bit = m->find(resp.profile);
					Bool seenPreviously = FALSE;
					GPEnum oldStatus = GP_OFFLINE;
					GPEnum newStatus = resp.arg.status.status;
					if (bit != m->end())
					{
						seenPreviously = TRUE;
						oldStatus = (*m)[resp.profile].m_status;
					}
					BuddyInfo info;
					info.m_countryCode = resp.arg.status.countrycode;
					info.m_email = resp.arg.status.email;
					info.m_name = resp.arg.status.nick;
					info.m_id = resp.profile;
					info.m_status = newStatus;
					info.m_statusString = UnicodeString(MultiByteToWideCharSingleLine(resp.arg.status.statusString).c_str());
					info.m_locationString = UnicodeString(MultiByteToWideCharSingleLine(resp.arg.status.location).c_str());
					(*m)[resp.profile] = info;

					updateBuddyInfo();
					PopulateLobbyPlayerListbox();
					RefreshGameListBoxes();
					if ( (newStatus == GP_OFFLINE && seenPreviously) ||
						(newStatus == GP_ONLINE && (oldStatus == GP_OFFLINE || !seenPreviously)) )
					//if (!info.m_statusString.compareNoCase(L"Offline") ||
					//!info.m_statusString.compareNoCase(L"Online"))
					{
						// insert status into box
						AsciiString marker;
						marker.format("Buddy:%lsNotification", info.m_statusString.str());

						lastNotificationWasStatus = TRUE;
						if (newStatus != GP_OFFLINE)
							++numOnlineInNotification;

						showNotificationBox(info.m_name, TheGameText->fetch(marker));
					}
					else if( newStatus == GP_RECV_GAME_INVITE && !seenPreviously)
					{
						lastNotificationWasStatus = TRUE;
						if (newStatus != GP_OFFLINE)
							++numOnlineInNotification;

						showNotificationBox(info.m_name, TheGameText->fetch("Buddy:OnlineNotification"));
					}
				}
				break;
			}
		}
	}
	else
	{
		DEBUG_CRASH(("No buddy message queue!"));
	}
	if(noticeLayout && timeGetTime() > noticeExpires)
	{
		deleteNotificationBox();
	}
}

void showNotificationBox( AsciiString nick, UnicodeString message)
{
//	if(!GameSpyIsOverlayOpen(GSOVERLAY_BUDDY))
//		return;
	if( !noticeLayout )
		noticeLayout = TheWindowManager->winCreateLayout( "Menus/PopupBuddyListNotification.wnd" );
	noticeLayout->hide( FALSE );
	if (buttonNotificationID == NAMEKEY_INVALID)
	{
		buttonNotificationID = TheNameKeyGenerator->nameToKey("PopupBuddyListNotification.wnd:ButtonNotification");
	}
	GameWindow *win = TheWindowManager->winGetWindowFromId(NULL,buttonNotificationID);
	if(!win)
	{
		deleteNotificationBox();
		return;
	}

	if (lastNotificationWasStatus && numOnlineInNotification > 1)
	{
		message = TheGameText->fetch("Buddy:MultipleOnlineNotification");
	}

	if (nick.isNotEmpty())
		message.format(message, nick.str());
	GadgetButtonSetText(win, message);
	//GadgetStaticTextSetText(win, message);
	noticeExpires = timeGetTime() + NOTIFICATION_EXPIRES;
	noticeLayout->bringForward();

	AudioEventRTS buttonClick("GUICommunicatorIncoming");

	if( TheAudio )
	{
		TheAudio->addAudioEvent( &buttonClick );
	}

}

void deleteNotificationBox( void )
{
	lastNotificationWasStatus = FALSE;
	numOnlineInNotification = 0;
	if(noticeLayout)
	{
		noticeLayout->destroyWindows();
		deleteInstance(noticeLayout);
		noticeLayout = NULL;
	}
}

void PopulateOldBuddyMessages(void)
{
	// show previous messages
	BuddyMessageList *messages = TheGameSpyInfo->getBuddyMessages();
	for (BuddyMessageList::iterator mIt = messages->begin(); mIt != messages->end(); ++mIt)
	{
		BuddyMessage message = *mIt;
		insertChat(message);
	}
}

//-------------------------------------------------------------------------------------------------
/** Initialize the WOL Buddy Overlay */
//-------------------------------------------------------------------------------------------------
void WOLBuddyOverlayInit( WindowLayout *layout, void *userData )
{
	parentID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:BuddyMenuParent" ) );
	buttonHideID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:ButtonHide" ) );
	buttonAddBuddyID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:ButtonAdd" ) );
	buttonDeleteBuddyID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:ButtonDelete" ) );
	//textEntryID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:TextEntryChat" ) );
	//listboxBuddyID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:ListboxBuddies" ) );
	//listboxChatID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:ListboxBuddyChat" ) );
	buttonAcceptBuddyID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:ButtonYes" ) );
	buttonDenyBuddyID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:ButtonNo" ) );
	radioButtonBuddiesID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:RadioButtonBuddies" ) );
	radioButtonIgnoreID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:RadioButtonIgnore" ) );
	parentBuddiesID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:BuddiesParent" ) );
	parentIgnoreID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:IgnoreParent" ) );
	listboxIgnoreID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLBuddyOverlay.wnd:ListboxIgnore" ) );


	parent = TheWindowManager->winGetWindowFromId( NULL, parentID );
	buttonHide = TheWindowManager->winGetWindowFromId( parent,  buttonHideID);
	buttonAddBuddy = TheWindowManager->winGetWindowFromId( parent,  buttonAddBuddyID);
	buttonDeleteBuddy = TheWindowManager->winGetWindowFromId( parent,  buttonDeleteBuddyID);
	//	textEntry = TheWindowManager->winGetWindowFromId( parent,  textEntryID);
	//listboxBuddy = TheWindowManager->winGetWindowFromId( parent,  listboxBuddyID);
	//listboxChat = TheWindowManager->winGetWindowFromId( parent,  listboxChatID);
	buttonAcceptBuddy = TheWindowManager->winGetWindowFromId( parent,  buttonAcceptBuddyID);
	buttonDenyBuddy = TheWindowManager->winGetWindowFromId( parent,  buttonDenyBuddyID);
	radioButtonBuddies = TheWindowManager->winGetWindowFromId( parent,  radioButtonBuddiesID);
	radioButtonIgnore = TheWindowManager->winGetWindowFromId( parent,  radioButtonIgnoreID);
	parentBuddies = TheWindowManager->winGetWindowFromId( parent,  parentBuddiesID);
	parentIgnore = TheWindowManager->winGetWindowFromId( parent,  parentIgnoreID);
	listboxIgnore = TheWindowManager->winGetWindowFromId( parent,  listboxIgnoreID);

	InitBuddyControls(BUDDY_WINDOW_BUDDIES);

	GadgetRadioSetSelection(radioButtonBuddies,FALSE);
	parentBuddies->winHide(FALSE);
	parentIgnore->winHide(TRUE);

	//GadgetTextEntrySetText(textEntry, UnicodeString::TheEmptyString);

	PopulateOldBuddyMessages();

	// Show Menu
	layout->hide( FALSE );

	// Set Keyboard to Main Parent
	TheWindowManager->winSetFocus( parent );

	isOverlayActive = true;
	updateBuddyInfo();

}

//-------------------------------------------------------------------------------------------------
/** WOL Buddy Overlay shutdown method */
//-------------------------------------------------------------------------------------------------
void WOLBuddyOverlayShutdown( WindowLayout *layout, void *userData )
{
	listboxIgnore = NULL;

	// hide menu
	layout->hide( TRUE );

	// our shutdown is complete
	//TheShell->shutdownComplete( layout );

	isOverlayActive = false;

	InitBuddyControls(BUDDY_RESETALL_CRAP);

}


//-------------------------------------------------------------------------------------------------
/** WOL Buddy Overlay update method */
//-------------------------------------------------------------------------------------------------
void WOLBuddyOverlayUpdate( WindowLayout * layout, void *userData)
{
	if (!TheGameSpyBuddyMessageQueue || !TheGameSpyBuddyMessageQueue->isConnected())
		GameSpyCloseOverlay(GSOVERLAY_BUDDY);
}

//-------------------------------------------------------------------------------------------------
/** WOL Buddy Overlay input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLBuddyOverlayInput( GameWindow *window, UnsignedInt msg,
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
						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
																							(WindowMsgData)buttonHide, buttonHideID );

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
/** WOL Buddy Overlay window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLBuddyOverlaySystem( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	UnicodeString txtInput;
	if(BuddyControlSystem(window, msg, mData1, mData2) == MSG_HANDLED)
	{
		return MSG_HANDLED;
	}
	switch( msg )
	{


		case GWM_CREATE:
			{

				break;
			}

		case GWM_DESTROY:
			{
				break;
			}

		case GWM_INPUT_FOCUS:
			{
				// if we're givin the opportunity to take the keyboard focus we must say we want it
				if( mData1 == TRUE )
					*(Bool *)mData2 = TRUE;

				return MSG_HANDLED;
			}
		case GLM_RIGHT_CLICKED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if( controlID == listboxIgnoreID )
				{
					RightClickStruct *rc = (RightClickStruct *)mData2;
					WindowLayout *rcLayout;
					if(rc->pos < 0)
						break;

					Bool isBuddy = false, isRequest = false;
					GPProfile profileID = (GPProfile)GadgetListBoxGetItemData(control, rc->pos);
					UnicodeString nick = GadgetListBoxGetText(control, rc->pos);
					BuddyInfoMap *buddies = TheGameSpyInfo->getBuddyMap();
					BuddyInfoMap::iterator bIt;
					bIt = buddies->find(profileID);
					if (bIt != buddies->end())
					{
						isBuddy = true;
					}
					else
					{
						buddies = TheGameSpyInfo->getBuddyRequestMap();
						bIt = buddies->find(profileID);
						if (bIt != buddies->end())
						{
							isRequest = true;
						}
						else
						{
							// neither buddy nor request
							//break;
						}
					}

					GadgetListBoxSetSelected(control, rc->pos);
					if (isBuddy)
						rcLayout = TheWindowManager->winCreateLayout(AsciiString("Menus/RCBuddiesMenu.wnd"));
					else if (isRequest)
						rcLayout = TheWindowManager->winCreateLayout(AsciiString("Menus/RCBuddyRequestMenu.wnd"));
					else
						rcLayout = TheWindowManager->winCreateLayout(AsciiString("Menus/RCNonBuddiesMenu.wnd"));
					rcMenu = rcLayout->getFirstWindow();
					rcMenu->winGetLayout()->runInit();
					rcMenu->winBringToTop();
					rcMenu->winHide(FALSE);



					rcMenu->winSetPosition(rc->mouseX, rc->mouseY);
					GameSpyRCMenuData *rcData = NEW GameSpyRCMenuData;
					rcData->m_id = profileID;
					rcData->m_nick.translate(nick);
					rcData->m_itemType = (isBuddy)?ITEM_BUDDY:((isRequest)?ITEM_REQUEST:ITEM_NONBUDDY);
					setUnignoreText(rcLayout, rcData->m_nick, rcData->m_id);
					rcMenu->winSetUserData((void *)rcData);
					TheWindowManager->winSetLoneWindow(rcMenu);
				}
				break;
			}
		case GBM_SELECTED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if (controlID == buttonHideID)
				{
					GameSpyCloseOverlay( GSOVERLAY_BUDDY );
				}
				else if (controlID == radioButtonBuddiesID)
				{
					parentBuddies->winHide(FALSE);
					parentIgnore->winHide(TRUE);
				}
				else if (controlID == radioButtonIgnoreID)
				{
					parentBuddies->winHide(TRUE);
					parentIgnore->winHide(FALSE);
					refreshIgnoreList();
				}
				else if (controlID == buttonAddBuddyID)
				{
					/*
					UnicodeString uName = GadgetTextEntryGetText(textEntry);
					AsciiString aName;
					aName.translate(uName);
					if (!aName.isEmpty())
					{
						TheWOLBuddyList->requestBuddyAdd(aName);
					}
					GadgetTextEntrySetText(textEntry, UnicodeString::TheEmptyString);
					*/
				}
				else if (controlID == buttonDeleteBuddyID)
				{
					/*
					int selected;
					AsciiString selectedName = AsciiString::TheEmptyString;

					GadgetListBoxGetSelected(listbox, &selected);
					if (selected >= 0)
						selectedName = TheNameKeyGenerator->keyToName((NameKeyType)(int)GadgetListBoxGetItemData(listbox, selected));

					if (!selectedName.isEmpty())
					{
						TheWOLBuddyList->requestBuddyDelete(selectedName);
					}
					*/
				}
				break;
			}
		case GLM_DOUBLE_CLICKED:
			{
				/*
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				if( controlID == listboxBuddyID )
				{
					int rowSelected = mData2;

					if (rowSelected >= 0)
					{
						UnicodeString buddyName;
						GameWindow *listboxWindow = TheWindowManager->winGetWindowFromId( parent, listboxBuddyID );

							// get text of buddy name
						buddyName = GadgetListBoxGetText( listboxWindow, rowSelected,0 );
						GPProfile buddyID = (GPProfile)GadgetListBoxGetItemData( listboxWindow, rowSelected, 0 );

						Int index = -1;
						gpGetBuddyIndex(TheGPConnection, buddyID, &index);
						if (index >= 0)
						{
							GPBuddyStatus status;
							gpGetBuddyStatus(TheGPConnection, rowSelected, &status);

							UnicodeString string;
							string.format(L"To join %s in %hs:", buddyName.str(), status.locationString);
							GameSpyAddText(string, GSCOLOR_DEFAULT);

							if (status.status == GP_CHATTING)
							{
								AsciiString location = status.locationString;
								AsciiString val;
								location.nextToken(&val, "/");
								location.nextToken(&val, "/");
								location.nextToken(&val, "/");

								string.format(L"  ???");
								if (!val.isEmpty())
								{
									Int groupRoom = atoi(val.str());
									if (TheGameSpyChat->getCurrentGroupRoomID() == groupRoom)
									{
										// already there
										string.format(L"  nothing");
										GameSpyAddText(string, GSCOLOR_DEFAULT);
									}
									else
									{
										GroupRoomMap *rooms = TheGameSpyChat->getGroupRooms();
										if (rooms)
										{
											Bool needToJoin = true;
											GroupRoomMap::iterator it = rooms->find(groupRoom);
											if (it != rooms->end())
											{
												// he's in a different room
												if (TheGameSpyChat->getCurrentGroupRoomID())
												{
													string.format(L"  leave group room");
													GameSpyAddText(string, GSCOLOR_DEFAULT);

													TheGameSpyChat->leaveRoom(GroupRoom);
												}
												else if (TheGameSpyGame->isInGame())
												{
													if (TheGameSpyGame->isGameInProgress())
													{
														string.format(L"  can't leave game in progress");
														GameSpyAddText(string, GSCOLOR_DEFAULT);
														needToJoin = false;
													}
													else
													{
														string.format(L"  leave game setup");
														GameSpyAddText(string, GSCOLOR_DEFAULT);

														TheGameSpyChat->leaveRoom(StagingRoom);
														TheGameSpyGame->leaveGame();
													}
												}
												if (needToJoin)
												{
													string.format(L"  join lobby %d", groupRoom);
													TheGameSpyChat->joinGroupRoom(groupRoom);
													GameSpyAddText(string, GSCOLOR_DEFAULT);
												}
											}
										}
									}
								}
							}
						}
						else
						{
							DEBUG_CRASH(("No buddy associated with that ProfileID"));
							GameSpyUpdateBuddyOverlay();
						}
					}
				}
				*/
				break;
			}

		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;
}

WindowMsgHandledType PopupBuddyNotificationSystem( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg )
	{
		case GWM_CREATE:
			{
				break;
			}

		case GWM_DESTROY:
			{
				break;
			}

		case GBM_SELECTED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if (controlID == buttonNotificationID)
				{
					GameSpyOpenOverlay( GSOVERLAY_BUDDY );
				}
				break;
			}

		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;
}

/*
static NameKeyType buttonAcceptBuddyID = NAMEKEY_INVALID;
static NameKeyType buttonDenyBuddyID = NAMEKEY_INVALID;
*/
static NameKeyType buttonAddID = NAMEKEY_INVALID;
static NameKeyType buttonDeleteID = NAMEKEY_INVALID;
static NameKeyType buttonPlayID = NAMEKEY_INVALID;
static NameKeyType buttonIgnoreID = NAMEKEY_INVALID;
static NameKeyType buttonStatsID = NAMEKEY_INVALID;
// Window Pointers ------------------------------------------------------------------------
//static GameWindow *rCparent = NULL;


//-------------------------------------------------------------------------------------------------
/** WOL Buddy Overlay Right Click menu callbacks */
//-------------------------------------------------------------------------------------------------
void WOLBuddyOverlayRCMenuInit( WindowLayout *layout, void *userData )
{
	AsciiString controlName;
	controlName.format("%s:ButtonAdd",layout->getFilename().str()+6);
	buttonAddID =  TheNameKeyGenerator->nameToKey( controlName );
	controlName.format("%s:ButtonDelete",layout->getFilename().str()+6);
	buttonDeleteID =  TheNameKeyGenerator->nameToKey( controlName );
	controlName.format("%s:ButtonPlay",layout->getFilename().str()+6);
	buttonPlayID =  TheNameKeyGenerator->nameToKey( controlName );
	controlName.format("%s:ButtonIgnore",layout->getFilename().str()+6);
	buttonIgnoreID =  TheNameKeyGenerator->nameToKey( controlName );
	controlName.format("%s:ButtonStats",layout->getFilename().str()+6);
	buttonStatsID =  TheNameKeyGenerator->nameToKey( controlName );
}
static void closeRightClickMenu(GameWindow *win)
{

	if(win)
	{
		WindowLayout *winLay = win->winGetLayout();
		if(!winLay)
			return;
		winLay->destroyWindows();
		deleteInstance(winLay);
		winLay = NULL;

	}
}

void RequestBuddyAdd(Int profileID, AsciiString nick)
{
	// request to add a buddy
	BuddyRequest req;
	req.buddyRequestType = BuddyRequest::BUDDYREQUEST_ADDBUDDY;
	req.arg.addbuddy.id = profileID;
	UnicodeString buddyAddstr;
	buddyAddstr = TheGameText->fetch("GUI:BuddyAddReq");
	wcslcpy(req.arg.addbuddy.text, buddyAddstr.str(), MAX_BUDDY_CHAT_LEN);
	TheGameSpyBuddyMessageQueue->addRequest(req);

	UnicodeString s;
	Bool exists = TRUE;
	s.format(TheGameText->fetch("Buddy:InviteSent", &exists));
	if (!exists)
	{
		// no string yet.  don't display.
		return;
	}

	// save message for future incarnations of the buddy window
	BuddyMessageList *messages = TheGameSpyInfo->getBuddyMessages();
	BuddyMessage message;
	message.m_timestamp = time(NULL);
	message.m_senderID = 0;
	message.m_senderNick = "";
	message.m_recipientID = TheGameSpyInfo->getLocalProfileID();
	message.m_recipientNick = TheGameSpyInfo->getLocalBaseName();
	message.m_message.format(TheGameText->fetch("Buddy:InviteSentToPlayer"), nick.str());

	// insert status into box
	messages->push_back(message);

	DEBUG_LOG(("Inserting buddy add request"));

	// put message on screen
	insertChat(message);

	// play audio notification
	AudioEventRTS buddyMsgAudio("GUIMessageReceived");
	if( TheAudio )
	{
		TheAudio->addAudioEvent( &buddyMsgAudio );
	}

	lastNotificationWasStatus = FALSE;
	numOnlineInNotification = 0;
	showNotificationBox(AsciiString::TheEmptyString, s);
}

WindowMsgHandledType WOLBuddyOverlayRCMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 )
{

	switch( msg )
	{

		case GWM_CREATE:
			{

				break;
			}

		case GWM_DESTROY:
			{
				rcMenu = NULL;
				break;
			}

		case GGM_CLOSE:
			{
				closeRightClickMenu(window);
				//rcMenu = NULL;
				break;
			}


		case GBM_SELECTED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				GameSpyRCMenuData *rcData = (GameSpyRCMenuData*)window->winGetUserData();
				if(!rcData)
					break;
				DEBUG_ASSERTCRASH(rcData, ("WOLBuddyOverlayRCMenuSystem GBM_SELECTED:: we're attempting to read the GameSpyRCMenuData from the window, but the data's not there"));
				GPProfile profileID = rcData->m_id;
				AsciiString nick = rcData->m_nick;

				Bool isBuddy = false, isRequest = false;
				Bool isGameSpyUser = profileID > 0;
				if (rcData->m_itemType == ITEM_BUDDY)
					isBuddy = TRUE;
				else if (rcData->m_itemType == ITEM_REQUEST)
					isRequest = TRUE;

				delete rcData;
				rcData = NULL;

				window->winSetUserData(NULL);
				//DEBUG_ASSERTCRASH(profileID > 0, ("Bad profile ID in user data!"));

				if( controlID == buttonAddID )
				{
					if(!isGameSpyUser)
						break;
					DEBUG_LOG(("ButtonAdd was pushed"));
					if (isRequest)
					{
						// ok the request
						BuddyRequest req;
						req.buddyRequestType = BuddyRequest::BUDDYREQUEST_OKADD;
						req.arg.profile.id = profileID;
						TheGameSpyBuddyMessageQueue->addRequest(req);

						BuddyInfoMap *m = TheGameSpyInfo->getBuddyRequestMap();
						m->erase( profileID );
						// if the profile ID is not from a buddy and we're okaying his request, then
						// request to add him to our list automatically CLH 2-18-03
						if(!TheGameSpyInfo->isBuddy(profileID))
						{
							RequestBuddyAdd(profileID, nick);
						}
						updateBuddyInfo();
					}
					else if (!isBuddy)
					{
						RequestBuddyAdd(profileID, nick);
					}
				}
				else if( controlID == buttonDeleteID )
				{
					if(!isGameSpyUser)
						break;
					if (isBuddy)
					{
						// delete the buddy
						BuddyRequest req;
						req.buddyRequestType = BuddyRequest::BUDDYREQUEST_DELBUDDY;
						req.arg.profile.id = profileID;
						TheGameSpyBuddyMessageQueue->addRequest(req);
					}
					else
					{
						// delete the request
						BuddyRequest req;
						req.buddyRequestType = BuddyRequest::BUDDYREQUEST_DENYADD;
						req.arg.profile.id = profileID;
						TheGameSpyBuddyMessageQueue->addRequest(req);
						BuddyInfoMap *m = TheGameSpyInfo->getBuddyRequestMap();
						m->erase( profileID );
					}
					BuddyInfoMap *buddies = (isBuddy)?TheGameSpyInfo->getBuddyMap():TheGameSpyInfo->getBuddyRequestMap();
					buddies->erase(profileID);
					updateBuddyInfo();
					DEBUG_LOG(("ButtonDelete was pushed"));
					PopulateLobbyPlayerListbox();
				}
				else if( controlID == buttonPlayID )
				{
					DEBUG_LOG(("buttonPlayID was pushed"));
				}
				else if( controlID == buttonIgnoreID )
				{
					DEBUG_LOG(("%s is isGameSpyUser %d", nick.str(), isGameSpyUser));
					if( isGameSpyUser )
					{
						if(TheGameSpyInfo->isSavedIgnored(profileID))
						{
							TheGameSpyInfo->removeFromSavedIgnoreList(profileID);
						}
						else
						{
							TheGameSpyInfo->addToSavedIgnoreList(profileID, nick);
						}
					}
					else
					{
						if(TheGameSpyInfo->isIgnored(nick))
						{
							TheGameSpyInfo->removeFromIgnoreList(nick);
						}
						else
						{
							TheGameSpyInfo->addToIgnoreList(nick);
						}
					}
					updateBuddyInfo();
					refreshIgnoreList();
					// repopulate our player listboxes now
					PopulateLobbyPlayerListbox();
				}
				else if( controlID == buttonStatsID )
				{
					DEBUG_LOG(("buttonStatsID was pushed"));
					GameSpyCloseOverlay(GSOVERLAY_PLAYERINFO);
					SetLookAtPlayer(profileID,nick );
					GameSpyOpenOverlay(GSOVERLAY_PLAYERINFO);
					PSRequest req;
					req.requestType = PSRequest::PSREQUEST_READPLAYERSTATS;
					req.player.id = profileID;
					TheGameSpyPSMessageQueue->addRequest(req);
				}
				closeRightClickMenu(window);
				break;
			}
		default:
			return MSG_IGNORED;

	}
	return MSG_HANDLED;
}


void setUnignoreText( WindowLayout *layout, AsciiString nick, GPProfile id)
{
	AsciiString controlName;
	controlName.format("%s:ButtonIgnore",layout->getFilename().str()+6);
	NameKeyType ID =  TheNameKeyGenerator->nameToKey( controlName );
	GameWindow *win = TheWindowManager->winGetWindowFromId(layout->getFirstWindow(), ID);
	if(win)
	{
		if(TheGameSpyInfo->isSavedIgnored(id) || TheGameSpyInfo->isIgnored(nick))
			GadgetButtonSetText(win, TheGameText->fetch("GUI:Unignore"));
	}
}

void refreshIgnoreList( void )
{


	SavedIgnoreMap tempMap;
	tempMap = TheGameSpyInfo->returnSavedIgnoreList();
	SavedIgnoreMap::iterator it = tempMap.begin();
	GadgetListBoxReset(listboxIgnore);
	while(it != tempMap.end())
	{
		UnicodeString name;
		name.translate(it->second);
		Int pos = GadgetListBoxAddEntryText(listboxIgnore, name, GameMakeColor(255,100,100,255),-1);
		GadgetListBoxSetItemData(listboxIgnore, (void *)it->first,pos );
		++it;
	}
	IgnoreList tempList;
	tempList = TheGameSpyInfo->returnIgnoreList();
	IgnoreList::iterator iListIt = tempList.begin();
	while( iListIt != tempList.end())
	{
		AsciiString aName = *iListIt;
		UnicodeString name;
		name.translate(aName);
		Int pos = GadgetListBoxAddEntryText(listboxIgnore, name, GameMakeColor(255,100,100,255),-1);
		GadgetListBoxSetItemData(listboxIgnore, 0,pos );
		++iListIt;
	}

//
//	GPProfile profileID = 0;
//	PlayerInfoMap::iterator it = TheGameSpyInfo->getPlayerInfoMap()->find(aName);
//	if (it != TheGameSpyInfo->getPlayerInfoMap()->end())
//		profileID = it->second.m_profileID;

}
