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

#include "WWDefines.h"

// Note: Retail compatibility must not be broken before this project officially does.
// Use RETAIL_COMPATIBLE_CRC and RETAIL_COMPATIBLE_XFER_SAVE to guard breaking changes.

#ifndef PRESERVE_RETAIL_BEHAVIOR
#define PRESERVE_RETAIL_BEHAVIOR (1) // Retain behavior present in retail Generals 1.08 and Zero Hour 1.04
#endif

#ifndef RETAIL_COMPATIBLE_CRC
#define RETAIL_COMPATIBLE_CRC (1) // Game is expected to be CRC compatible with retail Generals 1.08, Zero Hour 1.04
#endif

#ifndef RETAIL_COMPATIBLE_XFER_SAVE
#define RETAIL_COMPATIBLE_XFER_SAVE (1) // Game is expected to be Xfer Save compatible with retail Generals 1.08, Zero Hour 1.04
#endif

// This is here to easily toggle between the retail compatible with fixed pathfinding fallback and pure fixed pathfinding mode
#if RETAIL_COMPATIBLE_CRC
#define RETAIL_COMPATIBLE_PATHFINDING (1)
#else
#define RETAIL_COMPATIBLE_PATHFINDING (0)
#endif

// This is essentially synonymous for RETAIL_COMPATIBLE_CRC. There is a lot wrong with AIGroup, such as use-after-free, double-free, leaks,
// but we cannot touch it much without breaking retail compatibility. Do not shy away from using massive hacks when fixing issues with AIGroup,
// but put them behind this macro.

#ifndef RETAIL_COMPATIBLE_AIGROUP
#define RETAIL_COMPATIBLE_AIGROUP (1) // AIGroup logic is expected to be CRC compatible with retail Generals 1.08, Zero Hour 1.04
#endif

#ifndef ENABLE_GAMETEXT_SUBSTITUTES
#define ENABLE_GAMETEXT_SUBSTITUTES (1) // The code can provide substitute texts when labels and strings are missing in the STR or CSF translation file
#endif

// Previously the configurable shroud sat behind #if defined(RTS_DEBUG)
// Enable the configurable shroud to properly draw the terrain in World Builder without RTS_DEBUG compiled in.
// Disable the configurable shroud to make shroud hacking a bit less accessible in Release game builds.
#ifndef ENABLE_CONFIGURABLE_SHROUD
#define ENABLE_CONFIGURABLE_SHROUD (1) // When enabled, the GlobalData contains a field to turn on/off the shroud, otherwise shroud is always enabled
#endif

// Enable buffered IO in File System. Was disabled in retail game.
// Buffered IO generally is much faster than unbuffered for small reads and writes.
#ifndef USE_BUFFERED_IO
#define USE_BUFFERED_IO (1)
#endif

// Enable cache for local file existence. Reduces amount of disk accesses for better performance,
// but decreases file existence correctness and runtime stability, if a cached file is deleted on runtime.
#ifndef ENABLE_FILESYSTEM_EXISTENCE_CACHE
#define ENABLE_FILESYSTEM_EXISTENCE_CACHE (1)
#endif

// Enable prioritization of textures by size. This will improve the texture quality of 481 textures in Zero Hour
// by using the larger resolution textures from Generals. Content wise these textures are identical.
#ifndef PRIORITIZE_TEXTURES_BY_SIZE
#define PRIORITIZE_TEXTURES_BY_SIZE (1)
#endif

// Enable obsolete code. This mainly refers to code that existed in Generals but was removed in GeneralsMD.
// Disable and remove this when Generals and GeneralsMD are merged.
#if RTS_GENERALS
#ifndef USE_OBSOLETE_GENERALS_CODE
#define USE_OBSOLETE_GENERALS_CODE (1)
#endif
#endif

// Overwrite window settings until wnd data files are adapted or fixed.
#ifndef ENABLE_GUI_HACKS
#define ENABLE_GUI_HACKS (1)
#endif

// Tell our computer identity in the LAN lobby. Disable for privacy.
// Was enabled in the retail game and exposed the computer login and host names.
#ifdef RTS_DEBUG
#ifndef TELL_COMPUTER_IDENTITY_IN_LAN_LOBBY
#define TELL_COMPUTER_IDENTITY_IN_LAN_LOBBY (1)
#endif
#endif

#define MIN_DISPLAY_BIT_DEPTH       16
#define DEFAULT_DISPLAY_BIT_DEPTH   32
#define DEFAULT_DISPLAY_WIDTH      800 // The standard resolution this game was designed for
#define DEFAULT_DISPLAY_HEIGHT     600 // The standard resolution this game was designed for
