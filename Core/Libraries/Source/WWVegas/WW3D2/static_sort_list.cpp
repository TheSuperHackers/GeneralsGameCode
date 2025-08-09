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

/***************************************************************************************************
 ***                  C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***************************************************************************************************
 *                                                                                                 *
 *                     Project Name : G                                                            *
 *                                                                                                 *
 *                         $Archive::                                                             $*
 *                                                                                                 *
 *                          Creator::Scott K. Bowen - 7/15/2002                                        *
 *                                                                                                 *
 *                          $Author::                                                             $*
 *                                                                                                 *
 *                         $Modtime::                                                             $*
 *                                                                                                 *
 *                        $Revision::                                                             $*
 *                                                                                                 *
 *-------------------------------------------------------------------------------------------------*
 * Functions:                                                                                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  - - -  - - */

////////////////////////////////////////////////////////////////////////////////////////////////////
// Include files ///////////////////////////////////////////////////////////////////////////////////

#include "static_sort_list.h"

#include "rendobj.h"
#include "dx8renderer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialization Functions ////////////////////////////////////////////////////////////////////////

DefaultStaticSortListClass::DefaultStaticSortListClass(void) :
	StaticSortListClass(),
	SortLists(),
	MinSort(1),
	MaxSort(MAX_SORT_LEVEL)
{
}

DefaultStaticSortListClass::~DefaultStaticSortListClass(void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Virtual functions ///////////////////////////////////////////////////////////////////////////////

void DefaultStaticSortListClass::Add_To_List(RenderObjClass * robj, unsigned int sort_level)
{
	if(sort_level < 1 || sort_level > MAX_SORT_LEVEL) {
		WWASSERT(0);
		return;
	}
	SortLists[sort_level].Add_Tail(robj, false);
}

void DefaultStaticSortListClass::Render_And_Clear(RenderInfoClass & rinfo)
{
	// We go from higher sort level to lower, since lower sort level means higher priority (in
	// front), so lower sort level meshes need to be rendered later.
	for(unsigned int sort_level = MaxSort; sort_level >= MinSort; sort_level--) {
		bool render=false;
		for (	RenderObjClass *robj = SortLists[sort_level].Remove_Head(); robj;
				robj->Release_Ref(), robj = SortLists[sort_level].Remove_Head())
		{
			if (robj->Get_Render_Hook()) {
				if (robj->Get_Render_Hook()->Pre_Render(robj, rinfo)) {
					robj->Render(rinfo);
					render = true;
				}
				robj->Get_Render_Hook()->Post_Render(robj, rinfo);
			} else {
				robj->Render(rinfo);
				render = true;
			}
		}
		if (render) TheDX8MeshRenderer.Flush();
	}
}

