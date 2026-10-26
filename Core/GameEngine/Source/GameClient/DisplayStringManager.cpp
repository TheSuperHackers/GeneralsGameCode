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

// FILE: DisplayStringManager.cpp /////////////////////////////////////////////////////////////////
// Created:    Colin Day, July 2001
// Desc:       Access for creating game managed display strings
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"    // This must go first in EVERY cpp file in the GameEngine

#include "GameClient/DisplayStringManager.h"
#include "GameClient/GlobalLanguage.h"

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
DisplayStringManager* TheDisplayStringManager = nullptr;

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DisplayStringManager::DisplayStringManager()
{
	m_stringList = nullptr;
	m_currentCheckpoint = nullptr;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DisplayStringManager::~DisplayStringManager()
{
	//
	// we only keep track of the strings, we do NOT de-allocate them, our
	// list better be cleaned out before we destroy ourselves
	//
	assert(m_stringList == nullptr);
}

//-------------------------------------------------------------------------------------------------
/** Link a display string to the master list */
//-------------------------------------------------------------------------------------------------
void DisplayStringManager::link(DisplayString* string)
{
	assert(string);
	assert(string->m_next == nullptr);
	assert(string->m_prev == nullptr);

	string->m_next = m_stringList;
	if (m_stringList)
	{
		m_stringList->m_prev = string;
	}

	m_stringList = string;
}

//-------------------------------------------------------------------------------------------------
/** Unlink a display string from the master list */
//-------------------------------------------------------------------------------------------------
void DisplayStringManager::unLink(DisplayString* string)
{
	assert(string);
	assert(m_stringList);

	if (string->m_next)
	{
		string->m_next->m_prev = string->m_prev;
	}
	if (string->m_prev)
	{
		string->m_prev->m_next = string->m_next;
	}
	else
	{
		assert(string == m_stringList);
		m_stringList = string->m_next;
	}
}

//-------------------------------------------------------------------------------------------------
/** Assign the default font to a new display string */
//-------------------------------------------------------------------------------------------------
void DisplayStringManager::setDefaultFont(DisplayString* string)
{
	if (TheGlobalLanguageData && TheGlobalLanguageData->m_defaultDisplayStringFont.name.isNotEmpty())
	{
		string->setFont(TheFontLibrary->getFont(
		  TheGlobalLanguageData->m_defaultDisplayStringFont.name,
		  TheGlobalLanguageData->m_defaultDisplayStringFont.size,
		  TheGlobalLanguageData->m_defaultDisplayStringFont.bold));
	}
	else
	{
		string->setFont(TheFontLibrary->getFont("Times New Roman", 12, FALSE));
	}
}

//-------------------------------------------------------------------------------------------------
/** Remove a display string from the master list and delete the data */
//-------------------------------------------------------------------------------------------------
void DisplayStringManager::freeDisplayString(DisplayString* string)
{
	// sanity
	if (string == nullptr)
	{
		return;
	}

	// unlink
	unLink(string);

	// if the string happens to fall where our current checkpoint was, set the checkpoint to null
	if (m_currentCheckpoint == string)
	{
		m_currentCheckpoint = nullptr;
	}

	// free data
	deleteInstance(string);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DisplayStringManagerDummy::DisplayStringManagerDummy()
{
	m_markerString = nullptr;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DisplayStringManagerDummy::~DisplayStringManagerDummy()
{
	freeDisplayString(m_markerString);
	m_markerString = nullptr;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void DisplayStringManagerDummy::postProcessLoad()
{
	m_markerString = newDisplayString();
}

//-------------------------------------------------------------------------------------------------
/** Same default font as W3DDisplayStringManager, because callers read the font height */
//-------------------------------------------------------------------------------------------------
DisplayString* DisplayStringManagerDummy::newDisplayString()
{
	DisplayString* newString = newInstance(DisplayStringDummy);
	setDefaultFont(newString);
	link(newString);
	return newString;
}
