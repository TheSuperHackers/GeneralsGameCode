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

#if defined(RTS_PROFILE_LEGACY)
#include "../../Source/profile/profile.h"
#endif

#if defined(RTS_PROFILE_TRACY)

#include <cstdlib>
#include <tracy/Tracy.hpp>

namespace rts {
// Diagnostic switch for measuring zone overhead. Frame marks, rate plots,
// images, and messages stay enabled. Read once on the first scoped event.
inline bool Profile_Detailed_Scopes_Enabled() noexcept
{
    static const bool enabled = [] {
        const char* value = std::getenv("RTS_TRACE_DETAIL");
        return value == nullptr || value[0] != '0' || value[1] != '\0';
    }();
    return enabled;
}
}

#define PROFILER_ENABLED
#define PROFILER_FRAME_IMAGE_SIZE 256 // Horizontal size of the frame image in pixels.
#define PROFILER_FRAME_IMAGE_INTERVAL_MS 500 // Will capture every render frame if set to 0
#define PROFILER_SECTION ZoneNamed(___tracy_scoped_zone,::rts::Profile_Detailed_Scopes_Enabled())
#define PROFILER_SECTION_NAME(name) ZoneNamedN(___tracy_scoped_zone,name,::rts::Profile_Detailed_Scopes_Enabled())
#define PROFILER_SECTION_COLOR(color) ZoneNamedC(___tracy_scoped_zone,color,::rts::Profile_Detailed_Scopes_Enabled())
#define PROFILER_SECTION_NAMECOLOR(name, color) ZoneNamedNC(___tracy_scoped_zone,name,color,::rts::Profile_Detailed_Scopes_Enabled())
#define PROFILER_FRAME_MARK FrameMark
#define PROFILER_FRAME_MARK_NAME(name) FrameMarkNamed(name)
#define PROFILER_FRAME_IMAGE(image, width, height, offset, flip) FrameImage(image, width, height, offset, flip)
#define PROFILER_MSG(txt, size) TracyMessage(txt, size)
#define PROFILER_PLOT(name, value) TracyPlot(name, value)
#define PROFILER_IS_CONNECTED TracyIsConnected

#else

#define PROFILER_FRAME_IMAGE_SIZE 0
#define PROFILER_FRAME_IMAGE_INTERVAL_MS 0
#define PROFILER_SECTION ((void)0)
#define PROFILER_SECTION_NAME(name) ((void)0)
#define PROFILER_SECTION_COLOR(color) ((void)0)
#define PROFILER_SECTION_NAMECOLOR(name, color) ((void)0)
#define PROFILER_FRAME_MARK ((void)0)
#define PROFILER_FRAME_MARK_NAME(name) ((void)0)
#define PROFILER_FRAME_IMAGE(image, width, height, offset, flip) ((void)0)
#define PROFILER_MSG(txt, size) ((void)0)
#define PROFILER_PLOT(name, value) ((void)0)
#define PROFILER_IS_CONNECTED false

#endif
