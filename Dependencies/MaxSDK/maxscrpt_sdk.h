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

// Include this instead of <maxscrpt.h>. maxscrpt.h includes max.h, so it hides and restores
// the same macros as max_sdk.h, and like max_sdk.h it has no include guard.

#pragma push_macro("override")
#undef override

#pragma push_macro("PI")
#undef PI

#include <maxscrpt.h>

#pragma pop_macro("PI")
#pragma pop_macro("override")
