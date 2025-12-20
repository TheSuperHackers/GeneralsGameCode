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

// Registry.h
// Simple interface for storing/retreiving registry values
// Author: Matthew D. Campbell, December 2001

#pragma once

#include <Common/AsciiString.h>

/**
 * Get a string from the original Generals Registry
 */
Bool GetStringFromGeneralsRegistry(AsciiString path, AsciiString key, AsciiString& val);
/**
	* Get a string from the registry
	*/
Bool GetStringFromRegistry(AsciiString path, AsciiString key, AsciiString& val);

/**
	* Get an unsigned int from the registry
	*/
Bool GetUnsignedIntFromRegistry(AsciiString path, AsciiString key, UnsignedInt& val);

AsciiString GetRegistryLanguage(void); // convenience function
AsciiString GetRegistryGameName(void); // convenience function
UnsignedInt GetRegistryVersion(void); // convenience function
UnsignedInt GetRegistryMapPackVersion(void); // convenience function
