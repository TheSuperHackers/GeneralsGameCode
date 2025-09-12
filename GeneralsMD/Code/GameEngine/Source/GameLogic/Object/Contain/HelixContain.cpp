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

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  FILE: HelixContain.cpp ////////////////////////////////////////////////////////////////////////
//  Author: Mark Lorenzen, April, 2003
//
//  Desc:
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/HelixContain.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Weapon.h"




// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
HelixContainModuleData::HelixContainModuleData()
{
//	m_initialPayload.count = 0;
  m_drawPips = TRUE;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void HelixContainModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  TransportContainModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
    { "PayloadTemplateName",  INI::parseAsciiStringVectorAppend, NULL, offsetof(HelixContainModuleData, m_payloadTemplateNameData) },
    {"ShouldDrawPips",  INI::parseBool, NULL, offsetof(HelixContainModuleData, m_drawPips) },

		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
//void HelixContainModuleData::parseInitialPayload( INI* ini, void *instance, void *store, const void* /*userData*/ )
//{
//	HelixContainModuleData* self = (HelixContainModuleData*)instance;
//	const char* name = ini->getNextToken();
//	const char* countStr = ini->getNextTokenOrNull();
//	Int count = countStr ? INI::scanInt(countStr) : 1;

//	self->m_initialPayload.name.set(name);
//	self->m_initialPayload.count = count;
//}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
HelixContain::HelixContain( Thing *thing, const ModuleData *moduleData ) :
								 TransportContain( thing, moduleData )
{

  m_payloadCreated = FALSE;
  m_portableStructureID = INVALID_ID;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
HelixContain::~HelixContain( void )
{

}


void HelixContain::onObjectCreated( void )
{
  HelixContain::createPayload();
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime HelixContain::update()
{

  Object *portable = getPortableStructure();
  if ( portable )
  {
    portable->setPosition( getObject()->getPosition());
    portable->setOrientation( getObject()->getOrientation());
  }

  return TransportContain::update(); // extend base
}


void HelixContain::redeployOccupants( void )
{
  Coord3D firePos = *getObject()->getPosition();
  firePos.z += 8;


	for (ContainedItemsList::iterator it = m_containList.begin(); it != m_containList.end(); ++it)
  {
    Object* rider = *it;
    if (rider)
      rider->setPosition( &firePos );
  }
}


//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void HelixContain::createPayload()
{
	HelixContainModuleData* self = (HelixContainModuleData*)getHelixContainModuleData();


  // Any number of different passengers can be loaded here at init time
	Object* object = getObject();
	ContainModuleInterface *contain = object->getContain();
	if( contain )
  {
		contain->enableLoadSounds( FALSE );

	  TemplateNameList list = self->m_payloadTemplateNameData;
	  TemplateNameIterator iter = list.begin();
	  while ( iter != list.end() )
	  {
		  const ThingTemplate* temp = TheThingFactory->findTemplate( *iter );
		  if (temp)
		  {
			  Object* payload = TheThingFactory->newObject( temp, object->getTeam() );

			  if( contain->isValidContainerFor( payload, true ) )
			  {
				  contain->addToContain( payload );
			  }
			  else
			  {
				  DEBUG_CRASH( ( "HelixContain::createPayload: %s is full, or not valid for the payload %s!", object->getName().str(), self->m_initialPayload.name.str() ) );
			  }

      }

      ++iter;
    }

		contain->enableLoadSounds( TRUE );

  }

	m_payloadCreated = TRUE;

}

// ------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void HelixContain::onBodyDamageStateChange( const DamageInfo* damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState)  ///< state change callback
{
  // Need to apply state change to the portable structure
  Object *portable = getPortableStructure();
  if ( newState != BODY_RUBBLE  && portable )
  {
		portable->getBodyModule()->setDamageState( newState );
  }

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Object* HelixContain::getPortableStructure( void )
{
  return TheGameLogic->findObjectByID( m_portableStructureID );
}



//-------------------------------------------------------------------------------------------------
void HelixContain::onDie( const DamageInfo *damageInfo )
{
  Object *portable = getPortableStructure();
  if ( portable )
    portable->kill();

	TransportContain::onDie( damageInfo );//extend base class
}

//-------------------------------------------------------------------------------------------------
void HelixContain::onDelete( void )
{
  Object *portable = getPortableStructure();
  if ( portable )
    TheGameLogic->destroyObject( portable );

  TransportContain::onDelete( );
}

// ------------------------------------------------------------------------------------------------
void HelixContain::onCapture( Player *oldOwner, Player *newOwner )
{
//  Need to setteam() the portable structure, that's all;
  Object *portable = getPortableStructure();
  if ( portable )
	  portable->setTeam( newOwner->getDefaultTeam() );
}

//-------------------------------------------------------------------------------------------------
void HelixContain::addToContainList( Object *obj )
{
  if ( obj->isKindOf( KINDOF_PORTABLE_STRUCTURE ) && m_portableStructureID == INVALID_ID)
  {
    Object *portable = getPortableStructure();
    if ( portable )
      TheGameLogic->destroyObject( portable );

    m_portableStructureID = obj->getID();
    obj->friend_setContainedBy( getObject() );//fool portable into thinking my object is his container


  }
  else
		TransportContain::addToContainList( obj );
}

//-------------------------------------------------------------------------------------------------
void HelixContain::addToContain( Object *obj )
{
  if ( obj->isKindOf( KINDOF_PORTABLE_STRUCTURE ) && m_portableStructureID == INVALID_ID)
  {
    Object *portable = getPortableStructure();
    if ( portable )
      TheGameLogic->destroyObject( portable );

    m_portableStructureID = obj->getID();
    obj->friend_setContainedBy( getObject() );//fool portable into thinking my object is his container


  }
  else
		TransportContain::addToContain( obj );
}

//-------------------------------------------------------------------------------------------------
void HelixContain::removeFromContain( Object *obj, Bool exposeStealthUnits )
{
  if ( obj->isKindOf( KINDOF_PORTABLE_STRUCTURE ) && obj->getID() == m_portableStructureID )
	{
    Object *portable = getPortableStructure();
    if ( portable )

      m_portableStructureID = INVALID_ID;
      //portable->kill();

  }
  else
  {
		TransportContain::removeFromContain( obj, exposeStealthUnits );
	}
}


//-------------------------------------------------------------------------------------------------
Bool HelixContain::isValidContainerFor(const Object* obj, Bool checkCapacity) const
{
  if ( obj->isKindOf( KINDOF_PORTABLE_STRUCTURE ) && INVALID_ID == m_portableStructureID )
    return TRUE;

	return TransportContain::isValidContainerFor( obj, checkCapacity );
}


//-------------------------------------------------------------------------------------------------
const Object *HelixContain::friend_getRider() const
{
// The draw order dependency bug for riders means that our draw module needs to cheat to get around it.

  if ( m_portableStructureID != INVALID_ID )
  {
    const Object *portableAsRider = TheGameLogic->findObjectByID( m_portableStructureID );
    return portableAsRider;
  }

	return NULL;
}

//-------------------------------------------------------------------------------------------------
Bool HelixContain::isEnclosingContainerFor( const Object *obj ) const
{
  if ( m_portableStructureID == obj->getID() )
  {
    const Object *portableAsRider = TheGameLogic->findObjectByID( m_portableStructureID );
    if ( portableAsRider == obj )
      return FALSE;
  }


  return TransportContain::isEnclosingContainerFor( obj );
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// if my object gets selected, then my visible passengers should, too
// this gets called from
void HelixContain::clientVisibleContainedFlashAsSelected()
{
  if ( m_portableStructureID != INVALID_ID)
  {
    Object *portable = getPortableStructure();
    if ( portable && portable->isKindOf(KINDOF_PORTABLE_STRUCTURE) )
    {
			Drawable *draw = portable->getDrawable();
			if ( draw )
			{
				draw->flashAsSelected(); //WOW!
			}
		}
  }
}


Bool HelixContain::isPassengerAllowedToFire( ObjectID id ) const
{
  // WHETHER WE ARE ALLOWED TO FIRE DEPENDS ON WHO WE ARE
  // PASSENGERS (PROPER) MAY ONLY IF THE FLAG IS TRUE
  // RIDERS ARE ALWAYS ALLOWED TO FIRE (GATTLING CANNONS)

  if ( getObject() && getObject()->getContainedBy() ) // nested containment voids firing, always
    return FALSE;

  if ( m_portableStructureID != INVALID_ID && m_portableStructureID == id )
    return TRUE;
  else
  {
    const Object *rider = TheGameLogic->findObjectByID( id );
    if ( rider && rider->isKindOf( KINDOF_INFANTRY ))
      return TransportContain::isPassengerAllowedToFire( id );//extend
  }

  return FALSE;

}






//-------------------------------------------------------------------------------------------------
void HelixContain::onContaining( Object *obj, Bool wasSelected )
{
	// extend base class
	TransportContain::onContaining( obj, wasSelected );

	// give the object a garrisoned version of its weapon
	obj->setWeaponBonusCondition( WEAPONBONUSCONDITION_GARRISONED );
  obj->setDisabled( DISABLED_HELD );


  if ( obj->isKindOf( KINDOF_PORTABLE_STRUCTURE ) && getObject()->testStatus( OBJECT_STATUS_STEALTHED ) )
  {
    StealthUpdate *myStealth =  obj->getStealth();
    if ( myStealth )
    {
      myStealth->receiveGrant( true );
      // note to anyone... once stealth is granted to this gattlingcannon ( or such )
      // let its own stealthupdate govern the allowedtostealth cases
      // a portable structure never gets removed, so...
    }
  }




}

void HelixContain::onRemoving( Object *obj )
{
	// extend base class
	TransportContain::onRemoving(obj);

	// give the object back a regular weapon
	obj->clearWeaponBonusCondition( WEAPONBONUSCONDITION_GARRISONED );
  obj->clearDisabled( DISABLED_HELD );

}










// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void HelixContain::crc( Xfer *xfer )
{

	// extend base class
	TransportContain::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void HelixContain::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

  if (version >= 2)
    xfer->xferObjectID( &m_portableStructureID );

	// extend base class
  	TransportContain::xfer( xfer );


}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void HelixContain::loadPostProcess( void )
{

	// extend base class
	TransportContain::loadPostProcess();

}
