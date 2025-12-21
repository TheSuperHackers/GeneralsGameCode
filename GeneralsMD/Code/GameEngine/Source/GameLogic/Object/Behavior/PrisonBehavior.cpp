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

// FILE: PrisonBehavior.cpp ///////////////////////////////////////////////////////////////////////
// Author: Colin Day, August 2002
// Desc:   Prison Behavior
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/GameState.h"
#include "Common/Player.h"
#include "Common/RandomValue.h"
#include "Common/ThingFactory.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"
#include "GameClient/GameClient.h"
#include "GameClient/Line2D.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/PrisonBehavior.h"

#ifdef ALLOW_SURRENDER

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class PrisonVisual : public MemoryPoolObject
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( PrisonVisual, "PrisonVisual" )

public:

	PrisonVisual( void );
	// virtual destructor prototype provied by memory pool object

	ObjectID m_objectID;				///< object that is contained
	DrawableID m_drawableID;		///< associated visual prisoner drawable
	PrisonVisual *m_next;				///< next

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PrisonVisual::PrisonVisual( void )
{

	m_objectID = INVALID_ID;
	m_drawableID = INVALID_DRAWABLE_ID;
	m_next = nullptr;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PrisonVisual::~PrisonVisual( void )
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PrisonBehaviorModuleData::PrisonBehaviorModuleData( void )
{

	m_showPrisoners = FALSE;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void PrisonBehaviorModuleData::buildFieldParse( MultiIniFieldParse &p )
{
  OpenContainModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{

		{ "ShowPrisoners",	INI::parseBool,					nullptr,		offsetof( PrisonBehaviorModuleData, m_showPrisoners ) },
		{ "YardBonePrefix",	INI::parseAsciiString,	nullptr,		offsetof( PrisonBehaviorModuleData, m_prisonYardBonePrefix ) },
		{ 0, 0, 0, 0 }

	};

  p.add( dataFieldParse );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PrisonBehavior::PrisonBehavior( Thing *thing, const ModuleData *moduleData )
							: OpenContain( thing, moduleData )
{

	m_visualList = nullptr;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
PrisonBehavior::~PrisonBehavior( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void PrisonBehavior::onDelete( void )
{

	// extend functionality
	OpenContain::onDelete();

	// delete our list
	Drawable *draw;
	PrisonVisual *visual;
	while( m_visualList )
	{

		// delete drawable if found
		draw = TheGameClient->findDrawableByID( m_visualList->m_drawableID );
		if( draw )
			TheGameClient->destroyDrawable( draw );

		// delete element and set next to head
		visual = m_visualList->m_next;
		deleteInstance(m_visualList);
		m_visualList = visual;

	}

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void PrisonBehavior::onContaining( Object *obj, Bool wasSelected )
{

	// extend functionality
	OpenContain::onContaining( obj, wasSelected );

	// objects inside a prison are held
	obj->setDisabled( DISABLED_HELD );

	// if we show visuals, make one
	const PrisonBehaviorModuleData *modData = getPrisonBehaviorModuleData();
	if( modData->m_showPrisoners )
		addVisual( obj );

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void PrisonBehavior::onRemoving( Object *obj )
{

	// if we show visuals, remove one
	const PrisonBehaviorModuleData *modData = getPrisonBehaviorModuleData();
	if( modData->m_showPrisoners )
		removeVisual( obj );

	// object is no longer held inside a garrisoned building
	obj->clearDisabled( DISABLED_HELD );

	// extend functionality
	OpenContain::onRemoving( obj );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE ////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
/** Pick a random location inside the prison yard */
// ------------------------------------------------------------------------------------------------
void PrisonBehavior::pickVisualLocation( Coord3D *pos )
{
	Object *us = getObject();
	const PrisonBehaviorModuleData *modData = getPrisonBehaviorModuleData();
	Int i;

	// sanity
	if( pos == nullptr )
		return;

	// initialize the picked location to that of the prison center
	Coord3D pickedLocation = *us->getPosition();

	// get the positions of the bones that make up the prison yard area
	const Int MAX_YARD_BONES = 16;
	Coord3D yardPositions[ MAX_YARD_BONES ];
	Int yardBones = us->getMultiLogicalBonePosition( modData->m_prisonYardBonePrefix.str(),
																									 MAX_YARD_BONES,
																									 yardPositions,
																									 nullptr );

	//
	// we must have at least 3 bone locations to make a yard polygon, otherwise we'll
	// default to the object position
	//
	if( yardBones >= 3 )
	{

		// find the bounding region of the yard area
		Region2D yardRegion;
		yardRegion.lo.x = yardPositions[ 0 ].x;
		yardRegion.lo.y = yardPositions[ 0 ].y;
		yardRegion.hi.x = yardPositions[ 0 ].x;
		yardRegion.hi.y = yardPositions[ 0 ].y;
		for( i = 1; i < yardBones; i++ )
		{

			if( yardPositions[ i ].x < yardRegion.lo.x )
				yardRegion.lo.x = yardPositions[ i ].x;
			if( yardPositions[ i ].y < yardRegion.lo.y )
				yardRegion.lo.y = yardPositions[ i ].y;
			if( yardPositions[ i ].x > yardRegion.hi.x )
				yardRegion.hi.x = yardPositions[ i ].x;
			if( yardPositions[ i ].y > yardRegion.hi.y )
				yardRegion.hi.y = yardPositions[ i ].y;

		}

		//
		// now that we have a yard region, the default visual position will be in the middle
		// of the yard region instead of the position of our object
		//
		pickedLocation.x = yardRegion.lo.x + yardRegion.width() / 2.0f;
		pickedLocation.y = yardRegion.lo.y + yardRegion.height() / 2.0f;
		// NOTE: pickedLocation.z is left alone at the object center Z

		// loop till we find a valid location that is inside the yard area
		Int maxTries = 32;
		Coord3D loc;
		for( i = 0; i < maxTries; ++i )
		{

			// pick a location
			loc.x = GameLogicRandomValueReal( yardRegion.lo.x, yardRegion.hi.x );
			loc.y = GameLogicRandomValueReal( yardRegion.lo.y, yardRegion.hi.y );
			loc.z = pickedLocation.z;

			// must be inside the yard polygon
			if( PointInsideArea2D( &loc, yardPositions, yardBones ) == TRUE )
			{

				// use this location, leave Z alone as the center of the prison
				pickedLocation = loc;
				break;  // exit for i

			}

		}

	}

	// return the location picked
	*pos = pickedLocation;

}

// ------------------------------------------------------------------------------------------------
/** Add prisoner visual to the prison yard */
// ------------------------------------------------------------------------------------------------
void PrisonBehavior::addVisual( Object *obj )
{

	// sanity
	if( obj == nullptr )
		return;

	// create a drawable
	Drawable *draw = TheThingFactory->newDrawable( obj->getTemplate() );

	// set the color of the drawable to that of the object
	if (TheGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT)
		draw->setIndicatorColor( obj->getNightIndicatorColor() );
	else
		draw->setIndicatorColor( obj->getIndicatorColor() );

	// pick a location insid the prison yard
	Coord3D pos;
	pickVisualLocation( &pos );

	// place drawable withing the prison yard area
	draw->setPosition( &pos );
	draw->setOrientation( GameLogicRandomValueReal( 0, TWO_PI ) );
	DrawableInfo *drawInfo=draw->getDrawableInfo();
	drawInfo->m_shroudStatusObjectID=getObject()->getID();

	// record this object/drawable pair
	PrisonVisual *visual = newInstance(PrisonVisual);
	visual->m_objectID = obj->getID();
	visual->m_drawableID = draw->getID();
	visual->m_next = m_visualList;
	m_visualList = visual;

}

// ------------------------------------------------------------------------------------------------
/** Remove prisoner visual from the prison yard */
// ------------------------------------------------------------------------------------------------
void PrisonBehavior::removeVisual( Object *obj )
{

	// sanity
	if( obj == nullptr )
		return;

	// initialize a drawable ID to invalid
	DrawableID drawableID = INVALID_DRAWABLE_ID;

	// find visual info in our list, once found, take this opportunity to remove it from that list
	PrisonVisual *visual, *prevVisual = nullptr;
	for( visual = m_visualList; visual; visual = visual->m_next )
	{

		// is this the one we're looking for
		if( visual->m_objectID == obj->getID() )
		{

			// record the information we need here
			drawableID = visual->m_drawableID;

			// remove from list
			if( prevVisual )
				prevVisual->m_next = visual->m_next;
			else
				m_visualList = visual->m_next;

			// delete the element
			deleteInstance(visual);

			break;  // exit for

		}

		// keep a pointer to the previous element
		prevVisual = visual;

	}

	// find the drawable visual and destroy it
	Drawable *draw = TheGameClient->findDrawableByID( drawableID );
	if( draw )
		TheGameClient->destroyDrawable( draw );

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void PrisonBehavior::crc( Xfer *xfer )
{

	// extend base class
	OpenContain::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void PrisonBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	OpenContain::xfer( xfer );

	// count and data for the prison visuals
	UnsignedShort visualCount = 0;
	PrisonVisual *visual;
	for( visual = m_visualList; visual; visual = visual->m_next )
		visualCount++;
	xfer->xferUnsignedShort( &visualCount );
	if( xfer->getXferMode() == XFER_SAVE )
	{

		// write all data
		for( visual = m_visualList; visual; visual = visual->m_next )
		{

			// object id
			xfer->xferObjectID( &visual->m_objectID );

			// drawable id
			xfer->xferDrawableID( &visual->m_drawableID );

		}

	}
	else
	{

		// the visual list should be empty
		if( m_visualList != nullptr )
		{

			DEBUG_CRASH(( "PrisonBehavior::xfer - the visual list should be empty but is not" ));
			throw SC_INVALID_DATA;

		}

		// read each item
		for( UnsignedShort i = 0; i < visualCount; ++i )
		{

			// allocate a new visual and tie to list
			visual = newInstance(PrisonVisual);
			visual->m_next = m_visualList;
			m_visualList = visual;

			// read object id
			xfer->xferObjectID( &visual->m_objectID );

			// read drawable id
			xfer->xferDrawableID( &visual->m_drawableID );

		}

	}

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void PrisonBehavior::loadPostProcess( void )
{

	// extend base class
	OpenContain::loadPostProcess();

}

#endif

