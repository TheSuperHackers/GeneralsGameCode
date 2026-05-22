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

// RandomValue.h
// Random number generation system
// Author: Michael S. Booth, January 1998

#pragma once

#include "Lib/BaseType.h"

extern void InitRandom();
extern void InitRandom( UnsignedInt seed );
extern UnsignedInt GetGameLogicRandomSeed();   ///< Get the seed (used for replays)
extern UnsignedInt GetGameLogicRandomSeedCRC();///< Get the seed (used for CRCs)

struct RandomValueClass
{
	virtual Int GetRandomValueInt( Int lo, Int hi, const char *file, Int line ) const = 0;
	virtual Real GetRandomValueReal( Real lo, Real hi, const char *file, Int line ) const = 0;
};
struct LogicRandomValueClass final : RandomValueClass
{
	virtual Int GetRandomValueInt( Int lo, Int hi, const char *file, Int line ) const override;
	virtual Real GetRandomValueReal( Real lo, Real hi, const char *file, Int line ) const override;
};
struct ClientRandomValueClass final : RandomValueClass
{
	virtual Int GetRandomValueInt( Int lo, Int hi, const char *file, Int line ) const override;
	virtual Real GetRandomValueReal( Real lo, Real hi, const char *file, Int line ) const override;
};

// use these macros to access the random value functions
#define RandomValueInt(randomValueClass, lo, hi) randomValueClass.GetRandomValueInt( lo, hi, __FILE__, __LINE__ )
#define RandomValueReal(randomValueClass, lo, hi) randomValueClass.GetRandomValueReal( lo, hi, __FILE__, __LINE__ )

//--------------------------------------------------------------------------------------------------------------
