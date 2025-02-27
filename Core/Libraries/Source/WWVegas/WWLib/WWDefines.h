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

// Enable translation and rotation interpolation for raw animation (HRawAnimClass) updates.
// This was intentionally disabled in the retail version, but likely not fully thought through.
// Interpolation is certainly desired for animations that move and rotate meshes, but may not be
// desired for animations that teleport meshes from one location to another, such as blinking lights.
// @todo Implement a new flag per animation file to opt-out of interpolation.
#ifndef WW3D_ENABLE_RAW_ANIM_INTERPOLATION
#define WW3D_ENABLE_RAW_ANIM_INTERPOLATION (1)
#endif
