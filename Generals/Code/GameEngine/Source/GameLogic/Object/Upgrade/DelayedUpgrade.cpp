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

// FILE: DelayedUpgrade.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, April 2002
// Desc:	 An Upgrade that broadcasts to all DelayedUpgradeUpdates that maybe they should start 
//					counting down to execution
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/DelayedUpgrade.h"
#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DelayedUpgrade::DelayedUpgrade( Thing *thing, const ModuleData* moduleData ) : UpgradeModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DelayedUpgrade::~DelayedUpgrade( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void DelayedUpgrade::upgradeImplementation( )
{
	UnsignedInt delay = getDelayedUpgradeModuleData()->m_delayTime;
	DelayedUpgradeUpdateInterface *upgradeUpdate = NULL;
	Object *me = getObject();

	UpgradeMaskType activation, conflicting;
	getUpgradeActivationMasks(activation, conflicting);

	for (BehaviorModule** u = me->getBehaviorModules(); *u; ++u)
	{
		// Check all Upgradeupdate modules for firing
		if ((upgradeUpdate = (*u)->getDelayedUpgradeUpdateInterface()) != NULL)
		{
			if( upgradeUpdate->isTriggeredBy( activation ) )
			{
				// My mask is valid because otherwise I myself would have never fired.  So okay to send it
				upgradeUpdate->setDelay( delay );
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void DelayedUpgrade::crc( Xfer *xfer )
{

	// extend base class
	UpgradeModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void DelayedUpgrade::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpgradeModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void DelayedUpgrade::loadPostProcess( void )
{

	// extend base class
	UpgradeModule::loadPostProcess();

}  // end loadPostProcess
