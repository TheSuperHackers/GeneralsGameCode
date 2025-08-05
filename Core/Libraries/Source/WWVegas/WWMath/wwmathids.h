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
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/wwmathids.h                           $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/04/01 8:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef WWMATHIDS_H
#define WWMATHIDS_H

#include "saveloadids.h"

/*
** Persist Factory ID's for WWMATH
*/
enum
{
	WWMATH_CHUNKID_LINEARCURVE1D	= CHUNKID_WWMATH_BEGIN,
	WWMATH_CHUNKID_HERMITESPLINE1D,
	WWMATH_CHUNKID_CATMULLROMSPLINE1D,
	WWMATH_CHUNKID_CARDINALSPLINE1D,
	WWMATH_CHUNKID_TCBSPLINE1D,

	WWMATH_CHUNKID_LINEARCURVE3D	= CHUNKID_WWMATH_BEGIN + 0x100,
	WWMATH_CHUNKID_HERMITESPLINE3D,
	WWMATH_CHUNKID_CATMULLROMSPLINE3D,
	WWMATH_CHUNKID_CARDINALSPLINE3D,
	WWMATH_CHUNKID_TCBSPLINE3D,
	WWMATH_CHUNKID_VEHICLECURVE
};



#endif //WWMATHIDS_H

