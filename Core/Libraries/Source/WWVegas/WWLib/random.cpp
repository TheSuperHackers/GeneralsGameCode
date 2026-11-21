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
 *                     $Archive:: /VSS_Sync/wwlib/random.cpp                                  $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:24p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include "RANDOM.h"

// Timing tests for random these random number generators in seconds for
// 10000000 iterations. Testing done by Hector Yee, 6/20/01
// using DIEHARD and multidimensional Monte-Carlo Integration
/*
Time for Random=0.156000
Time for Random2=0.250000
Time for Random3=1.281000
Time for Random4=0.375000
Time for Built in Rand()=0.813000
Results from DIEHARD battery of tests
A p-value of 0.0 or 1.0 means it fails. Anything in between is ok.
R0 - FAILED almost all tests, didn't complete squeeze test
R2 - Passed all tests, p-value 0.6
R3 - Failed 11 of 253 tests, p-value 1.0
R4 - Passed all tests, p-value 0.2588
Rand() - FAILED almost all tests, didn't complete squeeze test
Results from Montecarlo Integration 2-96 dimensions
R0 - starts breaking in 24 dimensions
R2 - Ok, but will repeat with short period. Huge spike at 300000 samples in 64 dimensions
R3 - Strong bias from 2 dimensions up
R4 - Ok
Rand() - starts breaking in 24 dimensions
*/


// This constructor can take an integer as a parameter. This allows the class to be constructed by
// assigning an integer to an existing object. The compiler creates a temporary with the constructor
// and then performs a copy constructor operation.
RandomClass::RandomClass(unsigned seed) :
	Seed(seed)
{
}


// WARNING: This routine modifies the seed value so that subsequent calls will not return the same
// value. Take note that this routine only returns 15 bits of random number.
int RandomClass::operator ()()
{
	/*
	**	Transform the seed value into the next number in the sequence.
	*/
	Seed = (Seed * MULT_CONSTANT) + ADD_CONSTANT;

	/*
	**	Extract the 'random' bits from the seed and return that value as the
	**	random number result.
	*/
	return((Seed >> THROW_AWAY_BITS) & (~((~0) << SIGNIFICANT_BITS)));
}


// The range is inclusive. This replaces the functionality of IRandom() in the old library.
// WARNING: The range specified must fall within the maximum bit significance of the random number
// algorithm (15 bits), otherwise the value returned will be decidedly non-random.
int RandomClass::operator() (int minval, int maxval)
{
	return(Pick_Random_Number(*this, minval, maxval));
}


Random2Class::Random2Class(unsigned seed) :
	Index1(0),
	Index2(103)
{
	Random3Class random(seed);

	for (int index = 0; index < ARRAY_SIZE(Table); index++) {
		Table[index] = random;
	}
}


int Random2Class::operator() ()
{
	Table[Index1] ^= Table[Index2];
	int val = Table[Index1];

	Index1++;
	Index2++;

	if (Index1 >= ARRAY_SIZE(Table)) Index1 = 0;
	if (Index2 >= ARRAY_SIZE(Table)) Index2 = 0;

	return(val);
}


// The range is inclusive. It uses a method that will not bias the values in any way.
int Random2Class::operator() (int minval, int maxval)
{
	return(Pick_Random_Number(*this, minval, maxval));
}


/*
**	This is the seed table for the Random3Class generator. These ensure
**	that the algorithm is not vulnerable to being primed with a weak seed
**	and thus prevents the algorithm from breaking down as a result.
*/
int Random3Class::Mix1[20] = {
	(int)0xbaa96887, (int)0x1e17d32c, (int)0x03bcdc3c, (int)0x0f33d1b2,
	(int)0x76a6491d, (int)0xc570d85d, (int)0xe382b1e3, (int)0x78db4362,
	(int)0x7439a9d4, (int)0x9cea8ac5, (int)0x89537c5c, (int)0x2588f55d,
	(int)0x415b5e1d, (int)0x216e3d95, (int)0x85c662e7, (int)0x5e8ab368,
	(int)0x3ea5cc8c, (int)0xd26a0f74, (int)0xf3a9222b, (int)0x48aad7e4
};

int Random3Class::Mix2[20] = {
	(int)0x4b0f3b58, (int)0xe874f0c3, (int)0x6955c5a6, (int)0x55a7ca46,
	(int)0x4d9a9d86, (int)0xfe28a195, (int)0xb1ca7865, (int)0x6b235751,
	(int)0x9a997a61, (int)0xaa6e95c8, (int)0xaaa98ee1, (int)0x5af9154c,
	(int)0xfc8e2263, (int)0x390f5e8c, (int)0x58ffd802, (int)0xac0a5eba,
	(int)0xac4874f6, (int)0xa9df0913, (int)0x86be4c74, (int)0xed2c123b
};


// It is suggested that some random value based on user input seed the first value. The second seed
// adds randomness and thus allows this algorithm to use a 64 bit seed. Due to a peculiarity of the
// random number design, the second seed value can be used to find the Nth random number generated
// by this algorithm. The second seed is used as the Nth index value.
// WARNING: As with all random number generators. Randomness is only as strong as the initial seed
// value.
Random3Class::Random3Class(unsigned seed1, unsigned seed2) :
	Seed(seed1),
	Index(seed2)
{
}


// The number returned is strongly random and is nearly good enough for cryptography.
int Random3Class::operator() ()
{
	int loword = Seed;
	int hiword = Index++;
	for (int i = 0; i < 4; i++) {
		int hihold  = hiword;
		int temp    = hihold ^  Mix1[i];
		int itmpl   = temp   &  0xffff;
		int itmph   = temp   >> 16;
		temp    = itmpl * itmpl + ~(itmph * itmph);
		temp    = (temp >> 16) | (temp << 16);
		hiword  = loword ^ ((temp ^ Mix2[i]) + itmpl * itmph);
		loword  = hihold;
	}
	return(hiword);
}


// The range is inclusive. It uses a method that will not bias the values in any way.
int Random3Class::operator() (int minval, int maxval)
{
	return(Pick_Random_Number(*this, minval, maxval));
}

// Random4
// Hector Yee 6/11/01

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

Random4Class::Random4Class(unsigned int seed)
{
    /* setting initial seeds to mt[N] using         */
    /* the generator Line 25 of Table 1 in          */
    /* [KNUTH 1981, The Art of Computer Programming */
    /*    Vol. 2 (2nd Ed.), pp102]                  */
	if (!seed) seed=4375;

    mt[0]= seed & 0xffffffff;
    for (mti=1; mti<N; mti++)
        mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
	 // mti is N+1 after this
}

int Random4Class::operator() ()
{
    unsigned int y;
    static unsigned int mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) {
        /* generate N words at one time */
        int kk;

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }

    y = mt[mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

	 int *x=(int *)&y;

	 return *x;
}

int Random4Class::operator() (int minval, int maxval)
{
	return(Pick_Random_Number(*this, minval, maxval));
}

float Random4Class::Get_Float()
{
	int x=(*this)();
	unsigned int *y=(unsigned int *) &x;

	return (*y)*2.3283064370807973754314699618685e-10f;
}
