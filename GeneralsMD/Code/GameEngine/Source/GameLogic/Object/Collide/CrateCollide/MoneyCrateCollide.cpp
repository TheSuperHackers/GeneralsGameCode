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

// FILE: MoneyCrateCollide.cpp ///////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:   A crate that gives x money to the collider
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/AudioEventRTS.h"
#include "Common/MiscAudio.h"
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/MoneyCrateCollide.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MoneyCrateCollide::MoneyCrateCollide( Thing *thing, const ModuleData* moduleData ) : CrateCollide( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
MoneyCrateCollide::~MoneyCrateCollide( void )
{

}

//-------------------------------------------------------------------------------------------------
Bool MoneyCrateCollide::executeCrateBehavior( Object *other )
{
	UnsignedInt money = getMoneyCrateCollideModuleData()->m_moneyProvided;

	money += getUpgradedSupplyBoost(other);

	other->getControllingPlayer()->getMoney()->deposit( money );
	other->getControllingPlayer()->getScoreKeeper()->addMoneyEarned( money );

	//Play a crate pickup sound.
	AudioEventRTS soundToPlay = TheAudio->getMiscAudio()->m_crateMoney;
	soundToPlay.setObjectID( other->getID() );
	TheAudio->addAudioEvent(&soundToPlay);

	return TRUE;
}

//------------------------------------------------------------------------------------------------
Int MoneyCrateCollide::getUpgradedSupplyBoost( Object *other ) const
{
	Player *player = other->getControllingPlayer();
	if (!player) return 0;

	// Loop through the upgrade pairs and see if an upgrade is present that adds supply boost
	std::list<upgradePair>::const_iterator it = getMoneyCrateCollideModuleData()->m_upgradeBoost.begin();
	while (it != getMoneyCrateCollideModuleData()->m_upgradeBoost.end())
	{
		upgradePair info = *it;

		// Check if the player has the desired upgrade. If so return the boost
		static const UpgradeTemplate *upgradeTemplate = TheUpgradeCenter->findUpgrade( info.type.c_str() );
		if (player && upgradeTemplate && player->hasUpgradeComplete(upgradeTemplate))
		{
			return info.amount;
		}

		// check next
		++it;
	}

	return 0;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void MoneyCrateCollide::crc( Xfer *xfer )
{

	// extend base class
	CrateCollide::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void MoneyCrateCollide::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	CrateCollide::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void MoneyCrateCollide::loadPostProcess( void )
{

	// extend base class
	CrateCollide::loadPostProcess();

}
