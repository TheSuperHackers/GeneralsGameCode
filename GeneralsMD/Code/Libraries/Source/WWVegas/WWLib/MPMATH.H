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
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /VSS_Sync/wwlib/mpmath.h                                    $* 
 *                                                                                             * 
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             * 
 *                     $Modtime:: 3/21/01 12:01p                                              $*
 *                                                                                             * 
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef MPMATH_H
#define MPMATH_H

#include	"bool.h"

//lint -e740 -e534 -e537 -e760
//lint -d_LINT=1


#include	"STRAW.H"
#include	<stdlib.h>

#define	UNITSIZE					32
#define	MAX_BIT_PRECISION		2048
#define	MAX_UNIT_PRECISION	(MAX_BIT_PRECISION/UNITSIZE)
#define	signeddigit	signed long
#define	LOG_UNITSIZE			5

//#define	digit	unsigned long
typedef unsigned long digit;

#define MPEXPORT

#ifdef _MSC_VER
#pragma warning( disable : 4236)
#endif

extern "C" {
extern int MPEXPORT XMP_Fetch_Prime_Size(void);
unsigned short const * MPEXPORT XMP_Fetch_Prime_Table(void);
int MPEXPORT XMP_Significance(const digit * r, int precision);
void MPEXPORT XMP_Inc(digit * r, int precision);
void MPEXPORT XMP_Dec(digit * r, int precision);
void MPEXPORT XMP_Neg(digit * r, int precision);
void MPEXPORT XMP_Abs(digit * r, int precision);
void MPEXPORT XMP_Shift_Right_Bits(digit * r1, int bits, int precision);
void MPEXPORT XMP_Shift_Left_Bits(digit * r1, int bits, int precision);
bool MPEXPORT XMP_Rotate_Left(digit * r1, bool carry, int precision);
void MPEXPORT XMP_Not(digit * digit_ptr, int precision);
int MPEXPORT XMP_Prepare_Modulus(const digit * n_modulus, int precision);
void MPEXPORT XMP_Init(digit * r, digit value, int precision);
unsigned MPEXPORT XMP_Count_Bits(const digit * r, int precision);
int MPEXPORT XMP_Count_Bytes(const digit * r, int precision);
void MPEXPORT XMP_Move(digit * dest, digit const * source, int precision);
int MPEXPORT XMP_Digits_To_Bits(int digits);
int MPEXPORT XMP_Compare(const digit * r1, const digit * r2, int precision);
int MPEXPORT XMP_Bits_To_Digits(int bits);
bool MPEXPORT XMP_Add(digit * result, const digit * r1, const digit * r2, bool carry, int precision);
bool MPEXPORT XMP_Add_Int(digit * result, const digit * r1, digit r2, bool carry, int precision);
bool MPEXPORT XMP_Test_Bit(const digit * r, unsigned bit);
void MPEXPORT XMP_Set_Bit(digit * r, unsigned bit);
digit MPEXPORT XMP_Bits_To_Mask(int bits);
bool MPEXPORT XMP_Is_Negative(const digit * r, int precision);
bool MPEXPORT XMP_Test_Eq_Int(digit const * r, int i, int p);
bool MPEXPORT XMP_Sub(digit * result, const digit * r1, const digit * r2, bool borrow, int precision);
bool MPEXPORT XMP_Sub_Int(digit * result, const digit * r1, unsigned short r2, bool borrow, int precision);
int MPEXPORT XMP_Unsigned_Mult(digit * prod, const digit * multiplicand, const digit * multiplier, int precision);
int MPEXPORT XMP_Unsigned_Mult_Int(digit * prod, const digit * multiplicand, short multiplier, int precision);
int MPEXPORT XMP_Signed_Mult_Int(digit * prod, const digit * multiplicand, signed short multiplier, int precision);
int MPEXPORT XMP_Signed_Mult(digit * prod, const digit * multiplicand, const digit * multiplier, int precision);
unsigned short MPEXPORT XMP_Unsigned_Div_Int(digit * quotient, digit const * dividend, unsigned short divisor, int precision);
int MPEXPORT XMP_Unsigned_Div(digit * remainder, digit * quotient, digit const * dividend, digit const * divisor, int precision);
void MPEXPORT XMP_Signed_Div(digit * remainder, digit * quotient, digit const * dividend, digit const * divisor, int precision);
int MPEXPORT XMP_Reciprocal(digit * quotient, const digit * divisor, int precision);
void MPEXPORT XMP_Decode_ASCII(char const * str, digit * mpn, int precision);
//void MPEXPORT xmp_single_mul(unsigned short * prod, unsigned short * multiplicand, unsigned short multiplier, int precision);
void MPEXPORT XMP_Double_Mul(digit * prod, const digit * multiplicand, const digit * multiplier, int precision);
//int MPEXPORT xmp_stage_modulus(const digit * n_modulus, int precision);
int MPEXPORT XMP_Mod_Mult(digit * prod, const digit * multiplicand, const digit * multiplier, int precision);
void MPEXPORT XMP_Mod_Mult_Clear(int precision);
int MPEXPORT XMP_Exponent_Mod(digit * expout, const digit * expin, const digit * exponent_ptr, const digit * modulus, int precision);
bool MPEXPORT XMP_Is_Small_Prime(const digit * candidate, int precision);
bool MPEXPORT XMP_Small_Divisors_Test(const digit * candidate, int precision);
bool MPEXPORT XMP_Fermat_Test(const digit * candidate_prime, unsigned rounds, int precision);
void MPEXPORT XMP_Inverse_A_Mod_B(digit * result, digit const * number, digit const * modulus, int precision);
void MPEXPORT XMP_Signed_Decode(digit * result, const unsigned char * from, int frombytes, int precision);
void MPEXPORT XMP_Unsigned_Decode(digit * result, const unsigned char * from, int frombytes, int precision);
unsigned MPEXPORT XMP_Encode(unsigned char * to, digit const * from, int precision);
unsigned MPEXPORT XMP_Encode_Bounded(unsigned char * to, unsigned tobytes, digit const * from, int precision);
void MPEXPORT XMP_Randomize(digit * result, Straw & rng, int nbits, int precision);
void MPEXPORT XMP_Randomize_Bounded(digit * result, Straw & rng, digit const * min, digit const * max, int precision);
bool MPEXPORT XMP_Is_Prime(digit const * prime, int precision);
bool MPEXPORT XMP_Rabin_Miller_Test(Straw & rng, digit const * w, int rounds, int precision);
int MPEXPORT XMP_DER_Length_Encode(unsigned long length, unsigned char * output);
int MPEXPORT XMP_DER_Encode(digit const * from, unsigned char * output, int precision);
void MPEXPORT XMP_DER_Decode(digit * result, unsigned char const * input, int precision);
}


inline int MPEXPORT XMP_Digits_To_Bits(int digits)
{
	return(digits << LOG_UNITSIZE);
}


inline int MPEXPORT XMP_Bits_To_Digits(int bits)
{
	return ((bits + (UNITSIZE-1)) / UNITSIZE);
}


inline digit MPEXPORT XMP_Bits_To_Mask(int bits)
{
	if (!bits) return(0);
	return(1 << ((bits-1) % UNITSIZE));
}


inline bool MPEXPORT XMP_Is_Negative(const digit * r, int precision)
{
	return((signeddigit) *(r + (precision-1)) < 0);
}


inline void MPEXPORT XMP_Set_Bit(digit * r, unsigned bit)
{
	r[bit >> LOG_UNITSIZE] |= ((digit)1 << (bit & (UNITSIZE-1)));
}

inline bool MPEXPORT XMP_Test_Bit(const digit * r, unsigned bit)
{
	return ((r[bit >> LOG_UNITSIZE] & ((digit)1 << (bit & (UNITSIZE-1)))) != 0);
}


#endif
