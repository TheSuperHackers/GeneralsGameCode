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

// FILE: Chat.cpp //////////////////////////////////////////////////////
// Generals GameSpy chat-related code
// Author: Matthew D. Campbell, July 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/AudioEventRTS.h"
#include "Common/INI.h"
#include "GameClient/GameText.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/LanguageFilter.h"
#include "GameClient/GameWindowManager.h"
#include "GameNetwork/GameSpy/PeerDefsImplementation.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameClient/InGameUI.h"

#define OFFSET(x) (sizeof(Int) * (x))
static const FieldParse GameSpyColorFieldParse[] =
{

	{ "Default",						INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_DEFAULT) },
	{ "CurrentRoom",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CURRENTROOM) },
	{ "ChatRoom",						INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_ROOM) },
	{ "Game",								INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_GAME) },
	{ "GameFull",						INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_GAME_FULL) },
	{ "GameCRCMismatch",		INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_GAME_CRCMISMATCH) },
	{ "PlayerNormal",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_PLAYER_NORMAL) },
	{ "PlayerOwner",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_PLAYER_OWNER) },
	{ "PlayerBuddy",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_PLAYER_BUDDY) },
	{ "PlayerSelf",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_PLAYER_SELF) },
	{ "PlayerIgnored",			INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_PLAYER_IGNORED) },
	{ "ChatNormal",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_NORMAL) },
	{ "ChatEmote",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_EMOTE) },
	{ "ChatOwner",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_OWNER) },
	{ "ChatOwnerEmote",			INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_OWNER_EMOTE) },
	{ "ChatPriv",						INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_PRIVATE) },
	{ "ChatPrivEmote",			INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_PRIVATE_EMOTE) },
	{ "ChatPrivOwner",			INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_PRIVATE_OWNER) },
	{ "ChatPrivOwnerEmote",	INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_PRIVATE_OWNER_EMOTE) },
	{ "ChatBuddy",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_BUDDY) },
	{ "ChatSelf",						INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_CHAT_SELF) },
	{ "AcceptTrue",					INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_ACCEPT_TRUE) },
	{ "AcceptFalse",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_ACCEPT_FALSE) },
	{ "MapSelected",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_MAP_SELECTED) },
	{ "MapUnselected",			INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_MAP_UNSELECTED) },
	{ "MOTD",								INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_MOTD) },
	{ "MOTDHeading",				INI::parseColorInt,	NULL,	OFFSET(GSCOLOR_MOTD_HEADING) },

	{ NULL,					NULL,						NULL,						0 }

};

void INI::parseOnlineChatColorDefinition( INI* ini )
{
	// parse the ini definition
	ini->initFromINI( GameSpyColor, GameSpyColorFieldParse );
}


