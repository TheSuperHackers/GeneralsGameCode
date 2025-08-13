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
 *                     $Archive:: /VSS_Sync/wwlib/random.h                                    $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Pick_Random_Number -- Picks a random number between two values (inclusive).               *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef RANDOM_H
#define RANDOM_H

// "unreferenced inline function has been removed" Yea, so what?
#pragma warning(disable : 4514)

/*
**	This class functions like a 'magic' int value that returns a random number
**	every time it is read. To set the "random seed" for this, just assign a number
**	to the object (just as you would if it were a true 'int' value). Take note that although
**	this class will return an 'int', the actual significance of the random number is
**	limited to 15 bits (0..32767).
*/
class RandomClass {
	public:
		RandomClass(unsigned seed=0);

		operator int(void) {return(operator()());};
		int operator() (void);
		int operator() (int minval, int maxval);

		enum {
			SIGNIFICANT_BITS=15				// Random number bit significance.
		};

	protected:
		unsigned long Seed;

		/*
		**	Internal working constants that are used to generate the next
		**	random number.
		*/
		enum {
			MULT_CONSTANT=0x41C64E6D,		// K multiplier value.
			ADD_CONSTANT=0x00003039,		// K additive value.
			THROW_AWAY_BITS=10				// Low bits to throw away.
		};
};


/*
**	This class functions like a 'magic' number where it returns a different value every
**	time it is read. It is nearly identical in function to the RandomClass, but has the
**	following improvements.
**
**	1> It generates random numbers very quickly. No multiplies are
**		used in the algorithm.
**
**	2> The return value is a full 32 bits rather than 15 bits of
**		the RandomClass.
**
**	3>	The bit pattern won't ever repeat. (actually it will repeat
**		in about 10 to the 50th power times).
*/
// WARNING!!!!
// This random number generator starts breaking down in 64 dimensions
// behaving very badly in that domain
// HY 6/14/01
class Random2Class {
	public:
		Random2Class(unsigned seed=0);

		operator int(void) {return(operator()());};
		int operator() (void);
		int operator() (int minval, int maxval);

		enum {
			SIGNIFICANT_BITS=32				// Random number bit significance.
		};

	protected:
		int Index1;
		int Index2;
		int Table[250];
};


/*
**	This class functions like a 'magic' number where it returns a different value every
**	time it is read. It is nearly identical in function to the RandomClass and Random2Class,
**	but has the following improvements.
**
**	1> The random number returned is very strongly random. Approaching cryptographic
**		quality.
**
**	2> The return value is a full 32 bits rather than 15 bits of
**		the RandomClass.
**
**	3> The bit pattern won't repeat until 2^32 times.
*/
// WARNING!!!!
// This random number generator starts breaking down in 3 dimensions
// exhibiting a strange bias
// HY 6/14/01
class Random3Class {
	public:
		Random3Class(unsigned seed1=0, unsigned seed2=0);

		operator int(void) {return(operator()());};
		int operator() (void);
		int operator() (int minval, int maxval);

		enum {
			SIGNIFICANT_BITS=32				// Random number bit significance.
		};

	protected:
		static int Mix1[20];
		static int Mix2[20];
		int Seed;
		int Index;
};

/*
**	This class functions like a 'magic' number where it returns a different value every
**	time it is read. It is based on the paper "Mersenne Twister: A 623-Dimensionally
** Equidistributed Uniform Pseudo-Random Number Generator
** by Makoto Matsumoto and Takuji Nishimura, ACM Transactions on Modeling and
** Computer Simulation, Vol 8, No 1, January 1998, Pages 3-30
** Hector Yee
**
**	1> The random number returned is very strongly random
**
**	2> The return value is a full 32 bits rather than 15 bits of
**		the RandomClass.
**
**	3> The bit pattern won't repeat until 2^19937 -1 times
**
*/
// WARNING!!!!
// Do not use for cryptography. This random number generator is good for numerical
// simulation. Optimized, it's 4 times faster than rand()
// http://www.math.keio.ac.jp/~matumoto/emt.html
// HY 6/14/01
class Random4Class {
	public:
		Random4Class(unsigned int seed=4357);

		operator int(void) {return(operator()());};
		int operator() (void);
		int operator() (int minval, int maxval);
		float Get_Float();

		enum {
			SIGNIFICANT_BITS=32				// Random number bit significance.
		};

	protected:
		unsigned int mt[624]; // state vector
		int mti;			 // index
};


/***********************************************************************************************
 * Pick_Random_Number -- Picks a random number between two values (inclusive).                 *
 *                                                                                             *
 *    This is a utility template that works with one of the random number classes. Given a     *
 *    random number generator, this routine will return with a value that lies between the     *
 *    minimum and maximum values specified (subject to the bit precision limitations of the    *
 *    random number generator itself).                                                         *
 *                                                                                             *
 * INPUT:   generator   -- Reference to the random number generator to use for the pick        *
 *                         process.                                                            *
 *                                                                                             *
 *          minval      -- The minimum legal return value (inclusive).                         *
 *                                                                                             *
 *          maxval      -- The maximum legal return value (inclusive).                         *
 *                                                                                             *
 * OUTPUT:  Returns with a random number between the minval and maxval (inclusive).            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class T>
int Pick_Random_Number(T & generator, int minval, int maxval)
{
	/*
	**	Test for shortcut case where the range is null and thus
	**	the number to return is actually implicit from the
	**	parameters.
	*/
	if (minval == maxval) return(minval);

	/*
	**	Ensure that the min and max range values are in proper order.
	*/
	if (minval > maxval) {
		int temp = minval;
		minval = maxval;
		maxval = temp;
	}

	/*
	**	Find the highest bit that fits within the magnitude of the
	**	range of random numbers desired. Notice that the scan is
	**	limited to the range of significant bits returned by the
	**	random number algorithm.
	*/
	int magnitude = maxval - minval;
	int highbit = T::SIGNIFICANT_BITS-1;
	while ((magnitude & (1 << highbit)) == 0 && highbit > 0) {
		highbit--;
	}

	/*
	**	Create a full bit mask pattern that has all bits set that just
	**	barely covers the magnitude of the number range desired.
	*/
	int mask = ~( (~0L) << (highbit+1));

	/*
	**	Keep picking random numbers until it fits within the magnitude desired. With a
	**	good random number generator, it will have to perform this loop an average
	**	of one and a half times.
	*/
	int pick = magnitude+1;
	while (pick > magnitude) {
		pick = generator() & mask;
	}

	/*
	**	Finally, bias the random number pick to the start of the range
	**	requested.
	*/
	return(pick + minval);
}

#endif
