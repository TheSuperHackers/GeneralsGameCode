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
#include "Common/UserPreferences.h"
//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
GameMessageDisposition HotKeyTranslator::translateGameMessage(const GameMessage *msg)
{
	GameMessageDisposition disp = KEEP_MESSAGE;
	GameMessage::Type t = msg->getType();

	if ( t == GameMessage::MSG_RAW_KEY_UP)
	{

		//char key = msg->getArgument(0)->integer;
		Int keyState = msg->getArgument(1)->integer;

		// for our purposes here, we don't care to distinguish between right and left keys,
		// so just fudge a little to simplify things.
		Int newModState = 0;

		if( keyState & KEY_STATE_CONTROL )
		{
			newModState |= CTRL;
		}

		if( keyState & KEY_STATE_SHIFT )
		{
			newModState |= SHIFT;
		}

		if( keyState & KEY_STATE_ALT )
		{
			newModState |= ALT;
		}
		if(newModState != 0)
			return disp;
		WideChar key = TheKeyboard->getPrintableKey((KeyDefType)msg->getArgument(0)->integer, 0);
		UnicodeString uKey;
		uKey.concat(key);
		AsciiString aKey;
		aKey.translate(uKey);
		if(TheHotKeyManager && TheHotKeyManager->executeHotKey(aKey))
			disp = DESTROY_MESSAGE;
	}
	return disp;
}

//-----------------------------------------------------------------------------
HotKey::HotKey()
{
	m_win = nullptr;
	m_key.clear();
}

//-----------------------------------------------------------------------------
HotKeyManager::HotKeyManager()
{

}

//-----------------------------------------------------------------------------
HotKeyManager::~HotKeyManager()
{
	m_hotKeyMap.clear();
}

//-----------------------------------------------------------------------------
void HotKeyManager::init()
{
	m_hotKeyMap.clear();
	loadOverrides();
}

//-----------------------------------------------------------------------------
void HotKeyManager::reset()
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
		DEBUG_CRASH(("Hotkey %s is already mapped to window %s, current window is %s", key.str(), it->second.m_win->winGetInstanceData()->m_decoratedNameString.str(), win->winGetInstanceData()->m_decoratedNameString.str()));
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
// HOTKEY OVERRIDE METHODS (custom per-CommandButton remapping)
//-----------------------------------------------------------------------------

static const char* HOTKEY_OVERRIDES_FILENAME = "HotKeyOverrides.ini";
static const char* HOTKEY_OVERRIDE_PREFIX    = "HotKey_";

//-----------------------------------------------------------------------------
void HotKeyManager::setOverride( const AsciiString& commandButtonName, const AsciiString& newKey )
{
	if (commandButtonName.isEmpty() || newKey.isEmpty())
		return;
	AsciiString lowerKey = newKey;
	lowerKey.toLower();
	m_hotKeyOverrides[commandButtonName] = lowerKey;
}

//-----------------------------------------------------------------------------
void HotKeyManager::removeOverride( const AsciiString& commandButtonName )
{
	m_hotKeyOverrides.erase(commandButtonName);
}

//-----------------------------------------------------------------------------
AsciiString HotKeyManager::getOverride( const AsciiString& commandButtonName ) const
{
	OverrideMap::const_iterator it = m_hotKeyOverrides.find(commandButtonName);
	if (it != m_hotKeyOverrides.end())
		return it->second;
	return AsciiString::TheEmptyString;
}

//-----------------------------------------------------------------------------
void HotKeyManager::clearAllOverrides()
{
	m_hotKeyOverrides.clear();
}

//-----------------------------------------------------------------------------
void HotKeyManager::loadOverrides()
{
	m_hotKeyOverrides.clear();
	UserPreferences prefs;
	if (!prefs.load(HOTKEY_OVERRIDES_FILENAME))
		return;  // no saved overrides, that's fine

	const size_t prefixLen = strlen(HOTKEY_OVERRIDE_PREFIX);
	for (PreferenceMap::const_iterator it = prefs.begin(); it != prefs.end(); ++it)
	{
		const AsciiString& key = it->first;
		if (key.getLength() > (Int)prefixLen)
		{
			// strip "HotKey_" prefix to recover the CommandButton name
			AsciiString cmdName = AsciiString(key.str() + prefixLen);
			AsciiString val = it->second;
			val.toLower();
			if (cmdName.isNotEmpty() && val.isNotEmpty())
				m_hotKeyOverrides[cmdName] = val;
		}
	}
}

//-----------------------------------------------------------------------------
void HotKeyManager::saveOverrides()
{
	UserPreferences prefs;
	// don't bother loading existing file; we overwrite completely
	prefs.load(HOTKEY_OVERRIDES_FILENAME);  // sets internal filename

	// clear any old HotKey_ entries
	for (PreferenceMap::iterator it = prefs.begin(); it != prefs.end(); )
	{
		if (strncmp(it->first.str(), HOTKEY_OVERRIDE_PREFIX, strlen(HOTKEY_OVERRIDE_PREFIX)) == 0)
			it = prefs.erase(it);
		else
			++it;
	}

	// write current overrides
	for (OverrideMap::const_iterator it = m_hotKeyOverrides.begin();
			 it != m_hotKeyOverrides.end(); ++it)
	{
		AsciiString prefKey;
		prefKey.format("%s%s", HOTKEY_OVERRIDE_PREFIX, it->first.str());
		prefs.setAsciiString(prefKey, it->second);
	}

	prefs.write();
}

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

