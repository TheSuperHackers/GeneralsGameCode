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
// TheSuperHackers @feature for hold to aim quick cast
#include "Common/GlobalData.h"
#include "Common/OptionPreferences.h"
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
	GameMessageDisposition disp = KEEP_MESSAGE;
	GameMessage::Type t = msg->getType();

	// TheSuperHackers @feature In QuickCastWithIndicator mode a hotkey arms on key down so the
	// player can see the targeting decal and still adjust aim, then fires on key up. Every other
	// mode keeps the original behaviour of acting only on key up.
#if RTS_ZEROHOUR
	const Bool holdToAim = TheGlobalData &&
		TheGlobalData->m_castMode == CastMode_QuickCastWithIndicator;
#else
	const Bool holdToAim = FALSE;
#endif

	if ( t == GameMessage::MSG_RAW_KEY_UP || (holdToAim && t == GameMessage::MSG_RAW_KEY_DOWN) )
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
		if( TheHotKeyManager )
		{
			// key down arms and previews, key up commits at wherever the cursor ended up
			HotKeyManager::setQuickCastAiming( holdToAim && t == GameMessage::MSG_RAW_KEY_DOWN );

			if( TheHotKeyManager->executeHotKey(aKey) )
				disp = DESTROY_MESSAGE;

			HotKeyManager::setQuickCastAiming( FALSE );
		}
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

// TheSuperHackers @feature See HotKey.h -- set only while synthesizing a button press.
Bool HotKeyManager::s_executingHotKey = FALSE;
Bool HotKeyManager::s_quickCastAiming = FALSE;

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
		// TheSuperHackers @feature A button that is merely not ready yet -- a recharging ability,
		// or a weapon still working through its burst -- is disabled, which normally swallows the
		// hotkey outright. In quick cast let it through anyway.
		//
		// Without this a repeat press is dropped here, before quick cast ever runs, and the only
		// way to retarget is to Stop first. A FIRE_WEAPON cameo reports COMMAND_NOT_READY for as
		// long as its weapon is not READY_TO_FIRE, so any unit still shooting has its own button
		// disabled underneath the player. Retargeting mid burst is expected behaviour -- see the
		// DragonTank firewall note in ControlBarCommand.cpp, which describes the same case.
		//
		// Deliberately narrow: this only opens up buttons disabled by WIN_STATUS_NOT_READY. Ones
		// that are restricted or unaffordable stay rejected, and the order still only does
		// anything if the logic side accepts it, so this cannot fire something that is genuinely
		// unavailable -- it just stops the keypress being thrown away before it is even looked at.
#if RTS_ZEROHOUR
		Bool allowWhileNotReady = FALSE;
		if( !BitIsSet( win->winGetStatus(), WIN_STATUS_ENABLED ) &&
				BitIsSet( win->winGetStatus(), WIN_STATUS_NOT_READY ) &&
				TheGlobalData &&
				TheGlobalData->m_castMode != CastMode_Normal )
		{
			allowWhileNotReady = TRUE;
		}
#else
		const Bool allowWhileNotReady = FALSE;
#endif

		if( BitIsSet( win->winGetStatus(), WIN_STATUS_ENABLED ) || allowWhileNotReady )
 		{
			// TheSuperHackers @feature Tell the command bar this press came from the keyboard, so
			// quick cast can fire at the cursor. A mouse click on the cameo leaves the cursor over
			// the control bar, where there is nothing sensible to target.
			HotKeyManager::setExecutingHotKey( TRUE );
 			TheWindowManager->winSendSystemMsg( win->winGetParent(), GBM_SELECTED, (WindowMsgData)win, win->winGetWindowId() );
			HotKeyManager::setExecutingHotKey( FALSE );

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

