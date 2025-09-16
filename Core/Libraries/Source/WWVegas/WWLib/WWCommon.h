/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#pragma once

#include "stringex.h"


// This macro serves as a general way to determine the number of elements within an array.
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) int(sizeof(x)/sizeof(x[0]))
#endif


#if defined(_MSC_VER) && _MSC_VER < 1300
typedef unsigned MemValueType;
#else
typedef unsigned long long MemValueType;
#endif
