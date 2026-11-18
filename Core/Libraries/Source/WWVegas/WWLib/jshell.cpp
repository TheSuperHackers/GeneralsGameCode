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
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"


// Set (or clear) a bit in a bit array. Bit 32 is the first bit in the second long word of memory.
// The only values supported are 0 and 1.
// WARNING: Be sure that the array is big enough to set the specified bit.
void Set_Bit(void * array, int bit, int value)
{
	unsigned char mask = (unsigned char)(1 << (bit % 8));

	if (value != 0) {
		*((unsigned char *)array + (bit/8)) |= mask;
	} else {
		*((unsigned char *)array + (bit/8)) &= (unsigned char)~mask;
	}
}


int Get_Bit(void const * array, int bit)
{
	unsigned char mask = (unsigned char)(1 << (bit % 8));
	return((*((unsigned char *)array + (bit/8)) & mask) != 0);
}


// WARNING: This routine does not stop at the end of the array (it doesn't know where the end is) so
// there must be at least one true bit in the array or else it will end up scanning past the end
// (undefined results in that case).
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


// WARNING: This routine will not stop scanning until a false bit was found. This means that there
// must be at least one false bit in the array or else it will scan past the end of the array.
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

