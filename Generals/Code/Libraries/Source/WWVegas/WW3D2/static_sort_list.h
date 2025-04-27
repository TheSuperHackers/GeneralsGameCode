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

/***************************************************************************************************************** 
 ***                      C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S                         *** 
 ***************************************************************************************************************** 
 *                                                                                                               * 
 *                         Project Name : G                                                                      * 
 *                                                                                                               * 
 *                             $Archive::                                                                       $* 
 *                                                                                                               * 
 *                              Creator::Scott K. Bowen - 7/15/2002                                                  *
 *                                                                                                               * 
 *                              $Author::                                                                       $* 
 *                                                                                                               * 
 *                             $Modtime::                                                                       $* 
 *                                                                                                               * 
 *                            $Revision::                                                                       $* 
 *                                                                                                               * 
 *---------------------------------------------------------------------------------------------------------------* 
 * Functions:                                                                                                    * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef STATIC_SORT_LIST_H
#define STATIC_SORT_LIST_H

#include "robjlist.h"
#include "w3d_file.h"

class RenderInfoClass;

// Just defines the interface for the class as used by WW3D..
class StaticSortListClass
{
	public:
		///////////////////////////////////////////////////////////////////////////////////
		// Construction.
		StaticSortListClass(void) {}
		virtual ~StaticSortListClass(void) {}

		virtual void 	Add_To_List(RenderObjClass * robj, unsigned int sort_level) = 0;
		virtual void 	Render_And_Clear(RenderInfoClass & rinfo) = 0;

}; // end StaticSortListClass

// The actual implementation for the standard ww3d StaticSortList.
class DefaultStaticSortListClass : public StaticSortListClass
{
	public:
		///////////////////////////////////////////////////////////////////////////////////
		// Construction.
		DefaultStaticSortListClass(void);
		virtual ~DefaultStaticSortListClass(void);

		virtual void 	Add_To_List(RenderObjClass * robj, unsigned int sort_level);
		virtual void 	Render_And_Clear(RenderInfoClass & rinfo);


		unsigned int 	Get_Min_Sort(void) const 			{return MinSort;};
		unsigned int 	Get_Max_Sort(void) const 			{return MaxSort;};

		void				Set_Min_Sort(unsigned int value)	{MinSort = (value > MAX_SORT_LEVEL) ? MAX_SORT_LEVEL : value;}
		void				Set_Max_Sort(unsigned int value)	{MaxSort = (value > MAX_SORT_LEVEL) ? MAX_SORT_LEVEL : value;}

	private:
		// These are for use by controlling classes to allow control of what levels
		// to render when Render_And_Clear() is called.  As for this class, the values
		// are set to 1..MAX_SORT_LEVEL and then never changed.
		unsigned int				MinSort;
		unsigned int				MaxSort;
 
		// An array of lists - each object in a given list has same SortLevel.
		RefRenderObjListClass 	SortLists[MAX_SORT_LEVEL + 1];

}; // end StaticSortListClass




#endif //STATIC_SORT_LIST_H

