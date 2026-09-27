/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

// This file includes the Uniscribe API of usp10.h. It uses the header of the Windows SDK if the
// compiler can find it, and otherwise a subset of it for compilers that do not have it, such as VC6.
// The functions are implemented by Usp10Loader.cpp.

#pragma once

#if defined(__has_include)
	#if __has_include(<usp10.h>)
		#define HAVE_USP10_H 1
	#endif
#endif

#ifdef HAVE_USP10_H
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <usp10.h>
#else
	#include "usp10_subset.h"
#endif
