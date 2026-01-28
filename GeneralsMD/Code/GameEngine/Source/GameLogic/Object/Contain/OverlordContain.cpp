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

// FILE: OverlordContain.cpp ////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, September, 2002
// Desc:   Contain module that acts as transport normally, but when full it redirects queries to the first passenger
// All of this redirection stuff makes it so that while I am normally a transport
// for Overlord subObjects, once I have a passenger, _I_ become a transport of their type.
// So, the answer to this question depends on if it is my passenger asking, or theirs.
// As always, I can't use convenience functions that get redirected on a ? like this.

///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "Common/ThingTemplate.h"
#include "Common/ThingFactory.h"
#include "GameClient/ControlBar.h"
#include "GameClient/Drawable.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/OverlordContain.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"




// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
OverlordContainModuleData::OverlordContainModuleData()
{
//	m_initialPayload.count = 0;
	m_experienceSinkForRider = TRUE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void OverlordContainModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  TransportContainModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
    { "PayloadTemplateName",  INI::parseAsciiStringVectorAppend, nullptr, offsetof(OverlordContainModuleData, m_payloadTemplateNameData) },
    { "ExperienceSinkForRider",  INI::parseBool, nullptr, offsetof(OverlordContainModuleData, m_experienceSinkForRider) },

		{ nullptr, nullptr, nullptr, 0 }
	};
  p.add(dataFieldParse);
}
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
//void OverlordContainModuleData::parseInitialPayload( INI* ini, void *instance, void *store, const void* /*userData*/ )
//{
//	OverlordContainModuleData* self = (OverlordContainModuleData*)instance;
//	const char* name = ini->getNextToken();
//	const char* countStr = ini->getNextTokenOrNull();
//	Int count = countStr ? INI::scanInt(countStr) : 1;

//	self->m_initialPayload.name.set(name);
//	self->m_initialPayload.count = count;
//}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
OverlordContain::OverlordContain( Thing *thing, const ModuleData *moduleData ) :
								 TransportContain( thing, moduleData )
{
	m_redirectionActivated = FALSE;

  m_payloadCreated = FALSE;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
OverlordContain::~OverlordContain( void )
{

}


void OverlordContain::onObjectCreated( void )
{
  OverlordContain::createPayload();
}

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void OverlordContain::createPayload()
{
	OverlordContainModuleData* self = (OverlordContainModuleData*)getOverlordContainModuleData();


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
				  DEBUG_CRASH( ( "OverlordContain::createPayload: %s is full, or not valid for the payload %s!", object->getName().str(), self->m_initialPayload.name.str() ) );
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
void OverlordContain::onBodyDamageStateChange( const DamageInfo* damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState)  ///< state change callback
{
	// I can't use any convenience functions, as they will all get routed to the bunker I may carry.
	// I want just me.
	// Oh, and I don't want this function trying to do death.  That is more complicated and will be handled
	// on my death.
	if( newState != BODY_RUBBLE  &&  m_containListSize == 1 )
	{
		Object *myGuy = m_containList.front();
		myGuy->getBodyModule()->setDamageState( newState );
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ContainModuleInterface *OverlordContain::getRedirectedContain() const
{
	// Naturally, I can not use a redirectible convenience function
	// to answer if I am redirecting yet.

	// If I am empty, say no.
	if( m_containListSize < 1 )
		return nullptr;

	if( !m_redirectionActivated )
		return nullptr;// Shut off early to allow death to happen without my bunker having
	// trouble finding me to say goodbye as messages get sucked up the pipe to him.

	Object *myGuy = m_containList.front();
	if( myGuy )
		return myGuy->getContain();

	return nullptr;// Or say no if they have no contain.
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------
void OverlordContain::onDie( const DamageInfo *damageInfo )
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
	{
		TransportContain::onDie( damageInfo );
		return;
	}
	//Everything is fine if I am empty or carrying a regular guy.  If I have a redirected contain
	// set up, then I need to handle the order of death explicitly, or things will become confused
	// when I stop redirecting in the middle of the process.  Or I will get confused as my commands
	// get sucked up the pipe.

	// So this is an extend that lets me control the order of death.

	deactivateRedirectedContain();
	Object *myGuy = m_containList.front();
	myGuy->kill();

	TransportContain::onDie( damageInfo );
}

//-------------------------------------------------------------------------------------------------
void OverlordContain::onDelete( void )
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
	{
		TransportContain::onDelete( );
		return;
	}

	// Without my throwing the redirect switch, teardown deletion will get confused and fire off a bunch of asserts
	getRedirectedContain()->removeAllContained();

	deactivateRedirectedContain();
	removeAllContained();

	TransportContain::onDelete( );
}

// ------------------------------------------------------------------------------------------------
void OverlordContain::onCapture( Player *oldOwner, Player *newOwner )
{
	if( m_containListSize < 1 )
		return;

	// Need to capture our specific rider.  He will then kick passengers out if he is a Transport
	Object *myGuy = m_containList.front();
	myGuy->setTeam( newOwner->getDefaultTeam() );
}

//-------------------------------------------------------------------------------------------------
Bool OverlordContain::isGarrisonable() const
{
	if( getRedirectedContain() == nullptr )
		return FALSE;

	return getRedirectedContain()->isGarrisonable();
}

//-------------------------------------------------------------------------------------------------
Bool OverlordContain::isKickOutOnCapture()
{
	if( getRedirectedContain() == nullptr )
		return FALSE;// Me the Overlord doesn't want to

	return getRedirectedContain()->isKickOutOnCapture();
}

//-------------------------------------------------------------------------------------------------
void OverlordContain::addToContainList( Object *obj )
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
	{
		TransportContain::addToContainList( obj );
		return;
	}

	getRedirectedContain()->addToContainList( obj );
}

//-------------------------------------------------------------------------------------------------
void OverlordContain::addToContain( Object *obj )
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
	{
		TransportContain::addToContain( obj );
		return;
	}

	getRedirectedContain()->addToContain( obj );

}

//-------------------------------------------------------------------------------------------------
/** Remove 'obj' from the m_containList of objects in this module.
	* This will trigger an onRemoving event for the object that this module
	* is a part of and an onRemovedFrom event for the object being removed */
//-------------------------------------------------------------------------------------------------
void OverlordContain::removeFromContain( Object *obj, Bool exposeStealthUnits )
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
	{
		TransportContain::removeFromContain( obj, exposeStealthUnits );
		return;
	}

	getRedirectedContain()->removeFromContain( obj, exposeStealthUnits );

}

