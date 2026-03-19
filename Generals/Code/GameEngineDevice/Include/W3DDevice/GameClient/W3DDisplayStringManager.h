/*
**	Command & Conquer Generals(tm)
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

#include "GameClient/DisplayStringManager.h"
#include "W3DDevice/GameClient/W3DDisplayString.h"

//-------------------------------------------------------------------------------------------------
/** Access for creating game managed display strings */
//-------------------------------------------------------------------------------------------------

#define MAX_GROUPS 10

class W3DDisplayStringManager : public DisplayStringManager
{

public:

	W3DDisplayStringManager();
	virtual ~W3DDisplayStringManager();

	// Initialize our numeral strings in postProcessLoad
	virtual void postProcessLoad();

	/// update method for all our display strings
	virtual void update();

	/// allocate a new display string
	virtual DisplayString *newDisplayString();

	/// free a display string
	virtual void freeDisplayString( DisplayString *string );

	// This is used to save us a few FPS and storage space. There's no reason to
	// duplicate the DisplayString on every drawable when 1 copy will suffice.
	virtual DisplayString *getGroupNumeralString( Int numeral );
	virtual DisplayString *getFormationLetterString() { return m_formationLetterDisplayString; };

protected:
	DisplayString *m_groupNumeralStrings[ MAX_GROUPS ];
	DisplayString *m_formationLetterDisplayString;

};
