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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/hmdldef.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   HModelDefClass::HModelDefClass -- Constructor                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "hmdldef.H"
#include <assert.h>
#include <string.h>
#include "w3d_file.h"
#include "chunkio.h"
#include "snapPts.h"


/***********************************************************************************************
 * HModelDefClass::HModelDefClass -- Constructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/15/97   GTH : Created.                                                                 *
 *=============================================================================================*/
HModelDefClass::HModelDefClass(void) :
	SubObjectCount(0),
	SubObjects(NULL),
	SnapPoints(NULL)
{

}

/***********************************************************************************************
 * HModelDefClass::~HModelDefClass -- destructor                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
HModelDefClass::~HModelDefClass(void)
{
	Free();
}

/***********************************************************************************************
 * HModelDefClass::Free -- de-allocate all memory in use                                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
void HModelDefClass::Free(void)
{
	if (SubObjects != NULL) {
		delete[] SubObjects;
		SubObjects = NULL;
	}
	SubObjectCount = 0;

	if (SnapPoints != NULL) {
		SnapPoints->Release_Ref();
		SnapPoints = NULL;
	}
}


/***********************************************************************************************
 * HModelDefClass::Load -- load a set of mesh connections from a file                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
int HModelDefClass::Load_W3D(ChunkLoadClass & cload)
{
	bool pre30 = false;
	int subobjcounter = 0;

	Free();

	/*
	**	Read the first chunk, it should be the header
	*/
	if (!cload.Open_Chunk()) {
		return false;
	}

	if (cload.Cur_Chunk_ID() != W3D_CHUNK_HMODEL_HEADER) {
		goto Error;
	}

	/*
	** read in the header
	*/
	W3dHModelHeaderStruct header;
	if (cload.Read(&header,sizeof(W3dHModelHeaderStruct)) != sizeof(W3dHModelHeaderStruct)) {
		goto Error;
	}

	cload.Close_Chunk();

	/*
	** process the header info
	*/
	strlcpy(ModelName,header.Name,W3D_NAME_LEN);
	strlcpy(BasePoseName,header.HierarchyName,W3D_NAME_LEN);
	strcpy(Name,ModelName);

	/*
	** Just allocate a node for the number of sub objects we're expecting
	*/
	SubObjectCount = header.NumConnections;
	SubObjects = W3DNEWARRAY HmdlNodeDefStruct[SubObjectCount];
	if (SubObjects == NULL) {
		goto Error;
	}

	/*
	** If this is pre-3.0 set a flag so that each render object's
	** bone id will be incremented by one to account for the new
	** root node added with version3.0 of the file format.  Basically,
	** I'm making all of the code assume that node 0 is the root and
	** therefore, pre-3.0 files have to have it added and all of
	** the indices adjusted
	*/
	if (header.Version < W3D_MAKE_VERSION(3,0)) {
		pre30 = true;
	}

	/*
	** Process the rest of the chunks
	*/
	subobjcounter = 0;

	while (cload.Open_Chunk()) {

		switch (cload.Cur_Chunk_ID()) {

			case W3D_CHUNK_NODE:
			case W3D_CHUNK_COLLISION_NODE:
			case W3D_CHUNK_SKIN_NODE:
				if (!read_connection(cload,&(SubObjects[subobjcounter]),pre30)) {
					goto Error;
				}
				subobjcounter++;
				break;

			case W3D_CHUNK_POINTS:
				SnapPoints = W3DNEW SnapPointsClass;
				SnapPoints->Load_W3D(cload);
				break;

			default:
				break;
		}
		cload.Close_Chunk();
	}

	return OK;

Error:

	return LOAD_ERROR;

}


/***********************************************************************************************
 * HModelDefClass::read_connection -- read a single connection from the file                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/11/1997 GH  : Created.                                                                 *
 *   10/22/97   GH  : Check for mesh connections with PivotID=-1                               *
 *=============================================================================================*/
bool HModelDefClass::read_connection(ChunkLoadClass & cload,HmdlNodeDefStruct * node,bool pre30)
{

	W3dHModelNodeStruct con;
	if (cload.Read(&con,sizeof(W3dHModelNodeStruct)) != sizeof(W3dHModelNodeStruct)) {
		return false;
	}

	strcpy(node->RenderObjName,ModelName);
	strcat(node->RenderObjName,".");
	strcat(node->RenderObjName,con.RenderObjName);

	if (pre30) {
		if (con.PivotIdx == 65535) {
			node->PivotID = 0;
		} else {
			node->PivotID = con.PivotIdx + 1;
		}
	} else {
		assert(con.PivotIdx != 65535);
		node->PivotID = con.PivotIdx;
	}

	return true;
}

