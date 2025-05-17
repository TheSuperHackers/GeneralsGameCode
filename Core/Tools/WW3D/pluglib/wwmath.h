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
 *                     $Modtime:: 3/16/00 8:28p                                               $*
 *                                                                                             *
 *                    $Revision:: 40                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef WWMATH_H
#define WWMATH_H

#include "always.h"
#include <math.h>
#include <assert.h>

/*
** Some global constants.
*/
#define WWMATH_EPSILON		0.0001f
#define WWMATH_EPSILON2		WWMATH_EPSILON * WWMATH_EPSILON
#define WWMATH_PI				3.141592654f
#define WWMATH_FLOAT_MAX	(FLT_MAX)
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



/*
** Some simple math functions which work on the built-in types.  
** Include the various other header files in the WWMATH library 
** in order to get matrices, quaternions, etc.
*/
class WWMath
{
public:

static float		Fabs(float val) { return (float)fabs(val); }
static float		Sqrt(float val) { return (float)sqrt(val); }
static float		Inv_Sqrt(float val) { return 1.0f / (float)sqrt(val); }
static float		Sign(float val);
static float		Floor(float val) { return (float)floor(val); }
static bool			Fast_Is_Float_Positive(const float & val);

static float		Random_Float(void);
static float		Random_Float(float min,float max);
static float		Clamp(float val, float min = 0.0f, float max = 1.0f);
static double		Clamp(double val, double min = 0.0f, double max = 1.0f);
static float		Wrap(float val, float min = 0.0f, float max = 1.0f);
static double		Wrap(double val, double min = 0.0f, double max = 1.0f);
static float		Min(float a, float b);
static float		Max(float a, float b);

static float		Lerp(float a, float b, float lerp );
static double		Lerp(double a, double b, float lerp );

static long			Float_To_Long(float f);
static long			Float_To_Long(double f);

static unsigned char Unit_Float_To_Byte(float f) { return (unsigned char)(f*255.0f); }
static float			Byte_To_Unit_Float(unsigned char byte) { return ((float)byte) / 255.0f; }

static bool			Is_Valid_Float(float x);
static bool			Is_Valid_Double(double x);

};

inline float WWMath::Sign(float val) 
{ 
	if (val > 0.0f) { 
		return +1.0f; 
	} 
	if (val < 0.0f) { 
		return -1.0f; 
	} 
	return 0.0f; 
}

inline bool WWMath::Fast_Is_Float_Positive(const float & val)
{
	return !((*(int *)(&val)) & 0x80000000);
}

inline float WWMath::Random_Float(float min,float max) 
{ 
	return Random_Float() * (max-min) + min; 
}

inline float WWMath::Clamp(float val, float min /*= 0.0f*/, float max /*= 1.0f*/) 
{
	if(val < min) return min;
	if(val > max) return max;
	return val;
}

inline double WWMath::Clamp(double val, double min /*= 0.0f*/, double max /*= 1.0f*/) 
{
	if(val < min) return min;
	if(val > max) return max;
	return val;
}

inline float WWMath::Wrap(float val, float min /*= 0.0f*/, float max /*= 1.0f*/) 
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

inline double WWMath::Wrap(double val, double min /*= 0.0f*/, double max /*= 1.0f*/) 
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

inline float WWMath::Min(float a, float b)
{
	if (a<b) return a;
	return b;
}

inline float WWMath::Max(float a, float b)
{
	if (a>b) return a;
	return b;
}

inline float	WWMath::Lerp(float a, float b, float lerp )
{
	return (a + (b - a)*lerp);
}

inline double	WWMath::Lerp(double a, double b, float lerp )
{
	return (a + (b - a)*lerp);
}


inline long WWMath::Float_To_Long	(float f)	
{
#if defined(_MSC_VER) && defined(_M_IX86)
	long retval;
	__asm fld	dword ptr [f]
	__asm fistp dword ptr [retval]
	return retval;
#else 
	return (long) f;
#endif
}

inline long WWMath::Float_To_Long	(double f)	
{
#if defined(_MSC_VER) && defined(_M_IX86)
	long retval;
	__asm fld	qword ptr [f]
	__asm fistp dword ptr [retval]
	return retval;
#else 
	return (long) f;
#endif
}

inline bool WWMath::Is_Valid_Float(float x)
{
	unsigned long * plong = (unsigned long *)(&x);
	unsigned long exponent = ((*plong) & 0x7F800000) >> (32-9);

	// if exponent is 0xFF, this is a NAN 
	if (exponent == 0xFF) {
		return false;
	}
	return true;
}

inline bool WWMath::Is_Valid_Double(double x)
{
	unsigned long * plong = (unsigned long *)(&x) + 1;
	unsigned long exponent = ((*plong) & 0x7FF00000) >> (32-12);

	// if exponent is 0x7FF, this is a NAN 
	if (exponent == 0x7FF) {
		return false;
	}
	return true;
}

#endif
