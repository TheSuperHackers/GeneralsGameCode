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

// FILE: PropagandaTowerBehavior.cpp //////////////////////////////////////////////////////////////
// Author: Colin Day, August 2002
// Desc:   Behavior module for PropagandaTower
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/GameState.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Upgrade.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/Module/PropagandaTowerBehavior.h"
#include "GameLogic/Module/BodyModule.h"




// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
enum ObjectID CPP_11(: Int);

// ------------------------------------------------------------------------------------------------
/** This class is used to track objects as they exit our area of influence */
// ------------------------------------------------------------------------------------------------
class ObjectTracker : public MemoryPoolObject
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ObjectTracker, "ObjectTracker" );

public:

	ObjectTracker( void ) { objectID = INVALID_ID; next = NULL; }

	ObjectID objectID;
	ObjectTracker *next;

};
ObjectTracker::~ObjectTracker( void ) { }

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PropagandaTowerBehaviorModuleData::PropagandaTowerBehaviorModuleData( void )
{

	m_scanRadius = 1.0f;
	m_scanDelayInFrames = 100;
	m_autoHealPercentPerSecond = 0.01f;
	m_upgradedAutoHealPercentPerSecond = 0.02f;
	m_pulseFX = NULL;
	m_upgradeRequired = NULL;
	m_upgradedPulseFX = NULL;
	m_affectsSelf = FALSE;

}  // end PropagandaTowerBehaviorModuleData

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void PropagandaTowerBehaviorModuleData::buildFieldParse( MultiIniFieldParse &p )
{
  UpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] = 
	{
		{ "Radius",									INI::parseReal,									NULL,	offsetof( PropagandaTowerBehaviorModuleData, m_scanRadius ) },
		{ "DelayBetweenUpdates",		INI::parseDurationUnsignedInt,	NULL,	offsetof( PropagandaTowerBehaviorModuleData, m_scanDelayInFrames ) },
		{ "HealPercentEachSecond",	INI::parsePercentToReal,				NULL,	offsetof( PropagandaTowerBehaviorModuleData, m_autoHealPercentPerSecond ) },
		{ "UpgradedHealPercentEachSecond",	INI::parsePercentToReal,NULL,	offsetof( PropagandaTowerBehaviorModuleData, m_upgradedAutoHealPercentPerSecond ) },
		{ "PulseFX",								INI::parseFXList,								NULL,	offsetof( PropagandaTowerBehaviorModuleData, m_pulseFX ) },
		{ "UpgradeRequired",				INI::parseAsciiString,					NULL, offsetof( PropagandaTowerBehaviorModuleData, m_upgradeRequired ) },
		{ "UpgradedPulseFX",				INI::parseFXList,								NULL, offsetof( PropagandaTowerBehaviorModuleData, m_upgradedPulseFX ) },
		{ "AffectsSelf",						INI::parseBool,									NULL, offsetof( PropagandaTowerBehaviorModuleData, m_affectsSelf ) },
		{ 0, 0, 0, 0 }
	};

  p.add( dataFieldParse );

}  // end buildFieldParse

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PropagandaTowerBehavior::PropagandaTowerBehavior( Thing *thing, const ModuleData *modData )
											 : UpdateModule( thing, modData )
{
	//Added By Sadullah Nader
	//Initializations inserted
	m_lastScanFrame = 0;
	//
	m_insideList = NULL;
	setWakeFrame( getObject(), UPDATE_SLEEP_NONE );

}  // end PropagandaTowerBehavior

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PropagandaTowerBehavior::~PropagandaTowerBehavior( void )
{

}  // end ~PropagandaTowerBehavior

// ------------------------------------------------------------------------------------------------
/** Module is being deleted */
// ------------------------------------------------------------------------------------------------
void PropagandaTowerBehavior::onDelete( void )
{

	// remove any benefits from anybody in our area of influence
	removeAllInfluence();

}  // end onDelete

// ------------------------------------------------------------------------------------------------
/** Resolve */
// ------------------------------------------------------------------------------------------------
void PropagandaTowerBehavior::onObjectCreated( void )
{
	const PropagandaTowerBehaviorModuleData *modData = getPropagandaTowerBehaviorModuleData();

	// convert module upgrade name to a pointer
	m_upgradeRequired = TheUpgradeCenter->findUpgrade( modData->m_upgradeRequired );

}  // end onObjectCreated

// ------------------------------------------------------------------------------------------------
void PropagandaTowerBehavior::onCapture( Player *oldOwner, Player *newOwner )
{
	// We don't function for the neutral player.  
	if( newOwner == ThePlayerList->getNeutralPlayer() )
	{
		removeAllInfluence();
		setWakeFrame( getObject(), UPDATE_SLEEP_FOREVER );
	}
	else
		setWakeFrame( getObject(), UPDATE_SLEEP_NONE );
}

