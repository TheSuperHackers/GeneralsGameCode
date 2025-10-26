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

/* $Header: /Commando/Code/ww3d2/htreemgr.cpp 1     1/22/01 3:36p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/htreemgr.cpp                           $*
 *                                                                                             *
 *                       Author:: Byon_g                                                       *
 *                                                                                             *
 *                     $Modtime:: 1/08/01 10:04a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   HTreeManagerClass::HTreeManagerClass -- constructor                                       *
 *   HTreeManagerClass::~HTreeManagerClass -- destructor                                       *
 *   HTreeManagerClass::Free -- de-allocate all memory in use                                  *
 *   HTreeManagerClass::Free_All_Trees -- de-allocates all hierarchy trees currently loaded    *
 *   HTreeManagerClass::Load_Tree -- load a hierarchy tree from a file                         *
 *   HTreeManagerClass::Get_Tree_ID -- look up the ID of a named hierarchy tree                *
 *   HTreeManagerClass::Get_Tree -- get a pointer to the specified hierarchy tree              *
 *   HTreeManagerClass::Get_Tree -- get a pointer to the specified hierarchy tree              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "htreemgr.h"
#include <string.h>
#include "htree.h"
#include "chunkio.h"
#include "wwmemlog.h"
#include "w3dexclusionlist.h"


/***********************************************************************************************
 * HTreeManagerClass::HTreeManagerClass -- constructor                                         *
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
HTreeManagerClass::HTreeManagerClass(void) :
	NumTrees(0)
{
	for (int treeidx=0; treeidx < MAX_TREES; treeidx++) {
		TreePtr[treeidx] = NULL;
	}
}

/***********************************************************************************************
 * HTreeManagerClass::~HTreeManagerClass -- destructor                                         *
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
HTreeManagerClass::~HTreeManagerClass(void)
{
	Free();
}

/***********************************************************************************************
 * HTreeManagerClass::Free -- de-allocate all memory in use                                    *
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
void HTreeManagerClass::Free(void)
{
	Free_All_Trees();
}

/***********************************************************************************************
 * HTreeManagerClass::Free_All_Trees -- de-allocates all hierarchy trees currently loaded      *
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
void HTreeManagerClass::Free_All_Trees(void)
{
	for (int treeidx=0; treeidx < MAX_TREES; treeidx++) {
		delete TreePtr[treeidx];
		TreePtr[treeidx] = NULL;
	}
	NumTrees = 0;
}

/***********************************************************************************************
 * HTreeManagerClass::Free_All_Trees_With_Exclusion_List -- de-allocates all trees not in list *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/12/2002 GH  : Created.                                                                 *
 *=============================================================================================*/
void HTreeManagerClass::Free_All_Trees_With_Exclusion_List(const W3DExclusionListClass & exclusion_list)
{
	// For this system, since it is so simplistic, we simply loop over the array either deleting the tree
	// or copying it to the new tail index if it is excluded.
	int new_tail = 0;

	int treeidx=0;
	for (; treeidx < MAX_TREES; treeidx++) {
		if (TreePtr[treeidx] != NULL) {

			if (exclusion_list.Is_Excluded(TreePtr[treeidx])) {

				//WWDEBUG_SAY(("excluding tree %s",TreePtr[treeidx]->Get_Name()));
				TreePtr[new_tail] = TreePtr[treeidx];
				new_tail++;

			} else {

				//WWDEBUG_SAY(("deleting tree %s",TreePtr[treeidx]->Get_Name()));
				delete TreePtr[treeidx];
				TreePtr[treeidx] = NULL;
			}
		}
	}
	NumTrees = new_tail;
}

/***********************************************************************************************
 * HTreeManagerClass::Load_Tree -- load a hierarchy tree from a file                           *
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
int HTreeManagerClass::Load_Tree(ChunkLoadClass & cload)
{
	WWMEMLOG(MEM_ANIMATION);
	HTreeClass * newtree = W3DNEW HTreeClass;

	if (newtree == NULL) {
		goto Error;
	}

	if (newtree->Load_W3D(cload) != HTreeClass::OK) {

		// load failed, delete and return error
		delete newtree;
		goto Error;

	} else if (Get_Tree_ID(newtree->Get_Name()) != -1) {

		// tree with this name already exists, reject it!
		delete newtree;
		goto Error;

	} else {

		// ok, accept this hierarchy tree!
		TreePtr[NumTrees] = newtree;
		NumTrees++;
	}

	return 0;

Error:

	return 1;

}

/***********************************************************************************************
 * HTreeManagerClass::Get_Tree_ID -- look up the ID of a named hierarchy tree                  *
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
int HTreeManagerClass::Get_Tree_ID(const char * name)
{
	for (int i=0; i<NumTrees; i++) {
		if (TreePtr[i] && (stricmp(name,TreePtr[i]->Get_Name()) == 0)) {
			return i;
		}
	}
	return -1;
}

/***********************************************************************************************
 * HTreeManagerClass::Get_Tree_Name -- look up the name of a id'd hierarchy tree                  *
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
char *HTreeManagerClass::Get_Tree_Name(const int idx)
{
	if ((idx < NumTrees) && TreePtr[idx]) {
		if (TreePtr[idx]) {
			return (char *)TreePtr[idx]->Get_Name();
		}
	}

	return NULL;
}



/***********************************************************************************************
 * HTreeManagerClass::Get_Tree -- get a pointer to the specified hierarchy tree                *
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
HTreeClass * HTreeManagerClass::Get_Tree(const char * name)
{
	for (int i=0; i<NumTrees; i++) {
		if (TreePtr[i] && (stricmp(name,TreePtr[i]->Get_Name()) == 0)) {

			return TreePtr[i];
		}
	}
	return NULL;
}


/***********************************************************************************************
 * HTreeManagerClass::Get_Tree -- get a pointer to the specified hierarchy tree                *
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
HTreeClass * HTreeManagerClass::Get_Tree(int id)
{
	if ((id >= 0) && (id < NumTrees)) {
		return TreePtr[id];
	} else {
		return NULL;
	}
}
