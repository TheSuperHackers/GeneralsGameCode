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

// FILE: HotKey.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Sep 2002
//
//	Filename: 	HotKey.cpp
//
//	author:		Chris Huybregts
//
//	purpose:
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "GameClient/HotKey.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/MetaEvent.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Keyboard.h"
#include "GameClient/GameText.h"
#include "Common/AudioEventRTS.h"
//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
GameMessageDisposition HotKeyTranslator::translateGameMessage(const GameMessage *msg)
{
	switch (msg->getType())
	{
	case GameMessage::MSG_RAW_KEY_DOWN:
		if ((msg->getArgument(1)->integer & KEY_STATE_AUTOREPEAT) == 0)
			return KEEP_MESSAGE;

		FALLTHROUGH;
	case GameMessage::MSG_RAW_KEY_UP:
		if (msg->getArgument(1)->integer & (KEY_STATE_CONTROL | KEY_STATE_SHIFT | KEY_STATE_ALT))
			return KEEP_MESSAGE;

		if (TheHotKeyManager)
		{
			WideChar key = TheKeyboard->getPrintableKey((KeyDefType)msg->getArgument(0)->integer, 0);
			UnicodeString uKey;
			uKey.concat(key);
			AsciiString aKey;
			aKey.translate(uKey);

			if (TheHotKeyManager->executeHotKey(aKey))
				return DESTROY_MESSAGE;
		}

		return KEEP_MESSAGE;
	default:
		return KEEP_MESSAGE;
	}
}

//-----------------------------------------------------------------------------
HotKey::HotKey()
{
	m_win = nullptr;
	m_key.clear();
}

//-----------------------------------------------------------------------------
HotKeyManager::HotKeyManager( void )
{

}

//-----------------------------------------------------------------------------
HotKeyManager::~HotKeyManager( void )
{
	m_hotKeyMap.clear();
}

//-----------------------------------------------------------------------------
void HotKeyManager::init( void )
{
	m_hotKeyMap.clear();
}

//-----------------------------------------------------------------------------
void HotKeyManager::reset( void )
{
	m_hotKeyMap.clear();
}

//-----------------------------------------------------------------------------
void HotKeyManager::addHotKey( GameWindow *win, const AsciiString& keyIn)
{
	AsciiString key = keyIn;
	key.toLower();
	HotKeyMap::iterator it = m_hotKeyMap.find(key);
	if( it != m_hotKeyMap.end() )
	{
		DEBUG_ASSERTCRASH(FALSE,("Hotkey %s is already mapped to window %s, current window is %s", key.str(), it->second.m_win->winGetInstanceData()->m_decoratedNameString.str(), win->winGetInstanceData()->m_decoratedNameString.str()));
		return;
	}
	HotKey newHK;
	newHK.m_key.set(key);
	newHK.m_win = win;
	m_hotKeyMap[key] = newHK;
}

//-----------------------------------------------------------------------------
Bool HotKeyManager::executeHotKey( const AsciiString& keyIn )
{
	AsciiString key = keyIn;
	key.toLower();
	HotKeyMap::iterator it = m_hotKeyMap.find(key);
	if( it == m_hotKeyMap.end() )
		return FALSE;
	GameWindow *win = it->second.m_win;
	if( !win )
		return FALSE;
	if( !BitIsSet( win->winGetStatus(), WIN_STATUS_HIDDEN ) )
	{
		if( BitIsSet( win->winGetStatus(), WIN_STATUS_ENABLED ) )
 		{
 			TheWindowManager->winSendSystemMsg( win->winGetParent(), GBM_SELECTED, (WindowMsgData)win, win->winGetWindowId() );

 			// here we make the same click sound that the GUI uses when you click a button
 			AudioEventRTS buttonClick("GUIClick");

 			if( TheAudio )
 			{
 				TheAudio->addAudioEvent( &buttonClick );
 			}
			return TRUE;
 		}

		AudioEventRTS disabledClick( "GUIClickDisabled" );
		if( TheAudio )
		{
			TheAudio->addAudioEvent( &disabledClick );
		}
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
AsciiString HotKeyManager::searchHotKey( const AsciiString& label)
{
	return searchHotKey(TheGameText->fetch(label));
}

//-----------------------------------------------------------------------------
AsciiString HotKeyManager::searchHotKey( const UnicodeString& uStr )
{
	if(uStr.isEmpty())
		return AsciiString::TheEmptyString;

	const WideChar *marker = (const WideChar *)uStr.str();
	while (marker && *marker)
	{
		if (*marker == L'&')
		{
			// found a '&' - now look for the next char
			UnicodeString tmp = UnicodeString::TheEmptyString;
			tmp.concat(*(marker+1));
			AsciiString retStr;
			retStr.translate(tmp);
			return retStr;
		}
		marker++;
	}
	return AsciiString::TheEmptyString;
}

//-----------------------------------------------------------------------------
HotKeyManager *TheHotKeyManager = nullptr;

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

