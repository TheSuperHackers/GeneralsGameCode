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

// FILE: RayEffect.cpp ////////////////////////////////////////////////////////////////////////////
// Created:   Colin Day, May 2001
// Desc:      Ray effect system manager
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/RayEffect.h"
#include "GameClient/Drawable.h"

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
class RayEffectSystem *TheRayEffects = nullptr;

// PRIVATE METHODS ////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Find an effect entry given a drawable */
//-------------------------------------------------------------------------------------------------
RayEffectData *RayEffectSystem::findEntry( const Drawable *draw )
{
	Int i;
	RayEffectData *effectData = nullptr;

	// find the matching effect data entry
	for( i = 0; i < MAX_RAY_EFFECTS; i++ )
	{

		if( m_effectData[ i ].draw == draw )
		{

			effectData = &m_effectData[ i ];
			break;  // exit for i

		}

	}

	return effectData;

}

// PUBLIC METHODS /////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RayEffectSystem::RayEffectSystem( void )
{

	init();

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RayEffectSystem::~RayEffectSystem( void )
{

}

//-------------------------------------------------------------------------------------------------
/** initialize the system */
//-------------------------------------------------------------------------------------------------
void RayEffectSystem::init( void )
{
	Int i;

	for( i = 0; i < MAX_RAY_EFFECTS; i++ )
	{

		m_effectData[ i ].draw = nullptr;
		m_effectData[ i ].startLoc.zero();
		m_effectData[ i ].endLoc.zero();

	}

}

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void RayEffectSystem::reset( void )
{

	// nothing dynamic going on here, just initialize it
	init();

}

//-------------------------------------------------------------------------------------------------
/** add a ray effect entry for this drawable */
//-------------------------------------------------------------------------------------------------
void RayEffectSystem::addRayEffect( const Drawable *draw,
																	  const Coord3D *startLoc,
																	  const Coord3D *endLoc )
{
	Int i;
	RayEffectData *effectData = nullptr;

	// sanity
	if( draw == nullptr || startLoc == nullptr || endLoc == nullptr )
		return;

	/** @todo this should be more intelligent and should not be limited
	to any kind of max ray effects, this is all a temporary hack system for
	the demo anyway right now though */

	// search for a free effect slot
	for( i = 0; i < MAX_RAY_EFFECTS; i++ )
	{

		if( m_effectData[ i ].draw == nullptr )
		{

			effectData = &m_effectData[ i ];
			break;  // exit for

		}

	}

	// if no free slots we can't do it
	if( effectData == nullptr )
		return;

	// add the data to the entry
	effectData->draw = draw;
	effectData->startLoc = *startLoc;
	effectData->endLoc = *endLoc;

}

//-------------------------------------------------------------------------------------------------
/** given a drawable, remove its effect from the system */
//-------------------------------------------------------------------------------------------------
void RayEffectSystem::deleteRayEffect( const Drawable *draw )
{
	RayEffectData *effectData = nullptr;

	// sanity
	if( draw == nullptr )
		return;

	// find the effect entry
	effectData = findEntry( draw );
	if( effectData )
	{

		// remove the data for this entry
		effectData->draw = nullptr;

	}

}

//-------------------------------------------------------------------------------------------------
/** given a drawable, if it is in the ray effect system list retrieve
	*	the ray effect data for its entry */
//-------------------------------------------------------------------------------------------------
void RayEffectSystem::getRayEffectData( const Drawable *draw,
																			  RayEffectData *effectData )
{
	RayEffectData *entry = nullptr;

	// sanity
	if( draw == nullptr || effectData == nullptr )
		return;

	// find the effect data entry
	entry = findEntry( draw );
	if( entry )
	{

		// data has been found, copy to parameter
		*effectData = *entry;

	}

}

