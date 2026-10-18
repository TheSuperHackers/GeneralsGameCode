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

#pragma once

// Multiplies the color of each A8R8G8B8 pixel by its alpha, rounding to nearest. Alpha is kept.
void premultiply_alpha(unsigned char* pixels, int pitch, int width, int height);

// Premultiply alpha. Multiple implementations: 1) VC6-compatible in x86 ASM; 2) SSE2 intrinsics; 3) portable.
#if defined(_MSC_VER) && _MSC_VER < 1300
  #include "premultiply_alpha_x86asm.inl"
#else
  #if defined(__SSE2__) || defined(_M_X64) || (defined(_MSC_VER) && defined(_M_IX86))
    #include <emmintrin.h>
    #include "premultiply_alpha_sse2.inl"
  #else
    #include "premultiply_alpha.inl"
  #endif
#endif
