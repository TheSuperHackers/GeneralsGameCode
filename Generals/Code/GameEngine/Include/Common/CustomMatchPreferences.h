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
// FILE: CustomMatchPreferences.h
// Author: Matthew D. Campbell, August 2002
// Description: Saving/Loading of custom match preferences
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __CUSTOMMATCHPREFERENCES_H__
#define __CUSTOMMATCHPREFERENCES_H__

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
	AsciiString getLastLadderAddr( void );
	UnsignedShort getLastLadderPort( void );

	Int getPreferredFaction(void);
	void setPreferredFaction(Int val);

	Int getPreferredColor(void);
	void setPreferredColor(Int val);

	Int getChatSizeSlider(void);
	void setChatSizeSlider(Int val);

	AsciiString getPreferredMap(void);
	void setPreferredMap(AsciiString val);

	Bool usesSystemMapDir(void);
	void setUsesSystemMapDir(Bool val);

	Bool usesLongGameList(void);
	void setUsesLongGameList(Bool val);

	Bool allowsObservers(void);
	void setAllowsObserver(Bool val);

	Bool getDisallowAsianText( void );
	void setDisallowAsianText(Bool val);

	Bool getDisallowNonAsianText( void );
	void setDisallowNonAsianText( Bool val );

};

#endif // __CUSTOMMATCHPREFERENCES_H__
