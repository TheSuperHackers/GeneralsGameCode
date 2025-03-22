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
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   RefCountClass::Add_Active_Ref -- add a new referenced object to the list                   *
 *   RefCountClass::Set_Ref_Owner -- update the owner file/line for the given object            *
 *   RefCountClass::Remove_Active_Ref -- remove an object from the active refs list             *
 *   RefCountClass::Validate_Active_Ref -- Confirm a pointer has a node in the active ref list  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "refcount.h"
#include <windows.h>


#ifndef NDEBUG

// #define PARANOID_REFCOUNTS

/*
** Static variables for the reference counting system
*/
int							RefCountClass::TotalRefs = 0;
RefCountListClass			RefCountClass::ActiveRefList;



/***********************************************************************************************
 * RefCountClass::Add_Active_Ref -- add a new referenced object to the list                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/16/98    GTH : Created.                                                                 *
 *=============================================================================================*/
RefCountClass *	RefCountClass::Add_Active_Ref(RefCountClass *obj) 
{ 
	ActiveRefList.Add_Head(&(obj->ActiveRefNode)); 
	obj->ActiveRefInfo.File = NULL;	// default to no debug information added.
	obj->ActiveRefInfo.Line = 0;
	return obj;
}

/***********************************************************************************************
 * RefCountClass::Set_Ref_Owner -- update the owner file/line for the given object              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/16/98    GTH : Created.                                                                 *
 *=============================================================================================*/
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


/***********************************************************************************************
 * RefCountClass::Remove_Active_Ref -- remove an object from the active refs list               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/16/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void RefCountClass::Remove_Active_Ref(RefCountClass * obj) 
{ 
#ifdef PARANOID_REFCOUNTS
	assert(Validate_Active_Ref(obj));
#endif
	obj->ActiveRefNode.Unlink(); 
}

/***********************************************************************************************
 * RefCountClass::Validate_Active_Ref -- Confirm a pointer has a node in the active ref list    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/06/99    EHC: Created.                                                                 *
 *=============================================================================================*/
bool RefCountClass::Validate_Active_Ref(RefCountClass * obj)
{
	RefCountNodeClass *node = ActiveRefList.First();
	while (node) {
		if (node->Get() == obj) return true;
		node = node->Next();
	}
	return false;
}

/***********************************************************************************************
 * RefCountClass::Inc_Total_Refs -- increments the total reference count                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/06/99    EHC: Created.                                                                 *
 *=============================================================================================*/
void	RefCountClass::Inc_Total_Refs(RefCountClass * obj)
{
#ifdef PARANOID_REFCOUNTS
	assert(Validate_Active_Ref(obj));
#endif
	TotalRefs++;

}

// SKB 7/21/99 Set BreakOnRefernce to a pointer and it will break when called.
//					This is used for debugging, please do not deleted.
RefCountClass* BreakOnReference = 0;

#ifndef NDEBUG
void RefCountClass::Add_Ref(void)								
{ 
	NumRefs++;  	  

	// See if programmer set break on for a specific address.
	if (this == BreakOnReference) {
		DebugBreak();  // trigger the debugger
	}
	Inc_Total_Refs(this);
}
#endif																		

/***********************************************************************************************
 * RefCountClass::Validate_Active_Ref -- decrements the total reference count                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/06/99    EHC: Created.                                                                 *
 *=============================================================================================*/
void	RefCountClass::Dec_Total_Refs(RefCountClass * obj)
{
#ifdef PARANOID_REFCOUNTS
	assert(Validate_Active_Ref(obj));
#endif
	TotalRefs--;

	// See if programmer set break on for a specific address.
	if (obj == BreakOnReference) {
		 DebugBreak();  // trigger the debugger
	}
}



#endif


