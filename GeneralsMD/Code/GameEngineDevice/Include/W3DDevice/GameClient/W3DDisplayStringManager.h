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

// FILE: W3DDisplayStringManager.h ////////////////////////////////////////////////////////////////
// Created:    Colin Day, July 2001
// Desc:       Access for creating game managed display strings
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _W3DDISPLAYSTRINGMANAGER_H_
#define _W3DDISPLAYSTRINGMANAGER_H_

#include "GameClient/DisplayStringManager.h"
#include "W3DDevice/GameClient/W3DDisplayString.h"

//-------------------------------------------------------------------------------------------------
/** Access for creating game managed display strings */
//-------------------------------------------------------------------------------------------------
//#define KRIS_BRUTAL_HACK_FOR_AIRCRAFT_CARRIER_DEBUGGING

#ifdef KRIS_BRUTAL_HACK_FOR_AIRCRAFT_CARRIER_DEBUGGING
	#define MAX_GROUPS 20
#else
	#define MAX_GROUPS 10
#endif

class W3DDisplayStringManager : public DisplayStringManager
{

public:

	W3DDisplayStringManager( void );
	virtual ~W3DDisplayStringManager( void );

	// Initialize our numeral strings in postProcessLoad
	virtual void postProcessLoad( void );

	/// update method for all our display strings
	virtual void update( void );

	/// allocate a new display string
	virtual DisplayString *newDisplayString( void );

	/// free a display string
	virtual void freeDisplayString( DisplayString *string );

	// This is used to save us a few FPS and storage space. There's no reason to
	// duplicate the DisplayString on every drawable when 1 copy will suffice.
	virtual DisplayString *getGroupNumeralString( Int numeral );
	virtual DisplayString *getFormationLetterString( void ) { return m_formationLetterDisplayString; };

protected:
	DisplayString *m_groupNumeralStrings[ MAX_GROUPS ];
	DisplayString *m_formationLetterDisplayString;

};

#endif // _W3DDISPLAYSTRINGMANAGER_H_

