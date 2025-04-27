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

/* $Header: /Commando/Code/Tools/max2w3d/skindata.h 6     10/28/97 6:08p Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Commando Tools - WWSkin                                      * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/Tools/max2w3d/skindata.h                     $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $* 
 *                                                                                             * 
 *                     $Modtime:: 10/21/97 2:04p                                              $* 
 *                                                                                             * 
 *                    $Revision:: 6                                                           $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef SKINDATA_H
#define SKINDATA_H

#include "max.h"
#include "namedsel.h"

/*
** InfluenceStruct - structure which stores the bone 
** influence information for a single vertex.
*/
struct InfluenceStruct
{
	/*
	** vertices can be influenced by up to two bones.
	*/
	int		BoneIdx[2];
	float		BoneWeight[2];

	InfluenceStruct(void) { BoneIdx[0] = -1; BoneIdx[1] = -1; BoneWeight[0] = 1.0f; BoneWeight[1] = 0.0f; }
	
	void Set_Influence(int boneidx) {
		// TODO: make this actually let you set two bones with
		// weighting values.  Need UI to furnish this info...
		BoneIdx[0] = boneidx;
	}
};


/*
** SkinDataClass - a class which contains the bone influence data
** for the modifier.  One of these will be hung off of the 
** ModContext...
*/
class SkinDataClass : public LocalModData
{

public:

	SkinDataClass(void) { Held = FALSE; Valid = FALSE; }

	SkinDataClass(Mesh *mesh)
	{
		VertSel = mesh->vertSel;
		VertData.SetCount(mesh->getNumVerts());
		for (int i=0; i<VertData.Count(); i++) {
			VertData[i].BoneIdx[0] = VertData[i].BoneIdx[1] = -1;
			VertData[i].BoneWeight[0] = 1.0f;
			VertData[i].BoneWeight[1] = 0.0f;
		}
		Valid = TRUE;
		Held = FALSE;
	}

	void Invalidate() { Valid = FALSE; }

	BOOL IsValid() { return Valid; }

	void Validate(Mesh *mesh)
	{
		if (!Valid)
		{
			VertSel.SetSize(mesh->vertSel.GetSize(),1);
			VertData.SetCount(mesh->getNumVerts());
			Valid = TRUE;
		}
	}

	virtual LocalModData * Clone(void) 
	{ 
		SkinDataClass * newdata = new SkinDataClass();
		newdata->VertSel = VertSel;
		newdata->VertData = VertData;
		return newdata;
	}

	void Add_Influence(int boneidx) 
	{
		/*
		** Make this INode influence all currently selected vertices
		*/
		for (int i=0; i<VertData.Count(); i++) {
			if (VertSel[i]) {
				VertData[i].Set_Influence(boneidx);
			}
		}
	}

	IOResult Save(ISave *isave);
	IOResult Load(ILoad *iload);

public:

	BOOL							Valid;
	BOOL							Held;
	
	/*
	** Current selection
	*/
	BitArray						VertSel;
	
	/*
	** Named selection sets
	*/
	NamedSelSetList			VertSelSets;

	/*
	** Vertex influence data
	*/
	Tab<InfluenceStruct>		VertData;

	/*
	** Load/Save chunk ID's
	*/
	enum {
		FLAGS_CHUNK = 				0x0000,
		VERT_SEL_CHUNK = 			0x0010,	
		NAMED_SEL_SETS_CHUNK =	0x0020,
		INFLUENCE_DATA_CHUNK = 	0x0030
	};

};


#endif
