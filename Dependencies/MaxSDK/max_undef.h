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

// Force included after Precompiled/BasePrecomp.h into every target that uses the Max SDK,
// to undo macros from it that must already be undone when the first header includes Windows.h.
// The macros that clash with identifiers in the Max SDK headers are handled by max_sdk.h.

#pragma once

// The Max SDK expects Windows.h in full glory
#undef WIN32_LEAN_AND_MEAN
