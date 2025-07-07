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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// RandomValue.cpp
// Pseudo-random number generators 
// Author: Michael S. Booth, January 1998

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine


#include "Lib/BaseType.h"
#include "Common/RandomValue.h"
#include "Common/crc.h"
#include "Common/Debug.h"
#include "GameLogic/GameLogic.h"

//#define DETERMINISTIC				// to allow repetition for debugging


#undef DEBUG_RANDOM_CLIENT
#undef DEBUG_RANDOM_LOGIC
#undef DEBUG_RANDOM_AUDIO

//#define DEBUG_RANDOM_CLIENT
//#define DEBUG_RANDOM_LOGIC
//#define DEBUG_RANDOM_AUDIO

static const Real theMultFactor = 1.0f / (powf(2, 8 * sizeof(UnsignedInt)) - 1.0f);

// Initial seed values.
static UnsignedInt theGameClientSeed[6] =
{
    0xf22d0e56L, 0x883126e9L, 0xc624dd2fL, 0x702c49cL, 0x9e353f7dL, 0x6fdf3b64L
};

static UnsignedInt theGameAudioSeed[6] =
{
    0xf22d0e56L, 0x883126e9L, 0xc624dd2fL, 0x702c49cL, 0x9e353f7dL, 0x6fdf3b64L
};

static UnsignedInt theGameLogicBaseSeed = 0;
static UnsignedInt theGameLogicSeed[6] =
{
    0xf22d0e56L, 0x883126e9L, 0xc624dd2fL, 0x702c49cL, 0x9e353f7dL, 0x6fdf3b64L
};

// Add with carry. SUM is replaced with A + B + C, C is replaced with 1  if there was a carry, 0 if there wasn't. A carry occurred if the sum is  less than one of the inputs. This is addition, so carry can never be  more than one.
#define ADC(SUM, A, B, C)   SUM = (A) + (B) + (C); C = ((SUM < (A)) || (SUM < (B)))

static UnsignedInt randomValue(UnsignedInt *seed)
{
	UnsignedInt ax;
	UnsignedInt c = 0;
	
	
	ADC(ax, seed[5], seed[4], c);   /*  mov     ax,seed+20  */
	/*  add     ax,seed+16  */
	seed[4] = ax;                   /*  mov     seed+8,ax   */
	
	ADC(ax, ax, seed[3], c);        /*  adc     ax,seed+12  */
	seed[3] = ax;                   /*  mov     seed+12,ax  */
	
	ADC(ax, ax, seed[2], c);        /*  adc     ax,seed+8   */
	seed[2] = ax;                   /*  mov     seed+8,ax   */
	
	ADC(ax, ax, seed[1], c);        /*  adc     ax,seed+4   */
	seed[1] = ax;                   /*  mov     seed+4,ax   */
	
	ADC(ax, ax, seed[0], c);        /*  adc     ax,seed+0   */
	seed[0] = ax;                   /*  mov     seed+0,ax   */
	
	/* Increment seed array, bubbling up the carries. */
	if (!++seed[5])
	{
		if (!++seed[4])
		{
			if (!++seed[3])
			{
				if (!++seed[2])
				{
					if (!++seed[1])
					{
						++seed[0];
						++ax;
					}
				}
			}
		}
	}
	return(ax);
}

static void seedRandom(UnsignedInt SEED, UnsignedInt *seed)
{
	UnsignedInt ax;

	ax = SEED;                      /* mov     eax,SEED                     */
	ax += 0xf22d0e56;               /* add     eax,0f22d0e56h               */
	seed[0] = ax;                   /* mov     seed,eax                     */
	ax += 0x883126e9 - 0xf22d0e56;  /* add     eax,0883126e9h-0f22d0e56h    */
	seed[1] = ax;                   /* mov     seed+4,eax                   */
	ax += 0xc624dd2f - 0x883126e9;  /* add     eax,0c624dd2fh-0883126e9h    */
	seed[2] = ax;                   /* mov     seed+8,eax                   */
	ax += 0x0702c49c - 0xc624dd2f;  /* add     eax,00702c49ch-0c624dd2fh    */
	seed[3] = ax;                   /* mov     seed+12,eax                  */
	ax += 0x9e353f7d - 0x0702c49c;  /* add     eax,09e353f7dh-00702c49ch    */
	seed[4] = ax;                   /* mov     seed+16,eax                  */
	ax += 0x6fdf3b64 - 0x9e353f7d;  /* add     eax,06fdf3b64h-09e353f7dh    */
	seed[5] = ax;                   /* mov     seed+20,eax                  */
}

