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

// FILE: BoneFXDamage.cpp ///////////////////////////////////////////////////////////////////
// Author: Bryan Cleveland, March 2002
// Desc:   Damage module that goes with BoneFX update
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BoneFXDamage.h"
#include "GameLogic/Module/BoneFXUpdate.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BoneFXDamage::BoneFXDamage( Thing *thing, const ModuleData* moduleData )
																		  : DamageModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BoneFXDamage::~BoneFXDamage( void )
{

}

//-------------------------------------------------------------------------------------------------
void BoneFXDamage::onObjectCreated()
{
	static NameKeyType key_BoneFXUpdate = NAMEKEY("BoneFXUpdate");
	BoneFXUpdate* bfxu = (BoneFXUpdate*)getObject()->findUpdateModule(key_BoneFXUpdate);
	if (bfxu == NULL)
	{
		DEBUG_ASSERTCRASH(bfxu != NULL, ("BoneFXDamage requires BoneFXUpdate"));
		throw INI_INVALID_DATA;
	}
}

//-------------------------------------------------------------------------------------------------
/** Switching damage states */
//-------------------------------------------------------------------------------------------------
void BoneFXDamage::onBodyDamageStateChange( const DamageInfo *damageInfo,
																						BodyDamageType oldState,
																						BodyDamageType newState )
{
	static NameKeyType key_BoneFXUpdate = NAMEKEY("BoneFXUpdate");
	BoneFXUpdate* bfxu = (BoneFXUpdate*)getObject()->findUpdateModule(key_BoneFXUpdate);
	if (bfxu)
		bfxu->changeBodyDamageState(oldState, newState);

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BoneFXDamage::crc( Xfer *xfer )
{

	// extend base class
	DamageModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BoneFXDamage::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DamageModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BoneFXDamage::loadPostProcess( void )
{

	// extend base class
	DamageModule::loadPostProcess();

}
