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
// FILE: UserPreferences.h
// Author: Matthew D. Campbell, April 2002
// Description: Saving/Loading of user preferences
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/STLTypedefs.h"

class Money;

//-----------------------------------------------------------------------------
// PUBLIC TYPES ///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

typedef std::map<AsciiString, AsciiString> PreferenceMap;

//-----------------------------------------------------------------------------
// UserPreferences base class
//-----------------------------------------------------------------------------
class UserPreferences : public PreferenceMap
{
public:
	UserPreferences();
	virtual ~UserPreferences();

	// Loads or creates a file with the given name in the user data directory.
	virtual Bool load(AsciiString fname);
	virtual Bool write();

	Bool getBool(AsciiString key, Bool defaultValue) const;
	Real getReal(AsciiString key, Real defaultValue) const;
	Int getInt(AsciiString key, Int defaultValue) const;
	AsciiString getAsciiString(AsciiString key, AsciiString defaultValue) const;

	void setBool(AsciiString key, Bool val);
	void setReal(AsciiString key, Real val);
	void setInt(AsciiString key, Int val);
	void setAsciiString(AsciiString key, AsciiString val);

protected:
	AsciiString m_filename;
};

//-----------------------------------------------------------------------------
// LANPreferences class
//-----------------------------------------------------------------------------
class LANPreferences : public UserPreferences
{
public:
	LANPreferences();
	virtual ~LANPreferences();

	Bool loadFromIniFile();

	UnicodeString getUserName();		// convenience function
	Int getPreferredFaction();			// convenience function
	Int getPreferredColor();				// convenience function
	AsciiString getPreferredMap();	// convenience function
	Bool usesSystemMapDir();		// convenience function
	Int getNumRemoteIPs();					// convenience function
	UnicodeString getRemoteIPEntry(Int i);	// convenience function

  Bool getSuperweaponRestricted() const;
  Money getStartingCash() const;
  void setSuperweaponRestricted( Bool superweaponRestricted);
  void setStartingCash( const Money & startingCash );
};
