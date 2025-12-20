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

// FILE: SpecialPowerCompletionDie.cpp ////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, May 2002
// Desc:   Die method responsible for telling TheScriptEngine that a special power has been completed
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/SpecialPower.h"
#include "Common/Xfer.h"
#include "GameLogic/Module/SpecialPowerCompletionDie.h"
#include "GameLogic/Object.h"
#include "GameLogic/ScriptEngine.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpecialPowerCompletionDie::SpecialPowerCompletionDie( Thing *thing, const ModuleData* moduleData ) : DieModule( thing, moduleData )
{
	m_creatorID = INVALID_ID;
	m_creatorSet = FALSE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SpecialPowerCompletionDie::~SpecialPowerCompletionDie( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerCompletionDie::onDie( const DamageInfo *damageInfo )
{
	if (!isDieApplicable(damageInfo))
		return;
	notifyScriptEngine();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerCompletionDie::notifyScriptEngine( void )
{
	if (m_creatorID != INVALID_ID)
	{
		TheScriptEngine->notifyOfCompletedSpecialPower(
			getObject()->getControllingPlayer()->getPlayerIndex(),
			getSpecialPowerCompletionDieModuleData()->m_specialPowerTemplate->getName(),
			m_creatorID);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void SpecialPowerCompletionDie::setCreator( ObjectID creatorID )
{
	if (!m_creatorSet)
	{
		m_creatorSet = TRUE;
		m_creatorID = creatorID;
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SpecialPowerCompletionDie::crc( Xfer *xfer )
{

	// extend base class
	DieModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SpecialPowerCompletionDie::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DieModule::xfer( xfer );

	// creator id
	xfer->xferObjectID( &m_creatorID );

	// creator set
	xfer->xferBool( &m_creatorSet );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SpecialPowerCompletionDie::loadPostProcess( void )
{

	// extend base class
	DieModule::loadPostProcess();

}
