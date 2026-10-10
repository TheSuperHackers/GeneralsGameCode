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

#include "stringex_memcchr_common.h"

#if defined(_MSC_VER) && defined(_M_IX86)
// Internal linkage keeps this memcchr distinct from the other implementations linked into the test.
namespace
{
#include "Utility/stringex_memcchr_x86asm.inl"
}

TEST(StringEx, MemcchrX86Asm)
{
	testMemcchr(memcchr);
}
#endif
