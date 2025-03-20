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
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /G/wwlib/RNDSTRAW.H                                         $* 
 *                                                                                             * 
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 4/02/99 12:00p                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef RNDSTRAW_H
#define RNDSTRAW_H


#include	"RANDOM.H"
#include	"STRAW.H"

/*
**	This is a straw terminator class. It will generate random numbers to fill the data request.
**	Unlike regular straw terminators, this class will never run out of "data".
*/
class RandomStraw : public Straw
{
	public:
		RandomStraw(void);
		virtual ~RandomStraw(void);

		virtual int Get(void * source, int slen);

		void Reset(void);
		void Seed_Bit(int seed);
		void Seed_Byte(char seed);
		void Seed_Short(short seed);
		void Seed_Long(long seed);

		int Seed_Bits_Needed(void) const;

	private:
		/*
		**	Counter of the number of seed bits stored to this random number
		**	generator.
		*/
		int SeedBits;

		/*
		**	The current random generator to use when fetching the next random
		**	byte of data.
		*/
		int Current;

		/*
		**	Array of generators. There must be at least 448 bits of random number seed
		**	in order to be reasonably secure, however, using 1024 bits would be best.
		*/
		Random3Class Random[32];

		void Scramble_Seed(void);

		RandomStraw(RandomStraw & rvalue);
		RandomStraw & operator = (RandomStraw const & pipe);
};


#endif
