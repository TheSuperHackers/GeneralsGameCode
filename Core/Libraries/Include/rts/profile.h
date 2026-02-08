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

/////////////////////////////////////////////////////////////////////////EA-V1
// $File: //depot/GeneralsMD/Staging/code/Libraries/Include/rts/profile.h $
// $Author: mhoffe $
// $Revision: #1 $
// $DateTime: 2003/07/03 11:55:26 $
//
// (c) 2003 Electronic Arts
//
// Proxy header for profile module
//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#define TRACY_FRAMEIMAGE_SIZE 256
#else
#include "../../Source/profile/profile.h"
#define ZoneScopedN(name) ((void)0)
#define ZoneScopedNC(name, color) ((void)0)
#define TracyPlot(name, value) ((void)0)
#define FrameMark ((void)0)
#define FrameMarkNamed(name) ((void)0)
#define FrameImage(image, width, height, offset, flip) ((void)0)
#define TracyMessage(txt, size) ((void)0)
#define TracyIsConnected false
#endif
