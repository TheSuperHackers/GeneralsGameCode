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

// FILE: Money.cpp /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: Money.cpp
//
// Created:   Steven Johnson, October 2001
//
// Desc:      @todo
//
//-----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Money.h"

#include "Common/AudioSettings.h"
#include "Common/GameAudio.h"
#include "Common/MiscAudio.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Xfer.h"

// ------------------------------------------------------------------------------------------------
UnsignedInt Money::withdraw(UnsignedInt amountToWithdraw, Bool playSound)
{
#if defined(RTS_DEBUG)
	Player* player = ThePlayerList->getNthPlayer(m_playerIndex);
	if (player != NULL && player->buildsForFree())
		return 0;
#endif

	if (amountToWithdraw > m_money)
		amountToWithdraw = m_money;

	if (amountToWithdraw == 0)
		return amountToWithdraw;

	if (playSound)
	{
		triggerAudioEvent(TheAudio->getMiscAudio()->m_moneyWithdrawSound);
	}

	m_money -= amountToWithdraw;

	return amountToWithdraw;
}

// ------------------------------------------------------------------------------------------------
void Money::deposit(UnsignedInt amountToDeposit, Bool playSound)
{
	if (amountToDeposit == 0)
		return;

	if (playSound)
	{
		triggerAudioEvent(TheAudio->getMiscAudio()->m_moneyDepositSound);
	}

	m_money += amountToDeposit;
}

void Money::triggerAudioEvent(const AudioEventRTS& audioEvent)
{
	Real volume = TheAudio->getAudioSettings()->m_preferredMoneyTransactionVolume;
	volume *= audioEvent.getVolume();
	if (volume <= 0.0f)
		return;

	//@todo: Do we do this frequently enough that it is a performance hit?
	AudioEventRTS event = audioEvent;
	event.setPlayerIndex(m_playerIndex);
	event.setVolume(volume);
	TheAudio->addAudioEvent(&event);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void Money::crc( Xfer *xfer )
{

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void Money::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// money value
	xfer->xferUnsignedInt( &m_money );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void Money::loadPostProcess( void )
{

}


// ------------------------------------------------------------------------------------------------
/** Parse a money amount for the ini file. E.g. DefaultStartingMoney = 10000 */
// ------------------------------------------------------------------------------------------------
void Money::parseMoneyAmount( INI *ini, void *instance, void *store, const void* userData )
{
  // Someday, maybe, have mulitple fields like Gold:10000 Wood:1000 Tiberian:10
  Money * money = (Money *)store;
  INI::parseUnsignedInt( ini, instance, &money->m_money, userData );
}
