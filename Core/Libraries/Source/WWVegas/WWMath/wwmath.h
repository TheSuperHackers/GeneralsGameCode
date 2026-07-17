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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/wwmath.h                              $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/26/01 2:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 64                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include <math.h>
#include <float.h>
#include <assert.h>
#include "Lib/BaseDefines.h"

#if USE_DETERMINISTIC_MATH
#include "gmath.h"
#endif

/*
** Some global constants.
*/
#define WWMATH_EPSILON		0.0001f
#define WWMATH_EPSILON2		WWMATH_EPSILON * WWMATH_EPSILON
#define WWMATH_HALF_PI		1.570796327f
#define WWMATH_PI					3.141592654f
#define WWMATH_TWO_PI			6.283185308f
#define WWMATH_FLOAT_MAX	(FLT_MAX)
#define WWMATH_FLOAT_MIN	(FLT_MIN)
#define WWMATH_SQRT2			1.414213562f
#define WWMATH_SQRT3			1.732050808f
#define WWMATH_OOSQRT2		0.707106781f
#define WWMATH_OOSQRT3		0.577350269f

/*
**	Macros to convert between degrees and radians
*/
#ifndef RAD_TO_DEG
#define RAD_TO_DEG(x)	(((double)x)*180.0/WWMATH_PI)
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD(x)	(((double)x)*WWMATH_PI/180.0)
#endif

#ifndef RAD_TO_DEGF
#define RAD_TO_DEGF(x)	(((float)x)*180.0f/WWMATH_PI)
#endif

#ifndef DEG_TO_RADF
#define DEG_TO_RADF(x)	(((float)x)*WWMATH_PI/180.0f)
#endif

const int ARC_TABLE_SIZE=1024;
const int SIN_TABLE_SIZE=1024;
extern float _FastAcosTable[ARC_TABLE_SIZE];
extern float _FastAsinTable[ARC_TABLE_SIZE];
extern float _FastSinTable[SIN_TABLE_SIZE];
extern float _FastInvSinTable[SIN_TABLE_SIZE];

