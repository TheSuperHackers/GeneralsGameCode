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
 *                     $Archive:: /Commando/Code/wwmath/aabox.cpp                             $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 5/08/01 6:33p                                               $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AABoxClass::Init_Random -- initializes this box to a random state                         *
 *   AABoxClass::Contains -- test whether this box contains the given point                    *
 *   AABoxClass::Contains -- Test whether this box contains the given box                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "aabox.h"
#include "colmath.h"
#include "colmathinlines.h"
#include <float.h>


/***********************************************************************************************
 * AABoxClass::Init_Random -- initializes this box to a random state                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/17/2000  gth : Created.                                                                 *
 *=============================================================================================*/
void AABoxClass::Init_Random(float min_center,float max_center,float min_extent,float max_extent)
{
	Center.X = min_center + WWMath::Random_Float() * (max_center - min_center);
	Center.Y = min_center + WWMath::Random_Float() * (max_center - min_center);
	Center.Z = min_center + WWMath::Random_Float() * (max_center - min_center);
	
	Extent.X = min_extent + WWMath::Random_Float() * (max_extent - min_extent);
	Extent.Y = min_extent + WWMath::Random_Float() * (max_extent - min_extent);
	Extent.Z = min_extent + WWMath::Random_Float() * (max_extent - min_extent);
}


void AABoxClass::Transform(const Matrix3D & tm,const AABoxClass & in,AABoxClass * out)
{
	tm.Transform_Center_Extent_AABox(in.Center,in.Extent,&(out->Center),&(out->Extent));
}

void MinMaxAABoxClass::Init_Empty(void) 
{ 
	MinCorner.Set(FLT_MAX,FLT_MAX,FLT_MAX); 
	MaxCorner.Set(-FLT_MAX,-FLT_MAX,-FLT_MAX); 
}
