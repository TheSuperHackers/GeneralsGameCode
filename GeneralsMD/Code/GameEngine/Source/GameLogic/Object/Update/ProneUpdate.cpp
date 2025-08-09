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

// FILE: ProneUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:   Update module to encapsulate what it means to be "prone"
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"

#include "GameClient/Drawable.h"

#include "GameLogic/Damage.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ProneUpdate.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProneUpdateModuleData::ProneUpdateModuleData() :
  m_damageToFramesRatio(1.0f)
{
}

//-------------------------------------------------------------------------------------------------
/*static*/ void ProneUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "DamageToFramesRatio", INI::parseReal, NULL, offsetof(ProneUpdateModuleData, m_damageToFramesRatio) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
ProneUpdate::ProneUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_proneFrames = 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProneUpdate::~ProneUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime ProneUpdate::update( void )
{
/// @todo srj use SLEEPY_UPDATE here
	if( m_proneFrames > 0 )
	{
		m_proneFrames--;
		if( m_proneFrames == 0 )
			stopProneEffects();
	}
	return UPDATE_SLEEP_NONE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ProneUpdate::goProne( const DamageInfo *damageInfo )
{
	//add to the prone time
	Bool wasProne = (m_proneFrames > 0);
	Int damageTaken = damageInfo->out.m_actualDamageDealt;
	m_proneFrames += damageTaken * getProneUpdateModuleData()->m_damageToFramesRatio;

	if( !wasProne && (m_proneFrames > 0) )
		startProneEffects();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ProneUpdate::startProneEffects()
{
	Object *me = getObject();
	me->getDrawable()->setModelConditionState( MODELCONDITION_PRONE );
	me->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_NO_ATTACK ) );
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ProneUpdate::stopProneEffects()
{
	Object *me = getObject();
	me->getDrawable()->clearModelConditionState( MODELCONDITION_PRONE );
	me->clearStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_NO_ATTACK ) );
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ProneUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ProneUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// prone frames
	xfer->xferInt( &m_proneFrames );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ProneUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess

