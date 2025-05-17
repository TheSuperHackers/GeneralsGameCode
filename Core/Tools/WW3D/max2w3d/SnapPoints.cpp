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
 *                 Project Name : Max2W3d                                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/SnapPoints.cpp                 $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/23/98 11:05a                                             $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#include "SnapPoints.h"
#include "chunkio.h"
#include "max.h"
#include "nodelist.h"
#include "w3d_file.h"


class PointFilterClass : public INodeFilterClass
{
public:
	PointFilterClass(void) { }

	virtual BOOL Accept_Node(INode * node, TimeValue time)
	{
		if (node == NULL) return FALSE;
		Object * obj = node->EvalWorldState(time).obj;
		if (obj == NULL) return FALSE;
		
		if 
		(
			obj->ClassID() == Class_ID(POINTHELP_CLASS_ID,0) &&
			!node->IsHidden()
		) 
		{
			return TRUE;
		} else {
			return FALSE;
		} 
	}
};


void SnapPointsClass::Export_Points(INode * scene_root,TimeValue time,ChunkSaveClass & csave)
{
	if (scene_root == NULL) return;
	
	PointFilterClass pointfilter;
	INodeListClass pointlist(scene_root,time,&pointfilter);

	if (pointlist.Num_Nodes() > 0) {

		csave.Begin_Chunk(W3D_CHUNK_POINTS);

		for (unsigned int ci=0; ci<pointlist.Num_Nodes(); ci++) {

			W3dVectorStruct vect;
			Point3 pos = pointlist[ci]->GetNodeTM(time).GetTrans();
			vect.X = pos.x;
			vect.Y = pos.y;
			vect.Z = pos.z;
			csave.Write(&vect,sizeof(vect));

		}

		csave.End_Chunk();
	}
}
