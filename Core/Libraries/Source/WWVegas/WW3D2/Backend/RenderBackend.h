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

// TheSuperHackers @refactor bobtista 10/04/2026 Backend selection seam. The
// build links exactly one backend implementation, and that implementation
// defines Create_Render_Backend. WW3D owns the instance it returns; use
// WW3D::Get_Render_Backend() to reach the active backend.

#pragma once

class IRenderBackend;

// Construct and initialize the backend selected by the build. Exactly one
// backend implementation must define this function. Returns null on failure.
IRenderBackend* Create_Render_Backend(void* window, bool lite);
