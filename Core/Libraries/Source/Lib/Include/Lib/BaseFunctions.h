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

#pragma once

#include <Utility/bit_adapter.h>
#include "BaseDefines.h"

#if USE_DETERMINISTIC_MATH
#include <gmath.h>
#endif

//-----------------------------------------------------------------------------
inline Real rad2deg(Real rad) { return rad * (180/PI); }
inline Real deg2rad(Real rad) { return rad * (PI/180); }

//-------------------------------------------------------------------------------------------------

// note, this function depends on the cpu rounding mode, which we set to CHOP every frame,
// but apparently tends to be left in unpredictable modes by various system bits of
// code, so use this function with caution -- it might not round in the way you want.
__forceinline long fast_float2long_round(float f)
{
	long i;

#if defined(_MSC_VER) && _MSC_VER < 1300
	__asm {
		fld [f]
		fistp [i]
	}
#else
	i = lroundf(f);
#endif

	return i;
}

#if RTS_ZEROHOUR && RETAIL_COMPATIBLE_CRC
// super fast float trunc routine, works always (independent of any FPU modes)
// code courtesy of Martin Hoffesommer (grin)
// TheSuperHackers @info The shift count wraps at 32, so magnitudes of 2^32 and larger lose
// bits that are not fractional, and NaN becomes infinity. Retail Zero Hour depends on these results.
__forceinline float fast_float_trunc(float f)
{
#if defined(_MSC_VER) && _MSC_VER < 1300
  __asm
  {
    mov ecx,[f]
    shr ecx,23
    mov eax,0xff800000
    xor ebx,ebx
    sub cl,127
    cmovc eax,ebx
    sar eax,cl
    and [f],eax
  }
  return f;
#else
  unsigned x = std::bit_cast<unsigned>(f);
  unsigned char exp = x >> 23;
  int mask = exp < 127 ? 0 : 0xff800000;
  exp -= 127;
  mask >>= exp & 31;
  x &= mask;
  return std::bit_cast<float>(x);
#endif
}

// same here, fast floor function
// TheSuperHackers @info Subtracting almost 1 only floors correctly if the FPU rounds toward zero, but the game
// rounds to nearest (see setFPMode). So negative whole numbers above -2^24 return n-1, for example -2 returns -3.
// Negative values closer to zero than about 3e-8 also return 0 instead of -1.
// Retail Zero Hour depends on these results.
__forceinline float fast_float_floor(float f)
{
  static unsigned almost1=(126<<23)|0x7fffff;
  if (std::bit_cast<unsigned>(f) &0x80000000)
    f-=std::bit_cast<float>(almost1);
  return fast_float_trunc(f);
}

// same here, fast ceil function
// TheSuperHackers @info Adding almost 1 only ceils correctly if the FPU rounds toward zero, but the game
// rounds to nearest (see setFPMode). So positive whole numbers below 2^24 return n+1, for example 2 returns 3.
// Positive values closer to zero than about 3e-8 also return 0 instead of 1.
// Retail Zero Hour depends on these results.
__forceinline float fast_float_ceil(float f)
{
  static unsigned almost1=(126<<23)|0x7fffff;
  if ( (std::bit_cast<unsigned>(f) &0x80000000)==0)
    f+=std::bit_cast<float>(almost1);
  return fast_float_trunc(f);
}
#endif // RTS_ZEROHOUR && RETAIL_COMPATIBLE_CRC

#if RTS_ZEROHOUR && RETAIL_COMPATIBLE_CRC
	// once we've ceiled/floored, trunc and round are identical, and currently, round is faster... (srj)
	#define REAL_TO_INT_CEIL(x) (fast_float2long_round(fast_float_ceil(x)))
	#define REAL_TO_INT_FLOOR(x) (fast_float2long_round(fast_float_floor(x)))
#else
	#if USE_DETERMINISTIC_MATH
		#define REAL_TO_INT_CEIL(x) (fast_float2long_round(gm_ceilf(x)))
		#define REAL_TO_INT_FLOOR(x) (fast_float2long_round(gm_floorf(x)))
	#else
		#define REAL_TO_INT_CEIL(x) (fast_float2long_round(ceilf(x)))
		#define REAL_TO_INT_FLOOR(x) (fast_float2long_round(floorf(x)))
	#endif
#endif

//-------------------------------------------------------------------------------------------------
// Checking whether a size matches the bit flags count from an enum definition:
// enum MyEnum
// {
//   MyEnumValue1 = 1<<0,
//   MyEnumValue2 = 1<<1,
//   MyEnumValue3 = 1<<2,
//   MyEnumEnd
// }
// static_assert(equalsEnumBitFlagsCount(3, MyEnumEnd), "Incorrect size");
//
template <typename EnumType>
constexpr inline Bool equalsEnumBitFlagsCount(size_t expectedSize, EnumType bitFlagsEnd)
{
	return 1 << (expectedSize - 1) == static_cast<size_t>(bitFlagsEnd) - 1;
}
