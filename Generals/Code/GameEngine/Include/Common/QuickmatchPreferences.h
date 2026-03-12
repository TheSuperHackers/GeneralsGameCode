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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: QuickmatchPreferences.h
// Author: Matthew D. Campbell, August 2002
// Description: Saving/Loading of quickmatch preferences
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/UserPreferences.h"

//-----------------------------------------------------------------------------
// QuickMatchPreferences base class
//-----------------------------------------------------------------------------
class QuickMatchPreferences : public UserPreferences
{
public:
	QuickMatchPreferences();
	virtual ~QuickMatchPreferences();

	void setMapSelected(const AsciiString& mapName, Bool selected);
	Bool isMapSelected(const AsciiString& mapName);

	void setLastLadder(const AsciiString& addr, UnsignedShort port);
	AsciiString getLastLadderAddr();
	UnsignedShort getLastLadderPort();

	void setMaxDisconnects(Int val);
	Int getMaxDisconnects();

	void setMaxPoints(Int val);
	Int getMaxPoints();

	void setMinPoints(Int val);
	Int getMinPoints();

	void setWaitTime(Int val);
	Int getWaitTime();

	void setNumPlayers(Int val);
	Int getNumPlayers();

	void setMaxPing(Int val);
	Int getMaxPing();

	void setColor(Int val);
	Int getColor();

	void setSide(Int val);
	Int getSide();
};
