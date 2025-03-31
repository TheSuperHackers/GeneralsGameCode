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

/*************************************************************************** 
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     *** 
 *************************************************************************** 
 *                                                                         * 
 *                 Project Name : G                                        * 
 *                                                                         * 
 *                     $Archive:: /Commando/Code/ww3d2/pot.cpp            $* 
 *                                                                         * 
 *                      $Author:: Greg_h                                  $* 
 *                                                                         * 
 *                     $Modtime:: 1/08/01 10:04a                          $* 
 *                                                                         * 
 *                    $Revision:: 1                                       $* 
 *                                                                         * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 *   Find_POT -- finds closest inclusive power of 2 to a value             * 
 *   Find_POT_Log2 -- finds log2 of closest inclusive power of 2 to a value* 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "pot.h"

/************************************************************************** 
 * Find_POT -- finds closest inclusive power of 2 to a value              * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   10/20/1997 PWG : Created.                                            * 
 *========================================================================*/
int Find_POT(int val)
{
	// clear out the recorded position and the recorded count
	int recpos = 0;
	int reccnt = 0;

	// walk through the value shifting off bits and record the
	// position of the highest bit, and whether we have found
	// more than one bit.
	for (int lp = 0; val; lp++) {
		if (val & 1) {
			recpos = lp;
			reccnt++;
		}
		val >>= 1;
	}
	// if we have not found more than one bit then the number
	// was the power of two so return it.
	if (reccnt < 2) {
		return( 1 << recpos);
	}
	// if we found more than one bit, then the number needs to
	// be rounded up to the next highest power of 2.
	return( 1 << (recpos + 1));
}


/************************************************************************** 
 * Find_POT_Log2 -- finds log2 of closest inclusive power of 2 to a value * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *   12/23/1998 NH : Created.                                             * 
 *========================================================================*/
unsigned int Find_POT_Log2(unsigned int val)
{
	// clear out the recorded position and the recorded count
	int recpos = 0;
	int reccnt = 0;

	// walk through the value shifting off bits and record the
	// position of the highest bit, and whether we have found
	// more than one bit.
	for (int lp = 0; val; lp++) {
		if (val & 1) {
			recpos = lp;
			reccnt++;
		}
		val >>= 1;
	}
	// if we have not found more than one bit then the number
	// was the power of two so return it.
	if (reccnt < 2) {
		return recpos;
	}
	// if we found more than one bit, then the number needs to
	// be rounded up to the next highest power of 2.
	return recpos + 1;
}


