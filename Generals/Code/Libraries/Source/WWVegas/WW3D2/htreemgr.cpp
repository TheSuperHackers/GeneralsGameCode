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
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "htreemgr.h"
#include "htree.h"
#include "WWLib/chunkio.h"
#include "WWDebug/wwmemlog.h"
#include "w3dexclusionlist.h"


HTreeManagerClass::HTreeManagerClass() :
	NumTrees(0)
{
	for (int treeidx=0; treeidx < MAX_TREES; treeidx++) {
		TreePtr[treeidx] = nullptr;
	}
}

HTreeManagerClass::~HTreeManagerClass()
{
	Free();
}

void HTreeManagerClass::Free()
{
	Free_All_Trees();
}

void HTreeManagerClass::Free_All_Trees()
{
	for (int treeidx=0; treeidx < MAX_TREES; treeidx++) {
		delete TreePtr[treeidx];
		TreePtr[treeidx] = nullptr;
	}
	NumTrees = 0;
}

void HTreeManagerClass::Free_All_Trees_With_Exclusion_List(const W3DExclusionListClass & exclusion_list)
{
	// For this system, since it is so simplistic, we simply loop over the array either deleting the tree
	// or copying it to the new tail index if it is excluded.
	int new_tail = 0;

	int treeidx=0;
	for (; treeidx < MAX_TREES; treeidx++) {
		if (TreePtr[treeidx] != nullptr) {

			if (exclusion_list.Is_Excluded(TreePtr[treeidx])) {

				//WWDEBUG_SAY(("excluding tree %s",TreePtr[treeidx]->Get_Name()));
				TreePtr[new_tail] = TreePtr[treeidx];
				new_tail++;

			} else {

				//WWDEBUG_SAY(("deleting tree %s",TreePtr[treeidx]->Get_Name()));
				delete TreePtr[treeidx];
				TreePtr[treeidx] = nullptr;
			}
		}
	}
	NumTrees = new_tail;
}

int HTreeManagerClass::Load_Tree(ChunkLoadClass & cload)
{
	WWMEMLOG(MEM_ANIMATION);
	HTreeClass * newtree = W3DNEW HTreeClass;

	if (newtree == nullptr) {
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

int HTreeManagerClass::Get_Tree_ID(const char * name)
{
	for (int i=0; i<NumTrees; i++) {
		if (TreePtr[i] && (stricmp(name,TreePtr[i]->Get_Name()) == 0)) {
			return i;
		}
	}
	return -1;
}

char *HTreeManagerClass::Get_Tree_Name(const int idx)
{
	if ((idx < NumTrees) && TreePtr[idx]) {
		if (TreePtr[idx]) {
			return (char *)TreePtr[idx]->Get_Name();
		}
	}

	return nullptr;
}



HTreeClass * HTreeManagerClass::Get_Tree(const char * name)
{
	for (int i=0; i<NumTrees; i++) {
		if (TreePtr[i] && (stricmp(name,TreePtr[i]->Get_Name()) == 0)) {

			return TreePtr[i];
		}
	}
	return nullptr;
}


HTreeClass * HTreeManagerClass::Get_Tree(int id)
{
	if ((id >= 0) && (id < NumTrees)) {
		return TreePtr[id];
	} else {
		return nullptr;
	}
}
