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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: SkirmishPreferences.h
// Author: Chris Huybregts, August 2002
// Description: Saving/Loading of skirmish preferences
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/UserPreferences.h"

//-----------------------------------------------------------------------------
// SkirmishPreferences class
//-----------------------------------------------------------------------------
class SkirmishPreferences : public UserPreferences
{
public:
	SkirmishPreferences();
	virtual ~SkirmishPreferences();

	Bool loadFromIniFile();

	virtual Bool write();
	AsciiString getSlotList();
	void setSlotList();
	UnicodeString getUserName();		// convenience function
	Int getPreferredFaction();			// convenience function
	Int getPreferredColor();				// convenience function
	AsciiString getPreferredMap();	// convenience function
	Bool usesSystemMapDir();		// convenience function

  Bool getSuperweaponRestricted() const;
  void setSuperweaponRestricted( Bool superweaponRestricted);

  Money getStartingCash() const;
  void setStartingCash( const Money &startingCash );
};
