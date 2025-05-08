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
 *                     $Archive:: /Commando/Code/wwmath/colmath.cpp                           $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/16/00 2:19p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "colmath.h"

const float CollisionMath::COINCIDENCE_EPSILON = 0.000001f;
CollisionMath::ColmathStatsStruct CollisionMath::Stats;

CollisionMath::ColmathStatsStruct::ColmathStatsStruct(void)
{
	Reset();
}

void CollisionMath::ColmathStatsStruct::Reset(void)
{
	TotalCollisionCount = 0;
	TotalCollisionHitCount = 0;
	
	CollisionRayTriCount = 0;
	CollisionRayTriHitCount = 0;

	CollisionAABoxTriCount = 0;
	CollisionAABoxTriHitCount = 0;
	CollisionAABoxAABoxCount = 0;
	CollisionAABoxAABoxHitCount = 0;

	CollisionOBBoxTriCount = 0;
	CollisionOBBoxTriHitCount = 0;
	CollisionOBBoxAABoxCount = 0;
	CollisionOBBoxAABoxHitCount = 0;
	CollisionOBBoxOBBoxCount = 0;
	CollisionOBBoxOBBoxHitCount = 0;
}

