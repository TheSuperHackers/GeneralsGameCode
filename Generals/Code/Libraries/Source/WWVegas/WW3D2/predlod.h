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

/*************************************************************************** 
 ***    C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S     *** 
 *************************************************************************** 
 *                                                                         * 
 *                 Project Name : G                                        * 
 *                                                                         * 
 *                     $Archive:: /Commando/Code/ww3d2/predlod.h          $* 
 *                                                                         * 
 *                      $Author:: Jani_p                                  $* 
 *                                                                         * 
 *                     $Modtime:: 8/23/01 5:04p                           $* 
 *                                                                         * 
 *                    $Revision:: 2                                       $* 
 *                                                                         * 
 *-------------------------------------------------------------------------* 
 * Functions:                                                              * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef PREDLOD_H
#define PREDLOD_H

// This file contains the classes which support predictive LOD management
// similar to that outlined in "Adaptive Display Algorithm for Interactive
// Frame Rates During Visualization of Complex Virtual Environments",
// Thomas Funkhouser & Carlo Sequin, SIGGRAPH '93 Proceedings, pp. 247-253.
// To this "pure predictive" LOD we have added distance (actually screensize)
// clamping to control quality degradation.

#include "rendobj.h"
#include "float.h"
#include "Vector.H"

class LODHeapNode;

/*
** PredictiveLODOptimizerClass: Class which performs the predictive LOD
** optimization. All the members of this class are static.
*/
class PredictiveLODOptimizerClass {

	public:

		static void		Clear(void);
		static void		Add_Object(RenderObjClass *robj);
		static void		Add_Cost(float cost)								{ TotalCost += cost; }
		static void		Optimize_LODs(float max_cost);
		static float	Get_Total_Cost(void)								{ return TotalCost; }
		static void		Free(void);	// frees all memory

	private:
		static void		AllocVisibleObjArrays(int num_objects);

		static RenderObjClass **	ObjectArray;
		static int						ArraySize;
		static int						NumObjects;
		static float					TotalCost;

		static LODHeapNode *VisibleObjArray1;
		static LODHeapNode *VisibleObjArray2;
		static int NumVisibleObjects;

};

#endif
