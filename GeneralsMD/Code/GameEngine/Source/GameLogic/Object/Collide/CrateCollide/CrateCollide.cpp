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

// FILE: CrateCollide.cpp ///////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:   Abstract base Class Crate Collide
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/BitFlagsIO.h"
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "Common/GameAudio.h"
#include "Common/MiscAudio.h"
#include "GameClient/Anim2D.h"
#include "GameClient/FXList.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Drawable.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/CrateCollide.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CrateCollideModuleData::CrateCollideModuleData()
{
	m_isForbidOwnerPlayer = FALSE;
	m_executeAnimationDisplayTimeInSeconds = 0.0f;
	m_executeAnimationZRisePerSecond = 0.0f;
	m_executeAnimationFades = TRUE;
	m_isBuildingPickup = FALSE;
	m_isHumanOnlyPickup = FALSE;
	m_executeFX = NULL;
	m_pickupScience = SCIENCE_INVALID;
	m_executionAnimationTemplate = AsciiString::TheEmptyString;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void CrateCollideModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "RequiredKindOf", KindOfMaskType::parseFromINI, NULL, offsetof( CrateCollideModuleData, m_kindof ) },
		{ "ForbiddenKindOf", KindOfMaskType::parseFromINI, NULL, offsetof( CrateCollideModuleData, m_kindofnot ) },
		{ "ForbidOwnerPlayer", INI::parseBool,	NULL,	offsetof( CrateCollideModuleData, m_isForbidOwnerPlayer ) },
		{ "BuildingPickup", INI::parseBool,	NULL,	offsetof( CrateCollideModuleData, m_isBuildingPickup ) },
		{ "HumanOnly", INI::parseBool,	NULL,	offsetof( CrateCollideModuleData, m_isHumanOnlyPickup ) },
		{ "PickupScience", INI::parseScience,	NULL,	offsetof( CrateCollideModuleData, m_pickupScience ) },
		{ "ExecuteFX", INI::parseFXList, NULL, offsetof( CrateCollideModuleData, m_executeFX ) },
		{ "ExecuteAnimation", INI::parseAsciiString, NULL, offsetof( CrateCollideModuleData, m_executionAnimationTemplate ) },
		{ "ExecuteAnimationTime", INI::parseReal, NULL, offsetof( CrateCollideModuleData, m_executeAnimationDisplayTimeInSeconds ) },
		{ "ExecuteAnimationZRise", INI::parseReal, NULL, offsetof( CrateCollideModuleData, m_executeAnimationZRisePerSecond ) },
		{ "ExecuteAnimationFades", INI::parseBool, NULL, offsetof( CrateCollideModuleData, m_executeAnimationFades ) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CrateCollide::CrateCollide( Thing *thing, const ModuleData* moduleData ) : CollideModule( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
CrateCollide::~CrateCollide( void )
{

}

//-------------------------------------------------------------------------------------------------
/** The collide event.
	* Note that when other is NULL it means "collide with ground" */
//-------------------------------------------------------------------------------------------------
void CrateCollide::onCollide( Object *other, const Coord3D *, const Coord3D * )
{
	const CrateCollideModuleData *modData = getCrateCollideModuleData();
	// If the crate can be picked up, perform the game logic and destroy the crate.
	if( isValidToExecute( other ) )
	{
		if( executeCrateBehavior( other ) )
		{
			if( modData->m_executeFX != NULL )
			{
				// Note: We pass in other here, because the crate is owned by the neutral player, and
				// we want to do things that only the other person can see.
				FXList::doFXObj( modData->m_executeFX, other );
			}

			TheGameLogic->destroyObject( getObject() );
		}

		// play animation in the world at this spot if there is one
		if( TheAnim2DCollection && modData->m_executionAnimationTemplate.isEmpty() == FALSE && TheGameLogic->getDrawIconUI() )
		{
			Anim2DTemplate *animTemplate = TheAnim2DCollection->findTemplate( modData->m_executionAnimationTemplate );

			TheInGameUI->addWorldAnimation( animTemplate,
																			getObject()->getPosition(),
																			WORLD_ANIM_FADE_ON_EXPIRE,
																			modData->m_executeAnimationDisplayTimeInSeconds,
																			modData->m_executeAnimationZRisePerSecond );

		}

	}

}

//-------------------------------------------------------------------------------------------------
Bool CrateCollide::isValidToExecute( const Object *other ) const
{
	//The ground never picks up a crate
	if( other == NULL )
		return FALSE;

	//Nothing Neutral can pick up any type of crate
	if( other->isNeutralControlled() )
		return FALSE;

	const CrateCollideModuleData* md = getCrateCollideModuleData();
	Bool validBuildingAttempt = md->m_isBuildingPickup && other->isKindOf( KINDOF_STRUCTURE );

	// Must be a "Unit" type thing.  Real Game Object, not just Object
	if( other->getAIUpdateInterface() == NULL  &&  !validBuildingAttempt )// Building exception flag for Drop Zone
		return FALSE;

	// must match our kindof flags (if any)
	if ( !other->isKindOfMulti(md->m_kindof, md->m_kindofnot) )
		return FALSE;

	if( other->isEffectivelyDead() )
		return FALSE;

	// crates cannot be claimed while in the air, except by buildings
	if( getObject()->isAboveTerrain() && !validBuildingAttempt )
		return FALSE;

	if( md->m_isForbidOwnerPlayer  &&  (getObject()->getControllingPlayer() == other->getControllingPlayer()) )
		return FALSE; // Design has decreed this to not be picked up by the dead guy's team.

	if( md->m_isHumanOnlyPickup  &&  other->getControllingPlayer() && (other->getControllingPlayer()->getPlayerType() != PLAYER_HUMAN) )
		return FALSE; // Human only mission crate

	if( (md->m_pickupScience != SCIENCE_INVALID)  &&  other->getControllingPlayer()  &&  !other->getControllingPlayer()->hasScience(md->m_pickupScience) )
		return FALSE; // Science required to pick this up

	if( other->isKindOf( KINDOF_PARACHUTE ) )
		return FALSE;

	return TRUE;
}





void CrateCollide::doSabotageFeedbackFX( const Object *other, SabotageVictimType type )
{

  if ( ! getObject() )
    return;
  if ( ! other )
    return;

	AudioEventRTS soundToPlay;
  switch ( type )
  {
    case  CrateCollide::SAB_VICTIM_FAKE_BUILDING:
    {
      return; // THIS NEEDS NO ADD'L FEEDBACK
    }
    case 	CrateCollide::SAB_VICTIM_COMMAND_CENTER:
    case 	CrateCollide::SAB_VICTIM_SUPERWEAPON:
    {
      soundToPlay = TheAudio->getMiscAudio()->m_sabotageResetTimerBuilding;
      break;
    }
    case 	CrateCollide::SAB_VICTIM_DROP_ZONE:
    case 	CrateCollide::SAB_VICTIM_SUPPLY_CENTER:
    {
      soundToPlay = TheAudio->getMiscAudio()->m_moneyWithdrawSound;
      break;
    }
    case 	CrateCollide::SAB_VICTIM_INTERNET_CENTER:
    case 	CrateCollide::SAB_VICTIM_MILITARY_FACTORY:
    case 	CrateCollide::SAB_VICTIM_POWER_PLANT:
    default:
    {
      soundToPlay = TheAudio->getMiscAudio()->m_sabotageShutDownBuilding;
      break;
    }
  }

	soundToPlay.setPosition( other->getPosition() );
	TheAudio->addAudioEvent( &soundToPlay );

  Drawable *draw = other->getDrawable();
  if ( draw )
    draw->flashAsSelected();

}



// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void CrateCollide::crc( Xfer *xfer )
{

	// extend base class
	CollideModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer Method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void CrateCollide::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	CollideModule::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void CrateCollide::loadPostProcess( void )
{

	// extend base class
	CollideModule::loadPostProcess();

}