// ------------------------------------------------------------------------------------------------
/** The update callback */
// ------------------------------------------------------------------------------------------------
UpdateSleepTime PropagandaTowerBehavior::update( void )
{
/// @todo srj use SLEEPY_UPDATE here
	const PropagandaTowerBehaviorModuleData *modData = getPropagandaTowerBehaviorModuleData();
	
	//Sep 27, 2002 (Kris): Added this code to prevent the tower from working while under construction.
	Object *self = getObject();
	if( self->getStatusBits().test( OBJECT_STATUS_UNDER_CONSTRUCTION ) )
		return UPDATE_SLEEP_NONE;

	if( self->testStatus(OBJECT_STATUS_SOLD) )
	{
		removeAllInfluence();
		return UPDATE_SLEEP_FOREVER;
	}

	if( self->isEffectivelyDead() )
		return UPDATE_SLEEP_FOREVER;
	
	if( self->isDisabled() )
	{
		// We need to let go of everyone if we are EMPd or underpowered or yadda, but not if we are only held

		DisabledMaskType allButHeld = MAKE_DISABLED_MASK( DISABLED_HELD );
		FLIP_DISABLEDMASK(allButHeld);

		if( TEST_DISABLEDMASK_ANY(self->getDisabledFlags(), allButHeld) )
		{
			removeAllInfluence();
			return UPDATE_SLEEP_NONE;
		}
	}
	
	if( self->getContainedBy()  &&  self->getContainedBy()->getContainedBy() )
	{
		// If our container is contained, we turn the heck off.  Seems like a weird specific check, but all of 
		// attacking is guarded by the same check in isPassengersAllowedToFire.  We similarly work in a container,
		// but not in a double container.
		removeAllInfluence();
		return UPDATE_SLEEP_NONE;
	}
	
	// if it's not time to scan, nothing to do
	UnsignedInt currentFrame = TheGameLogic->getFrame();
	if( currentFrame - m_lastScanFrame >= modData->m_scanDelayInFrames )
	{

		// do a scan
		doScan();
		m_lastScanFrame = currentFrame;

	}  // end if

	// go through any objects in our area of influence and do the effect logic on them
	Object *obj;
	ObjectTracker *curr = NULL, *prev = NULL, *next = NULL;
	for( curr = m_insideList; curr; curr = next )
	{
		
		// get the next link
		next = curr->next;

		// find this object
		obj = TheGameLogic->findObjectByID( curr->objectID );
		if ((obj) &&
			 (obj->isKindOf(KINDOF_SCORE) || obj->isKindOf(KINDOF_SCORE_CREATE) || obj->isKindOf(KINDOF_SCORE_DESTROY) || obj->isKindOf(KINDOF_MP_COUNT_FOR_VICTORY)))
		{

			// give any bonus to this object
			effectLogic( obj, TRUE, getPropagandaTowerBehaviorModuleData() );

			// record this element as the previous one found in the list
			prev = curr;

		}  // end if
		else
		{

			//
			// actual object wasn't found, remove this entry from our inside list so we don't
			// have to search through it again
			//
			if( prev )
				prev->next = curr->next;
			else
				m_insideList = curr->next;
			deleteInstance(curr);
				
		}  // end else

	}  // end for, curr

	return UPDATE_SLEEP_NONE;

}  // end update

// ------------------------------------------------------------------------------------------------
/** The death callback */
// ------------------------------------------------------------------------------------------------
void PropagandaTowerBehavior::onDie( const DamageInfo *damageInfo )
{

	// remove any benefits from anybody in our area of influence
	removeAllInfluence();

}  // end onDie

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
/** Grant or remove effect to this object */
// ------------------------------------------------------------------------------------------------
void PropagandaTowerBehavior::effectLogic( Object *obj, Bool giving,
																					 const PropagandaTowerBehaviorModuleData *modData )
{
	Bool effectUpgraded = getObject()->getControllingPlayer()->hasUpgradeComplete( m_upgradeRequired );

	// if giving the effect
	if( giving )
	{
		if ( obj->hasAnyDamageWeapon() == TRUE )
		{
			if( obj->testWeaponBonusCondition( WEAPONBONUSCONDITION_ENTHUSIASTIC ) == FALSE )
				obj->setWeaponBonusCondition( WEAPONBONUSCONDITION_ENTHUSIASTIC );


			if (effectUpgraded)
			{
				if (obj->testWeaponBonusCondition( WEAPONBONUSCONDITION_SUBLIMINAL ) == FALSE)
					obj->setWeaponBonusCondition( WEAPONBONUSCONDITION_SUBLIMINAL );
			}

		} // hasdamageweapon


		// grant health to this object as well
		BodyModuleInterface *body = obj->getBodyModule();
		if( body )
		{
			Real healthPercent;
			if(effectUpgraded)
				healthPercent = modData->m_upgradedAutoHealPercentPerSecond;
			else
				healthPercent = modData->m_autoHealPercentPerSecond;

			Real amount = healthPercent / LOGICFRAMES_PER_SECOND * body->getMaxHealth();

	// Dustin wants the healing effect not to stack from multiple propaganda towers...
	// To accomplish this, I'll give every object a single healing-sender (ID)
	// Any given healing recipient (object) can only receive healing from one particular healing sender 
	// and cannot change healing senders until the previous one expires (its scandelay)

//		obj->attemptHealing(amount, getObject()); // the regular way to give healing... 
			obj->attemptHealingFromSoleBenefactor( amount, getObject(), modData->m_scanDelayInFrames );//the non-stacking way

		}  // end if

	}  // end if
	else
	{

		// taking effect away
		obj->clearWeaponBonusCondition( WEAPONBONUSCONDITION_ENTHUSIASTIC );
		obj->clearWeaponBonusCondition( WEAPONBONUSCONDITION_SUBLIMINAL );

	}	  // end else

}  // end effectLogic

