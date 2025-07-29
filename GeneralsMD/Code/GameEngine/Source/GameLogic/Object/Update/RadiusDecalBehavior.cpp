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

// FILE: RadiusDecalBehavior.cpp ///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/RadiusDecalBehavior.h"
#include "GameLogic/Object.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

RadiusDecalBehaviorModuleData::RadiusDecalBehaviorModuleData()
{
	m_initiallyActive = false;
	m_decalRadius = 0.0f;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/*static*/ void RadiusDecalBehaviorModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	UpdateModuleData::buildFieldParse(p);
	static const FieldParse dataFieldParse[] =
	{
		{ "StartsActive",	INI::parseBool, NULL, offsetof(RadiusDecalBehaviorModuleData, m_initiallyActive) },
		{ "RadiusDecal",						RadiusDecalTemplate::parseRadiusDecalTemplate,	NULL, offsetof( RadiusDecalBehaviorModuleData, m_decalTemplate) },
		{ "Radius",			INI::parseReal,									NULL,	offsetof( RadiusDecalBehaviorModuleData, m_decalRadius) },
		{ 0, 0, 0, 0 }
	};

	BehaviorModuleData::buildFieldParse(p);
	p.add(dataFieldParse);
	p.add(UpgradeMuxData::getFieldParse(), offsetof(RadiusDecalBehaviorModuleData, m_upgradeMuxData));
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RadiusDecalBehavior::RadiusDecalBehavior( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	if (getRadiusDecalBehaviorModuleData()->m_initiallyActive)
	{
		giveSelfUpgrade();
	}
	else {
		clearDecal();
		setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RadiusDecalBehavior::~RadiusDecalBehavior( void )
{
	clearDecal();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RadiusDecalBehavior::createRadiusDecal( void )
{
	const RadiusDecalBehaviorModuleData* data = getRadiusDecalBehaviorModuleData();
	const RadiusDecalTemplate& tmpl = data->m_decalTemplate;
	m_radiusDecal.clear();
	if (tmpl.valid()) {
		// DEBUG_LOG(("RadiusDecalBehavior::createRadiusDecal: \n"));
		// tmpl.debugPrint();
		tmpl.createRadiusDecal(*(getObject()->getPosition()), data->m_decalRadius, getObject()->getControllingPlayer(), m_radiusDecal);
		setWakeFrame(getObject(), m_radiusDecal.isEmpty() ? UPDATE_SLEEP_FOREVER : UPDATE_SLEEP_NONE);
	}
	else {
		// We don't have a decal defined. Do we need this?
		setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RadiusDecalBehavior::killRadiusDecal()
{
	clearDecal();
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

// -----------------------------------------------------------------------------------------------
void RadiusDecalBehavior::clearDecal()
{
	m_radiusDecal.clear();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime RadiusDecalBehavior::update( void )
{
	if (getObject()->isDisabledByType(DISABLED_HELD)) {
		if (!m_radiusDecal.isEmpty())
			clearDecal();
		return UPDATE_SLEEP_NONE;  // We wait to be re-enabled
	}

	// Upgrade has not been triggered, or it might have been removed.
	if (!isUpgradeActive()) {
		clearDecal();
		return UPDATE_SLEEP_FOREVER;
	}

	// The object is dead
	if (getObject()->isEffectivelyDead()) {
		clearDecal();
		return UPDATE_SLEEP_FOREVER;
	}

	// This should be our usual case
	if (!m_radiusDecal.isEmpty()) {
		m_radiusDecal.update();
		m_radiusDecal.setPosition(*(getObject()->getPosition()));
		return UPDATE_SLEEP_NONE;
	}

	// We get here if we were disabled
	createRadiusDecal();
	return UPDATE_SLEEP_NONE;

	// Something probably went wrong if we reach this point
	//return UPDATE_SLEEP_FOREVER;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void RadiusDecalBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void RadiusDecalBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// decal, if any
	m_radiusDecal.xferRadiusDecal(xfer);


}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void RadiusDecalBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