/*
** Some simple math functions which work on the built-in types.
** Include the various other header files in the WWMATH library
** in order to get matrices, quaternions, etc.
*/
// TheSuperHackers @todo The Legacy functions can be removed when retail compatibility is abandoned.
class WWMath
{
public:

// Initialization and Shutdown.  Other math sub-systems which require initialization and
// shutdown processing will be handled in these functions
static void			Init();
static void			Shutdown();

static WWINLINE double Pow(double x, double y);
static WWINLINE float  Powf(float x, float y);
static WWINLINE double Sqr(float x);
static WWINLINE float  Sqrt_Legacy(float val);
static WWINLINE float  Sqrt(float x);
	static WWINLINE float  Sqrt(int x);
	static WWINLINE double Sqrt(double x);
static WWINLINE float  Sqrtf(float x);
static WWINLINE float  Inv_Sqrt_Legacy(float a);
static WWINLINE double Inv_Sqrt(double x);
static WWINLINE float  Inv_Sqrtf(float x);

static WWINLINE float  Fast_Acos(float val);
static WWINLINE float  Fast_Asin(float val);
static WWINLINE float  Acos_Legacy(float val);
static WWINLINE float  Acos(float x);
	static WWINLINE double Acos(double x);
static WWINLINE float  Acosf(float x);
static WWINLINE float  Asin_Legacy(float val);
static WWINLINE float  Asin(float x);
	static WWINLINE double Asin(double x);
static WWINLINE float  Asinf(float x);
static WWINLINE float  Atan_Legacy(float x);
static WWINLINE float  Atan(float x);
	static WWINLINE double Atan(double x);
static WWINLINE float  Atanf(float x);
static WWINLINE float  Atan2_Legacy(float x, float y);
static WWINLINE float  Atan2(float x, float y);
	static WWINLINE double Atan2(double x, double y);
static WWINLINE float  Atan2f(float x, float y);

static WWINLINE float  Fast_Cos(float val);
static WWINLINE float  Fast_Inv_Cos(float val);
static WWINLINE float  Fast_Sin(float val);
static WWINLINE float  Fast_Inv_Sin(float val);
static WWINLINE float  Cos(float val);
	static WWINLINE double Cos(double val);
static WWINLINE float  Cosf(float val);
static WWINLINE float  Cosf_Legacy(float val);
// Prevent automatic compiler promotion of float arguments to double-precision variants.
	// Single-precision math in GameMath (gm_*f) is guaranteed to be cross-platform bit-identical,
	// whereas double-precision math (gm_*) can diverge by 1 ULP due to FPU precision differences (x87 vs NEON).
	static WWINLINE float  Sin(float val);
	static WWINLINE double Sin(double val);
static WWINLINE float  Sinf(float val);
static WWINLINE float  Sinf_Legacy(float val);
static WWINLINE float  Tan(float x);
	static WWINLINE double Tan(double x);
static WWINLINE float  Tanf(float x);

static WWINLINE double Cosh(double x);
static WWINLINE float  Coshf(float x);
static WWINLINE double Sinh(double x);
static WWINLINE float  Sinhf(float x);
static WWINLINE double Tanh(double x);
static WWINLINE float  Tanhf(float x);

static WWINLINE float  Fabs(float x);
	static WWINLINE double Fabs(double x);
static WWINLINE float  Fabsf(float x);
static WWINLINE float  Fabsf_Legacy(float val);

static WWINLINE double Ceil(double x);
static WWINLINE float  Ceilf(float x);
static WWINLINE double Floor(double x);
static WWINLINE float  Floorf(float x);
static WWINLINE double Round(double x) { return Floor(x + 0.5); }
static WWINLINE float  Roundf(float x) { return Floorf(x + 0.5f); }

static WWINLINE double Exp(double x);
static WWINLINE float  Expf(float x);
static WWINLINE double Log10(double x);
static WWINLINE float  Log10f(float x);
static WWINLINE double Log(double x);
static WWINLINE float  Logf(float x);

static WWINLINE bool		Fast_Is_Float_Positive(const float & val);
static WWINLINE bool		Is_Power_Of_2(const unsigned int val);

static float		Random_Float();

static WWINLINE float		Random_Float(float min,float max);
static WWINLINE float		Clamp(float val, float min = 0.0f, float max = 1.0f);
static WWINLINE double	Clamp(double val, double min = 0.0f, double max = 1.0f);
static WWINLINE int			Clamp_Int(int val, int min_val, int max_val);
static WWINLINE float		Wrap(float val, float min = 0.0f, float max = 1.0f);
static WWINLINE double	Wrap(double val, double min = 0.0f, double max = 1.0f);
static WWINLINE float		Min(float a, float b);
static WWINLINE float		Max(float a, float b);

// Linearly interpolates between a and b using parameter t in [0, 1].
// t = 0 returns a, t = 1 returns b, values in between return a proportionate blend.
static WWINLINE float		Lerp(float a, float b, float t);
static WWINLINE double	Lerp(double a, double b, float t);

// Computes the interpolation parameter t such that v = Lerp(a, b, t).
// Returns where v lies between a and b as a ratio, typically in [0, 1].
static WWINLINE float		Inverse_Lerp(float a, float b, float v);
static WWINLINE double	Inverse_Lerp(double a, double b, float v);

static WWINLINE bool			Is_Valid_Float(float x);
static WWINLINE bool			Is_Valid_Double(double x);

static WWINLINE int Float_To_Int_Chop(float f);
static WWINLINE int Float_To_Int_Floor(float f);
static WWINLINE long Float_To_Long(float f);
static WWINLINE long Float_To_Long(double f);
static WWINLINE int Float_As_Int(const float f) { return *((int*)&f); }
static WWINLINE unsigned char Unit_Float_To_Byte(float f) { return (unsigned char)(f*255.0f); }
static WWINLINE float Byte_To_Unit_Float(unsigned char byte) { return ((float)byte) / 255.0f; }

static WWINLINE float Normalize_Angle(float angle); // Normalizes the angle to the range -PI..PI

static WWINLINE float Div_Safe(float dividend, float divisor, float fallback = 0.0f);
static WWINLINE double Div_Safe(double dividend, double divisor, double fallback = 0.0);

};

WWINLINE double WWMath::Pow(double x, double y)
{
#if USE_DETERMINISTIC_MATH
	return gm_pow(x, y);
#else
	return pow(x, y);
#endif
}

WWINLINE double WWMath::Sqr(float x)
{
#if USE_DETERMINISTIC_MATH
	return (double)(x * x);
#else
	return Pow((double)x, 2.0);
#endif
}

WWINLINE float WWMath::Powf(float x, float y)
{
#if USE_DETERMINISTIC_MATH
	return gm_powf(x, y);
#else
	return powf(x, y);
#endif
}

