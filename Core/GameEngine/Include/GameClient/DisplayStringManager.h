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

// FILE: DisplayStringManager.h ///////////////////////////////////////////////////////////////////
// Created:    Colin Day, July 2001
// Desc:       Access for creating game managed display strings
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/SubsystemInterface.h"
#include "GameClient/DisplayString.h"

//-------------------------------------------------------------------------------------------------
/** Factory for managing and creating display strings */
//-------------------------------------------------------------------------------------------------
class DisplayStringManager : public SubsystemInterface
{

public:

	DisplayStringManager();
	virtual ~DisplayStringManager() override;

	virtual void init() override {}			///< initialize the factory
	virtual void reset() override {}			///< reset system
	virtual void update() override {};		///< update anything we need to in our strings

	virtual DisplayString *newDisplayString() = 0;  ///< allocate new display string
	virtual void freeDisplayString( DisplayString *string );  ///< free string

	virtual DisplayString *getGroupNumeralString( Int numeral ) = 0;
	virtual DisplayString *getFormationLetterString() = 0;
protected:

	void link( DisplayString *string );  ///< link display string to list
	void unLink( DisplayString *string );  ///< unlink display string from list
	void setDefaultFont( DisplayString *string );  ///< assign the default display string font

	DisplayString *m_stringList;  ///< list of all display strings
	DisplayString *m_currentCheckpoint; ///< current checkpoint of strings to be freed
};

// TheSuperHackers @performance CryoTheRenegade 14/10/2026
// DisplayStringManager that creates DisplayStringDummy strings. Used for Headless Mode.
class DisplayStringManagerDummy : public DisplayStringManager
{
public:
	DisplayStringManagerDummy();
	virtual ~DisplayStringManagerDummy() override;

	virtual void postProcessLoad() override;

	virtual DisplayString *newDisplayString() override;

	virtual DisplayString *getGroupNumeralString( Int numeral ) override { return m_markerString; }
	virtual DisplayString *getFormationLetterString() override { return m_markerString; }

private:
	DisplayString *m_markerString; ///< shared by all group numerals and the formation letter
};

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern DisplayStringManager *TheDisplayStringManager;  ///< singleton extern
