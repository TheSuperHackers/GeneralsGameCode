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

// FILE: GameSpyChat.cpp //////////////////////////////////////////////////////
// GameSpy chat handlers
// Author: Matthew D. Campbell, February 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/GameText.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/LanguageFilter.h"
#include "GameNetwork/GameSpy.h"
#include "GameNetwork/GameSpyChat.h"
#include "Common/QuotedPrintable.h"

typedef set<AsciiString>::const_iterator AsciiSetIter;

/**
	* handleSlashCommands looks for slash ccommands and handles them,
	* returning true if it found one, false otherwise.
	*   /i,/ignore									list ignored players
	*   /i,/ignore +name1 -name2		ignore name1, stop ignoring name2
	*   /m,/me:											shorthand for an action
	*   /o,/on:											find command to look up a user's location
	*   /f,/find:										find command to look up a user's location
	*   /p,/page:										page user(s)
	*   /r,/reply:									reply to last page
	*   /raw:												raw IRC command (only in debug & internal)
	*   /oper:											become an IRC op (only in debug & internal)
	*   /quit:											send the IRC quit command to exit WOL
	*/
static Bool handleSlashCommands( UnicodeString message, Bool isAction, GameWindow *playerListbox )
{
	/*
	if (message.getCharAt(0) == L'/')
	{
		UnicodeString remainder = UnicodeString(message.str() + 1);
		UnicodeString token;

		switch (message.getCharAt(1))
		{
		case L'i':
		case L'I':
			remainder.nextToken(&token);
			if (token.compareNoCase(L"i") == 0 || token.compareNoCase(L"ignore") == 0)
			{
				if (remainder.isEmpty())
				{
					// List the people we're ignoring
					TheWOL->addText(TheGameText->fetch("WOL:BeginIgnoreList"));
					set<AsciiString> *ignoreList = getIgnoreList();
					if (ignoreList)
					{
						UnicodeString msg;
						UnicodeString uName;
						AsciiSetIter iter = ignoreList->begin();
						while (iter != ignoreList->end())
						{
							uName.translate(*iter);
							msg.format(TheGameText->fetch("WOL:IgnoredUser"), uName.str());
							TheWOL->addText(msg);
							iter++;
						}
					}
					TheWOL->addText(TheGameText->fetch("WOL:EndIgnoreList"));
				}

				while ( remainder.nextToken(&token) )
				{
					AsciiString name;
					int doIgnore = 0;
					if (token.getCharAt(0) == L'+')
					{
						// Ignore somebody
						token = UnicodeString(token.str() + 1);
						name.translate(token);
						doIgnore = 1;
					}
					else if (token.getCharAt(0) == L'-')
					{
						// Listen to someone again
						token = UnicodeString(token.str() + 1);
						name.translate(token);
						doIgnore = 0;
					}
					else
					{
						// Ignore somebody
						token = UnicodeString(token.str());
						name.translate(token);
						doIgnore = 1;
					}
					IChat *ichat = TheWOL->getIChat();
					User user;
					strncpy((char *)user.name, name.str(), 9);
					user.name[9] = 0;
					ichat->SetSquelch(&user, doIgnore);

					if (doIgnore)
						addIgnore(name);
					else
						removeIgnore(name);

					UnicodeString msg;
					UnicodeString uName;
					uName.translate(name);
					msg.format(TheGameText->fetch("WOL:IgnoredUser"), uName.str());
					TheWOL->addText(msg);
				}
				return true;
			}
			break;
		case L'r':
		case L'R':
			remainder.nextToken(&token);
#if defined(RTS_DEBUG)
			if (token.compareNoCase(L"raw") == 0)
			{
				// Send raw IRC commands (Ascii only)
				AsciiString str;
				str.translate(remainder);
				str.concat('\n');
				IChat *ichat = TheWOL->getIChat();
				ichat->RequestRawMessage(str.str());
				TheWOL->addText(remainder);
				return true; // show it anyway
			}
#endif
			break;
#if defined(RTS_DEBUG)
		case L'k':
		case L'K':
			remainder.nextToken(&token);
			if (token.compareNoCase(L"kick") == 0)
			{

				while ( remainder.nextToken(&token) )
				{
					AsciiString name;
					name.translate(token);
					IChat *ichat = TheWOL->getIChat();
					User user;
					strncpy((char *)user.name, name.str(), 9);
					user.name[9] = 0;
					ichat->RequestUserKick(&user);
				}
				return true;
			}
			break;
#endif
		case L'o':
		case L'O':
			remainder.nextToken(&token);
			if (token.compareNoCase(L"on") == 0 || token.compareNoCase(L"o") == 0)
			{
				remainder.nextToken(&token);
				AsciiString userName;
				userName.translate(token);
				User user;
				strncpy((char *)user.name, userName.str(), 10);
				user.name[9] = 0;
				if (user.name[0] == 0)
				{
					// didn't enter a name
					TheWOL->addText(message);
				}
				else
				{
					// Send find command
					IChat *ichat = TheWOL->getIChat();
					ichat->RequestGlobalFind(&user);
				}
				return true; // show it anyway
			}
#if defined(RTS_DEBUG)
			else if (token.compareNoCase(L"oper") == 0)
			{
				// Send raw IRC oper command
				AsciiString str;
				str.translate(message);
				str.concat('\n');
				IChat *ichat = TheWOL->getIChat();
				ichat->RequestRawMessage(str.str());
				TheWOL->addText(message);
				return true; // show it anyway
			}
#endif
			break;
		case L'p':
		case L'P':
			remainder.nextToken(&token);
			if (token.compareNoCase(L"page") == 0 || token.compareNoCase(L"p") == 0)
			{
				remainder.nextToken(&token);
				AsciiString userName;
				userName.translate(token);
				User user;
				strncpy((char *)user.name, userName.str(), 10);
				user.name[9] = 0;
				remainder.trim();
				if (user.name[0] == 0 || remainder.isEmpty())
				{
					// didn't enter a name or message
					TheWOL->addText(message);
				}
				else
				{
					// Send page command
					IChat *ichat = TheWOL->getIChat();
					ichat->RequestGlobalUnicodePage(&user, remainder.str());
				}
				return true; // show it anyway
			}
			break;
		case L'q':
		case L'Q':
			remainder.nextToken(&token);
			if (token.compareNoCase(L"quit") == 0)
			{
				TheWOL->setState(WOLAPI_LOGIN);
				TheWOL->addCommand(WOLCOMMAND_LOGOUT);
				//TheWOL->setScreen(WOLAPI_MENU_WELCOME);
				return true; // show it anyway
			}
			break;
#if defined(RTS_DEBUG)
		case L'c':
		case L'C':
			remainder.nextToken(&token);
			if (token.compareNoCase(L"colortest") == 0)
			{
				addColorText(token, 0xDD, 0xE2, 0x0D, 0xff);
				addColorText(token, 0xFF, 0x19, 0x19, 0xff);
				addColorText(token, 0x2A, 0x74, 0xE2, 0xff);
				addColorText(token, 0x3E, 0xD1, 0x2E, 0xff);
				addColorText(token, 0xFF, 0xA0, 0x19, 0xff);
				addColorText(token, 0x32, 0xD7, 0xE6, 0xff);
				addColorText(token, 0x95, 0x28, 0xBD, 0xff);
				addColorText(token, 0xFF, 0x9A, 0xEB, 0xff);
				return true; // show it anyway
			}
			break;
#endif // RTS_DEBUG
		}
	}
	*/
	return false;
}

