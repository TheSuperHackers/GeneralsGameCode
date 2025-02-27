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

// FILE: Powers.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:	 Unit power definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//
// skeleton definition of unit powers
//

// power bit flags, keep this in sync with PowerNames
enum
{
	POWER_NONE = 0,				// 0x0000000000000000
	POWER_FASTER,					// 0x0000000000000001
	POWER_DOUBLE_SHOT,		// 0x0000000000000002
	POWER_SELF_HEALING,		// 0x0000000000000004

	POWERS_NUM_POWERS
};

#ifdef DEFINE_POWER_NAMES
static const char *const PowerNames[] =
{
	"NONE",
	"FASTER",
	"DOUBLE_SHOT",
	"SELF_HEALING",
	NULL
};
static_assert(ARRAY_SIZE(PowerNames) == POWERS_NUM_POWERS + 1, "Incorrect array size");
#endif  // end DEFINE_POWER_NAMES
