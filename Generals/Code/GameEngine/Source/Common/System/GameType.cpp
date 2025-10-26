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

// GameType.cpp ///////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

const char *const TimeOfDayNames[] =
{
	"NONE",
	"MORNING",
	"AFTERNOON",
	"EVENING",
	"NIGHT",

	NULL
};
static_assert(ARRAY_SIZE(TimeOfDayNames) == TIME_OF_DAY_COUNT + 1, "Incorrect array size");

const char *const WeatherNames[] =
{
	"NORMAL",
	"SNOWY",

	NULL
};
static_assert(ARRAY_SIZE(WeatherNames) == WEATHER_COUNT + 1, "Incorrect array size");