Color GameSpyColor[GSCOLOR_MAX] =
{
	GameMakeColor(255,255,255,255),	// GSCOLOR_DEFAULT
	GameMakeColor(255,255,  0,255),	// GSCOLOR_CURRENTROOM
	GameMakeColor(255,255,255,255),	// GSCOLOR_ROOM
	GameMakeColor(128,128,0,255),		// GSCOLOR_GAME
	GameMakeColor(128,128,128,255),	// GSCOLOR_GAME_FULL
	GameMakeColor(128,128,128,255),	// GSCOLOR_GAME_CRCMISMATCH
	GameMakeColor(255,  0,  0,255),	// GSCOLOR_PLAYER_NORMAL
	GameMakeColor(255,  0,255,255),	// GSCOLOR_PLAYER_OWNER
	GameMakeColor(255,  0,128,255),	// GSCOLOR_PLAYER_BUDDY
	GameMakeColor(255,  0,  0,255),	// GSCOLOR_PLAYER_SELF
	GameMakeColor(128,128,128,255),	// GSCOLOR_PLAYER_IGNORED
	GameMakeColor(255,0,0,255),			// GSCOLOR_CHAT_NORMAL
	GameMakeColor(255,128,0,255),		// GSCOLOR_CHAT_EMOTE,
	GameMakeColor(255,255,0,255),		// GSCOLOR_CHAT_OWNER,
	GameMakeColor(128,255,0,255),		// GSCOLOR_CHAT_OWNER_EMOTE,
	GameMakeColor(0,0,255,255),			// GSCOLOR_CHAT_PRIVATE,
	GameMakeColor(0,255,255,255),		// GSCOLOR_CHAT_PRIVATE_EMOTE,
	GameMakeColor(255,0,255,255),		// GSCOLOR_CHAT_PRIVATE_OWNER,
	GameMakeColor(255,128,255,255),	// GSCOLOR_CHAT_PRIVATE_OWNER_EMOTE,
	GameMakeColor(255,  0,255,255),	// GSCOLOR_CHAT_BUDDY,
	GameMakeColor(255,  0,128,255),	// GSCOLOR_CHAT_SELF,
	GameMakeColor(  0,255,  0,255),	// GSCOLOR_ACCEPT_TRUE,
	GameMakeColor(255,  0,  0,255),	// GSCOLOR_ACCEPT_FALSE,
	GameMakeColor(255,255,  0,255),	// GSCOLOR_MAP_SELECTED,
	GameMakeColor(255,255,255,255),	// GSCOLOR_MAP_UNSELECTED,
	GameMakeColor(255,255,255,255),	// GSCOLOR_MOTD,
	GameMakeColor(255,255,  0,255),	// GSCOLOR_MOTD_HEADING,
};

Bool GameSpyInfo::sendChat( UnicodeString message, Bool isAction, GameWindow *playerListbox )
{
	RoomType roomType = StagingRoom;
	if (getCurrentGroupRoom())
		roomType = GroupRoom;

	PeerRequest req;
	req.text = message.str();

	message.trim();
	// Echo the user's input to the chat window
	if (!message.isEmpty())
	{
		if (!playerListbox)
		{
			// Public message
			req.message.isAction = isAction;
			req.peerRequestType = PeerRequest::PEERREQUEST_MESSAGEROOM;
			TheGameSpyPeerMessageQueue->addRequest(req);
			return false;
		}

		// Get the selections (is this a private message?)
		Int maxSel = GadgetListBoxGetListLength(playerListbox);
		Int *selections;
		GadgetListBoxGetSelected(playerListbox, (Int *)&selections);

		if (selections[0] == -1)
		{
			// Public message
			req.message.isAction = isAction;
			req.peerRequestType = PeerRequest::PEERREQUEST_MESSAGEROOM;
			TheGameSpyPeerMessageQueue->addRequest(req);
			return false;
		}
		else
		{
			// Private message

			// Construct a list
			AsciiString names = AsciiString::TheEmptyString;
			AsciiString tmp = AsciiString::TheEmptyString;
			AsciiString aStr; // AsciiString buf for translating Unicode entries
			names.format("%s", TheGameSpyInfo->getLocalName().str());
			for (int i=0; i<maxSel; i++)
			{
				if (selections[i] != -1)
				{
					aStr.translate(GadgetListBoxGetText(playerListbox, selections[i], GadgetListBoxGetNumColumns(playerListbox)-1));
					if (aStr.compareNoCase(TheGameSpyInfo->getLocalName()))
					{
						tmp.format(",%s", aStr.str());
						names.concat(tmp);
					}
				}
				else
				{
					break;
				}
			}

			if (!names.isEmpty())
			{
				req.nick = names.str();
				req.message.isAction = isAction;
				req.peerRequestType = PeerRequest::PEERREQUEST_MESSAGEPLAYER;
				TheGameSpyPeerMessageQueue->addRequest(req);
			}

			return true;
		}
	}
	return false;
}

void GameSpyInfo::addChat( AsciiString nick, Int profileID, UnicodeString msg, Bool isPublic, Bool isAction, GameWindow *win )
{
	PlayerInfoMap::iterator it = getPlayerInfoMap()->find(nick);
	if (it != getPlayerInfoMap()->end())
	{
		addChat( it->second, msg, isPublic, isAction, win );
	}
	else
	{
	}
}

