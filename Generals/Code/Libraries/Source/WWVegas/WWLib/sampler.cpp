/*
**	Command & Conquer Generals(tm)
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
 *                 Project Name : Sampler                                                      *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/wwlib/sampler.cpp                                 $*
 *                                                                                             *
 *              Original Author:: Hector Yee                                                   *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RandomSamplingClass::Sample -- Samples randomly over a hypercube using Mersenne Twister   *
 *   RegularSamplingClass::Sample -- Samples over a regular hypergrid                          *
 *   StratifiedSamplingClass::Sample -- samples over a regular hypergrid with random offset    *
 *   QMCSamplingClass::Sample -- Samples using the Halton sequence                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

// Use Random Sampling if you're not sure
// Use Regular Sampling if you're not concerned about bias
// Use Stratified Sampling if you want good results in low dimensions
// Use QMC if you want low discrepancy and you want reproduceablility

#include "always.h"
#include "sampler.h"
#include "RANDOM.H"
#include <math.h>
#include <assert.h>
#include <memory.h>

Random4Class Random;

RandomSamplingClass::RandomSamplingClass(unsigned int dimensions,unsigned char divisions):
	SamplingClass(dimensions,divisions)
{		
}

/***********************************************************************************************
 * RandomSamplingClass::Sample -- Samples randomly over a hypercube using Mersenne Twister     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/11/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void RandomSamplingClass::Sample(float *target)
{
	unsigned int i;
	for (i=0; i<Dimensions; i++)
	{
		target[i]=Random.Get_Float();
	}
}

RegularSamplingClass::RegularSamplingClass(unsigned int dimensions,unsigned char divisions):
	SamplingClass(dimensions,divisions)
{
	index=W3DNEWARRAY unsigned char[Dimensions];
	Reset();
}

void RegularSamplingClass::Reset()
{
	memset(index,0,sizeof(unsigned char)*Dimensions);
}

RegularSamplingClass::~RegularSamplingClass()
{
	delete [] index;
}


/***********************************************************************************************
 * RegularSamplingClass::Sample -- Samples over a regular hypergrid                            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/11/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void RegularSamplingClass::Sample(float *target)
{
	unsigned int i;

	for (i=0; i<Dimensions; i++)
	{
		// minus one because we want to get 1.0f also
		target[i]=(float) index[i]/(Divisions-1.0f);				
	}
	
	// index[i] will always be 0..Divisons-1
	// add 1 and carry mod Divisions
	// e.g. increase x until x reaches Divisions
	// then and only then increase y. Now z increases
	// only when x=Divisions and y=Divisions etc..
	for (i=0; i<Dimensions; i++)
	{
		index[i]++;
		if (index[i]<Divisions) break;
		index[i]=0;
	}
}

StratifiedSamplingClass::StratifiedSamplingClass(unsigned int dimensions,unsigned char divisions):
	SamplingClass(dimensions,divisions)	
{	
	index=W3DNEWARRAY unsigned char[Dimensions];
	Reset();
}

void StratifiedSamplingClass::Reset()
{
	memset(index,0,sizeof(unsigned char)*Dimensions);
}

StratifiedSamplingClass::~StratifiedSamplingClass()
{
	delete [] index;
}

/***********************************************************************************************
 * StratifiedSamplingClass::Sample -- samples over a regular hypergrid with random offset      *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/11/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void StratifiedSamplingClass::Sample(float *target)
{
	unsigned int i;

	for (i=0; i<Dimensions; i++)
	{
		target[i]=(index[i]+Random.Get_Float())/(float) Divisions;
	}
	
	// index[i] will always be 0..Divisons-1
	// add 1 and carry mod Divisions
	// e.g. increase x until x reaches Divisions
	// then and only then increase y. Now z increases
	// only when x=Divisions and y=Divisions etc..
	for (i=0; i<Dimensions; i++)
	{
		index[i]++;
		if (index[i]<Divisions) break;
		index[i]=0;
	}
}

// first 100 primes

const static int primes[]=
{
   2,  3,  5,  7, 11, 13, 17, 19, 23, 29 
, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71 
, 73, 79, 83, 89, 97,101,103,107,109,113 
,127,131,137,139,149,151,157,163,167,173 
,179,181,191,193,197,199,211,223,227,229 
,233,239,241,251,257,263,269,271,277,281 
,283,293,307,311,313,317,331,337,347,349 
,353,359,367,373,379,383,389,397,401,409 
,419,421,431,433,439,443,449,457,461,463 
,467,479,487,491,499,503,509,521,523,541
};

inline float RadInv(int i,int base)
// returns the radical inverse of i in base b
// basically write a number in base b and reverse it over the decimal point
// e.g. RadInv(1) base 2 = 0.1 base 2 = 0.5 
{
	float sum=0;
	int residue;
	float power=1.0f/base;

	while (i!=0)
	{
		residue=i%base;
		i/=base;
		sum+=residue*power;
		power/=base;
	}

	return sum;
}

QMCSamplingClass::QMCSamplingClass(unsigned int dimensions,unsigned char divisions):
	SamplingClass(dimensions,divisions),
	index(0)
{	
	assert(Dimensions<100);
}


/***********************************************************************************************
 * QMCSamplingClass::Sample -- Samples using the Halton sequence                               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/11/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void QMCSamplingClass::Sample(float *target)
{
	unsigned int i;
	for (i=0; i<Dimensions; i++)
	{
		target[i]=RadInv(index,primes[i]);
	}

	index++;
}