//
// It is necessary to separate the GameClient and GameLogic usage of random
// values to ensure that the GameLogic remains deterministic, regardless
// of the effects displayed on the GameClient.
//

UnsignedInt GetGameLogicRandomSeed( void )
{
	return theGameLogicBaseSeed;
}

UnsignedInt GetGameLogicRandomSeedCRC( void )
{
	CRC c;
	c.computeCRC(theGameLogicSeed, 6*sizeof(UnsignedInt));
	return c.get();
}

void InitRandom( void )
{
#ifdef DETERMINISTIC
	// needs to be the same every time
	seedRandom(0, theGameClientSeed);
	seedRandom(0, theGameAudioSeed);
	seedRandom(0, theGameLogicSeed);
	theGameLogicBaseSeed = 0;
#else
	time_t seconds = time( NULL );

	seedRandom(seconds, theGameAudioSeed);
	seedRandom(seconds, theGameClientSeed);
	seedRandom(seconds, theGameLogicSeed);
	theGameLogicBaseSeed = seconds;
#endif
}

void InitRandom( UnsignedInt seed )
{
	seedRandom(seed, theGameAudioSeed);
	seedRandom(seed, theGameClientSeed);
	seedRandom(seed, theGameLogicSeed);
	theGameLogicBaseSeed = seed;
#ifdef DEBUG_RANDOM_LOGIC
DEBUG_LOG(( "InitRandom %08lx",seed));
#endif
}

void InitGameLogicRandom( UnsignedInt seed )
{
#ifdef DETERMINISTIC
	// needs to be the same every time
	seedRandom(0, theGameLogicSeed);
	theGameLogicBaseSeed = 0;
#else
	seedRandom(seed, theGameLogicSeed);
	theGameLogicBaseSeed = seed;
#endif
#ifdef DEBUG_RANDOM_LOGIC
DEBUG_LOG(( "InitRandom Logic %08lx",seed));
#endif
}

//
// Integer random value
//
Int GetGameLogicRandomValue( int lo, int hi, const char *file, int line )
{
	//Int delta = hi - lo + 1;
	//Int rval;

	//if (delta == 0)
		//return hi;

	//rval = ((Int)(randomValue(theGameLogicSeed) % delta)) + lo;

	UnsignedInt delta = hi - lo + 1;
	//UnsignedInt temp;
	Int rval;

	if (delta == 0)
		return hi;

	rval = ((Int)(randomValue(theGameLogicSeed) % delta)) + lo;
	//temp = randomValue(theGameLogicSeed);
	//temp = temp % delta;

	//rval = temp + lo;

/**/
#ifdef DEBUG_RANDOM_LOGIC
DEBUG_LOG(( "%d: GetGameLogicRandomValue = %d (%d - %d), %s line %d",
				 TheGameLogic->getFrame(), rval, lo, hi, file, line ));
#endif
/**/

	return rval;
}

//
// Integer random value
//
Int GetGameClientRandomValue( int lo, int hi, const char *file, int line )
{
	UnsignedInt delta = hi - lo + 1;
	Int rval;

	if (delta == 0)
		return hi;

	rval = ((Int)(randomValue(theGameClientSeed) % delta)) + lo;

/**/
#ifdef DEBUG_RANDOM_CLIENT
DEBUG_LOG(( "%d: GetGameClientRandomValue = %d (%d - %d), %s line %d",
				TheGameLogic ? TheGameLogic->getFrame() : -1, rval, lo, hi, file, line ));
#endif
/**/

	return rval;
}

//
// Integer random value
//
Int GetGameAudioRandomValue( int lo, int hi, const char *file, int line )
{
	UnsignedInt delta = hi - lo + 1;
	Int rval;

	if (delta == 0)
		return hi;

	rval = ((Int)(randomValue(theGameAudioSeed) % delta)) + lo;

/**/
#ifdef DEBUG_RANDOM_AUDIO
DEBUG_LOG(( "%d: GetGameAudioRandomValue = %d (%d - %d), %s line %d",
				TheGameLogic->getFrame(), rval, lo, hi, file, line ));
#endif
/**/

	return rval;
}

//
// Real valued random value
//
Real GetGameLogicRandomValueReal( Real lo, Real hi, const char *file, int line )
{
	Real delta = hi - lo;
	Real rval;

	if (delta <= 0.0f)
		return hi;

	rval = ((Real)(randomValue(theGameLogicSeed)) * theMultFactor ) * delta + lo;

	DEBUG_ASSERTCRASH( rval >= lo && rval <= hi, ("Bad random val"));
/**/
#ifdef DEBUG_RANDOM_LOGIC
DEBUG_LOG(( "%d: GetGameLogicRandomValueReal = %f, %s line %d",
					TheGameLogic->getFrame(), rval, file, line ));
#endif
/**/

	return rval;
}

