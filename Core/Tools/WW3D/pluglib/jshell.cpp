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
 *                     $Archive:: /Commando/Code/Tools/pluglib/jshell.cpp                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/07/00 2:32p                                              $*
 *                                                                                             *
 *                    $Revision:: 28                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Fatal -- General purpose fatal error handler.                                             *
 *   Set_Bit -- Set bit in a bit array.                                                        *
 *   Get_Bit -- Fetch the bit value from a bit array.                                          *
 *   First_True_Bit -- Return with the first true bit index.                                   *
 *   First_False_Bit -- Find the first false bit in the bit array.                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"


/***********************************************************************************************
 * Set_Bit -- Set bit in a bit array.                                                          *
 *                                                                                             *
 *    This routine is used to set (or clear) a bit in a bit array.                             *
 *                                                                                             *
 * INPUT:   array -- Pointer to the bit array.                                                 *
 *                                                                                             *
 *          bit   -- The bit number to set. I.e., bit 32 is the first bit in the second        *
 *                   long word of memory.                                                      *
 *                                                                                             *
 *          value -- The value to set the bit. The only values supported are 0 and 1.          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Be sure that the array is big enough to set the specified bit.                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/06/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Set_Bit(void * array, int bit, int value)
{
	unsigned char mask = (unsigned char)(1 << (bit % 8));

	if (value != 0) {
		*((unsigned char *)array + (bit/8)) |= mask;
	} else {
		*((unsigned char *)array + (bit/8)) &= (unsigned char)~mask;
	}
}


/***********************************************************************************************
 * Get_Bit -- Fetch the bit value from a bit array.                                            *
 *                                                                                             *
 *    This routine will fetch the specified bit value from the bit array. This is the          *
 *    counterpart function to the Set_Bit() function.                                          *
 *                                                                                             *
 * INPUT:   array -- Pointer to the bit array to fetch the bit value from.                     *
 *                                                                                             *
 *          bit   -- The bit number to fetch.                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the value of the bit. This return value will be either 1 or 0.        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/06/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int Get_Bit(void const * array, int bit)
{
	unsigned char mask = (unsigned char)(1 << (bit % 8));
	return((*((unsigned char *)array + (bit/8)) & mask) != 0);
}


/***********************************************************************************************
 * First_True_Bit -- Return with the first true bit index.                                     *
 *                                                                                             *
 *    This routine will scan the bit array and return with the index for the first true bit    *
 *    in the array.                                                                            *
 *                                                                                             *
 * INPUT:   array -- Pointer to the bit array to scan.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the index of the first true (set to 1) bit in the array.              *
 *                                                                                             *
 * WARNINGS:   This routine does not stop at the end of the array (it doesn't know where the   *
 *             end is) so there must be at least one true bit in the array or else it will     *
 *             end up scanning past the end (undefined results in that case).                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/06/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int First_True_Bit(void const * array)
{
	int index = 0;
	while (*((unsigned char *)array) == 0) {
		index++;
		array = ((char*)array) + 1;
	}
	int subindex;
	for (subindex = 0; subindex < 8; subindex++) {
		if (Get_Bit(array, subindex)) break;
	}
	return(index * 8 + subindex);
}


/***********************************************************************************************
 * First_False_Bit -- Find the first false bit in the bit array.                               *
 *                                                                                             *
 *    This routine will scan the bit array and return with the index of the first false (set   *
 *    to 0) bit found.                                                                         *
 *                                                                                             *
 * INPUT:   array -- Pointer to the bit array to scan.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the index of the first false bit found in the array.                  *
 *                                                                                             *
 * WARNINGS:   This routine will not stop scanning until a false bit was found. This means     *
 *             that there must be at least one false bit in the array or else it will scan     *
 *             past the end of the array.                                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/06/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int First_False_Bit(void const * array)
{
	int index = 0;
	while (*((unsigned char *)array) == 0xFF) {
		index++;
		array = ((char*)array) + 1;
	}
	int subindex;
	for (subindex = 0; subindex < 8; subindex++) {
		if (!Get_Bit(array, subindex)) break;
	}
	return(index * 8 + subindex);
}

