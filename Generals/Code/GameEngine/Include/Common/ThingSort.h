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

// FILE: ThingSort.h //////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   A couple of "buckets" so that we can have things sort easily in the editor
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __THINGSORT_H_
#define __THINGSORT_H_

//-------------------------------------------------------------------------------------------------
enum EditorSortingType
{
	ES_FIRST = 0,

	ES_NONE = ES_FIRST,
	ES_STRUCTURE,
	ES_INFANTRY,
	ES_VEHICLE,
	ES_SHRUBBERY,
	ES_MISC_MAN_MADE,
	ES_MISC_NATURAL,
	ES_DEBRIS,
	ES_SYSTEM,				// game "system" stuff (programmer objects, not objects to put on a map)
	ES_AUDIO,
	ES_TEST,					// for test stuff loaded for the world builder
	ES_FOR_REVIEW,						// awaiting review from the divine messenger
	ES_ROAD,						// road objects...should never actually be in the object panel.
	ES_WAYPOINT,					// waypoint objects...should never actually be in the object panel.

	ES_NUM_SORTING_TYPES      // keep this last

};
#ifdef DEFINE_EDITOR_SORTING_NAMES
static const char *EditorSortingNames[] = 
{
	"NONE",
	"STRUCTURE",
	"INFANTRY",
	"VEHICLE",
	"SHRUBBERY",
	"MISC_MAN_MADE",
	"MISC_NATURAL",
	"DEBRIS",
	"SYSTEM",
	"AUDIO",
	"TEST",
	"FOR_REVIEW",
	"ROAD",
	"WAYPOINT",

	NULL
};
#endif

#endif // __THINGSORT_H_

