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

// super fast float trunc routine, works always (independent of any FPU modes)
// code courtesy of Martin Hoffesommer (grin)
__forceinline float fast_float_trunc(float f)
{
#if defined(_MSC_VER) && _MSC_VER < 1300
  _asm
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
  unsigned x = *(unsigned *)&f;
  unsigned char exp = x >> 23;
  int mask = exp < 127 ? 0 : 0xff800000;
  exp -= 127;
  mask >>= exp & 31;
  x &= mask;
  return *(float *)&x;
#endif
}

// same here, fast floor function
__forceinline float fast_float_floor(float f)
{
  static unsigned almost1=(126<<23)|0x7fffff;
  if (*(unsigned *)&f &0x80000000)
    f-=*(float *)&almost1;
  return fast_float_trunc(f);
}

// same here, fast ceil function
__forceinline float fast_float_ceil(float f)
{
  static unsigned almost1=(126<<23)|0x7fffff;
  if ( (*(unsigned *)&f &0x80000000)==0)
    f+=*(float *)&almost1;
  return fast_float_trunc(f);
}

// once we've ceiled/floored, trunc and round are identical, and currently, round is faster... (srj)
#if RTS_GENERALS /*&& RETAIL_COMPATIBLE_CRC*/
#define REAL_TO_INT_CEIL(x)				(fast_float2long_round(ceilf(x)))
#define REAL_TO_INT_FLOOR(x)			(fast_float2long_round(floorf(x)))
#else
#define REAL_TO_INT_CEIL(x)				(fast_float2long_round(fast_float_ceil(x)))
#define REAL_TO_INT_FLOOR(x)			(fast_float2long_round(fast_float_floor(x)))
#endif

#define FAST_REAL_TRUNC(x)        fast_float_trunc(x)
#define FAST_REAL_CEIL(x)         fast_float_ceil(x)
#define FAST_REAL_FLOOR(x)        fast_float_floor(x)
