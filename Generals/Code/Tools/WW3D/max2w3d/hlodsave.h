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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Renegade / G                                                 *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/hlodsave.h                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/27/00 10:22a                                             $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef HLODSAVE_H
#define HLODSAVE_H

#include "always.h"

#include <max.h>
#include <stdio.h>

#include "w3d_file.h"
#include "PROGRESS.H"
#include "chunkio.h"
#include "meshcon.h"


class INodeListClass;
class MeshConnectionsClass;


/**
** HLodSaveClass
** This object takes an array of mesh-connections objects and exports an LOD model
** constructed from them.
*/
class HLodSaveClass
{
public:
	HLodSaveClass (MeshConnectionsClass **connections, int lod_count, TimeValue CurTime,
						char *name, const char *htree_name, Progress_Meter_Class &meter,
						INodeListClass *origin_list);
	~HLodSaveClass (void);

	bool Save (ChunkSaveClass &csave);


protected:

	/*
	** class HLodArrayEntry hold the HLOD tree that we will save out in the Save() method.
	*/
	class HLodArrayEntry
	{
	public:
		W3dHLodArrayHeaderStruct	header;
		W3dHLodSubObjectStruct		*sub_obj;
		int								num_sub_objects;

		HLodArrayEntry (int num_sub_objs = 0)
		{
			sub_obj = NULL;
			num_sub_objects = 0;
			Allocate_Sub_Objects(num_sub_objs);
		}

		~HLodArrayEntry (void)
		{
			if (sub_obj)
			{
				delete sub_obj;
				sub_obj = NULL;
				num_sub_objects = 0;
			}
		}

		bool Allocate_Sub_Objects (int num)
		{
			if (num <= 0) return false;
			num_sub_objects = 0;
			sub_obj = new W3dHLodSubObjectStruct[num];
			if (!sub_obj) return false;
			num_sub_objects = num;
			return true;
		}

		bool operator == (const HLodArrayEntry & that)	{ return false; }
		bool operator != (const HLodArrayEntry & that)	{ return !(*this == that); }
	};

	bool save_header (ChunkSaveClass &csave);
	bool save_lod_arrays (ChunkSaveClass &csave);
	bool save_aggregate_array (ChunkSaveClass & csave);
	bool save_proxy_array(ChunkSaveClass & csave);
	bool save_sub_object_array(ChunkSaveClass & csave, const HLodArrayEntry & array);

	W3dHLodHeaderStruct					header;
	HLodArrayEntry	*						lod_array;
	HLodArrayEntry							aggregate_array;
	HLodArrayEntry							proxy_array;
};



#endif