//
// Real valued random value
//
Real GetGameClientRandomValueReal( Real lo, Real hi, const char *file, int line )
{
	Real delta = hi - lo;
	Real rval;

	if (delta <= 0.0f)
		return hi;

	rval = ((Real)(randomValue(theGameClientSeed)) * theMultFactor ) * delta + lo;

	DEBUG_ASSERTCRASH( rval >= lo && rval <= hi, ("Bad random val"));
/**/
#ifdef DEBUG_RANDOM_CLIENT
DEBUG_LOG(( "%d: GetGameClientRandomValueReal = %f, %s line %d",
					TheGameLogic->getFrame(), rval, file, line ));
#endif
/**/

	return rval;
}

//
// Real valued random value
//
Real GetGameAudioRandomValueReal( Real lo, Real hi, const char *file, int line )
{
	Real delta = hi - lo;
	Real rval;

	if (delta <= 0.0f)
		return hi;

	rval = ((Real)(randomValue(theGameAudioSeed)) * theMultFactor ) * delta + lo;

	DEBUG_ASSERTCRASH( rval >= lo && rval <= hi, ("Bad random val"));
/**/
#ifdef DEBUG_RANDOM_AUDIO
DEBUG_LOG(( "%d: GetGameAudioRandomValueReal = %f, %s line %d",
					TheGameLogic->getFrame(), rval, file, line ));
#endif
/**/

	return rval;
}

//--------------------------------------------------------------------------------------------------------------
// GameClientRandomVariable
//

/*static*/ const char *GameClientRandomVariable::DistributionTypeNames[] = 
{
	"CONSTANT", "UNIFORM", "GAUSSIAN", "TRIANGULAR", "LOW_BIAS", "HIGH_BIAS"
};

/**
	define the range of random values, and the distribution of values
*/
void GameClientRandomVariable::setRange( Real low, Real high, DistributionType type )
{
	DEBUG_ASSERTCRASH(!(m_type == CONSTANT && m_low != m_high), ("CONSTANT GameClientRandomVariables should have low == high"));
	m_low = low;
	m_high = high;
	m_type = type;
}

/**
 * Return a value from the random distribution
 */
Real GameClientRandomVariable::getValue( void ) const
{
	switch( m_type )
	{
		case CONSTANT:
			DEBUG_ASSERTLOG(m_low == m_high, ("m_low != m_high for a CONSTANT GameClientRandomVariable\n"));
			if (m_low == m_high) {
				return m_low;
			} // else return as though a UNIFORM.
			FALLTHROUGH;

		case UNIFORM:
			return GameClientRandomValueReal( m_low, m_high );

		default:
			/// @todo fill in support for nonuniform GameClientRandomVariables.
			DEBUG_CRASH(("unsupported DistributionType in GameClientRandomVariable::getValue\n"));
			return 0.0f;
	}
}


//--------------------------------------------------------------------------------------------------------------
// GameLogicRandomVariable
//

/*static*/ const char *GameLogicRandomVariable::DistributionTypeNames[] = 
{
	"CONSTANT", "UNIFORM", "GAUSSIAN", "TRIANGULAR", "LOW_BIAS", "HIGH_BIAS"
};

/**
	define the range of random values, and the distribution of values
*/
void GameLogicRandomVariable::setRange( Real low, Real high, DistributionType type )
{
	DEBUG_ASSERTCRASH(!(m_type == CONSTANT && m_low != m_high), ("CONSTANT GameLogicRandomVariables should have low == high"));
	m_low = low;
	m_high = high;
	m_type = type;
}

/**
 * Return a value from the random distribution
 */
Real GameLogicRandomVariable::getValue( void ) const
{
	switch( m_type )
	{
		case CONSTANT:
			DEBUG_ASSERTLOG(m_low == m_high, ("m_low != m_high for a CONSTANT GameLogicRandomVariable"));
			if (m_low == m_high) {
				return m_low;
			} // else return as though a UNIFORM.
			FALLTHROUGH;

		case UNIFORM:
			return GameLogicRandomValueReal( m_low, m_high );

		default:
			/// @todo fill in support for nonuniform GameLogicRandomVariables.
			DEBUG_CRASH(("unsupported DistributionType in GameLogicRandomVariable::getValue\n"));
			return 0.0f;
	}
}


