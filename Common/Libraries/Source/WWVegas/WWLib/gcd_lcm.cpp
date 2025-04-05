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
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /G/wwlib/gcd_lcm.cpp                                        $*
 *                                                                                             *
 *                      $Author:: Naty_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/25/99 11:31a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "gcd_lcm.h"

unsigned int Greatest_Common_Divisor(unsigned int a, unsigned int b)
{
	// This uses Euclid's algorithm
	if (b == 0) {
		return a;
	} else {
		return Greatest_Common_Divisor(b, a % b);
	}
}


unsigned int Least_Common_Multiple(unsigned int a, unsigned int b)
{
	return (a * b) / Greatest_Common_Divisor(a, b);
}