WWINLINE float WWMath::Sqrt_Legacy(float val)
{
#if USE_DETERMINISTIC_MATH
	return gm_sqrtf(val);

#elif defined(_MSC_VER) && defined(_M_IX86)
	float retval;
	__asm {
		fld [val]
		fsqrt
		fstp [retval]
	}
	return retval;

#else
	return (float)sqrt((double)val);
#endif
}

WWINLINE float WWMath::Sqrt(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_sqrtf(x);
#else
	return (float)Sqrt((double)x);
#endif
}

WWINLINE float WWMath::Sqrt(int x)
{
#if USE_DETERMINISTIC_MATH
	return gm_sqrtf((float)x);
#else
	return (float)Sqrt((double)x);
#endif
}

WWINLINE double WWMath::Sqrt(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_sqrt(x);
#else
	return sqrt(x);
#endif
}

WWINLINE float WWMath::Sqrtf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_sqrtf(x);
#else
	return sqrtf(x);
#endif
}

WWINLINE float WWMath::Inv_Sqrt_Legacy(float a)
{
#if USE_DETERMINISTIC_MATH
	return 1.0f / gm_sqrtf(a);

#elif defined(_MSC_VER) && defined(_M_IX86)
	// Some 30% faster inverse square root than regular C++ compiled, from Intel's math library
	float retval;

	__asm {
		mov		eax, 0be6eb508h
		mov		DWORD PTR [esp-12],03fc00000h ;  1.5 on the stack
		sub		eax, DWORD PTR [a]; a
		sub		DWORD PTR [a], 800000h ; a/2 a=Y0
		shr		eax, 1     ; firs approx in eax=R0
		mov		DWORD PTR [esp-8], eax

		fld		DWORD PTR [esp-8] ;r
		fmul	st, st            ;r*r
		fld		DWORD PTR [esp-8] ;r
		fxch	st(1)
		fmul	DWORD PTR [a];a ;r*r*y0
		fld		DWORD PTR [esp-12];load 1.5
		fld		st(0)
		fsub	st,st(2)			   ;r1 = 1.5 - y1
		;x1 = st(3)
		;y1 = st(2)
		;1.5 = st(1)
		;r1 = st(0)

		fld		st(1)
		fxch	st(1)
		fmul	st(3),st			; y2=y1*r1*...
		fmul	st(3),st			; y2=y1*r1*r1
		fmulp	st(4),st            ; x2=x1*r1
		fsub	st,st(2)               ; r2=1.5-y2
		;x2=st(3)
		;y2=st(2)
		;1.5=st(1)
		;r2 = st(0)

		fmul	st(2),st			;y3=y2*r2*...
		fmul	st(3),st			;x3=x2*r2
		fmulp	st(2),st			;y3=y2*r2*r2
		fxch	st(1)
		fsubp	st(1),st			;r3= 1.5 - y3
		;x3 = st(1)
		;r3 = st(0)
		fmulp	st(1), st

		fstp retval
	}

	return retval;

#else
	return 1.0f / (float)sqrt((double)a);
#endif
}

WWINLINE double WWMath::Inv_Sqrt(double x)
{
	return 1.0 / Sqrt(x);
}

WWINLINE float WWMath::Inv_Sqrtf(float x)
{
	return 1.0f / Sqrtf(x);
}

WWINLINE float WWMath::Fast_Acos(float val)
{
	// Near -1 and +1, the table becomes too inaccurate
	if (Fabsf_Legacy(val) > 0.975f) {
		return Acos_Legacy(val);
	}

	val*=float(ARC_TABLE_SIZE/2);

	int idx0=Float_To_Int_Floor(val);
	int idx1=idx0+1;
	float frac=val-(float)idx0;

	idx0+=ARC_TABLE_SIZE/2;
	idx1+=ARC_TABLE_SIZE/2;

	// we dont even get close to the edge of the table...
	assert((idx0 >= 0) && (idx0 < ARC_TABLE_SIZE));
	assert((idx1 >= 0) && (idx1 < ARC_TABLE_SIZE));

	// compute and return the interpolated value
	return (1.0f - frac) * _FastAcosTable[idx0] + frac * _FastAcosTable[idx1];
}

