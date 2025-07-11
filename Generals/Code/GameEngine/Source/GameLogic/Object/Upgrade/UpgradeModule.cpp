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

// FILE: UpgradeModule.cpp ////////////////////////////////////////////////////////////////////////
// Author: Johnson, Day, Smallwood September 2002
// Desc:   Upgrade module basic implementations
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/Xfer.h"
#include "GameLogic/Module/UpgradeModule.h"


// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void UpgradeModule::crc( Xfer *xfer )
{

	// extend base class
	BehaviorModule::crc( xfer );
	
	// extned base class
	UpgradeMux::upgradeMuxCRC( xfer );
	
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void UpgradeModule::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// call base class
	BehaviorModule::xfer( xfer );

	// extend base class
	UpgradeMux::upgradeMuxXfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void UpgradeModule::loadPostProcess( void )
{

	// call base class
	BehaviorModule::loadPostProcess();

	// extend base class
	UpgradeMux::upgradeMuxLoadPostProcess();

}  // end loadPostProcess

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpgradeMux::UpgradeMux() : m_upgradeExecuted(false)
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool UpgradeMux::isAlreadyUpgraded() const 
{ 
	return m_upgradeExecuted; 
}

// ------------------------------------------------------------------------------------------------
// ***DANGER! DANGER! Don't use this, unless you are forcing an already made upgrade to refresh!!
// ------------------------------------------------------------------------------------------------
void UpgradeMux::forceRefreshUpgrade()
{
	if( m_upgradeExecuted )
	{
		//Only do this if we've already made the upgrade!
		upgradeImplementation();
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool UpgradeMux::attemptUpgrade( UpgradeMaskType keyMask )
{
	if (wouldUpgrade(keyMask))
	{
		// If I have an activation condition, and I haven't activated, and this key matches my condition.
		giveSelfUpgrade();
		return true;
	}
	return false;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool UpgradeMux::wouldUpgrade( UpgradeMaskType keyMask ) const
{
	UpgradeMaskType activation, conflicting;
	getUpgradeActivationMasks(activation, conflicting);

	//Make sure we have activation conditions and we haven't performed the upgrade already.
	if( activation.any() && keyMask.any() && !m_upgradeExecuted )
	{
		//Okay, make sure we don't have any conflicting upgrades
		if( !keyMask.testForAny( conflicting) )
		{
			//Finally check to see if our upgrade conditions match.
			if( requiresAllActivationUpgrades() )
			{
				//Make sure ALL triggers requirements are upgraded
				if( keyMask.testForAll( activation ) )
				{
					return TRUE;
				}
			}
			else 
			{
				//Check if ANY trigger requirements are met.
				if( keyMask.testForAny( activation ) )
				{
					return TRUE;
				}
			}
		}
	}
	//We can't upgrade!
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
Bool UpgradeMux::testUpgradeConditions( UpgradeMaskType keyMask ) const
{
	UpgradeMaskType activation, conflicting;
	getUpgradeActivationMasks(activation, conflicting);

	//Okay, make sure we don't have any conflicting upgrades
	if( !keyMask.any() || !keyMask.testForAny( conflicting ) )
	{
		//Make sure we have activation conditions
		if( activation.any() )
		{
			//Finally check to see if our upgrade conditions match.
			if( requiresAllActivationUpgrades() )
			{
				//Make sure ALL triggers requirements are upgraded
				if( keyMask.testForAll( activation ) )
				{
					return TRUE;
				}
			}
			else
			{
				//Check if ANY trigger requirements are met.
				if( keyMask.testForAny( activation ) )
				{
					return TRUE;
				}
			}
		}
		else
		{
			//This *upgrade* is relying only on not having conflicts.
			return true;
		}
	}
	//We can't upgrade!
	return false;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool UpgradeMux::resetUpgrade( UpgradeMaskType keyMask )
{
	UpgradeMaskType activation, conflicting;
	getUpgradeActivationMasks(activation, conflicting);
	if( keyMask.testForAny( activation ) && m_upgradeExecuted )
	{
		m_upgradeExecuted = false;
		return true;
	}
	return false;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UpgradeMux::upgradeMuxCRC( Xfer *xfer )
{

	// just call the regular xfer, it's simple
	upgradeMuxXfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer
	* Version Info
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void UpgradeMux::upgradeMuxXfer( Xfer *xfer )
{

	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// upgrade executed
	xfer->xferBool( &m_upgradeExecuted );

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UpgradeMux::upgradeMuxLoadPostProcess( void )
{

}
