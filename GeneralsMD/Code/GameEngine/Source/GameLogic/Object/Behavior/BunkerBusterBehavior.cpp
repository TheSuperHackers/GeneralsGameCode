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

// FILE: BunkerBusterBehavior.cpp ////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen, June 2003
// Desc:   Behavior module for Bunker Buster... it kills garrisoned objects
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/Upgrade.h"
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameLogic/Module/BunkerBusterBehavior.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"

#include "GameClient/TerrainVisual.h"//Seismic simulations!




static DomeStyleSeismicFilter bunkerBusterHeavingEarthSeismicFilter;


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BunkerBusterBehaviorModuleData::BunkerBusterBehaviorModuleData( void )
{

	m_upgradeRequired = nullptr;
	m_detonationFX = nullptr;
  m_crashThroughBunkerFX = nullptr;
  m_crashThroughBunkerFXFrequency = 4;

  m_seismicEffectRadius = 140.0f;
  m_seismicEffectMagnitude = 6.0f;

  m_shockwaveWeaponTemplate = nullptr;
  m_occupantDamageWeaponTemplate = nullptr;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void BunkerBusterBehaviorModuleData::buildFieldParse( MultiIniFieldParse &p )
{
  UpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "UpgradeRequired",	              INI::parseAsciiString,	        nullptr, offsetof( BunkerBusterBehaviorModuleData, m_upgradeRequired ) },
		{ "DetonationFX",			              INI::parseFXList,				        nullptr, offsetof( BunkerBusterBehaviorModuleData, m_detonationFX ) },
		{ "CrashThroughBunkerFX",			      INI::parseFXList,				        nullptr, offsetof( BunkerBusterBehaviorModuleData, m_crashThroughBunkerFX ) },
		{ "CrashThroughBunkerFXFrequency",	INI::parseDurationUnsignedInt,	nullptr, offsetof( BunkerBusterBehaviorModuleData, m_crashThroughBunkerFXFrequency ) },
		{ "SeismicEffectRadius",			      INI::parseReal,				          nullptr, offsetof( BunkerBusterBehaviorModuleData, m_seismicEffectRadius ) },
		{ "SeismicEffectMagnitude",	        INI::parseReal,	                nullptr, offsetof( BunkerBusterBehaviorModuleData, m_seismicEffectMagnitude ) },
    { "ShockwaveWeaponTemplate",        INI::parseWeaponTemplate,       nullptr, offsetof( BunkerBusterBehaviorModuleData, m_shockwaveWeaponTemplate ) },
    { "OccupantDamageWeaponTemplate",   INI::parseWeaponTemplate,       nullptr, offsetof( BunkerBusterBehaviorModuleData, m_occupantDamageWeaponTemplate ) },

		{ nullptr, nullptr, nullptr, 0 }
	};

  p.add( dataFieldParse );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BunkerBusterBehavior::BunkerBusterBehavior( Thing *thing, const ModuleData *modData )
											 : UpdateModule( thing, modData )
{
	// THIS HAS AN UPDATE... BECAUSE I FORSEE THE NEED FOR ONE, BUT RIGHT NOW IT DOES NOTHING
	setWakeFrame( getObject(), UPDATE_SLEEP_NONE );
  m_victimID = INVALID_ID;
  m_upgradeRequired = nullptr;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BunkerBusterBehavior::~BunkerBusterBehavior( void )
{

}



void BunkerBusterBehavior::onObjectCreated( void )
{
	const BunkerBusterBehaviorModuleData *modData = getBunkerBusterBehaviorModuleData();

	// convert module upgrade name to a pointer
	m_upgradeRequired = TheUpgradeCenter->findUpgrade( modData->m_upgradeRequired );

}


// ------------------------------------------------------------------------------------------------
/** The update callback */
// ------------------------------------------------------------------------------------------------
UpdateSleepTime BunkerBusterBehavior::update( void )
{
  const BunkerBusterBehaviorModuleData *modData = getBunkerBusterBehaviorModuleData();
  AIUpdateInterface *ai = getObject()->getAI();
  if ( ai )// is this a SMART bomb?
  {
    if ( m_victimID == INVALID_ID )
    {
      Object *victim = ai->getCurrentVictim();
      if ( victim )
        m_victimID = victim->getID();
      DEBUG_ASSERTCRASH( victim, ("BunkerBusterBehavior::update... AIUpdateInterface reports no victim." ) );
    }
    DEBUG_ASSERTCRASH( ai, ("BunkerBusterBehavior::update could not find an AIUpdateInterface." ) );


    if ( TheGameLogic->getFrame()%modData->m_crashThroughBunkerFXFrequency == 1 )// not too much
    {
      const FXList *crashFX = modData->m_crashThroughBunkerFX;
      if ( getObject()->testStatus( OBJECT_STATUS_MISSILE_KILLING_SELF ) && crashFX )
        FXList::doFXObj( crashFX, getObject() );// CrashFX done on the missile/bomb
    }

  }





	return UPDATE_SLEEP_NONE;

}

// ------------------------------------------------------------------------------------------------
/** The death callback */
// ------------------------------------------------------------------------------------------------
void BunkerBusterBehavior::onDie( const DamageInfo *damageInfo )
{
  // do what we came here to do!
  bustTheBunker();
}





// ------------------------------------------------------------------------------------------------
/** The bunker-busting effect callback */
// ------------------------------------------------------------------------------------------------
void BunkerBusterBehavior::bustTheBunker( void )
{
	const BunkerBusterBehaviorModuleData *modData = getBunkerBusterBehaviorModuleData();

  if ( m_upgradeRequired != nullptr )
  {
	  Bool weaponUpgraded = getObject()->getControllingPlayer()->hasUpgradeComplete( m_upgradeRequired );
    if ( ! weaponUpgraded )
      return;
  }


//  here is where we kill everyone inside any targeted garrisoned buildings
//  AIUpdateInterface *ai = getObject()->getAI();
  Object *target = TheGameLogic->findObjectByID( m_victimID );

  Object *objectForFX = getObject();

  if ( target ) // Was the pilot aiming at an object?
  {
    objectForFX = target;

    ContainModuleInterface *contain = target->getContain();
    if ( contain && contain->isBustable() ) // Was that object something that bunkerbusters bust?
    {

      if ( modData->m_occupantDamageWeaponTemplate )
      {
			  DamageInfo damageInfo;
			  damageInfo.in.m_damageType = modData->m_occupantDamageWeaponTemplate->getDamageType();
			  damageInfo.in.m_deathType = modData->m_occupantDamageWeaponTemplate->getDeathType();
			  damageInfo.in.m_sourceID = getObject()->getID();
			  damageInfo.in.m_sourcePlayerMask = getObject()->getControllingPlayer()->getPlayerMask();
			  damageInfo.in.m_amount = 100.0f;
        contain->harmAndForceExitAllContained( &damageInfo ); // Ouch!
      }
      else
        contain->killAllContained();



    }
  }

  const FXList *detonationFX = modData->m_detonationFX;
  if ( detonationFX )
  	FXList::doFXObj( detonationFX, objectForFX );//DetonationFX done on the building

#ifdef DO_SEISMIC_SIMULATIONS
  // Okay, the right proper way to do this is to add SeismicSim support to FXList...
  // But until that day, I'm just gonna do it here,  sorry, M Lorenzen 6/26/03
  SeismicSimulationNode sim(
    objectForFX->getPosition(),
    modData->m_seismicEffectRadius,
    modData->m_seismicEffectMagnitude,
    &bunkerBusterHeavingEarthSeismicFilter );

  TheTerrainVisual->addSeismicSimulation( sim );
#endif

  if ( modData->m_shockwaveWeaponTemplate )
		TheWeaponStore->createAndFireTempWeapon(modData->m_shockwaveWeaponTemplate, objectForFX, objectForFX->getPosition());


}

// ------------------------------------------------------------------------------------------------








// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BunkerBusterBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BunkerBusterBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BunkerBusterBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