WWINLINE float WWMath::Fast_Asin(float val)
{
	// Near -1 and +1, the table becomes too inaccurate
	if (Fabsf_Legacy(val) > 0.975f) {
		return Asin_Legacy(val);
	}

	val*=float(ARC_TABLE_SIZE/2);

	int idx0=Float_To_Int_Floor(val);
	int idx1=idx0+1;
	float frac=val-(float)idx0;

	idx0+=ARC_TABLE_SIZE/2;
	idx1+=ARC_TABLE_SIZE/2;

	// we dont even get close to the edge of the table...
	assert((idx0 >= 0) && (idx0 < ARC_TABLE_SIZE));
	assert((idx1 >= 0) && (idx1 < ARC_TABLE_SIZE));

	// compute and return the interpolated value
	return (1.0f - frac) * _FastAsinTable[idx0] + frac * _FastAsinTable[idx1];
}

WWINLINE float WWMath::Acos_Legacy(float val)
{
#if USE_DETERMINISTIC_MATH
	return gm_acosf(val);
#else
	return (float)acos((double)val);
#endif
}

WWINLINE float WWMath::Acos(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_acosf(x);
#else
	return (float)Acos((double)x);
#endif
}

WWINLINE double WWMath::Acos(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_acos(x);
#else
	return acos(x);
#endif
}

WWINLINE float WWMath::Acosf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_acosf(x);
#else
	return acosf(x);
#endif
}

WWINLINE float WWMath::Asin_Legacy(float val)
{
#if USE_DETERMINISTIC_MATH
	return gm_asinf(val);
#else
	return (float)asin((double)val);
#endif
}

WWINLINE float WWMath::Asin(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_asinf(x);
#else
	return (float)Asin((double)x);
#endif
}

WWINLINE double WWMath::Asin(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_asin(x);
#else
	return asin(x);
#endif
}
WWINLINE float WWMath::Asinf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_asinf(x);
#else
	return asinf(x);
#endif
}

WWINLINE float WWMath::Atan_Legacy(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_atanf(x);
#else
	return (float)atan((double)x);
#endif
}

WWINLINE float WWMath::Atan(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_atanf(x);
#else
	return (float)Atan((double)x);
#endif
}

WWINLINE double WWMath::Atan(double x)
{
#if USE_DETERMINISTIC_MATH
	return (double) gm_atanf((float)x);
#else
	return atan(x);
#endif
}

WWINLINE float WWMath::Atanf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_atanf(x);
#else
	return atanf(x);
#endif
}

WWINLINE float WWMath::Atan2_Legacy(float x, float y)
{
#if USE_DETERMINISTIC_MATH
	return gm_atan2f(x, y);
#else
	return (float)atan2((double)x, (double)y);
#endif
}

WWINLINE float WWMath::Atan2(float x, float y)
{
#if USE_DETERMINISTIC_MATH
	return gm_atan2f(x, y);
#else
	return (float)Atan2((double)x, (double)y);
#endif
}

WWINLINE double WWMath::Atan2(double x, double y)
{
#if USE_DETERMINISTIC_MATH
	return (double) gm_atan2f((float)x, (float)y);
#else
	return atan2(x, y);
#endif
}

WWINLINE float WWMath::Atan2f(float x, float y)
{
#if USE_DETERMINISTIC_MATH
	return gm_atan2f(x, y);
#else
	return atan2f(x, y);
#endif
}

WWINLINE float WWMath::Fast_Cos(float val)
{
	val+=(WWMATH_PI * 0.5f);
	val*=float(SIN_TABLE_SIZE) / (2.0f * WWMATH_PI);

	int idx0=Float_To_Int_Floor(val);
	int idx1=idx0+1;
	float frac=val-(float)idx0;

	idx0 = ((unsigned)idx0) & (SIN_TABLE_SIZE-1);
	idx1 = ((unsigned)idx1) & (SIN_TABLE_SIZE-1);

	return (1.0f - frac) * _FastSinTable[idx0] + frac * _FastSinTable[idx1];
}

WWINLINE float WWMath::Fast_Inv_Cos(float val)
{
#if 0 // TODO: more testing, not reliable!
	float index = val + (WWMATH_PI * 0.5f);
	index *= float(SIN_TABLE_SIZE) / (2.0f * WWMATH_PI);

	int idx0=Float_To_Int_Chop(index);
	int idx1=idx0+1;
	float frac=val-(float)idx0;

	idx0 = ((unsigned)idx0) & (SIN_TABLE_SIZE-1);
	idx1 = ((unsigned)idx1) & (SIN_TABLE_SIZE-1);

	// The table becomes inaccurate near 0 and 2pi so fall back to doing a divide.
	if ((idx0 <= 2) || (idx0 >= SIN_TABLE_SIZE-3)) {
		return 1.0f / Fast_Cos(val);
	} else {
		return (1.0f - frac) * _FastInvSinTable[idx0] + frac * _FastInvSinTable[idx1];
	}
#else
	return 1.0f / Fast_Cos(val);
#endif
}