static handleUnicodeMessage( const char *nick, UnicodeString msg, Bool isPublic, Bool isAction );

Bool GameSpySendChat( UnicodeString message, Bool isAction, GameWindow *playerListbox )
{
	RoomType roomType = StagingRoom;
	if (TheGameSpyChat->getCurrentGroupRoomID())
		roomType = GroupRoom;

	message.trim();
	// Echo the user's input to the chat window
	if (!message.isEmpty())
	{
		// Check for slash commands
		if (handleSlashCommands(message, isAction, playerListbox))
		{
			return false; // already handled
		}

		if (!playerListbox)
		{
			// Public message
			if (isAction)
			{
				peerMessageRoom(TheGameSpyChat->getPeer(), roomType, UnicodeStringToQuotedPrintable(message).str(), ActionMessage);
				//if (roomType == StagingRoom)
					//handleUnicodeMessage(TheGameSpyChat->getloginName().str(), message, true, true);
			}
			else
			{
				peerMessageRoom(TheGameSpyChat->getPeer(), roomType, UnicodeStringToQuotedPrintable(message).str(), NormalMessage);
				//if (roomType == StagingRoom)
					//handleUnicodeMessage(TheGameSpyChat->getloginName().str(), message, true, false);
			}
			return false;
		}

		// Get the selections (is this a private message?)
		Int maxSel = GadgetListBoxGetListLength(playerListbox);
		Int *selections;
		GadgetListBoxGetSelected(playerListbox, (Int *)&selections);

		if (selections[0] == -1)
		{
			// Public message
			if (isAction)
			{
				peerMessageRoom(TheGameSpyChat->getPeer(), roomType, UnicodeStringToQuotedPrintable(message).str(), ActionMessage);
				//if (roomType == StagingRoom)
					//handleUnicodeMessage(TheGameSpyChat->getloginName().str(), message, true, true);
			}
			else
			{
				peerMessageRoom(TheGameSpyChat->getPeer(), roomType, UnicodeStringToQuotedPrintable(message).str(), NormalMessage);
				//if (roomType == StagingRoom)
					//handleUnicodeMessage(TheGameSpyChat->getloginName().str(), message, true, false);
			}
			return false;
		}
		else
		{
			// Private message

			// Construct a list
			AsciiString names = AsciiString::TheEmptyString;
			AsciiString tmp = AsciiString::TheEmptyString;
			AsciiString aStr; // AsciiString buf for translating Unicode entries
			names.format("%s", TheGameSpyChat->getLoginName().str());
			for (int i=0; i<maxSel; i++)
			{
				if (selections[i] != -1)
				{
					aStr.translate(GadgetListBoxGetText(playerListbox, selections[i], 0));
					if (aStr.compareNoCase(TheGameSpyChat->getLoginName()))
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
				if (isAction)
				{
					peerMessagePlayer(TheGameSpyChat->getPeer(), names.str(), UnicodeStringToQuotedPrintable(message).str(), ActionMessage);
				}
				else
				{
					peerMessagePlayer(TheGameSpyChat->getPeer(), names.str(), UnicodeStringToQuotedPrintable(message).str(), NormalMessage);
				}
			}

			return true;
		}
	}
	return false;
}

void RoomMessageCallback(PEER peer, RoomType roomType,
												 const char * nick, const char * message,
												 MessageType messageType, void * param)
{
	DEBUG_LOG(("RoomMessageCallback"));
	handleUnicodeMessage(nick, QuotedPrintableToUnicodeString(message), true, (messageType == ActionMessage));
}

void PlayerMessageCallback(PEER peer,
												 const char * nick, const char * message,
												 MessageType messageType, void * param)
{
	DEBUG_LOG(("PlayerMessageCallback"));
	handleUnicodeMessage(nick, QuotedPrintableToUnicodeString(message), false, (messageType == ActionMessage));
}

static handleUnicodeMessage( const char *nick, UnicodeString msg, Bool isPublic, Bool isAction )
{
	GameSpyColors style;

	Bool isOwner = false;
	Int flags = 0;
	if (TheGameSpyChat->getCurrentGroupRoomID())
		peerGetPlayerFlags(TheGameSpyChat->getPeer(), nick, GroupRoom, &flags);
	else
		peerGetPlayerFlags(TheGameSpyChat->getPeer(), nick, StagingRoom, &flags);
	isOwner = flags & PEER_FLAG_OP;

	if (isPublic && isAction)
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
	name.translate(nick);

	// filters language
//  if( TheGlobalData->m_languageFilterPref )
//  {
    TheLanguageFilter->filterLine(msg);
//	}

	UnicodeString fullMsg;
	if (isAction)
	{
		fullMsg.format( L"%ls %ls", name.str(), msg.str() );
	}
	else
	{
		fullMsg.format( L"[%ls] %ls", name.str(), msg.str() );
	}
	GameSpyAddText(fullMsg, style);
}

void GameSpyAddText( UnicodeString message, GameSpyColors color )
{
	GameWindow *textWindow = nullptr;

	if (!textWindow)
		textWindow = listboxLobbyChat;
	if (!textWindow)
		textWindow = listboxGameSetupChat;

	if (!textWindow)
		return;

	GadgetListBoxAddEntryText(textWindow, message, GameSpyColor[color], -1, -1);

}

