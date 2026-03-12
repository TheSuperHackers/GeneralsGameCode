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
// FILE: CustomMatchPreferences.h
// Author: Matthew D. Campbell, August 2002
// Description: Saving/Loading of custom match preferences
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/UserPreferences.h"

//-----------------------------------------------------------------------------
// CustomMatchPreferences base class
//-----------------------------------------------------------------------------
class CustomMatchPreferences : public UserPreferences
{
public:
	CustomMatchPreferences();
	virtual ~CustomMatchPreferences();

	void setLastLadder(const AsciiString& addr, UnsignedShort port);
	AsciiString getLastLadderAddr();
	UnsignedShort getLastLadderPort();

	Int getPreferredFaction();
	void setPreferredFaction(Int val);

	Int getPreferredColor();
	void setPreferredColor(Int val);

	Int getChatSizeSlider();
	void setChatSizeSlider(Int val);

	AsciiString getPreferredMap();
	void setPreferredMap(AsciiString val);

	Bool usesSystemMapDir();
	void setUsesSystemMapDir(Bool val);

	Bool usesLongGameList();
	void setUsesLongGameList(Bool val);

	Bool allowsObservers();
	void setAllowsObserver(Bool val);

	Bool getDisallowAsianText();
	void setDisallowAsianText(Bool val);

	Bool getDisallowNonAsianText();
	void setDisallowNonAsianText( Bool val );

  Bool getSuperweaponRestricted() const;
  void setSuperweaponRestricted( Bool superweaponRestricted);

  Money getStartingCash() const;
  void setStartingCash( const Money &startingCash );

  Bool getFactionsLimited() const; // Prefers to only use the original 3 sides, not USA Air Force General, GLA Toxin General, et al
  void setFactionsLimited( Bool factionsLimited );

  Bool getUseStats() const;
  void setUseStats( Bool useStats );
};