WWINLINE float WWMath::Fast_Sin(float val)
{
	val*=float(SIN_TABLE_SIZE) / (2.0f * WWMATH_PI);

	int idx0=Float_To_Int_Floor(val);
	int idx1=idx0+1;
	float frac=val-(float)idx0;

	idx0 = ((unsigned)idx0) & (SIN_TABLE_SIZE-1);
	idx1 = ((unsigned)idx1) & (SIN_TABLE_SIZE-1);

	return (1.0f - frac) * _FastSinTable[idx0] + frac * _FastSinTable[idx1];
}

WWINLINE float WWMath::Fast_Inv_Sin(float val)
{
#if 0 // TODO: more testing, not reliable!
	float index = val * float(SIN_TABLE_SIZE) / (2.0f * WWMATH_PI);

	int idx0=Float_To_Int_Floor(index);
	int idx1=idx0+1;
	float frac=val-(float)idx0;

	idx0 = ((unsigned)idx0) & (SIN_TABLE_SIZE-1);
	idx1 = ((unsigned)idx1) & (SIN_TABLE_SIZE-1);

	// The table becomes inaccurate near 0 and 2pi so fall back to doing a divide.
	const int BUFFER = 16;
	if ((idx0 <= BUFFER) || (idx0 >= SIN_TABLE_SIZE-BUFFER-1)) {
		return 1.0f / Fast_Sin(val);
	} else {
		return (1.0f - frac) * _FastInvSinTable[idx0] + frac * _FastInvSinTable[idx1];
	}
#else
	return 1.0f / Fast_Sin(val);
#endif
}

WWINLINE float WWMath::Cos(float val)
{
#if USE_DETERMINISTIC_MATH
	return gm_cosf(val);
#else
	return (float)Cos((double)val);
#endif
}

WWINLINE double WWMath::Cos(double val)
{
#if USE_DETERMINISTIC_MATH
	return gm_cos(val);
#else
	return cos(val);
#endif
}

WWINLINE float WWMath::Cosf(float val)
{
#if USE_DETERMINISTIC_MATH
	return gm_cosf(val);
#else
	return cosf(val);
#endif
}

WWINLINE float WWMath::Cosf_Legacy(float val)
{
#if USE_DETERMINISTIC_MATH
	return gm_cosf(val);

#elif defined(_MSC_VER) && defined(_M_IX86)
	float retval;
	__asm {
		fld [val]
		fcos
		fstp [retval]
	}
	return retval;

#else
	return cosf(val);
#endif
}

WWINLINE float WWMath::Sin(float val)
{
#if USE_DETERMINISTIC_MATH
	return gm_sinf(val);
#else
	return (float)Sin((double)val);
#endif
}

WWINLINE double WWMath::Sin(double val)
{
#if USE_DETERMINISTIC_MATH
	return gm_sin(val);
#else
	return sin(val);
#endif
}

WWINLINE float WWMath::Sinf(float val)
{
#if USE_DETERMINISTIC_MATH
	return gm_sinf(val);
#else
	return sinf(val);
#endif
}

WWINLINE float WWMath::Sinf_Legacy(float val)
{
#if USE_DETERMINISTIC_MATH
	return gm_sinf(val);

#elif defined(_MSC_VER) && defined(_M_IX86)
	float retval;
	__asm {
		fld [val]
		fsin
		fstp [retval]
	}
	return retval;

#else
	return sinf(val);
#endif
}

WWINLINE float WWMath::Tan(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_tanf(x);
#else
	return (float)Tan((double)x);
#endif
}

WWINLINE double WWMath::Tan(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_tan(x);
#else
	return tan(x);
#endif
}

WWINLINE float WWMath::Tanf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_tanf(x);
#else
	return tanf(x);
#endif
}

WWINLINE double WWMath::Cosh(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_cosh(x);
#else
	return cosh(x);
#endif
}

WWINLINE float WWMath::Coshf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_coshf(x);
#else
	return coshf(x);
#endif
}

