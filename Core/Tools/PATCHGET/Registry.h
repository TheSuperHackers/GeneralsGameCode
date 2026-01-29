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

// Registry.h
// Simple interface for storing/retrieving registry values
// Author: Matthew D. Campbell, December 2001

#pragma once

#include <string>

namespace patchget
{

/**
	* Get a string from the registry
	*/
bool GetStringFromRegistry(std::string path, std::string key, std::string& val);

/**
	* Get an unsigned int from the registry
	*/
bool GetUnsignedIntFromRegistry(std::string path, std::string key, unsigned int& val);

/**
	* Store a string in the registry - returns true on success
	*/
bool SetStringInRegistry(std::string path, std::string key, std::string val);

/**
	* Store an unsigned int in the registry - returns true on success
	*/
bool SetUnsignedIntInRegistry(std::string path, std::string key, unsigned int val);

} // namespace patchget
