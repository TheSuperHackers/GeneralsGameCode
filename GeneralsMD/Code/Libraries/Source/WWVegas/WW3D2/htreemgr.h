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

/* $Header: /Commando/Code/ww3d2/htreemgr.h 2     9/19/01 6:17p Jani_p $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Library                                      *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/htreemgr.h                             $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 9/13/01 7:22p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef HTREEMGR_H
#define HTREEMGR_H

#include "always.h"
#include "bittype.h"
#include "hashtemplate.h"

class FileClass;
class ChunkLoadClass;
class HTreeClass;
class W3DExclusionListClass;
class StringClass;

/*

	HTreeManagerClass

	This class is used to keep track of all of the hierarchy trees.
	A hierarchy tree is the base pose for a hierarchy model.

*/
class HTreeManagerClass
{

public:

	HTreeManagerClass(void);
	~HTreeManagerClass(void);

	int							Load_Tree(ChunkLoadClass & cload);
	int							Num_Trees(void) { return NumTrees; }
	HTreeClass *				Get_Tree(const char * name);
	HTreeClass *				Get_Tree(int id);
	uint32						Get_Tree_Handle(char * name);
	void							Free_All_Trees(void);
	void							Free_All_Trees_With_Exclusion_List(const W3DExclusionListClass & exclusion_list);

	int							Get_Tree_ID(const char * name);
   char *						Get_Tree_Name(const int id);

private:

	enum {
		MAX_TREES = 16000
	};

	void Free(void);

	int							NumTrees;
	HTreeClass *				TreePtr[MAX_TREES];		// TODO: no no! make this dynamic...
	HashTemplateClass<StringClass,HTreeClass*> TreeHash;

};


#endif
