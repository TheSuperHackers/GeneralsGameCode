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
 *                 Project Name : Westwood Library                                             *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/refcount.cpp                           $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/28/01 9:24a                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "refcount.h"

// TheSuperHackers @build feliwir 17/04/2025 include __debugbreak macros
#include <Utility/intrin_compat.h>

#ifdef RTS_DEBUG

// #define PARANOID_REFCOUNTS

/*
** Static variables for the reference counting system
*/
int							RefCountClass::TotalRefs = 0;
RefCountListClass			RefCountClass::ActiveRefList;



RefCountClass *	RefCountClass::Add_Active_Ref(RefCountClass *obj)
{
	ActiveRefList.Add_Head(&(obj->ActiveRefNode));
	obj->ActiveRefInfo.File = nullptr;	// default to no debug information added.
	obj->ActiveRefInfo.Line = 0;
	return obj;
}

RefCountClass *	RefCountClass::Set_Ref_Owner(RefCountClass *obj,const char * file,int line)
{
//	static RefCountClass *hunt = (RefCountClass *)0x06558890;
	static RefCountClass *hunt = (RefCountClass *)0x0;
	if (obj == hunt) {
		assert(0);
	}
	obj->ActiveRefInfo.File = file;
	obj->ActiveRefInfo.Line = line;
	return obj;
}


void RefCountClass::Remove_Active_Ref(RefCountClass * obj)
{
#ifdef PARANOID_REFCOUNTS
	assert(Validate_Active_Ref(obj));
#endif
	obj->ActiveRefNode.Unlink();
}

bool RefCountClass::Validate_Active_Ref(RefCountClass * obj)
{
	RefCountNodeClass *node = ActiveRefList.First();
	while (node) {
		if (node->Get() == obj) return true;
		node = node->Next();
	}
	return false;
}

void	RefCountClass::Inc_Total_Refs(const RefCountClass * obj)
{
#ifdef PARANOID_REFCOUNTS
	assert(Validate_Active_Ref(obj));
#endif
	TotalRefs++;
}

// SKB 7/21/99 Set BreakOnRefernce to a pointer and it will break when called.
//					This is used for debugging, please do not deleted.
RefCountClass* BreakOnReference = 0;

void RefCountClass::Add_Ref() const
{
	NumRefs++;

	// See if programmer set break on for a specific address.
	if (this == BreakOnReference) {
		__debugbreak();  // trigger the debugger
	}
	Inc_Total_Refs(this);
}

void	RefCountClass::Dec_Total_Refs(const RefCountClass * obj)
{
#ifdef PARANOID_REFCOUNTS
	assert(Validate_Active_Ref(obj));
#endif
	TotalRefs--;

	// See if programmer set break on for a specific address.
	if (obj == BreakOnReference) {
		__debugbreak();  // trigger the debugger
	}
}

#endif // RTS_DEBUG