WWINLINE double WWMath::Sinh(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_sinh(x);
#else
	return sinh(x);
#endif
}

WWINLINE float WWMath::Sinhf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_sinhf(x);
#else
	return sinhf(x);
#endif
}

WWINLINE double WWMath::Tanh(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_tanh(x);
#else
	return tanh(x);
#endif
}

WWINLINE float WWMath::Tanhf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_tanhf(x);
#else
	return tanhf(x);
#endif
}

WWINLINE float WWMath::Fabs(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_fabsf(x);
#else
	return (float)Fabs((double)x);
#endif
}

WWINLINE double WWMath::Fabs(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_fabs(x);
#else
	return fabs(x);
#endif
}

WWINLINE float WWMath::Fabsf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_fabsf(x);
#else
	return fabsf(x);
#endif
}

WWINLINE float WWMath::Fabsf_Legacy(float val)
{
#if USE_DETERMINISTIC_MATH
	return gm_fabsf(val);

#elif defined(_MSC_VER) && defined(_M_IX86)
	int value=*(int*)&val;
	value&=0x7fffffff;
	return *(float*)&value;

#else
	return fabsf(val);
#endif
}

WWINLINE double WWMath::Ceil(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_ceil(x);
#else
	return ceil(x);
#endif
}

WWINLINE float WWMath::Ceilf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_ceilf(x);
#else
	return ceilf(x);
#endif
}

WWINLINE double WWMath::Floor(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_floor(x);
#else
	return floor(x);
#endif
}

WWINLINE float WWMath::Floorf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_floorf(x);
#else
	return floorf(x);
#endif
}

WWINLINE double WWMath::Exp(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_exp(x);
#else
	return exp(x);
#endif
}

WWINLINE float WWMath::Expf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_expf(x);
#else
	return expf(x);
#endif
}

WWINLINE double WWMath::Log10(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_log10(x);
#else
	return log10(x);
#endif
}

WWINLINE float WWMath::Log10f(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_log10f(x);
#else
	return log10f(x);
#endif
}

WWINLINE double WWMath::Log(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_log(x);
#else
	return log(x);
#endif
}

WWINLINE float WWMath::Logf(float x)
{
#if USE_DETERMINISTIC_MATH
	return gm_logf(x);
#else
	return logf(x);
#endif
}

WWINLINE bool WWMath::Fast_Is_Float_Positive(const float & val)
{
	return !((*(int *)(&val)) & 0x80000000);
}

WWINLINE bool WWMath::Is_Power_Of_2(const unsigned int val)
{
	return !((val)&val-1);
}

WWINLINE float WWMath::Random_Float(float min,float max)
{
	return Random_Float() * (max-min) + min;
}

WWINLINE float WWMath::Clamp(float val, float min /*= 0.0f*/, float max /*= 1.0f*/)
{
	if(val < min) return min;
	if(val > max) return max;
	return val;
}

WWINLINE double WWMath::Clamp(double val, double min /*= 0.0f*/, double max /*= 1.0f*/)
{
	if(val < min) return min;
	if(val > max) return max;
	return val;
}

WWINLINE int WWMath::Clamp_Int(int val, int min_val, int max_val)
{
	if(val < min_val) return min_val;
	if(val > max_val) return max_val;
	return val;
}

WWINLINE float WWMath::Wrap(float val, float min /*= 0.0f*/, float max /*= 1.0f*/)
{
	// Implemented as an if rather than a while, to long loops
	if ( val >= max )	val -= (max-min);
	if ( val < min )	val += (max-min);

	if ( val < min ) {
		val = min;
	}
	if ( val > max ) {
		val = max;
	}
	return val;
}

WWINLINE double WWMath::Wrap(double val, double min /*= 0.0f*/, double max /*= 1.0f*/)
{
	// Implemented as an if rather than a while, to long loops
	if ( val >= max )	val -= (max-min);
	if ( val < min )	val += (max-min);
	if ( val < min ) {
		val = min;
	}
	if ( val > max ) {
		val = max;
	}
	return val;
}

WWINLINE float WWMath::Min(float a, float b)
{
	if (a<b) return a;
	return b;
}

WWINLINE float WWMath::Max(float a, float b)
{
	if (a>b) return a;
	return b;
}

WWINLINE float WWMath::Lerp(float a, float b, float t)
{
	return (a + (b - a)*t);
}