//-------------------------------------------------------------------------------------------------
/** Remove all contained objects from the contained list */
//-------------------------------------------------------------------------------------------------
void OverlordContain::removeAllContained( Bool exposeStealthUnits )
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
	{
		TransportContain::removeAllContained( exposeStealthUnits );
		return;
	}

	const ContainedItemsList *fullList = getRedirectedContain()->getContainedItemsList();

	Object *obj;
	ContainedItemsList::const_iterator it;
	it = (*fullList).begin();
	while( it != (*fullList).end() )
	{
		obj = *it;
		it++;
		removeFromContain( obj, exposeStealthUnits );
	}
}

//-------------------------------------------------------------------------------------------------
/** Iterate the contained list and call the callback on each of the objects */
//-------------------------------------------------------------------------------------------------
void OverlordContain::iterateContained( ContainIterateFunc func, void *userData, Bool reverse )
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
	{
		TransportContain::iterateContained( func, userData, reverse );
		return;
	}

	getRedirectedContain()->iterateContained( func, userData, reverse );
}

//-------------------------------------------------------------------------------------------------
void OverlordContain::onContaining( Object *obj, Bool wasSelected )
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
	{
		TransportContain::onContaining( obj, wasSelected );


    if ( obj->isKindOf( KINDOF_PORTABLE_STRUCTURE ) )
    {
  		activateRedirectedContain();//Am now carrying something

			// And this contain style explicitly sucks XP from our little friend.
			if( getOverlordContainModuleData()->m_experienceSinkForRider  &&  obj->getExperienceTracker() )
				obj->getExperienceTracker()->setExperienceSink(getObject()->getID());


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


    return;
	}

	OpenContain::onContaining( obj, wasSelected );

	getRedirectedContain()->onContaining( obj, wasSelected );

}

//-------------------------------------------------------------------------------------------------
void OverlordContain::killAllContained( void )
{
	// This is a game call meant to clear actual passengers.  We don't want it to kill our turret.  That'd be weird.
	if( getRedirectedContain() )
	{
		getRedirectedContain()->killAllContained();
	}
}

//-------------------------------------------------------------------------------------------------
void OverlordContain::onRemoving( Object *obj )
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
	{
		TransportContain::onRemoving( obj );
		return;
	}

	OpenContain::onRemoving(obj);

	getRedirectedContain()->onRemoving( obj );

}

//-------------------------------------------------------------------------------------------------
Bool OverlordContain::isValidContainerFor(const Object* obj, Bool checkCapacity) const
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
		return TransportContain::isValidContainerFor( obj, checkCapacity );

	return getRedirectedContain()->isValidContainerFor( obj, checkCapacity );
}

