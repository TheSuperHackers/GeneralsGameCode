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

// DisabledTypes.cpp /////////////////////////////////////////////////////////////////////////////////////
// Kris Morness, September 2002

#include "PreRTS.h"

#include "Common/DisabledTypes.h"
#include "Common/BitFlagsIO.h"

template<>
const char* DisabledMaskType::s_bitNameList[] =
{
	"DEFAULT",
	"DISABLED_HACKED",
	"DISABLED_EMP",
	"DISABLED_HELD",
	"DISABLED_PARALYZED",
	"DISABLED_UNMANNED",
	"DISABLED_UNDERPOWERED",
	"DISABLED_FREEFALL",

	"DISABLED_SCRIPT_DISABLED",
	"DISABLED_SCRIPT_UNDERPOWERED",

	NULL
};

DisabledMaskType DISABLEDMASK_NONE;	// inits to all zeroes
DisabledMaskType DISABLEDMASK_ALL;

void initDisabledMasks()
{
	SET_ALL_DISABLEDMASK_BITS( DISABLEDMASK_ALL );
}