WWINLINE double WWMath::Lerp(double a, double b, float t)
{
	return (a + (b - a)*t);
}

WWINLINE float WWMath::Inverse_Lerp(float a, float b, float v)
{
	return (v - a) / (b - a);
}

WWINLINE double WWMath::Inverse_Lerp(double a, double b, float v)
{
	return (v - a) / (b - a);
}

WWINLINE bool WWMath::Is_Valid_Float(float x)
{
	unsigned long * plong = (unsigned long *)(&x);
	unsigned long exponent = ((*plong) & 0x7F800000) >> (32-9);

	// if exponent is 0xFF, this is a NAN
	if (exponent == 0xFF) {
		return false;
	}
	return true;
}

WWINLINE bool WWMath::Is_Valid_Double(double x)
{
	unsigned long * plong = (unsigned long *)(&x) + 1;
	unsigned long exponent = ((*plong) & 0x7FF00000) >> (32-12);

	// if exponent is 0x7FF, this is a NAN
	if (exponent == 0x7FF) {
		return false;
	}
	return true;
}

WWINLINE long WWMath::Float_To_Long(float f)
{
#if USE_DETERMINISTIC_MATH
	return gm_lrintf(f);

#elif defined(_MSC_VER) && defined(_M_IX86)
	long i;
	__asm {
		fld [f]
		fistp [i]
	}
	return i;

#else
	return (long)f;
#endif
}

WWINLINE long WWMath::Float_To_Long(double f)
{
#if USE_DETERMINISTIC_MATH
	return gm_lrint(f);

#elif defined(_MSC_VER) && defined(_M_IX86)
	long retval;
	__asm {
		fld	qword ptr [f]
		fistp dword ptr [retval]
	}
	return retval;

#else
	return (long)f;
#endif
}

WWINLINE int WWMath::Float_To_Int_Chop(float f)
{
    int a	= *reinterpret_cast<const int*>(&f);				// take bit pattern of float into a register
    int sign	= (a>>31);												// sign = 0xFFFFFFFF if original value is negative, 0 if positive
    int mantissa	= (a&((1<<23)-1))|(1<<23);						// extract mantissa and add the hidden bit
    int exponent	= ((a&0x7fffffff)>>23)-127;					// extract the exponent
    int r	= ((unsigned int)(mantissa)<<8)>>(31-exponent);	// ((1<<exponent)*mantissa)>>24 -- (we know that mantissa > (1<<24))
    return ((r ^ (sign)) - sign ) &~ (exponent>>31);			// add original sign. If exponent was negative, make return value 0.
}

WWINLINE int WWMath::Float_To_Int_Floor(float f)
{
	int a			= *reinterpret_cast<const int*>(&f);			// take bit pattern of float into a register
	int sign		= (a>>31);												// sign = 0xFFFFFFFF if original value is negative, 0 if positive
	a&=0x7fffffff;															// we don't need the sign any more

	int exponent	= (a>>23)-127;										// extract the exponent
	int expsign	= ~(exponent>>31);									// 0xFFFFFFFF if exponent is positive, 0 otherwise
	int imask		= ( (1<<(31-(exponent))))-1;					// mask for true integer values
	int mantissa	= (a&((1<<23)-1));								// extract mantissa (without the hidden bit)
	int r			= ((unsigned int)(mantissa|(1<<23))<<8)>>(31-exponent);	// ((1<<exponent)*(mantissa|hidden bit))>>24 -- (we know that mantissa > (1<<24))

	r = ((r & expsign) ^ (sign)) + ((!((mantissa<<8)&imask)&(expsign^((a-1)>>31)))&sign);	// if (fabs(value)<1.0) value = 0; copy sign; if (value < 0 && value==(int)(value)) value++;
	return r;
}

WWINLINE float WWMath::Normalize_Angle(float angle)
{
	return angle - (WWMATH_TWO_PI * Floor((angle + WWMATH_PI) / WWMATH_TWO_PI));
}

WWINLINE float WWMath::Div_Safe(float dividend, float divisor, float fallback)
{
#if USE_DETERMINISTIC_MATH
	return (divisor == 0.0f) ? fallback : dividend / divisor;
#else
	return dividend / divisor;
#endif
}

WWINLINE double WWMath::Div_Safe(double dividend, double divisor, double fallback)
{
#if USE_DETERMINISTIC_MATH
	return (divisor == 0.0) ? fallback : dividend / divisor;
#else
	return dividend / divisor;
#endif
}
