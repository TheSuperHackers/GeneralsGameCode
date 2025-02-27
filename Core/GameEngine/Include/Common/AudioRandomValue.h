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

// AudioRandomValue.h
// Random number generation system
// Author: Michael S. Booth, January 1998
// Split out into separate Logic/Client/Audio headers by MDC Sept 2002

#pragma once

#include "Lib/BaseType.h"

// do NOT use these functions directly, rather use the macros below
extern Int GetGameAudioRandomValue( int lo, int hi, const char *file, int line );
extern Real GetGameAudioRandomValueReal( Real lo, Real hi, const char *file, int line );

// use these macros to access the random value functions
#define GameAudioRandomValue( lo, hi ) GetGameAudioRandomValue( lo, hi, __FILE__, __LINE__ )
#define GameAudioRandomValueReal( lo, hi ) GetGameAudioRandomValueReal( lo, hi, __FILE__, __LINE__ )

//--------------------------------------------------------------------------------------------------------------
