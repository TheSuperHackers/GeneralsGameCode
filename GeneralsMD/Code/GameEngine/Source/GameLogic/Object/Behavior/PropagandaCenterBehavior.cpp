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

// FILE: PropagandaCenterBehavior.cpp /////////////////////////////////////////////////////////////
// Author: Colin Day, August 2002
// Desc:   Propaganda Center Behavior
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/PropagandaCenterBehavior.h"

#ifdef ALLOW_SURRENDER

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PropagandaCenterBehaviorModuleData::PropagandaCenterBehaviorModuleData( void )
{

	m_brainwashDuration = 0;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void PropagandaCenterBehaviorModuleData::buildFieldParse( MultiIniFieldParse &p )
{
  PrisonBehaviorModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{

		{	"BrainwashDuration", INI::parseDurationUnsignedInt, NULL, offsetof( PropagandaCenterBehaviorModuleData, m_brainwashDuration ) },
		{ 0, 0, 0, 0 }

	};

  p.add( dataFieldParse );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PropagandaCenterBehavior::PropagandaCenterBehavior( Thing *thing, const ModuleData *moduleData )
												: PrisonBehavior( thing, moduleData )
{

	m_brainwashingSubjectID = INVALID_ID;
	m_brainwashingSubjectStartFrame = 0;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PropagandaCenterBehavior::~PropagandaCenterBehavior( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void PropagandaCenterBehavior::onDelete( void )
{

	// extend functionality
	PrisonBehavior::onDelete();

	//
	// go through our list of brainwashed objects, and if they are still under our
	// control, return them to their original owners
	//
	for( BrainwashedIDListContIterator it = m_brainwashedList.begin();
			 it != m_brainwashedList.end();
			 ++it )
	{
		Object *obj;

		// get this object
		obj = TheGameLogic->findObjectByID( *it );
		if( obj )
		{

			// return this object under the control of the original owner
			obj->restoreOriginalTeam();

		}

	}

	// clear the list
	m_brainwashedList.clear();

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime PropagandaCenterBehavior::update( void )
{
	Object *us = getObject();
	const PropagandaCenterBehaviorModuleData *modData = getPropagandaCenterBehaviorModuleData();

	// extend functionality
	PrisonBehavior::update();

	// if we have a prisoner inside, continue the brainwashing on them (one at a time)
	if( m_brainwashingSubjectID != INVALID_ID )
	{
		Object *brainwashingSubject = TheGameLogic->findObjectByID( m_brainwashingSubjectID );

		if( brainwashingSubject )
		{

			// if we've been in here long enough, we come out brainwashed
			if( TheGameLogic->getFrame() - m_brainwashingSubjectStartFrame >= modData->m_brainwashDuration )
			{

				// only can exit if the prison allows us to
				ExitDoorType exitDoor = reserveDoorForExit(brainwashingSubject->getTemplate(), brainwashingSubject);
				if(exitDoor != DOOR_NONE_AVAILABLE)
				{

					// place this object under the control of the player
					Player *player = us->getControllingPlayer();
					DEBUG_ASSERTCRASH( player, ("Brainwashing: No controlling player for '%s'", us->getTemplate()->getName().str()) );
					if( player )
						brainwashingSubject->setTemporaryTeam( player->getDefaultTeam() );

					// remove any surrender status from this object
					AIUpdateInterface *ai = brainwashingSubject->getAIUpdateInterface();
					if( ai )
						ai->setSurrendered( NULL, FALSE );

					// add this object to our brainwashed list if we're not already in it
					for( BrainwashedIDListIterator it = m_brainwashedList.begin();
							 it != m_brainwashedList.end(); ++it )
						if( *it == brainwashingSubject->getID() )
							break;  // exit for

					if( it == m_brainwashedList.end() )
						m_brainwashedList.push_front( brainwashingSubject->getID() );

					// exit the prison
					exitObjectViaDoor( brainwashingSubject, exitDoor );

				}

			}

		}

	}

	// if we have no brainwashing subject, hook one up if we have people inside us
	if( m_brainwashingSubjectID == INVALID_ID )
	{

		// find the first object in our containment list
		if( getContainList().begin() != getContainList().end() )
		{
			Object *obj = getContainList().front();

			if( obj )
			{

				// assign brainwashing subject on this frame
				m_brainwashingSubjectID = obj->getID();
				m_brainwashingSubjectStartFrame = TheGameLogic->getFrame();

			}

		}

	}

	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void PropagandaCenterBehavior::onRemoving( Object *obj )
{

	// if we're removing the brainwashing subject, NULL the pointer
	if( m_brainwashingSubjectID == obj->getID() )
	{

		m_brainwashingSubjectID = INVALID_ID;
		m_brainwashingSubjectStartFrame = 0;

	}

	// extend functionality
	PrisonBehavior::onRemoving( obj );

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void PropagandaCenterBehavior::crc( Xfer *xfer )
{

	// extend base class
	PrisonBehavior::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void PropagandaCenterBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	PrisonBehavior::xfer( xfer );

	// brainwashing subject
	xfer->xferObjectID( &m_brainwashingSubjectID );

	// brainwashing subject start frame
	xfer->xferUnsignedInt( &m_brainwashingSubjectStartFrame );

	// brainwashed list size and data
	xfer->xferSTLObjectIDList( &m_brainwashedList );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void PropagandaCenterBehavior::loadPostProcess( void )
{

	// extend base class
	PrisonBehavior::loadPostProcess();

}

#endif