//-------------------------------------------------------------------------------------------------
UnsignedInt OverlordContain::getContainCount() const
{
	ContainModuleInterface* redir = getRedirectedContain();

	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( redir == nullptr )
		return TransportContain::getContainCount( );

	return redir->getContainCount();
}

//-------------------------------------------------------------------------------------------------
Bool OverlordContain::getContainerPipsToShow(Int& numTotal, Int& numFull)
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
	{
		numTotal = 0;
		numFull = 0;
		return false;
	}
	else
	{
		return getRedirectedContain()->getContainerPipsToShow(numTotal, numFull);
	}
}

//-------------------------------------------------------------------------------------------------
Int OverlordContain::getContainMax( ) const
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
		return TransportContain::getContainMax( );

	return getRedirectedContain()->getContainMax();
}

//-------------------------------------------------------------------------------------------------
const ContainedItemsList* OverlordContain::getContainedItemsList() const
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
		return TransportContain::getContainedItemsList( );

	return getRedirectedContain()->getContainedItemsList();
}

//-------------------------------------------------------------------------------------------------
Bool OverlordContain::isEnclosingContainerFor( const Object *obj ) const
{
	// All of this redirection stuff makes it so that while I am normally a transport
	// for Overlord subObjects, once I have a passenger, _I_ become a transport of their type.
	// So, the answer to this question depends on if it is my passenger asking, or theirs.
	// As always, I can't use convenience functions that get redirected on a ? like this.
	if( m_containListSize > 0  &&  obj ==  m_containList.front() )
		return FALSE;

	return TRUE;
}

//-------------------------------------------------------------------------------------------------
Bool OverlordContain::isDisplayedOnControlBar() const
{
	// Do you mean me the Overlord, or my behavior of passing stuff on to my passengers?
	if( getRedirectedContain() == nullptr )
		return FALSE;//No need to call up inheritance, this is a module based question, and I say no.

	return getRedirectedContain()->isDisplayedOnControlBar();
}

//-------------------------------------------------------------------------------------------------
const Object *OverlordContain::friend_getRider() const
{
// The draw order dependency bug for riders means that our draw module needs to cheat to get
	//around it.	So this is another function that knows it is getting around redirection to ask
	// an Overlord specific function.

 	if( m_containListSize > 0 )
 		return m_containList.front();


	return nullptr;
}

//-------------------------------------------------------------------------------------------------
void OverlordContain::activateRedirectedContain()
{
	m_redirectionActivated = TRUE;
}

//-------------------------------------------------------------------------------------------------
void OverlordContain::deactivateRedirectedContain()
{
	m_redirectionActivated = FALSE;
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// if my object gets selected, then my visible passengers should, too
// this gets called from
void OverlordContain::clientVisibleContainedFlashAsSelected()
{
	// THIS OVERRIDES GRAHAMS NASTY OVERRIDE THING
	// SO WE CAN FLASH THE PORTABLE BUNKER INSTEAD OF ITS OCCUPANTS
	const ContainedItemsList* items = TransportContain::getContainedItemsList();

	if( items )
	{
		ContainedItemsList::const_iterator it;
		it = items->begin();

		while( it != items->end() )
		{
			Object *object = *it;
			if ( object && object->isKindOf( KINDOF_PORTABLE_STRUCTURE ) )
			{
				Drawable *draw = object->getDrawable();
				if ( draw )
				{
					draw->flashAsSelected(); //WOW!
				}
			}

			++it;
		}
	}

}


Bool OverlordContain::isPassengerAllowedToFire( ObjectID id ) const
{
	Object *passenger = TheGameLogic->findObjectByID(id);

	if(passenger != nullptr)
	{
		//only allow infantry, and turrets and such.  no vehicles.
		if(passenger->isKindOf(KINDOF_INFANTRY) == FALSE && passenger->isKindOf(KINDOF_PORTABLE_STRUCTURE) == FALSE)
			return FALSE;
	}


  if ( getObject() && getObject()->getContainedBy() ) // nested containment voids firing, always
    return FALSE;

  return TransportContain::isPassengerAllowedToFire();
}



// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void OverlordContain::crc( Xfer *xfer )
{

	// extend base class
	TransportContain::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void OverlordContain::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	TransportContain::xfer( xfer );

	// redirection activated
	xfer->xferBool( &m_redirectionActivated );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void OverlordContain::loadPostProcess( void )
{

	// extend base class
	TransportContain::loadPostProcess();

}
