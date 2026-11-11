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

// Include this instead of <max.h>. It hides the macros that clash with identifiers in the
// Max SDK headers while they are read, and restores them afterwards, because the headers of
// the project that are read later still need them.
//
// It has no include guard, because max.h has none either: every time max.h is read, it
// defines assert through assert1.h again, as an assert that also checks in release builds.

// The Max SDK predates C++11 and uses 'override' as a function name, while headers that use
// the VC6 shim for 'override' need it to be defined.
#pragma push_macro("override")
#undef override

// The Max SDK defines its own PI. Both are the same float value.
#pragma push_macro("PI")
#undef PI

#include <max.h>

#pragma pop_macro("PI")
#pragma pop_macro("override")