// ------------------------------------------------------------------------------------------------
/** Remove all influence from objects we've given bonuses to */
// ------------------------------------------------------------------------------------------------
void PropagandaTowerBehavior::removeAllInfluence( void )
{
	ObjectTracker *o;

	// go through all objects we've given bonuses to and remove them
	Object *obj;
	for( o = m_insideList; o; o = o->next )
	{
		
		obj = TheGameLogic->findObjectByID( o->objectID );
		if( obj )
			effectLogic( obj, FALSE, getPropagandaTowerBehaviorModuleData() );

	}  // end for

	// delete the list of objects under our influence
	while( m_insideList )
	{

		o = m_insideList->next;
		deleteInstance(m_insideList);
		m_insideList = o;

	}  // end while

}  // end removeAllInfluence

// ------------------------------------------------------------------------------------------------
/** Do a scan */
// ------------------------------------------------------------------------------------------------
void PropagandaTowerBehavior::doScan( void )
{
	const PropagandaTowerBehaviorModuleData *modData = getPropagandaTowerBehaviorModuleData();
	Object *us = getObject();
	ObjectTracker *newInsideList = NULL;

	// The act of scanning is when we play our effect
	Bool upgradePresent = FALSE;
	if( m_upgradeRequired )
	{

		// see if we have the upgrade
		switch( m_upgradeRequired->getUpgradeType() )
		{

			// ------------------------------------------------------------------------------------------
			case UPGRADE_TYPE_PLAYER:
			{
				Player *player = us->getControllingPlayer();

				upgradePresent = player->hasUpgradeComplete( m_upgradeRequired );
				break;

			}  // end player upgrade

			// ------------------------------------------------------------------------------------------
			case UPGRADE_TYPE_OBJECT:
			{
				
				upgradePresent = us->hasUpgrade( m_upgradeRequired );				
				break;

			}  // end object upgrade

			// ------------------------------------------------------------------------------------------
			default:
			{

				DEBUG_CRASH(( "PropagandaTowerBehavior::doScan - Unknown upgrade type '%d'",
											m_upgradeRequired->getUpgradeType() ));
				break;

			}  // end default

		}  // end switch

	}  // end if


  Bool doFX = TRUE;
    // if it is us, or if it is he and we do see him 
  Object *overlord = us->getContainedBy();
  if ( overlord )
  {
	  if ( us->getControllingPlayer() != ThePlayerList->getLocalPlayer() )// daling with someone else's tower
    {
      if ( overlord->testStatus( OBJECT_STATUS_STEALTHED ) && !overlord->testStatus( OBJECT_STATUS_DETECTED ) )
        doFX = FALSE;// so they don't give up their position

    }

    // Sorry, this is a lot of hard coded mishmash, but it must be done... ship it!
    // Propaganda towers (proper) never get contained, so they don't care about this code
    // The PropTowers that ride on the backs of overlord tanks want to suppress their fx if the overlord is stealthed to the local player
    // The PropTowers that ride on the bellies of Helixes also suppress their fx when hiding stealthed (never happens?)
    // The PropTowers must do nothing, scanning-or-FX-wise when they are inside something... there are two cases
    //   1) When the prop tower is on an Overlord that is in a helicopter (likely a Helix II)
    //   2) When the prop tower is the EmperorTank, which is in a helicopter (likely a Helix II).

    if ( us->isKindOf( KINDOF_VEHICLE ) && !us->isKindOf( KINDOF_PORTABLE_STRUCTURE ) )//craptacular!
      // oh my dear Lord... I am not a propaganda tower, I am an emperorTank! that means the overlord is a helix or something!
      return; // proptowers that are riding IN things cannot do their scan!

    //okay this is wacky, but this proptower may be on an overlord thank that is riding in a helix... oy!
    Object *helix = overlord->getContainedBy();
    if ( helix )
      return; // proptowers that are riding ON things that are in-turn riding IN things cannot do their scan!

  }
	
	if ( us->getControllingPlayer() != ThePlayerList->getLocalPlayer() )// daling with someone else's tower
	{
		if ( us->testStatus( OBJECT_STATUS_STEALTHED ) && !us->testStatus( OBJECT_STATUS_DETECTED ) )
		{
			doFX = FALSE;// Certainly don't play if we ourselves are stelthed.
		}
	}

  if ( doFX )
  {
    // play the right pulse
	  if( upgradePresent == TRUE )
		  FXList::doFXObj( modData->m_upgradedPulseFX, us );
	  else
		  FXList::doFXObj( modData->m_pulseFX, us );
  }

	// setup scan filters
	PartitionFilterRelationship relationship( us, PartitionFilterRelationship::ALLOW_ALLIES );
	PartitionFilterAlive filterAlive;
	PartitionFilterSameMapStatus filterMapStatus(us);
	PartitionFilterAcceptByKindOf filterOutBuildings(KINDOFMASK_NONE, MAKE_KINDOF_MASK(KINDOF_STRUCTURE));
	PartitionFilter *filters[] = {	&relationship, 
																	&filterAlive, 
																	&filterMapStatus, 
																	&filterOutBuildings, 
																	NULL 
																};

	// scan objects in our region
	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( us->getPosition(),
																																		 modData->m_scanRadius,
																																		 FROM_CENTER_2D, 
																																		 filters );
	MemoryPoolObjectHolder hold( iter );
	Object *obj;
	ObjectTracker *newEntry;
	for( obj = iter->first(); obj; obj = iter->next() )
	{

		// ignore ourselves, unless Design wants us to affect ourselves
		if( obj == us  &&  !modData->m_affectsSelf)
			continue;

		// record this object as being in the new "in list"
		newEntry = newInstance(ObjectTracker);
		newEntry->objectID = obj->getID();
		newEntry->next = newInsideList;
		newInsideList = newEntry;

	}  // end for obj

	//
	// now that we have a list of objects that are in our area of influence, look through
	// the objects that were last recorded as in our area of influence and remove any
	// bonus we've given them (if they are within the area of effect of another tower it's
	// OK, they'll get the bonus back again when that tower does a scan which won't be too long
	//
	for( ObjectTracker *curr = m_insideList; curr; curr = curr->next )
	{
	
		// find this entry in the new list
		ObjectTracker *o = NULL;
		for( o = newInsideList; o; o = o->next )
			if( o->objectID == curr->objectID )
				break;

		// if entry wasn't there, remove the bonus from this object
		if( o == NULL )
		{

			obj = TheGameLogic->findObjectByID( curr->objectID );
			if( obj )
				effectLogic( obj, FALSE, modData );

		}  // end if

	}  // end for
		
	// delete the inside list we have recoreded
	ObjectTracker *next;
	while( m_insideList )
	{

		next = m_insideList->next;
		deleteInstance(m_insideList);
		m_insideList = next;

	}  // end while

	// set the new inside list to the one we're recording
	m_insideList = newInsideList;

}  // end doScan

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void PropagandaTowerBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void PropagandaTowerBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// last scan frame
	xfer->xferUnsignedInt( &m_lastScanFrame );

	// inside list tracking
	ObjectTracker *trackerEntry;
	UnsignedShort insideCount = 0;
	for( trackerEntry = m_insideList; trackerEntry; trackerEntry = trackerEntry->next )
		insideCount++;
	xfer->xferUnsignedShort( &insideCount );
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// write all entries
		for( trackerEntry = m_insideList; trackerEntry; trackerEntry = trackerEntry->next )
		{

			// object id
			xfer->xferObjectID( &trackerEntry->objectID );

		}  // end for

	}  // end if, save
	else
	{

		// sanity
		if( m_insideList != NULL )
		{

			DEBUG_CRASH(( "PropagandaTowerBehavior::xfer - m_insideList should be empty but is not" ));
			throw SC_INVALID_DATA;

		}  // end if

		// read all entries
		for( UnsignedShort i = 0; i < insideCount; ++i )
		{

			// allocate new tracker entry and tie to list
			trackerEntry = newInstance(ObjectTracker);
			trackerEntry->next = m_insideList;
			m_insideList = trackerEntry;

			// read object id
			xfer->xferObjectID( &trackerEntry->objectID );

		}  // end for i

	}  // end else, load

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void PropagandaTowerBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
