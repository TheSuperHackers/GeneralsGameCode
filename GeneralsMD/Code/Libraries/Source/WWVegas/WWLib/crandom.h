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
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando                                                     * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Combat/crandom.h            $* 
 *                                                                                             * 
 *                      $Author:: Byon_g                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 5/09/01 3:48p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 2                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef	CRANDOM_H
#define	CRANDOM_H

#ifndef	ALWAYS_H
	#include "always.h"
#endif

#ifndef	RANDOM_H
	#include "RANDOM.H"
#endif

#ifndef	WWDEBUG_H
	#include "wwdebug.h"
#endif

#define	CRANDOM_FLOAT_RANGE			0x1000

/*
** Commando Random Numbers Manager
*/
class CRandom {

public:
	CRandom( void )		{}
	~CRandom( void )		{}

	// Get a random 32 bit long integer
	inline int Get_Int( void )		{ return Generator(); }

	// Get a random 32 bit long integer less than max
	inline int Get_Int( int max )	{	WWASSERT( max > 0 );	return (Generator() & 0x7FFFFFFF) % max;	}

	// Get a random 32 bit long between min and max (both inclusive)
	inline int Get_Int( int min, int max );

	// Get a random float between 0 and 1 (both inclusive)
	inline float Get_Float( void )	{	return (float)(Get_Int( CRANDOM_FLOAT_RANGE+1 )) / (float)CRANDOM_FLOAT_RANGE;	}

	// Get a random float between 0 and max (both inclusive)
	inline float Get_Float( float max )	{	return Get_Float() * max; }

	// Get a random float between min and max (both inclusive)
	inline float Get_Float( float min, float max );

private:
	Random2Class	Generator;
};


// Get a random 32 bit long between min and max (both inclusive)
inline int CRandom::Get_Int( int min, int max )
{
	// make sure we have a valid range
	if ( min > max ) {
		int temp = min;
		min = max;
		max = temp;
	}

	// Get one
	return Get_Int( max - min ) + min;
}


// Get a random float between min and max (both inclusive)
inline float CRandom::Get_Float( float min, float max )
{
	// make sure we have a valid range
	if ( min > max ) {
		float temp = min;
		min = max;
		max = temp;
	}

	// Get one
	return Get_Float() *  ( max - min ) + min;
}



/*
** A free random number generator.  This can be used for any numbers not required to
** be synced between other computers.  Good for simple visual and sound effects.
*/
extern	CRandom	FreeRandom;

#endif