void GameSpyInfo::addChat( PlayerInfo p, UnicodeString msg, Bool isPublic, Bool isAction, GameWindow *win )
{
	Int style;
	if(isSavedIgnored(p.m_profileID) || isIgnored(p.m_name))
		return;

	Bool isOwner = p.m_flags & PEER_FLAG_OP;
	Bool isBuddy = getBuddyMap()->find(p.m_profileID) != getBuddyMap()->end();

	Bool isMe = p.m_name.compare(TheGameSpyInfo->getLocalName()) == 0;

	if(!isMe)
	{
		if(m_disallowAsainText)
		{
			const WideChar *buff = msg.str();
			Int length =  msg.getLength();
			for(Int i = 0; i < length; ++i)
			{
				if(buff[i] >= 256)
					return;
			}
		}
		else if(m_disallowNonAsianText)
		{
			const WideChar *buff = msg.str();
			Int length =  msg.getLength();
			Bool hasUnicode = FALSE;
			for(Int i = 0; i < length; ++i)
			{
				if(buff[i] >= 256)
				{
					hasUnicode = TRUE;
					break;
				}
			}
			if(!hasUnicode)
				return;
		}

		if (!isPublic)
		{
			AudioEventRTS privMsgAudio("GUIMessageReceived");

			if( TheAudio )
			{
				TheAudio->addAudioEvent( &privMsgAudio );
			}
		}
	}


	if (isBuddy)
	{
		style = GSCOLOR_CHAT_BUDDY;
	}
	else if (isPublic && isAction)
	{
		style = (isOwner)?GSCOLOR_CHAT_OWNER_EMOTE:GSCOLOR_CHAT_EMOTE;
	}
	else if (isPublic)
	{
		style = (isOwner)?GSCOLOR_CHAT_OWNER:GSCOLOR_CHAT_NORMAL;
	}
	else if (isAction)
	{
		style = (isOwner)?GSCOLOR_CHAT_PRIVATE_OWNER_EMOTE:GSCOLOR_CHAT_PRIVATE_EMOTE;
	}
	else
	{
		style = (isOwner)?GSCOLOR_CHAT_PRIVATE_OWNER:GSCOLOR_CHAT_PRIVATE;
	}

	UnicodeString name;
	name.translate(p.m_name);

	// filters language
//  if( TheGlobalData->m_languageFilterPref )
//  {
    TheLanguageFilter->filterLine(msg);
//  }

	UnicodeString fullMsg;
	if (isAction)
	{
		fullMsg.format( L"%ls %ls", name.str(), msg.str() );
	}
	else
	{
		fullMsg.format( L"[%ls] %ls", name.str(), msg.str() );
	}

	Int index = addText(fullMsg, GameSpyColor[style], win);
	if (index >= 0)
	{
		GadgetListBoxSetItemData(win, (void *)p.m_profileID, index);
	}
}

Int GameSpyInfo::addText( UnicodeString message, Color c, GameWindow *win )
{
	if (TheGameSpyGame && TheGameSpyGame->isInGame() && TheGameSpyGame->isGameInProgress())
	{
		static AudioEventRTS messageFromChatSound("GUIMessageReceived");
		TheAudio->addAudioEvent(&messageFromChatSound);

		TheInGameUI->message(message);
	}

	if (!win)
	{
		// try to pick up a registered text window
		if (m_textWindows.empty())
			return -1;

		win = *(m_textWindows.begin());
	}
	Int index = GadgetListBoxAddEntryText(win, message, c, -1, -1);
	GadgetListBoxSetItemData(win, (void *)-1, index);

	return index;
}

void GameSpyInfo::registerTextWindow( GameWindow *win )
{
	m_textWindows.insert(win);
}

void GameSpyInfo::unregisterTextWindow( GameWindow *win )
{
	m_textWindows.erase(win);
}

