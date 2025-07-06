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

// GameCommon.h
// Part of header detangling
// John McDonald, Aug 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameCommon.h"

const char *TheVeterancyNames[] = 
{
	"REGULAR",
	"VETERAN",
	"ELITE",
	"HEROIC",
	NULL
};

const char *TheRelationshipNames[] =
{
	"ENEMIES",
	"NEUTRAL",
	"ALLIES",
	NULL
};

//-------------------------------------------------------------------------------------------------
Real normalizeAngle(Real angle)
{
	DEBUG_ASSERTCRASH(!_isnan(angle), ("Angle is NAN in normalizeAngle!\n"));

	if( _isnan(angle) )
		return 0;// ARGH!!!! Don't assert and then not handle it!  Error bad!  Fix error!

	while (angle > PI) 
		angle -= 2*PI;

	while (angle <= -PI) 
		angle += 2*PI;

	return angle;
}

